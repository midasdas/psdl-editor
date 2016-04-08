#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <iostream>
#include <stack>

extern struct GlobalOptions
{
	struct GeneralOptions
	{
		bool test;

		GeneralOptions() : test(false) {}
	}
	general;

	struct DisplayOptions
	{
		bool bWireframe;
		bool bTextures;
		bool bTextureNearest;

		DisplayOptions() : bWireframe(false), bTextures(true), bTextureNearest(false) {}
	}
	display;

	struct FileOptions
	{
		std::string strBrowseDir;
		std::stack<std::string> aRecentFiles;
	}
	files;

	struct DirectoryOptions
	{
	//	std::string strBrowseDir;
	}
	directories;

	struct ToolOptions
	{
		std::string strMM2Dir;
		std::string strMM2Exe;
	}
	tools;

	struct Dialogs
	{
		struct GeneratePerimeters
		{
			bool bNeighbours;
			bool bDeleteExisting;
			bool bSelection;

			GeneratePerimeters() : bNeighbours(false), bDeleteExisting(true), bSelection(true) {}
		}
		gen_perimeters;
	}
	dialogs;
}
g_options;

#endif
