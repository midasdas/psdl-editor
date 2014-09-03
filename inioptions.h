#ifndef __INI_OPTIONS_H__
#define __INI_OPTIONS_H__

#include <string>

class IniOptions
{
public:

	IniOptions(CString strFileName) : m_strFileName(strFileName), m_lpAppName(NULL)
	{
		ATLTRACE("Initialization file: %s\n", m_strFileName);
	}

	void SetSection(LPCTSTR lpAppName)
	{
		m_lpAppName = lpAppName;
	}

	std::string GetString(LPCTSTR lpKeyName, LPCTSTR lpDefault = _T(""))
	{
		unsigned nSize, nCnt = 0;
		TCHAR* buf;
		DWORD dwRet;

		do
		{
			nSize = MAX_PATH * ++nCnt;
			buf = new TCHAR[nSize];
			dwRet = GetPrivateProfileString(m_lpAppName, lpKeyName, lpDefault, buf, nSize, m_strFileName);
		}
		while (dwRet >= nSize - 1);

		return buf;
	}

	BOOL WriteString(LPCTSTR lpKeyName, std::string strValue = _T(""))
	{
		return WritePrivateProfileString(m_lpAppName, lpKeyName, strValue.c_str(), m_strFileName);
	}

	UINT GetInt(LPCTSTR lpKeyName, INT nDefault = 0)
	{
		return GetPrivateProfileInt(m_lpAppName, lpKeyName, nDefault, m_strFileName);
	}

	BOOL WriteInt(LPCTSTR lpKeyName, UINT nValue = 0)
	{
		CString strValue;
		strValue.Format("%u", nValue);
		return WritePrivateProfileString(m_lpAppName, lpKeyName, strValue, m_strFileName);
	}

private:

	CString m_strFileName;
	LPCTSTR m_lpAppName;
};

#endif
