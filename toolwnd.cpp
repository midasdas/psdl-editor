#include "stdafx.h"
#include "toolwnd.h"
#include "psdl-editor.h"
#include "strings.h"
#include "psdl.h"

void CAttributesWindow::SetBlock(psdl::block* pBlock)
{
	m_list.DeleteAllItems();

	if (!pBlock) return;

	m_pBlock = pBlock;

	m_list.SetRedraw(FALSE);

	for (unsigned long i = 0; i < pBlock->num_attributes(); i++)
		m_list.InsertItem(i, _T(""));

	m_list.SetRedraw();
	AdjustColumns();
}

LRESULT CAttributesWindow::OnItemChanged(int, LPNMHDR lpnmh, BOOL&)
{
	NMLISTVIEW* nm = (NMLISTVIEW*) lpnmh;
	if (nm->uNewState & LVIS_FOCUSED)
		CMainFrame::SelectAttribute(m_pBlock, nm->iItem);
	return 0;
}

void CAttributesWindow::OnContextMenu(CWindow wnd, CPoint pt)
{
	CMenu menu;
	menu.LoadMenu(IDR_POPUP_ATTRIBUTE);
	menu.GetSubMenu(0).TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, CMainFrame::GetWnd());
}

void CAttributesWindow::OnDrawItem(UINT, LPDRAWITEMSTRUCT lpdis)
{
	CDCHandle dc(lpdis->hDC);

	dc.SetTextColor(GetSysColor(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
	dc.SetBkColor  (GetSysColor(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT     : COLOR_BTNFACE));

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int y1 = (lpdis->rcItem.bottom + lpdis->rcItem.top - tm.tmHeight) / 2;

	CString str;
	RECT rc;

	m_list.GetSubItemRect(lpdis->itemID, 1, LVIR_BOUNDS, &rc);
	rc.left = 1;
	rc.right--;

	// Subitem 0
	str.Format("%0*x", m_nDigits, lpdis->itemID);
	dc.ExtTextOut(rc.left + 3, y1, ETO_CLIPPED | ETO_OPAQUE, &rc, str, str.GetLength(), NULL);

	rc.left = rc.right++;
	dc.FillRect(&rc, GetSysColorBrush(COLOR_BTNSHADOW));
	rc.left = 0;
	rc.right = 1;
	dc.FillRect(&rc, GetSysColorBrush(COLOR_BTNHILIGHT));

	psdl::attribute* pAtb = m_pBlock->get_attribute(lpdis->itemID);

	// Subitem 1
	const CBitmap bmp = CreateBitmap(4, 4, 1, 1, wBullet);
	CDC dcMem;
	dcMem.CreateCompatibleDC(dc);
	dcMem.SelectBitmap(bmp);

	m_list.GetSubItemRect(lpdis->itemID, 1, LVIR_BOUNDS, &rc);

	int y = (rc.top + rc.bottom - 4) / 2;

	if (pAtb->enabled)
		dc.BitBlt(rc.left + 5, y, 4, 4, dcMem, 0, 0, SRCCOPY);
//	else
//		m_ImageList.Draw(dc, 3, rc.left, y - 4, ILD_NORMAL);

	// Subitem 2
	if (!(lpdis->itemState & ODS_SELECTED))
		dc.SetBkColor(pAtb->type == ATB_TEXTURE ? RGB_AVERAGE(GetSysColor(COLOR_BTNFACE), GetSysColor(COLOR_BTNHILIGHT)) : GetSysColor(COLOR_WINDOW));

	m_list.GetSubItemRect(lpdis->itemID, 2, LVIR_BOUNDS, &rc);
//	rc.right += 2;

	str = AttributeString(pAtb);
	dc.ExtTextOut(rc.left + 4, y1, ETO_CLIPPED | ETO_OPAQUE, &rc, str, str.GetLength());

/*	if ((pAtb = m_pBlock->get_attribute(lpdis->itemID + 1)) && pAtb->type == ATB_TEXTURE)
	{
		rc.top = rc.bottom - 1;
		dc.FillRect(&rc, GetSysColorBrush(COLOR_BTNSHADOW));
	}*/
}

void CAttributesWindow::OnMeasureItem(UINT, LPMEASUREITEMSTRUCT lpmis)
{
	lpmis->itemHeight = 15;
}

/*
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

void CPerimeterWindow::SetBlock(psdl::block* pBlock)
{
	m_list.DeleteAllItems();

	if (!pBlock) return;

	m_pBlock = pBlock;

	m_list.SetRedraw(FALSE);
	for (unsigned long i = 0; i < pBlock->num_perimeters(); i++)
	{
		CString strVertex, strBlock("-");

		psdl::perimeter_pt* pp = pBlock->get_perimeter_point(i);

		strVertex.Format("%x, ", pp->vertex);

		if (pp->block > 0)
			strBlock.Format("%x", pp->block);

		m_list.InsertItem(i, strVertex + strBlock);
	}
	m_list.SetRedraw();
}*/

void CPerimeterWindow::SetBlock(psdl::block* pBlock)
{
	m_list.DeleteAllItems();

	if (!pBlock) return;

	m_pBlock = pBlock;

	m_list.SetRedraw(FALSE);

	for (unsigned long i = 0; i < pBlock->num_perimeters(); i++)
		m_list.InsertItem(i, _T(""));

	m_list.SetRedraw();
	AdjustColumns();
}

void CPerimeterWindow::OnDrawItem(UINT, LPDRAWITEMSTRUCT lpdis)
{
	CDCHandle dc(lpdis->hDC);

	dc.SetTextColor(GetSysColor(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
	dc.SetBkColor  (GetSysColor(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT     : COLOR_WINDOW));

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int y1 = (lpdis->rcItem.bottom + lpdis->rcItem.top - tm.tmHeight) / 2;

	CString str;
	RECT rc;

	// Subitem 0
	m_list.GetSubItemRect(lpdis->itemID, 0, LVIR_BOUNDS, &rc);

	str.Format("%0*x", m_nDigits, lpdis->itemID);
	dc.ExtTextOut(rc.left + 3, y1, ETO_CLIPPED | ETO_OPAQUE, &rc, str, str.GetLength(), NULL);

	// Subitem 1
	m_list.GetSubItemRect(lpdis->itemID, 1, LVIR_BOUNDS, &rc);

	CString strBlock("-");

	psdl::perimeter_pt* pp = m_pBlock->get_perimeter_point(lpdis->itemID);

	str.Format("%x, ", pp->vertex);

	if (pp->block > 0)
		strBlock.Format("%x", pp->block);

	str += strBlock;

	dc.ExtTextOut(rc.left + 4, y1, ETO_CLIPPED | ETO_OPAQUE, &rc, str, str.GetLength());
}

void CPerimeterWindow::OnMeasureItem(UINT, LPMEASUREITEMSTRUCT lpmis)
{
	lpmis->itemHeight = 15;
}

void CBlocksWindow::OnContextMenu(CWindow wnd, CPoint pt)
{
	CMenu menu;
	menu.LoadMenu(IDR_POPUP_BLOCK);
	menu.GetSubMenu(0).TrackPopupMenu(TPM_RIGHTBUTTON, pt.x, pt.y, CMainFrame::GetWnd());
}

LRESULT CBlocksWindow::OnItemChanged(int, LPNMHDR lpnmh, BOOL&)
{
	NMLISTVIEW* nm = (NMLISTVIEW*) lpnmh;
	if (nm->uNewState & LVIS_FOCUSED)
		CMainFrame::SelectBlock(nm->iItem);
	return 0;
}

LRESULT CBlocksWindow::OnClick(int, LPNMHDR lpnmh, BOOL&)
{
	NMITEMACTIVATE* nmia = (NMITEMACTIVATE*) lpnmh;

	if (nmia->iItem >= 0)
	{
		if (nmia->iSubItem == 1)
		{
			int iSubSubItem = (nmia->ptAction.x - m_list.GetColumnWidth(0)) / 15;

			psdl::block* pBlock = m_pDoc->get_block(nmia->iItem);

			if (iSubSubItem == 0)// Show/hide
			{
				pBlock->enabled = !pBlock->enabled;
				CMainFrame::GetView()->Invalidate();
			}
			else if (iSubSubItem == 1)// Lock/unlock
			{
				pBlock->locked = !pBlock->locked;
			}

			m_list.RedrawItems(nmia->iItem, nmia->iItem);
		}
		else if (nmia->iSubItem == 2)
		{
			int iSubSubItem = (nmia->ptAction.x - m_list.GetColumnWidth(1) - m_list.GetColumnWidth(0)) / 11;

			if (iSubSubItem < 7)
			{
				psdl::block* pBlock = m_pDoc->get_block(nmia->iItem);
				pBlock->type ^= 1 << (7 - iSubSubItem);
				m_list.RedrawItems(nmia->iItem, nmia->iItem);
			}
		}
	}

	return FALSE;
}

void CBlocksWindow::OnDrawItem(UINT, LPDRAWITEMSTRUCT lpdis)
{
	CDCHandle dc(lpdis->hDC);

	dc.SetTextColor(GetSysColor(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));
	dc.SetBkColor  (GetSysColor(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHT     : COLOR_BTNFACE));

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int y1 = (lpdis->rcItem.bottom + lpdis->rcItem.top - tm.tmHeight) / 2;

	CString str;

//	lpdis->rcItem.right += 2;

	// Subitem 0
	str.Format("%0*x", m_nDigits, lpdis->itemID);
	dc.ExtTextOut(lpdis->rcItem.left + 4, y1, ETO_CLIPPED | ETO_OPAQUE, &lpdis->rcItem, str, str.GetLength(), NULL);

	RECT rc;
	psdl::block* pBlock = m_pDoc->get_block(lpdis->itemID);

	const CBitmap bmp = CreateBitmap(4, 4, 1, 1, wBullet);
	CDC dcMem;
	dcMem.CreateCompatibleDC(dc);
	dcMem.SelectBitmap(bmp);

	// Subitem 1
	m_list.GetSubItemRect(lpdis->itemID, 1, LVIR_BOUNDS, &rc);

	int y = (rc.top + rc.bottom - 4) / 2;

	if (pBlock->enabled)
		dc.BitBlt(rc.left + 5, y, 4, 4, dcMem, 0, 0, SRCCOPY);
	else
		m_ImageList.Draw(dc, 3, rc.left, y - 4, ILD_NORMAL);

	if (pBlock->locked)
		m_ImageList.Draw(dc, 1, rc.left + 15, y - 4, ILD_NORMAL);
	else
		dc.BitBlt(rc.left + 20, y, 4, 4, dcMem, 0, 0, SRCCOPY);

	// Subitem 2
	m_list.GetSubItemRect(lpdis->itemID, 2, LVIR_BOUNDS, &rc);

	unsigned char i;
	for (i = 1; i < 8; i++)// Not showing the first bit
	{
		if (pBlock->type >> i & 1)
			dc.BitBlt(rc.left + 4 + (7 - i) * 11, y, 4, 4, dcMem, 0, 0, SRCCOPY);
	}

	dcMem.DeleteDC();

	CPen pen;
	HPEN hOldPen = dc.SelectPen(pen.CreatePen(PS_SOLID, 0, RGB_AVERAGE(GetSysColor(COLOR_BTNHILIGHT), GetSysColor(COLOR_BTNFACE))));

	for (i = 0; i < 8; i++)
	{
		dc.MoveTo(rc.left + i * 11, rc.top, NULL);
		dc.LineTo(rc.left + i * 11, rc.bottom);
	}

	pen.DeleteObject();

	dc.MoveTo(lpdis->rcItem.left, lpdis->rcItem.bottom, NULL);

	dc.SelectPen(pen.CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNHILIGHT)));
	dc.LineTo(lpdis->rcItem.left, lpdis->rcItem.top);
	dc.LineTo(lpdis->rcItem.right - 1, lpdis->rcItem.top);
	pen.DeleteObject();

	dc.SelectPen(pen.CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW)));
	dc.LineTo(lpdis->rcItem.right - 1, lpdis->rcItem.bottom - 1);
	dc.LineTo(lpdis->rcItem.left, lpdis->rcItem.bottom - 1);

	dc.SelectPen(hOldPen);

//	if (lpdis->itemState & ODS_FOCUS)
//		dc.DrawFocusRect(&lpdis->rcItem);

	// Subitem 3
	m_list.GetSubItemRect(lpdis->itemID, 3, LVIR_BOUNDS, &rc);
	rc.top    += 2;
	rc.right  -= 2;
	rc.bottom -= 2;

	str.Format("%x", pBlock->proprule);

	dc.FillRect(&rc, GetSysColorBrush(COLOR_WINDOW));
	dc.DrawEdge(&rc, BDR_SUNKENINNER, BF_RECT);
	dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
	dc.TextOut(rc.left + 4, y1, str, str.GetLength());
}

void CBlocksWindow::OnMeasureItem(UINT, LPMEASUREITEMSTRUCT lpmis)
{
	lpmis->itemHeight = 17;
}
