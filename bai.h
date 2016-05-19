#ifndef __BAI_H__
#define __BAI_H__

#include "io_error.h"
#include "3dtypes.h"

#include <vector>

class bai
{
public:
/*	typedef struct Vertex
	{
		float x, y, z;

		Vertex(float x = 0,
		       float y = 0,
			   float z = 0) :
		x(x), y(y), z(z) {}
	}
	Vertex;

	typedef Vertex Vector;*/

	class RoadData
	{
	public:
		RoadData() : nLanes(0), nTrams(0), nTrains(0), unknown3(1), unknown4(0)
		{
			unknown.assign(3 * (nLanes + 1), 0);
			unknown.resize(11 + nLanes, -4.3160208e+008);
		}

		unsigned short nLanes;        // Number of lanes
		unsigned short nTrams;        // Number of tram rails (bool?)
		unsigned short nTrains;       // Number of train rails (bool?)
		unsigned short unknown3;      // Sidewalk? Always == 1 (bool?)
		unsigned short unknown4;      // Road type? 0, 1, 2 or 3

		std::vector< std::vector<float> > lanesDistances;  // Distances for right lanes
		std::vector<float> distance;                       // Outer edge distance
		std::vector<float> unknown;                        // Something about the lanes, gladly padded with 0xcdcdcdcd
		std::vector< std::vector<Vertex> > lLanesVertices; // Vertices for driving lane splines
		std::vector<Vertex> sidewalkCenter;                // Vertices for sidewalk central spline
		std::vector< std::vector<Vertex> > tramVertices;   // Vertices for tram rail splines
		std::vector< std::vector<Vertex> > trainVertices;  // Vertices for train rail splines
		std::vector<Vertex> sidewalkInner;                 // Vertices for sidewalk inner spline
		std::vector<Vertex> sidewalkOuter;                 // Vertices for sidewalk outer spline
	};

	class RoadEnd
	{
	public:
		RoadEnd() : intersectionID(0), unknown0(0xcdcd), vehicleRule(0), intersectionRoadIndex(0) {}

		unsigned long	intersectionID;
		unsigned short	unknown0;              // Always 0xcdcd
		unsigned long	vehicleRule;
		unsigned long	intersectionRoadIndex; // Index in the intersections road list or 0xcdcdcdcd
		Vertex			trafficLightOrigin;    // Origin of traffic light
		Vertex			trafficLightAxis;      // Orientation of traffic light
	};

	class Road
	{
	public:
		Road() : unknown0(0), unknown1(0), unknown2(15.0f) {}

	//	unsigned short nSections;
		unsigned short unknown0;
	//	unsigned short nBlocks;
		std::vector<unsigned short> blocks;
		float unknown1;   // Not sure. Speed threshold?
		float unknown2;   // Always == 15

		RoadData data[2]; // Left and right sides

		std::vector<float>  distance;     // Distances between centre points of the road
		std::vector<Vertex> origin;       // Coordsys for tangent, origin
		std::vector<Vector> xOrientation; // Coordsys for tangent, X axis
		std::vector<Vector> yOrientation; // Coordsys for tangent, Y axis
		std::vector<Vector> zOrientation; // Coordsys for tangent, Z axis
		std::vector<Vector> wOrientation; // Direction of the road at each cross-section

		RoadEnd end[2];
	};

	class Intersection
	{
	public:
		Intersection() : block(0) {}

	//	unsigned short id;
		unsigned short block;         // Reference to the PSDL, (index + 1)
		Vertex         center;        // Center point of intersection
	//	unsigned short nRoads;        // Number of connected roads
		std::vector<unsigned long> roads;  // Clockwise ordered references to roads connected to this intersection
	};

	bai()
	{
		culling[0].resize(1);
		culling[1].resize(1);
	}

	typedef std::vector< std::vector<unsigned short> > CullList;

	std::vector<Road> roads;
	std::vector<Intersection> junctions;
	CullList culling[2];

	error::code read_file(const char* filename, ProgressMonitor* pMonitor);
	error::code write_file(const char* filename);
};

#endif
