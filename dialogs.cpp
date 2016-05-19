#include "stdafx.h"
#include "dialogs.h"
#include "mainfrm.h"

CProgressDlg* CProgressDlg::pThis = NULL;

LRESULT CTransformDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
{
	if (wID == IDAPPLY)
	{
		DoDataExchange(TRUE);
		if (!CMainFrame::_TransformEntities(m_sProps)) return 0;
	}
	DestroyWindow();
	return 0;
}
