#ifndef __PSDL_H__
#define __PSDL_H__

#include "io_error.h"

#include <fstream>
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

class psdl
{
public:

	typedef struct
	{
		float x, y, z;
	}
	vertex;

	typedef struct
	{
		unsigned short vertex;
		unsigned short block;
	}
	perimeter_pt;

	class attribute
	{
		public:

			attribute() : enabled(true) { }
			virtual ~attribute() { }

			virtual attribute* clone(void) {
				return new attribute(*this);
			}

			unsigned char type, subtype;
			bool last, enabled;
	};

	class vertex_based : public attribute
	{
		public:

			void insert_vertex(unsigned short i_vertex, unsigned short i_pos)
			{
				_i_vertices.insert(_i_vertices.begin() + i_pos, i_vertex);		
			}
			void add_vertex(unsigned short i_vertex)
			{
				_i_vertices.push_back(i_vertex);
			}
			unsigned short get_vertex_ref(unsigned short i_pos)
			{
				return _i_vertices[i_pos];
			}
			void set_vertex_ref(unsigned short i_pos, unsigned short i_vertex)
			{
				_i_vertices[i_pos] = i_vertex;
			}
			unsigned short num_vertices(void)
			{
				return _i_vertices.size();
			}

			virtual void f_write(std::ofstream& f)
			{
				for (unsigned short i = 0; i < num_vertices(); i++)
					f.write((char*) &_i_vertices[i], 2);
			}

		private:

			std::vector<unsigned short> _i_vertices;
	};

	class facade_base : public attribute
	{
		public:

			unsigned short top;   // index in the height list
			unsigned short left;  // index in the vertex list
			unsigned short right; // index in the vertex list
	};

// 0x0
	class road_strip : public vertex_based
	{
		public:

			// conversion to attribute class
			operator attribute() { return attribute(); }

			unsigned short num_sections(void)
			{
				return num_vertices() / 4;
			}

			void f_write(std::ofstream& f)
			{
				if (!subtype)
				{
					unsigned short n_sections = num_sections();
					f.write((char*) &n_sections, 2);
				}

				vertex_based::f_write(f);
			}
	};

// 0x1
	class sidewalk_strip : public vertex_based
	{
		public:

			// conversion to attribute class
			operator attribute() { return attribute(); }

			unsigned short num_sections(void)
			{
				return num_vertices() / 2;
			}

			void f_write(std::ofstream& f)
			{
				if (!subtype)
				{
					unsigned short n_sections = num_sections();
					f.write((char*) &n_sections, 2);
				}

				vertex_based::f_write(f);
			}
	};

// 0x2
	typedef sidewalk_strip rectangle_strip;

// 0x3
	class sliver : public facade_base
	{
		public:

			// conversion to attribute class
			operator attribute() { return attribute(); }

			unsigned short tex_scale; // value for uniformly scaling the texture
	};

// 0x4
	class crosswalk : public attribute
	{
		public:

			// conversion to attribute class
			operator attribute() { return attribute(); }

			unsigned short get_vertex_ref(unsigned char i_pos)
			{
				return i_vertices[i_pos];
			}
			void set_vertex_ref(unsigned char i_pos, unsigned short i_vertex)
			{
				i_vertices[i_pos] = i_vertex;
			}

			unsigned short i_vertices[4];
	};

// 0x5
	class road_triangle_fan : public vertex_based
	{
		public:

			// conversion to attribute class
			operator attribute() { return attribute(); }

			unsigned short num_triangles(void)
			{
				return num_vertices() - 2;
			}

			void f_write(std::ofstream& f)
			{
				if (!subtype)
				{
					unsigned short n_triangles = num_triangles();
					f.write((char*) &n_triangles, 2);
				}

				vertex_based::f_write(f);
			}
	};

// 0x6
	typedef road_triangle_fan triangle_fan;

// 0x7
	class facade_bound : public facade_base
	{
		public:

			// conversion to attribute class
			operator attribute() { return attribute(); }

			unsigned short angle;
	};
// 0x8
	class divided_road_strip : public vertex_based
	{
		public:

			// conversion to attribute class
			operator attribute() { return attribute(); }

			unsigned short num_sections(void)
			{
				return num_vertices() / 6;
			}

			void f_write(std::ofstream& f)
			{
				if (!subtype)
				{
					unsigned short n_sections = num_sections();
					f.write((char*) &n_sections, 2);
				}

				f.write((char*) &flags,     1);
				f.write((char*) &i_texture, 1);
				f.write((char*) &value,     2);

				vertex_based::f_write(f);
			}

			unsigned char  flags;
		//	unsigned char  divider_type;
			unsigned char  i_texture;
			unsigned short value; // usage depends on the divider type
	};

// 0x9
	class tunnel : public attribute
	{
		public:

			// conversion to attribute class
			operator attribute() { return attribute(); }

			bool get_flag(unsigned char flag_id)
			{
				return (flags >> flag_id) & 1;
			}
			void set_flag(unsigned char flag_id, bool state)
			{
				flags &= ~(1 << flag_id);
				flags |= state << flag_id;
			}

			unsigned short flags;
			unsigned short height1;
			unsigned short height2;
	};

	class junction : public tunnel
	{
		public:

			void add_wall(bool enable = true)
			{
				_enabled_walls.push_back(enable);
			}
			unsigned char get_wall(unsigned long i_pos)
			{
				return _enabled_walls[i_pos];
			}
			size_t num_walls(void)
			{
				return _enabled_walls.size();
			}

			unsigned short unknown3;

		private:

			std::vector<bool> _enabled_walls;
	};

// 0xa
	class texture : public attribute
	{
		public:

			// conversion to attribute class
			operator attribute() { return attribute(); }

			unsigned short i_texture;
	};

// 0xb
	class facade : public facade_base
	{
		public:

			// conversion to attribute class
			operator attribute() { return attribute(); }

			unsigned short bottom;   // index in the height list
			unsigned short u_repeat; // number of times to repeat the texture along its u-axis
			unsigned short v_repeat; // number of times to repeat the texture along its v-axis
	};

// 0xc
	class roof_triangle_fan : public vertex_based
	{
		public:

			// conversion to attribute class
			operator attribute() { return attribute(); }

			void f_write(std::ofstream& f)
			{
				if (!subtype)
				{
					unsigned short n_vertices = num_vertices() - 1;
					f.write((char*) &n_vertices, 2);
				}

				f.write((char*) &i_height, 2);

				vertex_based::f_write(f);
			}

			unsigned short i_height;
	};

	class block
	{
		std::vector<perimeter_pt>	m_perimeter;
		unsigned char				m_bType, m_bPropRule;
		unsigned short				m_nAttributeSize;
	//	PSDL						*m_owner;

		public:
			std::vector<attribute*>	m_attributes;

/*			block(PSDL *owner, unsigned char bType)
			: m_owner(owner), m_bType(bType) {}

			block(PSDL *owner)
				: m_owner(owner), m_bType(0) {}
*/
			block(unsigned char bType = BIT_PLAIN, unsigned char bPropRule = 0)
				: m_bType(bType), m_bPropRule(bPropRule), m_nAttributeSize(0) {}

			attribute* get_attribute(unsigned long i_pos);
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

			void add_perimeter_point(perimeter_pt pp)
			{
				m_perimeter.push_back(pp);
			}

			void add_perimeter_point(unsigned short i_vertex)
			{
				perimeter_pt pp = { i_vertex, 0 };
				add_perimeter_point(pp);
			}

			void addPerimeterRange(unsigned short nVertexRef, long nOffset)
			{
				perimeter_pt pp = { 0, 0 };

				for (unsigned short i = nVertexRef; i --> nVertexRef + nOffset;)
				{
					pp.vertex = i;
					m_perimeter.push_back(pp);
				}
			}

			perimeter_pt* get_perimeter_point(unsigned long i_pos)
			{
				return &m_perimeter[i_pos];
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

			void add_attribute(attribute *atb)
			{
				m_attributes.push_back(atb);
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

	size_t add_block(block block)
	{
		size_t index = m_aBlocks.size();
		m_aBlocks.push_back(block);

		m_aBlockRefs.push_back(index);
		index = m_aBlockRefs.size() - 1;

		return index;
	}

	void insert_block(block block, unsigned int nPos)
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
				if (m_aBlocks[i].get_perimeter_point(j)->block >= nPos)
					m_aBlocks[i].get_perimeter_point(j)->block++;
			}
		}

		m_aBlocks.insert(m_aBlocks.begin() + nPos, block);
	//	m_aBlockRefs.push_back(nPos);
	}

	size_t addVertex(vertex v)
	{
		size_t index = m_aVertices.size();
		m_aVertices.push_back(v);

		m_aVertexRefs.push_back(index);
		index = m_aVertexRefs.size() - 1;

		return index;
	}

	unsigned long getReferredBlockId(unsigned long nRef)
	{
		return m_aBlockRefs[nRef];
	}

	block *getRefferedBlock(unsigned long nRef)
	{
		return &m_aBlocks[m_aBlockRefs[nRef]];
	}

	block *get_block(unsigned long nIndex)
	{
		if (nIndex < m_aBlocks.size())
			return &m_aBlocks[nIndex];
		return 0;
	}

	BlockPath *getBlockPath(unsigned long nIndex)
	{
		return &m_aBlockPaths[nIndex];
	}

	long getBlockIndex(block *block);

	vertex getVertex(unsigned long nIndex)
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

	attribute* next_attribute(unsigned char yType, texture *textureRef, long nStartBlock = -1)
	{
		static size_t s_nBlock = 0, s_nAtb = 0;
		block *block = 0;

		if (nStartBlock >= 0)
		{
			s_nBlock = nStartBlock;
			s_nAtb = 0;
		}

		block = get_block(s_nBlock);

		while (block)
		{
			attribute *atb = block->get_attribute(s_nAtb);

			while (atb)
			{
				if (atb->type == yType)
					return atb;
				else if (atb->type == ATB_TEXTURE)
					textureRef = static_cast<texture*>(atb);

				atb = block->get_attribute(s_nAtb++);
			}

			block = get_block(s_nBlock++);
			s_nAtb = 0;
		}

		return 0;
	}

	size_t num_vertices(void)	{ return m_aVertices.size(); }

	void addTexture(char *sName)
	{
		m_aTextures.push_back(sName);
	}

	std::vector<char*>			m_aTextures;
	std::vector<block>			m_aBlocks;

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

	std::vector<vertex>			m_aVertices;
	std::vector<unsigned long>	m_aVertexRefs;

	std::vector<float>			m_aHeights;

	std::vector<unsigned long>	m_aBlockRefs;

	std::vector<BlockPath>		m_aBlockPaths;

	std::vector<unsigned char>	m_aJunk;

	unsigned long m_lUnknown0;

	vertex m_vMin;
	vertex m_vMax;
	vertex m_vCenter;
	vertex m_fRadius;
};

#endif
