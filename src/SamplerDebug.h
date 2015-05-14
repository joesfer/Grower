/*
================================================================================
Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html
================================================================================
*/

#pragma once

#include <maya/MPxLocatorNode.h>
#include <maya/MPointArray.h>

/////////////////////////////////////////////////////////////////////
//
// class MesherUI
//
//	Implements the part in charge of drawing the shape in the 
//	viewports
// 
/////////////////////////////////////////////////////////////////////

class SamplerDebug : public MPxLocatorNode
{
public:
	SamplerDebug() {}
	virtual ~SamplerDebug() {}

	/////////////////////////////////////////////////////////////////////
	//
	// Overrides
	//
	/////////////////////////////////////////////////////////////////////

	
	virtual void draw(M3dView &view, const MDagPath &path, M3dView::DisplayStyle style, M3dView::DisplayStatus);
	virtual bool isBounded() const { return false; }

	static  void *      creator();
	static  MStatus			initialize();

	static	MObject		inputSamples;	// input vector array
	static	MObject		inputPoints;
	static	MObject		inputNormals;

	// The typeid is a unique 32bit identifier that describes this node.
	// It is used to save and retrieve nodes of this type from the binary
	// file format.  If it is not unique, it will cause file IO problems.
	//
	static const MTypeId	id;
	static const MString	typeName;
};
