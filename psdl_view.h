#ifndef __PSDL_VIEW_H__
#define __PSDL_VIEW_H__

#include "docview.h"
#include "psdl.h"

#include <gl\gl.h>

class PSDLView : public View<psdl>
{
public:
	void v(unsigned long iVert)
	{
		glVertex3fv(&(m_pDoc->get_vertex(iVert)->x));
	}

	void vdy(unsigned long iVert, GLfloat fdY)
	{
		glVertex3f(m_pDoc->get_vertex(iVert)->x,
			       m_pDoc->get_vertex(iVert)->y + fdY,
				   m_pDoc->get_vertex(iVert)->z);
	}

	void RenderScene(HDC hDC, HGLRC hRC);
	error::code LoadTextures(HDC hDC, HGLRC hRC, notify_func callbackFunc = default_callback);
	error::code UnloadTextures(HDC hDC, HGLRC hRC);

private:
	std::vector<GLuint> m_textures;
};

#endif
