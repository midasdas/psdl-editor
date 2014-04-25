#include "stdafx.h"
#include "psdl_view.h"
#include "tools.h"

void PSDLView::RenderScene(HDC hDC, HGLRC hRC)
{
	unsigned short nHue = 0;

	for (unsigned long i = 0; i < m_pDoc->num_blocks(); i++)
	{
		psdl::block* pBlock = m_pDoc->get_block(i);

		if (!pBlock->enabled) continue;

		psdl::texture*  pTexAtb = NULL;
		psdl::tunnel*   pTunnelAtb = NULL;
		psdl::junction* pJunctionAtb = NULL;

		glColor3f(1, 1, 1);

		for (unsigned long j = 0; j < pBlock->num_attributes(); j++)
		{
			psdl::attribute* pAtb = pBlock->get_attribute(j);

			switch (pAtb->type)
			{
				case ATB_ROAD:
					{
						unsigned short k;
						psdl::road_strip* road = static_cast<psdl::road_strip*>(pAtb);

					/*	for (k = 0; k < road->num_sections() - 1; k++)
						{
							glBegin(GL_QUAD_STRIP);
								v(road->get_vertex_ref(k * 4 + 0));
								v(road->get_vertex_ref(k * 4 + 4));

								vdy(road->get_vertex_ref(k * 4 + 1), SW_H);
								vdy(road->get_vertex_ref(k * 4 + 5), SW_H);
								v(road->get_vertex_ref(k * 4 + 1));
								v(road->get_vertex_ref(k * 4 + 5));

								v(road->get_vertex_ref(k * 4 + 2));
								v(road->get_vertex_ref(k * 4 + 6));
								vdy(road->get_vertex_ref(k * 4 + 2), SW_H);
								vdy(road->get_vertex_ref(k * 4 + 6), SW_H);

								v(road->get_vertex_ref(k * 4 + 3));
								v(road->get_vertex_ref(k * 4 + 7));
							glEnd();
						}*/

						glBegin(GL_TRIANGLE_STRIP);
							for (k = 0; k < road->num_vertices(); k += 4)
							{
								psdl::vertex vert = *m_pDoc->get_vertex(road->get_vertex_ref(k + 1));
								vert.y += SW_H;
								glVertex3fv(&(m_pDoc->get_vertex(road->get_vertex_ref(k))->x));
								glVertex3fv(&(vert.x));
							}
						glEnd();

						glBegin(GL_TRIANGLE_STRIP);
							for (k = 0; k < road->num_vertices(); k += 4)
							{
								psdl::vertex vert = *m_pDoc->get_vertex(road->get_vertex_ref(k + 1));
								glVertex3f(vert.x, vert.y + SW_H, vert.z);
								glVertex3fv(&(vert.x));
							}
						glEnd();

						glBegin(GL_TRIANGLE_STRIP);
							for (k = 0; k < road->num_vertices(); k += 4)
							{
								glVertex3fv(&(m_pDoc->get_vertex(road->get_vertex_ref(k + 1))->x));
								glVertex3fv(&(m_pDoc->get_vertex(road->get_vertex_ref(k + 2))->x));
							}
						glEnd();

						glBegin(GL_TRIANGLE_STRIP);
							for (k = 0; k < road->num_vertices(); k += 4)
							{
								psdl::vertex vert = *m_pDoc->get_vertex(road->get_vertex_ref(k + 2));
								glVertex3fv(&(vert.x));
								glVertex3f(vert.x, vert.y + SW_H, vert.z);
							}
						glEnd();

						glBegin(GL_TRIANGLE_STRIP);
							for (k = 0; k < road->num_vertices(); k += 4)
							{
								psdl::vertex vert = *m_pDoc->get_vertex(road->get_vertex_ref(k + 2));
								vert.y += SW_H;
								glVertex3fv(&(vert.x));
								glVertex3fv(&(m_pDoc->get_vertex(road->get_vertex_ref(k + 3))->x));
							}
						glEnd();

						if (pTunnelAtb)
						{
							for (k = 0; k < road->num_sections() - 1; k++)
							{
								glBegin(GL_QUAD_STRIP);
									v(road->get_vertex_ref(k * 4 + 3));
									v(road->get_vertex_ref(k * 4 + 7));

									vdy(road->get_vertex_ref(k * 4 + 3), (GLfloat) pTunnelAtb->height1);
									vdy(road->get_vertex_ref(k * 4 + 7), (GLfloat) pTunnelAtb->height1);

									vdy(road->get_vertex_ref(k * 4 + 0), (GLfloat) pTunnelAtb->height1);
									vdy(road->get_vertex_ref(k * 4 + 4), (GLfloat) pTunnelAtb->height1);

									v(road->get_vertex_ref(k * 4 + 0));
									v(road->get_vertex_ref(k * 4 + 4));
								glEnd();
							}
						}
					}
					break;

				case ATB_SIDEWALK:
					break;

				case ATB_RECTANGLE:
					break;

				case ATB_SLIVER:
					break;

				case ATB_CROSSWALK:
					break;

				case ATB_TRIANGLEFAN:

				case ATB_ROADTRIANGLEFAN:
					{
						psdl::vertex_based* pTypeAtb = static_cast<psdl::vertex_based*>(pAtb);

						glBegin(GL_TRIANGLE_FAN);
							for (unsigned short k = 0; k < pTypeAtb->num_vertices(); k++)
								glVertex3fv(&(m_pDoc->get_vertex(pTypeAtb->get_vertex_ref(k))->x));
						glEnd();
					}
					break;

				case ATB_ROOFTRIANGLEFAN:
					{
						psdl::roof_triangle_fan* pTypeAtb = static_cast<psdl::roof_triangle_fan*>(pAtb);

						glBegin(GL_TRIANGLE_FAN);
							for (unsigned short k = 0; k < pTypeAtb->num_vertices(); k++)
							{
								psdl::vertex vert = *m_pDoc->get_vertex(pTypeAtb->get_vertex_ref(k));
								vert.y = m_pDoc->get_height(pTypeAtb->i_height);
								glVertex3fv(&(vert.x));
							}
						glEnd();
					}
					break;

				case ATB_DIVIDEDROAD:
					break;

				case ATB_TUNNEL:
					pTunnelAtb = static_cast<psdl::tunnel*>(pAtb);
					break;

				case ATB_TEXTURE:
					glColorRandom(nHue);
					break;

				case ATB_FACADE:
					break;
			}
		}
	}
}
