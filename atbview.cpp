#include "stdafx.h"
#include "atbview.h"
#include "psdl-editor.h"
#include "toolwnd.h"
#include "strings.h"

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
