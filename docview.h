#ifndef __DOCVIEW_H__
#define __DOCVIEW_H__

#include "io_error.h"
#include <string>

template <class TDoc>
class View
{
	public:
		View(void) : m_pDoc(NULL) {}

		void SetDocument(TDoc* pDoc) { m_pDoc = pDoc; }
		virtual void RenderScene(HDC hDC, HGLRC hRC) {}

	protected:
		TDoc* m_pDoc;
};

class DocTemplateBase
{
public:

	virtual std::string GetFileName(bool bFull = false) = 0;
	virtual error::code OpenDocument(std::string strFileName) = 0;
	virtual error::code SaveDocument(std::string strFileName = _T("")) = 0;
	virtual bool FileExists(void) = 0;
	virtual bool IsModified(void) = 0;
};

template <class TDoc, class TView>
class DocTemplate : public DocTemplateBase
{
public:

	DocTemplate(void)
	{
		m_pDoc  = NULL;
		m_pView = new TView();
		m_bFileExists = false;
		m_bModified   = false;
	}

	virtual ~DocTemplate(void)
	{
		if (m_pDoc)  delete m_pDoc;
		if (m_pView) delete m_pView;
	}

	std::string GetFileName(bool bFull = false)
	{
		if (!bFull)
		{
			size_t pos = m_strFileName.rfind(_T('\\'));
			
			if (pos == m_strFileName.npos)
				pos = m_strFileName.rfind(_T('/'));
			
			if (pos != m_strFileName.npos)
				return m_strFileName.substr(++pos);
		}

		return m_strFileName;
	}

	virtual void NewDocument(std::string strFileName = _T("untitled"))
	{
		if (m_pDoc) delete m_pDoc;

		m_pDoc        = new TDoc();
		m_strFileName = strFileName;
		m_bModified   = false;
		m_bFileExists = false;

		m_pView->SetDocument(m_pDoc);
		UpdateViews();
	}

	virtual error::code OpenDocument(std::string strFileName)
	{
		TDoc* pDoc = new TDoc();

		error::code code = pDoc->read_file(strFileName.c_str());

		if (code & error::ok)
		{
			if (m_pDoc) delete m_pDoc;

			m_pDoc        = pDoc;
			m_strFileName = strFileName;
			m_bModified   = false;
			m_bFileExists = true;

			m_pView->SetDocument(m_pDoc);
			UpdateViews();
		}
		else
		{
			delete pDoc;
		}

		return code;
	}

	error::code SaveDocument(std::string strFileName)
	{
		if (strFileName.empty())
			strFileName = m_strFileName;

		error::code code = m_pDoc->write_file(strFileName.c_str());

		if (code & error::ok)
		{
			m_strFileName = strFileName;
			m_bModified   = false;
			m_bFileExists = true;
		}

		return code;
	}

	bool FileExists(void)    { return m_bFileExists; }
	bool IsModified(void)    { return m_bModified;   }

	TDoc*  GetDocument(void) { return m_pDoc;  }
	TView* GetView(void)     { return m_pView; }

	virtual void UpdateViews(void) {}

	bool m_bModified;

protected:

	TDoc* m_pDoc;
	TView* m_pView;

	std::string m_strFileName;
	bool m_bFileExists;
};

#endif
