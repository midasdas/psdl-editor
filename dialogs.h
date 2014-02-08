#ifndef __DLGS_H__
#define __DLGS_H__

#include "resource.h"

#include <vector>

struct duplicateProps
{
	unsigned int nCount;
	bool bVertices;
	bool bPerimeters;
	bool bNeighbours;
	bool bTextures;
	bool bExclude;
	std::vector<unsigned char> aAttributes;

	duplicateProps(void) :	nCount(1),
							bVertices(true),
							bPerimeters(true),
							bNeighbours(true),
							bTextures(false),
							bExclude(false)
	{}
};

struct optimizeProps
{
	bool bTextureRefs;
	bool bTextures;
	bool bEmpty;

	optimizeProps(void) :	bTextureRefs(false),
							bTextures(false),
							bEmpty(false)
	{}
};

extern duplicateProps	g_duplicateProps;
extern optimizeProps	g_optimizeProps;

class CDuplicateDlg :
	public CDialogImpl<CDuplicateDlg>,
	public CWinDataExchange<CDuplicateDlg>
{
	public:

		BEGIN_DDX_MAP(CDuplicateDlg)
			DDX_UINT(IDC_COUNT,			g_duplicateProps.nCount)
			DDX_CHECK(IDC_VERTICES,		g_duplicateProps.bVertices)
			DDX_CHECK(IDC_PERIMETERS,	g_duplicateProps.bPerimeters)
			DDX_CHECK(IDC_NEIGHBOURS,	g_duplicateProps.bNeighbours)
			DDX_CHECK(IDC_TEXTURES,		g_duplicateProps.bTextures)
			DDX_CHECK(IDC_ATTRIBUTES,	g_duplicateProps.bExclude)
		END_DDX_MAP()

		enum { IDD = IDD_DUPLICATE };

		BEGIN_MSG_MAP(CDuplicateDlg)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
			COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
			COMMAND_ID_HANDLER(IDC_RESET, OnReset)
		END_MSG_MAP()

		LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
		{
			CenterWindow(GetParent());

			DoDataExchange(FALSE);

			CUpDownCtrl ud;
			ud.Create(m_hWnd, rcDefault, NULL,	WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT |
												UDS_ARROWKEYS | UDS_NOTHOUSANDS | UDS_SETBUDDYINT
			);
			ud.SetBuddy(GetDlgItem(IDC_COUNT));
			ud.SetRange(1, 100);

			return TRUE;
		}

		LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
		{
			DoDataExchange(TRUE);
			EndDialog(wID);
			return FALSE;
		}

		LRESULT OnReset(WORD, WORD wID, HWND, BOOL&)
		{
			g_duplicateProps = duplicateProps();
			DoDataExchange(FALSE);
			NextDlgCtrl();
			return TRUE;
		}
};

class COptimizeDlg :
	public CDialogImpl<COptimizeDlg>,
	public CWinDataExchange<COptimizeDlg>
{
public:

	BEGIN_DDX_MAP(CDuplicateDlg)
		DDX_CHECK(IDC_TEXTURE_REFS,	g_optimizeProps.bTextureRefs)
		DDX_CHECK(IDC_TEXTURES,		g_optimizeProps.bTextures)
		DDX_CHECK(IDC_EMPTY_BLOCKS,	g_optimizeProps.bEmpty)
	END_DDX_MAP()

	enum { IDD = IDD_OPTIMIZE };

	BEGIN_MSG_MAP(CDuplicateDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
		CenterWindow(GetParent());
		DoDataExchange(FALSE);
		return TRUE;
	}

	LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		DoDataExchange(TRUE);
		EndDialog(wID);
		return FALSE;
	}
};

template <class T>
class CCenterFileDialogImpl : public CFileDialogImpl<T>
{
public:

	BEGIN_MSG_MAP(CCenterFileDialog)
		CHAIN_MSG_MAP(CFileDialogImpl<T>)
	END_MSG_MAP()

	CCenterFileDialogImpl(
		BOOL bOpenFileDialog,
		LPCTSTR lpszDefExt		= NULL,
		LPCTSTR lpszFileName	= NULL,
		DWORD dwFlags			= OFN_HIDEREADONLY,
		LPCTSTR lpszFilter		= NULL,
		HWND hWndParent			= NULL)
		: CFileDialogImpl<T>(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, hWndParent)
	{ }

	void OnInitDone(LPOFNOTIFY lpon)
	{
		GetFileDialogWindow().CenterWindow(lpon->lpOFN->hwndOwner);
	}

	char* GetFileExt(void)
	{
		return strrchr(m_szFileName, '.') + 1;
	}

	char* GetFolderPath(void)
	{
		char* pos = strrchr(m_szFileName, '\\');

		if (!pos)
			pos = strrchr(m_szFileName, '/');

		if (pos)
		{
			unsigned nLen = pos - m_szFileName;

			char* ret = new char[nLen + 1];
			*ret = '\0';
			strncat(ret, m_szFileName, nLen);

			return ret;
		}

		return NULL;
	}

	TCHAR* GetPathName(void)
	{
		return m_szFileName;
	}
};

class CCenterFileDialog : public CCenterFileDialogImpl<CCenterFileDialog>
{
public:
	CCenterFileDialog(
		BOOL bOpenFileDialog,
		LPCTSTR lpszDefExt		= NULL,
		LPCTSTR lpszFileName	= NULL,
		DWORD dwFlags			= OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter		= NULL,
		HWND hWndParent			= NULL)
		: CCenterFileDialogImpl<CCenterFileDialog>(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, hWndParent)
	{ }
};

#endif
