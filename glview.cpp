#include "stdafx.h"
#include "glview.h"

void COpenGLView::RenderAxes(void)
{
	glBegin(GL_LINES);
		// red x axis
		glColor3f(1.f,0.f,0.f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(1.0f,0.0f,0.0f);
		glVertex3f(1.0f,0.0f,0.0f);
		glVertex3f(0.9f,0.1f,0.0f);
		glVertex3f(1.0f,0.0f,0.0f);
		glVertex3f(0.9f,-0.1f,0.0f);
		// green y axis
		glColor3f(0.f,1.f,0.f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,1.0f,0.0f);
		glVertex3f(0.0f,1.0f,0.0f);
		glVertex3f(0.1f,0.9f,0.0f);
		glVertex3f(0.0f,1.0f,0.0f);
		glVertex3f(-0.1f,0.9f,0.0f);
		// blue z axis
		glColor3f(0.f,0.f,1.f);
		glVertex3f(0.0f,0.0f,0.0f);
		glVertex3f(0.0f,0.0f,1.0f);
		glVertex3f(0.0f,0.0f,1.0f);
		glVertex3f(0.0f,0.1f,0.9f);
		glVertex3f(0.0f,0.0f,1.0f);
		glVertex3f(0.0f,-0.1f,0.9f);
	glEnd();
}

LRESULT COpenGLView::OnPaint(UINT, WPARAM, LPARAM, BOOL&)
{
	wglMakeCurrent(m_hDC, m_hRC);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 	glPushMatrix();
		glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(60.0, dAspect, 1.0, 5000.0);
		glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glViewport(0, 0, nWidth, nHeight);

	glPolygonMode(GL_FRONT, GL_LINE);

	glPushMatrix();
		glTranslatef(0.f, 0.f, -200.f);
		glRotatef(xRot, 1.f, 0.f, 0.f);
		glRotatef(yRot, 0.f, 1.f, 0.f);
		glScalef(.3f, .3f, .3f);
		::SendMessage(GetParent(), WM_PAINT_DESCENDANTS, (WPARAM) m_hDC, (LPARAM) m_hRC);
	glPopMatrix();

	glViewport(0, 0, nWidth / 5, nHeight / 5);

	glPushMatrix();
		glTranslatef(0.f, 0.f, -2.5f);
		glRotatef(xRot, -1.f, 0.f, 0.f);
		glRotatef(yRot, 0.f, -1.f, 0.f);
		RenderAxes();
	glPopMatrix();

	SwapBuffers(m_hDC);
	wglMakeCurrent(NULL, NULL);
	ValidateRect(NULL);
	return 0;
}

LRESULT COpenGLView::OnSize(UINT, WPARAM, LPARAM lParam, BOOL& bHandled)
{
	nWidth  = LOWORD(lParam);
	nHeight = max(HIWORD(lParam), 1);
	dAspect = (GLdouble) nWidth / (GLdouble) nHeight;

	Invalidate(TRUE);
	return 0;
}

LRESULT COpenGLView::OnKeyDown(UINT, WPARAM wParam, LPARAM, BOOL&)
{
	GLfloat fAdd = 5.0f;

	if (KEYDOWN(VK_SHIFT))	fAdd *= 3;

	switch (wParam)
	{
		case VK_UP:		xRot += fAdd; break;
		case VK_DOWN:	xRot -= fAdd; break;
		case VK_LEFT:	yRot += fAdd; break;
		case VK_RIGHT:	yRot -= fAdd; break;
	}

	xRot = GLfloat((const int) xRot % 360);
	yRot = GLfloat((const int) yRot % 360);

	Invalidate(TRUE);

	return 0;
}
