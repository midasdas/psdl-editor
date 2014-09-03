#ifndef __ABOUTDLG_H__
#define __ABOUTDLG_H__

#include "resource.h"
#include "versionno.h"

class CAboutDlg : public CDialogImpl<CAboutDlg>
{
public:
	enum { IDD = IDD_ABOUTBOX };

	BEGIN_MSG_MAP(CAboutDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		CString sGet, sOut;

		sGet.LoadString(IDS_VERSION);
		sOut.Format(sGet, STRFILEVER);
		SetDlgItemText(IDC_VERSIONSTATIC, sOut);

		sGet.LoadString(IDS_INFO);
		sOut.Format(sGet, STRBUILDDATE);
		SetDlgItemText(IDC_INFOSTATIC, sOut);

		CenterWindow(GetParent());
		return 1;
	}

	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		EndDialog(wID);
		return 0;
	}
};

#endif
