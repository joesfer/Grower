/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
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
