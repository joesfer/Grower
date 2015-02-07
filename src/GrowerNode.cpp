/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/
#include "GrowerNode.h"
#include "GrowerData.h"
#include "NearestNeighbors.h"

#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MVectorArray.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MIntArray.h>
#include <maya/MPointArray.h>
#include <maya/MVector.h>
#include <maya/MFnPluginData.h>
#include <maya/MGlobal.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnMatrixData.h>

#include <stack>
#include <set>

//////////////////////////////////////////////////////////////////////
//
// Error checking
//
//    MCHECKERROR       - check the status and print the given error message
//    MCHECKERRORNORET  - same as above but does not return
//
//////////////////////////////////////////////////////////////////////

#define MCHECKERROR(STAT,MSG)       \
	if ( MS::kSuccess != STAT ) {   \
	cerr << MSG << endl;        \
	return MS::kFailure;    \
	}

#define MCHECKERRORNORET(STAT,MSG)  \
	if ( MS::kSuccess != STAT ) {   \
	cerr << MSG << endl;        \
	}

// You MUST change this to a unique value!!!  The typeId is a 32bit value used
// to identify this type of node in the binary file format.  
//
const MTypeId   Grower::id( 0x80097 );
const MString	Grower::typeName( "GrowerNode" );

// Attributes
MObject		Grower::cacheSolution;
MObject		Grower::inputSamples;
MObject		Grower::inputPoints;
MObject		Grower::inputNormals;
MObject		Grower::inputPosition;
MObject		Grower::world2Local;
MObject		Grower::searchRadius;
MObject		Grower::killRadius;
MObject		Grower::growDist;
MObject		Grower::maxNeighbors;
MObject		Grower::aoMeshData;


MStatus Grower::compute( const MPlug& plug, MDataBlock& data )
//
//	Description:
//		This method computes the value of the given output plug based
//		on the values of the input attributes.
//
//	Arguments:
//		plug - the plug to compute
//		data - object that provides access to the attributes for this node
//
{
	MStatus stat;
	if ( plug == aoMeshData ) {

		MDataHandle inputPointsHandle = data.inputValue( inputSamples, &stat );

		MObject pointArrayObj = inputPointsHandle.child( inputPoints ).data();
		MFnPointArrayData pointVecData;
		pointVecData.setObject(pointArrayObj);
		MPointArray pointVec = pointVecData.array();

		MObject normalArrayObj = inputPointsHandle.child( inputNormals ).data();
		MFnVectorArrayData normalVecData;
		normalVecData.setObject(normalArrayObj);
		MVectorArray normalVec = normalVecData.array();

		MFnPluginData fnDataCreator;
		MTypeId tmpid( GrowerData::id );
		GrowerData * newData = NULL;

		MDataHandle outHandle = data.outputValue( aoMeshData );	
		newData = (GrowerData*)outHandle.asPluginData();

		if ( newData == NULL ) {
			// Create some output data
			fnDataCreator.create( tmpid, &stat );
			MCHECKERROR( stat, "compute : error creating GrowerData")
			newData = (GrowerData*)fnDataCreator.data( &stat );
			MCHECKERROR( stat, "compute : error gettin at proxy GrowerData object")
		}

		// compute the output values			

		MBoundingBox srcBounds;
		srcBounds.clear();
		for( unsigned int i = 0; i < pointVec.length(); i++ ) {
			srcBounds.expand( pointVec[ i ] );
		}		

		MPoint sourcePos = data.inputValue( Grower::inputPosition ).asFloatVector();
		MFnMatrixData matrixData( data.inputValue( Grower::world2Local ).data() );
		MMatrix matrix = matrixData.matrix();
		sourcePos *= matrix;

		float searchRadius = data.inputValue( Grower::searchRadius ).asFloat(); 
		float killRadius   = data.inputValue( Grower::killRadius ).asFloat();
		float nodeGrowDist = data.inputValue( Grower::growDist ).asFloat();
		int maxNeighbors   = data.inputValue( Grower::maxNeighbors ).asInt();

		// invalidate the cache if the input settings differ too much (note for
		// the distances we're using the multiplier, not the absolute distance
		// we'll pass in to the Grow method below).

		const bool cacheGrowth = data.inputValue(cacheSolution, &stat).asBool();

		bool useCachedSolution = cacheGrowth && 
								 fabsf(searchRadius - newData->m_cachedSearchRadius) < 1e-1f &&
								 fabsf(killRadius - newData->m_cachedKillRadius) < 1e-1f &&
								 maxNeighbors == newData->m_cachedNumNeighbours &&
								 fabsf(nodeGrowDist - newData->m_cachedNodeGrowDist) < 1e-1f;

		if ( !useCachedSolution )
		{
			// we'll regenerate the cache inside the grow method, store the
			// input parameters here for next time.
			newData->m_cachedSearchRadius = searchRadius;
			newData->m_cachedKillRadius = killRadius;
			newData->m_cachedNumNeighbours = maxNeighbors;
			newData->m_cachedNodeGrowDist = nodeGrowDist;
		}

		// calculate the scene-sized distance thresholds
		float maxExtents = (float)std::max( srcBounds.width(), std::max( srcBounds.height(), srcBounds.depth() ) );
		searchRadius = searchRadius * maxExtents;
		killRadius	 = killRadius	* maxExtents;
		nodeGrowDist = nodeGrowDist * maxExtents;


		newData->nodes.resize( 0 );
#if GROWER_DISPLAY_DEBUG_INFO
		newData->samples.resize( 0 );
#endif
		Grow( pointVec, 
			  normalVec, 
			  sourcePos, 
			  searchRadius, 
			  killRadius, 
			  maxNeighbors, 
			  nodeGrowDist, 
			  useCachedSolution, // we either use the cache, or generate it
			  newData);
	
		newData->bounds.clear();
		for( unsigned int i = 0; i < newData->nodes.size(); i++ ) {
			newData->bounds.expand( newData->nodes[ i ].pos );
		}

		// Assign the new data to the outputSurface handle

		if ( newData != outHandle.asPluginData() ) {
			outHandle.set( newData );
		}

		data.setClean(plug);
		return MS::kSuccess;

	}

	return MS::kInvalidParameter;
}


void* Grower::creator()
//
//	Description:
//		this method exists to give Maya a way to create new objects
//      of this type. 
//
//	Return Value:
//		a new object of this type
//
{
	return new Grower();
}

MStatus Grower::initialize()
//
//	Description:
//		This method is called to create and initialize all of the attributes
//      and attribute dependencies for this node type.  This is only called 
//		once when the node type is registered with Maya.
//
//	Return Values:
//		MS::kSuccess
//		MS::kFailure
//		
{
	// This sample creates a single input float attribute and a single
	// output float attribute.
	//
	MFnNumericAttribute nFn;
	MFnTypedAttribute	typedFn;	
	MFnCompoundAttribute cFn;
	MStatus				stat;

	cacheSolution = nFn.create("cacheGrowth", "cg", MFnNumericData::kBoolean, false, &stat);
	if (!stat) return stat;
	nFn.setWritable(true);
	nFn.setStorable(true);

	inputPoints = typedFn.create( "samplesPoints", "sp", MFnData::kPointArray );
	typedFn.setStorable( false );
	typedFn.setWritable( true );

	inputNormals = typedFn.create( "samplesNormals", "sn", MFnData::kVectorArray );
	typedFn.setStorable( false );
	typedFn.setWritable( true );

	inputSamples = cFn.create( "samples", "s" );
	cFn.setWritable( true );
	cFn.addChild( inputPoints );
	cFn.addChild( inputNormals );
	cFn.setHidden( true );

	inputPosition = nFn.createPoint( "inputPos", "ip" );
	nFn.setStorable( false );
	nFn.setWritable( true );

	world2Local	= typedFn.create( "worldToLocal", "wtl", MFnData::kMatrix );
	typedFn.setStorable( false );
	typedFn.setWritable( true );

	searchRadius = nFn.create( "searchRadius", "sr", MFnNumericData::kFloat, 0.5f );
	nFn.setSoftMin( 0.001f );
	nFn.setSoftMax( 1.0f );
	nFn.setStorable( true );
	nFn.setWritable( true );

	killRadius = nFn.create( "killRadius", "kr", MFnNumericData::kFloat, 0.01f );
	nFn.setSoftMin( 0.001f );
	nFn.setSoftMax( 0.1f );
	nFn.setStorable( true );
	nFn.setWritable( true );

	growDist = nFn.create( "growDist", "gd", MFnNumericData::kFloat, 0.01f );
	nFn.setSoftMin( 0.0005f );
	nFn.setSoftMax( 0.01f );
	nFn.setStorable( true );
	nFn.setWritable( true );

	maxNeighbors = nFn.create( "maxNeighbors", "mnb", MFnNumericData::kInt, 10 );
	nFn.setSoftMin( 1 );
	nFn.setSoftMax( 1000 );
	nFn.setStorable( true );
	nFn.setWritable( true );

	aoMeshData = typedFn.create( "output", "out", GrowerData::id );
	typedFn.setWritable( false );
	typedFn.setStorable(false);
	typedFn.setHidden( true );

	// Add the attributes we have created to the node
	//
	stat = addAttribute(cacheSolution);
	if (!stat) { stat.perror("addAttribute"); return stat; }
	stat = addAttribute(inputSamples);
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( inputPosition );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( world2Local );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( aoMeshData );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( searchRadius );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( killRadius );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( growDist );
	if (!stat) { stat.perror("addAttribute"); return stat;}
	stat = addAttribute( maxNeighbors );
	if (!stat) { stat.perror("addAttribute"); return stat;}

	attributeAffects( cacheSolution, aoMeshData );
	attributeAffects( inputSamples, aoMeshData );
	attributeAffects( inputPosition, aoMeshData );
	attributeAffects( world2Local, aoMeshData );
	attributeAffects( searchRadius, aoMeshData );
	attributeAffects( killRadius, aoMeshData );
	attributeAffects( growDist, aoMeshData );
	attributeAffects( maxNeighbors, aoMeshData );

	return MS::kSuccess;

}

inline void insertUnique(std::vector<RenderLib::DataStructures::SampleIndex_t>& v, RenderLib::DataStructures::SampleIndex_t s) {
	// we could use std::map instead of std::vector, but since the arrays are reused many times
	// during running time, it's faster to do std::vector::resize(0) instead of std::set::clear() 
	// to avoid memory reallocations.
	std::vector<RenderLib::DataStructures::SampleIndex_t>::const_iterator it = v.begin();
	for( ; it != v.end(); it++ ) {
		if ( *it == s ) break;
	}
	if ( it == v.end() ) v.push_back(s);
}

//////////////////////////////////////////////////////////////////////////

void Grower::Grow( const MPointArray& points, 
				   const MVectorArray& normals, 
				   const MPoint& sourcePos, 
				   const float searchRadius, 
				   const float killRadius, 
				   const int maxNeighbors, 
				   const float nodeGrowDist, 
				   bool useCachedSolution,
				   GrowerData* inOutData) {

	using namespace std;

	std::vector< growerNode_t >& nodes = inOutData->nodes;
	
	KdTree knn;
	if ( !knn.Init( points, normals ) ) {
		return;
	}
	
	vector< RenderLib::DataStructures::SampleIndex_t > aliveNodes;

	growerNode_t seed;
	seed.pos = sourcePos;
	RenderLib::DataStructures::SampleIndex_t* neighbors = (RenderLib::DataStructures::SampleIndex_t*)alloca( ( maxNeighbors + 1 ) * sizeof(RenderLib::DataStructures::SampleIndex_t) );

	vector<bool> activeAttractors;
	activeAttractors.resize(points.length());
	vector<RenderLib::DataStructures::SampleIndex_t> closestNode;
	closestNode.resize(points.length());
	vector<float> distance;
	distance.resize(points.length());

	for( size_t i = 0; i < points.length(); i++ ) { 
		activeAttractors[i] = true; 
		closestNode[i] = UINT_MAX;
		distance[i] = FLT_MAX;
	}

	nodes.push_back( seed );
	aliveNodes.push_back( 0 );

	const bool generateSolutionCache = !useCachedSolution;

	if (generateSolutionCache)
	{
		inOutData->m_cachedAffectedPoints.resize(0);
		inOutData->m_cachedClosestNode.resize(0);
		inOutData->m_cachedBannedAliveNodes.resize(0);
		inOutData->m_cachedActiveAttractors.resize(0);
	}

	vector< RenderLib::DataStructures::SampleIndex_t > affectedPoints;
	vector< RenderLib::DataStructures::SampleIndex_t > bannedAliveNodes;

	int iterationCount = 0;

	while( !aliveNodes.empty() ) {
		vector< RenderLib::DataStructures::SampleIndex_t > newNodes;
		{
			affectedPoints.resize(0);

			if (useCachedSolution && inOutData->m_cachedAffectedPoints.size() > iterationCount)
			{
				affectedPoints.resize(inOutData->m_cachedAffectedPoints[iterationCount].size());
				if (affectedPoints.size() > 0)
				{
					memcpy(&affectedPoints[0], &inOutData->m_cachedAffectedPoints[iterationCount][0], inOutData->m_cachedAffectedPoints[iterationCount].size() * sizeof(RenderLib::DataStructures::SampleIndex_t));
				}

				closestNode.resize(inOutData->m_cachedClosestNode[iterationCount].size());
				if (closestNode.size() > 0)
				{
					memcpy(&closestNode[0], &inOutData->m_cachedClosestNode[iterationCount][0], inOutData->m_cachedClosestNode[iterationCount].size() * sizeof(RenderLib::DataStructures::SampleIndex_t));
				}

				bannedAliveNodes.resize(inOutData->m_cachedBannedAliveNodes[iterationCount].size());
				if (bannedAliveNodes.size() > 0)
				{
					memcpy(&bannedAliveNodes[0], &inOutData->m_cachedBannedAliveNodes[iterationCount][0], inOutData->m_cachedBannedAliveNodes[iterationCount].size() * sizeof(RenderLib::DataStructures::SampleIndex_t));
				}
				iterationCount++;
			}
			else
			{
				// find the closest attraction point to each alive node
				for (size_t i = 0; i < aliveNodes.size(); i++) {
					const RenderLib::DataStructures::SampleIndex_t aliveNode = aliveNodes[i];
					size_t found = knn.NearestNeighbors(nodes[aliveNode].pos, searchRadius, maxNeighbors, neighbors);
					assert((int)found <= maxNeighbors);
#if _DEBUG
					for (size_t j = 0; j < found; j++) {
						const double d = points[neighbors[j]].distanceTo(nodes[aliveNode].pos);
						assert(d <= searchRadius);
					}
#endif

					for (size_t j = 0; j < found; j++) {
						RenderLib::DataStructures::SampleIndex_t neighbor = neighbors[j];

						// TODO: since we're checking whether the node is active outside of the knn.NearestNeighbors query
						// some (or many) of the returned neighbors may be invalid, and therefore we're wasting space
						// in the neighbors array. It would be more efficient to store the validity of a node within the
						// kdtree, to skip invalid nodes during the nearestNeighbors query, but this would "pollute" the
						// kdtree class with irrelevant filtering knowledge (we want to keep the class generic for further reuse).
						if (!activeAttractors[neighbor]) continue;

						insertUnique(affectedPoints, neighbor);

						if (closestNode[neighbor] != UINT_MAX) {
							if (closestNode[neighbor] != aliveNode) {
								float dist = (float)nodes[aliveNode].pos.distanceTo(points[neighbor]);
								if (dist < distance[neighbor]) {
									closestNode[neighbor] = aliveNode;
									distance[neighbor] = dist;
								}
							}
						}
						else 
						{
							closestNode[neighbor] = aliveNode;
							distance[neighbor] = (float)nodes[aliveNode].pos.distanceTo(points[neighbor]);
						}
					} // for found
				} // for alive nodes

				inOutData->m_cachedAffectedPoints.push_back(affectedPoints);
				inOutData->m_cachedClosestNode.push_back(closestNode);
				
			} // else useCachedSolution
			
			// those nodes which are marked as closest to an attraction point
			// are the candidates to spawn new nodes, and therefore are the
			// only ones which remain active for the next iteration
			aliveNodes.resize(0);
			for( size_t i = 0; i < affectedPoints.size(); i++ ) {
				RenderLib::DataStructures::SampleIndex_t node = closestNode[affectedPoints[i]];
				insertUnique(aliveNodes, node);
			}

			if (generateSolutionCache)
			{
				inOutData->m_cachedBannedAliveNodes.push_back(std::vector<RenderLib::DataStructures::SampleIndex_t>());
			}
			
			// spawn new nodes	
			for( size_t i = 0; i < aliveNodes.size(); i++ ) {

				const RenderLib::DataStructures::SampleIndex_t& nodeIdx = aliveNodes[i];
				growerNode_t& srcNode = nodes[ nodeIdx ];

				MVector growDirection( 0, 0, 0 );
				size_t nAttractors = 0;

				for( size_t j = 0; j < affectedPoints.size(); j++ ) {
					if ( closestNode[affectedPoints[j]] != nodeIdx ) continue;

					nAttractors ++;
					MVector dir = points[affectedPoints[j]] - srcNode.pos;
					dir.normalize();
					growDirection += dir;
				}

				assert( nAttractors > 0 );
				growDirection.normalize();

				growerNode_t newNode;
				newNode.pos = srcNode.pos + nodeGrowDist * growDirection;

				bool duplicated = false;
				if (generateSolutionCache)
				{ 
					std::vector<RenderLib::DataStructures::SampleIndex_t>& cachedBannedAliveNodesEntry = inOutData->m_cachedBannedAliveNodes[inOutData->m_cachedBannedAliveNodes.size() - 1];
					for (size_t j = 0; j < srcNode.children.size(); j++) {
						if (nodes[srcNode.children[j]].pos.distanceTo(newNode.pos) <= 0.0001f) {
							duplicated = true;
							cachedBannedAliveNodesEntry.push_back(nodeIdx);
							break;
						}
					}
				}
				else
				{
					for (size_t j = 0; j < bannedAliveNodes.size(); ++j)
					{
						if (bannedAliveNodes[j] == nodeIdx)
						{
							duplicated = true;
							break;
						}
					}
				}

				if ( duplicated ) {
					// erase active element, as it is stuck in a loop trying to produce the same children
				
					aliveNodes[ i ] = aliveNodes[ aliveNodes.size() - 1 ];
					aliveNodes.resize( aliveNodes.size() - 1 );
					i--;		
				
				} else {
					newNode.parent = nodeIdx;
					RenderLib::DataStructures::SampleIndex_t newNodeIdx = (RenderLib::DataStructures::SampleIndex_t)nodes.size();
					srcNode.children.push_back( newNodeIdx );
					nodes.push_back( newNode );
					newNodes.push_back( newNodeIdx );
				}
			}
			for( size_t i = 0; i < newNodes.size(); i++ ) { 
				aliveNodes.push_back( newNodes[ i ] );
			}
		}

		// use the new spawned nodes to kill close attractor points
		if (generateSolutionCache)
		{
			for (size_t i = 0; i < newNodes.size(); i++) {
				size_t found = knn.NearestNeighbors(nodes[newNodes[i]].pos, killRadius, maxNeighbors, neighbors);
				assert((int)found <= maxNeighbors);
				for (size_t j = 0; j < found; j++) {
					activeAttractors[neighbors[j]] = false;
				}
			}
		}
	
	} // while alive

	// reactivate all the samples, we're going to retrieve the normals from them
	for( unsigned int i = 0; i < points.length(); i++ ) {
		activeAttractors[ i ] = true;
	}

	const MVector zero(0,0,0);
	const double minCosAngle = cos( 3.14159265 / 4 ); // 45 degrees
	size_t numNodes = nodes.size(); // size will change inside the loop
	for( size_t i = 0; i < numNodes; i++ ) {
		growerNode_t& node = nodes[ i ];
		// set normals
		size_t found = knn.NearestNeighbors( node.pos, killRadius, 1, neighbors );
		if ( found == 1 ) {
			node.surfaceNormal = normals[neighbors[0]];
		} else if ( node.parent != INVALID_PARENT && !nodes[ node.parent ].surfaceNormal.isEquivalent( zero, 0.001f ) ) {
			node.surfaceNormal = nodes[ node.parent ].surfaceNormal;
		/*} else if ( node.children.size() > 0 ) {
			MVector avgNormal;
			for( size_t j = 0; j < node.children.size(); j++ ) {
				avgNormal += nodes[ node.children[ j ] ].surfaceNormal;
			}
			node.surfaceNormal = avgNormal / (double)node.children.size();
		} else {*/
		} else {
			node.surfaceNormal = MVector( 0, 1, 0 );
		}

		//
		if ( node.parent != INVALID_PARENT ) {
			growerNode_t& parent = nodes[ node.parent ];
			MVector fromParent = node.pos - parent.pos;
			const double fromParentLength = fromParent.length();
			fromParent /= fromParentLength;
			for( size_t j = 0; j < node.children.size(); j++ ) {
				growerNode_t& child = nodes[ node.children[ j ] ];
				MVector toChild = child.pos - node.pos;
				const double toChildLength = toChild.length();
				toChild /= toChildLength;
				const double cosAngle = fromParent * toChild;
				if ( cosAngle < minCosAngle ) { 

					child.parent = node.parent;
					parent.children.push_back( node.children[ j ] );
					if ( node.children.size() > 1 ) {
						node.children[ j ] = node.children.back();
						node.children.resize( node.children.size() - 1 );
						j--;
					} else {
						node.children.clear();
						break;
					}							
				}
			}
		}
	}

#if GROWER_DISPLAY_DEBUG_INFO
	for (unsigned int i = 0; i < points.length(); i++) {
		attractionPointVis_t p;
		p.pos = points[i];
		p.active = activeAttractors[i];
		inOutData->samples.push_back( p );
	}
#endif
}
