#ifndef __DOCVIEW_H__
#define __DOCVIEW_H__

#include "io_error.h"
#include "thread.h"
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

	virtual std::string GetFileName(bool bFull = false) = 0;
	virtual std::string GetPathName(void) = 0;
	virtual error::code OpenDocument(std::string strFileName, notify_func callbackFunc = default_callback) = 0;
	virtual error::code SaveDocument(std::string strFileName = _T("")) = 0;
	virtual error::code LoadDocument(void*& pDoc, std::string strFileName, notify_func callbackFunc = default_callback) = 0;
	virtual void SetDocument(void*& pDoc, std::string strFileName) = 0;
	virtual bool FileExists(void) = 0;
	virtual bool IsModified(void) = 0;
	virtual void UpdateViews(void) = 0;

	static unsigned _stdcall _LoadDocumentThread(void* pThreadData)
	{
		ThreadData* pData = static_cast<ThreadData*>(pThreadData);

		if (IOParams* pPio = static_cast<IOParams*>(pData->pParams))
		{
			return pPio->pDocTmpl->LoadDocument(pPio->pDoc, pPio->strFileName, pData->callbackFunc);
		}

		return error::failure;
	}

	static unsigned _stdcall _OpenDocumentThread(void* pThreadData)
	{
		ThreadData* pData = static_cast<ThreadData*>(pThreadData);

		if (IOParams* pPio = static_cast<IOParams*>(pData->pParams))
		{
			return pPio->pDocTmpl->OpenDocument(pPio->strFileName, pData->callbackFunc);
		}

		return error::failure;
	}

	static unsigned _OpenDocument(void* pParams)
	{
		if (IOParams* pPio = reinterpret_cast<IOParams*>(pParams))
		{
			return pPio->pDocTmpl->OpenDocument(pPio->strFileName, pPio->callbackFunc);
		}

		return error::failure;
	}
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

	virtual error::code OpenDocument(std::string strFileName, notify_func callbackFunc = default_callback)
	{
		TDoc* pDoc = new TDoc();

		error::code code = pDoc->read_file(strFileName.c_str(), callbackFunc);

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

		callbackFunc(_T(""), 100);

		return code;
	}

	virtual error::code LoadDocument(void*& pDoc, std::string strFileName, notify_func callbackFunc = default_callback)
	{
		pDoc = new TDoc();

		error::code code = static_cast<TDoc*>(pDoc)->read_file(strFileName.c_str(), callbackFunc);

		callbackFunc(_T(""), 100);

		return code;
	}

	void SetDocument(void*& pDoc, std::string strFileName)
	{
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
