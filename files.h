#ifndef __FILES_H__
#define __FILES_H__

#include <map>
#include <string>

class BAISettings
{
public:
	class RoadSettings
	{
	public:
		RoadSettings(unsigned short nRightLanes = 0, unsigned long vehicleRule1 = 1, unsigned long vehicleRule2 = 1, unsigned short unknown0 = 0)
			: nRightLanes(nRightLanes), vehicleRule1(vehicleRule1), vehicleRule2(vehicleRule2), unknown0(unknown0) {}

		unsigned short nRightLanes;
		unsigned long vehicleRule1;
		unsigned long vehicleRule2;
		unsigned short unknown0;
	};

	class BlockSettings
	{
	public:
		BlockSettings(float fEdgeDistance = 0)
			: fEdgeDistance(fEdgeDistance) {}

		float fEdgeDistance;
	};

	class TextureSettings
	{
	public:
		TextureSettings(unsigned short nLanes = 0, float fInnerEdge = 0, float fOuterEdge = 1)
			: nLanes(nLanes), fInnerEdge(fInnerEdge), fOuterEdge(fOuterEdge) {}

		unsigned short nLanes;
		float fInnerEdge;
		float fOuterEdge;
	};

	BAISettings(std::string strFile);

	std::map< unsigned long, RoadSettings  > roads;
	std::map< unsigned long, BlockSettings > blocks;
	std::map< std::string, TextureSettings > textures;
};

#endif
