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
#include "Command.h"

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
	MFnPlugin plugin( obj, "Jose Esteve. www.joesfer.com", "2011", "Any");

	status = plugin.registerNode( "Sampler", Sampler::id, Sampler::creator, Sampler::initialize );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	status = plugin.registerData( GrowerData::typeName, GrowerData::id, GrowerData::creator, MPxData::kGeometryData );
	if (!status) {
		status.perror("registerData");
		return status;
	}

	status = plugin.registerNode( "Grower", Grower::id, Grower::creator, Grower::initialize );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	status = plugin.registerNode( "Trimmer", Trimmer::id, Trimmer::creator, Trimmer::initialize );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	status = plugin.registerShape( Shape::typeName, Shape::id, Shape::creator,
		Shape::initialize, MesherUI::creator );
	if (!status) {
		status.perror("registerNode");
		return status;
	}

	status = plugin.registerCommand( "grow", GrowerCmd::creator, GrowerCmd::syntax );
	if (!status) {
		status.perror("registerCommand");
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

	status = plugin.deregisterNode( Shape::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}

	status = plugin.deregisterCommand( "grow" );
	if (!status) {
		status.perror("deregisterCommand");
		return status;
	}


	return status;
}
