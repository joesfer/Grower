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
	AttractionPoint() : closestNode( UINT_MAX ), dist( FLT_MAX ) {}
	inline RenderLib::Point3f	GetPos() const { return RenderLib::Point3f( (float)pos.x, (float)pos.y, (float)pos.z ); }
	inline float				GetPos( int axis ) const { return GetPos()[axis]; }
	inline short				GetSplitPlane() const { return (int)split; }
	inline void					SetSplitPlane( int sp ) { split = (short)sp; }
	inline bool					IsValid() const { return active; }

public:
	MPoint		pos;
	MVector		normal;
	bool		active;
	short		split;

	size_t		closestNode;
	float		dist;
};

class KdTree {
public:
	KdTree() : pm(NULL), samples(NULL) {}
	~KdTree() { delete(pm); delete(samples); }

	bool Init( const MPointArray& points, const MVectorArray& normals );
	size_t NearestNeighbors( const MPoint pos, const float searchRadius, const int maxNeighbors, AttractionPoint** result );

public:
	RenderLib::PhotonMap< AttractionPoint >* pm;
	RenderLib::PhotonMapSamplesPool< AttractionPoint >* samples;
};

#endif // NearestNeighbors_h__