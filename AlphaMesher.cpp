#include "AlphaMesher.h"

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MDataHandle.h>
#include <maya/MColor.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MImage.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>

MTypeId AlphaMesher::id(0x89543);
MObject AlphaMesher::m_inTexture;
MObject AlphaMesher::m_outMesh;
MObject AlphaMesher::m_inScaleValue;
MObject AlphaMesher::m_inResolution;

AlphaMesher::AlphaMesher()
{
}


AlphaMesher::~AlphaMesher()
{
}

MStatus AlphaMesher::initialize()
{
	MStatus stat;
	MFnNumericAttribute nAttr;
	m_inTexture = nAttr.createColor("inTexture", "itx");
	nAttr.setKeyable(true);

	m_inScaleValue = nAttr.create("inScale", "isv", MFnNumericData::kDouble, 0.0, &stat);
	CHECK_MSTATUS(stat);

	m_inResolution = nAttr.create("inResolution", "ire", MFnNumericData::kDouble, 10.0f, &stat);
	CHECK_MSTATUS(stat);

	nAttr.setMin(1.0);
	nAttr.setMax(100.0f);
		
	MFnTypedAttribute tAttr;
	m_outMesh = tAttr.create("outMesh", "ome", MFnData::kMesh, &stat);
	CHECK_MSTATUS(stat);

	stat = addAttribute(m_inTexture);
	CHECK_MSTATUS(stat);

	stat = addAttribute(m_inScaleValue);
	CHECK_MSTATUS(stat);

	stat = addAttribute(m_outMesh);
	CHECK_MSTATUS(stat);

	stat = addAttribute(m_inResolution);
	CHECK_MSTATUS(stat);

	stat = attributeAffects(m_inTexture, m_outMesh);
	CHECK_MSTATUS(stat);

	stat = attributeAffects(m_inResolution, m_outMesh);
	CHECK_MSTATUS(stat);
	
	stat = attributeAffects(m_inScaleValue, m_outMesh);
	CHECK_MSTATUS(stat)

	return stat;
}

MStatus AlphaMesher::compute(const MPlug & plug, MDataBlock & data)
{
	MStatus stat;
	if (plug == m_outMesh)
	{
		MDataHandle colorHandle = data.inputValue(m_inTexture, &stat);
		CHECK_MSTATUS(stat);

		MFloatVector & color = data.inputValue(m_inTexture).asFloatVector();

		// get somehow the resolution
		MPlug texturePlug(thisMObject(), m_inTexture);

		MPlugArray connectedPlugs;
		bool isConnected = texturePlug.connectedTo(connectedPlugs, true, false, &stat);
		
		if (!isConnected)
		{
			std::cerr << "AlphaMesher: input is not connected - abort" << std::endl;
			return MS::kInvalidParameter;
		}

		// take the first connected input texture
		// TODO: improve the looping accross multiple textures

		MPlug textureOutputPlug = connectedPlugs[0];
		MObject textureObj = textureOutputPlug.node();
		MFnDependencyNode textureNode(textureObj, &stat);
		CHECK_MSTATUS(stat);

		MDataHandle scaleHandle = data.inputValue(m_inScaleValue, &stat);
		CHECK_MSTATUS(stat);

		MDataHandle resolutionHandle = data.inputValue(m_inResolution, &stat);
		CHECK_MSTATUS(stat);

		double scaleValue = scaleHandle.asDouble();
		double resolutionValue = resolutionHandle.asDouble();

		if (textureNode.typeName() != "file")
		{
			std::cerr << "AlphaMesher: connected node is not a file node - abort" << std::endl;
			return MS::kInvalidParameter;
		}

		MPlug fileTextureNodePathPlug = textureNode.findPlug("fileTextureName", &stat);
		CHECK_MSTATUS(stat);
		
		MString fileTextureNodePath = fileTextureNodePathPlug.asString();
		MImage depthImage;
		stat = depthImage.readFromFile(fileTextureNodePath);
		CHECK_MSTATUS(stat);

		unsigned width, height;
		depthImage.getSize(width, height);
		unsigned char* pixels = depthImage.pixels();
		unsigned numChannels = 3;
		if (depthImage.isRGBA())
		{
			numChannels = 4;
		}
		// this will prove that resolution can be never smaller then pixels (width or height) / 2 
        double step = std::max(std::min(width / resolutionValue, height / resolutionValue), 2.0);

		MFnMeshData meshData;
		MObject meshObj = meshData.create();

		MPointArray vertexArray;

		MIntArray polyCountArray;
		MIntArray faceConnectArray;

		unsigned width2 = width / step;
		unsigned height2 = height / step;
		for (unsigned i = 0; i < height2; i++)
		{	
			for (unsigned j = 0; j < width2; j++)
			{
				if (i != height2 - 1 && j != width2 - 1)
				{
					polyCountArray.append(4);
					for (unsigned vertexCount = 0; vertexCount < 4; vertexCount++)
					{
						if (vertexCount > 1)
						{
							// number of verticies per face (4) - 1 [starting from index 0 and not 1 ] = 3
							faceConnectArray.append((i * width2) + j + width2 + (3 - vertexCount));
						}
						else
						{
							faceConnectArray.append((i * width2) + j + vertexCount);
						}
					}
				}
				unsigned pixelIndex = (i * step * width * numChannels) + j * step * numChannels;

				vertexArray.append(MPoint(double(j), double(i), scaleValue * unsigned(pixels[pixelIndex])));
			}
		}
		MFnMesh meshFn;
		meshFn.create(vertexArray.length(), polyCountArray.length(), vertexArray, polyCountArray, faceConnectArray, meshObj, &stat);
		CHECK_MSTATUS(stat);
		
		MDataHandle outputHandle = data.outputValue(m_outMesh, &stat);
		CHECK_MSTATUS(stat);

		outputHandle.setMObject(meshObj);
		outputHandle.setClean();
		data.setClean(plug);

		return MS::kSuccess;
	}
	return MS::kSuccess;
}

 
