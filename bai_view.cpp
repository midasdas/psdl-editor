#include "bai_view.h"
#include "mainfrm.h"
#include "psdl.h"
#include "config.h"

#include <math.h>

using namespace std;

void DrawCircle(float *point, float r = 0.25f, int num_segments = 8) 
{ 
	glBegin(GL_TRIANGLE_FAN);
	for(int ii = 0; ii < num_segments; ii++)
	{
		float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle

		float x = r * sinf(theta);
		float z = r * cosf(theta);

		glVertex3f(*point + x, *(point + 1), *(point + 2) + z);
	}
	glEnd();
}

void BAIView::RenderScene(void)
{
//	wglMakeCurrent(hDC, hRC);

	unsigned short i, j, k;

	vector<bool> culledRoads(m_pDoc->roads.size(), false);

	if (config.display.bTestAICulling)
	{
		vector<unsigned long> selBlocks;
		CMainFrame::BlocksWindow()->GetSelected(&selBlocks);

		for (i = 0; i < selBlocks.size(); ++i)
		{
			unsigned long nBlock = selBlocks[i] + 1;

			for (j = 0; j < m_pDoc->culling[0][nBlock].size(); ++j)
			{
				culledRoads[m_pDoc->culling[0][nBlock][j]] = true;
			}
		}
	}

	GLint iMode;
	GLboolean bCullFace = glIsEnabled(GL_CULL_FACE);
	glGetIntegerv(GL_POLYGON_MODE, &iMode);

	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, GL_FILL);

	glTranslatef(0, 1.0f, 0);

	for (i = 0; i < m_pDoc->roads.size(); ++i)
	{
		if (config.display.bTestAICulling && !culledRoads[i]) continue;

	//	unsigned short nLanes = m_pDoc->roads[i].data[0].lLanesVertices.size() > 0 ? 1 : 0;
		unsigned short nLanes = m_pDoc->roads[i].data[0].lLanesVertices.size();
		glColor3f(1, 0, 0);
		for (j = 0; j < nLanes; ++j)
		{
			DrawCircle(&m_pDoc->roads[i].data[0].lLanesVertices[j][0].x);

			glBegin(GL_LINE_STRIP);
			for (k = 0; k < m_pDoc->roads[i].data[0].lLanesVertices[j].size(); ++k)
			{
				glVertex3fv(&(m_pDoc->roads[i].data[0].lLanesVertices[j][k].x));
			}
			glEnd();
		}

	//	nLanes = m_pDoc->roads[i].data[1].lLanesVertices.size() > 0 ? 1 : 0;
		nLanes = m_pDoc->roads[i].data[1].lLanesVertices.size();
		glColor3f(0, 1, 1);
		for (j = 0; j < nLanes; ++j)
		{
			DrawCircle(&m_pDoc->roads[i].data[1].lLanesVertices[j][0].x);

			glBegin(GL_LINE_STRIP);
			for (k = 0; k < m_pDoc->roads[i].data[1].lLanesVertices[j].size(); ++k)
			{
				glVertex3fv(&(m_pDoc->roads[i].data[1].lLanesVertices[j][k].x));
			}
			glEnd();
		}

		glColor3f(0, 1, 0);
		DrawCircle(&m_pDoc->roads[i].data[0].sidewalkCenter[0].x);
		glBegin(GL_LINE_STRIP);
		for (k = 0; k < m_pDoc->roads[i].data[0].sidewalkCenter.size(); ++k)
		{
			glVertex3fv(&(m_pDoc->roads[i].data[0].sidewalkCenter[k].x));
		}
		glEnd();

		DrawCircle(&m_pDoc->roads[i].data[1].sidewalkCenter[0].x);
		glBegin(GL_LINE_STRIP);
		for (k = 0; k < m_pDoc->roads[i].data[1].sidewalkCenter.size(); ++k)
		{
			glVertex3fv(&(m_pDoc->roads[i].data[1].sidewalkCenter[k].x));
		}
		glEnd();

		glColor3f(1, 1, 1);

		DrawCircle(&m_pDoc->roads[i].data[0].sidewalkOuter[0].x);
		glBegin(GL_LINE_STRIP);
		for (k = 0; k < m_pDoc->roads[i].data[0].sidewalkOuter.size(); ++k)
		{
			glVertex3fv(&(m_pDoc->roads[i].data[0].sidewalkOuter[k].x));
		}
		glEnd();

		DrawCircle(&m_pDoc->roads[i].data[0].sidewalkInner[0].x);
		glBegin(GL_LINE_STRIP);
		for (k = 0; k < m_pDoc->roads[i].data[0].sidewalkInner.size(); ++k)
		{
			glVertex3fv(&(m_pDoc->roads[i].data[0].sidewalkInner[k].x));
		}
		glEnd();

		DrawCircle(&m_pDoc->roads[i].data[1].sidewalkInner[0].x);
		glBegin(GL_LINE_STRIP);
		for (k = 0; k < m_pDoc->roads[i].data[1].sidewalkInner.size(); ++k)
		{
			glVertex3fv(&(m_pDoc->roads[i].data[1].sidewalkInner[k].x));
		}
		glEnd();

		DrawCircle(&m_pDoc->roads[i].data[1].sidewalkOuter[0].x);
		glBegin(GL_LINE_STRIP);
		for (k = 0; k < m_pDoc->roads[i].data[1].sidewalkOuter.size(); ++k)
		{
			glVertex3fv(&(m_pDoc->roads[i].data[1].sidewalkOuter[k].x));
		}
		glEnd();

/*		glBegin(GL_LINES);
			glVertex3fv(&(m_pDoc->roads[i].data[0].sidewalkInner[0].x));
			glVertex3fv(&(m_pDoc->roads[i].data[0].sidewalkOuter[0].x));
			glVertex3fv(&(m_pDoc->roads[i].data[1].sidewalkInner[0].x));
			glVertex3fv(&(m_pDoc->roads[i].data[1].sidewalkOuter[0].x));
		glEnd();
*/
		glColor3f(1, 0, 1);
		DrawCircle(&m_pDoc->roads[i].origin[0].x);
		glBegin(GL_LINE_STRIP);
		for (k = 0; k < m_pDoc->roads[i].origin.size(); ++k)
		{
			glVertex3fv(&(m_pDoc->roads[i].origin[k].x));
		}
		glEnd();

		glColor3f(1, 1, 0);
		for (k = 0; k < m_pDoc->roads[i].origin.size(); ++k)
		{
			Vector v;
			DrawCircle(&m_pDoc->roads[i].origin[k].x);
			glBegin(GL_LINES);
				glVertex3fv(&(m_pDoc->roads[i].origin[k].x));
				v = m_pDoc->roads[i].origin[k] + (m_pDoc->roads[i].xOrientation[k] * 4);
				glVertex3fv(&(v.x));

				glVertex3fv(&(m_pDoc->roads[i].origin[k].x));
				v = m_pDoc->roads[i].origin[k] + (m_pDoc->roads[i].yOrientation[k] * 4);
				glVertex3fv(&(v.x));

				glVertex3fv(&(m_pDoc->roads[i].origin[k].x));
				v = m_pDoc->roads[i].origin[k] + (m_pDoc->roads[i].zOrientation[k] * 4);
				glVertex3fv(&(v.x));

				glVertex3fv(&(m_pDoc->roads[i].origin[k].x));
				v = m_pDoc->roads[i].origin[k] + (m_pDoc->roads[i].wOrientation[k] * 4);
				glVertex3fv(&(v.x));
			glEnd();
		}
	}

	glColor3f(0, 1, 0);
	for (i = 0; i < m_pDoc->junctions.size(); ++i)
	{
		DrawCircle(&m_pDoc->junctions[i].center.x, 2.5f, 20);
	}

	glPolygonMode(GL_FRONT, iMode);
	if (bCullFace) glEnable(GL_CULL_FACE);

	glTranslatef(0, 0, 0);
//	wglMakeCurrent(NULL, NULL);
}
