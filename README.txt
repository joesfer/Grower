Description:
	
	Maya plugin implementing the Space Colonization algorithm to grow
	vein-like structures around objects. 
	
	Visit http://www.joesfer.com/?p=46 for further information.

License:

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

Compilation:

	- Clone the git repository into a local folder:

		mkdir <grower_folder>
		cd <grower_folder>
		git clone git://github.com/joesfer/Grower.git 

	- The project depends on the RenderLib shared library, included as a submodule.
		in <grower_folder>
		git submodules init
		git submodules update
		
		git should say Cloning into 'src/RenderLib'...
		
	- Build RenderLib using CMake:

		cd <grower_folder>/src/RenderLib
		mkdir .build
		cd .build
		cmake ..

		Under windows: cmake will generate a Visual studio solution on .build
		Under linux: cmake will generate a GCC makefile

		Build the library. If everything went well, a new folder structure 
		<grower_folder>/src/RenderLib/lib containing the static library 
		should have been generated.
		
	- Build the Grower plugin using CMake:

		cd <grower_folder>
		mkdir .build
		cd .build
		cmake ..
		
		Under windows: cmake will generate a Visual studio solution on .build
		Under linux: cmake will generate a GCC makefile

		Build the plugin using visual studio or make.

		This will find the precompiled renderLib and build the .mll plugin
		under <grower_folder>/bin

	- Load the .mll file in Maya's plugin manager.