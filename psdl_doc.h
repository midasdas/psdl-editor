#ifndef __PSDL_DOC_H__
#define __PSDL_DOC_H__

#include "docview.h"
#include "psdl.h"
#include "psdl_view.h"
#include "toolwnd.h"
#include "tools.h"

#include <math.h>

typedef struct
{
	unsigned short from, to;
}
vertexMap;

class PSDLDocTemplate : public DocTemplate<psdl, PSDLView>
{
private:

	typedef DocTemplate<psdl, PSDLView> baseClass;

	CBlocksWindow*     m_pBlocksWindow;
	CPerimeterWindow*  m_pPerimeterWindow;
	CAttributesWindow* m_pAttribsWindow;
	CPropertiesWindow* m_pPropsWindow;

public:

	void SetViews(	CBlocksWindow*     pBlocksWindow,
					CPerimeterWindow*  pPerimeterWindow,
					CAttributesWindow* pAttribsWindow,
					CPropertiesWindow* pPropsWindow)
	{
		m_pBlocksWindow    = pBlocksWindow;
		m_pPerimeterWindow = pPerimeterWindow;
		m_pAttribsWindow   = pAttribsWindow;
		m_pPropsWindow     = pPropsWindow;
	}

	void UpdateViews(void)
	{
		ATLASSERT(m_pDoc != NULL);
		m_pBlocksWindow->SetPSDL(m_pDoc);
		m_pPerimeterWindow->SetBlock(NULL);
		m_pAttribsWindow->SetBlock(NULL);
		m_pPropsWindow->SetPSDL(m_pDoc);
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

	error::code Read3DS(std::string strFileName);

	error::code ReadSDL(std::string strFileName)
	{
		return m_pDoc->read_sdl(strFileName.c_str());
	}

	error::code WriteSDL(std::string strFileName)
	{
		return m_pDoc->write_sdl(strFileName.c_str());
	}

	error::code UnloadTextures(HDC hDC, HGLRC hRC)
	{
		return m_pView->UnloadTextures(hDC, hRC);
	}

	error::code LoadTextures(HDC hDC, HGLRC hRC, notify_func callbackFunc = default_callback)
	{
		return m_pView->LoadTextures(hDC, hRC, callbackFunc);
	}

	static unsigned _stdcall _LoadTextures(void* pThreadData)
	{
		ThreadData* pData = static_cast<ThreadData*>(pThreadData);

		if (GLParams* pParams = static_cast<GLParams*>(pData->pParams))
		{
			return static_cast<PSDLDocTemplate*>(pParams->pDocTmpl)->LoadTextures(pParams->hDC, pParams->hRC, pData->callbackFunc);
		}

		return error::failure;
	}

	// --- Document Operations ---

	unsigned long NumBlocks(void)
	{
		return m_pDoc->num_blocks();
	}

	void InsertBlock(psdl::block* pBlock, int iPos)
	{
		m_pDoc->insert_block(pBlock, iPos);
		m_pBlocksWindow->InsertItem(iPos);
	}

	void AddBlock(psdl::block* block)
	{
		m_pDoc->add_block(block);
		m_pBlocksWindow->InsertItem();
	}

	psdl::block* GetBlock(unsigned long iIndex)
	{
		return m_pDoc->get_block(iIndex);
	}

	// --- View Operations ---

	void SelectBlock(long iIndex)
	{
		m_pPerimeterWindow->SetBlock(m_pDoc->get_block(iIndex));
		m_pAttribsWindow->SetBlock(m_pDoc->get_block(iIndex));
	}

	// Do these belong here?

	void RotateVertex(psdl::vertex *vTarget, psdl::vertex vOrigin, double dAngle)
	{
		float fDx = vTarget->x - vOrigin.x;
		float fDz = vTarget->z - vOrigin.z;
		double s = sin(dAngle);
		double c = cos(dAngle);

		vTarget->x = vOrigin.x + fDx * c - fDz * s;
		vTarget->z = vOrigin.z + fDx * s + fDz * c;
	}

	void MoveVertex(psdl::vertex *vTarget, psdl::vertex vOffset)
	{
		vTarget->x += vOffset.x;
		vTarget->y += vOffset.y;
		vTarget->z += vOffset.z;
	}

	unsigned short CopyVertex(unsigned short nFrom, std::vector<vertexMap>& aLookup)
	{
		for (size_t i = 0; i < aLookup.size(); i++)
		{
			if (aLookup[i].from == nFrom)

				return aLookup[i].to;
		}

		psdl::vertex vCopy(*m_pDoc->get_vertex(nFrom));

		unsigned short nTo = m_pDoc->add_vertex(vCopy);

		vertexMap newMap = { nFrom, nTo };

		aLookup.push_back(newMap);

		return nTo;
	}

	unsigned short CopyVertex(unsigned short nFrom, psdl::vertex vOffset, std::vector<vertexMap>& aLookup)
	{
		for (size_t i = 0; i < aLookup.size(); i++)
		{
			if (aLookup[i].from == nFrom)

				return aLookup[i].to;
		}

		psdl::vertex vCopy(*m_pDoc->get_vertex(nFrom));

		psdl::vertex vFixed(-940.3245f, 0, 1349.235f);
		RotateVertex(&vCopy, vFixed, PI);

		vCopy.x += vOffset.x;
		vCopy.y += vOffset.y;
		vCopy.z += vOffset.z;

		unsigned short nTo = m_pDoc->add_vertex(vCopy);

		vertexMap newMap = { nFrom, nTo };

		aLookup.push_back(newMap);

		return nTo;
	}

	unsigned short CopyHeight(unsigned short iHeight)
	{
		return m_pDoc->add_height(iHeight);
	}
};

#endif
