/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/

#ifndef TrimmerNode_h__
#define TrimmerNode_h__


#include <maya/MPxNode.h>
#include <maya/MTypeId.h> 
#include <vector>

struct growerNode_t;

class Trimmer : public MPxNode
{
public:
	Trimmer();
	virtual				~Trimmer(); 

	virtual MStatus		connectionMade( const MPlug& plug, const MPlug& otherPlug, bool asSrc );

	virtual MStatus		compute( const MPlug& plug, MDataBlock& data );

	static  void*		creator();
	static  MStatus		initialize();

public:

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	static  MObject		maxLength;		// 
	static	MObject		inputData;		// GrowerData
	static	MObject		outputData;		// GrowerData

	// The typeid is a unique 32bit identifier that describes this node.
	// It is used to save and retrieve nodes of this type from the binary
	// file format.  If it is not unique, it will cause file IO problems.
	//
	static	MTypeId		id;

private:
	int GetMaxDepth( const std::vector< growerNode_t >& nodes ) const;
	void Trim( std::vector< growerNode_t >& nodes, const int maxLength );
};


#endif // TrimmerNode_h__