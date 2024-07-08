/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/math.h>
#include <base/system.h>
#include <base/vmath.h>

#include <cmath>
#include <engine/kernel.h>
#include <engine/map.h>

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

vec2 CCollision::GetBlockTournamentSpawn(int Num)
{
	return GetTileAtNum(TILE_BLOCK_TOURNA_SPAWN, Num);
}

vec2 CCollision::GetTileAtNum(int Tile, int Num)
{
	if(!m_vTiles[Tile].empty())
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
	if(!m_vTiles[Tile].empty())
	{
		int Rand = rand() % m_vTiles[Tile].size();
		return m_vTiles[Tile][Rand];
	}
	return vec2(-1, -1);
}

void CCollision::ModifyTile(int x, int y, int Group, int Layer, int Index, int Flags)
{
	CMapItemGroup *pGroup = m_pLayers->GetGroup(Group);
	CMapItemLayer *pLayer = m_pLayers->GetLayer(pGroup->m_StartLayer + Layer);
	if(pLayer->m_Type == LAYER_GAME)
	{
		dbg_msg("modify_tile", "is game layer");
		SetCollisionAt(x, y, Index);
		return;
	}
	if(pLayer->m_Type != LAYERTYPE_TILES)
	{
		dbg_msg("modify_tile", "expected layertype tiles but got %d", pLayer->m_Type);
		return;
	}

	CMapItemLayerTilemap *pTilemap = reinterpret_cast<CMapItemLayerTilemap *>(pLayer);
	int TotalTiles = pTilemap->m_Width * pTilemap->m_Height;
	int MapIndex = y * pTilemap->m_Width + x;
	if(MapIndex < 0 || MapIndex >= TotalTiles)
	{
		dbg_msg("modify_tiles", "index %d is out of range %d-%d x=%d y=%d w=%d h=%d", 0, MapIndex, TotalTiles, x, y, pTilemap->m_Width, pTilemap->m_Height);
		return;
	}

	CTile *pTiles = static_cast<CTile *>(m_pLayers->Map()->GetData(pTilemap->m_Data));
	pTiles[MapIndex].m_Index = Index;
	pTiles[MapIndex].m_Flags = Flags;
}
