#ifndef __GLVIEW_H__
#define __GLVIEW_H__

#include <gl\gl.h>

#define WM_PAINT_DESCENDANTS WM_USER + 1

#define KEYDOWN(vkey) (GetAsyncKeyState(vkey) & 0x8000)
#define KEYUP(vkey) !KEYDOWN(vkey)

typedef struct GLVector
{
	GLfloat x, y, z;
	GLVector() : x(0), y(0), z(0) {}
}
GLVector;

typedef struct GLPoint
{
	GLfloat x, y;
	GLPoint() : x(0), y(0) {}
}
GLPoint;

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
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel);
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove);
		MESSAGE_RANGE_HANDLER(WM_LBUTTONDOWN, WM_RBUTTONUP, OnMouseClick);
	END_MSG_MAP()

	COpenGLView() : nWidth(0), nHeight(0), dAspect(0), mode(0)
	{
		ResetCamera();
	}

	void ResetCamera(void)
	{
		xPos = yPos = zPos = yRot = 0;
		xRot = 90;
		fZoom = .1f;
	}

	void SetDCPixelFormat(PIXELFORMATDESCRIPTOR* pfd)
	{
		int nPixelFormat = ChoosePixelFormat(m_hDC, pfd);
		SetPixelFormat(m_hDC, nPixelFormat, pfd);

		ATLTRACE("\nPixel Format: %i", nPixelFormat);

		m_hRC = wglCreateContext(m_hDC);

		SetupRC();
	}

	HGLRC GetRC(void)
	{
		ATLASSERT(m_hRC);
		return m_hRC;
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

	LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&)
	{
		return 1;
	}

	LRESULT OnMouseActivate(UINT, WPARAM, LPARAM, BOOL&)
	{
		SetFocus();
		return 0;
	}

	void RenderAxes(void);
	void SetupRC(void);

	LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnKeyDown(UINT, WPARAM wParam, LPARAM, BOOL&);
	LRESULT OnMouseClick(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnMouseWheel(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnMouseMove(UINT, WPARAM, LPARAM, BOOL&);

	HGLRC m_hRC;
	HDC m_hDC;

private:

	GLdouble dAspect;

	int nWidth;
	int nHeight;

	enum
	{
		pan = 1, rotate, zoom
	};
	unsigned char mode;

	bool bWheelZoom;
	GLfloat fZoom, fZoomStart;

	GLfloat xMouseStart, yMouseStart;

	GLfloat xPos, yPos, zPos;
	GLfloat xRot, yRot;

	GLfloat xPosStart, yPosStart, zPosStart;
	GLfloat xRotStart, yRotStart;
};

#endif
