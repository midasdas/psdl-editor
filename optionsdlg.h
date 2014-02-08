#ifndef __OPTIONSDLG_H__
#define __OPTIONSDLG_H__

#include "resource.h"
#include "aboutdlg.h"
#include "psdl-editor.h"
//#include "dlgs.h"

class CGeneralOptsPage :
	public CDialogImpl<CGeneralOptsPage>,
	public CWinDataExchange<CGeneralOptsPage>
{
public:
	enum { IDD = IDD_GENERAL_OPTS };

	DECLARE_EMPTY_MSG_MAP()

	BEGIN_DDX_MAP(CGeneralOptsPage)
		DDX_CHECK(IDC_VERTICES, g_options.general.test)
	END_DDX_MAP()
};

class CToolsOptsPage :
	public CDialogImpl<CToolsOptsPage>,
	public CWinDataExchange<CToolsOptsPage>
{
public:
	enum { IDD = IDD_TOOLS_OPTS };

	BEGIN_MSG_MAP(CToolsOptsPage)
		COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowse)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CToolsOptsPage)
		DDX_TEXT(IDC_MM2PATH, m_sMM2Path)
	END_DDX_MAP()

	CToolsOptsPage()
	{
		ExpandEnvironmentStrings(g_options.tools.mm2Exe, m_sMM2Path.GetBuffer(MAX_PATH), MAX_PATH);
	}

	~CToolsOptsPage()
	{
		strcpy(g_options.tools.mm2Exe, m_sMM2Path.GetBuffer(MAX_PATH));
	}

	LRESULT OnBrowse(WORD, WORD/* wID*/, HWND, BOOL&)
	{
		DoDataExchange(TRUE, IDC_MM2PATH);
		CFileDialog fDlg(TRUE, NULL, m_sMM2Path, OFN_HIDEREADONLY, _T("Executables\0*.exe\0All Files\0*.*\0"));

		if (IDOK == fDlg.DoModal())
		{
			m_sMM2Path = fDlg.m_szFileName;
			DoDataExchange(FALSE, IDC_MM2PATH);
		}
		return 0;
	}

	CString m_sMM2Path;
};

class COptionsDlg : public CDialogImpl<COptionsDlg>
{
public:
	enum { IDD = IDD_OPTIONS };

	BEGIN_MSG_MAP(COptionsDlg)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnItemSelected)
	END_MSG_MAP()

	COptionsDlg(int nStartPage = 0) : nCurSel(nStartPage), hWndLast(NULL) {}

	LRESULT OnItemSelected(int, LPNMHDR pnmh, BOOL&)
	{
		::ShowWindow(hWndLast, SW_HIDE);

		hWndLast = (HWND) m_tree.GetItemData(LPNMTREEVIEW(pnmh)->itemNew.hItem);
		::ShowWindow(hWndLast, SW_SHOW);

		return 0;
	}

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		CenterWindow(GetParent());

		HTREEITEM hItem, hRoot;
		HWND hWndHolder = GetDlgItem(IDC_PLACEHOLDER);

		m_tree = GetDlgItem(IDC_TREE);

		hRoot = m_tree.InsertItem(_T("General"), TVI_ROOT, TVI_LAST);
		m_tree.SetItemData(hRoot, (long) pgGeneral.Create(hWndHolder));
		pgGeneral.DoDataExchange(FALSE);

			hItem = m_tree.InsertItem(_T("Directories"), hRoot, TVI_LAST);

			hItem = m_tree.InsertItem(_T("Tools"), hRoot, TVI_LAST);
			m_tree.SetItemData(hItem, (long) pgTools.Create(hWndHolder));
			pgTools.DoDataExchange(FALSE);

		hRoot = m_tree.InsertItem(_T("File Formats"), TVI_ROOT, TVI_LAST);

			hItem = m_tree.InsertItem(_T("PSDL"), hRoot, TVI_LAST);
			hItem = m_tree.InsertItem(_T("CPVS"), hRoot, TVI_LAST);
			hItem = m_tree.InsertItem(_T("INST"), hRoot, TVI_LAST);
			hItem = m_tree.InsertItem(_T("BAI"), hRoot, TVI_LAST);

		int i = -1;
		hRoot = m_tree.GetRootItem();

		while (hRoot)
		{
			if (++i == nCurSel)
				m_tree.SelectItem(hRoot);

			m_tree.Expand(hRoot);
			hItem = m_tree.GetNextItem(hRoot, TVGN_CHILD);

			while (hItem)
			{
				if (++i == nCurSel)
					m_tree.SelectItem(hItem);

				hItem = m_tree.GetNextItem(hItem, TVGN_NEXT);
			}

			hRoot = m_tree.GetNextItem(hRoot, TVGN_NEXT);
		}

		return 1;
	}

	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		if (wID == IDOK)
		{
			pgGeneral.DoDataExchange(TRUE);
			pgTools.DoDataExchange(TRUE);
		}

		EndDialog(wID);
		return 0;
	}

	int nCurSel;
	HWND hWndLast;

	CTreeViewCtrl m_tree;

	CGeneralOptsPage	pgGeneral;
	CToolsOptsPage		pgTools;
};

#endif
