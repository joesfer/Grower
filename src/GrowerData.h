/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/
#ifndef GrowerData_h__
#define GrowerData_h__

#include <maya/MPxGeometryData.h>
#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MPointArray.h>
#include <maya/MBoundingBox.h>
#include <maya/MPointArray.h>
#include <vector>

#include "common.h"
#include "NearestNeighbors.h"

#define INVALID_PARENT	1 << 30
struct growerNode_t {

	growerNode_t() : parent( INVALID_PARENT ), trimmed( false ) {}

	MPoint					pos;
	MVector					surfaceNormal;
	size_t					parent : 31;
	bool					trimmed : 1;
	std::vector< size_t >	children;
};

#if GROWER_DISPLAY_DEBUG_INFO
// Just used to preview the attraction points
struct attractionPointVis_t {
	MPoint	pos;
	bool	active;
};
#endif

/////////////////////////////////////////////////////////////////////
//
// class GrowerData
//
/////////////////////////////////////////////////////////////////////

class GrowerData : public MPxGeometryData {

public:
	//////////////////////////////////////////////////////////////////
	//
	// Overrides from MPxData
	//
	//////////////////////////////////////////////////////////////////
	GrowerData();
	virtual					~GrowerData();

	virtual	void			copy ( const MPxData& );

	virtual MTypeId         typeId() const;
	virtual MString         name() const;

	//////////////////////////////////////////////////////////////////
	//
	// Helper methods
	//
	//////////////////////////////////////////////////////////////////

	static void *	creator();

	bool			hasGeometry() const { return nodes.size() > 0; }

public:
	static const MString typeName;
	static const MTypeId id;

#if GROWER_DISPLAY_DEBUG_INFO
	std::vector< attractionPointVis_t > samples;
#endif
	std::vector< growerNode_t > nodes;
	MBoundingBox bounds;

	// cache data
	std::vector< std::vector<RenderLib::DataStructures::SampleIndex_t> > m_cachedAffectedPoints;
	std::vector< std::vector<RenderLib::DataStructures::SampleIndex_t> > m_cachedClosestNode;
	std::vector< std::vector<RenderLib::DataStructures::SampleIndex_t> > m_cachedBannedAliveNodes;
	std::vector< std::vector<bool> >									 m_cachedActiveAttractors;

	float m_cachedSearchRadius;
	float m_cachedKillRadius;
	int	  m_cachedNumNeighbours;
	float m_cachedNodeGrowDist;

};
#endif // GrowerData_h__
