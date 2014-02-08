#ifndef __TOOLWND_H__
#define __TOOLWND_H__

#include "resource.h"
#include "atbview.h"
#include "strings.h"

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
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		CHAIN_MSG_MAP(CFrameWindowImpl<CPropertiesWindow>)
	END_MSG_MAP()

	CPropertiesWindow() : m_atb(NULL), m_view(NULL) {}

	void SetView(int nID)
	{
		if (m_view)
		{
			m_view->DoDataExchange(TRUE);
			m_view->DestroyWindow();
			delete m_view;
		}

		switch (nID)
		{
			case ATB_ROAD:
			case ATB_DIVIDEDROAD:	m_view = new CRoadView(m_atb); break;
			case ATB_TUNNEL:		m_view = new CTunnelView(m_atb); break;
			case ATB_TEXTURE:		m_view = new CTextureView(m_atb); break;
			case -1:				m_view = new CAtbView<IDD_ATB_NONE>; break;
			default:				m_view = new CAtbView<IDD_ATB_UNKNOWN>;
		}

		m_hWndClient = m_view->Create(m_hWnd);
		m_view->DoDataExchange();
		UpdateLayout();
	}

	void SetAttribute(PSDL::Attribute *atb)
	{
		m_atb = atb;

		if (m_atb)
			SetView(m_atb->type());
		else
			SetView(-1);
	}

	PSDL::Attribute* GetAttribute(void)
	{
		return m_atb;
	}

	LRESULT OnCreate(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		SetView(-1);
		return 0;
	}

private:

	PSDL::Attribute *m_atb;
	CPropView *m_view;
};

class CAttributesWindow : public CWindowImpl<CAttributesWindow>
{
	CMyListBox m_list;
//	PSDL::Block *m_block;

	BEGIN_MSG_MAP(CAttributesWindow)
		COMMAND_HANDLER(IDC_LIST, LBN_SELCHANGE, OnSelChange);
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SIZING, OnSizing)
		NOTIFY_CODE_HANDLER(NM_SETFOCUS, OnSetFocus)
	END_MSG_MAP()

	void SetBlock(PSDL::Block *block);

	LRESULT OnSelChange(WORD, WORD, HWND hWnd, BOOL&);
	LRESULT OnSetFocus(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	LRESULT OnCreate(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		m_list.Create(m_hWnd, rcDefault, IDC_LIST);
		m_list.SetTabStops(20);
		return 0;
	}

	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&)
	{
		CRect rc;
		GetClientRect(&rc);
		m_list.SetWindowPos(NULL, 0, 0, rc.right, rc.bottom, SWP_NOMOVE);
		return 0;
	}

	LRESULT OnSizing(UINT, WPARAM, LPARAM, BOOL&)
	{
		return 0;
	}
};

class CBlocksWindow : public CWindowImpl<CBlocksWindow>
{
	BEGIN_MSG_MAP(CBlocksWindow)
		COMMAND_HANDLER(IDC_LIST, LBN_SELCHANGE, OnSelChange);
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	void InsertBlock(PSDL::Block *pBlock, int nPos)
	{
		int nID;
		CString sText, sType;

		switch (pBlock->getType())
		{
			case BIT_SUBTERANEAN:	nID = IDS_SUBTERANEAN;	break;
			case BIT_PLAIN:			nID = IDS_PLAIN;		break;
			case BIT_ROAD:			nID = IDS_ROAD;			break;
			case BIT_INTERSECTION:	nID = IDS_INTERSECTION;	break;
			case BIT_INST:			nID = IDS_INST;			break;
			case BIT_UNKNOWN1:
			case BIT_UNKNOWN2:
			case BIT_UNKNOWN3:		nID = IDS_UNKNOWN;		break;
			default:				nID = IDS_MULTIPLE;
		}
		sType.LoadString(nID);

		sText.Format("%x\t", nPos >= 0 ? nPos : m_list.GetCount());
		m_list.InsertString(nPos, sText + sType);
	}

	void SetPSDL(PSDL *psdl) {
//		m_psdl = psdl;

		CString sText, sType;

		m_list.SetRedraw(FALSE);
		m_list.ResetContent();
		for (size_t i = 0; i < psdl->numBlocks(); i++)
		{
			InsertBlock(psdl->getBlock(i), -1);
		}
		m_list.SetRedraw();
	}

//	PSDL *getPSDL(void) {
//		return m_psdl;
//	}

	CMyListBox *list(void)
	{
		return &m_list;
	}

	LRESULT OnSelChange(WORD, WORD, HWND hWnd, BOOL&);

	LRESULT OnCreate(UINT, WPARAM, LPARAM lParam, BOOL&)
	{
		m_list.Create(m_hWnd, rcDefault, IDC_LIST);
		m_list.SetTabStops(20);
		return 0;
	}

	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&)
	{
		CRect rc;
		GetClientRect(&rc);
		m_list.SetWindowPos(NULL, 0, 0, rc.right, rc.bottom, SWP_NOMOVE);
		return 0;
	}

private:
	CMyListBox m_list;
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

extern CPropertiesWindow	g_properties;
extern CAttributesWindow	g_attributes;
extern CBlocksWindow		g_blocks;

#endif
