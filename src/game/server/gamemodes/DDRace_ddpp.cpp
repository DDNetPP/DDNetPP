/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
/* Based on Race mod stuff and tweaked by GreYFoX@GTi and others to fit our DDRace needs. */
#include "DDRace.h"
#include <engine/server.h>
#include <engine/shared/config.h>
#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/entities/flag.h>
#include <game/server/gamecontext.h>

#include <cmath>

bool CGameControllerDDRace::OnEntity(int Index, int x, int y, int Layer, int Flags, bool Initial, int Number)
{
	IGameController::OnEntity(Index, x, y, Layer, Flags, Initial, Number);

	const vec2 Pos(x * 32.0f + 16.0f, y * 32.0f + 16.0f);
	int Team = -1;
	if(Index == ENTITY_FLAGSTAND_RED)
		Team = TEAM_RED;
	if(Index == ENTITY_FLAGSTAND_BLUE)
		Team = TEAM_BLUE;
	if(Team == -1 || m_apFlags[Team])
		return false;

	CFlag *F = new CFlag(&GameServer()->m_World, Team);
	F->m_StandPos = Pos;
	F->m_Pos = Pos;
	m_apFlags[Team] = F;
	GameServer()->m_World.InsertEntity(F);
	return true;
}

void CGameControllerDDRace::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	int FlagCarrierRed = FLAG_MISSING;
	if(m_apFlags[TEAM_RED])
	{
		if(m_apFlags[TEAM_RED]->m_AtStand)
			FlagCarrierRed = FLAG_ATSTAND;
		else if(m_apFlags[TEAM_RED]->GetCarrier() && m_apFlags[TEAM_RED]->GetCarrier()->GetPlayer())
			FlagCarrierRed = m_apFlags[TEAM_RED]->GetCarrier()->GetPlayer()->GetCid();
		else
			FlagCarrierRed = FLAG_TAKEN;
	}

	int FlagCarrierBlue = FLAG_MISSING;
	if(m_apFlags[TEAM_BLUE])
	{
		if(m_apFlags[TEAM_BLUE]->m_AtStand)
			FlagCarrierBlue = FLAG_ATSTAND;
		else if(m_apFlags[TEAM_BLUE]->GetCarrier() && m_apFlags[TEAM_BLUE]->GetCarrier()->GetPlayer())
			FlagCarrierBlue = m_apFlags[TEAM_BLUE]->GetCarrier()->GetPlayer()->GetCid();
		else
			FlagCarrierBlue = FLAG_TAKEN;
	}

	if(Server()->IsSixup(SnappingClient))
	{
		protocol7::CNetObj_GameDataFlag *pGameDataObj = static_cast<protocol7::CNetObj_GameDataFlag *>(Server()->SnapNewItem(-protocol7::NETOBJTYPE_GAMEDATAFLAG, 0, sizeof(protocol7::CNetObj_GameDataFlag)));
		if(!pGameDataObj)
			return;

		pGameDataObj->m_FlagCarrierRed = FlagCarrierRed;
		pGameDataObj->m_FlagCarrierBlue = FlagCarrierBlue;
	}
	else
	{
		CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
		if(!pGameDataObj)
			return;

		pGameDataObj->m_FlagCarrierRed = FlagCarrierRed;
		pGameDataObj->m_FlagCarrierBlue = FlagCarrierBlue;
	}
}

void CGameControllerDDRace::HandleCharacterTilesDDPP(class CCharacter *pChr, int m_TileIndex, int m_TileFIndex, int Tile1, int Tile2, int Tile3, int Tile4, int FTile1, int FTile2, int FTile3, int FTile4, int PlayerDDRaceState)
{
	int ClientId = pChr->GetPlayer()->GetCid();
	// start
	if(((m_TileIndex == TILE_START) || (m_TileFIndex == TILE_START) || FTile1 == TILE_START || FTile2 == TILE_START || FTile3 == TILE_START || FTile4 == TILE_START || Tile1 == TILE_START || Tile2 == TILE_START || Tile3 == TILE_START || Tile4 == TILE_START) && (PlayerDDRaceState == DDRACE_NONE || PlayerDDRaceState == DDRACE_FINISHED || (PlayerDDRaceState == DDRACE_STARTED && !GameServer()->GetDDRaceTeam(ClientId) && g_Config.m_SvTeam != 3)))
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
	if(((m_TileIndex == TILE_FINISH) || (m_TileFIndex == TILE_FINISH) || FTile1 == TILE_FINISH || FTile2 == TILE_FINISH || FTile3 == TILE_FINISH || FTile4 == TILE_FINISH || Tile1 == TILE_FINISH || Tile2 == TILE_FINISH || Tile3 == TILE_FINISH || Tile4 == TILE_FINISH) && PlayerDDRaceState == DDRACE_STARTED)
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
