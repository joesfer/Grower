/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/

#ifndef _GrowerNode
#define _GrowerNode

#include <maya/MPxNode.h>
#include <maya/MTypeId.h> 
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MPxSurfaceShape.h>
#include <vector>

#include "common.h"

class GrowerData;
struct growerNode_t;
struct attractionPointVis_t;

/////////////////////////////////////////////////////////////////////
//
// class Grower
//
//	Constructs the hierarchy data from the sampling points
// 
/////////////////////////////////////////////////////////////////////

class Grower : public MPxNode {
public:
	// overrides

	virtual MStatus	compute( const MPlug& plug, MDataBlock& dataBlock );

	// methods 
	static  void*			creator();
	static  MStatus			initialize();

public:

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	static	MObject		inputSamples;	// input vector array
	static	MObject		inputPoints;	
	static	MObject		inputNormals;

	static	MObject		inputPosition;	// where the growing starts, in world coordinates
	static	MObject		world2Local;	// to transform inputPosition to local coordinates
	static	MObject		searchRadius;
	static	MObject		killRadius;
	static	MObject		growDist;
	static	MObject		maxNeighbors;
	static	MObject		aoMeshData;		// GrowerData
	static	MObject		cacheSolution;	// toggle to cache solution, used to stick grower to moving surfaces

	// The typeid is a unique 32bit identifier that describes this node.
	// It is used to save and retrieve nodes of this type from the binary
	// file format.  If it is not unique, it will cause file IO problems.
	//
	static const MTypeId	id;
	static const MString	typeName;

private: 
	void Grow( const MPointArray& points, 
			   const MVectorArray& normals, 
			   const MPoint& sourcePos, 
			   const float searchRadius, 
			   const float killRadius, 
			   const int maxNeighbors, 
			   const float nodeGrowDist, 
			   bool useCachedSolution,
			   GrowerData* inOutData );
};

#endif
