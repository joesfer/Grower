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

#pragma once

#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MTypeId.h> 
#include <maya/MPxSurfaceShape.h>
#include <maya/MBoundingBox.h>
#include <maya/MPointArray.h>
#include <maya/MPxGeometryData.h>
#include <maya/MTypeId.h>
#include <maya/MString.h>

class MPointArray;


/////////////////////////////////////////////////////////////////////
//
// class SampleShape
//
//	Implements the custom shape generation. A different class, 
//  SampleShapeUI will be in charge of displaying the results 
//  in the viewports.
// 
/////////////////////////////////////////////////////////////////////

class SamplePreviewData;

/* ==========================================
	Class SampleShape

	Helper node used to preview the sample positions.
	Plug a MFnPointArray output attribute from the
	samplers to 'sampleData' and trigger the evaluation
	of 'outData'
   ========================================== */

class SampleShape : public MPxSurfaceShape
{
public:
						SampleShape() {}
	virtual				~SampleShape() {}

	// overrides

	virtual MStatus			compute( const MPlug& plug, MDataBlock& data );

	virtual bool			isBounded() const { return true;}
	virtual MBoundingBox	boundingBox() const { return bounds; }

	virtual MObject			localShapeOutAttr() const { return outData; }
	virtual MObject			geometryData() const;

	// methods

	MObject					meshDataRef();
	SamplePreviewData*		getData();


	static  void*		creator();
	static  MStatus		initialize();
public:
	static const MString typeName;
	static const MTypeId id;

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	static MObject		sampleData;	// input sample data
	static MObject		outData;	// output data

private:
	MBoundingBox		bounds;
	
};


/////////////////////////////////////////////////////////////////////
//
// class SamplePreviewData
//
/////////////////////////////////////////////////////////////////////

class SamplePreviewData : public MPxGeometryData {
public:

	MPointArray& getSamples() { return samples; }

	// overrides 

	virtual	void			copy ( const MPxData& );

	virtual MTypeId         typeId() const { return id; }
	virtual MString         name() const { return typeName; }

	static void * creator();

public:

	static const MString typeName;
	static const MTypeId id;

	MPointArray  samples;
};
