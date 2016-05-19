#ifndef __CPVS_H__
#define __CPVS_H__

#include "psdl.h"
#include "io_error.h"

#include <vector>

class cpvs
{
public:
//	typedef std::vector< unsigned long > PVSList; // I use block ID's, not bit arrays
	typedef std::vector< bool > PVSList;

	std::vector< PVSList > pvsLists;

	error::code read_file (const char* filename, ProgressMonitor* monitor, psdl* psdl = NULL);
	error::code write_file(const char* filename);
};

#endif
