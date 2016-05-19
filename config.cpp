#include "stdafx.h"
#include "config.h"

GlobalOptions config;

GlobalOptions::GlobalOptions()
{
	general.test = false;

	display.bAutoCenter = false;
	display.bWireframe = false;
	display.bTextures = true;
	display.bTextureNearest = false;
	display.bTestPVS = false;
	display.bTestAICulling = false;
	display.bRenderPSDL = true;
	display.bRenderBAI = true;
	display.eNumeral = hex;

//	directories.texturePaths.push_back("../texture/");

	dialogs.perimeters.bNeighbours = true;
	dialogs.perimeters.bDeleteExisting = true;
	dialogs.perimeters.bSelection = true;
	dialogs.perimeters.fExtend = 5.0f;

	dialogs.optimize.bTextureRefs = true;
	dialogs.optimize.bTextures = false;
	dialogs.optimize.bEmpty = false;
}

GlobalOptions::Dialogs::DuplicateBlocks::DuplicateBlocks()
{
	nCount = 1;
	bVertices = true;
	bPerimeters = true;
	bNeighbours = true;
	bTextures = false;
	bExclude = false;
}

GlobalOptions::Dialogs::GenerateBAI::GenerateBAI()
{
	strConfigFile = "";
	bRoutes = true;
	bCulling = true;
	iCullingMethod = 1;
	fTolerance = 0.01f;
	fLaneWidth = 5.0f;
	fRadius1 = 250.0f;
	fRadius2 = 150.0f;
}
