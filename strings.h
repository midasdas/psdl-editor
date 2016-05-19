#ifndef __STRINGS_H__
#define __STRINGS_H__

#include "psdl.h"

CString AttributeString(psdl::attribute *pAtb);
CString VertexString(Vertex v);
CString IndexString(unsigned int index);

#endif
