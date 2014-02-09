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

//extern PSDL			*g_psdl;
extern PSDL::Block	*g_block;

extern CMainFrame* g_wndMain;

#endif
