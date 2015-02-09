/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/
#include "GrowerShape.h"
#include "GrowerData.h"
#include "NearestNeighbors.h"

#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MVectorArray.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MIntArray.h>
#include <maya/MPointArray.h>
#include <maya/MVector.h>
#include <maya/MFnPluginData.h>
#include <maya/MGlobal.h>
#include <maya/MFnMeshData.h>
#include <maya/MRampAttribute.h>
#include <iostream>

#include <stack>

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
	std::cerr << MSG << std::endl;        \
	return MS::kFailure;    \
	}

#define MCHECKERRORNORET(STAT,MSG)  \
	if ( MS::kSuccess != STAT ) {   \
	std::cerr << MSG << std::endl;        \
	}

// You MUST change this to a unique value!!!  The typeId is a 32bit value used
// to identify this type of node in the binary file format.  
//
const MTypeId   GrowerShape::id( 0x80098 );
const MString	GrowerShape::typeName( "GrowerShape" );

// Attributes
MObject		GrowerShape::tubeSections;
MObject		GrowerShape::thicknessScale;
MObject		GrowerShape::thickness;
MObject		GrowerShape::inputData;
MObject		GrowerShape::outMesh;

/////////////////////////////////////////////////////////////////////////
// GrowerShape::geometryData (override)
//
// Returns the data object for the surface. This gets
// called internally for grouping (set) information.
//////////////////////////////////////////////////////////////////////////

MObject GrowerShape::geometryData() const {
	return MObject();
	GrowerShape* nonConstThis = const_cast<GrowerShape*>(this);
	MDataBlock datablock = nonConstThis->forceCache();
	MDataHandle handle = datablock.inputValue( outMesh );
	return handle.data();
}

//////////////////////////////////////////////////////////////////////////
// GrowerShape::localShapeOutAttr (override)
//////////////////////////////////////////////////////////////////////////

MObject GrowerShape::localShapeOutAttr() const {
	return outMesh;
}

//////////////////////////////////////////////////////////////////////////
// GrowerShape::isBounded (override)
////////////////////////////////////////////////////////////////////////////

bool GrowerShape::isBounded() const {
	return MeshGeometry() != NULL && MeshGeometry()->hasGeometry(); //otherwise we won't have valid bounds
}

//////////////////////////////////////////////////////////////////////////
// GrowerShape::boundingBox (override)
////////////////////////////////////////////////////////////////////////////

MBoundingBox GrowerShape::boundingBox() const {
	if ( MeshGeometry() != NULL ) {
		return MeshGeometry()->bounds;
	} else {
		MBoundingBox bb;
		bb.clear();
		return bb;
	}
}

//////////////////////////////////////////////////////////////////////////
// GrowerShape::MeshDataRef (override)
//
//	Get a reference to the mesh data (inputData)
//	from the datablock. If dirty then an evaluation is
//	triggered.
////////////////////////////////////////////////////////////////////////////

MObject GrowerShape::MeshDataRef() {
	// Get the datablock for this node
	//
	MDataBlock datablock = forceCache();

	// Calling inputValue will force a recompute if the
	// connection is dirty. This means the most up-to-date
	// mesh data will be returned by this method.
	//
	MDataHandle handle = datablock.inputValue( inputData );
	return handle.data();
}


//////////////////////////////////////////////////////////////////////////
// GrowerShape::MeshGeometry
//
// Returns a pointer to the MeshGeom underlying the GrowerShape.
////////////////////////////////////////////////////////////////////////////

GrowerData* GrowerShape::MeshGeometry() {
	MStatus stat;

	MObject tmpObj = MeshDataRef();
	MFnPluginData fnData( tmpObj );
	GrowerData * data = (GrowerData*)fnData.data( &stat );
	MCHECKERRORNORET( stat, "MeshGeometry : Failed to get MeshData");

	return data;
}

const GrowerData* GrowerShape::MeshGeometry() const {
	MStatus stat;

	MObject tmpObj = const_cast<GrowerShape*>(this)->MeshDataRef();
	MFnPluginData fnData( tmpObj );
	GrowerData * data = (GrowerData*)fnData.data( &stat );
	MCHECKERRORNORET( stat, "MeshGeometry : Failed to get MeshData");

	return data;
}

MStatus GrowerShape::compute( const MPlug& plug, MDataBlock& data )
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
	MStatus stat;
	if( plug == outMesh ) {

		// Convert aoMeshData into a fnMesh readable by Maya
		MDataHandle aoMeshHandle = data.inputValue( inputData, &stat );
		MDataHandle fnMeshHandle = data.outputValue( outMesh, &stat );

		GrowerData* aoMeshData = static_cast< GrowerData* >( aoMeshHandle.asPluginData() );
		if ( aoMeshData == NULL ) {
			std::cerr << "output mesh data not calculated" << std::endl;
			return MS::kFailure;
		}

		if ( aoMeshData->nodes.size() == 0 ) {
			// nothing to mesh
			data.setClean(plug);
			return MS::kSuccess;
		}

		MFnMeshData meshData;
		MObject fnMeshObj = meshData.create();
		MFnMesh fnMesh;
		{
			MPointArray vertexArray;
			MIntArray polygonCounts, indices;
			int tubeSections = data.inputValue( GrowerShape::tubeSections ).asInt();
			float* thicknessArray = (float*)calloc( aoMeshData->nodes.size(), sizeof(float) );
			float thicknessScale = data.inputValue(GrowerShape::thicknessScale).asFloat();
			size_t activeNodes = CalculateThickness(aoMeshData->nodes, thicknessScale, thicknessArray);
			CreateMesh( aoMeshData, activeNodes, tubeSections, thicknessArray, vertexArray, indices, polygonCounts );
			free( thicknessArray );
			const int numQuads = indices.length() / 4;
			fnMesh.create( vertexArray.length(), numQuads, vertexArray, polygonCounts, indices, fnMeshObj );
		}

		fnMeshHandle.set( fnMeshObj );
		data.setClean(plug);
		return MS::kSuccess;
	}
	return MS::kUnknownParameter;
}

void GrowerShape::CreateMesh( const GrowerData* data, const size_t activeNodes, const int tubeSections, const float* thickness, MPointArray& vertices, MIntArray& indices, MIntArray& polygonCounts ) const {

	if ( activeNodes == 0 || tubeSections == 0 ) {
		return;
	}

	enum trimmStatus_e {
		TS_UNVISITED	= 0,
		TS_TRIMMED		= 1,
		TS_ACTIVE		= 2
	};
	short* trimmedNodes = ( short* )calloc( data->nodes.size(), sizeof( short ) );
	int* vertexOffsets = ( int* )malloc( data->nodes.size() * sizeof( int ) );

	size_t remaining = 0;
	for( size_t i = 0; i < data->nodes.size(); i++ ) {
		bool trimmed = data->nodes[ i ].trimmed;
		if ( !trimmed ) {
			size_t parent = data->nodes[ i ].parent;
			if ( parent != INVALID_PARENT ) {
				// by construction of the array, parents are always
				// processed before a child is, so we can rely on
				// the parent node's trimmed value to have been set
				trimmed = ( trimmedNodes[ parent ] != TS_ACTIVE ) ;
				assert( trimmedNodes[ parent ] != TS_UNVISITED );
			}			
		}
		trimmedNodes[ i ] = trimmed ? (short)TS_TRIMMED : (short)TS_ACTIVE;
		if ( !trimmed ) {
			remaining += std::max( (size_t)1, data->nodes[ i ].children.size() );
		}
	}
	//assert( remaining == activeNodes );

	// create vertices
	unsigned int vOffset = 0;
	vertices.setLength( tubeSections * (unsigned int)remaining );
	for( size_t i = 0; i < data->nodes.size(); i++ ) {
		if ( trimmedNodes[ i ] == TS_TRIMMED ) {
			vertexOffsets[ i ] = -1;
			continue;
		}
		const growerNode_t& node = data->nodes[ i ];
		MVector axis;
		if ( node.children.size() > 0 ) {
			size_t thickerChild = 0;
			float largestThickness = 0;
			for( size_t j = 0; j < node.children.size(); j++ ) {
				const growerNode_t& child = data->nodes[ node.children[ j ] ];
				if( thickness[ node.children[ j ] ] > largestThickness ) {
					largestThickness = thickness[ node.children[ j ] ];
					thickerChild = j;

					axis = child.pos - node.pos;
				}
			}
			axis.normalize();
		} else if( node.parent != INVALID_PARENT ) {
			axis = node.pos - data->nodes[ node.parent ].pos;
			axis.normalize();
		} else {
			// isolated node?
			axis = MVector( 1, 0, 0 );
		}

		MMatrix t;
		MVector ox, oy, oz;
		oz = axis;
		oz.normalize();
		ox = oz ^ node.surfaceNormal;
		oy = oz ^ ox;

		const float thick = thickness [ i ];

		t[ 0 ][ 0 ] = ox.x;	t[ 0 ][ 1 ] = ox.y;	t[ 0 ][ 2 ] = ox.z;	t[ 0 ][ 3 ] = 0; 
		t[ 1 ][ 0 ] = oy.x;	t[ 1 ][ 1 ] = oy.y;	t[ 1 ][ 2 ] = oy.z;	t[ 1 ][ 3 ] = 0; 
		t[ 2 ][ 0 ] = oz.x;	t[ 2 ][ 1 ] = oz.y;	t[ 2 ][ 2 ] = oz.z;	t[ 2 ][ 3 ] = 0; 
		t[ 3 ][ 0 ] = node.pos.x + node.surfaceNormal.x * thick; 
		t[ 3 ][ 1 ] = node.pos.y + node.surfaceNormal.y * thick; 
		t[ 3 ][ 2 ] = node.pos.z + node.surfaceNormal.z * thick; 
		t[ 3 ][ 3 ] = 1;
		float radStep = 2.0f * 3.141592f / tubeSections;
		for( unsigned int k = 0; k < std::max( (unsigned int)1, (unsigned int)node.children.size() ); k++ ) {
			float angle = 0;
			for( unsigned int j = 0; j < (unsigned int)tubeSections; j++ ) {
				MPoint p( thick * cos( angle ), thick * sin( angle ), 0 );
				vertices[ vOffset + tubeSections * k + j ] = p * t;
				angle += radStep;
			}
		}

		vertexOffsets[ i ] = vOffset;
		vOffset += tubeSections * std::max( (unsigned int)1, (unsigned int)node.children.size() );
	}

	assert( vOffset == remaining * tubeSections );
	free( trimmedNodes );

	// create triangles
	const unsigned int numTris = 2 * tubeSections * ((unsigned int)activeNodes - 1 ); // do not count the root node (as we generate triangles towards it, but not from it)
	indices.setLength( 2 * numTris );
	unsigned int offset = 0;
	for( int i = 1; i < (int)data->nodes.size(); i++ ) {
		if ( vertexOffsets[ i ] == -1 ) {
			continue;
		}

		const growerNode_t& node = data->nodes[ i ];
		assert( node.parent != INVALID_PARENT );
		const growerNode_t& parent = data->nodes[ node.parent ];
		unsigned int childIdx = 0;
		for( ; ; childIdx++ ) {
			if( parent.children[ childIdx ] == i ) {
				break;
			}
		}
		const int vertexOffsetA = vertexOffsets[ node.parent ] + tubeSections * childIdx;
		const int vertexOffsetB = vertexOffsets[ i ];
		for( int j = 0; j < tubeSections; j++ ) {
			assert( vertexOffsetA + j < tubeSections * (int)remaining );
			assert( vertexOffsetA + j + ( j + 1 ) % tubeSections < tubeSections * (int)remaining );

			indices[ offset++ ] = vertexOffsetA + j;
			indices[ offset++ ] = vertexOffsetA + ( j + 1 ) % tubeSections;
			indices[offset++] = vertexOffsetB + (j + 1) % tubeSections;
			indices[offset++] = vertexOffsetB + j;
		}
	}
	assert( offset == 2 * numTris );
	// 
	polygonCounts.setLength( (int)offset / 4 );
	for( unsigned int i = 0; i < offset / 4; i++ ) {
		polygonCounts[ i ] = 4;		
	}

	
	free( vertexOffsets );

}

size_t GrowerShape::CalculateThickness(std::vector< growerNode_t >& nodes, float thicknessScale, float* thicknessArray) {
	
	// calculate branch thickness. This is a recursive process where 
	// thickness( node_i ) = function( thickness( child0(node_i) ), thickness( child0(node_i) ), ... )
	// but let's not perform recursive function calls as we can easily blow up the stack

	size_t activeNodes = 0;
	const float baseThickness = 1.f;

	MRampAttribute thicknessRemapping(thisMObject(), thickness);
	
	std::vector< size_t > terminators;
	std::stack< size_t > recursion;
	recursion.push( 0 );
	while( !recursion.empty() ) {
		size_t node = recursion.top();

		bool calculated = false;

		if ( nodes[ node ].children.size() == 0 || 
			( nodes[ node].children.size() == 1 && nodes[ nodes[ node ].children[ 0 ] ].trimmed ) ) {
			terminators.push_back( node );
		}

		if ( nodes[ node ].children.size() == 0 || nodes[ node ].trimmed ) {			
			calculated = true;			
		} else {
			size_t childrenReady = 0;
			for( size_t i = 0; i < nodes[ node ].children.size(); i++ ) {
				if ( thicknessArray[ nodes[ node ].children[ i ] ] >= baseThickness ) {
					childrenReady++;
				} else {
					break;
				}
			}
			calculated = ( childrenReady == nodes[ node ].children.size() );
		}

		if ( calculated ) {
			recursion.pop();
			
			if ( !nodes[ node ].trimmed ) {
				activeNodes++;
			}

			if ( nodes[ node ].children.size() == 0 || nodes[ node ].trimmed ) {
				thicknessArray[ node ] = baseThickness;
			} else {
				float sqRadius = 0;
				for( size_t i = 0; i < nodes[ node ].children.size(); i++ ) {
					const float t = thicknessArray[ nodes[ node ].children[ i ] ];
					sqRadius += t * t;
				}
				thicknessArray[ node ] = sqrtf( sqRadius );
			}

		} else {
			for( size_t i = 0; i < nodes[ node ].children.size(); i++ ) {
				const size_t child = nodes[ node ].children[ i ];
				if ( thicknessArray[ child ] < baseThickness ) {
					recursion.push( child );
				}
			}
		}
	}

	// now force the terminator nodes to have a thickness of 0 so they end in a spike
	for( size_t i = 0; i < terminators.size(); i++ ) {
		thicknessArray[ terminators[ i ] ] = 0.0001f;
	}

	// track down the bifurcations, for each single-child node path, interpolate
	// the nodes thickness to smooth out appearance
	
	for( size_t i = 0; i < nodes.size(); i++ ) {
		growerNode_t& node = nodes[ i ];
		if ( node.children.size() > 0 ) {
			for( size_t j = 0; j < node.children.size(); j++ ) {
				size_t start = i;
				size_t finish = i;
				size_t pathLength = 0;
				while( nodes[ finish ].children.size() == 1 ) {
					finish = nodes[ finish ].children[ 0 ];
					pathLength++;
				}
				if ( pathLength > 1 ) {
					const float startThickness = thicknessArray[ start ];
					const float finishThickness = thicknessArray[ finish ];
					const float delta = ( finishThickness - startThickness ) / pathLength;
					if ( delta < 0.001f ) {
						// not worth it
						break;
					}
					size_t k = 0;
					finish = nodes[ finish ].parent; // avoid reaching the node which numChildren != 1 (could be 0!)
					while( start != finish ) {
						thicknessArray[ start ] += delta * k;
						k++;
						start = nodes[ k ].children[ 0 ];
					}
				}
			}
		}
	}
	
	float maxThickness = 0;
	float minThickness = FLT_MAX;
	for (size_t i = 0; i < nodes.size(); i++) {
		const float thickness = thicknessArray[i];
		maxThickness = std::max(maxThickness, thickness);
		minThickness = std::min(minThickness, thickness);
	}
	minThickness = std::min(minThickness, maxThickness);
	maxThickness = std::max(maxThickness, minThickness + 1e-8f);

	for (size_t i = 0; i < nodes.size(); i++) {
		float normalizedThickness = (thicknessArray[i] - minThickness) / (1e-8f + maxThickness - minThickness);
		float inputThickness = normalizedThickness * thicknessScale;
		float remappedThickness;
		// the 1.0f - X is because it's more intuitive to see the curve from thick to thin, instead of the natural order thin (0) to thick (1)
		thicknessRemapping.getValueAtPosition(std::max(0.f, std::min(1.f, 1.0f - inputThickness)), remappedThickness);
		thicknessArray[i] = remappedThickness;
	}

	return activeNodes;
}

void* GrowerShape::creator()
//
//	Description:
//		this method exists to give Maya a way to create new objects
//      of this type. 
//
//	Return Value:
//		a new object of this type
//
{
	return new GrowerShape();
}

MStatus GrowerShape::initialize()
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
	
	MFnNumericAttribute	nFn;
	MFnTypedAttribute	typedFn;	
	MStatus				stat;

	tubeSections = nFn.create( "tubeSections", "ts", MFnNumericData::kInt, 8 );
	nFn.setWritable( true );
	nFn.setReadable( true );
	nFn.setStorable( true );
	nFn.setMin( 3 );
	nFn.setSoftMax( 24 );

	thicknessScale = nFn.create("thicknessScale", "ths", MFnNumericData::kFloat, 1.0f);
	nFn.setWritable(true);
	nFn.setReadable(true);
	nFn.setStorable(true);
	nFn.setMin(1e-4f);
	nFn.setSoftMax(2.f);

	thickness = MRampAttribute::createCurveRamp("thickness", "th");

	inputData = typedFn.create( "input", "in", GrowerData::id );
	typedFn.setWritable( true );
	typedFn.setReadable( true );
	typedFn.setStorable(false);
	typedFn.setHidden( true );

	outMesh = typedFn.create( "outMesh", "out", MFnData::kMesh );
	typedFn.setStorable(false);
	typedFn.setWritable(false);

	// Add the attributes we have created to the node
	//
	stat = addAttribute( tubeSections );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute(thicknessScale);
	if (!stat) { stat.perror("addAttribute"); return stat; }
	stat = addAttribute( thickness );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( inputData );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( outMesh );
	if (!stat) { stat.perror("addAttribute"); return stat;}

	attributeAffects( tubeSections, outMesh );
	attributeAffects( thicknessScale, outMesh );
	attributeAffects( thickness, outMesh );
	attributeAffects( inputData,	outMesh );

	return MS::kSuccess;

}
