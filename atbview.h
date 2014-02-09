#ifndef __ATBVIEW_H__
#define __ATBVIEW_H__

#include "resource.h"
#include "psdl.h"
#include "ctrls.h"

class ATL_NO_VTABLE CPropView
{
public:
	virtual ~CPropView() {}
	virtual HWND Create(HWND hParent) = 0;
	virtual BOOL DestroyWindow() = 0;
	virtual BOOL ShowWindow(int nCmdShow) = 0;
	virtual BOOL DoDataExchange(BOOL bSaveAndValidate = FALSE, UINT nCtlID = (UINT) -1) = 0;
};

template <class T>
class ATL_NO_VTABLE CPropViewImpl :
	public CDialogImpl<T>, public CPropView
{
public:
	virtual ~CPropViewImpl() {}

	virtual HWND Create(HWND hParent)
		{ return CDialogImpl<T>::Create(hParent); }
	virtual BOOL DestroyWindow()
		{ return CDialogImpl<T>::DestroyWindow(); }
	virtual BOOL ShowWindow(int nCmdShow)
		{ return CDialogImpl<T>::ShowWindow(nCmdShow); }
	virtual BOOL DoDataExchange(BOOL bSaveAndValidate = FALSE, UINT nCtlID = (UINT) -1)
		{ return TRUE; }
};

template <WORD t_wDlgTemplateID>
class CAtbView :
	public CPropViewImpl<CAtbView>,
	public CWinDataExchange<CAtbView>,
	public CDialogResize<CAtbView>
{
public:
	enum { IDD = t_wDlgTemplateID };

	BEGIN_DLGRESIZE_MAP(CAtbView)
		DLGRESIZE_CONTROL(IDC_INFO, DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()

	BEGIN_DDX_MAP(CAtbView)
	END_DDX_MAP()

	BEGIN_MSG_MAP(CAtbView)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CDialogResize<CAtbView>)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		DlgResize_Init(false, false, WS_CHILD | WS_VISIBLE);
		return FALSE;
	}
};

class CRoadView :
	public CPropViewImpl<CRoadView>,
	public CWinDataExchange<CRoadView>,
	public CDialogResize<CRoadView>
{
	public:
		enum { IDD = IDD_ATB_ROAD };

		BEGIN_DLGRESIZE_MAP(CRoadView)
			DLGRESIZE_CONTROL(IDC_LIST, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		END_DLGRESIZE_MAP()

		BEGIN_DDX_MAP(CRoadView)
		END_DDX_MAP()

		BEGIN_MSG_MAP(CRoadView)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			CHAIN_MSG_MAP(CDialogResize<CRoadView>)
		END_MSG_MAP()

		CRoadView(psdl::attribute *pAtb)
		{
			m_atb = pAtb;
		}

		LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);

	private:
		CMyListBox		m_list;
		psdl::attribute* m_atb;
};

class CTextureView :
	public CPropViewImpl<CTextureView>,
	public CWinDataExchange<CTextureView>,
	public CDialogResize<CTextureView>
{
	public:
		enum { IDD = IDD_ATB_TEXTURE };

		BEGIN_DLGRESIZE_MAP(CTextureView)
			DLGRESIZE_CONTROL(IDC_EDIT, DLSZ_SIZE_X)
		END_DLGRESIZE_MAP()

		BEGIN_DDX_MAP(CTextureView)
			DDX_TEXT(IDC_EDIT, m_textureName)
		END_DDX_MAP()

		BEGIN_MSG_MAP(CTextureView)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			CHAIN_MSG_MAP(CDialogResize<CTextureView>)
		END_MSG_MAP()

		CTextureView(psdl::attribute* pAtb)
		{
			m_atb = pAtb;
		}

		LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);

	private:
		psdl::attribute *m_atb;
		CString m_textureName;

};

class CTunnelView :
	public CPropViewImpl<CTunnelView>,
	public CDialogResize<CTunnelView>
{
	public:
		enum { IDD = IDD_ATB_TUNNEL };

		BEGIN_DLGRESIZE_MAP(CTunnelView)
			DLGRESIZE_CONTROL(IDC_GROUP1, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_GROUP2, DLSZ_SIZE_X)
		END_DLGRESIZE_MAP()

		BEGIN_MSG_MAP(CTunnelView)
			COMMAND_RANGE_HANDLER(IDC_RAILING, IDC_WALL, OnRadioClicked)
			COMMAND_CODE_HANDLER(BN_CLICKED, OnCheckClicked)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			CHAIN_MSG_MAP(CDialogResize<CTunnelView>)
		END_MSG_MAP()

		CTunnelView(psdl::attribute *pAtb)
		{
			m_atb = static_cast<psdl::tunnel*>(pAtb);
		}

		void SetState(WORD wID, bool bState)
		{
			::SendMessage(GetDlgItem(wID), BM_SETCHECK, bState ? BST_CHECKED : BST_UNCHECKED, 0);
		}

		LRESULT OnCheckClicked(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL&)
		{
			unsigned char nBit;

			switch (wID)
			{
				case IDC_LEFT:			nBit = BIT_LEFT;			break;
				case IDC_RIGHT:			nBit = BIT_RIGHT;			break;
				case IDC_CLOSEDSTART_L:	nBit = BIT_CLOSEDSTART_L;	break;
				case IDC_CLOSEDEND_L:	nBit = BIT_CLOSEDEND_L;		break;
				case IDC_CLOSEDSTART_R:	nBit = BIT_CLOSEDSTART_R;	break;
				case IDC_CLOSEDEND_R:	nBit = BIT_CLOSEDEND_R;		break;
				case IDC_OFFSETSTART_L:	nBit = BIT_OFFSETSTART_L;	break;
				case IDC_OFFSETEND_L:	nBit = BIT_OFFSETEND_L;		break;
				case IDC_OFFSETSTART_R:	nBit = BIT_OFFSETSTART_R;	break;
				case IDC_OFFSETEND_R:	nBit = BIT_OFFSETEND_R;		break;
				case IDC_CURVEDWALL:	nBit = BIT_CURVEDSIDES;		break;
				case IDC_FLAT:			nBit = BIT_CULLED;			break;
				case IDC_CURVED:		nBit = BIT_CURVEDCEILING;	break;

				default: return 0;
			}

			m_atb->set_flag(nBit, BST_CHECKED == ::SendMessage(GetDlgItem(wID), BM_GETCHECK, 0, 0));

			return 0;
		}

		LRESULT OnRadioClicked(WORD wNotifyCode, WORD, HWND, BOOL&)
		{
			if (wNotifyCode == BN_CLICKED)
			{
				m_atb->set_flag(BIT_STYLE, BST_CHECKED == ::SendMessage(GetDlgItem(IDC_WALL), BM_GETCHECK, 0, 0));
			}
			return 0;
		}

		LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
		{
			SetRedraw(FALSE);

			SetState(IDC_RAILING,		!m_atb->get_flag(BIT_STYLE));
			SetState(IDC_WALL,			m_atb->get_flag(BIT_STYLE));
			SetState(IDC_LEFT,			m_atb->get_flag(BIT_LEFT));
			SetState(IDC_RIGHT,			m_atb->get_flag(BIT_RIGHT));
			SetState(IDC_CLOSEDSTART_L,	m_atb->get_flag(BIT_CLOSEDSTART_L));
			SetState(IDC_CLOSEDEND_L,	m_atb->get_flag(BIT_CLOSEDEND_L));
			SetState(IDC_CLOSEDSTART_R,	m_atb->get_flag(BIT_CLOSEDSTART_R));
			SetState(IDC_CLOSEDEND_R,	m_atb->get_flag(BIT_CLOSEDEND_R));
			SetState(IDC_OFFSETSTART_L,	m_atb->get_flag(BIT_OFFSETSTART_L));
			SetState(IDC_OFFSETEND_L,	m_atb->get_flag(BIT_OFFSETEND_L));
			SetState(IDC_OFFSETSTART_R,	m_atb->get_flag(BIT_OFFSETSTART_R));
			SetState(IDC_OFFSETEND_R,	m_atb->get_flag(BIT_OFFSETEND_R));
			SetState(IDC_CURVEDWALL,	m_atb->get_flag(BIT_CURVEDSIDES));
			SetState(IDC_FLAT,			m_atb->get_flag(BIT_CULLED));
			SetState(IDC_CURVED,		m_atb->get_flag(BIT_CURVEDCEILING));

			SetRedraw();

			DlgResize_Init(false, true, WS_CHILD | WS_VISIBLE);
			return FALSE;
		}

	private:

		psdl::tunnel* m_atb;
};

#endif
