cmake_minimum_required(VERSION 2.6)

project(Grower)

set ( MAYA_PLUGIN_NAME "Grower" )

# Platform specific definitions
if(WIN32)
    if(MSVC)
        # To enable /MP, parralel build on MSVC
        option(WIN32_USE_MP "Set to OFF to diable /MP." ON)
        MARK_AS_ADVANCED(WIN32_USE_MP)
        if(WIN32_USE_MP)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        endif(WIN32_USE_MP)

        # Other MSVC compilation flags
        add_definitions(-D_SCL_SECURE_NO_WARNINGS)
        add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
		add_definitions(-D NOMINMAX) # prevent windef.h min/max macros from interfering with stl's
		add_definitions(-D REQUIRE_IOSTREAM)
		add_definitions(-D _BOOL)
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")

    endif(MSVC)
else()
add_definitions(-D REQUIRE_IOSTREAM)
add_definitions(-D _BOOL)
add_definitions(-D LINUX)
endif()

# Set the Maya version and architecture (default values)
set(MAYA_VERSION 2015 CACHE STRING "Maya Version")
set(MAYA_ARCH x64 CACHE STRING "HW Architecture")

if(WIN32)
	set( MAYA_USER_DIR $ENV{USERPROFILE}/Documents/maya CACHE PATH "Maya user home directory" )
	set( MAYA_ROOT $ENV{PROGRAMFILES}/Autodesk/Maya${MAYA_VERSION} CACHE PATH "Maya installation directory" )
	set( MAYA_SDK_DEFINITIONS "_AFXDLL;_MBCS;NT_PLUGIN;REQUIRE_IOSTREAM" )
    set( MAYA_HEADERS_DIR ${MAYA_ROOT}/include )
    set( MAYA_LIBRARY_DIR ${MAYA_ROOT}/lib )
else()
  set( MAYA_HEADERS_DIR "/usr/autodesk/maya/include" CACHE PATH "Maya Include Dir" )
  set( MAYA_LIBRARY_DIR "/usr/autodesk/maya/lib" CACHE PATH "Maya Library Dir" )
endif()

set( CORELIB_INCLUDE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/CoreLib/include CACHE STRING "path to coreLib include folder" )
set( CORELIB_LIBRARY_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/CoreLib/lib CACHE STRING "path to coreLib libray folder" )

set( RENDERLIB_INCLUDE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/RenderLib/include CACHE STRING "path to renderlib include folder" )
set( RENDERLIB_LIBRARY_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/RenderLib/lib CACHE STRING "path to renderlib libray folder" )

set ( LOCAL_WARNING_FLAGS /W3 )
set ( LOCAL_RTTI_FLAGS /GR )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# add base to include directories
include_directories ( ${PUBLIC_INCLUDE_DIRS} ${MAYA_HEADERS_DIR} ${RENDERLIB_INCLUDE_DIR} ${CORELIB_INCLUDE_DIR})
link_directories ( ${PUBLIC_LIBRARY_DIRS} ${MAYA_LIBRARY_DIR} ${RENDERLIB_LIBRARY_DIR} ${CORELIB_LIBRARY_DIR})

if(WIN32)
set( RENDER_LIB RenderLib.lib )
set( CORE_LIB CoreLib.lib )
else()
set( RENDER_LIB RenderLib )
set( CORE_LIB CoreLib )
add_library( ${RENDER_LIB} STATIC IMPORTED)
set_property(TARGET ${RENDER_LIB} PROPERTY IMPORTED_LOCATION ${RENDERLIB_LIBRARY_DIR}/${RENDER_LIB}.a)
add_library( ${CORE_LIB} STATIC IMPORTED)
set_property(TARGET ${CORE_LIB} PROPERTY IMPORTED_LOCATION ${CORELIB_LIBRARY_DIR}/${CORE_LIB}.a)
endif()
set ( MAYA_DEFINITIONS "_AFXDLL,_MBCS,NT_PLUGIN,REQUIRE_IOSTREAM" )

if(WIN32)
set( MAYASDK_LIBRARIES "Foundation.lib" "OpenMaya.lib" "OpenMayaUI.lib" "OpenMayaAnim.lib" "OpenMayaFX.lib" "OpenMayaRender.lib" )
else()
set( MAYASDK_LIBRARIES "Foundation" "OpenMaya" "OpenMayaUI" "OpenMayaAnim" "OpenMayaFX" "OpenMayaRender" )
endif()

#specify app sources
file(GLOB_RECURSE SOURCE_FILES src/*.c src/*.cpp src/*.h src/*.inl src/*.hpp src/*.glsl src/*.ui)

add_library( ${MAYA_PLUGIN_NAME} SHARED ${SOURCE_FILES} )
target_link_libraries( ${MAYA_PLUGIN_NAME} ${MAYASDK_LIBRARIES} ${RENDER_LIB} ${CORE_LIB})

set_target_properties( ${MAYA_PLUGIN_NAME} PROPERTIES COMPILE_DEFINITIONS ${MAYA_DEFINITIONS} )
set_target_properties( ${MAYA_PLUGIN_NAME} PROPERTIES OUTPUT_NAME ${MAYA_PLUGIN_NAME} )
if(WIN32)
set_target_properties( ${MAYA_PLUGIN_NAME} PROPERTIES SUFFIX ".mll" )
else()
set_target_properties( ${MAYA_PLUGIN_NAME} PROPERTIES SUFFIX ".so" )
endif()
set_target_properties( ${MAYA_PLUGIN_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1 )
if(WIN32)
set_target_properties( ${MAYA_PLUGIN_NAME} PROPERTIES LINK_FLAGS "/export:initializePlugin /export:uninitializePlugin" )
endif()

