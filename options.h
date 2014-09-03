#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <iostream>
#include <stack>

struct GlobalOptions
{
	struct GeneralOptions
	{
		bool test;

		GeneralOptions() : test(true) {}
	}
	general;

	struct DisplayOptions
	{
		bool bWireframe;

		DisplayOptions() : bWireframe(false) {}
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
};

extern GlobalOptions g_options;

#endif
