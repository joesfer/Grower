/* 
	================================================================================
	Copyright (c) 2012, Jose Esteve. http://www.joesfer.com
	This software is released under the LGPL-3.0 license: http://www.opensource.org/licenses/lgpl-3.0.html	
	================================================================================
*/

#ifndef Command_h__
#define Command_h__

#include <maya/MTypeId.h>
#include <maya/MString.h>
#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MSelectionList.h>

class GrowerCmd : public MPxCommand {
public:
						GrowerCmd();

	// overrides
	virtual MStatus   	doIt( const MArgList& args );
	virtual MStatus   	redoIt( );
	virtual bool		hasSyntax() const { return true; }

	// methods
	static  void*		creator();
	static	MSyntax		syntax();
private:
	MSelectionList sel;
};

// Register all strings used by the plugin C++ code
MStatus registerGrowerCmdStrings(void);


#endif // Command_h__