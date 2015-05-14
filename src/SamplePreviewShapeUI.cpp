/* 
	================================================================================
	Copyright (c) 2011, Jose Esteve. http://www.joesfer.com
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

#include <GL/gl.h>

#include "SamplePreviewShapeUI.h"
#include "SamplePreviewShape.h"
#include <maya/MColor.h>
#include <maya/MDrawData.h>
#include <maya/MSelectionMask.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>

// Object and component color defines
//
#define LEAD_COLOR				18	// green
#define ACTIVE_COLOR			15	// white
#define ACTIVE_AFFECTED_COLOR	8	// purple
#define DORMANT_COLOR			4	// blue
#define HILITE_COLOR			17	// pale blue
#define DORMANT_VERTEX_COLOR	8	// purple
#define ACTIVE_VERTEX_COLOR		16	// yellow

SampleShapeUI::SampleShapeUI() {}
SampleShapeUI::~SampleShapeUI() {}

void* SampleShapeUI::creator()
{
	return new SampleShapeUI();
}

//////////////////////////////////////////////////////////////////////////
// SampleShapeUI::getDrawRequests (override)
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

void SampleShapeUI::getDrawRequests(	const MDrawInfo & info,
									bool objectAndActiveOnly,
									MDrawRequestQueue & queue ) {

	// Get the data necessary to draw the shape
	//
	SampleShape* shape =  (SampleShape*)surfaceShape();
	SamplePreviewData* previewData = shape->getData();
	if ( previewData == NULL ) {
		cerr << "NO DrawRequest for SampleShapeUI\n";
		return;
	}

	// This call creates a prototype draw request that we can fill
	// in and then add to the draw queue.
	//
	MDrawData data;
	MDrawRequest request = info.getPrototype( *this );
	getDrawData( previewData, data );
	request.setDrawData( data );

	// Decode the draw info and determine what needs to be drawn
	//

	M3dView::DisplayStyle  appearance    = info.displayStyle();
	M3dView::DisplayStatus displayStatus = info.displayStatus();

	// Are we displaying meshes?
	if ( ! info.objectDisplayStatus( M3dView::kDisplayMeshes ) )
	  return;

	switch ( appearance ) {

		case M3dView::kWireFrame:  // fall through
		case M3dView::kFlatShaded: // fall through
		case M3dView::kGouraudShaded:
		{
		  request.setToken( kDrawWireframe );
		  request.setDisplayStyle( M3dView::kWireFrame );

		  M3dView::ColorTable activeColorTable = M3dView::kActiveColors;
		  M3dView::ColorTable dormantColorTable = M3dView::kDormantColors;

		  switch ( displayStatus ) {
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

	default: 
	  break;
	}
}



//////////////////////////////////////////////////////////////////////////
// SampleShapeUI::draw (override)
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

void SampleShapeUI::draw( const MDrawRequest & request, M3dView & view ) const{ 
	// Get the token from the draw request.
	// The token specifies what needs to be drawn.
	//
	int token = request.token();
	MDrawData data = request.drawData();
	SamplePreviewData* previewData = (SamplePreviewData*)data.geometry();

	switch( token )
	{
	case kDrawWireframe :
		{
			view.beginGL();
			float oldPointSize;
			glGetFloatv( GL_POINT_SIZE, &oldPointSize );
			glPointSize( 2.0 );

			glBegin( GL_POINTS );

			const MPointArray& samples = previewData->samples;
			for( unsigned int i = 0; i < samples.length(); i++ ) {
				glVertex3d( samples[ i ].x, samples[ i ].y, samples[ i ].z );
			}

			glEnd();

			glPointSize( oldPointSize );
			view.endGL();
			break;
		}
	default: break;
	}
}



//////////////////////////////////////////////////////////////////////////
// SampleShapeUI::select (override)
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

bool SampleShapeUI::select( MSelectInfo &selectInfo, 
					  MSelectionList &selectionList,
					  MPointArray &worldSpaceSelectPts ) const {
					  
	 SampleShape* shape = (SampleShape*)surfaceShape();
	 if ( shape == NULL ) return false;

	 // NOTE: If the geometry has an intersect routine it should
	 // be called here with the selection ray to determine if the
	 // the object was selected.

	 MSelectionMask priorityMask( MSelectionMask::kSelectMeshes );
	 MSelectionList item;
	 item.add( selectInfo.selectPath() );
	 MPoint xformedPt;
	 if ( selectInfo.singleSelection() ) {
		 MPoint center = shape->boundingBox().center();
		 xformedPt = center;
		 xformedPt *= selectInfo.selectPath().inclusiveMatrix();
	 }

	 selectInfo.addSelection( item, xformedPt, selectionList,
		 worldSpaceSelectPts, priorityMask, false );

	 return true;
}
