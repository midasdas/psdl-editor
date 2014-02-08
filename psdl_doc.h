#ifndef __PSDL_DOC_H__
#define __PSDL_DOC_H__

#include "docview.h"
#include "psdl.h"
#include "psdl_view.h"
#include "toolwnd.h"

class PSDLDocTemplate : public DocTemplate<PSDL, PSDLView>
{
private:

	typedef DocTemplate<PSDL, PSDLView> baseClass;

	CBlocksWindow*		m_pBlocksWindow;
	CAttributesWindow*	m_pAttribsWindow;
	CPropertiesWindow*	m_pPropsWindow;

public:

	void SetViews(	CBlocksWindow*		pBlocksWindow,
					CAttributesWindow*	pAttribsWindow,
					CPropertiesWindow*	pPropsWindow)
	{
		m_pBlocksWindow		= pBlocksWindow;
		m_pAttribsWindow	= pAttribsWindow;
		m_pPropsWindow		= pPropsWindow;
	}

	void UpdateViews(void)
	{
		m_pBlocksWindow->SetPSDL(m_pDoc);
		m_pAttribsWindow->SetBlock(NULL);
		m_pPropsWindow->SetAttribute(NULL);
	}

	void RenderScene(HDC hDC, HGLRC hRC)
	{
		m_pView->RenderScene(hDC, hRC);
	}

	void NewDocument(std::string sFileName = _T("untitled.psdl"))
	{
		baseClass::NewDocument(sFileName);
	}

	// --- Document Operations ---

	void InsertBlock(PSDL::Block block, unsigned int nPos)
	{
		m_pDoc->insertBlock(block, nPos);
		m_pBlocksWindow->InsertBlock(&block, nPos);
	}

	// --- View Operations ---

	void SelectBlock(int iIndex)
	{
		m_pAttribsWindow->SetBlock(m_pDoc->getBlock(iIndex));
	}
};

#endif
