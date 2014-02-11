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

	CBlocksWindow*		m_pBlocksWindow;
	CPerimeterWindow*	m_pPerimeterWindow;
	CAttributesWindow*	m_pAttribsWindow;
	CPropertiesWindow*	m_pPropsWindow;

public:

	void SetViews(	CBlocksWindow*		pBlocksWindow,
					CPerimeterWindow*	pPerimeterWindow,
					CAttributesWindow*	pAttribsWindow,
					CPropertiesWindow*	pPropsWindow)
	{
		m_pBlocksWindow		= pBlocksWindow;
		m_pPerimeterWindow	= pPerimeterWindow;
		m_pAttribsWindow	= pAttribsWindow;
		m_pPropsWindow		= pPropsWindow;
	}

	void UpdateViews(void)
	{
		m_pBlocksWindow->SetPSDL(m_pDoc);
		m_pPerimeterWindow->SetBlock(NULL);
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

	unsigned long NumBlocks(void)
	{
		return m_pDoc->num_blocks();
	}

	void InsertBlock(psdl::block& block, unsigned int nPos)
	{
		m_pDoc->insert_block(block, nPos);
		m_pBlocksWindow->InsertBlock(&block, nPos);
	}

	void AddBlock(psdl::block& block)
	{
		m_pDoc->add_block(block);
		m_pBlocksWindow->InsertBlock(&block);
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

	// --- Static Functions ---

	// Do these belong here?

	void RotateVertex(psdl::vertex *vTarget, psdl::vertex vOrigin, double dAngle)
	{
		vTarget->x = vOrigin.x + (vTarget->x - vOrigin.x) * cos(dAngle) - (vTarget->z - vOrigin.z) * sin(dAngle);
		vTarget->z = vOrigin.z + (vTarget->x - vOrigin.x) * sin(dAngle) + (vTarget->z - vOrigin.z) * cos(dAngle);
	}

	unsigned short CopyVertex(unsigned short nFrom, psdl::vertex vOffset, std::vector<vertexMap>& aLookup)
	{
		long nTo = -1;

		for (size_t i = 0; i < aLookup.size(); i++)
		{
			if (aLookup[i].from == nFrom)
			{
				nTo = aLookup[i].to;
			}
		}

		if (nTo < 0)
		{
			psdl::vertex vCopy = m_pDoc->getVertex(nFrom);

			psdl::vertex vFixed(-940.3245f, 0, 1349.235f);
			RotateVertex(&vCopy, vFixed, PI);

			vCopy.x += vOffset.x;
			vCopy.y += vOffset.y;
			vCopy.z += vOffset.z;

			nTo = m_pDoc->add_vertex(vCopy);

			vertexMap newMap = { nFrom, nTo };

			aLookup.push_back(newMap);
		}

		return nTo;
	}

	unsigned short CopyHeight(unsigned short iHeight)
	{
		return m_pDoc->add_height(iHeight);
	}
};

#endif
