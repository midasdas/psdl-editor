#ifndef __PSDL_EDITOR_H__
#define __PSDL_EDITOR_H__

#include "psdl.h"
#include "mainfrm.h"

#include <iostream>

#define DEFAULT_MM2_DIR "%PROGRAMFILES%\\Microsoft Games\\Midtown Madness 2"
#define DEFAULT_MM2_EXE DEFAULT_MM2_DIR "\\Midtown2.exe"

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

	struct FilesOptions
	{
		char* browseDir;
	}
	files;

	struct ToolsOptions
	{
		char* mm2Dir;
		char* mm2Exe;
	}
	tools;
};

extern GlobalOptions g_options;

#endif
