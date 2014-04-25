#include "stdafx.h"
#include "cpvs.h"
#include "psdl.h"

#include <fstream>

using namespace std;

error::code cpvs::read_file(const char* filename, notify_func callback, psdl* psdl)
{
	callback(_T("Busy..."), 0);

	error::code ret = error::ok;

	ifstream f(filename, ios::in | ios::binary);

	if (!f.is_open()) return error::cant_open;

	ATLTRACE("\nReading file: %s\n", filename);

	char identifier[4];
	f.read(identifier, 4);
	if (strncmp(identifier, "PVS0", 4))
		return error::wrong_format;

	unsigned long n_size;

	f.read((char*) &n_size, 4);
	n_size -= 2;
	ATLTRACE("CPVS file for a city with %x blocks\n", n_size);

	if (psdl)
	{
		if (n_size < psdl->num_blocks())
			ret |= error::cpvs_less_blocks, ATLTRACE("less cpvs block\n");

		else if (n_size > psdl->num_blocks())
			ret |= error::cpvs_more_blocks, ATLTRACE("more cpvs block\n");
	}

	return ret;
}

error::code cpvs::write_file(const char* filename)
{
	return error::ok;
}
