#ifndef __PSDL_VIEW_H__
#define __PSDL_VIEW_H__

#include "docview.h"
#include "psdl.h"
#include "cpvs.h"

#include <gl\gl.h>

class PSDLView : public View<psdl>
{
public:
	void RenderScene(void);
	error::code LoadTextures(HDC hDC, HGLRC hRC, ProgressMonitor* pMonitor);
	error::code UnloadTextures(HDC hDC, HGLRC hRC);

	error::code LoadTextures(ProgressMonitor* pMonitor);

	void SetCPVS(cpvs* pCPVS)
	{
		m_pCPVS = pCPVS;
	}

	std::vector<GLuint> m_textures;
private:
	cpvs* m_pCPVS;
};

#endif
