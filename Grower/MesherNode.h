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
	size_t CalculateThickness( std::vector< growerNode_t >& nodes, const float baseThickness, float* thicknessArray );

};

#endif // MesherNode_h__