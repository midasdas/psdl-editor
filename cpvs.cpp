#include "stdafx.h"
#include "cpvs.h"

#include <fstream>

using namespace std;

error::code cpvs::read_file(const char* filename)
{
	ifstream f(filename, ios::in | ios::binary);

	if (!f.is_open()) return error::cant_open;

	ATLTRACE("\nReading file: %s\n", filename);

	char identifier[4];
	f.read(identifier, 4);
	if (strncmp(identifier, "PVS0", 4))
		return error::wrong_format;

	return error::ok;
}

error::code cpvs::write_file(const char* filename)
{
	return error::ok;
}
