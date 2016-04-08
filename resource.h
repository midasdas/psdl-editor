#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#define IDAPPLY 3

#define IDD_OPEN_EXT		105
#define IDC_OPEN_IN			50
#define IDC_PROGRESS		80

#define IDD_ABOUTBOX		100
	#define IDC_VERSIONSTATIC	7000
	#define IDC_INFOSTATIC		7001
	#define IDS_VERSION			7002
	#define IDS_INFO			7003

#define IDR_MAINFRAME		128
#define IDR_TOOLBOX			130
#define IDR_PROPVIEW		131
#define IDR_POPUP_BLOCK		140
#define IDR_POPUP_ATTRIBUTE	141
#define IDR_POPUP_PERIMETER	142

#define IDB_FLASH4			150

#define ID_FILE_NEW_PSDL					1101
#define ID_FILE_NEW_CPVS					1102
#define ID_FILE_NEW_INST					1103
#define ID_FILE_NEW_BAI						1104
#define ID_FILE_NEW_PATHSET					1105
#define ID_FILE_NEW_WORKSPACE				1106
#define ID_FILE_OPEN_CITY					1107
#define ID_FILE_OPEN_WORKSPACE				1108
#define ID_FILE_SAVE_ALL					1119
#define ID_FILE_SAVE_WORKSPACE				1110
#define ID_FILE_SAVE_WORKSPACE_AS			1111
#define ID_FILE_IMPORT						1112
#define ID_FILE_EXPORT						1113
#define ID_FILE_CLOSE_WORKSPACE				1114
#define ID_FILE_CLOSE_ALL					1115
#define ID_FILE_OPENCONTAININGFOLDER		1116

#define ID_EDIT_TRANSFORM					1117

#define ID_WINDOWS_CITYBLOCKS				1201
#define ID_WINDOWS_PERIMETER				1202
#define ID_WINDOWS_ATTRIBUTES				1203
#define ID_WINDOWS_PROPERTIES				1204
#define ID_WINDOWS_VERTICES					1205
#define ID_WINDOWS_TEXTURES					1206
#define ID_WINDOWS_BAI_ROADS				1207
#define ID_WINDOWS_BAI_INTERSECTIONS		1208
#define ID_WINDOWS_BAI_CULLING				1209

#define ID_VIEW_RESET_CAMERA				1210
#define ID_VIEW_GRID						1211
#define ID_VIEW_WIREFRAME					1212

#define ID_VIEW_ROADS						1221
#define ID_VIEW_SIDEWALKSTRIPS				1222
#define ID_VIEW_RECTANGLESTRIPS				1223
#define ID_VIEW_SLIVERS						1224
#define ID_VIEW_CROSSWALKRECTS				1225
#define ID_VIEW_ROADTRIFANS					1226
#define ID_VIEW_TRIFANS						1227
#define ID_VIEW_DIVIDEDROADS				1228
#define ID_VIEW_TUNNELJUNCTIONS				1229
#define ID_VIEW_TUNNELS						1230
#define ID_VIEW_TEXTURES					1231
#define ID_VIEW_FACADES						1232
#define ID_VIEW_ROOFTRIFANS					1233
#define ID_VIEW_FILTER						1234

#define ID_VIEW_MACROS						1235

#define ID_MODE_PSDL						1241
#define ID_MODE_CPVS						1242
#define ID_MODE_INST						1243
#define ID_MODE_BAI							1244
#define ID_MODE_PATHSET						1245

#define ID_INSERT_CITYBLOCK					1301
#define ID_INSERT_VERTEX					1302
#define ID_INSERT_TEXTURE					1303
#define ID_INSERT_MACRO_ROAD				1304
#define ID_INSERT_MACRO_INTERSECTION		1305
#define ID_INSERT_MACRO_BUILDING			1306
#define ID_INSERT_DUPLICATE_BLOCKS			1307
#define ID_INSERT_DUPLICATE_VERTICES		1308
#define ID_INSERT_GENERATE_PERIMETER		1309

#define ID_MACRO_GENERATE					1401
#define ID_MACRO_GENERATE_ALL				1402

#define ID_TOOLS_MM2						1501
#define ID_TOOLS_GENERATE_PERIMETERS		1502
#define ID_TOOLS_OPTIMIZE					1503
#define ID_TOOLS_OPTIONS					1504
#define ID_TOOLS_SCREENSHOT					1505

#define ID_OPTIONS_TOOLS					2101

#define IDS_SAVE_CHANGES		9001
#define IDS_INVALID_FILE		9002
#define IDS_UNDO_COMMAND		9101
#define IDS_REDO_COMMAND		9102

#define ID_UNDO_OFFSET				8000
#define IDS_INSERT_CITYBLOCK			9301
#define IDS_INSERT_VERTEX				9302
#define IDS_INSERT_DUPLICATE_BLOCKS		9307
#define IDS_INSERT_DUPLICATE_VERTICES	9308

#define IDD_DUPLICATE		101
	#define IDC_COUNT			1100
	#define IDC_VERTICES		1101
	#define IDC_PERIMETERS		1102
	#define IDC_NEIGHBOURS		1103
	#define IDC_TEXTURES		1104
	#define IDC_ATTRIBUTES		1105
	#define IDC_SELECT			1106

#define IDD_PERIMETERS		102

#define IDD_OPTIMIZE		103
	#define IDC_TEXTURE_REFS	1300
	#define IDC_EMPTY_BLOCKS	1301

#define IDD_TRANSFORM		104
	#define IDC_X				1401
	#define IDC_Y				1402
	#define IDC_Z				1403
	#define IDC_ANGLE			1404
	#define IDC_X_STATIC		1405
	#define IDC_Y_STATIC		1406
	#define IDC_Z_STATIC		1407
	#define IDC_ANGLE_STATIC	1408

#define IDD_OPTIONS			500
	#define IDD_OPTIONS_GENERAL		501
	#define IDD_OPTIONS_RENDER		502
		#define IDC_USE_TEXTURES		5021
		#define IDC_TEXTURE_FILTER		5022
	#define IDD_OPTIONS_TOOLS		503
		#define IDC_MM2PATH				5031
	#define IDD_OPTIONS_DIRECTORIES	504

#define IDD_PROGRESS		800

#define IDS_UNKNOWN			1
#define IDS_SUBTERANEAN		2
#define IDS_PLAIN			3
#define IDS_ROAD			4
#define IDS_INTERSECTION	5
#define IDS_INST			6
#define IDS_MULTIPLE		7
#define IDS_NONE			8

#define IDS_ATB_UNKNOWN			4000
#define IDS_ATB_ROAD			4001
#define IDS_ATB_SIDEWALK		4002
#define IDS_ATB_RECTANGLE		4003
#define IDS_ATB_SLIVER			4004
#define IDS_ATB_CROSSWALK		4005
#define IDS_ATB_ROADTRIANGLEFAN	4006
#define IDS_ATB_TRIANGLEFAN		4007
#define IDS_ATB_FACADEBOUND		4008
#define IDS_ATB_DIVIDEDROAD		4009
#define IDS_ATB_TUNNEL			4010
#define IDS_ATB_TEXTURE			4011
#define IDS_ATB_FACADE			4012
#define IDS_ATB_ROOFTRIANGLEFAN	4013

#define IDD_ATB_NONE			200
#define IDD_ATB_UNKNOWN			201
#define IDD_ATB_ROAD			202
#define IDD_ATB_SIDEWALK		203
#define IDD_ATB_RECTANGLE		204
#define IDD_ATB_SLIVER			205
#define IDD_ATB_CROSSWALK		206
#define IDD_ATB_ROADTRIANGLEFAN	207
#define IDD_ATB_TRIANGLEFAN		208
#define IDD_ATB_FACADEBOUND		209
#define IDD_ATB_DIVIDEDROAD		210
#define IDD_ATB_TUNNEL			211
#define IDD_ATB_TEXTURE			212
#define IDD_ATB_FACADE			213
#define IDD_ATB_ROOFTRIANGLEFAN	214

#define IDC_FRAME1                      1001
#define IDC_EDIT1                       1002
#define IDC_CHECK1                      1003
#define IDC_RADIO1                      1004
#define IDC_LIST1                       1005
#define IDC_STATIC_A                    1006
#define IDC_GROUPBOX_A                  1007
#define IDC_TREE						1008
#define IDC_PLACEHOLDER					1009
#define IDC_BROWSE						1010
#define IDC_SPIN1						1011
#define IDC_SPIN2						1012
#define IDC_COMBO						1013		

#define IDC_INFO			2000
#define IDC_BOX				2001
#define IDC_LIST			2002
#define IDC_EDIT			2003
#define IDC_RESET			2004
#define IDC_GROUP1			2005
#define IDC_GROUP2			2006
#define IDC_GROUP3			2007

#define IDC_ADD				2101
#define IDC_DEL				2102
#define IDC_UP				2103
#define IDC_DOWN			2104

#define IDC_RAILING			2901
#define IDC_WALL			2902
#define IDC_LEFT			2903
#define IDC_RIGHT			2904
#define IDC_CURVEDWALL		2905
#define IDC_FLAT			2906
#define IDC_CURVED			2907
#define IDC_HEIGHT1			2908
#define IDC_HEIGHT2			2909

#define IDC_CLOSEDSTART_L	2911
#define IDC_CLOSEDEND_L		2912
#define IDC_CLOSEDSTART_R	2913
#define IDC_CLOSEDEND_R		2914
#define IDC_OFFSETSTART_L	2915
#define IDC_OFFSETEND_L		2916
#define IDC_OFFSETSTART_R	2917
#define IDC_OFFSETEND_R		2918

#endif
