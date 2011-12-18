/* 
	================================================================================
	Copyright (c) 2010, Jose Esteve. http://www.joesfer.com
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
};
#endif // GrowerData_h__
