#ifndef __CPVS_DOC_H__
#define __CPVS_DOC_H__

#include "docview.h"
#include "cpvs.h"
#include "cpvs_view.h"
#include "toolwnd.h"

class CPVSDocTemplate : public DocTemplate<CPVS, CPVSView>
{
private:

	typedef DocTemplate<CPVS, CPVSView> baseClass;

	CBlocksWindow*		m_pBlocksWindow;

public:

	void SetViews(CBlocksWindow* pBlocksWindow)
	{
		m_pBlocksWindow = pBlocksWindow;
	}

	void RenderScene(HDC hDC, HGLRC hRC)
	{
		m_pView->RenderScene(hDC, hRC);
	}

	void NewDocument(std::string sFileName = _T("untitled.cpvs"))
	{
		baseClass::NewDocument(sFileName);
	}
};

#endif
