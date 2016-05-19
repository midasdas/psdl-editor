#include "stdafx.h"
#include "strings.h"
#include "resource.h"
#include "config.h"

CString AttributeString(psdl::attribute* pAtb)
{
//	unsigned char id		= pAtb->id & 0x7f;
	unsigned char type		= pAtb->type;
	unsigned char subtype	= pAtb->subtype;
	unsigned char id		= type << 3 | subtype;

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

CString VertexString(Vertex vert)
{
	CString sRet;
	sRet.Format("%.03f, %.03f, %.03f", vert.x, vert.y, vert.z);
	return sRet;
}

CString IndexString(unsigned int index)
{
	CString sRet;
	if (config.display.eNumeral == ::Numeral::hex)
		sRet.Format("%x", index);
	else
		sRet.Format("%d", index);
	return sRet;
}
