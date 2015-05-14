/* 
	================================================================================
	Copyright (c) 2011, Jose Esteve. http://www.joesfer.com
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

#include "SamplePreviewShape.h"

#include <assert.h>

#include <math.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>

#include <maya/MGlobal.h>

#include <maya/MFnPluginData.h>         
#include <maya/MFnTypedAttribute.h>
#include <maya/MPointArray.h>
#include <maya/MFnPointArrayData.h>

const MTypeId SampleShape::id(0x81002);
const MString SampleShape::typeName( "SamplePreview" );
const MTypeId SamplePreviewData::id(0x80299);
const MString SamplePreviewData::typeName( "SamplePreviewData" );

MObject     SampleShape::sampleData;
MObject     SampleShape::outData;

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


//////////////////////////////////////////////////////////////////////////
// SampleShape::geometryData (override)
//
// Returns the data object for the surface. This gets
// called internally for grouping (set) information.
//////////////////////////////////////////////////////////////////////////

MObject SampleShape::geometryData() const {
	SampleShape* nonConstThis = const_cast<SampleShape*>(this);
	MDataBlock datablock = nonConstThis->forceCache();
	MDataHandle handle = datablock.inputValue( outData );
	return handle.data();
}


//////////////////////////////////////////////////////////////////////////
// SampleShape::compute (override)
//
//	Description:
//		This method computes the value of the given output plug based
//		on the values of the input attributes.
//
//	Arguments:
//		plug - the plug to compute
//		data - object that provides access to the attributes for this node
////////////////////////////////////////////////////////////////////////////

MStatus SampleShape::compute( const MPlug& plug, MDataBlock& data ) {
	MStatus stat;

	// Check which output attribute we have been asked to compute.  If this 
	// node doesn't know how to compute it, we must return 
	// MS::kUnknownParameter.
	// 
	if( plug == outData )
	{
		// Get a handle to the input attribute that we will need for the
		// computation.  If the value is being supplied via a connection 
		// in the dependency graph, then this call will cause all upstream  
		// connections to be evaluated so that the correct value is supplied.
		// 
		MDataHandle inputDataHandle = data.inputValue( sampleData, &stat );

		MObject inputDataObj = inputDataHandle.data();
		
		MFnPluginData fnDataCreator;
		MTypeId tmpid( SamplePreviewData::id );
		SamplePreviewData * newData = NULL;

		MDataHandle outHandle = data.outputValue( outData );	
		newData = (SamplePreviewData*)outHandle.asPluginData();

		if ( newData == NULL ) {
			// Create some output data
			fnDataCreator.create( tmpid, &stat );
			MCHECKERROR( stat, "compute : error creating SamplePreviewData")
				newData = (SamplePreviewData*)fnDataCreator.data( &stat );
			MCHECKERROR( stat, "compute : error getting proxy SamplePreviewData object")
		}

		// compute the output values			
		MFnPointArrayData inputData;
		inputData.setObject(inputDataObj);
		newData->samples = inputData.array();

		// compute bounding box for fast retrieval
		bounds.clear();			
		for( unsigned int i = 0; i < newData->samples.length(); i += 2 ) {
			bounds.expand( newData->samples[ i ] );
		}
		
		// Assign the new data to the outputSurface handle
		if ( newData != outHandle.asPluginData() ) {
			outHandle.set( newData );
		}

	} else {
		return MS::kUnknownParameter;
	}

	data.setClean( plug );

	return MS::kSuccess;
}


//////////////////////////////////////////////////////////////////////////
// SampleShape::meshDataRef
//
//	Get a reference to the mesh data (aoMeshData)
//	from the datablock. If dirty then an evaluation is
//	triggered.
////////////////////////////////////////////////////////////////////////////

MObject SampleShape::meshDataRef() {
	// Get the datablock for this node
	//
	MDataBlock datablock = forceCache();

	// Calling inputValue will force a recompute if the
	// connection is dirty. This means the most up-to-date
	// mesh data will be returned by this method.
	//
	MDataHandle handle = datablock.inputValue( outData );
	return handle.data();
}



//////////////////////////////////////////////////////////////////////////
// SampleShape::getData
//
// Returns a pointer to an updated version of the internal data
////////////////////////////////////////////////////////////////////////////

SamplePreviewData* SampleShape::getData() {
	MStatus stat;
	
	MObject tmpObj = meshDataRef();
	MFnPluginData fnData( tmpObj );
	SamplePreviewData* data = (SamplePreviewData*)fnData.data( &stat );
	MCHECKERRORNORET( stat, "getData : Failed to get SamplePreviewData");

	return data;
}


//////////////////////////////////////////////////////////////////////////
// SampleShape::creator
//
//	This method exists to give Maya a way to create new objects
//	of this type. 
////////////////////////////////////////////////////////////////////////////

void* SampleShape::creator() {
	return new SampleShape();
}

//////////////////////////////////////////////////////////////////////////
// SampleShape::creator
//
//	This method is called to create and initialize all of the attributes
//	and attribute dependencies for this node type.  This is only called
//	once when the node type is registered with Maya.
////////////////////////////////////////////////////////////////////////////

MStatus SampleShape::initialize() {
	MFnTypedAttribute	typedAttr;
	MFnNumericAttribute nAttr;
	MStatus				stat;

	// Input attributes

	MPointArray defaultPointArray;
	MFnPointArrayData pointArrayDataFn;
	pointArrayDataFn.create( defaultPointArray );

	sampleData = typedAttr.create( "sampleData", "sd", MFnData::kPointArray, pointArrayDataFn.object() );
	typedAttr.setWritable( true );
	typedAttr.setReadable( true );

	outData = typedAttr.create( "output", "out", SamplePreviewData::id );
	typedAttr.setWritable( false );
	typedAttr.setStorable(false);
	typedAttr.setHidden( true );

	// Add the attributes to the node

	addAttribute( sampleData );
	addAttribute( outData );

	// Set the attribute dependencies
	attributeAffects( sampleData, outData );
	
	return MS::kSuccess;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// SamplePreviewData::creator
//
//	This method exists to give Maya a way to create new objects
//	of this type. 
////////////////////////////////////////////////////////////////////////////

void* SamplePreviewData::creator() {
	return new SamplePreviewData();
}


//////////////////////////////////////////////////////////////////////////
// SamplePreviewData::copy
////////////////////////////////////////////////////////////////////////////

void SamplePreviewData::copy( const MPxData& other ) {
	if ( other.typeId() == typeId() ) {
		*this = (const SamplePreviewData&)other;
	}
}
