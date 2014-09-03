#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <gl/gl.h>

#define PI 3.14159265359f
#define DEG2RAD(x) (x) * PI / 180.f
#define RAD2DEG(x) (x) * 180.f / PI

#define LS(stringID) CString(LPCTSTR(stringID))

#define RGB_AVERAGE(rgb1, rgb2) RGB(\
			(GetRValue(rgb1) + GetRValue(rgb2)) / 2,\
			(GetGValue(rgb1) + GetGValue(rgb2)) / 2,\
			(GetBValue(rgb1) + GetBValue(rgb2)) / 2)

inline float hue2rgb(unsigned short h)
{
	h %= 360;
	if (h < 60)  return h / 60.f;
	if (h < 180) return 1;
	if (h < 240) return (h - 180) / 60.f;
	return 0;
}

inline void glColorRandom(unsigned short& nHue)
{
//	static float fHue = 0;

//	static unsigned short hue = 0;

	glColor3f(hue2rgb(nHue + 120),
	          hue2rgb(nHue),
			  hue2rgb(nHue - 120));

//	fHue += 1/360.f;
//	if (fHue > 1) fHue = 0;

	nHue++;
	nHue %= 360;
}

#endif
