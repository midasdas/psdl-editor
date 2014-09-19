#include "stdafx.h"

#include <algorithm>
#include <shellapi.h>
#include <process.h>

#include "mainfrm.h"
#include "aboutdlg.h"
#include "dialogs.h"
#include "optionsdlg.h"

using namespace std;

PSDLDocTemplate CMainFrame::m_psdlDoc;
CPVSDocTemplate CMainFrame::m_cpvsDoc;

CMainFrame* CMainFrame::this_ptr = NULL;

const extMap CMainFrame::m_sExtMap[] =
{
	{ "psdl",    &m_psdlDoc, ID_MODE_PSDL    },
	{ "psd",     &m_psdlDoc, ID_MODE_PSDL    },
	{ "cpvs",    &m_cpvsDoc, ID_MODE_CPVS    },
	{ "inst",    NULL,       ID_MODE_INST    },
	{ "bai",     NULL,       ID_MODE_BAI     },
	{ "pathset", NULL,       ID_MODE_PATHSET }
};

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// Process tab key
	if (m_wndProps.IsDialogMessage(pMsg))
		return TRUE;
	if (m_dlgTransform && m_dlgTransform.IsDialogMessage(pMsg))
		return TRUE;

	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return FALSE;
}

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();
	UIUpdateMenuBar(FALSE, TRUE);

	UIEnable(ID_FILE_SAVE, CanSave());

	UISetText(ID_EDIT_UNDO, GetUndoDescription());
	UISetText(ID_EDIT_REDO, GetRedoDescription());
	UIEnable(ID_EDIT_UNDO, CanUndo());
	UIEnable(ID_EDIT_REDO, CanRedo());
	UIEnable(ID_EDIT_TRANSFORM, !m_dlgTransform);

	UIEnable(ID_MODE_CPVS, FALSE);
	UIEnable(ID_MODE_INST, FALSE);
	UIEnable(ID_MODE_BAI, FALSE);
	UIEnable(ID_MODE_PATHSET, FALSE);

	UIEnable(ID_FILE_OPENCONTAININGFOLDER, GetActiveDocument()->FileExists());

	UISetCheck(ID_WINDOWS_CITYBLOCKS, ::IsWindowVisible(m_wndBlocks));
	UISetCheck(ID_WINDOWS_PERIMETER,  ::IsWindowVisible(m_wndPerimeter));
	UISetCheck(ID_WINDOWS_ATTRIBUTES, ::IsWindowVisible(m_wndAttribs));
	UISetCheck(ID_WINDOWS_PROPERTIES, ::IsWindowVisible(m_wndProps));

	UISetCheck(ID_VIEW_WIREFRAME, g_options.display.bWireframe);

	return FALSE;
}

LRESULT CMainFrame::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	CString strDocName;

	for (int i = GetMaxEntries() - 1; i >= 0; i--)
	{
		if (GetFromList(ID_FILE_MRU_FIRST + i, strDocName))
			g_options.files.aRecentFiles.push((LPCTSTR) strDocName);
	}

	bHandled = FALSE;
	return FALSE;
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

	CreateSimpleToolBar();
	CToolBarCtrl tool = m_hWndToolBar;
	tool.SetStyle(tool.GetStyle() | TBSTYLE_FLAT);

	CreateSimpleStatusBar();

	UIAddMenuBar(m_hWnd);
	UIAddToolBar(m_hWndToolBar);

	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	SetMenuHandle(GetSubMenu(GetMenu(), 0));
	SetMaxEntries(8);
	SetMaxItemLength(20);

	while (!g_options.files.aRecentFiles.empty())
	{
		AddToList(g_options.files.aRecentFiles.top().c_str());
		g_options.files.aRecentFiles.pop();
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
	m_cpvsDoc.SetPSDL(m_psdlDoc.GetDocument());

	SetEditingMode(ID_MODE_PSDL);
	UpdateCaption();

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	return 0;
}

LRESULT CMainFrame::OnPaintDescendants(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
	HDC hDC = (HDC) wParam;
	HGLRC hRC = (HGLRC) lParam;

	m_psdlDoc.RenderScene(hDC, hRC);
	m_cpvsDoc.RenderScene(hDC, hRC);

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

LRESULT CMainFrame::OnViewWireframe(WORD, WORD, HWND, BOOL&)
{
	g_options.display.bWireframe ^= 1;
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

	if (::IsWindowVisible(hWnd))
		m_dock.HideWindow(hWnd);
	else
		m_dock.DockWindow(hWnd, DOCK_LASTKNOWN);

	return 0;
}

void CMainFrame::SelectBlock(long iIndex)
{
	this_ptr->m_psdlDoc.SelectBlock(iIndex);
	this_ptr->m_cpvsDoc.SelectBlock(iIndex);

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

bool CMainFrame::TransformEntities(transformProps& sProps)
{
	std::vector<unsigned long> aVerts;

	int iBlock = -1;

	while ((iBlock = m_wndBlocks.m_list.GetNextItem(iBlock, LVNI_SELECTED)) >= 0)
	{
		psdl::block* block = m_psdlDoc.GetBlock(iBlock);

		for (unsigned long iPt = 0; iPt < block->num_perimeters(); iPt++)
		{
			unsigned short iVertex = block->_perimeter[iPt].vertex;

			if (std::find(aVerts.begin(), aVerts.end(), iVertex) == aVerts.end())
				aVerts.push_back(iVertex);
		}

		for (unsigned long iAtb = 0; iAtb < block->num_attributes(); iAtb++)
		{
			psdl::attribute* atb = block->get_attribute(iAtb);

			unsigned long j;

			switch (atb->type)
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

						if (std::find(aVerts.begin(), aVerts.end(), iVertex) == aVerts.end())
							aVerts.push_back(iVertex);
					}
					break;

				case ATB_CROSSWALK:

					for (j = 0; j < 4; j++)
					{
						unsigned short iVertex = static_cast<psdl::crosswalk*>(atb)->get_vertex_ref(j);

						if (std::find(aVerts.begin(), aVerts.end(), iVertex) == aVerts.end())
							aVerts.push_back(iVertex);
					}
					break;

				case ATB_SLIVER:
				case ATB_FACADEBOUND:
					{
						unsigned short iVertex;

						iVertex = static_cast<psdl::facade*>(atb)->left;
						if (std::find(aVerts.begin(), aVerts.end(), iVertex) == aVerts.end())
							aVerts.push_back(iVertex);

						iVertex = static_cast<psdl::facade*>(atb)->right;
						if (std::find(aVerts.begin(), aVerts.end(), iVertex) == aVerts.end())
							aVerts.push_back(iVertex);

						iVertex = static_cast<psdl::facade*>(atb)->top;
						if (std::find(aVerts.begin(), aVerts.end(), iVertex) == aVerts.end())
							aVerts.push_back(iVertex);
					}
					break;
			}
		}
	}

	if (aVerts.size() > 0)
	{
		psdl::vertex vTranslate(sProps.fX, sProps.fY, sProps.fZ);
		double dRotate = sProps.fAngle / 180.0 * PI;

		psdl::vertex vOrigin(0, 0, 0);

		for (size_t i = 0; i < aVerts.size(); i++)
		{
			psdl::vertex* pVertex = m_psdlDoc.GetDocument()->get_vertex(aVerts[i]);
			m_psdlDoc.MoveVertex(pVertex, vTranslate);
			m_psdlDoc.RotateVertex(pVertex, vOrigin, dRotate);
		}

		m_view.Invalidate();
		return true;
	}

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
	//	psdl::vertex vOffset(1220.0787f, -20.2605f, -870.612f);

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

LRESULT CMainFrame::OnOptimizePSDL(WORD, WORD, HWND, BOOL&)
{
	COptimizeDlg dlg;

	if (IDOK == dlg.DoModal())
	{
		if (g_optimizeProps.bTextureRefs)
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
					/*	if (nLastTexRef == nTexRef)
						{
							if ((*it)->last)
							{
								(*(it - 1))->last = true;
							}

							it = pBlock->_attributes.erase(it);
							pBlock->addAttributeSize(-2);

							ATLTRACE("Double texture reference: 0x%x = 0x%x\n", nTexRef, nLastTexRef);
						}
						else
						{
							++it;
						}*/
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
/*
			if (g_optimizeProps.bTextures)
				m_psdlDoc.GetDocument()->_textures = newTexList;
			//	g_psdl->m_aTextures.assign(newTexList.begin(), newTexList.end());
			}*/
		}
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
	}

	return 0;
}

LRESULT CMainFrame::OnLaunchMM2(WORD, WORD wID, HWND, BOOL&)
{
	SHELLEXECUTEINFO sei;
	TCHAR lpDir[MAX_PATH];
	LPCTSTR lpFile = g_options.tools.strMM2Exe.c_str();

	_splitpath(lpFile, NULL, lpDir, NULL, NULL);

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
	//	COptionsDlg dlg(2);
	//	dlg.DoModal();
		ShowOptions(ID_OPTIONS_TOOLS);
	}

	return 0;
}

INT CMainFrame::ShowOptions(INT iPageID = -1)
{
	COptionsPageGeneral     pgGeneral;
	COptionsPageTools       pgTools;
	COptionsPageDirectories pgDirectories;

	COptionsDialog dlg;
	dlg.AddPage(&pgGeneral);
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

		IOParams pio((LPCTSTR) strDocName, pDoc, pDocTmpl);

		strExt.MakeUpper();

		CProgressDlg dlg(&DocTemplateBase::_LoadDocumentThread, &pio, "Loading " + strExt + " file...");
		dlg.DoModal();

		error::code code = dlg.GetError();
		ATLTRACE("\nReturn code: %x\n", code);

	//	error::code code = pDocTmpl->OpenDocument(strFileName);

		if (code & error::ok)
		{
			if (iMode == ID_MODE_PSDL)
			{
				m_psdlDoc.UnloadTextures(m_view.m_hDC, m_view.m_hRC);
			}

			pDocTmpl->SetDocument(pDoc, (LPCTSTR) strDocName);

			ATLTRACE("Working directory: %s\n", pDocTmpl->GetPathName().c_str());
			SetCurrentDirectory(pDocTmpl->GetPathName().c_str());

			if (iMode == ID_MODE_PSDL)
			{
				GLParams pgl(m_view.GetDC(), m_view.GetRC(), &m_psdlDoc);
				CProgressDlg texDlg(&PSDLDocTemplate::_LoadTextures, &pgl, "Loading PSDL file...");
				texDlg.DoModal();
			//	m_psdlDoc.LoadTextures(m_view.m_hDC, m_view.m_hRC);
			}

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
				CString strErr = _T("Error");

				switch (iMode)
				{
					case ID_MODE_PSDL: strErr = "Not a PSDL file"; break;
					case ID_MODE_CPVS: strErr = "Not a CPVS file"; break;
				}

				MessageBox(strErr, LS(IDR_MAINFRAME), MB_ICONWARNING);
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

	CCenterFileDialog fDlg(TRUE, _T("psdl"), NULL, OFN_NOCHANGEDIR | OFN_HIDEREADONLY, _T("\
		MM2 Files (*.psdl; *.psd; *.cpvs; *.inst; *.bai; *.pathset)\0*.psdl; *.psd; *.cpvs; *.inst; *.bai; *.pathset\0\
		MM2 City Geometry (*.psdl; *.psd)\0*.psdl; *.psd\0\
		MM2 Potentially Visible Sets (*.cpvs)\0*.cpvs\0\
		MM2 INST (*.inst)\0*.inst\0\
		MM2 Ambient Paths (*.bai)\0*.bai\0\
		MM2 Pathset (*.pathset)\0*.pathset\0\
		All Files (*.*)\0*.*\0")
	);

	fDlg.m_ofn.lpstrInitialDir = g_options.files.strBrowseDir.c_str();

	if (IDOK == fDlg.DoModal())
	{
		g_options.files.strBrowseDir = fDlg.GetFolderPath();

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

	CCenterFileDialog fDlg(FALSE, _T("psdl"), GetActiveDocument()->GetFileName().c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("\
		MM2 City Geometry (*.psdl; *.psd)\0*.psdl; *.psd\0")
	);

	if (IDOK == fDlg.DoModal())
	{
		HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

		error::code code = m_psdlDoc.SaveDocument(fDlg.GetPathName());

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
		3D Studio Mesh (*.3ds)\0*.3ds\0\
		MM2 SDL (*.sdl)\0*.sdl\0")
	);

	fDlg.SetCaption(_T("Import"));

	if (IDOK == fDlg.DoModal())
	{
		HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

		error::code code = m_psdlDoc.ReadSDL(fDlg.GetPathName());

		SetCursor(hCursor);

		if (code & error::ok)
		{
			m_psdlDoc.LoadTextures(m_view.m_hDC, m_view.m_hRC);
			SetEditingMode(ID_MODE_PSDL);
			m_psdlDoc.UpdateViews();
			m_view.Invalidate();
			m_view.SetFocus();
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

	return FALSE;
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
		HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

		error::code code = m_psdlDoc.WriteSDL(fDlg.GetPathName());

		SetCursor(hCursor);

		if (code != error::ok)
		{
			DWORD dwErrorCode = GetLastError();
			LPTSTR lpMsg = NULL;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsg, 0, NULL);
			MessageBox((CString) fDlg.GetPathName() + "\n\n" + lpMsg, LS(IDR_MAINFRAME), MB_ICONWARNING);
			LocalFree(lpMsg);
		}
	}

	return FALSE;
}

DocTemplateBase* CMainFrame::GetActiveDocument(void)
{
	switch (m_iEditMode)
	{
		case ID_MODE_PSDL: return &m_psdlDoc;
		case ID_MODE_CPVS: return &m_cpvsDoc;
	}
	return NULL;
}

BOOL CMainFrame::CanSave(void)
{
	DocTemplateBase* pDoc = GetActiveDocument();

	if (pDoc)
		return pDoc->IsModified();

	return FALSE;
}

LRESULT CMainFrame::OnFileSave(WORD, WORD, HWND, BOOL& bHandled)
{
	if (!GetActiveDocument()->FileExists())
		return OnFileSaveAs(NULL, NULL, NULL, bHandled);

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
