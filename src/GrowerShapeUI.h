/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/

#ifndef GrowerUI_h__
#define GrowerUI_h__

#include <maya/MPxSurfaceShapeUI.h> 
#include "GrowerData.h" 
#include "common.h"

/////////////////////////////////////////////////////////////////////
//
// class MesherUI
//
//	Implements the part in charge of drawing the shape in the 
//	viewports
// 
/////////////////////////////////////////////////////////////////////

class MesherUI : public MPxSurfaceShapeUI
{
public:
	MesherUI();
	virtual ~MesherUI(); 

	/////////////////////////////////////////////////////////////////////
	//
	// Overrides
	//
	/////////////////////////////////////////////////////////////////////

	// Puts draw request on the draw queue
	//
	virtual void	getDrawRequests( const MDrawInfo & info, bool objectAndActiveOnly, MDrawRequestQueue & requests );

	// Main draw routine. Gets called by maya with draw requests.
	//
	virtual void	draw( const MDrawRequest & request, M3dView & view ) const;

	// Main selection routine
	//
	virtual bool	select( MSelectInfo &selectInfo,
							MSelectionList &selectionList,
							MPointArray &worldSpaceSelectPts ) const;

	/////////////////////////////////////////////////////////////////////
	//
	// Helper routines
	//
	/////////////////////////////////////////////////////////////////////

	void	DrawWireframe( const MDrawRequest & request, M3dView & view ) const;	
	bool 	SelectVertices( MSelectInfo &selectInfo,
							MSelectionList &selectionList,
							MPointArray &worldSpaceSelectPts ) const;

	static  void *      creator();

private:

	// Draw Tokens
	//
	enum {
		kDrawVertices, // component token
		kDrawWireframe,
		kDrawWireframeOnShaded,
		kDrawSmoothShaded,
		kDrawFlatShaded,
		kDrawBoundingBox,
		kDrawRedPointAtCenter,  // for userInteraction example code
		kLastToken
	};

};

#endif // GrowerUI_h__