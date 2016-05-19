#include "stdafx.h"
#include "psdl-editor.h"
#include "config.h"
#include "mainfrm.h"
#include "inioptions.h"

CAppModule _Module;

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

	unsigned int i, nCount;

	IniOptions ini(strDir + _T("\\psdl-editor.ini"));

	ini.SetSection("Files");
	config.files.strBrowseDir = ini.GetString("Browse_Dir");

	ini.SetSection("TexturePaths");
	nCount = ini.GetInt("Count");
	for (i = 1; i <= nCount; ++i)
	{
		CString strKey;
		strKey.Format("%u", i);
		config.directories.texturePaths.push_back(ini.GetString(strKey));
	}

	ini.SetSection("Tools");
	config.tools.strMM2Dir = ini.GetString("MM2_Dir", DEFAULT_MM2_DIR);
	config.tools.strMM2Exe = ini.GetString("MM2_Exe", DEFAULT_MM2_EXE);
	config.tools.strSDLViewExe = ini.GetString("SDLView_Exe");

	ini.SetSection("Recent");
	nCount = ini.GetInt("Count");
	for (i = 1; i <= nCount; ++i)
	{
		CString strKey;
		strKey.Format("%u", i);
		config.files.aRecentFiles.push(ini.GetString(strKey));
	}

	::DefWindowProc(NULL, 0, 0, 0L);

	HRESULT hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();

	ini.SetSection("Files");
	ini.WriteString("Browse_Dir", config.files.strBrowseDir);

	ini.SetSection("TexturePaths");
	nCount = config.directories.texturePaths.size();
	ini.WriteString(NULL);
	ini.WriteInt("Count", nCount);
	for (i = 1; i <= nCount; ++i)
	{
		CString strKey;
		strKey.Format("%u", i);
		ini.WriteString(strKey, config.directories.texturePaths[i-1].c_str());
	}

	ini.SetSection("Tools");
	ini.WriteString("MM2_Dir", config.tools.strMM2Dir);
	ini.WriteString("MM2_Exe", config.tools.strMM2Exe);
	ini.WriteString("SDLView_Exe", config.tools.strSDLViewExe);

	ini.SetSection("Recent");
	nCount = config.files.aRecentFiles.size();
	ini.WriteString(NULL);
	ini.WriteInt("Count", nCount);
	for (i = 1; i <= nCount; ++i)
	{
		CString strKey;
		strKey.Format("%u", i);
		ini.WriteString(strKey, config.files.aRecentFiles.top().c_str());
		config.files.aRecentFiles.pop();
	}

	return nRet;
}
