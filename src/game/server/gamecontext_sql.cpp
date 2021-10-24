// gamecontext scoped sql ddnet++ methods

#include <engine/shared/config.h>

#include <cstring>

#include "gamecontext.h"

void CQuerySQLstatus::OnData()
{
	int n = Next();
	if(m_ClientID == -1)
		return;
	if(n)
		m_pGameServer->SendChatTarget(m_ClientID, "[SQL] result: got rows.");
	else
		m_pGameServer->SendChatTarget(m_ClientID, "[SQL] result: no rows.");
}

void CQueryRegister::OnData()
{
	if(Next())
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
	struct LoginData *pData = static_cast<struct LoginData*>(pArg);
	CGameContext *pGS = static_cast<CGameContext*>(pData->pGameContext);
	CQueryLogin *pSQL = static_cast<CQueryLogin*>(pData->pSQL);
	CPlayer *pPlayer = static_cast<CPlayer*>(pData->pTmpPlayer);
	int id = pData->id;
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "[THREAD] hello world3 your id=%d", id);
	pGS->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
	pPlayer->SetMoney(420);
}

void CGameContext::Login(void *pArg, int id)
{
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
	CPlayer *pPlayer = m_pGameServer->m_apPlayers[m_ClientID];
	if(!pPlayer)
		return;
	CPlayer::CLoginData *pData = &pPlayer->m_LoginData;
	if(!pData)
		return;
	if(!Next())
	{
		m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Login failed. Wrong password or username.");
		m_pGameServer->SaveWrongLogin(m_pGameServer->m_apPlayers[m_ClientID]->m_aWrongLogin);
		pData->m_LoginState = CPlayer::LOGIN_OFF;
		m_pGameServer->LoginBanCheck(m_ClientID);
		return;
	}
	if(m_pGameServer->CheckAccounts(GetInt(GetID("ID"))))
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
	if(Next())
	{
		if(m_pGameServer->CheckAccounts(GetInt(GetID("ID"))))
		{
			m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] This account is already logged in on this server.");
		}
		else
		{
			if(g_Config.m_SvSpeedLogin)
			{
				if(m_pGameServer->m_apPlayers[m_ClientID])
				{
					m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] speed login success.");
					//m_pGameServer->m_apPlayers[m_ClientID]->ThreadLoginStart(this); //crashes the server still in work
				}
				return;
			}
			if(m_pGameServer->m_apPlayers[m_ClientID])
			{
				//#####################################################
				//       W A R N I N G
				// if you add a new var here
				// make sure to reset it in the Logout(); function
				// src/game/server/player.cpp
				//#####################################################
#if defined(CONF_DEBUG)
				dbg_msg("cBug", "gamecontext.cpp '%s' CID=%d loading data...", m_pGameServer->Server()->ClientName(m_ClientID), m_ClientID);
#endif

				//basic
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccountLoginName, GetText(GetID("Username")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccountLoginName));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccountPassword, GetText(GetID("Password")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccountPassword));
				str_copy(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccountRegDate, GetText(GetID("RegisterDate")), sizeof(m_pGameServer->m_apPlayers[m_ClientID]->m_aAccountRegDate));
				m_pGameServer->m_apPlayers[m_ClientID]->SetAccID(GetInt(GetID("ID")));

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
				m_pGameServer->m_apPlayers[m_ClientID]->SetLevel(GetInt(GetID("Level")));
				m_pGameServer->m_apPlayers[m_ClientID]->SetXP(GetInt64(GetID("Exp")));
				m_pGameServer->m_apPlayers[m_ClientID]->SetMoney(GetInt64(GetID("Money")));
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
				if(g_Config.m_SvInstaScore) //load scoreboard scores
				{
					if(g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2) //gdm & zCatch grenade
					{
						m_pGameServer->m_apPlayers[m_ClientID]->m_Score = GetInt(GetID("GrenadeKills"));
					}
					else if(g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4) // idm & zCatch rifle
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

			if(m_pGameServer->m_apPlayers[m_ClientID]->m_IsAccFrozen)
			{
				m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Login failed.(Account is frozen)");
				m_pGameServer->m_apPlayers[m_ClientID]->Logout();
				return;
			}

			if(GetInt(GetID("IsLoggedIn")) == 1)
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
			if(m_pGameServer->m_apPlayers[m_ClientID]->m_NoboSpawnStop > m_pGameServer->Server()->Tick())
			{
				m_pGameServer->m_apPlayers[m_ClientID]->m_IsNoboSpawn = false;
				m_pGameServer->m_apPlayers[m_ClientID]->m_NoboSpawnStop = 0;
				m_pGameServer->SendChatTarget(m_ClientID, "[NoboSpawn] Real spawn unlocked due to login.");
			}

			//jail
			if(m_pGameServer->m_apPlayers[m_ClientID]->m_JailTime)
			{
				if(m_pGameServer->m_apPlayers[m_ClientID]->GetCharacter())
				{
					vec2 JailPlayerSpawn = m_pGameServer->Collision()->GetRandomTile(TILE_JAIL);

					if(JailPlayerSpawn != vec2(-1, -1))
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
			if(m_pGameServer->m_apPlayers[m_ClientID]->m_aFngConfig[0] == '1') //auto fng join
			{
				if(!g_Config.m_SvAllowInsta)
				{
					m_pGameServer->SendChatTarget(m_ClientID, "[INSTA] fng autojoin failed because fng is deactivated by an admin.");
				}
				else
				{
					m_pGameServer->SendChatTarget(m_ClientID, "[INSTA] you automatically joined an fng game. (use '/fng' to change this setting)");
					m_pGameServer->JoinInstagib(5, true, m_ClientID);
				}
			}
			else if(m_pGameServer->m_apPlayers[m_ClientID]->m_aFngConfig[0] == '2') //auto boomfng join
			{
				if(!g_Config.m_SvAllowInsta)
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
			if(!str_comp(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileEmail, "") && !str_comp(m_pGameServer->m_apPlayers[m_ClientID]->m_ProfileSkype, ""))
			{
				m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] set an '/profile email' or '/profile skype' to restore your password if you forget it.");
			}

			//========================================
			//LEAVE THIS CODE LAST!!!!
			//multiple server account protection stuff
			//========================================
			//works how it should but is possible crashing the server

			m_pGameServer->ExecuteSQLf("UPDATE `Accounts` SET `IsLoggedIn` = '%i', `LastLoginPort` = '%i' WHERE `ID` = '%i'", 1, g_Config.m_SvPort, m_pGameServer->m_apPlayers[m_ClientID]->GetAccID());
		}
	}
	else
	{
		m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Login failed. Wrong password or username.");
		m_pGameServer->SaveWrongLogin(m_pGameServer->m_apPlayers[m_ClientID]->m_aWrongLogin);
		m_pGameServer->LoginBanCheck(m_ClientID);
	}
}

void CQueryChangePassword::OnData()
{
	if(Next())
	{
		if(m_pGameServer->m_apPlayers[m_ClientID])
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
	if(Next())
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

void CGameContext::SQLcleanZombieAccounts(int ClientID)
{
	/*
		support up to 99 999 999 (8 digit long) registered accounts
		if more accounts are registered the system breaks :c

		related issue https://github.com/DDNetPP/DDNetPP/issues/279
	*/
	static const int MAX_SQL_ID_LENGTH = 8;
	char aBuf[128 + (MAX_CLIENTS * (MAX_SQL_ID_LENGTH + 1))];
	bool IsLoggedIns = false;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i])
			continue;
		if(m_apPlayers[i]->IsLoggedIn())
		{
			IsLoggedIns = true;
			break;
		}
	}
	str_format(aBuf, sizeof(aBuf), "UPDATE Accounts SET IsLoggedIn = 0 WHERE LastLoginPort = '%i' ", g_Config.m_SvPort);
	if(IsLoggedIns)
	{
		str_append(aBuf, " AND ID NOT IN (", sizeof(aBuf));
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(!m_apPlayers[i])
				continue;
			if(!m_apPlayers[i]->IsLoggedIn())
				continue;
			char aBufBuf[MAX_SQL_ID_LENGTH + 2]; // max supported id len + comma + nullterm
			str_format(aBufBuf, sizeof(aBufBuf), "%d,", m_apPlayers[i]->GetAccID());
			str_append(aBuf, aBufBuf, sizeof(aBuf));
		}
		aBuf[strlen(aBuf) - 1] = '\0'; // chop of the last comma
		str_append(aBuf, ")", sizeof(aBuf));
	}
	ExecuteSQLvf(ClientID, aBuf);
}

void CGameContext::SQLaccount(int mode, int ClientID, const char *pUsername, const char *pPassword)
{
	if(mode == SQL_LOGIN)
	{
		char *pQueryBuf = sqlite3_mprintf("SELECT * FROM Accounts WHERE Username='%q' AND Password='%q'", pUsername, pPassword);
		CQueryLogin *pQuery = new CQueryLogin();
		pQuery->m_ClientID = ClientID;
		pQuery->m_pGameServer = this;
		pQuery->Query(m_Database, pQueryBuf);
		sqlite3_free(pQueryBuf);
	}
	else if(mode == SQL_LOGIN_THREADED)
	{
		CPlayer *pPlayer = m_apPlayers[ClientID];
		if(!pPlayer)
			return;
		if(pPlayer->m_LoginData.m_LoginState != CPlayer::LOGIN_OFF)
			return;
		pPlayer->ThreadLoginStart(pUsername, pPassword);
	}
	else if(mode == SQL_REGISTER)
	{
		char time_str[64];
		time_t rawtime;
		struct tm *timeinfo;
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
	else if(mode == SQL_CHANGE_PASSWORD)
	{
		char *pQueryBuf = sqlite3_mprintf("SELECT * FROM Accounts WHERE Username='%q' AND Password='%q'", pUsername, pPassword);
		CQueryChangePassword *pQuery = new CQueryChangePassword();
		pQuery->m_ClientID = ClientID;
		pQuery->m_pGameServer = this;
		pQuery->Query(m_Database, pQueryBuf);
		sqlite3_free(pQueryBuf);
	}
	else if(mode == SQL_SET_PASSWORD)
	{
		char *pQueryBuf = sqlite3_mprintf("SELECT * FROM Accounts WHERE Username='%q'", pUsername);
		CQuerySetPassword *pQuery = new CQuerySetPassword();
		pQuery->m_ClientID = ClientID;
		pQuery->m_pGameServer = this;
		pQuery->Query(m_Database, pQueryBuf);
		sqlite3_free(pQueryBuf);
	}
}

void CGameContext::ExecuteSQLBlockingf(const char *pSQL, ...)
{
	va_list ap;
	va_start(ap, pSQL);
	char *pQueryBuf = sqlite3_vmprintf(pSQL, ap);
	va_end(ap);
	CQuery *pQuery = new CQuery();
	pQuery->QueryBlocking(m_Database, pQueryBuf);
	sqlite3_free(pQueryBuf);
	delete pQuery;
	dbg_msg("blocking-sql", "should be last...");
}

void CGameContext::ExecuteSQLf(const char *pSQL, ...)
{
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
	va_list ap;
	va_start(ap, pSQL);
	char *pQueryBuf = sqlite3_vmprintf(pSQL, ap);
	va_end(ap);
	if(VerboseID != -1)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "[SQL] executing: %s", pQueryBuf);
		SendChatTarget(VerboseID, aBuf);
	}
	CQuerySQLstatus *pQuery;
	pQuery = new CQuerySQLstatus();
	pQuery->m_ClientID = VerboseID;
	pQuery->m_pGameServer = this;
	pQuery->Query(m_Database, pQueryBuf);
	sqlite3_free(pQueryBuf);
}
