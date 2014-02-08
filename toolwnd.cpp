#include "stdafx.h"
#include "toolwnd.h"
#include "psdl-editor.h"
#include "strings.h"
#include "psdl.h"

CPropertiesWindow	g_properties;
CAttributesWindow	g_attributes;
CBlocksWindow		g_blocks;

void CAttributesWindow::SetBlock(PSDL::Block *block)
{
	g_block = block;

	m_list.ResetContent();

	if (!block) return;

	CString sText;

	m_list.SetRedraw(FALSE);
	for (size_t i = 0; i < block->numAttributes(); i++)
	{
		sText.Format("%x\t", i);
		m_list.AddString(sText + AttributeString(block->getAttribute(i)));
	}
	m_list.SetRedraw();
}

LRESULT CAttributesWindow::OnSelChange(WORD, WORD, HWND hWnd, BOOL&)
{
	g_properties.SetAttribute(g_block->getAttribute(m_list.GetCaretIndex()));
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
