#ifndef __PSDL_H__
#define __PSDL_H__

#include "io_error.h"

#include <fstream>
#include <vector>
#include <math.h>

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

#define SW_H .15f

class psdl;

class psdl
{
public:

	typedef struct vertex
	{
		float x, y, z;

		vertex(float x = 0,
		       float y = 0,
			   float z = 0) :
		x(x), y(y), z(z) {}
	}
	vertex;

	typedef std::vector<vertex>::iterator vertex_ref;

	typedef std::pair<vertex_ref, vertex_ref> edge;

	static float d2v(const vertex* v1, const vertex* v2)
	{
		float dx = v1->x - v2->x;
	//	float dy = v1->y - v2->y;
		float dy = 0;
		float dz = v1->z - v2->z;

		return sqrtf(dx * dx + dy * dy + dz * dz);
	}

	typedef struct
	{
		unsigned short vertex;
		unsigned short block;
	}
	perimeter_pt;


	class block;

/*	typedef struct
	{
		std::vector<vertex>::iterator vertex;
		std::vector<block*>::iterator block;
	}
	perimeter_pt;*/


// -- Base Classes -----------------------------------------------------------

	class attribute
	{
	public:
		attribute(char type) : type(type), subtype(0), last(false), enabled(true) {}
		virtual attribute* clone() const = 0;

		virtual void write(std::ostream& f) = 0;

		unsigned char type, subtype;
		bool last, enabled;
	};

	class vertex_attribute : public attribute
	{
	public:
		vertex_attribute(std::vector<vertex>* vertex_list, char type) : attribute(type) {
			_vertex_list = vertex_list;
		}

	/*	unsigned short vertex2index(std::vector<vertex>::iterator it)
		{
			return it - _vertex_list->begin();
		}*/

		virtual vertex* get_vertex(unsigned short pos) = 0;
		virtual unsigned short get_vertex_ref(unsigned short pos) = 0;
		virtual void set_vertex(unsigned short pos, unsigned short index) = 0;
		virtual unsigned short num_vertices(void) = 0;

	protected:
		void write_indices(std::ostream& f)
		{
			for (unsigned short i = 0; i < num_vertices(); ++i)
			{
				unsigned short index = get_vertex_ref(i);
				f.write((char*) &index, 2);
			}
		}

		std::vector<vertex>* _vertex_list; // Pointer to the vertex list
	};

	class vertex_container : public vertex_attribute
	{
	public:
		vertex_container(std::vector<vertex>* vertex_list, char type) : vertex_attribute(vertex_list, type) {}

		void add_vertex(unsigned short index)
		{
			_vertices.push_back(_vertex_list->begin() + index);
		}
		void add_vertex(const std::vector<vertex>::iterator& it)
		{
			_vertices.push_back(it);
		}
		virtual vertex* get_vertex(unsigned short pos)
		{
			return _vertices[pos];
		}
		virtual unsigned short get_vertex_ref(unsigned short pos)
		{
			return _vertices[pos] - _vertex_list->begin();
		}
		virtual void set_vertex(unsigned short pos, unsigned short index)
		{
			_vertices[pos] = _vertex_list->begin() + index;
		}
		virtual unsigned short num_vertices(void)
		{
			return _vertices.size();
		}

	protected:
		std::vector< std::vector<vertex>::iterator > _vertices;
	};

	class facade_base : public vertex_attribute
	{
	public:
		facade_base(std::vector<vertex>* vertex_list, char type) : vertex_attribute(vertex_list, type) {}
		virtual ~facade_base() {}

		virtual vertex* get_vertex(unsigned short pos)
		{
			return _vertices[pos];
		}
		virtual unsigned short get_vertex_ref(unsigned short pos)
		{
			return _vertices[pos] - _vertex_list->begin();
		}
		virtual void set_vertex(unsigned short pos, unsigned short index)
		{
			_vertices[pos] = _vertex_list->begin() + index;
		}
		virtual unsigned short num_vertices(void)
		{
			return 2;
		}

		unsigned short top; // Index in the height list

	protected:
		std::vector<vertex>::iterator _vertices[2];
	};


// -- PSDL Attributes --------------------------------------------------------

// 0x0
	class road_strip : public vertex_container
	{
	public:
		road_strip(std::vector<vertex>* vertex_list, char type = ATB_ROAD) : vertex_container(vertex_list, type) {}
		virtual attribute* clone(void) const {
			return new road_strip(*this);
		}

		virtual unsigned short num_sections(void)
		{
			return num_vertices() / 4;
		}

		virtual void write(std::ostream& f)
		{
			if (!subtype)
			{
				unsigned short n = num_sections();
				f.write((char*) &n, 2);
			}
			vertex_attribute::write_indices(f);
		}
	};

// 0x1
	class sidewalk_strip : public vertex_container
	{
	public:
		sidewalk_strip(std::vector<vertex>* vertex_list, char type = ATB_SIDEWALK) : vertex_container(vertex_list, type) {}
		virtual ~sidewalk_strip() {}
		virtual attribute* clone(void) const {
			return new sidewalk_strip(*this);
		}

		unsigned short num_sections(void)
		{
			return num_vertices() / 2;
		}

		virtual void write(std::ostream& f)
		{
			if (!subtype)
			{
				unsigned short n = num_sections();
				f.write((char*) &n, 2);
			}
			vertex_attribute::write_indices(f);
		}
	};

// 0x2
	class rectangle_strip : public sidewalk_strip
	{
	public:
		rectangle_strip(std::vector<vertex>* vertex_list) : sidewalk_strip(vertex_list, ATB_RECTANGLE) {}
		virtual attribute* clone(void) const {
			return new rectangle_strip(*this);
		}
	};

// 0x3
	class sliver : public facade_base
	{
	public:
		sliver(std::vector<vertex>* vertex_list) : facade_base(vertex_list, ATB_SLIVER) {}
		virtual attribute* clone(void) const {
			return new sliver(*this);
		}

		virtual void write(std::ostream& f)
		{
			f.write((char*) &top, 2);
			f.write((char*) &tex_scale, 2);

			vertex_attribute::write_indices(f);
		}

		unsigned short tex_scale; // Value for uniformly scaling the texture
	};

// 0x4
	class crosswalk : public vertex_attribute
	{
	public:
		crosswalk(std::vector<vertex>* vertex_list) : vertex_attribute(vertex_list, ATB_CROSSWALK) {}
		virtual attribute* clone(void) const {
			return new crosswalk(*this);
		}

		vertex* get_vertex(unsigned short pos)
		{
			return _vertices[pos];
		}
		virtual unsigned short get_vertex_ref(unsigned short pos)
		{
			return _vertices[pos] - _vertex_list->begin();
		}
		virtual void set_vertex(unsigned short pos, unsigned short index)
		{
			_vertices[pos] = _vertex_list->begin() + index;
		}
		unsigned short num_vertices(void)
		{
			return 4;
		}

		virtual void write(std::ostream& f)
		{
			vertex_attribute::write_indices(f);
		}

	private:
		std::vector<vertex>::iterator _vertices[4];
	};

// 0x5
	class road_triangle_fan : public vertex_container
	{
	public:
		road_triangle_fan(std::vector<vertex>* vertex_list, char type = ATB_ROADTRIANGLEFAN) : vertex_container(vertex_list, type) {}
		virtual attribute* clone(void) const {
			return new road_triangle_fan(*this);
		}

		unsigned short num_triangles(void)
		{
			return num_vertices() - 2;
		}

		virtual void write(std::ostream& f)
		{
			if (!subtype)
			{
				unsigned short n = num_triangles();
				f.write((char*) &n, 2);
			}
			vertex_attribute::write_indices(f);
		}
	};

// 0x6
	class triangle_fan : public road_triangle_fan
	{
	public:
		triangle_fan(std::vector<vertex>* vertex_list) : road_triangle_fan(vertex_list, ATB_TRIANGLEFAN) {}
		virtual attribute* clone(void) const {
			return new triangle_fan(*this);
		}
	};

// 0x7
	class facade_bound : public facade_base
	{
	public:
		facade_bound(std::vector<vertex>* vertex_list) : facade_base(vertex_list, ATB_FACADEBOUND) {}
		virtual attribute* clone(void) const {
			return new facade_bound(*this);
		}

		virtual void write(std::ostream& f)
		{
			f.write((char*) &angle, 2);
			f.write((char*) &top, 2);

			vertex_attribute::write_indices(f);
		}

		unsigned short angle;
	};

// 0x8
	class divided_road_strip : public road_strip
	{
	public:
		divided_road_strip(std::vector<vertex>* vertex_list) : road_strip(vertex_list, ATB_DIVIDEDROAD) {}
		virtual attribute* clone(void) const {
			return new divided_road_strip(*this);
		}

		virtual unsigned short num_sections(void)
		{
			return num_vertices() / 6;
		}

		bool get_flag(unsigned char flag_id)
		{
			return (flags >> flag_id) & 1;
		}
		void set_flag(unsigned char flag_id, bool state)
		{
			flags &= ~(1 << flag_id);
			flags |= state << flag_id;
		}

		virtual void write(std::ostream& f)
		{
			if (!subtype)
			{
				unsigned short n = num_sections();
				f.write((char*) &n, 2);
			}

			unsigned char texture = i_texture + 1;

			f.write((char*) &flags,   1);
			f.write((char*) &texture, 1);
			f.write((char*) &height,  1);
			f.write((char*) &value,   1);

			vertex_attribute::write_indices(f);
		}

		unsigned char  flags;
		unsigned char  divider_type;
		unsigned char  i_texture;
	//	unsigned short value; // Usage depends on the divider type
		unsigned char  height;
		unsigned char  value;
	};

// 0x9
	class tunnel : public attribute
	{
	public:
		tunnel() : attribute(ATB_TUNNEL) {
			height2 = 0;
		}
		virtual ~tunnel() {}
		virtual attribute* clone(void) const {
			return new tunnel(*this);
		}

		bool get_flag(unsigned char flag_id)
		{
			return (flags >> flag_id) & 1;
		}
		void set_flag(unsigned char flag_id, bool state)
		{
			flags &= ~(1 << flag_id);
			flags |= state << flag_id;
		}

		virtual void write(std::ostream& f)
		{
			f.write((char*) &flags, 2);
		//	f.put(0x00);
			f.write((char*) &unknown1, 1);
			f.write((char*) &height1, 1);

			if (subtype > 2)
			{
			//	f.put(0x00);
				f.write((char*) &unknown2, 1);
				f.write((char*) &height2, 1);
			}
		}

		unsigned short flags;
		unsigned char height1;
		unsigned char height2;
		unsigned char unknown1;
		unsigned char unknown2;
	};

// 0x9
	class junction : public tunnel
	{
		friend class psdl; // Public access to the private vector

	public:
		virtual attribute* clone(void) const {
			return new junction(*this);
		}

		void add_wall(bool enable = true)
		{
			_enabled_walls.push_back(enable);
		}
		bool get_wall(unsigned long pos)
		{
			return (_enabled_walls[pos / 8] >> (pos % 8)) & 1;
		}
		void set_wall(unsigned long pos, bool state)
		{
			_enabled_walls[pos / 8] &= ~(1 << (pos % 8));
			_enabled_walls[pos / 8] |= state << (pos % 8);
		}
		unsigned short num_walls(void)
		{
			return _enabled_walls.size() * 8;
		}

		virtual void write(std::ostream& f)
		{
			if (_enabled_walls.size() < 12) _enabled_walls.resize(12); // REMOVE!!!!

			unsigned short n_length = _enabled_walls.size() / 2 + 4;

			f.write((char*) &n_length, 2);
			f.write((char*) &flags,    2);
		//	f.put(0x00);
			f.write((char*) &unknown1, 1);
			f.write((char*) &height1,  1);
			f.write((char*) &unknown2, 1);
		//	f.put(0x00);
			f.write((char*) &height2,  1);
			f.write((char*) &unknown3, 2);

			f.write((char*) &_enabled_walls[0], _enabled_walls.size());
		}

		unsigned char  unknown1;
		unsigned char  unknown2;
		unsigned short unknown3;

	private:
		std::vector<unsigned char> _enabled_walls;
	};

// 0xa
	class texture : public attribute
	{
	public:
		texture() : attribute(ATB_TEXTURE) {}
		virtual attribute* clone(void) const {
			return new texture(*this);
		}

		virtual void write(std::ostream& f)
		{
			unsigned short tex_ref = i_texture + 1 - (256 * subtype);

			f.write((char*) &tex_ref, 2);
		}

		unsigned short i_texture;
	};

// 0xb
	class facade : public facade_base
	{
	public:
		facade(std::vector<vertex>* vertex_list) : facade_base(vertex_list, ATB_FACADE) {}
		virtual attribute* clone(void) const {
			return new facade(*this);
		}

		virtual void write(std::ostream& f)
		{
			f.write((char*) &bottom,   2);
			f.write((char*) &top,      2);
			f.write((char*) &u_repeat, 2);
			f.write((char*) &v_repeat, 2);

			vertex_attribute::write_indices(f);
		}

		unsigned short bottom;   // Index in the height list
		unsigned short u_repeat; // Number of times to repeat the texture along its u-axis
		unsigned short v_repeat; // Number of times to repeat the texture along its v-axis
	};

// 0xc
	class roof_triangle_fan : public vertex_container
	{
	public:
		roof_triangle_fan(std::vector<vertex>* vertex_list) : vertex_container(vertex_list, ATB_ROOFTRIANGLEFAN) {}
		virtual attribute* clone(void) const {
			return new roof_triangle_fan(*this);
		}

		virtual void write(std::ostream& f)
		{
			if (!subtype)
			{
				unsigned short n = num_vertices() - 1; // !!!
				f.write((char*) &n, 2);
			}
			f.write((char*) &i_height, 2);
			vertex_attribute::write_indices(f);
		}

		unsigned short i_height;
	};


// -- PSDL Block -------------------------------------------------------------

	class block
	{
		public:

			block(unsigned char type = BIT_PLAIN, unsigned char proprule = 0)
				: enabled(true), locked(false), type(type), proprule(proprule), attributesize(0) {}

			block(const block& b)
			{
				type           = b.type;
				proprule       = b.proprule;
				attributesize  = b.attributesize;
				_perimeter     = b._perimeter;

				_attributes.reserve(b._attributes.size());

				for (unsigned long i = 0; i < b._attributes.size(); i++)

					_attributes.push_back(b._attributes[i]->clone());

				ATLTRACE("\npsdl::block copy constructor called");
			}

			~block()
			{
				std::vector<attribute*>::iterator it = _attributes.begin();
				while (it != _attributes.end())
				{
					delete *it;
					++it;
				}
			}

			attribute* get_attribute(unsigned long i_pos);

			void add_perimeter_point(perimeter_pt pp)
			{
				_perimeter.push_back(pp);
			}

			void add_perimeter_point(unsigned short i_vertex, unsigned short i_block = 0)
			{
				perimeter_pt pp = { i_vertex, i_block };
				add_perimeter_point(pp);
			}
/*
			void add_perimeter_point(std::vector<vertex>::iterator v, std::vector<block*>::iterator b = NULL)
			{
				perimeter_pt pp = { v, b };
				add_perimeter_point(pp);
			}
*/
			void addPerimeterRange(unsigned short nVertexRef, long nOffset)
			{
/*				perimeter_pt pp = { 0, 0 };

				for (unsigned short i = nVertexRef; i --> nVertexRef + nOffset;)
				{
					pp.vertex = i;
					m_perimeter.push_back(pp);
				}*/
			}

			perimeter_pt* get_perimeter_point(unsigned long i_pos)
			{
				return &_perimeter[i_pos];
			}

			void setPerimeterPoint(size_t nIndex, unsigned short nVertexRef, long nBlockID)
			{
/*				m_perimeter[nIndex].vertex = nVertexRef;

				if (nBlockID > 0)
					m_perimeter[nIndex].block = nBlockID;*/
			}

			void emptyPerimeter(void)
			{
				_perimeter.clear();
			}

			void add_attribute(attribute* atb)
			{
				_attributes.push_back(atb);
			}

			void setAttributeSize(long nSize)
			{
				attributesize = nSize;
			}

			void addAttributeSize(long nSize)
			{
				attributesize += nSize;
			}

			unsigned short getAttributeSize(void)
			{
				return attributesize;
			}

			bool generate_perimeter(void)
			{
/*				std::vector<edge> container;

				std::vector<attribute*>::iterator i;
				std::vector<edge>::iterator j, k;

				bool generated = false;

				for (i = _attributes.begin(); i != _attributes.end(); ++i)
				{
					generated |= (*i)->get_edges(container);
				}

				if (!generated) return false;

				for (j = container.begin(); j != container.end(); )
				{
					bool closed = false;

					for (k = container.begin(); k != container.end(); )
					{
						if ( k != j &&
							( (((*k).first == (*j).first) && ((*k).second == (*j).second)) ||
							  (((*k).first == (*j).second) && ((*k).second == (*j).first)) ))
						{
							closed = true;
							container.erase(k);

							if (k < j)
								--j;
						}
						else
							++k;
					}

					if (closed)
						container.erase(j);
					else
						++j;
				}

				std::vector<vertex>::iterator tmp;

				j = container.begin();
				k = container.begin();

				vertex_ref pivot = (*j).first;
				add_perimeter_point(pivot);

				while (!container.empty())
				{
					for (k = container.begin(); k != container.end(); )
					{
						if ((*j).second == (*k).first)
						{
						}
						else
						if ((*j).second == (*k).second)
						{
							tmp = (*k).first;
							(*k).first = (*k).second;
							(*k).second = tmp;
						}
						else
						{
							++k;
							continue;
						}
						break;
					}

					container.erase(j);

					if (k != container.end())
					{
						j = k;
						add_perimeter_point((*k).first);
					}
				}

				return generated;*/
				return 0;
			}

			unsigned long num_attributes(void) { return _attributes.size(); }
			unsigned long num_perimeters(void) { return _perimeter.size();  }

			std::vector<perimeter_pt> _perimeter;
			std::vector<attribute*>   _attributes;

			unsigned short attributesize;
			unsigned char  type, proprule;

			bool enabled, locked;// Flags for UI
	};

	class blockpath
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

	size_t add_block(block* block)
	{
		size_t index = _blocks.size();
		_blocks.push_back(block);

		m_aBlockRefs.push_back(index);
		index = m_aBlockRefs.size() - 1;

		return index;
	}

	void insert_block(block* block, int i_pos)
	{
		if (i_pos < 0) i_pos = num_blocks();

		_blocks.insert(_blocks.begin() + i_pos, block);

/*		for (size_t i = 0; i < num_blocks(); i++)
		{
			for (size_t j = 0; j < _blocks[i]->num_perimeters(); j++)
			{
				if (_blocks[i]->get_perimeter_point(j)->block >= i_pos)
					_blocks[i]->get_perimeter_point(j)->block++;
			}
		}*/
	}

	unsigned long add_vertex(vertex vert)
	{
		size_t index = _vertices.size();
		_vertices.push_back(vert);

		m_aVertexRefs.push_back(index);
		index = m_aVertexRefs.size() - 1;

		return index;
	}

	block* get_block(unsigned long i_pos)
	{
		if (i_pos < _blocks.size())
			return _blocks[i_pos];
		return NULL;
	}

	blockpath* get_blockpath(unsigned long i_pos)
	{
		return &_blockpaths[i_pos];
	}

	long get_block_index(block* block)
	{
		for (long i = 0; i < _blocks.size(); i++) {
			if (_blocks[i] == block)
				return i;
		}
		return -1;
	}

	std::string get_texname(unsigned long i_pos)
	{
		if (i_pos < num_textures())
			return _textures[i_pos];
		return NULL;
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

	error::code read_file (const char* filename, notify_func callback = default_callback);
	error::code write_file(const char* filename);
	error::code read_sdl  (const char* filename);
	error::code write_sdl (const char* filename);


// -- Inline Functions -------------------------------------------------------

	void add_texname(const char* name)        { _textures.push_back(name); }
	void add_texname(std::string const& name) { _textures.push_back(name); }

	void add_blockpath(blockpath* path) { _blockpaths.push_back(*path); }

	unsigned long add_height(float height)
	{
		_heights.push_back(height);
		return _heights.size();
	}

//	vertex* get_vertex(unsigned long i_pos) { return &_vertices[i_pos]; }
	std::vector<vertex>::iterator get_vertex(unsigned long pos) { return _vertices.begin() + pos; }
	float get_height(unsigned long i_pos)   { return _heights[i_pos];   }

	unsigned long num_vertices(void)   { return _vertices.size();   }
	unsigned long num_heights(void)    { return _heights.size();    }
	unsigned long num_textures(void)   { return _textures.size();   }
	unsigned long num_blocks(void)     { return _blocks.size();     }
	unsigned long num_blockpaths(void) { return _blockpaths.size(); }

	psdl() : f_radius(0), _unknown0(0), _unknown1(0x00), _unknown2(0xcd) {}
	~psdl()
	{
		std::vector<block*>::iterator it = _blocks.begin();
		while (it != _blocks.end())
		{
			delete *it;
			++it;
		}
	}

	vertex v_min;
	vertex v_max;
	vertex v_center;
	float  f_radius;

	std::vector<std::string>    _textures;

private:

	std::vector<unsigned long> m_aVertexRefs;
	std::vector<unsigned long> m_aBlockRefs;

	std::vector<vertex>        _vertices;
	std::vector<float>         _heights;
	std::vector<block*>        _blocks;
	std::vector<blockpath>     _blockpaths;
	std::vector<unsigned char> _junk; // junk found at the end of the file

	unsigned long _unknown0; // number of junctions?
	unsigned char _unknown1; // 0x00
	unsigned char _unknown2; // 0xcd
	char identifier[4];
};

#endif
