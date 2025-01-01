#include <game/mapitems.h>
#include <game/server/gamemodes/DDRace.h>

#include "base/system.h"
#include "ddnetpp.h"

void CGameControllerDDNetPP::HandleCharacterTiles(class CCharacter *pChr, int MapIndex)
{
	if(g_Config.m_SvOffDDPP)
	{
		CGameControllerDDRace::HandleCharacterTiles(pChr, MapIndex);
		return;
	}

	CPlayer *pPlayer = pChr->GetPlayer();
	const int ClientId = pPlayer->GetCid();

	int TileIndex = GameServer()->Collision()->GetTileIndex(MapIndex);
	int TileFIndex = GameServer()->Collision()->GetFrontTileIndex(MapIndex);

	//Sensitivity
	int S1 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x + (pChr->GetProximityRadius() / 3.f), pChr->GetPos().y - (pChr->GetProximityRadius() / 3.f)));
	int S2 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x + (pChr->GetProximityRadius() / 3.f), pChr->GetPos().y + (pChr->GetProximityRadius() / 3.f)));
	int S3 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x - (pChr->GetProximityRadius() / 3.f), pChr->GetPos().y - (pChr->GetProximityRadius() / 3.f)));
	int S4 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x - (pChr->GetProximityRadius() / 3.f), pChr->GetPos().y + (pChr->GetProximityRadius() / 3.f)));
	int Tile1 = GameServer()->Collision()->GetTileIndex(S1);
	int Tile2 = GameServer()->Collision()->GetTileIndex(S2);
	int Tile3 = GameServer()->Collision()->GetTileIndex(S3);
	int Tile4 = GameServer()->Collision()->GetTileIndex(S4);
	int FTile1 = GameServer()->Collision()->GetFrontTileIndex(S1);
	int FTile2 = GameServer()->Collision()->GetFrontTileIndex(S2);
	int FTile3 = GameServer()->Collision()->GetFrontTileIndex(S3);
	int FTile4 = GameServer()->Collision()->GetFrontTileIndex(S4);

	const int PlayerDDRaceState = pChr->m_DDRaceState;

	// solo part
	if(((TileIndex == TILE_SOLO_ENABLE) || (TileFIndex == TILE_SOLO_ENABLE)) && !Teams().m_Core.GetSolo(ClientId))
	{
		CharacterDropFlag(pChr);
	}

	for(auto &Minigame : GameServer()->m_vMinigames)
		if(Minigame->HandleCharacterTiles(pChr, MapIndex))
			return;

	HandleCharacterTilesDDPP(pChr, TileIndex, TileFIndex, Tile1, Tile2, Tile3, Tile4, FTile1, FTile2, FTile3, FTile4, PlayerDDRaceState);
	HandleCosmeticTiles(pChr);

	CGameControllerDDRace::HandleCharacterTiles(pChr, MapIndex);
}

void CGameControllerDDNetPP::HandleCharacterTilesDDPP(class CCharacter *pChr, int TileIndex, int TileFIndex, int Tile1, int Tile2, int Tile3, int Tile4, int FTile1, int FTile2, int FTile3, int FTile4, int PlayerDDRaceState)
{
	int ClientId = pChr->GetPlayer()->GetCid();
	// start
	if(((TileIndex == TILE_START) || (TileFIndex == TILE_START) || FTile1 == TILE_START || FTile2 == TILE_START || FTile3 == TILE_START || FTile4 == TILE_START || Tile1 == TILE_START || Tile2 == TILE_START || Tile3 == TILE_START || Tile4 == TILE_START) && (PlayerDDRaceState == DDRACE_NONE || PlayerDDRaceState == DDRACE_FINISHED || (PlayerDDRaceState == DDRACE_STARTED && !GameServer()->GetDDRaceTeam(ClientId) && g_Config.m_SvTeam != 3)))
	{
		pChr->GetPlayer()->m_MoneyTilePlus = true;
		if(pChr->GetPlayer()->m_QuestState == CPlayer::QUEST_RACE)
		{
			if((pChr->GetPlayer()->m_QuestStateLevel == 3 || pChr->GetPlayer()->m_QuestStateLevel == 8) && pChr->GetPlayer()->m_QuestProgressValue)
			{
				GameServer()->QuestAddProgress(pChr->GetPlayer()->GetCid(), 2);
			}
			else if(pChr->GetPlayer()->m_QuestStateLevel == 9 && pChr->GetPlayer()->m_QuestFailed)
			{
				// GameServer()->SendChatTarget(pChr->GetPlayer()->GetCid(), "[QUEST] running agian.");
				pChr->GetPlayer()->m_QuestFailed = false;
			}
		}
		pChr->m_DDPP_Finished = false;
	}
	// finish
	if(((TileIndex == TILE_FINISH) || (TileFIndex == TILE_FINISH) || FTile1 == TILE_FINISH || FTile2 == TILE_FINISH || FTile3 == TILE_FINISH || FTile4 == TILE_FINISH || Tile1 == TILE_FINISH || Tile2 == TILE_FINISH || Tile3 == TILE_FINISH || Tile4 == TILE_FINISH) && PlayerDDRaceState == DDRACE_STARTED)
	{
		if(pChr->GetPlayer()->m_QuestState == CPlayer::QUEST_RACE)
		{
			if(pChr->GetPlayer()->m_QuestStateLevel == 5)
			{
				if(HasFlag(pChr) != -1) //has flag
				{
					GameServer()->QuestCompleted(pChr->GetPlayer()->GetCid());
				}
				else
				{
					GameServer()->QuestFailed(pChr->GetPlayer()->GetCid());
				}
			}
			else if(pChr->GetPlayer()->m_QuestStateLevel == 9)
			{
				if(!pChr->GetPlayer()->m_QuestFailed)
				{
					GameServer()->QuestCompleted(pChr->GetPlayer()->GetCid());
				}
			}
		}

		pChr->m_DummyFinished = true;
		pChr->m_DummyFinishes++;

		/*
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "xp [%d/1000]", pChr->GetPlayer()->GetXP());
		GameServer()->SendBroadcast(aBuf, pChr->GetPlayer()->GetCid(), 0);
		*/
	}
}

void CGameControllerDDNetPP::HandleCosmeticTiles(CCharacter *pChr)
{
	int TileIndex = pChr->m_TileIndex;
	int FrontTileIndex = pChr->m_TileFIndex;
	CPlayer *pPlayer = pChr->GetPlayer();

	// cosmetic tiles
	// rainbow
	if(((TileIndex == TILE_RAINBOW) || (FrontTileIndex == TILE_RAINBOW)))
	{
		if(((pChr->m_LastIndexTile == TILE_RAINBOW) || (pChr->m_LastIndexFrontTile == TILE_RAINBOW)))
			return;

		if((pChr->m_Rainbow) || (pChr->GetPlayer()->m_InfRainbow))
		{
			GameServer()->SendChatTarget(pChr->GetPlayer()->GetCid(), "You lost rainbow!");
			pChr->m_Rainbow = false;
			pChr->GetPlayer()->m_InfRainbow = false;
		}
		else
		{
			GameServer()->SendChatTarget(pChr->GetPlayer()->GetCid(), "You got rainbow!");
			pChr->m_Rainbow = true;
		}
	}

	// bloody
	if(((TileIndex == TILE_BLOODY) || (FrontTileIndex == TILE_BLOODY)))
	{
		if(((pChr->m_LastIndexTile == TILE_BLOODY) || (pChr->m_LastIndexFrontTile == TILE_BLOODY)))
			return;

		if(pChr->HasBloody())
		{
			GameServer()->SendChatTarget(pChr->GetPlayer()->GetCid(), "You lost bloody!");
			pChr->m_Bloody = false;
			pChr->m_StrongBloody = false;
			pChr->GetPlayer()->m_InfBloody = false;
		}
		else
		{
			GameServer()->SendChatTarget(pChr->GetPlayer()->GetCid(), "You got bloody!");
			pChr->m_Bloody = true;
		}
	}

	// atom
	if(((TileIndex == TILE_ATOM) || (FrontTileIndex == TILE_ATOM)))
	{
		if(((pChr->m_LastIndexTile == TILE_ATOM) || (pChr->m_LastIndexFrontTile == TILE_ATOM)))
			return;

		if(pChr->m_Atom || pChr->GetPlayer()->m_InfAtom)
		{
			GameServer()->SendChatTarget(pChr->GetPlayer()->GetCid(), "You lost atom!");
			pChr->m_Atom = false;
			pChr->GetPlayer()->m_InfAtom = false;
		}
		else
		{
			GameServer()->SendChatTarget(pChr->GetPlayer()->GetCid(), "You got atom!");
			pChr->m_Atom = true;
		}
	}

	// trail
	if(((TileIndex == TILE_TRAIL) || (FrontTileIndex == TILE_TRAIL)))
	{
		if(((pChr->m_LastIndexTile == TILE_TRAIL) || (pChr->m_LastIndexFrontTile == TILE_TRAIL)))
			return;

		if(pChr->m_Trail || pChr->GetPlayer()->m_InfTrail)
		{
			GameServer()->SendChatTarget(pChr->GetPlayer()->GetCid(), "You lost trail!");
			pChr->m_Trail = false;
			pChr->GetPlayer()->m_InfTrail = false;
		}
		else
		{
			GameServer()->SendChatTarget(pChr->GetPlayer()->GetCid(), "You got trail!");
			pChr->m_Trail = true;
		}
	}

	// spread gun
	if(((TileIndex == TILE_SPREAD_GUN) || (FrontTileIndex == TILE_SPREAD_GUN)))
	{
		if(((pChr->m_LastIndexTile == TILE_SPREAD_GUN) || (pChr->m_LastIndexFrontTile == TILE_SPREAD_GUN)))
			return;

		if(pChr->m_autospreadgun || pPlayer->m_InfAutoSpreadGun)
		{
			GameServer()->SendChatTarget(pPlayer->GetCid(), "You lost spread gun!");
			pChr->m_autospreadgun = false;
			pPlayer->m_InfAutoSpreadGun = false;
		}
		else
		{
			GameServer()->SendChatTarget(pPlayer->GetCid(), "You got spread gun!");
			pChr->m_autospreadgun = true;
		}
	}
}
