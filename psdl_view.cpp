#include "stdafx.h"
#include "psdl_view.h"
#include "tools.h"

#include "gltools.h"

#define GL_CLAMP_TO_EDGE   0x812F
#define GL_MIRRORED_REPEAT 0x8370

#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF

error::code PSDLView::LoadTextures(HDC hDC, HGLRC hRC, notify_func callbackFunc)
{
	callbackFunc("Loading textures...", 0);

	unsigned long n_tex = m_pDoc->num_textures();

	GLint iWidth, iHeight, iComponents;
	GLenum eFormat;
	GLbyte *pBytes;

	wglMakeCurrent(hDC, hRC);

	m_textures.resize(n_tex);
	glGenTextures(n_tex, &m_textures[0]);

//	const CString sAnim = "-0007";
	const CString sExt = ".tga";

	for (unsigned long i = 0; i < n_tex; )
	{
		if (!m_pDoc->get_texname(i).empty())
		{
			CString sTexName = (CString) "../texture/" + m_pDoc->get_texname(i).c_str();

		//	if (GetFileAttributes(sTexName + sAnim + sExt) != INVALID_FILE_ATTRIBUTES)
		//		sTexName += sAnim;

			sTexName += sExt;

			if (pBytes = gltLoadTGA(sTexName, &iWidth, &iHeight, &iComponents, &eFormat))
			{
				glBindTexture(GL_TEXTURE_2D, m_textures[i]);

				glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight,
											0, eFormat, GL_UNSIGNED_BYTE, pBytes);
				free(pBytes);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			}
			else
				ATLTRACE("Cannot load %s\n", sTexName);
		}

		callbackFunc("", 100 * ++i / n_tex);
	}

	wglMakeCurrent(NULL, NULL);

	callbackFunc("", 100);

	return error::ok;
}

error::code PSDLView::UnloadTextures(HDC hDC, HGLRC hRC)
{
	glDeleteTextures(m_pDoc->num_textures(), &m_textures[0]);
	return error::ok;
}

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

		unsigned long iTex = 0;

	//	glColor3f(1, 1, 1);

		for (unsigned long j = 0; j < pBlock->num_attributes(); j++)
		{
			psdl::attribute* pAtb = pBlock->get_attribute(j);

			switch (pAtb->type)
			{

	case ATB_ROAD:
		{
			unsigned short k;
			psdl::road_strip* road = static_cast<psdl::road_strip*>(pAtb);

		//	GLfloat dist = psdl::d2v();

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

			glBindTexture(GL_TEXTURE_2D, m_textures[iTex + 1]);

			glBegin(GL_TRIANGLE_STRIP);
				for (k = 0; k < road->num_vertices(); k += 4)
				{
					psdl::vertex vert = *m_pDoc->get_vertex(road->get_vertex_ref(k + 1));
					vert.y += SW_H;
					glTexCoord2f(k, 0); glVertex3fv(&(m_pDoc->get_vertex(road->get_vertex_ref(k))->x));
					glTexCoord2f(k, 1); glVertex3fv(&(vert.x));
				}
			glEnd();

			glBegin(GL_TRIANGLE_STRIP);
				for (k = 0; k < road->num_vertices(); k += 4)
				{
					psdl::vertex vert = *m_pDoc->get_vertex(road->get_vertex_ref(k + 1));
					glTexCoord2f(k, 1); glVertex3f(vert.x, vert.y + SW_H, vert.z);
					glTexCoord2f(k, 1); glVertex3fv(&(vert.x));
				}
			glEnd();

			glBegin(GL_TRIANGLE_STRIP);
				for (k = 0; k < road->num_vertices(); k += 4)
				{
					psdl::vertex vert = *m_pDoc->get_vertex(road->get_vertex_ref(k + 2));
					glTexCoord2f(k,-1); glVertex3fv(&(vert.x));
					glTexCoord2f(k,-1); glVertex3f(vert.x, vert.y + SW_H, vert.z);
				}
			glEnd();

			glBegin(GL_TRIANGLE_STRIP);
				for (k = 0; k < road->num_vertices(); k += 4)
				{
					psdl::vertex vert = *m_pDoc->get_vertex(road->get_vertex_ref(k + 2));
					vert.y += SW_H;
					glTexCoord2f(k,-1); glVertex3fv(&(vert.x));
					glTexCoord2f(k, 0); glVertex3fv(&(m_pDoc->get_vertex(road->get_vertex_ref(k + 3))->x));
				}
			glEnd();

			glBindTexture(GL_TEXTURE_2D, m_textures[iTex]);

			glBegin(GL_TRIANGLE_STRIP);
				for (k = 0; k < road->num_sections(); k++)
				{
					glTexCoord2f(k, 0); glVertex3fv(&(m_pDoc->get_vertex(road->get_vertex_ref(k * 4 + 1))->x));
					glTexCoord2f(k, 2); glVertex3fv(&(m_pDoc->get_vertex(road->get_vertex_ref(k * 4 + 2))->x));
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
		{
			psdl::vertex_based* pTypeAtb = static_cast<psdl::vertex_based*>(pAtb);
			psdl::vertex v;
			unsigned short k;
			bool curb = false;

			glBindTexture(GL_TEXTURE_2D, m_textures[iTex + 1]);

			unsigned short f1 = pTypeAtb->get_vertex_ref(0),
			               f2 = pTypeAtb->get_vertex_ref(1);

			if (f1 == f2 && f1 <= 1)
			{
				if (f1 == 0) glFrontFace(GL_CW);
				glBegin(GL_TRIANGLES);
					v = *m_pDoc->get_vertex(pTypeAtb->get_vertex_ref(2));
					glTexCoord2f(v.x/5, v.z/5); glVertex3f(v.x, v.y + SW_H, v.z);
					glTexCoord2f(v.x/5, v.z/5); glVertex3fv(&(v.x));
					v = *m_pDoc->get_vertex(pTypeAtb->get_vertex_ref(3));
					glTexCoord2f(v.x/5, v.z/5); glVertex3fv(&(v.x));
				glEnd();
				glFrontFace(GL_CCW);
			}
			else
			{
				glBegin(GL_TRIANGLE_STRIP);
				for (k = 0; k < pTypeAtb->num_vertices(); k++)
				{
					v = *m_pDoc->get_vertex(pTypeAtb->get_vertex_ref(k));
					if (curb ^= 1) v.y += SW_H;
					glTexCoord2f(v.x/5, v.z/5); glVertex3fv(&(v.x));
				}
				glEnd();

				glBegin(GL_TRIANGLE_STRIP);
				for (k = 0; k < pTypeAtb->num_vertices(); k += 2)
				{
					v = *m_pDoc->get_vertex(pTypeAtb->get_vertex_ref(k));
					glTexCoord2f(v.x/5, v.z/5);
					glVertex3fv(&(v.x));
					glVertex3f(v.x, v.y + SW_H, v.z);
				}
				glEnd();
			}
		}
		break;

	case ATB_RECTANGLE:
		{
			psdl::vertex_based* pTAtb = static_cast<psdl::vertex_based*>(pAtb);

			glBegin(GL_TRIANGLE_STRIP);
				for (unsigned short k = 0; k < pTAtb->num_vertices(); k+= 2)
				{
					glTexCoord2f(k, 0); glVertex3fv(&(m_pDoc->get_vertex(pTAtb->get_vertex_ref(k))->x));
					glTexCoord2f(k, 1); glVertex3fv(&(m_pDoc->get_vertex(pTAtb->get_vertex_ref(k + 1))->x));
				}
			glEnd();
		}
		break;

	case ATB_SLIVER:
		{
			psdl::sliver* pTAtb = static_cast<psdl::sliver*>(pAtb);

			glBegin(GL_QUADS);

GLfloat scale = pTAtb->tex_scale;
glBindTexture(GL_TEXTURE_2D, m_textures[iTex]);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

glTexCoord2f( (m_pDoc->get_vertex(pTAtb->right)->x - m_pDoc->get_vertex(pTAtb->left)->x) / scale, 0);
glVertex3f(m_pDoc->get_vertex(pTAtb->right)->x, m_pDoc->get_height(pTAtb->top), m_pDoc->get_vertex(pTAtb->right)->z);

glTexCoord2f(0, 0);
glVertex3f(m_pDoc->get_vertex(pTAtb->left)->x,  m_pDoc->get_height(pTAtb->top), m_pDoc->get_vertex(pTAtb->left)->z);

glTexCoord2f(0, (m_pDoc->get_vertex(pTAtb->left)->y - m_pDoc->get_height(pTAtb->top)) / scale);
glVertex3fv(&(m_pDoc->get_vertex(pTAtb->left)->x));

glTexCoord2f( (m_pDoc->get_vertex(pTAtb->right)->x - m_pDoc->get_vertex(pTAtb->left)->x) / scale, (m_pDoc->get_vertex(pTAtb->right)->y - m_pDoc->get_height(pTAtb->top)) / scale);
glVertex3fv(&(m_pDoc->get_vertex(pTAtb->right)->x));

			glEnd();
		}
		break;

	case ATB_CROSSWALK:
		{
			psdl::crosswalk* pTAtb = static_cast<psdl::crosswalk*>(pAtb);

			glBindTexture(GL_TEXTURE_2D, m_textures[iTex + 2]);
			glBegin(GL_TRIANGLE_STRIP);
				glTexCoord2f(0, 0); glVertex3fv( &(m_pDoc->get_vertex(pTAtb->get_vertex_ref(1))->x) );
				glTexCoord2f(1, 0); glVertex3fv( &(m_pDoc->get_vertex(pTAtb->get_vertex_ref(0))->x) );
				glTexCoord2f(0, 1); glVertex3fv( &(m_pDoc->get_vertex(pTAtb->get_vertex_ref(3))->x) );
				glTexCoord2f(1, 1); glVertex3fv( &(m_pDoc->get_vertex(pTAtb->get_vertex_ref(2))->x) );
			glEnd();
		}
		break;

	case ATB_TRIANGLEFAN:
	case ATB_ROADTRIANGLEFAN:
		{
			psdl::vertex_based* pTypeAtb = static_cast<psdl::vertex_based*>(pAtb);
			psdl::vertex* v;


			glBindTexture(GL_TEXTURE_2D, m_textures[iTex]);
			glBegin(GL_TRIANGLE_FAN);
			for (unsigned short k = 0; k < pTypeAtb->num_vertices(); k++)
			{
				v = m_pDoc->get_vertex(pTypeAtb->get_vertex_ref(k));
				glTexCoord2f(v->x/10, v->z/10); glVertex3fv(&(v->x));
			}
			glEnd();
		}
		break;

	case ATB_ROOFTRIANGLEFAN:
		{
			psdl::roof_triangle_fan* pTypeAtb = static_cast<psdl::roof_triangle_fan*>(pAtb);
			psdl::vertex v;

			glBegin(GL_TRIANGLE_FAN);
				for (unsigned short k = 0; k < pTypeAtb->num_vertices(); k++)
				{
					v = *m_pDoc->get_vertex(pTypeAtb->get_vertex_ref(k));
					v.y = m_pDoc->get_height(pTypeAtb->i_height);
					glTexCoord2f(v.x/10, v.z/10); glVertex3fv(&(v.x));
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
	//	glColorRandom(nHue);
		{
			unsigned long iTexture = static_cast<psdl::texture*>(pAtb)->i_texture;
			if (iTexture < m_textures.size())
			{
				glBindTexture(GL_TEXTURE_2D, m_textures[iTexture]);
				iTex = iTexture;
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, 0);
				iTex = 0;
			}
		}
		break;

	case ATB_FACADE:
		{
			psdl::facade* pTAtb = static_cast<psdl::facade*>(pAtb);

			glBegin(GL_QUADS);

glTexCoord2f(0, 0);                             glVertex3f(m_pDoc->get_vertex(pTAtb->left)->x,  m_pDoc->get_height(pTAtb->bottom), m_pDoc->get_vertex(pTAtb->left)->z);
glTexCoord2f(pTAtb->u_repeat, 0);               glVertex3f(m_pDoc->get_vertex(pTAtb->right)->x, m_pDoc->get_height(pTAtb->bottom), m_pDoc->get_vertex(pTAtb->right)->z);
glTexCoord2f(pTAtb->u_repeat, pTAtb->v_repeat); glVertex3f(m_pDoc->get_vertex(pTAtb->right)->x, m_pDoc->get_height(pTAtb->top),    m_pDoc->get_vertex(pTAtb->right)->z);
glTexCoord2f(0, pTAtb->v_repeat);               glVertex3f(m_pDoc->get_vertex(pTAtb->left)->x,  m_pDoc->get_height(pTAtb->top),    m_pDoc->get_vertex(pTAtb->left)->z);

			glEnd();
		}
		break;

			}
		}
	}
}
