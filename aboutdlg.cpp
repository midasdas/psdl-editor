#include "aboutdlg.h"
#include "versionno.h"

LRESULT CAboutDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
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
