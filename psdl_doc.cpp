#include "stdafx.h"
#include "psdl_doc.h"

#include <algorithm>
#include <fstream>

using namespace std;

typedef struct
{
	unsigned short a, b, c;
	long material;
}
triangle;

// What a mess :-/
PSDLDocTemplate* PSDLDocTemplate::ReadSDLThread::pDocTmpl;
PSDLDocTemplate* PSDLDocTemplate::WriteSDLThread::pDocTmpl;
string PSDLDocTemplate::ReadSDLThread::strFileName;
string PSDLDocTemplate::WriteSDLThread::strFileName;

error::code PSDLDocTemplate::Read3DS(std::string strFileName)
{/*
	ifstream f(strFileName.c_str(), ios::in | ios::binary);
	if (!f.is_open()) return error::cant_open;

	unsigned short n_chunk_id, k, l, n_qty;
	unsigned int   i_chunk_sz;
	unsigned long  m;

	vector<unsigned short> clustering;

	vector<triangle> faces;

	vector<string> materials;
	vector<string> textures;
	vector<string>::iterator it;

	unsigned long vertex_offset, texture_offset;

	string name;
	psdl::vertex v;
	triangle face;

	psdl::block* block;

	while (!f.eof())
	{
		f.read((char*) &n_chunk_id, 2);
		f.read((char*) &i_chunk_sz, 4);

		switch (n_chunk_id)
		{
			case 0x4d4d:
				f.seekg(10, ios_base::cur);
				break;

			case 0x3d3d:
				break;

			case 0x4100:
				// Triangular mesh
				break;

			case 0x4110:
				f.read((char*) &n_qty, 2);

				clustering.resize(n_qty);

				vertex_offset = m_pDoc->num_vertices();
				texture_offset = m_pDoc->num_textures();

				for (k = 0; k < n_qty; k++)
				{
					f.read((char*) &v.x, 4);
					f.read((char*) &v.z, 4);
					f.read((char*) &v.y, 4);

					m_pDoc->add_vertex(v);
				}
				break;

			case 0x4120:
				f.read((char*) &n_qty, 2);

				faces.reserve(faces.size() + n_qty);

				for (k = 0; k < n_qty; k++)
				{
					f.read((char*) &face.a, 2);
					f.read((char*) &face.b, 2);
					f.read((char*) &face.c, 2);

					face.material = -1;

					faces.push_back(face);

					f.seekg(2, ios_base::cur);

					clustering[face.a]++;
					clustering[face.b]++;
					clustering[face.c]++;
				}
				break;

			case 0x4130:
				getline(f, name, '\0');

				it = find(materials.begin(), materials.end(), name);
				if (it != materials.end())
					m = it - materials.begin();
				else
				{
					m = materials.size();
					materials.push_back(name);
				}

				f.read((char*) &n_qty, 2);

				for (k = 0; k < n_qty; k++)
				{
					f.read((char*) &l, 2);
					faces[l].material = m;
				}

				break;

			case 0x4150:
				{
					psdl::triangle_fan* fan;
					psdl::texture* tex;

					vector<unsigned short>::iterator i;
					vector<triangle>::iterator j;

					while ((i = max_element(clustering.begin(), clustering.end())) && *i > 0)
					{
						unsigned short pivot = i - clustering.begin();
						long outer1 = -1, outer2 = -1;
						long material = 0;

						for (j = faces.begin(); j != faces.end(); )
						{
							if (j->a == pivot)
							{
								outer1 = j->b;
								outer2 = j->c;
								faces.erase(j);
								break;
							} else
							if (j->b == pivot)
							{
								outer1 = j->c;
								outer2 = j->a;
								faces.erase(j);
								break;
							} else
							if (j->c == pivot)
							{
								outer1 = j->a;
								outer2 = j->b;
								faces.erase(j);
								break;
							} else
								++j;
						}

						if (outer1 >= 0 && outer2 >= 0)
						{
							tex = new psdl::texture();
							fan = new psdl::triangle_fan();

							fan->_i_vertices.push_back(outer1 + vertex_offset);
							fan->_vertices.push_back(m_pDoc->get_vertex(outer1 + vertex_offset));

							fan->_i_vertices.push_back(outer2 + vertex_offset);
							fan->_vertices.push_back(m_pDoc->get_vertex(outer2 + vertex_offset));

							for (j = faces.begin(); j != faces.end(); )
							{
								if (material != j->material)
								{
									++j;
									continue;
								}

								if (j->a == pivot && j->b == outer2)
									outer2 = j->c; else
								if (j->b == pivot && j->c == outer2)
									outer2 = j->a; else
								if (j->c == pivot && j->a == outer2)
									outer2 = j->b; else
								{
									++j;
									continue;
								}

								faces.erase(j);

								fan->_i_vertices.push_back(outer2 + vertex_offset);
								fan->_vertices.push_back(m_pDoc->get_vertex(outer2 + vertex_offset));
							}

							for (j = faces.begin(); j != faces.end(); )
							{
								if (material != j->material)
								{
									++j;
									continue;
								}

								if (j->a == outer1 && j->b == pivot)
									outer1 = j->c; else
								if (j->b == outer1 && j->c == pivot)
									outer1 = j->a; else
								if (j->c == outer1 && j->a == pivot)
									outer1 = j->b; else
								{
									++j;
									continue;
								}

								faces.erase(j);

								fan->_i_vertices.insert(fan->_i_vertices.begin(), outer1 + vertex_offset);
								fan->_vertices.insert(fan->_vertices.begin(), m_pDoc->get_vertex(outer1 + vertex_offset));
							}

							fan->_i_vertices.insert(fan->_i_vertices.begin(), pivot + vertex_offset);
							fan->_vertices.insert(fan->_vertices.begin(), m_pDoc->get_vertex(pivot + vertex_offset));

							if (fan->num_triangles() < 8)
								fan->subtype = fan->num_triangles();

							tex->i_texture = material + texture_offset;

							block->add_attribute(tex);
							block->add_attribute(fan);
						}

						*i = 0;
					}

					fan->last = true;

					m_pDoc->add_block(block);
				}

				f.seekg(i_chunk_sz - 6, ios_base::cur);
				break;

//			case 0x4150:
//				{
//					vector<triangle>::iterator i, j;
//					psdl::triangle_fan* fan;
//
//					while (!faces.empty())
//					{
//						for (i = faces.begin(); i != faces.end(); )
//						{
//							fan = new psdl::triangle_fan();
//
//							unsigned short pivot = i->a;
//							unsigned short outer = i->b;
//
//							fan->_i_vertices.push_back(i->c + vertex_offset);
//							fan->_vertices.push_back(m_pDoc->get_vertex(i->c + vertex_offset));
//
//							fan->_i_vertices.push_back(i->b + vertex_offset);
//							fan->_vertices.push_back(m_pDoc->get_vertex(i->b + vertex_offset));
//
//							for (j = faces.begin(); j != faces.end(); )
//							{
//								if (i == j || i->material != j->material)
//								{
//									++j;
//									continue;
//								}
//
//								if (j->b == pivot && j->a == outer)
//									outer = j->c; else
//								if (j->c == pivot && j->b == outer)
//									outer = j->a; else
//								if (j->a == pivot && j->c == outer)
//									outer = j->b; else
//								{
//									++j;
//									continue;
//								}
//
//								faces.erase(j);
//
//								fan->_i_vertices.push_back(outer + vertex_offset);
//								fan->_vertices.push_back(m_pDoc->get_vertex(outer + vertex_offset));
//							}
//
//							outer = i->c;
//
//							for (j = faces.begin(); j != faces.end(); )
//							{
//								if (i == j || i->material != j->material)
//								{
//									++j;
//									continue;
//								}
//
//								if (j->b == pivot && j->a == outer)
//									outer = j->c; else
//								if (j->c == pivot && j->b == outer)
//									outer = j->a; else
//								if (j->a == pivot && j->c == outer)
//									outer = j->b; else
//								{
//									++j;
//									continue;
//								}
//
//								faces.erase(j);
//
//								fan->_i_vertices.insert(fan->_i_vertices.begin(), outer + vertex_offset);
//								fan->_vertices.insert(fan->_vertices.begin(), m_pDoc->get_vertex(outer + vertex_offset));
//							}
//
//							fan->_i_vertices.insert(fan->_i_vertices.begin(), i->a + vertex_offset);
//							fan->_vertices.insert(fan->_vertices.begin(), m_pDoc->get_vertex(i->a + vertex_offset));
//
//							if (fan->num_triangles() < 8)
//								fan->subtype = fan->num_triangles();
//
//							block->add_attribute(fan);
//							faces.erase(i);
//						}
//					}
//
//					fan->last = true;
//
//					m_pDoc->add_block(block);
//				}
//
//				f.seekg(i_chunk_sz - 6, ios_base::cur);
//				break;

			case 0xafff: // Materials
				break;

		//	case 0xa000: // Material name
		//		break;

			case 0xa200: // Material texture mapping
				break;

			case 0xa300: // Material texture
				getline(f, name, '\0');
				ATLTRACE("%s\n", name.c_str());
				{
					size_t ext_pos = name.find('.');
					m_pDoc->add_texname(name.substr(0, ext_pos));
				}
				break;

			case 0x4000:
				getline(f, name, '\0');

				if (name.find("GROUND", 0) == 0)
					block = new psdl::block();
				else
					f.seekg(i_chunk_sz - 6 - name.size() - 1, ios_base::cur);

				break;

			default:
				f.seekg(i_chunk_sz - 6, ios_base::cur);
		}
	}
*/
	return error::ok;
}
