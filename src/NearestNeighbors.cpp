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

#include "NearestNeighbors.h"

bool KdTree::Init( const MPointArray& points, const MVectorArray& normals ) {
	//samples = new RenderLib::PhotonMapSamplesPool< AttractionPoint >();
	//samples->Allocate( points.length() );
	//AttractionPoint* data = samples->ReserveChunk( points.length() );
	//samples = new AttractionPoint[points.length()];
	std::vector<RenderLib::Math::Point3f> samplePos;
	samplePos.resize(points.length());
	for( unsigned int i = 0; i < points.length(); i++ ) {
		//samples[ i ].pos = points[ i ];
		//samples[ i ].normal = normals[ i ];		

		const MPoint& p = points[i];
		samplePos[i] = RenderLib::Math::Point3f((float)p[0], (float)p[1], (float)p[2]);
	}

	pm = new RenderLib::DataStructures::PhotonMap( samplePos );

	return true;
}

size_t KdTree::NearestNeighbors( const MPoint pos, const float searchRadius, const int maxNeighbors, RenderLib::DataStructures::SampleIndex_t* result ) {
	RenderLib::Math::Point3f p( (float)pos.x, (float)pos.y, (float)pos.z );
	int found = 0;
	pm->nearestSamples( p, maxNeighbors, searchRadius, result, found );
	// the first element is always null
	for( int i = 0; i < found; i++ ) {
		result[i] = result[i+1];
	}
	return (size_t)found;
}
