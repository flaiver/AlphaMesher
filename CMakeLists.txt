cmake_minimum_required(VERSION 2.8)
project(AlphaMesher)

set (SOURCES
	AlphaMesher.cpp
	mainPlugin.cpp
)

MESSAGE(STATUS ${SOURCES})


set (MAYA_PATH "/usr/autodesk/maya2017")
set (MAYA_LIB_PATH "${MAYA_PATH}/lib")
set (MAYA_INC_PATH "${MAYA_PATH}/include")

set(MAYA_LIBS
    Foundation.so
    OpenMaya.so
    )

set(FULLPATH_MAYA_LIBS)
foreach(l ${MAYA_LIBS})
    list(APPEND ${l})
endforeach()

MESSAGE(STATUS ${FULLPATH_MAYA_LIBS})
INCLUDE_DIRECTORIES(${MAYA_INC_PATH})
add_library(AlphaMesherProj SHARED ${SOURCES} ${MAYA_LIB_PATH})

