/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/

#ifndef MesherNode_h__
#define MesherNode_h__


#include <maya/MPxNode.h>
#include <maya/MTypeId.h> 
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MPxSurfaceShape.h>
#include <vector>

class GrowerData;
struct growerNode_t;
struct attractionPointVis_t;

/////////////////////////////////////////////////////////////////////
//
// class Shape
//
//	Implements the custom shape generation and produces a MFnMesh
//	for Maya to handle. A different class, GrowerUI will be in charge 
//	of displaying the results in the viewports.
// 
/////////////////////////////////////////////////////////////////////

class Shape : public MPxSurfaceShape
{
public:
	Shape();
	virtual					~Shape(); 

	// overrides

	virtual void			postConstructor();
	virtual MStatus			compute( const MPlug& plug, MDataBlock& data );

	virtual bool			isBounded() const;
	virtual MBoundingBox	boundingBox() const;

	virtual MObject			localShapeOutAttr() const;
	virtual MObject			geometryData() const;

	// methods

	MObject					MeshDataRef();
	GrowerData*				MeshGeometry();
	const GrowerData*		MeshGeometry() const;

	static  void*			creator();
	static  MStatus			initialize();

public:

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	static	MObject		tubeSections;
	static  MObject		thicknessScale;
	static	MObject		thickness;
	static	MObject		inputData;		// GrowerData
	static	MObject		outMesh;		// output MFnMesh

	// The typeid is a unique 32bit identifier that describes this node.
	// It is used to save and retrieve nodes of this type from the binary
	// file format.  If it is not unique, it will cause file IO problems.
	//
	static const MTypeId	id;
	static const MString	typeName;

private:
	void CreateMesh( const GrowerData* data, const size_t activeNodes, const int tubeSections, const float* thickness, MPointArray& vertices, MIntArray& indices, MIntArray& polygonCounts ) const;
	size_t CalculateThickness(std::vector< growerNode_t >& nodes, float thicknessScale, float* thicknessArray);
};

#endif // MesherNode_h__