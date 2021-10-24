/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
/* Based on Race mod stuff and tweaked by GreYFoX@GTi and others to fit our DDRace needs. */
#include "DDRace.h"
#include "gamemode.h"
#include <engine/server.h>
#include <engine/shared/config.h>
#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/entities/flag.h>
#include <game/server/gamecontext.h>


void CGameControllerDDRace::HandleCharacterTilesDDPP(class CCharacter *pChr, int Tile1, int Tile2, int Tile3, int Tile4, int FTile1, int FTile2, int FTile3, int FTile4)
{
    // start
	if(((m_TileIndex == TILE_START) || (m_TileFIndex == TILE_START) || FTile1 == TILE_START || FTile2 == TILE_START || FTile3 == TILE_START || FTile4 == TILE_START || Tile1 == TILE_START || Tile2 == TILE_START || Tile3 == TILE_START || Tile4 == TILE_START) && (PlayerDDRaceState == DDRACE_NONE || PlayerDDRaceState == DDRACE_FINISHED || (PlayerDDRaceState == DDRACE_STARTED && !GetPlayerTeam(ClientID) && g_Config.m_SvTeam != 3)))
    {
		pChr->GetPlayer()->m_MoneyTilePlus = true;
		if(pChr->GetPlayer()->m_QuestState == CPlayer::QUEST_RACE)
		{
			if((pChr->GetPlayer()->m_QuestStateLevel == 3 || pChr->GetPlayer()->m_QuestStateLevel == 8) && pChr->GetPlayer()->m_QuestProgressValue)
			{
				GameServer()->QuestAddProgress(pChr->GetPlayer()->GetCID(), 2);
			}
			else if(pChr->GetPlayer()->m_QuestStateLevel == 9 && pChr->GetPlayer()->m_QuestFailed)
			{
				// GameServer()->SendChatTarget(pChr->GetPlayer()->GetCID(), "[QUEST] running agian.");
				pChr->GetPlayer()->m_QuestFailed = false;
			}
		}
		pChr->m_DDPP_Finished = false;
    }
    // finish
	if(((m_TileIndex == TILE_FINISH) || (m_TileFIndex == TILE_FINISH) || FTile1 == TILE_FINISH || FTile2 == TILE_FINISH || FTile3 == TILE_FINISH || FTile4 == TILE_FINISH || Tile1 == TILE_FINISH || Tile2 == TILE_FINISH || Tile3 == TILE_FINISH || Tile4 == TILE_FINISH) && PlayerDDRaceState == DDRACE_STARTED)
    {
        #pragma message "TODO what is this teams finish here? should be uncommented"
		// Controller->m_Teams.OnCharacterFinish(m_pPlayer->GetCID()); // Quest 3 lvl 0-4 is handled in here teams.cpp
		if(m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
		{
			if(m_pPlayer->m_QuestStateLevel == 5)
			{
				if(((CGameControllerDDRace *)GameServer()->m_pController)->HasFlag(this) != -1) //has flag
				{
					GameServer()->QuestCompleted(m_pPlayer->GetCID());
				}
				else
				{
					GameServer()->QuestFailed(m_pPlayer->GetCID());
				}
			}
			else if(m_pPlayer->m_QuestStateLevel == 9)
			{
				if(!m_pPlayer->m_QuestFailed)
				{
					GameServer()->QuestCompleted(m_pPlayer->GetCID());
				}
			}
		}

		m_DummyFinished = true;
		m_DummyFinishes++;

		/*
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "xp [%d/1000]", m_pPlayer->GetXP());
		GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
		*/
    }
}

