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

	return FALSE;
}

LRESULT CTextureView::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
{
	unsigned short nIndex = static_cast<psdl::texture*>(m_pAtb)->i_texture;

	m_textureName.Format("0x%x: %s", nIndex, m_pDoc->get_texname(nIndex).c_str());

	DoDataExchange(FALSE);
	DlgResize_Init(false, false, WS_CHILD | WS_VISIBLE);

	return FALSE;
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
