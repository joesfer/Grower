/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/
#include <GL/gl.h>

#include "GrowerShapeUI.h"
#include "GrowerShape.h"
#include "GrowerData.h"

#include <maya/MObjectArray.h>
#include <maya/MGlobal.h>

#include <maya/MMaterial.h>
#include <maya/MColor.h>
#include <maya/MDrawData.h>
#include <maya/MMatrix.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MFnSingleIndexedComponent.h>

#include <iostream>

#define RANDOM_FLOAT (float)rand() / RAND_MAX

// Object and component color defines
//
#define LEAD_COLOR				18	// green
#define ACTIVE_COLOR			15	// white
#define ACTIVE_AFFECTED_COLOR	8	// purple
#define DORMANT_COLOR			4	// blue
#define HILITE_COLOR			17	// pale blue
#define DORMANT_VERTEX_COLOR	8	// purple
#define ACTIVE_VERTEX_COLOR		16	// yellow

// Vertex point size
//
#define POINT_SIZE				2.0	
#define UV_POINT_SIZE			4.0

////////////////////////////////////////////////////////////////////////////////
//
// UI implementation
//
////////////////////////////////////////////////////////////////////////////////

GrowerShapeUI::GrowerShapeUI() {}
GrowerShapeUI::~GrowerShapeUI() {}

void* GrowerShapeUI::creator() {
	return new GrowerShapeUI();
}


///////////////////////////////////////////////////////////////////////////////
//
// Overrides
//
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// GrowerShapeUI::getDrawRequests (override)
//
// Description:
//
//     Add draw requests to the draw queue
//
// Arguments:
//
//     info                 - current drawing state
//     objectsAndActiveOnly - no components if true
//     queue                - queue of draw requests to add to
//
//////////////////////////////////////////////////////////////////////////
void GrowerShapeUI::getDrawRequests( const MDrawInfo & info,
								bool /*objectAndActiveOnly*/,
								MDrawRequestQueue & queue ) {
  // Get the data necessary to draw the shape
  //
  MDrawData data;
  GrowerShape* meshNode =  (GrowerShape*)surfaceShape();
  GrowerData * geom = meshNode->MeshGeometry();
  if ( NULL == geom ) {
	  std::cerr << "NO DrawRequest for Grower" << std::endl;
	  return;
  }

  // This call creates a prototype draw request that we can fill
  // in and then add to the draw queue.
  //
  MDrawRequest request = info.getPrototype( *this );
  getDrawData( geom, data );
  request.setDrawData( data );

  // Decode the draw info and determine what needs to be drawn
  //

  M3dView::DisplayStyle  appearance    = info.displayStyle();
  M3dView::DisplayStatus displayStatus = info.displayStatus();

  // Are we displaying meshes?
  if ( info.objectDisplayStatus( M3dView::kDisplayMeshes ) )
	  return;

  switch ( appearance )
  {
  case M3dView::kWireFrame :
	  {
		  request.setToken( kDrawWireframe );
		  request.setDisplayStyle( M3dView::kWireFrame );

		  M3dView::ColorTable activeColorTable = M3dView::kActiveColors;
		  M3dView::ColorTable dormantColorTable = M3dView::kDormantColors;

		  switch ( displayStatus )
		  {
		  case M3dView::kLead :
			  request.setColor( LEAD_COLOR, activeColorTable );
			  break;
		  case M3dView::kActive :
			  request.setColor( ACTIVE_COLOR, activeColorTable );
			  break;
		  case M3dView::kActiveAffected :
			  request.setColor( ACTIVE_AFFECTED_COLOR, activeColorTable );
			  break;
		  case M3dView::kDormant :
			  request.setColor( DORMANT_COLOR, dormantColorTable );
			  break;
		  case M3dView::kHilite :
			  request.setColor( HILITE_COLOR, activeColorTable );
			  break;
		  default:	
			  break;
		  }

		  queue.add( request );

		  break;
	  }
  case M3dView::kFlatShaded:
	  request.setToken( kDrawFlatShaded );
	  queue.add( request );
	  break;
  case M3dView::kGouraudShaded:
	  {

		  request.setToken( kDrawSmoothShaded );
		  request.setDisplayStyle( appearance );


		  // create a draw request for wireframe on shaded if
		  // necessary.
		  //
		  if ( (displayStatus == M3dView::kActive) ||
			  (displayStatus == M3dView::kLead) ||
			  (displayStatus == M3dView::kHilite) )
		  {
			  MDrawRequest wireRequest = info.getPrototype( *this );
			  wireRequest.setDrawData( data );
			  wireRequest.setToken( kDrawWireframeOnShaded );
			  wireRequest.setDisplayStyle( M3dView::kWireFrame );

			  M3dView::ColorTable activeColorTable = M3dView::kActiveColors;

			  switch ( displayStatus )
			  {
			  case M3dView::kLead :
				  wireRequest.setColor( LEAD_COLOR, activeColorTable );
				  break;
			  case M3dView::kActive :
				  wireRequest.setColor( ACTIVE_COLOR, activeColorTable );
				  break;
			  case M3dView::kHilite :
				  wireRequest.setColor( HILITE_COLOR, activeColorTable );
				  break;
			  default:	
				  break;

			  }

			  queue.add( wireRequest );
		  }
	  }
	  break;
  default: 
	  break;
}
}



//////////////////////////////////////////////////////////////////////////
// GrowerShapeUI::draw (override)
//
// Description:
//
//     Main (OpenGL) draw routine
//
// Arguments:
//
//     request - request to be drawn
//     view    - view to draw into
//
//////////////////////////////////////////////////////////////////////////
void GrowerShapeUI::draw( const MDrawRequest & request, M3dView & view ) const{ 
	// Get the token from the draw request.
	// The token specifies what needs to be drawn.
	//
	int token = request.token();

	switch( token )
	{
	case kDrawWireframe :
		DrawWireframe( request, view );
		break;
	default: break;
	}
}



//////////////////////////////////////////////////////////////////////////
// GrowerShapeUI::select (override)
//
// Description:
//
//     Main selection routine
//
// Arguments:
//
//     selectInfo           - the selection state information
//     selectionList        - the list of selected items to add to
//     worldSpaceSelectPts  -
//
//////////////////////////////////////////////////////////////////////////
bool GrowerShapeUI::select( MSelectInfo &/*selectInfo*/, MSelectionList &/*selectionList*/,
						MPointArray &/*worldSpaceSelectPts*/ ) const {
return true;
}

///////////////////////////////////////////////////////////////////////////////
//
// Helper routines
//
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// GrowerShapeUI::DrawWireframe
//
// Description:
//
//     Wireframe drawing routine
//
// Arguments:
//
//     request - request to be drawn
//     view    - view to draw into
//
//////////////////////////////////////////////////////////////////////////
void GrowerShapeUI::DrawWireframe( const MDrawRequest & request, M3dView & view ) const {
	MDrawData data = request.drawData();
	GrowerData * geom = (GrowerData*)data.geometry();

	int token = request.token();

	bool wireFrameOnShaded = false;
	if ( kDrawWireframeOnShaded == token ) {
		wireFrameOnShaded = true;
	}

	view.beginGL(); 

	// Query current state so it can be restored
	//
	bool lightingWasOn = glIsEnabled( GL_LIGHTING ) ? true : false;
	if ( lightingWasOn ) {
		glDisable( GL_LIGHTING );
	}

	if ( wireFrameOnShaded ) {
		glDepthMask( false );
	}
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

#if GROWER_DISPLAY_DEBUG_INFO && 0
	glLineWidth( 3.0f );
	glColor3f( 1, 1, 0 );
	glBegin( GL_LINES );
	glVertex3f(0,0,0);
	glVertex3f( geom->searchRadius, 0, 0);
	glEnd();

	glColor3f( 1, 0, 1 );
	glBegin( GL_LINES );
	glVertex3f(0,0.2f,0);
	glVertex3f( geom->killRadius, 0.2f, 0);
	glEnd();

	glColor3f( 0, 1, 1 );
	glBegin( GL_LINES );
	glVertex3f(0,0.4f,0);
	glVertex3f( geom->growDist, 0.4f, 0);
	glEnd();
#endif

	glColor3f( 1, 0, 0 );	
	for( unsigned int i = 0; i < geom->nodes.size(); i++ ) {
		const growerNode_t& node = geom->nodes[ i ];
		for( size_t j = 0; j < node.children.size(); j++ ) {
			const growerNode_t& child = geom->nodes[ node.children[ j ] ];

#if GROWER_DISPLAY_DEBUG_INFO
			glLineWidth( 3.0f );
			glColor3f( 1, 0, 0 );
			glBegin( GL_LINES );
			glVertex3f( (float)node.pos.x, (float)node.pos.y, (float)node.pos.z );
			glVertex3f( (float)child.pos.x, (float)child.pos.y, (float)child.pos.z );
			glEnd();

			glPointSize( 3.0f );
			glBegin( GL_POINTS );
			glColor3f( 1, 1, 1 );
			glVertex3f( (float)child.pos.x, (float)child.pos.y, (float)child.pos.z );
			glEnd();
#else 
			glLineWidth( 3.0f );
			glBegin( GL_LINES );
			glVertex3f( (float)node.pos.x, (float)node.pos.y, (float)node.pos.z );
			glVertex3f( (float)child.pos.x, (float)child.pos.y, (float)child.pos.z );
			glEnd();
#endif
		}
	}

#if GROWER_DISPLAY_DEBUG_INFO
	glPointSize( 3.0f );
	for( unsigned int i = 0; i < geom->samples.size(); i++ ) {
		if ( geom->samples[ i ].active ) {
			switch (i)
			{
			case 0: glColor3f(1, 1, 0); break;
			case 1: glColor3f(1, 0, 1); break;
			case 2: glColor3f(0, 1, 1); break;
			case 3: glColor3f(0, 0, 0); break;
			default:glColor3f(0, 1, 0); break;
			}

		} else {
			glColor3f( 1, 0, 0 );
		}
		glBegin( GL_POINTS );
		glVertex3f( (float)geom->samples[ i ].pos.x, (float)geom->samples[ i ].pos.y, (float)geom->samples[ i ].pos.z );
		glEnd();
	}
#endif

	// Restore the state
	//
	if ( lightingWasOn ) {
		glEnable( GL_LIGHTING );
	}
	if ( wireFrameOnShaded ) {
		glDepthMask( true );
	}

	glColor3f( 0,0,0 );
	glLineWidth( 1.0f );

	view.endGL(); 
}


//////////////////////////////////////////////////////////////////////////
// GrowerShapeUI::SelectVertices
//
// Description:
//
//     Vertex selection.
//
// Arguments:
//
//     selectInfo           - the selection state information
//     selectionList        - the list of selected items to add to
//     worldSpaceSelectPts  -
//
//////////////////////////////////////////////////////////////////////////
bool GrowerShapeUI::SelectVertices( MSelectInfo &/*selectInfo*/,
							 MSelectionList &/*selectionList*/,
							 MPointArray &/*worldSpaceSelectPts*/ ) const {
	return false;
}

