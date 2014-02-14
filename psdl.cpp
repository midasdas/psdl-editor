#include "stdafx.h"
#include "psdl.h"
#include "resource.h"
#include "toolwnd.h"

#include <ctime>
#include <fstream>

using namespace std;

long psdl::getBlockIndex(psdl::block* block)
{
	for (long i = 0; i < _blocks.size(); i++) {
		if (&_blocks[i] == block)
			return i;
	}
	return -1;
}

error::code psdl::read_file(const char* filename)
{
	clock_t start_time = clock();
	
	unsigned long i, n_size;

	ifstream f(filename, ios::in | ios::binary);

	if (!f.is_open()) return error::cant_open;

	ATLTRACE("\nReading file: %s\n", filename);

	char identifier[4];
	f.read(identifier, 4);
	if (strncmp(identifier, "PSD0", 4))
		return error::wrong_format;

	f.seekg(4, ios_base::cur);

	f.read((char*) &n_size, 4);
	ATLTRACE("Number of vertices: 0x%x\n", n_size);
	_vertices.reserve(n_size);

	vertex vert;
	for (i = 0; i < n_size; i++)
	{
		f.read((char*) &vert, sizeof(vertex));
		add_vertex(vert);
	//	vertices.push_back(vertex);
	//	vertexRefs.push_back(i);

	//	ATLTRACE("Vertex %x: %f, %f, %f\n", i, vertexRefs[i]->x, vertexRefs[i]->y, vertexRefs[i]->z);
	}

	f.read((char*) &n_size, 4);
	ATLTRACE("Number of heights: 0x%x\n", n_size);
	_heights.reserve(n_size);

	float height;
	for (i = 0; i < n_size; i++)
	{
		f.read((char*) &height, 4);
		add_height(height);
	}

	f.read((char*) &n_size, 4);
	ATLTRACE("Number of textures: 0x%x\n", n_size - 1);
	_textures.reserve(n_size);

	unsigned char n_length;
	for (i = 0; i < n_size - 1; i++)
	{
		f.read((char*) &n_length, 1);
		char* textureName;

		if (n_length > 0)
		{
			textureName = new char[n_length];
			f.read(textureName, n_length);
		}
		else
		{
			textureName = "";
		}

		add_texname(textureName);

	//	ATLTRACE("Texture %x: %s\n", i, textureName);
	}

	f.read((char*) &n_size, 4);
	f.read((char*) &_unknown0, 4);
	n_size--;
	ATLTRACE("Number of blocks: 0x%x\n", n_size);
	_blocks.reserve(n_size);

//	vector<perimeter_pt_i>* perimeter_data = new vector<perimeter_pt_i>[n_size];

	for (i = 0; i < n_size; i++)
	{
		psdl::block block;
		unsigned long j, n_perimeters, n_attributesize;

		f.read((char*) &n_perimeters, 4);
		f.read((char*) &n_attributesize, 4);

	//	perimeter_data[i].reserve(n_perimeters);
		block._perimeter.reserve(n_perimeters);
		block.setAttributeSize(n_attributesize);

		for (j = 0; j < n_perimeters; j++)
		{
		//	perimeter_pt_i ppi;
			perimeter_pt pp;

		//	f.read((char*) &ppi.i_vertex, 2);
		//	f.read((char*) &ppi.i_block,  2);

			f.read((char*) &pp, sizeof(perimeter_pt));

		//	perimeter_data[i].push_back(ppi);
			block.add_perimeter_point(pp);
		}

		unsigned int nAttributes = 0;

		long targetPos = f.tellg() + 2 * (long) n_attributesize;

		while (f.tellg() < targetPos)
		{
			unsigned char id, type, subtype;
			bool last;

			f.read((char*) &id, 2);
			                                 // ---- masks ----
		//	last    = (id & 0x80) >> 7 == 1; // 0x80 = 10000000
		//	type    = (id & 0x78) >> 3;      // 0x78 = 01111000
		//	subtype = (id & 0x07);           // 0x07 = 00000111

			last    = id >> 7 & 0x1 == 1;    // 0x1  = 00000001
			type    = id >> 3 & 0xf;         // 0xf  = 00001111
			subtype = id      & 0x7;         // 0x7  = 00000111

			if (type > 0xc) // no such attribute!
			{
				f.seekg(targetPos); // set file position to end of attribute list
				break; // exit while loop
			}

			attribute* atb = NULL;

			switch (type)
			{
				case 0x0:
					atb = new road_strip();
					{
						unsigned short k, nSections, vertexRef;

						if (subtype)
							nSections = subtype;
						else
							f.read((char*) &nSections, 2);

						for (k = 0; k < 4 * nSections; k++)
						{
							f.read((char*) &vertexRef, 2);
							static_cast<road_strip*>(atb)->add_vertex(vertexRef);
						}
					}
					break;

				case 0x1:
					atb = new sidewalk_strip();
					{
						unsigned short k, nSections, vertexRef;

						if (subtype)
							nSections = subtype;
						else
							f.read((char*) &nSections, 2);

						for (k = 0; k < 2 * nSections; k++)
						{
							f.read((char*) &vertexRef, 2);
							static_cast<sidewalk_strip*>(atb)->add_vertex(vertexRef);
						}
					}
					break;

				case 0x2:
					atb = new rectangle_strip();
					{
						unsigned short k, nSections, vertexRef;

						if (subtype)
							nSections = subtype;
						else
							f.read((char*) &nSections, 2);

						for (k = 0; k < 2 * nSections; k++)
						{
							f.read((char*) &vertexRef, 2);
							static_cast<rectangle_strip*>(atb)->add_vertex(vertexRef);
						}
					}
					break;

				case 0x3:
					atb = new sliver();

					f.read((char*) &static_cast<sliver*>(atb)->top,       2);
					f.read((char*) &static_cast<sliver*>(atb)->tex_scale, 2);
					f.read((char*) &static_cast<sliver*>(atb)->left,      2);
					f.read((char*) &static_cast<sliver*>(atb)->right,     2);
					break;

				case 0x4:
					atb = new crosswalk();
					{
						unsigned short k;

						for (k = 0; k < 4; k++) // always 4 vertices

							f.read((char*) &static_cast<crosswalk*>(atb)->_i_vertices[k], 2);
					}
					break;

				case 0x5:
					atb = new road_triangle_fan();
					{
						unsigned short k, nTriangles, vertexRef;

						if (subtype)
							nTriangles = subtype;
						else
							f.read((char*) &nTriangles, 2);

						for (k = 0; k < nTriangles + 2; k++)
						{
							f.read((char*) &vertexRef, 2);
							static_cast<road_triangle_fan*>(atb)->add_vertex(vertexRef);
						}
					}
					break;

				case 0x6:
					atb = new triangle_fan();
					{
						unsigned short k, nTriangles, vertexRef;

						if (subtype)
							nTriangles = subtype;
						else
							f.read((char*) &nTriangles, 2);

						for (k = 0; k < nTriangles + 2; k++) {
							f.read((char*) &vertexRef, 2);
							static_cast<triangle_fan*>(atb)->add_vertex(vertexRef);
						}
					}
					break;

				case 0x7:
					atb = new facade_bound();
					f.read((char*) &static_cast<facade_bound*>(atb)->angle,	2);
					f.read((char*) &static_cast<facade_bound*>(atb)->top,	2);
					f.read((char*) &static_cast<facade_bound*>(atb)->left,	2);
					f.read((char*) &static_cast<facade_bound*>(atb)->right,	2);
					break;

				case 0x8:
					atb = new divided_road_strip();
					{
						unsigned short k, nSections, vertexRef;

						if (subtype)
							nSections = subtype;
						else
							f.read((char*) &nSections, 2);

						f.read((char*) &static_cast<divided_road_strip*>(atb)->flags,     1);
						f.read((char*) &static_cast<divided_road_strip*>(atb)->i_texture, 1);
						f.read((char*) &static_cast<divided_road_strip*>(atb)->value,     2);

						for (k = 0; k < 6 * nSections; k++) {
							f.read((char*) &vertexRef, 2);
							static_cast<divided_road_strip*>(atb)->add_vertex(vertexRef);
						}
					}
					break;

				case 0x9:
					{
						if (subtype) // subtype 2 = banktest.psdl !!
						{
							atb = new tunnel();

							f.read((char*) &static_cast<tunnel*>(atb)->flags,	2);
							f.read((char*) &static_cast<tunnel*>(atb)->height1,	2);

							if (subtype > 2)
								f.read((char*) &static_cast<tunnel*>(atb)->height2, 2);
						}
						else
						{
							atb = new junction();
							unsigned short n_length;

							f.read((char*) &n_length,                              2);
							f.read((char*) &static_cast<junction*>(atb)->flags,    2);
							f.read((char*) &static_cast<junction*>(atb)->height1,  2);
							f.read((char*) &static_cast<junction*>(atb)->height2,  2);
							f.read((char*) &static_cast<junction*>(atb)->unknown3, 2);

							n_length = 2 * (n_length - 4);
							static_cast<junction*>(atb)->_enabled_walls.resize(n_length);
							f.read((char*) &static_cast<junction*>(atb)->_enabled_walls[0], n_length);
						}
					}
					break;

				case 0xa:
					atb = new texture();
					{
						unsigned short textureRef;
						f.read((char*) &textureRef, 2);
						static_cast<texture*>(atb)->i_texture = textureRef + (256 * subtype) - 1;
					}
					break;

				case 0xb:
					atb = new facade();
					f.read((char*) &static_cast<facade*>(atb)->bottom,   2);
					f.read((char*) &static_cast<facade*>(atb)->top,      2);
					f.read((char*) &static_cast<facade*>(atb)->u_repeat, 2);
					f.read((char*) &static_cast<facade*>(atb)->v_repeat, 2);
					f.read((char*) &static_cast<facade*>(atb)->left,     2);
					f.read((char*) &static_cast<facade*>(atb)->right,    2);
					break;

				case 0xc: // Roof triangle fan
					atb = new roof_triangle_fan();
					{
						unsigned short k, nVertices, vertexRef;

						if (subtype)
							nVertices = subtype;
						else
							f.read((char*) &nVertices, 2);

						f.read((char*) &static_cast<roof_triangle_fan*>(atb)->i_height, 2);

						for (k = 0; k < nVertices + 1; k++) {
							f.read((char*) &vertexRef, 2);
							static_cast<roof_triangle_fan*>(atb)->add_vertex(vertexRef);
						}
					}
					break;
			}

			if (atb)
			{
				atb->last    = last;
				atb->type    = type;
				atb->subtype = subtype;
				block.add_attribute(atb);
				nAttributes++;
			}
		}

		add_block(block);
	//	ATLTRACE("Block %x: done!\n", i);
	}

/*	// all vertices and blocks are read, so we can convert the perimeter indices to pointers:
	for (i = 0; i < n_size; i++)
	{
		block* block = get_block(i);

		for (unsigned long j = 0; j < perimeter_data[i].size(); j++)
		{
			perimeter_pt pp =
			{
				&_vertices[perimeter_data[i][j].i_vertex],
				_blocks   [perimeter_data[i][j].i_block]
			};

			block->add_perimeter_point(pp);
		}
	}*/

	_unknown1 = f.get();
	ATLTRACE("unknown1 = %x\n", _unknown1);

	for (i = 0; i < n_size; i++)
	{
		f.read((char*) &_blocks[i].type, 1);
	}

	_unknown2 = f.get();
	ATLTRACE("unknown2 = %x\n", _unknown2);

	for (i = 0; i < n_size; i++)
	{
		f.read((char*) &_blocks[i].proprule, 1);
	}

	f.read((char*) &v_min,    sizeof(vertex));
	f.read((char*) &v_max,    sizeof(vertex));
	f.read((char*) &v_center, sizeof(vertex));
	f.read((char*) &f_radius, 4);

	unsigned char l;
	unsigned short j, nRoadBlock;
	float fDensity;

	f.read((char*) &n_size, 4);
	ATLTRACE("Number of blockpaths: 0x%x\n", n_size);
	_blockpaths.reserve(n_size);

	for (i = 0; i < n_size; i++)
	{
		blockpath *path = new blockpath();;

		f.read((char*) &path->unknown4,	2);
		f.read((char*) &path->unknown5,	2);
		f.read((char*) &path->nFLanes,	1);
		f.read((char*) &path->nBLanes,	1);

		for (j = 0; j < path->nFLanes + path->nBLanes; j++)
		{
			f.read((char*) &fDensity, 4);
			path->density.push_back(fDensity);
		}

		f.read((char*) &path->unknown6, 2);

		for (l = 0; l < 4; l++)
			f.read((char*) &path->startCrossroads[l], 2);

		for (l = 0; l < 4; l++)
			f.read((char*) &path->endCrossroads[l], 2);

		f.read((char*) &path->nRoadBlocks, 1);

		for (l = 0; l < path->nRoadBlocks; l++)
		{
			f.read((char*) &nRoadBlock, 2);
			path->roadBlocks.push_back(nRoadBlock);
		}

		add_blockpath(path);
	}

	unsigned long nRemaining = 0;
	unsigned char data;

	while (data = f.get() != EOF)
	{
		_junk.push_back(data);
		nRemaining++;
	}

	ATLTRACE("Remaining bytes: 0x%x\n", nRemaining);

//	unsigned long nRand = rand() % numVertices();
//	ATLTRACE("Testing vertex %x: %f, %f, %f\n",
//		nRand, getVertex(nRand).x, getVertex(nRand).y, getVertex(nRand).z);

	ATLTRACE("\nTime: %d", clock() - start_time);

	return error::ok;
}

error::code psdl::write_file(const char* filename)
{
	unsigned long i = 0, n_size = 0;

	ofstream f(filename, ios::out | ios::binary);

	if (!f.is_open()) return error::cant_open;

	ATLTRACE("\nWriting file: %s\n", filename);

	f.write("PSD0", 4);
	unsigned long n_target_size = 2;
	f.write((char*) &n_target_size, 4);

	n_size = num_vertices();
	f.write((char*) &n_size, 4);

	for (i = 0; i < n_size; i++)
	{
		f.write((char*) &_vertices[i], sizeof(vertex));
	}

	n_size = num_heights();
	f.write((char*) &n_size, 4);

	for (i = 0; i < n_size; i++)
	{
		f.write((char*) &_heights[i], sizeof(float));
	}

	n_size = num_textures() + 1;
	f.write((char*) &n_size, 4);

	unsigned char n_length;
	for (i = 0; i < n_size - 1; i++)
	{
		char* texname = _textures[i];
		n_length = strlen(texname) + 1;

		if (n_length > 1)
		{
			f.write((char*) &n_length, 1);
			f.write(texname, n_length);
		}
		else
		{
			f.put(0);
		}
	}

	n_size = num_blocks() + 1;
	f.write((char*) &n_size, 4);
	f.write((char*) &_unknown0, 4);
	n_size--;

	for (i = 0; i < n_size; i++)
	{
		block* block = &_blocks[i];

		unsigned long j = 0, n_perimeters = 0, n_attributesize = 0, n_attributes = 0;

		n_perimeters = block->num_perimeters();
		f.write((char*) &n_perimeters, 4);

		n_attributesize = block->attributesize;
		f.write((char*) &n_attributesize, 4);

		for (j = 0; j < n_perimeters; j++)
		{
			perimeter_pt pp = block->_perimeter[j];
			f.write((char*) &pp, sizeof(perimeter_pt));
		}

		n_attributes = block->num_attributes();
		if (n_attributes == 0) continue;

	//	long targetPos = ftell(f) + 2 * nAttributeSize;
		j = 0;

		while (j < n_attributes)
		{
			unsigned short id = 0;
			unsigned char type = 0, subtype = 0;
			bool last = false;

			attribute *atb = block->_attributes[j];

		//	ATLTRACE("Block %x: writing attribute %x ...\n", i, j);

			last	= atb->last;
			type	= atb->type;
			subtype	= atb->subtype;

			id = last << 7 | type << 3 | subtype;

			f.write((char*) &id, 2);

			unsigned short k;

			switch (type)
			{
				case 0x0: static_cast<road_strip*>(atb)->f_write(f);      break;
				case 0x1: static_cast<sidewalk_strip*>(atb)->f_write(f);  break;
				case 0x2: static_cast<rectangle_strip*>(atb)->f_write(f); break;

				case 0x3:
					f.write((char*) &static_cast<sliver*>(atb)->top,       2);
					f.write((char*) &static_cast<sliver*>(atb)->tex_scale, 2);
					f.write((char*) &static_cast<sliver*>(atb)->left,      2);
					f.write((char*) &static_cast<sliver*>(atb)->right,     2);
					break;

				case 0x4:
					for (k = 0; k < 4; k++)
						f.write((char*) &static_cast<crosswalk*>(atb)->_i_vertices[k], 2);
					break;

				case 0x5: static_cast<road_triangle_fan*>(atb)->f_write(f); break;
				case 0x6: static_cast<triangle_fan*>(atb)->f_write(f);      break;

				case 0x7:
					f.write((char*) &static_cast<facade_bound*>(atb)->angle, 2);
					f.write((char*) &static_cast<facade_bound*>(atb)->top,   2);
					f.write((char*) &static_cast<facade_bound*>(atb)->left,  2);
					f.write((char*) &static_cast<facade_bound*>(atb)->right, 2);
					break;

				case 0x8: static_cast<divided_road_strip*>(atb)->f_write(f); break;

				case 0x9:
					if (subtype)
					{
						f.write((char*) &static_cast<tunnel*>(atb)->flags,   2);
						f.write((char*) &static_cast<tunnel*>(atb)->height1, 2);

						if (subtype > 2)
							f.write((char*) &static_cast<tunnel*>(atb)->height2, 2);
					}
					else
					{
						unsigned short n_length =
							static_cast<junction*>(atb)->_enabled_walls.size() / 2 + 4;

						ATLTRACE("Tunnel: %d bytes\n", n_length);

						f.write((char*) &n_length,                              2);
						f.write((char*) &static_cast<junction*>(atb)->flags,    2);
						f.write((char*) &static_cast<junction*>(atb)->height1,  2);
						f.write((char*) &static_cast<junction*>(atb)->height2,  2);
						f.write((char*) &static_cast<junction*>(atb)->unknown3, 2);

						n_length = 2 * (n_length - 4);
						f.write((char*) &static_cast<junction*>(atb)->_enabled_walls[0], n_length);
					}
					break;

				case 0xa:
					{
						unsigned short tex_ref =
							static_cast<texture*>(atb)->i_texture + 1 - (256 * subtype);

						f.write((char*) &tex_ref, 2);
					}
					break;

				case 0xb:
					f.write((char*) &static_cast<facade*>(atb)->bottom,   2);
					f.write((char*) &static_cast<facade*>(atb)->top,      2);
					f.write((char*) &static_cast<facade*>(atb)->u_repeat, 2);
					f.write((char*) &static_cast<facade*>(atb)->v_repeat, 2);
					f.write((char*) &static_cast<facade*>(atb)->left,     2);
					f.write((char*) &static_cast<facade*>(atb)->right,    2);
					break;

				case 0xc:
					static_cast<roof_triangle_fan*>(atb)->f_write(f); break;
			}

			j++;
		}

	//	ATLTRACE("Block %x: done!\n", i);
	}

	f.put(_unknown1);

	for (i = 0; i < n_size; i++)
	{
		f.write((char*) &_blocks[i].type, 1);
	}

	f.put(_unknown2);

	for (i = 0; i < n_size; i++)
	{
		f.write((char*) &_blocks[i].proprule, 1);
	}

	f.write((char*) &v_min,    sizeof(vertex));
	f.write((char*) &v_max,    sizeof(vertex));
	f.write((char*) &v_center, sizeof(vertex));
	f.write((char*) &f_radius, 4);

	n_size = num_blockpaths();
	f.write((char*) &n_size, 4);
	ATLTRACE("Number of blockpaths: 0x%x\n", n_size);

	for (i = 0; i < n_size; i++)
	{
		blockpath* path = &_blockpaths[i];

		f.write((char*) &path->unknown4, 2);
		f.write((char*) &path->unknown5, 2);
		f.write((char*) &path->nFLanes,  1);
		f.write((char*) &path->nBLanes,  1);

		for (unsigned short j = 0; j < path->nFLanes + path->nBLanes; j++)
			f.write((char*) &path->density[j], 4);

		f.write((char*) &path->unknown6, 2);

		unsigned char l;

		for (l = 0; l < 4; l++)
			f.write((char*) &path->startCrossroads[l], 2);

		for (l = 0; l < 4; l++)
			f.write((char*) &path->endCrossroads[l], 2);

		f.write((char*) &path->nRoadBlocks, 1);

		for (l = 0; l < path->nRoadBlocks; l++)
		{
			f.write((char*) &path->roadBlocks[l], 2);
		}
	}

	if (!_junk.empty())
		f.write((char*) _junk[0], _junk.size());

	return error::ok;
}

psdl::attribute* psdl::block::get_attribute(unsigned long i_pos)
{
	if (i_pos < _attributes.size())
		return _attributes[i_pos];
	return 0;
}
