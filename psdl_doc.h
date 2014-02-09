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

	void AddBlock(PSDL::Block block)
	{
		m_pDoc->addBlock(block);
		m_pBlocksWindow->InsertBlock(&block);
	}

	PSDL::Block* GetBlock(unsigned long iIndex)
	{
		return m_pDoc->getBlock(iIndex);
	}

	// --- View Operations ---

	void SelectBlock(int iIndex)
	{
		m_pAttribsWindow->SetBlock(m_pDoc->getBlock(iIndex));
	}

	// --- Static Functions ---

	// Do these belong here?

	void RotateVertex(Vertex *vTarget, Vertex vOrigin, double dAngle)
	{
		vTarget->x = vOrigin.x + (vTarget->x - vOrigin.x) * cos(dAngle) - (vTarget->z - vOrigin.z) * sin(dAngle);
		vTarget->z = vOrigin.z + (vTarget->x - vOrigin.x) * sin(dAngle) + (vTarget->z - vOrigin.z) * cos(dAngle);
	}

	unsigned short CopyVertex(std::vector<vertexMap>* aLookup, unsigned short nFrom, Vertex vOffset)
	{
		long nTo = -1;

		for (size_t i = 0; i < aLookup->size(); i++)
		{
			if ((*aLookup)[i].from == nFrom)
			{
				nTo = (*aLookup)[i].to;
			}
		}

		if (nTo < 0)
		{
			Vertex vCopy = m_pDoc->getVertex(nFrom);

			Vertex vFixed = { -940.3245f, 0, 1349.235f };
			RotateVertex(&vCopy, vFixed, PI);

			vCopy.x += vOffset.x;
			vCopy.y += vOffset.y;
			vCopy.z += vOffset.z;

			nTo = m_pDoc->addVertex(vCopy);

			vertexMap newMap = { nFrom, nTo };

			aLookup->push_back(newMap);
		}

		return nTo;
	}
};

#endif
