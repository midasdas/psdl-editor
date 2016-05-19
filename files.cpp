#include "stdafx.h"
#include "files.h"

#include <fstream>
#include <iomanip>

using namespace std;

BAISettings::BAISettings(std::string strFile)
{
	if (strFile.empty()) return;

	ifstream in(strFile.c_str());

	char temp;

	string section;
	string buf;

	while (!in.eof())
	{
		temp = in.peek();

	//	ATLTRACE("%c\n", temp);

		if (temp == EOF)
		{
			break;
		}
		else if (temp == '\n')
		{
			in.ignore();
		}
		else if (temp == ';')
		{
			getline(in, buf);
		}
		else if (temp == '[')
		{
			in.ignore();
			getline(in, section, ']');
			ATLTRACE("%s:\n", section.c_str());
			getline(in, buf);
		}
		else if (section == "Textures")
		{
			string strName = "";
			unsigned short nLanes = 0;
			float fInnerEdge = 0, fOuterEdge = 0;

			in >> strName >> nLanes >> fInnerEdge >> fOuterEdge;
			getline(in, buf);

			textures[strName] = TextureSettings(nLanes, fInnerEdge, fOuterEdge);

			ATLTRACE("\tName=%s Lanes=%u InnerEdge=%f OuterEdge=%f\n", strName.c_str(), nLanes, fInnerEdge, fOuterEdge);
		}
		else if (section == "Parked Cars")
		{
			unsigned long iBlock = 0;
			float fEdgeDistance = 0;

			in >> setbase(0) >> iBlock >> fEdgeDistance;
			getline(in, buf);

			blocks[iBlock] = BlockSettings(fEdgeDistance);

			ATLTRACE("\tBlock=%u EdgeDistance=%f\n", iBlock, fEdgeDistance);
		}
		else if (section == "Roads")
		{
			unsigned long iBlock = 0;
			unsigned short nRightLanes = 0;
			unsigned long vehicleRule1 = 1;
			unsigned long vehicleRule2 = 1;
			unsigned short unknown0 = 0;

			in >> setbase(0) >> iBlock >> nRightLanes >> vehicleRule2 >> vehicleRule1 >> unknown0;
			getline(in, buf);

			roads[iBlock] = RoadSettings(nRightLanes, vehicleRule1, vehicleRule2, unknown0);

			ATLTRACE("\tBlock=%u RightLanes=%u\n", iBlock, nRightLanes);
		}
		else
		{
			in.ignore();
		}
	}
	ATLTRACE("\n");
}
