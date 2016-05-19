#include "stdafx.h"
#include "psdl_view.h"
#include "tools.h"
#include "config.h"
#include "mainfrm.h"

#include "gltools.h"

#include <vector>

#define GL_CLAMP_TO_EDGE   0x812F
#define GL_MIRRORED_REPEAT 0x8370

using namespace std;

error::code PSDLView::LoadTextures(ProgressMonitor* pMonitor)
{
	HDC hDC = CMainFrame::GetView()->GetDC();
	HGLRC hRC = CMainFrame::GetView()->GetRC();
	return LoadTextures(hDC, hRC, pMonitor);
}

error::code PSDLView::LoadTextures(HDC hDC, HGLRC hRC, ProgressMonitor* pMonitor)
{
//	callbackFunc("Loading textures...", 0);
	pMonitor->setNote("Loading textures...");

	unsigned long n_tex = m_pDoc->num_textures();
	pMonitor->setMaximum(n_tex);

	GLint iWidth, iHeight, iComponents;
	GLenum eFormat;
	GLbyte *pBytes;

	wglMakeCurrent(hDC, hRC);

//	m_textures.resize(n_tex + 1);
//	m_textures[0] = 0;
//	glGenTextures(n_tex, &m_textures[1]);
	m_textures.resize(n_tex);
	glGenTextures(n_tex, &m_textures[0]);

	const CString sAnim = "-0001";
	const CString sExt = ".tga";

//	GLfloat fLargest;
//	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);

	vector<string> searchPaths;
	searchPaths.push_back("../texture/");
	searchPaths.insert(searchPaths.end(), config.directories.texturePaths.begin(), config.directories.texturePaths.end());

	unsigned int j;
	unsigned int nSearchPaths = searchPaths.size();

	for (unsigned long i = 0; i < n_tex; )
	{
		if (!m_pDoc->get_texname(i).empty())
		{
			for (j = 0; j < nSearchPaths; ++j)
			{
				CString sTexName = (CString) searchPaths[j].c_str() + m_pDoc->get_texname(i).c_str();

				if (GetFileAttributes(sTexName + sExt) == INVALID_FILE_ATTRIBUTES)
				{
					if (GetFileAttributes(sTexName + sAnim + sExt) == INVALID_FILE_ATTRIBUTES)
						continue;
					else
						sTexName += sAnim;
				}

			//	if (GetFileAttributes(sTexName + sAnim + sExt) != INVALID_FILE_ATTRIBUTES)
			//		sTexName += sAnim;

				sTexName += sExt;

				if (pBytes = gltLoadTGA(sTexName, &iWidth, &iHeight, &iComponents, &eFormat))
				{
					glBindTexture(GL_TEXTURE_2D, m_textures[i]);

					glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight,
												0, eFormat, GL_UNSIGNED_BYTE, pBytes);
					free(pBytes);

					GLint iFilter = config.display.bTextureNearest ? GL_NEAREST : GL_LINEAR;

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, iFilter);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, iFilter);
				//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
				}
				else
				{
					ATLTRACE("Cannot load %s\n", sTexName);
				}
			}
		}

	//	callbackFunc("", 100 * ++i / n_tex);
		++i;
		pMonitor->setProgress(i);
	}

	wglMakeCurrent(NULL, NULL);

//	callbackFunc("", 100);

	return error::ok;
}

error::code PSDLView::UnloadTextures(HDC hDC, HGLRC hRC)
{
	glDeleteTextures(m_pDoc->num_textures(), &m_textures[0]);
	return error::ok;
}

void PSDLView::RenderScene(void)
{
//	wglMakeCurrent(hDC, hRC);

	unsigned short nHue = 0;

	// Bounding box
	{
		Vertex *v1, *v2;
		v1 = &m_pDoc->v_min;
		v2 = &m_pDoc->v_max;

		glColor3f(0.2f, 0.3f, 0.3f);
		glBegin(GL_LINE_LOOP);
			glVertex3f(v1->x, 0, v1->z);
			glVertex3f(v1->x, 0, v2->z);
			glVertex3f(v2->x, 0, v2->z);
			glVertex3f(v2->x, 0, v1->z);
		glEnd();
	}

	vector<unsigned long> blockIDs;
	vector<psdl::block*> selBlocks = CMainFrame::BlocksWindow()->GetSelected(&blockIDs);

	glColor3f(0.2f, 0.2f, 1.0f);
	glLineWidth(2);

	for (vector<psdl::block*>::iterator it = selBlocks.begin(); it != selBlocks.end(); ++it)
	{
		glBegin(GL_LINE_LOOP);
		for (unsigned long i = 0; i < (*it)->num_perimeters(); ++i)
		{
			Vertex* v = m_pDoc->get_vertex((*it)->get_perimeter_point(i)->vertex);
			glVertex3fv(&(v->x));
		}
		glEnd();
	}

	glLineWidth(1);
	glColor3f(1, 1, 1);

	for (unsigned long i = 0; i < m_pDoc->num_blocks(); ++i)
	{
		if (config.display.bTestPVS && m_pCPVS->pvsLists[ blockIDs[0] ][i] != true) continue;

		psdl::block* pBlock = m_pDoc->get_block(i);

		if (!pBlock->enabled) continue;

		psdl::texture* pTexAtb = NULL;
		psdl::tunnel*  pTunnelAtb = NULL;
		GLuint iTunnelTex = 0;

		unsigned long iTex = 0;
		glDisable(GL_TEXTURE_2D);

		for (unsigned long j = 0; j < pBlock->num_attributes(); ++j)
		{
			psdl::attribute* pAtb = pBlock->get_attribute(j);

			switch (pAtb->type)
			{

	case ATB_ROAD:
	case ATB_DIVIDEDROAD:
		{
			psdl::road_strip* pTAtb = static_cast<psdl::road_strip*>(pAtb);
			Vertex* v;

			unsigned short k, p, n = pTAtb->num_sections() - 1;
			GLfloat s;

			p = pAtb->type == ATB_ROAD ? 4 : 6;

			glBindTexture(GL_TEXTURE_2D, m_textures[iTex + 1]);

		// Sidewalk 1
			glBegin(GL_TRIANGLE_STRIP);
			for (k = 0; k < n; ++k)
			{
				s = psdl::d2v(pTAtb->get_vertex(k * p), pTAtb->get_vertex((k + 1) * p)) /
				    psdl::d2v(pTAtb->get_vertex(k * p), pTAtb->get_vertex(k * p + 1)) * 2.f;

				v = pTAtb->get_vertex(k * p + 1);
				glTexCoord2f(0, 0); glVertex3fv(&(pTAtb->get_vertex(k * p)->x));
				glTexCoord2f(0, 1); glVertex3f(v->x, v->y + SW_H, v->z);
				v = pTAtb->get_vertex((k + 1) * p + 1);
				glTexCoord2f(s, 0); glVertex3fv(&(pTAtb->get_vertex((k + 1) * p)->x));
				glTexCoord2f(s, 1); glVertex3f(v->x, v->y + SW_H, v->z);
			}
			glEnd();

			glBegin(GL_TRIANGLE_STRIP);
			for (k = 0; k < n; ++k)
			{
				s = psdl::d2v(pTAtb->get_vertex(k * p), pTAtb->get_vertex((k + 1) * p)) /
				    psdl::d2v(pTAtb->get_vertex(k * p), pTAtb->get_vertex(k * p + 1)) * 2.f;

				v = pTAtb->get_vertex(k * p + 1);
				glTexCoord2f(0, 1); glVertex3f(v->x, v->y + SW_H, v->z);
				                    glVertex3fv(&(v->x));
				v = pTAtb->get_vertex((k + 1) * p + 1);
				glTexCoord2f(s, 1); glVertex3f(v->x, v->y + SW_H, v->z);
				                    glVertex3fv(&(v->x));
			}
			glEnd();

		// Sidewalk 2
			glBegin(GL_TRIANGLE_STRIP);
			for (k = 0; k < n; ++k)
			{
				s = psdl::d2v(pTAtb->get_vertex((k + 1) * p - 1), pTAtb->get_vertex((k + 2) * p - 1)) /
				    psdl::d2v(pTAtb->get_vertex((k + 1) * p - 2), pTAtb->get_vertex((k + 1) * p - 1)) * 2.f;

				v = pTAtb->get_vertex((k + 1) * p - 2);
				glTexCoord2f(0, 1); glVertex3fv(&(v->x));
				                    glVertex3f(v->x, v->y + SW_H, v->z);
				v = pTAtb->get_vertex((k + 2) * p - 2);
				glTexCoord2f(s, 1); glVertex3fv(&(v->x));
				                    glVertex3f(v->x, v->y + SW_H, v->z);
			}
			glEnd();

			glBegin(GL_TRIANGLE_STRIP);
			for (k = 0; k < n; ++k)
			{
				s = psdl::d2v(pTAtb->get_vertex((k + 1) * p - 1), pTAtb->get_vertex((k + 2) * p - 1)) /
				    psdl::d2v(pTAtb->get_vertex((k + 1) * p - 2), pTAtb->get_vertex((k + 1) * p - 1)) * 2.f;

				v = pTAtb->get_vertex((k + 1) * p - 2);
				glTexCoord2f(0, 1); glVertex3f(v->x, v->y + SW_H, v->z);
				glTexCoord2f(0, 0); glVertex3fv(&(pTAtb->get_vertex((k + 1) * p - 1)->x));
				v = pTAtb->get_vertex((k + 2) * p - 2);
				glTexCoord2f(s, 1); glVertex3f(v->x, v->y + SW_H, v->z);
				glTexCoord2f(s, 0); glVertex3fv(&(pTAtb->get_vertex((k + 2) * p - 1)->x));
			}
			glEnd();

			glBindTexture(GL_TEXTURE_2D, m_textures[iTex]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

		// Road surface

			s = 0;
			if (pAtb->type == ATB_ROAD)
			{
				glBegin(GL_TRIANGLE_STRIP);
					glTexCoord2f(0, 0); glVertex3fv(&(pTAtb->get_vertex(1)->x));
					glTexCoord2f(0, 2); glVertex3fv(&(pTAtb->get_vertex(2)->x));
					for (k = 1; k < pTAtb->num_sections(); ++k)
					{
						s += psdl::d2v(pTAtb->get_vertex((k - 1) * 4 + 1), pTAtb->get_vertex(k * 4 + 1)) /
							 psdl::d2v(pTAtb->get_vertex(k * 4 + 1), pTAtb->get_vertex(k * 4 + 2));

						glTexCoord2f(s, 0); glVertex3fv(&(pTAtb->get_vertex(k * 4 + 1)->x));
						glTexCoord2f(s, 2); glVertex3fv(&(pTAtb->get_vertex(k * 4 + 2)->x));
					}
				glEnd();
			}
			else
			{
				glBegin(GL_TRIANGLE_STRIP);
					glTexCoord2f(0, 0); glVertex3fv(&(pTAtb->get_vertex(1)->x));
					glTexCoord2f(0, 1); glVertex3fv(&(pTAtb->get_vertex(2)->x));
					for (k = 1; k < pTAtb->num_sections(); ++k)
					{
						s += psdl::d2v(pTAtb->get_vertex((k - 1) * 6 + 1), pTAtb->get_vertex(k * 6 + 1)) /
							 psdl::d2v(pTAtb->get_vertex(k * 6 + 1), pTAtb->get_vertex(k * 6 + 2));

						glTexCoord2f(s, 0); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 1)->x));
						glTexCoord2f(s, 1); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 2)->x));
					}
				glEnd();

				s = 0;
				glBegin(GL_TRIANGLE_STRIP);
					glTexCoord2f(0, 1); glVertex3fv(&(pTAtb->get_vertex(3)->x));
					glTexCoord2f(0, 0); glVertex3fv(&(pTAtb->get_vertex(4)->x));
					for (k = 1; k < pTAtb->num_sections(); ++k)
					{
						s += psdl::d2v(pTAtb->get_vertex((k - 1) * 6 + 3), pTAtb->get_vertex(k * 6 + 3)) /
						     psdl::d2v(pTAtb->get_vertex(k * 6 + 3), pTAtb->get_vertex(k * 6 + 4));

						glTexCoord2f(s, 1); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 3)->x));
						glTexCoord2f(s, 0); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 4)->x));
					}
				glEnd();
			}

		// Divider

			if (pAtb->type == ATB_DIVIDEDROAD)
			{
				psdl::divided_road_strip* pDAtb = static_cast<psdl::divided_road_strip*>(pAtb);

				glBindTexture(GL_TEXTURE_2D, m_textures[pDAtb->i_texture]);

				switch (pDAtb->divider_type())
				{
					case 1:
						glBindTexture(GL_TEXTURE_2D, m_textures[pDAtb->i_texture + 1]);

						s = 0;
						glBegin(GL_TRIANGLE_STRIP);
							glTexCoord2f(0, pDAtb->value); glVertex3fv(&(pTAtb->get_vertex(2)->x));
							glTexCoord2f(0, 0); glVertex3fv(&(pTAtb->get_vertex(3)->x));
							for (k = 1; k < pTAtb->num_sections(); ++k)
							{
								s += psdl::d2v(pTAtb->get_vertex((k - 1) * 6 + 2), pTAtb->get_vertex(k * 6 + 2)) /
								     psdl::d2v(pTAtb->get_vertex(k * 6 + 2), pTAtb->get_vertex(k * 6 + 3)) * 2.f;

								glTexCoord2f(s, pDAtb->value); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 2)->x));
								glTexCoord2f(s, 0); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 3)->x));
							}
						glEnd();
						break;

					case 2:
						{
							GLfloat fHeight = pDAtb->height / 255.f;

							s = 0;
							glBegin(GL_TRIANGLE_STRIP);
								v = pTAtb->get_vertex(2);
								glTexCoord2f(0, 1); glVertex3fv(&(v->x));
													glVertex3f(v->x, v->y + fHeight, v->z);
								for (k = 1; k < pTAtb->num_sections(); ++k)
								{
									s += psdl::d2v(pTAtb->get_vertex((k - 1) * 6 + 2), pTAtb->get_vertex(k * 6 + 2)) /
										 psdl::d2v(pTAtb->get_vertex(k * 6 + 2), pTAtb->get_vertex(k * 6 + 3)) * 2.f;

									v = pTAtb->get_vertex(k * 6 + 2);
									glTexCoord2f(s, 1); glVertex3fv(&(v->x));
														glVertex3f(v->x, v->y + fHeight, v->z);
								}
							glEnd();

							s = 0;
							glBegin(GL_TRIANGLE_STRIP);
								v = pTAtb->get_vertex(3);
								glTexCoord2f(0, 1); glVertex3f(v->x, v->y + fHeight, v->z);
													glVertex3fv(&(v->x));
								for (k = 1; k < pTAtb->num_sections(); ++k)
								{
									s += psdl::d2v(pTAtb->get_vertex((k - 1) * 6 + 3), pTAtb->get_vertex(k * 6 + 3)) /
										 psdl::d2v(pTAtb->get_vertex(k * 6 + 2), pTAtb->get_vertex(k * 6 + 3)) * 2.f;

									v = pTAtb->get_vertex(k * 6 + 3);
									glTexCoord2f(s, 1); glVertex3f(v->x, v->y + fHeight, v->z);
														glVertex3fv(&(v->x));
								}
							glEnd();

							glBindTexture(GL_TEXTURE_2D, m_textures[pDAtb->i_texture + 2]);

							s = 0;
							glBegin(GL_TRIANGLE_STRIP);
								v = pTAtb->get_vertex(2);
								glTexCoord2f(0, 1); glVertex3f(v->x, v->y + fHeight, v->z);
								v = pTAtb->get_vertex(3);
								glTexCoord2f(0, 0); glVertex3f(v->x, v->y + fHeight, v->z);
								for (k = 1; k < pTAtb->num_sections(); ++k)
								{
									s += psdl::d2v(pTAtb->get_vertex((k - 1) * 6 + 2), pTAtb->get_vertex(k * 6 + 2)) /
									     psdl::d2v(pTAtb->get_vertex(k * 6 + 2), pTAtb->get_vertex(k * 6 + 3)) * 2.f;

									v = pTAtb->get_vertex(k * 6 + 2);
									glTexCoord2f(s, 1); glVertex3f(v->x, v->y + fHeight, v->z);
									v = pTAtb->get_vertex(k * 6 + 3);
									glTexCoord2f(s, 0); glVertex3f(v->x, v->y + fHeight, v->z);
								}
							glEnd();

							glBindTexture(GL_TEXTURE_2D, m_textures[pDAtb->i_texture + 3]);

						// Closed start
							if (pDAtb->get_flag(6))
							{
								glBegin(GL_TRIANGLE_STRIP);
									v = pTAtb->get_vertex(2);
									glTexCoord2f(0, 1); glVertex3f(v->x, v->y + fHeight, v->z);
														glVertex3fv(&(v->x));
									v = pTAtb->get_vertex(3);
									glTexCoord2f(1, 1); glVertex3f(v->x, v->y + fHeight, v->z);
														glVertex3fv(&(v->x));
								glEnd();
							}
						// Closed end
							if (pDAtb->get_flag(7))
							{
								glBegin(GL_TRIANGLE_STRIP);
									v = pTAtb->get_vertex(n * 6 + 2);
									glTexCoord2f(0, 1); glVertex3fv(&(v->x));
														glVertex3f(v->x, v->y + fHeight, v->z);
									v = pTAtb->get_vertex(n * 6 + 3);
									glTexCoord2f(1, 1); glVertex3fv(&(v->x));
														glVertex3f(v->x, v->y + fHeight, v->z);
								glEnd();
							}
						}
						break;

					case 3:
						{
							Vertex *v1, *v2, *v3, *v4;

							const GLfloat fHeight = 1.f;
							const GLfloat fWidth  = 0.4f;
							const GLfloat fSlopingSide = 1.11803398875f; // sqrtf(powf(0.5, 2) + 1)
							GLfloat a, b;

							glBindTexture(GL_TEXTURE_2D, m_textures[pDAtb->i_texture + 1]);

							for (k = 0; k < n; ++k)
							{
								v1 = pTAtb->get_vertex(k * 6 + 2);
								v2 = pTAtb->get_vertex(k * 6 + 3);
								v3 = pTAtb->get_vertex(k * 6 + 8);
								v4 = pTAtb->get_vertex(k * 6 + 9);

								a = atan2(v2->x - v1->x, v2->z - v1->z);
								b = atan2(v4->x - v3->x, v4->z - v3->z);

								s = int(0.5f + psdl::d2v(v1, v3) / fSlopingSide * 2.f);

								glBegin(GL_TRIANGLE_STRIP);
									glTexCoord2f(0, 0);
									glVertex3f(v1->x + sin(a) * fWidth, v1->y + fHeight, v1->z + cos(a) * fWidth);

									glTexCoord2f(s, 0);
									glVertex3f(v3->x + sin(b) * fWidth, v3->y + fHeight, v3->z + cos(b) * fWidth);

									glTexCoord2f(0, 1); glVertex3fv(&(v1->x));
									glTexCoord2f(s, 1); glVertex3fv(&(v3->x));
								glEnd();
								glBegin(GL_TRIANGLE_STRIP);
									glTexCoord2f(0, 1); glVertex3fv(&(v2->x));
									glTexCoord2f(s, 1); glVertex3fv(&(v4->x));

									glTexCoord2f(0, 0);
									glVertex3f(v2->x - sin(a) * fWidth, v2->y + fHeight, v2->z - cos(a) * fWidth);

									glTexCoord2f(s, 0);
									glVertex3f(v4->x - sin(b) * fWidth, v4->y + fHeight, v4->z - cos(b) * fWidth);
								glEnd();

								glBindTexture(GL_TEXTURE_2D, m_textures[pDAtb->i_texture + 2]);
								glBegin(GL_TRIANGLE_STRIP);
									glTexCoord2f(0, 0);
									glVertex3f(v2->x - sin(a) * fWidth, v2->y + fHeight, v2->z - cos(a) * fWidth);

									glTexCoord2f(s, 0);
									glVertex3f(v4->x - sin(b) * fWidth, v4->y + fHeight, v4->z - cos(b) * fWidth);

									glTexCoord2f(0, 1);
									glVertex3f(v1->x + sin(a) * fWidth, v1->y + fHeight, v1->z + cos(a) * fWidth);

									glTexCoord2f(s, 1);
									glVertex3f(v3->x + sin(b) * fWidth, v3->y + fHeight, v3->z + cos(b) * fWidth);
								glEnd();
							}

							glBindTexture(GL_TEXTURE_2D, m_textures[pDAtb->i_texture + 3]);

							if (pDAtb->get_flag(7))
							{
								a = atan2(v4->x - v3->x, v4->z - v3->z);

								glBegin(GL_TRIANGLE_STRIP);
									glTexCoord2f(0, 1); glVertex3fv(&(v4->x));
									glTexCoord2f(1, 1); glVertex3fv(&(v3->x));

									glTexCoord2f(0, 0);
									glVertex3f(v4->x - sin(a) * fWidth, v4->y + fHeight, v4->z - cos(a) * fWidth);

									glTexCoord2f(1, 0);
									glVertex3f(v3->x + sin(a) * fWidth, v3->y + fHeight, v3->z + cos(a) * fWidth);
								glEnd();
							}

							if (pDAtb->get_flag(6))
							{
								v1 = pTAtb->get_vertex(2);
								v2 = pTAtb->get_vertex(3);

								a = atan2(v2->x - v1->x, v2->z - v1->z);

								glBegin(GL_TRIANGLE_STRIP);
									glTexCoord2f(0, 0);
									glVertex3f(v2->x - sin(a) * fWidth, v2->y + fHeight, v2->z - cos(a) * fWidth);

									glTexCoord2f(1, 0);
									glVertex3f(v1->x + sin(a) * fWidth, v1->y + fHeight, v1->z + cos(a) * fWidth);

									glTexCoord2f(0, 1); glVertex3fv(&(v2->x));
									glTexCoord2f(1, 1); glVertex3fv(&(v1->x));
								glEnd();
							}
						}
						break;
				}

				glBindTexture(GL_TEXTURE_2D, m_textures[iTex]);
			}

			if (pTunnelAtb)
			{
				Vertex* v1;
				Vertex* v2;
				GLfloat s;
			//	GLfloat fThick = 2.0f;

				for (k = 0; k < pTAtb->num_sections() - 1; ++k)
				{
					s = int(0.5f + psdl::d2v(pTAtb->get_vertex(k * p), pTAtb->get_vertex((k + 1) * p)) / pTunnelAtb->height1);

		// Left wall
					if (pTunnelAtb->get_flag(BIT_LEFT))
					{
						glBindTexture(GL_TEXTURE_2D, iTunnelTex);
						glBegin(GL_TRIANGLE_STRIP);
							v1 = pTAtb->get_vertex((k + 1) * p);
							v2 = pTAtb->get_vertex(k * p);

						//	if (pTunnelAtb->get_flag(BIT_CULLED))
							{
								glTexCoord2f(s, 1); glVertex3fv(&(v1->x));
								glTexCoord2f(0, 1); glVertex3fv(&(v2->x));
							}
							glTexCoord2f(s, 0); glVertex3f(v1->x, v1->y + pTunnelAtb->height1, v1->z);
							glTexCoord2f(0, 0); glVertex3f(v2->x, v2->y + pTunnelAtb->height1, v2->z);

							glTexCoord2f(s, 1); glVertex3fv(&(v1->x));
							glTexCoord2f(0, 1); glVertex3fv(&(v2->x));
						glEnd();
					}

		// Right wall
					if (pTunnelAtb->get_flag(BIT_RIGHT))
					{
						glBindTexture(GL_TEXTURE_2D, iTunnelTex + 1);
						glBegin(GL_TRIANGLE_STRIP);
							v1 = pTAtb->get_vertex((k + 1) * p - 1);
							v2 = pTAtb->get_vertex((k + 2) * p - 1);

						//	if (pTunnelAtb->get_flag(BIT_CULLED))
							{
								glTexCoord2f(0, 1); glVertex3fv(&(v1->x));
								glTexCoord2f(s, 1); glVertex3fv(&(v2->x));
							}
							glTexCoord2f(0, 0); glVertex3f(v1->x, v1->y + pTunnelAtb->height1, v1->z);
							glTexCoord2f(s, 0); glVertex3f(v2->x, v2->y + pTunnelAtb->height1, v2->z);

							glTexCoord2f(0, 1); glVertex3fv(&(v1->x));
							glTexCoord2f(s, 1); glVertex3fv(&(v2->x));
						glEnd();
					}
				}
			}
			
/*
		// Sidewalk 1
			glBegin(GL_TRIANGLE_STRIP);
			for (k = 0; k < n; ++k)
			{
				s = psdl::d2v(pTAtb->get_vertex(k * 4), pTAtb->get_vertex(k * 4 + 4)) /
				    psdl::d2v(pTAtb->get_vertex(k * 4), pTAtb->get_vertex(k * 4 + 1)) * 2.f;

				v = pTAtb->get_vertex(k * 4 + 1);
				glTexCoord2f(0, 0); glVertex3fv(&(pTAtb->get_vertex(k * 4)->x));
				glTexCoord2f(0, 1); glVertex3f(v->x, v->y + SW_H, v->z);
				v = pTAtb->get_vertex(k * 4 + 5);
				glTexCoord2f(s, 0); glVertex3fv(&(pTAtb->get_vertex(k * 4 + 4)->x));
				glTexCoord2f(s, 1); glVertex3f(v->x, v->y + SW_H, v->z);
			}
			glEnd();

			glBegin(GL_TRIANGLE_STRIP);
			for (k = 0; k < n; ++k)
			{
				s = psdl::d2v(pTAtb->get_vertex(k * 4), pTAtb->get_vertex(k * 4 + 4)) /
				    psdl::d2v(pTAtb->get_vertex(k * 4), pTAtb->get_vertex(k * 4 + 1)) * 2.f;

				v = pTAtb->get_vertex(k * 4 + 1);
				glTexCoord2f(0, 1); glVertex3f(v->x, v->y + SW_H, v->z);
				                    glVertex3fv(&(v->x));
				v = pTAtb->get_vertex(k * 4 + 5);
				glTexCoord2f(s, 1); glVertex3f(v->x, v->y + SW_H, v->z);
				                    glVertex3fv(&(v->x));
			}
			glEnd();

		// Sidewalk 2
			glBegin(GL_TRIANGLE_STRIP);
			for (k = 0; k < n; ++k)
			{
				s = psdl::d2v(pTAtb->get_vertex(k * 4 + 3), pTAtb->get_vertex(k * 4 + 7)) /
				    psdl::d2v(pTAtb->get_vertex(k * 4 + 2), pTAtb->get_vertex(k * 4 + 3)) * 2.f;

				v = pTAtb->get_vertex(k * 4 + 2);
				glTexCoord2f(0, 1); glVertex3fv(&(v->x));
				                    glVertex3f(v->x, v->y + SW_H, v->z);
				v = pTAtb->get_vertex(k * 4 + 6);
				glTexCoord2f(s, 1); glVertex3fv(&(v->x));
				                    glVertex3f(v->x, v->y + SW_H, v->z);
			}
			glEnd();

			glBegin(GL_TRIANGLE_STRIP);
			for (k = 0; k < n; ++k)
			{
				s = psdl::d2v(pTAtb->get_vertex(k * 4 + 3), pTAtb->get_vertex(k * 4 + 7)) /
				    psdl::d2v(pTAtb->get_vertex(k * 4 + 2), pTAtb->get_vertex(k * 4 + 3)) * 2.f;

				v = pTAtb->get_vertex(k * 4 + 2);
				glTexCoord2f(0, 1); glVertex3f(v->x, v->y + SW_H, v->z);
				glTexCoord2f(0, 0); glVertex3fv(&(pTAtb->get_vertex(k * 4 + 3)->x));
				v = pTAtb->get_vertex(k * 4 + 6);
				glTexCoord2f(s, 1); glVertex3f(v->x, v->y + SW_H, v->z);
				glTexCoord2f(s, 0); glVertex3fv(&(pTAtb->get_vertex(k * 4 + 7)->x));
			}
			glEnd();

			glBindTexture(GL_TEXTURE_2D, m_textures[iTex]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

		// Road surface
			glBegin(GL_TRIANGLE_STRIP);
			for (k = 0; k < n; ++k)
			{
				s = psdl::d2v(pTAtb->get_vertex(k * 4 + 1), pTAtb->get_vertex(k * 4 + 5)) /
				    psdl::d2v(pTAtb->get_vertex(k * 4 + 1), pTAtb->get_vertex(k * 4 + 2));

				glTexCoord2f(0, 0); glVertex3fv(&(pTAtb->get_vertex(k * 4 + 1)->x));
				glTexCoord2f(0, 2); glVertex3fv(&(pTAtb->get_vertex(k * 4 + 2)->x));
				glTexCoord2f(s, 0); glVertex3fv(&(pTAtb->get_vertex(k * 4 + 5)->x));
				glTexCoord2f(s, 2); glVertex3fv(&(pTAtb->get_vertex(k * 4 + 6)->x));
			}
			glEnd();

			if (pTunnelAtb)
			{
				Vertex* v1;
				Vertex* v2;
				GLfloat s, a, b;
				GLfloat fThick = 2.0f;

				for (k = 0; k < pTAtb->num_sections() - 1; ++k)
				{
					s = int(0.5f + psdl::d2v(pTAtb->get_vertex(k * 4), pTAtb->get_vertex(k * 4 + 4)) / pTunnelAtb->height1);

		// Left wall
					if (pTunnelAtb->get_flag(BIT_LEFT))
					{
						glBindTexture(GL_TEXTURE_2D, iTunnelTex);
						glBegin(GL_QUAD_STRIP);
							if (pTunnelAtb->get_flag(BIT_CULLED))
							{
								glTexCoord2f(s, 1); glVertex3fv(&(pTAtb->get_vertex(k * 4 + 4)->x));
								glTexCoord2f(0, 1); glVertex3fv(&(pTAtb->get_vertex(k * 4)->x));
							}
							v = pTAtb->get_vertex(k * 4 + 4);
							glTexCoord2f(s, 0); glVertex3f(v->x, v->y + pTunnelAtb->height1, v->z);

							v = pTAtb->get_vertex(k * 4);
							glTexCoord2f(0, 0); glVertex3f(v->x, v->y + pTunnelAtb->height1, v->z);

							glTexCoord2f(s, 1); glVertex3fv(&(pTAtb->get_vertex(k * 4 + 4)->x));
							glTexCoord2f(0, 1); glVertex3fv(&(pTAtb->get_vertex(k * 4)->x));
						glEnd();

						if (pTunnelAtb->get_flag(BIT_STYLE))
						{
							v1 = pTAtb->get_vertex(k * 4);
							v2 = pTAtb->get_vertex(k * 4 + 3);
							a  = atan2(v2->x - v1->x, v2->z - v1->z);

							v2 = pTAtb->get_vertex(k * 4 + 4);
							v1 = pTAtb->get_vertex(k * 4 + 7);
							b  = atan2(v1->x - v2->x, v1->z - v2->z);

							v1 = pTAtb->get_vertex(k * 4);

							glBindTexture(GL_TEXTURE_2D, iTunnelTex + 3);
							glBegin(GL_QUAD_STRIP);
								glTexCoord2f(s, 0); glVertex3f(v1->x, v1->y + pTunnelAtb->height1, v1->z);
								glTexCoord2f(0, 0); glVertex3f(v2->x, v2->y + pTunnelAtb->height1, v2->z);
								glTexCoord2f(s, 1); glVertex3f(v1->x - sin(a) * fThick, v1->y + pTunnelAtb->height1, v1->z - cos(a) * fThick);
								glTexCoord2f(0, 1); glVertex3f(v2->x - sin(b) * fThick, v2->y + pTunnelAtb->height1, v2->z - cos(b) * fThick);
								glTexCoord2f(s, 2); glVertex3f(v1->x - sin(a) * fThick, v1->y - fThick, v1->z - cos(a) * fThick);
								glTexCoord2f(0, 2); glVertex3f(v2->x - sin(b) * fThick, v2->y - fThick, v2->z - cos(b) * fThick);
							glEnd();

							v1 = pTAtb->get_vertex(k * 4 + 3);
							v2 = pTAtb->get_vertex(k * 4 + 7);

							glBindTexture(GL_TEXTURE_2D, iTunnelTex + 4);
							glBegin(GL_QUAD_STRIP);
								glTexCoord2f(0, 0); glVertex3f(v2->x, v2->y + pTunnelAtb->height1, v2->z);
								glTexCoord2f(s, 0); glVertex3f(v1->x, v1->y + pTunnelAtb->height1, v1->z);
								glTexCoord2f(0, 1); glVertex3f(v2->x + sin(b) * fThick, v2->y + pTunnelAtb->height1, v2->z + cos(b) * fThick);
								glTexCoord2f(s, 1); glVertex3f(v1->x + sin(a) * fThick, v1->y + pTunnelAtb->height1, v1->z + cos(a) * fThick);
								glTexCoord2f(0, 2); glVertex3f(v2->x + sin(b) * fThick, v2->y - fThick, v2->z + cos(b) * fThick);
								glTexCoord2f(s, 2); glVertex3f(v1->x + sin(a) * fThick, v1->y - fThick, v1->z + cos(a) * fThick);
							glEnd();

							glBindTexture(GL_TEXTURE_2D, iTunnelTex + 5);
						}
					}

		// Right wall
					if (pTunnelAtb->get_flag(BIT_RIGHT))
					{
						glBindTexture(GL_TEXTURE_2D, iTunnelTex + 1);
						glBegin(GL_QUAD_STRIP);
							if (pTunnelAtb->get_flag(BIT_CULLED))
							{
								glTexCoord2f(0, 1); glVertex3fv(&(pTAtb->get_vertex(k * 4 + 3)->x));
								glTexCoord2f(s, 1); glVertex3fv(&(pTAtb->get_vertex(k * 4 + 7)->x));
							}
							v = pTAtb->get_vertex(k * 4 + 3);
							glTexCoord2f(0, 0); glVertex3f(v->x, v->y + pTunnelAtb->height1, v->z);

							v = pTAtb->get_vertex(k * 4 + 7);
							glTexCoord2f(s, 0); glVertex3f(v->x, v->y + pTunnelAtb->height1, v->z);

							glTexCoord2f(0, 1); glVertex3fv(&(pTAtb->get_vertex(k * 4 + 3)->x));
							glTexCoord2f(s, 1); glVertex3fv(&(pTAtb->get_vertex(k * 4 + 7)->x));
						glEnd();
					}
				}
			}*/
		}
		break;

	case ATB_SIDEWALK:
		{
			psdl::sidewalk_strip* pTAtb = static_cast<psdl::sidewalk_strip*>(pAtb);
			Vertex* v;
			unsigned short k;

			glBindTexture(GL_TEXTURE_2D, m_textures[iTex + 1]);

			unsigned short f1 = pTAtb->get_vertex_ref(0),
			               f2 = pTAtb->get_vertex_ref(1);

			if (f1 == f2 && f1 <= 1)// Road end piece
			{
				if (f1 == 0) glFrontFace(GL_CW);
				glBegin(GL_TRIANGLES);
					v = pTAtb->get_vertex(2);
					glTexCoord2f(v->x/5, v->z/5);
					glVertex3f(v->x, v->y + SW_H, v->z);
					glVertex3fv(&(v->x));

					v = pTAtb->get_vertex(3);
					glTexCoord2f(v->x/5, v->z/5);
					glVertex3fv(&(v->x));
				glEnd();
				glFrontFace(GL_CCW);
			}
			else
			{
				glBegin(GL_TRIANGLE_STRIP);
				for (k = 0; k < pTAtb->num_sections(); ++k)
				{
					v = pTAtb->get_vertex(k * 2);
					glTexCoord2f(v->x/5, v->z/5); glVertex3f(v->x, v->y + SW_H, v->z);
					v = pTAtb->get_vertex(k * 2 + 1);
					glTexCoord2f(v->x/5, v->z/5); glVertex3fv(&(v->x));
				}
				glEnd();

				glBegin(GL_TRIANGLE_STRIP);
				for (k = 0; k < pTAtb->num_sections(); ++k)
				{
					v = pTAtb->get_vertex(k * 2);
					glTexCoord2f(v->x/5, v->z/5);
					glVertex3fv(&(v->x));
					glVertex3f(v->x, v->y + SW_H, v->z);
				}
				glEnd();
			}
		}
		break;

	case ATB_RECTANGLE:
		{
			psdl::rectangle_strip* pTAtb = static_cast<psdl::rectangle_strip*>(pAtb);
			GLfloat s = 0;

			glBegin(GL_TRIANGLE_STRIP);
				glTexCoord2f(0, 0); glVertex3fv(&(pTAtb->get_vertex(0)->x));
				glTexCoord2f(0, 1); glVertex3fv(&(pTAtb->get_vertex(1)->x));
				for (unsigned short k = 1; k < pTAtb->num_sections(); ++k)
				{
					s += psdl::d2v(pTAtb->get_vertex((k - 1) * 2), pTAtb->get_vertex(k * 2)) /
					     psdl::d2v(pTAtb->get_vertex(k * 2), pTAtb->get_vertex(k * 2 + 1)) * 2.f;

					glTexCoord2f(s, 0); glVertex3fv(&(pTAtb->get_vertex(k * 2)->x));
					glTexCoord2f(s, 1); glVertex3fv(&(pTAtb->get_vertex(k * 2 + 1)->x));
				}
			glEnd();
		}
		break;

	case ATB_SLIVER:
		{
			psdl::sliver* pTAtb = static_cast<psdl::sliver*>(pAtb);
			Vertex *v1, *v2;

			GLfloat scale = m_pDoc->get_height(pTAtb->tex_scale);

			v1 = pTAtb->get_vertex(0);
			v2 = pTAtb->get_vertex(1);

			GLfloat d1 = psdl::d2v(v1, v2);
			GLfloat d2 = m_pDoc->get_height(pTAtb->top) - v1->y;
			GLfloat d3 = m_pDoc->get_height(pTAtb->top) - v2->y;

			GLfloat u = int(d1 * scale + 0.5f);// Add 0.5 for normal rounding

			glBegin(GL_TRIANGLE_STRIP);
				glTexCoord2f(0, d2 * scale); glVertex3fv(&(v1->x));
				glTexCoord2f(u, d3 * scale); glVertex3fv(&(v2->x));
				glTexCoord2f(0, 0); glVertex3f(v1->x, m_pDoc->get_height(pTAtb->top), v1->z);
				glTexCoord2f(u, 0); glVertex3f(v2->x, m_pDoc->get_height(pTAtb->top), v2->z);
			glEnd();
		}
		break;

	case ATB_CROSSWALK:
		{
			psdl::crosswalk* pTAtb = static_cast<psdl::crosswalk*>(pAtb);

			GLfloat t =
				psdl::d2v(pTAtb->get_vertex(0), pTAtb->get_vertex(3)) /
				psdl::d2v(pTAtb->get_vertex(0), pTAtb->get_vertex(1));

			glBindTexture(GL_TEXTURE_2D, m_textures[iTex + 2]);
			glBegin(GL_TRIANGLE_STRIP);
				glTexCoord2f(0, 0); glVertex3fv( &(pTAtb->get_vertex(1)->x) );
				glTexCoord2f(1, 0); glVertex3fv( &(pTAtb->get_vertex(0)->x) );
				glTexCoord2f(0, t); glVertex3fv( &(pTAtb->get_vertex(3)->x) );
				glTexCoord2f(1, t); glVertex3fv( &(pTAtb->get_vertex(2)->x) );
			glEnd();
		}
		break;

	case ATB_TRIANGLEFAN:
	case ATB_ROADTRIANGLEFAN:
		{
			psdl::road_triangle_fan* pTAtb = static_cast<psdl::road_triangle_fan*>(pAtb);
			const Vertex* v;

			glBindTexture(GL_TEXTURE_2D, m_textures[iTex]);
			glBegin(GL_TRIANGLE_FAN);
			for (unsigned short k = 0; k < pTAtb->num_vertices(); ++k)
			{
				v = pTAtb->get_vertex(k);
				glTexCoord2f(v->x/10, v->z/10); glVertex3fv(&(v->x));
			}
			glEnd();
		}
		break;

	case ATB_ROOFTRIANGLEFAN:
		{
			psdl::roof_triangle_fan* pTAtb = static_cast<psdl::roof_triangle_fan*>(pAtb);
			Vertex* v;

			glBegin(GL_TRIANGLE_FAN);
			for (unsigned short k = 0; k < pTAtb->num_vertices(); ++k)
			{
				v = pTAtb->get_vertex(k);
				glTexCoord2f(v->x/10, v->z/10); glVertex3f(v->x, m_pDoc->get_height(pTAtb->i_height), v->z);
			}
			glEnd();
		}
		break;
/*
	case ATB_DIVIDEDROAD:
		{
			psdl::divided_road_strip* pTAtb = static_cast<psdl::divided_road_strip*>(pAtb);
			Vertex* v;

			unsigned short k, n = pTAtb->num_sections() - 1;
			GLfloat s;

			glBindTexture(GL_TEXTURE_2D, m_textures[iTex + 1]);

		// Sidewalk 1
			glBegin(GL_TRIANGLE_STRIP);
			for (k = 0; k < n; ++k)
			{
				s = psdl::d2v(pTAtb->get_vertex(k * 6), pTAtb->get_vertex(k * 6 + 6)) /
				    psdl::d2v(pTAtb->get_vertex(k * 6), pTAtb->get_vertex(k * 6 + 1)) * 2.f;

				v = pTAtb->get_vertex(k * 6 + 1);
				glTexCoord2f(0, 0); glVertex3fv(&(pTAtb->get_vertex(k * 6)->x));
				glTexCoord2f(0, 1); glVertex3f(v->x, v->y + SW_H, v->z);
				v = pTAtb->get_vertex(k * 6 + 7);
				glTexCoord2f(s, 0); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 6)->x));
				glTexCoord2f(s, 1); glVertex3f(v->x, v->y + SW_H, v->z);
			}
			glEnd();

			glBegin(GL_TRIANGLE_STRIP);
			for (k = 0; k < n; ++k)
			{
				s = psdl::d2v(pTAtb->get_vertex(k * 6), pTAtb->get_vertex(k * 6 + 6)) /
				    psdl::d2v(pTAtb->get_vertex(k * 6), pTAtb->get_vertex(k * 6 + 1)) * 2.f;

				v = pTAtb->get_vertex(k * 6 + 1);
				glTexCoord2f(0, 1); glVertex3f(v->x, v->y + SW_H, v->z);
				                    glVertex3fv(&(v->x));
				v = pTAtb->get_vertex(k * 6 + 7);
				glTexCoord2f(s, 1); glVertex3f(v->x, v->y + SW_H, v->z);
				                    glVertex3fv(&(v->x));
			}
			glEnd();

		// Sidewalk 2
			glBegin(GL_TRIANGLE_STRIP);
			for (k = 0; k < n; ++k)
			{
				s = psdl::d2v(pTAtb->get_vertex(k * 6 + 5), pTAtb->get_vertex(k * 6 + 11)) /
				    psdl::d2v(pTAtb->get_vertex(k * 6 + 4), pTAtb->get_vertex(k * 6 + 5)) * 2.f;

				v = pTAtb->get_vertex(k * 6 + 4);
				glTexCoord2f(0, 1); glVertex3fv(&(v->x));
				                    glVertex3f(v->x, v->y + SW_H, v->z);
				v = pTAtb->get_vertex(k * 6 + 10);
				glTexCoord2f(s, 1); glVertex3fv(&(v->x));
				                    glVertex3f(v->x, v->y + SW_H, v->z);
			}
			glEnd();

			glBegin(GL_TRIANGLE_STRIP);
			for (k = 0; k < n; ++k)
			{
				s = psdl::d2v(pTAtb->get_vertex(k * 6 + 5), pTAtb->get_vertex(k * 6 + 11)) /
				    psdl::d2v(pTAtb->get_vertex(k * 6 + 4), pTAtb->get_vertex(k * 6 + 5)) * 2.f;

				v = pTAtb->get_vertex(k * 6 + 4);
				glTexCoord2f(0, 1); glVertex3f(v->x, v->y + SW_H, v->z);
				glTexCoord2f(0, 0); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 5)->x));
				v = pTAtb->get_vertex(k * 6 + 10);
				glTexCoord2f(s, 1); glVertex3f(v->x, v->y + SW_H, v->z);
				glTexCoord2f(s, 0); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 11)->x));
			}
			glEnd();

			glBindTexture(GL_TEXTURE_2D, m_textures[pTAtb->i_texture]);

		// Divider

			switch (pTAtb->divider_type)
			{
				case 1:
					glBegin(GL_TRIANGLE_STRIP);
					for (k = 0; k < n; ++k)
					{
						s = psdl::d2v(pTAtb->get_vertex(k * 6 + 2), pTAtb->get_vertex(k * 6 + 8)) /
							psdl::d2v(pTAtb->get_vertex(k * 6 + 2), pTAtb->get_vertex(k * 6 + 3)) * 2.f;

						glTexCoord2f(0, pTAtb->value); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 2)->x));
						glTexCoord2f(0, 0); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 3)->x));
						glTexCoord2f(s, pTAtb->value); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 8)->x));
						glTexCoord2f(s, 0); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 9)->x));
					}
					glEnd();
					break;

				case 2:
					{
						GLfloat height = pTAtb->height / 255.f;

						glBegin(GL_TRIANGLE_STRIP);
						for (k = 0; k < n; ++k)
						{
							s = psdl::d2v(pTAtb->get_vertex(k * 6 + 2), pTAtb->get_vertex(k * 6 + 8)) /
								psdl::d2v(pTAtb->get_vertex(k * 6 + 2), pTAtb->get_vertex(k * 6 + 3)) * 2.f;

							v = pTAtb->get_vertex(k * 6 + 2);
							glTexCoord2f(0, 1); glVertex3f(v->x, v->y + height, v->z);
							v = pTAtb->get_vertex(k * 6 + 3);
							glTexCoord2f(0, 0); glVertex3f(v->x, v->y + height, v->z);
							v = pTAtb->get_vertex(k * 6 + 8);
							glTexCoord2f(s, 1); glVertex3f(v->x, v->y + height, v->z);
							v = pTAtb->get_vertex(k * 6 + 9);
							glTexCoord2f(s, 0); glVertex3f(v->x, v->y + height, v->z);
						}
						glEnd();

						glBindTexture(GL_TEXTURE_2D, m_textures[pTAtb->i_texture + 2]);

						glBegin(GL_TRIANGLE_STRIP);
						for (k = 0; k < n; ++k)
						{
							s = psdl::d2v(pTAtb->get_vertex(k * 6 + 2), pTAtb->get_vertex(k * 6 + 8)) /
								psdl::d2v(pTAtb->get_vertex(k * 6 + 2), pTAtb->get_vertex(k * 6 + 3)) * 2.f;

							v = pTAtb->get_vertex(k * 6 + 2);
							glTexCoord2f(0, 1); glVertex3fv(&(v->x));
							                    glVertex3f(v->x, v->y + height, v->z);
							v = pTAtb->get_vertex(k * 6 + 8);
							glTexCoord2f(s, 1); glVertex3fv(&(v->x));
							                    glVertex3f(v->x, v->y + height, v->z);
						}
						glEnd();

						glBegin(GL_TRIANGLE_STRIP);
						for (k = 0; k < n; ++k)
						{
							s = psdl::d2v(pTAtb->get_vertex(k * 6 + 3), pTAtb->get_vertex(k * 6 + 9)) /
								psdl::d2v(pTAtb->get_vertex(k * 6 + 2), pTAtb->get_vertex(k * 6 + 3)) * 2.f;

							v = pTAtb->get_vertex(k * 6 + 3);
							glTexCoord2f(0, 1); glVertex3f(v->x, v->y + height, v->z);
							                    glVertex3fv(&(v->x));
							v = pTAtb->get_vertex(k * 6 + 9);
							glTexCoord2f(s, 1); glVertex3f(v->x, v->y + height, v->z);
							                    glVertex3fv(&(v->x));
						}
						glEnd();

						if (pTAtb->get_flag(6))
						{
							glBegin(GL_TRIANGLE_STRIP);
								v = pTAtb->get_vertex(2);
								glTexCoord2f(0, 1); glVertex3f(v->x, v->y + height, v->z);
								                    glVertex3fv(&(v->x));
								v = pTAtb->get_vertex(3);
								glTexCoord2f(1, 1); glVertex3f(v->x, v->y + height, v->z);
								                    glVertex3fv(&(v->x));
							glEnd();
						}
						if (pTAtb->get_flag(7))
						{
							glBegin(GL_TRIANGLE_STRIP);
								v = pTAtb->get_vertex(n * 6 + 2);
								glTexCoord2f(0, 1); glVertex3fv(&(v->x));
								                    glVertex3f(v->x, v->y + height, v->z);
								v = pTAtb->get_vertex(n * 6 + 3);
								glTexCoord2f(1, 1); glVertex3fv(&(v->x));
								                    glVertex3f(v->x, v->y + height, v->z);
							glEnd();
						}
					}
					break;

				case 3:
					break;
			}

			glBindTexture(GL_TEXTURE_2D, m_textures[iTex]);

		// Road surface
			glBegin(GL_TRIANGLE_STRIP);
			for (k = 0; k < n; ++k)
			{
				s = psdl::d2v(pTAtb->get_vertex(k * 6 + 1), pTAtb->get_vertex(k * 6 + 7)) /
				    psdl::d2v(pTAtb->get_vertex(k * 6 + 1), pTAtb->get_vertex(k * 6 + 2));

				glTexCoord2f(0, 0); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 1)->x));
				glTexCoord2f(0, 1); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 2)->x));
				glTexCoord2f(s, 0); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 7)->x));
				glTexCoord2f(s, 1); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 8)->x));
			}
			glEnd();

			glBegin(GL_TRIANGLE_STRIP);
			for (k = 0; k < n; ++k)
			{
				s = psdl::d2v(pTAtb->get_vertex(k * 6 + 3), pTAtb->get_vertex(k * 6 + 9)) /
				    psdl::d2v(pTAtb->get_vertex(k * 6 + 3), pTAtb->get_vertex(k * 6 + 4));

				glTexCoord2f(0, 1); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 3)->x));
				glTexCoord2f(0, 0); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 4)->x));
				glTexCoord2f(s, 1); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 9)->x));
				glTexCoord2f(s, 0); glVertex3fv(&(pTAtb->get_vertex(k * 6 + 10)->x));
			}
			glEnd();
		}
		break;
*/
	case ATB_TUNNEL:
		if (pAtb->subtype)
		{
			glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*) &iTunnelTex);
			pTunnelAtb = static_cast<psdl::tunnel*>(pAtb);
		}
		else
		{
		//	ATLTRACE("Junction\n");
			psdl::junction* pTAtb = static_cast<psdl::junction*>(pAtb);

			GLfloat s, fHeight = pTAtb->height1;
			unsigned short k, l;
			Vertex *v1, *v2;

			for (k = 0; k < pBlock->num_perimeters(); ++k)
			{
				if (pTAtb->get_wall(k))
				{
					l = (k > 0 ? k : pBlock->num_perimeters()) - 1;

					v1 = m_pDoc->get_vertex(pBlock->_perimeter[l].vertex);
					v2 = m_pDoc->get_vertex(pBlock->_perimeter[k].vertex);

					s = int(0.5f + psdl::d2v(v1, v2) / fHeight);

					glBegin(GL_TRIANGLE_STRIP);
						glTexCoord2f(0, 1); glVertex3fv(&(v1->x));
						glTexCoord2f(0, 0); glVertex3f(v1->x, v1->y + fHeight, v1->z);

						glTexCoord2f(s, 1); glVertex3fv(&(v2->x));
						glTexCoord2f(s, 0); glVertex3f(v2->x, v2->y + fHeight, v2->z);

						glTexCoord2f(0, 1); glVertex3fv(&(v1->x));
						glTexCoord2f(0, 0); glVertex3f(v1->x, v1->y + fHeight, v1->z);
					glEnd();
				}
			}
		}
		break;

	case ATB_TEXTURE:
		if (config.display.bTextures)
		{
			unsigned long iTexture = static_cast<psdl::texture*>(pAtb)->i_texture;
			if (iTexture < m_textures.size())
			{
				glBindTexture(GL_TEXTURE_2D, m_textures[iTexture]);
				glEnable(GL_TEXTURE_2D);
				iTex = iTexture;
			}
			else
			{
				iTex = 0;
				glBindTexture(GL_TEXTURE_2D, iTex);
			}
		}
		else
		{
			nHue = static_cast<psdl::texture*>(pAtb)->i_texture % 360;
			glColor3f(hue2rgb(nHue + 120),
				hue2rgb(nHue),
				hue2rgb(nHue - 120));

		//	glColorRandom(nHue);
		}
		break;

	case ATB_FACADE:
		{
			psdl::facade* pTAtb = static_cast<psdl::facade*>(pAtb);

			glBegin(GL_TRIANGLE_STRIP);

				glTexCoord2f(0, 0);
				glVertex3f(pTAtb->get_vertex(0)->x, m_pDoc->get_height(pTAtb->bottom), pTAtb->get_vertex(0)->z);

				glTexCoord2f(pTAtb->u_repeat, 0);
				glVertex3f(pTAtb->get_vertex(1)->x, m_pDoc->get_height(pTAtb->bottom), pTAtb->get_vertex(1)->z);

				glTexCoord2f(0, pTAtb->v_repeat);
				glVertex3f(pTAtb->get_vertex(0)->x, m_pDoc->get_height(pTAtb->top),    pTAtb->get_vertex(0)->z);

				glTexCoord2f(pTAtb->u_repeat, pTAtb->v_repeat);
				glVertex3f(pTAtb->get_vertex(1)->x, m_pDoc->get_height(pTAtb->top),    pTAtb->get_vertex(1)->z);

			glEnd();
		}
		break;

			}
		}
	}

	glDisable(GL_TEXTURE_2D);
//	wglMakeCurrent(NULL, NULL);
}
