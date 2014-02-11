#ifndef __CPVS_H__
#define __CPVS_H__

#include "io_error.h"

class cpvs
{
public:

	error::code read_file (const char* filename);
	error::code write_file(const char* filename);
};

#endif
