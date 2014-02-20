#include "stdafx.h"
#include "psdl_view.h"

void PSDLView::RenderScene(HDC hDC, HGLRC hRC)
{
	for (unsigned long i = 0; i < m_pDoc->num_blocks(); i++)
	{
		psdl::block* pBlock = m_pDoc->get_block(i);

		psdl::texture*  pTexAtb = NULL;
		psdl::tunnel*   pTunnelAtb = NULL;
		psdl::junction* pJunctionAtb = NULL;

		for (unsigned long j = 0; j < pBlock->num_attributes(); j++)
		{
			psdl::attribute* pAtb = pBlock->get_attribute(j);

			switch (pAtb->type)
			{
				case ATB_ROAD:
					{
						glColor3f(.4f, .4f, .4f);

						unsigned short k;
						psdl::road_strip* road = static_cast<psdl::road_strip*>(pAtb);

						for (k = 0; k < road->num_sections() - 1; k++)
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
						}

						if (pTunnelAtb)
						{
							glColor3f(.55f, .5f, .6f);

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

				case ATB_TUNNEL:
					pTunnelAtb = static_cast<psdl::tunnel*>(pAtb);
					break;
			}
		}
	}
}
