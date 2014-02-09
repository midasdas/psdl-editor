#ifndef __PSDL_VIEW_H__
#define __PSDL_VIEW_H__

#include "docview.h"
#include "psdl.h"

#include <gl\gl.h>
#include <gl\glut.h>

class PSDLView : public View<psdl>
{
public:
	void v(size_t iVert)
	{
		glVertex3fv(&(m_pDoc->getVertex(iVert).x));
	}

	void RenderScene(HDC hDC, HGLRC hRC)
	{
		for (size_t i = 0; i < m_pDoc->numBlocks(); i++)
		{
			psdl::block* block = m_pDoc->get_block(i);

			for (size_t j = 0; j < block->numAttributes(); j++)
			{
				psdl::attribute* atb = block->get_attribute(j);

				switch (atb->type)
				{
					case ATB_ROAD:
						glColor3f(.5f, .5f, .5f);
						{
							psdl::road_strip* road = static_cast<psdl::road_strip*>(atb);

							for (size_t k = 0; k < road->num_sections() - 1; k++)
							{
								glBegin(GL_QUAD_STRIP);
									v(road->get_vertex_ref(k * 4 + 0));
									v(road->get_vertex_ref(k * 4 + 4));

									v(road->get_vertex_ref(k * 4 + 1));
									v(road->get_vertex_ref(k * 4 + 5));

									v(road->get_vertex_ref(k * 4 + 2));
									v(road->get_vertex_ref(k * 4 + 6));

									v(road->get_vertex_ref(k * 4 + 3));
									v(road->get_vertex_ref(k * 4 + 7));
								glEnd();
							}
						}
						break;

				}
			}
		}


/*		glPushMatrix();

		glTranslatef(100.f, 0.f, 0.f);
		glColor3f(0.f, 0.f, 1.f);
		glutWireCube(50.f);

		glPopMatrix();

		ATLTRACE("\nPSDL has %d blocks", m_pDoc->numBlocks());
*/	}
};

#endif
