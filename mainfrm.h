#ifndef __MAINFRM_H__
#define __MAINFRM_H__

#include <string>

#include "resource.h"
#include "docview.h"
#include "psdl.h"
#include "psdl_doc.h"
#include "psdl_view.h"
#include "cpvs_doc.h"
#include "bai_doc.h"
#include "toolwnd.h"
#include "glview.h"
#include "histmgr.h"
#include "dialogs.h"
#include "psdl-editor.h"

#include "include/atldock.h"
#include "include/atldock2.h"

typedef struct
{
	const char* strExt;
	DocTemplateBase* pDocTmpl;
	int iEditMode;
}
extMap;

class CMainFrame;

class CMainFrame :
	public CFrameWindowImpl<CMainFrame>,
	public CUpdateUI<CMainFrame>,
	public CMessageFilter,
	public CIdleHandler,
	public CRecentDocumentList,
	public HistoryManager
{
public:

	CMainFrame() { this_ptr = this; }

	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	virtual BOOL PreTranslateMessage(MSG *pMsg);
	virtual BOOL OnIdle();

	BEGIN_UPDATE_UI_MAP(CMainFrame)
	#if !PD_SAVE_ALWAYS
		UPDATE_ELEMENT(ID_FILE_SAVE,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	#endif
	#if PD_ENABLE_HISTORY
		UPDATE_ELEMENT(ID_EDIT_UNDO,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_EDIT_REDO,			UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
	#endif
		UPDATE_ELEMENT(ID_EDIT_TRANSFORM,            UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR,              UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_WINDOWS_CITYBLOCKS,        UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_WINDOWS_PERIMETER,         UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_WINDOWS_ATTRIBUTES,        UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_WINDOWS_PROPERTIES,        UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_VIEW_AUTO_CAMERA,          UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_WIREFRAME,            UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_TEST_PVS,             UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_TEST_AI_CULLING,      UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_RENDER_PSDL,          UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_RENDER_BAI,           UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_STATUS_BAR,           UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_NUMERAL_HEX,          UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_VIEW_NUMERAL_DEC,          UPDUI_MENUPOPUP)
		UPDATE_ELEMENT(ID_MODE_PSDL,                 UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MODE_CPVS,                 UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MODE_INST,                 UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MODE_BAI,                  UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_MODE_PATHSET,              UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
		UPDATE_ELEMENT(ID_FILE_OPENCONTAININGFOLDER, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_PAINT_DESCENDANTS, OnPaintDescendants);

		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_RANGE_HANDLER(ID_FILE_NEW_PSDL, ID_FILE_NEW_WORKSPACE, OnFileNew)
		COMMAND_RANGE_HANDLER(ID_FILE_MRU_FIRST, ID_FILE_MRU_LAST, OnFileOpenRecent)
		COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
		COMMAND_ID_HANDLER(ID_FILE_OPENCONTAININGFOLDER, OnOpenContainingFolder)
		COMMAND_ID_HANDLER(ID_FILE_SAVE, OnFileSave)
		COMMAND_ID_HANDLER(ID_FILE_SAVE_AS, OnFileSaveAs)
		COMMAND_ID_HANDLER(ID_FILE_IMPORT, OnFileImport)
		COMMAND_ID_HANDLER(ID_FILE_EXPORT, OnFileExport)
		COMMAND_ID_HANDLER(ID_APP_EXIT,	OnFileExit)

		COMMAND_ID_HANDLER(ID_EDIT_TRANSFORM, OnEditTransform)
		COMMAND_ID_HANDLER(ID_EDIT_REVERSE, OnReverseVertices)

		COMMAND_ID_HANDLER(ID_VIEW_RESET_CAMERA, OnViewResetCamera)
		COMMAND_RANGE_HANDLER(ID_VIEW_AUTO_CAMERA, ID_VIEW_RENDER_BAI, OnViewSettingChange)

		COMMAND_ID_HANDLER(ID_VIEW_HIDE_ALL, OnViewHideAll)

		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_RANGE_HANDLER(ID_WINDOWS_CITYBLOCKS, ID_WINDOWS_BAI_CULLING, OnViewBar)
		COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
		COMMAND_RANGE_HANDLER(ID_VIEW_NUMERAL_HEX, ID_VIEW_NUMERAL_DEC, OnViewSetNumeral)
		COMMAND_RANGE_HANDLER(ID_MODE_PSDL, ID_MODE_PATHSET, OnSetEditingMode)

		COMMAND_ID_HANDLER(ID_INSERT_CITYBLOCK, OnInsertBlock)
		COMMAND_ID_HANDLER(ID_INSERT_DUPLICATE_BLOCKS, OnDuplicateBlock)

		COMMAND_RANGE_HANDLER(ID_INSERT_ROADSTRIP, ID_INSERT_ROOFTRIANGLEFAN, OnInsertAttribute)
		COMMAND_ID_HANDLER(ID_TOOLS_GENERATE_PERIMETERS, OnGeneratePerimeters)
		COMMAND_ID_HANDLER(ID_TOOLS_OPTIMIZE, OnOptimizePSDL)
		COMMAND_ID_HANDLER(ID_TOOLS_MM2, OnLaunchMM2)
		COMMAND_ID_HANDLER(ID_TOOLS_SDLVIEW, OnLoadSDLView)
		COMMAND_ID_HANDLER(ID_TOOLS_OPTIONS, OnOptions)
		COMMAND_ID_HANDLER(ID_TOOLS_GENERATE_BAI, OnGenerateBAI)

		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)

		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	LRESULT OnDestroy         (UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnCreate          (UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnPaintDescendants(UINT, WPARAM, LPARAM, BOOL&);

	LRESULT OnFileExit            (WORD, WORD, HWND, BOOL&);
	LRESULT OnFileNew             (WORD, WORD, HWND, BOOL&);
	LRESULT OnFileOpen            (WORD, WORD, HWND, BOOL&);
	LRESULT OnFileOpenRecent      (WORD, WORD, HWND, BOOL&);
	LRESULT OnOpenContainingFolder(WORD, WORD, HWND, BOOL&);
	LRESULT OnFileSave            (WORD, WORD, HWND, BOOL&);
	LRESULT OnFileSaveAs          (WORD, WORD, HWND, BOOL&);
	LRESULT OnFileImport          (WORD, WORD, HWND, BOOL&);
	LRESULT OnFileExport          (WORD, WORD, HWND, BOOL&);

	LRESULT OnEditTransform  (WORD, WORD, HWND, BOOL&);
	LRESULT OnReverseVertices(WORD, WORD, HWND, BOOL&);

	LRESULT OnViewResetCamera(WORD, WORD, HWND, BOOL&);
	LRESULT OnViewSettingChange(WORD, WORD, HWND, BOOL&);
	LRESULT OnViewToolBar    (WORD, WORD, HWND, BOOL&);
	LRESULT OnViewStatusBar  (WORD, WORD, HWND, BOOL&);
	LRESULT OnViewBar        (WORD, WORD, HWND, BOOL&);
	LRESULT OnViewHideAll    (WORD, WORD, HWND, BOOL&);
	LRESULT OnViewSetNumeral (WORD, WORD, HWND, BOOL&);
	LRESULT OnSetEditingMode (WORD, WORD, HWND, BOOL&);

	LRESULT OnInsertBlock       (WORD, WORD, HWND, BOOL&);
	LRESULT OnDuplicateBlock    (WORD, WORD, HWND, BOOL&);
	LRESULT OnInsertAttribute   (WORD, WORD, HWND, BOOL&);
	LRESULT OnGeneratePerimeters(WORD, WORD, HWND, BOOL&);
	LRESULT OnGenerateBAI       (WORD, WORD, HWND, BOOL&);

	LRESULT OnOptimizePSDL(WORD, WORD, HWND, BOOL&);
	LRESULT OnLoadSDLView (WORD, WORD, HWND, BOOL&);
	LRESULT OnLaunchMM2   (WORD, WORD, HWND, BOOL&);
	LRESULT OnOptions     (WORD, WORD, HWND, BOOL&);

	LRESULT OnAppAbout(WORD, WORD, HWND, BOOL&);

	int ShowOptions(int iPageID);

	static COpenGLView* GetView(void) { return &this_ptr->m_view; }
	static void PaintDescendants(void);
	void UpdateCaption(void);
	void SetEditingMode(int iMode);
	void SetNumeral(WORD wID);
	static void SelectBlock(long iIndex);
	static void SelectAttribute(psdl::block* pBlock, long iIndex);
	bool TransformEntities(TransformProps& sProps);

	static bool _TransformEntities(TransformProps& sProps) // Called by transform dialog
	{
		return this_ptr->TransformEntities(sProps);
	}

	DocTemplateBase* GetActiveDocument(void);
	BOOL OpenDocument(CString strDocName);
	BOOL CanSave(void);

	// HistoryManager
	CString GetUndoDescription(void);
	CString GetRedoDescription(void);

	static CMainFrame* this_ptr;

	static HWND GetWnd(void)
	{
		return this_ptr->m_hWnd;
	}

	static CBlocksWindow* BlocksWindow(void)
	{
		return &this_ptr->m_wndBlocks;
	}

	static PSDLDocTemplate m_psdlDoc;
private:

	int m_iEditMode;

	static CPVSDocTemplate m_cpvsDoc;
	static BAIDocTemplate  m_baiDoc;

	COpenGLView m_view;
	CFlatDockingWindow m_dock;

	CBlocksWindow		m_wndBlocks;
	CPerimeterWindow	m_wndPerimeter;
	CAttributesWindow	m_wndAttribs;
	CPropertiesWindow	m_wndProps;

	CTransformDlg		m_dlgTransform;

	static const extMap m_sExtMap[PD_NUM_EXTENSIONS];
};

#endif
