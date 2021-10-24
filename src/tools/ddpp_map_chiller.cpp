/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

/******************************************************
*													  *
*				map_chiller.cpp						  *
*			Map Generator Tool by QshaR				  *
*		      modded by ChillerDragon				  *
*													  *
*													  *
*******************************************************/

/*

	This tool is hard modded version of QshaR's map gerator
	ChillerDragon does some experimental hardcoding here

*/

#include <base/system.h>
#include <engine/shared/datafile.h>
#include <engine/storage.h>
#include <game/gamecore.h>
#include <game/mapitems.h>

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include <sys/resource.h>

#define GO_LEFT 1
#define GO_RIGHT 2
#define GO_UP 4
#define GO_DOWN 8
#define USED 16
#define SOLUTION 32

#define M(x, y) maze_data[(x) + 1 + ((y) + 1) * (c + 2)]

#define dprintf \
	if(0) \
	printf
#define cprintf \
	if(0) \
	printf
#define cputchar \
	if(0) \
	putchar

enum
{
	Q_TILE_LAYER_HOOK = 0,
	Q_TILE_LAYER_FREEZE,
	Q_TILE_LAYER_GAME,
	Q_TILE_LAYER_CHILLER,
};
//initial numbers
int maze_seed = 5000;
int mapwidth = 0;
int mapheight = 0;
int maze_cells_width = 7;
int maze_cells_height = 7;
int maze_cols = (mapwidth - 1) / maze_cells_width;
int maze_rows = (mapheight - 1) / maze_cells_width;

void update_tile(CTile *pTile, int tile_type) // hardcode tiles and generate start-finish
{
	if((tile_type == Q_TILE_LAYER_GAME) || (tile_type == Q_TILE_LAYER_FREEZE))
	{
		for(int i = 0; i < mapwidth * mapheight; i++)
		{
			if((pTile[i].m_Index == 0) && (i >= mapwidth) && (mapwidth <= mapwidth * (mapheight - 1)))
				if((pTile[i - 1].m_Index == 1) || (pTile[i + 1].m_Index == 1) || (pTile[i - mapwidth].m_Index == 1) || (pTile[i + mapwidth].m_Index == 1) ||
					(pTile[i - mapwidth - 1].m_Index == 1) || (pTile[i - mapwidth + 1].m_Index == 1) || (pTile[i + mapwidth - 1].m_Index == 1) || (pTile[i + mapwidth + 1].m_Index == 1))
					pTile[i].m_Index = 9;
		}
	}

	if(tile_type == Q_TILE_LAYER_FREEZE)
	{
		for(int i = 0; i < mapwidth * mapheight; i++)
		{
			if(pTile[i].m_Index == 1)
				pTile[i].m_Index = 110;
			if(pTile[i].m_Index == 9)
				pTile[i].m_Index = 110;
		}
		pTile[mapwidth * 1 + 1].m_Index = 0;
		pTile[mapwidth * 1 + 2].m_Index = 0;
		pTile[mapwidth * 1 + 3].m_Index = 0;
		pTile[mapwidth * 1 + 4].m_Index = 0;
		pTile[mapwidth * 1 + 5].m_Index = 0;
	}
	if(tile_type == Q_TILE_LAYER_GAME)
	{
		pTile[mapwidth * 2 + 2].m_Index = 192;
		pTile[mapwidth * 2 + 3].m_Index = 192;
		pTile[mapwidth * 2 + 4].m_Index = 192;

		pTile[mapwidth * 1 + 1].m_Index = 33;
		pTile[mapwidth * 1 + 2].m_Index = 33;
		pTile[mapwidth * 1 + 3].m_Index = 33;
		pTile[mapwidth * 1 + 4].m_Index = 33;
		pTile[mapwidth * 1 + 5].m_Index = 33;
		pTile[mapwidth * 3 + 1].m_Index = 33;
		pTile[mapwidth * 3 + 2].m_Index = 33;
		pTile[mapwidth * 3 + 3].m_Index = 33;
		pTile[mapwidth * 3 + 4].m_Index = 33;
		pTile[mapwidth * 3 + 5].m_Index = 33;
		pTile[mapwidth * 2 + 1].m_Index = 33;
		pTile[mapwidth * 2 + 5].m_Index = 33;

		pTile[mapwidth * 4 + 2].m_Index = 1;
		pTile[mapwidth * 4 + 3].m_Index = 1;
		pTile[mapwidth * 4 + 4].m_Index = 1;

		pTile[mapwidth * (mapheight - (mapheight % maze_cells_height) - 1) - (mapwidth % maze_cells_width)].m_Index = 34;
		pTile[mapwidth * (mapheight - (mapheight % maze_cells_height) - 2) - (mapwidth % maze_cells_width)].m_Index = 34;
		pTile[mapwidth * (mapheight - (mapheight % maze_cells_height) - 3) - (mapwidth % maze_cells_width)].m_Index = 34;
		pTile[mapwidth * (mapheight - (mapheight % maze_cells_height) - 4) - (mapwidth % maze_cells_width)].m_Index = 34;
	}

	if(tile_type == Q_TILE_LAYER_HOOK)
	{
		/*
		int spawn_width = 20;
		for (int i = 2; i < spawn_width; i++)
		{
			pTile[mapwidth * 4 + i].m_Index = 1;
		}
		*/
		pTile[mapwidth * 4 + 2].m_Index = 1;
		pTile[mapwidth * 4 + 3].m_Index = 1;
		//pTile[mapwidth * 4 + 10].m_Index = 1;
	}

	if(tile_type == Q_TILE_LAYER_CHILLER) //generates a spawn platform offsetby 1 tile
	{
		pTile[mapwidth * 4 + 1].m_Index = 1;
		pTile[mapwidth * 4 + 2].m_Index = 1;
		pTile[mapwidth * 4 + 3].m_Index = 1;
	}
}

int main(int argc, const char **argv)
{
	IStorage *pStorage = CreateLocalStorage();
	char aFileName[1024];
	str_format(aFileName, sizeof(aFileName), "%s", argv[2]);

	CDataFileReader DataFile;
	DataFile.Open(pStorage, argv[1], IStorage::TYPE_ALL);

	CDataFileWriter df;
	df.Init();
	dbg_logger_stdout();
	int SettingsIndex = DataFile.NumData();
	//dbg_msg("chiller", "settingsindex: %d", SettingsIndex); //holds the number of all tile layers in the base map

	int Start, Num;
	DataFile.GetType(MAPITEMTYPE_LAYER, &Start, &Num);

	CMapItemLayer *pItem;
	CMapItemLayerTilemap *pTilemap;

	int GameLayerNumber = 0;
	int FreezeLayerNumber = 0;
	int StonesLayerNumber = 0;
	int mapwidthFreeze = 0;
	int mapheightFreeze = 0;
	int mapwidthStones = 0;
	int mapheightStones = 0;

	//ChillerDragon
	int ChillerLayerNumber = 0;
	int mapwidthChiller = 0;
	int mapheightChiller = 0;

	maze_seed = time(0) % 100000; // generates random seed
	for(int i = 0; i < DataFile.NumItems(); i++)
	{
		if(i >= Start && i < Start + Num)
		{
			pItem = (CMapItemLayer *)DataFile.GetItem(i, 0, 0);
			pTilemap = (CMapItemLayerTilemap *)pItem;
			(CTile *)DataFile.GetData(pTilemap->m_Data);
			char aName[16];
			IntsToStr(pTilemap->m_aName, sizeof(pTilemap->m_aName) / sizeof(int), aName);
			if(str_comp_num(aName, "Game", sizeof("Game")) == 0)
			{
				GameLayerNumber = pTilemap->m_Data;
				mapwidth = pTilemap->m_Width;
				mapheight = pTilemap->m_Height;
				maze_cols = (mapwidth - 1) / maze_cells_width;
				maze_rows = (mapheight - 1) / maze_cells_width;
				dbg_msg("MazeInfo", "Cols: %d | Rows: %d | mapwidth: %d | mapheight: %d | Seed: %d | cell_width: %d | cell_height: %d", maze_cols, maze_rows, mapwidth, mapheight, maze_seed, maze_cells_width, maze_cells_height);
			}
			if(str_comp_num(aName, "Freeze", sizeof("Freeze")) == 0)
			{
				FreezeLayerNumber = pTilemap->m_Data;
				mapwidthFreeze = pTilemap->m_Width;
				mapheightFreeze = pTilemap->m_Height;
			}
			if(str_comp_num(aName, "Stones", sizeof("Stones")) == 0)
			{
				StonesLayerNumber = pTilemap->m_Data;
				mapwidthStones = pTilemap->m_Width;
				mapheightStones = pTilemap->m_Height;
			}
			if(str_comp_num(aName, "Chiller", sizeof("Chiller")) == 0)
			{
				ChillerLayerNumber = pTilemap->m_Data;
				mapwidthChiller = pTilemap->m_Width;
				mapheightChiller = pTilemap->m_Height;
			}
			dbg_msg("LayerInfo", "aName: %s i=%d Width=%d Height=%d Data=%d", aName, i, pTilemap->m_Width, pTilemap->m_Height, pTilemap->m_Data);
		}
		int TypeID;
		int ItemID;
		int *pData = (int *)DataFile.GetItem(i, &TypeID, &ItemID);
		int Size = DataFile.GetItemSize(i) - sizeof(int) * 2;

		df.AddItem(TypeID, ItemID, Size, pData);
	}

	for(int i = 0; i < DataFile.NumData() || i == SettingsIndex; i++) //iterate through all tile layers (each iteration is a single layer)
	{
		// int Size = DataFile.GetUncompressedDataSize(i);
		if(GameLayerNumber && i == GameLayerNumber) //game
		{
			CTile *pTile;
			pTile = (CTile *)DataFile.GetData(i);
			dbg_msg("Layer", "Game i: %d", i);
			update_tile(&*pTile, Q_TILE_LAYER_GAME);
			df.AddData(mapwidth * mapheight * sizeof(CTile), pTile);
			DataFile.UnloadData(i);
		}
		else if(FreezeLayerNumber && i == FreezeLayerNumber) //Freeze
		{
			if(mapwidthFreeze != mapwidth || mapheightFreeze != mapheight)
			{
				dbg_msg("ERROR", "Freeze layer (%d x %d) is not the same size as Game (%d x %d)", mapwidthFreeze, mapheightFreeze, mapwidth, mapheight);
				return false;
			}
			CTile *pTile;
			pTile = (CTile *)DataFile.GetData(i);
			dbg_msg("Layer", "Freeze i: %d", i);
			update_tile(&*pTile, Q_TILE_LAYER_FREEZE);
			df.AddData(mapwidthFreeze * mapheightFreeze * sizeof(CTile), pTile);
			DataFile.UnloadData(i);
		}
		else if(StonesLayerNumber && i == StonesLayerNumber) //Stones
		{
			if(mapwidthStones != mapwidth || mapheightStones != mapheight)
			{
				dbg_msg("ERROR", "Stones layer (%d x %d) is not the same size as Game (%d x %d)", mapwidthStones, mapheightStones, mapwidth, mapheight);
				return false;
			}
			CTile *pTile;
			pTile = (CTile *)DataFile.GetData(i);
			dbg_msg("Layer", "Stones i: %d", i);
			update_tile(&*pTile, Q_TILE_LAYER_HOOK);
			df.AddData(mapwidthStones * mapheightStones * sizeof(CTile), pTile);
			DataFile.UnloadData(i);
		}
		else if(ChillerLayerNumber && i == ChillerLayerNumber) //Chiller
		{
			if(mapwidthChiller != mapwidth || mapheightStones != mapheight)
			{
				dbg_msg("ERROR", "Chiller layer (%d x %d) is not the same size as Game (%d x %d)", mapwidthChiller, mapheightChiller, mapwidth, mapheight);
				return false;
			}
			CTile *pTile;
			pTile = (CTile *)DataFile.GetData(i);
			dbg_msg("Layer", "Chiller i: %d", i);
			update_tile(&*pTile, Q_TILE_LAYER_CHILLER);
			df.AddData(mapwidthChiller * mapheightChiller * sizeof(CTile), pTile);
			DataFile.UnloadData(i);
		}
		else
		{
			dbg_msg("Layer", "Other i: %d", i);
			// unsigned char *pData = (unsigned char *)DataFile.GetData(i);
			// df.AddData(Size, pData);
			// DataFile.UnloadData(i);
		}
	}

	dbg_msg("mapchange", "imported settings");
	DataFile.Close();
	df.OpenFile(pStorage, aFileName);
	df.Finish();

	return 0;
}
