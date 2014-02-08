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
	if (g_properties.IsDialogMessage(pMsg))
		return TRUE;

	if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	return FALSE;
}

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();
	UIUpdateMenuBar(FALSE, TRUE);

	UISetText(ID_EDIT_UNDO, GetUndoDescription());
	UISetText(ID_EDIT_REDO, GetRedoDescription());
	UIEnable(ID_EDIT_UNDO, CanUndo());
	UIEnable(ID_EDIT_REDO, CanRedo());

	UISetCheck(ID_WINDOWS_CITYBLOCKS,	::IsWindowVisible(g_blocks));
	UISetCheck(ID_WINDOWS_ATTRIBUTES,	::IsWindowVisible(g_attributes));
	UISetCheck(ID_WINDOWS_PROPERTIES,	::IsWindowVisible(g_properties));

	return FALSE;
}

LRESULT CMainFrame::OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	bHandled = FALSE;
	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{
//	AddDocTemplate(&m_psdlDocTmp);

	CreateSimpleToolBar();
	CToolBarCtrl tool = m_hWndToolBar;
	tool.SetStyle(tool.GetStyle() | TBSTYLE_FLAT);

	CreateSimpleStatusBar();

	UIAddMenuBar(m_hWnd);
	UIAddToolBar(m_hWndToolBar);

//	UIEnable(ID_MODE_CPVS, 0);
//	UIEnable(ID_MODE_INST, 0);
//	UIEnable(ID_MODE_BAI, 0);
//	UIEnable(ID_MODE_PATHSET, 0);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE);

//	g_psdl = new PSDL();

	// Initiate docking framework
	HWND hwndDock = m_dock.Create(m_hWnd, rcDefault);
	m_dock.SetClient(m_hWndClient);

	g_properties.Create(m_hWnd,	rcDefault, _T("Attribute Properties"),	ATL_SIMPLE_DOCKVIEW_STYLE);
	g_attributes.Create(m_hWnd,	rcDefault, _T("Block Attributes"),		ATL_SIMPLE_DOCKVIEW_STYLE);
	g_blocks.Create(m_hWnd,		rcDefault, _T("City Blocks"),			ATL_SIMPLE_DOCKVIEW_STYLE);

	m_dock.AddWindow(g_properties);
	m_dock.AddWindow(g_attributes);
	m_dock.AddWindow(g_blocks);
	m_dock.DockWindow(g_properties,	DOCK_RIGHT);
	m_dock.DockWindow(g_attributes,	DOCK_RIGHT);
	m_dock.DockWindow(g_blocks,		DOCK_LEFT);

	m_dock.SetPaneSize(DOCK_LEFT, 200);
	m_dock.SetPaneSize(DOCK_RIGHT, 200);

	m_hWndClient = hwndDock;

	m_psdlDoc.SetViews(&g_blocks, &g_attributes, &g_properties);
	m_psdlDoc.NewDocument();
	SetEditingMode(ID_MODE_PSDL);

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

	m_view.SetDCPixelFormat(&pfd);
	m_view.SetFocus();

	glClearColor(0.f, 0.f, 0.f, 1.f);

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

//	RenderScene();

	m_psdlDoc.RenderScene(hDC, hRC);

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
		case ID_WINDOWS_CITYBLOCKS:	hWnd = g_blocks;		break;
		case ID_WINDOWS_ATTRIBUTES:	hWnd = g_attributes;	break;
		case ID_WINDOWS_PROPERTIES:	hWnd = g_properties;	break;
		default: return 0;
	}

	if (::IsWindowVisible(hWnd))
		m_dock.HideWindow(hWnd);
	else
		m_dock.DockWindow(hWnd, DOCK_LASTKNOWN);

	return 0;
}

void CMainFrame::SelectBlock(int iIndex)
{
	m_psdlDoc.SelectBlock(iIndex);
}

void CMainFrame::SetEditingMode(int iMode)
{
	UISetCheck(m_iEditMode, 0);
	UISetCheck(iMode, 1);

	m_iEditMode = iMode;

	DocTemplateBase* doc = NULL;

	switch (m_iEditMode)
	{
		case ID_MODE_PSDL:
			doc = &m_psdlDoc;
			break;
	}

	CString sTitle = _T("");

	if (doc)
		sTitle.Format(" - [%s]", doc->GetFileName().c_str());

	SetWindowText(LS(IDR_MAINFRAME) + sTitle);
}

LRESULT CMainFrame::OnSetEditingMode(WORD, WORD wID, HWND, BOOL&)
{
	SetEditingMode(wID);
	return 0;
}

typedef struct
{
	unsigned short from, to;
}
vertexMap;

void rotateVertex(Vertex *vTarget, Vertex vOrigin, double dAngle)
{
	vTarget->x = vOrigin.x + (vTarget->x - vOrigin.x) * cos(dAngle) - (vTarget->z - vOrigin.z) * sin(dAngle);
	vTarget->z = vOrigin.z + (vTarget->x - vOrigin.x) * sin(dAngle) + (vTarget->z - vOrigin.z) * cos(dAngle);
}

unsigned short copyVertex(vector<vertexMap> *aLookup, unsigned short nFrom, Vertex vOffset)
{
	long nTo = -1;

	for (size_t i = 0; i < aLookup->size(); i++)
	{
		if ((*aLookup)[i].from == nFrom)
		{
			nTo = (*aLookup)[i].to;
		}
	}
/*
	if (nTo < 0)
	{
		Vertex vCopy = g_psdl->getVertex(nFrom);

		Vertex vFixed = { -940.3245f, 0, 1349.235f };
		rotateVertex(&vCopy, vFixed, PI);

		vCopy.x += vOffset.x;
		vCopy.y += vOffset.y;
		vCopy.z += vOffset.z;

		nTo = g_psdl->addVertex(vCopy);

		vertexMap newMap = { nFrom, nTo };

		aLookup->push_back(newMap);
	}
*/
	return nTo;
}

LRESULT CMainFrame::OnInsertBlock(WORD, WORD wID, HWND, BOOL&)
{
	unsigned int nInsert = g_blocks.list()->GetCaretIndex();
	PSDL::Block block;

//	block.addPerimeterPoint(0);

	m_psdlDoc.InsertBlock(block, nInsert);
//	g_blocks.insertBlock(&block, nInsert);

	m_psdlDoc.m_bModified = true;
	AddHistoryState(wID);
	return 0;
}

LRESULT CMainFrame::OnDuplicateBlock(WORD, WORD wID, HWND, BOOL&)
{
/*	CDuplicateDlg dlg;

	if (IDOK == dlg.DoModal())
	{
		int nBlocks = g_blocks.list()->GetSelCount();
		int *aBlockIDs = new int[nBlocks];
		g_blocks.list()->GetSelItems(nBlocks, aBlockIDs);

		Vertex vOffset = { 1220.0787f, -20.2605f, -870.612f };

		vector<vertexMap> aDuplicates;

		for (size_t it = 0; it < nBlocks; it++)
		{
			unsigned long nIndex = aBlockIDs[it];

			PSDL::Block *origBlock = g_psdl->getBlock(nIndex);
			PSDL::Block *block = new PSDL::Block(*origBlock);

			if (g_duplicateProps.bVertices)
			{
				size_t k;

				for (k = 0; k < block->numPerimeterPoints(); k++)
				{
					unsigned short nVertexRef = block->getPerimeterPoint(k)->vertex;

					block->setPerimeterPoint(
						k, copyVertex(&aDuplicates, nVertexRef, vOffset), 0
					);
				}

				int j = 0;

				for (size_t i = 0; i < block->numAttributes(); i++)
				{
					block->m_attributes[i] = origBlock->getAttribute(i)->clone();
					PSDL::Attribute *atb = block->getAttribute(i);
					j++;

					unsigned char type = atb->type();
					switch (type)
					{
						case ATB_ROAD:
						case ATB_SIDEWALK:
						case ATB_RECTANGLE:
						case ATB_ROADTRIANGLEFAN:
						case ATB_TRIANGLEFAN:
						case ATB_DIVIDEDROAD:
						case ATB_ROOFTRIANGLEFAN:
							for (k = 0; k < static_cast<PSDL::RoadStrip*>(atb)->numVertices(); k++)
							{
								unsigned short nVertexRef = static_cast<PSDL::RoadStrip*>(atb)->getVertexRef(k);

								static_cast<PSDL::RoadStrip*>(atb)->setVertexRef(
									k, copyVertex(&aDuplicates, nVertexRef, vOffset)
								);
							}
							break;

						case ATB_CROSSWALK:
							for (k = 0; k < 4; k++)
							{
								unsigned short nVertexRef = static_cast<PSDL::Crosswalk*>(atb)->getVertexRef(k);

								static_cast<PSDL::Crosswalk*>(atb)->setVertexRef(
									k, copyVertex(&aDuplicates, nVertexRef, vOffset)
								);
							}
							break;

						case ATB_SLIVER:
						case ATB_FACADEBOUND:
						case ATB_FACADE:
							// TODO!!
						//	break;

						default:
							j--;
					}
				}

				ATLTRACE("Block %x copied; vertices copied for %d attributes\n", nIndex, j);
			}

			for (unsigned char i = 0; i < g_duplicateProps.nCount; i++)
			{
				g_psdl->addBlock(*block);
				g_blocks.insertBlock(block, -1);
			}
		}
	}

	g_psdl->m_bModified = true;
	AddHistoryState(wID);*/
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
				PSDL::Block *block = g_psdl->getBlock(i);

				vector<PSDL::Attribute*>::iterator it = block->m_attributes.begin();

				while (it != block->m_attributes.end())
				{
					switch ((*it)->type())
					{
						case ATB_TEXTURE:
							nTexRef = static_cast<PSDL::Texture*>(*it)->textureRef;

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
					PSDL::Texture *lastTex = NULL;

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
							lastTex = static_cast<PSDL::Texture*>(*it);
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
			PSDL::Attribute *atb;
			PSDL::Texture *tex = 0;

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
	if (wID == ID_FILE_NEW)
	{
		switch (m_iEditMode)
		{
			case ID_MODE_PSDL:	wID = ID_FILE_NEW_PSDL;
		}
	}

	switch (wID)
	{
		case ID_FILE_NEW_PSDL:
			m_psdlDoc.NewDocument();
			SetEditingMode(ID_MODE_PSDL);
			break;
	}

	m_view.Invalidate();
	return 0;
}

LRESULT CMainFrame::OnFileOpen(WORD, WORD, HWND, BOOL&)
{
	if (m_psdlDoc.m_bModified)
	{
		CString sText;
		sText.Format(LS(IDS_SAVE_CHANGES), m_psdlDoc.GetFileName().c_str());

		int ret = MessageBox(sText, LS(IDR_MAINFRAME), MB_YESNOCANCEL | MB_ICONQUESTION);

		if (ret == IDCANCEL)
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
		char* sExt = fDlg.GetFileExt();
		ATLTRACE("%s", sExt);

		if (!strcmpi(sExt, "psdl") || !strcmpi(sExt, "psd"))
		{
/*			PSDL psdl(fDlg.GetPathName());

			if (psdl.ReadFile(fDlg.GetPathName()))
			{
				g_psdl = new PSDL(psdl);
				g_blocks.SetPSDL(g_psdl);
				g_blocks.list()->SetFocus();
				g_attributes.SetBlock(NULL);
				g_properties.SetAttribute(NULL);
				SetEditingMode(ID_MODE_PSDL);*/

			if (m_psdlDoc.OpenDocument(fDlg.GetPathName()) == error::ok)
			{
				SetEditingMode(ID_MODE_PSDL);
				m_view.Invalidate();
				m_view.SetFocus();
			}
			else
			{
				MessageBox(_T("Not a PSDL file"), NULL, MB_ICONERROR);
			}
		}
	}

	g_options.files.browseDir = fDlg.GetFolderPath();

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

LRESULT CMainFrame::OnFileSaveAs(WORD, WORD, HWND, BOOL&)
{
/*	CString sSelectedFile;

	CCenterFileDialog fDlg(FALSE, _T("psdl"), NULL, OFN_HIDEREADONLY, _T("\
		MM2 City Geometry (*.psdl; *.psd)\0*.psdl; *.psd\0")
	);

	if (IDOK == fDlg.DoModal()) {

		if (!g_psdl->WriteFile(fDlg.m_szFileName))
			MessageBox(_T("File writing error"), NULL, MB_ICONERROR);
	}
*/
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
