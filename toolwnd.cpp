#include "stdafx.h"
#include "toolwnd.h"
#include "psdl-editor.h"
#include "strings.h"
#include "psdl.h"

void CAttributesWindow::SetBlock(psdl::block* pBlock)
{
	m_pBlock = pBlock;

	m_list.ResetContent();

	if (!m_pBlock) return;

	CString sText;

	m_list.SetRedraw(FALSE);
	for (size_t i = 0; i < m_pBlock->numAttributes(); i++)
	{
		sText.Format("%x\t", i);
		m_list.AddString(sText + AttributeString(m_pBlock->get_attribute(i)));
	}
	m_list.SetRedraw();
}

LRESULT CAttributesWindow::OnSelChange(WORD, WORD, HWND hWnd, BOOL&)
{
	g_wndMain->SelectAttribute(m_pBlock, m_list.GetCaretIndex());
	return 0;
}

LRESULT CAttributesWindow::OnSetFocus(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	ATLTRACE("CAttributesWindow::OnSetFocus\n");

//	g_properties.SetAttribute(m_block->GetAttribute(m_list.GetNextItem(-1, LVNI_FOCUSED)));
	return 0;
}

LRESULT CBlocksWindow::OnSelChange(WORD, WORD, HWND hWnd, BOOL&)
{
	static int iLast = -1;
	int iCurrent = m_list.GetCaretIndex();

	if (iLast != iCurrent)
	{
		g_wndMain->SelectBlock(iCurrent);
		iLast = iCurrent;
	}

	return 0;
}
