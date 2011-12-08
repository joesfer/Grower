#include "Command.h"
#include <maya/MArgDatabase.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MGlobal.h>
#include <maya/MPoint.h>
#include <maya/MBoundingBox.h>
#include <maya/MMatrix.h>
#include <maya/MFnTransform.h>

GrowerCmd::GrowerCmd() {
}

MStatus GrowerCmd::doIt( const MArgList& args ) {

	// CREATE THE PARSER:
	MArgDatabase argData( syntax(), args);

	argData.getObjects( sel );
	return redoIt();
}

MStatus GrowerCmd::redoIt() {

	MStatus stat;
	if (sel.length() == 0) {
		MString msg = "A mesh or its transform node must be selected.";
		displayError(msg);
		return MStatus::kFailure;
	}

	MDagPath meshPath;
	sel.getDagPath( 0, meshPath );
	meshPath.extendToShape();
	if ( !meshPath.node().hasFn( MFn::kMesh ) ) {
		MString msg = "A mesh or its transform must be the first item selected.";
		displayError(msg);
		return MStatus::kFailure;
	}
	MFnDagNode meshNode( meshPath.node() );
	MFnTransform meshTransform = MFnTransform( meshPath.transform() );
	MBoundingBox meshBounds = meshNode.boundingBox();		 
	meshBounds.transformUsing( meshTransform.transformationMatrix() );

	MDagPath locatorPath;
	sel.getDagPath( 1, locatorPath );
	if ( locatorPath.node().isNull() ) {		
		
		// if no locator is provided, create one right next to te mesh

		MPoint locatorPos;
		locatorPos.x = meshBounds.min().x - meshBounds.width() * 0.05f;
		locatorPos.y = meshBounds.center().y;
		locatorPos.z = meshBounds.center().z;

		MStringArray locatorName;
		MGlobal::executeCommand( "spaceLocator", locatorName );
		MSelectionList list;
		MGlobal::getSelectionListByName( locatorName[ 0 ], list );
		list.getDagPath( 0, locatorPath );
		MFnTransform locatorTransform( locatorPath.transform() );
		locatorTransform.setTranslation( locatorPos, MSpace::kTransform );
	} 
	
	if ( !locatorPath.node().isNull() && !(locatorPath.node().hasFn( MFn::kLocator ) || locatorPath.node().hasFn( MFn::kTransform ) ) ) {
		MString msg = "A transform node must be the last item selected.";
		displayError(msg);
		return MStatus::kFailure;
	}

	MFnTransform locatorTransform( locatorPath.transform() );

	// Create sampler node
	MObject samplerNode;
	{
		MString res;
		MGlobal::executeCommand( "createNode \"Sampler\";", res, true );
		MSelectionList list;
		MGlobal::getSelectionListByName( res, list );
		list.getDependNode( 0, samplerNode );
		MString cmd = "connectAttr " + meshTransform.name() + ".outMesh " + res + ".inputMesh;";
		MGlobal::executeCommand( cmd, true );
	}

	// Create grower
	MObject growerNode;
	{
		MString res;
		MGlobal::executeCommand( "createNode \"Grower\";", res, true );
		MSelectionList list;
		MGlobal::getSelectionListByName( res, list );
		list.getDependNode( 0, growerNode );
		MString cmd = "connectAttr " + MFnDependencyNode( samplerNode ).name() + ".outSamples " + res + ".samples;";
		MGlobal::executeCommand( cmd, true );
		cmd = "connectAttr " + MFnDependencyNode( samplerNode ).name() + ".worldToLocal " + res + ".worldToLocal;";
		MGlobal::executeCommand( cmd, true );
		cmd = "connectAttr " + locatorTransform.name() + ".translate " + res + ".inputPos;";
		MGlobal::executeCommand( cmd, true );
	}

	// Create trimmer
	MObject trimmerNode;
	{
		MString res;
		MGlobal::executeCommand( "createNode \"Trimmer\";", res, true );
		MSelectionList list;
		MGlobal::getSelectionListByName( res, list );
		list.getDependNode( 0, trimmerNode );
		MString cmd = "connectAttr " + MFnDependencyNode( growerNode ).name() + ".output " + res + ".input;";
		MGlobal::executeCommand( cmd, true );
	}

	// Create shape
	MObject shapeNode;
	{
		MString res;
		MGlobal::executeCommand( "createNode \"GrowerShape\" -parent " + meshTransform.name() + ";", res, true );
		MSelectionList list;
		MGlobal::getSelectionListByName( res, list );
		list.getDependNode( 0, shapeNode );
		MGlobal::executeCommand( "connectAttr " + MFnDependencyNode( trimmerNode ).name() + ".output " + res + ".input;" );
		MString cmd = "setAttr \"" + res + ".thickness\" " + meshBounds.width() * 0.002f;
		MGlobal::executeCommand( cmd, true );
	}

	// Create output mesh
	{
		MString res;
		MGlobal::executeCommand( "createNode mesh -parent " + meshTransform.name() + ";", res, true );
		MGlobal::executeCommand( "connectAttr " + MFnDependencyNode( shapeNode ).name() + ".outMesh " + res + ".inMesh;" );
		MGlobal::executeCommand( "sets -e -forceElement initialShadingGroup " + res + ";" );
	}

	MFnDagNode locatorNode( locatorPath.transform() );
	MGlobal::executeCommand( "print(\"First selected " + meshNode.name() + "\");" );
	MGlobal::executeCommand( "print(\"Second selected " + locatorNode.name() + "\");" );

	return MS::kSuccess;
}

void* GrowerCmd::creator() {
	return new GrowerCmd;
}

MSyntax GrowerCmd::syntax() {
	MSyntax cmdSyntax;
	cmdSyntax.useSelectionAsDefault( true );
	cmdSyntax.setObjectType( MSyntax::kSelectionList, 1, 2 );
	cmdSyntax.enableQuery( true );
	cmdSyntax.enableEdit( false );

	return cmdSyntax;
}