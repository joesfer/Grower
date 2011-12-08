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
	samples = new RenderLib::PhotonMapSamplesPool< AttractionPoint >();
	samples->Allocate( points.length() );
	AttractionPoint* data = samples->ReserveChunk( points.length() );
	for( unsigned int i = 0; i < points.length(); i++ ) {
		data[ i ].pos = points[ i ];
		data[ i ].normal = normals[ i ];
		data[ i ].active = true;
		data[ i ].closestNode = UINT_MAX;
	}

	pm = new RenderLib::PhotonMap< AttractionPoint >( *samples );

	return true;
}

size_t KdTree::NearestNeighbors( const MPoint pos, const float searchRadius, const int maxNeighbors, AttractionPoint** result ) {
	RenderLib::Point3f p( (float)pos.x, (float)pos.y, (float)pos.z );
	int found = 0;
	pm->NearestSamples( p, maxNeighbors, searchRadius, result, found );
	if ( found > 0 ) {
		// the first element is always null
		memcpy( result, &result[ 1 ], found * sizeof( AttractionPoint* ) );
	}
	return (size_t)found;
}
