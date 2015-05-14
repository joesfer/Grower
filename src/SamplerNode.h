/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/

#ifndef _SamplerNode
#define _SamplerNode

#include <maya/MPxNode.h>
#include <maya/MTypeId.h> 
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MVectorArray.h>

class SamplerCacheData;

class Sampler : public MPxNode
{
public:
						Sampler();
	virtual				~Sampler(); 

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	static  void*		creator();
	static  MStatus		initialize();

public:

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	static  MObject		cachePlacement; // whether to stick samples to surface by caching previously generated solution until topology changes.
	static  MObject		nSamples;		// number of desired samples
	static	MObject		inputMesh;		// input mesh to sample
	static	MObject		useVertexCol;	// use vertex color to determine where to sample
	static	MObject		colorSet;
	static  MObject		outputSamples;	// output samples array
	static	MObject		outputPoints;	// child of outputSamples
	static	MObject		outputNormals;	// child of outputSamples
	static	MObject		worldToLocal;	// output copy of mesh transform

	static  MObject		samplerCache;	// SamplerCacheData

	// The typeid is a unique 32bit identifier that describes this node.
	// It is used to save and retrieve nodes of this type from the binary
	// file format.  If it is not unique, it will cause file IO problems.
	//
	static	MTypeId		id;

private:
	void SampleMesh( MFnMesh& mesh, 
					 int numSamples, 
					 bool vertexColor, 
					 const MString& colorSetName, 
					 bool doCachePlacement,
					 SamplerCacheData* samplerCacheData,
					 MPointArray& points,
					 MVectorArray& normals );
};

#endif
