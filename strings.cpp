#include "stdafx.h"
#include "strings.h"
#include "resource.h"

CString AttributeString(PSDL::Attribute *pAtb)
{
	unsigned char id		= pAtb->id & 0x7f;
	unsigned char type		= pAtb->type();
	unsigned char subtype	= pAtb->subtype();

	CString sRet, sName;

	sRet.Format("%02x ", id);

	sName.LoadString(
		IDS_ATB_UNKNOWN + (type <= ATB_ROOFTRIANGLEFAN ? type + 1 : 0)
	);

	sRet += sName;

	if (type < ATB_SLIVER && subtype) {
		CString sInfo;
		sInfo.Format(" (%u sections)", subtype);
		sRet += sInfo;
	}

	return sRet;
}

CString VertexString(Vertex vertex)
{
	ATLTRACE("Hi vertex: %f, %f, %f\n", vertex.x, vertex.y, vertex.z);
	CString sRet;
	sRet.Format("%.03f, %.03f, %.03f", vertex.x, vertex.y, vertex.z);
	return sRet;
}
