#include "stdafx.h"
#include "psdl-editor.h"
#include "options.h"
#include "glview.h"

#include "tools.h"

#include <math.h>
#include <gl\glu.h>

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
			gluPerspective(60.0, dAspect, 1.0, 1000.0);
		glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glViewport(0, 0, nWidth, nHeight);

	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, g_options.display.bWireframe ? GL_LINE : GL_FILL);

	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_LIGHTING);
//	glEnable(GL_NORMALIZE);

	GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glPushMatrix();

	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.f, 0.f, -300.f);
		glScalef(fZoom, fZoom, fZoom);

		glRotatef(xRot, 1.f, 0.f, 0.f);
		glRotatef(yRot, 0.f, 1.f, 0.f);

		glTranslatef(xPos, yPos, zPos);

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
	GLfloat fAdd = 3.0f;

	if (KEYDOWN(VK_SHIFT)) fAdd *= 2;

	switch (wParam)
	{
		case VK_UP:        xRot += fAdd; break;
		case VK_DOWN:      xRot -= fAdd; break;
		case VK_LEFT:      yRot += fAdd; break;
		case VK_RIGHT:     yRot -= fAdd; break;
		case VK_OEM_PLUS:  fZoom *= 1.1f; break;
		case VK_OEM_MINUS: fZoom /= 1.1f; break;
	}

	xRot = GLfloat((const int) xRot % 360);
	yRot = GLfloat((const int) yRot % 360);

	Invalidate(TRUE);

	return 0;
}

LRESULT COpenGLView::OnMouseClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
	mode = 0;

	if (KEYDOWN(VK_MENU))
	{
		xMouseStart = LOWORD(lParam);
		yMouseStart = HIWORD(lParam);

		if (KEYDOWN(VK_LBUTTON) && KEYDOWN(VK_RBUTTON))
		{
			mode = pan;
			xPosStart = xPos;
			zPosStart = zPos;
		}
		else if (KEYDOWN(VK_LBUTTON))
		{
			mode = rotate;
			xRotStart = xRot;
			yRotStart = yRot;
		}
		else if (KEYDOWN(VK_RBUTTON))
		{
			mode = zoom;
			fZoomStart = fZoom;
		}
	}

	if (mode)
		SetCapture();
	else
		ReleaseCapture();

	return 0;
}

LRESULT COpenGLView::OnMouseMove(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
	if (mode)
	{
		GLfloat dx = GET_X_LPARAM(lParam) - xMouseStart;
		GLfloat dy = GET_Y_LPARAM(lParam) - yMouseStart;

		if (mode == pan)
		{
			dx /= fZoom;
			dy /= fZoom;

			xPos = xPosStart;
			zPos = zPosStart;

			xPos += dx * sin(DEG2RAD(yRot + 90.f)) - dy * sin(DEG2RAD(yRot));
			zPos -= dx * cos(DEG2RAD(yRot + 90.f)) - dy * cos(DEG2RAD(yRot));
		}
		else if (mode == rotate)
		{
			xRot = int(xRotStart + dy / nHeight * 360.f) % 360;
			yRot = int(yRotStart + dx / nWidth  * 360.f) % 360;
		}
		else if (mode == zoom)
		{
			fZoom = fZoomStart / (1 + dy / nHeight);
		}

		Invalidate(TRUE);
	}

	return 0;
}

LRESULT COpenGLView::OnMouseWheel(UINT, WPARAM wParam, LPARAM, BOOL&)
{
	fZoom *= 1 + short(HIWORD(wParam)) / 1200.f;
	Invalidate(TRUE);
	return 0;
}
