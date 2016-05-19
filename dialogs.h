#ifndef __DLGS_H__
#define __DLGS_H__

#include "resource.h"
#include "config.h"
#include "ctrls.h"
#include "io_error.h"
#include "thread.h"

#include <vector>
#include <algorithm>
#include <ctime>

struct TransformProps
{
	float fX, fY, fZ, fAngle;

	TransformProps(void) :
		fX(0), fY(0), fZ(0), fAngle(0)
	{}
};

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
		DDX_UINT(IDC_COUNT,       config.dialogs.duplicate.nCount)
		DDX_CHECK(IDC_VERTICES,   config.dialogs.duplicate.bVertices)
		DDX_CHECK(IDC_PERIMETERS, config.dialogs.duplicate.bPerimeters)
		DDX_CHECK(IDC_NEIGHBOURS, config.dialogs.duplicate.bNeighbours)
		DDX_CHECK(IDC_TEXTURES,   config.dialogs.duplicate.bTextures)
		DDX_CHECK(IDC_ATTRIBUTES, config.dialogs.duplicate.bExclude)
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
		config.dialogs.duplicate = GlobalOptions::Dialogs::DuplicateBlocks();
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
		DDX_CHECK(IDC_NEIGHBOURS,	config.dialogs.perimeters.bNeighbours)
		DDX_FLOAT(IDC_EXTEND,		config.dialogs.perimeters.fExtend)
	END_DDX_MAP()

	enum { IDD = IDD_PERIMETERS };

	BEGIN_MSG_MAP(CPerimetersDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		m_ud1.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS);
		m_ud1.SetBuddy(GetDlgItem(IDC_EXTEND));
		m_ud1.SetRange(SHRT_MIN, SHRT_MAX);
		m_ud1.SetIncrement(0.5);

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

private:
	CNumUpDownCtrl m_ud1;
};

class COptimizeDlg :
	public CDialogImpl<COptimizeDlg>,
	public CWinDataExchange<COptimizeDlg>
{
public:

	BEGIN_DDX_MAP(CDuplicateDlg)
		DDX_CHECK(IDC_TEXTURE_REFS, config.dialogs.optimize.bTextureRefs)
		DDX_CHECK(IDC_TEXTURES,     config.dialogs.optimize.bTextures)
		DDX_CHECK(IDC_EMPTY_BLOCKS, config.dialogs.optimize.bEmpty)
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

class CGenerateBAIDlg :
	public CDialogImpl<CGenerateBAIDlg>,
	public CWinDataExchange<CGenerateBAIDlg>
{
public:

	BEGIN_DDX_MAP(CGenerateBAIDlg)
		DDX_CHECK(IDC_CHECK1, config.dialogs.generateBAI.bRoutes)
		DDX_CHECK(IDC_CHECK4, config.dialogs.generateBAI.bCulling)
		DDX_RADIO(IDC_RADIO1, config.dialogs.generateBAI.iCullingMethod)
		DDX_FLOAT(IDC_SPIN1,  config.dialogs.generateBAI.fTolerance)
		DDX_FLOAT(IDC_SPIN2,  config.dialogs.generateBAI.fLaneWidth)
		DDX_FLOAT(IDC_SPIN3,  config.dialogs.generateBAI.fRadius1)
		DDX_FLOAT(IDC_SPIN4,  config.dialogs.generateBAI.fRadius2)
	END_DDX_MAP()

	enum { IDD = IDD_GENERATE_BAI };

	BEGIN_MSG_MAP(CGenerateBAIDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_RESET, OnReset)
		COMMAND_HANDLER(IDC_CHECK1, BN_CLICKED, OnCheckClicked)
		COMMAND_HANDLER(IDC_CHECK4, BN_CLICKED, OnCheckClicked)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		CenterWindow(GetParent());
		DoDataExchange(FALSE);

		CString sPath(config.dialogs.generateBAI.strConfigFile.c_str());
		SetDlgItemText(IDC_PATH, sPath);

		if (!sPath.IsEmpty())
		{
			SendDlgItemMessage(IDC_CHECK1, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(IDC_CHECK3, BM_SETCHECK, BST_UNCHECKED, 0);
			SendDlgItemMessage(IDC_CHECK4, BM_SETCHECK, BST_UNCHECKED, 0);
			EnableControlGroup(IDC_CHECK1, false);
			EnableControlGroup(IDC_CHECK4, false);
		}

		const DWORD dwStyle = WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS;

		m_ud1.Create(m_hWnd, rcDefault, NULL, dwStyle);
		m_ud2.Create(m_hWnd, rcDefault, NULL, dwStyle);
		m_ud3.Create(m_hWnd, rcDefault, NULL, dwStyle);
		m_ud4.Create(m_hWnd, rcDefault, NULL, dwStyle);
		
		m_ud1.SetBuddy(GetDlgItem(IDC_SPIN1));
		m_ud2.SetBuddy(GetDlgItem(IDC_SPIN2));
		m_ud3.SetBuddy(GetDlgItem(IDC_SPIN3));
		m_ud4.SetBuddy(GetDlgItem(IDC_SPIN4));

		m_ud1.SetRange(SHRT_MIN, SHRT_MAX);
		m_ud2.SetRange(SHRT_MIN, SHRT_MAX);
		m_ud3.SetRange(SHRT_MIN, SHRT_MAX);
		m_ud4.SetRange(SHRT_MIN, SHRT_MAX);

		m_ud1.SetIncrement(0.01);
		m_ud2.SetIncrement(0.10);
		m_ud3.SetIncrement(10.0);
		m_ud4.SetIncrement(10.0);

		return 1;
	}

	void EnableControlGroup(WORD wID, bool bEnable = true)
	{
		if (wID == IDC_CHECK1)
		{
			::EnableWindow(GetDlgItem(IDC_CHECK2),  bEnable);
			::EnableWindow(GetDlgItem(IDC_SPIN2),   bEnable);
			::EnableWindow(GetDlgItem(IDC_STATIC1), bEnable);
		}
		else if (wID == IDC_CHECK4)
		{
			::EnableWindow(GetDlgItem(IDC_RADIO1),  bEnable);
			::EnableWindow(GetDlgItem(IDC_RADIO2),  bEnable);
			::EnableWindow(GetDlgItem(IDC_SPIN3),   bEnable);
			::EnableWindow(GetDlgItem(IDC_SPIN4),   bEnable);
			::EnableWindow(GetDlgItem(IDC_STATIC2), bEnable);
			::EnableWindow(GetDlgItem(IDC_STATIC3), bEnable);
		}
	}

	LRESULT OnCheckClicked(WORD, WORD wID, HWND, BOOL&)
	{
		EnableControlGroup(wID, BST_CHECKED == ::SendMessage(GetDlgItem(wID), BM_GETCHECK, 0, 0));
		return 0;
	}

	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		DoDataExchange(TRUE);
		EndDialog(wID);
		return 0;
	}

	LRESULT OnReset(WORD, WORD wID, HWND, BOOL&)
	{
		config.dialogs.generateBAI = GlobalOptions::Dialogs::GenerateBAI();

		CString sPath(config.dialogs.generateBAI.strConfigFile.c_str());
		SetDlgItemText(IDC_PATH, sPath);

		EnableControlGroup(IDC_CHECK1, config.dialogs.generateBAI.bRoutes);
		EnableControlGroup(IDC_CHECK4, config.dialogs.generateBAI.bCulling);

		DoDataExchange(FALSE);
		return 1;
	}

private:
	CNumUpDownCtrl m_ud1, m_ud2, m_ud3, m_ud4;
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
		DWORD dwFlags			= OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
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

typedef unsigned (_stdcall Procedure)(void*);

class CProgressDlg2 : public CDialogImpl<CProgressDlg2>, public ProgressMonitor
{
public:

	enum { IDD = IDD_PROGRESS };

	BEGIN_MSG_MAP(CProgressDlg2)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	CProgressDlg2()
	{
		m_strCaption = "Working...";
		m_iOldValue = 0;
	}

	error::code Run(Procedure* pProc, void* pArgs = NULL)
	{
		m_pProc = pProc;
		m_pArgs = pArgs;
	//	return DoModal();

		if (DoModal() == IDCANCEL)
		{
			return error::aborted;
		}
		else
		{
			DWORD dwCode;
			if (GetExitCodeThread(m_hThread, &dwCode) != FALSE)
				return dwCode;
		}
		return error::failure;
	}

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		SetWindowText(m_strCaption);
		m_bar.Attach(GetDlgItem(IDC_PROGRESS));

		CenterWindow(GetParent());
	//	SetWindowText(m_strCaption);

		m_hThread = (HANDLE) _beginthreadex(NULL, 0, m_pProc, (ProgressMonitor*) this, 0, NULL);
		return 0;
	}

	void* GetArgs(void)
	{
		return m_pArgs;
	}

	virtual void setMaximum(unsigned int max)
	{
		m_bar.SetRange(0, 170);
		m_iMax = max;
	}
	virtual void setProgress(unsigned int value)
	{
//		clock_t time = clock();
//
//		if (time - m_clock > 10)
//		{
//			m_bar.SetPos(value * 170 / m_iMax);
//			m_clock = time;
//		}
		unsigned int iNewValue = value * 170 / m_iMax;

		if (iNewValue != m_iOldValue)
		{
			m_bar.SetPos(iNewValue);
			m_iOldValue = iNewValue;
		}
	}
	virtual void setNote(std::string strNote)
	{
	//	if (IsWindow())
	//	{
			ATLTRACE("%s\n", strNote.c_str());
			SetDlgItemText(IDC_INFOSTATIC, strNote.c_str());
	//	}
	}
	virtual void setCaption(std::string strCaption)
	{
		SetWindowText(strCaption.c_str());
	}
	void SetCaption(CString strCaption)
	{
		m_strCaption = strCaption;
	}
	virtual void done(void)
	{
		EndDialog(IDOK);
	}

	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		if (WaitForSingleObject(m_hThread, 1000) == WAIT_TIMEOUT)
		{
			TerminateThread(m_hThread, error::aborted); // Hrm...
		}
		CloseHandle(m_hThread);
		EndDialog(wID);
		return 0;
	}

private:

	CProgressBarCtrl m_bar;
	HANDLE m_hThread;
	Procedure *m_pProc;
	void *m_pArgs;
	CString m_strCaption;
	clock_t m_clock;

	unsigned int m_iMax;
	unsigned int m_iOldValue;
};

#endif
