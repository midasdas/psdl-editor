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

	COpenGLView() : nWidth(0), nHeight(0), dAspect(0), xRot(0), yRot(0) {}

	void RenderAxes(void);

	void SetDCPixelFormat(PIXELFORMATDESCRIPTOR* pfd)
	{
		int nPixelFormat = ChoosePixelFormat(m_hDC, pfd);
		SetPixelFormat(m_hDC, nPixelFormat, pfd);

		ATLTRACE("\nPixel Format: %i", nPixelFormat);

		m_hRC = wglCreateContext(m_hDC);
	}

	LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnKeyDown(UINT, WPARAM wParam, LPARAM, BOOL&);

	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		m_hDC = GetDC();
		glEnable(GL_DEPTH_TEST);
		return 0;
	}

	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		wglMakeCurrent(m_hDC, NULL);
		wglDeleteContext(m_hRC);
		return 0;
	}

	LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&)
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

	int nWidth;
	int nHeight;

	GLdouble dAspect;

	GLfloat xRot;
	GLfloat yRot;
};

#endif
