/*
================================================================================
Copyright (c) 2014, Jose Esteve. http://www.joesfer.com
This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html
================================================================================
*/

#include "SamplerCacheData.h"

const MTypeId SamplerCacheData::id(0x80767);
const MString SamplerCacheData::typeName("SamplerCacheData");

//////////////////////////////////////////////////////////////////////////
// SamplerCacheData::SamplerCacheData()
//////////////////////////////////////////////////////////////////////////

SamplerCacheData::SamplerCacheData() {
}

//////////////////////////////////////////////////////////////////////////
// SamplerCacheData::~SamplerCacheData()
//////////////////////////////////////////////////////////////////////////

SamplerCacheData::~SamplerCacheData() {
}

//////////////////////////////////////////////////////////////////////////
// SamplerCacheData::copy (override)
//////////////////////////////////////////////////////////////////////////

void SamplerCacheData::copy(const MPxData& other) {
	if (&other != this) {
		const SamplerCacheData& _other = (const SamplerCacheData &)other;
		triangleIds = _other.triangleIds;
		triangleBarycentricCoords = _other.triangleBarycentricCoords;
		randomNumbers = _other.randomNumbers;
	}
}

//////////////////////////////////////////////////////////////////////////
// SamplerCacheData::typeId (override)
//
//	Binary tag used to identify this kind of data
//////////////////////////////////////////////////////////////////////////

MTypeId SamplerCacheData::typeId() const {
	return SamplerCacheData::id;
}

//////////////////////////////////////////////////////////////////////////
// SamplerCacheData::typeId (override)
//
//	String name used to identify this kind of data
//////////////////////////////////////////////////////////////////////////
MString SamplerCacheData::name() const {
	return SamplerCacheData::typeName;
}

//////////////////////////////////////////////////////////////////////////
// SamplerCacheData::creator
//
//	This method exists to give Maya a way to create new objects
//	of this type. 
//////////////////////////////////////////////////////////////////////////
void * SamplerCacheData::creator() {
	return new SamplerCacheData;
}
