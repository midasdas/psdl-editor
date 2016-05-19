#ifndef __BAI_VIEW_H__
#define __BAI_VIEW_H__

#include "stdafx.h"
#include "docview.h"
#include "bai.h"

#include <gl\gl.h>

class BAIView : public View<bai>
{
public:

	void RenderScene(void);
};

#endif
