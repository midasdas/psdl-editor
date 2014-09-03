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

	COpenGLView() : nWidth(0), nHeight(0), dAspect(0), xPos(0), yPos(0), zPos(0), xRot(90), yRot(0), fZoom(.1f), mode(0) {}

	void ResetCamera(void)
	{
		xPos = yPos = zPos = yRot = 0;
		xRot = 90;
		fZoom = .1f;
	}

	void RenderAxes(void);

	void SetDCPixelFormat(PIXELFORMATDESCRIPTOR* pfd)
	{
		int nPixelFormat = ChoosePixelFormat(m_hDC, pfd);
		SetPixelFormat(m_hDC, nPixelFormat, pfd);

		ATLTRACE("\nPixel Format: %i", nPixelFormat);

		m_hRC = wglCreateContext(m_hDC);

		OnInitDone();
	}

	void OnInitDone(void)
	{
		wglMakeCurrent(m_hDC, m_hRC);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glGetFloatv(GL_MODELVIEW_MATRIX, translationMatrix);

		wglMakeCurrent(NULL, NULL);
	}

	LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnKeyDown(UINT, WPARAM wParam, LPARAM, BOOL&);
	LRESULT OnMouseClick(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnMouseWheel(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnMouseMove(UINT, WPARAM, LPARAM, BOOL&);

	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled)
	{
		m_hDC = GetDC();

/*		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_NORMALIZE);

		GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
		GLfloat diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };

		glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);

		glEnable(GL_LIGHT0);

		glEnable(GL_COLOR_MATERIAL);

		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

		glClearColor(0.f, 0.f, 0.f, 1.f);
*/
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

	GLfloat translationMatrix[16];
};

#endif
