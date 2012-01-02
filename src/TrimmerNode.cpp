/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/

#include "TrimmerNode.h"
#include "GrowerData.h"

#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>

// You MUST change this to a unique value!!!  The id is a 32bit value used
// to identify this type of node in the binary file format.  
//
MTypeId     Trimmer::id( 0x80096 );

// Attributes
MObject		Trimmer::maxLength;
MObject     Trimmer::inputData;        
MObject     Trimmer::outputData;

Trimmer::Trimmer() {}
Trimmer::~Trimmer() {}

MStatus	Trimmer::connectionMade( const MPlug& plug, const MPlug& otherPlug, bool asSrc ) {
	//if ( ( plug == inputData ) && ( asSrc == false ) ) {
	//	MStatus stat;
	//	MObject thisObj = thisMObject();
	//	MFnDependencyNode node( thisObj );
	//	MFnNumericAttribute maxLength( node.attribute( "maxLength" ) );

	//	//maxLength.setMax()
	//}
	return MPxNode::connectionMade( plug, otherPlug, asSrc );
}

MStatus Trimmer::compute( const MPlug& plug, MDataBlock& data )
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
	if ( plug == outputData ) {
		MStatus stat;

		MDataHandle inDataHandle = data.inputValue( Trimmer::inputData, &stat );
		MDataHandle outDataHandle = data.outputValue( Trimmer::outputData, &stat );

		GrowerData* growerData = static_cast< GrowerData* >( inDataHandle.asPluginData() );
		if ( growerData == NULL ) {
			cerr << "Trimmer: error retrieving data" << endl;
			return MS::kFailure;
		}

		int maxDepth = GetMaxDepth( growerData->nodes );
		int length = (int)ceilf( (float)maxDepth * data.inputValue( Trimmer::maxLength ).asFloat() ) + 1;

		Trim( growerData->nodes, length );

		outDataHandle.setMPxData( growerData );
		data.setClean( plug );
		return MS::kSuccess;
	}

	return MS::kUnknownParameter;
}

int Trimmer::GetMaxDepth( const std::vector< growerNode_t >& nodes ) const {
	int depth = 0;
	std::vector< size_t > nodeList[ 2 ];
	nodeList[ 0 ].push_back( 0 );
	while( !nodeList[ depth % 2 ].empty() ) {
		std::vector< size_t >& activeNodes = nodeList[ depth % 2 ];
		std::vector< size_t >& activeChildren = nodeList[ ( depth % 2 ) ^ 1 ];

		activeChildren.resize( 0 );
		for( size_t i = 0; i < activeNodes.size(); i++ ) {
			const growerNode_t& node = nodes[ activeNodes[ i ] ];
			for( size_t j = 0; j < node.children.size(); j++ ) {
				activeChildren.push_back( node.children[ j ] );
			}
		}

		depth ++;		
	}
	return depth;
}

void Trimmer::Trim( std::vector< growerNode_t >& nodes, const int maxLength ) {
	size_t depth = 0;
	std::vector< size_t > nodeList[ 2 ];
	nodeList[ 0 ].push_back( 0 );
	while( !nodeList[ depth % 2 ].empty() ) {
		std::vector< size_t >& activeNodes = nodeList[ depth % 2 ];
		std::vector< size_t >& activeChildren = nodeList[ ( depth % 2 ) ^ 1 ];

		activeChildren.resize( 0 );
		for( size_t i = 0; i < activeNodes.size(); i++ ) {
			growerNode_t& node = nodes[ activeNodes[ i ] ];
			node.trimmed = ( depth == maxLength );
			for( size_t j = 0; j < node.children.size(); j++ ) {
				activeChildren.push_back( node.children[ j ] );
			}
		}

		depth ++;
		if ( depth > (size_t)maxLength ) {
			// don't keep going deeper, the current active nodes have been marked as trimmed
			// and will stop the meshing
			break;
		}
	}
}

void* Trimmer::creator()
//
//	Description:
//		this method exists to give Maya a way to create new objects
//      of this type. 
//
//	Return Value:
//		a new object of this type
//
{
	return new Trimmer();
}

MStatus Trimmer::initialize()
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
	MStatus				stat;

	maxLength = nAttr.create( "percentLength", "l", MFnNumericData::kFloat, 1.0f, &stat );
	if ( !stat ) return stat;
	nAttr.setMin( 0 );
	nAttr.setMax( 1.0f );
	nAttr.setWritable( true );
	nAttr.setStorable( true );

	inputData = tAttr.create( "input", "in", GrowerData::id, MObject::kNullObj, &stat );
	if ( !stat ) return stat;
	tAttr.setWritable( true );
	tAttr.setReadable( true );
	tAttr.setStorable(false);
	tAttr.setHidden( true );

	outputData = tAttr.create( "output", "out", GrowerData::id, MObject::kNullObj, &stat );
	if ( !stat ) return stat;
	tAttr.setWritable( false );
	tAttr.setReadable( true );
	tAttr.setStorable(false);

	// Add the attributes we have created to the node
	//
	stat = addAttribute( maxLength );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( inputData );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( outputData );
	if (!stat) { stat.perror("addAttribute"); return stat;}

	// Set up a dependency between the input and the output.  This will cause
	// the output to be marked dirty when the input changes.  The output will
	// then be recomputed the next time the value of the output is requested.
	//
	stat = attributeAffects( maxLength, outputData );
	if (!stat) { stat.perror("attributeAffects"); return stat;}
	stat = attributeAffects( inputData, outputData );
	if (!stat) { stat.perror("attributeAffects"); return stat;}

	return MS::kSuccess;

}

