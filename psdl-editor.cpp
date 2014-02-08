#include "stdafx.h"
#include "psdl-editor.h"
#include "mainfrm.h"

CAppModule _Module;
GlobalOptions g_options;

//PSDL		*g_psdl;
PSDL::Block	*g_block;

CMainFrame* g_wndMain;

int Run(LPTSTR = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	g_wndMain = new CMainFrame();

	if (g_wndMain->CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	g_wndMain->ShowWindow(nCmdShow);

	int nRet = theLoop.Run();
	_Module.RemoveMessageLoop();

	delete g_wndMain;

	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpstrCmdLine, int nCmdShow)
{
	TCHAR lpDir[MAX_PATH],lpFile[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, lpDir);
	sprintf(lpFile, "%s\\psdl-editor.ini", lpDir);

	g_options.files.browseDir	= new char[MAX_PATH];
	g_options.tools.mm2Dir		= new char[MAX_PATH];
	g_options.tools.mm2Exe		= new char[MAX_PATH];

	GetPrivateProfileString("Files", "Browse_Dir", NULL, g_options.files.browseDir, MAX_PATH, lpFile);
	GetPrivateProfileString("Tools", "MM2_Dir", DEFAULT_MM2_DIR, g_options.tools.mm2Dir, MAX_PATH, lpFile);
	GetPrivateProfileString("Tools", "MM2_Exe", DEFAULT_MM2_EXE, g_options.tools.mm2Exe, MAX_PATH, lpFile);

	//

	HRESULT hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));

	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	//

	WritePrivateProfileString("Files", "Browse_Dir", g_options.files.browseDir, lpFile);
	WritePrivateProfileString("Tools", "MM2_Dir", g_options.tools.mm2Dir, lpFile);
	WritePrivateProfileString("Tools", "MM2_Exe", g_options.tools.mm2Exe, lpFile);
	
	return nRet;
}
