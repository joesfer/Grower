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

#include "GrowerUI.h"
#include "GrowerNode.h"
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

GrowerUI::GrowerUI() {}
GrowerUI::~GrowerUI() {}

void* GrowerUI::creator() {
	return new GrowerUI();
}


///////////////////////////////////////////////////////////////////////////////
//
// Overrides
//
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// GrowerUI::getDrawRequests (override)
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
void GrowerUI::getDrawRequests( const MDrawInfo & info,
								bool objectAndActiveOnly,
								MDrawRequestQueue & queue ) {
  // Get the data necessary to draw the shape
  //
  MDrawData data;
  Shape* meshNode =  (Shape*)surfaceShape();
  GrowerData * geom = meshNode->MeshGeometry();
  if ( NULL == geom ) {
	  cerr << "NO DrawRequest for Shape\n";
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
  if ( !info.objectDisplayStatus( M3dView::kDisplayMeshes ) )
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
// GrowerUI::draw (override)
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
void GrowerUI::draw( const MDrawRequest & request, M3dView & view ) const{ 
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
// GrowerUI::select (override)
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
bool GrowerUI::select( MSelectInfo &selectInfo, MSelectionList &selectionList,
						MPointArray &worldSpaceSelectPts ) const {
return false;
						// bool selected = false;
						// bool componentSelected = false;
						// bool hilited = false;

						// hilited = (selectInfo.displayStatus() == M3dView::kHilite);
						// if ( hilited ) {
						//	 componentSelected = SelectVertices( selectInfo, selectionList,
						//		 worldSpaceSelectPts );
						//	 selected = selected || componentSelected;
						// }

						// if ( !selected ) {

						//	 Shape* meshNode = (Shape*)surfaceShape();

						//	 // NOTE: If the geometry has an intersect routine it should
						//	 // be called here with the selection ray to determine if the
						//	 // the object was selected.

						//	 selected = true;
						//	 MSelectionMask priorityMask( MSelectionMask::kSelectMeshes );
						//	 MSelectionList item;
						//	 item.add( selectInfo.selectPath() );
						//	 MPoint xformedPt;
						//	 if ( selectInfo.singleSelection() ) {
						//		 MPoint center = meshNode->boundingBox().center();
						//		 xformedPt = center;
						//		 xformedPt *= selectInfo.selectPath().inclusiveMatrix();
						//	 }

						//	 selectInfo.addSelection( item, xformedPt, selectionList,
						//		 worldSpaceSelectPts, priorityMask, false );
						// }

						// return selected;
}

///////////////////////////////////////////////////////////////////////////////
//
// Helper routines
//
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// GrowerUI::DrawWireframe
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
void GrowerUI::DrawWireframe( const MDrawRequest & request,
M3dView & view ) const {
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

#if GROWER_DISPLAY_DEBUG_INFO
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
			glLineWidth( node.thickness * 10 );
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
			glLineWidth( node.thickness * 10 );
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
			glColor3f( 0, 1, 0 );
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

	view.endGL(); 
}


//////////////////////////////////////////////////////////////////////////
// GrowerUI::SelectVertices
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
bool GrowerUI::SelectVertices( MSelectInfo &selectInfo,
							 MSelectionList &selectionList,
							 MPointArray &worldSpaceSelectPts ) const {
	return false;
								// bool selected = false;
								// M3dView view = selectInfo.view();

								// MPoint 		xformedPoint;
								// MPoint 		selectionPoint;
								// double		z,previousZ = 0.0;
								// int			closestPointVertexIndex = -1;

								// const MDagPath & path = selectInfo.multiPath();

								// // Create a component that will store the selected vertices
								// //
								// MFnSingleIndexedComponent fnComponent;
								// MObject surfaceComponent = fnComponent.create( MFn::kMeshVertComponent );
								// size_t vertexIndex;

								// // if the user did a single mouse click and we find > 1 selection
								// // we will use the alignmentMatrix to find out which is the closest
								// //
								// MMatrix	alignmentMatrix;
								// MPoint singlePoint; 
								// bool singleSelection = selectInfo.singleSelection();
								// if( singleSelection ) {
								//	 alignmentMatrix = selectInfo.getAlignmentMatrix();
								// }

								// // Get the geometry information
								// //
								// Shape* meshNode = (Shape*)surfaceShape();
								// MeshGeom * geom = meshNode->MeshGeometry();

								// // Loop through all vertices of the mesh and
								// // see if they lie withing the selection area
								// //
								// size_t numVertices = geom->vertices.size();
								// for ( vertexIndex=0; vertexIndex<numVertices; vertexIndex++ )
								// {
								//	 MPoint currentPoint = geom->vertices[ vertexIndex ].pos;

								//	 // Sets OpenGL's render mode to select and stores
								//	 // selected items in a pick buffer
								//	 //
								//	 view.beginSelect();

								//	 glBegin( GL_POINTS );
								//	 glVertex3f( (float)currentPoint[0], 
								//		 (float)currentPoint[1], 
								//		 (float)currentPoint[2] );
								//	 glEnd();

								//	 if ( view.endSelect() > 0 )	// Hit count > 0
								//	 {
								//		 selected = true;

								//		 if ( singleSelection ) {
								//			 xformedPoint = currentPoint;
								//			 xformedPoint.homogenize();
								//			 xformedPoint*= alignmentMatrix;
								//			 z = xformedPoint.z;
								//			 if ( closestPointVertexIndex < 0 || z > previousZ ) {
								//				 closestPointVertexIndex = (int)vertexIndex;
								//				 singlePoint = currentPoint;
								//				 previousZ = z;
								//			 }
								//		 } else {
								//			 // multiple selection, store all elements
								//			 //
								//			 fnComponent.addElement( (int)vertexIndex );
								//		 }
								//	 }
								// }

								// // If single selection, insert the closest point into the array
								// //
								// if ( selected && selectInfo.singleSelection() ) {
								//	 fnComponent.addElement(closestPointVertexIndex);

								//	 // need to get world space position for this vertex
								//	 //
								//	 selectionPoint = singlePoint;
								//	 selectionPoint *= path.inclusiveMatrix();
								// }

								// // Add the selected component to the selection list
								// //
								// if ( selected ) {
								//	 MSelectionList selectionItem;
								//	 selectionItem.add( path, surfaceComponent );

								//	 MSelectionMask mask( MSelectionMask::kSelectComponentsMask );
								//	 selectInfo.addSelection(
								//		 selectionItem, selectionPoint,
								//		 selectionList, worldSpaceSelectPts,
								//		 mask, true );
								// }

								// return selected;
}

