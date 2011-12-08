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

#include "GrowerData.h"

const MTypeId GrowerData::id( 0x80777 );
const MString GrowerData::typeName( "GrowerData" );

//////////////////////////////////////////////////////////////////////////
// GrowerData::GrowerData()
//////////////////////////////////////////////////////////////////////////

GrowerData::GrowerData() {
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
