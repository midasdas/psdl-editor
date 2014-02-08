#ifndef __CTRLS_H__
#define __CTRLS_H__

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
