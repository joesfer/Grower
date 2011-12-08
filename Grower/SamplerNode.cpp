/* 
	================================================================================
	Copyright (c) 2010, Jose Esteve. http://www.joesfer.com
	All rights reserved. 

	Redistribution and use in source and binary forms, with or without modification, 
	are permitted provided that the following conditions are met: 

	* Redistributions of source code must retain the above copyright notice, this 
	  list of conditions and the following disclaimer. 
	
	* Redistributions in binary form must reproduce the above copyright notice, 
	  this list of conditions and the following disclaimer in the documentation 
	  and/or other materials provided with the distribution. 
	
	* Neither the name of the organization nor the names of its contributors may 
	  be used to endorse or promote products derived from this software without 
	  specific prior written permission. 

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR 
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
	================================================================================
*/

#include "SamplerNode.h"

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

#include <vector>

// You MUST change this to a unique value!!!  The id is a 32bit value used
// to identify this type of node in the binary file format.  
//
MTypeId     Sampler::id( 0x80099 );

// Attributes
MObject		Sampler::nSamples;
MObject     Sampler::inputMesh;        
MObject		Sampler::useVertexCol;
MObject		Sampler::colorSet;
MObject     Sampler::outputSamples;
MObject		Sampler::outputPoints;
MObject		Sampler::outputNormals;
MObject		Sampler::worldToLocal;

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
		// Get a handle to the input attribute that we will need for the
		// computation.  If the value is being supplied via a connection 
		// in the dependency graph, then this call will cause all upstream  
		// connections to be evaluated so that the correct value is supplied.
		// 
		MDataHandle inputMeshHandle = data.inputValue( inputMesh, &returnStatus );
		if ( returnStatus != MS::kSuccess ) return MStatus::kInvalidParameter;
		int numSamples = data.inputValue( nSamples, &returnStatus ).asInt();
		if ( returnStatus != MS::kSuccess ) return MStatus::kInvalidParameter;

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
		
			SampleMesh( mesh, numSamples, useVertexColor, colorSet, points, normals );

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

void Sampler::SampleMesh( MFnMesh& mesh, int numSamples, bool useVertexColor, const MString& colorSetName, MPointArray& points, MVectorArray& normals ) {
	points.clear();
	normals.clear();
	
	MColorArray vertexColors;	
	if ( useVertexColor ) {
		mesh.getVertexColors( vertexColors, &colorSetName );
	}

	MIntArray triangleCounts, triangleVertices;
	mesh.getTriangles( triangleCounts, triangleVertices );
	unsigned int numTriangles = triangleVertices.length() / 3;
	triSampling_t* triSampling = (triSampling_t*)malloc( numTriangles * sizeof( triSampling_t ) );
	
	MFloatVectorArray vNormals;
	mesh.getVertexNormals( true, vNormals );

	MPointArray verts;
	mesh.getPoints( verts, MSpace::kWorld );
	for( unsigned int i = 0; i < numTriangles; i ++ ) {
		const int iA = triangleVertices[ 3 * i + 0 ];
		const int iB = triangleVertices[ 3 * i + 1 ];
		const int iC = triangleVertices[ 3 * i + 2 ];
		const MVector AB = verts[ iB ] - verts[ iA ];
		const MVector AC = verts[ iC ] - verts[ iA ];
		const float area = 0.5f * (float)( AB ^ AC ).length();

		float importance = area;
		if ( useVertexColor && vertexColors.length() > 0 ) {
			MColor triCol = vertexColors[ iA ] + vertexColors[ iB ] + vertexColors[ iC ];
			const float third = 1.0f / 3.0f;
			triCol.r *= third;
			triCol.g *= third;
			triCol.b *= third;
			const float lightness = __min( 1.0f, __max( 0, ( triCol.r + triCol.g + triCol.b ) * third ) );
			importance *= lightness;
		}

		triSampling[ i ].triangle = i;
		triSampling[ i ].importance = importance;
	}

	qsort( triSampling, numTriangles, sizeof(triSampling_t), ImportanceSort );
	while( numTriangles > 0 && triSampling[ numTriangles - 1 ].importance < 1e-5f ) { numTriangles--; }
	if ( numTriangles == 0 ) {
		free( triSampling );
		return;
	}

	// cumulative probability distribution for faces
	std::vector< int > triangleId;	
	// normalize sorted areas against the smaller triangle (so smaller importance is 1)
	const float commonDenominator = triSampling[ numTriangles - 1 ].importance;
	for( unsigned int i = 0; i < numTriangles; i++ ) {
		int area = (int)ceilf( triSampling[ i ].importance / commonDenominator );
		for( int j = 0; j < area; j++ ) {
			triangleId.push_back( triSampling[ i ].triangle );
		}
	}

	free( triSampling );

	// Sample triangles

	for( int i = 0; i < numSamples; i++ ) {
		float r = (float)rand() / RAND_MAX;
		int triId = triangleId[ (int)( r * ( triangleId.size() - 1 ) ) ];

		// sample using barycentric coordinates
		float u, v;
		do {
			u = (float)rand() / RAND_MAX;
			v = (float)rand() / RAND_MAX;
		} while( u + v > 1 );

		const int iA = triangleVertices[ 3 * triId + 0 ];
		const int iB = triangleVertices[ 3 * triId + 1 ];
		const int iC = triangleVertices[ 3 * triId + 2 ];
		const MPoint A = verts[ iA ];
		const MVector AB = verts[ iB ] - A;
		const MVector AC = verts[ iC ] - A;

		points.append( A + AB * u + AC * v );
		MVector n = vNormals[ iA ] * (1.0f - u - v ) + vNormals[ iB ] * u + vNormals[ iC ] * v; 
		n.normalize();
		normals.append( n );
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

	// Add the attributes we have created to the node
	//
	stat = addAttribute( nSamples );
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

