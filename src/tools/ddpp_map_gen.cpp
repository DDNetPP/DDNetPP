/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

/******************************************************
*													  *
*					map_gen.cpp						  *
*			Map Generator Tool by QshaR				  *
*		      modded by ChillerDragon				  *
*													  *
*													  *
*******************************************************/

/*

	This tool is the map generator tool by QshaR's only slightly modded by ChillerDragon
	It creates maze gores maps as described at: https://forum.ddnet.tw/viewtopic.php?f=48&t=5552

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
		pTile[mapwidth * 4 + 2].m_Index = 1;
		pTile[mapwidth * 4 + 3].m_Index = 1;
		pTile[mapwidth * 4 + 4].m_Index = 1;
	}

	if(tile_type == Q_TILE_LAYER_CHILLER) //generates a spawn platform offsetby 1 tile
	{
		pTile[mapwidth * 4 + 1].m_Index = 1;
		pTile[mapwidth * 4 + 2].m_Index = 1;
		pTile[mapwidth * 4 + 3].m_Index = 1;
	}
}

//generate_maze(&*pTile, maze_cols, maze_rows, maze_cells_width, maze_cells_height, maze_seed);
void generate_maze(CTile *pTile, int c, int r, int w, int h, int s)
{
	int solve = 0;

	char *maze_data;
	int x, y;
	int *xt, *yt;

	if(c < 1 || r < 1 || w < 1 || h < 1)
		return;

	maze_data = (char *)calloc(r + 2, c + 2);
	xt = (int *)malloc(c * r * sizeof(int));
	yt = (int *)malloc(c * r * sizeof(int));

	for(x = 0; x < c; x++)
		for(y = 0; y < r; y++)
		{
			xt[x + y * c] = x;
			yt[x + y * c] = y;
		}

	for(x = -1; x <= c; x++)
	{
		M(x, -1) = USED;
		M(x, r) = USED;
	}
	for(y = 0; y <= r; y++)
	{
		M(-1, y) = USED;
		M(c, y) = USED;
	}

	if(s == 0)
		s = time(0);
	srand(s);

	for(x = r * c - 1; x > 0; x--)
	{
		y = rand() % (x + 1);
		if(y != x)
		{
			int t = xt[x];
			xt[x] = xt[y];
			xt[y] = t;
			t = yt[x];
			yt[x] = yt[y];
			yt[y] = t;
		}
	}

	M(-1, 0) |= USED | GO_RIGHT;
	M(0, 0) |= USED | GO_LEFT;

	int num_left = r * c - 1;
	while(num_left)
		for(x = 0; x < c; x++)
			for(y = 0; y < r; y++)
			{
				dprintf("%d, %d\n", x, y);
				while(1)
				{
					int px = xt[x + y * c];
					int py = yt[x + y * c];
					int ways[8], wp;
					if(!(M(px, py) & USED))
						break;
					dprintf("\tstarting\n");

					while(1)
					{
						wp = 0;

						if(!(M(px, py - 1) & USED))
							ways[wp++] = GO_UP;
						if(!(M(px, py - 1) & USED))
							ways[wp++] = GO_UP;
						if(!(M(px, py + 1) & USED))
							ways[wp++] = GO_DOWN;
						if(!(M(px - 1, py) & USED))
							ways[wp++] = GO_LEFT;
						if(!(M(px - 1, py) & USED))
							ways[wp++] = GO_LEFT;
						if(!(M(px + 1, py) & USED))
							ways[wp++] = GO_RIGHT;
						if(wp == 0)
						{
							dprintf("\t\tblocked at %d,%d\n", px, py);
							break; // blocked at this point
						}
						dprintf("\t\t%d ways found from %d,%d\n", wp, px, py);

						wp = rand() % wp;

						switch(ways[wp])
						{
						case GO_LEFT:
							dprintf("\t\tleft\n");
							M(px, py) |= GO_LEFT;
							px--;
							M(px, py) |= GO_RIGHT | USED;
							break;
						case GO_RIGHT:
							dprintf("\t\tright\n");
							M(px, py) |= GO_RIGHT;
							px++;
							M(px, py) |= GO_LEFT | USED;
							break;
						case GO_UP:
							dprintf("\t\tup\n");
							M(px, py) |= GO_UP;
							py--;
							M(px, py) |= GO_DOWN | USED;
							break;
						case GO_DOWN:
							dprintf("\t\tdown\n");
							M(px, py) |= GO_DOWN;
							py++;
							M(px, py) |= GO_UP | USED;
							break;
						}
						num_left--;
					}
					dprintf("\tending at %d,%d\n", px, py);
					if(px == xt[x + y * c] && py == yt[x + y * c])
						break;
				}
			}

	M(c, r - 1) |= USED | GO_LEFT;
	M(c - 1, r - 1) |= USED | GO_RIGHT;

	if(solve)
	{
		for(x = 0; x < c; x++)
			for(y = 0; y < r; y++)
				M(x, y) |= SOLUTION;
		M(-1, 0) |= SOLUTION;
		M(0, 0) |= SOLUTION;
		M(c, r - 1) |= SOLUTION;
		M(c - 1, r - 1) |= SOLUTION;

		for(x = 0; x < c; x++)
			for(y = 0; y < r; y++)
			{
				int px = x;
				int py = y;
				int ways[4], wp;

				while(1)
				{
					wp = 0;

					if(M(px, py) & GO_UP && M(px, py - 1) & SOLUTION)
						ways[wp++] = GO_UP;
					if(M(px, py) & GO_DOWN && M(px, py + 1) & SOLUTION)
						ways[wp++] = GO_DOWN;
					if(M(px, py) & GO_LEFT && M(px - 1, py) & SOLUTION)
						ways[wp++] = GO_LEFT;
					if(M(px, py) & GO_RIGHT && M(px + 1, py) & SOLUTION)
						ways[wp++] = GO_RIGHT;
					if(wp != 1)
						break; // not a dead end

					switch(ways[0])
					{
					case GO_LEFT:
						M(px, py) &= ~SOLUTION;
						px--;
						break;
					case GO_RIGHT:
						M(px, py) &= ~SOLUTION;
						px++;
						break;
					case GO_UP:
						M(px, py) &= ~SOLUTION;
						py--;
						break;
					case GO_DOWN:
						M(px, py) &= ~SOLUTION;
						py++;
						break;
					}
				}
			}
	}

	{
		int currentPos = 0;
		int i;
		cprintf("\n");
		for(y = 0; y <= r; y++)
		{
			pTile[currentPos].m_Index = 1;
			currentPos++;
			cputchar('+');
			for(x = 0; x < c; x++)
			{
				if(M(x, y) & GO_UP || x == -1)
					for(i = 1; i < w; i++)
					{
						pTile[currentPos].m_Index = 0;
						currentPos++;
						cputchar((M(x, y) & SOLUTION && M(x, y - 1) & SOLUTION) ? '#' : ' ');
					}
				else
					for(i = 1; i < w; i++)
					{
						pTile[currentPos].m_Index = 1;
						currentPos++;
						cputchar('-');
					}
				pTile[currentPos].m_Index = 1;
				currentPos++;
				cprintf("+");
			}
			cprintf("\n");
			if(currentPos % mapwidth != 0)
				currentPos = currentPos - currentPos % mapwidth + mapwidth;
			if(y < r)
				for(int dy = 1; dy < h; dy++)
				{
					if(M(0, y) & GO_LEFT)
					{
						pTile[currentPos].m_Index = 0;
						currentPos++;
						cputchar((M(0, y) & SOLUTION && M(-1, y) & SOLUTION) ? '#' : ' ');
					}
					else
					{
						pTile[currentPos].m_Index = 1;
						currentPos++;
						cputchar('|');
					}
					for(x = 0; x < c; x++)
					{
						for(i = 1; i < w; i++)
						{
							pTile[currentPos].m_Index = 0;
							currentPos++;
							cputchar(M(x, y) & SOLUTION ? '#' : ' ');
						}
						if(M(x, y) & GO_RIGHT)
						{
							pTile[currentPos].m_Index = 0;
							currentPos++;
							cputchar((M(x, y) & SOLUTION && M(x + 1, y) & SOLUTION) ? '#' : ' ');
						}
						else
						{
							pTile[currentPos].m_Index = 1;
							currentPos++;
							cprintf("|");
						}
					}
					cprintf("\n");
					if(currentPos % mapwidth != 0)
						currentPos = currentPos - currentPos % mapwidth + mapwidth;
				}
		}
		cprintf("\n");
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
			generate_maze(&*pTile, maze_cols, maze_rows, maze_cells_width, maze_cells_height, maze_seed);
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
			generate_maze(&*pTile, maze_cols, maze_rows, maze_cells_width, maze_cells_height, maze_seed);
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
			generate_maze(&*pTile, maze_cols, maze_rows, maze_cells_width, maze_cells_height, maze_seed);
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
			generate_maze(&*pTile, maze_cols, maze_rows, maze_cells_width, maze_cells_height, maze_seed);
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
