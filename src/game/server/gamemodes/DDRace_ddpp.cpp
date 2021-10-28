/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
/* Based on Race mod stuff and tweaked by GreYFoX@GTi and others to fit our DDRace needs. */
#include "DDRace.h"
#include <engine/server.h>
#include <engine/shared/config.h>
#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/entities/flag.h>
#include <game/server/gamecontext.h>

bool CGameControllerDDRace::OnEntity(int Index, vec2 Pos, int Layer, int Flags, int Number)
{
	IGameController::OnEntity(Index, Pos, Layer, Flags, Number);

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

int CGameControllerDDRace::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int WeaponID)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, WeaponID);
	int HadFlag = 0;

	// drop flags
	for(int i = 0; i < 2; i++)
	{
		CFlag *F = m_apFlags[i];
		if(F && pKiller && pKiller->GetCharacter() && F->m_pCarryingCharacter == pKiller->GetCharacter())
			HadFlag |= 2;
		if(F && F->m_pCarryingCharacter == pVictim)
		{
			if(g_Config.m_SvFlagSounds)
			{
				GameServer()->CreateSoundGlobal(SOUND_CTF_DROP);
			}
			/*pVictim->GetPlayer()->m_Rainbow = false;
			pVictim->GetPlayer()->m_TeeInfos.m_ColorBody = pVictim->GetPlayer()->m_ColorBodyOld;
			pVictim->GetPlayer()->m_TeeInfos.m_ColorFeet = pVictim->GetPlayer()->m_ColorFeetOld;*/
			F->m_DropTick = Server()->Tick();
			F->m_pCarryingCharacter = 0;
			F->m_Vel = vec2(0, 0);

			HadFlag |= 1;
		}
		if(F && F->m_pLastCarryingCharacter == pVictim)
			F->m_pLastCarryingCharacter = 0;
	}

	return HadFlag;
}

void CGameControllerDDRace::ChangeFlagOwner(int id, int character)
{
	CFlag *F = m_apFlags[id];
	if((m_apFlags[0] && m_apFlags[0]->m_pCarryingCharacter == GameServer()->GetPlayerChar(character)) || (m_apFlags[1] && m_apFlags[1]->m_pCarryingCharacter == GameServer()->GetPlayerChar(character)))
	{
	}
	else
	{
		F->m_AtStand = 0;

		if(g_Config.m_SvFlagSounds)
		{
			GameServer()->CreateSoundGlobal(SOUND_CTF_DROP);
		}
		/*
		F->m_pCarryingCharacter->GetPlayer()->m_Rainbow = false;
		F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorBody = F->m_pCarryingCharacter->GetPlayer()->m_ColorBodyOld;
		F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorFeet = F->m_pCarryingCharacter->GetPlayer()->m_ColorFeetOld;*/

		if(GameServer()->m_apPlayers[character] && GameServer()->m_apPlayers[character]->GetCharacter()) //ChillerDragon's crashbug protection //didnt understand the bug didnt test the portection better comment it out //uncommented agian yolo
		{
			F->m_pCarryingCharacter = GameServer()->m_apPlayers[character]->GetCharacter();
			/*
			if (!F->m_pCarryingCharacter->GetPlayer()->m_Rainbow){
			F->m_pCarryingCharacter->GetPlayer()->m_ColorBodyOld = F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorBody;
			F->m_pCarryingCharacter->GetPlayer()->m_ColorFeetOld = F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorFeet;
			}
			F->m_pCarryingCharacter->GetPlayer()->m_Rainbow = RAINBOW_BLACKWHITE;*/
			F->m_pCarryingCharacter->GetPlayer()->GetCharacter()->m_FirstFreezeTick = 0;
		}
	}
}

int CGameControllerDDRace::HasFlag(CCharacter *character)
{
	for(int i = 0; i < 2; i++)
	{
		if(!m_apFlags[i])
			continue;
		if(m_apFlags[i]->m_pCarryingCharacter == character)
		{
			return i;
		}
	}
	return -1;
}

void CGameControllerDDRace::DropFlag(int id, int dir)
{
	CFlag *F = m_apFlags[id]; //red=0 blue=1

	if(g_Config.m_SvFlagSounds)
	{
		GameServer()->CreateSoundGlobal(SOUND_CTF_DROP);
	}
	/*F->m_pCarryingCharacter->GetPlayer()->m_Rainbow = false;
	F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorBody = F->m_pCarryingCharacter->GetPlayer()->m_ColorBodyOld;
	F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorFeet = F->m_pCarryingCharacter->GetPlayer()->m_ColorFeetOld;*/
	F->m_pCarryingCharacter->GetPlayer()->m_ChangeTeamOnFlag = true;
	F->m_DropTick = Server()->Tick();
	F->m_DropFreezeTick = Server()->Tick();
	F->m_pLastCarryingCharacter = F->m_pCarryingCharacter;
	F->m_pCarryingCharacter = 0;
	F->m_Vel = vec2(5 * dir, -5);
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
			FlagCarrierRed = m_apFlags[TEAM_RED]->GetCarrier()->GetPlayer()->GetCID();
		else
			FlagCarrierRed = FLAG_TAKEN;
	}

	int FlagCarrierBlue = FLAG_MISSING;
	if(m_apFlags[TEAM_BLUE])
	{
		if(m_apFlags[TEAM_BLUE]->m_AtStand)
			FlagCarrierBlue = FLAG_ATSTAND;
		else if(m_apFlags[TEAM_BLUE]->GetCarrier() && m_apFlags[TEAM_BLUE]->GetCarrier()->GetPlayer())
			FlagCarrierBlue = m_apFlags[TEAM_BLUE]->GetCarrier()->GetPlayer()->GetCID();
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


void CGameControllerDDRace::HandleCharacterTilesDDPP(class CCharacter *pChr, int m_TileIndex, int m_TileFIndex, int Tile1, int Tile2, int Tile3, int Tile4, int FTile1, int FTile2, int FTile3, int FTile4)
{
    int ClientID = pChr->GetPlayer()->GetCID();
	const int PlayerDDRaceState = pChr->m_DDRaceState;
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
		if(pChr->GetPlayer()->m_QuestState == CPlayer::QUEST_RACE)
		{
			if(pChr->GetPlayer()->m_QuestStateLevel == 5)
			{
				if(HasFlag(pChr) != -1) //has flag
				{
					GameServer()->QuestCompleted(pChr->GetPlayer()->GetCID());
				}
				else
				{
					GameServer()->QuestFailed(pChr->GetPlayer()->GetCID());
				}
			}
			else if(pChr->GetPlayer()->m_QuestStateLevel == 9)
			{
				if(!pChr->GetPlayer()->m_QuestFailed)
				{
					GameServer()->QuestCompleted(pChr->GetPlayer()->GetCID());
				}
			}
		}

		pChr->m_DummyFinished = true;
		pChr->m_DummyFinishes++;

		/*
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "xp [%d/1000]", pChr->GetPlayer()->GetXP());
		GameServer()->SendBroadcast(aBuf, pChr->GetPlayer()->GetCID(), 0);
		*/
    }
}

