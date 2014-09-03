#ifndef __OPTIONSDLG_H__
#define __OPTIONSDLG_H__

#include "resource.h"
#include "options.h"

class COptionsDialog;

class COptionsPage
{
public:
	virtual HWND Create(HWND hWndParent) = 0;
	virtual CString GetTitle(void) = 0;
	virtual BOOL IsCreated(void) = 0;
	virtual void Show(void) = 0;
	virtual void Hide(void) = 0;
	virtual void OnOk(void) = 0;
};

template <class T>
class COptionsPageImpl : public CDialogImpl<T>, public COptionsPage
{
public:
	virtual ~COptionsPageImpl() {}

	HWND Create(HWND hWndParent)
	{
		return CDialogImpl<T>::Create(hWndParent);
	}
	CString GetTitle(void)
	{
		CString strTitle;
		strTitle.LoadString(T::IDD);
		return strTitle;
	}
	BOOL IsCreated(void) { return IsWindow(); }
	void Show(void) { ShowWindow(SW_SHOW); }
	void Hide(void) { ShowWindow(SW_HIDE); }
	virtual void OnOk(void) {}
};

class COptionsPageGeneral :
	public COptionsPageImpl<COptionsPageGeneral>,
	public CWinDataExchange<COptionsPageGeneral>
{
public:
	enum { IDD = IDD_OPTIONS_GENERAL };

	BEGIN_MSG_MAP(COptionsPageTools)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DDX_MAP(COptionsPageGeneral)
		DDX_CHECK(IDC_VERTICES, g_options.general.test)
	END_DDX_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		DoDataExchange(FALSE);
		return 1;
	}

	void OnOk(void)
	{
		DoDataExchange(TRUE);
	}
};

class COptionsPageTools :
	public COptionsPageImpl<COptionsPageTools>,
	public CWinDataExchange<COptionsPageTools>
{
public:
	enum { IDD = IDD_OPTIONS_TOOLS };

	BEGIN_MSG_MAP(COptionsPageTools)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowse)
	END_MSG_MAP()

	BEGIN_DDX_MAP(COptionsPageTools)
		DDX_TEXT(IDC_MM2PATH, m_strMM2Path)
	END_DDX_MAP()

	COptionsPageTools()
	{
		ExpandEnvironmentStrings(g_options.tools.strMM2Exe.c_str(), m_strMM2Path.GetBuffer(MAX_PATH), MAX_PATH);
	}

	~COptionsPageTools()
	{
		g_options.tools.strMM2Exe = m_strMM2Path;
	}

	LRESULT OnBrowse(WORD, WORD, HWND, BOOL&)
	{
		DoDataExchange(TRUE, IDC_MM2PATH);
		CFileDialog fDlg(TRUE, NULL, m_strMM2Path, OFN_HIDEREADONLY, _T("Executables\0*.exe\0All Files\0*.*\0"));

		if (IDOK == fDlg.DoModal())
		{
			m_strMM2Path = fDlg.m_szFileName;
			DoDataExchange(FALSE, IDC_MM2PATH);
		}
		return 0;
	}

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		DoDataExchange(FALSE);
		return 1;
	}

	void OnOk(void)
	{
		DoDataExchange(TRUE);
	}

	CString m_strMM2Path;
};

class COptionsPageDirectories :
	public COptionsPageImpl<COptionsPageDirectories>,
	public CWinDataExchange<COptionsPageDirectories>
{
public:
	enum { IDD = IDD_OPTIONS_DIRECTORIES };

	BEGIN_MSG_MAP(COptionsPageDirectories)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	BEGIN_DDX_MAP(COptionsPageGeneral)
	END_DDX_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		DoDataExchange(FALSE);

		const WORD upArrow[] = { 0xef, 0xc7, 0x83, 0x01 };
		const WORD dnArrow[] = { 0x01, 0x83, 0xc7, 0xef };

		HBITMAP hBitmap;

		hBitmap = CreateBitmap(7, 4, 1, 1, upArrow);
		SendDlgItemMessage(IDC_UP, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hBitmap);

		hBitmap = CreateBitmap(7, 4, 1, 1, dnArrow);
		SendDlgItemMessage(IDC_DOWN, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hBitmap);

		return 1;
	}

	void OnOk(void)
	{
		DoDataExchange(TRUE);
	}
};

struct PageEntry;

typedef struct PageEntry
{
	HTREEITEM     hItem;
	COptionsPage* pPage;
	PageEntry*    pParent;

	PageEntry(COptionsPage* pPage, PageEntry* pParent = NULL) :
	hItem(NULL), pPage(pPage), pParent(pParent) {}
}
PageEntry;

class COptionsDialog : public CDialogImpl<COptionsDialog>
{
public:

	COptionsDialog() : m_pCurrentPage(NULL) {}

	enum { IDD = IDD_OPTIONS };

	BEGIN_MSG_MAP(COptionsDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnItemSelected)
	END_MSG_MAP()

	LRESULT OnItemSelected(int, LPNMHDR pnmh, BOOL&)
	{
		if (m_pCurrentPage && m_pCurrentPage->IsCreated())
			m_pCurrentPage->Hide();

		m_pCurrentPage = (COptionsPage*) m_tree.GetItemData(LPNMTREEVIEW(pnmh)->itemNew.hItem);

		if (m_pCurrentPage)
		{
			if (!m_pCurrentPage->IsCreated())
				m_pCurrentPage->Create(m_hWndHolder);
			m_pCurrentPage->Show();
		}

		return 0;
	}

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		CenterWindow(GetParent());

		m_tree       = GetDlgItem(IDC_TREE);
		m_hWndHolder = GetDlgItem(IDC_PLACEHOLDER);

		HTREEITEM hItem;

		for (int i = 0; i < m_arrPages.GetSize(); i++)
		{
			HTREEITEM hParent = TVI_ROOT;

			if (m_arrPages[i]->pParent && m_arrPages[i]->pParent->hItem)
				hParent = m_arrPages[i]->pParent->hItem;

			m_arrPages[i]->hItem = m_tree.InsertItem(m_arrPages[i]->pPage->GetTitle(), hParent, TVI_LAST);
			m_tree.SetItemData(m_arrPages[i]->hItem, (DWORD_PTR) m_arrPages[i]->pPage);
		}

		HTREEITEM hRoot = m_tree.GetRootItem();
		while (hRoot)
		{
			m_tree.Expand(hRoot);
			if (m_tree.GetItemData(hRoot) == (LPARAM) m_pCurrentPage)
				m_tree.SelectItem(hRoot);

			hItem = m_tree.GetNextItem(hRoot, TVGN_CHILD);
			while (hItem)
			{
				if (m_tree.GetItemData(hItem) == (LPARAM) m_pCurrentPage)
					m_tree.SelectItem(hItem);

				hItem = m_tree.GetNextItem(hRoot, TVGN_NEXT);
			}

			hRoot = m_tree.GetNextItem(hRoot, TVGN_NEXT);
		};

		return 1;
	}

	PageEntry* AddPage(COptionsPage* pPage, PageEntry* pParent = NULL)
	{
		ATLASSERT(pPage != NULL);

		PageEntry* pEntry = new PageEntry(pPage, pParent);
		m_arrPages.Add(pEntry);

		return pEntry;
	}

	void SetInitialPage(COptionsPage* pPage)
	{
		m_pCurrentPage = pPage;
	}

	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		if (wID == IDOK)
		{
			for (int i = 0; i < m_arrPages.GetSize(); i++)
			{
				if (m_arrPages[i]->pPage->IsCreated())
					m_arrPages[i]->pPage->OnOk();
			}
		}

		EndDialog(wID);
		return 0;
	}

private:

	HWND m_hWndHolder;
	CTreeViewCtrl m_tree;
	CSimpleArray<PageEntry*> m_arrPages;
	COptionsPage* m_pCurrentPage;
};

#endif
