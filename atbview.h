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
	virtual BOOL IsWindow(void) = 0;
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
	BOOL IsWindow(void)
		{ return CDialogImpl<T>::IsWindow(); }
	void OnFinalMessage(HWND)
		{ delete this; }
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
		CMyListBox       m_list;
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

		CTextureView(psdl::attribute* pAtb, psdl* pDoc) : m_pAtb(pAtb), m_pDoc(pDoc) {}

		LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);

	private:
		psdl* m_pDoc;
		psdl::attribute* m_pAtb;
		CString m_textureName;

};

class CTunnelView :
	public CPropViewImpl<CTunnelView>,
	public CDialogResize<CTunnelView>,
	public COwnerDraw<CTunnelView>
{
	public:
		enum { IDD = IDD_ATB_TUNNEL };

		BEGIN_DLGRESIZE_MAP(CTunnelView)
			DLGRESIZE_CONTROL(IDC_GROUP1, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_GROUP2, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_GROUP3, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_GROUP3, DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDC_LIST,   DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_LIST,   DLSZ_SIZE_Y)
		END_DLGRESIZE_MAP()

		BEGIN_MSG_MAP(CTunnelView)
			COMMAND_RANGE_HANDLER(IDC_RAILING, IDC_WALL, OnRadioClicked)
			COMMAND_CODE_HANDLER(BN_CLICKED, OnCheckClicked)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			NOTIFY_HANDLER(IDC_LIST, NM_CLICK, OnClick)
			CHAIN_MSG_MAP(CDialogResize<CTunnelView>)
			CHAIN_MSG_MAP(COwnerDraw<CTunnelView>)
		END_MSG_MAP()

		CTunnelView(psdl::attribute *pAtb)
		{
			m_atb = static_cast<psdl::tunnel*>(pAtb);
		}

		void SetState(WORD wID, bool bState)
		{
			::SendMessage(GetDlgItem(wID), BM_SETCHECK, bState ? BST_CHECKED : BST_UNCHECKED, 0);
		}

		LRESULT OnCheckClicked(WORD, WORD, HWND, BOOL&);

		LRESULT OnRadioClicked(WORD wNotifyCode, WORD, HWND, BOOL&)
		{
			if (wNotifyCode == BN_CLICKED)
			{
				m_atb->set_flag(BIT_STYLE, BST_CHECKED == ::SendMessage(GetDlgItem(IDC_WALL), BM_GETCHECK, 0, 0));
			}
			return 0;
		}

		LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);
		LRESULT OnClick(int, LPNMHDR lpnmh, BOOL&);
		void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
		void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);

	private:

		CListViewCtrl m_list;
		psdl::tunnel* m_atb;
};

#endif
