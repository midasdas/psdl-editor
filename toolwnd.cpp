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
	for (unsigned long i = 0; i < m_pBlock->num_attributes(); i++)
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

void CPerimeterWindow::SetBlock(psdl::block* pBlock)
{
	m_pBlock = pBlock;

	m_list.ResetContent();

	if (!m_pBlock) return;

	m_list.SetRedraw(FALSE);
	for (unsigned long i = 0; i < m_pBlock->num_perimeters(); i++)
	{
		CString sText;
		CString sNb("-");

		sText.Format("%x\t%x,\t", i, m_pBlock->get_perimeter_point(i)->vertex);

		unsigned short iNb = m_pBlock->get_perimeter_point(i)->block;

		if (iNb > 0)
			sNb.Format("%x", iNb);

		m_list.AddString(sText + sNb);
	}
	m_list.SetRedraw();
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
