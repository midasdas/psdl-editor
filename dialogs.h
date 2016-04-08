#ifndef __DLGS_H__
#define __DLGS_H__

#include "resource.h"
#include "options.h"
#include "ctrls.h"
#include "io_error.h"
#include "thread.h"

#include <vector>
#include <algorithm>

struct TransformProps
{
	float fX, fY, fZ, fAngle;

	TransformProps(void) :
		fX(0), fY(0), fZ(0), fAngle(0)
	{}
};

extern struct DuplicateProps
{
	unsigned int nCount;
	bool bVertices;
	bool bPerimeters;
	bool bNeighbours;
	bool bTextures;
	bool bExclude;
	std::vector<unsigned char> aAttributes;

	DuplicateProps(void) : nCount(1),
	                       bVertices(true),
	                       bPerimeters(true),
	                       bNeighbours(true),
	                       bTextures(false),
	                       bExclude(false)
	{}
}
g_duplicateProps;

extern struct OptimizeProps
{
	bool bTextureRefs;
	bool bTextures;
	bool bEmpty;

	OptimizeProps(void) : bTextureRefs(true),
	                      bTextures(false),
	                      bEmpty(false)
	{}
}
g_optimizeProps;

class CTransformDlg :
	public CDialogImpl<CTransformDlg>,
	public CWinDataExchange<CTransformDlg>
{
public:
	typedef CDialogImpl<CTransformDlg> baseClass;

	BEGIN_DDX_MAP(CTransformDlg)
		DDX_FLOAT(IDC_X,     m_sProps.fX)
		DDX_FLOAT(IDC_Y,     m_sProps.fY)
		DDX_FLOAT(IDC_Z,     m_sProps.fZ)
		DDX_FLOAT(IDC_ANGLE, m_sProps.fAngle)
	END_DDX_MAP()

	enum { IDD = IDD_TRANSFORM };

	BEGIN_MSG_MAP(CTransformDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDAPPLY,  OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		m_udX.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS);
		m_udX.SetBuddy(GetDlgItem(IDC_X));
		m_udX.SetRange(SHRT_MIN, SHRT_MAX);

		m_udY.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS);
		m_udY.SetBuddy(GetDlgItem(IDC_Y));
		m_udY.SetRange(SHRT_MIN, SHRT_MAX);

		m_udZ.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS);
		m_udZ.SetBuddy(GetDlgItem(IDC_Z));
		m_udZ.SetRange(SHRT_MIN, SHRT_MAX);

		m_udAngle.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_WRAP);
		m_udAngle.SetBuddy(GetDlgItem(IDC_ANGLE));
		m_udAngle.SetRange(-180, 180);

		NextDlgCtrl();

		return 1;
	}

	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&);

	CNumUpDownCtrl m_udX, m_udY, m_udZ, m_udAngle;
	TransformProps m_sProps;
};

class CDuplicateDlg :
	public CDialogImpl<CDuplicateDlg>,
	public CWinDataExchange<CDuplicateDlg>
{
public:

	BEGIN_DDX_MAP(CDuplicateDlg)
		DDX_UINT(IDC_COUNT,			g_duplicateProps.nCount)
		DDX_CHECK(IDC_VERTICES,		g_duplicateProps.bVertices)
		DDX_CHECK(IDC_PERIMETERS,	g_duplicateProps.bPerimeters)
		DDX_CHECK(IDC_NEIGHBOURS,	g_duplicateProps.bNeighbours)
		DDX_CHECK(IDC_TEXTURES,		g_duplicateProps.bTextures)
		DDX_CHECK(IDC_ATTRIBUTES,	g_duplicateProps.bExclude)
	END_DDX_MAP()

	enum { IDD = IDD_DUPLICATE };

	BEGIN_MSG_MAP(CDuplicateDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_RESET, OnReset)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		CenterWindow(GetParent());

		DoDataExchange(FALSE);

		CUpDownCtrl ud;
		ud.Create(m_hWnd, rcDefault, NULL,	WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT |
											UDS_ARROWKEYS | UDS_NOTHOUSANDS | UDS_SETBUDDYINT
		);
		ud.SetBuddy(GetDlgItem(IDC_COUNT));
		ud.SetRange(1, 100);

		return TRUE;
	}

	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		DoDataExchange(TRUE);
		EndDialog(wID);
		return FALSE;
	}

	LRESULT OnReset(WORD, WORD wID, HWND, BOOL&)
	{
		g_duplicateProps = DuplicateProps();
		DoDataExchange(FALSE);
		NextDlgCtrl();
		return TRUE;
	}
};

class CPerimetersDlg :
	public CDialogImpl<CPerimetersDlg>,
	public CWinDataExchange<CPerimetersDlg>
{
public:

	BEGIN_DDX_MAP(CPerimetersDlg)
		DDX_CHECK(IDC_NEIGHBOURS, g_options.dialogs.gen_perimeters.bNeighbours)
	END_DDX_MAP()

	enum { IDD = IDD_PERIMETERS };

	BEGIN_MSG_MAP(CPerimetersDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		CenterWindow(GetParent());
		DoDataExchange(FALSE);
		return TRUE;
	}

	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		DoDataExchange(TRUE);
		EndDialog(wID);
		return FALSE;
	}
};

class COptimizeDlg :
	public CDialogImpl<COptimizeDlg>,
	public CWinDataExchange<COptimizeDlg>
{
public:

	BEGIN_DDX_MAP(CDuplicateDlg)
		DDX_CHECK(IDC_TEXTURE_REFS,	g_optimizeProps.bTextureRefs)
		DDX_CHECK(IDC_TEXTURES,		g_optimizeProps.bTextures)
		DDX_CHECK(IDC_EMPTY_BLOCKS,	g_optimizeProps.bEmpty)
	END_DDX_MAP()

	enum { IDD = IDD_OPTIMIZE };

	BEGIN_MSG_MAP(CDuplicateDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		CenterWindow(GetParent());
		DoDataExchange(FALSE);
		return TRUE;
	}

	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		DoDataExchange(TRUE);
		EndDialog(wID);
		return FALSE;
	}
};

template <class T>
class CCenterFileDialogImpl : public CFileDialogImpl<T>
{
public:

	BEGIN_MSG_MAP(CCenterFileDialog)
		CHAIN_MSG_MAP(CFileDialogImpl<T>)
	END_MSG_MAP()

	CCenterFileDialogImpl(
		BOOL bOpenFileDialog,
		LPCTSTR lpszDefExt		= NULL,
		LPCTSTR lpszFileName	= NULL,
		DWORD dwFlags			= OFN_HIDEREADONLY,
		LPCTSTR lpszFilter		= NULL,
		HWND hWndParent			= NULL)
		: CFileDialogImpl<T>(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, hWndParent)
	{ }

	void OnInitDone(LPOFNOTIFY lpon)
	{
		GetFileDialogWindow().CenterWindow(lpon->lpOFN->hwndOwner);
	}

	void SetCaption(LPTSTR szCaption)
	{
		m_ofn.lpstrTitle = szCaption;
	}

	char* GetFileExt(void)
	{
		return strrchr(m_szFileName, '.') + 1;
	}

	char* GetFolderPath(void)
	{
		char* pos = strrchr(m_szFileName, '\\');

		if (!pos)
			pos = strrchr(m_szFileName, '/');

		if (pos)
		{
			unsigned nLen = pos - m_szFileName;

			char* ret = new char[nLen + 1];
			*ret = '\0';
			strncat(ret, m_szFileName, nLen);

			return ret;
		}

		return NULL;
	}

	TCHAR* GetPathName(void)
	{
		return m_szFileName;
	}
};

class CCenterFileDialog : public CCenterFileDialogImpl<CCenterFileDialog>
{
public:
	CCenterFileDialog(
		BOOL bOpenFileDialog,
		LPCTSTR lpszDefExt		= NULL,
		LPCTSTR lpszFileName	= NULL,
		DWORD dwFlags			= OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter		= NULL,
		HWND hWndParent			= NULL)
		: CCenterFileDialogImpl<CCenterFileDialog>(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, hWndParent)
	{ }
};

typedef unsigned (_stdcall *USER_PROC)(void*);

class CProgressDlg;
class CProgressDlg : public CDialogImpl<CProgressDlg>
{
public:

	enum { IDD = IDD_PROGRESS };

	BEGIN_MSG_MAP(CProgressDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	CProgressDlg(USER_PROC pProc, void* pParams = NULL, CString strCaption = _T("Loading"), bool bAsync = false)
	{
		m_pProc = pProc;
		m_pData.pParams = pParams;
		m_pData.callbackFunc = &CProgressDlg::_SetProgress;
		m_strCaption = strCaption;
		m_bAsync = bAsync;
		m_bChanged = false;
		m_hThread = NULL;
		pThis = this;
	}

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		m_bar.Attach(GetDlgItem(IDC_PROGRESS));

		CenterWindow(GetParent());
		SetWindowText(m_strCaption);
		SetTimer(1, 10);

		m_hThread = (HANDLE) _beginthreadex(NULL, 0, m_pProc, &m_pData, 0, NULL);
		return 0;
	}

	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&)
	{
		KillTimer(1);
		return 0;
	}

	LRESULT OnTimer(UINT, WPARAM wParam, LPARAM, BOOL&)
	{
		UpdateProgress();
		if (!m_bAsync) UpdateProgressMsg();
		return 0;
	}

	void UpdateProgress(void)
	{
		if (m_iVal == 100) EndDialog(IDOK);
		else if (m_iVal >= 0) m_bar.SetPos(m_iVal);
	}

	void UpdateProgressMsg(void)
	{
		if (IsWindow() && !m_bChanged)
		{
			ATLTRACE("%s\n", m_strMsg);
			SetDlgItemText(IDC_INFOSTATIC, m_strMsg);
			m_bChanged = true;
		}
	}

	void SetProgress(CString strMsg = _T(""), int iVal = -1)
	{
		if (!strMsg.IsEmpty())
		{
			m_strMsg = strMsg;
			m_bChanged = false;
		}

		m_iVal = iVal;

		if (m_bAsync) UpdateProgressMsg();
	}

	static void _SetProgress(std::string strFmt = _T(""), int iVal = -1, ...)
	{
		size_t nArgs = std::count(strFmt.begin(), strFmt.end(), '%');

		va_list argList;
		va_start(argList, iVal);

		CString strMsg;
		strMsg.FormatV(strFmt.c_str(), argList);

		va_end(argList);

		pThis->SetProgress(strMsg, iVal);
	}

	error::code GetError(void)
	{
		DWORD dwCode;
		int ret = GetExitCodeThread(m_hThread, &dwCode);
		return ret == FALSE ? error::failure : dwCode == STILL_ACTIVE ? error::aborted : dwCode;
	}

	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		TerminateThread(m_hThread, error::aborted);
		EndDialog(wID);
		return 0;
	}

private:

	static CProgressDlg* pThis;

	HANDLE m_hThread;
	USER_PROC m_pProc;
	ThreadData m_pData;

	CProgressBarCtrl m_bar;

	CString m_strCaption, m_strMsg;
	unsigned m_iVal;
	bool m_bAsync;
	bool m_bChanged;
};

#endif
