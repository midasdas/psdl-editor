#include "stdafx.h"
#include "dialogs.h"
#include "psdl-editor.h"

transformProps g_transformProps;
duplicateProps g_duplicateProps;
optimizeProps  g_optimizeProps;

CProgressDlg* CProgressDlg::pThis = NULL;

LRESULT CTransformDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
{
	if (wID == IDAPPLY)
	{
		DoDataExchange(TRUE);
		if (!CMainFrame::TransformCallback(m_sProps)) return 0;
	}
	DestroyWindow();
	return 0;
}
