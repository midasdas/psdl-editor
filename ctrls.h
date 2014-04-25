#ifndef __CTRLS_H__
#define __CTRLS_H__

#include "tools.h"
#include "resource.h"
#include <math.h>

class CCheckButton : public CWindowImpl<CCheckButton>
{
public:

	CCheckButton(bool bChecked = false) : m_bChecked(bChecked) {}

	HWND Create(HWND hWndParent, RECT& rcPos = rcDefault, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = WS_CHILD | WS_VISIBLE, DWORD dwExStyle = 0,
			UINT nID = 0, LPVOID lpCreateParam = NULL)
	{
		return CWindowImpl<CCheckButton>::Create(hWndParent, rcPos, szWindowName, dwStyle, dwExStyle, nID, lpCreateParam);
	}

	static LPCTSTR GetWndClassName()
	{
		return _T("FLASH 4 BUTTON");
	}

	int GetCheck() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return m_bChecked ? BST_CHECKED : BST_UNCHECKED;
	}

	void SetCheck(int nCheck)
	{
		SetCheck(BST_CHECKED == nCheck);
	}

	void SetCheck(bool bCheck)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		m_bChecked = bCheck;
		::RedrawWindow(GetParent(), NULL, NULL, RDW_INVALIDATE);
	}

	void SetStateImages(CImageList* pImageList, int nImage1, int nImage2)
	{
		ATLASSERT(!::IsWindow(m_hWnd));
		m_pImageList = pImageList;
		m_nImage1 = nImage1;
		m_nImage2 = nImage2;
	}

private:

	BEGIN_MSG_MAP_EX(CCheckButton)
		MSG_WM_ERASEBKGND(OnEraseBkgnd);
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDown)
	END_MSG_MAP()

	LRESULT OnEraseBkgnd(HDC hDC)
	{
		ATLASSERT(m_pImageList);
		m_pImageList->Draw(hDC, m_bChecked ? m_nImage1 : m_nImage2, 0, 2, ILD_NORMAL);
		return 0;
	}

	LRESULT OnLButtonDown(UINT, WPARAM, LPARAM, BOOL&)
	{
		SetCheck(!m_bChecked);
		return 0;
	}

	bool m_bChecked;
	int m_nImage1, m_nImage2;
	CImageList* m_pImageList;
};

class CLayersHeaderCtrl : public CWindowImpl<CLayersHeaderCtrl, CHeaderCtrl>
{
	BEGIN_MSG_MAP_EX(CLayersHeaderCtrl)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MSG_WM_PAINT(OnPaint)
		MESSAGE_HANDLER(HDM_LAYOUT, OnLayout)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	END_MSG_MAP()

	LRESULT OnPaint(HDC hDC)
	{
		PAINTSTRUCT ps;
		hDC = BeginPaint(&ps);

		NONCLIENTMETRICS ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICS);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);

		HFONT hFont = CreateFontIndirect(&ncm.lfMessageFont);
		HFONT hOldFont = (HFONT) SelectObject(hDC, hFont);

		SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));

		TEXTMETRIC tm;
		GetTextMetrics(hDC, &tm);

		RECT rcItem;

		for (int iItem = 0; iItem < GetItemCount(); iItem++)
		{
			TCHAR str[16];

			HDITEM hdi;
			hdi.mask = HDI_TEXT;
			hdi.pszText = str;
			hdi.cchTextMax = 16;

			GetItem(iItem, &hdi);
			GetItemRect(iItem, &rcItem);
			rcItem.bottom--;

			int y = (rcItem.top + rcItem.bottom - tm.tmHeight) / 2;

			ExtTextOut(hDC, rcItem.left + 2, y, ETO_CLIPPED | ETO_OPAQUE, &rcItem, str, _tcslen(str), NULL);
		}

		// Fill right remainder
		RECT rc;
		GetClientRect(&rc);
		rc.left = rcItem.right;
		rc.bottom--;
		FillRect(hDC, &rc, GetSysColorBrush(COLOR_BTNFACE));

		// Bottom edge
		rc.left = 0;
		rc.top = rc.bottom++;
		FillRect(hDC, &rc, GetSysColorBrush(COLOR_BTNHILIGHT));

		SelectObject(hDC, hOldFont);
		DeleteObject(hFont);

		EndPaint(&ps);
		return FALSE;
	}

	LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&)
	{
		return TRUE;
	}

	LRESULT OnLayout(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
	{
		LRESULT lResult = DefWindowProc(uMsg, wParam, lParam);

		HDLAYOUT* pHL = (HDLAYOUT*) lParam;

		pHL->pwpos->cy = 18;
		pHL->pwpos->cx += GetSystemMetrics(SM_CXBORDER) * 2;
		pHL->prc->top = 17;

		return lResult;
	}

	LRESULT OnSetCursor(UINT, WPARAM, LPARAM, BOOL&)
	{
		return TRUE;
	}

//	CCheckButton m_btnShowAll, m_btnLockAll;
	CImageList m_ImageList;

public:

	BOOL SubclassWindow(HWND hWnd)
	{
		BOOL bRet = CWindowImpl<CLayersHeaderCtrl, CHeaderCtrl>::SubclassWindow(hWnd);

		if (bRet == TRUE)
		{
			CBitmap bm;
			bm.LoadBitmap(IDB_FLASH4);

			m_ImageList.Create(15, 13,  ILC_COLOR8 | ILC_MASK, 1, 1);
			m_ImageList.SetBkColor(CLR_NONE);
			m_ImageList.Add(HBITMAP(bm), RGB(255, 0, 255));

		/*	m_btnShowAll.SetStateImages(&m_ImageList, 0, 0);
			m_btnLockAll.SetStateImages(&m_ImageList, 1, 1);
			RECT rcBtn = { 30, 0, 45, 19 };
			m_btnShowAll.Create(m_hWnd, rcBtn);
			rcBtn.left += 15;
			rcBtn.right += 15;
			m_btnLockAll.Create(m_hWnd, rcBtn);*/
		}

		return bRet;
	}
};

template <class T>
class CPDListViewCtrlBase : public CWindowImpl<T, CListViewCtrl>
{
	BEGIN_MSG_MAP_EX(CPDListViewCtrlBase)
		MSG_WM_CONTEXTMENU(OnContextMenu)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MSG_WM_PAINT(OnPaint)
		NOTIFY_CODE_HANDLER(HDN_BEGINTRACKA, OnHeaderSizing)
		NOTIFY_CODE_HANDLER(HDN_BEGINTRACKW, OnHeaderSizing)
		NOTIFY_CODE_HANDLER(HDN_DIVIDERDBLCLICKA, OnHeaderSizing)
		NOTIFY_CODE_HANDLER(HDN_DIVIDERDBLCLICKW, OnHeaderSizing)
	END_MSG_MAP()

	void OnContextMenu(CWindow wnd, CPoint pt)
	{
		if (pt.x == -1 && pt.y == -1)
		{
			int iItem = GetNextItem(-1, LVNI_FOCUSED);
			if (iItem >= 0)
			{
				CRect rc;
				GetItemRect(iItem, &rc, LVIR_BOUNDS);
				pt.x = rc.left;
				pt.y = rc.bottom;
			}
			else
				pt.x = pt.y = 0;

			ClientToScreen(&pt);
		}
		else
			pt.x++;

		SendMessage(GetParent(), WM_CONTEXTMENU, (WPARAM) m_hWnd, MAKELPARAM(pt.x, pt.y));
	}

	LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&)
	{
		return TRUE;
	}

	LRESULT OnPaint(HDC hDC)
	{
		CRect rc;
		GetClientRect(&rc);

		PAINTSTRUCT ps;
		hDC = BeginPaint(&ps);

		CDC dcMem;
		dcMem.CreateCompatibleDC(hDC);

		CBitmap BmMem;
		BmMem.CreateCompatibleBitmap(hDC, rc.Width(), rc.Height());
		CBitmap pBmOld = dcMem.SelectBitmap(BmMem);

	//	SetBkColor(RGB_AVERAGE(GetSysColor(COLOR_BTNFACE), GetSysColor(COLOR_BTNHILIGHT)));
		dcMem.FillSolidRect(rc, GetBkColor());

		DefWindowProc(WM_PAINT, (WPARAM) dcMem.m_hDC, 0);

		BitBlt(hDC, 0, 18, rc.Width(), rc.Height(), dcMem, 0, 18, SRCCOPY);
		dcMem.SelectBitmap(pBmOld);

		EndPaint(&ps);

	//	GetHeader().Invalidate();

		return FALSE;
	}

	LRESULT OnHeaderSizing(int, LPNMHDR, BOOL&)
	{
		return TRUE;// Prevent tracking of dividers
	}
};

class CPDListViewCtrl : public CPDListViewCtrlBase<CPDListViewCtrl>
{
};

class CNumListViewCtrl : public CWindowImpl<CNumListViewCtrl, CListViewCtrl>
{
	BEGIN_MSG_MAP_EX(CNumListViewCtrl)
	//	MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	//	MSG_WM_LBUTTONDOWN(OnLButtonDown)
	//	MSG_WM_LBUTTONDBLCLK(OnLButtonDown)
	//	NOTIFY_CODE_HANDLER(HDN_BEGINTRACKA, OnBeginTrack)
	//	NOTIFY_CODE_HANDLER(HDN_BEGINTRACKW, OnBeginTrack)
	END_MSG_MAP()

	LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&)
	{
		return TRUE;
	}
};

class CLayersCtrl : public CPDListViewCtrlBase<CLayersCtrl>
{
	BEGIN_MSG_MAP_EX(CLayersCtrl)
		MSG_WM_NCPAINT(OnNCPaint)
		MSG_WM_LBUTTONDOWN(OnLButtonDown)
		MSG_WM_LBUTTONDBLCLK(OnLButtonDown)
	//	REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnRClick)
		CHAIN_MSG_MAP(CPDListViewCtrlBase<CLayersCtrl>)
	//	CHAIN_MSG_MAP(CMainFrame)
	END_MSG_MAP()

	LRESULT OnRClick(int, LPNMHDR lpnmh, BOOL&)
	{
		NMITEMACTIVATE* nmia = (NMITEMACTIVATE*) lpnmh;
		ClientToScreen(&nmia->ptAction);
		CMenu menu;
		menu.LoadMenu(IDR_POPUP_BLOCK);
		menu.GetSubMenu(0).TrackPopupMenu(TPM_LEFTALIGN, nmia->ptAction.x + 1, nmia->ptAction.y, m_hWnd);
		return TRUE;
	}

	LRESULT OnNCPaint(CRgn rgn)
	{
		SetBkColor(RGB_AVERAGE(GetSysColor(COLOR_BTNFACE), GetSysColor(COLOR_BTNHILIGHT)));
		SetMsgHandled(FALSE);
		return FALSE;
	}

	void OnLButtonDown(UINT nFlags, CPoint point)
	{
		BOOL bHandled = TRUE;

		if (nFlags & MK_CONTROL || nFlags & MK_SHIFT || point.x <= GetColumnWidth(0))
			bHandled = FALSE;
//		else
//		{
			RECT rc;
			GetItemRect(GetItemCount() - 1, &rc, LVIR_BOUNDS);
			if (point.y > rc.bottom)
				bHandled = FALSE;

			if (bHandled == TRUE)
			{
				SetFocus();

				LVHITTESTINFO hti;
				hti.pt = point;
				int iItem = SubItemHitTest(&hti);

				if (iItem >= 0)
				{
					NMITEMACTIVATE nmia = { 0 };
					nmia.hdr.code = NM_CLICK;
					nmia.hdr.hwndFrom = m_hWnd;
					nmia.hdr.idFrom = IDC_LIST;
					nmia.iItem = iItem;
					nmia.iSubItem = hti.iSubItem;
					nmia.ptAction = point;
					::SendMessage(GetParent(), WM_NOTIFY, IDC_LIST, (LPARAM) &nmia);
				}
			}
	//	}
		SetMsgHandled(bHandled);
	}
};

/*
class CLayersCtrl : public CWindowImpl<CLayersCtrl, CListViewCtrl>
{
public:

	typedef CWindowImpl<CLayersCtrl, CListViewCtrl> baseClass;

	BEGIN_MSG_MAP_EX(CLayersCtrl)
		MSG_OCM_MEASUREITEM(OnMeasureItem)
		MSG_OCM_DRAWITEM(OnDrawItem)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	BOOL SubclassWindow(HWND hWnd)
	{
		BOOL bRet = baseClass::SubclassWindow(hWnd);
		ModifyStyle(0, LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDRAWFIXED);
		SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
		SetBkColor(GetSysColor(COLOR_BTNFACE) + RGB(22, 24, 28));
		return bRet;
	}

	void OnMeasureItem(UINT, LPMEASUREITEMSTRUCT lpmis)
	{
		lpmis->itemHeight = 18;
	}

	void OnDrawItem(UINT, LPDRAWITEMSTRUCT lpdis)
	{
		CDC dc(lpdis->hDC);

		dc.SetBkColor  (GetSysColor(lpdis->itemState & ODS_SELECTED ? COLOR_3DDKSHADOW    : COLOR_BTNFACE));
		dc.SetTextColor(GetSysColor(lpdis->itemState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT));

		TEXTMETRIC tm;
		GetTextMetrics(dc, &tm);
		int y = (lpdis->rcItem.bottom + lpdis->rcItem.top - tm.tmHeight) / 2;

		CString strText;
		RECT rc;

		strText.Format("%03x", lpdis->itemID);
		dc.ExtTextOut(3, y, ETO_CLIPPED | ETO_OPAQUE, &lpdis->rcItem, strText, strText.GetLength(), NULL);

		GetSubItemRect(lpdis->itemID, 1, LVIR_BOUNDS, &rc);
		GetItemText(lpdis->itemID, 1, strText);
		dc.ExtTextOut(rc.left + 3, y, ETO_CLIPPED, NULL, strText, strText.GetLength(), NULL);

		dc.MoveTo(lpdis->rcItem.left, lpdis->rcItem.bottom, NULL);

		dc.SelectPen(CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNHIGHLIGHT)));
		dc.LineTo(lpdis->rcItem.left, lpdis->rcItem.top);
		dc.LineTo(lpdis->rcItem.right - 1, lpdis->rcItem.top);

		dc.SelectPen(CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW)));
		dc.LineTo(lpdis->rcItem.right - 1, lpdis->rcItem.bottom - 1);
		dc.LineTo(lpdis->rcItem.left, lpdis->rcItem.bottom - 1);

		if (lpdis->itemState & ODS_FOCUS)
			dc.DrawFocusRect(&lpdis->rcItem);
	}
};
*/
class CNumUpDownCtrl :
	public CWindowImpl<CNumUpDownCtrl, CUpDownCtrl>
{
public:
	BEGIN_MSG_MAP(CNumUpDownCtrl)
		REFLECTED_NOTIFY_CODE_HANDLER(UDN_DELTAPOS, OnDeltaPos);
	END_MSG_MAP()

	void SetValueForBuddy(double val)
	{
		CEdit edit = GetBuddy();
		ATLASSERT(edit.IsWindow());

		CString str;
		str.Format("%.2f", val);

		edit.SetWindowText(str);
	}

	double GetPos(void)
	{
		CEdit edit = GetBuddy();
		ATLASSERT(edit.IsWindow());

		int len = edit.GetWindowTextLength() + 1;
		LPTSTR str = new TCHAR[len];
		edit.GetWindowText(str, len);

		double val = atof(str);
		delete[] str;
		return val;
	}

	LRESULT OnDeltaPos(int, LPNMHDR pnmh, BOOL& bHandled)
	{
		NM_UPDOWN* pUd = (NM_UPDOWN*) pnmh;
	
		double pos = GetPos();

		if (UDS_WRAP & GetStyle())
		{
			int iMin, iMax;
			GetRange(iMin, iMax);

			if (pos >= iMax)
				pos = iMin;
			else if (pos <= iMin)
				pos = iMax;
		}

		SetValueForBuddy(pos + .1 * pUd->iDelta);
		bHandled = FALSE;
		return 0;
	}

	CWindow SetBuddy(HWND hWndBuddy)
	{
		CWindow wnd = CUpDownCtrl::SetBuddy(hWndBuddy);
		SetValueForBuddy(GetPos());
		return wnd;
	}
};

class CMyListBox :
	public CWindowImpl<CMyListBox, CListBox>
{
	public:
		DECLARE_EMPTY_MSG_MAP()

		HWND Create(HWND hWndParent, _U_RECT rect, UINT nID = NULL)
		{
			m_hWnd = CListBox::Create(hWndParent, rect, NULL,
				WS_CHILD | WS_VISIBLE | WS_VSCROLL |
				LBS_EXTENDEDSEL | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | LBS_USETABSTOPS,
				WS_EX_STATICEDGE, nID
			);

			NONCLIENTMETRICS ncm;
			ncm.cbSize = sizeof(NONCLIENTMETRICS);
			SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
			HFONT hFont = CreateFontIndirect(&ncm.lfMessageFont);

			SetFont(hFont);
			SetTabStops(15);

			return m_hWnd;
		}

		CMyListBox(HWND hWnd = NULL)
		{
			m_hWnd = hWnd;
		}

		~CMyListBox(void)
		{
			m_hWnd = NULL; 
		}
};

class CMyTabCtrl :
	public CWindowImpl<CMyTabCtrl, CTabCtrl>
{
	BEGIN_MSG_MAP(CMyTabCtrl)
		MESSAGE_HANDLER(WM_ERASEBKGND,	OnEraseBkgnd)
		MESSAGE_HANDLER(WM_PAINT,		OnPaint)
		MESSAGE_HANDLER(OCM_DRAWITEM,	OnDrawItem)
	END_MSG_MAP()
	
	CMyTabCtrl() {}

	BOOL SubclassWindow(HWND hWnd)
	{
		BOOL	bRet = CWindowImpl<CMyTabCtrl, CTabCtrl>::SubclassWindow(hWnd);
		ModifyStyle(0, TCS_OWNERDRAWFIXED);
		return	bRet;
	}

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CRect rc;
		GetClientRect(rc);
		FillRect((HDC) wParam, &rc, (HBRUSH) GetStockObject(BLACK_BRUSH));
		return 0;
	}

	LRESULT OnDrawItem(UINT, WPARAM wParam, LPARAM lParam, BOOL&)	
	{
		UINT				uiCtrlId	= wParam;
		LPDRAWITEMSTRUCT	pInfo		= (LPDRAWITEMSTRUCT)lParam;	

		TC_ITEM	Item;
		char	Text[256];

		Item.mask		= TCIF_TEXT;
		Item.pszText	= Text;     
		Item.cchTextMax = 255;

		if (pInfo->CtlType == ODT_TAB && pInfo->hDC && pInfo->itemID >= 0 && GetItem(pInfo->itemID, &Item))
		{	
			CFont	Font;		
			BOOL	bSelected = (pInfo->itemID == (UINT)GetCurSel());
			CRect	rc(pInfo->rcItem);

			int nSavedDC = SaveDC(pInfo->hDC);		

			SetBkMode(pInfo->hDC, TRANSPARENT);

			FillRect((HDC) wParam, &rc, (HBRUSH) GetStockObject(GRAY_BRUSH));

			{
				DrawText(pInfo->hDC, Text, strlen(Text), rc, DT_SINGLELINE|DT_VCENTER|DT_CENTER);
			}

			RestoreDC(pInfo->hDC, nSavedDC);
		}

		return 0;
	}

	LRESULT OnEraseBkgnd(UINT, WPARAM wParam, LPARAM, BOOL&)
	{
		return TRUE;
	}

};

#endif
