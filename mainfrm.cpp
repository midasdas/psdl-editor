#include "stdafx.h"

#include <algorithm>
#include <map>
#include <shellapi.h>
#include <process.h>

#include "mainfrm.h"
#include "aboutdlg.h"
#include "optionsdlg.h"

#include "files.h"

using namespace std;

PSDLDocTemplate CMainFrame::m_psdlDoc;
CPVSDocTemplate CMainFrame::m_cpvsDoc;
BAIDocTemplate  CMainFrame::m_baiDoc;

CMainFrame* CMainFrame::this_ptr = NULL;

const extMap CMainFrame::m_sExtMap[] =
{
	{ "psdl",    &m_psdlDoc, ID_MODE_PSDL    },
	{ "psd",     &m_psdlDoc, ID_MODE_PSDL    },
	{ "cpvs",    &m_cpvsDoc, ID_MODE_CPVS    },
	{ "inst",    NULL,       ID_MODE_INST    },
	{ "bai",     &m_baiDoc,  ID_MODE_BAI     },
	{ "pathset", NULL,       ID_MODE_PATHSET }
};

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// Process tab key
	if (m_wndProps.IsDialogMessage(pMsg))
		return 1;
	if (m_dlgTransform && m_dlgTransform.IsDialogMessage(pMsg))
		return 1;

	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return 1;

	return 0;
}

BOOL CMainFrame::OnIdle()
{
	UIEnable(ID_FILE_SAVE, CanSave());

	UISetText(ID_EDIT_UNDO, GetUndoDescription());
	UISetText(ID_EDIT_REDO, GetRedoDescription());
	UIEnable(ID_EDIT_UNDO, CanUndo());
	UIEnable(ID_EDIT_REDO, CanRedo());
	UIEnable(ID_EDIT_TRANSFORM, !m_dlgTransform);

//	UIEnable(ID_MODE_CPVS, FALSE);
	UIEnable(ID_MODE_INST, FALSE);
//	UIEnable(ID_MODE_BAI, FALSE);
	UIEnable(ID_MODE_PATHSET, FALSE);

	UIEnable(ID_FILE_OPENCONTAININGFOLDER, GetActiveDocument()->FileExists());

	UISetCheck(ID_WINDOWS_CITYBLOCKS, ::IsWindowVisible(m_wndBlocks));
	UISetCheck(ID_WINDOWS_PERIMETER,  ::IsWindowVisible(m_wndPerimeter));
	UISetCheck(ID_WINDOWS_ATTRIBUTES, ::IsWindowVisible(m_wndAttribs));
	UISetCheck(ID_WINDOWS_PROPERTIES, ::IsWindowVisible(m_wndProps));

	UIUpdateToolBar();
	UIUpdateMenuBar(FALSE, TRUE);

	return 0;
}

LRESULT CMainFrame::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	CString strDocName;

	for (int i = GetMaxEntries() - 1; i >= 0; i--)
	{
		if (GetFromList(ID_FILE_MRU_FIRST + i, strDocName))
			config.files.aRecentFiles.push((LPCTSTR) strDocName);
	}

	bHandled = 0;
	return 0;
}

LRESULT CMainFrame::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR), 1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,// Color depth
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		24,// Size of depth buffer
		0, 0,
		PFD_MAIN_PLANE,
		0, 0, 0, 0
	};

	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);
	UISetCheck(ID_WINDOWS_CITYBLOCKS, 1);
	UISetCheck(ID_WINDOWS_PERIMETER, 1);
	UISetCheck(ID_WINDOWS_ATTRIBUTES, 1);
	UISetCheck(ID_WINDOWS_PROPERTIES, 1);
	UISetCheck(ID_VIEW_AUTO_CAMERA,     config.display.bAutoCenter);
	UISetCheck(ID_VIEW_WIREFRAME,       config.display.bWireframe);
	UISetCheck(ID_VIEW_TEST_PVS,        config.display.bTestPVS);
	UISetCheck(ID_VIEW_TEST_AI_CULLING, config.display.bTestAICulling);
	UISetCheck(ID_VIEW_RENDER_PSDL,     config.display.bRenderPSDL);
	UISetCheck(ID_VIEW_RENDER_BAI,      config.display.bRenderBAI);

	SetNumeral(config.display.eNumeral == ::Numeral::hex ? ID_VIEW_NUMERAL_HEX : ID_VIEW_NUMERAL_DEC);

	CreateSimpleToolBar();
	CToolBarCtrl tool = m_hWndToolBar;
	tool.SetStyle(tool.GetStyle() | TBSTYLE_FLAT);

	CreateSimpleStatusBar();

	UIAddMenuBar(m_hWnd);
	UIAddToolBar(m_hWndToolBar);

	SetMenuHandle(GetSubMenu(GetMenu(), 0));
	SetMaxEntries(16);
	SetMaxItemLength(20);

	while (!config.files.aRecentFiles.empty())
	{
		AddToList(config.files.aRecentFiles.top().c_str());
		config.files.aRecentFiles.pop();
	}

	m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE);
	m_view.SetDCPixelFormat(&pfd);
	m_view.SetFocus();

	// Initiate docking framework
	HWND hwndDock = m_dock.Create(m_hWnd, rcDefault);
	m_dock.SetClient(m_hWndClient);

	m_wndProps.Create    (m_hWnd, rcDefault, _T("Attribute Properties"), ATL_SIMPLE_DOCKVIEW_STYLE);
	m_wndAttribs.Create  (m_hWnd, rcDefault, _T("Block Attributes"),     ATL_SIMPLE_DOCKVIEW_STYLE);
	m_wndPerimeter.Create(m_hWnd, rcDefault, _T("Perimeter"),            ATL_SIMPLE_DOCKVIEW_STYLE);
	m_wndBlocks.Create   (m_hWnd, rcDefault, _T("City Blocks"),          ATL_SIMPLE_DOCKVIEW_STYLE);

	m_dock.AddWindow(m_wndProps);
	m_dock.AddWindow(m_wndAttribs);
	m_dock.AddWindow(m_wndPerimeter);
	m_dock.AddWindow(m_wndBlocks);
	m_dock.DockWindow(m_wndProps,	  DOCK_RIGHT);
	m_dock.DockWindow(m_wndAttribs,	  DOCK_RIGHT);
	m_dock.DockWindow(m_wndPerimeter, DOCK_LEFT);
	m_dock.DockWindow(m_wndBlocks,	  DOCK_LEFT);

	m_dock.SetPaneSize(DOCK_LEFT,  210);
	m_dock.SetPaneSize(DOCK_RIGHT, 210);

	m_hWndClient = hwndDock;

	m_psdlDoc.SetViews(&m_wndBlocks, &m_wndPerimeter, &m_wndAttribs, &m_wndProps);
	m_cpvsDoc.SetViews(&m_wndBlocks);
	m_psdlDoc.NewDocument();
	m_psdlDoc.UpdateViews();
	m_cpvsDoc.NewDocument();
	m_baiDoc.NewDocument();
	m_cpvsDoc.SetPSDL(m_psdlDoc.GetDocument());
	m_psdlDoc.GetView()->SetCPVS(m_cpvsDoc.GetDocument());

	SetEditingMode(ID_MODE_PSDL);
	UpdateCaption();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	return 0;
}

void CMainFrame::PaintDescendants(void)
{
	if (config.display.bRenderPSDL) this_ptr->m_psdlDoc.RenderScene();
	if (config.display.bRenderBAI)  this_ptr->m_baiDoc.RenderScene();
}

LRESULT CMainFrame::OnPaintDescendants(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
	if (config.display.bRenderPSDL) m_psdlDoc.RenderScene();
	if (config.display.bRenderBAI)  m_baiDoc.RenderScene();
//	m_cpvsDoc.RenderScene();
	return 0;
}

LRESULT CMainFrame::OnFileExit(WORD, WORD, HWND, BOOL&)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnViewResetCamera(WORD, WORD, HWND, BOOL&)
{
	m_view.ResetCamera();
	m_view.Invalidate();
	return 0;
}

LRESULT CMainFrame::OnViewSettingChange(WORD, WORD wID, HWND, BOOL&)
{
	bool b = false;

	switch (wID)
	{
		case ID_VIEW_AUTO_CAMERA:     b = config.display.bAutoCenter    ^= 1; break;
		case ID_VIEW_WIREFRAME:       b = config.display.bWireframe     ^= 1; break;
		case ID_VIEW_TEST_PVS:        b = config.display.bTestPVS       ^= 1; break;
		case ID_VIEW_TEST_AI_CULLING: b = config.display.bTestAICulling ^= 1; break;
		case ID_VIEW_RENDER_PSDL:     b = config.display.bRenderPSDL    ^= 1; break;
		case ID_VIEW_RENDER_BAI:      b = config.display.bRenderBAI     ^= 1; break;
	}

	UISetCheck(wID, b);
	m_view.Invalidate();
	return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD, WORD, HWND, BOOL&)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndToolBar);
	::ShowWindow(m_hWndToolBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD, WORD, HWND, BOOL&)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewBar(WORD, WORD wID, HWND, BOOL&)
{
	HWND hWnd;

	switch (wID)
	{
		case ID_WINDOWS_CITYBLOCKS: hWnd = m_wndBlocks;    break;
		case ID_WINDOWS_PERIMETER:  hWnd = m_wndPerimeter; break;
		case ID_WINDOWS_ATTRIBUTES: hWnd = m_wndAttribs;   break;
		case ID_WINDOWS_PROPERTIES: hWnd = m_wndProps;     break;

		default:
			return 0;
	}

	BOOL bVisible = ::IsWindowVisible(hWnd);

	if (bVisible)
		m_dock.HideWindow(hWnd);
	else
		m_dock.DockWindow(hWnd, DOCK_LASTKNOWN);

	UISetCheck(wID, !bVisible);
	return 0;
}

void CMainFrame::SetNumeral(WORD wID)
{
	WORD wIDOther = wID == ID_VIEW_NUMERAL_HEX ? ID_VIEW_NUMERAL_DEC : ID_VIEW_NUMERAL_HEX;
	UISetCheck(wID, true);
	UISetCheck(wIDOther, false);
}

LRESULT CMainFrame::OnViewSetNumeral(WORD, WORD wID, HWND, BOOL&)
{
	config.display.eNumeral = wID == ID_VIEW_NUMERAL_HEX ? ::Numeral::hex : ::Numeral::dec;
	SetNumeral(wID);
	m_wndBlocks.m_list.Invalidate();
	m_wndAttribs.m_list.Invalidate();
	m_wndPerimeter.m_list.Invalidate();
	return 0;
}

LRESULT CMainFrame::OnViewHideAll(WORD, WORD wID, HWND, BOOL&)
{
	psdl* pPSDL = m_psdlDoc.GetDocument();
	for (unsigned long i = 0; i < pPSDL->num_blocks(); ++i)
	{
		pPSDL->get_block(i)->enabled = false;
	}
	m_view.Invalidate();
	m_wndBlocks.m_list.Invalidate();
	return 0;
}
void CMainFrame::SelectBlock(long iIndex)
{
	this_ptr->m_psdlDoc.SelectBlock(iIndex);
	this_ptr->m_cpvsDoc.SelectBlock(iIndex);

	if (config.display.bAutoCenter)
	{
		Vertex v = -this_ptr->m_psdlDoc.GetBlock(iIndex)->get_center();
		this_ptr->m_view.MoveAxis(v.x, v.y, v.z);
	}
	this_ptr->m_view.Invalidate();
}

void CMainFrame::SelectAttribute(psdl::block* pBlock, long iIndex)
{
	this_ptr->m_wndProps.SetAttribute(pBlock->get_attribute(iIndex));
}

void CMainFrame::UpdateCaption(void)
{
	DocTemplateBase* pDoc = GetActiveDocument();

	CString strTitle = _T("");

	if (pDoc)
		strTitle = (CString) " - [" + pDoc->GetFileName().c_str()
		           + (pDoc->IsModified() ? " *" : "") + "]";

	SetWindowText(LS(IDR_MAINFRAME) + strTitle);
}

void CMainFrame::SetEditingMode(int iMode)
{
	UISetCheck(m_iEditMode, 0);
	UISetCheck(iMode, 1);

	m_iEditMode = iMode;
}

LRESULT CMainFrame::OnEditTransform(WORD, WORD wID, HWND, BOOL&)
{
	m_dlgTransform.Create(m_view);
	m_dlgTransform.ShowWindow(SW_SHOWNORMAL);
	return 0;
}

LRESULT CMainFrame::OnReverseVertices(WORD, WORD wID, HWND, BOOL&)
{
	int nItem = -1;
	if ((nItem = m_wndAttribs.m_list.GetNextItem(nItem, LVNI_SELECTED)) != -1)
	{
		m_wndAttribs.GetBlock()->get_attribute(nItem)->reverse();
	}
	return 0;
}

bool CMainFrame::TransformEntities(TransformProps& sProps)
{
	vector<unsigned long> blockIDs;
	vector<psdl::block*> blocks = m_wndBlocks.GetSelected(&blockIDs);

	psdl* pPSDL = m_psdlDoc.GetDocument();
	psdl::block* pBlock;
	psdl::attribute* pAtb;

	unsigned long nVertices = pPSDL->num_vertices();

	unsigned long i, j, k;

	vector<bool> vertices(nVertices, false);

	for (i = 0; i < blocks.size(); ++i)
	{
		pBlock = blocks[i];

		for (j = 0; j < pBlock->num_attributes(); ++j)
		{
			pAtb = pBlock->get_attribute(j);

			switch (pAtb->type)
			{
				case ATB_ROAD:
				case ATB_SIDEWALK:
				case ATB_RECTANGLE:
				case ATB_ROADTRIANGLEFAN:
				case ATB_TRIANGLEFAN:
				case ATB_DIVIDEDROAD:
				case ATB_ROOFTRIANGLEFAN:
				case ATB_CROSSWALK:
				case ATB_SLIVER:
				case ATB_FACADEBOUND:
					
					for (k = 0; k < static_cast<psdl::vertex_attribute*>(pAtb)->num_vertices(); ++k)
					{
						vertices[static_cast<psdl::vertex_attribute*>(pAtb)->get_vertex_ref(k)] = true;
					}
					break;
			}
		}
	}

	Vertex* pVertex;
	Vertex vOrigin(0, 0, 0);
	Vertex vTranslate(sProps.fX, sProps.fY, sProps.fZ);
	double fRotate = DEG2RAD(sProps.fAngle);

	for (i = 0; i < nVertices; ++i)
	{
		if (vertices[i] == true)
		{
			pVertex = pPSDL->get_vertex(i);
			m_psdlDoc.MoveVertex(pVertex, vTranslate);
			m_psdlDoc.RotateVertex(pVertex, vOrigin, fRotate);
		}
	}

	m_view.Invalidate();
	return false;
}

LRESULT CMainFrame::OnSetEditingMode(WORD, WORD wID, HWND, BOOL&)
{
	SetEditingMode(wID);
	UpdateCaption();
	return 0;
}

LRESULT CMainFrame::OnInsertBlock(WORD, WORD wID, HWND, BOOL&)
{
	int iPos = m_wndBlocks.m_list.GetNextItem(-1, LVNI_FOCUSED);
	if (iPos > 0) iPos++;

	m_psdlDoc.InsertBlock(new psdl::block(0), iPos);
	m_psdlDoc.m_bModified = true;

	AddHistoryState(wID);
	UpdateCaption();
	return 0;
}

LRESULT CMainFrame::OnDuplicateBlock(WORD, WORD wID, HWND, BOOL&)
{
/*	CDuplicateDlg dlg;

	if (IDOK == dlg.DoModal())
	{
		unsigned long nBlocks = m_wndBlocks.m_list.GetSelCount();
		int* aBlockIds = new int[nBlocks];
		m_wndBlocks.m_list.GetSelItems(nBlocks, aBlockIds);

		// Experiment, should be removed
	//	Vertex vOffset(1220.0787f, -20.2605f, -870.612f);

		for (unsigned char i = 0; i < g_duplicateProps.nCount; i++)
		{
			unsigned long nOffset = m_psdlDoc.NumBlocks(); // Number of PSDL blocks at this point
			vector<vertexMap> aDuplicates;

			for (unsigned long iBlock = 0; iBlock < nBlocks; iBlock++)
			{
				unsigned long iIndex = aBlockIds[iBlock];

				psdl::block* block = new psdl::block(*m_psdlDoc.GetBlock(iIndex));

				// Duplicate the vertices in the PSDL used by the block's perimeter
				if (g_duplicateProps.bPerimeters)
				{
					for (unsigned long iPt = 0; iPt < block->num_perimeters(); iPt++)
					{
						unsigned long iNb = 0; // 0 = no neighbour

						if (g_duplicateProps.bNeighbours)
						{
							int* beg = aBlockIds;
							int* end = aBlockIds + nBlocks;

							int* iFind = std::find(beg, end, (long) block->_perimeter[iPt].block - 1);
							if (iFind != end)
							{
								iNb = iFind - beg + nOffset + 1;
								ATLTRACE("\nNeigbour %x adjusted (%x)", iPt, iNb);
							}
						}

						block->_perimeter[iPt].vertex = m_psdlDoc.CopyVertex(block->_perimeter[iPt].vertex, aDuplicates);
						block->_perimeter[iPt].block  = iNb;
					}
				}

				// Duplicate the vertices in the PSDL used by the block's attributes
				if (g_duplicateProps.bVertices)
				{
					int i = 0;
					unsigned long j;

					for (unsigned long iAtb = 0; iAtb < block->num_attributes(); iAtb++)
					{
						psdl::attribute* atb = block->get_attribute(iAtb);
						i++;

						unsigned char type = atb->type;
						switch (type)
						{
							case ATB_ROAD:
							case ATB_SIDEWALK:
							case ATB_RECTANGLE:
							case ATB_ROADTRIANGLEFAN:
							case ATB_TRIANGLEFAN:
							case ATB_DIVIDEDROAD:
							case ATB_ROOFTRIANGLEFAN:

								for (j = 0; j < static_cast<psdl::road_strip*>(atb)->num_vertices(); j++)
								{
									unsigned short iVertex = static_cast<psdl::road_strip*>(atb)->get_vertex_ref(j);

									static_cast<psdl::road_strip*>(atb)->set_vertex_ref(
										j, m_psdlDoc.CopyVertex(iVertex, aDuplicates)
									);
								}
								break;

							case ATB_CROSSWALK:

								for (j = 0; j < 4; j++)
								{
									unsigned short nVertexRef = static_cast<psdl::crosswalk*>(atb)->get_vertex_ref(j);

									static_cast<psdl::crosswalk*>(atb)->set_vertex_ref(
										j, m_psdlDoc.CopyVertex(nVertexRef, aDuplicates)
									);
								}
								break;

							case ATB_FACADE:

								static_cast<psdl::facade*>(atb)->bottom = m_psdlDoc.CopyHeight(static_cast<psdl::facade*>(atb)->bottom);

							case ATB_SLIVER:
							case ATB_FACADEBOUND:

								static_cast<psdl::facade*>(atb)->left   = m_psdlDoc.CopyVertex(static_cast<psdl::facade*>(atb)->left, aDuplicates);
								static_cast<psdl::facade*>(atb)->right  = m_psdlDoc.CopyVertex(static_cast<psdl::facade*>(atb)->right, aDuplicates);
								static_cast<psdl::facade*>(atb)->top    = m_psdlDoc.CopyHeight(static_cast<psdl::facade*>(atb)->bottom);

								break;

							default:
								i--;
						}
					}

					ATLTRACE("Block %x copied; vertices copied for %d attributes\n", iIndex, i);
				}

				m_psdlDoc.AddBlock(block);
			}
		}

		m_psdlDoc.m_bModified = true;
		AddHistoryState(wID);
		UpdateCaption();
	}
*/
	return 0;
}
/*
LRESULT CMainFrame::OnGeneratePerimeters(WORD, WORD, HWND, BOOL&)
{
	int iItem = -1;
	int nGenerated = 0;

	while ((iItem = m_wndAttribs.m_list.GetNextItem(iItem, LVNI_SELECTED)) >= 0)
	{
		psdl::block* pBlock = m_wndAttribs.GetBlock();
		psdl::vertex_based* pAtb = static_cast<psdl::vertex_based*>(pBlock->get_attribute(iItem));

		int n = 0;

		switch (pAtb->type)
		{
			case ATB_RECTANGLE:   n = 1; break;
			case ATB_ROAD:        n = 2; break;
			case ATB_DIVIDEDROAD: n = 3; break;
		}

		if (n > 0)
		{
			n *= 2;

			unsigned short i = 0, nVertices = pAtb->num_vertices();

			for (i; i < n - 1; i++)
				pBlock->add_perimeter_point(pAtb->get_vertex_ref(i));

			for (i; i < nVertices - n; i += n)
				pBlock->add_perimeter_point(pAtb->get_vertex_ref(i));

			for (i; i > nVertices - n; i--)
				pBlock->add_perimeter_point(pAtb->get_vertex_ref(i));

			for (i; i > 0; i -= n)
				pBlock->add_perimeter_point(pAtb->get_vertex_ref(i));

			m_wndPerimeter.InsertItems(nVertices);

			nGenerated++;
		}
	}

	if (nGenerated < 1)
	{
		MessageBox(_T("No perimeters to generate"), LS(IDR_MAINFRAME), MB_ICONWARNING);
	}

	return FALSE;
}
*/
/*
LRESULT CMainFrame::OnDeleteBlock(WORD, WORD, HWND, BOOL&)
{
	vector<psdl::block*> blocks = m_wndBlocks.GetSelected();
	vector<psdl::block*>::iterator i;

	for (i = blocks.begin(); i != blocks.end(); ++i)
	{
		delete *i;
	}

	return FALSE;
}
*/
LRESULT CMainFrame::OnInsertAttribute(WORD, WORD wID, HWND, BOOL&)
{
	psdl::block* pBlock = m_wndAttribs.GetBlock();
	psdl::attribute* pAtb = NULL;
	vector<Vertex>* pVertices = &m_psdlDoc.GetDocument()->_vertices;

	switch (wID)
	{
		case ID_INSERT_ROADSTRIP:			pAtb = new psdl::road_strip(pVertices);			break;
		case ID_INSERT_SIDEWALKSTRIP:		pAtb = new psdl::sidewalk_strip(pVertices);		break;
		case ID_INSERT_RECTANGLESTRIP:		pAtb = new psdl::rectangle_strip(pVertices);	break;
		case ID_INSERT_SLIVER:				pAtb = new psdl::sliver(pVertices);				break;
		case ID_INSERT_CROSSWALK:			pAtb = new psdl::crosswalk(pVertices);			break;
		case ID_INSERT_ROADTRIANGLEFAN:		pAtb = new psdl::road_triangle_fan(pVertices);	break;
		case ID_INSERT_TRIANGLEFAN:			pAtb = new psdl::triangle_fan(pVertices);		break;
		case ID_INSERT_FACADEBOUND:			pAtb = new psdl::facade_bound(pVertices);		break;
		case ID_INSERT_DIVIDEDROADSTRIP:	pAtb = new psdl::divided_road_strip(pVertices);	break;
		case ID_INSERT_TUNNEL:				pAtb = new psdl::tunnel();						break;
		case ID_INSERT_TUNNELJUNCTION:		pAtb = new psdl::junction();					break;
		case ID_INSERT_TEXTUREREF:			pAtb = new psdl::texture();						break;
		case ID_INSERT_FACADE:				pAtb = new psdl::facade(pVertices);				break;
		case ID_INSERT_ROOFTRIANGLEFAN:		pAtb = new psdl::roof_triangle_fan(pVertices);	break;
	}

	if (pAtb)
	{
		int nItem = -1;
		while((nItem = m_wndAttribs.m_list.GetNextItem(nItem, LVNI_SELECTED)) != -1)
		{
			pBlock->_attributes.insert(pBlock->_attributes.begin() + nItem, pAtb);
			m_wndAttribs.m_list.Invalidate();
		}
	}

	return FALSE;
}
/*
psdl::road_strip* FindRoadAttribute(psdl::block* pBlock)
{
	for (unsigned long j = 0; j < pBlock->num_attributes(); ++j)
	{
		unsigned char type = pBlock->get_attribute(j)->type;

		if (type == ATB_RECTANGLE || type == ATB_ROAD || type == ATB_DIVIDEDROAD)
		{
			return static_cast<psdl::road_strip*>(pBlock->get_attribute(j));
		}
	}
	return NULL;
}

int FindVertex(psdl* pPSDL, Vertex* pSearchVertex, unsigned long nSearchOffset)
{
	for (unsigned long i = nSearchOffset; i < pPSDL->num_vertices(); ++i)
	{
		if (pSearchVertex->Match(*pPSDL->get_vertex(i)))
		{
			return i;
		}
	}
	return -1;
}
*/
LRESULT CMainFrame::OnGeneratePerimeters(WORD, WORD, HWND, BOOL&)
{
/*	CPerimetersDlg dlg;

	if (IDOK == dlg.DoModal())
	{
		vector<unsigned long> blockIDs;
		vector<psdl::block*> blocks = m_wndBlocks.GetSelected(&blockIDs);
		vector<psdl::block*>::iterator it;

	//	vector<Vertex>::iterator vertexListBegin = m_psdlDoc.GetDocument()->_vertices.begin();

		psdl* pPSDL = m_psdlDoc.GetDocument();

	//	const float fExtend = 3.f;
	//	const float fExtend = 5.f;
		float fExtend = config.dialogs.perimeters.fExtend;

		unsigned long nVertices = pPSDL->num_vertices();

		for (it = blocks.begin(); it != blocks.end(); ++it)
		{
			if (config.dialogs.perimeters.bDeleteExisting)
				(*it)->_perimeter.clear();

		//	(*it)->generate_perimeter();

			psdl::road_strip* pTAtb = FindRoadAttribute(*it);

			if (pTAtb == NULL) continue;

			unsigned char  p = pTAtb->type == ATB_RECTANGLE ? 2 : (pTAtb->type == ATB_ROAD ? 4 : 6);
			unsigned short n = pTAtb->num_vertices();

			Vertex *v1, *v2;
			Vertex outerVertex1, outerVertex2;
			unsigned long nOuterVertex1, nOuterVertex2;
			float a;

			v1 = pTAtb->get_vertex(0);
			v2 = pTAtb->get_vertex(p - 1);
			a = atan2(v2->x - v1->x, v2->z - v1->z);

			outerVertex1.x = v1->x - sin(a) * fExtend;
			outerVertex1.y = v1->y;
			outerVertex1.z = v1->z - cos(a) * fExtend;

			outerVertex2.x = v2->x + sin(a) * fExtend;
			outerVertex2.y = v2->y;
			outerVertex2.z = v2->z + cos(a) * fExtend;

			nOuterVertex1 = FindVertex(pPSDL, &outerVertex1, nVertices);
			nOuterVertex2 = FindVertex(pPSDL, &outerVertex2, nVertices);
			if (nOuterVertex1 == -1) nOuterVertex1 = pPSDL->add_vertex(outerVertex1);
			if (nOuterVertex2 == -1) nOuterVertex2 = pPSDL->add_vertex(outerVertex2);

			(*it)->add_perimeter_point(nOuterVertex1, 0);
			if (p > 2) {
				(*it)->add_perimeter_point(pTAtb->get_vertex_ref(1), 0);
				(*it)->add_perimeter_point(pTAtb->get_vertex_ref(p - 2), 0);
			}
			(*it)->add_perimeter_point(nOuterVertex2, 0);

			v1 = pTAtb->get_vertex(n - 1);
			v2 = pTAtb->get_vertex(n - p);
			a = atan2(v2->x - v1->x, v2->z - v1->z);

			outerVertex1.x = v1->x - sin(a) * fExtend;
			outerVertex1.y = v1->y;
			outerVertex1.z = v1->z - cos(a) * fExtend;

			outerVertex2.x = v2->x + sin(a) * fExtend;
			outerVertex2.y = v2->y;
			outerVertex2.z = v2->z + cos(a) * fExtend;

			nOuterVertex1 = FindVertex(pPSDL, &outerVertex1, nVertices);
			nOuterVertex2 = FindVertex(pPSDL, &outerVertex2, nVertices);
			if (nOuterVertex1 == -1) nOuterVertex1 = pPSDL->add_vertex(outerVertex1);
			if (nOuterVertex2 == -1) nOuterVertex2 = pPSDL->add_vertex(outerVertex2);

			(*it)->add_perimeter_point(nOuterVertex1, 0);
			if (p > 2) {
				(*it)->add_perimeter_point(pTAtb->get_vertex_ref(n - 2), 0);
				(*it)->add_perimeter_point(pTAtb->get_vertex_ref(n - p + 1), 0);
			}
			(*it)->add_perimeter_point(nOuterVertex2, 0);

		//	ATLTRACE("Block %x done\n", it - blocks.begin());
		}

		if (config.dialogs.perimeters.bNeighbours)
		{
			for (unsigned long l = 0; l < blocks.size(); ++l)
			{
				for (unsigned long i = 0; i < blocks[l]->num_perimeters(); ++i)
				{
					for (unsigned long j = 0; j < blocks.size(); ++j)
					{
						if (l == j) continue;

						for (unsigned long k = 0; k < blocks[j]->num_perimeters(); ++k)
						{
							if (blocks[l]->get_perimeter_point(i)->vertex == blocks[j]->get_perimeter_point(k)->vertex)
							{
								blocks[l]->get_perimeter_point(i)->block = blockIDs[j] + 1;
								break;
							}
						}
					}
				}
			}
		}

		GetView()->Invalidate();
	}
*/
	return FALSE;
}

LRESULT CMainFrame::OnOptimizePSDL(WORD, WORD, HWND, BOOL&)
{
	psdl *pPSDL = m_psdlDoc.GetDocument();

	unsigned long i, j, nVertices = pPSDL->_vertices.size();
	unsigned long* vertexRemap = new unsigned long[nVertices];
	unsigned long nRemaining = nVertices;

	for (i = 0; i < nVertices; ++i)
	{
		j = i;
		vertexRemap[i] = j;
	}

	for (i = 0; i < nVertices; ++i)
	{
		for (j = 0; j < nVertices; ++j)
		{
			if (i != j && vertexRemap[j] == j && pPSDL->get_vertex(i)->Match(*pPSDL->get_vertex(j)))
			{
				vertexRemap[j] = i;
				--nRemaining;
			}
		}
	}

	delete[] vertexRemap;

	ATLTRACE("Vertex count reduced from %u to %u\n", nVertices, nRemaining);
	return 0;

	COptimizeDlg dlg;

	if (IDOK == dlg.DoModal())
	{
/*		if (g_optimizeProps.bTextureRefs)
		{
		//	vector<vertexMap> textureMap[3];
			vector<std::string> newTexList;

			for (size_t i = 0; i < m_psdlDoc.NumBlocks(); i++)
			{
				psdl::block* pBlock = m_psdlDoc.GetBlock(i);

			//	if (pBlock->num_attributes() < 1) continue;

				vector<psdl::attribute*>::iterator it = pBlock->_attributes.begin();

				while (it != pBlock->_attributes.end())
				{
					unsigned char nAtbType = (*it)->type;

					if (nAtbType == ATB_TEXTURE)
					{
						if (it == pBlock->_attributes.end() - 1 ||
							*(it+1) && (*(it+1))->type == nAtbType)
						{
							it = pBlock->_attributes.erase(it);
						}
						else
							++it;

					// WRONG !!
					//	if (nLastTexRef == nTexRef)
					//	{
					//		if ((*it)->last)
					//		{
					//			(*(it - 1))->last = true;
					//		}

					//		it = pBlock->_attributes.erase(it);
					//		pBlock->addAttributeSize(-2);

					//		ATLTRACE("Double texture reference: 0x%x = 0x%x\n", nTexRef, nLastTexRef);
					//	}
					//	else
					//	{
					//		++it;
					//	}
					}
					else
						++it;
				}

				if (g_optimizeProps.bTextures)
				{
					unsigned char nMin = 0, nMax = 0;
					psdl::texture *pLastTex = NULL;

					it = pBlock->_attributes.begin();

					while (it != pBlock->_attributes.end())
					{
						switch ((*it)->type)
						{
							case ATB_TEXTURE:     nMin = 0;	break;
							case ATB_SIDEWALK:    nMin = 2;	break;
							case ATB_CROSSWALK:
							case ATB_ROAD:
							case ATB_DIVIDEDROAD: nMin = 3;	break;
							case ATB_TUNNEL:      nMin = 6;	break;
							default:              nMin = 1;
						}

						nMax = max(nMin, nMax);

						if (pLastTex && ((*it)->type == ATB_TEXTURE || it == pBlock->_attributes.end() - 1))
						{
							ATLTRACE("@ block %x, attribute %x: reassigning textures (%d)", i, it - pBlock->_attributes.begin(), nMax);

							vector<std::string>::iterator fnd;

							vector<std::string>::iterator first = m_psdlDoc.GetDocument()->_textures.begin() + pLastTex->i_texture;
							vector<std::string>::iterator last  = first + nMax;

							fnd = search(newTexList.begin(), newTexList.end(), first, last);

							if (fnd != newTexList.end())
							{
								pLastTex->type = ATB_TEXTURE;
								pLastTex->i_texture = fnd - newTexList.begin();
								ATLTRACE(": using existing");
							}
							else
							{
								pLastTex->type = ATB_TEXTURE;
								pLastTex->i_texture = newTexList.size();
								copy(first, last, back_inserter(newTexList));
								ATLTRACE(": new entry (0x%x)", newTexList.size() - 1);
							}

							ATLTRACE("\n");
						}

						if ((*it)->type == ATB_TEXTURE)
						{
							pLastTex = static_cast<psdl::texture*>(*it);
						}

						++it;
					}
				}
			}

		//	if (g_optimizeProps.bTextures)
		//		m_psdlDoc.GetDocument()->_textures = newTexList;
		//	//	g_psdl->m_aTextures.assign(newTexList.begin(), newTexList.end());
		//	}
		}*/
/*
		if (g_optimizeProps.bTextures)
		{
			size_t i = 0, j = 0;
			vector<vertexMap> textureMap;
			vector<vertexMap>::iterator it;
			vector<char*> newTexList;
			psdl::Attribute *atb;
			psdl::Texture *tex = 0;

			atb = g_psdl->nextAttribute(ATB_DIVIDEDROAD, tex, 0);
			do
			{
				if (tex)
				{
					it = textureMap.begin();
					while (it != textureMap.end())
					{
						if (it->from == tex->textureRef)
						{
							tex->textureRef = it->to;
							break;
						}
						++it;
					}
					if (it == textureMap.end())
					{
						vertexMap map = { tex->textureRef, newTexList.size() };
						tex->textureRef = newTexList.size();
						textureMap.push_back(map);
					}
				}
			}
			while (atb = g_psdl->nextAttribute(ATB_DIVIDEDROAD, tex));
		}*/

		m_view.Invalidate();
	}

	return 0;
}

LRESULT CMainFrame::OnLaunchMM2(WORD, WORD wID, HWND, BOOL&)
{
	SHELLEXECUTEINFO sei;
	LPCTSTR lpFile = config.tools.strMM2Exe.c_str();
	LPCTSTR lpDir  = config.tools.strMM2Dir.c_str();

//	_splitpath(lpFile, NULL, lpDir, NULL, NULL);
//	ExpandEnvironmentStrings(lpDirEnv, lpDir, MAX_PATH);

	ATLTRACE("\n%s\n", lpDir);

	ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));

	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_FLAG_NO_UI;
//	sei.hwnd = ;
//	sei.lpVerb = ;
	sei.lpFile = lpFile;
	sei.lpParameters = "-nomovie -nolockcheck";
	sei.lpDirectory = lpDir;
	sei.nShow = SW_SHOWNORMAL;
//	sei.hInstApp = ;
//	sei.lpIDList = ;
//	sei.lpClass = ;
//	sei.hkeyClass = ;
//	sei.dwHotKey = ;

	ShellExecuteEx(&sei);

	if (int(sei.hInstApp) == SE_ERR_FNF &&
		IDYES == MessageBox("The path currently set for the Midtown Madness 2 executable is invalid.\nDo you want to specify a new path in the \"Options\" dialog?", "Invalid path", MB_YESNO | MB_ICONQUESTION))
	{
		ShowOptions(ID_OPTIONS_TOOLS);
	}

	return 0;
}

LRESULT CMainFrame::OnLoadSDLView(WORD, WORD wID, HWND, BOOL&)
{
	LPTSTR lpTempPath = new TCHAR[MAX_PATH+1];
	GetTempPath(MAX_PATH+1, lpTempPath);

	string strTempFile = lpTempPath + m_psdlDoc.GetBaseName() + ".sdl";

	delete[] lpTempPath;

//	DocTemplateBase::ThreadParams params(&m_psdlDoc, strTempFile);
	CProgressDlg2 dlg;
//	error::code code = dlg.Run(&m_psdlDoc.WriteSDLThread, &params);

	PSDLDocTemplate::WriteSDLThread::pDocTmpl = &m_psdlDoc;
	PSDLDocTemplate::WriteSDLThread::strFileName = strTempFile;

	error::code code = dlg.Run(&PSDLDocTemplate::WriteSDLThread::Run);
	
	if (code & error::ok)
	{
		SHELLEXECUTEINFO sei;
		LPCTSTR lpFile = config.tools.strSDLViewExe.c_str();
		LPCTSTR lpDir  = _T("..");

		ATLTRACE("\n%s\n", strTempFile.c_str());

		ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));

		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_DOENVSUBST | SEE_MASK_FLAG_NO_UI;
		sei.lpFile = lpFile;
		sei.lpParameters = strTempFile.c_str();
		sei.lpDirectory = lpDir;
		sei.nShow = SW_SHOWNORMAL;

		ShellExecuteEx(&sei);
	}
	return 0;
}

int CMainFrame::ShowOptions(int iPageID = -1)
{
	COptionsPageGeneral     pgGeneral;
	COptionsPageRendering   pgRendering;
	COptionsPageTools       pgTools;
	COptionsPageDirectories pgDirectories;

	COptionsDialog dlg;
	dlg.AddPage(&pgGeneral);
	dlg.AddPage(&pgRendering);
	dlg.AddPage(&pgTools);
	dlg.AddPage(&pgDirectories);

	if (iPageID == ID_OPTIONS_TOOLS)
		dlg.SetInitialPage(&pgTools);

	return dlg.DoModal();
}

LRESULT CMainFrame::OnOptions(WORD, WORD, HWND, BOOL&)
{
	ShowOptions();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD, WORD, HWND, BOOL&)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainFrame::OnFileNew(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	DocTemplateBase* pDocTmpl = GetActiveDocument();

	if (pDocTmpl && pDocTmpl->FileExists() && pDocTmpl->IsModified())
	{
		CString sText;
		sText.Format(LS(IDS_SAVE_CHANGES), m_psdlDoc.GetFileName().c_str());
		int ret = MessageBox(sText, LS(IDR_MAINFRAME), MB_YESNOCANCEL | MB_ICONQUESTION);

		if (ret == IDYES)
			OnFileSave(NULL, NULL, NULL, bHandled);
		else if (ret == IDCANCEL)
			return 0;
	}

	if (wID == ID_FILE_NEW)
	{
		switch (m_iEditMode)
		{
			case ID_MODE_PSDL: wID = ID_FILE_NEW_PSDL; break;
			case ID_MODE_CPVS: wID = ID_FILE_NEW_CPVS; break;
		}
	}

	switch (wID)
	{
		case ID_FILE_NEW_PSDL:
			m_psdlDoc.NewDocument();
			m_cpvsDoc.SetPSDL(m_psdlDoc.GetDocument());
			SetEditingMode(ID_MODE_PSDL);
			break;

		case ID_FILE_NEW_CPVS:
			m_cpvsDoc.NewDocument();
			SetEditingMode(ID_MODE_CPVS);
			break;

		case ID_FILE_NEW_WORKSPACE:
			m_psdlDoc.NewDocument();
			m_cpvsDoc.NewDocument();
			break;
	}

	UpdateCaption();
	pDocTmpl->UpdateViews();
	m_view.Invalidate();
	return 0;
}

BOOL CMainFrame::OpenDocument(CString strDocName)
{
	CString strExt = strrchr(strDocName, '.') + 1;

	DocTemplateBase* pDocTmpl = NULL;
	int iMode = -1;

	for (int i = 0; i < PD_NUM_EXTENSIONS; i++)
	{
		if (strcmpi(strExt, m_sExtMap[i].strExt) == 0)
		{
			iMode    = m_sExtMap[i].iEditMode;
			pDocTmpl = m_sExtMap[i].pDocTmpl;
		}
	}

	if (pDocTmpl)
	{
		void* pDoc = NULL;

//		IOParams pio((LPCTSTR) strDocName, pDoc, pDocTmpl);

		strExt.MakeUpper();

//		CProgressDlg dlg(&DocTemplateBase::_LoadDocumentThread, &pio, "Loading " + strExt + " file...");
//		dlg.DoModal();

//		error::code code = dlg.GetError();
//		ATLTRACE("\nReturn code: %x\n", code);

		

		CProgressDlg2 dlg;
	//	dlg.Run(&DocTemplateBase::LoadDocumentThread);
		dlg.SetCaption("Loading " + strExt + " file...");

		DocTemplateBase::ThreadParams params(pDocTmpl, (LPCTSTR)strDocName, &pDoc);

		error::code code = dlg.Run(&DocTemplateBase::LoadDocumentThread, &params);

	//	error::code code = pDocTmpl->OpenDocument(strFileName);

		if (code & error::ok)
		{
		/*	if (iMode == ID_MODE_PSDL)
			{
				m_psdlDoc.UnloadTextures(m_view.m_hDC, m_view.m_hRC);
			}*/

			pDocTmpl->SetDocument(pDoc, (LPCTSTR) strDocName);

			ATLTRACE("Working directory: %s\n", pDocTmpl->GetPathName().c_str());
			SetCurrentDirectory(pDocTmpl->GetPathName().c_str());

/*			if (iMode == ID_MODE_PSDL)
			{
				GLParams pgl(m_view.GetDC(), m_view.GetRC(), &m_psdlDoc);
				CProgressDlg texDlg(&PSDLDocTemplate::_LoadTextures, &pgl, "Loading PSDL file...");
				texDlg.DoModal();
			//	m_psdlDoc.LoadTextures(m_view.m_hDC, m_view.m_hRC);
			}*/

			pDocTmpl->UpdateViews();

			if (code != error::ok) // additional warnings
			{
				if (code & error::cpvs_less_blocks)
					MessageBox("The CPVS file contains less blocks than the PSDL file currently loaded.\nBlocks will be added to match the size.", LS(IDR_MAINFRAME), MB_ICONINFORMATION);

				else if (code & error::cpvs_more_blocks)
					MessageBox("The CPVS file contains more blocks than the PSDL file currently loaded.\nBlocks will be removed to match the size.", LS(IDR_MAINFRAME), MB_ICONINFORMATION);
			}

			if (iMode == ID_MODE_PSDL)
				m_cpvsDoc.SetPSDL(m_psdlDoc.GetDocument());
			else if (iMode == ID_MODE_CPVS)
				m_psdlDoc.GetView()->SetCPVS(m_cpvsDoc.GetDocument());

			SetEditingMode(iMode);
			UpdateCaption();
			AddToList(strDocName);

			m_view.Invalidate();
			m_view.SetFocus();
		}
		else
		{
			if (pDoc != NULL) delete pDoc;

			if (code & error::wrong_format)
			{
			//	CString strErr = _T("Error");

			//	switch (iMode)
			//	{
			//		case ID_MODE_PSDL: strErr = "Not a PSDL file"; break;
			//		case ID_MODE_CPVS: strErr = "Not a CPVS file"; break;
			//		case ID_MODE_BAI:  strErr = "Not a BAI file";  break;
			//	}

				CString sGet, sErr;
				sGet.LoadString(IDS_INVALID_FORMAT);
				sErr.Format(sGet, strExt);

				MessageBox(sErr, LS(IDR_MAINFRAME), MB_ICONWARNING);
			}
			else if (code & error::aborted)
			{
				// Do nothing
			}
			else
			{
				DWORD dwErrorCode = GetLastError();
				LPTSTR lpMsg = NULL;
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsg, 0, NULL);
				MessageBox(strDocName + "\n\n" + lpMsg, LS(IDR_MAINFRAME), MB_ICONWARNING);
				LocalFree(lpMsg);
				return FALSE;
			}
		}
	}
	else
	{
		MessageBox(strDocName + "\n\n" + LS(IDS_INVALID_FILE), LS(IDR_MAINFRAME), MB_ICONWARNING);
		return FALSE;
	}

	return TRUE;
}

LRESULT CMainFrame::OnFileOpen(WORD, WORD, HWND, BOOL& bHandled)
{
/*	DocTemplateBase* pDocTmpl = GetActiveDocument();

	if (pDocTmpl && pDocTmpl->FileExists() && pDocTmpl->IsModified())
	{
		CString sText;
		sText.Format(LS(IDS_SAVE_CHANGES), m_psdlDoc.GetFileName().c_str());
		int ret = MessageBox(sText, LS(IDR_MAINFRAME), MB_YESNOCANCEL | MB_ICONQUESTION);

		if (ret == IDYES)
			OnFileSave(NULL, NULL, NULL, bHandled);
		else if (ret == IDCANCEL)
			return 0;
	}
*/
	CString sSelectedFile;

	CCenterFileDialog fDlg(TRUE, _T("psdl"), NULL, OFN_NOCHANGEDIR | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, _T("\
		MM2 Files (*.psdl; *.psd; *.cpvs; *.inst; *.bai; *.pathset)\0*.psdl; *.psd; *.cpvs; *.inst; *.bai; *.pathset\0\
		MM2 City Geometry (*.psdl; *.psd)\0*.psdl; *.psd\0\
		MM2 Potentially Visible Sets (*.cpvs)\0*.cpvs\0\
		MM2 INST (*.inst)\0*.inst\0\
		MM2 Ambient Paths (*.bai)\0*.bai\0\
		MM2 Pathset (*.pathset)\0*.pathset\0\
		All Files (*.*)\0*.*\0")
	);

	fDlg.m_ofn.lpstrInitialDir = config.files.strBrowseDir.c_str();

	if (IDOK == fDlg.DoModal())
	{
		config.files.strBrowseDir = fDlg.GetFolderPath();

		OpenDocument(fDlg.m_szFileName);
	}

	return 0;
}

LRESULT CMainFrame::OnFileOpenRecent(WORD, WORD wID, HWND, BOOL&)
{
	CString strDocName;
	GetFromList(wID, strDocName);
	if (OpenDocument(strDocName) == FALSE)
		RemoveFromList(wID);
	return 0;
}

LRESULT CMainFrame::OnFileSaveAs(WORD, WORD, HWND, BOOL&)
{
	CString sSelectedFile;

//	CCenterFileDialog fDlg(FALSE, _T("psdl"), GetActiveDocument()->GetFileName().c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("\
//		MM2 City Geometry (*.psdl; *.psd)\0*.psdl; *.psd\0")
//	);

	CString sFilter(GetActiveDocument()->GetExtensionString().c_str());

	CCenterFileDialog fDlg(
		FALSE,
		NULL,
		GetActiveDocument()->GetFileName().c_str(),
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		sFilter
	);

	if (IDOK == fDlg.DoModal())
	{
		HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	//	error::code code = m_psdlDoc.SaveDocument(fDlg.GetPathName());
		error::code code = GetActiveDocument()->SaveDocument(fDlg.GetPathName());

		SetCursor(hCursor);

		if (code & error::ok)
		{
			UpdateCaption();
			AddToList(fDlg.GetPathName());
		}
		else
		{
			DWORD dwErrorCode = GetLastError();
			LPTSTR lpMsg = NULL;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsg, 0, NULL);
			MessageBox((CString) fDlg.GetPathName() + "\n\n" + lpMsg, LS(IDR_MAINFRAME), MB_ICONWARNING);
			LocalFree(lpMsg);
		}
	}

	return 0;
}

LRESULT CMainFrame::OnFileImport(WORD, WORD, HWND, BOOL&)
{
	CString sSelectedFile;

	CCenterFileDialog fDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("\
		Scenery Description Language (*.sdl)\0*.sdl\0")
	//	3D Studio Mesh (*.3ds)\0*.3ds\0")
	);

	fDlg.SetCaption(_T("Import"));

	if (IDOK == fDlg.DoModal())
	{
//		HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

		LPTSTR strDocName = fDlg.GetPathName();
		CString strExt = strrchr(strDocName, '.') + 1;

		error::code code = error::ok;

//		if (strcmpi(strExt, "3ds") == 0)
//			code = m_psdlDoc.Read3DS(strDocName); else
//		if (strcmpi(strExt, "sdl") == 0)
//			code = m_psdlDoc.ReadSDL(strDocName);

//		SetCursor(hCursor);

		CProgressDlg2 dlg;
	//	DocTemplateBase::ThreadParams params(&m_psdlDoc, (LPCTSTR)strDocName);
	//	code = dlg.Run(&m_psdlDoc.ReadSDLThread, &params);

		PSDLDocTemplate::ReadSDLThread::pDocTmpl = &m_psdlDoc;
		PSDLDocTemplate::ReadSDLThread::strFileName = (LPCTSTR)strDocName;

		code = dlg.Run(&PSDLDocTemplate::ReadSDLThread::Run);

		if (code & error::ok)
		{
		//	m_psdlDoc.LoadTextures(m_view.m_hDC, m_view.m_hRC);
			SetEditingMode(ID_MODE_PSDL);
			m_psdlDoc.UpdateViews();
			m_view.Invalidate();
			m_view.SetFocus();
		}
		else if (!(code & error::aborted))
		{
			DWORD dwErrorCode = GetLastError();
			LPTSTR lpMsg = NULL;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsg, 0, NULL);
			MessageBox((CString) fDlg.GetPathName() + "\n\n" + lpMsg, LS(IDR_MAINFRAME), MB_ICONWARNING);
			LocalFree(lpMsg);
		}
	}

	return 0;
}

LRESULT CMainFrame::OnFileExport(WORD, WORD, HWND, BOOL&)
{
	CString sSelectedFile;

	CCenterFileDialog fDlg(FALSE, _T("sdl"), _T(""), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("\
		MM2 SDL (*.sdl)\0*.sdl\0")
	);

	fDlg.SetCaption(_T("Export"));

	if (IDOK == fDlg.DoModal())
	{
		CProgressDlg2 dlg;

		PSDLDocTemplate::WriteSDLThread::pDocTmpl = &m_psdlDoc;
		PSDLDocTemplate::WriteSDLThread::strFileName = fDlg.GetPathName();

		error::code code = dlg.Run(&PSDLDocTemplate::WriteSDLThread::Run);

		if (!(code & error::ok))
		{
			DWORD dwErrorCode = GetLastError();
			LPTSTR lpMsg = NULL;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsg, 0, NULL);
			MessageBox((CString) fDlg.GetPathName() + "\n\n" + lpMsg, LS(IDR_MAINFRAME), MB_ICONWARNING);
			LocalFree(lpMsg);
		}
	}

	return 0;
}

DocTemplateBase* CMainFrame::GetActiveDocument(void)
{
	switch (m_iEditMode)
	{
		case ID_MODE_PSDL: return &m_psdlDoc;
		case ID_MODE_CPVS: return &m_cpvsDoc;
		case ID_MODE_BAI:  return &m_baiDoc;
	}
	return NULL;
}

BOOL CMainFrame::CanSave(void)
{
	DocTemplateBase* pDoc = GetActiveDocument();

	if (pDoc)
		return pDoc->IsModified();

	return 0;
}

LRESULT CMainFrame::OnFileSave(WORD, WORD, HWND, BOOL& bHandled)
{
//	if (!GetActiveDocument()->FileExists())
//		return OnFileSaveAs(NULL, NULL, NULL, bHandled);

	if (!GetActiveDocument()->FileExists() || MessageBox("Overwrite same file? Click No to save under a different name.", "Save", MB_YESNOCANCEL | MB_ICONWARNING) == IDNO)
	{
		return OnFileSaveAs(NULL, NULL, NULL, bHandled);
	}

	if (GetActiveDocument()->SaveDocument() == error::ok)
	{
		UpdateCaption();
	}
	else
	{
		DWORD dwErrorCode = GetLastError();
		LPTSTR lpMsg = NULL;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsg, 0, NULL);
		MessageBox((CString) GetActiveDocument()->GetFileName().c_str() + "\n\n" + lpMsg, LS(IDR_MAINFRAME), MB_ICONWARNING);
		LocalFree(lpMsg);
	}

	return 0;
}

LRESULT CMainFrame::OnOpenContainingFolder(WORD, WORD, HWND, BOOL&)
{
	DocTemplateBase* doc = NULL;

	switch (m_iEditMode)
	{
		case ID_MODE_PSDL:
			doc = &m_psdlDoc;
			break;

		default:
			return 0;
	}

	TCHAR lpDir[MAX_PATH];
	_splitpath(doc->GetFileName(true).c_str(), NULL, lpDir, NULL, NULL);

	ShellExecute(m_hWnd, "open", "explorer", (CString) "/select," + doc->GetFileName(true).c_str(), NULL, SW_SHOWNORMAL);

	return 0;
}

CString CMainFrame::GetUndoDescription(void)
{
	CString sRet;
	sRet.Format(LS(IDS_UNDO_COMMAND), HistoryManager::GetUndoDescription());
	return sRet;
}

CString CMainFrame::GetRedoDescription(void)
{
	CString sRet;
	sRet.Format(LS(IDS_REDO_COMMAND), HistoryManager::GetRedoDescription());
	return sRet;
}

LRESULT CMainFrame::OnGenerateBAI(WORD, WORD, HWND, BOOL&) // Generates routes + culling
{
	string strConfigFile = m_psdlDoc.GetPathName() + m_psdlDoc.GetBaseName() + "_baiconfig.txt";

	if (GetFileAttributes(strConfigFile.c_str()) != INVALID_FILE_ATTRIBUTES)
		config.dialogs.generateBAI.strConfigFile = strConfigFile;

	CGenerateBAIDlg dlg;

	if (IDOK == dlg.DoModal())
	{
	//	bool bResult1 = true, bResult2 = true;

	//	if (config.dialogs.generateBAI.bRoutes)  bResult1 = GenerateAIRoutes();
	//	if (config.dialogs.generateBAI.bCulling) bResult2 = GenerateAICulling();

		BAIDocTemplate::GenerateBAIThread::pDocTmpl = &m_baiDoc;
		BAIDocTemplate::GenerateBAIThread::pPSDL = m_psdlDoc.GetDocument();

		CProgressDlg2 dlg;

		if (dlg.Run(&BAIDocTemplate::GenerateBAIThread::Run))
		{
			m_view.Invalidate();
		//	MessageBox("Done!");
		//	SetEditingMode(ID_MODE_BAI);
		}
		else
		{
			MessageBox("Failed");
		}
	}

/*	{
		// Write TXT file
		ofstream ofs(strTxtFile.c_str());

		ofs << "nRoads: "         << joinedRoads.size()  << endl;
		ofs << "nIntersections: " << roadIDs.size()      << endl;
		ofs << "nBlocks: "        << pPSDL->num_blocks() << endl << endl;

		for (j = 0; j < joinedRoads.size(); ++j)
		{
			psdl::road_strip* pJoinedRoad = joinedRoads[j];

			ofs << "Road " << (j + 1) << " {" << endl;

			ofs << "  Blocks: " << roadBlocks[j].size();

			for (k = 0; k < roadBlocks[j].size(); ++k)
			{
				ofs << " " << roadBlocks[j][k];
			}

			ofs << endl;

			ofs << "  Settings: 0 0.0 15.0" << endl;

			ofs << endl;

			ofs << "  RightLanes: 1"    << endl;
			ofs << "  RightTrams: 0"    << endl;
			ofs << "  RightTrains: 0"   << endl;
			ofs << "  RightUnknown0: 1" << endl;
			ofs << "  RightUnknown1: 0" << endl;

			ofs << endl;

			ofs << "  LeftLanes: 1"     << endl;
			ofs << "  LeftTrams: 0"     << endl;
			ofs << "  LeftTrains: 0"    << endl;
			ofs << "  LeftUnknown0: 1"  << endl;
			ofs << "  LeftUnknown1: 0"  << endl;

			ofs << endl;

			ofs << "  EndIntersection: " << junctionIDs[j].first << endl;
			ofs << "  EndSettings: 52685 1 0" << endl;
			ofs << "  EndTrafficLight: 0 0 0" << endl;
			ofs << "                   0 0 0" << endl;

			ofs << endl;

			ofs << "  StartIntersection: " << junctionIDs[j].second << endl;
			ofs << "  StartSettings: 52685 1 0" << endl;
			ofs << "  StartTrafficLight: 0 0 0" << endl;
			ofs << "                     0 0 0" << endl;

			ofs << endl;

			unsigned short nSections = pJoinedRoad->num_vertices() / (pJoinedRoad->type == ATB_ROAD ? 4 : 6);

			ofs << "  Sections: " << nSections << endl;

			for (k = 0; k < pJoinedRoad->num_vertices(); ++k)
			{
				Vertex* v = pJoinedRoad->get_vertex(k);

				ofs << "    " << v->x << " " << v->y << " " << v->z << endl;
			}

			ofs << "}" << endl << endl;
		}

		for (j = 0; j < roadIDs.size(); ++j)
		{
			Vertex vCenter = CenterPoint(pPSDL->get_block(junctionBlocks[j]), pPSDL);

			ofs << "Intersection " << (j + 1) << " {" << endl;

			ofs << "  Block: " << junctionBlocks[j] + 1 << endl;

			ofs << "  Center: " << vCenter.x << " " << vCenter.y << " " << vCenter.z << endl;

			ofs << "  Roads: " << roadIDs[j].size();

			for (k = 0; k < roadIDs[j].size(); ++k)
			{
				ofs << " " << roadIDs[j][k];
			}

			ofs << endl;

			ofs << "}" << endl << endl;
		}

		delete[] usedBlocks;
	}

	if (CBaiGenerator::Run(strTxtFile, strBaiFile, strLogFile))
	{
		MessageBox(_T("Done!"), _T("Generate BAI"));
	}
	else
	{
		MessageBox(_T("Error!"), _T("Generate BAI"));
	}*/

	return 0;
}
