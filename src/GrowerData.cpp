/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/

#include "GrowerData.h"

const MTypeId GrowerData::id( 0x80777 );
const MString GrowerData::typeName( "GrowerData" );

//////////////////////////////////////////////////////////////////////////
// GrowerData::GrowerData()
//////////////////////////////////////////////////////////////////////////

GrowerData::GrowerData() {
	m_cachedSearchRadius = -1;
	m_cachedKillRadius = -1;
	m_cachedNumNeighbours = -1;
	m_cachedNodeGrowDist = -1;
}

//////////////////////////////////////////////////////////////////////////
// GrowerData::~GrowerData()
//////////////////////////////////////////////////////////////////////////

GrowerData::~GrowerData() {
}

//////////////////////////////////////////////////////////////////////////
// GrowerData::copy (override)
//////////////////////////////////////////////////////////////////////////

void GrowerData::copy ( const MPxData& other ) {
	if ( &other != this ) {
		const GrowerData& _other = (const GrowerData &)other;
		bounds      = _other.bounds;
		nodes		= _other.nodes;
#if GROWER_DISPLAY_DEBUG_INFO
		samples		= _other.samples;
#endif
	}
}

//////////////////////////////////////////////////////////////////////////
// GrowerData::typeId (override)
//
//	Binary tag used to identify this kind of data
//////////////////////////////////////////////////////////////////////////

MTypeId GrowerData::typeId() const {
	return GrowerData::id;
}

//////////////////////////////////////////////////////////////////////////
// GrowerData::typeId (override)
//
//	String name used to identify this kind of data
//////////////////////////////////////////////////////////////////////////
MString GrowerData::name() const {
	return GrowerData::typeName;
}

//////////////////////////////////////////////////////////////////////////
// GrowerData::creator
//
//	This method exists to give Maya a way to create new objects
//	of this type. 
//////////////////////////////////////////////////////////////////////////
void * GrowerData::creator() {
	return new GrowerData;
}
