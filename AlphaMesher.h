#pragma once
#include <maya/MPxNode.h>


class AlphaMesher :
	public MPxNode
{
public:
	AlphaMesher();
	virtual ~AlphaMesher();

	static MStatus initialize();
	virtual MStatus compute(const MPlug& plug, MDataBlock& data);
	static void* creator() { return new AlphaMesher;  }

	static MObject m_inTexture;
	static MObject m_outMesh;
	static MObject m_inScaleValue;
	static MObject m_inResolution;

	static MTypeId id;
};

