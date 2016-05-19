#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <iostream>
#include <stack>
#include <vector>

enum Numeral
{
	hex,
	dec
};

extern struct GlobalOptions
{
	struct General
	{
		bool test;
	} general;

	struct
	{
		bool bAutoCenter;
		bool bWireframe;
		bool bTextures;
		bool bTextureNearest;
		bool bTestPVS;
		bool bTestAICulling;
		bool bRenderPSDL;
		bool bRenderBAI;
		Numeral eNumeral;
	} display;

	struct Files
	{
		std::string strBrowseDir;
		std::stack<std::string> aRecentFiles;
	} files;

	struct Directories
	{
		std::vector<std::string> texturePaths;
		std::vector<std::string> geometryPaths;
	} directories;

	struct Tools
	{
		std::string strMM2Dir;
		std::string strMM2Exe;
		std::string strSDLViewExe;
	} tools;

	struct Dialogs
	{
		struct DuplicateBlocks
		{
			unsigned long nCount;
			bool bVertices;
			bool bPerimeters;
			bool bNeighbours;
			bool bTextures;
			bool bExclude;
			std::vector<unsigned char> aAttributes;

			DuplicateBlocks();
		} duplicate;

		struct GeneratePerimeters
		{
			bool bNeighbours;
			bool bDeleteExisting;
			bool bSelection;

			float fExtend;
		} perimeters;

		struct OptimizePSDL
		{
			bool bTextureRefs;
			bool bTextures;
			bool bEmpty;
		} optimize;

		struct GenerateBAI
		{
			std::string strConfigFile;

			bool  bRoutes;
			bool  bCulling;
			int   iCullingMethod;

			float fTolerance;
			float fLaneWidth;
			float fRadius1;
			float fRadius2;

			GenerateBAI();
		} generateBAI;

	} dialogs;

	GlobalOptions();
}
config;

#endif
