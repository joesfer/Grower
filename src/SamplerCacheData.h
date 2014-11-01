/*
================================================================================
Copyright (c) 2014, Jose Esteve. http://www.joesfer.com
This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html
================================================================================
*/

#ifndef SamplerCacheData_h__
#define SamplerCacheData_h__

#include <maya/MPxGeometryData.h>
#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MPointArray.h>
#include <maya/MBoundingBox.h>
#include <maya/MPointArray.h>
#include <vector>

/////////////////////////////////////////////////////////////////////
//
// class SamplerCacheData
//
/////////////////////////////////////////////////////////////////////

class SamplerCacheData : public MPxGeometryData {

public:
	//////////////////////////////////////////////////////////////////
	//
	// Overrides from MPxData
	//
	//////////////////////////////////////////////////////////////////
	SamplerCacheData();
	virtual					~SamplerCacheData();

	virtual	void			copy(const MPxData&);

	virtual MTypeId         typeId() const;
	virtual MString         name() const;

	//////////////////////////////////////////////////////////////////
	//
	// Helper methods
	//
	//////////////////////////////////////////////////////////////////

	static void *	creator();

public:
	static const MString typeName;
	static const MTypeId id;

	std::vector< int > triangleIds;
	std::vector< std::pair<float, float> > triangleBarycentricCoords;
	std::vector<float> randomNumbers;
};
#endif // SamplerCacheData_h__
