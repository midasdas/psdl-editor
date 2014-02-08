#ifndef __GLVIEW_H__
#define __GLVIEW_H__

#include <gl\gl.h>
#include <gl\glu.h>

#define WM_PAINT_DESCENDANTS WM_USER + 1

#define KEYDOWN(vkey) (GetAsyncKeyState(vkey) & 0x8000)
#define KEYUP(vkey) !KEYDOWN(vkey)

class COpenGLView : public CWindowImpl<COpenGLView>
{
public:

	BEGIN_MSG_MAP(COpenGLView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown);
		MESSAGE_HANDLER(WM_MOUSEACTIVATE, OnMouseActivate);
	END_MSG_MAP()

	COpenGLView() : xRot(0), yRot(0) {}

	void SetDCPixelFormat(PIXELFORMATDESCRIPTOR* pfd)
	{
		int nPixelFormat = ChoosePixelFormat(m_hDC, pfd);
		SetPixelFormat(m_hDC, nPixelFormat, pfd);
		ATLTRACE("\nnPixelFormat: %i", nPixelFormat);

		m_hRC = wglCreateContext(m_hDC);
	}

	void RenderAxes(void)
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

	void SetupScene(void)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
	//	glEnable(GL_CULL_FACE);
	//	glPolygonMode(GL_BACK, GL_LINE);

		glTranslatef(0.0f,0.0f,-300.0f);
		glRotatef(xRot, 1.0f, 0.0f, 0.0f);
		glRotatef(yRot, 0.0f, 1.0f, 0.0f);
	}

	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		m_hDC = GetDC();
		return 0;
	}

	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		wglMakeCurrent(m_hDC, NULL);
		wglDeleteContext(m_hRC);

		return 0;
	}

	LRESULT OnSize(UINT, WPARAM, LPARAM lParam, BOOL& bHandled)
	{
		int w = LOWORD(lParam);
		int h = max(HIWORD(lParam), 1);
	//	ATLTRACE("\n%i : %i x %i", (int) m_hWnd, w, h);
		GLdouble dAspect = (GLdouble) w / (GLdouble) h;

		wglMakeCurrent(m_hDC, m_hRC);

		glViewport(0, 0, w, h);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0, dAspect, 1.0, 5000.0);

	/*	if (w < h)
			glFrustum(-100.0, 100.0, -100 / dAspect, 100 / dAspect, 1.0, 400.0);
		else
			glFrustum(-100.0 * dAspect, 100.0 * dAspect, -100, 100, 1.0, 400.0);*/

	//	GLdouble fovY = 60.0;
			
	//	h = tan( fovY / 360 * pi ) * zNear


	//	glFrustum(-w/2, w/2, -h/2, h/2, 1.0, 400.0);

		wglMakeCurrent(NULL, NULL);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		InvalidateRect(NULL, FALSE);

		return 0;
	}

	LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&)
	{
		wglMakeCurrent(m_hDC, m_hRC);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
	//	glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT, GL_LINE);

		glPushMatrix();
		glLoadIdentity();
		glTranslatef(-.8f, -.8f, 0);
		glRotatef(xRot, -1.0f, 0.0f, 0.0f);
		glRotatef(yRot, 0.0f, -1.0f, 0.0f);
		glScalef(.15f, .15f, .15f);
		RenderAxes();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0.0f,0.0f,-300.0f);
		glRotatef(xRot, 1.0f, 0.0f, 0.0f);
		glRotatef(yRot, 0.0f, 1.0f, 0.0f);
		glScalef(.2f, .2f, .2f);
		::SendMessage(GetParent(), WM_PAINT_DESCENDANTS, (WPARAM) m_hDC, (LPARAM) m_hRC);
		glPopMatrix();

		wglMakeCurrent(NULL, NULL);

		SwapBuffers(m_hDC);
		ValidateRect(NULL);

		return 0;
	}

	LRESULT OnKeyDown(UINT, WPARAM wParam, LPARAM, BOOL&)
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

		InvalidateRect(NULL, FALSE);

		return 0;
	}

	LRESULT OnEraseBkgnd(UINT, WPARAM wParam, LPARAM, BOOL& bHandled)
	{
		return 1;
	}

	LRESULT OnMouseActivate(UINT, WPARAM, LPARAM, BOOL&)
	{
		SetFocus();
		return 0;
	}

private:
	HGLRC m_hRC;
	HDC m_hDC;
	float xRot;
	float yRot;
};

#endif
