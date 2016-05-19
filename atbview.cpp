#include "stdafx.h"
#include "atbview.h"
#include "psdl-editor.h"
#include "toolwnd.h"
#include "strings.h"
#include "mainfrm.h"

LRESULT CRoadView::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	CRect rc;
	GetClientRect(&rc);

	m_list.Create(m_hWnd, rc, IDC_LIST);
	m_list.SetTabStops(20);

	CString sText;
	psdl::road_strip *atb = static_cast<psdl::road_strip*>(m_atb);
	for (size_t i = 0; i < atb->num_vertices(); i++)
	{
	//	sText.Format("%x\t", i);
		sText.Format("%x\t[%04x]\t", i, atb->get_vertex_ref(i));
	//	sText += VertexString(g_psdl->getVertex(atb->getVertexRef(i)));
		m_list.AddString(sText);
	}

	DlgResize_Init(false, false, WS_CHILD | WS_VISIBLE);

	return 0;
}

LRESULT CTextureView::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	SetRedraw(FALSE);

	unsigned short nIndex = static_cast<psdl::texture*>(m_pAtb)->i_texture;

//	m_textureName.Format("0x%x: %s", nIndex, m_pDoc->get_texname(nIndex).c_str());
	m_textureName = IndexString(nIndex) + ": " + m_pDoc->get_texname(nIndex).c_str();

	DoDataExchange(FALSE);
	DlgResize_Init(false, false, WS_CHILD | WS_VISIBLE);

	CWindow wnd;
	wnd.Attach(GetDlgItem(IDC_PLACEHOLDER));

	if (nIndex < CMainFrame::m_psdlDoc.GetView()->m_textures.size())
	{
		wnd.ModifyStyle(0, SS_OWNERDRAW);
	}
	else
	{
		HFONT hFont = wnd.GetFont();
		LOGFONT lf;
		GetObject(hFont, sizeof(LOGFONT), &lf);
		lf.lfItalic = TRUE;
		wnd.SetFont(CreateFontIndirect(&lf));
	}

	SetRedraw();
	return 0;
}

void CTextureView::OnDrawItem(UINT, LPDRAWITEMSTRUCT lpdis)
{
	unsigned short nIndex = static_cast<psdl::texture*>(m_pAtb)->i_texture;

	GLint nWidth = 0, nHeight = 0;

	wglMakeCurrent(CMainFrame::GetView()->GetDC(), CMainFrame::GetView()->GetRC());

	GLuint iTexName = CMainFrame::m_psdlDoc.GetView()->m_textures[nIndex];
	glBindTexture(GL_TEXTURE_2D, iTexName);

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &nWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &nHeight);

	GLbyte *pBytes = new GLbyte[nWidth * nHeight * 3];

	glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR_EXT /* == 24 bits */, GL_UNSIGNED_BYTE, &pBytes[0]);

	wglMakeCurrent(NULL, NULL);

	BITMAPINFOHEADER bmih;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = nWidth;
	bmih.biHeight = nHeight;
	bmih.biPlanes = 1;
	bmih.biBitCount = 24;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;
//	bmih.biXPelsPerMeter = ;
//	bmih.biYPelsPerMeter = ;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;

	BITMAPINFO bmi;
	bmi.bmiHeader = bmih;

	HBITMAP hBmp = CreateDIBitmap(lpdis->hDC, &bmih, CBM_INIT, pBytes, &bmi, DIB_RGB_COLORS);
	delete[] pBytes;

	HDC hdcMem = CreateCompatibleDC(lpdis->hDC);
	SelectObject(hdcMem, hBmp);
	BitBlt(lpdis->hDC, 0, 0, nWidth, nHeight, hdcMem, 0, 0, SRCCOPY);
	DeleteObject(hBmp);
	DeleteDC(hdcMem);
}

LRESULT CTunnelView::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	SetRedraw(FALSE);

	SetState(IDC_RAILING,      !m_atb->get_flag(BIT_STYLE));
	SetState(IDC_WALL,			m_atb->get_flag(BIT_STYLE));
	SetState(IDC_LEFT,			m_atb->get_flag(BIT_LEFT));
	SetState(IDC_RIGHT,			m_atb->get_flag(BIT_RIGHT));
	SetState(IDC_CLOSEDSTART_L,	m_atb->get_flag(BIT_CLOSEDSTART_L));
	SetState(IDC_CLOSEDEND_L,	m_atb->get_flag(BIT_CLOSEDEND_L));
	SetState(IDC_CLOSEDSTART_R,	m_atb->get_flag(BIT_CLOSEDSTART_R));
	SetState(IDC_CLOSEDEND_R,	m_atb->get_flag(BIT_CLOSEDEND_R));
	SetState(IDC_OFFSETSTART_L,	m_atb->get_flag(BIT_OFFSETSTART_L));
	SetState(IDC_OFFSETEND_L,	m_atb->get_flag(BIT_OFFSETEND_L));
	SetState(IDC_OFFSETSTART_R,	m_atb->get_flag(BIT_OFFSETSTART_R));
	SetState(IDC_OFFSETEND_R,	m_atb->get_flag(BIT_OFFSETEND_R));
	SetState(IDC_CURVEDWALL,	m_atb->get_flag(BIT_CURVEDSIDES));
	SetState(IDC_FLAT,			m_atb->get_flag(BIT_CULLED));
	SetState(IDC_CURVED,		m_atb->get_flag(BIT_CURVEDCEILING));

	m_ud1.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS, 0, IDC_SPIN1);
	m_ud1.SetBuddy(GetDlgItem(IDC_HEIGHT1));
	m_ud1.SetRange(SHRT_MIN, SHRT_MAX);
	m_ud1.SetIncrement(1);
	m_ud1.SetValueForBuddy(m_atb->height1);

	m_ud2.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS, 0, IDC_SPIN2);
	m_ud2.SetBuddy(GetDlgItem(IDC_HEIGHT2));
	m_ud2.SetRange(SHRT_MIN, SHRT_MAX);
	m_ud2.SetIncrement(1);
	m_ud2.SetValueForBuddy(m_atb->height2);

	CWindow wnd(m_hWnd);
	RECT rc;

	::GetWindowRect(GetDlgItem(IDC_PLACEHOLDER), &rc);
	wnd.ScreenToClient(&rc);

	m_list.Create(m_hWnd, rc, NULL, WS_CHILD |
		LVS_OWNERDRAWFIXED | LVS_REPORT | LVS_NOSORTHEADER,
		WS_EX_STATICEDGE, IDC_LIST);

	if (m_atb->subtype)
	{
		::ShowWindow(GetDlgItem(IDC_GROUP3), SW_HIDE);
		::ShowWindow(GetDlgItem(IDC_PLACEHOLDER), SW_HIDE);
	}
	else
	{
		m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

		m_list.InsertColumn(0, _T("#"));
		m_list.InsertColumn(1, _T("-"));

		m_list.SetColumnWidth(0, 60);
		m_list.SetColumnWidth(1, 60);

		for (unsigned short i = 0; i < static_cast<psdl::junction*>(m_atb)->num_walls(); ++i)
		{
			LVITEM lvi;
			lvi.iItem = i;
			lvi.iSubItem = 1;
			lvi.mask = LVIF_TEXT;
			lvi.pszText = static_cast<psdl::junction*>(m_atb)->get_wall(i) ? _T("+") : _T("-");

			CString str;
			str.Format("%02x", i);

			m_list.InsertItem(i, str);
			m_list.SetItem(&lvi);
		}

		m_list.ShowWindow(SW_SHOW);
	}

	SetRedraw();

	DlgResize_Init(false, true, WS_CHILD | WS_VISIBLE);
	return 0;
}

LRESULT CTunnelView::OnCheckClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL&)
{
	unsigned char nBit;

	switch (wID)
	{
		case IDC_LEFT:			nBit = BIT_LEFT;			break;
		case IDC_RIGHT:			nBit = BIT_RIGHT;			break;
		case IDC_CLOSEDSTART_L:	nBit = BIT_CLOSEDSTART_L;	break;
		case IDC_CLOSEDEND_L:	nBit = BIT_CLOSEDEND_L;		break;
		case IDC_CLOSEDSTART_R:	nBit = BIT_CLOSEDSTART_R;	break;
		case IDC_CLOSEDEND_R:	nBit = BIT_CLOSEDEND_R;		break;
		case IDC_OFFSETSTART_L:	nBit = BIT_OFFSETSTART_L;	break;
		case IDC_OFFSETEND_L:	nBit = BIT_OFFSETEND_L;		break;
		case IDC_OFFSETSTART_R:	nBit = BIT_OFFSETSTART_R;	break;
		case IDC_OFFSETEND_R:	nBit = BIT_OFFSETEND_R;		break;
		case IDC_CURVEDWALL:	nBit = BIT_CURVEDSIDES;		break;
		case IDC_FLAT:			nBit = BIT_CULLED;			break;
		case IDC_CURVED:		nBit = BIT_CURVEDCEILING;	break;

		default: return 0;
	}

	m_atb->set_flag(nBit, BST_CHECKED == ::SendMessage(GetDlgItem(wID), BM_GETCHECK, 0, 0));

	CMainFrame::GetView()->Invalidate();

	return 0;
}

LRESULT CTunnelView::OnClick(int, LPNMHDR lpnmh, BOOL&)
{
	NMITEMACTIVATE* nmia = (NMITEMACTIVATE*) lpnmh;

	if (nmia->iItem >= 0)
	{
		if (nmia->iSubItem == 1)
		{
			static_cast<psdl::junction*>(m_atb)->set_wall(nmia->iItem, !static_cast<psdl::junction*>(m_atb)->get_wall(nmia->iItem));

			m_list.RedrawItems(nmia->iItem, nmia->iItem);
			CMainFrame::GetView()->Invalidate();
		}
	}

	return 0;
}

LRESULT CTunnelView::OnEditChange(UINT, int idCtrl, HWND)
{
//	MessageBox("lol");
//	DoDataExchange(TRUE);

	if (idCtrl == IDC_SPIN1)
		m_atb->height1 = m_ud1.GetPos();
	else
		m_atb->height2 = m_ud2.GetPos();

	CMainFrame::GetView()->Invalidate();
	SetMsgHandled(FALSE);
	return 0;
}

LRESULT CTunnelView::OnDeltaPos(int idCtrl, LPNMHDR lpnmh, BOOL& bHandled)
{
	if (idCtrl == IDC_SPIN1)
		m_atb->height1 = m_ud1.GetPos();
	else
		m_atb->height2 = m_ud2.GetPos();

//	bHandled = FALSE;
	return 0;
}

void CTunnelView::DrawItem(LPDRAWITEMSTRUCT lpdis)
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

	str.Format("%0*x", 2, lpdis->itemID);
	dc.ExtTextOut(rc.left + 3, y1, ETO_CLIPPED | ETO_OPAQUE, &rc, str, str.GetLength(), NULL);

	// Subitem 1
	m_list.GetSubItemRect(lpdis->itemID, 1, LVIR_BOUNDS, &rc);

	rc.top++;
	rc.bottom--;

	dc.DrawFrameControl(&rc, DFC_BUTTON, DFCS_BUTTONCHECK | DFCS_FLAT |
		
		(static_cast<psdl::junction*>(m_atb)->get_wall(lpdis->itemID) ? DFCS_CHECKED : 0)

	);

/*	CString strBlock("-");

	psdl::perimeter_pt* pp = m_pBlock->get_perimeter_point(lpdis->itemID);

	str.Format("%x, ", pp->vertex);

	if (pp->block > 0)
		strBlock.Format("%x", pp->block);

	str += strBlock;

	dc.ExtTextOut(rc.left + 4, y1, ETO_CLIPPED | ETO_OPAQUE, &rc, str, str.GetLength());*/
}

void CTunnelView::MeasureItem(LPMEASUREITEMSTRUCT lpmis)
{
	lpmis->itemHeight = 15;
}
