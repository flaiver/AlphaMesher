#include "AlphaMesher.h"

#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

MStatus initializePlugin(MObject obj)
{
	MStatus stat;
	MFnPlugin plugin(obj, "Christoph Genzwuerker");
	plugin.registerNode("AlphaMesher", AlphaMesher::id, AlphaMesher::creator, AlphaMesher::initialize);
	return stat;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus stat;
	MFnPlugin plugin(obj);
	plugin.deregisterNode(AlphaMesher::id);
	return stat;
}