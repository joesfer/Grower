/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/

#ifndef NearestNeighbors_h__
#define NearestNeighbors_h__

#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MVector.h>
#include <maya/MVectorArray.h>
#include <RenderLib.h>
#pragma comment( lib, "RenderLib.lib" )

class AttractionPoint {
public:
	AttractionPoint() : active(true), closestNode( UINT_MAX ), dist( FLT_MAX ) {}
	virtual ~AttractionPoint() {};
	
	bool		active;
	MPoint		pos;
	MVector		normal;
	size_t		closestNode;
	float		dist;
};

class KdTree {
public:
	KdTree() : pm(NULL) {}
	~KdTree() { delete(pm); }

	bool Init( const MPointArray& points, const MVectorArray& normals );
	size_t NearestNeighbors( const MPoint pos, const float searchRadius, const int maxNeighbors, RenderLib::DataStructures::SampleIndex_t* result );

public:
	RenderLib::DataStructures::PhotonMap* pm;
};

#endif // NearestNeighbors_h__