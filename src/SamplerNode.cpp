/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/

#include "SamplerNode.h"
#include "SamplerCacheData.h"

#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MVectorArray.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MIntArray.h>
#include <maya/MPointArray.h>
#include <maya/MVector.h>
#include <maya/MFloatVector.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MDagPath.h>
#include <maya/MFnTransform.h>
#include <maya/MMatrix.h>
#include <maya/MFnMatrixData.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MGlobal.h>
#include <maya/MFnPluginData.h>

#include <vector>
#include <algorithm>

//////////////////////////////////////////////////////////////////////
//
// Error checking
//
//    MCHECKERROR       - check the status and print the given error message
//    MCHECKERRORNORET  - same as above but does not return
//
//////////////////////////////////////////////////////////////////////

#define MCHECKERROR(STAT,MSG)       \
	if ( MS::kSuccess != STAT ) {   \
	cerr << MSG << endl;        \
	return MS::kFailure;    \
		}

#define MCHECKERRORNORET(STAT,MSG)  \
	if ( MS::kSuccess != STAT ) {   \
	cerr << MSG << endl;        \
		}

// You MUST change this to a unique value!!!  The id is a 32bit value used
// to identify this type of node in the binary file format.  
//
MTypeId     Sampler::id( 0x80099 );

// Attributes
MObject		Sampler::cachePlacement;
MObject		Sampler::nSamples;
MObject     Sampler::inputMesh;        
MObject		Sampler::useVertexCol;
MObject		Sampler::colorSet;
MObject     Sampler::outputSamples;
MObject		Sampler::outputPoints;
MObject		Sampler::outputNormals;
MObject		Sampler::worldToLocal;
MObject		Sampler::samplerCache;

Sampler::Sampler() {}
Sampler::~Sampler() {}

MStatus Sampler::compute( const MPlug& plug, MDataBlock& data )
//
//	Description:
//		This method computes the value of the given output plug based
//		on the values of the input attributes.
//
//	Arguments:
//		plug - the plug to compute
//		data - object that provides access to the attributes for this node
//
{
	MStatus returnStatus;
 
	// Check which output attribute we have been asked to compute.  If this 
	// node doesn't know how to compute it, we must return 
	// MS::kUnknownParameter.
	// 
	if( plug == outputSamples ) {
		MStatus stat;
		// Get a handle to the input attribute that we will need for the
		// computation.  If the value is being supplied via a connection 
		// in the dependency graph, then this call will cause all upstream  
		// connections to be evaluated so that the correct value is supplied.
		// 
		MDataHandle inputMeshHandle = data.inputValue( inputMesh, &returnStatus );
		if ( returnStatus != MS::kSuccess ) return MStatus::kInvalidParameter;
		int numSamples = data.inputValue( nSamples, &returnStatus ).asInt();
		if ( returnStatus != MS::kSuccess ) return MStatus::kInvalidParameter;


		MFnPluginData fnDataCreator;
		MTypeId tmpid(SamplerCacheData::id);
		SamplerCacheData* newData = NULL;
		MDataHandle samplerCacheHandle;

		const bool doCachePlacement = data.inputValue(cachePlacement, &returnStatus).asBool();
		if ( doCachePlacement )
		{ 
			samplerCacheHandle = data.outputValue(samplerCache);
			newData = (SamplerCacheData*)samplerCacheHandle.asPluginData();
		
			if (newData == NULL) {
				// Create some output data
				fnDataCreator.create(tmpid, &stat);
				MCHECKERROR(stat, "compute : error creating SamplerCacheData")
				newData = (SamplerCacheData*)fnDataCreator.data(&stat);
				MCHECKERROR(stat, "compute : error gettin at proxy SamplerCacheData object")
			}
		}

		MFnMesh mesh( inputMeshHandle.asMesh() );
		{					
			MDataHandle outputHandle = data.outputValue( Sampler::outputSamples );
			MFnPointArrayData pointsHandle( outputHandle.child( Sampler::outputPoints ).data() );
			MPointArray points = pointsHandle.array();
			MFnVectorArrayData normalsHandle( outputHandle.child( Sampler::outputNormals ).data() );
			MVectorArray normals = normalsHandle.array();
			bool useVertexColor = data.inputValue( Sampler::useVertexCol ).asBool();
			MString colorSet = data.inputValue( Sampler::colorSet ).asString();
						
			MFnDependencyNode thisNode( thisMObject() );
			MPlug meshPlug = thisNode.findPlug( Sampler::inputMesh );
			MPlugArray connected;
			meshPlug.connectedTo( connected, true, false );
			if ( connected.length() > 0 ) {
				MPlug meshShapePlug = connected[ 0 ];
				MFnMesh meshShapeNode( meshShapePlug.node() );
				MDagPath meshPath;
				meshShapeNode.getPath( meshPath );
				MFnTransform meshTransformNode( meshPath.transform() );
				MMatrix meshTransform = meshPath.inclusiveMatrix();

				MDataHandle world2LocalHandle = data.outputValue( Sampler::worldToLocal );
				MMatrix worldToLocal = meshTransform.inverse();
				MFnMatrixData matrixData;
				MObject matrixDataObject = matrixData.create();
				matrixData.set( worldToLocal );
				world2LocalHandle.set( matrixDataObject );	
			}
		
			SampleMesh(mesh, numSamples, useVertexColor, colorSet, newData, points, normals);

			// Assign the new data to the outputSurface handle

			if (doCachePlacement /*&& 
				newData != samplerCacheHandle.asPluginData()*/) 
			{
				samplerCacheHandle.set(newData);
			}

			// Mark the destination plug as being clean.  This will prevent the
			// dependency graph from repeating this calculation until an input 
			// of this node changes.
			// 
			data.setClean(plug);
		}
	} else {
		return MS::kUnknownParameter;
	}

	return MS::kSuccess;
}

struct triSampling_t {
	int triangle;
	float importance;
};

int ImportanceSort( const void* a, const void* b ) {
	const float importanceA = static_cast< const triSampling_t* >(a)->importance;
	const float importanceB = static_cast< const triSampling_t* >(b)->importance;
	if ( importanceA < importanceB ) {
		return 1;
	} else if( importanceA > importanceB ) {
		return -1;
	} else {
		return 0;
	}
}

void Sampler::SampleMesh(MFnMesh& mesh,
	int numSamples,
	bool useVertexColor,
	const MString& colorSetName,
	SamplerCacheData* samplerCacheData,
	MPointArray& points,
	MVectorArray& normals) {
	points.clear();
	normals.clear();

	MColorArray vertexColors;
	if (useVertexColor) {
		mesh.getVertexColors(vertexColors, &colorSetName);
	}

	MIntArray triangleCounts, triangleVertices;
	mesh.getTriangles(triangleCounts, triangleVertices);
	unsigned int numTriangles = triangleVertices.length() / 3;

	MFloatVectorArray vNormals;
	mesh.getVertexNormals(true, vNormals);

	MPointArray verts;
	mesh.getPoints(verts, MSpace::kWorld);

	std::vector< int >* pTriangleId = NULL;
	std::vector< std::pair<float, float> >* pBarycentricCoord = NULL;
	std::vector<float>* pRNG = NULL;

	bool useSampleCache = samplerCacheData != NULL &&
		samplerCacheData->triangleIds.size() == numTriangles &&
		samplerCacheData->randomNumbers.size() == numSamples;
	
	if (useSampleCache)
	{
		pTriangleId = &samplerCacheData->triangleIds;
		pBarycentricCoord = &samplerCacheData->triangleBarycentricCoords;
		pRNG = &samplerCacheData->randomNumbers;
	}
	else
	{
		// recompute sample placement

		if (samplerCacheData != NULL)
		{			
			pTriangleId = &samplerCacheData->triangleIds;
			pBarycentricCoord = &samplerCacheData->triangleBarycentricCoords;
			pRNG = &samplerCacheData->randomNumbers;
		}
		else
		{
			pTriangleId = new std::vector<int>();
			pBarycentricCoord = new std::vector<std::pair<float, float>>();
			pRNG = new std::vector<float>();
		}

		pTriangleId->reserve(numTriangles);
		pTriangleId->resize(0);
		(*pBarycentricCoord).resize(numSamples);
		memset(&(*pBarycentricCoord)[0], 0, numSamples * sizeof(std::pair<float, float>));

		triSampling_t* triSampling = (triSampling_t*)malloc(numTriangles * sizeof(triSampling_t));

		pRNG->resize(numSamples);
		for (int i = 0; i < numSamples; ++i)
		{
			(*pRNG)[i] = (float)rand() / RAND_MAX;
		}

		for (unsigned int i = 0; i < numTriangles; i++) {
			const int iA = triangleVertices[3 * i + 0];
			const int iB = triangleVertices[3 * i + 1];
			const int iC = triangleVertices[3 * i + 2];
			const MVector AB = verts[iB] - verts[iA];
			const MVector AC = verts[iC] - verts[iA];
			const float area = 0.5f * (float)(AB ^ AC).length();

			float importance = area;
			if (useVertexColor && vertexColors.length() > 0) {
				MColor triCol = vertexColors[iA] + vertexColors[iB] + vertexColors[iC];
				const float third = 1.0f / 3.0f;
				triCol.r *= third;
				triCol.g *= third;
				triCol.b *= third;
				const float lightness = __min(1.0f, __max(0, (triCol.r + triCol.g + triCol.b) * third));
				importance *= lightness;
			}

			triSampling[i].triangle = i;
			triSampling[i].importance = importance;
		}

		qsort(triSampling, numTriangles, sizeof(triSampling_t), ImportanceSort);
		// FIXME: for now ensure numTriangles matches the mesh number of triangles (for the caching)
		while (numTriangles > 0 && triSampling[numTriangles - 1].importance < 1e-5f) { numTriangles--; }
		if (numTriangles == 0) {
			free(triSampling);
			if (samplerCacheData == NULL)
			{
				delete pTriangleId;
				delete pBarycentricCoord;
				delete pRNG;
			}
			return;
		}

		// cumulative probability distribution for faces
		// FIXME: perform search instead of inflating an array of probabilities.
		// normalize sorted areas against the smaller triangle (so smaller importance is 1)
		const float commonDenominator = triSampling[numTriangles - 1].importance;
		for (unsigned int i = 0; i < numTriangles; i++) {
			int area = (int)ceilf(triSampling[i].importance / commonDenominator);
			for (int j = 0; j < area; j++) {
				(*pTriangleId).push_back(triSampling[i].triangle);
			}
		}
		free(triSampling);
	}


	// Sample triangles
	for (int i = 0; i < numSamples; i++) {
		float r = (*pRNG)[i];
		int triId = (*pTriangleId)[ (int)( r * ((*pTriangleId).size()-1))];

		// sample using barycentric coordinates
		float u, v;
		if (useSampleCache)
		{
			u = (*pBarycentricCoord)[i].first;
			v = (*pBarycentricCoord)[i].second;
		}
		else
		{
			do {
				u = (float)rand() / RAND_MAX;
				v = (float)rand() / RAND_MAX;
			} while (u + v > 1);
			if (pBarycentricCoord != NULL)
			{
				(*pBarycentricCoord)[i] = std::pair<float, float>(u, v);
			}
		}
		const int iA = triangleVertices[3 * triId + 0];
		const int iB = triangleVertices[3 * triId + 1];
		const int iC = triangleVertices[3 * triId + 2];
		const MPoint A = verts[iA];
		const MPoint B = verts[iB];
		const MPoint C = verts[iC];
	
		const float w = 1.0f - u - v;
		points.append(A * w + B * u + C * v);
	
		MVector n = vNormals[iA] * w + vNormals[iB] * u + vNormals[iC] * v;
		n.normalize();
		normals.append(n);
	}

	if (samplerCacheData == NULL)
	{
		delete pTriangleId;
		delete pBarycentricCoord;
		delete pRNG;
	}
}

void* Sampler::creator()
//
//	Description:
//		this method exists to give Maya a way to create new objects
//      of this type. 
//
//	Return Value:
//		a new object of this type
//
{
	return new Sampler();
}

MStatus Sampler::initialize()
//
//	Description:
//		This method is called to create and initialize all of the attributes
//      and attribute dependencies for this node type.  This is only called 
//		once when the node type is registered with Maya.
//
//	Return Values:
//		MS::kSuccess
//		MS::kFailure
//		
{
	// This sample creates a single input float attribute and a single
	// output float attribute.
	//
	MFnTypedAttribute	tAttr;
	MFnNumericAttribute nAttr;
	MFnCompoundAttribute cAttr;
	MStatus				stat;

	cachePlacement = nAttr.create("cachePlacement", "cp", MFnNumericData::kBoolean, true, &stat);
	if (!stat) return stat;
	nAttr.setWritable(true);
	nAttr.setStorable(true);

	nSamples = nAttr.create( "sampleCount", "s", MFnNumericData::kInt, 1000, &stat );
	if ( !stat ) return stat;
	nAttr.setMin( 1 );
	nAttr.setWritable( true );
	nAttr.setStorable( true );

	
	inputMesh = tAttr.create( "inputMesh", "m", MFnData::kMesh, MObject::kNullObj, &stat );
	if ( !stat ) return stat;
	tAttr.setWritable( true );
	tAttr.setStorable( false );
	tAttr.setHidden( true );


	useVertexCol = nAttr.create( "useVertexCol", "vc", MFnNumericData::kBoolean, 0, &stat );
	if ( !stat ) return stat;
	nAttr.setWritable( true );
	nAttr.setStorable( true );

	colorSet = tAttr.create( "colorSet", "cs", MFnData::kString, MObject::kNullObj, &stat );
	if ( !stat ) return stat;
	tAttr.setWritable( true );
	tAttr.setStorable( true );

	MFnPointArrayData pCreator;
	MObject pa = pCreator.create();	
	outputPoints = tAttr.create( "outPoints", "osp", MFnData::kPointArray, pa, &stat );
	if ( !stat ) return stat;
	// Attribute is read-only because it is an output attribute
	tAttr.setWritable(false);
	// Attribute will not be written to files when this type of node is stored
	tAttr.setStorable(false);
	tAttr.setHidden( true );

	MFnVectorArrayData nCreator;
	MObject va = nCreator.create();	
	outputNormals = tAttr.create( "outNormals", "osn", MFnData::kVectorArray, va, &stat );
	if ( !stat ) return stat;
	// Attribute is read-only because it is an output attribute
	tAttr.setWritable(false);
	// Attribute will not be written to files when this type of node is stored
	tAttr.setStorable(false);
	//tAttr.setHidden( true );

	outputSamples = cAttr.create( "outSamples", "os", &stat );
	if ( !stat ) return stat;

	MFnCompoundAttribute os(outputSamples );
	os.addChild( outputPoints );
	os.addChild( outputNormals );
	os.setHidden( true );

	worldToLocal = tAttr.create( "worldToLocal", "wtl", MFnData::kMatrix, MObject::kNullObj, &stat );
	if ( !stat ) return stat;
	tAttr.setWritable( false );
	tAttr.setStorable( false );
	tAttr.setHidden( true );

	samplerCache = tAttr.create("samplerCache", "sc", SamplerCacheData::id);
	tAttr.setWritable(false);
	tAttr.setStorable(true);
	tAttr.setHidden(true);

	// Add the attributes we have created to the node
	//

	stat = addAttribute(cachePlacement);
	if (!stat) { stat.perror("addAttribute"); return stat; }
	stat = addAttribute(nSamples);
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( inputMesh );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( useVertexCol );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( colorSet );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( outputSamples );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( worldToLocal );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute(samplerCache);
	if (!stat) { stat.perror("addAttribute"); return stat; }

	// Set up a dependency between the input and the output.  This will cause
	// the output to be marked dirty when the input changes.  The output will
	// then be recomputed the next time the value of the output is requested.
	//
	stat = attributeAffects( nSamples, outputSamples );
	if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( useVertexCol, outputSamples );
	if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( colorSet, outputSamples );
	if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( inputMesh, outputSamples );
	if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( inputMesh, worldToLocal );
	if (!stat) { stat.perror("attributeAffects"); return stat;}

	return MS::kSuccess;

}

