#ifndef __CPVS_H__
#define __CPVS_H__

#include "psdl.h"
#include "io_error.h"

class cpvs
{
public:

	error::code read_file (const char* filename, notify_func callback = default_callback, psdl* psdl = NULL);
	error::code write_file(const char* filename);
};

#endif
