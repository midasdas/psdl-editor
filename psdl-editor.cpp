#include "stdafx.h"
#include "psdl-editor.h"
#include "options.h"
#include "mainfrm.h"
#include "inioptions.h"

CAppModule _Module;
GlobalOptions g_options;

int Run(LPTSTR = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	RECT rc = { 0, 0, 960, 677 };

	CMainFrame wndMain;

	if (wndMain.CreateEx(NULL, rc) == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	wndMain.CenterWindow();
	wndMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();
	_Module.RemoveMessageLoop();

	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpstrCmdLine, int nCmdShow)
{
	CString strDir;
	GetCurrentDirectory(MAX_PATH, strDir.GetBuffer(MAX_PATH));
	strDir.ReleaseBuffer();

	unsigned i, nRecent;

	IniOptions ini(strDir + _T("\\psdl-editor.ini"));

	ini.SetSection("Files");
	g_options.files.strBrowseDir = ini.GetString("Browse_Dir");

	ini.SetSection("Tools");
	g_options.tools.strMM2Dir = ini.GetString("MM2_Dir", DEFAULT_MM2_DIR);
	g_options.tools.strMM2Exe = ini.GetString("MM2_Exe", DEFAULT_MM2_EXE);

	ini.SetSection("Recent");
	nRecent = ini.GetInt("Count");

	for (i = 1; i <= nRecent; i++)
	{
		CString strKey;
		strKey.Format("%u", i);
		g_options.files.aRecentFiles.push(ini.GetString(strKey));
	}

	::DefWindowProc(NULL, 0, 0, 0L);

	HRESULT hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();

	ini.SetSection("Files");
	ini.WriteString("Browse_Dir", g_options.files.strBrowseDir);

	ini.SetSection("Tools");
	ini.WriteString("MM2_Dir", g_options.tools.strMM2Dir);
	ini.WriteString("MM2_Exe", g_options.tools.strMM2Exe);

	ini.SetSection("Recent");
	nRecent = g_options.files.aRecentFiles.size();
	ini.WriteString(NULL);
	ini.WriteInt("Count", nRecent);

	for (i = 1; i <= nRecent; i++)
	{
		CString strKey;
		strKey.Format("%u", i);
		ini.WriteString(strKey, g_options.files.aRecentFiles.top().c_str());
		g_options.files.aRecentFiles.pop();
	}

	return nRet;
}
