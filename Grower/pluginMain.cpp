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
