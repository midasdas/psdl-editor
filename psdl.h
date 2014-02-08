#ifndef __PSDL_H__
#define __PSDL_H__

#include "io_error.h"
#include <vector>

#define BIT_UNKNOWN1		0x01 // 00000001
#define BIT_SUBTERANEAN		0x02 // 00000010
#define BIT_PLAIN			0x04 // 00000100
#define BIT_ROAD			0x08 // 00001000
#define BIT_INTERSECTION	0x10 // 00010000
#define BIT_UNKNOWN2		0x20 // 00100000
#define BIT_UNKNOWN3		0x40 // 01000000
#define BIT_INST			0x80 // 10000000

#define ATB_ROAD			0x0
#define ATB_SIDEWALK		0x1
#define ATB_RECTANGLE		0x2
#define ATB_SLIVER			0x3
#define ATB_CROSSWALK		0x4
#define ATB_ROADTRIANGLEFAN	0x5
#define ATB_TRIANGLEFAN		0x6
#define ATB_FACADEBOUND		0x7
#define ATB_DIVIDEDROAD		0x8
#define ATB_TUNNEL			0x9
#define ATB_TEXTURE			0xa
#define ATB_FACADE			0xb
#define ATB_ROOFTRIANGLEFAN	0xc

#define BIT_LEFT			0x0 // enables left side
#define BIT_RIGHT			0x1 // enables right side
#define BIT_STYLE			0x2 // railing = 0 or wall = 1
#define BIT_FLATCEILING		0x3 // ignored if curved ceiling is used
#define BIT_CLOSEDSTART_L	0x4 // closes the start of the left wall
#define BIT_CLOSEDEND_L		0x5 // closes the end of the left wall
#define BIT_CLOSEDSTART_R	0x6 // ...
#define BIT_CLOSEDEND_R		0x7
#define BIT_CURVEDCEILING	0x8 // subdivided ceiling
#define BIT_OFFSETSTART_L	0x9 // chamfers the start of the left wall
#define BIT_OFFSETEND_L		0xa // chamfers the end of the left wall
#define BIT_OFFSETSTART_R	0xb // ...
#define BIT_OFFSETEND_R		0xc
#define BIT_CURVEDSIDES		0xd // subdivided sides
#define BIT_CULLED			0xe // disables outer faces if railings are used
#define BIT_UNKNOWN			0xf // probably unused

typedef struct
{
	float x, y, z;
}
Vertex;

class PSDL
{
public:

	typedef struct
	{
		unsigned short	vertex,
						block;
	}
	PerimeterPoint;

	class Attribute
	{
		bool enabled;

		protected:
			void enable(void)	{ enabled = true;		}
			void disable(void)	{ enabled = false;		}
			void toggle(void)	{ enabled = !enabled;	}

		public:
			Attribute() : enabled(true) {}
			virtual ~Attribute() {}
			virtual Attribute* clone(void) { return new Attribute(*this); }

			unsigned char id;
			unsigned char type(void)	{ return id >> 3 & 15;	}
			unsigned char subtype(void)	{ return id & 7;		}
			bool hasLastFlag(void)		{ return id >> 7 & 1;	}
			void setLastFlag(void)		{ id |= 1 << 7;			}
	};

	class VertexBased : public Attribute
	{
		public:
			void addVertex(void) {
				vertexRefs.push_back(0);
			}
			void addVertex(unsigned short vertexRef) {
				vertexRefs.push_back(vertexRef);
			}
			void insertVertex(unsigned short pos) {
				vertexRefs.insert(vertexRefs.begin() + pos, 0);			
			}
			unsigned short getVertexRef(unsigned short id) {
				return vertexRefs[id];
			}
			void setVertexRef(size_t nIndex, unsigned short nValue) {
				vertexRefs[nIndex] = nValue;
			}
			size_t numVertices(void) {
				return vertexRefs.size();
			}
			virtual void writeToFile(FILE *f)
			{
				for (unsigned short k = 0; k < numVertices(); k++)
				{
					fwrite(&vertexRefs[k], sizeof(short), 1, f);
				}
			}

			std::vector<unsigned short> vertexRefs;
	};

	class FacadeBase : public Attribute
	{
		public:
			unsigned short top;		// index in the height list
			unsigned short left;	// index in the vertex list
			unsigned short right;	// index in the vertex list
	};

// 0x0
	class RoadStrip : public VertexBased
	{
		public:
			size_t numSections(void) {
				return numVertices() / 4;
			}

			void writeToFile(FILE *f)
			{
				if (!subtype())
				{
					unsigned short nSections = numSections();
					fwrite(&nSections, sizeof(short), 1, f);
				}
				VertexBased::writeToFile(f);
			}

			Attribute* clone(void) { return new RoadStrip(*this); }
	};
// 0x1
	class SidewalkStrip : public VertexBased
	{
		public:
			size_t numSections(void) {
				return numVertices() / 2;
			}

			void writeToFile(FILE *f)
			{
				if (!subtype())
				{
					unsigned short nSections = numSections();
					fwrite(&nSections, sizeof(short), 1, f);
				}
				VertexBased::writeToFile(f);
			}

			Attribute* clone(void) { return new SidewalkStrip(*this); }
	};
// 0x2
	class RectangleStrip : public SidewalkStrip {};
// 0x3
	class Sliver : public FacadeBase
	{
		public:
			unsigned short textureScale; // value for uniformly scaling the texture

			Attribute* clone(void) { return new Sliver(*this); }
	};
// 0x4
	class Crosswalk : public Attribute
	{
		public:
			unsigned short getVertexRef(unsigned char id) {
				return vertexRefs[id];
			}
			void setVertexRef(unsigned char nIndex, unsigned short nValue) {
				vertexRefs[nIndex] = nValue;
			}

			unsigned short vertexRefs[4];

			Attribute* clone(void) { return new Crosswalk(*this); }
	};
// 0x5
	class RoadTriangleFan : public VertexBased
	{
		public:
			size_t numTriangles(void) {
				return numVertices() - 2;
			}

			void writeToFile(FILE *f)
			{
				if (!subtype())
				{
					unsigned short nTriangles = numTriangles();
					fwrite(&nTriangles, sizeof(short), 1, f);
				}
				VertexBased::writeToFile(f);
			}

			Attribute* clone(void) { return new RoadTriangleFan(*this); }
	};
// 0x6
	class TriangleFan : public RoadTriangleFan {};
// 0x7
	class FacadeBound : public FacadeBase
	{
		public:
			unsigned short angle;

			Attribute* clone(void) { return new FacadeBound(*this); }
	};
// 0x8
	class DividedRoadStrip : public VertexBased
	{
		public:
			size_t numSections(void) {
				return numVertices() / 6;
			}

			void writeToFile(FILE *f)
			{
				if (!subtype())
				{
					unsigned short nSections = numSections();
					fwrite(&nSections, sizeof(short), 1, f);
				}

				fwrite(&flags,		sizeof(char), 1, f);
				fwrite(&textureRef,	sizeof(char), 1, f);
				fwrite(&value,		sizeof(short), 1, f);

				VertexBased::writeToFile(f);
			}

			unsigned char	flags;
			unsigned char	dividerType;
			unsigned char	textureRef;
			unsigned short	value; // usage depends on the divider type

			Attribute* clone(void) { return new DividedRoadStrip(*this); }
	};
// 0x9
	class Tunnel : public Attribute
	{
		public:
			bool getFlag(unsigned char nID)
			{
				return (flags >> nID) & 1;
			}
			void setFlag(unsigned char nID, bool bState)
			{
				flags &= ~(1 << nID);
				flags |= bState << nID;
			}

			unsigned short	flags,
							height1,
							height2;

			Attribute* clone(void) { return new Tunnel(*this); }
	};
	class Junction : public Tunnel
	{
		std::vector<unsigned char> enabledWalls;

		public:
			unsigned short	unknown3;

			void addWall(unsigned char bState)
			{
				enabledWalls.push_back(bState);
			}
			unsigned char getWall(unsigned long nIndex)
			{
				return enabledWalls[nIndex];
			}
			size_t numWalls(void)
			{
				return enabledWalls.size();
			}

			Attribute* clone(void) { return new Junction(*this); }
	};
// 0xa
	class Texture : public Attribute
	{
		public:
			unsigned short textureRef;

			Attribute* clone(void) { return new Texture(*this); }
	};
// 0xb
	class Facade : public FacadeBase
	{
		public:
			unsigned short bottom;	// index in the height list
			unsigned short uRepeat; // number of times to repeat the texture along its u-axis
			unsigned short vRepeat; // number of times to repeat the texture along its v-axis

			Attribute* clone(void) { return new Facade(*this); }
	};
// 0xc
	class RoofTriangleFan : public VertexBased
	{
		public:
			void writeToFile(FILE *f)
			{
				unsigned short nVertices = numVertices() - 1;

				if (!subtype())
					fwrite(&nVertices, sizeof(short), 1, f);

				fwrite(&heightRef, sizeof(short), 1, f);

				VertexBased::writeToFile(f);
			}

			unsigned short heightRef;

			Attribute* clone(void) { return new RoofTriangleFan(*this); }
	};

	class Block
	{
		std::vector<PerimeterPoint>	m_perimeter;
		unsigned char				m_bType, m_bPropRule;
		unsigned short				m_nAttributeSize;
	//	PSDL						*m_owner;

		public:
			std::vector<Attribute*>	m_attributes;

/*			Block(PSDL *owner, unsigned char bType)
			: m_owner(owner), m_bType(bType) {}

			Block(PSDL *owner)
				: m_owner(owner), m_bType(0) {}
*/
			Block(unsigned char bType = BIT_PLAIN, unsigned char bPropRule = 0)
				: m_bType(bType), m_bPropRule(bPropRule), m_nAttributeSize(0) {}

			Attribute *getAttribute(unsigned long nIndex);
			unsigned char getType(void);
			void setType(unsigned char bType);

			unsigned char getPropRule(void)
			{
				return m_bPropRule;
			}

			void setPropRule(unsigned char bRule)
			{
				m_bPropRule = bRule;
			}

			void addPerimeterPoint(PerimeterPoint point)
			{
				m_perimeter.push_back(point);
			}

			void addPerimeterPoint(unsigned short nVertexRef)
			{
				PerimeterPoint pp = { nVertexRef, 0 };
				m_perimeter.push_back(pp);
			}

			void addPerimeterRange(unsigned short nVertexRef, long nOffset)
			{
				PerimeterPoint pp = { 0, 0 };
				
				for (unsigned short i = nVertexRef; i --> nVertexRef + nOffset;)
				{
					pp.vertex = i;
					m_perimeter.push_back(pp);
				}
			}

			PerimeterPoint *getPerimeterPoint(unsigned long nIndex)
			{
				return &m_perimeter[nIndex];
			}

			void setPerimeterPoint(size_t nIndex, unsigned short nVertexRef, long nBlockID)
			{
				m_perimeter[nIndex].vertex = nVertexRef;

				if (nBlockID > 0)
					m_perimeter[nIndex].block = nBlockID;
			}

			void emptyPerimeter(void)
			{
				m_perimeter.clear();
			}

			void addAttribute(Attribute *attribute)
			{
				m_attributes.push_back(attribute);
			}

			void setAttributeSize(long nSize)
			{
				m_nAttributeSize = nSize;
			}

			void addAttributeSize(long nSize)
			{
				m_nAttributeSize += nSize;
			}

			unsigned short getAttributeSize(void)
			{
				return m_nAttributeSize;
			}

			size_t numAttributes(void)		{	return m_attributes.size();	}
			size_t numPerimeterPoints(void) {	return m_perimeter.size();	}

	};

	class BlockPath
	{
		public:
			unsigned short		unknown4,
								unknown5;
			unsigned char		nFLanes,				// Number of lanes in the forward direction?
								nBLanes;				// Number of lanes in the backward direction?
			std::vector<float>	density;				// Traffic density on each lane?
			unsigned short		unknown6;				// Looks like some flags in a bit-field.
			unsigned short		startCrossroads[4],		// Vertices defining the road part of a
														// crossing at the start of the road.
								endCrossroads[4];		// Vertices defining the road part of a
														// crossing at the end of the road.
			unsigned char				nRoadBlocks;	// Number of blocks that make up this road.
			std::vector<unsigned short>	roadBlocks;		// Block ID + 1 of each block that makes up
														// this road.
	};

	error::code ReadFile(const char *pathname);
	bool WriteFile(const char *pathname);

	size_t addBlock(Block block)
	{
		size_t index = m_aBlocks.size();
		m_aBlocks.push_back(block);

		m_aBlockRefs.push_back(index);
		index = m_aBlockRefs.size() - 1;

		return index;
	}

	void insertBlock(Block block, unsigned int nPos)
	{
	/*	for (std::vector<unsigned long>::iterator it = m_aBlockRefs.begin() + nPos;
			it != m_aBlockRefs.end(); ++it)
		{
			(*it)++;
		}*/

		for (size_t i = 0; i < numBlocks(); i++)
		{
			for (size_t j = 0; j < m_aBlocks[i].numPerimeterPoints(); j++)
			{
				if (m_aBlocks[i].getPerimeterPoint(j)->block >= nPos)
					m_aBlocks[i].getPerimeterPoint(j)->block++;
			}
		}

		m_aBlocks.insert(m_aBlocks.begin() + nPos, block);
	//	m_aBlockRefs.push_back(nPos);
	}

	size_t addVertex(Vertex vertex)
	{
		size_t index = m_aVertices.size();
		m_aVertices.push_back(vertex);

		m_aVertexRefs.push_back(index);
		index = m_aVertexRefs.size() - 1;

		return index;
	}

	unsigned long getReferredBlockId(unsigned long nRef)
	{
		return m_aBlockRefs[nRef];
	}

	Block *getRefferedBlock(unsigned long nRef)
	{
		return &m_aBlocks[m_aBlockRefs[nRef]];
	}

	Block *getBlock(unsigned long nIndex)
	{
		if (nIndex < m_aBlocks.size())
			return &m_aBlocks[nIndex];
		return 0;
	}

	BlockPath *getBlockPath(unsigned long nIndex)
	{
		return &m_aBlockPaths[nIndex];
	}

	long getBlockIndex(Block *block);

	Vertex getVertex(unsigned long nIndex)
	{
		return m_aVertices[m_aVertexRefs[nIndex]];
	}

	char *getTextureName(unsigned long nIndex)
	{
		if (nIndex < numTextures())
			return m_aTextures[nIndex];
		return NULL;
	}

	size_t numBlocks(void)
	{
		return m_aBlocks.size();
	}

	Attribute *nextAttribute(unsigned char yType, Texture *textureRef, long nStartBlock = -1)
	{
		static size_t s_nBlock = 0, s_nAtb = 0;
		Block *block = 0;

		if (nStartBlock >= 0)
		{
			s_nBlock = nStartBlock;
			s_nAtb = 0;
		}

		block = getBlock(s_nBlock);

		while (block)
		{
			Attribute *atb = block->getAttribute(s_nAtb);

			while (atb)
			{
				if (atb->type() == yType)
					return atb;
				else if (atb->type() == ATB_TEXTURE)
					textureRef = static_cast<Texture*>(atb);

				atb = block->getAttribute(s_nAtb++);
			}

			block = getBlock(s_nBlock++);
			s_nAtb = 0;
		}

		return 0;
	}

	size_t numVertices(void)	{ return m_aVertices.size(); }

	void addTexture(char *sName)
	{
		m_aTextures.push_back(sName);
	}

	std::vector<char*>			m_aTextures;
	std::vector<Block>			m_aBlocks;

private:
	void addBlockPath(BlockPath *path)
	{
		m_aBlockPaths.push_back(*path);
	}

	void addHeight(float fHeight)
	{
		m_aHeights.push_back(fHeight);
	}

	float getHeight(unsigned long nIndex)
	{
		return m_aHeights[nIndex];
	}

	size_t numHeights(void)		{ return m_aHeights.size();		}
	size_t numTextures(void)	{ return m_aTextures.size();	}
	size_t numBlockPaths(void)	{ return m_aBlockPaths.size();	}

	std::vector<Vertex>			m_aVertices;
	std::vector<unsigned long>	m_aVertexRefs;

	std::vector<float>			m_aHeights;

	std::vector<unsigned long>	m_aBlockRefs;

	std::vector<BlockPath>		m_aBlockPaths;

	std::vector<unsigned char>	m_aJunk;

	unsigned long m_lUnknown0;

	Vertex m_vMin;
	Vertex m_vMax;
	Vertex m_vCenter;
	Vertex m_fRadius;
};

#endif
