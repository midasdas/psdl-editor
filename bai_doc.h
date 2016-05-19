#ifndef __BAI_DOC_H__
#define __BAI_DOC_H__

#include "docview.h"
#include "bai.h"
#include "bai_view.h"
#include "psdl.h"

class BAIDocTemplate : public DocTemplate<bai, BAIView>
{
private:

	typedef DocTemplate<bai, BAIView> baseClass;

public:

	std::string GetExtensionString(void) const
	{
		return _T("MM2 Binary AI (*.bai)\0*.bai\0");
	}

	void RenderScene(void)
	{
		m_pView->RenderScene();
	}

	bool GenerateAIRoutes (psdl*, ProgressMonitor*);
	bool GenerateAICulling(psdl*, ProgressMonitor*);
	bool GenerateBAI      (psdl*, ProgressMonitor*);

	class GenerateBAIThread
	{
	public:

		static BAIDocTemplate* pDocTmpl;
		static psdl* pPSDL;

		static unsigned _stdcall Run(void* pArgs)
		{
			return pDocTmpl->GenerateBAI(pPSDL, static_cast<ProgressMonitor*>(pArgs));
		}
	};
};

#endif
