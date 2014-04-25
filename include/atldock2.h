#ifndef __ATL_DOCK2_H__
#define __ATL_DOCK2_H__

/////////////////////////////////////////////////////////////////////////////
// Additional docking classes
//
// These classes are extensions to the atldock.h classes.
// They provide a different look and some helper classes
// but are not essential to compiling the docking framework.
//
// They are based on suggestions and improvements from 
// Jens Nilsson <jnilsson@icebreaker.com>.
//
// Also included is a persistance class for the panel sizes.
//

#pragma once

#ifndef __ATL_DOCK_H__
   #error atldock2.h requires atldock.h to be included first
#endif


///////////////////////////////////////////////////////
// Flat looking Docking Framework

class CFlatDockingPaneChildWindow : public CDockingPaneChildWindowImpl<CFlatDockingPaneChildWindow>
{
public:
	typedef CDockingPaneChildWindowImpl<CFlatDockingPaneChildWindow> parentClass;

   DECLARE_WND_CLASS_EX(_T("WTL_FlatDockingChildWindow"), CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, NULL)

	BEGIN_MSG_MAP(CFlatDockingPaneChildWindow)
		MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingsChange)
		CHAIN_MSG_MAP(parentClass)
	END_MSG_MAP()

	CFont m_fontSmCaption;

/*   CFlatDockingPaneChildWindow(DOCKCONTEXT* ctx) : 
      CDockingPaneChildWindowImpl<CFlatDockingPaneChildWindow>(ctx) 
   { 
   }*/

	CFlatDockingPaneChildWindow(DOCKCONTEXT *ctx) : parentClass(ctx) 
	{
		BOOL bHandled;
		OnSettingsChange(0,0,0,bHandled); // Get the caption font
	}

	void DrawGripperBar(CDCHandle dc, short Side, RECT &rcBar, RECT &rcCloseButton, bool bCloseDown)
	{
		if( ::IsRectEmpty(&rcBar) )
			return;
		const int INSET = 6;
		const int Y  = 2;

		if( !::IsRectEmpty(&rcCloseButton) )
		{
			CMemoryDC dcMem(dc, rcCloseButton);
			dcMem.DrawFrameControl(&rcCloseButton, DFC_CAPTION, DFCS_FLAT | (bCloseDown ? DFCS_CAPTIONCLOSE|DFCS_PUSHED : DFCS_CAPTIONCLOSE));
			dcMem.FrameRect(&rcCloseButton, GetSysColorBrush(COLOR_BTNFACE));
		}

		RECT rcText;
		::SetRect(&rcText,rcBar.left+INSET, rcBar.top+Y, rcCloseButton.left - 4, rcBar.bottom - 1);
		// Draw caption title
		TCHAR szCaption[80]; // Q99046 tells us max length is 78 chars
		::GetWindowText(m_ctx->hwndChild, szCaption, sizeof(szCaption)/sizeof(TCHAR));
		dc.SetBkMode(TRANSPARENT);
		HFONT fontOld = dc.SelectFont(m_fontSmCaption);
		dc.DrawText(szCaption, ::lstrlen(szCaption), &rcText, DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX);
		dc.SelectFont(fontOld);
	}

/*   void DrawGripperBar(CDCHandle dc, short Side, const RECT& rcBar, RECT& rcCloseButton, bool bCloseDown)
   {
      if( ::IsRectEmpty(&rcBar) ) return;
      const int INSET = 5;
      const int Y = 7;
      const int CY = 3;
      bool bVertical = IsDockedVertically(Side);
      RECT rcLine;
      if( bVertical ) {
         ::SetRect(&rcLine, rcBar.left + INSET, rcBar.top + Y, rcBar.right - INSET, rcBar.top + Y + CY);
      }
      else {
         ::SetRect(&rcLine, rcBar.left + Y, rcBar.top + INSET, rcBar.left + Y + CY, rcBar.bottom - INSET);
      }
      dc.Draw3dRect(&rcLine, ::GetSysColor(COLOR_BTNHIGHLIGHT), ::GetSysColor(COLOR_BTNSHADOW));
      if( !::IsRectEmpty(&rcCloseButton) ) {
         dc.DrawFrameControl(&rcCloseButton, DFC_CAPTION, bCloseDown ? DFCS_CAPTIONCLOSE|DFCS_PUSHED : DFCS_CAPTIONCLOSE);
      }
   }*/

	LRESULT OnSettingsChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// Caption font
		NONCLIENTMETRICS ncm = { 0 };
		ncm.cbSize = sizeof(ncm);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
		if( !m_fontSmCaption.IsNull() )
			m_fontSmCaption.DeleteObject();
		// Don't use bold font
		ncm.lfSmCaptionFont.lfWeight = FW_NORMAL;
		m_fontSmCaption.CreateFontIndirect(&ncm.lfSmCaptionFont); //Small caption font
		bHandled = FALSE; // Let other handlers get a shot too
		return 0;
	}

   void DrawPaneFrame(CDCHandle dc, short Side, const RECT& rc)
   {      
      bool bVertical = IsDockedVertically(Side);
      // Left highlight
      if( Side != DOCK_LEFT ) dc.FillSolidRect(rc.left, rc.top, 1, rc.bottom - rc.top, ::GetSysColor(COLOR_3DHILIGHT));
      if( Side != DOCK_LEFT ) dc.FillSolidRect(rc.left, rc.top, 1, rc.bottom - rc.top, ::GetSysColor(COLOR_3DHILIGHT));
      // Right shadow
      if( Side != DOCK_RIGHT ) dc.FillSolidRect(rc.right - 1 - (bVertical ? 0 : m_cxySplitter), rc.top, 1, rc.bottom - rc.top, ::GetSysColor(COLOR_3DSHADOW));
      // Top highlight
      dc.FillSolidRect(rc.left, rc.top, rc.right - rc.left - 1, 1, ::GetSysColor(COLOR_3DHILIGHT));
      // Bottom shadow
      dc.FillSolidRect(rc.left, rc.bottom - 1 - (bVertical ? m_cxySplitter : 0), rc.right - rc.left, 1, ::GetSysColor(COLOR_3DSHADOW));
   }

   void DrawSplitterBar(CDCHandle dc, bool bVertical, RECT& rc)
   {      
      // Splitter bar
      if( m_cxySplitter > 0 ) {
         if( bVertical) {
            dc.FillSolidRect(rc.left, rc.bottom - m_cxySplitter, rc.right - rc.left - 1, 1, ::GetSysColor(COLOR_3DHILIGHT));
            dc.FillSolidRect(rc.left, rc.bottom - 1, rc.right - rc.left, 1, ::GetSysColor(COLOR_3DSHADOW));
         }
         else {
            dc.FillSolidRect(rc.right - m_cxySplitter, rc.top , 1, rc.bottom - rc.top - 1, ::GetSysColor(COLOR_3DHILIGHT));
            dc.FillSolidRect(rc.right - 1, rc.top, 1, rc.bottom - rc.top, ::GetSysColor(COLOR_3DSHADOW));
         }
      }
   }
};

class CFlatDockingPaneWindow : public CDockingPaneWindowImpl<CFlatDockingPaneWindow>
{
public:
   DECLARE_WND_CLASS_EX(_T("WTL_FlatDockingPaneWindow"), CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, COLOR_WINDOW)

   void DrawSplitterBar(CDCHandle dc, bool /*bVertical*/, RECT& rc)
   {      
      if( ::IsRectEmpty(&rc) ) return;

      RECT rect = rc;
      if( m_Side == DOCK_LEFT ) {
         rect.left += 1;
         rect.right -= 1;
         dc.FillSolidRect(rc.left, rc.top, 1, rc.bottom - rc.top, ::GetSysColor(COLOR_3DHILIGHT));
         dc.FillSolidRect(rc.right-1, rc.top, 1, rc.bottom - rc.top, ::GetSysColor(COLOR_3DSHADOW));
      }
      if( m_Side == DOCK_RIGHT ) {
         rect.left += 1;
         rect.right -= 1;
         dc.FillSolidRect(rc.right - 1, rc.top, 1, rc.bottom - rc.top, ::GetSysColor(COLOR_3DSHADOW));
         dc.FillSolidRect(rc.left, rc.top, 1, rc.bottom - rc.top, ::GetSysColor(COLOR_3DHILIGHT));
      }
      if( m_Side == DOCK_TOP ) {
         rect.bottom -= 1;
         rect.top += 1;
         dc.FillSolidRect(rc.left, rc.top, rc.right - rc.left, 1, ::GetSysColor(COLOR_3DHILIGHT));
         dc.FillSolidRect(rc.left, rc.bottom - 1, rc.right - rc.left, 1, ::GetSysColor(COLOR_3DSHADOW));
      }
      if( m_Side == DOCK_BOTTOM ) {
         rect.top += 1;
         rect.bottom -= 1;
         dc.FillSolidRect(rc.left, rc.bottom - 1, rc.right - rc.left, 1, ::GetSysColor(COLOR_3DSHADOW));
         dc.FillSolidRect(rc.left, rc.top, rc.right - rc.left, 1, ::GetSysColor(COLOR_3DHILIGHT));
      }
      dc.FillRect(&rect, ::GetSysColorBrush(COLOR_3DFACE));
   }
};


class CFlatDockingWindow : 
   public CDockingWindowImpl<CFlatDockingWindow, CFlatDockingPaneWindow, CFlatDockingPaneChildWindow>
{
public:
   DECLARE_WND_CLASS_EX(_T("WTL_FlatDockingWindow"), CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, NULL)
};


///////////////////////////////////////////////////////
// DotNet looking Docking Framework

class CDotNetDockingPaneChildWindow : public CDockingPaneChildWindowImpl<CDotNetDockingPaneChildWindow>
{
public:
   DECLARE_WND_CLASS_EX(_T("WTL_DotNetDockingChildWindow"), CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, NULL)

   CDotNetDockingPaneChildWindow(DOCKCONTEXT* ctx) : 
      CDockingPaneChildWindowImpl<CDotNetDockingPaneChildWindow>(ctx) 
   { 
   }

   void DrawGripperBar(CDCHandle dc, short Side, RECT& rcBar, RECT& rcCloseButton, bool bCloseDown)
   {
      if( ::IsRectEmpty(&rcBar) ) return;
      
      const int INSET = 3;
      const int CX = 2;
      const int CY = 2;
      bool bVertical = IsDockedVertically(Side);

      // Figure out the sizes of things
#ifndef _ATL_MIN_CRT
      static CFont s_fontV;
      static CFont s_fontH;
#else
      CFont s_fontV;
      CFont s_fontH;
#endif
      if( s_fontV.IsNull() ) {
         NONCLIENTMETRICS ncm = { 0 };
         ncm.cbSize = sizeof(ncm);
         ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
         s_fontV.CreateFontIndirect(&ncm.lfMenuFont);
         ATLASSERT(!s_fontV.IsNull());
      }
      if( s_fontH.IsNull() ) {
         NONCLIENTMETRICS ncm = { 0 };
         ncm.cbSize = sizeof(ncm);
         ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);
         LOGFONT& lf = ncm.lfMenuFont;
         lf.lfEscapement = 900;                  // Rotate 270 degrees
         lf.lfClipPrecision |= CLIP_LH_ANGLES;   // For Win95 compatibility (Q82932)
         lf.lfOutPrecision = OUT_TT_ONLY_PRECIS; // Choose a truetype font (Q72021)
         s_fontH.CreateFontIndirect(&lf);
         ATLASSERT(!s_fontH.IsNull());
      }

      RECT rcLine;
      RECT rcText;
      RECT rcButton;
      if( bVertical ) {
         ::SetRect(&rcLine, rcBar.left + CX, rcBar.top + CY, rcBar.right - CX, rcBar.bottom);
         rcText = rcLine;
         ::InflateRect(&rcText, -4, -1);
         ::SetRect(&rcButton, rcLine.right, rcLine.top, rcCloseButton.right, rcLine.bottom);
      }
      else {
         ::SetRect(&rcLine, rcBar.left + CY, rcBar.top + CX, rcBar.right - CY, rcBar.bottom - CX - INSET);
         rcText = rcLine;
         ::InflateRect(&rcText, 4, -4);
         ::SetRect(&rcButton, rcLine.left, rcCloseButton.top, rcLine.right, rcLine.top);
      }

      // Paint the caption (with a dark line)
      bool bSelected = IsChild(::GetFocus())==TRUE;
      dc.FillRect(&rcLine, bSelected ? ::GetSysColorBrush(COLOR_ACTIVECAPTION) : ::GetSysColorBrush(COLOR_3DFACE));
      dc.Draw3dRect(&rcLine, ::GetSysColor(COLOR_BTNSHADOW), ::GetSysColor(COLOR_BTNSHADOW));
      
      // Paint caption text
      TCHAR szCaption[128] = { 0 };      
      ::GetWindowText(GetWindow(GW_CHILD), szCaption, (sizeof(szCaption)/sizeof(TCHAR))-1);
      dc.SetBkMode(TRANSPARENT);
      dc.SetTextColor(bSelected ? ::GetSysColor(COLOR_CAPTIONTEXT) : ::GetSysColor(COLOR_BTNTEXT));
      HFONT hOldFont = dc.SelectFont( bVertical ? (HFONT) s_fontV : (HFONT) s_fontH );
      if( bVertical ) {
         dc.DrawText(szCaption, -1, &rcText, DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS | DT_VCENTER);
      }
      else {
         ::ExtTextOut(dc, rcText.left + 2, rcText.bottom, ETO_CLIPPED, &rcText, szCaption, ::lstrlen(szCaption), NULL);
      }
      dc.SelectFont(hOldFont);

      // Draw close button
      if( !::IsRectEmpty(&rcCloseButton) ) {
         UINT uFlags = DFCS_FLAT | DFCS_CAPTIONCLOSE;
         if( bCloseDown ) uFlags |= DFCS_PUSHED;
         dc.DrawFrameControl(&rcButton, DFC_CAPTION, uFlags);
      }
   }

   void DrawPaneFrame(CDCHandle /*dc*/, short /*Side*/, const RECT& /*rc*/)
   {      
   }

   void DrawSplitterBar(CDCHandle /*dc*/, bool /*bVertical*/, RECT& /*rc*/)
   {      
   }
};

class CDotNetDockingPaneWindow : public CDockingPaneWindowImpl<CDotNetDockingPaneWindow>
{
public:
   DECLARE_WND_CLASS_EX(_T("WTL_DotNetDockingPaneWindow"), CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, COLOR_WINDOW)

   void DrawSplitterBar(CDCHandle dc, bool /*bVertical*/, RECT& rc)
   {      
      if( ::IsRectEmpty(&rc) ) return;
      dc.FillRect(&rc, ::GetSysColorBrush(COLOR_3DFACE));
   }
};


class CDotNetDockingWindow : 
   public CDockingWindowImpl<CDotNetDockingWindow, CDotNetDockingPaneWindow, CDotNetDockingPaneChildWindow>
{
public:
   DECLARE_WND_CLASS_EX(_T("WTL_DotNetDockingWindow"), CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, NULL)
};


#endif // __ATL_DOCK2_H__
