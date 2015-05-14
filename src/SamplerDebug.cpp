/*
================================================================================
Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html
================================================================================
*/


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
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnMatrixData.h>

#include "SamplerDebug.h"

// You MUST change this to a unique value!!!  The typeId is a 32bit value used
// to identify this type of node in the binary file format.  
//
const MTypeId   SamplerDebug::id(0x80092);
const MString	SamplerDebug::typeName("SamplerDebug");

// Attributes
MObject		SamplerDebug::inputSamples;
MObject		SamplerDebug::inputPoints;
MObject		SamplerDebug::inputNormals;

void *  SamplerDebug::creator()
{
	return new SamplerDebug();
}


MStatus SamplerDebug::initialize()
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
	MFnTypedAttribute	typedFn;
	MFnCompoundAttribute cFn;
	MStatus				stat;

	inputPoints = typedFn.create("samplesPoints", "sp", MFnData::kPointArray);
	typedFn.setStorable(false);
	typedFn.setWritable(true);

	inputNormals = typedFn.create("samplesNormals", "sn", MFnData::kVectorArray);
	typedFn.setStorable(false);
	typedFn.setWritable(true);

	inputSamples = cFn.create("samples", "s");
	cFn.setWritable(true);
	cFn.addChild(inputPoints);
	cFn.addChild(inputNormals);
	cFn.setHidden(true);

	// Add the attributes we have created to the node
	//
	stat = addAttribute(inputSamples);
	if (!stat) { stat.perror("addAttribute"); return stat; }
	
	return MS::kSuccess;

}

void SamplerDebug::draw(M3dView &view, const MDagPath &path, M3dView::DisplayStyle style, M3dView::DisplayStatus)
{
	MObject thisNode = thisMObject();
	MPlug p = MPlug(thisNode, inputPoints);
	MObject cvObject;
	MStatus stat;
	stat = p.getValue(cvObject);

	MFnPointArrayData pointVecData(cvObject);
	MPointArray pointVec = pointVecData.array();

	view.beginGL();
	glBegin(GL_POINTS);
	for (unsigned int i = 0; i < pointVec.length(); ++i)
	{
		glVertex3f((float)pointVec[i].x, (float)pointVec[i].y, (float)pointVec[i].z);
	}
	glEnd();
	view.endGL();
}
