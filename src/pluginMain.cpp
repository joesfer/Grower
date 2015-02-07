/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/

#include "SamplerNode.h"
#include "GrowerNode.h"
#include "TrimmerNode.h"
#include "MesherNode.h"
#include "MesherUI.h"
#include "GrowerData.h"
#include "SamplerCacheData.h"
#include "SamplePreviewShape.h"
#include "SamplePreviewShapeUI.h"

#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj )
//
//	Description:
//		this method is called when the plug-in is loaded into Maya.  It 
//		registers all of the services that this plug-in provides with 
//		Maya.
//
//	Arguments:
//		obj - a handle to the plug-in object (use MFnPlugin to access it)
//
{ 
	MStatus   status;
	MFnPlugin plugin( obj, "Jose Esteve. www.joesfer.com", "2014", "Any");

	status = plugin.registerData(SamplerCacheData::typeName, SamplerCacheData::id, SamplerCacheData::creator, MPxData::kGeometryData);
	if (!status) {
		status.perror("registerData SamplerCacheData");
		return status;
	}

	status = plugin.registerNode( "Sampler", Sampler::id, Sampler::creator, Sampler::initialize );
	if (!status) {
		status.perror("registerNode Sampler");
		return status;
	}

	status = plugin.registerData( GrowerData::typeName, 
								  GrowerData::id, 
								  GrowerData::creator, 
								  MPxData::kGeometryData );
	if (!status) {
		status.perror("registerData GrowerData");
		return status;
	}
	
	status = plugin.registerNode( "Grower", Grower::id, Grower::creator, Grower::initialize );
	if (!status) {
		status.perror("registerNode Grower");
		return status;
	}

	status = plugin.registerNode( "Trimmer", 
								  Trimmer::id, 
								  Trimmer::creator, 
								  Trimmer::initialize );
	if (!status) {
		status.perror("registerNode Trimmer");
		return status;
	}

	status = plugin.registerShape(GrowerShape::typeName, 
								  GrowerShape::id, 
								  GrowerShape::creator,
								  GrowerShape::initialize, 
								  MesherUI::creator );
	if (!status) {
		status.perror("registerShape GrowerShape");
		return status;
	}

	status = plugin.registerData(SamplePreviewData::typeName, 
								 SamplePreviewData::id, 
								 SamplePreviewData::creator, 
								 MPxData::kGeometryData);
	if (!status) {
		status.perror("registerData SamplePreviewData");
		return status;
	}

	status = plugin.registerShape(SampleShape::typeName, 
								  SampleShape::id,
								  SampleShape::creator, 
								  SampleShape::initialize,
								  SampleShapeUI::creator);
	if (!status) {
		status.perror("registerShape SampleShape");
		return status;
	}
	
	return status;
}

MStatus uninitializePlugin( MObject obj)
//
//	Description:
//		this method is called when the plug-in is unloaded from Maya. It 
//		deregisters all of the services that it was providing.
//
//	Arguments:
//		obj - a handle to the plug-in object (use MFnPlugin to access it)
//
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterNode( Sampler::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	status = plugin.deregisterData( GrowerData::id );
	if (!status) {
		status.perror("deregisterData");
		return status;
	}

	status = plugin.deregisterData(SamplerCacheData::id);
	if (!status) {
		status.perror("deregisterData");
		return status;
	}

	status = plugin.deregisterNode( Grower::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	status = plugin.deregisterNode( Trimmer::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	status = plugin.deregisterNode( GrowerShape::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	status = plugin.deregisterData(SamplePreviewData::id);
	if (!status) {
		status.perror("deregisterData");
		return status;
	}

	return status;
}
