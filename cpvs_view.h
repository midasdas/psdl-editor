#ifndef __CPVS_VIEW_H__
#define __CPVS_VIEW_H__

#include "docview.h"
#include "cpvs.h"

#include <gl\gl.h>
#include <gl\glut.h>

class CPVSView : public View<cpvs>
{
public:

	CPVSView(void) : m_pPSDL(NULL), iBlock(-1) { View<cpvs>(); }

	void RenderScene(HDC hDC, HGLRC hRC)
	{
		psdl::block* pBlock = m_pPSDL->get_block(iBlock);
		if (!pBlock) return;

		glColor3f(.0f, .0f, 1.f);

		glLineWidth(2.f);
		glBegin(GL_LINE_LOOP);

		for (unsigned long iPt = 0; iPt < pBlock->num_perimeters(); iPt++)
		{
			glVertex3fv(&(m_pPSDL->getVertex(pBlock->_perimeter[iPt].vertex).x));
		}

		glEnd();
		glLineWidth(1.f);
	}

	void SetPSDL(psdl* pPSDL)
	{
		m_pPSDL = pPSDL;
	}

	void SelectBlock(int iIndex)
	{
		iBlock = iIndex;
	}

private:

	psdl* m_pPSDL;
	int iBlock;
};

#endif
