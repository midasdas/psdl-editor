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

	std::string GetExtensionString(void) const
	{
		return _T("MM2 City Geometry (*.psdl; *.psd)\0*.psdl; *.psd\0");
	}

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

	void RenderScene(void)
	{
		m_pView->RenderScene();
	}

	void NewDocument(std::string sFileName = _T("untitled.psdl"))
	{
		baseClass::NewDocument(sFileName);
	}

	error::code Read3DS(std::string strFileName);

	error::code ReadSDL(std::string strFileName, ProgressMonitor* pMonitor)
	{
		error::code code = m_pDoc->read_sdl(strFileName.c_str(), pMonitor);

		if (code & error::ok)
		{
			code = m_pView->LoadTextures(pMonitor);
		}

		pMonitor->done();
		return code;
	}

	error::code WriteSDL(std::string strFileName, ProgressMonitor* pMonitor)
	{
		error::code code = m_pDoc->write_sdl(strFileName.c_str(), pMonitor);
	//	return m_pDoc->write_blockdata((strFileName + ".block").c_str());
		pMonitor->done();
		return code;
	}

	class ReadSDLThread
	{
	public:
		static PSDLDocTemplate* pDocTmpl;
		static std::string strFileName;

		static unsigned _stdcall Run(void* pArgs)
		{
			return pDocTmpl->ReadSDL(strFileName.c_str(), static_cast<ProgressMonitor*>(pArgs));
		}
	};

	class WriteSDLThread
	{
	public:
		static PSDLDocTemplate* pDocTmpl;
		static std::string strFileName;

		static unsigned _stdcall Run(void* pArgs)
		{
			return pDocTmpl->WriteSDL(strFileName.c_str(), static_cast<ProgressMonitor*>(pArgs));
		}
	};

	error::code UnloadTextures(HDC hDC, HGLRC hRC)
	{
		return m_pView->UnloadTextures(hDC, hRC);
	}

	error::code LoadTextures(HDC hDC, HGLRC hRC, ProgressMonitor* pMonitor = NULL)
	{
		return m_pView->LoadTextures(hDC, hRC, pMonitor);
	}

	error::code LoadDocument(void** pDoc, std::string strFileName, ProgressMonitor* pMonitor)
	{
	//	error::code code = baseClass::LoadDocument(pDoc, strFileName, pMonitor);

		*pDoc = new psdl();

		error::code code = static_cast<psdl*>(*pDoc)->read_file(strFileName.c_str(), pMonitor);

		if (code & error::ok)
		{
			SetDocument(*pDoc, strFileName);
			SetCurrentDirectory(GetPathName().c_str()); // API function
			code = m_pView->LoadTextures(pMonitor);
		}

		pMonitor->done();

		return code;
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

	void RotateVertex(Vertex *vTarget, Vertex vOrigin, double dAngle)
	{
		float fDx = vTarget->x - vOrigin.x;
		float fDz = vTarget->z - vOrigin.z;
		double s = sin(dAngle);
		double c = cos(dAngle);

		vTarget->x = vOrigin.x + fDx * c - fDz * s;
		vTarget->z = vOrigin.z + fDx * s + fDz * c;
	}

	void MoveVertex(Vertex *vTarget, Vertex vOffset)
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

		Vertex vCopy(*m_pDoc->get_vertex(nFrom));

		unsigned short nTo = m_pDoc->add_vertex(vCopy);

		vertexMap newMap = { nFrom, nTo };

		aLookup.push_back(newMap);

		return nTo;
	}

	unsigned short CopyVertex(unsigned short nFrom, Vertex vOffset, std::vector<vertexMap>& aLookup)
	{
		for (size_t i = 0; i < aLookup.size(); i++)
		{
			if (aLookup[i].from == nFrom)

				return aLookup[i].to;
		}

		Vertex vCopy(*m_pDoc->get_vertex(nFrom));

		Vertex vFixed(-940.3245f, 0, 1349.235f);
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
