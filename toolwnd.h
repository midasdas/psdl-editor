#ifndef __TOOLWND_H__
#define __TOOLWND_H__

#include "resource.h"
#include "atbview.h"
#include "strings.h"
#include "tools.h"
#include "dialogs.h"

const WORD wBullet[] = { 0x009f, 0x000f, 0x000f, 0x009f };

static inline unsigned char CalcLZeros(int nItems)
{
	unsigned char nDigits = 0;
	nItems--;
	while (nItems > 0) nItems /= 0x10, nDigits++;
	return nDigits;
}

class CToolWindow : public CWindowImpl<CToolWindow>
{
	public:
		BEGIN_MSG_MAP(CToolWindow)
			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

		LRESULT OnEraseBkgnd(UINT, WPARAM wParam, LPARAM, BOOL&)
		{
			CRect rc;
			GetClientRect(&rc);
			FillRect((HDC) wParam, &rc, (HBRUSH) (COLOR_BTNFACE));
			return 1;
		}
};

class CPropertiesWindow : public CFrameWindowImpl<CPropertiesWindow>
{
public:

	BEGIN_MSG_MAP(CPropertiesWindow)
	//	MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CFrameWindowImpl<CPropertiesWindow>)
	END_MSG_MAP()

	CPropertiesWindow() : m_pDoc(NULL), m_pAtb(NULL), m_pView(NULL) {}

	void SetView(int nID)
	{
		if (m_pView && m_pView->IsWindow())
		{
			m_pView->DoDataExchange(TRUE);
			m_pView->ShowWindow(SW_HIDE);
			m_pView->DestroyWindow();
		}

		switch (nID)
		{
			case ATB_ROAD:
			case ATB_DIVIDEDROAD: m_pView = new CRoadView(m_pAtb); break;
			case ATB_TUNNEL:      m_pView = new CTunnelView(m_pAtb); break;
			case ATB_TEXTURE:     m_pView = new CTextureView(m_pAtb, m_pDoc); break;
			case -1:              m_pView = new CAtbView<IDD_ATB_NONE>; break;
			default:              m_pView = new CAtbView<IDD_ATB_UNKNOWN>;
		}

		m_hWndClient = m_pView->Create(m_hWnd);
		m_pView->DoDataExchange();
		UpdateLayout();
	}

	void SetPSDL(psdl* pDoc)
	{
		m_pDoc = pDoc;
	}

	void SetAttribute(psdl::attribute* pAtb)
	{
		m_pAtb = pAtb;

		if (m_pDoc && m_pAtb)
			SetView(m_pAtb->type);
		else
			SetView(-1);
	}

	psdl::attribute* GetAttribute(void)
	{
		return m_pAtb;
	}

	LRESULT OnCreate(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		SetView(-1);
		return 0;
	}

private:

	psdl* m_pDoc;
	psdl::attribute* m_pAtb;
	CPropView* m_pView;
};

class CAttributesWindow : public CWindowImpl<CAttributesWindow>
{
public:

	CAttributesWindow() : m_nDigits(0) {}

private:

	BEGIN_MSG_MAP_EX(CAttributesWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		MSG_WM_DRAWITEM(OnDrawItem)
		MSG_WM_MEASUREITEM(OnMeasureItem)
		NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnItemChanged)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	void SetBlock(psdl::block* pBlock);
	psdl::block* GetBlock(void) { return m_pBlock; }

	void OnContextMenu(CWindow wnd, CPoint pt);
	void OnDrawItem(UINT, LPDRAWITEMSTRUCT lpdis);
	void OnMeasureItem(UINT, LPMEASUREITEMSTRUCT lpmis);
	LRESULT OnItemChanged(int, LPNMHDR, BOOL&);

	LRESULT OnCreate(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		m_list.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE |
			LVS_OWNERDRAWFIXED | LVS_REPORT | LVS_NOSORTHEADER, WS_EX_STATICEDGE, IDC_LIST);

		m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

		m_list.InsertColumn(0, _T("#"));
		m_list.InsertColumn(1, _T(""));
		m_list.InsertColumn(2, _T("Attribute"));

		m_list.SetColumnWidth(0, 20);
		m_list.SetColumnWidth(1, 20);// 15

		m_headerCtrl.SubclassWindow(m_list.GetHeader());
		return 0;
	}

	void AdjustColumns(void)
	{
		RECT rc;
		m_list.GetClientRect(&rc);
		m_list.SetColumnWidth(2, rc.right - 42);
	}

	LRESULT OnSize(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		RECT rc;
		m_list.SetWindowPos(NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOMOVE);
		m_list.GetClientRect(&rc);
		AdjustColumns();
		return FALSE;
	}

	unsigned char m_nDigits;
	CLayersHeaderCtrl m_headerCtrl;
	psdl::block* m_pBlock;

public:
	CPDListViewCtrl m_list;
};

class CPerimeterWindow : public CWindowImpl<CPerimeterWindow>
{
public:

	CPerimeterWindow() : m_nDigits(0) {}

private:

	BEGIN_MSG_MAP_EX(CPerimeterWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MSG_WM_DRAWITEM(OnDrawItem)
		MSG_WM_MEASUREITEM(OnMeasureItem)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	void SetBlock(psdl::block* pBlock);
	void InsertItems(int nItems)
	{
		int nBegin = m_list.GetItemCount();
		
		m_list.SetRedraw(FALSE);
		for (int i = 0; i < nItems; i++)
			m_list.InsertItem(nBegin + i, _T(""));

		m_list.SetRedraw();
		AdjustColumns();
	}

	void OnDrawItem(UINT, LPDRAWITEMSTRUCT lpdis);
	void OnMeasureItem(UINT, LPMEASUREITEMSTRUCT lpmis);

	LRESULT OnCreate(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		m_list.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE |
			LVS_OWNERDRAWFIXED | LVS_REPORT | LVS_NOSORTHEADER, WS_EX_STATICEDGE, IDC_LIST);

		m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

		m_list.InsertColumn(0, _T("#"));
		m_list.InsertColumn(1, _T("Vertex, block"));

		m_list.SetColumnWidth(0, 30);

		m_headerCtrl.SubclassWindow(m_list.GetHeader());
		return 0;
	}

	void AdjustColumns(void)
	{
		RECT rc;
		m_list.GetClientRect(&rc);
		m_list.SetColumnWidth(1, rc.right - 32);
	}

	LRESULT OnSize(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		m_list.SetWindowPos(NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOMOVE);
		AdjustColumns();
		return 0;
	}

	CPDListViewCtrl m_list;
	unsigned char m_nDigits;
	CLayersHeaderCtrl m_headerCtrl;
	psdl::block* m_pBlock;
};

class CBlocksWindow : public CWindowImpl<CBlocksWindow>
{
public:

	CBlocksWindow() : m_pDoc(NULL), m_nDigits(0) {}

private:

	BEGIN_MSG_MAP_EX(CBlocksWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		MSG_WM_DRAWITEM(OnDrawItem)
		MSG_WM_MEASUREITEM(OnMeasureItem)
		NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnItemChanged)
		NOTIFY_HANDLER(IDC_LIST, NM_CLICK, OnClick)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	void OnDrawItem(UINT, LPDRAWITEMSTRUCT lpdis);
	void OnMeasureItem(UINT, LPMEASUREITEMSTRUCT lpmis);
	LRESULT OnItemChanged(int, LPNMHDR, BOOL&);
	LRESULT OnClick(int, LPNMHDR lpnmh, BOOL&);

	void OnContextMenu(CWindow wnd, CPoint pt);

	void InsertItem(int iPos = -1)
	{
		if (iPos < 0) iPos = m_list.GetItemCount();
		m_list.InsertItem(iPos, _T(""));
	//	m_nDigits = CalcLZeros(m_list.GetItemCount());
		m_list.Invalidate();
		AdjustColumns();
	}

	std::vector<psdl::block*> GetSelected(void)
	{
		std::vector<psdl::block*> blocks;
		int i = -1;

		while ((i = m_list.GetNextItem(i, LVNI_SELECTED)) >= 0)
			blocks.push_back(m_pDoc->get_block(i));

		return blocks;
	}

	void SetPSDL(psdl* pDoc)
	{
		m_pDoc = pDoc;

		m_list.SetRedraw(FALSE);
		m_list.DeleteAllItems();

		for (int i = 0; i < pDoc->num_blocks(); ++i)
			m_list.InsertItem(i, _T(""));

	//	m_nDigits = CalcLZeros(i);

		m_list.SetRedraw();
		AdjustColumns();
	}

	LRESULT OnCreate(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		m_list.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE |
			LVS_OWNERDRAWFIXED | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOSORTHEADER,
			WS_EX_STATICEDGE, IDC_LIST);

		m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

		m_list.InsertColumn(0, _T("#"));
		m_list.InsertColumn(1, _T(""));
		m_list.InsertColumn(2, _T("Type"));
		m_list.InsertColumn(3, _T("Proprule"));

		m_list.SetColumnWidth(0, 30);
		m_list.SetColumnWidth(1, 35);// 30
		m_list.SetColumnWidth(2, 83);// 78

		m_headerCtrl.SubclassWindow(m_list.GetHeader());

		CBitmap bm;
		bm.LoadBitmap(IDB_FLASH4);

		m_ImageList.Create(15, 13,  ILC_COLOR8 | ILC_MASK, 1, 1);
		m_ImageList.SetBkColor(CLR_NONE);
		m_ImageList.Add(bm, RGB(255, 0, 255));

		return FALSE;
	}

	void AdjustColumns(void)
	{
		RECT rc;
		m_list.GetClientRect(&rc);
		m_list.SetColumnWidth(3, rc.right - 150);
	}

	LRESULT OnSize(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		m_list.SetWindowPos(NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOMOVE);
		AdjustColumns();
		return FALSE;
	}

	psdl* m_pDoc;
	unsigned char m_nDigits;
	CImageList m_ImageList;

	CLayersCtrl m_list;
	CLayersHeaderCtrl m_headerCtrl;
};

class CToolbox : public CFrameWindowImpl<CToolbox>
{
	public:
		DECLARE_FRAME_WND_CLASS(NULL, IDR_TOOLBOX)

		BEGIN_MSG_MAP(CToolbox)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
			CHAIN_MSG_MAP(CFrameWindowImpl<CToolbox>)
		END_MSG_MAP()

		LRESULT OnCreate(UINT, WPARAM, LPARAM lParam, BOOL&)
		{
			CreateSimpleToolBar();
			CToolBarCtrl tool = m_hWndToolBar;
			tool.SetStyle(tool.GetStyle() | TBSTYLE_FLAT | TBSTYLE_WRAPABLE);
			return 0;
		}

		LRESULT OnSize(UINT, WPARAM, LPARAM lParam, BOOL&)
		{
			return 0;
		}

		LRESULT OnEraseBkgnd(UINT, WPARAM wParam, LPARAM, BOOL&)
		{
			RECT rc;
			GetClientRect(&rc);
			FillRect((HDC) wParam, &rc, (HBRUSH) (COLOR_WINDOW));
			return 1;
		}
};

#endif
