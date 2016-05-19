#ifndef __CPVS_DOC_H__
#define __CPVS_DOC_H__

#include "docview.h"
#include "cpvs.h"
#include "cpvs_view.h"
#include "psdl.h"
#include "toolwnd.h"

class CPVSDocTemplate : public DocTemplate<cpvs, CPVSView>
{
private:

	typedef DocTemplate<cpvs, CPVSView> baseClass;
	CBlocksWindow* m_pBlocksWindow;
	psdl* m_pPSDL;

public:

	CPVSDocTemplate(void) : m_pPSDL(NULL) { baseClass(); }

	std::string GetExtensionString(void) const
	{
		return _T("Potentially Visible Sets (*.cpvs)\0*.cpvs\0");
	}

	void SetViews(CBlocksWindow* pBlocksWindow)
	{
		m_pBlocksWindow = pBlocksWindow;
	}

	void SetPSDL(psdl* pPSDL)
	{
		m_pPSDL = pPSDL;
		m_pView->SetPSDL(m_pPSDL);
	}

	void RenderScene(void)
	{
		m_pView->RenderScene();
	}

	void NewDocument(std::string sFileName = _T("untitled.cpvs"))
	{
		baseClass::NewDocument(sFileName);
	}

	error::code OpenDocument(std::string strFileName, ProgressMonitor* pMonitor)
	{
		cpvs* pDoc = new cpvs();

		error::code code = pDoc->read_file(strFileName.c_str(), pMonitor, m_pPSDL);

	//	callbackFunc(_T(""), 100);

		if (code & error::ok)
		{
			if (m_pDoc) delete m_pDoc;

			m_pDoc        = pDoc;
			m_strFileName = strFileName;
			m_bModified   = false;
			m_bFileExists = true;

			m_pView->SetDocument(m_pDoc);
		}
		else
		{
			delete pDoc;
		}

		return code;
	}

	// --- View Operations ---

	void SelectBlock(long iIndex)
	{
		m_pView->SelectBlock(iIndex);
	}
};

#endif
