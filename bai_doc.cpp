#include "stdafx.h"
#include "bai_doc.h"
#include "config.h"
#include "files.h" // BAISettings

using namespace std;

BAIDocTemplate* BAIDocTemplate::GenerateBAIThread::pDocTmpl;
psdl* BAIDocTemplate::GenerateBAIThread::pPSDL;

inline bool PointInCircle(const Vertex* pOriginVertex, const float& fRadius, const Vertex* pTestVertex)
{
	float fDX = pTestVertex->x - pOriginVertex->x;
	float fDY = pTestVertex->z - pOriginVertex->z;

	return (fDX * fDX + fDY * fDY) < (fRadius * fRadius);
}

inline bool PointInCircle2(const Vertex* pOriginVertex, const float& fRadius2, const Vertex* pTestVertex)
{
	float fDX = pTestVertex->x - pOriginVertex->x;
	float fDY = pTestVertex->z - pOriginVertex->z;

	return (fDX * fDX + fDY * fDY) < fRadius2;
}

psdl::road_strip* FindRoadAttribute(psdl::block* pBlock)
{
	for (unsigned long j = 0; j < pBlock->num_attributes(); ++j)
	{
		unsigned char type = pBlock->get_attribute(j)->type;

		if (type == ATB_RECTANGLE || type == ATB_ROAD || type == ATB_DIVIDEDROAD)
		{
			return static_cast<psdl::road_strip*>(pBlock->get_attribute(j));
		}
	}
	return NULL;
}

typedef vector< pair< unsigned long, long > > JunctionRoadList;

bool BAIDocTemplate::GenerateAIRoutes(psdl* pPSDL, ProgressMonitor* pMonitor)
{
	ATLTRACE("\nGenerating AI routes...\n");
	pMonitor->setNote("Generating routes");

	BAISettings conf(config.dialogs.generateBAI.strConfigFile.c_str());
	double fTol = config.dialogs.generateBAI.fTolerance;

	unsigned long nBlocks = pPSDL->num_blocks();
	unsigned long i, t;
	unsigned short j, l;

	bool* usedBlocks = new bool[nBlocks];
	fill_n(&usedBlocks[0], nBlocks, false);

	vector<JunctionRoadList> blocks;

	m_pDoc->junctions.clear();

	// Find intersections
	for (i = 0; i < nBlocks; ++i)
	{
		if (usedBlocks[i] == true) continue;

		psdl::block* block = pPSDL->get_block(i);

		if (!(block->type & BIT_INTERSECTION)) continue;

		usedBlocks[i] = true;

		bai::Intersection junc;
		junc.center = block->get_center();
		junc.block = i + 1;
		m_pDoc->junctions.push_back(junc);

		JunctionRoadList roadList;

		for (t = 0; t < block->num_perimeters(); ++t)
		{
			psdl::perimeter_pt* pt = block->get_perimeter_point(t);

			if (!pt || pt->block == 0) continue;
			if (!(pPSDL->get_block(pt->block - 1)->type & BIT_ROAD)) continue;

			JunctionRoadList::iterator it = roadList.begin();

			bool omslachtiggedoe = true;

			for (it; it != roadList.end(); ++it)
			{
				if ((*it).first == pt->block - 1) omslachtiggedoe = false;
			}

			if (omslachtiggedoe)
			{
				pair< unsigned long, long > pr;
				pr.first  = pt->block - 1;
				pr.second = -1;
				roadList.push_back(pr);
			}
		}

		blocks.push_back(roadList);
	}

	if (!config.dialogs.generateBAI.bRoutes)
		return true;

	pMonitor->setMaximum(nBlocks - m_pDoc->junctions.size());

	m_pDoc->roads.clear();

	// Find roads
	for (i = 0; i < nBlocks; ++i)
	{
		pMonitor->setProgress(i);

		if (usedBlocks[i] == true) continue;

		psdl::block* block = pPSDL->get_block(i);

		if (!(block->type & BIT_ROAD)) continue;

		bool enablePeds = true, enableLights = true;

	//	psdl::road_strip* atb = FindRoadAttribute(block);
	//	if (!atb) continue;
		psdl::road_strip* atb = NULL;
		for (t = 0; t < block->num_attributes(); ++t)
		{
			unsigned char type = block->get_attribute(t)->type;

			if (type == ATB_RECTANGLE || type == ATB_ROAD || type == ATB_DIVIDEDROAD)
			{
				atb = static_cast<psdl::road_strip*>(block->get_attribute(t));
				break;
			}
			if (type == ATB_TUNNEL)
			{
				psdl::tunnel* tunnel = static_cast<psdl::tunnel*>(block->get_attribute(t));
				if (tunnel->get_flag(BIT_STYLE) == true) // Walls
				{
					enablePeds = false;
				}
			}
		}
		if (!atb) continue;

		if (block->type & BIT_SUBTERANEAN)
		{
			enableLights = false;
			enablePeds   = false;
		}

		usedBlocks[i] = true;

		bai::Road road;
		road.blocks.push_back(i + 1);

		// Defaults
		road.data[0].nLanes = 1;
		road.data[1].nLanes = 1;

		for (t = t - 1; t >= 0; --t)
		{
			unsigned char type = block->get_attribute(t)->type;

			if (type == ATB_TEXTURE)
			{
				string textureName = pPSDL->get_texname(static_cast<psdl::texture*>(block->get_attribute(t))->i_texture);

				if (textureName.length() > 1)
				{
					unsigned short nLanes = 0;

					if (conf.textures.count(textureName) != 0)
					{
						nLanes = conf.textures[textureName].nLanes;
					}

					if (conf.roads.count(i) != 0)
					{
						road.data[0].nLanes = conf.roads[i].nRightLanes;

						if (nLanes >= road.data[0].nLanes)
						{
							road.data[1].nLanes = nLanes - road.data[0].nLanes;
						}
					}
					else
					{
						road.data[0].nLanes = nLanes / 2;
						road.data[1].nLanes = nLanes - road.data[0].nLanes;
					}
				}
				break;
			}
		}

		vector<Vertex*> verts = atb->get_vertices();
		vector<Vertex*> mesh(verts.begin(), verts.end()); // Joined road mesh

		unsigned char p = atb->num_vertices() / atb->num_sections();

		unsigned long id = i;

		bool hasEndJunction = false, hasStartJunction = false;

		psdl::block    *block1 = NULL;
		psdl::perimeter_pt *pt = NULL;

		while (true)
		{
			bool brol = false;

		//	if (FindConnectedJunction(pPSDL, block, mesh[1], m_pDoc, road, blocks))
		//	{
		//		isDummyRoad = false;
		//		break;
		//	}

			for (t = 0; t < nBlocks; ++t)
			{
				block1 = pPSDL->get_block(t);
				if (!(block1->type & BIT_INTERSECTION && block1->find_perimeter_point(mesh[1], pt))) continue;

				for (j = 0; j < m_pDoc->junctions.size(); ++j)
				{
					if (m_pDoc->junctions[j].block == t + 1)
					{
						bool omslachtiggedoe = false;
						for (JunctionRoadList::iterator it = blocks[j].begin(); it != blocks[j].end(); ++it)
						{
							if ((*it).first == id)
							{
								(*it).second = m_pDoc->roads.size();
								road.end[1].intersectionID = j;
								omslachtiggedoe = true;
								hasStartJunction = true;
								break;
							}
						}
						if (omslachtiggedoe) break;
					}
				}
				brol = true;
				break;
			}
			if (brol) break;

			for (t = 0; t < nBlocks; ++t)
			{
				if (t == id) continue;

				block1 = pPSDL->get_block(t);

				if (block1->type & BIT_ROAD)
				{
					if (usedBlocks[t] == true) continue;

					psdl::road_strip* atb2 = FindRoadAttribute(block1);
					if (!atb2 || atb->type != atb2->type) continue;

					vector<Vertex*> verts2 = atb2->get_vertices();

					if (mesh[1]->Match(*verts2[p - 2], fTol))
					{
						vector<Vertex*> temp(verts2.begin() + p, verts2.end());
						reverse(temp.begin(), temp.end());

						mesh.insert(mesh.begin(), temp.begin(), temp.end());
					}
					else if (mesh[1]->Match(*verts2.end()[1 - p], fTol))
					{
						mesh.insert(mesh.begin(), verts2.begin(), verts2.end() - p);
					}
					else continue;

					block = block1;

					id = t;
					usedBlocks[id] = true;
					road.blocks.push_back(id + 1);
					brol = true;
					break;
				}
			}
			if (!brol) break;
		}

		block = pPSDL->get_block(i);
		id = i;

		while (true)
		{
			bool brol = false;

			for (t = 0; t < nBlocks; ++t)
			{
				block1 = pPSDL->get_block(t);

				if (!(block1->type & BIT_INTERSECTION && block1->find_perimeter_point(mesh.end()[-2], pt))) continue;

				for (j = 0; j < m_pDoc->junctions.size(); ++j)
				{
					if (m_pDoc->junctions[j].block == t + 1)
					{
						bool omslachtiggedoe = false;
						for (JunctionRoadList::iterator it = blocks[j].begin(); it != blocks[j].end(); ++it)
						{
							if ((*it).first == id)
							{
								(*it).second = m_pDoc->roads.size();
								road.end[0].intersectionID = j;
								omslachtiggedoe = true;
								hasEndJunction = true;
								break;
							}
						}
						if (omslachtiggedoe) break;
					}
				}
				brol = true;
				break;
			}
			if (brol) break;

			for (t = 0; t < nBlocks; ++t)
			{
				if (t == id) continue;

				block1 = pPSDL->get_block(t);

				if (block1->type & BIT_ROAD)
				{
					if (usedBlocks[t] == true) continue;

					psdl::road_strip* atb2 = FindRoadAttribute(block1);
					if (!atb2 || atb->type != atb2->type) continue;

					vector<Vertex*> verts2 = atb2->get_vertices();

					if (mesh.end()[-2]->Match(*verts2[p - 2], fTol))
					{
						mesh.insert(mesh.end(), verts2.begin() + p, verts2.end());
					}
					else if (mesh.end()[-2]->Match(*verts2.end()[1 - p], fTol))
					{
						vector<Vertex*> temp(verts2.begin(), verts2.end() - p);
						reverse(temp.begin(), temp.end());

						mesh.insert(mesh.end(), temp.begin(), temp.end());
					}
					else continue;

					block = block1;

					id = t;
					usedBlocks[id] = true;
					road.blocks.push_back(id + 1);
					brol = true;
					break;
				}
			}
			if (!brol) break;
		}

		if (!(hasEndJunction && hasStartJunction))
		{
			ATLTRACE("Block %x has %s end junction and %s start junction\n", i, hasEndJunction ? "an" : "no", hasStartJunction ? "a" : "no");
			continue; // Do not include this road
		}

		unsigned char a = (p == 2) ? 0 : 1;
		unsigned char b = (p == 2) ? 1 : p - 2;
		unsigned char c = (p == 2) ? 1 : p - 1;
		unsigned char d = (p == 6) ? 2 : a;
		unsigned char e = (p == 6) ? 3 : b;

		unsigned char f = 0;

		if (conf.roads.count(i) != 0)
		{
			road.unknown0 = conf.roads[i].unknown0;
			road.end[0].vehicleRule = conf.roads[i].vehicleRule1;
			road.end[1].vehicleRule = conf.roads[i].vehicleRule2;
		}/*
		else if (enableLights)
		{
			road.end[0].vehicleRule = 1;
			road.end[1].vehicleRule = 1;
		}
		else
		{
			road.end[0].vehicleRule = 3;
			road.end[1].vehicleRule = 3;
		}*/
		else
		{
			road.end[0].vehicleRule = 1;
			road.end[1].vehicleRule = 1;
		}

		if (road.end[0].vehicleRule == 3 || road.end[1].vehicleRule == 3)
		{
			road.data[0].unknown4 = 2;
			road.data[1].unknown4 = 2;
		}
		else if (road.data[0].nLanes + road.data[1].nLanes == 0)
		{
			road.data[0].unknown4 = 3;
			road.data[1].unknown4 = 3;
			road.data[1].nLanes = 1;

			road.end[0].vehicleRule = 3;
			road.end[1].vehicleRule = 3;
		}

		road.data[0].lLanesVertices.resize(road.data[0].nLanes);
		road.data[0].lanesDistances.resize(road.data[0].nLanes);
		road.data[1].lLanesVertices.resize(road.data[1].nLanes);
		road.data[1].lanesDistances.resize(road.data[1].nLanes);

		road.data[0].unknown.assign(3 * (road.data[0].nLanes + 1), 0);
		road.data[1].unknown.assign(3 * (road.data[1].nLanes + 1), 0);
		road.data[0].unknown.resize(11 + road.data[0].nLanes, -4.3160208e+008);
		road.data[1].unknown.resize(11 + road.data[1].nLanes, -4.3160208e+008);

		// Simplify the mesh
		if (mesh.size() > p * 2)
		{
			const float fMinDistance = 8.0f; // Min. distance between cross sections

			float fDistance;
			unsigned char k;

			for (vector<Vertex*>::iterator it = mesh.begin() + p; it != mesh.end() - p; )
			{
				float fLongest1 = 0;
				float fLongest2 = 0;
				bool bStraight = false;

				for (k = 0; k < p; ++k)
				{
					Vector v1 = **(it + k) - **(it + k - p);
					Vector v2 = **(it + k + p) - **(it + k);
					v1.Normalize();
					v2.Normalize();

					float res = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
					if (res == 1) // Zero degrees
					{
						bStraight = true;
						break;
					}
				//	ATLTRACE("%f\n", res);

					fDistance = Vertex::Distance( **(it + k), **(it + k - p) );
					if (fDistance > fLongest1) fLongest1 = fDistance;

					fDistance = Vertex::Distance( **(it + k + p), **(it + k) );
					if (fDistance > fLongest2) fLongest2 = fDistance;
				}

				if (bStraight || (fLongest1 + fLongest2) < (fMinDistance * 2))
					it = mesh.erase(it, it + p);
				else
					advance(it, p);
			}
		}

		// Generate additional cross sections (hard turns)
		if (mesh.size() > p * 2)
		{
			const float fChamferLength = 5.0f;

			vector<Vertex*> temp(mesh.begin(), mesh.begin() + p);

			for (unsigned long j = p; j < mesh.size() - p; j += p)
			{
				Vector vec;
				unsigned long k;

				bool tooShort = false;
				vector<Vertex*> chamf;

				for (k = 0; k < p; ++k)
				{
					vec = *mesh[j + k] - *mesh[j + k - p];

					float fLength1 = vec.Length();
					float fLength2 = fLength1 - fChamferLength;

					if (fLength2 <= fChamferLength)
					{
						tooShort = true;
						break;
					}

					vec.x *= fLength2 / fLength1;
					vec.y *= fLength2 / fLength1;
					vec.z *= fLength2 / fLength1;

					vec = vec + *mesh[j + k - p];

					chamf.push_back(new Vertex(vec));
				}

				if (!tooShort)
				{
					for (k = 0; k < p; ++k)
					{
						vec = *mesh[j + k] - *mesh[j + k + p];

						float fLength1 = vec.Length();
						float fLength2 = fLength1 - fChamferLength;

						if (fLength2 <= fChamferLength)
						{
							tooShort = true;
							break;
						}

						vec.x *= fLength2 / fLength1;
						vec.y *= fLength2 / fLength1;
						vec.z *= fLength2 / fLength1;

						vec = vec + *mesh[j + k + p];

						chamf.push_back(new Vertex(vec));
					}
				}

				if (tooShort)
				{
					temp.insert(temp.end(), mesh.begin() + j, mesh.begin() + j + p);
				}
				else
				{
					temp.insert(temp.end(), chamf.begin(), chamf.end());
				}
			}

			temp.insert(temp.end(), mesh.end() - p, mesh.end());
			mesh.assign(temp.begin(), temp.end());
		}

		Vertex v1, v2, v3, v4, v5;
		float d1 = 0, d2 = 0, d3 = 0;
		float d4;

		road.unknown1 = Vector(( *mesh[f] - *mesh[c] ) / 2).Length(); // Width of the road divided by 2

		for (unsigned long j = 0, k = 0; j < mesh.size(); j += p, ++k)
		{
			v1 = (*mesh[j + f] + *mesh[j + a]) / 2;
		//	v1 = (*mesh[j + 0] + *mesh[j + a]) / 2;
			v2 = (*mesh[j + b] + *mesh[j + c]) / 2;
			v3 = (*mesh[j + d] + *mesh[j + e]) / 2;

			if (j >= p) d1 += Vector( *mesh[j - p + f] - *mesh[j + f]).Length();
		//	if (j >= p) d1 += Vector( *mesh[j - p + 0] - *mesh[j + 0]).Length();
			if (j >= p) d2 += Vector( *mesh[j - p + c] - *mesh[j + c]).Length();
			if (j >= p) d3 += Vector((*mesh[j - p + d] + *mesh[j - p + e]) / 2 - v3).Length();

			road.origin.push_back(v3);
			road.distance.push_back(d3);

			road.data[0].distance.push_back(d1);
			road.data[1].distance.push_back(d2);

			road.data[0].sidewalkCenter.push_back(v1);
			road.data[1].sidewalkCenter.push_back(v2);

			road.data[0].sidewalkOuter.push_back(*mesh[j + f]);
		//	road.data[0].sidewalkOuter.push_back(*mesh[j + 0]);
			road.data[0].sidewalkInner.push_back(*mesh[j + a]);
			road.data[1].sidewalkInner.push_back(*mesh[j + b]);
			road.data[1].sidewalkOuter.push_back(*mesh[j + c]);

			unsigned short nLanes = road.data[0].nLanes + road.data[1].nLanes;
			unsigned short nMaxLanes = nLanes > 1 ? nLanes / 2 : 1;

			v1 = road.data[0].sidewalkInner[k];
			v2 = (p == 6) ? *mesh[j + d] : road.origin[k]; // Divroad uses the divider
			v3 = (p == 6) ? *mesh[j + e] : road.origin[k]; // Divroad uses the divider
			v4 = road.data[1].sidewalkInner[k];

			if (p != 6 && nLanes % 2 != 0) // Odd number of lanes
			{
				nMaxLanes = nLanes;

				v2 = v4;
				v3 = v1;
			}

			unsigned short nLanesR1 = min(nMaxLanes, road.data[0].nLanes);
			unsigned short nLanesR2 = road.data[0].nLanes - nLanesR1;
			unsigned short nLanesL1 = min(nMaxLanes, road.data[1].nLanes);
			unsigned short nLanesL2 = road.data[1].nLanes - nLanesL1;

			for (l = 0; l < nLanesR2; ++l)
			{
				v5 = v4 + (v3 - v4) / nMaxLanes * (l + nLanesL1 + 0.5);
				road.data[0].lLanesVertices[l].insert(road.data[0].lLanesVertices[l].begin(), v5);
			}
			for (l; l < road.data[0].nLanes; ++l)
			{
				v5 = v2 + (v1 - v2) / nMaxLanes * (l - nLanesR2 + nLanesL2 + 0.5);
				road.data[0].lLanesVertices[l].insert(road.data[0].lLanesVertices[l].begin(), v5);
			}
			for (l = 0; l < nLanesL2; ++l)
			{
				v5 = v1 + (v2 - v1) / nMaxLanes * (l + nLanesR1 + 0.5);
				road.data[1].lLanesVertices[l].push_back(v5);
			}
			for (l; l < road.data[1].nLanes; ++l)
			{
				v5 = v3 + (v4 - v3) / nMaxLanes * (l - nLanesL2 + nLanesR2 + 0.5);
				road.data[1].lLanesVertices[l].push_back(v5);
			}
		}

		Vector vec1, vec2, vec3;
		for (k = 0; k < road.origin.size(); ++k)
		{
			// Right lanes distances
			for (l = 0; l < road.data[0].nLanes; ++l)
			{
				d4 = (k > 0) ? road.data[0].lanesDistances[l][k-1] + Vector(road.data[0].lLanesVertices[l][k-1] - road.data[0].lLanesVertices[l][k]).Length() : 0;
				road.data[0].lanesDistances[l].push_back(d4);
			}

			// Left lanes distances
			for (l = 0; l < road.data[1].nLanes; ++l)
			{
				d4 = (k > 0) ? road.data[1].lanesDistances[l][k-1] + Vector(road.data[1].lLanesVertices[l][k-1] - road.data[1].lLanesVertices[l][k]).Length() : 0;
				road.data[1].lanesDistances[l].push_back(d4);
			}

			// Orientation matrix
			vec2 = (k > 0) ? (road.origin[k - 1] - road.origin[k]) : (road.origin[k] - road.origin[k + 1]);

			vec3 = (k > 0 && k < road.origin.size() - 1) ? (road.origin[k + 1] - road.origin[k - 1]) : -vec2;

			vec1.x = -vec2.z;
		//	vec1.y =  vec2.y;
			vec1.y =  road.origin[k].y - road.data[0].sidewalkInner[k].y;
			vec1.z =  vec2.x;

			vec1.Normalize();
			vec2.Normalize();
			vec3.Normalize();

			road.xOrientation.push_back( vec1);
			road.yOrientation.push_back( vec1 * vec2); // Cross product
			road.zOrientation.push_back( vec2);
			road.wOrientation.push_back( vec3);
		}

		k = mesh.size() / p - 1;

		// Traffic lights placed on left side of the road
	//	road.end[0].trafficLightOrigin = road.data[0].sidewalkInner[k];
	//	road.end[1].trafficLightOrigin = road.data[1].sidewalkInner[0];
	//	road.end[0].trafficLightAxis   = road.data[1].sidewalkInner[k];
	//	road.end[1].trafficLightAxis   = road.data[0].sidewalkInner[0];

	//	road.end[0].trafficLightOrigin = road.data[0].sidewalkInner[k];
	//	road.end[1].trafficLightOrigin = road.data[1].sidewalkInner[0];
	//	road.end[0].trafficLightAxis   = road.data[0].sidewalkOuter[k];
	//	road.end[1].trafficLightAxis   = road.data[1].sidewalkOuter[0];

		// Traffic lights placed on right side of the road
		if (road.end[0].vehicleRule == 1)
		{
			road.end[0].trafficLightOrigin = (road.data[1].sidewalkInner[k] + road.data[1].sidewalkOuter[k]) / 2;
			road.end[0].trafficLightAxis = road.data[1].sidewalkOuter[k];
		}
		if (road.end[1].vehicleRule == 1)
		{
			road.end[1].trafficLightOrigin = (road.data[0].sidewalkInner[0] + road.data[0].sidewalkOuter[0]) / 2;
			road.end[1].trafficLightAxis = road.data[0].sidewalkOuter[0];
		}

		m_pDoc->roads.push_back(road);
	}

	for (j = 0; j < blocks.size(); ++j)
	{
	//	int lol = 0;
		vector< pair< unsigned long, long > >::iterator it = blocks[j].begin();

		unsigned char roadIndex = 0;

		for (it; it != blocks[j].end(); ++it)
		{
			if ((*it).second != -1)
			{
				m_pDoc->junctions[j].roads.push_back((*it).second);

				if (m_pDoc->roads[(*it).second].end[0].intersectionID == j)
				{
					m_pDoc->roads[(*it).second].end[0].intersectionRoadIndex = roadIndex;
				}
				else
				{
					m_pDoc->roads[(*it).second].end[1].intersectionRoadIndex = roadIndex;
				}
				++roadIndex;
			}
	//		else
	//		{
	//			++lol;
	//		}
		}
	//	if (lol > 0)
	//		ATLTRACE("%x\n", m_pDoc->junctions[j].block);

	}

	return true;
}

bool BAIDocTemplate::GenerateAICulling(psdl* pPSDL, ProgressMonitor* pMonitor)
{
	pMonitor->setNote("Generating culling groups");

	unsigned long i;
	unsigned short j;

	unsigned long nBlocks = pPSDL->num_blocks();
	unsigned short nRoads = m_pDoc->roads.size();

	m_pDoc->culling[0].resize(nBlocks + 1);
	m_pDoc->culling[1].resize(nBlocks + 1);

	m_pDoc->culling[0][0].clear();
	m_pDoc->culling[1][0].clear();

	// Zentra

	if (config.dialogs.generateBAI.iCullingMethod == 0)
	{
		vector<unsigned short> all(nRoads);

		for (j = 0; j < nRoads; ++j)
		{
			all[j] = j;
		}

		for (i = 1; i <= nBlocks; ++i)
		{
			m_pDoc->culling[0][i].clear();
			m_pDoc->culling[1][i].clear();

			m_pDoc->culling[0][i].assign(all.begin(), all.end());
			m_pDoc->culling[1][i].assign(all.begin(), all.end());
		}

		return true;
	}

	// Radius (proper method)

	const float fRadi[] = {
		config.dialogs.generateBAI.fRadius1 * config.dialogs.generateBAI.fRadius1,
		config.dialogs.generateBAI.fRadius2 * config.dialogs.generateBAI.fRadius2
	};

	psdl::block *block, *block1;
	Vertex *vCenter, *vTest;
	unsigned char b;
	unsigned short k;
	unsigned long l, m;

	pMonitor->setMaximum(nBlocks);

	clock_t start_time = clock();

	for (i = 0; i < nBlocks; ++i)
	{
		block = pPSDL->get_block(i);

		for (b = 0; b < 2; ++b)
		{
			m_pDoc->culling[b][i+1].clear();

			for (j = 0; j < nRoads; ++j)
			{
				for (k = 0; k < m_pDoc->roads[j].blocks.size(); ++k)
				{
					if (i == m_pDoc->roads[j].blocks[k] - 1) // No need to test vertices
					{
						m_pDoc->culling[b][i+1].push_back(j);
						goto next;
					}
				}

				for (l = 0; l < block->num_perimeters(); ++l)
				{
					vCenter = pPSDL->get_vertex(block->get_perimeter_point(l)->vertex);

					for (k = 0; k < m_pDoc->roads[j].blocks.size(); ++k)
					{
						block1 = pPSDL->get_block(m_pDoc->roads[j].blocks[k] - 1);
						for (m = 0; m < block1->num_perimeters(); ++m)
						{
							vTest = pPSDL->get_vertex(block1->get_perimeter_point(m)->vertex);

							if (PointInCircle2(vCenter, fRadi[b], vTest))
							{
								m_pDoc->culling[b][i+1].push_back(j);
								goto next;
							}
						}
					}
				}

				next: continue;
			}
		}

		pMonitor->setProgress(i);
	}

	ATLTRACE("Time: %u\n", clock() - start_time);
	return true;
}

bool BAIDocTemplate::GenerateBAI(psdl* pPSDL, ProgressMonitor* pMonitor)
{
	bool ret1 = true, ret2 = true;

	if (config.dialogs.generateBAI.bRoutes)  ret1 = GenerateAIRoutes (pPSDL, pMonitor);
	if (config.dialogs.generateBAI.bCulling) ret2 = GenerateAICulling(pPSDL, pMonitor);

	pMonitor->done();

	return (ret1 & ret2); // Bad
}
