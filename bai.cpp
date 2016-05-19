#include "stdafx.h"
#include "bai.h"

#include <fstream>

using namespace std;

const unsigned char sz_short = 2, sz_long = 4, sz_float = 4, sz_vertex = 12;

error::code bai::read_file(const char* filename, ProgressMonitor* monitor)
{
	unsigned short i, k, nJunctions, nRoads;
	unsigned char j;

	ifstream f(filename, ios::in | ios::binary);

	if (!f.is_open()) return error::cant_open;

	ATLTRACE("\nReading file: %s\n", filename);

	f.seekg(0, ios_base::end);
	int fsize = f.tellg();
	f.seekg(0, ios_base::beg);

	monitor->setCaption("Loading BAI File...");
	monitor->setMaximum(fsize);

	char identifier[4];
	f.read(identifier, 4);

	if (strncmp(identifier, "CAI1", 4) != 0) return error::wrong_format;

	f.read((char*) &nJunctions, 2);
	ATLTRACE("Number of intersections: %u\n", nJunctions);

	f.read((char*) &nRoads, 2);
	ATLTRACE("Number of roads: %u\n", nRoads);

	roads.resize(nRoads);

//	callback(_T("Reading roads"), f.tellg() / fsize);

	monitor->setNote("Reading roads");

	for (i = 0; i < nRoads; ++i)
	{
	//	ATLTRACE("\nRoad %u\n", i);

	//	callback(_T(""), f.tellg() / fsize);

		unsigned short nSections, nBlocks;

		f.seekg(2, ios_base::cur); // unsigned short id

		f.read((char*) &nSections, sz_short);
		f.read((char*) &roads[i].unknown0, sz_short);
		f.read((char*) &nBlocks, sz_short);

		roads[i].blocks.resize(nBlocks);
		f.read((char*) &roads[i].blocks[0], nBlocks * sz_short);

/*		ATLTRACE("\tBlocks:\n");
		for (j = 0; j < nBlocks; ++j)
		{
			ATLTRACE("\t\t%x\n", roads[i].blocks[j]);
		}
*/
		f.read((char*) &roads[i].unknown1, sz_float);
		f.read((char*) &roads[i].unknown2, sz_float);

	//	roads[i].nSections = nSections;
	//	roads[i].nBlocks = nBlocks;

		for (j = 0; j < 2; ++j)
		{
			unsigned short nLanes, nTrams, nTrains;

			f.read((char*) &nLanes, 2);
			f.read((char*) &nTrams, 2);
			f.read((char*) &nTrains, 2);
			f.read((char*) &roads[i].data[j].unknown3, sz_short);
			f.read((char*) &roads[i].data[j].unknown4, sz_short);

			roads[i].data[j].nLanes  = nLanes;
			roads[i].data[j].nTrams  = nTrams;
			roads[i].data[j].nTrains = nTrains;

		//	ATLTRACE("\tnLanes: %u\n", nLanes);
		//	ATLTRACE("\tnTrams: %u\n", nTrams);
		//	ATLTRACE("\tnTrains: %u\n", nTrains);

			roads[i].data[j].lanesDistances.resize(nLanes);
			roads[i].data[j].distance.resize(nSections);
			roads[i].data[j].unknown.resize(11 + nLanes);
			roads[i].data[j].lLanesVertices.resize(nLanes);
			roads[i].data[j].sidewalkCenter.resize(nSections);
			roads[i].data[j].tramVertices.resize(nTrams);
			roads[i].data[j].trainVertices.resize(nTrains);
			roads[i].data[j].sidewalkInner.resize(nSections);
			roads[i].data[j].sidewalkOuter.resize(nSections);

			for (k = 0; k < nLanes; ++k)
			{
				roads[i].data[j].lanesDistances[k].resize(nSections);
				f.read((char*) &roads[i].data[j].lanesDistances[k][0], sz_float * nSections);
			}

			f.read((char*) &roads[i].data[j].distance[0], sz_float * nSections);
			f.read((char*) &roads[i].data[j].unknown[0],  sz_float * (11 + nLanes));

			for (k = 0; k < nLanes; ++k)
			{
				roads[i].data[j].lLanesVertices[k].resize(nSections);
				f.read((char*) &roads[i].data[j].lLanesVertices[k][0], sz_vertex * nSections);
			}

			f.read((char*) &roads[i].data[j].sidewalkCenter[0], sz_vertex * nSections);

			for (k = 0; k < nTrams; ++k)
			{
				roads[i].data[j].tramVertices[k].resize(nSections);
				f.read((char*) &roads[i].data[j].tramVertices[k][0], sz_vertex * nSections);
			}

			for (k = 0; k < nTrains; ++k)
			{
				roads[i].data[j].trainVertices[k].resize(nSections);
				f.read((char*) &roads[i].data[j].trainVertices[k][0], sz_vertex * nSections);
			}

			f.read((char*) &roads[i].data[j].sidewalkInner[0], sz_vertex * nSections);
			f.read((char*) &roads[i].data[j].sidewalkOuter[0], sz_vertex * nSections);
		}

		roads[i].distance.resize(nSections);
		roads[i].origin.resize(nSections);
		roads[i].xOrientation.resize(nSections);
		roads[i].yOrientation.resize(nSections);
		roads[i].zOrientation.resize(nSections);
		roads[i].wOrientation.resize(nSections);

		f.read((char*) &roads[i].distance[0],     nSections * sz_float);
		f.read((char*) &roads[i].origin[0],       nSections * sz_vertex);
		f.read((char*) &roads[i].xOrientation[0], nSections * sz_vertex);
		f.read((char*) &roads[i].yOrientation[0], nSections * sz_vertex);
		f.read((char*) &roads[i].zOrientation[0], nSections * sz_vertex);
		f.read((char*) &roads[i].wOrientation[0], nSections * sz_vertex);

	//	f.read((char*) &roads[i].end[0], 38);
	//	f.read((char*) &roads[i].end[1], 38);

		for (j = 0; j < 2; ++j)
		{
			f.read((char*) &roads[i].end[j].intersectionID,        sz_long);
			f.read((char*) &roads[i].end[j].unknown0,              sz_short);
			f.read((char*) &roads[i].end[j].vehicleRule,           sz_long);
			f.read((char*) &roads[i].end[j].intersectionRoadIndex, sz_long);
			f.read((char*) &roads[i].end[j].trafficLightOrigin,    sz_vertex);
			f.read((char*) &roads[i].end[j].trafficLightAxis,      sz_vertex);
		}

/*		ATLTRACE("\tRoad %x distances:\n", i);
		for (j = 0; j < nSections; ++j)
		{
			ATLTRACE("\t\t%f\n", roads[i].distance[j]);
		}*/

		monitor->setProgress(f.tellg());
	}

	junctions.resize(nJunctions);

//	callback(_T("Reading intersections"), f.tellg() / fsize);

	monitor->setNote("Reading intersections");

	for (i = 0; i < nJunctions; ++i)
	{
	//	callback(_T(""), f.tellg() / fsize);

		unsigned short nID, nRoads;

		f.read((char*) &nID, 2);
		f.read((char*) &junctions[i].block, 2);
		f.read((char*) &junctions[i].center, sizeof(Vertex));
		f.read((char*) &nRoads, 2);

		junctions[i].roads.resize(nRoads);
		f.read((char*) &junctions[i].roads[0], nRoads * 4);

	//	ATLTRACE("\nIntersection %u", nID);

		monitor->setProgress(f.tellg());
	}

	// -- Culling --

	unsigned long t, nBlocks; // Number of blocks in the PSDL + 1
	f.read((char*) &nBlocks, 4);

	monitor->setNote("Reading culls");

	ATLTRACE("\nnBlocks: %x\n", nBlocks - 1);

	culling[0].resize(nBlocks);
	culling[1].resize(nBlocks);

	for (t = 0; t < nBlocks; ++t)
	{
		f.read((char*) &nRoads, 2);
		culling[0][t].resize(nRoads);
		f.read((char*) &culling[0][t][0], nRoads * 2);
	}
	monitor->setProgress(f.tellg());

	for (t = 0; t < nBlocks; ++t)
	{
		f.read((char*) &nRoads, 2);
		culling[1][t].resize(nRoads);
		f.read((char*) &culling[1][t][0], nRoads * 2);
	}

	unsigned long nRemaining = 0;

	while (f.get() != EOF)
	{
		++nRemaining;
	}

	ATLTRACE("Remaining bytes: %u\n", nRemaining);

	monitor->setProgress(f.tellg());
	return error::ok;
}

error::code bai::write_file(const char* filename)
{
	unsigned short i, k, nJunctions = junctions.size(), nRoads = roads.size();
	unsigned char j;

	ofstream f(filename, ios::out | ios::binary);

	if (!f.is_open()) return error::cant_open;

	ATLTRACE("\nWriting file: %s\n", filename);

	f.write("CAI1", 4);
	f.write((char*) &nJunctions, sz_short);
	f.write((char*) &nRoads,     sz_short);

	for (i = 0; i < nRoads; ++i)
	{
		unsigned short nSections = roads[i].origin.size(), nBlocks = roads[i].blocks.size();

		f.write((char*) &i, sz_short);

		f.write((char*) &nSections,         sz_short);
		f.write((char*) &roads[i].unknown0, sz_short);
		f.write((char*) &nBlocks,           sz_short);

		f.write((char*) &roads[i].blocks[0], nBlocks * sz_short);

/*		for (j = 0; j < nBlocks; ++j)
		{
			switch (roads[i].blocks[j])
			{
			case 0x255 + 1:
			case 0x257 + 1:
			case 0x258 + 1:
			case 0x335 + 1:
			case 0x341 + 1:
			case 0x3db + 1:
			case 0x3dd + 1:
				ATLTRACE("%d: %d %d\n", i, roads[i].data[0].unknown4, roads[i].data[1].unknown4);

				roads[i].data[0].unknown4 = 1;
				roads[i].data[1].unknown4 = 1;
				break;
			}
		}*/

		f.write((char*) &roads[i].unknown1, sz_float);
		f.write((char*) &roads[i].unknown2, sz_float);

		for (j = 0; j < 2; ++j)
		{
			unsigned short nLanes =  roads[i].data[j].nLanes,
			               nTrams =  roads[i].data[j].nTrams,
			               nTrains = roads[i].data[j].nTrains;

			f.write((char*) &nLanes,  sz_short);
			f.write((char*) &nTrams,  sz_short);
			f.write((char*) &nTrains, sz_short);
			f.write((char*) &roads[i].data[j].unknown3, sz_short);
			f.write((char*) &roads[i].data[j].unknown4, sz_short);

			for (k = 0; k < nLanes; ++k)
			{
				f.write((char*) &roads[i].data[j].lanesDistances[k][0], sz_float * nSections);
			}

			f.write((char*) &roads[i].data[j].distance[0], sz_float * nSections);
			f.write((char*) &roads[i].data[j].unknown[0],  sz_float * (11 + nLanes));

			for (k = 0; k < nLanes; ++k)
			{
				f.write((char*) &roads[i].data[j].lLanesVertices[k][0], sz_vertex * nSections);
			}

			f.write((char*) &roads[i].data[j].sidewalkCenter[0], sz_vertex * nSections);

			for (k = 0; k < nTrams; ++k)
			{
				f.write((char*) &roads[i].data[j].tramVertices[k][0], sz_vertex * nSections);
			}

			for (k = 0; k < nTrains; ++k)
			{
				f.write((char*) &roads[i].data[j].trainVertices[k][0], sz_vertex * nSections);
			}

			f.write((char*) &roads[i].data[j].sidewalkInner[0], sz_vertex * nSections);
			f.write((char*) &roads[i].data[j].sidewalkOuter[0], sz_vertex * nSections);
		}

		f.write((char*) &roads[i].distance[0],     nSections * sz_float);
		f.write((char*) &roads[i].origin[0],       nSections * sz_vertex);
		f.write((char*) &roads[i].xOrientation[0], nSections * sz_vertex);
		f.write((char*) &roads[i].yOrientation[0], nSections * sz_vertex);
		f.write((char*) &roads[i].zOrientation[0], nSections * sz_vertex);
		f.write((char*) &roads[i].wOrientation[0], nSections * sz_vertex);

		for (j = 0; j < 2; ++j)
		{
			f.write((char*) &roads[i].end[j].intersectionID,        sz_long);
			f.write((char*) &roads[i].end[j].unknown0,              sz_short);
			f.write((char*) &roads[i].end[j].vehicleRule,           sz_long);
			f.write((char*) &roads[i].end[j].intersectionRoadIndex, sz_long);
			f.write((char*) &roads[i].end[j].trafficLightOrigin,    sz_vertex);
			f.write((char*) &roads[i].end[j].trafficLightAxis,      sz_vertex);
		}
	}

	for (i = 0; i < nJunctions; ++i)
	{
		nRoads = junctions[i].roads.size();

		f.write((char*) &i, sz_short);
		f.write((char*) &junctions[i].block, sz_short);
		f.write((char*) &junctions[i].center, sz_vertex);
		f.write((char*) &nRoads, sz_short);
		f.write((char*) &junctions[i].roads[0], nRoads * sz_long);
	}

	unsigned long t, nBlocks = culling[0].size();
	f.write((char*) &nBlocks, sz_long);

	ATLTRACE("Number of culling groups: %u\n", nBlocks);

	for (t = 0; t < nBlocks; ++t)
	{
		nRoads = culling[0][t].size();
		f.write((char*) &nRoads, sz_short);
		f.write((char*) &culling[0][t][0], nRoads * sz_short);
	}

	for (t = 0; t < nBlocks; ++t)
	{
		nRoads = culling[1][t].size();
		f.write((char*) &nRoads, sz_short);
		f.write((char*) &culling[1][t][0], nRoads * sz_short);
	}

	return error::ok;
}
