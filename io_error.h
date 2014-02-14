#ifndef __IO_ERROR_H__
#define __IO_ERROR_H__

namespace error
{
	typedef int code;

	const code ok               = 0x1;
	const code cant_open        = 0x2;
	const code wrong_format     = 0x4;

	const code cpvs_less_blocks = 0x8;
	const code cpvs_more_blocks = 0x10;
};

#endif
