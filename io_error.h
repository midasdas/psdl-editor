#ifndef __IO_ERROR_H__
#define __IO_ERROR_H__

#include <string>

typedef void(*notify_func)(std::string, int, ...);
inline void default_callback(std::string, int, ...) {}

namespace error
{
	typedef int code;

	const code failure          = 0x0;
	const code ok               = 0x1;
	const code aborted          = 0x2;
	const code cant_open        = 0x4;
	const code wrong_format     = 0x8;

	const code cpvs_less_blocks = 0x10;
	const code cpvs_more_blocks = 0x20;
};

#endif
