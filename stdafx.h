#ifndef __STDAFX_H__
#define __STDAFX_H__

#define INVALID_FILE_ATTRIBUTES 0xFFFFFFFF

#define VK_OEM_PLUS     107
#define VK_OEM_MINUS    109
#define WM_MOUSEWHEEL   0x020A

#define _WTL_USE_CSTRING

#define WIN32_LEAN_AND_MEAN
#define _WIN32_IE 0x0400

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#define _ATL_USE_DDX_FLOAT
#define _ATL_USE_CSTRING_FLOAT

#include <atlframe.h>

#include <atlctrls.h>
#include <atldlgs.h>
#include <atlmisc.h>

#include <atlcrack.h>
#include <atlddx.h>

#endif
