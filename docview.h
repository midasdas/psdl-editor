#ifndef __DOCVIEW_H__
#define __DOCVIEW_H__

#include "io_error.h"
#include "thread.h"
#include "dialogs.h"
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

class DocTemplateBase;

typedef struct IOParams
{
	std::string strFileName;
	void*& pDoc;
	DocTemplateBase* pDocTmpl;
	notify_func callbackFunc;

	IOParams(
		std::string _strFileName,
		void*& _pDoc,
		DocTemplateBase* _pDocTmpl,
		notify_func _callbackFunc = default_callback) :

		strFileName(_strFileName),
		pDoc(_pDoc),
		pDocTmpl(_pDocTmpl),
		callbackFunc(_callbackFunc)
	{}
}
IOParams;

typedef struct GLParams
{
	HDC hDC;
	HGLRC hRC;
	DocTemplateBase* pDocTmpl;
	notify_func callbackFunc;

	GLParams(
		HDC _hDC,
		HGLRC _hRC,
		DocTemplateBase* _pDocTmpl,
		notify_func _callbackFunc = default_callback) :

		hDC(_hDC),
		hRC(_hRC),
		pDocTmpl(_pDocTmpl),
		callbackFunc(_callbackFunc)
	{}
}
GLParams;

class DocTemplateBase
{
public:

	struct ThreadParams
	{
		DocTemplateBase* pDocTmpl;
		std::string strFileName;
		void** pDoc;

		ThreadParams(DocTemplateBase* pDocTmpl, std::string strFileName, void** pDoc = NULL) :
			pDocTmpl(pDocTmpl), strFileName(strFileName), pDoc(pDoc)
		{}
	};

	virtual std::string GetExtensionString(void) const = 0;
	virtual std::string GetFileName(bool bFull = false) = 0;
	virtual std::string GetPathName(void) = 0;
	virtual error::code OpenDocument(std::string strFileName, ProgressMonitor* pMonitor) = 0;
	virtual error::code SaveDocument(std::string strFileName = _T("")) = 0;
	virtual error::code LoadDocument(void** pDoc, std::string strFileName, ProgressMonitor* pMonitor) = 0;
	virtual void SetDocument(void*& pDoc, std::string strFileName) = 0;
	virtual bool FileExists(void) = 0;
	virtual bool IsModified(void) = 0;
	virtual void UpdateViews(void) = 0;

	static unsigned _stdcall LoadDocumentThread(void* pArgs)
	{
		ProgressMonitor* pMonitor = static_cast<ProgressMonitor*>(pArgs);
		ThreadParams* pParams = (ThreadParams*) static_cast<CProgressDlg2*>(pMonitor)->GetArgs(); // Bad

		return pParams->pDocTmpl->LoadDocument(pParams->pDoc, pParams->strFileName, pMonitor);
	}

private:
	std::string m_strOpenFileName;
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

	virtual std::string GetExtensionString(void) const
	{
		return _T("");
	}

	std::string GetBaseName(void)
	{
		std::string strFileName = GetFileName();

		size_t pos = strFileName.rfind(_T('.'));
		
		if (pos != strFileName.npos)
			return strFileName.substr(0, pos);

		return strFileName;
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

	std::string GetPathName(void)
	{
		size_t pos = m_strFileName.rfind(_T('\\'));
		
		if (pos == m_strFileName.npos)
			pos = m_strFileName.rfind(_T('/'));

		if (pos != m_strFileName.npos)
			return m_strFileName.substr(0, ++pos);

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
	//	UpdateViews();
	}

	virtual error::code OpenDocument(std::string strFileName, ProgressMonitor* pMonitor)
	{
		TDoc* pDoc = new TDoc();

		error::code code = pDoc->read_file(strFileName.c_str(), pMonitor);

		if (code & error::ok)
		{
			if (m_pDoc) delete m_pDoc;

			m_pDoc        = pDoc;
			m_strFileName = strFileName;
			m_bModified   = false;
			m_bFileExists = true;

			m_pView->SetDocument(m_pDoc);
		//	UpdateViews();
		}
		else
		{
			delete pDoc;
		}

		return code;
	}

	virtual error::code LoadDocument(void** pDoc, std::string strFileName, ProgressMonitor* pMonitor)
	{
		*pDoc = new TDoc();
		error::code code = static_cast<TDoc*>(*pDoc)->read_file(strFileName.c_str(), pMonitor);
		pMonitor->done();
		return code;
	}

	void SetDocument(void*& pDoc, std::string strFileName)
	{
		if (m_pDoc == pDoc) return;

		if (m_pDoc) delete m_pDoc;

		m_pDoc        = static_cast<TDoc*>(pDoc);
		m_strFileName = strFileName;
		m_bModified   = false;
		m_bFileExists = true;

		ATLASSERT(pDoc != NULL);

		m_pView->SetDocument(m_pDoc);
	//	UpdateViews();
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
