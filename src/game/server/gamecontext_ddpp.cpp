// gamecontext scoped ddnet++ methods

#include <base/ddpp_logs.h>
#include <base/system_ddpp.h>
#include <cstring>
#include <engine/shared/config.h>
#include <fstream> //acc2 sys
#include <game/server/teams.h>
#include <limits> //acc2 sys

#include "save.h"

#include "gamecontext.h"

bool CGameContext::CheckAccounts(int AccountID)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i])
			continue;

		if(m_apPlayers[i]->GetAccID() == AccountID)
			return true;
	}
	return false;
}

int CGameContext::GetNextClientID()
{
	int ClientID = -1;
	for(int i = 0; i < g_Config.m_SvMaxClients; i++)
	{
		if(m_apPlayers[i])
			continue;

		ClientID = i;
		break;
	}

	return ClientID;
}

void CGameContext::OnStartBlockTournament()
{
	if(m_BlockTournaState)
	{
		SendChat(-1, CGameContext::CHAT_ALL, "[EVENT] error tournament already running.");
		return;
	}
	if(g_Config.m_SvAllowBlockTourna == 0)
	{
		SendChat(-1, CGameContext::CHAT_ALL, "[EVENT] error tournaments are deactivated by an admin.");
		return;
	}

	m_BlockTournaState = 1;
	m_BlockTournaLobbyTick = g_Config.m_SvBlockTournaDelay * Server()->TickSpeed();
}

//void CGameContext::OnDDPPshutdown()
//{
//#if defined(CONF_DEBUG)
//#endif
//	SendChat(-1, CGameContext::CHAT_ALL, "[DDNet++] server shutdown!");
//}

void CGameContext::AbuseMotd(const char *pMsg, int ClientID)
{
	if(m_apPlayers[ClientID])
	{
		m_apPlayers[ClientID]->m_IsFakeMotd = true;
	}
	// send motd
	CNetMsg_Sv_Motd Msg;
	Msg.m_pMessage = pMsg;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

bool CGameContext::IsDDPPgametype(const char *pGametype)
{
	return !str_comp_nocase(g_Config.m_SvDDPPgametype, pGametype);
}

int CGameContext::GetCIDByName(const char *pName)
{
	int nameID = -1;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			if(!str_comp(pName, Server()->ClientName(i)))
			{
				nameID = i;
				break;
			}
		}
	}
	return nameID;
}

int CGameContext::GetShopBot()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			if(m_apPlayers[i]->m_DummyMode == 99)
			{
				return i;
			}
		}
	}
	return -1;
}

int CGameContext::CountConnectedPlayers()
{
	int cPlayers = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			cPlayers++;
		}
	}
	//char aBuf[64];
	//str_format(aBuf, sizeof(aBuf), "counted %d players", i);
	//dbg_msg("count", aBuf);
	return cPlayers;
}

int CGameContext::CountConnectedHumans()
{
	int cPlayers = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && !m_apPlayers[i]->m_IsDummy)
		{
			cPlayers++;
		}
	}
	return cPlayers;
}

int CGameContext::CountIngameHumans()
{
	int cPlayers = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->GetCharacter() && !m_apPlayers[i]->m_IsDummy)
		{
			cPlayers++;
		}
	}
	return cPlayers;
}

bool CGameContext::IsAllowedCharSet(const char *pStr)
{
#if defined(CONF_DEBUG)
#endif
	int i = 0;
	bool IsOk = false;
	//dbg_msg("AllowedChar", "checking str '%s'", pStr);

	while(true)
	{
		IsOk = false;
		for(int j = 0; j < str_length(m_aAllowedCharSet); j++)
		{
			if(pStr[i] == m_aAllowedCharSet[j])
			{
				//dbg_msg("AllowedChar","found valid char '%c' - '%c'", pStr[i], m_aAllowedCharSet[j]);
				IsOk = true;
				break;
			}
		}

		if(!IsOk)
		{
			//dbg_msg("AllowedChar", "found evil char '%c'", pStr[i]);
			return false;
		}
		i++;
		if(pStr[i] == '\0')
		{
			//dbg_msg("AllowedChar", "string ends at %d", i);
			return true;
		}
	}
	return true;
}

int CGameContext::GetPlayerByTimeoutcode(const char *pTimeout)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i])
			continue;
		if(!m_apPlayers[i]->m_TimeoutCode[0])
			continue;
		if(str_comp(m_apPlayers[i]->m_TimeoutCode, pTimeout))
			continue;
		return i;
	}
	return -1;
}

int CGameContext::CountConnectedBots()
{
	int lum_tt_zv_1_zz_04032018_lt3 = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_IsDummy)
		{
			lum_tt_zv_1_zz_04032018_lt3++;
		}
	}
	return lum_tt_zv_1_zz_04032018_lt3;
}

int CGameContext::CountTimeoutCodePlayers()
{
	int p = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i])
			continue;
		if(!m_apPlayers[i]->m_TimeoutCode[0])
			continue;
		p++;
	}
	return p;
}

void CGameContext::SendBroadcastAll(const char *pText, int importance, bool supermod)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			SendBroadcast(pText, m_apPlayers[i]->GetCID(), importance, supermod);
		}
	}
}

void CGameContext::KillAll()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->GetCharacter()) //only kill alive dudes
		{
			GetPlayerChar(i)->Die(i, WEAPON_WORLD);
		}
	}
}

void CGameContext::LoadFNNvalues()
{
	std::ifstream readfile;
	char aFilePath[512];
	str_format(aFilePath, sizeof(aFilePath), "FNN/move_stats.fnn");
	readfile.open(aFilePath);
	if(readfile.is_open())
	{
		std::string line;

		std::getline(readfile, line); //distance
		m_FNN_best_distance = atoi(line.c_str());

		std::getline(readfile, line); //fitness
		m_FNN_best_fitness = atoi(line.c_str());

		std::getline(readfile, line); //distance_finish
		m_FNN_best_distance_finish = atoi(line.c_str());
	}
	else
	{
		m_FNN_best_distance = -9999999;
		m_FNN_best_fitness = -9999999;
		m_FNN_best_distance_finish = 9999999;
		dbg_msg("FNN", "LoadFNNvalues() error failed to load best stats. failed to open '%s'", aFilePath);
	}
}

void CGameContext::SQLPortLogout(int port)
{
	ExecuteSQLf("UPDATE `Accounts` SET `IsLoggedIn` = '%i' WHERE `LastLoginPort` = '%i'", 0, g_Config.m_SvPort);
}

bool CGameContext::IsPosition(int playerID, int pos)
{
#if defined(CONF_DEBUG)
	//dbg_msg("debug", "IsPosition(playerID = %d, pos = %d)", playerID, pos);
#endif
	if(!m_apPlayers[playerID])
	{
		return false;
	}
	if(!GetPlayerChar(playerID))
	{
		return false;
	}

	if(pos == 0) //cb5 jail release spot
	{
		if(GetPlayerChar(playerID)->m_Pos.x > 480 * 32 && GetPlayerChar(playerID)->m_Pos.x < 500 * 32 && GetPlayerChar(playerID)->m_Pos.y > 229 * 32 && GetPlayerChar(playerID)->m_Pos.y < 237 * 32)
		{
			return true;
		}
	}
	//else if (pos == 1) //cb5 spawn
	//{
	//	if (GetPlayerChar(playerID)->m_Pos.x > 325 * 32
	//		&& GetPlayerChar(playerID)->m_Pos.x < 362 * 32
	//		&& GetPlayerChar(playerID)->m_Pos.y > 191 * 32
	//		&& GetPlayerChar(playerID)->m_Pos.y < 206 * 32)
	//	{
	//		return true;
	//	}
	//}
	else if(pos == 2) //cb5 far in map (block area and race)
	{
		if(GetPlayerChar(playerID)->m_Pos.x > 415 * 32)
		{
			return true;
		}
	}
	else if(pos == 3) //configurated spawn area
	{
		if(GetPlayerChar(playerID)->m_Pos.x > g_Config.m_SvSpawnareaLowX * 32 && GetPlayerChar(playerID)->m_Pos.x < g_Config.m_SvSpawnareaHighX * 32 && GetPlayerChar(playerID)->m_Pos.y > g_Config.m_SvSpawnareaLowY * 32 && GetPlayerChar(playerID)->m_Pos.y < g_Config.m_SvSpawnareaHighY * 32)
		{
			return true;
		}
	}

	return false;
}

void CGameContext::StartAsciiAnimation(int viewerID, int creatorID, int medium)
{
	if(!m_apPlayers[viewerID])
		return;
	if(!m_apPlayers[creatorID])
	{
		SendChatTarget(viewerID, "player not found.");
		return;
	}
	//dont start new animation while old is running
	if(m_apPlayers[viewerID]->m_AsciiWatchingID != -1)
	{
		return;
	}

	if(medium == 0) // '/ascii view <cid>'
	{
		if(m_apPlayers[creatorID]->m_aAsciiPublishState[0] == '0')
		{
			SendChatTarget(viewerID, "ascii art not public.");
			return;
		}

		m_apPlayers[creatorID]->m_AsciiViewsDefault++;
		//COULDDO: code: cfv45
	}
	else if(medium == 1) // '/profile view <player>'
	{
		if(m_apPlayers[creatorID]->m_aAsciiPublishState[1] == '0')
		{
			//SendChatTarget(viewerID, "ascii art not published on profile");
			return;
		}

		m_apPlayers[creatorID]->m_AsciiViewsProfile++;
	}
	else if(medium == 2) // not used yet
	{
		if(m_apPlayers[creatorID]->m_aAsciiPublishState[2] == '0')
		{
			SendChatTarget(viewerID, "ascii art not published on medium 2");
			return;
		}
	}
	else if(medium == 3) // not used yet
	{
		if(m_apPlayers[creatorID]->m_aAsciiPublishState[3] == '0')
		{
			SendChatTarget(viewerID, "ascii art not published on medium 3");
			return;
		}
	}

	m_apPlayers[viewerID]->m_AsciiWatchingID = creatorID;
}

bool CGameContext::IsHooked(int hookedID, int power)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		CCharacter *pChar = GetPlayerChar(i);

		if(!pChar || !pChar->IsAlive() || pChar->GetPlayer()->GetCID() == hookedID)
			continue;
		if(pChar->Core()->m_HookedPlayer == hookedID && pChar->GetPlayer()->m_HookPower == power)
		{
			return true;
		}
	}

	return false;
}

bool CGameContext::IsSameIP(int ID_1, int ID_2)
{
	char aIP_1[64];
	char aIP_2[64];
	Server()->GetClientAddr(ID_1, aIP_1, sizeof(aIP_1));
	Server()->GetClientAddr(ID_2, aIP_2, sizeof(aIP_2));
	if(!str_comp_nocase(aIP_1, aIP_2))
	{
		//dbg_msg("IP_CHECKER", "[%s] [%s]  EQUAL", aIP_1, aIP_2);
		return true;
	}
	//dbg_msg("IP_CHECKER", "[%s] [%s] UNQUAL", aIP_1, aIP_2);
	return false;
}

char CGameContext::BoolToChar(bool b)
{
	if(b)
		return '1';
	return '0';
}

bool CGameContext::CharToBool(char c)
{
	if(c == '0')
		return false;
	return true;
}

void CGameContext::ShowHideConfigBoolToChar(int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return;
	//[0] = blockpoints [1] = blockxp [2] = xp [3] = jail [4] = instafeed(1n1) [5] = questprogress [6] = questwarning
	pPlayer->m_aShowHideConfig[0] = BoolToChar(pPlayer->m_ShowBlockPoints);
	pPlayer->m_aShowHideConfig[1] = BoolToChar(pPlayer->m_HideBlockXp);
	pPlayer->m_aShowHideConfig[2] = BoolToChar(pPlayer->m_xpmsg);
	pPlayer->m_aShowHideConfig[3] = BoolToChar(pPlayer->m_hidejailmsg);
	pPlayer->m_aShowHideConfig[4] = BoolToChar(pPlayer->m_HideInsta1on1_killmessages);
	pPlayer->m_aShowHideConfig[5] = BoolToChar(pPlayer->m_HideQuestProgress);
	pPlayer->m_aShowHideConfig[6] = BoolToChar(pPlayer->m_HideQuestWarning);
	pPlayer->m_aShowHideConfig[7] = '\0';
#if defined(CONF_DEBUG)
	//dbg_msg("BoolToChar", "UPDATED ShowHideChar='%s'", pPlayer->m_aShowHideConfig);
#endif
}

void CGameContext::ShowHideConfigCharToBool(int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return;
	//[0] = blockpoints [1] = blockxp [2] = xp [3] = jail [4] = instafeed(1n1) [5] = questprogress [6] = questwarning
	pPlayer->m_ShowBlockPoints = CharToBool(pPlayer->m_aShowHideConfig[0]);
	pPlayer->m_HideBlockXp = CharToBool(pPlayer->m_aShowHideConfig[1]);
	pPlayer->m_xpmsg = CharToBool(pPlayer->m_aShowHideConfig[2]);
	pPlayer->m_hidejailmsg = CharToBool(pPlayer->m_aShowHideConfig[3]);
	pPlayer->m_HideInsta1on1_killmessages = CharToBool(pPlayer->m_aShowHideConfig[4]);
	pPlayer->m_HideQuestProgress = CharToBool(pPlayer->m_aShowHideConfig[5]);
	pPlayer->m_HideQuestWarning = CharToBool(pPlayer->m_aShowHideConfig[6]);
#if defined(CONF_DEBUG)
	/*
	dbg_msg("CharToBool", "ShowHideChar='%s'", pPlayer->m_aShowHideConfig);
	dbg_msg("ShowHide", "BlockPoints	: %d", pPlayer->m_ShowBlockPoints);
	dbg_msg("ShowHide", "BlockXp		: %d", pPlayer->m_HideBlockXp);
	dbg_msg("ShowHide", "Xp				: %d", pPlayer->m_xpmsg);
	dbg_msg("ShowHide", "Jail			: %d", pPlayer->m_hidejailmsg);
	dbg_msg("ShowHide", "insta1n1		: %d", pPlayer->m_HideInsta1on1_killmessages);
	dbg_msg("ShowHide", "questprogress	: %d", pPlayer->m_HideQuestProgress);
	dbg_msg("ShowHide", "questwarning	: %d", pPlayer->m_HideQuestWarning);
	*/
#endif
}

void CGameContext::FNN_LoadRun(const char *path, int botID)
{
	CPlayer *pPlayer = m_apPlayers[botID];
	if(!pPlayer)
	{
		dbg_msg("FNN", "failed to load run player with id=%d doesn't exist", botID);
		return;
	}
	CCharacter *pChr = GetPlayerChar(botID);
	if(!pChr)
	{
		dbg_msg("FNN", "failed to load run character with id=%d, name=%s doesn't exist", botID, Server()->ClientName(botID));
		return;
	}

	//reset values
	pChr->m_FNN_CurrentMoveIndex = 0;
	float loaded_distance = 0;
	float loaded_fitness = 0;
	float loaded_distance_finish = 0;
	char aBuf[128];

	//load run
	std::ifstream readfile;
	char aFilePath[512];
	str_copy(aFilePath, path, sizeof(aFilePath));
	readfile.open(aFilePath);
	if(readfile.is_open())
	{
		std::string line;
		int i = 0;

		//first four five are stats:
		std::getline(readfile, line); // read but ignore header

		std::getline(readfile, line); //moveticks
		pChr->m_FNN_ticks_loaded_run = atoi(line.c_str());

		std::getline(readfile, line); //distance
		loaded_distance = atof(line.c_str());

		std::getline(readfile, line); //fitness
		loaded_fitness = atof(line.c_str());

		std::getline(readfile, line); //distance_finish
		loaded_distance_finish = atof(line.c_str());

		while(std::getline(readfile, line))
		{
			pChr->m_aRecMove[i] = atoi(line.c_str());
			i++;
		}
	}
	else
	{
		dbg_msg("FNN", "failed to load move. failed to open '%s'", aFilePath);
		pPlayer->m_dmm25 = -1;
	}

	//start run
	pPlayer->m_dmm25 = 4; //replay submode
	str_format(aBuf, sizeof(aBuf), "[FNN] loaded run with ticks=%d distance=%.2f fitness=%.2f distance_finish=%.2f", pChr->m_FNN_ticks_loaded_run, loaded_distance, loaded_fitness, loaded_distance_finish);
	SendChat(botID, CGameContext::CHAT_ALL, aBuf);
}

void CGameContext::TestPrintTiles(int botID)
{
	CPlayer *pPlayer = m_apPlayers[botID];
	if(!pPlayer)
		return;
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	//moved to character
}

vec2 CGameContext::GetFinishTile()
{
	/*
	int BIG_NUMBER = 0;
	BIG_NUMBER = ~BIG_NUMBER; //binary lyfe hacks with chiller
	int TODO = BIG_NUMBER; //find a better way maybe actual map index size or something

	for (int i = 0; i < TODO; i++)
	{
		if (Collision()->TileExists(i))
		{
			if (Collision()->GetTileIndex(i) == TILE_END)
			{
				dbg_msg("tile-finder","found finish tile at index=%d",i);
				return i;
			}
		}
	}
	*/

	/*
	for (int i = 0; i < Collision()->GetWidth() * Collision()->GetHeight(); i++)
	{
		if (GameServer()->m_pTiles[i].m_Index == TILE_END)
		{
			dbg_msg("tile-finder", "found finish tile at index=%d", i);
			return i;
		}
	}
	*/

	/*
	int Width = Collision()->GetWidth();
	int Height = Collision()->GetHeight();
	for (int i = 0; i < Width; i++)
	{
		for (int j = 0; j < Height; i++)
		{
			if (Collision()->GetTile(i, j) == TILE_END || Collision()->GetFTile(i, j) == TILE_END)
			{
				dbg_msg("tile-finder", "found finish tile at (%d/%d)", i,j);
				return vec2(i, j);
			}
		}
	}
	*/

	int Width = Collision()->GetWidth();
	int Height = Collision()->GetHeight();
	for(int i = 0; i < Width * Height; i++)
	{
		if(Collision()->GetTileIndex(i) == TILE_FINISH || Collision()->GetFTileIndex(i) == TILE_FINISH)
		{
			/*
			dbg_msg("tile-finder", "found finish tile at index=%d", i);
			dbg_msg("tile-finder", "height: %d", Height);
			dbg_msg("tile-finder", "width: %d", Width);
			dbg_msg("tile-finder", "x: %d", i % Width);
			dbg_msg("tile-finder", "y: %d", int(i / Width));
			*/
			return vec2(i % Width, int(i / Width));
		}
	}

	return vec2(0, 0);
}

void CGameContext::ShowInstaStats(int requestID, int requestedID)
{
	if(!m_apPlayers[requestID])
		return;
	CPlayer *pPlayer = m_apPlayers[requestedID];
	if(!pPlayer)
		return;

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "~~~ '%s's Grenade instagib ~~~", Server()->ClientName(pPlayer->GetCID()));
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Kills: %d", pPlayer->m_GrenadeKills);
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Deaths: %d", pPlayer->m_GrenadeDeaths);
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Highest spree: %d", pPlayer->m_GrenadeSpree);
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Total shots: %d", pPlayer->m_GrenadeShots);
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Shots without RJ: %d", pPlayer->m_GrenadeShotsNoRJ);
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Rocketjumps: %d", pPlayer->m_GrenadeShots - pPlayer->m_GrenadeShotsNoRJ);
	SendChatTarget(requestID, aBuf);
	//str_format(aBuf, sizeof(aBuf), "Failed shots (no kill, no rj): %d", pPlayer->m_GrenadeShots - (pPlayer->m_GrenadeShots - pPlayer->m_GrenadeShotsNoRJ) - pPlayer->m_GrenadeKills); //can be negative with double and tripple kills but this isnt a bug its a feature xd
	//SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "~~~ '%s's Rifle instagib ~~~", Server()->ClientName(pPlayer->GetCID()));
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Kills: %d", pPlayer->m_RifleKills);
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Deaths: %d", pPlayer->m_RifleDeaths);
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Highest spree: %d", pPlayer->m_RifleSpree);
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Total shots: %d", pPlayer->m_RifleShots);
	SendChatTarget(requestID, aBuf);
}

void CGameContext::ShowSurvivalStats(int requestID, int requestedID)
{
	if(!m_apPlayers[requestID])
		return;
	CPlayer *pPlayer = m_apPlayers[requestedID];
	if(!pPlayer)
		return;

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "~~~ '%s's survival stats ~~~", Server()->ClientName(pPlayer->GetCID()));
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Kills: %d", pPlayer->m_SurvivalKills);
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Deaths: %d", pPlayer->m_SurvivalDeaths);
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Wins: %d", pPlayer->m_SurvivalWins);
	SendChatTarget(requestID, aBuf);
}

void CGameContext::ShowDDPPStats(int requestID, int requestedID)
{
	if(!m_apPlayers[requestID])
		return;
	CPlayer *pPlayer = m_apPlayers[requestedID];
	if(!pPlayer)
		return;

	char aBuf[128];

	str_format(aBuf, sizeof(aBuf), "--- %s's Stats ---", Server()->ClientName(requestedID));
	SendChatTarget(requestID, aBuf);
	if(pPlayer->GetLevel() == ACC_MAX_LEVEL)
		str_format(aBuf, sizeof(aBuf), "Level[%d] ( MAX LEVEL ! )", pPlayer->GetLevel());
	else
		str_format(aBuf, sizeof(aBuf), "Level[%d]", pPlayer->GetLevel());
	SendChatTarget(requestID, aBuf);
	if(!pPlayer->IsLoggedIn())
		str_format(aBuf, sizeof(aBuf), "Xp[%llu] (not logged in)", pPlayer->GetXP());
	else
		str_format(aBuf, sizeof(aBuf), "Xp[%llu/%llu]", pPlayer->GetXP(), pPlayer->GetNeededXP());
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Money[%llu]", pPlayer->GetMoney());
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "PvP-Arena Tickets[%d]", pPlayer->m_pvp_arena_tickets);
	SendChatTarget(requestID, aBuf);
	SendChatTarget(requestID, "---- BLOCK ----");
	str_format(aBuf, sizeof(aBuf), "Points: %d", pPlayer->m_BlockPoints);
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Kills: %d", pPlayer->m_BlockPoints_Kills);
	SendChatTarget(requestID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Deaths: %d", pPlayer->m_BlockPoints_Deaths);
	SendChatTarget(requestID, aBuf);

	// str_format(aBuf, sizeof(aBuf), "Skillgroup: %s", GetBlockSkillGroup(StatsID));
	// SendChatTarget(requestID, aBuf);
}

bool CGameContext::ChillWriteToLine(char const *filename, unsigned lineNo, char const *data)
{
	std::fstream file(filename);
	if(!file)
		return false;

	unsigned currentLine = 0;
	while(currentLine < lineNo)
	{
		// We don't actually care about the lines we're reading,
		// so just discard them.
		file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		++currentLine;
	}

	// Position the put pointer -- switching from reading to writing.
	file.seekp(file.tellg());

	dbg_msg("acc2", "writing [%s] to line [%d]", data, currentLine);

	//return file << data; //doesnt compile with MinGW
	return false;
}

int CGameContext::ChillUpdateFileAcc(const char *account, unsigned int line, const char *value, int requestingID)
{
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "%s/%s.acc", g_Config.m_SvFileAccPath, account);
	std::fstream Acc2File(aBuf);

	if(!std::ifstream(aBuf))
	{
		SendChatTarget(requestingID, "[ACCOUNT] username not found.");
		Acc2File.close();
		return -1; //return error code -1
	}

	std::string data[32];
	int index = 0;

	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] password: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] loggedin: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] port: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] frozen: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] vip: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] vip+: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] sup: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] money: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] level: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] xp: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] shit: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] police: '%s'", index, data[index].c_str());
	index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] taser: '%s'", index, data[index].c_str());
	index++;

	if(data[1] == "1")
	{
		str_format(aBuf, sizeof(aBuf), "[ACC2] '%s' is logged in on port '%s'", account, data[2].c_str());
		SendChatTarget(requestingID, aBuf);
		Acc2File.close();
		return -2;
	}

	if(line != 3 && data[3] == "1") //only can update the frozen value if acc is frozen
	{
		str_format(aBuf, sizeof(aBuf), "[ACC2] '%s' is frozen cant set line '%d'", account, line);
		SendChatTarget(requestingID, aBuf);
		Acc2File.close();
		return -3;
	}

	//===============
	//finish reading
	//start writing
	//===============

	//set new data
	data[line] = value;

	str_format(aBuf, sizeof(aBuf), "%s/%s.acc", g_Config.m_SvFileAccPath, account);
	std::ofstream Acc2FileW(aBuf);

	if(Acc2FileW.is_open())
	{
		dbg_msg("acc2", "write acc '%s'", account);
		index = 0;

		Acc2FileW << data[index++] << "\n"; //0 password
		Acc2FileW << data[index++] << "\n"; //1 loggedin
		Acc2FileW << data[index++] << "\n"; //2 port
		Acc2FileW << data[index++] << "\n"; //3 frozen
		Acc2FileW << data[index++] << "\n"; //4 vip
		Acc2FileW << data[index++] << "\n"; //5 vip+
		Acc2FileW << data[index++] << "\n"; //6 sup
		Acc2FileW << data[index++] << "\n"; //7 money
		Acc2FileW << data[index++] << "\n"; //8 level
		Acc2FileW << data[index++] << "\n"; //9 xp
		Acc2FileW << data[index++] << "\n"; //10 shit
		Acc2FileW << data[index++] << "\n"; //11 police
		Acc2FileW << data[index++] << "\n"; //12 taser

		Acc2FileW.close();
	}
	else
	{
		dbg_msg("acc2", "[WARNING] account '%s' (%s) failed to save", account, aBuf);
		Acc2FileW.close();
		return -4;
	}

	str_format(aBuf, sizeof(aBuf), "[ACC2] '%s' updated line [%d] to value [%s]", account, line, value);
	SendChatTarget(requestingID, aBuf);

	Acc2File.close();
	return 0; //all clean no errors --> return false
}

void CGameContext::ConnectFngBots(int amount, int mode)
{
	for(int i = 0; i < amount; i++)
	{
		if(mode == 0) //rifle
		{
			CreateNewDummy(-4);
		}
		else if(mode == 1) // grenade
		{
			CreateNewDummy(-5);
		}
		else
		{
			dbg_msg("WARNING", "ConnectFngBots() mode %d not valid.", mode);
			return;
		}
	}
}

void CGameContext::SaveCosmetics(int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return;
	CCharacter *pChr = m_apPlayers[id]->GetCharacter();
	if(!pChr)
		return;

	//backup cosmetics for lobby (save)
	pPlayer->m_IsBackupRainbow = pChr->m_Rainbow;
	pPlayer->m_IsBackupBloody = pChr->m_Bloody;
	pPlayer->m_IsBackupStrongBloody = pChr->m_StrongBloody;
	pPlayer->m_IsBackupAtom = pChr->m_Atom;
	pPlayer->m_IsBackupTrail = pChr->m_Trail;
	pPlayer->m_IsBackupAutospreadgun = pChr->m_autospreadgun;
	pPlayer->m_IsBackupWaveBloody = pChr->m_WaveBloody;
}

void CGameContext::LoadCosmetics(int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return;
	CCharacter *pChr = m_apPlayers[id]->GetCharacter();
	if(!pChr)
		return;

	//backup cosmetics for lobby (save)
	pChr->m_Rainbow = pPlayer->m_IsBackupRainbow;
	pChr->m_Bloody = pPlayer->m_IsBackupBloody;
	pChr->m_StrongBloody = pPlayer->m_IsBackupStrongBloody;
	pChr->m_Atom = pPlayer->m_IsBackupAtom;
	pChr->m_Trail = pPlayer->m_IsBackupTrail;
	pChr->m_autospreadgun = pPlayer->m_IsBackupAutospreadgun;
	pChr->m_WaveBloody = pPlayer->m_IsBackupWaveBloody;
}

void CGameContext::DeleteCosmetics(int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return;
	CCharacter *pChr = m_apPlayers[id]->GetCharacter();
	if(!pChr)
		return;

	pChr->m_Rainbow = false;
	pChr->m_Bloody = false;
	pChr->m_StrongBloody = false;
	pChr->m_Atom = false;
	pChr->m_Trail = false;
	pChr->m_autospreadgun = false;
	pChr->m_RandomCosmetics = false;
	pChr->m_WaveBloody = false;
	pChr->UnsetSpookyGhost();
}

void CGameContext::CheckDDPPshutdown()
{
	if(g_Config.m_SvDDPPshutdown)
	{
		int players = CountConnectedHumans();
		time_t now;
		struct tm *now_tm;
		int hour;
		int min;
		now = time(NULL);
		now_tm = localtime(&now);
		hour = now_tm->tm_hour;
		min = now_tm->tm_min;
		if(hour == g_Config.m_SvDDPPshutdownHour && (min == 0 || min == 5 || min == 10)) //Try it 3 times (slow tick shouldnt trigger it multiple times a minute)
		{
			if(players < g_Config.m_SvDDPPshutdownPlayers)
			{
				//SendChat(-1, CGameContext::CHAT_ALL, "[DDNet++] WARNING SERVER SHUTDOWN!");
				CallVote(-1, "shutdown server", "shutdown", "Update", "[DDNet++] do you want to update the server now?", 0, true);
			}
			else
			{
				SendChat(-1, CGameContext::CHAT_ALL, "[DDNet++] shutdown failed: too many players online.");
			}
		}
	}
}

void CGameContext::DDPP_Tick()
{
	if(m_iBroadcastDelay > 0)
	{
		m_iBroadcastDelay--;
	}

	if(m_BlockTournaState)
	{
		BlockTournaTick();
	}

	if(m_BalanceBattleState == 1)
	{
		BalanceBattleTick();
	}

	if(m_BombGameState)
	{
		BombTick();
	}

	if(m_survivalgamestate == 1)
	{
		SurvivalLobbyTick();
	}
	else
	{
		if(m_survival_game_countdown > 0)
		{
			m_survival_game_countdown--;
		}
		if(m_survival_game_countdown == 0)
		{
			SendSurvivalChat("[SURVIVAL] Game ended due to timeout. Nobody won.");
			str_copy(m_aLastSurvivalWinnerName, "", sizeof(m_aLastSurvivalWinnerName));
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(m_apPlayers[i] && m_apPlayers[i]->m_IsSurvivaling)
				{
					SetPlayerSurvival(i, SURVIVAL_LOBBY);
					if(m_apPlayers[i]->GetCharacter()) //only kill if isnt dead already or server crashes (he should respawn correctly anayways)
					{
						m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_GAME);
					}
				}
			}
			SurvivalSetGameState(SURVIVAL_LOBBY);
		}
		if(m_survivalgamestate == SURVIVAL_DM_COUNTDOWN)
		{
			SurvivalDeathmatchTick();
		}
	}

	if(m_BlockWaveGameState)
	{
		BlockWaveGameTick();
	}

	if(m_CreateShopBot && (Server()->Tick() % 50 == 0))
	{
		CreateNewDummy(99); //shop bot
		m_CreateShopBot = false;
	}
	for(int i = 0; i < MAX_CLIENTS; i++) //all the tick stuff which needs all players
	{
		if(!m_apPlayers[i])
			continue;

		if(m_LastAccountMode != g_Config.m_SvAccountStuff)
		{
			if(m_apPlayers[i]->IsLoggedIn())
			{
				SendChatTarget(i, "[ACCOUNT] you have been logged out due to changes in the system");
				m_apPlayers[i]->Logout();
			}
		}

		ChilliClanTick(i);
		AsciiTick(i);
		InstaGrenadeRoundEndTick(i);
		InstaRifleRoundEndTick(i);
		C3_MultiPlayer_GameTick(i);
	}
	if(m_InstaGrenadeRoundEndTickTicker)
	{
		m_InstaGrenadeRoundEndTickTicker--;
	}
	if(m_InstaRifleRoundEndTickTicker)
	{
		m_InstaRifleRoundEndTickTicker--;
	}
	m_LastAccountMode = g_Config.m_SvAccountStuff;

	if(Server()->Tick() % 600 == 0) //slow ddpp sub tick
	{
		DDPP_SlowTick();
	}
}

void CGameContext::LogoutAllPlayers()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i])
			continue;
		if(m_apPlayers[i]->IsLoggedIn())
		{
			dbg_msg("ddnet++", "logging out id=%d", i);
			m_apPlayers[i]->Logout();
		}
	}
}

void CGameContext::LogoutAllPlayersMessage()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i])
			continue;
		if(m_apPlayers[i]->IsLoggedIn())
		{
			dbg_msg("ddnet++", "logging out id=%d", i);
			m_apPlayers[i]->Logout();
			SendChatTarget(i, "[ACCOUNT] you were logged out.");
		}
	}
}

void CGameContext::ConnectAdventureBots()
{
	CreateNewDummy(CCharacter::DUMMYMODE_ADVENTURE, true, 1);
}

void CGameContext::DDPP_SlowTick()
{
	bool StopSurvival = true;
	int NumQuesting = 0;
	int TotalPlayers = 0;
	int NumAdventureBots = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i])
			continue;

		TotalPlayers++;
		CheckDeleteLoginBanEntry(i);
		CheckDeleteRegisterBanEntry(i);
		CheckDeleteNamechangeMuteEntry(i);
		if(m_apPlayers[i]->IsQuesting())
		{
			NumQuesting++;
			if(m_apPlayers[i]->m_QuestPlayerID != -1) //if player is on a <specfic player> quest
			{
				if(!m_apPlayers[m_apPlayers[i]->m_QuestPlayerID])
				{
					SendChatTarget(i, "[QUEST] Looks like your quest destination left the server.");
					QuestFailed(i);
				}
				else if(m_apPlayers[m_apPlayers[i]->m_QuestPlayerID]->GetTeam() == TEAM_SPECTATORS)
				{
					SendChatTarget(i, "[QUEST] Looks like your quest destination is a spectator.");
					QuestFailed(i);
				}
			}
		}
		if(m_apPlayers[i]->m_IsSurvivaling)
		{
			StopSurvival = false;
		}
		if(m_BlockTournaState == 3)
		{
			if(m_apPlayers[i]->m_IsBlockTourning)
			{
				m_apPlayers[i]->m_IsBlockTourning = false;
				if(m_apPlayers[i]->GetCharacter())
				{
					m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_GAME);
				}
			}
		}
		if(m_apPlayers[i]->m_IsDummy)
		{
			if(m_apPlayers[i]->m_DummyMode == CCharacter::DUMMYMODE_ADVENTURE)
				NumAdventureBots++;
		}
	}

	if(TotalPlayers + 3 > g_Config.m_SvMaxClients ||
		NumQuesting == 0)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(!m_apPlayers[i])
				continue;
			if(!m_apPlayers[i]->m_IsDummy)
				continue;
			if(m_apPlayers[i]->m_DummyMode == CCharacter::DUMMYMODE_QUEST)
				Server()->BotLeave(i);
		}
	}
	if(NumAdventureBots < g_Config.m_SvAdventureBots)
	{
		ConnectAdventureBots();
	}

	if(StopSurvival)
	{
		m_survivalgamestate = 0; //don't waste ressource on lobby checks if nobody is playing
	}
	if(m_BlockTournaState == 3)
	{
		m_BlockTournaState = 0;
	}
	if(g_Config.m_SvAllowGlobalChat)
	{
		GlobalChatPrintMessage();
	}

	if(g_Config.m_SvMinDoubleTilePlayers > 0)
	{
		if(CountIngameHumans() >= g_Config.m_SvMinDoubleTilePlayers && MoneyDoubleEnoughPlayers == true) // MoneyTileDouble();  bla bla
		{
			SendChat(-1, CGameContext::CHAT_ALL, "The double-moneytile has been activated!");
			MoneyDoubleEnoughPlayers = false;
		}
		if(CountIngameHumans() < g_Config.m_SvMinDoubleTilePlayers && MoneyDoubleEnoughPlayers == false)
		{
			SendChat(-1, CGameContext::CHAT_ALL, "The double-moneytile has been deactivated!");
			MoneyDoubleEnoughPlayers = true;
		}
	}
	CheckDDPPshutdown();
	if(g_Config.m_SvAutoFixBrokenAccs)
		SQLcleanZombieAccounts(-1);
}

void CGameContext::ChilliClanTick(int i)
{
	if(!g_Config.m_SvKickChilliClan)
		return;

	if(!m_apPlayers[i])
		return;

	CPlayer *pPlayer = m_apPlayers[i];

	int AbstandWarnungen = 10;
	if((str_comp_nocase(Server()->ClientClan(i), "Chilli.*") == 0 && str_comp_nocase(pPlayer->m_TeeInfos.m_SkinName, "greensward") != 0) && (!pPlayer->m_SpookyGhostActive))
	{
		if(pPlayer->m_LastWarning + AbstandWarnungen * Server()->TickSpeed() <= Server()->Tick())
		{
			pPlayer->m_ChilliWarnings++;

			if(pPlayer->m_ChilliWarnings >= 4)
			{
				if(g_Config.m_SvKickChilliClan == 1)
				{
					GetPlayerChar(i)->m_FreezeTime = 1000;
					SendBroadcast("WARNING! You are using the wrong 'Chilli.*' clanskin.\n Leave the clan or change skin.", i);
					SendChatTarget(i, "You got freezed by Chilli.* clanportection. Change skin or clantag!");
				}
				else if(g_Config.m_SvKickChilliClan == 2)
				{
					char aRcon[128];
					str_format(aRcon, sizeof(aRcon), "kick %d Chilli.* clanfake", i);
					Console()->ExecuteLine(aRcon);
				}
			}
			else
			{
				SendChatTarget(i, "#######################################");
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "You are using the wrong skin! Change skin or clantag! Warning: [%d/3]", pPlayer->m_ChilliWarnings);
				SendChatTarget(i, aBuf);
				SendChatTarget(i, "For more information about the clan visit: www.chillerdragon.weebly.com");
				SendChatTarget(i, "#######################################");

				/*
				char aRcon[128];
				str_format(aRcon, sizeof(aRcon), "broadcast YOU USE THE WRONG SKIN!\nCHANGE CLANTAG OR USE THE SKIN 'greensward'\n\nWARNINGS UNTIL KICK[%d/3]", pPlayer->m_ChilliWarnings);
				Console()->ExecuteLine(aRcon);
				*/

				char aBuf2[256];
				if(g_Config.m_SvKickChilliClan == 1)
				{
					str_format(aBuf2, sizeof(aBuf2), "Your are using the wrong skin!\nChange you clantag or use skin 'greensward'!\n\nWARNINGS UNTIL FREEZE[%d / 3]", pPlayer->m_ChilliWarnings);
				}
				else if(g_Config.m_SvKickChilliClan == 2)
				{
					str_format(aBuf2, sizeof(aBuf2), "Your are using the wrong skin!\nChange you clantag or use skin 'greensward'!\n\nWARNINGS UNTIL KICK[%d / 3]", pPlayer->m_ChilliWarnings);
				}
				SendBroadcast(aBuf2, i);
			}

			pPlayer->m_LastWarning = Server()->Tick();
		}
	}
}

void CGameContext::AsciiTick(int i)
{
	if(!m_apPlayers[i])
		return;

	if(m_apPlayers[i]->m_AsciiWatchingID != -1)
	{
		if(!m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]) //creator left -> stop animation
		{
			//SendChatTarget(i, "Ascii animation stopped because the creator left the server.");
			//SendBroadcast(" ERROR LOADING ANIMATION ", i);
			m_apPlayers[i]->m_AsciiWatchingID = -1;
			m_apPlayers[i]->m_AsciiWatchTicker = 0;
			m_apPlayers[i]->m_AsciiWatchFrame = 0;
		}
		else
		{
			m_apPlayers[i]->m_AsciiWatchTicker++;
			if(m_apPlayers[i]->m_AsciiWatchTicker >= m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_AsciiAnimSpeed) //new frame
			{
				m_apPlayers[i]->m_AsciiWatchTicker = 0;
				m_apPlayers[i]->m_AsciiWatchFrame++;
				if(m_apPlayers[i]->m_AsciiWatchFrame > 15) //animation over -> stop animation
				{
					//SendChatTarget(i, "Ascii animation is over.");
					//SendBroadcast(" ANIMATION OVER ", i);
					m_apPlayers[i]->m_AsciiWatchingID = -1;
					m_apPlayers[i]->m_AsciiWatchTicker = 0;
					m_apPlayers[i]->m_AsciiWatchFrame = 0;
				}
				else //display new frame
				{
					if(m_apPlayers[i]->m_AsciiWatchFrame == 0)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame0, i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 1)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame1, i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 2)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame2, i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 3)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame3, i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 4)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame4, i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 5)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame5, i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 6)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame6, i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 7)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame7, i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 8)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame8, i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 9)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame9, i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 10)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame10, i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 11)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame11, i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 12)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame12, i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 13)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame13, i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 14)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame14, i);
					}
					else if(m_apPlayers[i]->m_AsciiWatchFrame == 15)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame15, i);
					}
					else
					{
						SendChatTarget(i, "error loading frame");
					}
				}
			}
		}
	}
}

void CGameContext::LoadSinglePlayer()
{
	FILE *pFile;
	struct CBinaryStorage statsBuff;

	pFile = fopen("ddpp-stats.dat", "rb");
	if(!pFile)
	{
		dbg_msg("ddpp-stats", "[load] failed to open ddpp singleplayer stats");
		return;
	}

	if(!fread(&statsBuff, sizeof(struct CBinaryStorage), 1, pFile))
		goto fail;
	dbg_msg("ddpp-stats", "loaded data UnlockedLevel=%d", statsBuff.x);
	m_MissionUnlockedLevel = statsBuff.x;
	if(!fread(&statsBuff, sizeof(struct CBinaryStorage), 1, pFile))
		goto fail;
	dbg_msg("ddpp-stats", "loaded data CurrentLevel=%d", statsBuff.x);
	m_MissionCurrentLevel = statsBuff.x;

	if(fclose(pFile))
		dbg_msg("ddpp-stats", "failed to close singleplayer file='%s' errno=%d", "ddpp-stats.dat", errno);

fail:
	dbg_msg("ddpp-stats", "failed to read ddpp singleplayer stats");
}

void CGameContext::SaveSinglePlayer()
{
	FILE *pFile;
	struct CBinaryStorage statsBuff;

	pFile = fopen("ddpp-stats.dat", "wb");
	if(!pFile)
	{
		dbg_msg("ddpp-stats", "[save] failed to open ddpp singleplayer stats");
		return;
	}
	statsBuff.x = m_MissionUnlockedLevel;
	fwrite(&statsBuff, sizeof(struct CBinaryStorage), 1, pFile);
	statsBuff.x = m_MissionCurrentLevel;
	fwrite(&statsBuff, sizeof(struct CBinaryStorage), 1, pFile);

	fclose(pFile);
}

void CGameContext::SaveMapPlayerData()
{
	FILE *pFile;
	char aSaveFile[256];
	str_format(aSaveFile, sizeof(aSaveFile), "%s_playerdata.dat", Server()->GetMapName());
	pFile = fopen(aSaveFile, "wb");
	if(!pFile)
	{
		dbg_msg("ddpp-mapsave", "failed to write ddpp player data file '%s'.", aSaveFile);
		return;
	}
	int saved = 0;
	int players = CountTimeoutCodePlayers();
	fwrite(&players, sizeof(players), 1, pFile);
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = m_apPlayers[i];
		if(!pPlayer)
			continue;
		CCharacter *pChr = pPlayer->GetCharacter();
		if(!pChr)
			continue;
		if(!pPlayer->m_TimeoutCode[0])
			continue;
		if(!pChr)
			continue;
		fwrite(&pPlayer->m_TimeoutCode, 64, 1, pFile);
		char IsLoaded = 0;
		fpos_t pos;
		fgetpos(pFile, &pos);
		dbg_msg("ddpp-mapsave", "writing isloaded at pos %lld", fpost_get_pos(pos));
		fwrite(&IsLoaded, sizeof(IsLoaded), 1, pFile);

		CSaveTee savetee;
		savetee.save(pChr);
		fwrite(&savetee, sizeof(savetee), 1, pFile);

		dbg_msg("ddpp-mapsave", "save player=%s code=%s", Server()->ClientName(i), pPlayer->m_TimeoutCode);
		saved++;
	}
	fclose(pFile);
	dbg_msg("ddpp-mapsave", "saved %d/%d players", saved, players);
}

void CGameContext::LoadMapPlayerData()
{
	FILE *pFile;
	char aSaveFile[256];
	str_format(aSaveFile, sizeof(aSaveFile), "%s_playerdata.dat", Server()->GetMapName());
	pFile = fopen(aSaveFile, "rb+");
	if(!pFile)
	{
		m_MapsavePlayers = 0;
		dbg_msg("ddpp-mapload", "failed to open ddpp player data file '%s'.", aSaveFile);
		return;
	}
	int loaded = 0;
	int players = 0;
	int err = 0;
	if(!fread(&players, sizeof(players), 1, pFile))
	{
		err = 1;
		goto fail;
	}
	for(int i = 0; i < players; i++)
	{
		// ValidPlayer replaces continue to make sure the binary cursor is at the right offset
		bool ValidPlayer = true;
		char aTimeoutCode[64];
		fpos_t pos;
		fgetpos(pFile, &pos);
		dbg_msg("ddpp-mapload", "read timeout code at %lld", fpost_get_pos(pos));
		if(!fread(&aTimeoutCode, 64, 1, pFile))
		{
			err = 2;
			goto fail;
		}
		int id = GetPlayerByTimeoutcode(aTimeoutCode);
		if(id == -1)
		{
			dbg_msg("ddpp-mapload", "player not online code=%s", aTimeoutCode);
			ValidPlayer = false;
		}
		else
		{
			CPlayer *pPlayer = m_apPlayers[id];
			if(!pPlayer)
			{
				dbg_assert(false, "loadmap invalid player");
				return;
			}
			CCharacter *pChr = pPlayer->GetCharacter();
			if(ValidPlayer && !pChr)
			{
				dbg_msg("ddpp-mapload", "player not alive id=%d code=%s", id, aTimeoutCode);
				ValidPlayer = false;
			}
			if(ValidPlayer && str_comp(aTimeoutCode, pPlayer->m_TimeoutCode))
			{
				dbg_msg("ddpp-mapload", "wrong timeout code player=%s file=%s", pPlayer->m_TimeoutCode, aTimeoutCode);
				ValidPlayer = false;
			}
			if(ValidPlayer && pPlayer->m_MapSaveLoaded)
			{
				// shouldn't happen? couldn't happen? too lazy to think probably possible by abusing /timeout command or share
				// and can be bypassed by reconnect lol
				dbg_msg("ddpp-mapload", "Warning: %d:'%s' code=%s is loaded already", id, Server()->ClientName(id), pPlayer->m_TimeoutCode);
				ValidPlayer = false;
			}
		}
		char IsLoaded;
		fgetpos(pFile, &pos);
		dbg_msg("ddpp-mapload", "reading isloaded at pos %lld", fpost_get_pos(pos));
		if(!fread(&IsLoaded, sizeof(IsLoaded), 1, pFile))
		{
			err = 3;
			goto fail;
		}
		// reset file pos after read to overwrite isloaded or keep clean offset on continue
		if(IsLoaded)
		{
			dbg_msg("ddpp-mapload", "loaded already");
			ValidPlayer = false;
		}
		IsLoaded = ValidPlayer ? 1 : IsLoaded; // only change loaded state if the player is actually loaded
		fsetpos(pFile, &pos);
		fwrite(&IsLoaded, sizeof(IsLoaded), 1, pFile);

		CSaveTee savetee;
		if(!fread(&savetee, sizeof(savetee), 1, pFile))
		{
			err = 4;
			goto fail;
		}

		if(ValidPlayer)
		{
			CPlayer *pPlayer = m_apPlayers[id];
			CCharacter *pChr = pPlayer->GetCharacter();

			savetee.load(pChr, 0); // load to team0 always xd cuz teams sokk!

			m_MapsaveLoadedPlayers++;
			pPlayer->m_MapSaveLoaded = true;
			fgetpos(pFile, &pos);
			dbg_msg("ddpp-mapload", "load player=%s code=%s fp=%lld", Server()->ClientName(id), pPlayer->m_TimeoutCode, fpost_get_pos(pos));
			loaded++;
		}
	}
	if(fclose(pFile))
		dbg_msg("ddpp-mapload", "failed to close file '%s' errno=%d", aSaveFile, errno);
	m_MapsavePlayers = players;
	dbg_msg("ddpp-mapload", "loaded %d/%d players", loaded, players);

fail:
	dbg_msg("ddpp-mapload", "failed to read data=%d", err);
}

void CGameContext::ReadMapPlayerData(int ClientID)
{
	FILE *pFile;
	char aSaveFile[256];
	str_format(aSaveFile, sizeof(aSaveFile), "%s_playerdata.dat", Server()->GetMapName());
	pFile = fopen(aSaveFile, "rb");
	if(!pFile)
	{
		dbg_msg("ddpp-mapread", "failed to read ddpp player data file '%s'.", aSaveFile);
		return;
	}
	int loaded = 0;
	int red = 0;
	int players = 0;
	int err = 0;
	if(!fread(&players, sizeof(players), 1, pFile))
	{
		err = 1;
		goto fail;
	}
	for(int i = 0; i < players; i++)
	{
		char aTimeoutCode[64];
		fpos_t pos;
		fgetpos(pFile, &pos);
		dbg_msg("ddpp-mapread", "read timeout code at %lld", fpost_get_pos(pos));
		if(!fread(&aTimeoutCode, 64, 1, pFile))
		{
			err = 2;
			goto fail;
		}
		int id = GetPlayerByTimeoutcode(aTimeoutCode);
		char IsLoaded;
		if(!fread(&IsLoaded, sizeof(IsLoaded), 1, pFile))
		{
			err = 3;
			goto fail;
		}
		if(IsLoaded)
			loaded++;

		CSaveTee savetee;
		if(!fread(&savetee, sizeof(savetee), 1, pFile))
		{
			err = 4;
			goto fail;
		}

		fgetpos(pFile, &pos);
		dbg_msg("ddpp-mapread", "read player=%d code=%s loaded=%d fp=%lld", id, aTimeoutCode, IsLoaded, fpost_get_pos(pos));
		red++;
	}
	if(fclose(pFile))
		dbg_msg("ddpp-mapread", "failed to close file '%s' errno=%d", aSaveFile, errno);
	if(ClientID != -1)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "[MAPSAVE] Debug: loaded %d/%d players", loaded, players);
		SendChatTarget(ClientID, aBuf);
		if(red != players)
		{
			str_format(aBuf, sizeof(aBuf), "[MAPSAVE] Debug: WARNING only found %d/%d players", red, players);
			SendChatTarget(ClientID, aBuf);
		}
	}
	dbg_msg("ddpp-mapread", "red %d/%d players (%d loaded)", red, players, loaded);

fail:
	dbg_msg("ddpp-mapread", "failed to read data=%d", err);
}

void CGameContext::GlobalChatPrintMessage()
{
	char aData[1024];
	std::string data;

	std::fstream ChatReadFile(g_Config.m_SvGlobalChatFile);

	if(!std::ifstream(g_Config.m_SvGlobalChatFile))
	{
		SendChat(-1, CGameContext::CHAT_ALL, "[CHAT] global chat stopped working.");
		g_Config.m_SvAllowGlobalChat = 0;
		ChatReadFile.close();
		return;
	}

	getline(ChatReadFile, data);
	str_format(aData, sizeof(aData), "%s", data.c_str());
	aData[0] = ' '; //remove the confirms before print in chat

	if(!str_comp(m_aLastPrintedGlobalChatMessage, aData))
	{
		//SendChat(-1, CGameContext::CHAT_ALL, "[CHAT] no new global message");
		ChatReadFile.close();
		return;
	}

	GlobalChatUpdateConfirms(data.c_str());
	str_format(m_aLastPrintedGlobalChatMessage, sizeof(m_aLastPrintedGlobalChatMessage), "%s", aData);
	SendChat(-1, CGameContext::CHAT_ALL, aData);
	//str_format(aBuf, sizeof(aBuf), "[CHAT] '%s'", aData);
	//SendChat(-1, CGameContext::CHAT_ALL, aBuf);

	ChatReadFile.close();
}

void CGameContext::GlobalChatUpdateConfirms(const char *pStr)
{
	char aBuf[1024];
	str_format(aBuf, sizeof(aBuf), "%s", pStr);
	int confirms = 0;
	if(pStr[0] == '1')
		confirms = 1;
	else if(pStr[0] == '2')
		confirms = 2;
	else if(pStr[0] == '3')
		confirms = 3;
	else if(pStr[0] == '4')
		confirms = 4;
	else if(pStr[0] == '5')
		confirms = 5;
	else if(pStr[0] == '6')
		confirms = 6;
	else if(pStr[0] == '7')
		confirms = 7;
	else if(pStr[0] == '8')
		confirms = 8;
	else if(pStr[0] == '9')
		confirms = 9;

	std::ofstream ChatFile(g_Config.m_SvGlobalChatFile);
	if(!ChatFile)
	{
		SendChat(-1, CGameContext::CHAT_ALL, "[CHAT] global chat failed.... deactivating it.");
		dbg_msg("CHAT", "ERROR1 writing file '%s'", g_Config.m_SvGlobalChatFile);
		g_Config.m_SvAllowGlobalChat = 0;
		ChatFile.close();
		return;
	}

	if(ChatFile.is_open())
	{
		confirms++;
		aBuf[0] = confirms + '0';
		ChatFile << aBuf << "\n";
	}
	else
	{
		SendChat(-1, CGameContext::CHAT_ALL, "[CHAT] global chat failed.... deactivating it.");
		dbg_msg("CHAT", "ERROR2 writing file '%s'", g_Config.m_SvGlobalChatFile);
		g_Config.m_SvAllowGlobalChat = 0;
	}

	ChatFile.close();
}

void CGameContext::SendAllPolice(const char *pMessage)
{
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "[POLICE-CHANNEL] %s", pMessage);
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_PoliceRank)
		{
			SendChatTarget(i, aBuf);
		}
	}
}

void CGameContext::AddEscapeReason(int ID, const char *pReason)
{
	//dbg_msg("cBug", "current reaso is %s", m_apPlayers[ID]->m_aEscapeReason);

	//dont add already exsisting reasons agian
	if(str_find(m_apPlayers[ID]->m_aEscapeReason, pReason))
	{
		//dbg_msg("cBug", "skipping exsisting reason %s", pReason);
		return;
	}
	//reset all
	if(!str_comp(pReason, "unknown"))
	{
		str_format(m_apPlayers[ID]->m_aEscapeReason, sizeof(m_apPlayers[ID]->m_aEscapeReason), "%s", pReason);
		//dbg_msg("cBug", "resetting to reason %s", pReason);
		return;
	}

	if(!str_comp(m_apPlayers[ID]->m_aEscapeReason, "unknown")) //keine vorstrafen
	{
		str_format(m_apPlayers[ID]->m_aEscapeReason, sizeof(m_apPlayers[ID]->m_aEscapeReason), "%s", pReason);
		dbg_msg("cBug", "set escape reason to %s -> %s", pReason, m_apPlayers[ID]->m_aEscapeReason);
	}
	else
	{
		str_format(m_apPlayers[ID]->m_aEscapeReason, sizeof(m_apPlayers[ID]->m_aEscapeReason), "%s, %s", m_apPlayers[ID]->m_aEscapeReason, pReason);
	}
	//wtf doesnt work with the seconds first then the reason always gets printed as (null) wtf !)!)!)!
	/*
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "'%s' seconds [%d] reason [%s]", Server()->ClientName(ID), m_apPlayers[ID]->m_EscapeTime / Server()->TickSpeed(), m_apPlayers[ID]->m_aEscapeReason);
	dbg_msg("DEBUG", aBuf);
	dbg_msg("cBug", "set escape reason to %s -> %s", pReason, m_apPlayers[ID]->m_aEscapeReason);
	str_format(aBuf, sizeof(aBuf), " seconds [%d] reason [%s]", m_apPlayers[ID]->m_EscapeTime / Server()->TickSpeed(), m_apPlayers[ID]->m_aEscapeReason);
	SendChatTarget(ID, aBuf);
	*/
}

void CGameContext::ShowProfile(int ViewerID, int ViewedID)
{
	char aBuf[128];
	int GiveView = 1;

	if(!m_apPlayers[ViewedID] || !m_apPlayers[ViewerID])
	{
		return;
	}

	if(m_apPlayers[ViewedID]->IsLoggedIn())
	{
		SendChatTarget(ViewerID, "Player has to be logged in to view his profile.");
		return;
	}

	if(!str_comp(m_apPlayers[ViewerID]->m_LastViewedProfile, Server()->ClientName(ViewedID)) && !m_apPlayers[ViewerID]->m_IsProfileViewLoaded) //repeated same profile and view not loaded yet
	{
		GiveView = 0;
	}
	else
	{
		if(!str_comp(Server()->ClientName(ViewedID), Server()->ClientName(ViewerID))) //viewing own profile --> random xd
		{
			GiveView = rand() % 2;
		}
	}

	if(GiveView)
	{
		m_apPlayers[ViewedID]->m_ProfileViews++;
		str_copy(m_apPlayers[ViewerID]->m_LastViewedProfile, Server()->ClientName(ViewedID), 32);
		m_apPlayers[ViewerID]->m_IsProfileViewLoaded = false;
	}

	//ASCII - ANIMATIONS
	StartAsciiAnimation(ViewerID, ViewedID, 1);

	if(m_apPlayers[ViewedID]->m_ProfileStyle == 0) //default
	{
		str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(ViewedID));
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[ViewedID]->m_ProfileStatus);
		SendChatTarget(ViewerID, aBuf);
		SendChatTarget(ViewerID, "-------------------------");
		str_format(aBuf, sizeof(aBuf), "Level: %d", m_apPlayers[ViewedID]->GetLevel());
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Money: %lld", m_apPlayers[ViewedID]->GetMoney());
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Shit: %d", m_apPlayers[ViewedID]->m_shit);
		SendChatTarget(ViewerID, aBuf);
	}
	else if(m_apPlayers[ViewedID]->m_ProfileStyle == 1) //shit
	{
		str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(ViewedID));
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[ViewedID]->m_ProfileStatus);
		SendChatTarget(ViewerID, aBuf);
		SendChatTarget(ViewerID, "-------------------------");
		str_format(aBuf, sizeof(aBuf), "Shit: %d", m_apPlayers[ViewedID]->m_shit);
		SendChatTarget(ViewerID, aBuf);
	}
	else if(m_apPlayers[ViewedID]->m_ProfileStyle == 2) //social
	{
		str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(ViewedID));
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[ViewedID]->m_ProfileStatus);
		SendChatTarget(ViewerID, aBuf);
		SendChatTarget(ViewerID, "-------------------------");
		str_format(aBuf, sizeof(aBuf), "Skype: %s", m_apPlayers[ViewedID]->m_ProfileSkype);
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Youtube: %s", m_apPlayers[ViewedID]->m_ProfileYoutube);
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "e-mail: %s", m_apPlayers[ViewedID]->m_ProfileEmail);
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Homepage: %s", m_apPlayers[ViewedID]->m_ProfileHomepage);
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Twitter: %s", m_apPlayers[ViewedID]->m_ProfileTwitter);
		SendChatTarget(ViewerID, aBuf);
	}
	else if(m_apPlayers[ViewedID]->m_ProfileStyle == 3) //show-off
	{
		str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(ViewedID));
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[ViewedID]->m_ProfileStatus);
		SendChatTarget(ViewerID, aBuf);
		SendChatTarget(ViewerID, "-------------------------");
		str_format(aBuf, sizeof(aBuf), "Profileviews: %d", m_apPlayers[ViewedID]->m_ProfileViews);
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Policerank: %d", m_apPlayers[ViewedID]->m_PoliceRank);
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Level: %d", m_apPlayers[ViewedID]->GetLevel());
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Shit: %d", m_apPlayers[ViewedID]->m_shit);
		SendChatTarget(ViewerID, aBuf);
	}
	else if(m_apPlayers[ViewedID]->m_ProfileStyle == 4) //pvp
	{
		str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(ViewedID));
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[ViewedID]->m_ProfileStatus);
		SendChatTarget(ViewerID, aBuf);
		SendChatTarget(ViewerID, "-------------------------");
		str_format(aBuf, sizeof(aBuf), "PVP-ARENA Games: %d", m_apPlayers[ViewedID]->m_pvp_arena_games_played);
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "PVP-ARENA Kills: %d", m_apPlayers[ViewedID]->m_pvp_arena_kills);
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "PVP-ARENA Deaths: %d", m_apPlayers[ViewedID]->m_pvp_arena_deaths);
		SendChatTarget(ViewerID, aBuf);
		//str_format(aBuf, sizeof(aBuf), "PVP-ARENA K/D: %d", m_apPlayers[ViewedID]->m_pvp_arena_kills / m_pvp_arena_deaths);
		//SendChatTarget(ViewerID, aBuf);
	}
	else if(m_apPlayers[ViewedID]->m_ProfileStyle == 5) //bomber
	{
		str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(ViewedID));
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[ViewedID]->m_ProfileStatus);
		SendChatTarget(ViewerID, aBuf);
		SendChatTarget(ViewerID, "-------------------------");
		str_format(aBuf, sizeof(aBuf), "Bomb Games Played: %d", m_apPlayers[ViewedID]->m_BombGamesPlayed);
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Bomb Games Won: %d", m_apPlayers[ViewedID]->m_BombGamesWon);
		SendChatTarget(ViewerID, aBuf);
		//str_format(aBuf, sizeof(aBuf), "PVP-ARENA K/D: %d", m_apPlayers[ViewedID]->m_pvp_arena_kills / m_pvp_arena_deaths);
		//SendChatTarget(ViewerID, aBuf);
	}
}

void CGameContext::ShowAdminWelcome(int ID)
{
	SendChatTarget(ID, "============= admin login =============");
	char aBuf[128];
	if(m_WrongRconAttempts >= g_Config.m_SvRconAttemptReport)
	{
		str_format(aBuf, sizeof(aBuf), "Warning %d failed rcon attempts since last successful login! 'logs wrong_rcon'", m_WrongRconAttempts);
		// Server()->SendRconLine(ID, aBuf); // TODO: uncomment
	}
	if(aDDPPLogs[DDPP_LOG_AUTH_RCON][1][0]) // index 1 because index 0 is current login
	{
		str_format(aBuf, sizeof(aBuf), "last login %s", aDDPPLogs[DDPP_LOG_AUTH_RCON][1]);
		// Server()->SendRconLine(ID, aBuf); // TODO: uncomment
	}
	int surv_error = TestSurvivalSpawns();
	if(surv_error == -1)
	{
		SendChatTarget(ID, "[ADMIN:Test] WARNING: less survival spawns on map than slots possible in ddnet++ (no problem as long as slots stay how they are)");
	}
	else if(surv_error == -2)
	{
		SendChatTarget(ID, "[ADMIN:Test] WARNING: not enough survival spawns (less survival spawns than slots)");
	}
	int protections = 0;
	if(g_Config.m_SvRegisterHumanLevel)
	{
		str_format(aBuf, sizeof(aBuf), "[ADMIN:Prot] Warning sv_register_human_level = %d", g_Config.m_SvRegisterHumanLevel);
		SendChatTarget(ID, aBuf);
		protections++;
	}
	if(g_Config.m_SvChatHumanLevel)
	{
		str_format(aBuf, sizeof(aBuf), "[ADMIN:Prot] Warning sv_chat_human_level = %d", g_Config.m_SvChatHumanLevel);
		SendChatTarget(ID, aBuf);
		protections++;
	}
	if(g_Config.m_SvShowConnectionMessages != CON_SHOW_ALL)
	{
		str_format(aBuf, sizeof(aBuf), "[ADMIN:Prot] Warning sv_show_connection_msg = %d", g_Config.m_SvShowConnectionMessages);
		SendChatTarget(ID, aBuf);
		protections++;
	}
	if(g_Config.m_SvHideConnectionMessagesPattern[0])
	{
		str_format(aBuf, sizeof(aBuf), "[ADMIN:Prot] Warning sv_hide_connection_msg_pattern = %s", g_Config.m_SvHideConnectionMessagesPattern);
		SendChatTarget(ID, aBuf);
		protections++;
	}
	if(protections)
	{
		str_format(aBuf, sizeof(aBuf), "[ADMIN:Prot] Warning you have %d protective systems running!", protections);
		SendChatTarget(ID, aBuf);
		SendChatTarget(ID, "[ADMIN:Prot] As effective those are under attack and as good protection sounds.");
		SendChatTarget(ID, "[ADMIN:Prot] Those should not run if there is no attack since they lower UX.");
		protections++;
	}
	PrintSpecialCharUsers(ID);
}

int CGameContext::PrintSpecialCharUsers(int ID)
{
	char aUsers[2048]; //wont show all users if too many special char users are there but this shouldnt be the case
	int users = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->IsLoggedIn())
		{
			if(IsAllowedCharSet(m_apPlayers[i]->m_aAccountLoginName) == false)
			{
				if(!users)
				{
					str_format(aUsers, sizeof(aUsers), "[id='%d' acc='%s']", i, m_apPlayers[i]->m_aAccountLoginName);
				}
				else
				{
					str_format(aUsers, sizeof(aUsers), "%s, [id='%d' acc='%s']", aUsers, i, m_apPlayers[i]->m_aAccountLoginName);
				}
				users++;
			}
		}
	}

	if(users)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "[#########] %d special char user online [#########]", users);
		SendChatTarget(ID, aBuf);
		SendChatTarget(ID, aUsers);
	}
	return users;
}

int CGameContext::TestSurvivalSpawns()
{
	vec2 SurvivalGameSpawnTile = Collision()->GetSurvivalSpawn(g_Config.m_SvMaxClients);
	vec2 SurvivalGameSpawnTile2 = Collision()->GetSurvivalSpawn(MAX_CLIENTS);

	if(SurvivalGameSpawnTile == vec2(-1, -1))
	{
		//SendChatTarget(ClientID, "[ADMIN:Test] ERROR: not enough survival spawns (less survival spawns than slots)");
		return -2;
	}
	else if(SurvivalGameSpawnTile2 == vec2(-1, -1))
	{
		//SendChatTarget(ClientID, "[ADMIN:Test] WARNING: less survival spawns on map than slots possible in ddnet++ (no problem as long as slots stay how they are)");
		return -1;
	}
	else
	{
		//SendChatTarget(ClientID, "[ADMIN:Test] Test Finished. Everything looks good c:");
		return 0;
	}
	return 0;
}

void CGameContext::ChatCommands()
{
}

void CGameContext::DummyChat()
{
	//unused cuz me knoop putting all the stuff here
}

void CGameContext::CreateBasicDummys()
{
	if(!str_comp(Server()->GetMapName(), "ChillBlock5"))
	{
		CreateNewDummy(31); //police
		CreateNewDummy(29); //blocker
		CreateNewDummy(29); //blocker 2
		CreateNewDummy(23); //racer
		CreateNewDummy(-6); //blocker dm v3
	}
	else if(!str_comp(Server()->GetMapName(), "BlmapChill"))
	{
		CreateNewDummy(32); //police
		//CreateNewDummy(28);//racer
	}
	else if(!str_comp(Server()->GetMapName(), "blmapV5"))
	{
		CreateNewDummy(104); //lower blocker
		CreateNewDummy(104); //lower blocker
		CreateNewDummy(105); //upper blocker
	}
	else if(!str_comp(Server()->GetMapName(), "blmapV5_ddpp"))
	{
		CreateNewDummy(104); //lower blocker
		CreateNewDummy(104); //lower blocker
		CreateNewDummy(105); //upper blocker
		g_Config.m_SvDummyMapOffsetX = -226;
	}
	else if(!str_comp(Server()->GetMapName(), "ddpp_survival"))
	{
		CreateNewDummy(34); //dynamic pvp mode
		CreateNewDummy(34); //dynamic pvp mode
	}
	else
	{
		CreateNewDummy(0); //dummy
		dbg_msg("basic_dummys", "waring map=%s not supported", Server()->GetMapName());
	}
	if(m_ShopBotTileExists)
	{
		m_CreateShopBot = true;
	}
	dbg_msg("basic_dummys", "map=%s", Server()->GetMapName());
}

int CGameContext::CreateNewDummy(int dummymode, bool silent, int tile)
{
	int DummyID = GetNextClientID();
	if(DummyID < 0)
	{
		dbg_msg("dummy", "Can't get ClientID. Server is full or something like that.");
		return -1;
	}

	if(m_apPlayers[DummyID])
	{
		m_apPlayers[DummyID]->OnDisconnect("");
		delete m_apPlayers[DummyID];
		m_apPlayers[DummyID] = 0;
	}

	m_apPlayers[DummyID] = new(DummyID) CPlayer(this, DummyID, TEAM_RED);

	m_apPlayers[DummyID]->m_NoboSpawnStop = 0;
	m_apPlayers[DummyID]->m_IsDummy = true;
	m_apPlayers[DummyID]->m_DummyMode = dummymode;
	Server()->BotJoin(DummyID);

	str_copy(m_apPlayers[DummyID]->m_TeeInfos.m_SkinName, "greensward", MAX_NAME_LENGTH);
	m_apPlayers[DummyID]->m_TeeInfos.m_UseCustomColor = true;
	m_apPlayers[DummyID]->m_TeeInfos.m_ColorFeet = 0;
	m_apPlayers[DummyID]->m_TeeInfos.m_ColorBody = 0;
	m_apPlayers[DummyID]->m_DummySpawnTile = tile;

	dbg_msg("dummy", "Dummy connected: %d", DummyID);

	if(dummymode == -1) //balancedummy1
	{
		m_apPlayers[DummyID]->m_IsBalanceBattlePlayer1 = true;
		m_apPlayers[DummyID]->m_IsBalanceBattleDummy = true;
	}
	else if(dummymode == -2) //balancedummy2
	{
		m_apPlayers[DummyID]->m_IsBalanceBattlePlayer1 = false;
		m_apPlayers[DummyID]->m_IsBalanceBattleDummy = true;
	}
	else if(dummymode == -3) //blockwavebot
	{
		m_apPlayers[DummyID]->m_IsBlockWaving = true;
	}
	else if(dummymode == -4) //laser fng
	{
		JoinInstagib(5, true, DummyID);
	}
	else if(dummymode == -5) //grenade fng
	{
		JoinInstagib(4, true, DummyID);
	}
	else if(dummymode == -6) //ChillBlock5 v3 deathmatch
	{
		m_apPlayers[DummyID]->m_IsBlockDeathmatch = true;
	}
	else if(dummymode == -7) // vanilla based mode
	{
		m_apPlayers[DummyID]->m_IsVanillaDmg = true;
		m_apPlayers[DummyID]->m_IsVanillaWeapons = true;
		m_apPlayers[DummyID]->m_IsVanillaCompetetive = true;
	}

	OnClientEnter(DummyID, silent);

	return DummyID;
}

bool CGameContext::CheckIpJailed(int ClientID)
{
	if(!m_apPlayers[ClientID])
		return false;
	NETADDR Addr;
	Server()->GetClientAddr(ClientID, &Addr);
	Addr.port = 0;
	for(int i = 0; i < m_NumJailIPs; i++)
	{
		if(!net_addr_comp(&Addr, &m_aJailIPs[i]))
		{
			SendChatTarget(ClientID, "[JAIL] you have been jailed for 2 minutes.");
			m_apPlayers[ClientID]->JailPlayer(120);
			return true;
		}
	}
	return false;
}

void CGameContext::SetIpJailed(int ClientID)
{
	char aBuf[128];
	int Found = 0;
	NETADDR NoPortAddr;
	Server()->GetClientAddr(ClientID, &NoPortAddr);
	NoPortAddr.port = 0;
	// find a matching Mute for this ip, update expiration time if found
	for(int i = 0; i < m_NumJailIPs; i++)
	{
		if(net_addr_comp(&m_aJailIPs[i], &NoPortAddr) == 0)
		{
			Found = 1;
			break;
		}
	}
	if(!Found) // nothing found so far, find a free slot..
	{
		if(m_NumJailIPs < MAX_MUTES)
		{
			m_aJailIPs[m_NumJailIPs] = NoPortAddr;
			m_NumJailIPs++;
			Found = 1;
		}
	}
	if(Found)
	{
		str_format(aBuf, sizeof aBuf, "'%s' has been ip jailed.", Server()->ClientName(ClientID));
		SendChat(-1, CGameContext::CHAT_ALL, aBuf);
	}
	else // no free slot found
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mute", "name change mute array is full!");
	}
}

void CGameContext::SaveWrongLogin(const char *pLogin)
{
	if(!g_Config.m_SvSaveWrongLogin)
		return;

	std::ofstream LoginFile(g_Config.m_SvWrongLoginFile, std::ios::app);
	if(!LoginFile)
	{
		dbg_msg("login_sniff", "ERROR1 writing file '%s'", g_Config.m_SvWrongLoginFile);
		g_Config.m_SvSaveWrongLogin = 0;
		LoginFile.close();
		return;
	}

	if(LoginFile.is_open())
	{
		//dbg_msg("login_sniff", "sniffed msg [ %s ]", pLogin);
		LoginFile << pLogin << "\n";
	}
	else
	{
		dbg_msg("login_sniff", "ERROR2 writing file '%s'", g_Config.m_SvWrongLoginFile);
		g_Config.m_SvSaveWrongLogin = 0;
	}

	LoginFile.close();
}

bool CGameContext::AdminChatPing(const char *pMsg)
{
	if(!g_Config.m_SvMinAdminPing)
		return false;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i])
			continue;
		if(Server()->GetAuthedState(i))
			continue;
		if(str_find_nocase(pMsg, Server()->ClientName(i)))
		{
			int len_name = str_length(Server()->ClientName(i));
			int len_msg = str_length(pMsg);
			if(len_msg - len_name - 2 < g_Config.m_SvMinAdminPing)
				return true;
		}
	}
	return false;
}

bool CGameContext::ShowJoinMessage(int ClientID)
{
	if(!m_apPlayers[ClientID])
		return false;
	if(g_Config.m_SvShowConnectionMessages == CON_SHOW_NONE)
		return false;
	if(g_Config.m_SvHideConnectionMessagesPattern[0]) // if regex filter active
		if(!regex_compile(g_Config.m_SvHideConnectionMessagesPattern, Server()->ClientName(ClientID)))
			return false;
	return true;
}

bool CGameContext::ShowLeaveMessage(int ClientID)
{
	if(!m_apPlayers[ClientID])
		return false;
	if(g_Config.m_SvShowConnectionMessages == CON_SHOW_NONE)
		return false;
	if(g_Config.m_SvShowConnectionMessages == CON_SHOW_JOIN)
		return false;
	if(g_Config.m_SvHideConnectionMessagesPattern[0]) // if regex filter active
		if(!regex_compile(g_Config.m_SvHideConnectionMessagesPattern, Server()->ClientName(ClientID)))
			return false;
	return true;
}

bool CGameContext::ShowTeamSwitchMessage(int ClientID)
{
	if(!m_apPlayers[ClientID])
		return false;
	if(g_Config.m_SvShowConnectionMessages != CON_SHOW_ALL)
		return false;
	if(g_Config.m_SvHideConnectionMessagesPattern[0]) // if regex filter active
		if(!regex_compile(g_Config.m_SvHideConnectionMessagesPattern, Server()->ClientName(ClientID)))
			return false;
	return true;
}

void CGameContext::GetSpreeType(int ClientID, char *pBuf, size_t BufSize, bool IsRecord)
{
	CPlayer *pPlayer = m_apPlayers[ClientID];
	if(!pPlayer)
		return;

	if(pPlayer->m_IsInstaArena_fng && (pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm))
	{
		if(pPlayer->m_IsInstaArena_gdm)
			str_copy(pBuf, "boomfng", BufSize);
		else if(pPlayer->m_IsInstaArena_idm)
			str_copy(pBuf, "fng", BufSize);
	}
	else if(pPlayer->m_IsInstaArena_gdm)
	{
		if(IsRecord && pPlayer->m_KillStreak > pPlayer->m_GrenadeSpree)
		{
			pPlayer->m_GrenadeSpree = pPlayer->m_KillStreak;
			SendChatTarget(pPlayer->GetCID(), "New grenade spree record!");
		}
		str_copy(pBuf, "grenade", BufSize);
	}
	else if(pPlayer->m_IsInstaArena_idm)
	{
		if(IsRecord && pPlayer->m_KillStreak > pPlayer->m_RifleSpree)
		{
			pPlayer->m_RifleSpree = pPlayer->m_KillStreak;
			SendChatTarget(pPlayer->GetCID(), "New rifle spree record!");
		}
		str_copy(pBuf, "rifle", BufSize);
	}
	else if(pPlayer->m_IsVanillaDmg)
	{
		str_copy(pBuf, "killing", BufSize);
	}
	else //no insta at all
	{
		if(IsRecord && pPlayer->m_KillStreak > pPlayer->m_BlockSpreeHighscore)
		{
			pPlayer->m_BlockSpreeHighscore = pPlayer->m_KillStreak;
			SendChatTarget(pPlayer->GetCID(), "New Blockspree record!");
		}
		str_copy(pBuf, "blocking", BufSize);
	}
}
