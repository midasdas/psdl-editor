#include "stdafx.h"
#include "psdl.h"
#include "resource.h"
#include "toolwnd.h"

#include <ctime>
#include <fstream>

using namespace std;

long psdl::getBlockIndex(psdl::block *block)
{
	for (long i = 0; i < m_aBlocks.size(); i++) {
		if (&m_aBlocks[i] == block)
			return i;
	}
	return -1;
}

error::code psdl::ReadFile(const char *pathname)
{
	unsigned long i, nSize;

	ifstream f(pathname, ios::in | ios::binary);

	ATLTRACE("\nReading file: %s\n", pathname);

	char identifier[4];
	f.read(identifier, 4);
	if (strncmp(identifier, "PSD0", 4))
		return error::wrong_format;

	f.seekg(4, ios_base::cur);

	f.read((char*) &nSize, 4);
	ATLTRACE("Number of vertices: 0x%x\n", nSize);
	vertex vert;
	for (i = 0; i < nSize; i++)
	{
		f.read((char*) &vert, sizeof(vertex));
		addVertex(vert);
	//	vertices.push_back(vertex);
	//	vertexRefs.push_back(i);

	//	ATLTRACE("Vertex %x: %f, %f, %f\n", i, vertexRefs[i]->x, vertexRefs[i]->y, vertexRefs[i]->z);
	}

	f.read((char*) &nSize, 4);
	ATLTRACE("Number of heights: 0x%x\n", nSize);
	float height;
	for (i = 0; i < nSize; i++)
	{
		f.read((char*) &height, 4);
		addHeight(height);
	}

	f.read((char*) &nSize, 4);
	ATLTRACE("Number of textures: 0x%x\n", nSize - 1);
	unsigned char nLength;
	for (i = 0; i < nSize - 1; i++)
	{
		f.read((char*) &nLength, 1);
		char* textureName;

		if (nLength > 0)
		{
			textureName = new char[nLength];
			f.read(textureName, nLength);
		}
		else
		{
			textureName = "";
		}

		addTexture(textureName);

	//	ATLTRACE("Texture %x: %s\n", i, textureName);
	}

	f.read((char*) &nSize, 4);
	f.read((char*) &m_lUnknown0, 4);
	nSize--;
	ATLTRACE("Number of blocks: 0x%x\n", nSize);
	for (i = 0; i < nSize; i++)
	{
		block block;
		unsigned long j, nPerimeterPoints, nAttributeSize;

		f.read((char*) &nPerimeterPoints, 4);
		f.read((char*) &nAttributeSize, 4);

		block.setAttributeSize(nAttributeSize);

		for (j = 0; j < nPerimeterPoints; j++)
		{
			perimeter_pt point;
			f.read((char*) &point, sizeof(point));
			block.add_perimeter_point(point);
		}

		unsigned int nAttributes = 0;

		long targetPos = f.tellg() + 2 * (long) nAttributeSize;

		while (f.tellg() < targetPos)
		{
			unsigned char id, type, subtype;
			bool last;

			f.read((char*) &id, 2);
			                            // --- Masks ---
			last    = id >> 7 & 1 == 1; // 1 =	00000001
			type    = id >> 3 & 15;     // 15 =	00001111
			subtype = id & 7;

			if (type > 0xc) // Attribute doesn't exist!
			{
				f.seekg(targetPos); // Set file position to end of attribute list
				break; // Exit while loop
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

							f.read((char*) &static_cast<crosswalk*>(atb)->i_vertices[k], 2);
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
							unsigned short k, nLength;
							bool bState;

							f.read((char*) &nLength,								2);
							f.read((char*) &static_cast<junction*>(atb)->flags,		2);
							f.read((char*) &static_cast<junction*>(atb)->height1,	2);
							f.read((char*) &static_cast<junction*>(atb)->height2,	2);
							f.read((char*) &static_cast<junction*>(atb)->unknown3,	2);

							for (k = 0; k < 2 * (nLength - 4); k++)
							{
								f.read((char*) &bState, 1);
								static_cast<junction*>(atb)->add_wall(bState);
							}
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
		if (i == 0x487)
			ATLTRACE("Block %x: %x attributes\n", i, nAttributes);
	}

//	fseek(f, 1, SEEK_CUR);
	unsigned char unknown1 = f.get();
	ATLTRACE("unknown1 = %x\n", unknown1);

	for (i = 0; i < nSize; i++)
	{
		unsigned char bType;
		f.read((char*) &bType, 1);
		m_aBlocks[i].setType(bType);
	}

//	fseek(f, 1, SEEK_CUR);
	unsigned char unknown2 = f.get();
	ATLTRACE("unknown2 = %x\n", unknown2);

	for (i = 0; i < nSize; i++)
	{
		unsigned char bProp;
		f.read((char*) &bProp, 1);
		m_aBlocks[i].setPropRule(bProp);
	}

	f.read((char*) &m_vMin,		sizeof(vertex));
	f.read((char*) &m_vMax,		sizeof(vertex));
	f.read((char*) &m_vCenter,	sizeof(vertex));
	f.read((char*) &m_fRadius,	4);

	unsigned char l;
	unsigned short j, nRoadBlock;
	float fDensity;

	f.read((char*) &nSize, 4);
	ATLTRACE("Number of blockpaths: 0x%x\n", nSize);

	for (i = 0; i < nSize; i++)
	{
		BlockPath *path = new BlockPath();;

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

		addBlockPath(path);
	}

	unsigned long nRemaining = 0;
	unsigned char data;

	while (data = f.get() != EOF)
	{
		m_aJunk.push_back(data);
		nRemaining++;
	}

	ATLTRACE("Remaining bytes: 0x%x\n", nRemaining);

//	unsigned long nRand = rand() % numVertices();
//	ATLTRACE("Testing vertex %x: %f, %f, %f\n",
//		nRand, getVertex(nRand).x, getVertex(nRand).y, getVertex(nRand).z);

	return error::ok;
}

bool psdl::WriteFile(const char *pathname)
{
/*	unsigned long i = 0, nSize = 0;

	FILE *f = fopen(pathname, "wb");

	if (!f) return 0;

	ATLTRACE("\nWriting file: %s\n", pathname);

	fputs("PSD0", f);
	unsigned long nTargetSize = 2;
	fwrite(&nTargetSize, 4);

	nSize = numVertices();
	fwrite(&nSize, 4);
	Vertex vertex;
	for (i = 0; i < nSize; i++)
	{
		vertex = getVertex(i);
		fwrite(&vertex, sizeof(vertex));
	}

	nSize = numHeights();
	fwrite(&nSize, 4);
	float height;
	for (i = 0; i < nSize; i++)
	{
		height = getHeight(i);
		fwrite(&height, sizeof(height));
	}

	nSize = numTextures() + 1;
	fwrite(&nSize, 4);
	unsigned char nLength = 0;
	for (i = 0; i < nSize - 1; i++)
	{
		char *textureName = getTextureName(i);
		nLength = strlen(textureName) + 1;

		if (nLength > 1)
		{
			fwrite(&nLength, 1);
			fwrite(textureName, 1, nLength, f);
		}
		else
		{
			fputc(0, f);
		}
	}

	nSize = numBlocks() + 1;
	fwrite(&nSize, 4);
	fwrite(&m_lUnknown0, 4);
//	ATLTRACE("Number of blocks: 0x%x\n", nSize);
	nSize--;
	for (i = 0; i < nSize; i++)
	{
		Block *block = getBlock(i);

		unsigned long j = 0, nPerimeterPoints = 0, nAttributeSize = 0, nAttributes = 0;

		nPerimeterPoints = block->numPerimeterPoints();
		fwrite(&nPerimeterPoints, 4);
		nAttributeSize = block->getAttributeSize();
		fwrite(&nAttributeSize, 4);

		for (j = 0; j < nPerimeterPoints; j++)
		{
			perimeter_pt point = *block->getPerimeterPoint(j);
			fwrite(&point, sizeof(point));
		}

		nAttributes = block->numAttributes();
		if (nAttributes == 0) continue;

	//	long targetPos = ftell(f) + 2 * nAttributeSize;
		j = 0;

		while (j < nAttributes)
		{
			unsigned char id = 0, last = 0, type = 0, subtype = 0;
			Attribute *atb = block->getAttribute(j);

		//	ATLTRACE("Block %x: writing attribute %x ...\n", i, j);

			id = atb->id;
			fwrite(&id, 1);
			fputc(0, f);

			last	= atb->id >> 7 & 1;
			type	= atb->type();
			subtype	= atb->subtype();

			unsigned short k;

			switch (type)
			{
				case 0x0:	static_cast<RoadStrip*>(atb)->writeToFile(f);		break;
				case 0x1:	static_cast<SidewalkStrip*>(atb)->writeToFile(f);	break;
				case 0x2:	static_cast<RectangleStrip*>(atb)->writeToFile(f);	break;

				case 0x3:
					fwrite(&static_cast<Sliver*>(atb)->top,				2);
					fwrite(&static_cast<Sliver*>(atb)->textureScale,	2);
					fwrite(&static_cast<Sliver*>(atb)->left,			2);
					fwrite(&static_cast<Sliver*>(atb)->right,			2);
					break;

				case 0x4:
					for (k = 0; k < 4; k++)
						fwrite(&static_cast<Crosswalk*>(atb)->vertexRefs[k], 2);
					break;

				case 0x5:	static_cast<RoadTriangleFan*>(atb)->writeToFile(f);	break;
				case 0x6:	static_cast<TriangleFan*>(atb)->writeToFile(f);		break;

				case 0x7:
					fwrite(&static_cast<FacadeBound*>(atb)->angle,	2);
					fwrite(&static_cast<FacadeBound*>(atb)->top,	2);
					fwrite(&static_cast<FacadeBound*>(atb)->left,	2);
					fwrite(&static_cast<FacadeBound*>(atb)->right,	2);
					break;

				case 0x8:	static_cast<DividedRoadStrip*>(atb)->writeToFile(f);	break;

				case 0x9:
					if (subtype)
					{
						fwrite(&static_cast<Tunnel*>(atb)->flags,	2);
						fwrite(&static_cast<Tunnel*>(atb)->height1,	2);

						if (subtype > 2)
							fwrite(&static_cast<Tunnel*>(atb)->height2, 2);
					}
					else
					{
						unsigned char bState;
						unsigned short nLength = static_cast<Junction*>(atb)->numWalls() / 2 + 4;

						fwrite(&nLength,								2,	1, f);
						fwrite(&static_cast<Junction*>(atb)->flags,		2,	1, f);
						fwrite(&static_cast<Junction*>(atb)->height1,	2,	1, f);
						fwrite(&static_cast<Junction*>(atb)->height2,	2,	1, f);
						fwrite(&static_cast<Junction*>(atb)->unknown3,	2,	1, f);

						for (k = 0; k < static_cast<Junction*>(atb)->numWalls(); k++)
						{
							bState = static_cast<Junction*>(atb)->getWall(k);
							fwrite(&bState, 1);
						}
					}
					break;

				case 0xa:
					{
						unsigned short textureRef = static_cast<Texture*>(atb)->textureRef
							+ 1 - (256 * subtype);
						fwrite(&textureRef, 2);
					}
					break;

				case 0xb:
					fwrite(&static_cast<Facade*>(atb)->bottom,	2);
					fwrite(&static_cast<Facade*>(atb)->top,		2);
					fwrite(&static_cast<Facade*>(atb)->uRepeat,	2);
					fwrite(&static_cast<Facade*>(atb)->vRepeat,	2);
					fwrite(&static_cast<Facade*>(atb)->left,	2);
					fwrite(&static_cast<Facade*>(atb)->right,	2);
					break;

				case 0xc:
					static_cast<RoofTriangleFan*>(atb)->writeToFile(f); break;
			}

		//	if (last) break;
			j++;
		}

		ATLTRACE("Block %x: done!\n", i);
	}

	fputc(0x00, f);

	for (i = 0; i < nSize; i++)
	{
		unsigned char bType = m_aBlocks[i].getType();
		fwrite(&bType, 1);
	}

	fputc(0xcd, f);

	for (i = 0; i < nSize; i++)
	{
		unsigned char bProp = m_aBlocks[i].getPropRule();
		fwrite(&bProp, 1);
	}

	fwrite(&m_vMin,		sizeof(Vertex),	1, f);
	fwrite(&m_vMax,		sizeof(Vertex),	1, f);
	fwrite(&m_vCenter,	sizeof(Vertex),	1, f);
	fwrite(&m_fRadius,	4,	1, f);

	unsigned char l;
	unsigned short j, nRoadBlock;
	float fDensity;
	BlockPath *path;

	nSize = numBlockPaths();
	fwrite(&nSize, 4);
	ATLTRACE("Number of blockpaths: 0x%x\n", nSize);

	for (i = 0; i < nSize; i++)
	{
		path = getBlockPath(i);

		fwrite(&path->unknown4,	2,	1, f);
		fwrite(&path->unknown5,	2,	1, f);
		fwrite(&path->nFLanes,	1,	1, f);
		fwrite(&path->nBLanes,	1,	1, f);

		for (j = 0; j < path->nFLanes + path->nBLanes; j++)
		{
			fDensity = path->density[j];
			fwrite(&fDensity, 4);
		}

		fwrite(&path->unknown6, 2);

		for (l = 0; l < 4; l++)
			fwrite(&path->startCrossroads[l], 2);

		for (l = 0; l < 4; l++)
			fwrite(&path->endCrossroads[l], 2);

		fwrite(&path->nRoadBlocks, 1);

		for (l = 0; l < path->nRoadBlocks; l++)
		{
			nRoadBlock = path->roadBlocks[l];
			fwrite(&nRoadBlock, 2);
		}
	}

	for (i = 0; i < m_aJunk.size(); i++)
	{
		fputc(m_aJunk[i], f);
	}
*/
	return 1;
}

psdl::attribute *psdl::block::get_attribute(unsigned long nIndex)
{
	if (nIndex < m_attributes.size())
		return m_attributes[nIndex];
	return 0;
}

unsigned char psdl::block::getType(void)
{
	return m_bType;
}

void psdl::block::setType(unsigned char bType)
{
	m_bType = bType;

//	CString sIndex;
//	unsigned long nIndex = m_owner->GetBlockIndex(this);

//	sIndex.Format("%x", GetType());
//	g_blocks.list().SetItem(nIndex, 1, LVIF_TEXT, GetTypeString(), 0, 0, 0, 0);
}
