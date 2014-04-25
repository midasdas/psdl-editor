#include "stdafx.h"
#include "psdl-editor.h"
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
			gluPerspective(60.0, dAspect, 1.0, 5000.0);
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
		glTranslatef(0.f, 0.f, -200.f);

		glTranslatef(xPos, yPos, zPos);
		glRotatef(xRot, 1.f, 0.f, 0.f);
		glRotatef(yRot, 0.f, 1.f, 0.f);
	//	glTranslatef(0, - (GLfloat) cos(xRot / 180 * PI) * yPos, - (GLfloat) sin(xRot / 180 * PI) * yPos);

		glScalef(zoom, zoom, zoom);
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
		case VK_UP:		xRot += fAdd; break;
		case VK_DOWN:	xRot -= fAdd; break;
		case VK_LEFT:	yRot += fAdd; break;
		case VK_RIGHT:	yRot -= fAdd; break;
		case 0x51:      zoom *= 1.1f; break;
		case 0x57:      zoom /= 1.1f; break;
	}

	xRot = GLfloat((const int) xRot % 360);
	yRot = GLfloat((const int) yRot % 360);

	Invalidate(TRUE);

	return 0;
}

LRESULT COpenGLView::OnMouseDownL(UINT, WPARAM, LPARAM lParam, BOOL&)
{
	xMouseDrag = LOWORD(lParam);
	yMouseDrag = HIWORD(lParam);

	xRotDrag = xRot;
	yRotDrag = yRot;

	xPosDrag = xPos;
	yPosDrag = yPos;
	zPosDrag = zPos;

	SetCapture();
	return 0;
}

LRESULT COpenGLView::OnMouseDownR(UINT, WPARAM, LPARAM lParam, BOOL&)
{
	xMouseDrag = LOWORD(lParam);
	yMouseDrag = HIWORD(lParam);

	zoomDrag = zoom;
	ATLTRACE("\n%f", zoomDrag);

	SetCapture();
	return 0;
}

LRESULT COpenGLView::OnMouseUpL(UINT, WPARAM, LPARAM lParam, BOOL&)
{
	ReleaseCapture();
	return 0;
}

LRESULT COpenGLView::OnMouseUpR(UINT, WPARAM, LPARAM lParam, BOOL&)
{
	xMouseDrag = LOWORD(lParam);
	yMouseDrag = HIWORD(lParam);

	xRotDrag = xRot;
	yRotDrag = yRot;
	return 0;
}

LRESULT COpenGLView::OnMouseMove(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
	if (KEYDOWN(VK_MENU)) // ALT key
	{
		int xMouse = GET_X_LPARAM(lParam);
		int yMouse = GET_Y_LPARAM(lParam);

		if (wParam & MK_LBUTTON)
		{
			if (wParam & MK_RBUTTON)
			{
				xPos = xPosDrag + (xMouse - xMouseDrag) / nWidth * 360.f;
				yPos = yPosDrag - (yMouse - yMouseDrag) / nHeight * 360.f;
			//	zPos = zPosDrag + (yMouse - yMouseDrag) / nHeight * 360.f;
			}
			else
			{
				xRot = xRotDrag + (yMouse - yMouseDrag) / nHeight * 360.f;
				yRot = yRotDrag + (xMouse - xMouseDrag) / nWidth * 360.f;

				xRot = GLfloat((const int) xRot % 360);
				yRot = GLfloat((const int) yRot % 360);
			}

			ATLTRACE("\nxPos: %f", xPos);
		}
/*		else if (wParam & MK_RBUTTON)
		{
			zoom = zoomDrag + (yMouse - yMouseDrag) / nHeight * 4;
			ATLTRACE("\n%f", zoom);
		}*/
		Invalidate(TRUE);
	}

	return 0;
}
