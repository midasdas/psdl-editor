#include "stdafx.h"
#include "cpvs.h"
#include "psdl.h"

#include <algorithm>
#include <fstream>

using namespace std;

error::code cpvs::read_file(const char* filename, ProgressMonitor* monitor, psdl* psdl)
{
//	callback(_T("Reading PVS lists..."), 0);
	monitor->setNote("Reading PVS lists...");

	error::code ret = error::ok;

	ifstream f(filename, ios::binary);

	if (!f.is_open()) return error::cant_open;

	ATLTRACE("\nReading file: %s\n", filename);

	char identifier[4];
	f.read(identifier, 4);
	if (strncmp(identifier, "PVS0", 4))
		return error::wrong_format;

	unsigned long n_size;

	f.read((char*) &n_size, 4);
	n_size -= 2;
	monitor->setMaximum(n_size);
	ATLTRACE("CPVS file for a city with %x blocks\n", n_size);

	if (psdl)
	{
		if (n_size < psdl->num_blocks())
			ret |= error::cpvs_less_blocks, ATLTRACE("less cpvs block\n");

		else if (n_size > psdl->num_blocks())
			ret |= error::cpvs_more_blocks, ATLTRACE("more cpvs block\n");
	}

	unsigned long* offsets = new unsigned long[n_size + 1];
	f.read((char*) &offsets[0], (n_size + 1) * 4);

	streampos startpos = f.tellg();
	f.seekg(0, ios_base::end);
	unsigned int endpos = f.tellg();
	f.seekg(startpos);

	unsigned long i = 0, read;
	unsigned char c, n, k, l, m;

	pvsLists.resize(n_size);

	do
	{
		pvsLists[i].reserve(n_size);

		unsigned long blocksize = offsets[i+1] - offsets[i];
		read = 0;
		l = 1;

		while (read < blocksize)
		{
			c = f.get();
			n = c & 0x7f;

			++read;

			char* buf;

			if ((c >> 7) & 1) // Read n + 1 bytes
			{
				++n;
				buf = new char[n];
				f.read((char*) &buf[0], n);
				read += n;
			}
			else // Duplicate next byte n times
			{
				buf = new char[n];
				c = f.get();
				fill_n(buf, n, c);
				++read;
			}

			if (l > 1) l = 0;

			for (k = 0, m = 0; k < n && m < n * 4; ++m)
			{
				bool b = (buf[k] >> (l * 2)) & 0x3 == 0x3;

				pvsLists[i].push_back(b);

				if (l < 3)
				{
					++l;
				}
				else
				{
					++k;
					l = 0;
				}
			}

			delete[] buf;
		}

		ATLTRACE("%u: %u\n", i, pvsLists[i].size());

		// BT TOWER
	//	pvsLists[i][146] = true;
	//	pvsLists[i][236] = true;

	//	callback(_T(""), (double) i / n_size * 100);
		monitor->setProgress(i);
		++i;
	}
	while (i < n_size && !f.eof());

	delete[] offsets;

	unsigned long remaining = 0;
	while (f.get() != EOF)
	{
		++remaining;
	}

	ATLTRACE("Remaining bytes: %u\n", remaining);

	return ret;
}

error::code cpvs::write_file(const char* filename)
{
	ofstream f(filename, ios::binary);

	if (!f.is_open()) return error::cant_open;

	ATLTRACE("\nWriting file: %s\n", filename);

	f.write("PVS0", 4);

	unsigned long n = pvsLists.size();
	unsigned long n_size = n + 2;

	f.write((char*) &n_size, 4);

	unsigned long i;

	streampos table_start = f.tellp();
	for (i = 0; i < (n + 1) * 4; ++i) f.put(0); // Reserve space for offset table
	streampos table_end  = f.tellp();

	for (i = 0; i < n; ++i)
	{
		unsigned long j = 0;

		PVSList list(pvsLists[i].begin(), pvsLists[i].end());
		list.insert(list.begin(), false);

		while (j < list.size())
		{
			unsigned long m = j;
		//	bool val = list[j];
			bool val = false; // Only compress zeros
			unsigned char equal = 0;
			unsigned char diff  = 0;
			unsigned char k, l;

			do
			{
				unsigned char matches = 0;

				for (k = 0; k < 4; ++k)
				{
					bool test = (j + k) < list.size() ? list[j + k] : false;
					if (test == val) ++matches;
				}

				if (matches == 4 && diff > 0)
					break;
				else if (matches != 4 && equal > 0)
					break;

				j += 4;

				if (matches == 4)
					++equal;
				else
					++diff;
			}
			while (equal < 0x7f && diff < 0x80 && j < list.size());

			if (equal > 0)
			{
				unsigned char data = val ? 0xff : 0;

				f.write((char*) &equal, 1);
				f.write((char*) &data,  1);
			}
			else if (diff > 0)
			{
				unsigned char hdr = 0x80 | (diff - 1);

				unsigned char* buf = new unsigned char[diff];
				fill_n(buf, diff, 0);

				k = 0, l = 0;

				for (k, m; k < diff && m < j; ++m)
				{
					if (list[m] == true) buf[k] |= 0x3 << (l * 2);

					if (l < 3)
						++l;
					else
						++k, l = 0;
				}

				f.write((char*) &hdr, 1);
				f.write((char*) &buf[0], diff);

				delete[] buf;
			}
			else
			{
				ATLTRACE("WTF %u %u-%u/%u, equal=%u diff=%u\n", k, m, j, list.size(), equal, diff);
				return error::failure;
			}
		}

		streampos pos = f.tellp();
		unsigned int offset = pos - table_end;
		f.seekp(table_start);
		f.seekp((i + 1) * 4, ios_base::cur);
		f.write((char*) &offset, 4);
		f.seekp(pos);
	}

	return error::ok;
}
