#include "stdafx.h"

#include <math.h>
#include <algorithm>
#include <vector>
#include <shlwapi.h>

#include "mainfrm.h"
#include "psdl-editor.h"
#include "optionsdlg.h"
#include "aboutdlg.h"
#include "dialogs.h"
#include "tools.h"

using namespace std;

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// Process tab key
	if (m_wndProps.IsDialogMessage(pMsg))
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

	UISetCheck(ID_WINDOWS_CITYBLOCKS, ::IsWindowVisible(m_wndBlocks));
	UISetCheck(ID_WINDOWS_ATTRIBUTES, ::IsWindowVisible(m_wndAttribs));
	UISetCheck(ID_WINDOWS_PROPERTIES, ::IsWindowVisible(m_wndProps));

	return FALSE;
}

LRESULT CMainFrame::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	bHandled = FALSE;
	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{
	glClearColor(0.f, 0.f, 0.f, 1.f);

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

	m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE);
	m_view.SetDCPixelFormat(&pfd);
	m_view.SetFocus();

	// Initiate docking framework
	HWND hwndDock = m_dock.Create(m_hWnd, rcDefault);
	m_dock.SetClient(m_hWndClient);

	m_wndProps.Create(m_hWnd,	rcDefault, _T("Attribute Properties"),	ATL_SIMPLE_DOCKVIEW_STYLE);
	m_wndAttribs.Create(m_hWnd,	rcDefault, _T("Block Attributes"),		ATL_SIMPLE_DOCKVIEW_STYLE);
	m_wndPerimeter.Create(m_hWnd,	rcDefault, _T("Perimeter"),			ATL_SIMPLE_DOCKVIEW_STYLE);
	m_wndBlocks.Create(m_hWnd,	rcDefault, _T("City Blocks"),			ATL_SIMPLE_DOCKVIEW_STYLE);

	m_dock.AddWindow(m_wndProps);
	m_dock.AddWindow(m_wndAttribs);
	m_dock.AddWindow(m_wndPerimeter);
	m_dock.AddWindow(m_wndBlocks);
	m_dock.DockWindow(m_wndProps,	DOCK_RIGHT);
	m_dock.DockWindow(m_wndAttribs,	DOCK_RIGHT);
	m_dock.DockWindow(m_wndPerimeter,	DOCK_LEFT);
	m_dock.DockWindow(m_wndBlocks,	DOCK_LEFT);

	m_dock.SetPaneSize(DOCK_LEFT, 200);
	m_dock.SetPaneSize(DOCK_RIGHT, 200);

	m_hWndClient = hwndDock;

	m_psdlDoc.SetViews(&m_wndBlocks, &m_wndPerimeter, &m_wndAttribs, &m_wndProps);
	m_cpvsDoc.SetViews(&m_wndBlocks);
	m_psdlDoc.NewDocument();
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

	switch (wID) {
		case ID_WINDOWS_CITYBLOCKS:	hWnd = m_wndBlocks;		break;
		case ID_WINDOWS_ATTRIBUTES:	hWnd = m_wndAttribs;	break;
		case ID_WINDOWS_PROPERTIES:	hWnd = m_wndProps;		break;
		default: return 0;
	}

	if (::IsWindowVisible(hWnd))
		m_dock.HideWindow(hWnd);
	else
		m_dock.DockWindow(hWnd, DOCK_LASTKNOWN);

	return 0;
}

void CMainFrame::SelectBlock(long iIndex)
{
	m_psdlDoc.SelectBlock(iIndex);
	m_cpvsDoc.SelectBlock(iIndex);

	m_view.Invalidate();
}

void CMainFrame::SelectAttribute(psdl::block* pBlock, long iIndex)
{
	m_wndProps.SetAttribute(pBlock->get_attribute(iIndex));
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

LRESULT CMainFrame::OnSetEditingMode(WORD, WORD wID, HWND, BOOL&)
{
	SetEditingMode(wID);
	UpdateCaption();
	return 0;
}

LRESULT CMainFrame::OnInsertBlock(WORD, WORD wID, HWND, BOOL&)
{
	unsigned int nInsert = m_wndBlocks.list()->GetCaretIndex();
	psdl::block block;

//	block.addPerimeterPoint(0);

	m_psdlDoc.InsertBlock(block, nInsert);
	m_psdlDoc.m_bModified = true;

	AddHistoryState(wID);
	UpdateCaption();
	return 0;
}

LRESULT CMainFrame::OnDuplicateBlock(WORD, WORD wID, HWND, BOOL&)
{
	CDuplicateDlg dlg;

	if (IDOK == dlg.DoModal())
	{
		unsigned long nBlocks = m_wndBlocks.m_list.GetSelCount();
		int* aBlockIds = new int[nBlocks];
		m_wndBlocks.m_list.GetSelItems(nBlocks, aBlockIds);

		// Experiment, should be removed
		psdl::vertex vOffset(1220.0787f, -20.2605f, -870.612f);

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

						block->_perimeter[iPt].vertex = m_psdlDoc.CopyVertex(block->_perimeter[iPt].vertex, vOffset, aDuplicates);
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
										j, m_psdlDoc.CopyVertex(iVertex, vOffset, aDuplicates)
									);
								}
								break;

							case ATB_CROSSWALK:

								for (j = 0; j < 4; j++)
								{
									unsigned short nVertexRef = static_cast<psdl::crosswalk*>(atb)->get_vertex_ref(j);

									static_cast<psdl::crosswalk*>(atb)->set_vertex_ref(
										j, m_psdlDoc.CopyVertex(nVertexRef, vOffset, aDuplicates)
									);
								}
								break;

							case ATB_FACADE:

								static_cast<psdl::facade*>(atb)->bottom = m_psdlDoc.CopyHeight(static_cast<psdl::facade*>(atb)->bottom);

							case ATB_SLIVER:
							case ATB_FACADEBOUND:

								static_cast<psdl::facade*>(atb)->left   = m_psdlDoc.CopyVertex(static_cast<psdl::facade*>(atb)->left, vOffset, aDuplicates);
								static_cast<psdl::facade*>(atb)->right  = m_psdlDoc.CopyVertex(static_cast<psdl::facade*>(atb)->right, vOffset, aDuplicates);
								static_cast<psdl::facade*>(atb)->top    = m_psdlDoc.CopyHeight(static_cast<psdl::facade*>(atb)->bottom);

								break;

							default:
								i--;
						}
					}

					ATLTRACE("Block %x copied; vertices copied for %d attributes\n", iIndex, j);
				}

				m_psdlDoc.AddBlock(*block);
			}
		}
	}

	m_psdlDoc.m_bModified = true;
	AddHistoryState(wID);
	UpdateCaption();
	return 0;
}

LRESULT CMainFrame::OnOptimizePSDL(WORD, WORD wID, HWND, BOOL&)
{
/*	COptimizeDlg dlg;

	if (IDOK == dlg.DoModal())
	{
		if (g_optimizeProps.bTextureRefs)
		{
		//	vector<vertexMap> textureMap[3];
			vector<char*> newTexList;

			for (size_t i = 0; i < g_psdl->numBlocks(); i++)
			{
				unsigned short nTexRef, nLastTexRef = 0;
				psdl::block *block = g_psdl->getBlock(i);

				vector<psdl::Attribute*>::iterator it = block->m_attributes.begin();

				while (it != block->m_attributes.end())
				{
					switch ((*it)->type())
					{
						case ATB_TEXTURE:
							nTexRef = static_cast<psdl::Texture*>(*it)->textureRef;

							if (nLastTexRef == nTexRef)
							{
								if ((*it)->hasLastFlag())
								{
									(*(it - 1))->setLastFlag();
								}

								it = block->m_attributes.erase(it);
								block->addAttributeSize(- 2);

							//	ATLTRACE("Double texture reference: 0x%x = 0x%x\n", nTexRef, nLastTexRef);
							}
							else
							{
								++it;
							}

							nLastTexRef = nTexRef;
							break;

						default:
							++it;
					}
				}
*/
/*				if (g_optimizeProps.bTextures)
				{
					unsigned char nMin = 0, nMax = 0;
					psdl::Texture *lastTex = NULL;

					it = block->m_attributes.begin();

					while (it != block->m_attributes.end())
					{
						switch ((*it)->type())
						{
							case ATB_TEXTURE:		nMin = 0;	break;
							case ATB_SIDEWALK:		nMin = 2;	break;
							case ATB_CROSSWALK:
							case ATB_ROAD:
							case ATB_DIVIDEDROAD:	nMin = 3;	break;
							case ATB_TUNNEL:		nMin = 6;	break;
							default:				nMin = 1;
						}

						nMax = max(nMin, nMax);

						if (lastTex && ((*it)->type() == ATB_TEXTURE || it == block->m_attributes.end() - 1))
						{
							ATLTRACE("@ block %x, attribute %x: reassigning textures (%d)", i, it - block->m_attributes.begin(), nMax);

							vector<char*>::iterator fnd;

							vector<char*>::iterator first = g_psdl->m_aTextures.begin() + lastTex->textureRef;
							vector<char*>::iterator last  = first + nMax;

							fnd = search(newTexList.begin(), newTexList.end(), first, last);

							if (fnd != newTexList.end())
							{
								lastTex->id = 0x50;
								lastTex->textureRef = fnd - newTexList.begin();
								ATLTRACE(": using existing");
							}
							else
							{
								lastTex->id = 0x50;
								lastTex->textureRef = newTexList.size();
								copy(first, last, back_inserter(newTexList));
								ATLTRACE(": new entry (0x%x)", newTexList.size() - 1);
							}

							ATLTRACE("\n");
						}

						if ((*it)->type() == ATB_TEXTURE)
						{
							lastTex = static_cast<psdl::Texture*>(*it);
						}

						++it;
					}
				}
			}

			if (g_optimizeProps.bTextures)
				g_psdl->m_aTextures = newTexList;
			//	g_psdl->m_aTextures.assign(newTexList.begin(), newTexList.end());*/
/*			}
		}

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
		}
	}
*/
	return 0;
}

LRESULT CMainFrame::OnLaunchMM2(WORD, WORD wID, HWND, BOOL&)
{
	SHELLEXECUTEINFO sei;
	TCHAR lpDir[MAX_PATH];
	LPCTSTR lpFile = g_options.tools.mm2Exe;

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

	if (int(sei.hInstApp) == SE_ERR_FNF && IDYES == MessageBox("The path currently set for the Midtown Madness 2 executable is invalid.\nDo you want to specify a new path in the \"Options\" dialog?", "Invalid path", MB_YESNO | MB_ICONQUESTION))
	{
		COptionsDlg dlg(2);
		dlg.DoModal();
	}

	return 0;
}

LRESULT CMainFrame::OnOptions(WORD, WORD wID, HWND, BOOL&)
{
	COptionsDlg dlg;
	dlg.DoModal();
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
	DocTemplateBase* pDoc = GetActiveDocument();

	if (pDoc && pDoc->FileExists() && pDoc->IsModified())
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
	m_view.Invalidate();
	return 0;
}

LRESULT CMainFrame::OnFileOpen(WORD, WORD, HWND, BOOL& bHandled)
{
	DocTemplateBase* pDoc = GetActiveDocument();

	if (pDoc && pDoc->FileExists() && pDoc->IsModified())
	{
		CString sText;
		sText.Format(LS(IDS_SAVE_CHANGES), m_psdlDoc.GetFileName().c_str());
		int ret = MessageBox(sText, LS(IDR_MAINFRAME), MB_YESNOCANCEL | MB_ICONQUESTION);

		if (ret == IDYES)
			OnFileSave(NULL, NULL, NULL, bHandled);
		else if (ret == IDCANCEL)
			return 0;
	}

	CString sSelectedFile;

	CCenterFileDialog fDlg(TRUE, _T("psdl"), NULL, OFN_HIDEREADONLY, _T("\
		MM2 Files (*.psdl; *.psd; *.cpvs; *.inst; *.bai; *.pathset)\0*.psdl; *.psd; *.cpvs; *.inst; *.bai; *.pathset\0\
		MM2 City Geometry (*.psdl; *.psd)\0*.psdl; *.psd\0\
		MM2 Potentially Visible Sets (*.cpvs)\0*.cpvs\0\
		MM2 INST (*.inst)\0*.inst\0\
		MM2 Ambient Paths (*.bai)\0*.bai\0\
		MM2 Pathset (*.pathset)\0*.pathset\0\
		All Files (*.*)\0*.*\0")
	);

	fDlg.m_ofn.lpstrInitialDir = g_options.files.browseDir;

	if (IDOK == fDlg.DoModal())
	{
		g_options.files.browseDir = fDlg.GetFolderPath();

		char* strExtension = fDlg.GetFileExt();

		DocTemplateBase* pDoc = NULL;
		int iMode = -1;

		struct extMap { const char* strExt; DocTemplateBase* pDocTmpl; int iEditMode; };

		extMap map[6] =
		{
			{ "psdl",    &m_psdlDoc, ID_MODE_PSDL    },
			{ "psd",     &m_psdlDoc, ID_MODE_PSDL    },
			{ "cpvs",    &m_cpvsDoc, ID_MODE_CPVS    },
			{ "inst",    NULL,       ID_MODE_INST    },
			{ "bai",     NULL,       ID_MODE_BAI     },
			{ "pathset", NULL,       ID_MODE_PATHSET }
		};

		for (int i = 0; i < 6; i++)
		{
			if (!strcmpi(strExtension, map[i].strExt))
			{
				iMode = map[i].iEditMode;
				pDoc  = map[i].pDocTmpl;
			}
		}

		if (pDoc)
		{
			error::code code = pDoc->OpenDocument(fDlg.GetPathName());

			if (code & error::ok)
			{
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
				m_view.Invalidate();
				m_view.SetFocus();
			}
			else if (code & error::wrong_format)
			{
				CString strErr = _T("Error");

				switch (iMode)
				{
					case ID_MODE_PSDL: strErr = "Not a PSDL file"; break;
					case ID_MODE_CPVS: strErr = "Not a CPVS file"; break;
				}

				MessageBox(strErr, LS(IDR_MAINFRAME), MB_ICONWARNING);
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
		else
		{
			MessageBox((CString) fDlg.GetPathName() + "\n\n" + LS(IDS_INVALID_FILE), LS(IDR_MAINFRAME), MB_ICONWARNING);
		}

	}

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
		if (m_psdlDoc.SaveDocument(fDlg.GetPathName()) == error::ok)
		{
			UpdateCaption();
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

	ShellExecute(m_hWnd, NULL, lpDir, NULL, NULL, SW_SHOWNORMAL);

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
