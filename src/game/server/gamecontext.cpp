/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/tl/sorted_array.h>

#include <new>
#include <base/math.h>
#include <base/ddpp_logs.h>
#include <engine/shared/config.h>
#include <engine/map.h>
#include <engine/console.h>
#include <engine/shared/datafile.h>
#include <engine/shared/linereader.h>
#include <engine/storage.h>
#include "gamecontext.h"
#include <game/version.h>
#include <game/collision.h>
#include <game/gamecore.h>
#include <game/server/entities/flag.h>
/*#include "gamemodes/dm.h"
#include "gamemodes/tdm.h"
#include "gamemodes/ctf.h"
#include "gamemodes/mod.h"*/

#include "../../black_hole.h" //testy by ChillerDragon random back_hole.h file i recoved from random russian guy giving no information what it is
#include <stdio.h>
#include <string.h>
#include <engine/server/server.h> // ddpp imported for dummys
#include "gamemodes/DDRace.h"
#include "score.h"
#include "score/file_score.h"
#include <time.h>
#if defined(CONF_SQL)
#include "score/sql_score.h"
#endif

//ChillerDragon (ddpp)
#include <game/server/teams.h>
#include <fstream> //acc2 sys
#include <limits> //acc2 sys

enum
{
	RESET,
	NO_RESET
};

void CQuerySQLstatus::OnData()
{
	if (Next())
		m_pGameServer->SendChatTarget(m_ClientID, "[SQL] result: got rows.");
	else
		m_pGameServer->SendChatTarget(m_ClientID, "[SQL] result: no rows.");
}

void CQueryRegister::OnData()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Next())
	{
		m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Username already exists.");
	}
	else
	{
		char *pQueryBuf = sqlite3_mprintf("INSERT INTO Accounts (Username, Password, RegisterDate) VALUES ('%q', '%q', '%q');", m_Name.c_str(), m_Password.c_str(), m_Date.c_str());

		CQuery *pQuery = new CQuery();
		pQuery->Query(m_pDatabase, pQueryBuf);
		sqlite3_free(pQueryBuf);

		m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Account has been registered.");
		m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Login with: /login <name> <pass>");

		m_pGameServer->RegisterBanCheck(m_ClientID);
	}
}

/*
void CGameContext::LoginThread(void * pArg)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	struct LoginData *pData = static_cast<struct LoginData*>(pArg);
	CGameContext *pGS = static_cast<CGameContext*>(pData->pGameContext);
	CQueryLogin *pSQL = static_cast<CQueryLogin*>(pData->pSQL);
	CPlayer *pPlayer = static_cast<CPlayer*>(pData->pTmpPlayer);
	int id = pData->id;
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "[THREAD] hello world3 your id=%d", id);
	pGS->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
	pPlayer->m_money = 420;
}

void CGameContext::Login(void *pArg, int id)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	struct LoginData *pData = (struct LoginData*)malloc(sizeof(struct LoginData));
	pData->id = id;
	pData->pGameContext = this;
	pData->pSQL = pArg;
	pData->pTmpPlayer = m_apPlayers[id];
	void *pt = thread_init(*LoginThread, pData); //setzte die werte von pTmpPlayer
	//the thread result gets catched in LoginDone function called everytick
}
*/

void CQueryLoginThreaded::OnData()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_pGameServer->m_apPlayers[m_ClientID];
	if (!pPlayer)
		return;
	CPlayer::CLoginData *pData = &pPlayer->m_LoginData;
	if (!pData)
		return;
	if (!Next())
	{
		m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Login failed. Wrong password or username.");
		m_pGameServer->SaveWrongLogin(m_pGameServer->m_apPlayers[m_ClientID]->m_aWrongLogin);
		pData->m_LoginState = CPlayer::LOGIN_OFF;
		return;
	}
	if (m_pGameServer->CheckAccounts(GetInt(GetID("ID"))))
	{
		m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] This account is already logged in on this server.");
		pData->m_LoginState = CPlayer::LOGIN_OFF;
		return;
	}
	//#####################################################
	//       W A R N I N G
	// if you add a new var here
	// make sure to reset it in the Logout(); function
	// src/game/server/player.cpp
	//#####################################################

	//basic
	str_copy(pData->m_aUsername, GetText(GetID("Username")), sizeof(pData->m_aUsername));
	str_copy(pData->m_aPassword, GetText(GetID("Password")), sizeof(pData->m_aPassword));
	str_copy(pData->m_aAccountRegDate, GetText(GetID("RegisterDate")), sizeof(pData->m_aAccountRegDate));
	pData->m_AccountID = GetInt(GetID("ID"));

	//Accounts
	pData->m_IsModerator = GetInt(GetID("IsModerator"));
	pData->m_IsSuperModerator = GetInt(GetID("IsSuperModerator"));
	pData->m_IsSupporter = GetInt(GetID("IsSupporter"));
	pData->m_IsAccFrozen = GetInt(GetID("IsAccFrozen"));

	//city
	pData->m_level = GetInt(GetID("Level"));
	pData->m_xp = GetInt64(GetID("Exp"));
	pData->m_money = GetInt64(GetID("Money"));
	pData->m_shit = GetInt(GetID("Shit"));
	pData->m_GiftDelay = GetInt(GetID("LastGift"));

	m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Login success");

	pData->m_LoginState = CPlayer::LOGIN_DONE;
}

void CQueryLogin::OnData()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Next())
	{
		if (m_pGameServer->CheckAccounts(GetInt(GetID("ID"))))
		{
			m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] This account is already logged in on this server.");
		}
		else
		{
			if (g_Config.m_SvSpeedLogin)
			{
				if (m_pGameServer->m_apPlayers[m_ClientID])
				{
					m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] speed login success.");
					//m_pGameServer->m_apPlayers[m_ClientID]->ThreadLoginStart(this); //crashes the server still in work
				}
				return;
			}
			if (m_pGameServer->m_apPlayers[m_ClientID])
			{
				//#####################################################
				//       W A R N I N G
				// if you add a new var here
				// make sure to reset it in the Logout(); function
				// src/game/server/player.cpp
				//#####################################################
#if defined(CONF_DEBUG)
				dbg_msg("cBug","gamecontext.cpp '%s' CID=%d loading data...", m_pGameServer->Server()->ClientName(m_ClientID), m_ClientID);
#endif

				//basic
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccountLoginName, GetText(GetID("Username")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccountLoginName));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccountPassword, GetText(GetID("Password")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccountPassword));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccountRegDate, GetText(GetID("RegisterDate")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccountRegDate));
				m_pGameServer->m_apPlayers[m_ClientID]->m_AccountID = GetInt(GetID("ID"));

				//Accounts
				m_pGameServer->m_apPlayers[m_ClientID]->m_IsModerator = GetInt(GetID("IsModerator"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_IsSuperModerator = GetInt(GetID("IsSuperModerator"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_IsSupporter = GetInt(GetID("IsSupporter"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_IsAccFrozen = GetInt(GetID("IsAccFrozen"));

				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_LastLogoutIGN1, GetText(GetID("LastLogoutIGN1")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_LastLogoutIGN1));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_LastLogoutIGN2, GetText(GetID("LastLogoutIGN2")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_LastLogoutIGN2));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_LastLogoutIGN3, GetText(GetID("LastLogoutIGN3")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_LastLogoutIGN3));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_LastLogoutIGN4, GetText(GetID("LastLogoutIGN4")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_LastLogoutIGN4));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_LastLogoutIGN5, GetText(GetID("LastLogoutIGN5")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_LastLogoutIGN5));

				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aIP_1, GetText(GetID("IP_1")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aIP_1));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aIP_2, GetText(GetID("IP_2")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aIP_2));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aIP_3, GetText(GetID("IP_3")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aIP_3));

				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aClan1, GetText(GetID("Clan1")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aClan1));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aClan2, GetText(GetID("Clan2")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aClan2));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aClan3, GetText(GetID("Clan3")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aClan3));

				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccSkin, GetText(GetID("Skin")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccSkin));

				//ninjajetpack
				m_pGameServer->m_apPlayers[m_ClientID]->m_NinjaJetpackBought = GetInt(GetID("NinjaJetpackBought"));

				//spooky ghost
				m_pGameServer->m_apPlayers[m_ClientID]->m_SpookyGhost = GetInt(GetID("SpookyGhost"));

				//spawn weapons
				m_pGameServer->m_apPlayers[m_ClientID]->m_UseSpawnWeapons = GetInt(GetID("UseSpawnWeapons"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_SpawnWeaponShotgun = GetInt(GetID("SpawnWeaponShotgun"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_SpawnWeaponGrenade = GetInt(GetID("SpawnWeaponGrenade"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_SpawnWeaponRifle = GetInt(GetID("SpawnWeaponRifle"));

				//city
				m_pGameServer->m_apPlayers[m_ClientID]->m_level = GetInt(GetID("Level"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_xp = GetInt64(GetID("Exp"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_money = GetInt64(GetID("Money"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_shit = GetInt(GetID("Shit"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_GiftDelay = GetInt(GetID("LastGift"));

				//police
				m_pGameServer->m_apPlayers[m_ClientID]->m_PoliceRank = GetInt(GetID("PoliceRank"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_JailTime = GetInt(GetID("JailTime"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_EscapeTime = GetInt(GetID("EscapeTime"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_TaserLevel = GetInt(GetID("TaserLevel"));

				//pvp arena
				m_pGameServer->m_apPlayers[m_ClientID]->m_pvp_arena_tickets = GetInt(GetID("PvPArenaTickets"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_pvp_arena_games_played = GetInt(GetID("PvPArenaGames"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_pvp_arena_kills = GetInt(GetID("PvPArenaKills"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_pvp_arena_deaths = GetInt(GetID("PvPArenaDeaths"));

				//profiles (int)
				m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileStyle = GetInt(GetID("ProfileStyle"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileViews = GetInt(GetID("ProfileViews"));

				//profiles (str)
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileStatus, GetText(GetID("ProfileStatus")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileStatus));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileSkype, GetText(GetID("ProfileSkype")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileSkype));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileYoutube, GetText(GetID("ProfileYoutube")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileYoutube));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileEmail, GetText(GetID("ProfileEmail")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileEmail));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileHomepage, GetText(GetID("ProfileHomepage")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileHomepage));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileTwitter, GetText(GetID("ProfileTwitter")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileTwitter));

				//ascii animation
				m_pGameServer->m_apPlayers[m_ClientID]->m_AsciiViewsDefault = GetInt(GetID("AsciiViewsDefault"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_AsciiViewsProfile = GetInt(GetID("AsciiViewsProfile"));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiPublishState, GetText(GetID("AsciiState")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiPublishState));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame0, GetText(GetID("AsciiFrame0")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame0));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame1, GetText(GetID("AsciiFrame1")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame1));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame2, GetText(GetID("AsciiFrame2")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame2));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame3, GetText(GetID("AsciiFrame3")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame3));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame4, GetText(GetID("AsciiFrame4")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame4));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame5, GetText(GetID("AsciiFrame5")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame5));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame6, GetText(GetID("AsciiFrame6")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame6));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame7, GetText(GetID("AsciiFrame7")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame7));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame8, GetText(GetID("AsciiFrame8")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame8));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame9, GetText(GetID("AsciiFrame9")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame9));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame10, GetText(GetID("AsciiFrame10")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame10));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame11, GetText(GetID("AsciiFrame11")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame11));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame12, GetText(GetID("AsciiFrame12")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame12));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame13, GetText(GetID("AsciiFrame13")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame13));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame14, GetText(GetID("AsciiFrame14")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame14));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame15, GetText(GetID("AsciiFrame15")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAsciiFrame15));

				//Missiles
				m_pGameServer->m_apPlayers[m_ClientID]->m_homing_missiles_ammo = GetInt(GetID("HomingMissiles"));

				//Block
				m_pGameServer->m_apPlayers[m_ClientID]->m_BlockPoints = GetInt(GetID("BlockPoints"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_BlockPoints_Kills = GetInt(GetID("BlockKills"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_BlockPoints_Deaths = GetInt(GetID("BlockDeaths"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_BlockSkill = GetInt(GetID("BlockSkill"));

				//bomb
				m_pGameServer->m_apPlayers[m_ClientID]->m_BombGamesPlayed = GetInt(GetID("BombGamesPlayed"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_BombGamesWon = GetInt(GetID("BombGamesWon"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_BombBanTime = GetInt(GetID("BombBanTime"));

				//survival
				m_pGameServer->m_apPlayers[m_ClientID]->m_SurvivalKills = GetInt(GetID("SurvivalKills"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_SurvivalDeaths = GetInt(GetID("SurvivalDeaths"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_SurvivalWins = GetInt(GetID("SurvivalWins"));

				//instagib
				m_pGameServer->m_apPlayers[m_ClientID]->m_GrenadeKills = GetInt(GetID("GrenadeKills"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_GrenadeDeaths = GetInt(GetID("GrenadeDeaths"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_GrenadeSpree = GetInt(GetID("GrenadeSpree"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_GrenadeShots = GetInt(GetID("GrenadeShots"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_GrenadeShotsNoRJ = GetInt(GetID("GrenadeShotsNoRJ"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_GrenadeWins = GetInt(GetID("GrenadeWins")); //zCatch
				m_pGameServer->m_apPlayers[m_ClientID]->m_RifleKills = GetInt(GetID("RifleKills"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_RifleDeaths = GetInt(GetID("RifleDeaths"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_RifleDeaths = GetInt(GetID("RifleSpree"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_RifleShots = GetInt(GetID("RifleShots"));
				m_pGameServer->m_apPlayers[m_ClientID]->m_RifleWins = GetInt(GetID("RifleWins")); //zCatch
				if (g_Config.m_SvInstaScore) //load scoreboard scores
				{
					if (g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2) //gdm & zCatch grenade
					{
						m_pGameServer->m_apPlayers[m_ClientID]->m_Score = GetInt(GetID("GrenadeKills"));
					}
					else if (g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4) // idm & zCatch rifle
					{
						m_pGameServer->m_apPlayers[m_ClientID]->m_Score = GetInt(GetID("RifleKills"));
					}
				}
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aFngConfig, GetText(GetID("FngConfig")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aFngConfig));

				//ShowHide config
                /*
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aShowHideConfig, GetText(GetID("ShowHideConfig")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aShowHideConfig));
				m_pGameServer->ShowHideConfigCharToBool(m_ClientID); //update the actual bools used ingame
                */
			}

			//================================
			// ABORT LOGIN AFTER LOADING DATA
			//================================

			if (m_pGameServer->m_apPlayers[m_ClientID]->m_IsAccFrozen)
			{
				m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Login failed.(Account is frozen)");
				m_pGameServer->m_apPlayers[m_ClientID]->Logout();
				return;
			}

			if (GetInt(GetID("IsLoggedIn")) == 1)
			{
				m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Login failed. This account is already logged in on another server.");
				m_pGameServer->m_apPlayers[m_ClientID]->Logout(1); //Set IsLoggedIn to 1 to keep the account logged in on this logout
				return;
			}

			//==========================
			// Done loading stuff
			// Start to do something...
			//==========================


			m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Login successful.");

			//nobo spawn
			if (m_pGameServer->m_apPlayers[m_ClientID]->m_NoboSpawnStop > m_pGameServer->Server()->Tick())
			{
				m_pGameServer->m_apPlayers[m_ClientID]->m_IsNoboSpawn = false;
				m_pGameServer->m_apPlayers[m_ClientID]->m_NoboSpawnStop = 0;
				m_pGameServer->SendChatTarget(m_ClientID, "[NoboSpawn] Real spawn unlocked due to login.");
			}

			//jail
			if (m_pGameServer->m_apPlayers[m_ClientID]->m_JailTime)
			{
				if (m_pGameServer->m_apPlayers[m_ClientID]->GetCharacter())
				{
					vec2 JailPlayerSpawn = m_pGameServer->Collision()->GetRandomTile(TILE_JAIL);

					if (JailPlayerSpawn != vec2(-1, -1))
					{
						m_pGameServer->m_apPlayers[m_ClientID]->GetCharacter()->SetPosition(JailPlayerSpawn);
					}
					else //no jailplayer
					{
						m_pGameServer->SendChatTarget(m_ClientID, "No jail set.");
					}
				}
			}

			//auto joins
			if (m_pGameServer->m_apPlayers[m_ClientID]->m_aFngConfig[0] == '1') //auto fng join
			{
				if (!g_Config.m_SvAllowInsta)
				{
					m_pGameServer->SendChatTarget(m_ClientID, "[INSTA] fng autojoin failed because fng is deactivated by an admin.");
				}
				else
				{
					m_pGameServer->SendChatTarget(m_ClientID, "[INSTA] you automatically joined an fng game. (use '/fng' to change this setting)");
					m_pGameServer->JoinInstagib(5, true, m_ClientID);
				}
			}
			else if (m_pGameServer->m_apPlayers[m_ClientID]->m_aFngConfig[0] == '2') //auto boomfng join
			{
				if (!g_Config.m_SvAllowInsta)
				{
					m_pGameServer->SendChatTarget(m_ClientID, "[INSTA] boomfng autojoin failed because fng is deactivated by an admin.");
				}
				else
				{
					m_pGameServer->SendChatTarget(m_ClientID, "[INSTA] you automatically joined an boomfng game. (use '/fng' to change this setting)");
					m_pGameServer->JoinInstagib(4, true, m_ClientID);
				}
			}


			//account reset info
			if (!str_comp(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileEmail, "") && !str_comp(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileSkype, ""))
			{
				m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] set an '/profile email' or '/profile skype' to restore your password if you forget it.");
			}



			//========================================
			//LEAVE THIS CODE LAST!!!!
			//multiple server account protection stuff
			//========================================
			//works how it should but is possible crashing the server

			m_pGameServer->ExecuteSQLf("UPDATE `Accounts` SET `IsLoggedIn` = '%i', `LastLoginPort` = '%i' WHERE `ID` = '%i'", 1, g_Config.m_SvPort, m_pGameServer->m_apPlayers[m_ClientID]->m_AccountID);
		}
	}
	else
	{
		m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Login failed. Wrong password or username.");
		m_pGameServer->SaveWrongLogin(m_pGameServer->m_apPlayers[m_ClientID]->m_aWrongLogin);
	}
}

void CQueryChangePassword::OnData()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Next())
	{
		if (m_pGameServer->m_apPlayers[m_ClientID])
		{
			//m_pGameServer->m_apPlayers[m_ClientID]->ChangePassword();
			str_format(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccountPassword, sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccountPassword), "%s", m_pGameServer->m_apPlayers[m_ClientID]->m_aChangePassword);
			m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Changed password.");
		}
	}
	else
	{
		m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Wrong old password.");
	}
}

void CQuerySetPassword::OnData()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Next())
	{
		//send acc infos on found
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "[SQL] Updated '%s's password to [ %s ]", m_pGameServer->m_apPlayers[m_ClientID]->m_aSQLNameName, m_pGameServer->m_apPlayers[m_ClientID]->m_aSetPassword);
		m_pGameServer->SendChatTarget(m_ClientID, aBuf);

		//do the actual sql update
		char *pQueryBuf = sqlite3_mprintf("UPDATE Accounts SET Password='%q' WHERE Username='%q'", m_pGameServer->m_apPlayers[m_ClientID]->m_aSetPassword, m_pGameServer->m_apPlayers[m_ClientID]->m_aSQLNameName);
		CQuery *pQuery = new CQuery();
		pQuery->Query(m_pDatabase, pQueryBuf);
		sqlite3_free(pQueryBuf);
	}
	else
	{
		m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Invalid account.");
	}
}

bool CGameContext::CheckAccounts(int AccountID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!m_apPlayers[i])
			continue;

		if (m_apPlayers[i]->m_AccountID == AccountID)
			return true;
	}
	return false;
}

void CGameContext::Construct(int Resetting)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_Resetting = 0;
	m_pServer = 0;

	for(int i = 0; i < MAX_CLIENTS; i++)
		m_apPlayers[i] = 0;

	m_pController = 0;
	m_VoteCloseTime = 0;
	m_pVoteOptionFirst = 0;
	m_pVoteOptionLast = 0;
	m_NumVoteOptions = 0;
	m_LastMapVote = 0;
	//m_LockTeams = 0;

	m_Database = new CSql();

	if(Resetting==NO_RESET)
	{
		m_pVoteOptionHeap = new CHeap();
		m_pScore = 0;
		m_NumMutes = 0;
		m_pLetters = new CLetters(this);
	}
	m_ChatResponseTargetID = -1;
	m_aDeleteTempfile[0] = 0;
}

CGameContext::CGameContext(int Resetting)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	Construct(Resetting);
}

CGameContext::CGameContext()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	Construct(NO_RESET);
}

CGameContext::~CGameContext()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for(int i = 0; i < MAX_CLIENTS; i++)
		delete m_apPlayers[i];
	if(!m_Resetting)
		delete m_pVoteOptionHeap;

	//m_Database->~CSql();
	//delete m_Database;

	if(m_pScore)
		delete m_pScore;
}

void CGameContext::Clear()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CHeap *pVoteOptionHeap = m_pVoteOptionHeap;
	CVoteOptionServer *pVoteOptionFirst = m_pVoteOptionFirst;
	CVoteOptionServer *pVoteOptionLast = m_pVoteOptionLast;
	int NumVoteOptions = m_NumVoteOptions;
	CTuningParams Tuning = m_Tuning;

	m_Resetting = true;
	this->~CGameContext();
	mem_zero(this, sizeof(*this));
	new (this) CGameContext(RESET);

	m_pVoteOptionHeap = pVoteOptionHeap;
	m_pVoteOptionFirst = pVoteOptionFirst;
	m_pVoteOptionLast = pVoteOptionLast;
	m_NumVoteOptions = NumVoteOptions;
	m_Tuning = Tuning;
}


class CCharacter *CGameContext::GetPlayerChar(int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if(ClientID < 0 || ClientID >= MAX_CLIENTS || !m_apPlayers[ClientID])
		return 0;
	return m_apPlayers[ClientID]->GetCharacter();
}

void CGameContext::CreateDamageInd(vec2 Pos, float Angle, int Amount, int64_t Mask)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	float a = 3 * 3.14159f / 2 + Angle;
	//float a = get_angle(dir);
	float s = a-pi/3;
	float e = a+pi/3;
	for(int i = 0; i < Amount; i++)
	{
		float f = mix(s, e, float(i+1)/float(Amount+2));
		CNetEvent_DamageInd *pEvent = (CNetEvent_DamageInd *)m_Events.Create(NETEVENTTYPE_DAMAGEIND, sizeof(CNetEvent_DamageInd), Mask);
		if(pEvent)
		{
			pEvent->m_X = (int)Pos.x;
			pEvent->m_Y = (int)Pos.y;
			pEvent->m_Angle = (int)(f*256.0f);
		}
	}
}

void CGameContext::CreateHammerHit(vec2 Pos, int64_t Mask)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// create the event
	CNetEvent_HammerHit *pEvent = (CNetEvent_HammerHit *)m_Events.Create(NETEVENTTYPE_HAMMERHIT, sizeof(CNetEvent_HammerHit), Mask);
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
	}
}


void CGameContext::CreateExplosion(vec2 Pos, int Owner, int Weapon, bool NoDamage, int ActivatedTeam, int64_t Mask)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// create the event
	CNetEvent_Explosion *pEvent = (CNetEvent_Explosion *)m_Events.Create(NETEVENTTYPE_EXPLOSION, sizeof(CNetEvent_Explosion), Mask);
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
	}
/*
	if (!NoDamage)
	{
	*/
		// deal damage
		CCharacter *apEnts[MAX_CLIENTS];
		float Radius = 135.0f;
		float InnerRadius = 48.0f;
		int Num = m_World.FindEntities(Pos, Radius, (CEntity**)apEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
		for(int i = 0; i < Num; i++)
		{
			vec2 Diff = apEnts[i]->m_Pos - Pos;
			vec2 ForceDir(0,1);
			float l = length(Diff);
			if(l)
				ForceDir = normalize(Diff);
			l = 1-clamp((l-InnerRadius)/(Radius-InnerRadius), 0.0f, 1.0f);
			float Strength;
			if (Owner == -1 || !m_apPlayers[Owner] || !m_apPlayers[Owner]->m_TuneZone)
				Strength = Tuning()->m_ExplosionStrength;
			else
				Strength = TuningList()[m_apPlayers[Owner]->m_TuneZone].m_ExplosionStrength;

			float Dmg = Strength * l;
			if((int)Dmg)
				if((GetPlayerChar(Owner) ? !(GetPlayerChar(Owner)->m_Hit&CCharacter::DISABLE_HIT_GRENADE) : g_Config.m_SvHit || NoDamage) || Owner == apEnts[i]->GetPlayer()->GetCID())
				{
					if(Owner != -1 && apEnts[i]->IsAlive() && !apEnts[i]->CanCollide(Owner)) continue;
					if(Owner == -1 && ActivatedTeam != -1 && apEnts[i]->IsAlive() && apEnts[i]->Team() != ActivatedTeam) continue;
					apEnts[i]->TakeDamage(ForceDir*Dmg*2, (int)Dmg, Owner, Weapon);
					if(GetPlayerChar(Owner) ? GetPlayerChar(Owner)->m_Hit&CCharacter::DISABLE_HIT_GRENADE : !g_Config.m_SvHit || NoDamage) break;
				}
		}
	//}
}

/*
void create_smoke(vec2 Pos)
{
	// create the event
	EV_EXPLOSION *pEvent = (EV_EXPLOSION *)events.create(EVENT_SMOKE, sizeof(EV_EXPLOSION));
	if(pEvent)
	{
		pEvent->x = (int)Pos.x;
		pEvent->y = (int)Pos.y;
	}
}*/

void CGameContext::CreatePlayerSpawn(vec2 Pos, int64_t Mask)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// create the event
	CNetEvent_Spawn *ev = (CNetEvent_Spawn *)m_Events.Create(NETEVENTTYPE_SPAWN, sizeof(CNetEvent_Spawn), Mask);
	if(ev)
	{
		ev->m_X = (int)Pos.x;
		ev->m_Y = (int)Pos.y;
		//SendChatTarget(SpamProtectionClientID, "Spauuuuuuuuuuuuuuuuun!");
	}
}

void CGameContext::CreateDeath(vec2 Pos, int ClientID, int64_t Mask)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// create the event
	CNetEvent_Death *pEvent = (CNetEvent_Death *)m_Events.Create(NETEVENTTYPE_DEATH, sizeof(CNetEvent_Death), Mask);
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_ClientID = ClientID;
	}
}

void CGameContext::CreateSound(vec2 Pos, int Sound, int64_t Mask)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Sound < 0)
		return;

	// create a sound
	CNetEvent_SoundWorld *pEvent = (CNetEvent_SoundWorld *)m_Events.Create(NETEVENTTYPE_SOUNDWORLD, sizeof(CNetEvent_SoundWorld), Mask);
	if(pEvent)
	{
		pEvent->m_X = (int)Pos.x;
		pEvent->m_Y = (int)Pos.y;
		pEvent->m_SoundID = Sound;
	}
}

void CGameContext::CreateSoundGlobal(int Sound, int Target)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Sound < 0)
		return;

	CNetMsg_Sv_SoundGlobal Msg;
	Msg.m_SoundID = Sound;
	if(Target == -2)
		Server()->SendPackMsg(&Msg, MSGFLAG_NOSEND, -1);
	else
	{
		int Flag = MSGFLAG_VITAL;
		if(Target != -1)
			Flag |= MSGFLAG_NORECORD;
		Server()->SendPackMsg(&Msg, Flag, Target);
	}
}

void CGameContext::CallVote(int ClientID, const char *aDesc, const char *aCmd, const char *pReason, const char *aChatmsg, bool IsDDPPVetoVote)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// check if a vote is already running
	if(m_VoteCloseTime)
		return;

	m_IsDDPPVetoVote = IsDDPPVetoVote; // Veto votes only pass if nobody voted agianst it (vote yes doesnt count at all so if nobody votes yes or no the vote will pass)

	int64 Now = Server()->Tick();
	if (ClientID == -1) //Server vote
	{
		SendChat(-1, CGameContext::CHAT_ALL, aChatmsg);
		StartVote(aDesc, aCmd, pReason);
		m_VoteCreator = ClientID;
		m_LastVoteCallAll = Now;
	}
	else
	{
		CPlayer *pPlayer = m_apPlayers[ClientID];
		if (!pPlayer)
			return;

		SendChat(-1, CGameContext::CHAT_ALL, aChatmsg);
		StartVote(aDesc, aCmd, pReason);
		pPlayer->m_Vote = 1;
		pPlayer->m_VotePos = m_VotePos = 1;
		m_VoteCreator = ClientID;
		pPlayer->m_LastVoteCall = Now;
		m_LastVoteCallAll = Now;
	}
}

void CGameContext::SendChatTarget(int To, const char *pText)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CNetMsg_Sv_Chat Msg;
	Msg.m_Team = 0;
	Msg.m_ClientID = -1;
	Msg.m_pMessage = pText;
	if(g_Config.m_SvDemoChat)
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, To);
	else
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, To);
}

void CGameContext::SendChatTeam(int Team, const char *pText)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for(int i = 0; i<MAX_CLIENTS; i++)
		if(((CGameControllerDDRace*)m_pController)->m_Teams.m_Core.Team(i) == Team)
			SendChatTarget(i, pText);
}

void CGameContext::SendChat(int ChatterClientID, int Team, const char *pText, int SpamProtectionClientID, int ToClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if(SpamProtectionClientID >= 0 && SpamProtectionClientID < MAX_CLIENTS)
	{
		if(ProcessSpamProtection(SpamProtectionClientID))
		{
			SendChatTarget(SpamProtectionClientID, "Stop spamming!");
			//SendChatTarget(SpamProtectionClientID, pText);
			return;
		}
	}

	char aBuf[256], aText[256];
	str_copy(aText, pText, sizeof(aText));
	if(ChatterClientID >= 0 && ChatterClientID < MAX_CLIENTS)
		str_format(aBuf, sizeof(aBuf), "%d:%d:%s: %s", ChatterClientID, Team, Server()->ClientName(ChatterClientID), aText);
	else if(ChatterClientID == -2)
	{
		str_format(aBuf, sizeof(aBuf), "### %s", aText);
		str_copy(aText, aBuf, sizeof(aText));
		ChatterClientID = -1;
	}
	else
		str_format(aBuf, sizeof(aBuf), "*** %s", aText);
	Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, Team!=CHAT_ALL?"teamchat":"chat", aBuf);

	if(Team == CHAT_ALL)
	{
		CNetMsg_Sv_Chat Msg;
		Msg.m_Team = 0;
		Msg.m_ClientID = ChatterClientID;
		Msg.m_pMessage = aText;

		// pack one for the recording only
		if(g_Config.m_SvDemoChat)
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NOSEND, -1);

		// send to the clients
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_apPlayers[i] != 0) {
				if(!m_apPlayers[i]->m_DND)
					Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, i);
			}
		}
	}
	else if (Team == CHAT_TO_ONE_CLIENT)
	{
		CNetMsg_Sv_Chat Msg;
		Msg.m_Team = 0;
		Msg.m_ClientID = ChatterClientID;
		Msg.m_pMessage = aText;

		// pack one for the recording only
		if (g_Config.m_SvDemoChat)
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NOSEND, -1);

		// send to the clients
		if (!m_apPlayers[ToClientID]->m_DND)
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, ToClientID);
	}
	else
	{
		CTeamsCore * Teams = &((CGameControllerDDRace*)m_pController)->m_Teams.m_Core;
		CNetMsg_Sv_Chat Msg;
		Msg.m_Team = 1;
		Msg.m_ClientID = ChatterClientID;
		Msg.m_pMessage = aText;

		// pack one for the recording only
		if(g_Config.m_SvDemoChat)
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NOSEND, -1);

		// send to the clients
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_apPlayers[i] != 0) {
				if(Team == CHAT_SPEC) {
					if(m_apPlayers[i]->GetTeam() == CHAT_SPEC) {
						Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, i);
					}
				} else {
					if(Teams->Team(i) == Team && m_apPlayers[i]->GetTeam() != CHAT_SPEC) {
						Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, i);
					}
				}
			}
		}
	}
}

void CGameContext::SendEmoticon(int ClientID, int Emoticon)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CNetMsg_Sv_Emoticon Msg;
	Msg.m_ClientID = ClientID;
	if (m_apPlayers[ClientID]->m_SpookyGhostActive)
		Msg.m_Emoticon = 7; // ghost emote only
	else
		Msg.m_Emoticon = Emoticon;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
}

void CGameContext::SendWeaponPickup(int ClientID, int Weapon)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CNetMsg_Sv_WeaponPickup Msg;
	Msg.m_Weapon = Weapon;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}


void CGameContext::SendBroadcast(const char *pText, int ClientID, int importance, bool supermod)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (ClientID == -1) //classical rcon broadcast
	{
		CNetMsg_Sv_Broadcast Msg;
		Msg.m_pMessage = pText; //default broadcast
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);

		//set important broadcast for all
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (m_apPlayers[i])
			{
				m_apPlayers[i]->m_LastBroadcastImportance = 1;
				m_apPlayers[i]->m_LastBroadcast = Server()->Tick();
				//dbg_msg("broadcast","[%s] importance is %d", Server()->ClientName(i), m_apPlayers[i]->m_LastBroadcastImportance);
			}
		}
	}
	else //non rcon broadcast
	{
		if (!m_apPlayers[ClientID]) //ddpp added by ChillerDragon because we handel player vriables here and idk why we should send it to non exsisting players anyways
		{
			//dbg_msg("cBug", "returned id=%d", ClientID);
			return;
		}

		if (m_apPlayers[ClientID]->m_LastBroadcastImportance) //only care if last broadcast was important
		{
			if (m_apPlayers[ClientID]->m_LastBroadcast > Server()->Tick() - Server()->TickSpeed() * 6) //dont overwrite broadcasts send 6 seconds ago
			{
				if (importance == 0)
				{
					//SendChat(-1, CGameContext::CHAT_ALL, "broadcast got ignored");
					return;
				}
				else if (importance == 1 && supermod && m_apPlayers[ClientID]->m_LastBroadcastImportance == 2) //supermoderators can't overwrite broadcaste with lvl 2 importance
				{
					//SendChat(-1, CGameContext::CHAT_ALL, "broadcast got ignored");
					return;
				}
			}
		}

		//dbg_msg("cBug", "curr_imp[%d] last_imp[%d]     curr_t[%d] last_t[%d]", importance, m_apPlayers[ClientID]->m_LastBroadcastImportance, Server()->Tick(), m_apPlayers[ClientID]->m_LastBroadcast);

		CNetMsg_Sv_Broadcast Msg;
		//if (supermod)
		//{
		//	if (m_iBroadcastDelay) { return; } //only send supermod broadcasts if no other broadcast recencly was sent
		//									   //char aText[256];																//supermod broadcast with advertisement attached
		//									   //str_format(aText, sizeof(aText), "%s\n[%s]", pText, aBroadcastMSG);			//supermod broadcast with advertisement attached
		//									   //Msg.m_pMessage = aText;														//supermod broadcast with advertisement attached

		//	Msg.m_pMessage = pText; //default broadcast (comment this out if you want to use the adveertisement string)
		//	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
		//}
		//else
		//{
			Msg.m_pMessage = pText; //default broadcast
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);

			m_iBroadcastDelay = Server()->TickSpeed() * 4; //set 4 second delay after normal broadcasts before supermods can send a new one
		//}

		m_apPlayers[ClientID]->m_LastBroadcast = Server()->Tick();
		m_apPlayers[ClientID]->m_LastBroadcastImportance = importance;
	}
}

void CGameContext::StartVote(const char *pDesc, const char *pCommand, const char *pReason)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// reset votes
	m_VoteEnforce = VOTE_ENFORCE_UNKNOWN;
	m_VoteEnforcer = -1;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			m_apPlayers[i]->m_Vote = 0;
			m_apPlayers[i]->m_VotePos = 0;
		}
	}

	// start vote
	m_VoteCloseTime = time_get() + time_freq() * g_Config.m_SvVoteTime;
	str_copy(m_aVoteDescription, pDesc, sizeof(m_aVoteDescription));
	str_copy(m_aVoteCommand, pCommand, sizeof(m_aVoteCommand));
	str_copy(m_aVoteReason, pReason, sizeof(m_aVoteReason));
	SendVoteSet(-1);
	m_VoteUpdate = true;
}


void CGameContext::EndVote()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_VoteCloseTime = 0;
	SendVoteSet(-1);
}

void CGameContext::SendVoteSet(int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CNetMsg_Sv_VoteSet Msg;
	if(m_VoteCloseTime)
	{
		Msg.m_Timeout = (m_VoteCloseTime-time_get())/time_freq();
		Msg.m_pDescription = m_aVoteDescription;
		Msg.m_pReason = m_aVoteReason;
	}
	else
	{
		Msg.m_Timeout = 0;
		Msg.m_pDescription = "";
		Msg.m_pReason = "";
	}
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGameContext::SendVoteStatus(int ClientID, int Total, int Yes, int No)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Total > VANILLA_MAX_CLIENTS && m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_ClientVersion <= VERSION_DDRACE)
	{
		Yes = float(Yes) * VANILLA_MAX_CLIENTS / float(Total);
		No = float(No) * VANILLA_MAX_CLIENTS / float(Total);
		Total = VANILLA_MAX_CLIENTS;
	}

	CNetMsg_Sv_VoteStatus Msg = {0};
	Msg.m_Total = Total;
	Msg.m_Yes = Yes;
	Msg.m_No = No;
	Msg.m_Pass = Total - (Yes+No);

	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);

}

void CGameContext::AbortVoteKickOnDisconnect(int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if(m_VoteCloseTime && ((!str_comp_num(m_aVoteCommand, "kick ", 5) && str_toint(&m_aVoteCommand[5]) == ClientID) ||
		(!str_comp_num(m_aVoteCommand, "set_team ", 9) && str_toint(&m_aVoteCommand[9]) == ClientID)))
		m_VoteCloseTime = -1;
}


void CGameContext::CheckPureTuning()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// might not be created yet during start up
	if(!m_pController)
		return;

	if(	str_comp(m_pController->m_pGameType, "DM")==0 ||
		str_comp(m_pController->m_pGameType, "TDM")==0 ||
		str_comp(m_pController->m_pGameType, "CTF")==0)
	{
		CTuningParams p;
		if(mem_comp(&p, &m_Tuning, sizeof(p)) != 0)
		{
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "resetting tuning due to pure server");
			m_Tuning = p;
		}
	}
}

void CGameContext::SendTuningParams(int ClientID, int Zone)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (ClientID == -1)
	{
			for(int i = 0; i < MAX_CLIENTS; ++i)
			{
				if (m_apPlayers[i])
				{
					if(m_apPlayers[i]->GetCharacter())
					{
						if (m_apPlayers[i]->GetCharacter()->m_TuneZone == Zone)
							SendTuningParams(i, Zone);
					}
					else if (m_apPlayers[i]->m_TuneZone == Zone)
					{
						SendTuningParams(i, Zone);
					}
				}
			}
			return;
	}

	CheckPureTuning();

	CMsgPacker Msg(NETMSGTYPE_SV_TUNEPARAMS);
	int *pParams = 0;
	if (Zone == 0)
		pParams = (int *)&m_Tuning;
	else
		pParams = (int *)&(m_TuningList[Zone]);

	unsigned int last = sizeof(m_Tuning)/sizeof(int);
	if (m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_ClientVersion < VERSION_DDNET_EXTRATUNES)
		last = 33;
	else if (m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_ClientVersion < VERSION_DDNET_HOOKDURATION_TUNE)
		last = 37;
	else if (m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_ClientVersion < VERSION_DDNET_FIREDELAY_TUNE)
		last = 38;

	for(unsigned i = 0; i < last; i++)
		{
			if (m_apPlayers[ClientID] && m_apPlayers[ClientID]->GetCharacter())
			{
				if((i==31) // collision
				&& (m_apPlayers[ClientID]->GetCharacter()->NeededFaketuning() & FAKETUNE_SOLO
				 || m_apPlayers[ClientID]->GetCharacter()->NeededFaketuning() & FAKETUNE_NOCOLL))
				{
					Msg.AddInt(0);
				}
				else if((i==32) // hooking
				&& (m_apPlayers[ClientID]->GetCharacter()->NeededFaketuning() & FAKETUNE_SOLO
				 || m_apPlayers[ClientID]->GetCharacter()->NeededFaketuning() & FAKETUNE_NOHOOK))
				{
					Msg.AddInt(0);
				}
				else if((i==3) // ground jump impulse
				&& m_apPlayers[ClientID]->GetCharacter()->NeededFaketuning() & FAKETUNE_NOJUMP)
				{
					Msg.AddInt(0);
				}
				else if((i==33) // jetpack
				&& !(m_apPlayers[ClientID]->GetCharacter()->NeededFaketuning() & FAKETUNE_JETPACK))
				{
					Msg.AddInt(0);
				}
				else if((i==12) // gravity for 420 trolling
				&& m_apPlayers[ClientID]->m_TROLL420)
				{
					Msg.AddInt(-1000000);
				}
				else
				{
					Msg.AddInt(pParams[i]);
				}
			}
			else
				Msg.AddInt(pParams[i]); // if everything is normal just send true tunings
		}
		Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientID);
}
/*
void CGameContext::SwapTeams()
{
	if(!m_pController->IsTeamplay())
		return;

	SendChat(-1, CGameContext::CHAT_ALL, "Teams were swapped");

	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
			m_apPlayers[i]->SetTeam(m_apPlayers[i]->GetTeam()^1, false);
	}

	(void)m_pController->CheckTeamBalance();
}
*/
void CGameContext::OnTick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	// check tuning
	CheckPureTuning();

	// copy tuning
	m_World.m_Core.m_Tuning[0] = m_Tuning;
	m_World.Tick();

	//if(world.paused) // make sure that the game object always updates
	m_pController->Tick();

	// process sql queries
	m_Database->Tick();

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			// send vote options
			ProgressVoteOptions(i);

			m_apPlayers[i]->Tick();
			m_apPlayers[i]->PostTick();
		}
	}

	// update voting
	if(m_VoteCloseTime)
	{
		// abort the kick-vote on player-leave
		if(m_VoteCloseTime == -1)
		{
			SendChat(-1, CGameContext::CHAT_ALL, "Vote aborted");
			EndVote();
		}
		else
		{
			int Total = 0, Yes = 0, No = 0;
			bool Veto = false, VetoStop = false;
			if(m_VoteUpdate)
			{
				// count votes
				char aaBuf[MAX_CLIENTS][NETADDR_MAXSTRSIZE] = {{0}};
				for(int i = 0; i < MAX_CLIENTS; i++)
					if(m_apPlayers[i])
						Server()->GetClientAddr(i, aaBuf[i], NETADDR_MAXSTRSIZE);
				bool aVoteChecked[MAX_CLIENTS] = {0};
				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					//if(!m_apPlayers[i] || m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS || aVoteChecked[i])	// don't count in votes by spectators
					if(!m_apPlayers[i] ||
							(g_Config.m_SvSpectatorVotes == 0 &&
									m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS) ||
									aVoteChecked[i])	// don't count in votes by spectators if the admin doesn't want it
						continue;

					if((m_VoteKick || m_VoteSpec) && ((!m_apPlayers[i] || m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS)))
						continue;

					if (m_VoteCreator != -1) // Ignore Server Votes
					{
						if (GetPlayerChar(m_VoteCreator) && GetPlayerChar(i) &&
							GetPlayerChar(m_VoteCreator)->Team() != GetPlayerChar(i)->Team())
							continue;
					}

					if(m_apPlayers[i]->m_Afk && i != m_VoteCreator)
						continue;

					int ActVote = m_apPlayers[i]->m_Vote;
					int ActVotePos = m_apPlayers[i]->m_VotePos;

					// check for more players with the same ip (only use the vote of the one who voted first)
					for(int j = i+1; j < MAX_CLIENTS; ++j)
					{
						if(!m_apPlayers[j] || aVoteChecked[j] || str_comp(aaBuf[j], aaBuf[i]))
							continue;

						aVoteChecked[j] = true;
						if(m_apPlayers[j]->m_Vote && (!ActVote || ActVotePos > m_apPlayers[j]->m_VotePos))
						{
							ActVote = m_apPlayers[j]->m_Vote;
							ActVotePos = m_apPlayers[j]->m_VotePos;
						}
					}

					Total++;
					if(ActVote > 0)
						Yes++;
					else if(ActVote < 0)
						No++;

					// veto right for players with much progress and who're not afk
					if(!m_VoteKick && !m_VoteSpec && !m_apPlayers[i]->m_Afk &&
						m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS &&
						m_apPlayers[i]->GetCharacter() &&
						m_apPlayers[i]->GetCharacter()->m_DDRaceState == DDRACE_STARTED &&
						g_Config.m_SvVoteVetoTime &&
						(Server()->Tick() - m_apPlayers[i]->GetCharacter()->m_StartTime) / (Server()->TickSpeed() * 60) > g_Config.m_SvVoteVetoTime)
					{
						if(ActVote == 0)
							Veto = true;
						else if(ActVote < 0)
							VetoStop = true;
					}
				}

				if(g_Config.m_SvVoteMaxTotal && Total > g_Config.m_SvVoteMaxTotal &&
						(m_VoteKick || m_VoteSpec))
					Total = g_Config.m_SvVoteMaxTotal;

				if((Yes > Total / (100.0 / g_Config.m_SvVoteYesPercentage)) && !Veto && !m_IsDDPPVetoVote)
					m_VoteEnforce = VOTE_ENFORCE_YES;
				else if(No >= Total - Total / (100.0 / g_Config.m_SvVoteYesPercentage))
					m_VoteEnforce = VOTE_ENFORCE_NO;

				if(VetoStop)
					m_VoteEnforce = VOTE_ENFORCE_NO;
				else if (m_IsDDPPVetoVote && No)
					m_VoteEnforce = VOTE_ENFORCE_NO;

				m_VoteWillPass = Yes > (Yes + No) / (100.0 / g_Config.m_SvVoteYesPercentage);
			}

			if(time_get() > m_VoteCloseTime && !g_Config.m_SvVoteMajority)
				m_VoteEnforce = (m_VoteWillPass && !Veto && !m_IsDDPPVetoVote) ? VOTE_ENFORCE_YES : VOTE_ENFORCE_NO;
			if (time_get() > m_VoteCloseTime && m_IsDDPPVetoVote && !No) // pass vote even if nobody votes yes
				m_VoteEnforce = VOTE_ENFORCE_YES;

			if(m_VoteEnforce == VOTE_ENFORCE_YES)
			{
				Server()->SetRconCID(IServer::RCON_CID_VOTE);
				Console()->ExecuteLine(m_aVoteCommand);
				Server()->SetRconCID(IServer::RCON_CID_SERV);
				EndVote();
				if (m_IsDDPPVetoVote)
				{
					SendChat(-1, CGameContext::CHAT_ALL, "Vote passed because nobody used veto (Veto Vote)");
				}
				else
				{
					SendChat(-1, CGameContext::CHAT_ALL, "Vote passed");
				}

				if (m_VoteCreator != -1) // Ignore server votes
				{
					if (m_apPlayers[m_VoteCreator])
						m_apPlayers[m_VoteCreator]->m_LastVoteCall = 0;
				}
			}
			else if(m_VoteEnforce == VOTE_ENFORCE_YES_ADMIN)
			{
				char aBuf[64];
				str_format(aBuf, sizeof(aBuf),"Vote passed enforced by server moderator");
				Console()->ExecuteLine(m_aVoteCommand, m_VoteEnforcer);
				SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				EndVote();
			}
			else if(m_VoteEnforce == VOTE_ENFORCE_NO_ADMIN)
			{
				char aBuf[64];
				str_format(aBuf, sizeof(aBuf),"Vote failed enforced by server moderator");
				EndVote();
				SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
			//else if(m_VoteEnforce == VOTE_ENFORCE_NO || time_get() > m_VoteCloseTime)
			else if(m_VoteEnforce == VOTE_ENFORCE_NO || (time_get() > m_VoteCloseTime && g_Config.m_SvVoteMajority))
			{
				EndVote();
				if(VetoStop || (m_VoteWillPass && Veto))
					SendChat(-1, CGameContext::CHAT_ALL, "Vote failed because of veto. Find an empty server instead");
				else if (m_IsDDPPVetoVote)
					SendChat(-1, CGameContext::CHAT_ALL, "Vote failed because someone voted agianst it. (Veto Vote)");
				else
					SendChat(-1, CGameContext::CHAT_ALL, "Vote failed");
			}
			else if(m_VoteUpdate)
			{
				m_VoteUpdate = false;
				for(int i = 0; i < MAX_CLIENTS; ++i)
					if(Server()->ClientIngame(i))
						SendVoteStatus(i, Total, Yes, No);
			}
		}
	}
	for(int i = 0; i < m_NumMutes; i++)
	{
		if(m_aMutes[i].m_Expire <= Server()->Tick())
		{
			m_NumMutes--;
			m_aMutes[i] = m_aMutes[m_NumMutes];
		}
	}

	if(Server()->Tick() % (g_Config.m_SvAnnouncementInterval * Server()->TickSpeed() * 60) == 0)
	{
		char *Line = ((CServer *) Server())->GetAnnouncementLine(g_Config.m_SvAnnouncementFileName);
		if(Line)
			SendChat(-1, CGameContext::CHAT_ALL, Line);
	}

	if(Collision()->m_NumSwitchers > 0)
		for (int i = 0; i < Collision()->m_NumSwitchers+1; ++i)
		{
			for (int j = 0; j < MAX_CLIENTS; ++j)
			{
				if(Collision()->m_pSwitchers[i].m_EndTick[j] <= Server()->Tick() && Collision()->m_pSwitchers[i].m_Type[j] == TILE_SWITCHTIMEDOPEN)
				{
					Collision()->m_pSwitchers[i].m_Status[j] = false;
					Collision()->m_pSwitchers[i].m_EndTick[j] = 0;
					Collision()->m_pSwitchers[i].m_Type[j] = TILE_SWITCHCLOSE;
				}
				else if(Collision()->m_pSwitchers[i].m_EndTick[j] <= Server()->Tick() && Collision()->m_pSwitchers[i].m_Type[j] == TILE_SWITCHTIMEDCLOSE)
				{
					Collision()->m_pSwitchers[i].m_Status[j] = true;
					Collision()->m_pSwitchers[i].m_EndTick[j] = 0;
					Collision()->m_pSwitchers[i].m_Type[j] = TILE_SWITCHOPEN;
				}
			}
		}


	if (m_CreateShopBot && (Server()->Tick() % 50 == 0))
	{
		CreateNewDummy(99);//shop bot
		m_CreateShopBot = false;
	}

	DDPP_Tick();

#ifdef CONF_DEBUG
	if(g_Config.m_DbgDummies)
	{
		for(int i = 0; i < g_Config.m_DbgDummies ; i++)
		{
			CNetObj_PlayerInput Input = {0};
			Input.m_Direction = (i&1)?-1:1;
			m_apPlayers[MAX_CLIENTS-i-1]->OnPredictedInput(&Input);
		}
	}
#endif
}

// Server hooks
void CGameContext::OnClientDirectInput(int ClientID, void *pInput)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if(!m_World.m_Paused)
		m_apPlayers[ClientID]->OnDirectInput((CNetObj_PlayerInput *)pInput);
}

void CGameContext::OnClientPredictedInput(int ClientID, void *pInput)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if(!m_World.m_Paused)
		m_apPlayers[ClientID]->OnPredictedInput((CNetObj_PlayerInput *)pInput);
}

struct CVoteOptionServer *CGameContext::GetVoteOption(int Index)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CVoteOptionServer *pCurrent;
	for (pCurrent = m_pVoteOptionFirst;
			Index > 0 && pCurrent;
			Index--, pCurrent = pCurrent->m_pNext);

	if (Index > 0)
		return 0;
	return pCurrent;
}

void CGameContext::ProgressVoteOptions(int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPl = m_apPlayers[ClientID];

	if (pPl->m_SendVoteIndex == -1)
		return; // we didn't start sending options yet

	if (pPl->m_SendVoteIndex > m_NumVoteOptions)
		return; // shouldn't happen / fail silently

	int VotesLeft = m_NumVoteOptions - pPl->m_SendVoteIndex;
	int NumVotesToSend = min(g_Config.m_SvSendVotesPerTick, VotesLeft);

	if (!VotesLeft)
	{
		// player has up to date vote option list
		return;
	}

	// build vote option list msg
	int CurIndex = 0;

	CNetMsg_Sv_VoteOptionListAdd OptionMsg;
	OptionMsg.m_pDescription0 = "";
	OptionMsg.m_pDescription1 = "";
	OptionMsg.m_pDescription2 = "";
	OptionMsg.m_pDescription3 = "";
	OptionMsg.m_pDescription4 = "";
	OptionMsg.m_pDescription5 = "";
	OptionMsg.m_pDescription6 = "";
	OptionMsg.m_pDescription7 = "";
	OptionMsg.m_pDescription8 = "";
	OptionMsg.m_pDescription9 = "";
	OptionMsg.m_pDescription10 = "";
	OptionMsg.m_pDescription11 = "";
	OptionMsg.m_pDescription12 = "";
	OptionMsg.m_pDescription13 = "";
	OptionMsg.m_pDescription14 = "";

	// get current vote option by index
	CVoteOptionServer *pCurrent = GetVoteOption(pPl->m_SendVoteIndex);

	while(CurIndex < NumVotesToSend && pCurrent != NULL)
	{
		switch(CurIndex)
		{
			case 0: OptionMsg.m_pDescription0 = pCurrent->m_aDescription; break;
			case 1: OptionMsg.m_pDescription1 = pCurrent->m_aDescription; break;
			case 2: OptionMsg.m_pDescription2 = pCurrent->m_aDescription; break;
			case 3: OptionMsg.m_pDescription3 = pCurrent->m_aDescription; break;
			case 4: OptionMsg.m_pDescription4 = pCurrent->m_aDescription; break;
			case 5: OptionMsg.m_pDescription5 = pCurrent->m_aDescription; break;
			case 6: OptionMsg.m_pDescription6 = pCurrent->m_aDescription; break;
			case 7: OptionMsg.m_pDescription7 = pCurrent->m_aDescription; break;
			case 8: OptionMsg.m_pDescription8 = pCurrent->m_aDescription; break;
			case 9: OptionMsg.m_pDescription9 = pCurrent->m_aDescription; break;
			case 10: OptionMsg.m_pDescription10 = pCurrent->m_aDescription; break;
			case 11: OptionMsg.m_pDescription11 = pCurrent->m_aDescription; break;
			case 12: OptionMsg.m_pDescription12 = pCurrent->m_aDescription; break;
			case 13: OptionMsg.m_pDescription13 = pCurrent->m_aDescription; break;
			case 14: OptionMsg.m_pDescription14 = pCurrent->m_aDescription; break;
		}

		CurIndex++;
		pCurrent = pCurrent->m_pNext;
	}

	// send msg
	OptionMsg.m_NumOptions = NumVotesToSend;
	Server()->SendPackMsg(&OptionMsg, MSGFLAG_VITAL, ClientID);

	pPl->m_SendVoteIndex += NumVotesToSend;
}

void CGameContext::OnClientEnter(int ClientID, bool silent)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	if (IsDDPPgametype("survival"))
	{
		SetPlayerSurvival(ClientID, 1);
	}
	else if (IsDDPPgametype("vanilla"))
	{
		if (m_apPlayers[ClientID])
		{
			m_apPlayers[ClientID]->m_IsVanillaDmg = true;
			m_apPlayers[ClientID]->m_IsVanillaWeapons = true;
			m_apPlayers[ClientID]->m_IsVanillaCompetetive = true;
		}
	}
	else if (IsDDPPgametype("fng"))
	{
		if (m_apPlayers[ClientID])
		{
			m_apPlayers[ClientID]->m_IsInstaMode_idm = true;
			m_apPlayers[ClientID]->m_IsInstaMode_fng = true;
		}
	}

	//world.insert_entity(&players[client_id]);
	m_apPlayers[ClientID]->Respawn();
	// init the player
	Score()->PlayerData(ClientID)->Reset();
	m_apPlayers[ClientID]->m_Score = -9999;

	// Can't set score here as LoadScore() is threaded, run it in
	// LoadScoreThreaded() instead
	Score()->LoadScore(ClientID);

	m_apPlayers[ClientID]->m_Score = (Score()->PlayerData(ClientID)->m_BestTime) ? Score()->PlayerData(ClientID)->m_BestTime : -9999;

	if (g_Config.m_SvDDPPscore == 0)
	{
		m_apPlayers[ClientID]->m_Score = 0;
		m_apPlayers[ClientID]->m_AllowTimeScore = 0;
		CMsgPacker ScoreMsg(NETMSG_TIME_SCORE);
		ScoreMsg.AddInt(m_apPlayers[ClientID]->m_AllowTimeScore);
		Server()->SendMsg(&ScoreMsg, MSGFLAG_VITAL|MSGFLAG_NORECORD, ClientID, true);
	}

	if(((CServer *) Server())->m_aPrevStates[ClientID] < CServer::CClient::STATE_INGAME)
	{
		char aBuf[512];
		if (!silent && (g_Config.m_SvHideJoinLeaveMessages == 3 || g_Config.m_SvHideJoinLeaveMessages == 1))
		{
			if (g_Config.m_SvActivatePatternFilter)
			{
				if (str_find(Server()->ClientName(ClientID), g_Config.m_SvHideJoinLeaveMessagesPattern))
				{
					//hide pattern
					str_format(aBuf, sizeof(aBuf), "player='%d:%s' join (message hidden)", ClientID, Server()->ClientName(ClientID));
					Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
				}
				else
				{
					str_format(aBuf, sizeof(aBuf), "'%s' entered and joined the %s", Server()->ClientName(ClientID), m_pController->GetTeamName(m_apPlayers[ClientID]->GetTeam()));
					SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				}
			}
			else if (!str_comp(g_Config.m_SvHideJoinLeaveMessagesPlayer, Server()->ClientName(ClientID)))
			{
				str_format(aBuf, sizeof(aBuf), "player='%d:%s' join (message hidden)", ClientID, Server()->ClientName(ClientID));
				Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "'%s' entered and joined the %s", Server()->ClientName(ClientID), m_pController->GetTeamName(m_apPlayers[ClientID]->GetTeam()));
				SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
		}
		if (g_Config.m_SvInstagibMode)
		{
			SendChatTarget(ClientID, "DDNet++ Instagib Mod (" DDNETPP_VERSION ") based on DDNet 9.0.2");
		}
		else
		{
			char aWelcome[128];
			str_format(aWelcome, sizeof(aWelcome), "DDNet++ %s Mod (%s) based on DDNet 9.0.2", g_Config.m_SvDDPPgametype, DDNETPP_VERSION);
			SendChatTarget(ClientID, aWelcome);
		}

		if(g_Config.m_SvWelcome[0]!=0)
			SendChatTarget(ClientID,g_Config.m_SvWelcome);
		str_format(aBuf, sizeof(aBuf), "team_join player='%d:%s' team=%d", ClientID, Server()->ClientName(ClientID), m_apPlayers[ClientID]->GetTeam());

		Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

		if (g_Config.m_SvShowOthersDefault)
		{
			if (g_Config.m_SvShowOthers)
				SendChatTarget(ClientID, "You can see other players. To disable this use the DDNet client and type /showothers .");

			m_apPlayers[ClientID]->m_ShowOthers = true;
		}
	}
	m_VoteUpdate = true;

	// send active vote
	if(m_VoteCloseTime)
		SendVoteSet(ClientID);

	m_apPlayers[ClientID]->m_Authed = ((CServer*)Server())->m_aClients[ClientID].m_Authed;
}

void CGameContext::OnClientConnected(int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_ClientLeftServer[ClientID] = false;

	// Check which team the player should be on (copyed all the stuff cuz const int mukked)
	//if (g_Config.m_SvInstagibMode == 2 || g_Config.m_SvInstagibMode == 4) //grenade zCatch and rifle zCatch
	if (m_insta_survival_gamestate) //running survival game
	{
		const int StartTeam = TEAM_SPECTATORS;

		if (!m_apPlayers[ClientID])
			m_apPlayers[ClientID] = new(ClientID) CPlayer(this, ClientID, StartTeam);
		else
		{
			delete m_apPlayers[ClientID];
			m_apPlayers[ClientID] = new(ClientID) CPlayer(this, ClientID, StartTeam);
			//	//m_apPlayers[ClientID]->Reset();
			//	//((CServer*)Server())->m_aClients[ClientID].Reset();
			//	((CServer*)Server())->m_aClients[ClientID].m_State = 4;
		}
		//players[client_id].init(client_id);
		//players[client_id].client_id = client_id;

		//(void)m_pController->CheckTeamBalance();
	}
	else
	{
		const int StartTeam = g_Config.m_SvTournamentMode ? TEAM_SPECTATORS : m_pController->GetAutoTeam(ClientID);

		if (!m_apPlayers[ClientID])
			m_apPlayers[ClientID] = new(ClientID) CPlayer(this, ClientID, StartTeam);
		else
		{
			delete m_apPlayers[ClientID];
			m_apPlayers[ClientID] = new(ClientID) CPlayer(this, ClientID, StartTeam);
			//	//m_apPlayers[ClientID]->Reset();
			//	//((CServer*)Server())->m_aClients[ClientID].Reset();
			//	((CServer*)Server())->m_aClients[ClientID].m_State = 4;
		}
		//players[client_id].init(client_id);
		//players[client_id].client_id = client_id;

		//(void)m_pController->CheckTeamBalance();
	}


#ifdef CONF_DEBUG
	if(g_Config.m_DbgDummies)
	{
		if(ClientID >= MAX_CLIENTS-g_Config.m_DbgDummies)
			return;
	}
#endif

	// send motd
	CNetMsg_Sv_Motd Msg;
	char aBuf[128]; 
	char aBroad[2048];
	bool IsSupporterOnline = false;
	str_format(aBroad, sizeof(aBroad), "%s\n[ONLINE SUPPORTER]:\n", g_Config.m_SvMotd);

	//lass mal durch alle spieler iterieren und schauen ob n mod online is
	for (int i = 0; i < MAX_CLIENTS; i++) //iteriert durch alle 64 client ids
	{
		if (m_apPlayers[i] && m_apPlayers[i]->m_IsSupporter) //schaut ob der spieler existiert und supporter is
		{
			str_format(aBuf, sizeof(aBuf), "• '%s'\n", Server()->ClientName(i));
			str_append(aBroad, aBuf, sizeof(aBroad));
			IsSupporterOnline = true;
		}
	}

	if (IsSupporterOnline) // so wenn ein mod online ist schicken wir die modifizierte message of the day mit dem namen des sup 
	{
		Msg.m_pMessage = aBroad;
	}
	else //sonst schicken wir die normale 
	{
		Msg.m_pMessage = g_Config.m_SvMotd; //hier wird der string aus der config variable in die message geklatscht du meinst das was man in der autoexec eingibt? yes oder ngame mit sv_modt yy also lass das mal modifizieren davo
	}
	
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

void CGameContext::OnClientDrop(int ClientID, const char *pReason, bool silent)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_ClientLeftServer[ClientID] = true;
	AbortVoteKickOnDisconnect(ClientID);
	m_apPlayers[ClientID]->OnDisconnect(pReason, silent);
	delete m_apPlayers[ClientID];
	m_apPlayers[ClientID] = 0;

	//(void)m_pController->CheckTeamBalance();
	m_VoteUpdate = true;

	// update spectator modes
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_SpectatorID == ClientID)
			m_apPlayers[i]->m_SpectatorID = SPEC_FREEVIEW;
	}

	// update conversation targets
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_LastWhisperTo == ClientID)
			m_apPlayers[i]->m_LastWhisperTo = -1;
	}
}

void CGameContext::OnStartBlockTournament()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_BlockTournaState)
	{
		SendChat(-1, CGameContext::CHAT_ALL, "[EVENT] error tournament already running.");
		return;
	}
	if (g_Config.m_SvAllowBlockTourna == 0)
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
//	CALL_STACK_ADD();
//#endif
//	SendChat(-1, CGameContext::CHAT_ALL, "[DDNet++] server shutdown!");
//}

void CGameContext::AbuseMotd(const char * pMsg, int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_apPlayers[ClientID])
	{
		m_apPlayers[ClientID]->m_IsFakeMotd = true;
	}
	// send motd
	CNetMsg_Sv_Motd Msg;
	Msg.m_pMessage = pMsg;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
}

int CGameContext::IsMinigame(int playerID) //if you update this function please also update the '/minigames' chat command
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[playerID];
	if (!pPlayer)
		return 0;
	CCharacter *pChr = GetPlayerChar(playerID);

	if (pPlayer->m_JailTime)
	{
		return -1;
	}
	if (pPlayer->m_IsInstaArena_gdm)
	{
		return 1;
	}
	if (pPlayer->m_IsInstaArena_idm)
	{
		return 2;
	}
	if (pPlayer->m_IsBalanceBatteling)
	{
		return 3;
	}
	if (pPlayer->m_IsSurvivaling)
	{
		return 4;
	}
	//if (pPlayer->m_Ischidraqul3) //dont return the broadcast only game because it doesnt make too much trouble. You can play chidraqul in jail or while being in insta no problem.
	//{
	//	return x;
	//}
	if (pChr)
	{
		if (pChr->m_IsBombing)
		{
			return 5;
		}
		if (pChr->m_IsPVParena)
		{
			return 6;
		}
	}
	if (pPlayer->m_IsBlockWaving)
	{
		return 7;
	}
	if (pPlayer->m_IsBlockTourning)
	{
		return 8;
	}
	if (pPlayer->m_IsBlockDeathmatch)
	{
		return 9;
	}

	return 0;
}

bool CGameContext::IsDDPPgametype(const char * pGametype)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	return !str_comp_nocase(g_Config.m_SvDDPPgametype, pGametype);
}

int CGameContext::GetCIDByName(const char * pName)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int nameID = -1;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i])
		{
			if (!str_comp(pName, Server()->ClientName(i)))
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i])
		{
			if (m_apPlayers[i]->m_DummyMode == 99)
			{
				return i;
			}
		}
	}
	return -1;
}

int CGameContext::CountConnectedPlayers()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int cPlayers = 0;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i])
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int cPlayers = 0;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && !m_apPlayers[i]->m_IsDummy)
		{
			cPlayers++;
		}
	}
	return cPlayers;
}

int CGameContext::CountIngameHumans()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int cPlayers = 0;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && m_apPlayers[i]->GetCharacter() && !m_apPlayers[i]->m_IsDummy)
		{
			cPlayers++;
		}
}
	return cPlayers;
}

bool CGameContext::IsAllowedCharSet(const char *pStr)
{
#if defined(CONF_DEBUG)
    CALL_STACK_ADD();
#endif
    int i = 0;
    bool IsOk = false;
    //dbg_msg("AllowedChar", "checking str '%s'", pStr);
    
    while (true)
    {
        IsOk = false;
        for (int j = 0; j < str_length(m_aAllowedCharSet); j++)
        {
            if (pStr[i] == m_aAllowedCharSet[j])
            {
                //dbg_msg("AllowedChar","found valid char '%c' - '%c'", pStr[i], m_aAllowedCharSet[j]);
                IsOk = true;
                break;
            }
        }
        
        if (!IsOk)
        {
            //dbg_msg("AllowedChar", "found evil char '%c'", pStr[i]);
            return false;
        }
        i++;
        if (pStr[i] == '\0')
        {
            //dbg_msg("AllowedChar", "string ends at %d", i);
            return true;
        }
    }
    return true;
}

int CGameContext::CountConnectedBots()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int lum_tt_zv_1_zz_04032018_lt3 = 0;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && m_apPlayers[i]->m_IsDummy)
		{
			lum_tt_zv_1_zz_04032018_lt3++;
		}
	}
	return lum_tt_zv_1_zz_04032018_lt3;
}

void CGameContext::SendBroadcastAll(const char * pText, int importance, bool supermod)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i])
		{
			SendBroadcast(pText, m_apPlayers[i]->GetCID(), importance, supermod);
		}
	}
}

void CGameContext::KillAll()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && m_apPlayers[i]->GetCharacter()) //only kill alive dudes
		{
			GetPlayerChar(i)->Die(i, WEAPON_WORLD);
		}
	}
}

void CGameContext::GiveBlockPoints(int ID, int points)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!m_apPlayers[ID])
		return;

	char aBuf[128];
	bool FlagBonus = false;

	if (m_apPlayers[ID]->GetCharacter())
	{
		if (((CGameControllerDDRace*)m_pController)->HasFlag(m_apPlayers[ID]->GetCharacter()) != -1)
		{
			points++;
			FlagBonus = true;
		}
	}

	m_apPlayers[ID]->m_BlockPoints += points;
	if (m_apPlayers[ID]->m_ShowBlockPoints)
	{
		if (m_apPlayers[ID]->m_AccountID > 0)
		{
			if (!FlagBonus)
			{
				if (points == 1)
				{
					str_format(aBuf, sizeof(aBuf), "+1 point");
				}
				else if (points > 1)
				{
					str_format(aBuf, sizeof(aBuf), "+%d points", points);
				}
			}
			else
			{
				if (points == 1)
				{
					str_format(aBuf, sizeof(aBuf), "+1 point (flag bonus)");
				}
				else if (points > 1)
				{
					str_format(aBuf, sizeof(aBuf), "+%d points (flag bonus)", points);
				}
			}
		}
		else
		{
			if (points == 1)
			{
				str_format(aBuf, sizeof(aBuf), "+%d point (warning! use '/login' to save your '/points')", points);
			}
			else if (points > 1)
			{
				str_format(aBuf, sizeof(aBuf), "+%d points (warning! use '/login' to save your '/points')", points);
			}
		}

		SendChatTarget(ID, aBuf);
	}
	else //chat info deactivated
	{
		if (m_apPlayers[ID]->m_AccountID <= 0)
		{
			if (m_apPlayers[ID]->m_BlockPoints == 5 || m_apPlayers[ID]->m_BlockPoints == 10) //after 5 and 10 unsaved kills and no messages actiavted --> inform the player about accounts
			{
				str_format(aBuf, sizeof(aBuf), "you made %d unsaved block points. Use '/login' to save your '/points'.", m_apPlayers[ID]->m_BlockPoints);
				SendChatTarget(ID, aBuf);
				SendChatTarget(ID, "Use '/accountinfo' for more information.");
			}
		}
	}
}

void CGameContext::GiveXp(int id, int value)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!m_apPlayers[id])
		return;
	if (m_apPlayers[id]->IsMaxLevel())
		return;

	m_apPlayers[id]->m_xp += value; // TODO: clamp to max needed level if max level reached
}

void CGameContext::LoadFNNvalues()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	std::ifstream readfile;
	char aFilePath[512];
	str_format(aFilePath, sizeof(aFilePath), "FNN/move_stats.fnn");
	readfile.open(aFilePath);
	if (readfile.is_open())
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	ExecuteSQLf("UPDATE `Accounts` SET `IsLoggedIn` = '%i' WHERE `LastLoginPort` = '%i'", 0, g_Config.m_SvPort);
}

bool CGameContext::IsPosition(int playerID, int pos)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
	//dbg_msg("debug", "IsPosition(playerID = %d, pos = %d)", playerID, pos);
#endif
	if (!m_apPlayers[playerID])
	{
		return false;
	}
	if (!GetPlayerChar(playerID))
	{
		return false;
	}

	if (pos == 0) //cb5 jail release spot
	{
		if (GetPlayerChar(playerID)->m_Pos.x > 480 * 32
			&& GetPlayerChar(playerID)->m_Pos.x < 500 * 32
			&& GetPlayerChar(playerID)->m_Pos.y > 229 * 32
			&& GetPlayerChar(playerID)->m_Pos.y < 237 * 32)
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
	else if (pos == 2) //cb5 far in map (block area and race)
	{
		if (GetPlayerChar(playerID)->m_Pos.x > 415 * 32)
		{
			return true;
		}
	}
	else if (pos == 3) //configurated spawn area
	{
		if (GetPlayerChar(playerID)->m_Pos.x > g_Config.m_SvSpawnareaLowX * 32
			&& GetPlayerChar(playerID)->m_Pos.x < g_Config.m_SvSpawnareaHighX * 32
			&& GetPlayerChar(playerID)->m_Pos.y > g_Config.m_SvSpawnareaLowY * 32
			&& GetPlayerChar(playerID)->m_Pos.y < g_Config.m_SvSpawnareaHighY * 32)
		{
			return true;
		}
	}

	return false;
}

void CGameContext::StartAsciiAnimation(int viewerID, int creatorID, int medium)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!m_apPlayers[viewerID])
		return;
	if (!m_apPlayers[creatorID])
	{
		SendChatTarget(viewerID, "player not found.");
		return;
	}
	//dont start new animation while old is running
	if (m_apPlayers[viewerID]->m_AsciiWatchingID != -1)
	{
		return;
	}

	if (medium == 0) // '/ascii view <cid>'
	{
		if (m_apPlayers[creatorID]->m_aAsciiPublishState[0] == '0')
		{
			SendChatTarget(viewerID, "ascii art not public.");
			return;
		}

		m_apPlayers[creatorID]->m_AsciiViewsDefault++;
		//COULDDO: code: cfv45
	}
	else if (medium == 1) // '/profile view <player>'
	{
		if (m_apPlayers[creatorID]->m_aAsciiPublishState[1] == '0')
		{
			//SendChatTarget(viewerID, "ascii art not published on profile");
			return;
		}

		m_apPlayers[creatorID]->m_AsciiViewsProfile++;
	}
	else if (medium == 2) // not used yet
	{
		if (m_apPlayers[creatorID]->m_aAsciiPublishState[2] == '0')
		{
			SendChatTarget(viewerID, "ascii art not published on medium 2");
			return;
		}
	}
	else if (medium == 3) // not used yet
	{
		if (m_apPlayers[creatorID]->m_aAsciiPublishState[3] == '0')
		{
			SendChatTarget(viewerID, "ascii art not published on medium 3");
			return;
		}
	}

	m_apPlayers[viewerID]->m_AsciiWatchingID = creatorID;
}

bool CGameContext::IsHooked(int hookedID, int power)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CCharacter *pChar = GetPlayerChar(i);

		if (!pChar || !pChar->IsAlive() || pChar->GetPlayer()->GetCID() == hookedID)
			continue;
		if (pChar->Core()->m_HookedPlayer == hookedID && pChar->GetPlayer()->m_HookPower == power)
		{
			return true;
		}
	}


	return false;
}

bool CGameContext::IsSameIP(int ID_1, int ID_2)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aIP_1[64];
	char aIP_2[64];
	Server()->GetClientAddr(ID_1, aIP_1, sizeof(aIP_1));
	Server()->GetClientAddr(ID_2, aIP_2, sizeof(aIP_2));
	if (!str_comp_nocase(aIP_1, aIP_2))
	{
		//dbg_msg("IP_CHECKER", "[%s] [%s]  EQUAL", aIP_1, aIP_2);
		return true;
	}
	//dbg_msg("IP_CHECKER", "[%s] [%s] UNQUAL", aIP_1, aIP_2);
	return false;
}

int CGameContext::C3_GetFreeSlots()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int c = g_Config.m_SvChidraqulSlots;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && m_apPlayers[i]->m_C3_GameState == 2)
		{
			c--;
		}
	}
	return c;
}

int CGameContext::C3_GetOnlinePlayers()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int c = 0;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && m_apPlayers[i]->m_C3_GameState == 2)
		{
			c++;
		}
	}
	return c;
}

void CGameContext::C3_MultiPlayer_GameTick(int id)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_apPlayers[id]->m_C3_UpdateFrame || Server()->Tick() % 120 == 0)
	{
		C3_RenderFrame();
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (m_apPlayers[i])
			{
				m_apPlayers[i]->m_C3_UpdateFrame = false; //only render once a tick
			}
		}
	}
}

void CGameContext::C3_RenderFrame()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	char aBuf[128];
	char aHUD[64];
	char aWorld[64]; //max world size
	int players = C3_GetOnlinePlayers();

	//init world
	for (int i = 0; i < g_Config.m_SvChidraqulWorldX; i++)
	{
		aWorld[i] = '_';
	}
	
	//place players
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && m_apPlayers[i]->m_C3_GameState == 2)
		{
			aWorld[m_apPlayers[i]->m_HashPos] = m_apPlayers[i]->m_HashSkin[0];
		}
	}
	
	//finish string
	aWorld[g_Config.m_SvChidraqulWorldX] = '\0';


	//add hud and send to players
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && m_apPlayers[i]->m_C3_GameState == 2)
		{
			str_format(aHUD, sizeof(aHUD), "\n\nPos: %d Players: %d/%d", m_apPlayers[i]->m_HashPos, players, g_Config.m_SvChidraqulSlots);
			str_format(aBuf, sizeof(aBuf), "%s%s", aWorld, aHUD);

			//dbg_msg("debug", "printing: %s", aBuf);

			SendBroadcast(aBuf, i, 0);
		}
	}
}

char CGameContext::BoolToChar(bool b)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (b)
		return '1';
	return '0';
}

bool CGameContext::CharToBool(char c)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (c == '0')
		return false;
	return true;
}

void CGameContext::ShowHideConfigBoolToChar(int id)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[id];
	if (!pPlayer)
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[id];
	if (!pPlayer)
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

void CGameContext::FNN_LoadRun(const char * path, int botID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[botID];
	if (!pPlayer)
	{
		dbg_msg("FNN","failed to load run player with id=%d doesn't exist", botID);
		return;
	}
	CCharacter *pChr = GetPlayerChar(botID);
	if (!pChr)
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
	str_format(aFilePath, sizeof(aFilePath), path);
	readfile.open(aFilePath);
	if (readfile.is_open())
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

		while (std::getline(readfile, line))
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[botID];
	if (!pPlayer)
		return;
	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	//moved to character
}

vec2 CGameContext::GetFinishTile()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
	for (int i = 0; i < Width*Height; i++)
	{
		if (Collision()->GetTileIndex(i) == TILE_END || Collision()->GetFTileIndex(i) == TILE_END)
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

	return vec2(0,0);
}

void CGameContext::JoinInstagib(int weapon, bool fng, int ID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
	//dbg_msg("cBug", "PLAYER '%s' ID=%d JOINED INSTAGIB WITH WEAPON = %d ANF FNG = %d", Server()->ClientName(ID), ID, weapon, fng);
#endif

	//die first to not count death
	if (m_apPlayers[ID]->GetCharacter())
	{
		m_apPlayers[ID]->GetCharacter()->Die(ID, WEAPON_SELF);
	}

	//reset values
	m_apPlayers[ID]->m_HasInstaRoundEndPos = false;
	m_apPlayers[ID]->m_IsInstaArena_idm = false;
	m_apPlayers[ID]->m_IsInstaArena_gdm = false;
	m_apPlayers[ID]->m_IsInstaMode_idm = false;
	m_apPlayers[ID]->m_IsInstaMode_gdm = false;
	m_apPlayers[ID]->m_InstaScore = 0;

	m_apPlayers[ID]->m_IsInstaArena_fng = fng;
	m_apPlayers[ID]->m_IsInstaMode_fng = fng;
	if (weapon == 5)
	{
		m_apPlayers[ID]->m_IsInstaArena_idm = true;
		m_apPlayers[ID]->m_IsInstaMode_idm = true;
	}
	else if (weapon == 4)
	{
		m_apPlayers[ID]->m_IsInstaArena_gdm = true;
		m_apPlayers[ID]->m_IsInstaMode_gdm = true;
	}
	else
	{
		SendChatTarget(ID, "[WARNING] Something went horrible wrong please report an admin");
		return;
	}

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' joined the game.", Server()->ClientName(ID));
	SayInsta(aBuf, weapon);
}

void CGameContext::ShowInstaStats(int requestID, int requestedID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!m_apPlayers[requestID])
		return;
	CPlayer *pPlayer = m_apPlayers[requestedID];
	if (!pPlayer)
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!m_apPlayers[requestID])
		return;
	CPlayer *pPlayer = m_apPlayers[requestedID];
	if (!pPlayer)
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

void CGameContext::LeaveInstagib(int ID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	CPlayer *pPlayer = m_apPlayers[ID];
	if (!pPlayer)
		return;
	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' left the game.", Server()->ClientName(ID));
	if (pPlayer->m_IsInstaArena_gdm) 
	{ SayInsta(aBuf, 4);}
	else if (pPlayer->m_IsInstaArena_idm) 
	{ SayInsta(aBuf, 5);}



	if ((pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm) && pPlayer->m_Insta1on1_id != -1)
	{
		WinInsta1on1(pPlayer->m_Insta1on1_id, ID);
		SendChatTarget(ID, "[INSTA] You left the 1on1.");
		SendBroadcast("", ID);
		return;
	}

	bool left = true;

	if (pPlayer->m_IsInstaArena_fng)
	{
		if (pPlayer->m_IsInstaArena_gdm)
		{
			SendChatTarget(ID, "[INSTA] You left boomfng.");
		}
		else if (pPlayer->m_IsInstaArena_idm)
		{
			SendChatTarget(ID, "[INSTA] You left fng.");
		}
		else
		{
			left = false;
		}
	}
	else
	{
		if (pPlayer->m_IsInstaArena_gdm)
		{
			SendChatTarget(ID, "[INSTA] You left grenade deathmatch.");
		}
		else if (pPlayer->m_IsInstaArena_idm)
		{
			SendChatTarget(ID, "[INSTA] You left rifle deathmatch.");
		}
		else
		{
			left = false;
		}
	}

	if (left)
	{
		pPlayer->m_IsInstaArena_gdm = false;
		pPlayer->m_IsInstaArena_idm = false;
		pPlayer->m_IsInstaArena_fng = false;
		pPlayer->m_IsInstaMode_gdm = false;
		pPlayer->m_IsInstaMode_idm = false;
		pPlayer->m_IsInstaMode_fng = false;
		if (pChr) { pChr->Die(pPlayer->GetCID(), WEAPON_SELF); }
		SendBroadcast("", ID); //clear score
	}
	else
	{
		SendChatTarget(ID, "[INSTA] You are not in a instagib game.");
	}
}

void CGameContext::SayInsta(const char * pMsg, int weapon)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
	//dbg_msg("cBug", "SayInsta got called with weapon %d and message '%s'", weapon, pMsg);
#endif
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i])
		{
			if (weapon == 4) //grenade
			{
				if (m_apPlayers[i]->m_IsInstaArena_gdm)
				{
					SendChatTarget(i, pMsg);
				}
			}
			else if (weapon == 5) //rifle
			{
				if (m_apPlayers[i]->m_IsInstaArena_idm)
				{
					SendChatTarget(i, pMsg);
				}
			}
		}
	}
}

void CGameContext::DoInstaScore(int score, int id)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
	dbg_msg("insta", "'%s' scored %d in instagib [score: %d]", Server()->ClientName(id), score, m_apPlayers[id]->m_InstaScore);
#endif
	CPlayer *pPlayer = m_apPlayers[id];
	if (!pPlayer)
		return;

	pPlayer->m_InstaScore += score;
	if (pPlayer->GetCharacter())
		if (pPlayer->m_ShowInstaScoreBroadcast)
			pPlayer->GetCharacter()->m_UpdateInstaScoreBoard = true;
	CheckInstaWin(id);
}

void CGameContext::CheckInstaWin(int ID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_apPlayers[ID]->m_IsInstaArena_gdm)
	{
		if (m_apPlayers[ID]->m_InstaScore >= g_Config.m_SvGrenadeScorelimit)
		{
			m_InstaGrenadeRoundEndDelay = Server()->TickSpeed() * 20; //stored the value to be on the save side. I have no idea how this func works and i need the EXACT value lateron
			m_InstaGrenadeRoundEndTickTicker = m_InstaGrenadeRoundEndDelay; //start grenade round end tick
			m_InstaGrenadeWinnerID = ID;
		}
	}
	else if (m_apPlayers[ID]->m_IsInstaArena_idm)
	{
		if (m_apPlayers[ID]->m_InstaScore >= g_Config.m_SvRifleScorelimit)
		{
			m_InstaRifleRoundEndDelay = Server()->TickSpeed() * 20; //stored the value to be on the save side. I have no idea how this func works and i need the EXACT value lateron
			m_InstaRifleRoundEndTickTicker = m_InstaRifleRoundEndDelay; //start grenade round end tick
			m_InstaRifleWinnerID = ID;
		}
	}
}

void CGameContext::InstaGrenadeRoundEndTick(int ID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!m_InstaGrenadeRoundEndTickTicker) { return; }
	if (!m_apPlayers[ID]->m_IsInstaArena_gdm) { return; }

	char aBuf[256];

	if (m_InstaGrenadeRoundEndTickTicker == m_InstaGrenadeRoundEndDelay)
	{
		str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' won the grenade game", Server()->ClientName(m_InstaGrenadeWinnerID));
		SendChatTarget(ID, aBuf);
		str_format(aBuf, sizeof(aBuf), "'%s' won the grenade game", Server()->ClientName(m_InstaGrenadeWinnerID));
		SendBroadcast(aBuf, ID);
		m_apPlayers[ID]->m_HasInstaRoundEndPos = false;

		//PlayerArryaID / PlayerTeeworldsID / PlayerScore == 64x2
		int aaScorePlayers[MAX_CLIENTS][2];

		for (int i = 0; i < MAX_CLIENTS; i++) //prepare array
		{
			//aaScorePlayers[i][1] = -1; //set all score to -1 to lateron filter them so please keep in mind to never let the score become negative or the poor tees will be hidden in scoreboard
			aaScorePlayers[i][0] = -1; //set all ids to -1 to lateron filter these out of scoreboard
		}

		for (int i = 0; i < MAX_CLIENTS; i++) //fill array
		{
			if (m_apPlayers[i] && m_apPlayers[i]->m_IsInstaArena_gdm)
			{
				aaScorePlayers[i][1] = m_apPlayers[i]->m_InstaScore;
				aaScorePlayers[i][0] = i;
			}
		}

		for (int i = 0; i < MAX_CLIENTS; i++) //sort array (bubble mubble)
		{
			for (int k = 0; k < MAX_CLIENTS - 1; k++)
			{
				if (aaScorePlayers[k][1] < aaScorePlayers[k + 1][1])
				{
					//move ids
					int tmp = aaScorePlayers[k][0];
					aaScorePlayers[k][0] = aaScorePlayers[k + 1][0];
					aaScorePlayers[k + 1][0] = tmp;
					//move score
					tmp = aaScorePlayers[k][1];
					aaScorePlayers[k][1] = aaScorePlayers[k + 1][1];
					aaScorePlayers[k + 1][1] = tmp;
				}
			}
		}

		str_format(m_aInstaGrenadeScoreboard, sizeof(m_aInstaGrenadeScoreboard), "==== Scoreboard [GRENADE] ====\n");
		int Rank = 1;

		for (int i = 0; i < MAX_CLIENTS; i++) //print array in scoreboard
		{
			if (aaScorePlayers[i][0] != -1)
			{
				str_format(aBuf, sizeof(aBuf), "%d. '%s' - %d \n", Rank++, Server()->ClientName(aaScorePlayers[i][0]), aaScorePlayers[i][1]);
				strcat(m_aInstaGrenadeScoreboard, aBuf);
			}
		}
	}
	if (m_InstaGrenadeRoundEndTickTicker == 1)
	{
		//reset stats
		m_apPlayers[ID]->m_InstaScore = 0;

		if (m_apPlayers[ID]->GetCharacter())
		{
			m_apPlayers[ID]->GetCharacter()->Die(ID, WEAPON_WORLD);
		}
		SendChatTarget(ID, "[INSTA] new round new luck.");
	}

	if (m_apPlayers[ID]->GetCharacter())
	{
		if (!m_apPlayers[ID]->m_HasInstaRoundEndPos)
		{
			m_apPlayers[ID]->m_InstaRoundEndPos = m_apPlayers[ID]->GetCharacter()->GetPosition();
			m_apPlayers[ID]->m_HasInstaRoundEndPos = true;
		}

		if (m_apPlayers[ID]->m_HasInstaRoundEndPos)
		{
			m_apPlayers[ID]->GetCharacter()->SetPosition(m_apPlayers[ID]->m_InstaRoundEndPos);
		}
	}

	AbuseMotd(m_aInstaGrenadeScoreboard, ID); //send the scoreboard every fokin tick hehe
}

void CGameContext::InstaRifleRoundEndTick(int ID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!m_InstaRifleRoundEndTickTicker) { return; }
	if (!m_apPlayers[ID]->m_IsInstaArena_idm) { return; }

	char aBuf[256];

	if (m_InstaRifleRoundEndTickTicker == m_InstaRifleRoundEndDelay)
	{
		str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' won the rifle game", Server()->ClientName(m_InstaRifleWinnerID));
		SendChatTarget(ID, aBuf);
		str_format(aBuf, sizeof(aBuf), "'%s' won the rifle game", Server()->ClientName(m_InstaRifleWinnerID));
		SendBroadcast(aBuf, ID);
		m_apPlayers[ID]->m_HasInstaRoundEndPos = false;

		//PlayerArryaID / PlayerTeeworldsID / PlayerScore == 64x2
		int aaScorePlayers[MAX_CLIENTS][2];

		for (int i = 0; i < MAX_CLIENTS; i++) //prepare array
		{
			//aaScorePlayers[i][1] = -1; //set all score to -1 to lateron filter them so please keep in mind to never let the score become negative or the poor tees will be hidden in scoreboard
			aaScorePlayers[i][0] = -1; //set all ids to -1 to lateron filter these out of scoreboard
		}

		for (int i = 0; i < MAX_CLIENTS; i++) //fill array
		{
			if (m_apPlayers[i] && m_apPlayers[i]->m_IsInstaArena_idm)
			{
				aaScorePlayers[i][1] = m_apPlayers[i]->m_InstaScore;
				aaScorePlayers[i][0] = i;
			}
		}

		for (int i = 0; i < MAX_CLIENTS; i++) //sort array (bubble mubble)
		{
			for (int k = 0; k < MAX_CLIENTS - 1; k++)
			{
				if (aaScorePlayers[k][1] < aaScorePlayers[k + 1][1])
				{
					//move ids
					int tmp = aaScorePlayers[k][0];
					aaScorePlayers[k][0] = aaScorePlayers[k + 1][0];
					aaScorePlayers[k + 1][0] = tmp;
					//move score
					tmp = aaScorePlayers[k][1];
					aaScorePlayers[k][1] = aaScorePlayers[k + 1][1];
					aaScorePlayers[k + 1][1] = tmp;
				}
			}
		}

		str_format(m_aInstaRifleScoreboard, sizeof(m_aInstaRifleScoreboard), "==== Scoreboard [Rifle] ====\n");
		int Rank = 1;

		for (int i = 0; i < MAX_CLIENTS; i++) //print array in scoreboard
		{
			if (aaScorePlayers[i][0] != -1)
			{
				str_format(aBuf, sizeof(aBuf), "%d. '%s' - %d \n", Rank++, Server()->ClientName(aaScorePlayers[i][0]), aaScorePlayers[i][1]);
				strcat(m_aInstaRifleScoreboard, aBuf);
			}
		}
	}
	if (m_InstaRifleRoundEndTickTicker == 1)
	{
		//reset stats
		m_apPlayers[ID]->m_InstaScore = 0;

		m_apPlayers[ID]->GetCharacter()->Die(ID, WEAPON_WORLD);
		SendChatTarget(ID, "[INSTA] new round new luck.");
	}

	if (m_apPlayers[ID]->GetCharacter())
	{
		if (!m_apPlayers[ID]->m_HasInstaRoundEndPos)
		{
			m_apPlayers[ID]->m_InstaRoundEndPos = m_apPlayers[ID]->GetCharacter()->GetPosition();
			m_apPlayers[ID]->m_HasInstaRoundEndPos = true;
		}

		if (m_apPlayers[ID]->m_HasInstaRoundEndPos)
		{
			m_apPlayers[ID]->GetCharacter()->SetPosition(m_apPlayers[ID]->m_InstaRoundEndPos);
		}
	}

	AbuseMotd(m_aInstaRifleScoreboard, ID); //send the scoreboard every fokin tick hehe
}

bool CGameContext::ChillWriteToLine(char const * filename, unsigned lineNo, char const * data)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	std::fstream file(filename);
	if (!file)
		return false;

	unsigned currentLine = 0;
	while (currentLine < lineNo)
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

int CGameContext::ChillUpdateFileAcc(const char * account, unsigned int line, const char * value, int requestingID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "%s/%s.acc", g_Config.m_SvFileAccPath, account);
	std::fstream Acc2File(aBuf);

	if (!std::ifstream(aBuf))
	{
		SendChatTarget(requestingID, "[ACCOUNT] username not found.");
		Acc2File.close();
		return -1; //return error code -1
	}

	std::string data[32];
	int index = 0;

	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] password: '%s'", index, data[index].c_str()); index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] loggedin: '%s'", index, data[index].c_str()); index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] port: '%s'", index, data[index].c_str()); index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] frozen: '%s'", index, data[index].c_str()); index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] vip: '%s'", index, data[index].c_str()); index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] vip+: '%s'", index, data[index].c_str()); index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] sup: '%s'", index, data[index].c_str()); index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] money: '%s'", index, data[index].c_str()); index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] level: '%s'", index, data[index].c_str()); index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] xp: '%s'", index, data[index].c_str()); index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] shit: '%s'", index, data[index].c_str()); index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] police: '%s'", index, data[index].c_str()); index++;
	getline(Acc2File, data[index]);
	dbg_msg("acc2", "[%d] taser: '%s'", index, data[index].c_str()); index++;

	if (data[1] == "1")
	{
		str_format(aBuf, sizeof(aBuf), "[ACC2] '%s' is logged in on port '%s'", account, data[2].c_str());
		SendChatTarget(requestingID, aBuf);
		Acc2File.close();
		return -2;
	}

	if (line != 3 && data[3] == "1") //only can update the frozen value if acc is frozen
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

	if (Acc2FileW.is_open())
	{
		dbg_msg("acc2", "write acc '%s'", account);
		index = 0;

		Acc2FileW << data[index++] << "\n";			//0 password
		Acc2FileW << data[index++] << "\n";			//1 loggedin
		Acc2FileW << data[index++] << "\n";			//2 port
		Acc2FileW << data[index++] << "\n";			//3 frozen
		Acc2FileW << data[index++] << "\n";			//4 vip
		Acc2FileW << data[index++] << "\n";			//5 vip+
		Acc2FileW << data[index++] << "\n";			//6 sup
		Acc2FileW << data[index++] << "\n";			//7 money
		Acc2FileW << data[index++] << "\n";			//8 level
		Acc2FileW << data[index++] << "\n";			//9 xp
		Acc2FileW << data[index++] << "\n";			//10 shit
		Acc2FileW << data[index++] << "\n";			//11 police
		Acc2FileW << data[index++] << "\n";			//12 taser

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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for (int i = 0; i < amount; i++)
	{
		if (mode == 0) //rifle
		{
			CreateNewDummy(-4);
		}
		else if (mode == 1) // grenade
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[id];
	if (!pPlayer)
		return;
	CCharacter *pChr = m_apPlayers[id]->GetCharacter();
	if (!pChr)
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[id];
	if (!pPlayer)
		return;
	CCharacter *pChr = m_apPlayers[id]->GetCharacter();
	if (!pChr)
		return;

	//backup cosmetics for lobby (save)
	pChr->m_Rainbow = pPlayer->m_IsBackupRainbow;
	pChr->m_Bloody = pPlayer->m_IsBackupBloody;
	pChr->m_StrongBloody = pPlayer->m_IsBackupStrongBloody;
	pChr->m_Atom =  pPlayer->m_IsBackupAtom;
	pChr->m_Trail = pPlayer->m_IsBackupTrail;
	pChr->m_autospreadgun = pPlayer->m_IsBackupAutospreadgun;
	pChr->m_WaveBloody = pPlayer->m_IsBackupWaveBloody;
}

void CGameContext::DeleteCosmetics(int id)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[id];
	if (!pPlayer)
		return;
	CCharacter *pChr = m_apPlayers[id]->GetCharacter();
	if (!pChr)
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (g_Config.m_SvDDPPshutdown)
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
		if (hour == g_Config.m_SvDDPPshutdownHour && (min == 0 || min == 5 || min == 10)) //Try it 3 times (slow tick shouldnt trigger it multiple times a minute)
		{
			if (players < g_Config.m_SvDDPPshutdownPlayers)
			{
				//SendChat(-1, CGameContext::CHAT_ALL, "[DDNet++] WARNING SERVER SHUTDOWN!");
				CallVote(-1, "shutdown server", "shutdown", "Update", "[DDNet++] do you want to update the server now?", true);
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_iBroadcastDelay > 0)
	{
		m_iBroadcastDelay--;
	}

	if (m_BlockTournaState)
	{
		BlockTournaTick();
	}

	if (m_BalanceBattleState == 1)
	{
		BalanceBattleTick();
	}

	if (m_BombGameState)
	{
		BombTick();
	}

	if (m_survivalgamestate == 1)
	{
		SurvivalLobbyTick();
	}
	else
	{
		if (m_survival_game_countdown > 0)
		{
			m_survival_game_countdown--;
		}
		if (m_survival_game_countdown == 0)
		{
			SendSurvivalChat("[SURVIVAL] Game ended due to timeout. Nobody won.");
			str_copy(m_aLastSurvivalWinnerName, "", sizeof(m_aLastSurvivalWinnerName));
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (m_apPlayers[i] && m_apPlayers[i]->m_IsSurvivaling)
				{
					SetPlayerSurvival(i, SURVIVAL_LOBBY);
					if (m_apPlayers[i]->GetCharacter()) //only kill if isnt dead already or server crashes (he should respawn correctly anayways)
					{
						m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_GAME);
					}
				}
			}
			SurvivalSetGameState(SURVIVAL_LOBBY);
		}
		if (m_survivalgamestate == SURVIVAL_DM_COUNTDOWN)
		{
			SurvivalDeathmatchTick();
		}
	}

	if (m_BlockWaveGameState)
	{
		BlockWaveGameTick();
	}

	for (int i = 0; i < MAX_CLIENTS; i++) //all the tick stuff which needs all players
	{
		if (!m_apPlayers[i])
			continue;

		ChilliClanTick(i);
		AsciiTick(i);
		InstaGrenadeRoundEndTick(i);
		InstaRifleRoundEndTick(i);
		C3_MultiPlayer_GameTick(i);
	}
	if (m_InstaGrenadeRoundEndTickTicker) { m_InstaGrenadeRoundEndTickTicker--; }
	if (m_InstaRifleRoundEndTickTicker) { m_InstaRifleRoundEndTickTicker--; }

	if (Server()->Tick() % 600 == 0) //slow ddpp sub tick
	{
		DDPP_SlowTick();
	}
}

void CGameContext::DDPP_SlowTick()
{
	bool StopSurvival = true;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!m_apPlayers[i])
			continue;

		if (m_apPlayers[i]->m_QuestState && m_apPlayers[i]->m_QuestPlayerID != -1) //if player is on a <specfic player> quest
		{
			if (!m_apPlayers[m_apPlayers[i]->m_QuestPlayerID])
			{
				SendChatTarget(i, "[QUEST] Looks like your quest destination left the server.");
				QuestFailed(i);
			}
			else if (m_apPlayers[m_apPlayers[i]->m_QuestPlayerID]->GetTeam() == TEAM_SPECTATORS)
			{
				SendChatTarget(i, "[QUEST] Looks like your quest destination is a spectator.");
				QuestFailed(i);
			}
		}
		if (m_apPlayers[i]->m_IsSurvivaling)
		{
			StopSurvival = false;
		}
		if (m_BlockTournaState == 3)
		{
			if (m_apPlayers[i]->m_IsBlockTourning)
			{
				m_apPlayers[i]->m_IsBlockTourning = false;
				if (m_apPlayers[i]->GetCharacter())
				{
					m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_GAME);
				}
			}
		}
	}


	if (StopSurvival)
	{
		m_survivalgamestate = 0; //don't waste ressource on lobby checks if nobody is playing
	}
	if (m_BlockTournaState == 3)
	{
		m_BlockTournaState = 0;
	}
	if (g_Config.m_SvAllowGlobalChat)
	{
		GlobalChatPrintMessage();
	}

	if (g_Config.m_SvMinDoubleTilePlayers > 0)
	{
		if (CountIngameHumans() >= g_Config.m_SvMinDoubleTilePlayers && MoneyDoubleEnoughPlayers == true) // MoneyTileDouble();  bla bla 
		{
			SendChat(-1, CGameContext::CHAT_ALL, "The double-moneytile has been activated!");
			MoneyDoubleEnoughPlayers = false;
		}
		if (CountIngameHumans() < g_Config.m_SvMinDoubleTilePlayers && MoneyDoubleEnoughPlayers == false)
		{
			SendChat(-1, CGameContext::CHAT_ALL, "The double-moneytile has been deactivated!");
			MoneyDoubleEnoughPlayers = true;
		}
	}
	CheckDDPPshutdown();
}

void CGameContext::ChilliClanTick(int i)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!g_Config.m_SvKickChilliClan)
		return;

	if (!m_apPlayers[i])
		return;

	CPlayer *pPlayer = m_apPlayers[i];

	int AbstandWarnungen = 10;
	if ((str_comp_nocase(Server()->ClientClan(i), "Chilli.*") == 0 && str_comp_nocase(pPlayer->m_TeeInfos.m_SkinName, "greensward") != 0) && (!pPlayer->m_SpookyGhostActive))
	{
		if (pPlayer->m_LastWarning + AbstandWarnungen*Server()->TickSpeed() <= Server()->Tick())
		{
			pPlayer->m_ChilliWarnings++;

			if (pPlayer->m_ChilliWarnings >= 4)
			{
				if (g_Config.m_SvKickChilliClan == 1)
				{
					GetPlayerChar(i)->m_FreezeTime = 1000;
					SendBroadcast("WARNING! You are using the wrong 'Chilli.*' clanskin.\n Leave the clan or change skin.", i);
					SendChatTarget(i, "You got freezed by Chilli.* clanportection. Change skin or clantag!");
				}
				else if (g_Config.m_SvKickChilliClan == 2)
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
				if (g_Config.m_SvKickChilliClan == 1)
				{
					str_format(aBuf2, sizeof(aBuf2), "Your are using the wrong skin!\nChange you clantag or use skin 'greensward'!\n\nWARNINGS UNTIL FREEZE[%d / 3]", pPlayer->m_ChilliWarnings);
				}
				else if (g_Config.m_SvKickChilliClan == 2)
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!m_apPlayers[i])
		return;

	if (m_apPlayers[i]->m_AsciiWatchingID != -1)
	{
		if (!m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]) //creator left -> stop animation
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
			if (m_apPlayers[i]->m_AsciiWatchTicker >= m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_AsciiAnimSpeed) //new frame
			{
				m_apPlayers[i]->m_AsciiWatchTicker = 0;
				m_apPlayers[i]->m_AsciiWatchFrame++;
				if (m_apPlayers[i]->m_AsciiWatchFrame > 15) //animation over -> stop animation
				{
					//SendChatTarget(i, "Ascii animation is over.");
					//SendBroadcast(" ANIMATION OVER ", i);
					m_apPlayers[i]->m_AsciiWatchingID = -1;
					m_apPlayers[i]->m_AsciiWatchTicker = 0;
					m_apPlayers[i]->m_AsciiWatchFrame = 0;
				}
				else //display new frame
				{
					if (m_apPlayers[i]->m_AsciiWatchFrame == 0)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame0, i);
					}
					else if (m_apPlayers[i]->m_AsciiWatchFrame == 1)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame1, i);
					}
					else if (m_apPlayers[i]->m_AsciiWatchFrame == 2)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame2, i);
					}
					else if (m_apPlayers[i]->m_AsciiWatchFrame == 3)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame3, i);
					}
					else if (m_apPlayers[i]->m_AsciiWatchFrame == 4)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame4, i);
					}
					else if (m_apPlayers[i]->m_AsciiWatchFrame == 5)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame5, i);
					}
					else if (m_apPlayers[i]->m_AsciiWatchFrame == 6)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame6, i);
					}
					else if (m_apPlayers[i]->m_AsciiWatchFrame == 7)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame7, i);
					}
					else if (m_apPlayers[i]->m_AsciiWatchFrame == 8)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame8, i);
					}
					else if (m_apPlayers[i]->m_AsciiWatchFrame == 9)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame9, i);
					}
					else if (m_apPlayers[i]->m_AsciiWatchFrame == 10)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame10, i);
					}
					else if (m_apPlayers[i]->m_AsciiWatchFrame == 11)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame11, i);
					}
					else if (m_apPlayers[i]->m_AsciiWatchFrame == 12)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame12, i);
					}
					else if (m_apPlayers[i]->m_AsciiWatchFrame == 13)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame13, i);
					}
					else if (m_apPlayers[i]->m_AsciiWatchFrame == 14)
					{
						SendBroadcast(m_apPlayers[m_apPlayers[i]->m_AsciiWatchingID]->m_aAsciiFrame14, i);
					}
					else if (m_apPlayers[i]->m_AsciiWatchFrame == 15)
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	FILE *pFile;
	struct CBinaryStorage statsBuff;

	pFile = fopen("ddpp-stats.dat","rb");
	if (!pFile)
	{
			dbg_msg("ddpp-stats", "[load] failed to open ddpp singleplayer stats");
			return;
	}

	fread(&statsBuff,sizeof(struct CBinaryStorage), 1, pFile);
	dbg_msg("ddpp-stats", "loaded data UnlockedLevel=%d", statsBuff.x);
	m_MissionUnlockedLevel = statsBuff.x;
	fread(&statsBuff,sizeof(struct CBinaryStorage), 1, pFile);
	dbg_msg("ddpp-stats", "loaded data CurrentLevel=%d", statsBuff.x);
	m_MissionCurrentLevel = statsBuff.x;

	fclose(pFile);
}

void CGameContext::SaveSinglePlayer()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	FILE *pFile;
	struct CBinaryStorage statsBuff;

	pFile = fopen("ddpp-stats.dat","wb");
	if (!pFile)
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

 
void CGameContext::GlobalChatPrintMessage()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aData[1024];
	std::string data;


	std::fstream ChatReadFile(g_Config.m_SvGlobalChatFile);

	if (!std::ifstream(g_Config.m_SvGlobalChatFile))
	{
		SendChat(-1, CGameContext::CHAT_ALL, "[CHAT] global chat stopped working.");
		g_Config.m_SvAllowGlobalChat = 0;
		ChatReadFile.close();
		return;
	}


	getline(ChatReadFile, data);
	str_format(aData, sizeof(aData), "%s", data.c_str());
	aData[0] = ' '; //remove the confirms before print in chat

	if (!str_comp(m_aLastPrintedGlobalChatMessage, aData))
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

void CGameContext::GlobalChatUpdateConfirms(const char * pStr)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[1024];
	str_format(aBuf, sizeof(aBuf), "%s", pStr);
	int confirms = 0;
	if (pStr[0] == '1')
		confirms = 1;
	else if (pStr[0] == '2')
		confirms = 2;
	else if (pStr[0] == '3')
		confirms = 3;
	else if (pStr[0] == '4')
		confirms = 4;
	else if (pStr[0] == '5')
		confirms = 5;
	else if (pStr[0] == '6')
		confirms = 6;
	else if (pStr[0] == '7')
		confirms = 7;
	else if (pStr[0] == '8')
		confirms = 8;
	else if (pStr[0] == '9')
		confirms = 9;

	std::ofstream ChatFile(g_Config.m_SvGlobalChatFile);
	if (!ChatFile)
	{
		SendChat(-1, CGameContext::CHAT_ALL, "[CHAT] global chat failed.... deactivating it.");
		dbg_msg("CHAT", "ERROR1 writing file '%s'", g_Config.m_SvGlobalChatFile);
		g_Config.m_SvAllowGlobalChat = 0;
		ChatFile.close();
		return;
	}

	if (ChatFile.is_open())
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

void CGameContext::SurvivalLobbyTick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[128];

	if (CountSurvivalPlayers() >= g_Config.m_SvSurvivalStartPlayers)
	{
		m_survivallobbycountdown--;
		if (m_survivallobbycountdown % Server()->TickSpeed() == 0 && m_survivallobbycountdown - 10 < Server()->TickSpeed() * 10) //only start to print last 10 seconds
		{
			if (!str_comp_nocase(m_aLastSurvivalWinnerName, ""))
			{
				str_format(aBuf, sizeof(aBuf), "survival game starts in %d seconds", m_survivallobbycountdown / Server()->TickSpeed());
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "Winner: %s\nsurvival game starts in %d seconds", m_aLastSurvivalWinnerName, m_survivallobbycountdown / Server()->TickSpeed());
			}
			SendSurvivalBroadcast(aBuf);

			if (m_survivallobbycountdown == (Server()->TickSpeed() * 9)) //teleport winner in lobby on last 10 sec countdown
			{
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (m_apPlayers[i] && m_apPlayers[i]->GetCharacter() && m_apPlayers[i]->m_IsSurvivaling && m_apPlayers[i]->m_IsSurvivalWinner)
					{
						vec2 SurvivalLobbySpawnTile = Collision()->GetRandomTile(TILE_SURVIVAL_LOBBY);

						if (SurvivalLobbySpawnTile == vec2(-1, -1)) //no survival lobby
						{
							SendSurvivalChat("[SURVIVAL] no survival lobby set.");
						}
						else
						{
							m_apPlayers[i]->GetCharacter()->SetPosition(SurvivalLobbySpawnTile);
						}
					}
				}
			}
		}
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "[SURVIVAL] %d/%d players to start a game", CountSurvivalPlayers(), g_Config.m_SvSurvivalStartPlayers);
		if (Server()->Tick() % 30 == 0)
		{
			SendSurvivalBroadcast(aBuf);
		}
		m_survivallobbycountdown = Server()->TickSpeed() * g_Config.m_SvSurvivalLobbyDelay;
	}

	if (m_survivallobbycountdown < 1)
	{
		SurvivalStartGame();
	}
}

void CGameContext::SurvivalDeathmatchTick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[256];
	m_survival_dm_countdown--;

	if (m_survival_dm_countdown % Server()->TickSpeed() == 0 && m_survival_dm_countdown - 10 < Server()->TickSpeed() * 10) //every second if seconds < 10
	{
		str_format(aBuf, sizeof(aBuf), "[SURVIVAL] deathmatch starts in %d seconds", m_survival_dm_countdown / Server()->TickSpeed());
		SendSurvivalBroadcast(aBuf);
	}
	else if (m_survival_dm_countdown % (Server()->TickSpeed() * 60) == 0 && m_survival_dm_countdown - 10 < (Server()->TickSpeed() * 60) * 5) //every minute if minutes < 5
	{
		str_format(aBuf, sizeof(aBuf), "[SURVIVAL] deathmatch starts in %d minutes", m_survival_dm_countdown / (Server()->TickSpeed() * 60));
		SendSurvivalChat(aBuf);
	}

	if (m_survival_dm_countdown < 1)
	{
		SurvivalSetGameState(SURVIVAL_DM);
	}
}

void CGameContext::SurvivalCheckWinnerAndDeathMatch()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int AliveTees = CountSurvivalPlayers(true);
	char aBuf[128];
	if (AliveTees < 2) //could also be == 1 but i think < 2 is saver. Check for winning.                        (much wow sentence inc..) if 2 were alive and now only 1 players alive and one dies we have a winner
	{
		if (!SurvivalPickWinner()) { SendSurvivalChat("[SURVIVAL] Nobody won."); }
		SurvivalSetGameState(SURVIVAL_LOBBY);
	}
	else if (AliveTees < g_Config.m_SvSurvivalDmPlayers)
	{
		SurvivalSetGameState(SURVIVAL_DM_COUNTDOWN);
		str_format(aBuf, sizeof(aBuf), "[SURVIVAL] deathmatch starts in %d minutes", m_survival_dm_countdown / (Server()->TickSpeed() * 60));
		SendSurvivalChat(aBuf);
	}
}

void CGameContext::SurvivalStartGame()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	vec2 SurvivalGameSpawnTile = Collision()->GetRandomTile(TILE_SURVIVAL_SPAWN);

	if (SurvivalGameSpawnTile == vec2(-1, -1)) //no survival arena
	{
		SurvivalSetGameState(SURVIVAL_LOBBY);
		SendSurvivalChat("[SURVIVAL] no survival arena set.");
		return;
	}
	else
	{
		SurvivalSetGameState(SURVIVAL_INGAME);
		SendSurvivalChat("[SURVIVAL] GAME STARTED !!!");
		//SendSurvivalBroadcast("STAY ALIVE!!!");
		SendSurvivalBroadcast(""); // clear countdown
		SurvivalCheckWinnerAndDeathMatch();
	}
}

void CGameContext::SendSurvivalChat(const char * pMsg)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i])
		{
			if (m_apPlayers[i]->m_IsSurvivaling)
			{
				SendChatTarget(i, pMsg);
			}
		}
	}
}

void CGameContext::SendSurvivalBroadcast(const char * pMsg)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i])
		{
			if (m_apPlayers[i]->m_IsSurvivaling)
			{
				SendBroadcast(pMsg, i);
			}
		}
	}
}

void CGameContext::SetPlayerSurvival(int id, int mode) //0=off 1=lobby 2=ingame 3=die
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_apPlayers[id])
	{
		if (mode == SURVIVAL_OFF)
		{
			m_apPlayers[id]->m_IsSurvivaling = false;
			m_apPlayers[id]->m_IsVanillaDmg = false;
			m_apPlayers[id]->m_IsVanillaWeapons = false;
			m_apPlayers[id]->m_IsVanillaCompetetive = false;
			m_apPlayers[id]->m_IsSurvivalAlive = false;
		}
		else if (mode == SURVIVAL_LOBBY)
		{
			m_apPlayers[id]->m_IsSurvivalAlive = false;
			m_apPlayers[id]->m_IsSurvivaling = true;
			m_apPlayers[id]->m_IsVanillaDmg = true;
			m_apPlayers[id]->m_IsVanillaWeapons = true;
			m_apPlayers[id]->m_IsVanillaCompetetive = true;
			m_apPlayers[id]->m_IsSurvivalLobby = true;
			if (!m_survivalgamestate) //no game running --> start lobby
			{
				SurvivalSetGameState(SURVIVAL_LOBBY);
				dbg_msg("survival", "lobby started");
			}
		}
		else if (mode == SURVIVAL_INGAME)
		{
			m_apPlayers[id]->m_IsSurvivalAlive = true;
			m_apPlayers[id]->m_IsSurvivaling = true;
			m_apPlayers[id]->m_IsVanillaDmg = true;
			m_apPlayers[id]->m_IsVanillaWeapons = true;
			m_apPlayers[id]->m_IsVanillaCompetetive = true;
			m_apPlayers[id]->m_IsSurvivalLobby = false;
			m_apPlayers[id]->m_IsSurvivalWinner = false;
		}
		else if (mode == SURVIVAL_DIE)
		{
			m_apPlayers[id]->m_IsSurvivalAlive = false;
			m_apPlayers[id]->m_IsSurvivalLobby = true;
			m_apPlayers[id]->m_SurvivalDeaths++;
		}
		else
		{
			dbg_msg("survival", "WARNING setted undefined mode %d", mode);
		}
	}
}

int CGameContext::SurvivalGetRandomAliveID(int NotThis)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int r = rand() % CountSurvivalPlayers(true);
	int x = 0;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (i == NotThis)
			continue;
		if (!m_apPlayers[i])
			continue;
		if (m_apPlayers[i]->m_IsSurvivaling && m_apPlayers[i]->m_IsSurvivalAlive)
		{
			if (x++ == r) { return i; }
		}
	}
	return -1;
}

void CGameContext::SurvivalGetNextSpectator(int UpdateID, int KillerID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[UpdateID];
	if (!pPlayer)
		return;

	int AliveTees = CountSurvivalPlayers(true);
	if (AliveTees > 1)
	{
		pPlayer->m_SpectatorID = UpdateID == KillerID ? SurvivalGetRandomAliveID() : KillerID;
		pPlayer->m_Paused = CPlayer::PAUSED_SPEC;
	}
	else
	{
		pPlayer->m_Paused = CPlayer::PAUSED_NONE;
	}
}

void CGameContext::SurvivalUpdateSpectators(int DiedID, int KillerID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!m_apPlayers[i] || !m_apPlayers[i]->m_IsSurvivaling)
			continue;
		if (m_apPlayers[i]->m_SpectatorID == DiedID)
		{
			SurvivalGetNextSpectator(i, KillerID);
		}
	}
}

int CGameContext::CountSurvivalPlayers(bool Alive)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int x = 0;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i])
		{
			if (m_apPlayers[i]->m_IsSurvivaling && (!Alive || m_apPlayers[i]->m_IsSurvivalAlive))
			{
				x++;
			}
		}
	}
	return x;
}

void CGameContext::SurvivalSetGameState(int state)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_survivalgamestate = state;
	if (state == SURVIVAL_OFF)
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (m_apPlayers[i])
			{
				SetPlayerSurvival(i, SURVIVAL_OFF);
			}
		}
	}
	else if (state == SURVIVAL_LOBBY)
	{
		m_survivallobbycountdown = Server()->TickSpeed() * g_Config.m_SvSurvivalLobbyDelay;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (m_apPlayers[i] && m_apPlayers[i]->m_IsSurvivaling)
			{
				m_apPlayers[i]->m_Paused = CPlayer::PAUSED_NONE;
			}
		}
	}
	else if (state == SURVIVAL_INGAME)
	{
		m_survival_game_countdown = g_Config.m_SvSurvivalMaxGameTime ? Server()->TickSpeed() * (g_Config.m_SvSurvivalMaxGameTime * 60) : -1;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (m_apPlayers[i] && m_apPlayers[i]->m_IsSurvivaling)
			{
				if (m_apPlayers[i]->GetCharacter()) //only kill if isnt dead already or server crashes (he should respawn correctly anayways)
				{
					SaveCosmetics(i);
					m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_GAME);
				}
				m_apPlayers[i]->m_Paused = CPlayer::PAUSED_NONE;
				SetPlayerSurvival(i, SURVIVAL_INGAME);
			}
		}
		m_survival_start_players = CountSurvivalPlayers(true); // all should be alive at game start. But in case we implment a afk state it should only count the active ones.
	}
	else if (state == SURVIVAL_DM_COUNTDOWN)
	{
		m_survival_dm_countdown = (Server()->TickSpeed() * 60) * g_Config.m_SvSurvivalDmDelay;
	}
	else if (state == SURVIVAL_DM)
	{
		SendSurvivalChat("[SURVIVAL] teleporting survivors to deathmatch arena.");

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (m_apPlayers[i] && m_apPlayers[i]->GetCharacter() && m_apPlayers[i]->m_IsSurvivaling && m_apPlayers[i]->m_IsSurvivalAlive)
			{
				vec2 SurvivalSpawn = Collision()->GetRandomTile(TILE_SURVIVAL_DEATHMATCH);

				if (SurvivalSpawn != vec2(-1, -1))
				{
					m_apPlayers[i]->GetCharacter()->SetPosition(SurvivalSpawn);
				}
				else //no dm spawn tile
				{
					SendSurvivalChat("[SURVIVAL] error no deathmatch arena found.");
					break;
				}
			}
		}
	}
}

bool CGameContext::SurvivalPickWinner()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int winnerID = -1;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && m_apPlayers[i]->m_IsSurvivaling && m_apPlayers[i]->m_IsSurvivalAlive)
		{
			winnerID = i;
			break;
		}
	}
	if (winnerID == -1) { return false; }
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "[SURVIVAL] '%s' won the game!", Server()->ClientName(winnerID));
	SendSurvivalChat(aBuf);
	SendSurvivalBroadcast(aBuf);
	m_apPlayers[winnerID]->m_IsSurvivalWinner = true;

	if (m_apPlayers[winnerID]->m_AccountID > 0)
	{
		SendChatTarget(winnerID, "[SURVIVAL] you won! [+50xp] [+50money]");
		m_apPlayers[winnerID]->MoneyTransaction(+50, "+50 (survival)");
		GiveXp(winnerID, 50);
	}
	else
	{
		SendChatTarget(winnerID, "[SURVIVAL] you won!");
	}

	str_copy(m_aLastSurvivalWinnerName, Server()->ClientName(winnerID), sizeof(m_aLastSurvivalWinnerName));
	m_apPlayers[winnerID]->m_SurvivalWins++;
	m_apPlayers[winnerID]->m_SurvivalDeaths--; //hacky method too keep deaths the same (because they get incremented in the next step)
	SetPlayerSurvival(winnerID, 3); //also set winner to dead now so that he can see names in lobby and respawns in lobby
	return true;
}

void CGameContext::BlockTournaTick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[128];

	if (m_BlockTournaState == 2) //ingame
	{
		m_BlockTournaTick++;
		if (m_BlockTournaTick > g_Config.m_SvBlockTournaGameTime * Server()->TickSpeed() * 60) //time over --> draw
		{
			//kill all tournas
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (m_apPlayers[i] && m_apPlayers[i]->m_IsBlockTourning)
				{
					m_apPlayers[i]->m_IsBlockTourning = false;
					if (m_apPlayers[i]->GetCharacter())
					{
						m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_GAME);
					}
				}
			}
			SendChat(-1, CGameContext::CHAT_ALL, "[EVENT] Block tournament stopped because time was over.");
			m_BlockTournaState = 0;
		}
	}
	else if (m_BlockTournaState == 1)
	{
		m_BlockTournaLobbyTick--;
		if (m_BlockTournaLobbyTick % Server()->TickSpeed() == 0)
		{
			int blockers = CountBlockTournaAlive();
			if (blockers < 0)
			{
				blockers = 1;
			}
			str_format(aBuf, sizeof(aBuf), "[EVENT] BLOCK IN %d SECONDS\n[%d/%d] '/join'ed already", m_BlockTournaLobbyTick / Server()->TickSpeed(), blockers, g_Config.m_SvBlockTournaPlayers);
			SendBroadcastAll(aBuf, 2);
		}


		if (m_BlockTournaLobbyTick < 0)
		{
			m_BlockTournaStartPlayers = CountBlockTournaAlive();
			if (m_BlockTournaStartPlayers < g_Config.m_SvBlockTournaPlayers) //minimum x players needed to start a tourna
			{
				SendBroadcastAll("[EVENT] Block tournament failed! Not enough players.", 2);
				EndBlockTourna();
				return;
			}


			SendBroadcastAll("[EVENT] Block tournament started!", 2);
			m_BlockTournaState = 2;
			m_BlockTournaTick = 0;

			//ready all players
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (m_apPlayers[i] && m_apPlayers[i]->m_IsBlockTourning)
				{
					if (m_apPlayers[i]->GetCharacter())
					{
						//delete weapons
						m_apPlayers[i]->GetCharacter()->SetActiveWeapon(WEAPON_GUN);
						m_apPlayers[i]->GetCharacter()->SetWeaponGot(2, false);
						m_apPlayers[i]->GetCharacter()->SetWeaponGot(3, false);
						m_apPlayers[i]->GetCharacter()->SetWeaponGot(4, false);

						//delete cosmentics (they are not competetive)
						DeleteCosmetics(i);

						//delete "cheats" from the race
						m_apPlayers[i]->GetCharacter()->m_Jetpack = false;
						m_apPlayers[i]->GetCharacter()->m_EndlessHook = false;
						m_apPlayers[i]->GetCharacter()->m_SuperJump = false;

						//kill speed
						m_apPlayers[i]->GetCharacter()->KillSpeed();

						//teleport
						vec2 BlockPlayerSpawn = Collision()->GetRandomTile(TILE_BLOCK_TOURNA_SPAWN);

						if (BlockPlayerSpawn != vec2(-1, -1))
						{
							m_apPlayers[i]->GetCharacter()->SetPosition(BlockPlayerSpawn);
						}
						else //no tile found
						{
							SendBroadcastAll("[EVENT] Block tournament failed! No spawntiles found.", 2);
							EndBlockTourna();
							return;
						}

						//freeze to get a fair start nobody should be surprised
						m_apPlayers[i]->GetCharacter()->UnFreeze();
						m_apPlayers[i]->GetCharacter()->Freeze(6);
					}
					else
					{
						m_apPlayers[i]->m_IsBlockTourning = false;
						SendChatTarget(i, "[BLOCK] you didn't join because you were dead on tournament start.");
					}
				}
			}
		}
	}
}

void CGameContext::EndBlockTourna()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_BlockTournaState = 0;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i])
		{
			m_apPlayers[i]->m_IsBlockTourning = false;
		}
	}
}

int CGameContext::CountBlockTournaAlive()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int c = 0;
	int id = -404;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i])
		{
			if (m_apPlayers[i]->m_IsBlockTourning)
			{
				c++;
				id = i;
			}
		}
	}

	if (c == 1) //one alive? --> return his id negative
	{
		if (id == 0)
		{
			return -420;
		}
		else
		{
			return id * -1;
		}
	}

	return c;
}

const char * CGameContext::GetBlockSkillGroup(int id)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[id];
	if (!pPlayer)
		return "error";

	if (pPlayer->m_BlockSkill < 1000)
	{
		return "nameless tee";
	}
	else if (pPlayer->m_BlockSkill < 3000)
	{
		return "brainless tee";
	}
	else if (pPlayer->m_BlockSkill < 6000)
	{
		return "novice tee";
	}
	else if (pPlayer->m_BlockSkill < 9000)
	{
		return "moderate tee";
	}
	else if (pPlayer->m_BlockSkill < 15000)
	{
		return "brutal tee";
	}
	else if (pPlayer->m_BlockSkill >= 20000)
	{
		return "insane tee";
	}
	else
	{
		return "unranked";
	}
}

int CGameContext::GetBlockSkillGroupInt(int id)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[id];
	if (!pPlayer)
		return -1;

	if (pPlayer->m_BlockSkill < 1000)
	{
		return 1;
	}
	else if (pPlayer->m_BlockSkill < 3000)
	{
		return 2;
	}
	else if (pPlayer->m_BlockSkill < 6000)
	{
		return 3;
	}
	else if (pPlayer->m_BlockSkill < 9000)
	{
		return 4;
	}
	else if (pPlayer->m_BlockSkill < 15000)
	{
		return 5;
	}
	else if (pPlayer->m_BlockSkill >= 20000)
	{
		return 6;
	}
	else
	{
		return 0;
	}
}

void CGameContext::UpdateBlockSkill(int value, int id)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[id];
	if (!pPlayer)
		return;

	int oldrank = GetBlockSkillGroupInt(id);
	pPlayer->m_BlockSkill += value; //update skill
	if (pPlayer->m_BlockSkill < 0)
	{
		pPlayer->m_BlockSkill = 0; //never go less than zero
	}
	else if (pPlayer->m_BlockSkill > 25000)
	{
		pPlayer->m_BlockSkill = 25000; //max skill lvl
	}
	int newrank = GetBlockSkillGroupInt(id);
	if (newrank != oldrank)
	{
		char aBuf[128];
		if (newrank < oldrank) //downrank
		{
			str_format(aBuf, sizeof(aBuf), "[BLOCK] New skillgroup '%s' (downrank)", GetBlockSkillGroup(id));
			SendChatTarget(id, aBuf);
			UpdateBlockSkill(-590, id); //lower skill agian to not get an uprank too fast agian
		}
		else //uprank
		{
			str_format(aBuf, sizeof(aBuf), "[BLOCK] New skillgroup '%s' (uprank)", GetBlockSkillGroup(id));
			SendChatTarget(id, aBuf);
			UpdateBlockSkill(+590, id); //push skill agian to not get an downrank too fast agian
		}
	}
}

void CGameContext::BlockWaveAddBots()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int OccSlots = 0;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i])
		{
			OccSlots++;
	}
}
	int FreeSlots = MAX_CLIENTS - OccSlots;



	if (m_BlockWaveRound < 15 + 1) //max 15 bots
	{
		for (int i = 1; i < m_BlockWaveRound + 1; i++)
		{
			CreateNewDummy(-3, true);
			if (i > FreeSlots - 5) //always leave 5 slots free for people to join
			{
				dbg_msg("BlockWave", "Stopped connecting at %d/%d bots because server has only %d free slots", i, m_BlockWaveRound + 1, FreeSlots);
				break;
			}
		}
	}
	else
	{
		for (int i = 1; i < 15 + 1; i++)
		{
			CreateNewDummy(-3, true);
			if (i > FreeSlots - 5) //always leave 5 slots free for people to join
			{
				dbg_msg("BlockWave", "Stopped connecting at %d/15 + 1 bots because server has only %d free slots", i, FreeSlots);
				break;
			}
		}
	}
}

void CGameContext::BlockWaveWonRound()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_BlockWaveRound++;
	SendBlockWaveSay("[BlockWave] round survived.");
	m_BlockWaveGameState = 1;

	//respawn all dead humans
	vec2 BlockWaveSpawnTile = Collision()->GetRandomTile(TILE_BLOCKWAVE_HUMAN);

	if (BlockWaveSpawnTile != vec2(-1, -1))
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (m_apPlayers[i] && m_apPlayers[i]->m_IsBlockWaving)
			{
				if ((!m_apPlayers[i]->m_IsDummy && m_apPlayers[i]->GetCharacter())
					&& (m_apPlayers[i]->GetCharacter()->m_FreezeTime || m_apPlayers[i]->m_IsBlockWaveWaiting)) //queue dudes waiting to join on new round or frozen ingames
				{
					m_apPlayers[i]->GetCharacter()->SetPosition(BlockWaveSpawnTile);
				}
				if (!m_apPlayers[i]->GetCharacter() || m_apPlayers[i]->m_IsBlockWaveWaiting) //if some queue dude is dead while waiting to join set him unqueue --> so on respawn he will enter the area
				{
					m_apPlayers[i]->m_IsBlockWaveWaiting = false;
				}
			}
		}
	}
	else //no BlockWaveSpawnTile
	{
		//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[BlockWave] No arena set.");
		m_BlockWaveGameState = 0;
	}

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i])
		{
			m_apPlayers[i]->m_IsBlockWaveDead = false; //noboy is dead on new round
			if (m_apPlayers[i]->m_IsBlockWaving && m_apPlayers[i]->m_IsDummy) //disconnect dummys
			{
				Server()->BotLeave(i, true);
			}
		}
	}
}

void CGameContext::StartBlockWaveGame()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
	dbg_msg("Blockwave", "Game started.");
#endif
	if (m_BlockWaveGameState) { return; } //no resatrt only start if not started yet
	m_BlockWaveGameState = 1;
	m_BlockWaveRound = 1; //reset rounds 
	m_BlockWavePrepareDelay = (10 * Server()->TickSpeed());
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i])
		{
			m_apPlayers[i]->m_IsBlockWaveDead = false;
		}
	}
}

void CGameContext::BlockWaveGameTick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[256];

	if (m_BlockWaveGameState == 1)
	{
		m_BlockWavePrepareDelay--;
		if (m_BlockWavePrepareDelay % Server()->TickSpeed() == 0)
		{
			str_format(aBuf, sizeof(aBuf), "[BlockWave] round %d starts in %d seconds", m_BlockWaveRound, m_BlockWavePrepareDelay / Server()->TickSpeed());
			SendBlockWaveBroadcast(aBuf);
		}
		if (m_BlockWavePrepareDelay < 0)
		{
			SendBlockWaveBroadcast("[BlockWave] Have fun and good luck!");
			m_BlockWaveGameState = 2; //start round!
			m_BlockWavePrepareDelay = (10 * Server()->TickSpeed()); //could add a cfg var in secs instead of 10 here
			BlockWaveAddBots();
		}
	}
	else //running round
	{
		//check for rip round or win round
		if (Server()->Tick() % 60 == 0)
		{
			bool ripall = true;
			bool won = true;
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (m_apPlayers[i] && m_apPlayers[i]->m_IsBlockWaving && !m_apPlayers[i]->m_IsBlockWaveDead && !m_apPlayers[i]->m_IsDummy)
				{
					ripall = false;
					break;
				}
			}
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (m_apPlayers[i] && m_apPlayers[i]->m_IsBlockWaving && !m_apPlayers[i]->m_IsBlockWaveDead && m_apPlayers[i]->m_IsDummy)
				{
					won = false;
					break;
				}
			}
			if (ripall)
			{
				BlockWaveStartNewGame();
			}
			if (won)
			{
				BlockWaveWonRound();
			}
		}
	}
}

void CGameContext::BlockWaveEndGame()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "[BlockWave] You lost! Survived %d rounds.", m_BlockWaveRound);
	SendBlockWaveSay(aBuf);
}

void CGameContext::BlockWaveStartNewGame()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	BlockWaveEndGame(); //send message to all players
	m_BlockWaveGameState = 0; //end old game
	StartBlockWaveGame(); //start new game
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && m_apPlayers[i]->m_IsBlockWaving && m_apPlayers[i]->GetCharacter())
		{
			m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_GAME);
			if (m_apPlayers[i]->m_IsDummy)
			{
				Server()->BotLeave(i, true);
			}
		}
	}
}

int CGameContext::CountBlockWavePlayers()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int c = 0;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && m_apPlayers[i]->m_IsBlockWaving)
		{
			c++;
		}
	}
	return c;
}

void CGameContext::SendBlockWaveBroadcast(const char * pMsg)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && m_apPlayers[i]->m_IsBlockWaving)
		{
			SendBroadcast(pMsg, i);
		}
	}
}

void CGameContext::SendBlockWaveSay(const char * pMsg)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && m_apPlayers[i]->m_IsBlockWaving)
		{
			SendChatTarget(i, pMsg);
		}
	}
}

void CGameContext::QuestReset(int playerID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!m_apPlayers[playerID])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player. Quest has been reset.", playerID, Server()->ClientName(playerID));
		return;
	}
	m_apPlayers[playerID]->m_QuestProgressValue = 0;
	m_apPlayers[playerID]->m_QuestProgressValue2 = 0;
	m_apPlayers[playerID]->m_QuestProgressBool = 0;
	m_apPlayers[playerID]->m_QuestPlayerID = -1;
	m_apPlayers[playerID]->m_QuestLastQuestedPlayerID = -1;
	m_apPlayers[playerID]->m_aQuestProgress[0] = -1;
	m_apPlayers[playerID]->m_aQuestProgress[1] = -1;
}

void CGameContext::QuestFailed(int playerID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!m_apPlayers[playerID])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player failed the quest.", playerID, Server()->ClientName(playerID));
		return;
	}
	if (!m_apPlayers[playerID]->m_QuestState)
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] failed a quest without being in a quest.", playerID, Server()->ClientName(playerID));
		return;
	}
	QuestReset(playerID);
	if (!m_apPlayers[playerID]->m_HideQuestWarning)
	{
		SendChatTarget(playerID, "[QUEST] You failed the quest.");
	}
	StartQuest(playerID);
}

void CGameContext::QuestFailed2(int playerID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!m_apPlayers[playerID])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player failed the quest", playerID, Server()->ClientName(playerID));
		return;
	}
	if (!m_apPlayers[playerID]->m_QuestState)
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] failed a quest without being in a quest.", playerID, Server()->ClientName(playerID));
		return;
	}
	if (m_apPlayers[playerID]->m_QuestFailed)
	{
		return;
	}
	m_apPlayers[playerID]->m_QuestFailed = true;
	//str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Quest failed."); //dont overwrite info what to do and how to start agian. I added a questfailed info in ddracechat.cpp
	QuestReset(playerID);
	if (!m_apPlayers[playerID]->m_HideQuestWarning)
	{
		SendChatTarget(playerID, "[QUEST] You failed the quest.");
	}
}

bool CGameContext::QuestAddProgress(int playerID, int globalMAX, int localMAX)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[256];
	if (!m_apPlayers[playerID])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player added progress.", playerID, Server()->ClientName(playerID));
		return false;
	}
	if (localMAX == -1)
	{
		localMAX = globalMAX;
	}
	if (m_apPlayers[playerID]->m_QuestProgressValue >= localMAX)
	{
		return false;
	}

	m_apPlayers[playerID]->m_QuestProgressValue++;
	m_apPlayers[playerID]->m_aQuestProgress[0] = m_apPlayers[playerID]->m_QuestProgressValue;
	m_apPlayers[playerID]->m_aQuestProgress[1] = globalMAX;

	
	//dbg_msg("QUEST", "Progress updated: %d/%d", m_apPlayers[playerID]->m_aQuestProgress[0], m_apPlayers[playerID]->m_aQuestProgress[1]);
	str_format(aBuf, sizeof(aBuf), "[QUEST] progress %d/%d", m_apPlayers[playerID]->m_QuestProgressValue, globalMAX);

	if (!m_apPlayers[playerID]->m_HideQuestProgress)
		SendChatTarget(playerID, aBuf);

	if (m_apPlayers[playerID]->m_QuestProgressValue >= globalMAX)
	{
		QuestCompleted(playerID);
	}
	
	return true;
}

void CGameContext::QuestCompleted(int playerID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[playerID];
	if (!pPlayer)
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player completed the quest", playerID, Server()->ClientName(playerID));
		return;
	}
	if (!pPlayer->m_QuestState)
	{
		dbg_msg("QUEST", "Warning! [%d][%s] completed quest without having it activated", pPlayer->GetCID(), Server()->ClientName(pPlayer->GetCID()));
		return;
	}

	// reward
	char aBuf[256];
	int RewardMoney = pPlayer->m_QuestStateLevel ? 100 : 50;
	int RewardXP = QuestReward(playerID);
	if (pPlayer->IsMaxLevel())
		str_format(aBuf, sizeof(aBuf), "[QUEST] Quest %d (lvl %d) completed. [+%d money]", pPlayer->m_QuestState, pPlayer->m_QuestStateLevel, RewardXP, RewardMoney);
	else // xp msg only if not max lvl
		str_format(aBuf, sizeof(aBuf), "[QUEST] Quest %d (lvl %d) completed. [+%d xp] [+%d money]", pPlayer->m_QuestState, pPlayer->m_QuestStateLevel, RewardXP, RewardMoney);
	SendChatTarget(playerID, aBuf);
	str_format(aBuf, sizeof(aBuf), "+%d (quest)", RewardMoney);
	pPlayer->MoneyTransaction(+100, aBuf);
	GiveXp(playerID, RewardXP);

	// next quest
	QuestReset(playerID);
	pPlayer->m_QuestState++;
	pPlayer->m_QuestUnlocked = pPlayer->m_QuestState; //save highscore
	if (pPlayer->m_QuestState > 5) // <--- update value depending on how many quests TODO: add a constant that replaces this magic number
	{
		pPlayer->m_QuestState = 1; // start at quest 1 in the next level
		pPlayer->m_QuestStateLevel++;
		str_format(aBuf, sizeof(aBuf), "[QUEST] level up... you are now level %d !", pPlayer->m_QuestStateLevel);
		if (pPlayer->m_QuestStateLevel > 9) // <--- update value depending on how many questlevels TODO: add a constant that replaces this magic number
		{
			pPlayer->m_QuestState = 0;
			pPlayer->m_QuestStateLevel = 0;
			SendChatTarget(playerID, "[QUEST] Hurray you finished all Quests !!!");
			return;
		}
		pPlayer->m_QuestLevelUnlocked = pPlayer->m_QuestStateLevel; // save highscore
		SendChatTarget(playerID, aBuf);
	}


	StartQuest(playerID);
}

int CGameContext::QuestReward(int playerID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!m_apPlayers[playerID])
	{
		return 0;
	}

	if (!m_apPlayers[playerID]->m_QuestStateLevel)
	{
		return 10;
	}
	else
	{
		return m_apPlayers[playerID]->m_QuestStateLevel * 100;
	}
}

//void CGameContext::PickNextQuest(int playerID)
//{
//#if defined(CONF_DEBUG)
//	CALL_STACK_ADD();
//#endif
//	m_apPlayers[playerID]->m_QuestState++;
//	StartQuest(playerID); 
//}

void CGameContext::StartQuest(int playerID) 
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!m_apPlayers[playerID])
	{
		dbg_msg("QUEST", "WARNING! [%d][%s] invalid player has started the quest.", playerID, Server()->ClientName(playerID));
		return;
	}

	char aBuf[256];
	QuestReset(playerID); //not needed but save clearup (should already be cleared up on every quest exit but save is save)
	int level = m_apPlayers[playerID]->m_QuestStateLevel;
	int quest = m_apPlayers[playerID]->m_QuestState; //old and bad because with many quests this can take forever and easts ressources of server or players have to do quests over and over agian rand() % 4 + 1; //valid quests + 1
	str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "ERROR invalid quest loaded");

	if (quest == 0)
	{
		dbg_msg("debug", "WARNING: QuestPicker triggered on non-questing player [%d][%s] [QUEST=%d LEVEL=%d]", m_apPlayers[playerID]->GetCID(), Server()->ClientName(m_apPlayers[playerID]->GetCID()), quest, level);
		return;
	}
	else if (quest == 1)
	{
		if (level == 0)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer 1 tee.");
		}
		else if (level == 1)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer 2 tees.");
		}
		else if (level == 2)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer 3 tees.");
		}
		else if (level == 3)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer 5 tees.");
		}
		else if (level == 4)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer 10 freezed tees.");
		}
		else if (level == 5)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer '%s' 20 times.", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if (level == 6)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer freezed '%s' 3 times.", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if (level == 7)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer '%s' 10 times and then block him.", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if (level == 8)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer 2 tees with one hit.");
		}
		else if (level == 9)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Hammer 10 freezed tees while holding the flag.");
		}
		else
		{
			dbg_msg("debug", "ERROR: invalid quest level [QUEST=%d LEVEL=%d]", quest, level);
			quest = 0;
		}
	}
	else if (quest == 2)
	{
		if (level == 0)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 1 tee.");
		}
		else if (level == 1)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 2 tees.");
		}
		else if (level == 2)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 3 tees.");
		}
		else if (level == 3)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 5 tees.");
		}
		else if (level == 4)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 10 tees without using any weapons.");
		}
		else if (level == 5)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 5 tees and then block '%s'.", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if (level == 6)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block a tee which is on a 5 blockingspree.");
		}
		else if (level == 7)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 11 tees without getting blocked.");
		}
		else if (level == 8)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 3 tees without using hook.");
		}
		else if (level == 9)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Block 11 tees while holding the flag without dying.");
		}
		else
		{
			dbg_msg("debug", "ERROR: invalid quest level [QUEST=%d LEVEL=%d]", quest, level);
			quest = 0;
		}
	}
	else if (quest == 3)
	{
		if (level == 0)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race.");
		}
		else if (level == 1)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race in under %d seconds.", g_Config.m_SvQuestRaceTime1);
		}
		else if (level == 2)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race in under %d seconds.", g_Config.m_SvQuestRaceTime2); 
		}
		else if (level == 3)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race backwards."); 
		}
		else if (level == 4)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race in under %d seconds.", g_Config.m_SvQuestRaceTime3);
		}
		else if (level == 5)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race with the flag."); 
		}
		else if (level == 6)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the special race."); 
		}
		else if (level == 7)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the special race in under %d seconds.", g_Config.m_SvQuestSpecialRaceTime); 
		}
		else if (level == 8)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the special race backwards."); 
		}
		else if (level == 9)
		{
			if (g_Config.m_SvQuestRaceCondition == 0)
			{
				str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race without using hammer.");
			}
			else if (g_Config.m_SvQuestRaceCondition == 1)
			{
				str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race without using gun.");
			}
			else if (g_Config.m_SvQuestRaceCondition == 2)
			{
				str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race without using shotgun.");
			}
			else if (g_Config.m_SvQuestRaceCondition == 3)
			{
				str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race without using grenade.");
			}
			else if (g_Config.m_SvQuestRaceCondition == 4)
			{
				str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race without using rifle.");
			}
			else if (g_Config.m_SvQuestRaceCondition == 5)
			{
				str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Finish the race without using ninja.");
			}
			else
			{
				dbg_msg("debug", "ERROR: invalid race condition [%d] at [QUEST=%d LEVEL=%d]", g_Config.m_SvQuestRaceCondition, quest, level);
				quest = 0;	
			}
		}
		else
		{
			dbg_msg("debug", "ERROR: invalid quest level [QUEST=%d LEVEL=%d]", quest, level);
			quest = 0;
		}
	}
	else if (quest == 4)
	{
		if (level == 0)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle 1 tee");
		}
		else if (level == 1)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle '%s' 5 times", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if (level == 2)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle freezed '%s' 5 times", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if (level == 3)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle '%s' and 10 others", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if (level == 4)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle 10 freezed tees");
		}
		else if (level == 5)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle yourself while being freezed");
		}
		else if (level == 6)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle yourself while being freezed 10 times");
		}
		else if (level == 7)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle '%s' and then block him", Server()->ClientName(PickQuestPlayer(playerID)));
		}
		else if (level == 8)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle 5 tees before blocking them");
		}
		else if (level == 9)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Rifle 20 freezed tees while having the flag");
		}
		else
		{
			dbg_msg("debug", "ERROR: invalid quest level [QUEST=%d LEVEL=%d]", quest, level);
			quest = 0;
		}
	}
	else if (quest == 5) 
	{
		if (level == 0)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 10 money on a moneytile");
		}
		else if (level == 1)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 20 money on a moneytile");
		}
		else if (level == 2)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 30 money on a moneytile");
		}
		else if (level == 3)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 40 money on a moneytile");
		}
		else if (level == 4)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 50 money on a moneytile");
		}
		else if (level == 5)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 60 money on a moneytile");
		}
		else if (level == 6)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 70 money on a moneytile");
		}
		else if (level == 7)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 100 money on a police moneytile");
		}
		else if (level == 8)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 100 money on a moneytile");
		}
		else if (level == 9)
		{
			str_format(m_apPlayers[playerID]->m_aQuestString, sizeof(m_apPlayers[playerID]->m_aQuestString), "Farm 200 xp with the flag");
		}
		else
		{
			dbg_msg("debug", "ERROR: invalid quest level [QUEST=%d LEVEL=%d]", quest, level);
			quest = 0;
		}
	}
	else
	{
		dbg_msg("debug", "ERROR: invalid quest [QUEST=%d LEVEL=%d]", quest, level);
		quest = 0;
	}


	if (m_apPlayers[playerID]->m_QuestState && quest)
	{
		str_format(aBuf, sizeof(aBuf), "[QUEST] %s", m_apPlayers[playerID]->m_aQuestString);
		SendBroadcast(aBuf, m_apPlayers[playerID]->GetCID());
		str_format(aBuf, sizeof(aBuf), "[QUEST] New Quest: %s", m_apPlayers[playerID]->m_aQuestString);
		SendChatTarget(m_apPlayers[playerID]->GetCID(), aBuf);
		return;
	}

	//quest stopped during the next quest election
	SendBroadcast("[QUEST] stopped", m_apPlayers[playerID]->GetCID());
	m_apPlayers[playerID]->m_QuestState = 0;
 }

 int CGameContext::PickQuestPlayer(int playerID)
 {
#if defined(CONF_DEBUG)
	 CALL_STACK_ADD();
#endif
	 if (!m_apPlayers[playerID])
	 {
		 dbg_msg("QUEST", "WARNING! [%d][%s] invalid player picked a quest", playerID, Server()->ClientName(playerID));
		 return -1;
	 }

	 int ID = -1;
	 int FoundTees[MAX_CLIENTS];
	 int Index = 0;
	 int FoundDeadTees[MAX_CLIENTS];
	 int IndexDead = 0;

	 for (int i = 0; i < MAX_CLIENTS; i++)
	 {
		 if (!m_apPlayers[i])
		 {
			 //dbg_msg("QUEST", "<PickPlayer> warning not exsisting player found");
			 continue;
		 }
		 if (m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS)
		 {
			 dbg_msg("QUEST", "<PickPlayer> warning spec player found");
			 continue;
		 }
		 if (IsSameIP(i, playerID))
		 {
			 //dummy found (also used to ignore the questing player it self. Keep this in mind if you remove or edit this one day)
			 dbg_msg("QUEST", "<PickPlayer> warning dummy found [%s]", Server()->ClientName(i)); //this will be triggerd for all serverside dummys if ur playing local -.-
			 continue;
		 }
		 if (m_apPlayers[i]->m_IsDummy && !g_Config.m_SvQuestCountBots)
		 {
			 //server side bot found
			 dbg_msg("QUEST", "<PickPlayer> warning found bot [%s]", Server()->ClientName(i));
			 continue;
		 }


		 //found valid non dummy or serverside bot
		 if (!GetPlayerChar(i)) //spec/dead players always second choice
		 {
			 // ---> store id + 1 in array so no 0 is in array and we can check if a tee is existing and stuff
			 FoundDeadTees[Index] = i + 1;
			 IndexDead++;
			 //dbg_msg("QUEST", "+1 dead player");
		 }
		 else //alive players primary choice
		 {
			 // ---> store id + 1 in array so no 0 is in array and we can check if a tee is existing and stuff
			 FoundTees[Index] = i + 1;
			 Index++;
			 //dbg_msg("QUEST", "+1 alive player");
		 }
	 }

	 if (Index < g_Config.m_SvQuestNeededPlayers) //not enough alive tees ---> check spectators
	 {
		 if (Index + IndexDead < g_Config.m_SvQuestNeededPlayers) //not enough dead or alive valid tees --> stop quest
		 {
			 m_apPlayers[playerID]->m_QuestState = 0;
			 SendChatTarget(playerID, "[QUEST] Quest stopped because there are not enough tees on the server.");
			 //dbg_msg("QUEST", "alive %d + dead %d = %d/%d tees to start a quest", Index, IndexDead, Index + IndexDead, g_Config.m_SvQuestNeededPlayers);
			 return -1;
		 }
		 else 
		 {
			 ID = FoundDeadTees[rand() % IndexDead]; //choose random one of the valid tees
			 if (!ID)
			 {
				 dbg_msg("QUEST", "WARNING! player [%d][%s] got invalid player [%d][%s] as specific quest", playerID, Server()->ClientName(playerID), ID, Server()->ClientName(ID));
				 m_apPlayers[playerID]->m_QuestState = 0;
				 SendChatTarget(playerID, "[QUEST] Quest stopped because something went wrong. (please contact an admin)");
				 SendChatTarget(playerID, "[QUEST] Try '/quest start' agian to load and start your quest agian");
				 return -1;
			 }

			 m_apPlayers[playerID]->m_QuestPlayerID = ID - 1;
			 return ID - 1; //before we stored id + 1 to have an better handling with false values
		 }
	 }

	 ID = FoundTees[rand() % Index]; //choose random one of the valid alive tees
	 if (!ID)
	 {
		 dbg_msg("QUEST", "WARNING! player [%d][%s] got invalid player [%d][%s] as specific quest", playerID, Server()->ClientName(playerID), ID, Server()->ClientName(ID));
		 m_apPlayers[playerID]->m_QuestState = 0;
		 SendChatTarget(playerID, "[QUEST] Quest stopped because something went wrong. (please contact an admin)");
		 SendChatTarget(playerID, "[QUEST] Try '/quest start' agian to load and start your quest agian");
		 return -1;
	 }

	 m_apPlayers[playerID]->m_QuestPlayerID = ID - 1;
	 return ID - 1; //before we stored id + 1 to have an better handling with false values
 }

void CGameContext::SendAllPolice(const char * pMessage)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "[POLICE-CHANNEL] %s", pMessage);
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && m_apPlayers[i]->m_PoliceRank)
		{
			SendChatTarget(i,aBuf);
		}
	}
}

void CGameContext::AddEscapeReason(int ID, const char * pReason)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	//dbg_msg("cBug", "current reaso is %s", m_apPlayers[ID]->m_aEscapeReason);

	//dont add already exsisting reasons agian
	if (str_find(m_apPlayers[ID]->m_aEscapeReason, pReason))
	{
		//dbg_msg("cBug", "skipping exsisting reason %s", pReason);
		return;
	}
	//reset all 
	if (!str_comp(pReason, "unknown"))
	{
		str_format(m_apPlayers[ID]->m_aEscapeReason, sizeof(m_apPlayers[ID]->m_aEscapeReason), "%s", pReason);
		//dbg_msg("cBug", "resetting to reason %s", pReason);
		return;
	}

	if (!str_comp(m_apPlayers[ID]->m_aEscapeReason, "unknown")) //keine vorstrafen
	{
		str_format(m_apPlayers[ID]->m_aEscapeReason, sizeof(m_apPlayers[ID]->m_aEscapeReason), "%s", pReason);
		dbg_msg("cBug", "set escape reason to %s -> %s", pReason, m_apPlayers[ID]->m_aEscapeReason);
	}
	else
	{
		str_format(m_apPlayers[ID]->m_aEscapeReason, sizeof(m_apPlayers[ID]->m_aEscapeReason), "%s, %s",m_apPlayers[ID]->m_aEscapeReason, pReason);
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[128];
	int GiveView = 1;

	if (!m_apPlayers[ViewedID] || !m_apPlayers[ViewerID])
	{
		return;
	}

	if (m_apPlayers[ViewedID]->m_AccountID <= 0)
	{
		SendChatTarget(ViewerID, "Player has to be logged in to view his profile.");
		return;
	}

	if (!str_comp(m_apPlayers[ViewerID]->m_LastViewedProfile, Server()->ClientName(ViewedID)) && !m_apPlayers[ViewerID]->m_IsProfileViewLoaded) //repeated same profile and view not loaded yet
	{
		GiveView = 0;
	}
	else
	{
		if (!str_comp(Server()->ClientName(ViewedID), Server()->ClientName(ViewerID))) //viewing own profile --> random xd
		{
			GiveView = rand() % 2;
		}
	}


	if (GiveView)
	{
		m_apPlayers[ViewedID]->m_ProfileViews++;
		str_copy(m_apPlayers[ViewerID]->m_LastViewedProfile, Server()->ClientName(ViewedID), 32);
		m_apPlayers[ViewerID]->m_IsProfileViewLoaded = false;
	}

	//ASCII - ANIMATIONS
	StartAsciiAnimation(ViewerID, ViewedID, 1);


	if (m_apPlayers[ViewedID]->m_ProfileStyle == 0)  //default
	{
		str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(ViewedID));
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[ViewedID]->m_ProfileStatus);
		SendChatTarget(ViewerID, aBuf);
		SendChatTarget(ViewerID, "-------------------------");
		str_format(aBuf, sizeof(aBuf), "Level: %d", m_apPlayers[ViewedID]->m_level);
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Money: %d", m_apPlayers[ViewedID]->m_money);
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Shit: %d", m_apPlayers[ViewedID]->m_shit);
		SendChatTarget(ViewerID, aBuf);
	}
	else if (m_apPlayers[ViewedID]->m_ProfileStyle == 1)  //shit
	{
		str_format(aBuf, sizeof(aBuf), "---  %s's Profile  ---", Server()->ClientName(ViewedID));
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "%s", m_apPlayers[ViewedID]->m_ProfileStatus);
		SendChatTarget(ViewerID, aBuf);
		SendChatTarget(ViewerID, "-------------------------");
		str_format(aBuf, sizeof(aBuf), "Shit: %d", m_apPlayers[ViewedID]->m_shit);
		SendChatTarget(ViewerID, aBuf);
	}
	else if (m_apPlayers[ViewedID]->m_ProfileStyle == 2)  //social
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
	else if (m_apPlayers[ViewedID]->m_ProfileStyle == 3)  //show-off
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
		str_format(aBuf, sizeof(aBuf), "Level: %d", m_apPlayers[ViewedID]->m_level);
		SendChatTarget(ViewerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Shit: %d", m_apPlayers[ViewedID]->m_shit);
		SendChatTarget(ViewerID, aBuf);
	}
	else if (m_apPlayers[ViewedID]->m_ProfileStyle == 4)  //pvp
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
	else if (m_apPlayers[ViewedID]->m_ProfileStyle == 5)  //bomber
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	SendChatTarget(ID, "============= admin login =============");
	int surv_error = TestSurvivalSpawns();
	if (surv_error == -1)
	{
		SendChatTarget(ID, "[ADMIN:Test] WARNING: less survival spawns on map than slots possible in ddnet++ (no problem as long as slots stay how they are)");
	}
	else if (surv_error == -2)
	{
		SendChatTarget(ID, "[ADMIN:Test] WARNING: not enough survival spawns (less survival spawns than slots)");
	}
	PrintSpecialCharUsers(ID);
}

int CGameContext::PrintSpecialCharUsers(int ID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aUsers[2048]; //wont show all users if too many special char users are there but this shouldnt be the case
	int users = 0;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && m_apPlayers[i]->m_AccountID > 0)
		{
			if (IsAllowedCharSet(m_apPlayers[i]->m_aAccountLoginName) == false)
			{
				if (!users)
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

	if (users)
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
	vec2 SurvivalGameSpawnTile = Collision()->GetSurvivalSpawn(g_Config.m_SvMaxClients, true);
	vec2 SurvivalGameSpawnTile2 = Collision()->GetSurvivalSpawn(MAX_CLIENTS, true);

	if (SurvivalGameSpawnTile == vec2(-1, -1))
	{
		//SendChatTarget(ClientID, "[ADMIN:Test] ERROR: not enough survival spawns (less survival spawns than slots)");
		return -2;
	}
	else if (SurvivalGameSpawnTile2 == vec2(-1, -1))
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
}

void CGameContext::DummyChat()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	//unused cuz me knoop putting all the stuff here
}

void CGameContext::WinInsta1on1(int WinnerID, int LooserID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
	if (!m_apPlayers[WinnerID])
		dbg_msg("cBug", "[WARNING] WinInsta1on1() at gamecontext.cpp");
#endif

	char aBuf[128];

	//WINNER
	if (m_apPlayers[WinnerID])
	{
		SendChatTarget(WinnerID, "==== Insta 1on1 WON ====");
		str_format(aBuf, sizeof(aBuf), "1. '%s' %d", Server()->ClientName(WinnerID), m_apPlayers[WinnerID]->m_Insta1on1_score);
		SendChatTarget(WinnerID, aBuf);
		str_format(aBuf, sizeof(aBuf), "2. '%s' %d", Server()->ClientName(LooserID), m_apPlayers[LooserID]->m_Insta1on1_score);
		SendChatTarget(WinnerID, aBuf);
		SendChatTarget(WinnerID, "==================");
		SendChatTarget(WinnerID, "+200 money for winning 1on1"); //actually it is only +100 because you have to pay to start an 1on1
		m_apPlayers[WinnerID]->MoneyTransaction(+200, "+200 (won insta 1on1)");

		m_apPlayers[WinnerID]->m_IsInstaArena_gdm = false;
		m_apPlayers[WinnerID]->m_IsInstaArena_idm = false;
		m_apPlayers[WinnerID]->m_IsInstaArena_fng = false;
		m_apPlayers[WinnerID]->m_Insta1on1_id = -1;
		if (m_apPlayers[WinnerID]->GetCharacter())
		{
			m_apPlayers[WinnerID]->GetCharacter()->Die(WinnerID, WEAPON_SELF);
		}
	}

	//LOOSER
	if (LooserID != -1)
	{
		SendChatTarget(LooserID, "==== Insta 1on1 LOST ====");
		str_format(aBuf, sizeof(aBuf), "1. '%s' %d", Server()->ClientName(WinnerID), m_apPlayers[WinnerID]->m_Insta1on1_score);
		SendChatTarget(LooserID, aBuf);
		str_format(aBuf, sizeof(aBuf), "2. '%s' %d", Server()->ClientName(LooserID), m_apPlayers[LooserID]->m_Insta1on1_score);
		SendChatTarget(LooserID, aBuf);
		SendChatTarget(LooserID, "==================");

		m_apPlayers[LooserID]->m_IsInstaArena_gdm = false;
		m_apPlayers[LooserID]->m_IsInstaArena_idm = false;
		m_apPlayers[LooserID]->m_IsInstaArena_fng = false;
		m_apPlayers[LooserID]->m_Insta1on1_id = -1;
		m_apPlayers[LooserID]->m_Insta1on1_score = 0;
		if (m_apPlayers[LooserID]->GetCharacter())
		{
			m_apPlayers[LooserID]->GetCharacter()->Die(LooserID, WEAPON_SELF); //needed for /insta leave where the looser culd be alive
		}
	}


	//RESET SCORE LAST CUZ SCOREBOARD
	if (m_apPlayers[WinnerID])
		m_apPlayers[WinnerID]->m_Insta1on1_score = 0;
	if (m_apPlayers[LooserID])
		m_apPlayers[LooserID]->m_Insta1on1_score = 0;
}

bool CGameContext::CanJoinInstaArena(bool grenade, bool PrivateMatch)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int cPlayer = 0;

	if (grenade)
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (m_apPlayers[i])
			{
				if (m_apPlayers[i]->m_IsInstaArena_gdm)
				{
					cPlayer++;
					if (m_apPlayers[i]->m_Insta1on1_id != -1) //if some1 is in 1on1
					{
						return false;
					}
				}
			}
		}

		if (cPlayer >= g_Config.m_SvGrenadeArenaSlots)
		{
			return false;
		}
	}
	else //rifle
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (m_apPlayers[i])
			{
				if (m_apPlayers[i]->m_IsInstaArena_idm)
				{
					cPlayer++;
					if (m_apPlayers[i]->m_Insta1on1_id != -1) //if some1 is in 1on1
					{
						return false;
					}
				}
			}
		}

		if (cPlayer >= g_Config.m_SvRifleArenaSlots)
		{
			return false;
		}
	}

	if (cPlayer && PrivateMatch)
	{
		return false;
	}

	return true;
}

void CGameContext::CreateBasicDummys()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!str_comp(g_Config.m_SvMap, "ChillBlock5"))
	{
		CreateNewDummy(31);//police
		//CreateNewDummy(30);//taxi (not needed in new cb5)
		CreateNewDummy(29);//blocker
		CreateNewDummy(29);//blocker 2
		CreateNewDummy(23);//racer
		CreateNewDummy(-6);//blocker dm v3
	}
	else if (!str_comp(g_Config.m_SvMap, "BlmapChill"))
	{
		CreateNewDummy(32);//police
		//CreateNewDummy(28);//racer
	}
	else if (!str_comp(g_Config.m_SvMap, "blmapV5"))
	{
		CreateNewDummy(104);//lower blocker
		CreateNewDummy(104);//lower blocker
		CreateNewDummy(105);//upper blocker
	}
	else if (!str_comp(g_Config.m_SvMap, "blmapV5_ddpp"))
	{
		CreateNewDummy(104);//lower blocker
		CreateNewDummy(104);//lower blocker
		CreateNewDummy(105);//upper blocker
		g_Config.m_SvDummyMapOffsetX = -226;
	}
	else if (!str_comp(g_Config.m_SvMap, "ddpp_survival"))
	{
		CreateNewDummy(34);//dynamic pvp mode
		CreateNewDummy(34);//dynamic pvp mode
	}
	else
	{
		CreateNewDummy(0); //dummy
		dbg_msg("basic_dummys", "waring map=%s not supported", g_Config.m_SvMap);
	}
	if (m_ShopBotTileExists)
	{
		m_CreateShopBot = true;
	}
	dbg_msg("basic_dummys","map=%s", g_Config.m_SvMap);
}

int CGameContext::CreateNewDummy(int dummymode, bool silent, int tile)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int DummyID = GetNextClientID();
	if (DummyID < 0)
	{
		dbg_msg("dummy", "Can't get ClientID. Server is full or something like that.");
		return -1;
	}

	if (m_apPlayers[DummyID])
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

	if (dummymode == -1) //balancedummy1 
	{
		m_apPlayers[DummyID]->m_IsBalanceBattlePlayer1 = true;
		m_apPlayers[DummyID]->m_IsBalanceBattleDummy = true;
	}
	else if (dummymode == -2) //balancedummy2
	{
		m_apPlayers[DummyID]->m_IsBalanceBattlePlayer1 = false;
		m_apPlayers[DummyID]->m_IsBalanceBattleDummy = true;
	}
	else if (dummymode == -3) //blockwavebot
	{
		m_apPlayers[DummyID]->m_IsBlockWaving = true;
	}
	else if (dummymode == -4) //laser fng
	{
		JoinInstagib(5, true, DummyID);
	}
	else if (dummymode == -5) //grenade fng
	{
		JoinInstagib(4, true, DummyID);
	}
	else if (dummymode == -6) //ChillBlock5 v3 deathmatch
	{
		m_apPlayers[DummyID]->m_IsBlockDeathmatch = true;
	}

	OnClientEnter(DummyID, silent);

	return DummyID;
}


void CGameContext::StopBalanceBattle()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i])
		{
			if (m_apPlayers[i]->m_BalanceBattle_id != -1)
			{
				m_apPlayers[i]->m_BalanceBattle_id = -1;
			}
			if (m_apPlayers[i]->m_IsBalanceBattleDummy)
			{
				Server()->BotLeave(i, true);
			}
		}
	}
	m_BalanceID1 = -1;
	m_BalanceID2 = -1;
	m_BalanceBattleState = 0; //set offline
}

void CGameContext::StartBalanceBattle(int ID1, int ID2)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	if (m_apPlayers[ID1] && !m_apPlayers[ID2])
	{
		SendChatTarget(ID1, "[balance] can't start a battle because your mate left.");
	}
	else if (!m_apPlayers[ID1] && m_apPlayers[ID2])
	{
		SendChatTarget(ID2, "[balance] can't start a battle because your mate left.");
	}
	else if (m_BalanceBattleState)
	{
		SendChatTarget(ID1, "[balance] can't start a battle because arena is full.");
		SendChatTarget(ID2, "[balance] can't start a battle because arena is full.");
	}
	else if (m_apPlayers[ID1] && m_apPlayers[ID2])
	{
		//moved to tick func
		//m_apPlayers[ID1]->m_IsBalanceBatteling = true;
		//m_apPlayers[ID2]->m_IsBalanceBatteling = true; 
		//m_apPlayers[ID1]->m_IsBalanceBattlePlayer1 = true;
		//m_apPlayers[ID2]->m_IsBalanceBattlePlayer1 = false;
		//SendChatTarget(ID1, "[balance] BATTLE STARTED!");
		//SendChatTarget(ID2, "[balance] BATTLE STARTED!");
		//m_apPlayers[ID1]->GetCharacter()->Die(ID1, WEAPON_SELF);
		//m_apPlayers[ID2]->GetCharacter()->Die(ID2, WEAPON_SELF);

		m_BalanceDummyID1 = CreateNewDummy(-1, true);
		m_BalanceDummyID2 = CreateNewDummy(-2, true);
		m_BalanceID1 = ID1;
		m_BalanceID2 = ID2;
		m_BalanceBattleCountdown = Server()->TickSpeed() * 10;
		m_BalanceBattleState = 1; //set state to preparing
	}
}

void CGameContext::BalanceBattleTick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[128];

	if (m_BalanceBattleState == 1) //preparing
	{
		m_BalanceBattleCountdown--;
		if (m_BalanceBattleCountdown % Server()->TickSpeed() == 0)
		{
			str_format(aBuf, sizeof(aBuf), "[balance] battle starts in %d seconds", m_BalanceBattleCountdown / Server()->TickSpeed());
			SendBroadcast(aBuf, m_BalanceID1);
			SendBroadcast(aBuf, m_BalanceID2);
		}
		if (!m_BalanceBattleCountdown)
		{
			//move the dummys
			if (m_apPlayers[m_BalanceDummyID1] && m_apPlayers[m_BalanceDummyID1]->GetCharacter())
			{
				m_apPlayers[m_BalanceDummyID1]->GetCharacter()->MoveTee(-4, -2);
			}
			if (m_apPlayers[m_BalanceDummyID2] && m_apPlayers[m_BalanceDummyID2]->GetCharacter())
			{
				m_apPlayers[m_BalanceDummyID2]->GetCharacter()->MoveTee(-4, -2);
			}

			if (m_apPlayers[m_BalanceID1] && m_apPlayers[m_BalanceID2]) //both on server
			{
				m_apPlayers[m_BalanceID1]->m_IsBalanceBatteling = true;
				m_apPlayers[m_BalanceID2]->m_IsBalanceBatteling = true;
				m_apPlayers[m_BalanceID1]->m_IsBalanceBattlePlayer1 = true;
				m_apPlayers[m_BalanceID2]->m_IsBalanceBattlePlayer1 = false;
				SendChatTarget(m_BalanceID1, "[balance] BATTLE STARTED!");
				SendChatTarget(m_BalanceID2, "[balance] BATTLE STARTED!");
				m_apPlayers[m_BalanceID1]->GetCharacter()->Die(m_BalanceID1, WEAPON_SELF);
				m_apPlayers[m_BalanceID2]->GetCharacter()->Die(m_BalanceID2, WEAPON_SELF);
				SendBroadcast("[balance] BATTLE STARTED", m_BalanceID1);
				SendBroadcast("[balance] BATTLE STARTED", m_BalanceID2);
				m_BalanceBattleState = 2; //set ingame
			}
			else if (m_apPlayers[m_BalanceID1])
			{
				SendBroadcast("[balance] BATTLE STOPPED (because mate left)", m_BalanceID1);
				StopBalanceBattle();
			}
			else if (m_apPlayers[m_BalanceID2])
			{
				SendBroadcast("[balance] BATTLE STOPPED (because mate left)", m_BalanceID2);
				StopBalanceBattle();
			}
			else
			{
				StopBalanceBattle();
			}
		}
	}
	//else if (m_BalanceBattleState == 2) //ingame //moved to die(); because it is less ressource to avoid it in tick functions
	//{
	//	if (m_apPlayers[m_BalanceID1] && m_apPlayers[m_BalanceID2])
	//	{
	//		if (!m_apPlayers[m_BalanceID1]->GetCharacter())
	//		{
	//			SendChatTarget(m_BalanceID1, "[balance] you lost!");
	//			SendChatTarget(m_BalanceID2, "[balance] you won!");
	//		}
	//	}
	//	else if (!m_apPlayers[m_BalanceID1] && !m_apPlayers[m_BalanceID2]) //all lef --> close game
	//	{
	//		m_BalanceBattleState = 0;
	//	}
	//}
}

void CGameContext::EndBombGame(int WinnerID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (GetPlayerChar(i))
		{
			if (GetPlayerChar(i)->m_IsBombing)
			{
				GetPlayerChar(i)->m_IsBombing = false;
			}
			if (GetPlayerChar(i)->m_IsBomb)
			{
				GetPlayerChar(i)->m_IsBomb = false;
			}
			if (GetPlayerChar(i)->m_IsBombReady)
			{
				GetPlayerChar(i)->m_IsBombReady = false;
			}
		}
	}
	m_BombGameState = 0;
	m_BombTick = g_Config.m_SvBombTicks;

	if (WinnerID == -1)
	{
		return;
	}

	//winner private
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "+%d bomb (won)", m_BombMoney * m_BombStartPlayers);
	m_apPlayers[WinnerID]->MoneyTransaction(m_BombMoney * m_BombStartPlayers, aBuf);
	str_format(aBuf, sizeof(aBuf), "[BOMB] You won the bomb game. +%d money.", m_BombMoney * m_BombStartPlayers);
	SendChatTarget(WinnerID, aBuf);
	m_apPlayers[WinnerID]->m_BombGamesWon++;
	m_apPlayers[WinnerID]->m_BombGamesPlayed++;
	if (!str_comp_nocase(m_BombMap, "NoArena"))
	{
		//GetPlayerChar(i)->ChillTelePortTile(GetPlayerChar(i)->m_BombPosX, GetPlayerChar(i)->m_BombPosY); //dont tele back in no arena
	}
	else
	{
		GetPlayerChar(WinnerID)->ChillTelePortTile(GetPlayerChar(WinnerID)->m_BombPosX, GetPlayerChar(WinnerID)->m_BombPosY); //tele on pos where game started
	}

	//winner public
	str_format(aBuf, sizeof(aBuf), "[BOMB] '%s' won and got %d money!", Server()->ClientName(WinnerID), m_BombMoney * m_BombStartPlayers);
	SendChat(-1, CGameContext::CHAT_ALL, aBuf);
}

void CGameContext::CheckStartBomb()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[128];
	bool AllReady = true;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing && !GetPlayerChar(i)->m_IsBombReady)
		{
			AllReady = false;
			//break; //back in the times this was an performance improvement but nowerdays we need all id's of the unready players to kick em

			//Kick unready players
			m_apPlayers[i]->m_BombTicksUnready++;
			if (m_apPlayers[i]->m_BombTicksUnready + 500 == g_Config.m_SvBombUnreadyKickDelay)
			{
				SendChatTarget(i, "[BOMB] WARNING! Type '/bomb start' or you will be kicked out of the bomb game.");
			}
			if (m_apPlayers[i]->m_BombTicksUnready > g_Config.m_SvBombUnreadyKickDelay)
			{
				SendBroadcast("", i); //send empty broadcast to signalize lobby leave
				SendChatTarget(i, "[BOMB] you got kicked out of lobby. (Reason: too late '/bomb start')");

				GetPlayerChar(i)->m_IsBombing = false;
				GetPlayerChar(i)->m_IsBomb = false;
				GetPlayerChar(i)->m_IsBombReady = false;
			}
		}
	}
	//if (CountReadyBombPlayers() == CountBombPlayers()) //eats more ressources than the other way
	if (AllReady)
	{
		if (m_BombStartCountDown > 1)
		{
			if (Server()->Tick() % 40 == 0)
			{
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (m_apPlayers[i] && GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing)
					{
						str_format(aBuf, sizeof(aBuf), "[BOMB] game starts in %d ...", m_BombStartCountDown);
						SendBroadcast(aBuf, i);
					}
				}
				m_BombStartCountDown--;
			}
		}
		else
		{
			m_BombStartPlayers = CountBombPlayers();
			m_BombGameState = 3;
			m_BombStartCountDown = g_Config.m_SvBombStartDelay;
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (m_apPlayers[i] && GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing)
				{
					if (!str_comp_nocase(m_BombMap, "Default"))
					{
						//GetPlayerChar(i)->m_Pos.x = g_Config.m_SvBombSpawnX + m_apPlayers[i]->GetCID() * 2; //spread the spawns round the cfg var depending on cid max distance is 63 * 2 = 126 = almost 4 tiles
						//GetPlayerChar(i)->m_Pos.x = g_Config.m_SvBombSpawnX;
						//GetPlayerChar(i)->m_Pos.y = g_Config.m_SvBombSpawnY;
						GetPlayerChar(i)->ChillTelePort((g_Config.m_SvBombSpawnX * 32) + m_apPlayers[i]->GetCID() * 2, g_Config.m_SvBombSpawnY * 32);
						//GetPlayerChar(i)->m_Pos = vec2(g_Config.m_SvBombSpawnX + m_apPlayers[i]->GetCID() * 2, g_Config.m_SvBombSpawnY); //doesnt tele but would freeze the tees (which could be nice but idk ... its scary) 
					}
					str_format(aBuf, sizeof(aBuf), "Bomb game has started! +%d money for the winner!", m_BombMoney * m_BombStartPlayers);
					SendBroadcast(aBuf, i);
					GetPlayerChar(i)->m_BombPosX = GetPlayerChar(i)->m_Pos.x / 32;
					GetPlayerChar(i)->m_BombPosY = GetPlayerChar(i)->m_Pos.y / 32;
				}
			}
		}
	}
}

void CGameContext::BombTick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[512];

	//bomb tickin'
	m_BombTick--;
	if (m_BombTick == 0) //time over --> kill the bomb (bomb explode)
	{
		m_BombTick = g_Config.m_SvBombTicks;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (GetPlayerChar(i))
			{
				if (GetPlayerChar(i)->m_IsBomb)
				{
					m_apPlayers[i]->m_BombGamesPlayed++;
					CreateExplosion(GetPlayerChar(i)->m_Pos, i, WEAPON_GRENADE, false, 0, GetPlayerChar(i)->Teams()->TeamMask(0)); //bomb explode! (think this explosion is always team 0 but yolo)
					str_format(aBuf, sizeof(aBuf), "'%s' exploded as bomb", Server()->ClientName(i));
					Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "bomb", aBuf);
					GetPlayerChar(i)->Die(i, WEAPON_GAME);
					break;
				}
			}
		}
	}

	//check start game
	if (m_BombGameState < 3) //not ingame
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing)
			{
				if (Server()->Tick() % 40 == 0)
				{
					if (GetPlayerChar(i)->m_IsBombReady)
					{
						str_format(aBuf, sizeof(aBuf), "--== Bomb Lobby ==--\n[%d/%d] players ready\nMap: %s   Money: %d", CountReadyBombPlayers(), CountBombPlayers(), m_BombMap, m_BombMoney);
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "--== Bomb Lobby ==--\n[%d/%d] players ready\nMap: %s   Money: %d\n\n\nType '/bomb start' to start.", CountReadyBombPlayers(), CountBombPlayers(), m_BombMap, m_BombMoney);
					}
					SendBroadcast(aBuf, i);
				}
			}
		}
		if (CountBombPlayers() > 1) //2+ tees required to start a game
		{
			CheckStartBomb();
		}
		else
		{
			m_BombGameState = 1; //unlock bomb lobbys with only 1 tee
		}
	}

	//check end game (no players)
	if (!CountBombPlayers())
	{
		EndBombGame(-1);
	}

	//check end game (only 1 player -> winner)
	if (CountBombPlayers() == 1 && m_BombGameState == 3)
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (GetPlayerChar(i))
			{
				if (GetPlayerChar(i)->m_IsBombing)
				{
					EndBombGame(i);
					break;
				}
			}
		}
	}

	//check for missing bomb
	if (m_BombGameState == 3)
	{
		bool BombFound = false;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (GetPlayerChar(i))
			{
				if (GetPlayerChar(i)->m_IsBomb)
				{
					BombFound = true;
					break;
				}
			}
		}
		if (!BombFound) //nobody bomb? -> pick new1
		{
			m_BombTick = g_Config.m_SvBombTicks;
			m_BombFinalColor = 180;

			//str_format(aBuf, sizeof(aBuf), "Bombfound: %d", FindNextBomb());
			//Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "bomb", aBuf);

			if (FindNextBomb() != -1)
			{
				GetPlayerChar(FindNextBomb())->m_IsBomb = true;
				SendChatTarget(FindNextBomb(), "The server has picked you as bomb.");
			}
			else
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "Failed to pick new bomb. Bombfound: %d", FindNextBomb());
				Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "bomb", aBuf);
			}
		}
	}



}

int CGameContext::GetNextClientID()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int ClientID = -1;
	for (int i = 0; i < g_Config.m_SvMaxClients; i++)
	{
		if (m_apPlayers[i])
			continue;

		ClientID = i;
		break;
	}

	return ClientID;
}

void CGameContext::OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	void *pRawMsg = m_NetObjHandler.SecureUnpackMsg(MsgID, pUnpacker);
	CPlayer *pPlayer = m_apPlayers[ClientID];

	if(!pRawMsg)
	{
		//char aBuf[256];
		//str_format(aBuf, sizeof(aBuf), "dropped weird message '%s' (%d), failed on '%s'", m_NetObjHandler.GetMsgName(MsgID), MsgID, m_NetObjHandler.FailedMsgOn());
		//Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "server", aBuf);
		return;
	}

	if(Server()->ClientIngame(ClientID))
	{
		if(MsgID == NETMSGTYPE_CL_SAY)
		{
			CNetMsg_Cl_Say *pMsg = (CNetMsg_Cl_Say *)pRawMsg;
			int Team = pMsg->m_Team;

			// trim right and set maximum length to 256 utf8-characters
			int Length = 0;
			const char *p = pMsg->m_pMessage;
			const char *pEnd = 0;
			while(*p)
			{
				const char *pStrOld = p;
				int Code = str_utf8_decode(&p);

				// check if unicode is not empty
				if(str_utf8_isspace(Code))
				{
					pEnd = 0;
				}
				else if(pEnd == 0)
					pEnd = pStrOld;

				if(++Length >= 256)
				{
					*(const_cast<char *>(p)) = 0;
					break;
				}
			}
			if(pEnd != 0)
				*(const_cast<char *>(pEnd)) = 0;

			// drop empty and autocreated spam messages (more than 32 characters per second)
			if(Length == 0 || (pMsg->m_pMessage[0]!='/' && (g_Config.m_SvSpamprotection && pPlayer->m_LastChat && pPlayer->m_LastChat+Server()->TickSpeed()*((31+Length)/32) > Server()->Tick())))
				return;

			//pPlayer->m_LastChat = Server()->Tick();

			int GameTeam = ((CGameControllerDDRace*)m_pController)->m_Teams.m_Core.Team(pPlayer->GetCID());
			if(Team)
				Team = ((pPlayer->GetTeam() == -1) ? CHAT_SPEC : GameTeam);
			else
				Team = CHAT_ALL;

			pPlayer->m_PlayerHumanLevelState++;

			////if (pMsg->m_pMessage[0] == apNames)
			////##########################
			////WORKING BUT UNUSED       #
			////[comment_start]          #
			////##########################
			//const char *pNames[] = { //Array für die Namen
			//	"flappy.*",
			//	"Chillingo.*",
			//	"Fluffy.*",
			//	"MLG_PRO.*",
			//	"Enzym.*",
			//	"ZillyDreck.*",
			//	"ciliDR[HUN].*",
			//	"fuzzle.*",
			//	"Piko.*",
			//	"chilliger.*",
			//	"ChilligerDrago",
			//	"GubbaFubba",
			//	"fuZZle.*",
			//	"<bot>",
			//	"<noob>",
			//	"<police>",
			//	"<train>",
			//	"<boat>",
			//	"<blocker>",
			//	"<racer>",
			//	"<hyper>",
			//	"sheep",
			//	"jeep",
			//	"chilluminatee.*",
			//	"auftragschiller",
			//	"abcJuhee",
			//	"BANANA.*",
			//	"POTATO.*",
			//	"<cucumber>",
			//	"<rape>",
			//	"<_BoT__>",
			//	"NotMyName",
			//	"NotChiller",
			//	"NotChiIIer",
			//	"NotChlIer",
			//	"fuckmesoon.*",
			//	"DataNub",
			//	"5.4.45.109.239",
			//	"<hacker>",
			//	"<cheater>",
			//	"<glitcher>",
			//	"__ERROR",
			//	"404_kein_tier",
			//	"ZitrusFRUCHT",
			//	"BAUMKIND",
			//	"KELLERKIND",
			//	"KINDERKIND",
			//	"einZug",
			//	"<bob>",
			//	"BezzyHill",
			//	"BeckySkill",
			//	"Skilli.*",
			//	"UltraVa.",
			//	"DONATE!",
			//	"SUBSCRIBE!",
			//	"SHARE!",
			//	"#like",
			//	"<#name_>",
			//	"KRISTIAN-.",
			//	".,-,08/524",
			//	"3113pimml34",
			//	"NotABot",
			//	"Human",
			//	"xxlddnnet64"
			//};

			////int c = 1; //Chillingo.*


			////for (int c = 0; c < 65; c++)
			//for (int c = 0; c < MAX_CLIENTS; c++)
			//{
			//	if (m_apPlayers[c] && GetPlayerChar(c)) //check if this player is existing and is alive
			//	{
			//		if (m_apPlayers[c]->m_DummyMode == 32) //check dummy mode
			//		{
			//			if (!strncmp(pMsg->m_pMessage, pNames[c], strlen(pNames[c]))) //search dummy name in message
			//			{
			//				if (!str_comp(Server()->ClientName(c), pNames[c])) //check if this is the rigth dummy name
			//				{
			//					if (pMsg->m_pMessage[strlen(pNames[c]) + 2] == '!') // COMMANDS
			//					{
			//						if (m_apPlayers[ClientID]->m_dummy_member || !str_comp(Server()->ClientName(ClientID), "ChillerDragon"))
			//						{
			//							if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "fire"))
			//							{
			//								GetPlayerChar(c)->m_Dummy_32fire = true;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "stop fire"))
			//							{
			//								GetPlayerChar(c)->m_Dummy_32fire = false;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "kill"))
			//							{
			//								GetPlayerChar(c)->m_Dummy_32kill = true;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "left"))
			//							{
			//								GetPlayerChar(c)->m_Dummy_32dir = -1;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "right"))
			//							{
			//								GetPlayerChar(c)->m_Dummy_32dir = 1;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "balance"))
			//							{
			//								GetPlayerChar(c)->m_Dummy_32balance = 1;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "balance left"))
			//							{
			//								GetPlayerChar(c)->m_Dummy_32balance = 2;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "balance right"))
			//							{
			//								GetPlayerChar(c)->m_Dummy_32balance = 3;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "dummy"))
			//							{
			//								m_apPlayers[c]->m_Dummy_32dummy = true;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "look right"))
			//							{
			//								m_apPlayers[c]->m_Dummy_32look = 0;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "look down"))
			//							{
			//								m_apPlayers[c]->m_Dummy_32look = 1;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "look left"))
			//							{
			//								m_apPlayers[c]->m_Dummy_32look = 2;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "look up"))
			//							{
			//								m_apPlayers[c]->m_Dummy_32look = 3;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "hammer"))
			//							{
			//								GetPlayerChar(c)->m_Dummy_32weapon = 0;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "gun"))
			//							{
			//								GetPlayerChar(c)->m_Dummy_32weapon = 1;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "shotgun"))
			//							{
			//								GetPlayerChar(c)->m_Dummy_32weapon = 2;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "grenade"))
			//							{
			//								GetPlayerChar(c)->m_Dummy_32weapon = 3;
			//							}
			//							else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "reset all"))
			//							{
			//								GetPlayerChar(c)->m_Dummy_32reset = true;
			//							}
			//					//		else if (!str_comp_nocase_num(pMsg->m_pMessage + 3, "script ", 9) == 0)
			//					//		{
			//					//			if (!str_comp_nocase_num(pMsg->m_pMessage + strlen(pNames[c]) + 10, "0 ", 11) == 0)
			//					//			{
			//					//	/*			if (!str_comp_nocase_num(pMsg->m_pMessage + strlen(pNames[c]) + 12, "step 0", 17) == 0)
			//					//				{
			//					//					SendChat(c, CGameContext::CHAT_ALL, "test failed.!!!!!!!!!!!!!!!!!!");
			//					//				}
			//					//				else
			//					//				{
			//					//					SendChat(c, CGameContext::CHAT_ALL, "error: wrong step. choose between 0, 1 and 2");
			//					//				}*/
			//					//				SendChat(c, CGameContext::CHAT_ALL, "test failed.!!!!!!!!!!!!!!!!!!");
			//					//			}
			//					//			else
			//					//			{
			//					//				SendChat(c, CGameContext::CHAT_ALL, "error: wrong script. choose between 0, 1 and 2");
			//					//			}
			//					//		}
			//					//		else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "!script ?"))
			//					//		{
			//					//			SendChat(c, CGameContext::CHAT_ALL, "struct: !script <0/1/2> step <0/1/2> cmd <cmd> t <time> del <stepstartdelay>");
			//					//		}
			//					//		else if (str_comp_nocase_num(pMsg->m_pMessage + 3, "tick_script fire 0 ", 21) == 0)
			//					//		{
			//					///*			char aBuf[256];
			//					//			char aUsername[MAX_NAME_LENGTH];
			//					//			str_copy(aUsername, pMsg->m_pMessage + 15, MAX_NAME_LENGTH + 7);

			//					//			dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, aUsername);*/


			//					//			if (pMsg->m_pMessage[strlen(pNames[c]) + 15] == '0' ||
			//					//				pMsg->m_pMessage[strlen(pNames[c]) + 15] == '1' ||
			//					//				pMsg->m_pMessage[strlen(pNames[c]) + 15] == '2' ||
			//					//				pMsg->m_pMessage[strlen(pNames[c]) + 15] == '3' ||
			//					//				pMsg->m_pMessage[strlen(pNames[c]) + 15] == '4' ||
			//					//				pMsg->m_pMessage[strlen(pNames[c]) + 15] == '5' ||
			//					//				pMsg->m_pMessage[strlen(pNames[c]) + 15] == '6' ||
			//					//				pMsg->m_pMessage[strlen(pNames[c]) + 15] == '7' ||
			//					//				pMsg->m_pMessage[strlen(pNames[c]) + 15] == '8' ||
			//					//				pMsg->m_pMessage[strlen(pNames[c]) + 15] == '9'
			//					//				)
			//					//			{
			//					//				SendChat(c, CGameContext::CHAT_ALL, "digit found.");
			//					//			}
			//					//			else
			//					//			{
			//					//				SendChat(c, CGameContext::CHAT_ALL, "error: no digit found for <start_tick>");
			//					//			}

			//					//			dbg_msg("test", "'%s' -> '%s'", pMsg->m_pMessage, pMsg->m_pMessage[strlen(pNames[c]) + 15]);
			//					//		}
			//					//		else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 3, "!tick_script ?"))
			//					//		{
			//					//			SendChat(c, CGameContext::CHAT_ALL, "struct: !tick_script <command> <command_id> <start_tick> <stop_tick>");
			//					//		}
			//							else
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "unknown command.");
			//							}
			//						}
			//						else //not trusted
			//						{
			//							char aBuf[128];
			//							str_format(aBuf, sizeof(aBuf), "%s: I don't trust you --> I don't do what you say.", Server()->ClientName(ClientID));
			//							SendChat(c, CGameContext::CHAT_ALL, aBuf);
			//						}
			//					}
			//					else //NO COMMANDS (PUBLIC CHAT)
			//					{
			//						if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "hello :)"))
			//						{
			//							SendChat(c, CGameContext::CHAT_ALL, "Hellu :)");


			//						}
			//						else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "fuck you"))
			//						{
			//							SendChat(c, CGameContext::CHAT_ALL, "ouch :c");
			//						}
			//						else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "trust me"))
			//						{
			//							if (!str_comp(Server()->ClientName(ClientID) ,"Drag*"))
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "my creator told me you are evil. I don't trust you.");
			//							}
			//							else
			//							{
			//								m_apPlayers[ClientID]->m_dummy_member = true;
			//							}
			//						}
			//						else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "fuck u"))
			//						{
			//							SendChat(c, CGameContext::CHAT_ALL, "dont say this plx.");
			//						}
			//						else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "secret"))
			//						{
			//							int r = rand() % 10;

			//							if (r == 0)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "top secret");
			//							}
			//							else if (r == 1)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "ye secret?");
			//							}
			//							else if (r == 2)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "o.O i love secrets wanna tell me one?");
			//							}
			//							else if (r == 3)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "what is secret? o.O");
			//							}
			//							else if (r == 4)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "kewl");
			//							}
			//							else if (r == 5)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "tree");
			//							}
			//							else if (r == 6)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "sdawdauhip");
			//							}
			//							else if (r == 7)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "wow");
			//							}
			//							else if (r == 8)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "and what is with this single word");
			//							}
			//							else
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "wanna tell me sumsin?");
			//							}

			//						}
			//						else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "hi"))
			//						{
			//							int r = rand() % 10;

			//							if (r == 0)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "salve");
			//							}
			//							else if (r == 1)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "priviet");
			//							}
			//							else if (r == 2)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "HELLO!");
			//							}
			//							else if (r == 3)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "hey!");
			//							}
			//							else if (r == 4)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "hay");
			//							}
			//							else if (r == 5)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "hi");
			//							}
			//							else if (r == 6)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "sup");
			//							}
			//							else if (r == 7)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "yo");
			//							}
			//							else if (r == 8)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "ay");
			//							}
			//							else
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "hello.");
			//							}
			//
			//						}
			//						else if (!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "y") ||
			//							!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "ye") ||
			//							!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "yas") ||
			//							!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "yes") ||
			//							!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "yap") ||
			//							!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "ya") ||
			//							!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "ja") ||
			//							!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "js") ||
			//							!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "yep") ||
			//							!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "ok") ||
			//							!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "allright") ||
			//							!str_comp_nocase(pMsg->m_pMessage + strlen(pNames[c]) + 2, "allight"))
			//						{
			//							int r = rand() % 10;

			//							if (r == 0)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "yes what?");
			//							}
			//							else if (r == 1)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "man you cant just say yes i dont know what we are talking about");
			//							}
			//							else if (r == 2)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "I dont have a brain so i cant remember what we were talking baut.");
			//							}
			//							else if (r == 3)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "y what?");
			//							}
			//							else if (r == 4)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "NO!");
			//							}
			//							else if (r == 5)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "Funfact: i have no idea what you are talking about but i can offer you a secret");
			//							}
			//							else if (r == 6)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "cool.");
			//							}
			//							else if (r == 7)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "ok lets get started");
			//							}
			//							else if (r == 8)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "ye right?");
			//							}
			//							else
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "ok.");
			//							}

			//						}
			//						else
			//						{
			//							int r = rand() % 20;
			//
			//							if (r == 0)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "Imagine a train coudl fly.");
			//							}
			//							else if (r == 1)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "meaning of lyfe.");
			//							}
			//							else if (r == 2)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "lol.");
			//							}
			//							else if (r == 3)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "what?");
			//							}
			//							else if (r == 4)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "I dont know the words you use please stay simple mate.");
			//							}
			//							else if (r == 5)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "I prefer short sentences :)");
			//							}
			//							else if (r == 6)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "There is a boat at the other side of the sea. We have to get it as fast as we can oky?");
			//							}
			//							else if (r == 7)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "Oh that is suprising isnt it?");
			//							}
			//							else if (r == 8)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "I feel so dump");
			//							}
			//							else if (r == 9)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "Wanna know a secret? feel free to ask me for a secret :)");
			//							}
			//							else if (r == 10)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "I cant help you on that sry.");
			//							}
			//							else if (r == 11)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "Do you eat fruits?");
			//							}
			//							else if (r == 12)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "haha. But what is 2x7^2?");
			//							}
			//							else if (r == 13)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "I have no brain.");
			//							}
			//							else if (r == 14)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "Fun fact im not a human xD");
			//							}
			//							else if (r == 15)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "Do you trust me ? Am not sure if i can trust you mate.");
			//							}
			//							else if (r == 16)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "BRASILIA!");
			//							}
			//							else if (r == 17)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "I am faster than you o.O");
			//							}
			//							else if (r == 18)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "wanna hear my secret?");
			//							}
			//							else if (r == 19)
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "i coudl tell you my secret but you have to ask for it.");
			//							}
			//							else
			//							{
			//								SendChat(c, CGameContext::CHAT_ALL, "meaning of life.");
			//							}
			//						}
			//					}
			//				}
			//			}
			//		}
			//	}
			//}
			////##########################
			////WORKING BUT UNUSED       #
			////[comment_end]            #
			////##########################

			//some old dummy chat stuff idk what dis is
			//for (int c = 0; c < 65; c++)
			//{
			//	if (!strncmp(pMsg->m_pMessage, pNames[c], strlen(pNames[c])))
			//	{
			//		if (!str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "hello") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 1, "hello"))
			//		{
			//			SendChat(ClientID, Team, pMsg->m_pMessage, ClientID);
			//			SendChat(c, CGameContext::CHAT_ALL, "Hellu :)");
			//			return;

			//			//TODO:
			//			//wenn man einen dummy anschreibt der nicht auf dem server ist und jemand anderes seine id hat antwortet er
			//			//mit der var hier unten könnte man prüfen ob der angeschriebenen auch den dummy namen hat
			//			//is nicht zu 100% perfekt aber besser
			//			//SendChat(c, CGameContext::CHAT_ALL, Server()->ClientName(c));
			//		}
			//		else if (!str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "fuck you") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 1, "fuck you"))
			//		{
			//			SendChat(ClientID, Team, pMsg->m_pMessage, ClientID);

			//			char aBuf[128];
			//			str_format(aBuf, sizeof(aBuf), "%s: do you want war?", Server()->ClientName(ClientID));
			//			SendChat(c, CGameContext::CHAT_ALL, aBuf);
			//			return;

			//		}
			//		else if (!str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "life") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 1, "life"))
			//		{
			//			SendChat(ClientID, Team, pMsg->m_pMessage, ClientID);

			//			char aBuf[128];
			//			str_format(aBuf, sizeof(aBuf), "%s: haha", Server()->ClientName(ClientID));
			//			SendChat(c, CGameContext::CHAT_ALL, aBuf);
			//			return;

			//		}
			//		else if (!str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "team?") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 1, "team?") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "peace?") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 1, "peace?") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "friends?") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 1, "friends?"))
			//		{
			//			SendChat(ClientID, Team, pMsg->m_pMessage, ClientID);



			//			//if (str_comp(GetPlayerChar(ClientID)->m_Dummy_friend, "nobody")) // das hier solltest du eigentlich auch nicht so machen, da muss str_comp hin
			//			if(GetPlayerChar(ClientID)->m_Dummy_FriendID == -1)
			//			{
			//				//GetPlayerChar(ClientID)->m_Dummy_friend = Server()->ClientName(ClientID);
			//				//str_format(GetPlayerChar(ClientID)->m_Dummy_friend, sizeof(GetPlayerChar(ClientID)->m_Dummy_friend), "%s", Server()->ClientName(ClientID));

			//				GetPlayerChar(ClientID)->m_Dummy_FriendID = ClientID;

			//				char aBuf2[256];
			//				str_format(aBuf2, sizeof(aBuf2), "setting FriendID: %d to ClientID: %d", GetPlayerChar(ClientID)->m_Dummy_FriendID, ClientID);
			//				Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf2);

			//				char aBuf[128];
			//				str_format(aBuf, sizeof(aBuf), "%s: oky :)", Server()->ClientName(ClientID));
			//				SendChat(c, CGameContext::CHAT_ALL, aBuf);
			//				return;
			//			}
			//			else
			//			{
			//				SendChat(c, CGameContext::CHAT_ALL, "No, sorry i already have a friend.");
			//				return;
			//			}




			//		}
			//		else if (!str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "who is your friend?") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 1, "who is your friend?") || !str_comp(pMsg->m_pMessage + strlen(pNames[c]) + 2, "who is ur friend?"))
			//		{
			//			SendChat(ClientID, Team, pMsg->m_pMessage, ClientID);

			//			if (GetPlayerChar(ClientID)->m_Dummy_FriendID == -1)
			//			{
			//				SendChat(c, CGameContext::CHAT_ALL, "i have no friends.");
			//				return;
			//			}
			//			else
			//			{
			//				char aBuf[128];
			//				//str_format(aBuf, sizeof(aBuf), "%s: is my friend", GetPlayerChar(ClientID)->m_Dummy_FriendID);
			//				str_format(aBuf, sizeof(aBuf), "%s: is my friend", Server()->ClientName(GetPlayerChar(ClientID)->m_Dummy_FriendID));
			//				SendChat(c, CGameContext::CHAT_ALL, aBuf);
			//				return;
			//			}


			//		}
			//	}
			//}

			//############
			//GLOBAL CHAT
			//############

			if (g_Config.m_SvAllowGlobalChat)
			{
				if (pMsg->m_pMessage[0] == '@' && pMsg->m_pMessage[1] == 'a' && pMsg->m_pMessage[2] == 'l' && pMsg->m_pMessage[3] == 'l')
				{
					char aBuf[1024];
					char aBuf2[1024];
					std::string msg_format = pMsg->m_pMessage;

					if (msg_format.length() > 6) //ignore too short messages
					{
						msg_format.erase(0, 4);

						//dont send messages twice
						str_format(aBuf, sizeof(aBuf), "%s", m_aLastPrintedGlobalChatMessage);
						str_format(aBuf2, sizeof(aBuf2), "0[CHAT@%s] %s: %s", g_Config.m_SvMap, Server()->ClientName(ClientID), msg_format.c_str());
						aBuf[0] = ' '; //ignore confirms on double check
						aBuf2[0] = ' '; //ignore confirms on double check
						if (!str_comp(aBuf, aBuf2))
						{
							SendChatTarget(ClientID, "[CHAT] global chat ignores doublicated messages");
							return;
						}
						else
						{
							dbg_msg("global_chat", "'%s' != '%s'", aBuf, aBuf2);

							//check if all servers confirmed the previous message before adding a new one
							std::fstream ChatReadFile(g_Config.m_SvGlobalChatFile);

							if (!std::ifstream(g_Config.m_SvGlobalChatFile))
							{
								SendChat(-1, CGameContext::CHAT_ALL, "[CHAT] global chat stopped working.");
								g_Config.m_SvAllowGlobalChat = 0;
								ChatReadFile.close();
								return;
							}

							std::string data;
							getline(ChatReadFile, data);
							int confirms = 0;
							if (data[0] == '1')
								confirms = 1;
							else if (data[0] == '2')
								confirms = 2;
							else if (data[0] == '3')
								confirms = 3;
							else if (data[0] == '4')
								confirms = 4;
							else if (data[0] == '5')
								confirms = 5;
							else if (data[0] == '6')
								confirms = 6;
							else if (data[0] == '7')
								confirms = 7;
							else if (data[0] == '8')
								confirms = 8;
							else if (data[0] == '9')
								confirms = 9;

							if (confirms < g_Config.m_SvGlobalChatServers)
							{
								SendChatTarget(ClientID, "[CHAT] Global chat is currently printing messages. Try agian later.");
								ChatReadFile.close();
								return; //idk if this is too good ._. better check if it skips any spam protections
							}








							//std::ofstream ChatFile(g_Config.m_SvGlobalChatFile, std::ios_base::app);
							std::ofstream ChatFile(g_Config.m_SvGlobalChatFile);
							if (!ChatFile)
							{
								SendChat(-1, CGameContext::CHAT_ALL, "[CHAT] global chat failed.... deactivating it.");
								dbg_msg("CHAT", "ERROR1 writing file '%s'", g_Config.m_SvGlobalChatFile);
								g_Config.m_SvAllowGlobalChat = 0;
								ChatFile.close();
								return;
							}

							if (ChatFile.is_open())
							{
								//SendChat(-1, CGameContext::CHAT_ALL, "global chat");

								str_format(aBuf, sizeof(aBuf), "0[CHAT@%s] %s: %s", g_Config.m_SvMap, Server()->ClientName(ClientID), msg_format.c_str());
								dbg_msg("global_chat", "msg [ %s ]", aBuf);
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
					}
				}
			}

			//############
			//CHAT COMMANDS
			//############
			if(pMsg->m_pMessage[0]=='/')
			{
				// todo: adde mal deine ganzen cmds hier in das system von ddnet ddracechat.cpp
				// geb mal ein cmd /join spec   && /join fight (player)
				if (!str_comp(pMsg->m_pMessage + 1, "leave"))
				{
					if (pPlayer->m_IsBlockDeathmatch)
					{
						SendChatTarget(ClientID, "[BLOCK] you left the deathmatch arena!");
						SendChatTarget(ClientID, "[BLOCK] now kys :p");
						pPlayer->m_IsBlockDeathmatch = false;
					}
					else
					{
						SendChatTarget(ClientID, "leave what? xd");
						SendChatTarget(ClientID, "Do you want to leave the minigame you are playing?");
						SendChatTarget(ClientID, "then type '/<minigame> leave'");
						SendChatTarget(ClientID, "check '/minigames status' for the minigame command you need");
					}
				}
				/*
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "join ", 5) == 0)
				{
					CCharacter *pOwner = GetPlayerChar(ClientID);
					if (!pOwner)
						return;

					if (Collision()->GetCustTile(pOwner->m_Pos.x, pOwner->m_Pos.y) != TILE_H_JOIN)
					{
						SendChatTarget(ClientID, "You need to be in the hammer lobby.");
						return;
					}

					char aArg[256];
					str_copy(aArg, pMsg->m_pMessage + 6, sizeof(aArg));

					if (!str_comp(aArg, "spec"))
					{
						vec2 SpecSpawn = Collision()->GetRandomTile(TILE_H_SPAWN_SPEC);

						if (SpecSpawn != vec2(-1, -1))
						{
							pOwner->SetPosition(SpecSpawn);
							pOwner->m_IsSpecHF = true;
						}
					}
				}
				*/
				else if (!str_comp(pMsg->m_pMessage + 1, "testcommand3000"))
				{
					char aBuf[256];

					if (g_Config.m_SvTestingCommands)
					{
						/*
						static int t = 0;
						t = !t ? 999999 : 0;
						vec2 post = Collision()->GetSurvivalSpawn(t, false);
						str_format(aBuf, sizeof(aBuf), "got t=%d pos %f %f",t, post.x / 32, post.y / 32);
						SendChatTarget(ClientID, aBuf);
						pPlayer->GetCharacter()->SetPosition(post);
						*/
						// pPlayer->m_IsBlockDeathmatch ^= true;
						// str_format(aBuf, sizeof(aBuf), "finish tile pos %f %f", m_FinishTilePos.x, m_FinishTilePos.y);
						str_format(aBuf, sizeof(aBuf), "human level: %d captcha score: %d", pPlayer->m_PlayerHumanLevel, pPlayer->m_pCaptcha->GetScore());
						SendChatTarget(ClientID, aBuf);
						//CreateNewDummy(35, true, 1);
                        //LoadSinglePlayer();
                        //str_format(aBuf, sizeof(aBuf), "unlocked level: %d current: %d", m_MissionUnlockedLevel, m_MissionCurrentLevel);
                        //SendChatTarget(ClientID, aBuf);
						/*
						vec2 vec_finish = GetFinishTile();
						vec2 your_pos(0, 0);
						float newest_distance_finish = 4.20;
						if (pPlayer->GetCharacter())
						{
							your_pos.x = pPlayer->GetCharacter()->GetPosition().x / 32;
							your_pos.y = pPlayer->GetCharacter()->GetPosition().y / 32;
							newest_distance_finish = distance(your_pos, m_FinishTilePos);
						}
						str_format(aBuf, sizeof(aBuf), "finish at (%.2f/%.2f) your position (%.2f/%.2f)  distance to finish: %.2f", vec_finish.x, vec_finish.y, your_pos.x, your_pos.y, newest_distance_finish);
						SendChatTarget(ClientID, aBuf);


						vec2 vector1(10, 10);
						vec2 vector2(200, 20);
						float vv_distance = distance(vector1, vector2);

						str_format(aBuf, sizeof(aBuf), "vector 1 (%.2f/%.2f) vector 2 (%.2f/%.2f)   distance=%.2f", vector1.x, vector1.y, vector2.x, vector2.y, vv_distance);
						*/
						//str_format(aBuf, sizeof(aBuf), "chidraqul3 gametstate: %d deathmatch %d mins %d seconds", pPlayer->m_C3_GameState, m_survival_dm_countdown / (Server()->TickSpeed() * 60), (m_survival_dm_countdown % (Server()->TickSpeed() * 60)) / Server()->TickSpeed());
						
						//ConnectFngBots(3, 0);
						//ConnectFngBots(3, 1);

						//str_format(aBuf, sizeof(aBuf), "bots: %d <3", CountConnectedBots());


						CCharacter *pChr = m_apPlayers[ClientID]->GetCharacter();
						if (pChr)
						{
							str_format(aBuf, sizeof(aBuf), "tile(%.2f/%.2f): %d", pChr->m_Pos.x / 32, pChr->m_Pos.y / 32, Collision()->GetCollisionAt(pChr->m_Pos.x, pChr->m_Pos.y));
							SendChatTarget(ClientID, aBuf);
						}
						else
						{
							SendChatTarget(ClientID, "error no character");
						}


						//pPlayer->m_PoliceRank = 5;
						//GetPlayerChar(ClientID)->FreezeAll(10);
						//pPlayer->m_IsJailed = true;
						//pPlayer->m_JailTime = Server()->TickSpeed() * 10; //4 min
						//QuestCompleted(pPlayer->GetCID());
						pPlayer->MoneyTransaction(+5000000, "+5000000 test cmd3000");
						pPlayer->m_xp += 100000000; //max level 100 (so the annoying level up message show up only once)
						//Server()->SetClientName(ClientID, "dad");
						//pPlayer->m_IsVanillaDmg = !pPlayer->m_IsVanillaDmg;
						//pPlayer->m_IsVanillaWeapons = !pPlayer->m_IsVanillaWeapons;

						//AddEscapeReason(ClientID, "testc");
						//pPlayer->m_EscapeTime = 400;

						//m_apPlayers[ClientID]->m_autospreadgun ^= true;
						//m_apPlayers[ClientID]->m_IsSupporter ^= true;

						//ChillUpdateFileAcc(,);

						//m_IsDebug = !m_IsDebug;
						//str_format(aBuf, sizeof(aBuf), "fnn 25 debug mode updated to %d", m_IsDebug);
						//SendChatTarget(ClientID, aBuf);

						time_t seconds;

						seconds = time(NULL);

						str_format(aBuf, sizeof(aBuf), "%d", seconds);

						//SendChatTarget(ClientID, aBuf);

						/*
						str_format(aBuf, sizeof(aBuf), "file_accounts/%s.acc", pPlayer->m_aAccountLoginName);
						if (ChillWriteToLine(aBuf, 1, "1"))
						{
							SendChatTarget(ClientID, "succesfully written to acc");
						}
						else
						{
							SendChatTarget(ClientID, "writing to acc failed");
						}
						*/

						//CBlackHole test;

						//##########
						//survival tests
						//##########
						//if (!m_apPlayers[ClientID]->GetCharacter())
						//{
						//	SendChatTarget(ClientID, "real testers are alive");
						//	return;
						//}

						//vec2 TestToTeleTile = Collision()->GetRandomTile(TILE_SURVIVAL_LOBBY);

						//if (TestToTeleTile != vec2(-1, -1))
						//{
						//	m_apPlayers[ClientID]->GetCharacter()->SetPosition(TestToTeleTile);
						//}
						//else //no TestToTeleTile
						//{
						//	SendChatTarget(ClientID, "gibts nich");
						//}
					}

					//char aIP_1[64];
					//for (int i = 0; i < MAX_CLIENTS; i++)
					//{
					//	if (m_apPlayers[i])
					//	{
					//		Server()->GetClientAddr(i, aIP_1, sizeof(aIP_1));
					//		str_format(aBuf, sizeof(aBuf), "[%s] '%s'", aIP_1, Server()->ClientName(i));
					//		SendChatTarget(ClientID, aBuf);
					//	}
					//}

					//Console()->ExecuteFile("testfile3000.txt");
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "testcommand3001"))
				{
					str_format(aBroadcastMSG, sizeof(aBroadcastMSG), " ", aBroadcastMSG);
					SendBroadcast(aBroadcastMSG, ClientID);
					//SendAllPolice("test");
					return;
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "push val"))
				{
					m_CucumberShareValue++;
					SendChatTarget(ClientID, "pushed val.");
					return;
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "pull val"))
				{
					m_CucumberShareValue--;
					SendChatTarget(ClientID, "pulled val.");
					return;
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "hax_me_admin_mummy"))
				{
					m_apPlayers[ClientID]->m_fake_admin = true;

					return;
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "fake_super"))
				{
					if (g_Config.m_SvFakeSuper == 0)
					{
						SendChatTarget(ClientID, "Admin has disabled this command.");
						return;
					}


					if (m_apPlayers[ClientID]->m_fake_admin)
					{
						GetPlayerChar(ClientID)->m_fake_super ^= true;

						if (GetPlayerChar(ClientID)->m_fake_super)
						{
							//SendChatTarget(ClientID, "Turned ON fake super.");
						}
						else
						{
							//SendChatTarget(ClientID, "Turned OFF fake super.");
						}
					}
					else
					{
						SendChatTarget(ClientID, "You don't have enough permission.");
					}

					return;
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "_"))
				{
					if (pPlayer->m_Authed == CServer::AUTHED_ADMIN)
						CreateBasicDummys();
				}
				//else if (!str_comp(pMsg->m_pMessage+1, "dummy"))
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "dummy ", 6) == 0) //hab den hier kopiert un dbissl abgeändert
				{
					//if (Server()->IsAuthed(ClientID))
					if (pPlayer->m_Authed == CServer::AUTHED_ADMIN)
					{
						char pValue[32];
						str_copy(pValue, pMsg->m_pMessage + 7, 32);
						dbg_msg("lol", "%s -> '%s'", pMsg->m_pMessage, pValue);
						int Value = str_toint(pValue);
						if (Value > 0)
						{
							for (int i = 0; i < Value; i++)
							{
								CreateNewDummy(0);
								SendChatTarget(ClientID, "Bot has been added.");
							}
						}
					}
					else
					{
						SendChatTarget(ClientID, "You don't have enough permission to use this command"); //passt erstmal so
					}
					return;
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "dcdummys"))
				{
					//if (Server()->IsAuthed(ClientID))
					if (pPlayer->m_Authed == CServer::AUTHED_ADMIN)
					{
						for (int i = 0; i < MAX_CLIENTS; i++)
						{
							if (m_apPlayers[i] && m_apPlayers[i]->m_IsDummy)
							{
								Server()->BotLeave(i);
								//delete m_apPlayers[i]; // keine ahnung wieso es crashen sollte ._. why kein kick? mach halt ._.
								//m_apPlayers[i] = 0x0;
							}
						}
						SendChatTarget(ClientID, "All bots have been removed."); //save? jo, muss aber normalerweise nicht sein kk
					}
					else
					{
						SendChatTarget(ClientID, "You don't have enough permission to use this command"); //passt erstmal so
					}
					return;
				}
				else if (!str_comp(pMsg->m_pMessage + 1, "taxi"))
				{
					SendChatTarget(ClientID, "You called a dummy! He is on his way to be your taxi!");
					GetPlayerChar(ClientID)->m_taxi = true;
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "w ", 2) == 0)
				{
					char pWhisperMsg[256];
					str_copy(pWhisperMsg, pMsg->m_pMessage + 3, 256);
					Whisper(pPlayer->GetCID(), pWhisperMsg);
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "whisper ", 8) == 0)
				{
					char pWhisperMsg[256];
					str_copy(pWhisperMsg, pMsg->m_pMessage + 9, 256);
					Whisper(pPlayer->GetCID(), pWhisperMsg);
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "c ", 2) == 0)
				{
					char pWhisperMsg[256];
					str_copy(pWhisperMsg, pMsg->m_pMessage + 3, 256);
					Converse(pPlayer->GetCID(), pWhisperMsg);
				}
				else if (str_comp_nocase_num(pMsg->m_pMessage + 1, "converse ", 9) == 0)
				{
					char pWhisperMsg[256];
					str_copy(pWhisperMsg, pMsg->m_pMessage + 10, 256);
					Converse(pPlayer->GetCID(), pWhisperMsg);
				}
				else
				{
					if(g_Config.m_SvSpamprotection && str_comp_nocase_num(pMsg->m_pMessage+1, "timeout ", 8) != 0
						&& pPlayer->m_LastCommands[0] && pPlayer->m_LastCommands[0]+Server()->TickSpeed() > Server()->Tick()
						&& pPlayer->m_LastCommands[1] && pPlayer->m_LastCommands[1]+Server()->TickSpeed() > Server()->Tick()
						&& pPlayer->m_LastCommands[2] && pPlayer->m_LastCommands[2]+Server()->TickSpeed() > Server()->Tick()
						&& pPlayer->m_LastCommands[3] && pPlayer->m_LastCommands[3]+Server()->TickSpeed() > Server()->Tick()
					)
						return;

					int64 Now = Server()->Tick();
					pPlayer->m_LastCommands[pPlayer->m_LastCommandPos] = Now;
					pPlayer->m_LastCommandPos = (pPlayer->m_LastCommandPos + 1) % 4;

					m_ChatResponseTargetID = ClientID;
					Server()->RestrictRconOutput(ClientID);
					Console()->SetFlagMask(CFGFLAG_CHAT);

					if (pPlayer->m_Authed)
						Console()->SetAccessLevel(pPlayer->m_Authed == CServer::AUTHED_ADMIN ? IConsole::ACCESS_LEVEL_ADMIN : IConsole::ACCESS_LEVEL_MOD);
					else
						Console()->SetAccessLevel(IConsole::ACCESS_LEVEL_USER);
					Console()->SetPrintOutputLevel(m_ChatPrintCBIndex, 0);

					Console()->ExecuteLine(pMsg->m_pMessage + 1, ClientID);
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "%d used %s", ClientID, pMsg->m_pMessage);
					Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "chat-command", aBuf);

					Console()->SetAccessLevel(IConsole::ACCESS_LEVEL_ADMIN);
					Console()->SetFlagMask(CFGFLAG_SERVER);
					m_ChatResponseTargetID = -1;
					Server()->RestrictRconOutput(-1);
				}
			}
			else
			{
				if (pPlayer->m_PlayerHumanLevel < g_Config.m_SvChatHumanLevel)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "your '/human_level' is too low %d/%d to use the chat.", m_apPlayers[ClientID]->m_PlayerHumanLevel, g_Config.m_SvChatHumanLevel);
					SendChatTarget(ClientID, aBuf);
				}
				else if (m_apPlayers[ClientID] && !m_apPlayers[ClientID]->m_Authed && AdminChatPing(pMsg->m_pMessage))
				{
					if (g_Config.m_SvMinAdminPing > 256)
						SendChatTarget(ClientID, "you are not allowed to ping admins in chat.");
					else
						SendChatTarget(ClientID, "your message is too short to bother an admin with that.");
				}
				else
				{
					if (!pPlayer->m_ShowName)
					{
						str_copy(pPlayer->m_ChatText, pMsg->m_pMessage, sizeof(pPlayer->m_ChatText));
						pPlayer->m_ChatTeam = Team;
						pPlayer->FixForNoName(1);
					}
					else
						SendChat(ClientID, Team, pMsg->m_pMessage, ClientID); //hier stehe ich eig SendChatFUNKTION
				}
			}
		}
		else if(MsgID == NETMSGTYPE_CL_CALLVOTE)
		{
			if(g_Config.m_SvSpamprotection && pPlayer->m_LastVoteTry && pPlayer->m_LastVoteTry+Server()->TickSpeed()*3 > Server()->Tick())
				return;

			int64 Now = Server()->Tick();
			pPlayer->m_LastVoteTry = Now;
			//if(pPlayer->GetTeam() == TEAM_SPECTATORS)
			if(g_Config.m_SvSpectatorVotes == 0 && pPlayer->GetTeam() == TEAM_SPECTATORS)
			{
				SendChatTarget(ClientID, "Spectators aren't allowed to start a vote.");
				return;
			}

			if(m_VoteCloseTime)
			{
				SendChatTarget(ClientID, "Wait for current vote to end before calling a new one.");
				return;
			}

			int Timeleft = pPlayer->m_LastVoteCall + Server()->TickSpeed()*g_Config.m_SvVoteDelay - Now;
			if(pPlayer->m_LastVoteCall && Timeleft > 0)
			{
				char aChatmsg[512] = {0};
				str_format(aChatmsg, sizeof(aChatmsg), "You must wait %d seconds before making another vote.", (Timeleft/Server()->TickSpeed())+1);
				SendChatTarget(ClientID, aChatmsg);
				return;
			}

			Timeleft = m_LastVoteCallAll + Server()->TickSpeed() * g_Config.m_SvVoteDelayAll - Now;

			if (Timeleft > 0)
			{
				char aChatmsg[512] = { 0 };
				str_format(aChatmsg, sizeof(aChatmsg), "there is a %d seconds delay between votes.", (Timeleft / Server()->TickSpeed()) + 1);
				SendChatTarget(ClientID, aChatmsg);
				return;
			}

			char aChatmsg[512] = {0};
			char aDesc[VOTE_DESC_LENGTH] = {0};
			char aCmd[VOTE_CMD_LENGTH] = {0};
			CNetMsg_Cl_CallVote *pMsg = (CNetMsg_Cl_CallVote *)pRawMsg;
			const char *pReason = pMsg->m_Reason[0] ? pMsg->m_Reason : "No reason given";

			if(str_comp_nocase(pMsg->m_Type, "option") == 0)
			{
				CVoteOptionServer *pOption = m_pVoteOptionFirst;
				while(pOption)
				{
					if(str_comp_nocase(pMsg->m_Value, pOption->m_aDescription) == 0)
					{
						if(!Console()->LineIsValid(pOption->m_aCommand))
						{
							SendChatTarget(ClientID, "Invalid option");
							return;
						}
						if(!m_apPlayers[ClientID]->m_Authed && (strncmp(pOption->m_aCommand, "sv_map ", 7) == 0 || strncmp(pOption->m_aCommand, "change_map ", 11) == 0 || strncmp(pOption->m_aCommand, "random_map", 10) == 0 || strncmp(pOption->m_aCommand, "random_unfinished_map", 21) == 0) && time_get() < m_LastMapVote + (time_freq() * g_Config.m_SvVoteMapTimeDelay))
						{
							char chatmsg[512] = {0};
							str_format(chatmsg, sizeof(chatmsg), "There's a %d second delay between map-votes, please wait %d seconds.", g_Config.m_SvVoteMapTimeDelay,((m_LastMapVote+(g_Config.m_SvVoteMapTimeDelay * time_freq()))/time_freq())-(time_get()/time_freq()));
							SendChatTarget(ClientID, chatmsg);

							return;
						}

						str_format(aChatmsg, sizeof(aChatmsg), "'%s' called vote to change server option '%s' (%s)", Server()->ClientName(ClientID),
									pOption->m_aDescription, pReason);
						str_format(aDesc, sizeof(aDesc), "%s", pOption->m_aDescription);

						if((strncmp(pOption->m_aCommand, "random_map", 10) == 0 || strncmp(pOption->m_aCommand, "random_unfinished_map", 21) == 0) && str_length(pReason) == 1 && pReason[0] >= '1' && pReason[0] <= '5')
						{
							int stars = pReason[0] - '0';
							str_format(aCmd, sizeof(aCmd), "%s %d", pOption->m_aCommand, stars);
						}
						else
						{
							str_format(aCmd, sizeof(aCmd), "%s", pOption->m_aCommand);
						}

						m_LastMapVote = time_get();
						break;
					}

					pOption = pOption->m_pNext;
				}

				if(!pOption)
				{
					if (pPlayer->m_Authed != CServer::AUTHED_ADMIN)  // allow admins to call any vote they want
					{
						str_format(aChatmsg, sizeof(aChatmsg), "'%s' isn't an option on this server", pMsg->m_Value);
						SendChatTarget(ClientID, aChatmsg);
						return;
					}
					else
					{
						str_format(aChatmsg, sizeof(aChatmsg), "'%s' called vote to change server option '%s'", Server()->ClientName(ClientID), pMsg->m_Value);
						str_format(aDesc, sizeof(aDesc), "%s", pMsg->m_Value);
						str_format(aCmd, sizeof(aCmd), "%s", pMsg->m_Value);
					}
				}

				m_LastMapVote = time_get();
				m_VoteKick = false;
				m_VoteSpec = false;
			}
			else if(str_comp_nocase(pMsg->m_Type, "kick") == 0)
			{
				if(!m_apPlayers[ClientID]->m_Authed && time_get() < m_apPlayers[ClientID]->m_Last_KickVote + (time_freq() * 5))
					return;
				else if(!m_apPlayers[ClientID]->m_Authed && time_get() < m_apPlayers[ClientID]->m_Last_KickVote + (time_freq() * g_Config.m_SvVoteKickTimeDelay))
				{
					char chatmsg[512] = {0};
					str_format(chatmsg, sizeof(chatmsg), "There's a %d second wait time between kick votes for each player please wait %d second(s)",
					g_Config.m_SvVoteKickTimeDelay,
					((m_apPlayers[ClientID]->m_Last_KickVote + (m_apPlayers[ClientID]->m_Last_KickVote*time_freq()))/time_freq())-(time_get()/time_freq())
					);
					SendChatTarget(ClientID, chatmsg);
					m_apPlayers[ClientID]->m_Last_KickVote = time_get();
					return;
				}
				//else if(!g_Config.m_SvVoteKick)
				else if(!g_Config.m_SvVoteKick && !pPlayer->m_Authed) // allow admins to call kick votes even if they are forbidden
				{
					SendChatTarget(ClientID, "Server does not allow voting to kick players");
					m_apPlayers[ClientID]->m_Last_KickVote = time_get();
					return;
				}

				if(g_Config.m_SvVoteKickMin)
				{
					int PlayerNum = 0;
					for(int i = 0; i < MAX_CLIENTS; ++i)
						if(m_apPlayers[i] && m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
							++PlayerNum;

					if(PlayerNum < g_Config.m_SvVoteKickMin)
					{
						str_format(aChatmsg, sizeof(aChatmsg), "Kick voting requires %d players on the server", g_Config.m_SvVoteKickMin);
						SendChatTarget(ClientID, aChatmsg);
						return;
					}
				}

				int KickID = str_toint(pMsg->m_Value);

				if(KickID < 0 || KickID >= MAX_CLIENTS || !m_apPlayers[KickID])
				{
					SendChatTarget(ClientID, "Invalid client id to kick");
					return;
				}
				if(KickID == ClientID)
				{
					SendChatTarget(ClientID, "You can't kick yourself");
					return;
				}
				if (!Server()->ReverseTranslate(KickID, ClientID))
				{
					return;
				}
				//if(Server()->IsAuthed(KickID))
				if((m_apPlayers[KickID]->m_Authed != CServer::AUTHED_HONEY) && // always allow kicking honeypot users
					(m_apPlayers[KickID]->m_Authed > 0 && m_apPlayers[KickID]->m_Authed >= pPlayer->m_Authed))
				{
					SendChatTarget(ClientID, "You can't kick moderators");
					m_apPlayers[ClientID]->m_Last_KickVote = time_get();
					char aBufKick[128];
					str_format(aBufKick, sizeof(aBufKick), "'%s' called for vote to kick you", Server()->ClientName(ClientID));
					SendChatTarget(KickID, aBufKick);
					return;
				}

				// Don't allow kicking if a player has no character
				if(!GetPlayerChar(ClientID) || !GetPlayerChar(KickID) || GetDDRaceTeam(ClientID) != GetDDRaceTeam(KickID))
				{
					SendChatTarget(ClientID, "You can kick only your team member");
					m_apPlayers[ClientID]->m_Last_KickVote = time_get();
					return;
				}
				if (m_apPlayers[KickID]->m_IsDummy)
				{
					SendChatTarget(ClientID, "You can't kick dummies");
					return;
				}

				str_format(aChatmsg, sizeof(aChatmsg), "'%s' called for vote to kick '%s' (%s)", Server()->ClientName(ClientID), Server()->ClientName(KickID), pReason);
				str_format(aDesc, sizeof(aDesc), "Kick '%s'", Server()->ClientName(KickID));
				if (!g_Config.m_SvVoteKickBantime)
					str_format(aCmd, sizeof(aCmd), "kick %d Kicked by vote", KickID);
				else
				{
					char aAddrStr[NETADDR_MAXSTRSIZE] = {0};
					Server()->GetClientAddr(KickID, aAddrStr, sizeof(aAddrStr));
					str_format(aCmd, sizeof(aCmd), "ban %s %d Banned by vote", aAddrStr, g_Config.m_SvVoteKickBantime);
				}
				m_apPlayers[ClientID]->m_Last_KickVote = time_get();
				m_VoteKick = true;
				m_VoteSpec = false;
			}
			else if(str_comp_nocase(pMsg->m_Type, "spectate") == 0)
			{
				if(!g_Config.m_SvVoteSpectate)
				{
					SendChatTarget(ClientID, "Server does not allow voting to move players to spectators");
					return;
				}

				int SpectateID = str_toint(pMsg->m_Value);

				if(SpectateID < 0 || SpectateID >= MAX_CLIENTS || !m_apPlayers[SpectateID] || m_apPlayers[SpectateID]->GetTeam() == TEAM_SPECTATORS)
				{
					SendChatTarget(ClientID, "Invalid client id to move");
					return;
				}
				if(SpectateID == ClientID)
				{
					SendChatTarget(ClientID, "You can't move yourself");
					return;
				}
				if (!Server()->ReverseTranslate(SpectateID, ClientID))
				{
					return;
				}

				if(!GetPlayerChar(ClientID) || !GetPlayerChar(SpectateID) || GetDDRaceTeam(ClientID) != GetDDRaceTeam(SpectateID))
				{
					SendChatTarget(ClientID, "You can only move your team member to specators");
					return;
				}

				if(g_Config.m_SvPauseable && g_Config.m_SvVotePause)
				{
					str_format(aChatmsg, sizeof(aChatmsg), "'%s' called for vote to pause '%s' for %d seconds (%s)", Server()->ClientName(ClientID), Server()->ClientName(SpectateID), g_Config.m_SvVotePauseTime, pReason);
					str_format(aDesc, sizeof(aDesc), "Pause '%s' (%ds)", Server()->ClientName(SpectateID), g_Config.m_SvVotePauseTime);
					str_format(aCmd, sizeof(aCmd), "force_pause %d %d", SpectateID, g_Config.m_SvVotePauseTime);
				}
				else
				{
					str_format(aChatmsg, sizeof(aChatmsg), "'%s' called for vote to move '%s' to spectators (%s)", Server()->ClientName(ClientID), Server()->ClientName(SpectateID), pReason);
					str_format(aDesc, sizeof(aDesc), "move '%s' to spectators", Server()->ClientName(SpectateID));
				str_format(aCmd, sizeof(aCmd), "set_team %d -1 %d", SpectateID, g_Config.m_SvVoteSpectateRejoindelay);
				}
				m_VoteKick = false;
				m_VoteSpec = true;
			}

			if(aCmd[0] && str_comp(aCmd,"info"))
				CallVote(ClientID, aDesc, aCmd, pReason, aChatmsg);
		}
		else if(MsgID == NETMSGTYPE_CL_VOTE)
		{
			CNetMsg_Cl_Vote *pMsg = (CNetMsg_Cl_Vote *)pRawMsg;
			CCharacter *pChr = pPlayer->GetCharacter();

			if (pMsg->m_Vote == 1) //vote yes (f3)
			{
				//SendChatTarget(ClientID, "you pressed f3");

				if (pChr)
				{
					if (pChr->m_InShop)
					{
						if (pChr->m_PurchaseState == 1)
						{
							pChr->ConfirmPurchase();
						}
						else if (pChr->m_PurchaseState == 2)
						{
							pChr->PurchaseEnd(false);
						}
					}
					else
					{
						if (pChr)
						{
							IGameController* ControllerDDrace = pPlayer->GetCharacter()->GameServer()->m_pController;
							if (((CGameControllerDDRace*)ControllerDDrace)->m_apFlags[0])
							{
								if (((CGameControllerDDRace*)ControllerDDrace)->m_apFlags[0]->m_pCarryingCharacter == pChr) {
									((CGameControllerDDRace*)ControllerDDrace)->DropFlag(0, pChr->GetAimDir()); //red
									//SendChatTarget(ClientID, "you dropped red flag");
								}
							}
							if (((CGameControllerDDRace*)ControllerDDrace)->m_apFlags[1])
							{
								if (((CGameControllerDDRace*)ControllerDDrace)->m_apFlags[1]->m_pCarryingCharacter == pChr) {
									((CGameControllerDDRace*)ControllerDDrace)->DropFlag(1, pChr->GetAimDir()); //blue
									//SendChatTarget(ClientID, "you dropped blue flag");
								}
							}
						}
					}
				}
			}
			else if (pMsg->m_Vote == -1) //vote no (f4)
			{
				//SendChatTarget(ClientID, "you pressed f4");

				if (pChr)
				{
					if (pChr->m_InShop)
					{
						if (pChr->m_PurchaseState == 2)
						{
							pChr->PurchaseEnd(true);
						}
						else if (pChr->m_ShopWindowPage == -1)
						{
							pChr->StartShop();
						}
					}
					else
					{
						if (g_Config.m_SvAllowDroppingWeapons)
						{
							pChr->DropWeapon(pChr->GetActiveWeapon()); // drop the weapon youre holding.
						}
					}
				}
			}

			if(!m_VoteCloseTime)
				return;

			if(g_Config.m_SvSpamprotection && pPlayer->m_LastVoteTry && pPlayer->m_LastVoteTry+Server()->TickSpeed()*3 > Server()->Tick())
				return;

			int64 Now = Server()->Tick();

			pPlayer->m_LastVoteTry = Now;

			if(!pMsg->m_Vote)
				return;

			pPlayer->m_Vote = pMsg->m_Vote;
			pPlayer->m_VotePos = ++m_VotePos;
			m_VoteUpdate = true;
		}
		else if (MsgID == NETMSGTYPE_CL_SETTEAM && !m_World.m_Paused)
		{
			CNetMsg_Cl_SetTeam *pMsg = (CNetMsg_Cl_SetTeam *)pRawMsg;

			//if(pPlayer->GetTeam() == pMsg->m_Team || (g_Config.m_SvSpamprotection && pPlayer->m_LastSetTeam && pPlayer->m_LastSetTeam+Server()->TickSpeed()*3 > Server()->Tick()))
			if(pPlayer->GetTeam() == pMsg->m_Team || (g_Config.m_SvSpamprotection && pPlayer->m_LastSetTeam && pPlayer->m_LastSetTeam + Server()->TickSpeed() * g_Config.m_SvTeamChangeDelay > Server()->Tick()))
				return;

			/*if(pMsg->m_Team != TEAM_SPECTATORS && m_LockTeams)
			{
				pPlayer->m_LastSetTeam = Server()->Tick();
				SendBroadcast("Teams are locked", ClientID);
				return;
			}*/

			if (IsMinigame(ClientID))
			{
				SendChatTarget(ClientID, "[MINIGAMES] You can't change team while playing minigames or being in jail.");
				return;
			}

			if (m_apPlayers[ClientID]->m_SpawnBlocks > 3)
			{
				SendChatTarget(ClientID, "[SPAWNBLOCK] You can't change team because you spawnblock too much. Try agian later.");
				return;
			}

			if (m_apPlayers[ClientID]->m_IsBlockWaving)
			{
				SendChatTarget(ClientID, "[BlockWave] you can't change team while block waving. Try '/blockwave leave'");
				return;
			}

			//zCatch survival LMS ChillerDragon Instagib grenade rifle
			if (g_Config.m_SvInstagibMode == 2 || g_Config.m_SvInstagibMode == 4) //gLMS iLMS
			{
				SendChatTarget(ClientID, "You can't join running survival games. Wait until the round ends.");
				return;
			}

			if (pPlayer->m_GangsterBagMoney)
			{
				SendChatTarget(ClientID, "Make sure to empty your gangsterbag before disconnecting/spectating or you will lose it.");
				SendChatTarget(ClientID, "or clear it yourself with '/gangsterbag clear'");
				return;
			}

			//Kill Protection
			CCharacter* pChr = pPlayer->GetCharacter();
			if(pChr)
			{
				int CurrTime = (Server()->Tick() - pChr->m_StartTime) / Server()->TickSpeed();
				if(g_Config.m_SvKillProtection != 0 && CurrTime >= (60 * g_Config.m_SvKillProtection) && pChr->m_DDRaceState == DDRACE_STARTED)
				{
					SendChatTarget(ClientID, "Kill Protection enabled. If you really want to join the spectators, first type /kill");
					return;
				}
			}

			if(pPlayer->m_TeamChangeTick > Server()->Tick())
			{
				pPlayer->m_LastSetTeam = Server()->Tick();
				int TimeLeft = (pPlayer->m_TeamChangeTick - Server()->Tick())/Server()->TickSpeed();
				char aBuf[128];
				str_format(aBuf, sizeof(aBuf), "Time to wait before changing team: %02d:%02d", TimeLeft/60, TimeLeft%60);
				SendBroadcast(aBuf, ClientID);
				return;
			}

			// Switch team on given client and kill/respawn him
			if(m_pController->CanJoinTeam(pMsg->m_Team, ClientID))
			{
				//if(m_pController->CanChangeTeam(pPlayer, pMsg->m_Team))

				if(pPlayer->m_Paused)
					SendChatTarget(ClientID,"Use /pause first then you can kill");
				else
				{
					//pPlayer->m_LastSetTeam = Server()->Tick();
					if(pPlayer->GetTeam() == TEAM_SPECTATORS || pMsg->m_Team == TEAM_SPECTATORS)
						m_VoteUpdate = true;
					pPlayer->SetTeam(pMsg->m_Team);
					//(void)m_pController->CheckTeamBalance();
					pPlayer->m_TeamChangeTick = Server()->Tick();
				}
				//else
					//SendBroadcast("Teams must be balanced, please join other team", ClientID);
			}
			else
			{
				char aBuf[128];
				str_format(aBuf, sizeof(aBuf), "Only %d active players are allowed", Server()->MaxClients()-g_Config.m_SvSpectatorSlots);
				SendBroadcast(aBuf, ClientID);
			}
		}
		else if (MsgID == NETMSGTYPE_CL_ISDDNET)
		{
			int Version = pUnpacker->GetInt();

			if (pUnpacker->Error())
			{
				if (pPlayer->m_ClientVersion < VERSION_DDRACE)
					pPlayer->m_ClientVersion = VERSION_DDRACE;
			}
			else if(pPlayer->m_ClientVersion < Version)
				pPlayer->m_ClientVersion = Version;

			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "%d using Custom Client %d", ClientID, pPlayer->m_ClientVersion);
			dbg_msg("DDNet", aBuf);

			if (Version >= 11043 && Version < 11073)
				m_apPlayers[ClientID]->m_ScoreFixForDDNet = true;

			//first update his teams state
			((CGameControllerDDRace*)m_pController)->m_Teams.SendTeamsState(ClientID);

			//second give him records
			SendRecord(ClientID);

			//third give him others current time for table score
			if(g_Config.m_SvHideScore) return;
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(m_apPlayers[i] && Score()->PlayerData(i)->m_CurrentTime > 0)
				{
					CNetMsg_Sv_PlayerTime Msg;
					Msg.m_Time = Score()->PlayerData(i)->m_CurrentTime * 100;
					Msg.m_ClientID = i;
					Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
					//also send its time to others

				}
			}
			//also send its time to others
			if(Score()->PlayerData(ClientID)->m_CurrentTime > 0)
			{
				//TODO: make function for this fucking steps
				CNetMsg_Sv_PlayerTime Msg;
				Msg.m_Time = Score()->PlayerData(ClientID)->m_CurrentTime * 100;
				Msg.m_ClientID = ClientID;
				Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
			}

			//and give him correct tunings
			if (Version >= VERSION_DDNET_EXTRATUNES)
				SendTuningParams(ClientID, pPlayer->m_TuneZone);


			//tell older clients than 11.4.3 to update to at least 11.4.3 to get zoom and eye wheel support for ddnet++
			if (Version < 11043 && g_Config.m_SvClientSuggestionOld[0] != '\0')
				SendBroadcast(g_Config.m_SvClientSuggestionSupported, ClientID);

			// block servers dont need those messages
			/*//tell old clients to update
			if (Version < VERSION_DDNET_UPDATER_FIXED && g_Config.m_SvClientSuggestionOld[0] != '\0')
				SendBroadcast(g_Config.m_SvClientSuggestionOld, ClientID);
			//tell known bot clients that they're botting and we know it
			if (((Version >= 15 && Version < 100) || Version == 502) && g_Config.m_SvClientSuggestionBot[0] != '\0')
				SendBroadcast(g_Config.m_SvClientSuggestionBot, ClientID);*/
		}
		else if (MsgID == NETMSGTYPE_CL_SHOWOTHERS)
		{
			if(g_Config.m_SvShowOthers && !g_Config.m_SvShowOthersDefault)
			{
				CNetMsg_Cl_ShowOthers *pMsg = (CNetMsg_Cl_ShowOthers *)pRawMsg;
				pPlayer->m_ShowOthers = (bool)pMsg->m_Show;
			}
		}
		else if (MsgID == NETMSGTYPE_CL_SETSPECTATORMODE && !m_World.m_Paused)
		{
			CNetMsg_Cl_SetSpectatorMode *pMsg = (CNetMsg_Cl_SetSpectatorMode *)pRawMsg;

			if(pMsg->m_SpectatorID != SPEC_FREEVIEW)
				if (!Server()->ReverseTranslate(pMsg->m_SpectatorID, ClientID))
					return;

			if((g_Config.m_SvSpamprotection && pPlayer->m_LastSetSpectatorMode && pPlayer->m_LastSetSpectatorMode+Server()->TickSpeed()/4 > Server()->Tick()))
				return;

			pPlayer->m_LastSetSpectatorMode = Server()->Tick();
			if(pMsg->m_SpectatorID != SPEC_FREEVIEW && (!m_apPlayers[pMsg->m_SpectatorID] || m_apPlayers[pMsg->m_SpectatorID]->GetTeam() == TEAM_SPECTATORS))
				SendChatTarget(ClientID, "Invalid spectator id used");
			else
				pPlayer->m_SpectatorID = pMsg->m_SpectatorID;
		}
		else if (MsgID == NETMSGTYPE_CL_CHANGEINFO)
		{
			if (!pPlayer->m_SpookyGhostActive)
			{
				if (g_Config.m_SvSpamprotection && pPlayer->m_LastChangeInfo && pPlayer->m_LastChangeInfo + Server()->TickSpeed()*g_Config.m_SvInfoChangeDelay > Server()->Tick())
					return;

				CNetMsg_Cl_ChangeInfo *pMsg = (CNetMsg_Cl_ChangeInfo *)pRawMsg;
				pPlayer->m_LastChangeInfo = Server()->Tick();

				// set infos
				char aOldName[MAX_NAME_LENGTH];
				str_copy(aOldName, Server()->ClientName(ClientID), sizeof(aOldName));
				Server()->SetClientName(ClientID, pMsg->m_pName);
				if (str_comp(aOldName, Server()->ClientName(ClientID)) != 0)
				{
					char aChatText[256];
					str_format(aChatText, sizeof(aChatText), "'%s' changed name to '%s'", aOldName, Server()->ClientName(ClientID));
					SendChat(-1, CGameContext::CHAT_ALL, aChatText);

					// reload scores

					Score()->PlayerData(ClientID)->Reset();
					Score()->LoadScore(ClientID);
					Score()->PlayerData(ClientID)->m_CurrentTime = Score()->PlayerData(ClientID)->m_BestTime;
					if (g_Config.m_SvInstagibMode || g_Config.m_SvDDPPscore == 0)
					{
						m_apPlayers[ClientID]->m_Score = 0;
					}
					else
					{
						m_apPlayers[ClientID]->m_Score = (Score()->PlayerData(ClientID)->m_BestTime) ? Score()->PlayerData(ClientID)->m_BestTime : -9999;
					}
				}
				Server()->SetClientClan(ClientID, pMsg->m_pClan);
				Server()->SetClientCountry(ClientID, pMsg->m_Country);
				str_copy(pPlayer->m_TeeInfos.m_SkinName, pMsg->m_pSkin, sizeof(pPlayer->m_TeeInfos.m_SkinName));
				pPlayer->m_TeeInfos.m_UseCustomColor = pMsg->m_UseCustomColor;
				pPlayer->m_TeeInfos.m_ColorBody = pMsg->m_ColorBody;
				pPlayer->m_TeeInfos.m_ColorFeet = pMsg->m_ColorFeet;
				//m_pController->OnPlayerInfoChange(pPlayer);

				if (pPlayer->GetCharacter())
				{
					pPlayer->GetCharacter()->SaveRealInfos();
				}
			}
		}
		else if (MsgID == NETMSGTYPE_CL_EMOTICON && !m_World.m_Paused)
		{
			CNetMsg_Cl_Emoticon *pMsg = (CNetMsg_Cl_Emoticon *)pRawMsg;

			if(g_Config.m_SvSpamprotection && pPlayer->m_LastEmote && pPlayer->m_LastEmote+Server()->TickSpeed()*g_Config.m_SvEmoticonDelay > Server()->Tick())
				return;

			pPlayer->m_LastEmote = Server()->Tick();

			SendEmoticon(ClientID, pMsg->m_Emoticon);
			CCharacter* pChr = pPlayer->GetCharacter();
			if(pChr && g_Config.m_SvEmotionalTees && pPlayer->m_EyeEmote)
			{
				switch(pMsg->m_Emoticon)
				{
				case EMOTICON_EXCLAMATION:
				case EMOTICON_GHOST:
				case EMOTICON_QUESTION:
				case EMOTICON_WTF:
						pChr->SetEmoteType(EMOTE_SURPRISE);
						break;
				case EMOTICON_DOTDOT:
				case EMOTICON_DROP:
				case EMOTICON_ZZZ:
						pChr->SetEmoteType(EMOTE_BLINK);
						break;
				case EMOTICON_EYES:
				case EMOTICON_HEARTS:
				case EMOTICON_MUSIC:
						pChr->SetEmoteType(EMOTE_HAPPY);
						break;
				case EMOTICON_OOP:
				case EMOTICON_SORRY:
				case EMOTICON_SUSHI:
						pChr->SetEmoteType(EMOTE_PAIN);
						break;
				case EMOTICON_DEVILTEE:
				case EMOTICON_SPLATTEE:
				case EMOTICON_ZOMG:
						pChr->SetEmoteType(EMOTE_ANGRY);
						break;
					default:
						pChr->SetEmoteType(EMOTE_NORMAL);
						break;
				}
				if (pPlayer->m_SpookyGhostActive)
				{
					pChr->SetEmoteType(EMOTE_SURPRISE);
				}
				pChr->SetEmoteStop(Server()->Tick() + 2 * Server()->TickSpeed());
			}
		}
		else if (MsgID == NETMSGTYPE_CL_KILL && !m_World.m_Paused)
		{
			if (m_InstaGrenadeRoundEndTickTicker && m_apPlayers[ClientID]->m_IsInstaArena_gdm)
			{
				return; //yy evil silent return
			}
			if (m_InstaRifleRoundEndTickTicker && m_apPlayers[ClientID]->m_IsInstaArena_idm)
			{
				return; //yy evil silent return
			}


			if (m_apPlayers[ClientID]->m_IsBlockTourning)
			{
				if (Server()->TickSpeed() * 5 > m_BlockTournaLobbyTick)
				{
					//silent return selfkill in last 5 secs of lobby tick to prevent the char being dead on tourna start
					return;
				}
			}

			if (m_apPlayers[ClientID]->m_IsBlockWaving && !pPlayer->m_IsBlockWaveWaiting)
			{
				SendChatTarget(ClientID, "[BlockWave] you can't selfkill while block waving. try '/blockwave leave'.");
				return;
			}

			if (m_apPlayers[ClientID]->m_SpawnBlocks > 3 && g_Config.m_SvSpawnBlockProtection == 2)
			{
				SendChatTarget(ClientID, "[SPAWNBLOCK] You can't selfkill because you spawnblock too much. Try agian later.");
				return;
			}

			if (!g_Config.m_SvAllowBombSelfkill && GetPlayerChar(ClientID) && GetPlayerChar(ClientID)->m_IsBombing)
			{
				SendChatTarget(ClientID, "[BOMB] selfkill protection activated. Try '/bomb leave' to leave and get the money back. All other ways of leaving the game are leading to lose your money.");
				return;
			}

			if (m_apPlayers[ClientID]->m_IsSurvivaling)
			{
				if (g_Config.m_SvSurvivalKillProtection == 2) //full on
				{
					SendChatTarget(ClientID, "[SURVIVAL] kill protection. '/survival leave' first to kill.");
					return;
				}
				else if (g_Config.m_SvSurvivalKillProtection == 1 && m_apPlayers[ClientID]->m_IsSurvivalLobby == false) //allowed in lobby
				{
					SendChatTarget(ClientID, "[SURVIVAL] kill protection. '/survival leave' first to kill.");
					return;
				}
				//else == off
			}

			if(m_VoteCloseTime && m_VoteCreator == ClientID && GetDDRaceTeam(ClientID) && (m_VoteKick || m_VoteSpec))
			{
				SendChatTarget(ClientID, "You are running a vote please try again after the vote is done!");
				return;
			}
			if(pPlayer->m_LastKill && pPlayer->m_LastKill+Server()->TickSpeed()*g_Config.m_SvKillDelay > Server()->Tick())
				return;
			if(pPlayer->m_Paused)
				return;

			CCharacter* pChr = pPlayer->GetCharacter();
			if(!pChr)
				return;

			//Kill Protection
			int CurrTime = (Server()->Tick() - pChr->m_StartTime) / Server()->TickSpeed();
			if(g_Config.m_SvKillProtection != 0 && CurrTime >= (60 * g_Config.m_SvKillProtection) && pChr->m_DDRaceState == DDRACE_STARTED)
			{
				SendChatTarget(ClientID, "Kill Protection enabled. If you really want to kill, type /kill");
				return;
			}

			if (m_apPlayers[ClientID]->m_IsInstaArena_fng && pChr->m_FreezeTime)
			{
				SendChatTarget(ClientID, "[INSTA] You can't suicide in fng games while being frozen.");
				return;
			}

			pPlayer->m_LastKill = Server()->Tick();
			pPlayer->KillCharacter(WEAPON_SELF);
			pPlayer->Respawn();
		}
	}
	if (MsgID == NETMSGTYPE_CL_STARTINFO)
	{
		if(pPlayer->m_IsReady)
			return;

		CNetMsg_Cl_StartInfo *pMsg = (CNetMsg_Cl_StartInfo *)pRawMsg;
		pPlayer->m_LastChangeInfo = Server()->Tick();

		// set start infos
		Server()->SetClientName(ClientID, pMsg->m_pName);
		Server()->SetClientClan(ClientID, pMsg->m_pClan);
		Server()->SetClientCountry(ClientID, pMsg->m_Country);
		str_copy(pPlayer->m_TeeInfos.m_SkinName, pMsg->m_pSkin, sizeof(pPlayer->m_TeeInfos.m_SkinName));
		pPlayer->m_TeeInfos.m_UseCustomColor = pMsg->m_UseCustomColor;
		pPlayer->m_TeeInfos.m_ColorBody = pMsg->m_ColorBody;
		pPlayer->m_TeeInfos.m_ColorFeet = pMsg->m_ColorFeet;
		//m_pController->OnPlayerInfoChange(pPlayer);

		// send clear vote options
		CNetMsg_Sv_VoteClearOptions ClearMsg;
		Server()->SendPackMsg(&ClearMsg, MSGFLAG_VITAL, ClientID);

		// begin sending vote options
		pPlayer->m_SendVoteIndex = 0;

		// send tuning parameters to client
		SendTuningParams(ClientID, pPlayer->m_TuneZone);

		// client is ready to enter
		if (!pPlayer->m_IsReady)
		{
			pPlayer->m_IsReady = true;
			CNetMsg_Sv_ReadyToEnter m;
			Server()->SendPackMsg(&m, MSGFLAG_VITAL|MSGFLAG_FLUSH, ClientID);
		}
	}
}

void CGameContext::ConTuneParam(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	const char *pParamName = pResult->GetString(0);
	float NewValue = pResult->GetFloat(1);

	if(pSelf->Tuning()->Set(pParamName, NewValue))
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "%s changed to %.2f", pParamName, NewValue);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", aBuf);
		pSelf->SendTuningParams(-1);
	}
	else
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", "No such tuning parameter");
}

void CGameContext::ConTuneReset(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	/*CTuningParams TuningParams;
	*pSelf->Tuning() = TuningParams;
	pSelf->SendTuningParams(-1);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", "Tuning reset");*/
	pSelf->ResetTuning();
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", "Tuning reset");
}

void CGameContext::ConTuneDump(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	char aBuf[256];
	for(int i = 0; i < pSelf->Tuning()->Num(); i++)
	{
		float v;
		pSelf->Tuning()->Get(i, &v);
		str_format(aBuf, sizeof(aBuf), "%s %.2f", pSelf->Tuning()->m_apNames[i], v);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", aBuf);
	}
}

void CGameContext::ConTuneZone(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	int List = pResult->GetInteger(0);
	const char *pParamName = pResult->GetString(1);
	float NewValue = pResult->GetFloat(2);

	if (List >= 0 && List < NUM_TUNINGZONES)
	{
		if(pSelf->TuningList()[List].Set(pParamName, NewValue))
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "%s in zone %d changed to %.2f", pParamName, List, NewValue);
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", aBuf);
			pSelf->SendTuningParams(-1, List);
		}
		else
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", "No such tuning parameter");
	}
}

void CGameContext::ConTuneDumpZone(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	int List = pResult->GetInteger(0);
	char aBuf[256];
	if (List >= 0 && List < NUM_TUNINGZONES)
	{
		for(int i = 0; i < pSelf->TuningList()[List].Num(); i++)
		{
			float v;
			pSelf->TuningList()[List].Get(i, &v);
			str_format(aBuf, sizeof(aBuf), "zone %d: %s %.2f", List, pSelf->TuningList()[List].m_apNames[i], v);
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", aBuf);
		}
	}
}

void CGameContext::ConTuneResetZone(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	CTuningParams TuningParams;
	if (pResult->NumArguments())
	{
		int List = pResult->GetInteger(0);
		if (List >= 0 && List < NUM_TUNINGZONES)
		{
			pSelf->TuningList()[List] = TuningParams;
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "Tunezone %d resetted", List);
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", aBuf);
			pSelf->SendTuningParams(-1, List);
		}
	}
	else
	{
		for (int i = 0; i < NUM_TUNINGZONES; i++)
		{
			*(pSelf->TuningList()+i) = TuningParams;
			pSelf->SendTuningParams(-1, i);
		}
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "tuning", "All Tunezones resetted");
	}
}

void CGameContext::ConTuneSetZoneMsgEnter(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (pResult->NumArguments())
	{
		int List = pResult->GetInteger(0);
		if (List >= 0 && List < NUM_TUNINGZONES)
		{
			str_format(pSelf->m_ZoneEnterMsg[List], sizeof(pSelf->m_ZoneEnterMsg[List]), pResult->GetString(1));
		}
	}
}

void CGameContext::ConTuneSetZoneMsgLeave(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (pResult->NumArguments())
	{
		int List = pResult->GetInteger(0);
		if (List >= 0 && List < NUM_TUNINGZONES)
		{
			str_format(pSelf->m_ZoneLeaveMsg[List], sizeof(pSelf->m_ZoneLeaveMsg[List]), pResult->GetString(1));
		}
	}
}

void CGameContext::ConSwitchOpen(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Switch = pResult->GetInteger(0);

	if (pSelf->Collision()->m_NumSwitchers > 0 && Switch >= 0 && Switch < pSelf->Collision()->m_NumSwitchers+1)
	{
		pSelf->Collision()->m_pSwitchers[Switch].m_Initial = false;
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "switch %d opened by default", Switch);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
	}
}

void CGameContext::ConPause(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;

	/*if(pSelf->m_pController->IsGameOver())
		return;*/

	pSelf->m_World.m_Paused ^= 1;
}

void CGameContext::ConChangeMap(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->m_pController->ChangeMap(pResult->NumArguments() ? pResult->GetString(0) : "");
}

void CGameContext::ConRandomMap(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;

	int stars = 0;
	if (pResult->NumArguments())
		stars = pResult->GetInteger(0);

	if (pSelf->m_VoteCreator == -1)
	{
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "[DDNet++] error server can't vote random map.");
		return;
	}

	pSelf->m_pScore->RandomMap(pSelf->m_VoteCreator, stars);
}

void CGameContext::ConRandomUnfinishedMap(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;

	int stars = 0;
	if (pResult->NumArguments())
		stars = pResult->GetInteger(0);

	if (pSelf->m_VoteCreator == -1)
	{
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "[DDNet++] error server can't vote random map.");
		return;
	}

	pSelf->m_pScore->RandomUnfinishedMap(pSelf->m_VoteCreator, stars);
}

void CGameContext::ConRestart(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(pResult->NumArguments())
		pSelf->m_pController->DoWarmup(pResult->GetInteger(0));
	else
		pSelf->m_pController->StartRound();
}

void CGameContext::ConBroadcast(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;

	char aBuf[1024];
	str_copy(aBuf, pResult->GetString(0), sizeof(aBuf));

	int i, j;
	for(i = 0, j = 0; aBuf[i]; i++, j++)
	{
		if(aBuf[i] == '\\' && aBuf[i+1] == 'n')
		{
			aBuf[j] = '\n';
			i++;
		}
		else if (i != j)
		{
			aBuf[j] = aBuf[i];
		}
	}
	aBuf[j] = '\0';

	pSelf->SendBroadcast(aBuf, -1);
}

void CGameContext::ConSay(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->SendChat(-1, CGameContext::CHAT_ALL, pResult->GetString(0));
}

void CGameContext::ConSetTeam(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	int ClientID = clamp(pResult->GetInteger(0), 0, (int)MAX_CLIENTS-1);
	int Team = clamp(pResult->GetInteger(1), -1, 1);
	int Delay = pResult->NumArguments()>2 ? pResult->GetInteger(2) : 0;
	if(!pSelf->m_apPlayers[ClientID])
		return;

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "moved client %d to team %d", ClientID, Team);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);

	pSelf->m_apPlayers[ClientID]->m_TeamChangeTick = pSelf->Server()->Tick()+pSelf->Server()->TickSpeed()*Delay*60;
	pSelf->m_apPlayers[ClientID]->SetTeam(Team);
	if(Team == TEAM_SPECTATORS)
		pSelf->m_apPlayers[ClientID]->m_Paused = CPlayer::PAUSED_NONE;
	// (void)pSelf->m_pController->CheckTeamBalance();
}

void CGameContext::ConSetTeamAll(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Team = clamp(pResult->GetInteger(0), -1, 1);

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "All players were moved to the %s", pSelf->m_pController->GetTeamName(Team));
	pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);

	for(int i = 0; i < MAX_CLIENTS; ++i)
		if(pSelf->m_apPlayers[i])
			pSelf->m_apPlayers[i]->SetTeam(Team, false);

	// (void)pSelf->m_pController->CheckTeamBalance();
}
/*
void CGameContext::ConSwapTeams(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->SwapTeams();
}

void CGameContext::ConShuffleTeams(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!pSelf->m_pController->IsTeamplay())
		return;

	int CounterRed = 0;
	int CounterBlue = 0;
	int PlayerTeam = 0;
	for(int i = 0; i < MAX_CLIENTS; ++i)
		if(pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
			++PlayerTeam;
	PlayerTeam = (PlayerTeam+1)/2;

	pSelf->SendChat(-1, CGameContext::CHAT_ALL, "Teams were shuffled");

	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)
		{
			if(CounterRed == PlayerTeam)
				pSelf->m_apPlayers[i]->SetTeam(TEAM_BLUE, false);
			else if(CounterBlue == PlayerTeam)
				pSelf->m_apPlayers[i]->SetTeam(TEAM_RED, false);
			else
			{
				if(rand() % 2)
				{
					pSelf->m_apPlayers[i]->SetTeam(TEAM_BLUE, false);
					++CounterBlue;
				}
				else
				{
					pSelf->m_apPlayers[i]->SetTeam(TEAM_RED, false);
					++CounterRed;
				}
			}
		}
	}

	// (void)pSelf->m_pController->CheckTeamBalance();
}

void CGameContext::ConLockTeams(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->m_LockTeams ^= 1;
	if(pSelf->m_LockTeams)
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "Teams were locked");
	else
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "Teams were unlocked");
}
*/
void CGameContext::ConAddVote(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	const char *pDescription = pResult->GetString(0);
	const char *pCommand = pResult->GetString(1);

	if(pSelf->m_NumVoteOptions == MAX_VOTE_OPTIONS)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "maximum number of vote options reached");
		return;
	}

	// check for valid option
	if(!pSelf->Console()->LineIsValid(pCommand) || str_length(pCommand) >= VOTE_CMD_LENGTH)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "skipped invalid command '%s'", pCommand);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
		return;
	}
	while(*pDescription && *pDescription == ' ')
		pDescription++;
	if(str_length(pDescription) >= VOTE_DESC_LENGTH || *pDescription == 0)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "skipped invalid option '%s'", pDescription);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
		return;
	}

	// check for duplicate entry
	CVoteOptionServer *pOption = pSelf->m_pVoteOptionFirst;
	while(pOption)
	{
		if(str_comp_nocase(pDescription, pOption->m_aDescription) == 0)
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "option '%s' already exists", pDescription);
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
			return;
		}
		pOption = pOption->m_pNext;
	}

	// add the option
	++pSelf->m_NumVoteOptions;
	int Len = str_length(pCommand);

	pOption = (CVoteOptionServer *)pSelf->m_pVoteOptionHeap->Allocate(sizeof(CVoteOptionServer) + Len);
	pOption->m_pNext = 0;
	pOption->m_pPrev = pSelf->m_pVoteOptionLast;
	if(pOption->m_pPrev)
		pOption->m_pPrev->m_pNext = pOption;
	pSelf->m_pVoteOptionLast = pOption;
	if(!pSelf->m_pVoteOptionFirst)
		pSelf->m_pVoteOptionFirst = pOption;

	str_copy(pOption->m_aDescription, pDescription, sizeof(pOption->m_aDescription));
	mem_copy(pOption->m_aCommand, pCommand, Len+1);
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "added option '%s' '%s'", pOption->m_aDescription, pOption->m_aCommand);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
}

void CGameContext::ConRemoveVote(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	const char *pDescription = pResult->GetString(0);

	// check for valid option
	CVoteOptionServer *pOption = pSelf->m_pVoteOptionFirst;
	while(pOption)
	{
		if(str_comp_nocase(pDescription, pOption->m_aDescription) == 0)
			break;
		pOption = pOption->m_pNext;
	}
	if(!pOption)
	{
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "option '%s' does not exist", pDescription);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
		return;
	}

	// start reloading vote option list
	// clear vote options
	CNetMsg_Sv_VoteClearOptions VoteClearOptionsMsg;
	pSelf->Server()->SendPackMsg(&VoteClearOptionsMsg, MSGFLAG_VITAL, -1);

	// reset sending of vote options
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(pSelf->m_apPlayers[i])
			pSelf->m_apPlayers[i]->m_SendVoteIndex = 0;
	}

	// TODO: improve this
	// remove the option
	--pSelf->m_NumVoteOptions;
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "removed option '%s' '%s'", pOption->m_aDescription, pOption->m_aCommand);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);

	CHeap *pVoteOptionHeap = new CHeap();
	CVoteOptionServer *pVoteOptionFirst = 0;
	CVoteOptionServer *pVoteOptionLast = 0;
	int NumVoteOptions = pSelf->m_NumVoteOptions;
	for(CVoteOptionServer *pSrc = pSelf->m_pVoteOptionFirst; pSrc; pSrc = pSrc->m_pNext)
	{
		if(pSrc == pOption)
			continue;

		// copy option
		int Len = str_length(pSrc->m_aCommand);
		CVoteOptionServer *pDst = (CVoteOptionServer *)pVoteOptionHeap->Allocate(sizeof(CVoteOptionServer) + Len);
		pDst->m_pNext = 0;
		pDst->m_pPrev = pVoteOptionLast;
		if(pDst->m_pPrev)
			pDst->m_pPrev->m_pNext = pDst;
		pVoteOptionLast = pDst;
		if(!pVoteOptionFirst)
			pVoteOptionFirst = pDst;

		str_copy(pDst->m_aDescription, pSrc->m_aDescription, sizeof(pDst->m_aDescription));
		mem_copy(pDst->m_aCommand, pSrc->m_aCommand, Len+1);
	}

	// clean up
	delete pSelf->m_pVoteOptionHeap;
	pSelf->m_pVoteOptionHeap = pVoteOptionHeap;
	pSelf->m_pVoteOptionFirst = pVoteOptionFirst;
	pSelf->m_pVoteOptionLast = pVoteOptionLast;
	pSelf->m_NumVoteOptions = NumVoteOptions;
}

void CGameContext::ConForceVote(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	const char *pType = pResult->GetString(0);
	const char *pValue = pResult->GetString(1);
	const char *pReason = pResult->NumArguments() > 2 && pResult->GetString(2)[0] ? pResult->GetString(2) : "No reason given";
	char aBuf[128] = {0};

	if(str_comp_nocase(pType, "option") == 0)
	{
		CVoteOptionServer *pOption = pSelf->m_pVoteOptionFirst;
		while(pOption)
		{
			if(str_comp_nocase(pValue, pOption->m_aDescription) == 0)
			{
				str_format(aBuf, sizeof(aBuf), "moderator forced server option '%s' (%s)", pValue, pReason);
				pSelf->SendChatTarget(-1, aBuf);
				pSelf->Console()->ExecuteLine(pOption->m_aCommand);
				break;
			}

			pOption = pOption->m_pNext;
		}

		if(!pOption)
		{
			str_format(aBuf, sizeof(aBuf), "'%s' isn't an option on this server", pValue);
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
			return;
		}
	}
	else if(str_comp_nocase(pType, "kick") == 0)
	{
		int KickID = str_toint(pValue);
		if(KickID < 0 || KickID >= MAX_CLIENTS || !pSelf->m_apPlayers[KickID])
		{
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "Invalid client id to kick");
			return;
		}

		if (!g_Config.m_SvVoteKickBantime)
		{
			str_format(aBuf, sizeof(aBuf), "kick %d %s", KickID, pReason);
			pSelf->Console()->ExecuteLine(aBuf);
		}
		else
		{
			char aAddrStr[NETADDR_MAXSTRSIZE] = {0};
			pSelf->Server()->GetClientAddr(KickID, aAddrStr, sizeof(aAddrStr));
			str_format(aBuf, sizeof(aBuf), "ban %s %d %s", aAddrStr, g_Config.m_SvVoteKickBantime, pReason);
			pSelf->Console()->ExecuteLine(aBuf);
		}
	}
	else if(str_comp_nocase(pType, "spectate") == 0)
	{
		int SpectateID = str_toint(pValue);
		if(SpectateID < 0 || SpectateID >= MAX_CLIENTS || !pSelf->m_apPlayers[SpectateID] || pSelf->m_apPlayers[SpectateID]->GetTeam() == TEAM_SPECTATORS)
		{
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "Invalid client id to move");
			return;
		}

		str_format(aBuf, sizeof(aBuf), "admin moved '%s' to spectator (%s)", pSelf->Server()->ClientName(SpectateID), pReason);
		pSelf->SendChatTarget(-1, aBuf);
		str_format(aBuf, sizeof(aBuf), "set_team %d -1 %d", SpectateID, g_Config.m_SvVoteSpectateRejoindelay);
		pSelf->Console()->ExecuteLine(aBuf);
	}
}

void CGameContext::ConClearVotes(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;

	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", "cleared votes");
	CNetMsg_Sv_VoteClearOptions VoteClearOptionsMsg;
	pSelf->Server()->SendPackMsg(&VoteClearOptionsMsg, MSGFLAG_VITAL, -1);
	pSelf->m_pVoteOptionHeap->Reset();
	pSelf->m_pVoteOptionFirst = 0;
	pSelf->m_pVoteOptionLast = 0;
	pSelf->m_NumVoteOptions = 0;

	// reset sending of vote options
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(pSelf->m_apPlayers[i])
			pSelf->m_apPlayers[i]->m_SendVoteIndex = 0;
	}
}

void CGameContext::ConVote(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;

	// check if there is a vote running
	if(!pSelf->m_VoteCloseTime)
		return;

	if(str_comp_nocase(pResult->GetString(0), "yes") == 0)
		pSelf->m_VoteEnforce = CGameContext::VOTE_ENFORCE_YES_ADMIN;
	else if(str_comp_nocase(pResult->GetString(0), "no") == 0)
		pSelf->m_VoteEnforce = CGameContext::VOTE_ENFORCE_NO_ADMIN;
	pSelf->m_VoteEnforcer = pResult->m_ClientID;
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "moderator forced vote %s", pResult->GetString(0));
	pSelf->SendChatTarget(-1, aBuf);
	str_format(aBuf, sizeof(aBuf), "forcing vote %s", pResult->GetString(0));
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
}

void CGameContext::ConchainSpecialMotdupdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->NumArguments())
	{
		CNetMsg_Sv_Motd Msg;
		Msg.m_pMessage = g_Config.m_SvMotd;
		CGameContext *pSelf = (CGameContext *)pUserData;
		for(int i = 0; i < MAX_CLIENTS; ++i)
			if(pSelf->m_apPlayers[i])
				pSelf->Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, i);
	}
}

void CGameContext::OnConsoleInit()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_pServer = Kernel()->RequestInterface<IServer>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();

	m_ChatPrintCBIndex = Console()->RegisterPrintCallback(0, SendChatResponse, this);

	Console()->Register("tune", "si", CFGFLAG_SERVER|CFGFLAG_GAME, ConTuneParam, this, "Tune variable to value");
	Console()->Register("tune_reset", "", CFGFLAG_SERVER, ConTuneReset, this, "Reset tuning");
	Console()->Register("tune_dump", "", CFGFLAG_SERVER, ConTuneDump, this, "Dump tuning");
	Console()->Register("tune_zone", "isi", CFGFLAG_SERVER|CFGFLAG_GAME, ConTuneZone, this, "Tune in zone a variable to value");
	Console()->Register("tune_zone_dump", "i", CFGFLAG_SERVER, ConTuneDumpZone, this, "Dump zone tuning in zone x");
	Console()->Register("tune_zone_reset", "?i", CFGFLAG_SERVER, ConTuneResetZone, this, "reset zone tuning in zone x or in all zones");
	Console()->Register("tune_zone_enter", "is", CFGFLAG_SERVER|CFGFLAG_GAME, ConTuneSetZoneMsgEnter, this, "which message to display on zone enter; use 0 for normal area");
	Console()->Register("tune_zone_leave", "is", CFGFLAG_SERVER|CFGFLAG_GAME, ConTuneSetZoneMsgLeave, this, "which message to display on zone leave; use 0 for normal area");
	Console()->Register("switch_open", "i", CFGFLAG_SERVER|CFGFLAG_GAME, ConSwitchOpen, this, "Whether a switch is open by default (otherwise closed)");
	Console()->Register("pause_game", "", CFGFLAG_SERVER, ConPause, this, "Pause/unpause game");
	Console()->Register("change_map", "?r", CFGFLAG_SERVER|CFGFLAG_STORE, ConChangeMap, this, "Change map");
	Console()->Register("random_map", "?i", CFGFLAG_SERVER, ConRandomMap, this, "Random map");
	Console()->Register("random_unfinished_map", "?i", CFGFLAG_SERVER, ConRandomUnfinishedMap, this, "Random unfinished map");
	Console()->Register("restart", "?i", CFGFLAG_SERVER|CFGFLAG_STORE, ConRestart, this, "Restart in x seconds (0 = abort)");
	Console()->Register("broadcast", "r", CFGFLAG_SERVER, ConBroadcast, this, "Broadcast message");
	Console()->Register("say", "r", CFGFLAG_SERVER, ConSay, this, "Say in chat");
	Console()->Register("set_team", "ii?i", CFGFLAG_SERVER, ConSetTeam, this, "Set team of player to team");
	Console()->Register("set_team_all", "i", CFGFLAG_SERVER, ConSetTeamAll, this, "Set team of all players to team");
	//Console()->Register("swap_teams", "", CFGFLAG_SERVER, ConSwapTeams, this, "Swap the current teams");
	//Console()->Register("shuffle_teams", "", CFGFLAG_SERVER, ConShuffleTeams, this, "Shuffle the current teams");
	//Console()->Register("lock_teams", "", CFGFLAG_SERVER, ConLockTeams, this, "Lock/unlock teams");

	Console()->Register("add_vote", "sr", CFGFLAG_SERVER, ConAddVote, this, "Add a voting option");
	Console()->Register("remove_vote", "s", CFGFLAG_SERVER, ConRemoveVote, this, "remove a voting option");
	Console()->Register("force_vote", "ss?r", CFGFLAG_SERVER, ConForceVote, this, "Force a voting option");
	Console()->Register("clear_votes", "", CFGFLAG_SERVER, ConClearVotes, this, "Clears the voting options");
	Console()->Register("vote", "r", CFGFLAG_SERVER, ConVote, this, "Force a vote to yes/no");

	Console()->Chain("sv_motd", ConchainSpecialMotdupdate, this);

#define CONSOLE_COMMAND(name, params, flags, callback, userdata, help) m_pConsole->Register(name, params, flags, callback, userdata, help);
#include "game/ddracecommands.h"
#define CHAT_COMMAND(name, params, flags, callback, userdata, help, ddpp_al) m_pConsole->Register(name, params, flags, callback, userdata, help, ddpp_al);
#include "ddracechat.h"
}

void CGameContext::OnInit(/*class IKernel *pKernel*/)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	// ChillerDragon konst constructor
	m_Database->CreateDatabase();
	LoadSinglePlayer();
	//Friends_counter = 0;
	m_vDropLimit.resize(2);
	m_BalanceID1 = -1;
	m_BalanceID2 = -1;
	m_survivalgamestate = 0;
	m_survival_game_countdown = 0;
	m_BlockWaveGameState = 0;
	m_insta_survival_gamestate = 0;
	m_CucumberShareValue = 10;
	m_BombTick = g_Config.m_SvBombTicks;
	m_BombStartCountDown = g_Config.m_SvBombStartDelay;
	str_copy(m_aAllowedCharSet, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.:+@-_", sizeof(m_aAllowedCharSet));
	str_copy(m_aLastSurvivalWinnerName, "", sizeof(m_aLastSurvivalWinnerName));

	m_pServer = Kernel()->RequestInterface<IServer>();
	m_pConsole = Kernel()->RequestInterface<IConsole>();
	m_World.SetGameServer(this);
	m_Events.SetGameServer(this);

	DeleteTempfile();

	//if(!data) // only load once
		//data = load_data_from_memory(internal_data);

	for(int i = 0; i < NUM_NETOBJTYPES; i++)
		Server()->SnapSetStaticsize(i, m_NetObjHandler.GetObjSize(i));

	m_Layers.Init(Kernel());
	m_Collision.Init(&m_Layers);

	// reset everything here
	//world = new GAMEWORLD;
	//players = new CPlayer[MAX_CLIENTS];

	// Reset Tunezones
	CTuningParams TuningParams;
	for (int i = 0; i < NUM_TUNINGZONES; i++)
	{
		//-- start comment for m_IsVanillaWeapons --
		//TuningList()[i] = TuningParams;
		//TuningList()[i].Set("gun_curvature", 0.0f);
		//TuningList()[i].Set("gun_speed", 1400.0f);
		//TuningList()[i].Set("shotgun_curvature", 0.0f);
		//TuningList()[i].Set("shotgun_speed", 500.0f);
		//TuningList()[i].Set("shotgun_speeddiff", 0.0f);
		//-- end comment for m_IsVanillaWeapons --

		//-- start add code for m_IsVanillaWeapons --
		TuningList()[i] = TuningParams;
		TuningList()[i].Set("gun_curvature", 0.0f);
		TuningList()[i].Set("gun_speed", 1400.0f);
		Tuning()->Set("shotgun_speed", 2750.00f);
		Tuning()->Set("shotgun_speeddiff", 0.80f);
		Tuning()->Set("shotgun_curvature", 7.00f);
		Tuning()->Set("shotgun_lifetime", 0.20f);
		//-- end add code for m_IsVanillaWeapons --
	}

	for (int i = 0; i < NUM_TUNINGZONES; i++) // decided to send no text on changing Tunezones for now
	{
		str_format(m_ZoneEnterMsg[i], sizeof(m_ZoneEnterMsg[i]), "", i);
		str_format(m_ZoneLeaveMsg[i], sizeof(m_ZoneLeaveMsg[i]), "", i);
	}

	// Reset Tuning
	if(g_Config.m_SvTuneReset)
	{
		ResetTuning();
	}
	else
	{
		//-- start2 comment for m_IsVanillaWeapons --
		//Tuning()->Set("gun_speed", 1400.0f);
		//Tuning()->Set("gun_curvature", 0.0f);
		//Tuning()->Set("shotgun_speed", 500.0f);
		//Tuning()->Set("shotgun_speeddiff", 0.0f);
		//Tuning()->Set("shotgun_curvature", 0.0f);
		//-- end2 comment for m_IsVanillaWeapons --

		//-- start2 add code for m_IsVanillaWeapons --
		Tuning()->Set("gun_speed", 1400.0f);
		Tuning()->Set("gun_curvature", 0.0f);
		//Shotgun tuning by chiller
		Tuning()->Set("shotgun_speed", 2750.00f);
		Tuning()->Set("shotgun_speeddiff", 0.80f);
		Tuning()->Set("shotgun_curvature", 7.00f);
		Tuning()->Set("shotgun_lifetime", 0.20f);
		//-- end2 add code for m_IsVanillaWeapons --
	}

	if(g_Config.m_SvDDRaceTuneReset)
	{
		g_Config.m_SvHit = 1;
		g_Config.m_SvEndlessDrag = 0;
		g_Config.m_SvOldLaser = 0;
		g_Config.m_SvOldTeleportHook = 0;
		g_Config.m_SvOldTeleportWeapons = 0;
		g_Config.m_SvTeleportHoldHook = 0;
		g_Config.m_SvTeam = 1;
		g_Config.m_SvShowOthersDefault = 0;

		if(Collision()->m_NumSwitchers > 0)
			for (int i = 0; i < Collision()->m_NumSwitchers+1; ++i)
				Collision()->m_pSwitchers[i].m_Initial = true;
	}

	Console()->ExecuteFile(g_Config.m_SvResetFile);

	LoadMapSettings();

/*	// select gametype
	if(str_comp(g_Config.m_SvGametype, "mod") == 0)
		m_pController = new CGameControllerMOD(this);
	else if(str_comp(g_Config.m_SvGametype, "ctf") == 0)
		m_pController = new CGameControllerCTF(this);
	else if(str_comp(g_Config.m_SvGametype, "tdm") == 0)
		m_pController = new CGameControllerTDM(this);
	else
		m_pController = new CGameControllerDM(this);*/
	m_pController = new CGameControllerDDRace(this);
	((CGameControllerDDRace*)m_pController)->m_Teams.Reset();

	if(g_Config.m_SvSoloServer)
	{
		g_Config.m_SvTeam = 3;
		g_Config.m_SvShowOthersDefault = 1;

		Tuning()->Set("player_collision", 0);
		Tuning()->Set("player_hooking", 0);

		for (int i = 0; i < NUM_TUNINGZONES; i++)
		{
			TuningList()[i].Set("player_collision", 0);
			TuningList()[i].Set("player_hooking", 0);
		}
	}

	// delete old score object
	if(m_pScore)
		delete m_pScore;

	// create score object (add sql later)
#if defined(CONF_SQL)
	if(g_Config.m_SvUseSQL)
		m_pScore = new CSqlScore(this);
	else
#endif
	m_pScore = new CFileScore(this);
	// setup core world
	//for(int i = 0; i < MAX_CLIENTS; i++)
	//	game.players[i].core.world = &game.world.core;

	// create all entities from the game layer
	CMapItemLayerTilemap *pTileMap = m_Layers.GameLayer();
	CTile *pTiles = (CTile *)Kernel()->RequestInterface<IMap>()->GetData(pTileMap->m_Data);




	/*
	num_spawn_points[0] = 0;
	num_spawn_points[1] = 0;
	num_spawn_points[2] = 0;
	*/

	CTile *pFront = 0;
	CSwitchTile *pSwitch = 0;
	if(m_Layers.FrontLayer())
		pFront = (CTile *)Kernel()->RequestInterface<IMap>()->GetData(m_Layers.FrontLayer()->m_Front);
	if(m_Layers.SwitchLayer())
		pSwitch = (CSwitchTile *)Kernel()->RequestInterface<IMap>()->GetData(m_Layers.SwitchLayer()->m_Switch);

	int ShopTiles = 0;

	for(int y = 0; y < pTileMap->m_Height; y++)
	{
		for(int x = 0; x < pTileMap->m_Width; x++)
		{
			int Index = pTiles[y*pTileMap->m_Width+x].m_Index;

			if(Index == TILE_OLDLASER)
			{
				g_Config.m_SvOldLaser = 1;
				dbg_msg("Game Layer", "Found Old Laser Tile");
			}
			else if(Index == TILE_NPC)
			{
				m_Tuning.Set("player_collision", 0);
				dbg_msg("Game Layer", "Found No Collision Tile");
			}
			else if(Index == TILE_EHOOK)
			{
				g_Config.m_SvEndlessDrag = 1;
				dbg_msg("Game Layer", "Found No Unlimited hook time Tile");
			}
			else if(Index == TILE_NOHIT)
			{
				g_Config.m_SvHit = 0;
				dbg_msg("Game Layer", "Found No Weapons Hitting others Tile");
			}
			else if(Index == TILE_NPH)
			{
				m_Tuning.Set("player_hooking", 0);
				dbg_msg("Game Layer", "Found No Player Hooking Tile");
			}
			else if (Index == TILE_SHOP_SPAWN)
			{
				m_ShopBotTileExists = true;
				dbg_msg("Game Layer", "Found Shop Spawn Tile");
			}
			else if (Index == TILE_SHOP)
			{
				m_ShopBotTileExists = true;
				ShopTiles++;
			}

			if(Index >= ENTITY_OFFSET)
			{
				vec2 Pos(x*32.0f+16.0f, y*32.0f+16.0f);
				//m_pController->OnEntity(Index-ENTITY_OFFSET, Pos);
				((CGameControllerDDRace*)m_pController)->OnEntity(Index-ENTITY_OFFSET, Pos);
				m_pController->OnEntity(Index - ENTITY_OFFSET, Pos, LAYER_GAME, pTiles[y * pTileMap->m_Width + x].m_Flags);
			}

			if(pFront)
			{
				Index = pFront[y * pTileMap->m_Width + x].m_Index;
				if(Index == TILE_OLDLASER)
				{
					g_Config.m_SvOldLaser = 1;
					dbg_msg("Front Layer", "Found Old Laser Tile");
				}
				else if(Index == TILE_NPC)
				{
					m_Tuning.Set("player_collision", 0);
					dbg_msg("Front Layer", "Found No Collision Tile");
				}
				else if(Index == TILE_EHOOK)
				{
					g_Config.m_SvEndlessDrag = 1;
					dbg_msg("Front Layer", "Found No Unlimited hook time Tile");
				}
				else if(Index == TILE_NOHIT)
				{
					g_Config.m_SvHit = 0;
					dbg_msg("Front Layer", "Found No Weapons Hitting others Tile");
				}
				else if(Index == TILE_NPH)
				{
					m_Tuning.Set("player_hooking", 0);
					dbg_msg("Front Layer", "Found No Player Hooking Tile");
				}
				else if(Index == TILE_JAIL)
				{
					CJail Jail;
					Jail.m_Center = vec2(x,y);
					dbg_msg("game layer", "got Jail tile at (%.2f|%.2f)", Jail.m_Center.x, Jail.m_Center.y);
					m_Jail.push_back(Jail);
				}
				else if(Index == TILE_JAILRELEASE) 
				{
					CJailrelease Jailrelease;
					Jailrelease.m_Center = vec2(x,y);
					dbg_msg("game layer", "got Jailrelease tile at (%.2f|%.2f)", Jailrelease.m_Center.x, Jailrelease.m_Center.y);
					m_Jailrelease.push_back(Jailrelease);
				}
				else if (Index == TILE_BALANCE_BATTLE_1)
				{
					CBalanceBattleTile1 Balancebattle;
					Balancebattle.m_Center = vec2(x,y);
					dbg_msg("game layer", "got balancebattle1 tile at (%.2f|%.2f)", Balancebattle.m_Center.x, Balancebattle.m_Center.y);
					m_BalanceBattleTile1.push_back(Balancebattle);
				}
				else if (Index == TILE_BALANCE_BATTLE_2)
				{
					CBalanceBattleTile2 Balancebattle;
					Balancebattle.m_Center = vec2(x, y);
					dbg_msg("game layer", "got balancebattle2 tile at (%.2f|%.2f)", Balancebattle.m_Center.x, Balancebattle.m_Center.y);
					m_BalanceBattleTile2.push_back(Balancebattle);
				}
				else if (Index == TILE_SURVIVAL_LOBBY)
				{
					CSurvivalLobbyTile Survivallobby;
					Survivallobby.m_Center = vec2(x, y);
					dbg_msg("game layer", "got survival lobby tile at (%.2f|%.2f)", Survivallobby.m_Center.x, Survivallobby.m_Center.y);
					m_SurvivalLobby.push_back(Survivallobby);
				}
				else if (Index == TILE_SURVIVAL_SPAWN)
				{
					CSurvivalSpawnTile Survivalspawn;
					Survivalspawn.m_Center = vec2(x, y);
					dbg_msg("game layer", "got survival spawn tile at (%.2f|%.2f)", Survivalspawn.m_Center.x, Survivalspawn.m_Center.y);
					m_SurvivalSpawn.push_back(Survivalspawn);
				}
				else if (Index == TILE_SURVIVAL_DEATHMATCH)
				{
					CSurvivalDeathmatchTile Survivaldeathmatch;
					Survivaldeathmatch.m_Center = vec2(x, y);
					dbg_msg("game layer", "got survival deathmatch tile at (%.2f|%.2f)", Survivaldeathmatch.m_Center.x, Survivaldeathmatch.m_Center.y);
					m_SurvivalDeathmatch.push_back(Survivaldeathmatch);
				}
				else if (Index == TILE_BLOCKWAVE_BOT)
				{
					CBlockWaveBotTile BlockWaveBot;
					BlockWaveBot.m_Center = vec2(x, y);
					dbg_msg("game layer", "got blockwave bot spawn tile at (%.2f|%.2f)", BlockWaveBot.m_Center.x, BlockWaveBot.m_Center.y);
					m_BlockWaveBot.push_back(BlockWaveBot);
				}
				else if (Index == TILE_BLOCKWAVE_HUMAN)
				{
					CBlockWaveHumanTile BlockWaveHuman;
					BlockWaveHuman.m_Center = vec2(x, y);
					dbg_msg("game layer", "got blockwave Human spawn tile at (%.2f|%.2f)", BlockWaveHuman.m_Center.x, BlockWaveHuman.m_Center.y);
					m_BlockWaveHuman.push_back(BlockWaveHuman);
				}
				else if (Index == TILE_FNG_SCORE)
				{
					CFngScore FngScore;
					FngScore.m_Center = vec2(x, y);
					dbg_msg("game layer", "got fng score tile at (%.2f|%.2f)", FngScore.m_Center.x, FngScore.m_Center.y);
					m_FngScore.push_back(FngScore);
				}
				else if (Index == TILE_BLOCK_TOURNA_SPAWN)
				{
					CBlockTournaSpawn BlockTournaSpawn;
					BlockTournaSpawn.m_Center = vec2(x, y);
					dbg_msg("game layer", "got fng score tile at (%.2f|%.2f)", BlockTournaSpawn.m_Center.x, BlockTournaSpawn.m_Center.y);
					m_BlockTournaSpawn.push_back(BlockTournaSpawn);
				}
				else if (Index == TILE_PVP_ARENA_SPAWN)
				{
					CPVPArenaSpawn PVPArenaSpawn;
					PVPArenaSpawn.m_Center = vec2(x, y);
					dbg_msg("game layer", "got pvp arena spawn tile at (%.2f|%.2f)", PVPArenaSpawn.m_Center.x, PVPArenaSpawn.m_Center.y);
					m_PVPArenaSpawn.push_back(PVPArenaSpawn);
				}
				else if (Index == TILE_VANILLA_MODE)
				{
					CVanillaMode VanillaMode;
					VanillaMode.m_Center = vec2(x, y);
					dbg_msg("game layer", "got vanilla mode tile at (%.2f|%.2f)", VanillaMode.m_Center.x, VanillaMode.m_Center.y);
					m_VanillaMode.push_back(VanillaMode);
				}
				else if (Index == TILE_DDRACE_MODE)
				{
					CDDraceMode DDraceMode;
					DDraceMode.m_Center = vec2(x, y);
					dbg_msg("game layer", "got ddrace mode tile at (%.2f|%.2f)", DDraceMode.m_Center.x, DDraceMode.m_Center.y);
					m_DDraceMode.push_back(DDraceMode);
				}
				else if (Index == TILE_BOTSPAWN_1)
				{
					CBotSpawn1 BotSpawn1;
					BotSpawn1.m_Center = vec2(x, y);
					dbg_msg("game layer", "got botspawn1 tile at (%.2f|%.2f)", BotSpawn1.m_Center.x, BotSpawn1.m_Center.y);
					m_BotSpawn1.push_back(BotSpawn1);
				}
				else if (Index == TILE_BOTSPAWN_2)
				{
					CBotSpawn2 BotSpawn2;
					BotSpawn2.m_Center = vec2(x, y);
					dbg_msg("game layer", "got botspawn2 tile at (%.2f|%.2f)", BotSpawn2.m_Center.x, BotSpawn2.m_Center.y);
					m_BotSpawn2.push_back(BotSpawn2);
				}
				else if (Index == TILE_BOTSPAWN_3)
				{
					CBotSpawn3 BotSpawn3;
					BotSpawn3.m_Center = vec2(x, y);
					dbg_msg("game layer", "got botspawn3 tile at (%.2f|%.2f)", BotSpawn3.m_Center.x, BotSpawn3.m_Center.y);
					m_BotSpawn3.push_back(BotSpawn3);
				}
				else if (Index == TILE_BOTSPAWN_4)
				{
					CBotSpawn4 BotSpawn4;
					BotSpawn4.m_Center = vec2(x, y);
					dbg_msg("game layer", "got botspawn4 tile at (%.2f|%.2f)", BotSpawn4.m_Center.x, BotSpawn4.m_Center.y);
					m_BotSpawn4.push_back(BotSpawn4);
				}
				else if (Index == TILE_NO_HAMMER)
				{
					CNoHammer NoHammer;
					NoHammer.m_Center = vec2(x, y);
					dbg_msg("game layer", "got no hammer tile at (%.2f|%.2f)", NoHammer.m_Center.x, NoHammer.m_Center.y);
					m_NoHammer.push_back(NoHammer);
				}
				else if (Index == TILE_BLOCK_DM_A1)
				{
					CBlockDMA1 BlockDMA1;
					BlockDMA1.m_Center = vec2(x, y);
					dbg_msg("game layer", "got block deathmatch(1) tile at (%.2f|%.2f)", BlockDMA1.m_Center.x, BlockDMA1.m_Center.y);
					m_BlockDMA1.push_back(BlockDMA1);
				}
				else if (Index == TILE_BLOCK_DM_A2)
				{
					CBlockDMA2 BlockDMA2;
					BlockDMA2.m_Center = vec2(x, y);
					dbg_msg("game layer", "got block deathmatch(2) tile at (%.2f|%.2f)", BlockDMA2.m_Center.x, BlockDMA2.m_Center.y);
					m_BlockDMA2.push_back(BlockDMA2);
				}
				if(Index >= ENTITY_OFFSET)
				{
					vec2 Pos(x*32.0f+16.0f, y*32.0f+16.0f);
					m_pController->OnEntity(Index-ENTITY_OFFSET, Pos, LAYER_FRONT, pFront[y*pTileMap->m_Width+x].m_Flags);
				}
			}
			if(pSwitch)
			{
				Index = pSwitch[y*pTileMap->m_Width + x].m_Type;
				// TODO: Add off by default door here
				// if (Index == TILE_DOOR_OFF)
				if(Index >= ENTITY_OFFSET)
				{
					vec2 Pos(x*32.0f+16.0f, y*32.0f+16.0f);
					m_pController->OnEntity(Index-ENTITY_OFFSET, Pos, LAYER_SWITCH, pSwitch[y*pTileMap->m_Width+x].m_Flags, pSwitch[y*pTileMap->m_Width+x].m_Number);
				}
			}
		}
	}
	dbg_msg("Game Layer", "Found Shop Tiles (%d)", ShopTiles);


	//game.world.insert_entity(game.Controller);


	//ChillerDragon
	//dummy_init
	if (g_Config.m_SvBasicDummys)
	{
		CreateBasicDummys();
	}

#ifdef CONF_DEBUG
	if(g_Config.m_DbgDummies)
	{
		for(int i = 0; i < g_Config.m_DbgDummies ; i++)
		{
			OnClientConnected(MAX_CLIENTS-i-1);
		}
	}
#endif
}

void CGameContext::DeleteTempfile()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if(m_aDeleteTempfile[0] != 0)
	{
		IStorage *pStorage = Kernel()->RequestInterface<IStorage>();
		pStorage->RemoveFile(m_aDeleteTempfile, IStorage::TYPE_SAVE);
		m_aDeleteTempfile[0] = 0;
	}
}

void CGameContext::OnMapChange(char *pNewMapName, int MapNameSize)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	IStorage *pStorage = Kernel()->RequestInterface<IStorage>();

	char aConfig[128];
	char aTemp[128];
	str_format(aConfig, sizeof(aConfig), "maps/%s.cfg", g_Config.m_SvMap);
	str_format(aTemp, sizeof(aTemp), "%s.temp.%d", pNewMapName, pid());

	IOHANDLE File = pStorage->OpenFile(aConfig, IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
	{
		// No map-specific config, just return.
		return;
	}
	CLineReader LineReader;
	LineReader.Init(File);

	array<char *> aLines;
	char *pLine;
	int TotalLength = 0;
	while((pLine = LineReader.Get()))
	{
		int Length = str_length(pLine) + 1;
		char *pCopy = (char *)mem_alloc(Length, 1);
		mem_copy(pCopy, pLine, Length);
		aLines.add(pCopy);
		TotalLength += Length;
	}

	char *pSettings = (char *)mem_alloc(TotalLength, 1);
	int Offset = 0;
	for(int i = 0; i < aLines.size(); i++)
	{
		int Length = str_length(aLines[i]) + 1;
		mem_copy(pSettings + Offset, aLines[i], Length);
		Offset += Length;
		mem_free(aLines[i]);
	}

	CDataFileReader Reader;
	Reader.Open(pStorage, pNewMapName, IStorage::TYPE_ALL);

	CDataFileWriter Writer;
	Writer.Init();

	int SettingsIndex = Reader.NumData();
	bool FoundInfo = false;
	for(int i = 0; i < Reader.NumItems(); i++)
	{
		int TypeID;
		int ItemID;
		int *pData = (int *)Reader.GetItem(i, &TypeID, &ItemID);
		// GetItemSize returns item size including header, remove that.
		int Size = Reader.GetItemSize(i) - sizeof(int) * 2;
		CMapItemInfoSettings MapInfo;
		if(TypeID == MAPITEMTYPE_INFO && ItemID == 0)
		{
			FoundInfo = true;
			CMapItemInfoSettings *pInfo = (CMapItemInfoSettings *)pData;
			if(Size >= (int)sizeof(CMapItemInfoSettings))
			{
				if(pInfo->m_Settings > -1)
				{
					SettingsIndex = pInfo->m_Settings;
					char *pMapSettings = (char *)Reader.GetData(SettingsIndex);
					int DataSize = Reader.GetUncompressedDataSize(SettingsIndex);
					if(DataSize == TotalLength && mem_comp(pSettings, pMapSettings, DataSize) == 0)
					{
						// Configs coincide, no need to update map.
						return;
					}
					Reader.UnloadData(pInfo->m_Settings);
				}
				else
				{
					MapInfo = *pInfo;
					MapInfo.m_Settings = SettingsIndex;
					pData = (int *)&MapInfo;
					Size = sizeof(MapInfo);
				}
			}
			else
			{
				*(CMapItemInfo *)&MapInfo = *(CMapItemInfo *)pInfo;
				MapInfo.m_Settings = SettingsIndex;
				pData = (int *)&MapInfo;
				Size = sizeof(MapInfo);
			}
		}
		Writer.AddItem(TypeID, ItemID, Size, pData);
	}

	if(!FoundInfo)
	{
		CMapItemInfoSettings Info;
		Info.m_Version = 1;
		Info.m_Author = -1;
		Info.m_MapVersion = -1;
		Info.m_Credits = -1;
		Info.m_License = -1;
		Info.m_Settings = SettingsIndex;
		Writer.AddItem(MAPITEMTYPE_INFO, 0, sizeof(Info), &Info);
	}

	for(int i = 0; i < Reader.NumData() || i == SettingsIndex; i++)
	{
		if(i == SettingsIndex)
		{
			Writer.AddData(TotalLength, pSettings);
			continue;
		}
		unsigned char *pData = (unsigned char *)Reader.GetData(i);
		int Size = Reader.GetUncompressedDataSize(i);
		Writer.AddData(Size, pData);
		Reader.UnloadData(i);
	}

	dbg_msg("mapchange", "imported settings");
	Reader.Close();
	Writer.OpenFile(pStorage, aTemp);
	Writer.Finish();

	str_copy(pNewMapName, aTemp, MapNameSize);
	str_copy(m_aDeleteTempfile, aTemp, sizeof(m_aDeleteTempfile));
}

void CGameContext::OnShutdown()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	DeleteTempfile();
	Console()->ResetServerGameSettings();
	Layers()->Dest();
	Collision()->Dest();
	delete m_pController;
	m_pController = 0;
	Clear();
}

void CGameContext::LoadMapSettings()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	IMap *pMap = Kernel()->RequestInterface<IMap>();
	int Start, Num;
	pMap->GetType(MAPITEMTYPE_INFO, &Start, &Num);
	for(int i = Start; i < Start + Num; i++)
	{
		int ItemID;
		CMapItemInfoSettings *pItem = (CMapItemInfoSettings *)pMap->GetItem(i, 0, &ItemID);
		int ItemSize = pMap->GetItemSize(i) - 8;
		if(!pItem || ItemID != 0)
			continue;

		if(ItemSize < (int)sizeof(CMapItemInfoSettings))
			break;
		if(!(pItem->m_Settings > -1))
			break;

		int Size = pMap->GetUncompressedDataSize(pItem->m_Settings);
		char *pSettings = (char *)pMap->GetData(pItem->m_Settings);
		char *pNext = pSettings;
		while(pNext < pSettings + Size)
		{
			int StrSize = str_length(pNext) + 1;
			Console()->ExecuteLine(pNext, IConsole::CLIENT_ID_GAME);
			pNext += StrSize;
		}
		pMap->UnloadData(pItem->m_Settings);
		break;
	}

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "maps/%s.map.cfg", g_Config.m_SvMap);
	Console()->ExecuteFile(aBuf, IConsole::CLIENT_ID_NO_GAME);
}

void CGameContext::OnSnap(int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// add tuning to demo
	CTuningParams StandardTuning;
	if(ClientID == -1 && Server()->DemoRecorder_IsRecording() && mem_comp(&StandardTuning, &m_Tuning, sizeof(CTuningParams)) != 0)
	{
		CMsgPacker Msg(NETMSGTYPE_SV_TUNEPARAMS);
		int *pParams = (int *)&m_Tuning;
		for(unsigned i = 0; i < sizeof(m_Tuning)/sizeof(int); i++)
			Msg.AddInt(pParams[i]);
		Server()->SendMsg(&Msg, MSGFLAG_RECORD|MSGFLAG_NOSEND, ClientID);
	}

	m_World.Snap(ClientID);
	m_pController->Snap(ClientID);
	m_Events.Snap(ClientID);

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
			m_apPlayers[i]->Snap(ClientID);
	}

	if(ClientID > -1)
		m_apPlayers[ClientID]->FakeSnap(ClientID);

}
void CGameContext::OnPreSnap() {}
void CGameContext::OnPostSnap()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_Events.Clear();
}

bool CGameContext::IsClientReady(int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	return m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_IsReady ? true : false;
}

bool CGameContext::IsClientPlayer(int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	return m_apPlayers[ClientID] && m_apPlayers[ClientID]->GetTeam() == TEAM_SPECTATORS ? false : true;
}

const char *CGameContext::GameType() { return m_pController && m_pController->m_pGameType ? m_pController->m_pGameType : ""; }
const char *CGameContext::Version() { return GAME_VERSION; }
const char *CGameContext::NetVersion() { return GAME_NETVERSION; }

IGameServer *CreateGameServer() { return new CGameContext; }

void CGameContext::SendChatResponseAll(const char *pLine, void *pUser) //TODO: schau das an sieht lustig aus
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUser;

	static volatile int ReentryGuard = 0;
	const char *pLineOrig = pLine;

	if(ReentryGuard)
		return;
	ReentryGuard++;

	if(*pLine == '[')
	do
		pLine++;
	while((pLine - 2 < pLineOrig || *(pLine - 2) != ':') && *pLine != 0);//remove the category (e.g. [Console]: No Such Command)

	pSelf->SendChat(-1, CHAT_ALL, pLine);

	ReentryGuard--;
}

void CGameContext::SendChatResponse(const char *pLine, void *pUser, bool Highlighted)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUser;
	int ClientID = pSelf->m_ChatResponseTargetID;

	if(ClientID < 0 || ClientID >= MAX_CLIENTS)
		return;

	const char *pLineOrig = pLine;

	static volatile int ReentryGuard = 0;

	if(ReentryGuard)
		return;
	ReentryGuard++;

	if(*pLine == '[')
	do
		pLine++;
	while((pLine - 2 < pLineOrig || *(pLine - 2) != ':') && *pLine != 0); // remove the category (e.g. [Console]: No Such Command)

	pSelf->SendChatTarget(ClientID, pLine);

	ReentryGuard--;
}

bool CGameContext::PlayerCollision()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	float Temp;
	m_Tuning.Get("player_collision", &Temp);
	return Temp != 0.0;
}

bool CGameContext::PlayerHooking()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	float Temp;
	m_Tuning.Get("player_hooking", &Temp);
	return Temp != 0.0;
}

float CGameContext::PlayerJetpack()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	float Temp;
	m_Tuning.Get("player_jetpack", &Temp);
	return Temp;
}

void CGameContext::OnSetAuthed(int ClientID, int Level)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CServer* pServ = (CServer*)Server();
	if(m_apPlayers[ClientID])
	{
		m_apPlayers[ClientID]->m_Authed = Level;
		char aBuf[512], aIP[NETADDR_MAXSTRSIZE];
		pServ->GetClientAddr(ClientID, aIP, sizeof(aIP));
		str_format(aBuf, sizeof(aBuf), "ban %s %d Banned by vote", aIP, g_Config.m_SvVoteKickBantime);
		if(!str_comp_nocase(m_aVoteCommand, aBuf) && Level > 0)
		{
			m_VoteEnforce = CGameContext::VOTE_ENFORCE_NO_ADMIN;
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "CGameContext", "Aborted vote by admin login.");
		}
		ShowAdminWelcome(ClientID);
	}
}

void CGameContext::SendRecord(int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CNetMsg_Sv_Record RecordsMsg;
	RecordsMsg.m_PlayerTimeBest = Score()->PlayerData(ClientID)->m_BestTime * 100.0f;
	RecordsMsg.m_ServerTimeBest = m_pController->m_CurrentRecord * 100.0f; //TODO: finish this
	Server()->SendPackMsg(&RecordsMsg, MSGFLAG_VITAL, ClientID);
}

int CGameContext::TradePrepareSell(const char *pToName, int FromID, const char * pItemName, int Price, bool IsPublic)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[FromID];
	if (!pPlayer)
		return -1;

	CCharacter *pChr = GetPlayerChar(FromID);
	if (!pChr)
	{
		SendChatTarget(FromID, "[TRADE] you have to be alive to use this command.");
		return -1;
	}

	char aBuf[256];

	if (pPlayer->m_TradeTick > Server()->Tick())
	{
		int TimeLeft = (pPlayer->m_TradeTick - Server()->Tick()) / Server()->TickSpeed();
		str_format(aBuf, sizeof(aBuf), "[TRADE] delay: %02d:%02d", TimeLeft / 60, TimeLeft % 60);
		SendChatTarget(FromID, aBuf);
		return -1;
	}

	if (pPlayer->m_AccountID <= 0) //LOGGED IN ???
	{
		SendChatTarget(FromID, "[TRADE] you have to be logged in to use this command. Check '/accountinfo'");
		return -1;
	}

	int item = TradeItemToInt(pItemName); // ITEM EXIST ???
	if (item == -1)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "[TRADE] unknown item '%s' check '/trade items' for a full list.", pItemName);
		SendChatTarget(FromID, aBuf);
		return -1;
	}

	if (item == 2 && pPlayer->m_SpawnShotgunActive)				// are items spawn weapons?
	{
		SendChatTarget(FromID, "[TRADE] you can't trade your spawn shotgun.");
		return -1;
	}
	if (item == 3 && pPlayer->m_SpawnGrenadeActive)
	{
		SendChatTarget(FromID, "[TRADE] you can't trade your spawn grenade.");
		return -1;
	}
	if (item == 4 && pPlayer->m_SpawnRifleActive)
	{
		SendChatTarget(FromID, "[TRADE] you can't trade your spawn rifle.");
		return -1;
	}
	if (item == 5 && (pPlayer->m_SpawnShotgunActive || pPlayer->m_SpawnGrenadeActive || pPlayer->m_SpawnRifleActive))
	{
		SendChatTarget(FromID, "[TRADE] you can't trade your spawn weapons.");
		return -1;
	}

	if (item == 2 && pChr->m_aDecreaseAmmo[WEAPON_SHOTGUN])				// do items have infinite ammo? (not a pickep up spawn weapon)
	{
		SendChatTarget(FromID, "[TRADE] you can't trade if your weapon doesn't have infinite bullets.");
		return -1;
	}
	if (item == 3 && pChr->m_aDecreaseAmmo[WEAPON_GRENADE])
	{
		SendChatTarget(FromID, "[TRADE] you can't trade if your weapon doesn't have infinite bullets.");
		return -1;
	}
	if (item == 4 && pChr->m_aDecreaseAmmo[WEAPON_RIFLE])
	{
		SendChatTarget(FromID, "[TRADE] you can't trade if your weapon doesn't have infinite bullets.");
		return -1;
	}
	if (item == 5 && (pChr->m_aDecreaseAmmo[WEAPON_SHOTGUN] || pChr->m_aDecreaseAmmo[WEAPON_GRENADE] || pChr->m_aDecreaseAmmo[WEAPON_RIFLE]))
	{
		SendChatTarget(FromID, "[TRADE] you can't trade if your weapons doesn't have infinite bullets.");
		return -1;
	}


	int HasItem = TradeHasItem(item, FromID); // ITEM OWNED ???
	if (HasItem == -1)
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] you don't own the item [ %s ]", pItemName);
		SendChatTarget(FromID, aBuf);
		return -1;
	}

	if (Price < 1) // TRADE MONEY TOO LOW ???
	{
		SendChatTarget(FromID, "[TRADE] the trade price has to be higher than zer0.");
		return -1;
	}

	if (!IsPublic) // private trade
	{
		return TradeSellCheckUser(pToName, FromID); // DOES THE USER EXIST ??? AND IS HE LOGGED IN ???
	}

	return 1;
}

int CGameContext::TradeSellCheckUser(const char * pToName, int FromID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[128];
	int TradeID = GetCIDByName(pToName);       //USER ONLINE ???
	if (TradeID == -1)
	{
		if (!str_comp_nocase(pToName, ""))
		{
			SendChatTarget(FromID, "[TRADE] Error: Missing username");
			return -1;
		}
		str_format(aBuf, sizeof(aBuf), "[TRADE] User '%s' not online", pToName);
		SendChatTarget(FromID, aBuf);
		return -1;
	}

	if (m_apPlayers[TradeID]->m_AccountID <= 0)    //USER LOGGED IN ???
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] player '%s' is not logged in.", pToName);
		SendChatTarget(FromID, aBuf);
		return -1;
	}
	return TradeID;
}

int CGameContext::TradePrepareBuy(int BuyerID, const char *pSellerName, int ItemID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pBPlayer = m_apPlayers[BuyerID];       // BUYER ONLINE ??
	if (!pBPlayer)
		return -1;

	char aBuf[128];
	int SellerID = GetCIDByName(pSellerName);       // SELLER ONLINE ??
	if (SellerID == -1)
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] User '%s' not online.", pSellerName);
		SendChatTarget(BuyerID, aBuf);
		return -1;
	}

	CPlayer *pSPlayer = m_apPlayers[SellerID];
	if (!pSPlayer)
		return -1;

	CCharacter *pBChr = GetPlayerChar(BuyerID);
	CCharacter *pSChr = GetPlayerChar(SellerID);

	if (pBPlayer->m_AccountID <= 0)					// BUYER LOGGED IN ??
	{
		SendChatTarget(BuyerID, "[TRADE] you have to be logged in to use this command. Check '/accountinfo'");
		return -1;
	}

	if (pSPlayer->m_AccountID <= 0)					// SELLER LOGGED IN ??
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] player '%s' is not logged in.", pSellerName);
		SendChatTarget(BuyerID, aBuf);
		return -1;
	}

	if (!pBChr || !pSChr)							// BOTH ALIVE ??
	{
		SendChatTarget(BuyerID, "[TRADE] both players have to be alive.");
		return -1;
	}

	if (BuyerID == SellerID)						// SAME TEE ??
	{
		SendChatTarget(BuyerID, "[TRADE] you can't trade alone, lol");
		return -1;
	}

	if (pSPlayer->m_TradeMoney > pBPlayer->m_money)	// ENOUGH MONEY ??
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] %d/%d money missing.", pBPlayer->m_money, pSPlayer->m_TradeMoney);
		SendChatTarget(BuyerID, aBuf);
		return -1;
	}

	if (pSPlayer->m_TradeID != -1 &&				// PRIVATE TRADE ??
		pSPlayer->m_TradeID != BuyerID)				// wrong private trade mate
	{
		SendChatTarget(BuyerID, "[TRADE] error, this trade is private.");
		return -1;
	}

	if (pSChr->HasWeapon(ItemID) || (ItemID == 5 && pSChr->HasWeapon(2) && pSChr->HasWeapon(3) && pSChr->HasWeapon(4)))
	{
		//has the weapons
	}
	else
	{
		SendChatTarget(BuyerID, "[TRADE] the seller doesn't own the item right now. try agian later.");
		return -1;
	}

	if (IsMinigame(SellerID))
	{
		SendChatTarget(BuyerID, "[TRADE] trade failed because seller is in jail or minigame.");
		return -1;
	}

	if (IsMinigame(BuyerID))
	{
		SendChatTarget(BuyerID, "[TRADE] trade failed because you are in jail or minigame.");
		return -1;
	}

	if (pSPlayer->m_SpawnShotgunActive && ItemID == 2)
	{
		SendChatTarget(BuyerID, "[TRADE] the wanted weapon is a spawn weapon and can't be bought.");
		return -1;
	}

	if (pSPlayer->m_SpawnGrenadeActive && ItemID == 3)
	{
		SendChatTarget(BuyerID, "[TRADE] the wanted weapon is a spawn weapon and can't be bought.");
		return -1;
	}

	if (pSPlayer->m_SpawnRifleActive && ItemID == 4)
	{
		SendChatTarget(BuyerID, "[TRADE] the wanted weapon is a spawn weapon and can't be bought.");
		return -1;
	}


	if (pSChr->m_aDecreaseAmmo[WEAPON_SHOTGUN] && ItemID == 2)
	{
		SendChatTarget(BuyerID, "[TRADE] the wanted weapon doesn't have infinite bullets and can't be bought.");
		return -1;
	}

	if (pSChr->m_aDecreaseAmmo[WEAPON_GRENADE] && ItemID == 3)
	{
		SendChatTarget(BuyerID, "[TRADE] the wanted weapon doesn't have infinite bullets and can't be bought.");
		return -1;
	}

	if (pSChr->m_aDecreaseAmmo[WEAPON_RIFLE] && ItemID == 4)
	{
		SendChatTarget(BuyerID, "[TRADE] the wanted weapon doesn't have infinite bullets and can't be bought.");
		return -1;
	}

	return 0;
}

/*
int CGameContext::TradeSellCheckItem(const char *pItemName, int FromID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	if (!str_comp_nocase(pItemName, "shotgun"))   // OWN TRADE ITEM ???
	{
		if (pChr->HasWeapon(2))
		{
			item = 2;
		}
		else
		{
			SendChatTarget(FromID, "[TRADE] you don't own this item.");
			return -1;
		}
	}
	else if (!str_comp_nocase(pItemName, "grenade"))
	{
		if (pChr->HasWeapon(3))
		{
			item = 3;
		}
		else
		{
			SendChatTarget(FromID, "[TRADE] you don't own this item.");
			return -1;
		}
	}
	else if (!str_comp_nocase(pItemName, "rifle"))
	{
		if (pChr->HasWeapon(4))
		{
			item = 4;
		}
		else
		{
			SendChatTarget(FromID, "[TRADE] you don't own this item.");
			return -1;
		}
	}
	else if (!str_comp_nocase(pItemName, "all_weapons"))
	{
		if (pChr->HasWeapon(4) && pChr->HasWeapon(3) && pChr->HasWeapon(2))
		{
			item = 5;
		}
		else
		{
			SendChatTarget(FromID, "[TRADE] you don't own this item.");
			return -1;
		}
	}

	if (item == -1)
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] unknown item '%s' check '/trade items' for a full list.", pItemName);
		SendChatTarget(FromID, aBuf);
		return -1;
	}

	return item;
}
*/

int CGameContext::TradeItemToInt(const char * pItemName)
{
	int item = -1;

	if (!str_comp_nocase(pItemName, "shotgun"))
	{
		item = 2;
	}
	else if (!str_comp_nocase(pItemName, "grenade"))
	{
		item = 3;
	}
	else if (!str_comp_nocase(pItemName, "rifle"))
	{
		item = 4;
	}
	else if (!str_comp_nocase(pItemName, "all_weapons"))
	{
		item = 5;
	}
	return item;
}

const char * CGameContext::TradeItemToStr(int ItemID)
{
	if (ItemID == 2)
	{
		return "shotgun";
	}
	else if (ItemID == 3)
	{
		return "grenade";
	}
	else if (ItemID == 4)
	{
		return "rifle";
	}
	else if (ItemID == 5)
	{
		return "all_weapons";
	}
	return "(null)";
}

int CGameContext::TradeHasItem(int ItemID, int ID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[ID];
	if (!pPlayer)
		return -1;

	CCharacter *pChr = GetPlayerChar(ID);
	if (!pChr)
		return -1;

	int item = -1;

	if (ItemID == 2) // shotgun
	{
		if (pChr->HasWeapon(2))
		{
			item = 2;
		}
	}
	else if (ItemID == 3) // grenade
	{
		if (pChr->HasWeapon(3))
		{
			item = 3;
		}
	}
	else if (ItemID == 4) // rifle
	{
		if (pChr->HasWeapon(4))
		{
			item = 4;
		}
	}
	else if (ItemID == 5) // all_weapons
	{
		if (pChr->HasWeapon(4) && pChr->HasWeapon(3) && pChr->HasWeapon(2))
		{
			item = 5;
		}
	}

	return item;
}

int CGameContext::ProcessSpamProtection(int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if(!m_apPlayers[ClientID])
		return 0;
	if(g_Config.m_SvSpamprotection && m_apPlayers[ClientID]->m_LastChat
		&& m_apPlayers[ClientID]->m_LastChat + Server()->TickSpeed() * g_Config.m_SvChatDelay > Server()->Tick())
		return 1;
	else
		m_apPlayers[ClientID]->m_LastChat = Server()->Tick();
	NETADDR Addr;
	Server()->GetClientAddr(ClientID, &Addr);
	int Muted = 0;

	for(int i = 0; i < m_NumMutes && !Muted; i++)
	{
		if(!net_addr_comp(&Addr, &m_aMutes[i].m_Addr))
			Muted = (m_aMutes[i].m_Expire - Server()->Tick()) / Server()->TickSpeed();
	}

	if (Muted > 0)
	{
		char aBuf[128];
		str_format(aBuf, sizeof aBuf, "You are not permitted to talk for the next %d seconds.", Muted);
		SendChatTarget(ClientID, aBuf);
		return 1;
	}

	if ((m_apPlayers[ClientID]->m_ChatScore += g_Config.m_SvChatPenalty) > g_Config.m_SvChatThreshold)
	{
		Mute(0, &Addr, g_Config.m_SvSpamMuteDuration, Server()->ClientName(ClientID));
		m_apPlayers[ClientID]->m_ChatScore = 0;
		return 1;
	}

	return 0;
}

int CGameContext::GetDDRaceTeam(int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameControllerDDRace* pController = (CGameControllerDDRace*)m_pController;
	return pController->m_Teams.m_Core.Team(ClientID);
}

void CGameContext::ResetTuning()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	//-- start comment for m_IsVanillaWeapons --
	//CTuningParams TuningParams;
	//m_Tuning = TuningParams;
	//Tuning()->Set("gun_speed", 1400);
	//Tuning()->Set("gun_curvature", 0);
	//Tuning()->Set("shotgun_speed", 500);
	//Tuning()->Set("shotgun_speeddiff", 0);
	//Tuning()->Set("shotgun_curvature", 0);
	//SendTuningParams(-1);
	//-- end comment for m_IsVanillaWeapons --

	//-- start add code for m_IsVanillaWeapons --
	//CTuningParams TuningParams;
	//m_Tuning = TuningParams;
	//Tuning()->Set("gun_speed", 1400);
	//Tuning()->Set("gun_curvature", 0);
	//Tuning()->Set("shotgun_speed", 2750.00);
	//Tuning()->Set("shotgun_speeddiff", 0.80);
	//Tuning()->Set("shotgun_curvature", 7.00);
	//Tuning()->Set("shotgun_lifetime", 0.14);
	//SendTuningParams(-1);

	//test value copied from vanilla src (New test from 29.05.2017) looks pretty ok
	CTuningParams TuningParams;
	m_Tuning = TuningParams;
	Tuning()->Set("gun_speed", 1400);
	Tuning()->Set("gun_curvature", 0);
	Tuning()->Set("shotgun_speed", 2750.00f);
	Tuning()->Set("shotgun_speeddiff", 0.80f);
	Tuning()->Set("shotgun_curvature", 1.25f);
	Tuning()->Set("shotgun_lifetime", 0.20f);
	SendTuningParams(-1);
	//-- end add code for m_IsVanillaWeapons --
}

bool CheckClientID2(int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	dbg_assert(ClientID >= 0 || ClientID < MAX_CLIENTS,
			"The Client ID is wrong");
	if (ClientID < 0 || ClientID >= MAX_CLIENTS)
		return false;
	return true;
}

void CGameContext::Whisper(int ClientID, char *pStr)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char *pName;
	char *pMessage;
	int Error = 0;

	if(ProcessSpamProtection(ClientID))
		return;

	pStr = str_skip_whitespaces(pStr);

	int Victim;

	// add token
	if(*pStr == '"')
	{
		pStr++;

		pName = pStr; // we might have to process escape data
		while(1)
		{
			if(pStr[0] == '"')
				break;
			else if(pStr[0] == '\\')
			{
				if(pStr[1] == '\\')
					pStr++; // skip due to escape
				else if(pStr[1] == '"')
					pStr++; // skip due to escape
			}
			else if(pStr[0] == 0)
				Error = 1;

			pStr++;
		}

		// write null termination
		*pStr = 0;
		pStr++;

		for(Victim = 0; Victim < MAX_CLIENTS; Victim++)
			if (str_comp(pName, Server()->ClientName(Victim)) == 0)
				break;

	}
	else
	{
		pName = pStr;
		while(1)
		{
			if(pStr[0] == 0)
			{
				Error = 1;
				break;
			}
			if(pStr[0] == ' ')
			{
				pStr[0] = 0;
				for(Victim = 0; Victim < MAX_CLIENTS; Victim++)
					if (str_comp(pName, Server()->ClientName(Victim)) == 0)
						break;

				pStr[0] = ' ';

				if (Victim < MAX_CLIENTS)
					break;
			}
			pStr++;
		}
	}

	if(pStr[0] != ' ')
	{
		Error = 1;
	}

	*pStr = 0;
	pStr++;

	pMessage = pStr;

	char aBuf[256];

	if (Error)
	{
		str_format(aBuf, sizeof(aBuf), "Invalid whisper");
		SendChatTarget(ClientID, aBuf);
		return;
	}

	if (Victim >= MAX_CLIENTS || !CheckClientID2(Victim))
	{
		str_format(aBuf, sizeof(aBuf), "No player with name \"%s\" found", pName);
		SendChatTarget(ClientID, aBuf);
		return;
	}

	WhisperID(ClientID, Victim, pMessage);
}


//TEST AREA START
//TESTAREA51


//DRAGON HUGE NUCLEAR TESTS


//WARININGGG


/*





void CGameContext::Playerinfo(int ClientID, char *pStr)
{
	char *pName;
	char *pMessage;
	int Error = 0;

	if (ProcessSpamProtection(ClientID))
		return;

	pStr = str_skip_whitespaces(pStr);

	int Victim;

	// add token
	if (*pStr == '"')
	{
		pStr++;

		pName = pStr; // we might have to process escape data
		while (1)
		{
			if (pStr[0] == '"')
				break;
			else if (pStr[0] == '\\')
			{
				if (pStr[1] == '\\')
					pStr++; // skip due to escape
				else if (pStr[1] == '"')
					pStr++; // skip due to escape
			}
			else if (pStr[0] == 0)
				Error = 1;

			pStr++;
		}

		// write null termination
		*pStr = 0;
		pStr++;

		for (Victim = 0; Victim < MAX_CLIENTS; Victim++)
			if (str_comp(pName, Server()->ClientName(Victim)) == 0)
				break;

	}
	else
	{
		pName = pStr;
		while (1)
		{
			if (pStr[0] == 0)
			{
				Error = 1;
				break;
			}
			if (pStr[0] == ' ')
			{
				pStr[0] = 0;
				for (Victim = 0; Victim < MAX_CLIENTS; Victim++)
					if (str_comp(pName, Server()->ClientName(Victim)) == 0)
						break;

				pStr[0] = ' ';

				if (Victim < MAX_CLIENTS)
					break;
			}
			pStr++;
		}
	}

	if (pStr[0] != ' ')
	{
		Error = 1;
	}

	*pStr = 0;
	pStr++;

	pMessage = pStr;

	char aBuf[256];

	if (Error)
	{
		str_format(aBuf, sizeof(aBuf), "Invalid whisper");
		SendChatTarget(ClientID, aBuf);
		return;
	}

	if (Victim >= MAX_CLIENTS || !CheckClientID2(Victim))
	{
		str_format(aBuf, sizeof(aBuf), "No player with name \"%s\" found", pName);
		SendChatTarget(ClientID, aBuf);
		return;
	}

	WhisperID(ClientID, Victim, pMessage);
}



*/


//TEST AREA 51

//DRAGON HUGE NUCLEAR TEST AREA

//TESTARE END






void CGameContext::WhisperID(int ClientID, int VictimID, char *pMessage)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	if (!CheckClientID2(ClientID))
		return;

	if (!CheckClientID2(VictimID))
		return;

	if (m_apPlayers[ClientID])
		m_apPlayers[ClientID]->m_LastWhisperTo = VictimID;

	char aBuf[256];

	if (m_apPlayers[ClientID] && m_apPlayers[ClientID]->m_ClientVersion >= VERSION_DDNET_WHISPER)
	{
		CNetMsg_Sv_Chat Msg;
		Msg.m_Team = CHAT_WHISPER_SEND;
		Msg.m_ClientID = VictimID;
		Msg.m_pMessage = pMessage;
		if(g_Config.m_SvDemoChat)
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientID);
		else
			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL|MSGFLAG_NORECORD, ClientID);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "[→ %s] %s", Server()->ClientName(VictimID), pMessage);
		SendChatTarget(ClientID, aBuf);
	}

	if (m_apPlayers[VictimID] && m_apPlayers[VictimID]->m_ClientVersion >= VERSION_DDNET_WHISPER)
	{
		CNetMsg_Sv_Chat Msg2;
		Msg2.m_Team = CHAT_WHISPER_RECV;
		Msg2.m_ClientID = ClientID;
		Msg2.m_pMessage = pMessage;
		if(g_Config.m_SvDemoChat)
			Server()->SendPackMsg(&Msg2, MSGFLAG_VITAL, VictimID);
		else
			Server()->SendPackMsg(&Msg2, MSGFLAG_VITAL|MSGFLAG_NORECORD, VictimID);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "[← %s] %s", Server()->ClientName(ClientID), pMessage);
		SendChatTarget(VictimID, aBuf);
	}

	str_format(aBuf, sizeof(aBuf), "['%s' -> '%s'] %s", Server()->ClientName(ClientID), Server()->ClientName(VictimID), pMessage);
	dbg_msg("whisper", "%s", aBuf);
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && i != VictimID && i != ClientID)
		{
			if (Server()->IsAuthed(i) && m_apPlayers[i]->m_Authed == CServer::AUTHED_ADMIN)
			{
				SendChatTarget(i, aBuf);
			}
		}
	}
}

void CGameContext::Converse(int ClientID, char *pStr)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CPlayer *pPlayer = m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if(ProcessSpamProtection(ClientID))
		return;

	if (pPlayer->m_LastWhisperTo < 0)
		SendChatTarget(ClientID, "You do not have an ongoing conversation. Whisper to someone to start one");
	else
	{
		WhisperID(ClientID, pPlayer->m_LastWhisperTo, pStr);
	}
}

void CGameContext::List(int ClientID, const char* filter)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int total = 0;
	char buf[256];
	int bufcnt = 0;
	if (filter[0])
		str_format(buf, sizeof(buf), "Listing players with \"%s\" in name:", filter);
	else
		str_format(buf, sizeof(buf), "Listing all players:", filter);
	SendChatTarget(ClientID, buf);
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			total++;
			const char* name = Server()->ClientName(i);
			if (str_find_nocase(name, filter) == NULL)
				continue;
			if (bufcnt + str_length(name) + 4 > 256)
			{
				SendChatTarget(ClientID, buf);
				bufcnt = 0;
			}
			if (bufcnt != 0)
			{
				str_format(&buf[bufcnt], sizeof(buf) - bufcnt, ", %s", name);
				bufcnt += 2 + str_length(name);
			}
			else
			{
				str_format(&buf[bufcnt], sizeof(buf) - bufcnt, "%s", name);
				bufcnt += str_length(name);
			}
		}
	}
	if (bufcnt != 0)
		SendChatTarget(ClientID, buf);
	str_format(buf, sizeof(buf), "%d players online", total);
	SendChatTarget(ClientID, buf);
}

void CGameContext::RegisterBanCheck(int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	NETADDR Addr;
	Server()->GetClientAddr(ClientID, &Addr);
	Addr.port = 0;
	char aBuf[128];
	int Found = 0;
	int regs = 0;
	// find a matching ban for this ip, update expiration time if found
	for (int i = 0; i < m_NumRegisterBans; i++)
	{
		if (net_addr_comp(&m_aRegisterBans[i].m_Addr, &Addr) == 0)
		{
			regs = ++m_aRegisterBans[i].m_NumRegisters;
			Found = 1;
		}
	}

	if (!Found) // nothing found so far, find a free slot..
	{
		if (m_NumRegisterBans < MAX_REGISTER_BANS)
		{
			m_aRegisterBans[m_NumRegisterBans].m_Addr = Addr;
			regs = m_aRegisterBans[m_NumRegisterBans].m_NumRegisters = 1;
			m_NumRegisterBans++;
			Found = 1;
		}
	}

	if (regs >= g_Config.m_SvMaxRegisterPerIp)
	{
		RegisterBan(&Addr, 60 * 60 * 12, Server()->ClientName(ClientID));
	}
	if (Found)
	{
		str_format(aBuf, sizeof(aBuf), "ClientID=%d has registered %d/%d accounts.", ClientID, regs, g_Config.m_SvMaxRegisterPerIp);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "accounts", aBuf);
	}
	else // no free slot found
	{
		if (g_Config.m_SvRegisterHumanLevel < 9)
			g_Config.m_SvRegisterHumanLevel++;
		str_format(aBuf, sizeof(aBuf), "ban array is full setting human level to %d", g_Config.m_SvRegisterHumanLevel);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "accounts", aBuf);
	}
}

void CGameContext::RegisterBan(NETADDR *Addr, int Secs, const char *pDisplayName)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[128];
	int Found = 0;
	NETADDR NoPortAddr = *Addr;
	NoPortAddr.port = 0;
	// find a matching ban for this ip, update expiration time if found
	for (int i = 0; i < m_NumRegisterBans; i++)
	{
		if (net_addr_comp(&m_aRegisterBans[i].m_Addr, &NoPortAddr) == 0)
		{
			m_aRegisterBans[i].m_Expire = Server()->Tick()
							+ Secs * Server()->TickSpeed();
			Found = 1;
		}
	}

	if (!Found) // nothing found so far, find a free slot..
	{
		if (m_NumRegisterBans < MAX_REGISTER_BANS)
		{
			m_aRegisterBans[m_NumRegisterBans].m_Addr = NoPortAddr;
			m_aRegisterBans[m_NumRegisterBans].m_Expire = Server()->Tick()
							+ Secs * Server()->TickSpeed();
			m_NumRegisterBans++;
			Found = 1;
		}
	}
	if (Found)
	{
		str_format(aBuf, sizeof aBuf, "'%s' has been banned from account system for %d seconds.",
				pDisplayName, Secs);
		SendChat(-1, CHAT_ALL, aBuf);
	}
	else // no free slot found
	{
		if (g_Config.m_SvRegisterHumanLevel < 9)
			g_Config.m_SvRegisterHumanLevel++;
		str_format(aBuf, sizeof(aBuf), "ban array is full setting human level to %d", g_Config.m_SvRegisterHumanLevel);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "accounts", aBuf);
	}
}

int CGameContext::FindNextBomb()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	//Check who has the furthest distance to all other players (no average middle needed)
	//New version with pythagoras
	int MaxDist = 0;
	int NextBombID = -1;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing)
		{
			int Dist = 0;
			for (int i_comp = 0; i_comp < MAX_CLIENTS; i_comp++)
			{
				if (GetPlayerChar(i_comp) && GetPlayerChar(i_comp)->m_IsBombing)
				{
					int a = GetPlayerChar(i)->m_Pos.x - GetPlayerChar(i_comp)->m_Pos.x;
					int b = GetPlayerChar(i)->m_Pos.y - GetPlayerChar(i_comp)->m_Pos.y;

					//|a| |b|
					a = abs(a);
					b = abs(b); 

					int c = sqrt((double)(a + b)); //pythagoras rocks
					Dist += c; //store all distances to all players
				}
			}
			if (Dist > MaxDist)
			{
				MaxDist = Dist;
				NextBombID = i;
			}
		}
	}
	return NextBombID;
}

int CGameContext::CountBannedBombPlayers()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int BannedPlayers = 0;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_apPlayers[i] && m_apPlayers[i]->m_BombBanTime)
		{
			BannedPlayers++;
		}
	}

	return BannedPlayers;
}

int CGameContext::CountBombPlayers()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	int BombPlayers = 0;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (GetPlayerChar(i))
		{
			if (GetPlayerChar(i)->m_IsBombing)
			{
				BombPlayers++;
			}
		}
	}
	return BombPlayers;
}

int CGameContext::CountReadyBombPlayers()
{
	int RdyPlrs = 0;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing && GetPlayerChar(i)->m_IsBombReady)
		{
			RdyPlrs++;
		}
	}
	return RdyPlrs;
}

void CGameContext::SaveWrongLogin(const char *pLogin)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
		if (!g_Config.m_SvSaveWrongLogin)
			return;

		std::ofstream LoginFile(g_Config.m_SvWrongLoginFile, std::ios::app);
		if (!LoginFile)
		{
			dbg_msg("login_sniff", "ERROR1 writing file '%s'", g_Config.m_SvWrongLoginFile);
			g_Config.m_SvSaveWrongLogin = 0;
			LoginFile.close();
			return;
		}

		if (LoginFile.is_open())
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

bool CGameContext::AdminChatPing(const char * pMsg)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (!g_Config.m_SvMinAdminPing)
		return false;
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!m_apPlayers[i])
			continue;
		if (!m_apPlayers[i]->m_Authed)
			continue;
		if (str_find_nocase(pMsg, Server()->ClientName(i)))
		{
			int len_name = str_length(Server()->ClientName(i));
			int len_msg = str_length(pMsg);
			if (len_msg - len_name - 2 < g_Config.m_SvMinAdminPing)
				return true;
		}
	}
	return false;
}

void CGameContext::SQLaccount(int mode, int ClientID, const char * pUsername, const char * pPassword)
{
	if (mode == SQL_LOGIN)
	{
		char *pQueryBuf = sqlite3_mprintf("SELECT * FROM Accounts WHERE Username='%q' AND Password='%q'", pUsername, pPassword);
		CQueryLogin *pQuery = new CQueryLogin();
		pQuery->m_ClientID = ClientID;
		pQuery->m_pGameServer = this;
		pQuery->Query(m_Database, pQueryBuf);
		sqlite3_free(pQueryBuf);
	}
	else if (mode == SQL_LOGIN_THREADED)
	{
		CPlayer *pPlayer = m_apPlayers[ClientID];
		if (!pPlayer)
			return;
		if (pPlayer->m_LoginData.m_LoginState != CPlayer::LOGIN_OFF)
			return;
		pPlayer->ThreadLoginStart(pUsername, pPassword);
	}
	else if (mode == SQL_REGISTER)
	{
		char time_str[64];
		time_t rawtime;
		struct tm * timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		str_format(time_str, sizeof(time_str), "%d-%d-%d_%d:%d:%d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

		char *pQueryBuf = sqlite3_mprintf("SELECT * FROM Accounts WHERE Username='%q'", pUsername);
		CQueryRegister *pQuery = new CQueryRegister();
		pQuery->m_ClientID = ClientID;
		pQuery->m_pGameServer = this;
		pQuery->m_Name = pUsername;
		pQuery->m_Password = pPassword;
		pQuery->m_Date = time_str;
		pQuery->Query(m_Database, pQueryBuf);
		sqlite3_free(pQueryBuf);
	}
	else if (mode == SQL_CHANGE_PASSWORD)
	{
		char *pQueryBuf = sqlite3_mprintf("SELECT * FROM Accounts WHERE Username='%q' AND Password='%q'", pUsername, pPassword);
		CQueryChangePassword *pQuery = new CQueryChangePassword();
		pQuery->m_ClientID = ClientID;
		pQuery->m_pGameServer = this;
		pQuery->Query(m_Database, pQueryBuf);
		sqlite3_free(pQueryBuf);
	}
	else if (mode == SQL_SET_PASSWORD)
	{
		char *pQueryBuf = sqlite3_mprintf("SELECT * FROM Accounts WHERE Username='%q'", pUsername);
		CQuerySetPassword *pQuery = new CQuerySetPassword();
		pQuery->m_ClientID = ClientID;
		pQuery->m_pGameServer = this;
		pQuery->Query(m_Database, pQueryBuf);
		sqlite3_free(pQueryBuf);
	}
}

void CGameContext::ExecuteSQLf(const char *pSQL, ...)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	va_list ap;
	va_start(ap, pSQL);
	char *pQueryBuf = sqlite3_vmprintf(pSQL, ap);
	va_end(ap);
	CQuery *pQuery = new CQuery();
	pQuery->Query(m_Database, pQueryBuf);
	sqlite3_free(pQueryBuf);
}

void CGameContext::ExecuteSQLvf(int VerboseID, const char *pSQL, ...)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	va_list ap;
	va_start(ap, pSQL);
	char *pQueryBuf = sqlite3_vmprintf(pSQL, ap);
	va_end(ap);
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "[SQL] executing: %s", pQueryBuf);
	SendChatTarget(VerboseID, aBuf);
	CQuerySQLstatus *pQuery;
	pQuery = new CQuerySQLstatus();
	pQuery->m_ClientID = VerboseID;
	pQuery->m_pGameServer = this;
	pQuery->Query(m_Database, pQueryBuf);
	sqlite3_free(pQueryBuf);
}
