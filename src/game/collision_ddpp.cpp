/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/system.h>
#include <base/vmath.h>

#include <engine/kernel.h>
#include <engine/map.h>
#include <math.h>

#include <game/collision.h>
#include <game/layers.h>
#include <game/mapitems.h>

#include <engine/shared/config.h>

int CCollision::GetCustTile(int x, int y)
{
	if(!m_pTiles)
		return 0;

	int Nx = clamp(x / 32, 0, m_Width - 1);
	int Ny = clamp(y / 32, 0, m_Height - 1);
	int pos = Ny * m_Width + Nx;

	return m_pTiles[pos].m_Index;
}

int CCollision::GetCustFTile(int x, int y)
{
	if(!m_pFront)
		return 0;

	int Nx = clamp(x / 32, 0, m_Width - 1);
	int Ny = clamp(y / 32, 0, m_Height - 1);
	int pos = Ny * m_Width + Nx;

	return m_pFront[pos].m_Index;
}

vec2 CCollision::GetSurvivalSpawn(int Num)
{
	return GetTileAtNum(TILE_SURVIVAL_SPAWN, Num);
}

vec2 CCollision::GetTileAtNum(int Tile, int Num)
{
	if(m_vTiles[Tile].size())
	{
		int Amount = m_vTiles[Tile].size();
		if(Num > Amount)
		{
			dbg_msg("GetTileAtNum", "Error: requested too high index %d/%d for tile=%d", Num, Amount, Tile);
			Num = Amount;
		}
		return m_vTiles[Tile][Num];
	}
	return vec2(-1, -1);
}

// by fokkonaut from F-DDrace
vec2 CCollision::GetRandomTile(int Tile)
{
	if(m_vTiles[Tile].size())
	{
		int Rand = rand() % m_vTiles[Tile].size();
		return m_vTiles[Tile][Rand];
	}
	return vec2(-1, -1);
}
