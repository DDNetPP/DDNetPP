/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#include "gamecontext.h"
#include <engine/shared/config.h>
#include <engine/shared/protocol.h>
#include <engine/server/server.h>
#include <game/server/captcha.h>
#include <game/server/teams.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/version.h>
#include <base/system_ddpp.h>
#include <time.h>          //ChillerDragon
#include <fstream> //ChillerDragon acc sys2
#include <limits> //ChillerDragon acc sys2 get specific line
#include <engine/external/sqlite3/sqlite3.h>
//#include <stdio.h> //strcat
#include <string.h> //strcat
#include <stdio.h> //acc2 to_str()
#include <stdlib.h>  //acc2 to_str()
//#include <string> //acc2 std::to_string
//#include <iostream> //acc2 std::to_string
//#include <sstream> //acc2 std::to_string

#if defined(CONF_SQL)
#include <game/server/score/sql_score.h>
#endif

bool CheckClientID(int ClientID); //TODO: whats this ? xd

//void CGameContext::ConAfk(IConsole::IResult *pResult, void *pUserData)
//{
//	CGameContext *pSelf = (CGameContext *)pUserData;
//
//	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
//	if (!pPlayer)
//		return;
//
//	CCharacter* pChr = pPlayer->GetCharacter();
//	if (!pChr)
//		return;
//
//
//	if (pPlayer->m_Afk)
//	{
//		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "cmd-Info",
//			"You are already Afk.");
//		return;
//	}
//
//	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Info",
//		"You are now Afk. (away from keyboard)");
//
//	//pPlayer->m_Afk = true;
//	pPlayer->m_LastPlaytime = time_get() - time_freq()*g_Config.m_SvMaxAfkVoteTime;
//}

void CGameContext::ConToggleSpawn(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (!pPlayer->m_IsSuperModerator)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission. You are not a VIP+.");
		return;
	}

	pPlayer->m_IsSuperModSpawn ^= true;

	if (pPlayer->m_IsSuperModSpawn)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "VIP+ Spawn activated");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "VIP+ Spawn deactivated");
	}
}

void CGameContext::ConSpawnWeapons(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (!g_Config.m_SvAllowSpawnWeapons)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Spawn weapons are deactivated by an administrator.");
		return;
	}

	if ((!pPlayer->m_SpawnWeaponShotgun) && (!pPlayer->m_SpawnWeaponGrenade) && (!pPlayer->m_SpawnWeaponRifle))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You don't have any spawn weapons.");
		return;
	}

	if (!pPlayer->m_UseSpawnWeapons)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Spawn weapons activated");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Spawn weapons deactivated");
	}

	pPlayer->m_UseSpawnWeapons ^= true;
}

void CGameContext::ConSayServer(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (!pPlayer->m_IsSuperModerator && !pPlayer->m_IsModerator)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[SAY] Missing permission.");
		return;
	}

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "[SERVER] %s", pResult->GetString(0));
	pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
}

void CGameContext::ConPolicehelper(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	char aBuf[128];

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "**** POLICEHELPER ****");
		pSelf->SendChatTarget(pResult->m_ClientID, "Police[2] can add/remove policehelpers with:");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/policehelper add/remove <playername>'.");
		pSelf->SendChatTarget(pResult->m_ClientID, "Police-Bots help policehelpers.");
		pSelf->SendChatTarget(pResult->m_ClientID, "*** Personal Stats ***");
		if (pPlayer->m_PoliceRank > 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Police[2]: true");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Police[2]: false");
		}
		if (pPlayer->m_PoliceHelper)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Policehelper: true");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Policehelper: false");
		}
		return;
	}


	if (pPlayer->m_PoliceRank < 2)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[POLICE] You have to be atleast Police[2] to use this command. Check '/policehelper' for more info.");
		return;
	}
	if (pResult->NumArguments() == 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[POLICE] Missing parameter: <player>");
		return;
	}

	int helperID = pSelf->GetCIDByName(pResult->GetString(1));
	if (helperID == -1)
	{
		str_format(aBuf, sizeof(aBuf), "[POLICE] Player '%s' is not online.", pResult->GetString(1));
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		return;
	}

	char aPara[32];
	str_format(aPara, sizeof(aPara), "%s", pResult->GetString(0));
	if (!str_comp_nocase(aPara, "add"))
	{
		if (pSelf->m_apPlayers[helperID])
		{
			if (pSelf->m_apPlayers[helperID]->m_PoliceHelper)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[POLICE] This player is already a policehelper.");
				return; 
			}

			pSelf->m_apPlayers[helperID]->m_PoliceHelper = true;
			str_format(aBuf, sizeof(aBuf), "[POLICE] '%s' promoted you to policehelper.", pSelf->Server()->ClientName(pResult->m_ClientID));
			pSelf->SendChatTarget(helperID, aBuf);

			str_format(aBuf, sizeof(aBuf), "[POLICE] '%s' is now a policehelper.", pSelf->Server()->ClientName(helperID));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
	}
	else if (!str_comp_nocase(aPara, "remove"))
	{
		if (pSelf->m_apPlayers[helperID])
		{
			if (!pSelf->m_apPlayers[helperID]->m_PoliceHelper)
			{
			pSelf->SendChatTarget(pResult->m_ClientID, "[POLICE] This player is not a policehelper.");
			return;
			}

			pSelf->m_apPlayers[helperID]->m_PoliceHelper = false;
			str_format(aBuf, sizeof(aBuf), "[POLICE] '%s' removed your policehelper rank.", pSelf->Server()->ClientName(pResult->m_ClientID));
			pSelf->SendChatTarget(helperID, aBuf);

			str_format(aBuf, sizeof(aBuf), "[POLICE] '%s' is no longer a policehelper.", pSelf->Server()->ClientName(helperID));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[POLICE] Unknown parameter. Check '/policehelper' for help.");
	}
}

//void CGameContext::ConTaserinfo(IConsole::IResult *pResult, void *pUserData)
//{
//#if defined(CONF_DEBUG)
//#endif
//	CGameContext *pSelf = (CGameContext *)pUserData;
//	if (!CheckClientID(pResult->m_ClientID))
//		return;
//
//	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
//	if (!pPlayer)
//		return;
//
//	CCharacter* pChr = pPlayer->GetCharacter();
//	if (!pChr)
//		return;
//
//
//	if (pPlayer->m_TaserLevel == 0)
//	{
//		pPlayer->m_TaserPrice = 50000;
//	}
//	else if (pPlayer->m_TaserLevel == 1)
//	{
//		pPlayer->m_TaserPrice = 75000;
//	}
//	else if (pPlayer->m_TaserLevel == 2)
//	{
//		pPlayer->m_TaserPrice = 100000;
//	}
//	else if (pPlayer->m_TaserLevel == 3)
//	{
//		pPlayer->m_TaserPrice = 150000;
//	}
//	else if (pPlayer->m_TaserLevel == 4)
//	{
//		pPlayer->m_TaserPrice = 200000;
//	}
//	else if (pPlayer->m_TaserLevel == 5)
//	{
//		pPlayer->m_TaserPrice = 200000;
//	}
//	else if (pPlayer->m_TaserLevel == 6)
//	{
//		pPlayer->m_TaserPrice = 200000;
//	}
//	else
//	{
//		pPlayer->m_TaserPrice = 0;
//	}
//
//
//	char aBuf[256];
//
//
//	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ TASER INFO ~~~");
//	pSelf->SendChatTarget(pResult->m_ClientID, "Police Ranks 3 or higher are allowed to carry a taser.");
//	pSelf->SendChatTarget(pResult->m_ClientID, "Use the taser to fight gangsters.");
//	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ YOUR TASER STATS ~~~");
//	str_format(aBuf, sizeof(aBuf), "TaserLevel: %d/7", pPlayer->m_TaserLevel);
//	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
//	str_format(aBuf, sizeof(aBuf), "Price for the next level: %d", pPlayer->m_TaserPrice);
//	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
//	str_format(aBuf, sizeof(aBuf), "FreezeTime: 0.%d seconds", pPlayer->m_TaserLevel * 5);
//	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
//	str_format(aBuf, sizeof(aBuf), "FailRate: %d%", 0);
//	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
//}

void CGameContext::ConOfferInfo(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	char aBuf[256];


	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ OFFER INFO ~~~");
	pSelf->SendChatTarget(pResult->m_ClientID, "Users can accept offers with '/<extra> <accept>'");
	pSelf->SendChatTarget(pResult->m_ClientID, "VIPs can give all players one rainbow offer.");
	pSelf->SendChatTarget(pResult->m_ClientID, "VIP+ can give all players more rainbow offers and one bloody.");
	//pSelf->SendChatTarget(pResult->m_ClientID, "Admins can give all players much more of everything."); //admins can't do shit lul
	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ YOUR OFFER STATS (Extras) ~~~");
	str_format(aBuf, sizeof(aBuf), "Rainbow: %d", pPlayer->m_rainbow_offer);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Bloody: %d", pPlayer->m_bloody_offer);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Trail: %d", pPlayer->m_trail_offer);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Atom: %d", pPlayer->m_atom_offer);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Spread gun: %d", pPlayer->m_autospreadgun_offer);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
}

void CGameContext::ConChangelog(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;


	//RELEASE NOTES:
	//20.9.2019 RELEASED v.0.0.7
	//7.9.2018 RELEASED v.0.0.6
	//7.10.2017 RELEASED v.0.0.3
	//25.5.2017 RELEASED v.0.0.2
	//9.4.2017 RELEASED v.0.0.1

	int page = pResult->GetInteger(0); //no parameter -> 0 -> page 1
	if (!page) { page = 1; }
	int pages = 7;
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "page %d/%d		'/changelog <page>'", page, pages);

	if (page == 1)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"=== Changelog (DDNet++ v.0.0.7) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"* fix flag crashbug");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"* fix account system (database) on windows");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"* finally own gametype name");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ add new shop (map/motd/dummy)");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ add '/score' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ add '/drop_armor' and '/drop_health' commands");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ add '/spawn' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ add '/survival' minigame");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ add '/regex' staff command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ add '/mapsave' staff command");
	}
	else if (page == 2)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"=== Changelog (DDNet++ v.0.0.6) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"* fix tons of bugs");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"* improved trade command and added public trades");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"* new shop");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"* upgraded fng (multis/onfire-mode/fng only mode)");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added spawnweapons to shop");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added drop weapons (on vote no key)");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added vanilla/ddrace mode tiles");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added spooky_ghost to shop");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added nobo spawn");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			aBuf);
	}
	else if (page == 3)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"=== Changelog (DDNet++ v.0.0.5) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"* drop flags in aim direction");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added cosmetic tiles");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"* fixed the chidraqul minigame");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added dummymodes for BlmapChill and blmapV5");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			aBuf);
	}
	else if (page == 4)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"=== Changelog (DDNet++ v.0.0.4) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added spreading guns for VIP+");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added global chat (@all)");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added block tournaments");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added fng settings (check '/fng')");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/wanted' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/viewers' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/blockwave' minigame");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			aBuf);
	}
	else if (page == 5)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"=== Changelog (DDNet++ v.0.0.3) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added minigames overview (check '/minigames')");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added balance battles (check '/balance')");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added new '/insta' commands and gametypes");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added new '/bounty' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added new '/trade' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			aBuf);
	}
	else if (page == 6)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"=== Changelog (DDNet++ v.0.0.2) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/ascii' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added block points check '/points'");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/hook <power>' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/hide' and '/show' commands");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added quests ('/quest' for more info)");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/insta gdm' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"* improved the racer bot");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			aBuf);
	}
	else if (page == 7)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"=== Changelog (DDNet++ v.0.0.1) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added VIP+");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added VIP");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added VIP+ Spawn");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/acc_logout' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/changepassword <old> <new> <new>' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/poop <amount> <player>' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/pay <amount> <player>' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/policeinfo' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/bomb <command>' command more info '/bomb help'");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added instagib modes (gdm, idm, gSurvival and iSurvival)");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"* dummys now join automatically on server start");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"* improved the blocker bot");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			aBuf);
	}
	else
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"unknow page.");
	}
	
}

void CGameContext::ConScore(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];

	if (!str_comp_nocase(pResult->GetString(0), "time"))
	{
		pPlayer->m_DisplayScore = CPlayer::SCORE_TIME;
		pSelf->SendChatTarget(pResult->m_ClientID, "[SCORE] Changed displayed score to 'time'.");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "level"))
	{
		pPlayer->m_DisplayScore = CPlayer::SCORE_LEVEL;
		pSelf->SendChatTarget(pResult->m_ClientID, "[SCORE] Changed displayed score to 'level'.");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "block"))
	{
		pPlayer->m_DisplayScore = CPlayer::SCORE_BLOCK;
		pSelf->SendChatTarget(pResult->m_ClientID, "[SCORE] Changed displayed score to 'blockpoints'.");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[SCORE] You can choose what the player score will display:");
		pSelf->SendChatTarget(pResult->m_ClientID, "time, level, block");
	}

	return;
}

void CGameContext::ConShop(IConsole::IResult *pResult, void *pUserData)
{

	// if you add something to the shop make sure to also add extend the list here and add a page to ShopWindow() and BuyItem() in character.cpp

	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "room_key | %d | 16 | disconnect", g_Config.m_SvRoomPrice);

	if (!str_comp_nocase(pResult->GetString(0), "help"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "~~~ SHOP HELP ~~~");
		pSelf->SendChatTarget(pResult->m_ClientID, "If you're in the shop you can open the shop menu using f4.");
		pSelf->SendChatTarget(pResult->m_ClientID, "By shooting to the right you go one site forward,");
		pSelf->SendChatTarget(pResult->m_ClientID, "and by shooting left you go one site backwards.");
		pSelf->SendChatTarget(pResult->m_ClientID, "If you want to buy an item you have to press f3.");
		pSelf->SendChatTarget(pResult->m_ClientID, "Then a confirmation will pop up and you have to press f3 again to confirm.");
		pSelf->SendChatTarget(pResult->m_ClientID, "NOTICE: f3 and f4 may not work for you, you have to press VOTE YES for f3 and VOTE NO for f4.");

		if (pSelf->GetShopBot() != -1)
		{
			char aShopBot[128];
			str_format(aShopBot, sizeof(aShopBot), "If you want to see the shop, watch '%s' in '/pause'.", pSelf->Server()->ClientName(pSelf->GetShopBot()));
			pSelf->SendChatTarget(pResult->m_ClientID, "-----------------------------");
			pSelf->SendChatTarget(pResult->m_ClientID, aShopBot);
		}
	}
	else
	{
		/*
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"***************************");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"          ~ SHOP ~");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"***************************");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"Type '/buy <itemname>'");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"***************************");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"ItemName | Price | Needed Level | OwnTime:");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"rainbow       1 500 | 5 | dead");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"bloody         3 500 | 15 | dead");
		//pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		//	"atom         3 500 money | 3 | dead");
		//pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		//	"trail         3 500 money | 3 | dead");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"chidraqul     250 | 2 | disconnect");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"shit              5 | 0 | forever");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		aBuf);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"police		   100 000 | 18 | forever");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"taser		  50 000 | Police[3] | forever");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"pvp_arena_ticket     150 | 0 | 1 use");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"ninjajetpack     10 000 | 21 | forever");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"spawn_shotgun     600 000 | 38 | forever");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"spawn_grenade     600 000 | 38 | forever");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"spawn_rifle     600 000 | 38 | forever");
		*/

		char aShop[2048];
		str_format(aShop, sizeof(aShop),
			"***************************\n"
			"        ~  S H O P  ~      \n"
			"***************************\n"
			"Usage: '/buy (itemname)'\n"
			"***************************\n"
			"Item | Price | Level | Time:\n"
			"-------+------+--------+-------\n"
			"rainbow  | 1 500 | 5 | dead\n"
			"bloody    | 3 500 | 15 | dead\n"
			"chidraqul | 250 | 2 | disconnect\n"
			"shit   | 5 | 0 | forever\n"
			"%s\n"
			"police | 100 000 | 18 | forever\n"
			"taser | 50 000 | Police[3] | forever\n"
			"pvp_arena_ticket | 150 | 0 | forever\n"
			"ninjajetpack | 10 000 | 21 | forever\n"
			"spawn_shotgun | 600 000 | 33 | forever\n"
			"spawn_grenade | 600 000 | 33 | forever\n"
			"spawn_rifle | 600 000 | 33 | forever\n"
			"spooky_ghost | 1 000 000 | 1 | forever\n", aBuf);

		pSelf->AbuseMotd(aShop, pResult->m_ClientID);
	}
}

void CGameContext::ConPoliceChat(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];

	char aBuf[256 + 24];

	str_format(aBuf, 256 + 24, "[%x][Police]%s: %s",
		pPlayer->m_PoliceRank,
		pSelf->Server()->ClientName(pResult->m_ClientID),
		pResult->GetString(0));
	if (pPlayer->m_PoliceRank > 0)
		pSelf->SendChat(-2, CGameContext::CHAT_ALL, aBuf, pResult->m_ClientID);
	else
		pSelf->Console()->Print(
			IConsole::OUTPUT_LEVEL_STANDARD,
			"Police",
			"You are not police.");
}

void CGameContext::ConCredits(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "credit",
		"ChillerDragon's Block Mod (" DDNETPP_VERSION ").");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "credit",
		"Created by ChillerDragon, timakro, FruchtiHD, fokkonaut, ReiTW, Henritees");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "credit",
		"heinrich5991, QshaR, Teecloud, noby, SarKro, Pikotee, toast & Blue");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "credit",
		"Based on DDNet.");
}

void CGameContext::ConInfo(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;

	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "credit",
		"ChillerDragon's Block mod. " DDNETPP_VERSION " (more info '/changelog')");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info",
			"Based on DDNet Version: " GAME_VERSION);
	if(GIT_SHORTREV_HASH)
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "Git revision hash: %s", GIT_SHORTREV_HASH);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	}
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info",
			"Official site: DDNet.tw");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info",
			"For more info: /cmdlist");
}

void CGameContext::ConHelp(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;

	if (pResult->NumArguments() == 0)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "help",
				"/cmdlist will show a list of all chat commands");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "help",
				"/help + any command will show you the help for this command.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "help",
				"Example /help settings will display the help about settings.");
	}
	else
	{
		const char *pArg = pResult->GetString(0);
		const IConsole::CCommandInfo *pCmdInfo =
				pSelf->Console()->GetCommandInfo(pArg, CFGFLAG_SERVER, false);
		if (pCmdInfo)
		{
			if (pCmdInfo->m_pParams)
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "Usage: %s %s", pCmdInfo->m_pName, pCmdInfo->m_pParams);
				pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "help", aBuf);
			}

			if (pCmdInfo->m_pHelp)
				pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "help", pCmdInfo->m_pHelp);
		}
		else
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"help",
					"Command is either unknown or you have given a blank command without any parameters.");
	}
}

void CGameContext::ConSettings(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;

	if (pResult->NumArguments() == 0)
	{
		pSelf->Console()->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"setting",
				"To server setting type '/settings <settingname>', setting names are:");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "setting",
				"teams, collision, hooking, endlesshooking, me, ");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "setting",
				"hitting, oldlaser, timeout, votes, pause and scores.");
	}
	else
	{
		const char *pArg = pResult->GetString(0);
		char aBuf[256];
		float ColTemp;
		float HookTemp;
		pSelf->m_Tuning.Get("player_collision", &ColTemp);
		pSelf->m_Tuning.Get("player_hooking", &HookTemp);
		if (str_comp(pArg, "teams") == 0)
		{
			str_format(
					aBuf,
					sizeof(aBuf),
					"%s %s",
					g_Config.m_SvTeam == 1 ?
							"Teams are available on this server" :
							(g_Config.m_SvTeam == 0 || g_Config.m_SvTeam == 3) ?
									"Teams are not available on this server" :
									"You have to be in a team to play on this server", /*g_Config.m_SvTeamStrict ? "and if you die in a team all of you die" : */
									"and if you die in a team only you die");
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "settings",
					aBuf);
		}
		else if (str_comp(pArg, "collision") == 0)
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"settings",
					ColTemp ?
							"Players can collide on this server" :
							"Players can't collide on this server");
		}
		else if (str_comp(pArg, "hooking") == 0)
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"settings",
					HookTemp ?
							"Players can hook each other on this server" :
							"Players can't hook each other on this server");
		}
		else if (str_comp(pArg, "endlesshooking") == 0)
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"settings",
					g_Config.m_SvEndlessDrag ?
							"Players endlesshook is activated" :
							"Players endlesshook is deactivated");
		}
		else if (str_comp(pArg, "hitting") == 0)
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"settings",
					g_Config.m_SvHit ?
							"Player's weapons affect others." :
							"Player's weapons don't affect others.");
		}
		else if (str_comp(pArg, "oldlaser") == 0)
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"settings",
					g_Config.m_SvOldLaser ?
							"Lasers can hit you if you shot them and they pull you towards the bounce origin (Like DDRace Beta)" :
							"Lasers can't hit you if you shot them, and they pull others towards the shooter");
		}
		else if (str_comp(pArg, "me") == 0)
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"settings",
					g_Config.m_SvSlashMe ?
							"Players can use '/me'" :
							"Players can't '/me'");
		}
		else if (str_comp(pArg, "timeout") == 0)
		{
			str_format(aBuf, sizeof(aBuf),
					"The server timeout is currently set to %d seconds",
					g_Config.m_ConnTimeout);
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "settings",
					aBuf);
		}
		else if (str_comp(pArg, "votes") == 0)
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"settings",
					g_Config.m_SvVoteKick ?
							"Players can use Callvote menu tab to kick offenders" :
							"Players can't use the Callvote menu tab to kick offenders");
			if (g_Config.m_SvVoteKick)
				str_format(
						aBuf,
						sizeof(aBuf),
						"Players are banned for %d second(s) if they get voted off",
						g_Config.m_SvVoteKickBantime);
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"settings",
					g_Config.m_SvVoteKickBantime ?
							aBuf :
							"Players are just kicked and not banned if they get voted off");
		}
		else if (str_comp(pArg, "pause") == 0)
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"settings",
					g_Config.m_SvPauseable ?
							g_Config.m_SvPauseTime ?
									"/pause is available on this server and it pauses your time too" :
									"/pause is available on this server but it doesn't pause your time"
									:"/pause is not available on this server");
		}
		else if (str_comp(pArg, "scores") == 0)
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"settings",
					g_Config.m_SvHideScore ?
							"Scores are private on this server" :
							"Scores are public on this server");
		}
	}
}

void CGameContext::ConRules(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	bool Printed = false;
	if (g_Config.m_SvDDRaceRules)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "rules",
				"Be nice.");
		Printed = true;
	}
	#define _RL(n) g_Config.m_SvRulesLine ## n
	char *pRuleLines[] = {
		_RL(1), _RL(2), _RL(3), _RL(4), _RL(5),
		_RL(6), _RL(7), _RL(8), _RL(9), _RL(10),
	};
	for(unsigned i = 0; i < sizeof(pRuleLines) / sizeof(pRuleLines[0]); i++)
	{
		if(pRuleLines[i][0])
		{
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD,
				"rules", pRuleLines[i]);
			Printed = true;
		}
	}
	if (!Printed)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "rules",
				"No Rules Defined, Kill em all!!");
	}
}

void CGameContext::ConToggleSpec(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	char aBuf[128];

	if(!g_Config.m_SvPauseable)
	{
		ConTogglePause(pResult, pUserData);
		return;
	}

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (pPlayer->GetCharacter() == 0)
	{
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "spec",
	"You can't spec while you're dead/a spectator.");
	return;
	}
	if (pPlayer->m_Paused == CPlayer::PAUSED_SPEC && g_Config.m_SvPauseable)
	{
		ConTogglePause(pResult, pUserData);
		return;
	}

	if (pPlayer->m_Paused == CPlayer::PAUSED_FORCE)
	{
		str_format(aBuf, sizeof(aBuf), "You are force-specced. %ds left.", pPlayer->m_ForcePauseTime/pSelf->Server()->TickSpeed());
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "spec", aBuf);
		return;
	}
	if ((pResult->NumArguments() > 0) && (pSelf->GetCIDByName(pResult->GetString(0)) != pPlayer->m_SpectatorID))
	{
		if (pSelf->GetCIDByName(pResult->GetString(0)) == pResult->m_ClientID)
		{
			pPlayer->m_SpectatorID = SPEC_FREEVIEW;
		}
		else
		{
			pPlayer->m_SpectatorID = pSelf->GetCIDByName(pResult->GetString(0));
			pPlayer->m_Paused = CPlayer::PAUSED_PAUSED;
		}
	}
	else
	{
		pPlayer->m_Paused = (pPlayer->m_Paused == CPlayer::PAUSED_PAUSED) ? CPlayer::PAUSED_NONE : CPlayer::PAUSED_PAUSED;
	}
}

void CGameContext::ConTogglePause(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientID(pResult->m_ClientID)) return;
	char aBuf[128];

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if(!pPlayer)
		return;

	if (pPlayer->GetCharacter() == 0)
	{
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "pause",
	"You can't pause while you're dead/a spectator.");
	return;
	}

	if(pPlayer->m_Paused == CPlayer::PAUSED_FORCE)
	{
		str_format(aBuf, sizeof(aBuf), "You are force-paused. %ds left.", pPlayer->m_ForcePauseTime/pSelf->Server()->TickSpeed());
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "pause", aBuf);
		return;
	}
	if ((pResult->NumArguments() > 0) && (pSelf->GetCIDByName(pResult->GetString(0)) != pPlayer->m_SpectatorID))
	{
		if (pSelf->GetCIDByName(pResult->GetString(0)) == pResult->m_ClientID)
		{
			pPlayer->m_SpectatorID = SPEC_FREEVIEW;
		}
		else
		{
			pPlayer->m_SpectatorID = pSelf->GetCIDByName(pResult->GetString(0));
			pPlayer->m_Paused = CPlayer::PAUSED_SPEC;
		}
	}
	else
	{
		pPlayer->m_Paused = (pPlayer->m_Paused == CPlayer::PAUSED_SPEC) ? CPlayer::PAUSED_NONE : CPlayer::PAUSED_SPEC;
	}
}

void CGameContext::ConTeamTop5(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

#if defined(CONF_SQL)
	if(pSelf->m_apPlayers[pResult->m_ClientID] && g_Config.m_SvUseSQL)
		if(pSelf->m_apPlayers[pResult->m_ClientID]->m_LastSQLQuery + pSelf->Server()->TickSpeed() >= pSelf->Server()->Tick())
			return;
#endif

	if (g_Config.m_SvHideScore)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "teamtop5",
				"Showing the team top 5 is not allowed on this server.");
		return;
	}

	if (pResult->NumArguments() > 0 && pResult->GetInteger(0) > 0)
		pSelf->Score()->ShowTeamTop5(pResult, pResult->m_ClientID, pUserData,
				pResult->GetInteger(0));
	else
		pSelf->Score()->ShowTeamTop5(pResult, pResult->m_ClientID, pUserData);

#if defined(CONF_SQL)
	if(pSelf->m_apPlayers[pResult->m_ClientID] && g_Config.m_SvUseSQL)
		pSelf->m_apPlayers[pResult->m_ClientID]->m_LastSQLQuery = pSelf->Server()->Tick();
#endif
}

void CGameContext::ConTop5(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	//if (g_Config.m_SvInstagibMode)
	//{
	//	pSelf->Console()->Print(
	//		IConsole::OUTPUT_LEVEL_STANDARD,
	//		"rank",
	//		"Instagib ranks coming soon... check '/stats (name)' for now.");
	//}
	//else
	{

#if defined(CONF_SQL)
		if (pSelf->m_apPlayers[pResult->m_ClientID] && g_Config.m_SvUseSQL)
			if (pSelf->m_apPlayers[pResult->m_ClientID]->m_LastSQLQuery + pSelf->Server()->TickSpeed() >= pSelf->Server()->Tick())
				return;
#endif

		if (g_Config.m_SvHideScore)
		{
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "top5",
				"Showing the top 5 is not allowed on this server.");
			return;
		}

	if (pResult->NumArguments() > 0 && pResult->GetInteger(0) > 0)
		pSelf->Score()->ShowTop5(pResult, pResult->m_ClientID, pUserData,
				pResult->GetInteger(0));
		else
			pSelf->Score()->ShowTop5(pResult, pResult->m_ClientID, pUserData);

#if defined(CONF_SQL)
		if (pSelf->m_apPlayers[pResult->m_ClientID] && g_Config.m_SvUseSQL)
			pSelf->m_apPlayers[pResult->m_ClientID]->m_LastSQLQuery = pSelf->Server()->Tick();
#endif
	}
}

#if defined(CONF_SQL)
void CGameContext::ConTimes(IConsole::IResult *pResult, void *pUserData)
{
	if(!CheckClientID(pResult->m_ClientID)) return;
	CGameContext *pSelf = (CGameContext *)pUserData;

#if defined(CONF_SQL)
	if(pSelf->m_apPlayers[pResult->m_ClientID] && g_Config.m_SvUseSQL)
		if(pSelf->m_apPlayers[pResult->m_ClientID]->m_LastSQLQuery + pSelf->Server()->TickSpeed() >= pSelf->Server()->Tick())
			return;
#endif

	if(g_Config.m_SvUseSQL)
	{
		CSqlScore *pScore = (CSqlScore *)pSelf->Score();
		CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
		if(!pPlayer)
			return;

		if(pResult->NumArguments() == 0)
		{
			pScore->ShowTimes(pPlayer->GetCID(),1);
			return;
		}

		else if(pResult->NumArguments() < 3)
		{
			if (pResult->NumArguments() == 1)
			{
				if(pResult->GetInteger(0) != 0)
					pScore->ShowTimes(pPlayer->GetCID(),pResult->GetInteger(0));
				else
					pScore->ShowTimes(pPlayer->GetCID(), (str_comp(pResult->GetString(0), "me") == 0) ? pSelf->Server()->ClientName(pResult->m_ClientID) : pResult->GetString(0),1);
				return;
			}
			else if (pResult->GetInteger(1) != 0)
			{
				pScore->ShowTimes(pPlayer->GetCID(), (str_comp(pResult->GetString(0), "me") == 0) ? pSelf->Server()->ClientName(pResult->m_ClientID) : pResult->GetString(0),pResult->GetInteger(1));
				return;
			}
		}

		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "times", "/times needs 0, 1 or 2 parameter. 1. = name, 2. = start number");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "times", "Example: /times, /times me, /times Hans, /times \"Papa Smurf\" 5");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "times", "Bad: /times Papa Smurf 5 # Good: /times \"Papa Smurf\" 5 ");

#if defined(CONF_SQL)
		if(pSelf->m_apPlayers[pResult->m_ClientID] && g_Config.m_SvUseSQL)
			pSelf->m_apPlayers[pResult->m_ClientID]->m_LastSQLQuery = pSelf->Server()->Tick();
#endif
	}
}
#endif

void CGameContext::ConDND(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientID(pResult->m_ClientID)) return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if(!pPlayer)
		return;

	if(pPlayer->m_DND)
	{
		pPlayer->m_DND = false;
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "dnd", "You will receive global chat and server messages");
	}
	else
	{
		pPlayer->m_DND = true;
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "dnd", "You will not receive any more global chat and server messages");
	}
}

void CGameContext::ConMap(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	if (g_Config.m_SvMapVote == 0)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "map",
				"Admin has disabled /map");
		return;
	}

	if (pResult->NumArguments() <= 0)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "map", "Use /map <mapname> to call vote for mapchange.");
		return;
	}

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

#if defined(CONF_SQL)
	if(g_Config.m_SvUseSQL)
		if(pPlayer->m_LastSQLQuery + pSelf->Server()->TickSpeed() >= pSelf->Server()->Tick())
			return;
#endif

	pSelf->Score()->MapVote(pResult->m_ClientID, pResult->GetString(0));

#if defined(CONF_SQL)
	if(g_Config.m_SvUseSQL)
		pPlayer->m_LastSQLQuery = pSelf->Server()->Tick();
#endif
}

void CGameContext::ConMapInfo(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

#if defined(CONF_SQL)
	if(g_Config.m_SvUseSQL)
		if(pPlayer->m_LastSQLQuery + pSelf->Server()->TickSpeed() >= pSelf->Server()->Tick())
			return;
#endif

	if (pResult->NumArguments() > 0)
		pSelf->Score()->MapInfo(pResult->m_ClientID, pResult->GetString(0));
	else
		pSelf->Score()->MapInfo(pResult->m_ClientID, g_Config.m_SvMap);

#if defined(CONF_SQL)
	if(g_Config.m_SvUseSQL)
		pPlayer->m_LastSQLQuery = pSelf->Server()->Tick();
#endif
}

void CGameContext::ConTimeout(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	for(int i = 0; i < pSelf->Server()->MaxClients(); i++)
	{
		if (i == pResult->m_ClientID) continue;
		if (!pSelf->m_apPlayers[i]) continue;
		if (str_comp(pSelf->m_apPlayers[i]->m_TimeoutCode, pResult->GetString(0))) continue;
		if (((CServer *)pSelf->Server())->m_NetServer.SetTimedOut(i, pResult->m_ClientID))
		{
			((CServer *)pSelf->Server())->DelClientCallback(pResult->m_ClientID, "Timeout protection used", ((CServer *)pSelf->Server()));
			((CServer *)pSelf->Server())->m_aClients[i].m_Authed = CServer::AUTHED_NO;
			if (pSelf->m_apPlayers[i]->GetCharacter())
				((CGameContext *)(((CServer *)pSelf->Server())->GameServer()))->SendTuningParams(i, pSelf->m_apPlayers[i]->GetCharacter()->m_TuneZone);
			return;
		}
	}

	((CServer *)pSelf->Server())->m_NetServer.SetTimeoutProtected(pResult->m_ClientID);
	str_copy(pPlayer->m_TimeoutCode, pResult->GetString(0), sizeof(pPlayer->m_TimeoutCode));
	if (pSelf->m_MapsavePlayers && pSelf->m_MapsaveLoadedPlayers < pSelf->m_MapsavePlayers)
		pSelf->LoadMapPlayerData();
}

void CGameContext::ConSave(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

#if defined(CONF_SQL)
	if(!g_Config.m_SvSaveGames)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Save-function is disabled on this server");
		return;
	}

	if(g_Config.m_SvUseSQL)
		if(pPlayer->m_LastSQLQuery + pSelf->Server()->TickSpeed() >= pSelf->Server()->Tick())
			return;

	int Team = ((CGameControllerDDRace*) pSelf->m_pController)->m_Teams.m_Core.Team(pResult->m_ClientID);

	const char* pCode = pResult->GetString(0);
	char aCountry[5];
	if(str_length(pCode) > 3 && pCode[0] >= 'A' && pCode[0] <= 'Z' && pCode[1] >= 'A'
		&& pCode[1] <= 'Z' && pCode[2] >= 'A' && pCode[2] <= 'Z')
	{
		if(pCode[3] == ' ')
		{
			str_copy(aCountry, pCode, 4);
			pCode = pCode + 4;
		}
		else if(str_length(pCode) > 4 && pCode[4] == ' ')
		{
			str_copy(aCountry, pCode, 5);
			pCode = pCode + 5;
		}
		else
		{
			str_copy(aCountry, g_Config.m_SvSqlServerName, sizeof(aCountry));
		}
	}
	else
	{
		str_copy(aCountry, g_Config.m_SvSqlServerName, sizeof(aCountry));
	}

	pSelf->Score()->SaveTeam(Team, pCode, pResult->m_ClientID, aCountry);

	if(g_Config.m_SvUseSQL)
		pPlayer->m_LastSQLQuery = pSelf->Server()->Tick();
#endif
}

void CGameContext::ConLoad(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

#if defined(CONF_SQL)
	if(!g_Config.m_SvSaveGames)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Save-function is disabled on this server");
		return;
	}

	if(g_Config.m_SvUseSQL)
		if(pPlayer->m_LastSQLQuery + pSelf->Server()->TickSpeed() >= pSelf->Server()->Tick())
			return;
#endif

	if (pResult->NumArguments() > 0)
		pSelf->Score()->LoadTeam(pResult->GetString(0), pResult->m_ClientID);
	else
		return;

#if defined(CONF_SQL)
	if(g_Config.m_SvUseSQL)
		pPlayer->m_LastSQLQuery = pSelf->Server()->Tick();
#endif
}

void CGameContext::ConTeamRank(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

#if defined(CONF_SQL)
	if(g_Config.m_SvUseSQL)
		if(pPlayer->m_LastSQLQuery + pSelf->Server()->TickSpeed() >= pSelf->Server()->Tick())
			return;
#endif

	if (pResult->NumArguments() > 0)
		if (!g_Config.m_SvHideScore)
			pSelf->Score()->ShowTeamRank(pResult->m_ClientID, pResult->GetString(0),
					true);
		else
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"teamrank",
					"Showing the team rank of other players is not allowed on this server.");
	else
		pSelf->Score()->ShowTeamRank(pResult->m_ClientID,
				pSelf->Server()->ClientName(pResult->m_ClientID));

#if defined(CONF_SQL)
	if(g_Config.m_SvUseSQL)
		pPlayer->m_LastSQLQuery = pSelf->Server()->Tick();
#endif
}

void CGameContext::ConRank(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

#if defined(CONF_SQL)
		if (g_Config.m_SvUseSQL)
			if (pPlayer->m_LastSQLQuery + pSelf->Server()->TickSpeed() >= pSelf->Server()->Tick())
				return;
#endif

	if (pResult->NumArguments() > 0)
		if (!g_Config.m_SvHideScore)
			pSelf->Score()->ShowRank(pResult->m_ClientID, pResult->GetString(0),
				true);
		else
			pSelf->Console()->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"rank",
				"Showing the rank of other players is not allowed on this server.");
	else
		pSelf->Score()->ShowRank(pResult->m_ClientID,
			pSelf->Server()->ClientName(pResult->m_ClientID));

#if defined(CONF_SQL)
	if (g_Config.m_SvUseSQL)
		pPlayer->m_LastSQLQuery = pSelf->Server()->Tick();
#endif
}

void CGameContext::ConLockTeam(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int Team = ((CGameControllerDDRace*) pSelf->m_pController)->m_Teams.m_Core.Team(pResult->m_ClientID);

	bool Lock = ((CGameControllerDDRace*) pSelf->m_pController)->m_Teams.TeamLocked(Team);

	if (pResult->NumArguments() > 0)
		Lock = !pResult->GetInteger(0);

	if(Team > TEAM_FLOCK && Team < TEAM_SUPER)
	{
		char aBuf[512];
		if(Lock)
		{
			((CGameControllerDDRace*) pSelf->m_pController)->m_Teams.SetTeamLock(Team, false);

			str_format(aBuf, sizeof(aBuf), "'%s' unlocked your team.", pSelf->Server()->ClientName(pResult->m_ClientID));

			for (int i = 0; i < MAX_CLIENTS; i++)
				if (((CGameControllerDDRace*) pSelf->m_pController)->m_Teams.m_Core.Team(i) == Team)
					pSelf->SendChatTarget(i, aBuf);
		}
		else if(!g_Config.m_SvTeamLock)
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"print",
					"Team locking is disabled on this server");
		}
		else
		{
			((CGameControllerDDRace*) pSelf->m_pController)->m_Teams.SetTeamLock(Team, true);

			str_format(aBuf, sizeof(aBuf), "'%s' locked your team. After the race started killing will kill everyone in your team.", pSelf->Server()->ClientName(pResult->m_ClientID));

			for (int i = 0; i < MAX_CLIENTS; i++)
				if (((CGameControllerDDRace*) pSelf->m_pController)->m_Teams.m_Core.Team(i) == Team)
					pSelf->SendChatTarget(i, aBuf);
		}
	}
	else
		pSelf->Console()->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"print",
				"This team can't be locked");
}

void CGameContext::ConJoinTeam(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (pSelf->m_VoteCloseTime && pSelf->m_VoteCreator == pResult->m_ClientID && (pSelf->m_VoteKick || pSelf->m_VoteSpec))
	{
		pSelf->Console()->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"join",
				"You are running a vote please try again after the vote is done!");
		return;
	}
	else if (g_Config.m_SvTeam == 0 || g_Config.m_SvTeam == 3)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "join",
				"Admin has disabled teams");
		return;
	}
	else if (g_Config.m_SvTeam == 2 && pResult->GetInteger(0) == 0 && pPlayer->GetCharacter() && pPlayer->GetCharacter()->m_LastStartWarning < pSelf->Server()->Tick() - 3 * pSelf->Server()->TickSpeed())
	{
		pSelf->Console()->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"join",
				"You must join a team in order to start.");
		pPlayer->GetCharacter()->m_LastStartWarning = pSelf->Server()->Tick();
	}

	if (pResult->NumArguments() > 0)
	{
		if (pPlayer->GetCharacter() == 0)
		{
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "join",
					"You can't change teams while you're dead/a spectator.");
		}
		else
		{
			if (pPlayer->m_Last_Team
					+ pSelf->Server()->TickSpeed()
					* g_Config.m_SvTeamChangeDelay
					> pSelf->Server()->Tick())
			{
				pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "join",
						"You can't change teams that fast!");
			}
			else if(pResult->GetInteger(0) > 0 && pResult->GetInteger(0) < MAX_CLIENTS && ((CGameControllerDDRace*) pSelf->m_pController)->m_Teams.TeamLocked(pResult->GetInteger(0)))
			{
				pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "join",
						"This team is locked using '/lock'. Only members of the team can unlock it using '/lock'.");
			}
			else if(pResult->GetInteger(0) > 0 && pResult->GetInteger(0) < MAX_CLIENTS && ((CGameControllerDDRace*) pSelf->m_pController)->m_Teams.Count(pResult->GetInteger(0)) >= g_Config.m_SvTeamMaxSize)
			{
				char aBuf[512];
				str_format(aBuf, sizeof(aBuf), "This team already has the maximum allowed size of %d players", g_Config.m_SvTeamMaxSize);
				pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "join", aBuf);
			}
			else if (((CGameControllerDDRace*) pSelf->m_pController)->m_Teams.SetCharacterTeam(
					pPlayer->GetCID(), pResult->GetInteger(0)))
			{
				char aBuf[512];
				str_format(aBuf, sizeof(aBuf), "%s joined team %d",
						pSelf->Server()->ClientName(pPlayer->GetCID()),
						pResult->GetInteger(0));
				pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				pPlayer->m_Last_Team = pSelf->Server()->Tick();
			}
			else
			{
				pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "join",
						"You can't join this team at the moment");
			}
		}
	}
	else
	{
		char aBuf[512];
		if (!pPlayer->IsPlaying())
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"join",
					"You can't check your team while you're dead/a spectator.");
		}
		else
		{
			str_format(
					aBuf,
					sizeof(aBuf),
					"You are in team %d",
					((CGameControllerDDRace*) pSelf->m_pController)->m_Teams.m_Core.Team(
							pResult->m_ClientID));
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "join",
					aBuf);
		}
	}
}

void CGameContext::ConMe(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	char aBuf[256 + 24];

	str_format(aBuf, 256 + 24, "'%s' %s",
			pSelf->Server()->ClientName(pResult->m_ClientID),
			pResult->GetString(0));
	if (g_Config.m_SvSlashMe)
		pSelf->SendChat(-2, CGameContext::CHAT_ALL, aBuf, pResult->m_ClientID);
	else
		pSelf->Console()->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"me",
				"/me is disabled on this server, admin can enable it by using sv_slash_me");
}

void CGameContext::ConConverse(IConsole::IResult *pResult, void *pUserData)
{
	// This will never be called
}

void CGameContext::ConWhisper(IConsole::IResult *pResult, void *pUserData)
{
	// This will never be called
}

void CGameContext::ConSetEyeEmote(IConsole::IResult *pResult,
		void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;
	if(pResult->NumArguments() == 0) {
		pSelf->Console()->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"emote",
				(pPlayer->m_EyeEmote) ?
						"You can now use the preset eye emotes." :
						"You don't have any eye emotes, remember to bind some. (until you die)");
		return;
	}
	else if(str_comp_nocase(pResult->GetString(0), "on") == 0)
		pPlayer->m_EyeEmote = true;
	else if(str_comp_nocase(pResult->GetString(0), "off") == 0)
		pPlayer->m_EyeEmote = false;
	else if(str_comp_nocase(pResult->GetString(0), "toggle") == 0)
		pPlayer->m_EyeEmote = !pPlayer->m_EyeEmote;
	pSelf->Console()->Print(
			IConsole::OUTPUT_LEVEL_STANDARD,
			"emote",
			(pPlayer->m_EyeEmote) ?
					"You can now use the preset eye emotes." :
					"You don't have any eye emotes, remember to bind some. (until you die)");
}

void CGameContext::ConEyeEmote(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (g_Config.m_SvEmotionalTees == -1)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "emote",
				"Admin disabled emotes.");
		return;
	}

	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (pResult->NumArguments() == 0)
	{
		pSelf->Console()->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"emote",
				"Emote commands are: /emote surprise /emote blink /emote close /emote angry /emote happy /emote pain");
		pSelf->Console()->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"emote",
				"Example: /emote surprise 10 for 10 seconds or /emote surprise (default 1 second)");
	}
	else
	{
			if(pPlayer->m_LastEyeEmote + g_Config.m_SvEyeEmoteChangeDelay * pSelf->Server()->TickSpeed() >= pSelf->Server()->Tick())
				return;

			if (!str_comp(pResult->GetString(0), "angry"))
				pPlayer->m_DefEmote = EMOTE_ANGRY;
			else if (!str_comp(pResult->GetString(0), "blink"))
				pPlayer->m_DefEmote = EMOTE_BLINK;
			else if (!str_comp(pResult->GetString(0), "close"))
				pPlayer->m_DefEmote = EMOTE_BLINK;
			else if (!str_comp(pResult->GetString(0), "happy"))
				pPlayer->m_DefEmote = EMOTE_HAPPY;
			else if (!str_comp(pResult->GetString(0), "pain"))
				pPlayer->m_DefEmote = EMOTE_PAIN;
			else if (!str_comp(pResult->GetString(0), "surprise"))
				pPlayer->m_DefEmote = EMOTE_SURPRISE;
			else if (!str_comp(pResult->GetString(0), "normal"))
				pPlayer->m_DefEmote = EMOTE_NORMAL;
			else
				pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD,
						"emote", "Unknown emote... Say /emote");

			int Duration = 1;
			if (pResult->NumArguments() > 1)
				Duration = pResult->GetInteger(1);

			pPlayer->m_DefEmoteReset = pSelf->Server()->Tick()
							+ Duration * pSelf->Server()->TickSpeed();
			pPlayer->m_LastEyeEmote = pSelf->Server()->Tick();
	}
}

void CGameContext::ConNinjaJetpack(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;
	if (pPlayer->m_NinjaJetpackBought)
	{
		if (pPlayer->m_NinjaJetpack)
		{
			pPlayer->m_NinjaJetpack = false;
			pSelf->SendChatTarget(pResult->m_ClientID, "Ninjajetpack disabled");
			return;
		}
		else
		{
			pPlayer->m_NinjaJetpack = true;
			pSelf->SendChatTarget(pResult->m_ClientID, "Ninjajetpack enabled");
			return;
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You don't have ninjajetpack. Buy it using '/buy ninjajetpack'.");
		return;
	}
}

void CGameContext::ConShowOthers(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;
	if (g_Config.m_SvShowOthers)
	{
		if (pResult->NumArguments())
			pPlayer->m_ShowOthers = pResult->GetInteger(0);
		else
			pPlayer->m_ShowOthers = !pPlayer->m_ShowOthers;
	}
	else
		pSelf->Console()->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"showotherschat",
				"Showing players from other teams is disabled by the server admin");
}

void CGameContext::ConShowAll(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (pResult->NumArguments())
		pPlayer->m_ShowAll = pResult->GetInteger(0);
	else
		pPlayer->m_ShowAll = !pPlayer->m_ShowAll;
}

void CGameContext::ConSpecTeam(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (pResult->NumArguments())
		pPlayer->m_SpecTeam = pResult->GetInteger(0);
	else
		pPlayer->m_SpecTeam = !pPlayer->m_SpecTeam;
}

bool CheckClientID(int ClientID)
{
	dbg_assert(ClientID >= 0 || ClientID < MAX_CLIENTS,
			"Wrong clientID!");
	if (ClientID < 0 || ClientID >= MAX_CLIENTS)
		return false;
	return true;
}

void CGameContext::ConSayTime(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID;
	char aBufname[MAX_NAME_LENGTH];

	if (pResult->NumArguments() > 0)
	{
		for(ClientID = 0; ClientID < MAX_CLIENTS; ClientID++)
			if (str_comp(pResult->GetString(0), pSelf->Server()->ClientName(ClientID)) == 0)
				break;

		if(ClientID == MAX_CLIENTS)
			return;

		str_format(aBufname, sizeof(aBufname), "%s's", pSelf->Server()->ClientName(ClientID));
	}
	else
	{
		str_copy(aBufname, "Your", sizeof(aBufname));
		ClientID = pResult->m_ClientID;
	}

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;
	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;
	if(pChr->m_DDRaceState != DDRACE_STARTED)
		return;

	char aBuftime[64];
	int IntTime = (int) ((float) (pSelf->Server()->Tick() - pChr->m_StartTime)
			/ ((float) pSelf->Server()->TickSpeed()));
	str_format(aBuftime, sizeof(aBuftime), "%s time is %s%d:%s%d",
			aBufname,
			((IntTime / 60) > 9) ? "" : "0", IntTime / 60,
			((IntTime % 60) > 9) ? "" : "0", IntTime % 60);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "time", aBuftime);
}

void CGameContext::ConSayTimeAll(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;
	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;
	if(pChr->m_DDRaceState != DDRACE_STARTED)
		return;

	char aBuftime[64];
	int IntTime = (int) ((float) (pSelf->Server()->Tick() - pChr->m_StartTime)
			/ ((float) pSelf->Server()->TickSpeed()));
	str_format(aBuftime, sizeof(aBuftime),
			"%s\'s current race time is %s%d:%s%d",
			pSelf->Server()->ClientName(pResult->m_ClientID),
			((IntTime / 60) > 9) ? "" : "0", IntTime / 60,
			((IntTime % 60) > 9) ? "" : "0", IntTime % 60);
	pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuftime, pResult->m_ClientID);
}

void CGameContext::ConTime(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;
	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	char aBuftime[64];
	int IntTime = (int) ((float) (pSelf->Server()->Tick() - pChr->m_StartTime)
			/ ((float) pSelf->Server()->TickSpeed()));
	str_format(aBuftime, sizeof(aBuftime), "Your time is %s%d:%s%d",
				((IntTime / 60) > 9) ? "" : "0", IntTime / 60,
				((IntTime % 60) > 9) ? "" : "0", IntTime % 60);
	pSelf->SendBroadcast(aBuftime, pResult->m_ClientID);
}

void CGameContext::ConSetTimerType(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;

	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	const char msg[3][128] = {"game/round timer.", "broadcast.", "both game/round timer and broadcast."};
	char aBuf[128];
	if(pPlayer->m_TimerType <= 2 && pPlayer->m_TimerType >= 0)
		str_format(aBuf, sizeof(aBuf), "Timer is displayed in", msg[pPlayer->m_TimerType]);
	else if(pPlayer->m_TimerType == 3)
		str_format(aBuf, sizeof(aBuf), "Timer isn't displayed.");

	if(pResult->NumArguments() == 0) {
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD,"timer",aBuf);
		return;
	}
	else if(str_comp_nocase(pResult->GetString(0), "gametimer") == 0) {
		pSelf->SendBroadcast("", pResult->m_ClientID);
		pPlayer->m_TimerType = 0;
	}
	else if(str_comp_nocase(pResult->GetString(0), "broadcast") == 0)
			pPlayer->m_TimerType = 1;
	else if(str_comp_nocase(pResult->GetString(0), "both") == 0)
			pPlayer->m_TimerType = 2;
	else if(str_comp_nocase(pResult->GetString(0), "none") == 0)
			pPlayer->m_TimerType = 3;
	else if(str_comp_nocase(pResult->GetString(0), "cycle") == 0) {
		if(pPlayer->m_TimerType < 3)
			pPlayer->m_TimerType++;
		else if(pPlayer->m_TimerType == 3)
			pPlayer->m_TimerType = 0;
	}
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD,"timer",aBuf);
}

void CGameContext::ConRescue(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;
	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (!g_Config.m_SvRescue) {
		pSelf->SendChatTarget(pPlayer->GetCID(), "Rescue is disabled on this server.");
		return;
	}

	pChr->Rescue();
}

void CGameContext::ConProtectedKill(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;
	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	int CurrTime = (pSelf->Server()->Tick() - pChr->m_StartTime) / pSelf->Server()->TickSpeed();
	if(g_Config.m_SvKillProtection != 0 && CurrTime >= (60 * g_Config.m_SvKillProtection) && pChr->m_DDRaceState == DDRACE_STARTED)
	{
			pPlayer->KillCharacter(WEAPON_SELF);

			//char aBuf[64];
			//str_format(aBuf, sizeof(aBuf), "You killed yourself in: %s%d:%s%d",
			//		((CurrTime / 60) > 9) ? "" : "0", CurrTime / 60,
			//		((CurrTime % 60) > 9) ? "" : "0", CurrTime % 60);
			//pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
}
//#if defined(CONF_SQL)
void CGameContext::ConPoints(IConsole::IResult *pResult, void *pUserData)
{
	if (g_Config.m_SvPointsMode == 1) //ddnet
	{
#if defined(CONF_SQL)
		CGameContext *pSelf = (CGameContext *)pUserData;
		if (!CheckClientID(pResult->m_ClientID))
			return;

		if (pSelf->m_apPlayers[pResult->m_ClientID] && g_Config.m_SvUseSQL)
			if (pSelf->m_apPlayers[pResult->m_ClientID]->m_LastSQLQuery + pSelf->Server()->TickSpeed() >= pSelf->Server()->Tick())
				return;

		CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
		if (!pPlayer)
			return;

		if (pResult->NumArguments() > 0)
			if (!g_Config.m_SvHideScore)
				pSelf->Score()->ShowPoints(pResult->m_ClientID, pResult->GetString(0),
					true);
			else
				pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"points",
					"Showing the global points of other players is not allowed on this server.");
		else
			pSelf->Score()->ShowPoints(pResult->m_ClientID,
				pSelf->Server()->ClientName(pResult->m_ClientID));

		if (pSelf->m_apPlayers[pResult->m_ClientID] && g_Config.m_SvUseSQL)
			pSelf->m_apPlayers[pResult->m_ClientID]->m_LastSQLQuery = pSelf->Server()->Tick();
#else
		CGameContext *pSelf = (CGameContext *)pUserData;
		if (!CheckClientID(pResult->m_ClientID))
			return;
		CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
		if (!pPlayer)
			return;

		pSelf->Console()->Print(
			IConsole::OUTPUT_LEVEL_STANDARD,
			"points",
			"This is not an SQL server.");
#endif
	}
	else if (g_Config.m_SvPointsMode == 2) //ddpp (blockpoints)
	{
		CGameContext *pSelf = (CGameContext *)pUserData;
		if (!CheckClientID(pResult->m_ClientID))
			return;
		CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
		if (!pPlayer)
			return;

		char aBuf[256];

		if (pResult->NumArguments() > 0) //show others
		{
			int pointsID = pSelf->GetCIDByName(pResult->GetString(0));
			if (pointsID == -1)
			{
				str_format(aBuf, sizeof(aBuf), "'%s' is not online", pResult->GetString(0));
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			str_format(aBuf, sizeof(aBuf), "'%s' points[%d] kills[%d] deaths[%d]", pResult->GetString(0), pSelf->m_apPlayers[pointsID]->m_BlockPoints, pSelf->m_apPlayers[pointsID]->m_BlockPoints_Kills, pSelf->m_apPlayers[pointsID]->m_BlockPoints_Deaths);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		else //show own
		{
			str_format(aBuf, sizeof(aBuf), "'%s' points[%d] kills[%d] deaths[%d]", pSelf->Server()->ClientName(pResult->m_ClientID), pPlayer->m_BlockPoints, pPlayer->m_BlockPoints_Kills, pPlayer->m_BlockPoints_Deaths);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
	}
	else //points deactivated
	{
		CGameContext *pSelf = (CGameContext *)pUserData;
		if (!CheckClientID(pResult->m_ClientID))
			return;
		CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
		if (!pPlayer)
			return;

		pSelf->Console()->Print(
			IConsole::OUTPUT_LEVEL_STANDARD,
			"points",
			"Showing points is deactivated on this DDNet++ server.");
	}
}
//#endif

#if defined(CONF_SQL)
void CGameContext::ConTopPoints(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	if(pSelf->m_apPlayers[pResult->m_ClientID] && g_Config.m_SvUseSQL)
		if(pSelf->m_apPlayers[pResult->m_ClientID]->m_LastSQLQuery + pSelf->Server()->TickSpeed() >= pSelf->Server()->Tick())
			return;

	if (g_Config.m_SvHideScore)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "toppoints",
				"Showing the global top points is not allowed on this server.");
		return;
	}

	if (pResult->NumArguments() > 0 && pResult->GetInteger(0) >= 0)
		pSelf->Score()->ShowTopPoints(pResult, pResult->m_ClientID, pUserData,
				pResult->GetInteger(0));
	else
		pSelf->Score()->ShowTopPoints(pResult, pResult->m_ClientID, pUserData);

	if(pSelf->m_apPlayers[pResult->m_ClientID] && g_Config.m_SvUseSQL)
		pSelf->m_apPlayers[pResult->m_ClientID]->m_LastSQLQuery = pSelf->Server()->Tick();
}
#endif

void CGameContext::ConBuy(IConsole::IResult *pResult, void *pUserData)
{

	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if ((g_Config.m_SvShopState == 1) && !pChr->m_InShop)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You have to be in the shop to buy some items.");
		return;
	}

	if (pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Unknown item. Type '/buy <itemname>' use '/shop' to see the full itemlist.");
		return;
	}

	int ItemID = -1;

	char aItem[32];
	str_copy(aItem, pResult->GetString(0), 32);

	if (!str_comp_nocase(aItem, "rainbow"))
		ItemID = 1;
	else if (!str_comp_nocase(aItem, "bloody"))
		ItemID = 2;
	else if (!str_comp_nocase(aItem, "chidraqul"))
		ItemID = 3;
	else if (!str_comp_nocase(aItem, "shit"))
		ItemID = 4;
	else if (!str_comp_nocase(aItem, "room_key"))
		ItemID = 5;
	else if (!str_comp_nocase(aItem, "police"))
		ItemID = 6;
	else if (!str_comp_nocase(aItem, "taser"))
		ItemID = 7;
	else if (!str_comp_nocase(aItem, "pvp_arena_ticket"))
		ItemID = 8;
	else if (!str_comp_nocase(aItem, "ninjajetpack"))
		ItemID = 9;
	else if (!str_comp_nocase(aItem, "spawn_shotgun"))
		ItemID = 10;
	else if (!str_comp_nocase(aItem, "spawn_grenade"))
		ItemID = 11;
	else if (!str_comp_nocase(aItem, "spawn_rifle"))
		ItemID = 12;
	else if (!str_comp_nocase(aItem, "spooky_ghost"))
		ItemID = 13;
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid shop item. Choose another one.");
		return;
	}

	pChr->BuyItem(ItemID);
}

void CGameContext::ConRegister(IConsole::IResult *pResult, void *pUserData)
{
	char aBuf[512];
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (g_Config.m_SvAccountStuff == 0)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Accounts are turned off.");
		return;
	}
	if (g_Config.m_SvAccountStuff == 2) //filebased
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] SQLite accounts are turned off. (try '/register2')");
		return;
	}

	if (pResult->NumArguments() != 3)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Please use '/register <name> <password> <password>'.");
		return;
	}

	if (pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] You are already logged in.");
		return;
	}

	if (pPlayer->m_PlayerHumanLevel < g_Config.m_SvRegisterHumanLevel)
	{
		str_format(aBuf, sizeof(aBuf), "[ACCOUNT] your '/human_level' is too low %d/%d to use this command.", pPlayer->m_PlayerHumanLevel, g_Config.m_SvRegisterHumanLevel);
		pSelf->SendChatTarget(ClientID, aBuf);
		return;
	}

	NETADDR Addr;
	pSelf->Server()->GetClientAddr(ClientID, &Addr);
	Addr.port = 0;
	int RegBanned = 0;

	for(int i = 0; i < pSelf->m_NumRegisterBans && !RegBanned; i++)
	{
		if(!net_addr_comp(&Addr, &pSelf->m_aRegisterBans[i].m_Addr))
			RegBanned = (pSelf->m_aRegisterBans[i].m_Expire - pSelf->Server()->Tick()) / pSelf->Server()->TickSpeed();
	}

	if (RegBanned > 0)
	{
		char aBuf[128];
		str_format(aBuf, sizeof aBuf, "[ACCOUNT] you have to wait %d seconds before you can register again.", RegBanned);
		pSelf->SendChatTarget(ClientID, aBuf);
		return;
	}

	char aUsername[32];
	char aPassword[MAX_PW_LEN+1];
	char aPassword2[MAX_PW_LEN+1];
	str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
	str_copy(aPassword, pResult->GetString(1), sizeof(aPassword));
	str_copy(aPassword2, pResult->GetString(2), sizeof(aPassword2));

	if (str_length(aUsername) > 20 || str_length(aUsername) < 3)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Username is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if ((str_length(aPassword) > MAX_PW_LEN || str_length(aPassword) < MIN_PW_LEN) || (str_length(aPassword2) > MAX_PW_LEN || str_length(aPassword2) < MIN_PW_LEN))
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Password is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if (str_comp_nocase(aPassword, aPassword2) != 0)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Passwords need to be identical.");
		return;
	}

	//if (EvilChar)
    if (pSelf->IsAllowedCharSet(aUsername) == false)
	{
		str_format(aBuf, sizeof(aBuf), "[ACCOUNT] please use only the following characters in your username '%s'", pSelf->m_aAllowedCharSet);
		pSelf->SendChatTarget(ClientID, aBuf);
		return;
	}

	pSelf->SQLaccount(pSelf->SQL_REGISTER, ClientID, aUsername, aPassword);
}

void CGameContext::ConSQLName(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (g_Config.m_SvAccountStuff == 0)
	{
		pSelf->SendChatTarget(ClientID, "Account stuff is turned off.");
		return;
	}

	if (pPlayer->m_Authed != CServer::AUTHED_ADMIN) 
	{
		//pSelf->SendChatTarget(ClientID, "No such command: sql_name.");
		pSelf->SendChatTarget(ClientID, "Missing permission.");
		return;
	}

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(ClientID, "---- COMMANDS -----");
		//pSelf->SendChatTarget(ClientID, "'/sql_name super_mod <acc_name> <true/false>'"); //coming soon...
		//pSelf->SendChatTarget(ClientID, "'/sql_name mod <acc_name> <true/false>'"); //coming soon...
		//pSelf->SendChatTarget(ClientID, "'/sql_name freeze_acc <acc_name> <true/false>'"); //coming soon...
		pSelf->SendChatTarget(ClientID, "'/sql_name set_passwd <acc_name> <passwd>' to reset password");
		pSelf->SendChatTarget(ClientID, "----------------------");
		pSelf->SendChatTarget(ClientID, "'/acc_info <name>' additional info");
		pSelf->SendChatTarget(ClientID, "'/sql' similiar command using sql ids");
		return;
	}

	if (!str_comp_nocase(pResult->GetString(0), "set_passwd"))
	{
		if ((str_length(pResult->GetString(2)) > MAX_PW_LEN || str_length(pResult->GetString(2)) < MIN_PW_LEN) || (str_length(pResult->GetString(2)) > MAX_PW_LEN || str_length(pResult->GetString(2)) < MIN_PW_LEN))
		{
			pSelf->SendChatTarget(ClientID, "[ACCOUNT] Password is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
			return;
		}
		str_format(pPlayer->m_aSetPassword, sizeof(pPlayer->m_aSetPassword), "%s", pResult->GetString(2));
		str_format(pPlayer->m_aSQLNameName, sizeof(pPlayer->m_aSQLNameName), "%s", pResult->GetString(1));
		pSelf->SQLaccount(pSelf->SQL_SET_PASSWORD, pResult->m_ClientID, pResult->GetString(1));
	}
	else
	{
		pSelf->SendChatTarget(ClientID, "Unknown command try '/sql_name' for full list.");
	}
}

void CGameContext::ConSQL(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (g_Config.m_SvAccountStuff == 0)
	{
		pSelf->SendChatTarget(ClientID, "Account stuff is turned off.");
		return;
	}

	//if (pResult->NumArguments() < 2)
	//{
	//	pSelf->SendChatTarget(ClientID, "Error: si?i");
	//	return;
	//}

	if (pPlayer->m_Authed != CServer::AUTHED_ADMIN) //after Arguments check to troll curious users
	{
		//pSelf->SendChatTarget(ClientID, "No such command: sql.");
		pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
		return;
	}

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(ClientID, "---- COMMANDS -----");
		pSelf->SendChatTarget(ClientID, "'/sql getid <clientid>' to get sql id");
		pSelf->SendChatTarget(ClientID, "'/sql super_mod <sqlid> <val>'");
		pSelf->SendChatTarget(ClientID, "'/sql mod <sqlid> <val>'");
		pSelf->SendChatTarget(ClientID, "'/sql supporter <sqlid> <val>'");
		pSelf->SendChatTarget(ClientID, "'/sql freeze_acc <sqlid> <val>'");
		pSelf->SendChatTarget(ClientID, "----------------------");
		pSelf->SendChatTarget(ClientID, "'/acc_info <clientID>' additional info");
		pSelf->SendChatTarget(ClientID, "'/sql_name' similar command using account names");
        pSelf->SendChatTarget(ClientID, "'/sql_logout <playername>' sets logout state (risky)");
        pSelf->SendChatTarget(ClientID, "'/sql_logout_all' sets logout state only for current port (save)");
		return;
	}

	char aBuf[128];
	char aCommand[32];
	int SQL_ID;
	str_copy(aCommand, pResult->GetString(0), sizeof(aCommand));
	SQL_ID = pResult->GetInteger(1);


	if (!str_comp_nocase(aCommand, "getid")) //2 argument commands
	{
		if (!pSelf->m_apPlayers[SQL_ID])
		{
			str_format(aBuf, sizeof(aBuf), "Can't find player with ID: %d.", SQL_ID);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}

		if (!pSelf->m_apPlayers[SQL_ID]->IsLoggedIn())
		{
			str_format(aBuf, sizeof(aBuf), "Player '%s' is not logged in.", pSelf->Server()->ClientName(SQL_ID));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}

		str_format(aBuf, sizeof(aBuf), "'%s' SQL-ID: %d", pSelf->Server()->ClientName(SQL_ID), pSelf->m_apPlayers[SQL_ID]->GetAccID());
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
	else if (!str_comp_nocase(aCommand, "help"))
	{
		pSelf->SendChatTarget(ClientID, "---- COMMANDS -----");
		pSelf->SendChatTarget(ClientID, "'/sql getid <clientid>' to get sql id");
		pSelf->SendChatTarget(ClientID, "'/sql super_mod <sqlid> <val>'");
		pSelf->SendChatTarget(ClientID, "'/sql mod <sqlid> <val>'");
		pSelf->SendChatTarget(ClientID, "'/sql freeze_acc <sqlid> <val>'");
		pSelf->SendChatTarget(ClientID, "----------------------");
		pSelf->SendChatTarget(ClientID, "'/acc_info <clientID>' additional info");
	}
	else if (!str_comp_nocase(aCommand, "supporter"))
	{
		if (pResult->NumArguments() < 3)
		{
			pSelf->SendChatTarget(ClientID, "Error: sql <command> <id> <value>");
			return;
		}
		int value;
		value = pResult->GetInteger(2);

		pSelf->ExecuteSQLf("UPDATE Accounts SET IsSupporter='%d' WHERE ID='%d'", value, SQL_ID);

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (pSelf->m_apPlayers[i])
			{
				if (pSelf->m_apPlayers[i]->GetAccID() == SQL_ID)
				{
					pSelf->m_apPlayers[i]->m_IsSupporter = value;
					if (value == 1)
					{
						pSelf->SendChatTarget(i, "[ACCOUNT] You are now Supporter.");
					}
					else
					{
						pSelf->SendChatTarget(i, "[ACCOUNT] You are no longer Supporter.");
					}
					str_format(aBuf, sizeof(aBuf), "UPDATED IsSupporter = %d (account is logged in)", value);
					pSelf->SendChatTarget(ClientID, aBuf);
					return;
				}
			}
		}
		str_format(aBuf, sizeof(aBuf), "UPDATED IsSupporter = %d (account is not logged in)", value);
		pSelf->SendChatTarget(ClientID, aBuf);
	}
	else if (!str_comp_nocase(aCommand, "super_mod"))
	{
		if (pResult->NumArguments() < 3)
		{
			pSelf->SendChatTarget(ClientID, "Error: sql <command> <id> <value>");
			return;
		}
		int value;
		value = pResult->GetInteger(2);

		pSelf->ExecuteSQLf("UPDATE Accounts SET IsSuperModerator='%d' WHERE ID='%d'", value, SQL_ID);

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (pSelf->m_apPlayers[i])
			{
				if (pSelf->m_apPlayers[i]->GetAccID() == SQL_ID)
				{
					pSelf->m_apPlayers[i]->m_IsSuperModerator = value;
					if (value == 1)
					{
						pSelf->SendChatTarget(i, "[ACCOUNT] You are now VIP+.");
					}
					else
					{
						pSelf->SendChatTarget(i, "[ACCOUNT] You are no longer VIP+.");
					}
					str_format(aBuf, sizeof(aBuf), "UPDATED IsSuperModerator = %d (account is logged in)", value);
					pSelf->SendChatTarget(ClientID, aBuf);
					return;
				}
			}
		}
		str_format(aBuf, sizeof(aBuf), "UPDATED IsSuperModerator = %d (account is not logged in)", value);
		pSelf->SendChatTarget(ClientID, aBuf);
	}
	else if (!str_comp_nocase(aCommand, "mod"))
	{
		if (pResult->NumArguments() < 3)
		{
			pSelf->SendChatTarget(ClientID, "Error: sql <command> <id> <value>");
			return;
		}
		int value;
		value = pResult->GetInteger(2);

		pSelf->ExecuteSQLf("UPDATE Accounts SET IsModerator='%d' WHERE ID='%d'", value, SQL_ID);

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (pSelf->m_apPlayers[i])
			{
				if (pSelf->m_apPlayers[i]->GetAccID() == SQL_ID)
				{
					pSelf->m_apPlayers[i]->m_IsModerator = value;
					if (value == 1)
					{
						pSelf->SendChatTarget(i, "[ACCOUNT] You are now VIP.");
					}
					else
					{
						pSelf->SendChatTarget(i, "[ACCOUNT] You are no longer VIP.");
					}
					str_format(aBuf, sizeof(aBuf), "UPDATED IsModerator = %d (account is logged in)", value);
					pSelf->SendChatTarget(ClientID, aBuf);
					return;
				}
			}
		}
		str_format(aBuf, sizeof(aBuf), "UPDATED IsModerator = %d (account is not logged in)", value);
		pSelf->SendChatTarget(ClientID, aBuf);
	}
	else if (!str_comp_nocase(aCommand, "freeze_acc"))
	{
		if (pResult->NumArguments() < 3)
		{
			pSelf->SendChatTarget(ClientID, "Error: sql <command> <id> <value>");
			return;
		}
		int value;
		value = pResult->GetInteger(2);

		pSelf->ExecuteSQLf("UPDATE Accounts SET IsAccFrozen='%d' WHERE ID='%d'", value, SQL_ID);

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (pSelf->m_apPlayers[i])
			{
				if (pSelf->m_apPlayers[i]->GetAccID() == SQL_ID)
				{
					pSelf->m_apPlayers[i]->m_IsAccFrozen = value;
					pSelf->m_apPlayers[i]->Logout(); //always logout and send you got frozen also if he gets unfreezed because if some1 gets unfreezed he is not logged in xd
					pSelf->SendChatTarget(i, "Logged out. (Reason: Account frozen)");
					str_format(aBuf, sizeof(aBuf), "UPDATED IsAccFrozen = %d (account is logged in)", value);
					pSelf->SendChatTarget(ClientID, aBuf);
					return;
				}
			}
		}
		str_format(aBuf, sizeof(aBuf), "UPDATED IsAccFrozen = %d (account is not logged in)", value);
		pSelf->SendChatTarget(ClientID, aBuf);
	}
	else 
	{
		pSelf->SendChatTarget(ClientID, "Unknown SQL command. Try '/SQL help' for more help.");
	}

}

void CGameContext::ConAcc_Info(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (g_Config.m_SvAccountStuff == 0)
	{
		pSelf->SendChatTarget(ClientID, "[SQL] Account stuff is turned off.");
		return;
	}


	if (pPlayer->m_Authed != CServer::AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(ClientID, "[SQL] Missing permission.");
		return;
	}

	if (pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(ClientID, "[SQL] Use '/acc_info <name>'.");
		return;
	}

	char aUsername[32];
	int InfoID = -1;
	str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!str_comp_nocase(aUsername, pSelf->Server()->ClientName(i)))
		{
			InfoID = i;
		}
	}

	if (InfoID > -1)
	{
		if (!pSelf->m_apPlayers[InfoID]->IsLoggedIn())
		{
			pSelf->SendChatTarget(ClientID, "[SQL] This player is not logged in.");
			return;
		}

		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "==== '%s' Account Info ====", pSelf->Server()->ClientName(pSelf->m_apPlayers[InfoID]->GetCID()));
		pSelf->SendChatTarget(ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Register date [%s]", pSelf->m_apPlayers[InfoID]->m_aAccountRegDate);
		pSelf->SendChatTarget(ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "==== Username: '%s' SQL: %d ====", pSelf->m_apPlayers[InfoID]->m_aAccountLoginName, pSelf->m_apPlayers[InfoID]->GetAccID());
		pSelf->SendChatTarget(ClientID, aBuf);
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_LastLogoutIGN1);
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_LastLogoutIGN2);
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_LastLogoutIGN3);
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_LastLogoutIGN4);
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_LastLogoutIGN5);
		pSelf->SendChatTarget(ClientID, "======== IP ========");
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_aIP_1);
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_aIP_2);
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_aIP_3);
		pSelf->SendChatTarget(ClientID, "======== Clan ========");
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_aClan1);
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_aClan2);
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_aClan3);
		str_format(aBuf, sizeof(aBuf), "========= Skin '%s' =========", pSelf->m_apPlayers[InfoID]->m_aAccSkin);
		pSelf->SendChatTarget(ClientID, aBuf);
	}
	else
	{
		pSelf->SendChatTarget(ClientID, "[SQL] Unkown player name.");
	}
}

void CGameContext::ConStats(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	int StatsID = ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	char aBuf[512];

	if (pResult->NumArguments() > 0) //other players stats
	{
		char aStatsName[32];
		str_copy(aStatsName, pResult->GetString(0), sizeof(aStatsName));
		StatsID = pSelf->GetCIDByName(aStatsName);
		if (StatsID == -1)
		{
			str_format(aBuf, sizeof(aBuf), "[STATS] Can't find user '%s'", aStatsName);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}
	}

	if (pPlayer->m_IsInstaArena_idm || pPlayer->m_IsInstaArena_gdm || g_Config.m_SvInstagibMode)
		pSelf->ShowInstaStats(ClientID, StatsID);
	else if (pPlayer->m_IsSurvivaling)
		pSelf->ShowSurvivalStats(ClientID, StatsID);
	else // blockcity stats
		pSelf->ShowDDPPStats(ClientID, StatsID);
}

void CGameContext::ConProfile(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "~~~ Profile help ~~~");
		pSelf->SendChatTarget(pResult->m_ClientID, "Profiles are connected to your account.");
		pSelf->SendChatTarget(pResult->m_ClientID, "More info about accounts with '/accountinfo'.");
		pSelf->SendChatTarget(pResult->m_ClientID, "--------------------");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/profile cmdlist' for command list.");
		return;
	}


	char aBuf[512];
	char aPara0[32];
	char aPara1[32];
	str_copy(aPara0, pResult->GetString(0), sizeof(aPara0));
	str_copy(aPara1, pResult->GetString(1), sizeof(aPara1));
	int ViewID = pSelf->GetCIDByName(aPara1);

	if (!str_comp_nocase(aPara0, "help"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "--- Profile help ---");
		pSelf->SendChatTarget(pResult->m_ClientID, "Profiles are connected with your account.");
		pSelf->SendChatTarget(pResult->m_ClientID, "More info about accounts with '/accountinfo'.");
		pSelf->SendChatTarget(pResult->m_ClientID, "--------------------");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/profile cmdlist' for command list.");
	}
	else if (!str_comp_nocase(aPara0, "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "--- Profile Commands ---");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/profile view <playername>' to view a players profile.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/profile style <style>' to change profile style.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/profile status <status> to change status.'");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/profile email <e-mail>' to change e-mail.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/profile homepage <homepage>' to change homepage.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/profile youtube <youtube>' to change youtube.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/profile skype <skype>' to change skype.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/profile twitter <twitter>' to change twitter.");
	}
	else if (!str_comp_nocase(aPara0, "view") || !str_comp_nocase(aPara0, "watch"))
	{
		if (pResult->NumArguments() < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Missing parameters. Stick to struct: 'profile view <playername>'.");
			return;
		}

		if (ViewID == -1)
		{
			str_format(aBuf, sizeof(aBuf), "Can't find user: '%s'", aPara1);
			return;
		}

		pSelf->ShowProfile(pResult->m_ClientID, ViewID);
	}
	else
	{
		if (!pPlayer->IsLoggedIn()) //also gets triggerd on unknown commands but whatever if logged in all works fine
		{
			//pSelf->SendChatTarget(pResult->m_ClientID, "Unknown command or:");
			pSelf->SendChatTarget(pResult->m_ClientID, "You have to be logged in to use this command.");
			pSelf->SendChatTarget(pResult->m_ClientID, "All info about accounts: '/accountinfo'");
			return;
		}
		
		if (!str_comp_nocase(aPara0, "style"))
		{
			if (!str_comp_nocase(aPara1, "default"))
			{
				pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileStyle = 0;
				pSelf->SendChatTarget(pResult->m_ClientID, "Changed profile-style to: default");
			}
			else if (!str_comp_nocase(aPara1, "shit"))
			{
				pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileStyle = 1;
				pSelf->SendChatTarget(pResult->m_ClientID, "Changed profile-style to: shit");
			}
			else if (!str_comp_nocase(aPara1, "social"))
			{
				pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileStyle = 2;
				pSelf->SendChatTarget(pResult->m_ClientID, "Changed profile-style to: social");
			}
			else if (!str_comp_nocase(aPara1, "show-off"))
			{
				pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileStyle = 3;
				pSelf->SendChatTarget(pResult->m_ClientID, "Changed profile-style to: show-off");
			}
			else if (!str_comp_nocase(aPara1, "pvp"))
			{
				pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileStyle = 4;
				pSelf->SendChatTarget(pResult->m_ClientID, "Changed profile-style to: pvp");
			}
			else if (!str_comp_nocase(aPara1, "bomber"))
			{
				pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileStyle = 5;
				pSelf->SendChatTarget(pResult->m_ClientID, "Changed profile-style to: bomber");
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "error: '%s' is not a profile style. Choose between the following: default, shit, social, show-off, pvp, bomber", aPara1);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			}
		}
		else if (!str_comp_nocase(aPara0, "status"))
		{
			if (pResult->NumArguments() < 2)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] status: %s", pPlayer->m_ProfileStatus);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			if (pSelf->IsAllowedCharSet(aPara1) == false)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] please use only the following characters in your status '%s'", pSelf->m_aAllowedCharSet);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			
			str_copy(pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileStatus, aPara1, sizeof(pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileStatus));
			str_format(aBuf, sizeof(aBuf), "Updated your profile status: %s", pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileStatus);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		else if (!str_comp_nocase(aPara0, "skype"))
		{
			if (pResult->NumArguments() < 2)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] skype: %s", pPlayer->m_ProfileSkype);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			if (pSelf->IsAllowedCharSet(aPara1) == false)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] please use only the following characters in your skype '%s'", pSelf->m_aAllowedCharSet);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			
			str_copy(pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileSkype, aPara1, sizeof(pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileSkype));
			str_format(aBuf, sizeof(aBuf), "Updated your profile skype: %s", pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileSkype);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		else if (!str_comp_nocase(aPara0, "youtube"))
		{
			if (pResult->NumArguments() < 2)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] youtube: %s", pPlayer->m_ProfileYoutube);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			if (pSelf->IsAllowedCharSet(aPara1) == false)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] please use only the following characters in your youtube '%s'", pSelf->m_aAllowedCharSet);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			
			str_copy(pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileYoutube, aPara1, sizeof(pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileYoutube));
			str_format(aBuf, sizeof(aBuf), "Updated your profile youtube: %s", pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileYoutube);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		else if (!str_comp_nocase(aPara0, "email") || !str_comp_nocase(aPara0, "e-mail"))
		{
			if (pResult->NumArguments() < 2)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] email: %s", pPlayer->m_ProfileEmail);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			if (pSelf->IsAllowedCharSet(aPara1) == false)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] please use only the following characters in your email '%s'", pSelf->m_aAllowedCharSet);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			
			str_copy(pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileEmail, aPara1, sizeof(pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileEmail));
			str_format(aBuf, sizeof(aBuf), "Updated your profile e-mail: %s", pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileEmail);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		else if (!str_comp_nocase(aPara0, "homepage") || !str_comp_nocase(aPara0, "website"))
		{
			if (pResult->NumArguments() < 2)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] homepage: %s", pPlayer->m_ProfileHomepage);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			if (pSelf->IsAllowedCharSet(aPara1) == false)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] please use only the following characters in your homepage '%s'", pSelf->m_aAllowedCharSet);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}

			str_copy(pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileHomepage, aPara1, sizeof(pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileHomepage));
			str_format(aBuf, sizeof(aBuf), "Updated your profile homepage: %s", pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileHomepage);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		else if (!str_comp_nocase(aPara0, "twitter"))
		{
			if (pResult->NumArguments() < 2)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] twitter: %s", pPlayer->m_ProfileTwitter);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			if (pSelf->IsAllowedCharSet(aPara1) == false)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] please use only the following characters in your twitter '%s'", pSelf->m_aAllowedCharSet);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}

			str_copy(pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileTwitter, aPara1, sizeof(pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileTwitter));
			str_format(aBuf, sizeof(aBuf), "Updated your profile twitter: %s", pSelf->m_apPlayers[pResult->m_ClientID]->m_ProfileTwitter);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Unknow command. '/profile cmdlist' or '/profile help' might help.");
		}
	}
}

void CGameContext::ConLogin(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;
	
	if (g_Config.m_SvAccountStuff == 0)
	{
		pSelf->SendChatTarget(ClientID, "Account stuff is turned off.");
		return;
	}
	if (g_Config.m_SvAccountStuff == 2) //filebased
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] SQLite accounts are turned off. (try '/login2')");
		return;
	}

	if (pPlayer->m_JailTime)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] you can't login in jail.");
		return;
	}

	if (pPlayer->m_PlayerHumanLevel < g_Config.m_SvLoginHumanLevel)
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "[ACCOUNT] your '/human_level' is too low %d/%d to use this command.", pPlayer->m_PlayerHumanLevel, g_Config.m_SvLoginHumanLevel);
		pSelf->SendChatTarget(ClientID, aBuf);
		return;
	}

	NETADDR Addr;
	pSelf->Server()->GetClientAddr(ClientID, &Addr);
	Addr.port = 0;
	int Banned = 0;

	for(int i = 0; i < pSelf->m_NumLoginBans && !Banned; i++)
	{
		if(!net_addr_comp(&Addr, &pSelf->m_aLoginBans[i].m_Addr))
			Banned = (pSelf->m_aLoginBans[i].m_Expire - pSelf->Server()->Tick()) / pSelf->Server()->TickSpeed();
	}

	if (Banned > 0)
	{
		char aBuf[128];
		str_format(aBuf, sizeof aBuf, "[ACCOUNT] you have to wait %d seconds before you can login again.", Banned);
		pSelf->SendChatTarget(ClientID, aBuf);
		return;
	}

	char aUsername[32];
	char aPassword[MAX_PW_LEN+1];

	if (pResult->NumArguments() == 1)
	{
		str_copy(aUsername, pSelf->Server()->ClientName(ClientID), sizeof(aUsername));
		str_copy(aPassword, pResult->GetString(0), sizeof(aPassword));
		//char aBuf[128];
		//pSelf->SendChatTarget(ClientID, aBuf);
		//str_format(aBuf, sizeof(aBuf), "[ACCOUNT] WARNING no username given. (trying '%s')", aUsername);
	}
	else if (pResult->NumArguments() == 2)
	{
		str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
		str_copy(aPassword, pResult->GetString(1), sizeof(aPassword));
	}
	else
	{
		pSelf->SendChatTarget(ClientID, "Use '/login <name> <password>'.");
		pSelf->SendChatTarget(ClientID, "Use '/accountinfo' for help.");
		return;
	}

	if (pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(ClientID, "You are already logged in.");
		return;
	}

	if (str_length(aUsername) > MAX_PW_LEN || str_length(aUsername) < MIN_PW_LEN)
	{
		pSelf->SendChatTarget(ClientID, "Username is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if (str_length(aPassword) > MAX_PW_LEN || str_length(aPassword) < MIN_PW_LEN)
	{
		pSelf->SendChatTarget(ClientID, "Password is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if (g_Config.m_SvSaveWrongLogin)
	{
		str_format(pPlayer->m_aWrongLogin, sizeof(pPlayer->m_aWrongLogin), "[%s] '%s' '%s'", pSelf->Server()->ClientName(pResult->m_ClientID), aUsername, aPassword);
	}

	pSelf->SQLaccount(pSelf->SQL_LOGIN, ClientID, aUsername, aPassword);
	// pSelf->SQLaccount(pSelf->SQL_LOGIN_THREADED, ClientID, aUsername, aPassword);
}

void CGameContext::ConChangePassword(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (!pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You are not logged in. (More info '/accountinfo')");
		return;
	}
	if (pResult->NumArguments() != 3)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Please use: '/changepassword <oldpw> <newpw> <repeat_newpw>'");
		return;
	}

	char aOldPass[MAX_PW_LEN+1];
	char aNewPass[MAX_PW_LEN+1];
	char aNewPass2[MAX_PW_LEN+1];
	str_copy(aOldPass, pResult->GetString(0), sizeof(aOldPass));
	str_copy(aNewPass, pResult->GetString(1), sizeof(aNewPass));
	str_copy(aNewPass2, pResult->GetString(2), sizeof(aNewPass2));

	if (str_length(aOldPass) > MAX_PW_LEN || str_length(aOldPass) < MIN_PW_LEN)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Your old password is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if ((str_length(aNewPass) > MAX_PW_LEN || str_length(aNewPass) < MIN_PW_LEN) || (str_length(aNewPass2) > MAX_PW_LEN || str_length(aNewPass2) < MIN_PW_LEN))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Your password is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if (str_comp_nocase(aNewPass, aNewPass2) != 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Passwords have to be identical.");
		return;
	}

	str_format(pPlayer->m_aChangePassword, sizeof(pPlayer->m_aChangePassword), "%s", aNewPass); 
	pSelf->SQLaccount(pSelf->SQL_CHANGE_PASSWORD, pResult->m_ClientID, pPlayer->m_aAccountLoginName, aOldPass);
}

void CGameContext::ConAccLogout(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (g_Config.m_SvAccountStuff == 0)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Account stuff is turned off.");
		return;
	}
	/*
	if (g_Config.m_SvAccountStuff == 2) //filebased
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] SQLite accounts are turned off.");
		return;
	}
	*/

	CCharacter* pChr = pPlayer->GetCharacter();
	if (pChr)
	{
		if (pChr->m_FreezeTime)
		{
			pSelf->SendChatTarget(ClientID, "[ACCOUNT] you can't logout while being frozen.");
			return;
		}
	}

	if (pPlayer->m_JailTime)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] you can't logout in jail.");
		return;
	}

	if (!pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] You are not logged in.");
		return;
	}

	if (pPlayer->m_Insta1on1_id != -1)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] You can't logout in 1on1. ('/insta leave' to leave)");
		return;
	}

	if (pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] You can't logout in insta matches. ('/insta leave' to leave)");
		return;
	}

	if (pSelf->IsMinigame(pResult->m_ClientID) > 0) //all minigames no jail returns bigger than zero
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] You can't logout during minigames try '/leave'");
		return;
	}

	if (pPlayer->GetCharacter())
	{
		if (pPlayer->GetCharacter()->m_IsBombing) 
		{
			pSelf->SendChatTarget(ClientID, "[ACCOUNT] You can't logout in bomb games. ('/bomb leave' to leave)");
			return;
		}
		if (pPlayer->GetCharacter()->m_IsPVParena)
		{
			pSelf->SendChatTarget(ClientID, "[ACCOUNT] You can't logout in pvp_arena. ('/pvp_arena leave' to leave)");
			return;
		}
	}

	pPlayer->Logout();
	pSelf->SendChatTarget(ClientID, "[ACCOUNT] Logged out.");
}

void CGameContext::ConChidraqul(IConsole::IResult * pResult, void * pUserData)
{

	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	char aCommand[64];
	str_copy(aCommand, pResult->GetString(0), sizeof(aCommand));

	if (!str_comp_nocase(aCommand, "info") || !str_comp_nocase(aCommand, "help"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "==== chidraqul3 ====");
		pSelf->SendChatTarget(pResult->m_ClientID, "The chidraqul minigame in his third generation.");
		pSelf->SendChatTarget(pResult->m_ClientID, "Buy the game in '/shop' with '/buy chidraqul' and you can use it until disconnect.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/chidraqul cmdlist' for a list of all commands");
	}
	else if (!str_comp_nocase(aCommand, "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== chirdraqul3 commands ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/chidraqul start' to start the game");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/chidraqul stop' to stop the game");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/chidraqul r' to move right");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/chidraqul l' to move left");
	}
	else if (!str_comp_nocase(aCommand, "start"))
	{
		if (pPlayer->m_BoughtGame)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[chidraqul] started.");
			str_format(pPlayer->m_HashSkin, sizeof(pPlayer->m_HashSkin), "%s", g_Config.m_SvChidraqulDefaultSkin);
			pPlayer->m_C3_GameState = 1; //singleplayer
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You don't have this game. You can buy it with '/buy chidraqul'");
		}
	}
	else if (!str_comp_nocase(aCommand, "stop") || !str_comp_nocase(aCommand, "quit"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[chidraqul] stopped.");
		pSelf->m_apPlayers[pResult->m_ClientID]->m_C3_GameState = false;
		pSelf->SendBroadcast(" ", pResult->m_ClientID);
	}
	else if (!str_comp_nocase(aCommand, "r"))
	{
		if (pPlayer->m_HashPos < g_Config.m_SvChidraqulWorldX - 1) //space for the string delimiter
		{
			pPlayer->m_HashPos++;
			pPlayer->m_C3_UpdateFrame = true;
		}
	}
	else if (!str_comp_nocase(aCommand, "l"))
	{
		if (pPlayer->m_HashPos > 0)
		{
			pPlayer->m_HashPos--;
			pPlayer->m_C3_UpdateFrame = true;
		}
	}
	else if (!str_comp_nocase(aCommand, "multiplayer"))
	{
		if (!pPlayer->m_BoughtGame)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You don't have this game. You can buy it with '/buy chidraqul'");
			return;
		}

		pPlayer->JoinMultiplayer();
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[chidraqul] Unknown command. try '/chidraqul help'");
	}
}

void CGameContext::ConMinigames(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You have to be ingame to use this command.");
		return;
	}

	if (pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== MINIGAMES ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "This command gives you overview");
		pSelf->SendChatTarget(pResult->m_ClientID, "off all minigames!");
		pSelf->SendChatTarget(pResult->m_ClientID, "Btw we also have quests. Check '/quest'.");
		pSelf->SendChatTarget(pResult->m_ClientID, "");
		pSelf->SendChatTarget(pResult->m_ClientID, "check '/minigames cmdlist' for all commands");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== MINIGAMES COMMANDS ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/minigames status' gives life minigame status");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/minigames list' lists all minigames");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/minigames info' shows some info about the command");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "list"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=========================");
		pSelf->SendChatTarget(pResult->m_ClientID, "===== LIST OF ALL MINIGAMES =====");
		pSelf->SendChatTarget(pResult->m_ClientID, "=========================");
		pSelf->SendChatTarget(pResult->m_ClientID, "GAME:             COMMAND:");
		pSelf->SendChatTarget(pResult->m_ClientID, "[INSTAGIB]        '/insta'"); 			// 1=grenade 2=rifle
		pSelf->SendChatTarget(pResult->m_ClientID, "[BALANCE]        '/balance'"); 			// 3
		pSelf->SendChatTarget(pResult->m_ClientID, "[SURVIVAL]       '/survival'");			// 4
		pSelf->SendChatTarget(pResult->m_ClientID, "[BOMB]             '/bomb'");			// 5
		pSelf->SendChatTarget(pResult->m_ClientID, "[PVP]                 '/pvp_arena'");	// 6
		pSelf->SendChatTarget(pResult->m_ClientID, "[BLOCKWAVE]   '/blockwave'");			// 7
		// block tournament										/join						// 8
		pSelf->SendChatTarget(pResult->m_ClientID, "[BLOCK]            '/block'");			// 9
	}
	else if (!str_comp_nocase(pResult->GetString(0), "status"))
	{
		int gameID = pSelf->IsMinigame(pResult->m_ClientID);

		pSelf->SendChatTarget(pResult->m_ClientID, "===== MINIGAME STATUS =====");

		if (!gameID)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You aren't currently minigaming.");
		}
		else if (gameID == -1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You are not allowed to play mingames.");
			pSelf->SendChatTarget(pResult->m_ClientID, "because you are jailed.");
		}
		else if (gameID == 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTAGIB] gdm (check '/insta' for more info)");
		}
		else if (gameID == 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTAGIB] idm (check '/insta' for more info)");
		}
		else if (gameID == 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BALANCE] (check '/balance' for more info)");
		}
		else if (gameID == 4)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[SURVIVAL] (check '/survival' for more info)");
		}
		else if (gameID == 5)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BOMB] (check '/bomb' for more info)");
		}
		else if (gameID == 6)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[PVP] (check '/pvp_arena' for more info)");
		}
		else if (gameID == 7)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BLOCKWAVE] (check '/blockwave' for more info)");
		}
		else if (gameID == 8)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[EVENT] Block (check '/event' for more info)");
		}
		else if (gameID == 9)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BLOCK] '/block' to join");
			pSelf->SendChatTarget(pResult->m_ClientID, "[BLOCK] '/leave' to leave block deathmatch");
		}
		else 
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[UNKNOWN] you are playing an unknown game.");
		}

		pSelf->SendChatTarget(pResult->m_ClientID, "======================");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Unknown minigames argument (Check '/minigames cmdlist').");
	}
}

void CGameContext::ConCC(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (pPlayer->m_Authed == CServer::AUTHED_ADMIN)
	{

		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'namless rofl' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'(1)namless tee' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'einFISCH' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'GAGAGA' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'Steve-' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'Gro�erDBC' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'cB | Bashcord' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'LIVUS BAGGUGE' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'BoByBANK' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'noobson tnP' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'vali' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'ChiliDreghugn' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'Stahkilla' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'Detztin' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'pikutee <3' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'namless tee' has left the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'BoByBANK' has left the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'Ubu' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'Magnet' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'Jambi*' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'HurricaneZ' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'Sonix' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'darkdragonovernoob' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'Ubu' has left the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'deen' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'fuck me soon' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'fik you!' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'fuckmeson' has left the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'(1)' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'Noved' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'Aoe' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'artkis' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'namless brain' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'(1)ChillerDrago' has left the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'HurricaneZ' has left the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'(2)ChillerDrago' has left the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'hax0r' has left the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'(3)ChillerDrago' has left the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'SarKro' has left the game");
	}
	else
	{
		//pSelf->SendChatTarget(pResult->m_ClientID, "No such command: %s.");
		pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
	}
}

void CGameContext::ConBalance(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[BALANCE] You have to be ingame to use this command.");
		return;
	}


	if (!g_Config.m_SvAllowBalance)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[BALANCE] this command is deactivated by an administrator");
		return;
	}


	char aBuf[128];

	if (pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== BALANCE HELP ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "Battle friends in a tee balance 1on1");
		pSelf->SendChatTarget(pResult->m_ClientID, "check '/balance cmdlist' for a list of all commands");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== INSTAGIB COMMANDS ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/balance battle <player>' to invite a player");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/balance accept <player>' to accept a battle");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/balance help' to show some help and info");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "battle"))
	{
		vec2 BalanceBattleSpawn1 = pSelf->Collision()->GetRandomTile(TILE_BALANCE_BATTLE_1);
		vec2 BalanceBattleSpawn2 = pSelf->Collision()->GetRandomTile(TILE_BALANCE_BATTLE_2);
		int mateID = pSelf->GetCIDByName(pResult->GetString(1));
		if (mateID == -1)
		{
			str_format(aBuf, sizeof(aBuf), "[BALANCE] Can't find the user '%s'", pResult->GetString(1));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}
		else if (mateID == pResult->m_ClientID)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BALANCE] You can't invite your self.");
			return;
		}
		else if (BalanceBattleSpawn1 == vec2(-1, -1) || BalanceBattleSpawn2 == vec2(-1, -1))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BALANCE] no battle arena found.");
			return;
		}
		else if (pSelf->IsMinigame(pResult->m_ClientID))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Error: maybe you are already in a minigame or jail. (check '/minigames status')");
		}
		//else if (!pSelf->m_apPlayers[mateID]->IsLoggedIn())
		//{
		//	pSelf->SendChatTarget(pResult->m_ClientID, "This player is not logged in.");
		//	return;
		//}
		//else if (pPlayer->GetMoney() < 10)
		//{
		//	pSelf->SendChatTarget(pResult->m_ClientID, "You don't have 10 money to start a game.");
		//	return;
		//}
		else
		{
			pPlayer->m_BalanceBattle_id = mateID;
			str_format(aBuf, sizeof(aBuf), "[BALANCE] Invited '%s' to a battle", pResult->GetString(1));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

			str_format(aBuf, sizeof(aBuf), "[BALANCE] '%s' invited you to a battle.", pSelf->Server()->ClientName(pResult->m_ClientID));
			pSelf->SendChatTarget(mateID, aBuf);
			str_format(aBuf, sizeof(aBuf), "('/balance accept %s' to accept)", pSelf->Server()->ClientName(pResult->m_ClientID));
			pSelf->SendChatTarget(mateID, aBuf);
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "accept")) 
	{ 
		int mateID = pSelf->GetCIDByName(pResult->GetString(1));
		if (mateID == -1)
		{
			str_format(aBuf, sizeof(aBuf), "[BALANCE] Can't find the user '%s'", pResult->GetString(1));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}
		else if (pSelf->m_apPlayers[mateID]->m_BalanceBattle_id != pResult->m_ClientID)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BALANCE] This player didn't invite you.");
			return;
		}
		else if (pSelf->IsMinigame(pResult->m_ClientID))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Error: maybe you are already in a minigame or jail. (check '/minigames status')");
		}
		else
		{
			pSelf->StartBalanceBattle(mateID, pResult->m_ClientID);
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[BALANCE] Unknown paramter. Check '/balance cmdlist' for all commands.");
	}
}

void CGameContext::ConInsta(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (!g_Config.m_SvAllowInsta)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] this command is deactivated by an administator.");
		return;
	}

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You have to be ingame to use this command.");
		return;
	}

	if (!pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You are not logged in. Use  '/accountinfo' or more info.");
		return;
	}

	char aBuf[256];

	if (pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== INSTAGIB HELP ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "Kill people with one shot in a special arena.");
		pSelf->SendChatTarget(pResult->m_ClientID, "Check '/insta cmdlist' for a list of all commands.");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== INSTAGIB COMMANDS ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/insta leave' to leave any kind of instagib game");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/insta gdm' to join grenade deathmatch instagib game");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/insta boomfng' to join grenade fng game");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/insta idm' to join rifle deathmatch instagib game"); 
		pSelf->SendChatTarget(pResult->m_ClientID, "'/insta fng' to join rifle fng game");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/insta 1on1 <mode> <player>' to 1on1 <player> (+100 money for the winner)");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/insta stats' to show game statistics");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/insta help' for help and info");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "leave"))
	{
		pSelf->LeaveInstagib(pResult->m_ClientID);
	}
	else if (!str_comp_nocase(pResult->GetString(0), "stats"))
	{
		pSelf->ShowInstaStats(pResult->m_ClientID, pResult->m_ClientID);
	}
	else if (!str_comp_nocase(pResult->GetString(0), "gdm"))
	{
		if (pPlayer->m_IsInstaArena_gdm)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You are already in a grenade game. ('/insta leave' to leave)");
		}
		else if (pPlayer->m_IsInstaArena_idm)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You are already in a rifle game. ('/insta leave' to leave)");
		}
		else if (pSelf->IsMinigame(pResult->m_ClientID))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
		}
		else if (!pSelf->CanJoinInstaArena(true, false))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Arena is full.");
		}
		else if (g_Config.m_SvAllowGrenade == 2 || g_Config.m_SvAllowGrenade == 0)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] gdm is currently deactivated by an admin.");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You joined a grenade game.");
			pSelf->JoinInstagib(4, false, pResult->m_ClientID);
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "idm"))
	{
		if (pPlayer->m_IsInstaArena_gdm)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You are already in a grenade game. ('/insta leave' to leave)");
		}
		else if (pPlayer->m_IsInstaArena_idm)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You are already in a rifle game. ('/insta leave' to leave)");
		}
		else if (pSelf->IsMinigame(pResult->m_ClientID))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
		}
		else if (!pSelf->CanJoinInstaArena(false, false))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Arena is full.");
		}
		else if (g_Config.m_SvAllowRifle == 2 || g_Config.m_SvAllowRifle == 0)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] idm is currently deactivated by an admin.");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You joined a rifle game.");
			pSelf->JoinInstagib(5, false, pResult->m_ClientID);
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "boomfng"))
	{
		if (pPlayer->m_IsInstaArena_gdm)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You are already in a grenade game. ('/insta leave' to leave)");
		}
		else if (pPlayer->m_IsInstaArena_idm)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You are already in a rifle game. ('/insta leave' to leave)");
		}
		else if (pSelf->IsMinigame(pResult->m_ClientID))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
		}
		else if (!pSelf->CanJoinInstaArena(true, false))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Arena is full.");
		}
		else if (g_Config.m_SvAllowGrenade == 0 || g_Config.m_SvAllowGrenade == 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] boomfng is currently deactivated by an admin.");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You joined a boomfng game.");
			pSelf->JoinInstagib(4, true, pResult->m_ClientID);
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "fng"))
	{
		if (pPlayer->m_IsInstaArena_gdm)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You are already in a grenade game. ('/insta leave' to leave)");
		}
		else if (pPlayer->m_IsInstaArena_idm)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You are already in a rifle game. ('/insta leave' to leave)");
		}
		else if (pSelf->IsMinigame(pResult->m_ClientID))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
		}
		else if (!pSelf->CanJoinInstaArena(false, false))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Arena is full.");
		}
		else if (g_Config.m_SvAllowRifle == 0 || g_Config.m_SvAllowRifle == 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] fng is currently deactivated by an admin.");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You joined a fng game.");
			pSelf->JoinInstagib(5, true, pResult->m_ClientID);
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "1on1"))
	{
		if (pResult->NumArguments() == 1 || !str_comp_nocase(pResult->GetString(1), "help") || !str_comp_nocase(pResult->GetString(1), "info"))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "=== Insta 1on1 ===");
			pSelf->SendChatTarget(pResult->m_ClientID, "Battle 1 vs 1 in gdm, idm, fng or boomfng.");
			pSelf->SendChatTarget(pResult->m_ClientID, "The winner gets 100 money from the looser.");
			pSelf->SendChatTarget(pResult->m_ClientID, "=== commands ===");
			pSelf->SendChatTarget(pResult->m_ClientID, "'/insta 1on1 gdm <player>'");
			pSelf->SendChatTarget(pResult->m_ClientID, "'/insta 1on1 idm <player>'");
			pSelf->SendChatTarget(pResult->m_ClientID, "'/insta 1on1 boomfng <player>'");
			pSelf->SendChatTarget(pResult->m_ClientID, "'/insta 1on1 fng <player>'");
			pSelf->SendChatTarget(pResult->m_ClientID, "'/insta 1on1 accept <player>'");

			//description is too long and goes newline --> lukz ugly af

			//pSelf->SendChatTarget(pResult->m_ClientID, "'/insta 1on1 gdm <player>' to send a gdm 1on1 request to <player>");
			//pSelf->SendChatTarget(pResult->m_ClientID, "'/insta 1on1 idm <player>' to send a idm 1on1 request to <player>");
			//pSelf->SendChatTarget(pResult->m_ClientID, "'/insta 1on1 boomfng <player>' to send a boomfng 1on1 request to <player>");
			//pSelf->SendChatTarget(pResult->m_ClientID, "'/insta 1on1 fng <player>' to send a fng 1on1 request to <player>");
			//pSelf->SendChatTarget(pResult->m_ClientID, "'/insta 1on1 accept <player>' to accept <player>'s 1on1 request");
		}
		else if (!str_comp_nocase(pResult->GetString(1), "gdm"))
		{
			int mateID = pSelf->GetCIDByName(pResult->GetString(2));
			if (mateID == -1)
			{
				str_format(aBuf, sizeof(aBuf), "[INSTA] Can't find playername: '%s'.", pResult->GetString(2));
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			else if (pSelf->IsMinigame(pResult->m_ClientID))
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			}
			else if (mateID == pResult->m_ClientID)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You can't invite yourself.");
				return;
			}
			else if (!pSelf->m_apPlayers[mateID]->IsLoggedIn())
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] This player is not logged in.");
				return;
			}
			else if (pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You can't invite while being ingame. Do '/insta leave' first.");
				return;
			}
			else if (pPlayer->GetMoney() < 100)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You need at least 100 money to start a game.");
				return;
			}
			
			pPlayer->m_Insta1on1_id = mateID; //set this id to -1 if you join any kind of insta game which is not 1on1
			pPlayer->m_Insta1on1_mode = 0; //gdm
			str_format(aBuf, sizeof(aBuf), "[INSTA] Invited '%s' to a gdm 1on1.", pResult->GetString(2));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

			str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' invited you to a gdm 1on1. ('/insta 1on1 accept %s' to accept)", pSelf->Server()->ClientName(pResult->m_ClientID), pSelf->Server()->ClientName(pResult->m_ClientID));
			pSelf->SendChatTarget(mateID, aBuf);
		}
		else if (!str_comp_nocase(pResult->GetString(1), "idm"))
		{
			int mateID = pSelf->GetCIDByName(pResult->GetString(2));
			if (mateID == -1)
			{
				str_format(aBuf, sizeof(aBuf), "[INSTA] Can't find playername: '%s'.", pResult->GetString(2));
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			else if (pSelf->IsMinigame(pResult->m_ClientID))
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			}
			else if (mateID == pResult->m_ClientID)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You can't invite yourself.");
				return;
			}
			else if (!pSelf->m_apPlayers[mateID]->IsLoggedIn())
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] This player is not logged in.");
				return;
			}
			else if (pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You can't invite while being ingame. Do '/insta leave' first.");
				return;
			}
			else if (pPlayer->GetMoney() < 100)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You need at least 100 money to start a game.");
				return;
			}

			pPlayer->m_Insta1on1_id = mateID; //set this id to -1 if you join any kind of insta game which is not 1on1
			pPlayer->m_Insta1on1_mode = 1; //idm
			str_format(aBuf, sizeof(aBuf), "[INSTA] Invited '%s' to a idm 1on1.", pResult->GetString(2));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

			str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' invited you to a idm 1on1. ('/insta 1on1 accept %s' to accept)", pSelf->Server()->ClientName(pResult->m_ClientID), pSelf->Server()->ClientName(pResult->m_ClientID));
			pSelf->SendChatTarget(mateID, aBuf);
		}
		else if (!str_comp_nocase(pResult->GetString(1), "boomfng"))
		{
			int mateID = pSelf->GetCIDByName(pResult->GetString(2));
			if (mateID == -1)
			{
				str_format(aBuf, sizeof(aBuf), "[INSTA] Can't find playername: '%s'.", pResult->GetString(2));
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			else if (pSelf->IsMinigame(pResult->m_ClientID))
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			}
			else if (mateID == pResult->m_ClientID)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You can't invite yourself.");
				return;
			}
			else if (!pSelf->m_apPlayers[mateID]->IsLoggedIn())
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] This player is not logged in.");
				return;
			}
			else if (pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You can't invite while being ingame. Do '/insta leave' first.");
				return;
			}
			else if (pPlayer->GetMoney() < 100)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You need at least 100 money to start a game.");
				return;
			}

			pPlayer->m_Insta1on1_id = mateID; //set this id to -1 if you join any kind of insta game which is not 1on1
			pPlayer->m_Insta1on1_mode = 2; //boomfng
			str_format(aBuf, sizeof(aBuf), "[INSTA] Invited '%s' to a boomfng 1on1.", pResult->GetString(2));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

			str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' invited you to a boomfng 1on1. ('/insta 1on1 accept %s' to accept)", pSelf->Server()->ClientName(pResult->m_ClientID), pSelf->Server()->ClientName(pResult->m_ClientID));
			pSelf->SendChatTarget(mateID, aBuf);
		}
		else if (!str_comp_nocase(pResult->GetString(1), "fng"))
		{
			int mateID = pSelf->GetCIDByName(pResult->GetString(2));
			if (mateID == -1)
			{
				str_format(aBuf, sizeof(aBuf), "[INSTA] Can't find playername: '%s'.", pResult->GetString(2));
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			else if (pSelf->IsMinigame(pResult->m_ClientID))
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			}
			else if (mateID == pResult->m_ClientID)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You can't invite yourself.");
				return;
			}
			else if (!pSelf->m_apPlayers[mateID]->IsLoggedIn())
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] This player is not logged in.");
				return;
			}
			else if (pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You can't invite while being ingame. Do '/insta leave' first.");
				return;
			}
			else if (pPlayer->GetMoney() < 100)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You don't have 100 money to start a game.");
				return;
			}

			pPlayer->m_Insta1on1_id = mateID; //set this id to -1 if you join any kind of insta game which is not 1on1
			pPlayer->m_Insta1on1_mode = 3; //fng
			str_format(aBuf, sizeof(aBuf), "[INSTA] Invited '%s' to a fng 1on1.", pResult->GetString(2));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

			str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' invited you to a fng 1on1. ('/insta 1on1 accept %s' to accept)", pSelf->Server()->ClientName(pResult->m_ClientID), pSelf->Server()->ClientName(pResult->m_ClientID));
			pSelf->SendChatTarget(mateID, aBuf);
		}
		else if (!str_comp_nocase(pResult->GetString(1), "accept"))
		{
			int mateID = pSelf->GetCIDByName(pResult->GetString(2));
			if (mateID == -1)
			{
				str_format(aBuf, sizeof(aBuf), "[INSTA] Can't find playername: '%s'.", pResult->GetString(2));
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			}
			else if (pSelf->IsMinigame(pResult->m_ClientID))
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			}
			else if (pSelf->m_apPlayers[mateID]->m_Insta1on1_id != pResult->m_ClientID)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] This player didn't invite you.");
			}
			else if (pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You can't accept while being ingame. Do '/insta leave' first.");
				return;
			}
			else if (pPlayer->GetMoney() < 100)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You need at least 100 money to start a game.");
				return;
			}
			else if (pSelf->m_apPlayers[mateID]->GetMoney() < 100)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Your 1on1 mate doesn't have enough money to start a game.");
				return;
			}
			else
			{
				if (pSelf->m_apPlayers[mateID]->m_Insta1on1_mode == 0 || pSelf->m_apPlayers[mateID]->m_Insta1on1_mode == 2) //grenade
				{
					if (!pSelf->CanJoinInstaArena(true, true))
					{
						pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA]Arena is too full to start 1on1.");
					}
					else //everything succeded! yay --> start 1on1
					{
						if (pSelf->m_apPlayers[mateID]->m_Insta1on1_mode == 0)
						{
							pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You joined a gdm 1on1 (-100 money)");
							pSelf->SendChatTarget(mateID, "[INSTA] You joined a gdm 1on1 (-100 money)");
						}
						else
						{
							pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You joined a boomfng 1on1 (-100 money)");
							pPlayer->m_IsInstaArena_fng = true;

							pSelf->SendChatTarget(mateID, "[INSTA] You joined a boomfng 1on1 (-100 money)");
							pSelf->m_apPlayers[mateID]->m_IsInstaArena_fng = true;
						}

						pSelf->m_apPlayers[mateID]->m_IsInstaArena_gdm = true;
						pSelf->m_apPlayers[mateID]->m_Insta1on1_score = 0;
						pSelf->m_apPlayers[mateID]->MoneyTransaction(-100, "join insta 1on1");
						pSelf->m_apPlayers[mateID]->GetCharacter()->Die(mateID, WEAPON_SELF);

						pPlayer->m_IsInstaArena_gdm = true;
						pPlayer->m_Insta1on1_score = 0;
						pPlayer->m_Insta1on1_id = mateID;
						pPlayer->MoneyTransaction(-100, "join insta 1on1");
						pChr->Die(pPlayer->GetCID(), WEAPON_SELF);
					}
				}
				else //rifle
				{
					if (!pSelf->CanJoinInstaArena(false, true))
					{
						pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Arena is too full to start 1on1.");
					}
					else //everything succeded! yay --> start 1on1
					{
						if (pSelf->m_apPlayers[mateID]->m_Insta1on1_mode == 1)
						{
							pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You joined a idm 1on1 (-100 money)");
							pSelf->SendChatTarget(mateID, "[INSTA] You joined a idm 1on1 (-100 money)");
						}
						else
						{
							pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] You joined a fng 1on1 (-100 money)");
							pPlayer->m_IsInstaArena_fng = true;

							pSelf->SendChatTarget(mateID, "[INSTA] You joined a fng 1on1 (-100 money)");
							pSelf->m_apPlayers[mateID]->m_IsInstaArena_fng = true;
						}

						pSelf->m_apPlayers[mateID]->m_IsInstaArena_idm = true;
						pSelf->m_apPlayers[mateID]->m_Insta1on1_score = 0;
						pSelf->m_apPlayers[mateID]->MoneyTransaction(-100, "join insta 1on1");
						pSelf->m_apPlayers[mateID]->GetCharacter()->Die(mateID, WEAPON_SELF);

						pPlayer->m_IsInstaArena_idm = true;
						pPlayer->m_Insta1on1_score = 0;
						pPlayer->m_Insta1on1_id = mateID;
						pPlayer->MoneyTransaction(-100, "join insta 1on1");
						pChr->Die(pPlayer->GetCID(), WEAPON_SELF);
					}
				}
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Unknown 1on1 parameter. Check '/insta 1on1 help' for more help");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[INSTA] Unknown parameter. Check '/insta cmdlist' for all commands");
	}
}

void CGameContext::ConJoin(IConsole::IResult * pResult, void * pUserData) //this command joins the currently running event... for now only Block tournaments
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[JOIN] you have to be alive to use this command");
		return;
	}




	/***********************************
	*                                  *
	*          BLOCK TOURNAMENT        *
	*                                  *
	************************************/

	if (!g_Config.m_SvAllowBlockTourna)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[JOIN] Block tournaments are deactivated by an admin.");
		return;
	}
	else if (pPlayer->m_IsBlockTourning)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[JOIN] You already joined the block tournament.");
		return;
	}
	else if (pSelf->IsMinigame(pResult->m_ClientID))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[JOIN] This command is not allowed in jail or minigames. try '/leave' first.");
		return;
	}
	else if (g_Config.m_SvAllowBlockTourna == 2 && !pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[JOIN] You have to be logged in to join block tournaments.");
		return;
	}
	else if (pSelf->m_BlockTournaState == 2)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[JOIN] Block tournament is already running please wait until its finished.");
		return;
	}
	else if (pSelf->m_BlockTournaState == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[JOIN] No block tournament running.");
		return;

		//pSelf->SendChatTarget(pResult->m_ClientID, "[JOIN] you started a block tournament.");
		//pPlayer->m_IsBlockTourning = true;
		//pSelf->m_BlockTournaState = 1;
		//pSelf->m_BlockTournaLobbyTick = g_Config.m_SvBlockTournaDelay * pSelf->Server()->TickSpeed();
		//return;
	}
	else if (pSelf->m_BlockTournaState == 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[JOIN] You joined a block tournament.");
		pPlayer->m_IsBlockTourning = true;
		return;
	}
}

void CGameContext::ConBlock(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[BLOCK] you have to be alive to use this command");
		return;
	}

	if (pPlayer->m_IsBlockDeathmatch)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[BLOCK] you are already in deathmatch mode.");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[BLOCK] you joined the deathmatch arena!");
		pSelf->SendChatTarget(pResult->m_ClientID, "[BLOCK] type /leave to leave");
		pChr->Die(pPlayer->GetCID(), WEAPON_SELF);
		pPlayer->m_IsBlockDeathmatch = true;
	}
}

void CGameContext::ConPvpArena(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[PVP] You have to be alive to use this command.");
		return;
	}

	if (pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[PVP] Invalid. Type '/pvp_arena <join/leave>'.");
		return;
	}


	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if (!str_comp_nocase(aInput, "join"))
	{
		if (pSelf->IsMinigame(pResult->m_ClientID))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[PVP] You can't join becasue your are in another mingame or jail (check '/minigames status')");
			return;
		}
		if (!g_Config.m_SvPvpArenaState)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[PVP] No pvp-arena found.");
			return;
		}
		if (pPlayer->m_pvp_arena_tickets < 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[PVP] You don't have a ticket. Buy a ticket first with '/buy pvp_arena_ticket'");
			return;
		}
		if (pPlayer->GetCharacter()->m_IsPVParena)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[PVP] You are already in the PvP-arena");
			return;
		}
		//save position before tele
		//pPlayer->m_PVP_return_posX = pChr->GetPosition().x;
		//pPlayer->m_PVP_return_posY = pChr->GetPosition().y;
		pPlayer->m_PVP_return_pos = pChr->GetPosition();


		if (g_Config.m_SvPvpArenaState == 3) //tilebased tele to spawns
		{
			vec2 PvPArenaSpawnTile = pSelf->Collision()->GetRandomTile(TILE_PVP_ARENA_SPAWN);

			if (PvPArenaSpawnTile != vec2(-1, -1))
			{
				pSelf->m_apPlayers[pResult->m_ClientID]->GetCharacter()->SetPosition(PvPArenaSpawnTile);
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[PVP] error, this map has no arena!");
				return;
			}
		}

		pSelf->SendChatTarget(pResult->m_ClientID, "[PVP] Teleport request sent. Don't move for 4 seconds.");
		pPlayer->GetCharacter()->m_pvp_arena_tele_request_time = pSelf->Server()->TickSpeed() * 4;
		pPlayer->GetCharacter()->m_pvp_arena_exit_request = false; // join request
	}
	else if (!str_comp_nocase(aInput, "leave"))
	{
		if (pPlayer->GetCharacter()->m_IsPVParena)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[PVP] Teleport request sent. Don't move for 6 seconds.");
			pPlayer->GetCharacter()->m_pvp_arena_tele_request_time = pSelf->Server()->TickSpeed() * 6;
			pPlayer->GetCharacter()->m_pvp_arena_exit_request = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[PVP] You are not in an arena.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[PVP] Invalid. Type '/pvp_arena <join/leave>'.");
	}
}

void CGameContext::ConMoney(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	char aBuf[256];

	pSelf->SendChatTarget(pResult->m_ClientID, "~~~~~~~~~~");
	str_format(aBuf, sizeof(aBuf), "Money: %d", pPlayer->GetMoney());
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	pSelf->SendChatTarget(pResult->m_ClientID, "~~~~~~~~~~");
	pSelf->SendChatTarget(pResult->m_ClientID, pPlayer->m_money_transaction0);
	pSelf->SendChatTarget(pResult->m_ClientID, pPlayer->m_money_transaction1);
	pSelf->SendChatTarget(pResult->m_ClientID, pPlayer->m_money_transaction2);
	pSelf->SendChatTarget(pResult->m_ClientID, pPlayer->m_money_transaction3);
	pSelf->SendChatTarget(pResult->m_ClientID, pPlayer->m_money_transaction4);
	pSelf->SendChatTarget(pResult->m_ClientID, pPlayer->m_money_transaction5);
	pSelf->SendChatTarget(pResult->m_ClientID, pPlayer->m_money_transaction6);
	pSelf->SendChatTarget(pResult->m_ClientID, pPlayer->m_money_transaction7);
	pSelf->SendChatTarget(pResult->m_ClientID, pPlayer->m_money_transaction8);
	pSelf->SendChatTarget(pResult->m_ClientID, pPlayer->m_money_transaction9);
	str_format(aBuf, sizeof(aBuf), "+%d (moneytiles)", pPlayer->m_MoneyTilesMoney);
	if (pPlayer->m_MoneyTilesMoney > 0)
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	pSelf->SendChatTarget(pResult->m_ClientID, "~~~~~~~~~~");
}

void CGameContext::ConPay(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (pResult->NumArguments() != 2)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Use '/pay <amount> <player>' to send money to other players'");
		return;
	}


	char aBuf[512];
	int Amount;
	char aUsername[32];
	Amount = pResult->GetInteger(0);
	str_copy(aUsername, pResult->GetString(1), sizeof(aUsername));
	int PayID = pSelf->GetCIDByName(aUsername);


	//COUDL DO:
	// add a blocker to pay money to ur self... but me funny mede it pozzible


	if (Amount > pPlayer->GetMoney())
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You don't have enough money.");
		return;
	}

	if (Amount < 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Are you trying to steal money?");
		return;
	}

	if (Amount == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You paid nothing.");
		return;
	}

	if (PayID == -1)
	{
		str_format(aBuf, sizeof(aBuf), "Can't find playername: '%s'.", aUsername);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
	else
	{
		if (!pSelf->m_apPlayers[PayID]->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: This player is not logged in. More info: '/accountinfo'.");
			return;
		}

		//player give
		str_format(aBuf, sizeof(aBuf), "You paid %d money to the player '%s'", Amount, aUsername);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "paid to '%s'", aUsername);
		pPlayer->MoneyTransaction(-Amount, aBuf);
		//dbg_msg("pay", "survived give"); //survives

		//player get
		str_format(aBuf, sizeof(aBuf), "'%s' paid you %d money", pSelf->Server()->ClientName(pResult->m_ClientID), Amount);
		pSelf->SendChatTarget(PayID, aBuf);
		str_format(aBuf, sizeof(aBuf), "paid by '%s'", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->m_apPlayers[PayID]->MoneyTransaction(Amount, aBuf);
		dbg_msg("pay", "survived get");
	}

}

void CGameContext::ConGift(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	char aBuf[256];

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "**** GIFT INFO ****");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/gift <player>' to send someone 150 money.");
		pSelf->SendChatTarget(pResult->m_ClientID, "You don't lose this money. It is coming from the server.");
		pSelf->SendChatTarget(pResult->m_ClientID, "**** GIFT STATUS ****");
		if (pPlayer->m_GiftDelay)
		{
			str_format(aBuf, sizeof(aBuf), "[GIFT] You can send gifts in %d seconds.", pPlayer->m_GiftDelay / pSelf->Server()->TickSpeed());
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[GIFT] You can send gifts.");
		}
		return;
	}

	int GiftID = pSelf->GetCIDByName(pResult->GetString(0));

	if (GiftID == -1)
	{
		str_format(aBuf, sizeof(aBuf), "[GIFT] '%s' is not online.", pResult->GetString(0));
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		return;
	}
	if (pPlayer->m_GiftDelay)
	{
		str_format(aBuf, sizeof(aBuf), "[GIFT] You can send gifts in %d seconds.", pPlayer->m_GiftDelay / pSelf->Server()->TickSpeed());
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		return;
	}
	if (pPlayer->GetLevel() < 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[GIFT] You have to be at least lvl 1 to use gifts.");
		return;
	}

	if (pSelf->m_apPlayers[GiftID])
	{
		char aOwnIP[128];
		char aGiftIP[128];
		pSelf->Server()->GetClientAddr(pResult->m_ClientID, aOwnIP, sizeof(aOwnIP));
		pSelf->Server()->GetClientAddr(GiftID, aGiftIP, sizeof(aGiftIP));

		if (!str_comp_nocase(aOwnIP, aGiftIP))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[GIFT] You can't give money to your dummy.");
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "gift (%s)", pSelf->Server()->ClientName(pResult->m_ClientID));
			pSelf->m_apPlayers[GiftID]->MoneyTransaction(+150, aBuf);
			str_format(aBuf, sizeof(aBuf), "[GIFT] You gave '%s' 150 money!", pSelf->Server()->ClientName(GiftID));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

			str_format(aBuf, sizeof(aBuf), "[GIFT] '%s' has gifted you 150 money. (more info: '/gift')", pSelf->Server()->ClientName(pResult->m_ClientID));
			pSelf->SendChatTarget(GiftID, aBuf);


			pSelf->m_apPlayers[pResult->m_ClientID]->m_GiftDelay = pSelf->Server()->TickSpeed() * 180; //180 seconds == 3 minutes
		}
	}
}

void CGameContext::ConEvent(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	bool IsEvent = false;

	pSelf->SendChatTarget(pResult->m_ClientID, "###########################");
	if (g_Config.m_SvFinishEvent == 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "~~~ Race Event ~~~");
		pSelf->SendChatTarget(pResult->m_ClientID, "Info: You get more xp for finishing the map!");
		IsEvent = true;
	}
	if (pSelf->m_BlockTournaState)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "~~~ Block Event ~~~");
		pSelf->SendChatTarget(pResult->m_ClientID, "Info: last ma standing fight in a block tournament use '/join' to join");
		IsEvent = true;
	}

	if (!IsEvent)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "No events running at the moment...");
	}

	pSelf->SendChatTarget(pResult->m_ClientID, "###########################");
}


// accept/turn-off cosmetic features

void CGameContext::ConRainbow(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/rainbow <accept/off>'.");
		return;
	}


	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if (!str_comp_nocase(aInput, "off"))
	{
		pPlayer->GetCharacter()->m_Rainbow = false;
		pPlayer->m_InfRainbow = false;
		pSelf->SendChatTarget(pResult->m_ClientID, "Rainbow turned off.");
	}
	else if (!str_comp_nocase(aInput, "accept"))
	{
		if (pPlayer->m_rainbow_offer > 0)
		{
			if (!pPlayer->GetCharacter()->m_Rainbow)
			{
				pPlayer->GetCharacter()->m_Rainbow = true;
				pPlayer->m_rainbow_offer--;
				pSelf->SendChatTarget(pResult->m_ClientID, "You accepted rainbow. You can turn it off with '/rainbow off'.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "You already have rainbow.");
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Nobody offered you rainbow.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/rainbow <accept/off>'.");
	}
}

void CGameContext::ConBloody(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/bloody <accept/off>'.");
		return;
	}


	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if (!str_comp_nocase(aInput, "off"))
	{
			pPlayer->GetCharacter()->m_Bloody = false;
			pPlayer->GetCharacter()->m_StrongBloody = false;
			pPlayer->m_InfBloody = false;
			pSelf->SendChatTarget(pResult->m_ClientID, "Bloody turned off.");
	}
	else if (!str_comp_nocase(aInput, "accept"))
	{
		if (pPlayer->m_bloody_offer > 0)
		{
			if (!pPlayer->GetCharacter()->m_Bloody)
			{
				pPlayer->GetCharacter()->m_Bloody = true;
				pPlayer->m_bloody_offer--;
				pSelf->SendChatTarget(pResult->m_ClientID, "You accepted bloody. You can turn it off with '/bloody off'.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "You already have bloody.");
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Nobody offered you bloody.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/bloody <accept/off>'.");
	}
}

void CGameContext::ConAtom(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/atom <accept/off>'.");
		return;
	}


	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if (!str_comp_nocase(aInput, "off"))
	{
			pPlayer->GetCharacter()->m_Atom = false;
			pPlayer->m_InfAtom = false;
			pSelf->SendChatTarget(pResult->m_ClientID, "Atom turned off.");
	}
	else if (!str_comp_nocase(aInput, "accept"))
	{
		if (pPlayer->m_atom_offer > 0)
		{
			if (!pPlayer->GetCharacter()->m_Atom)
			{
				pPlayer->GetCharacter()->m_Atom = true;
				pPlayer->m_atom_offer--;
				pSelf->SendChatTarget(pResult->m_ClientID, "You accepted atom. You can turn it off with '/atom off'.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "You already have atom.");
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Nobody offered you atom.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/atom <accept/off>'.");
	}
}

void CGameContext::ConAutoSpreadGun(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/spread_gun <accept/off>'.");
		return;
	}


	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if (!str_comp_nocase(aInput, "off"))
	{
		pPlayer->GetCharacter()->m_autospreadgun = false;
		pPlayer->m_InfAutoSpreadGun = false;
		pSelf->SendChatTarget(pResult->m_ClientID, "Spread gun turned off.");
	}
	else if (!str_comp_nocase(aInput, "accept"))
	{
		if (pPlayer->m_autospreadgun_offer > 0)
		{
			if (!pPlayer->GetCharacter()->m_autospreadgun)
			{
				pPlayer->GetCharacter()->m_autospreadgun = true;
				pPlayer->m_autospreadgun_offer--;
				pSelf->SendChatTarget(pResult->m_ClientID, "You accepted spread gun. You can turn it off with '/spread_gun off'.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "You already have spread gun.");
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Nobody offered you spread gun.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/spread_gun <accept/off>'.");
	}
}

void CGameContext::ConDropHealth(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (!pPlayer->m_IsSuperModerator)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[DROP] Missing permission.");
		return;
	}
	if (pPlayer->m_IsBlockTourning)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[DROP] you can't use that command during block tournaments.");
		return;
	}
	else if (pPlayer->m_IsSurvivaling && pSelf->m_survivalgamestate != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[DROP] you can't use that command during survival games. (only in lobby)");
		return;
	}
	else if (pSelf->m_survivalgamestate > 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[DROP] you can't use that command while a survival game is running.");
		return;
	}

	int amount = 1;
	if (pResult->NumArguments() > 0)
		amount = pResult->GetInteger(0);
	pChr->DropHealth(amount);
}

void CGameContext::ConDropArmor(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (!pPlayer->m_IsSuperModerator)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[DROP] Missing permission.");
		return;
	}
	if (pPlayer->m_IsBlockTourning)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[DROP] you can't use that command during block tournaments.");
		return;
	}
	else if (pPlayer->m_IsSurvivaling && pSelf->m_survivalgamestate != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[DROP] you can't use that command during survival games. (only in lobby)");
		return;
	}
	else if (pSelf->m_survivalgamestate > 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[DROP] you can't use that command while a survival game is running.");
		return;
	}

	int amount = 1;
	if (pResult->NumArguments() > 0)
		amount = pResult->GetInteger(0);
	pChr->DropArmor(amount);
}

void CGameContext::ConTrail(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/trail <accept/off>'.");
		return;
	}


	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if (!str_comp_nocase(aInput, "off"))
	{
			pPlayer->GetCharacter()->m_Trail = false;
			pPlayer->m_InfTrail = false;
			pSelf->SendChatTarget(pResult->m_ClientID, "Trail turned off.");
	}
	else if (!str_comp_nocase(aInput, "accept"))
	{
		if (pPlayer->m_trail_offer > 0)
		{
			if (!pPlayer->GetCharacter()->m_Trail)
			{
				pPlayer->GetCharacter()->m_Trail = true;
				pPlayer->m_trail_offer--;
				pSelf->SendChatTarget(pResult->m_ClientID, "You accepted trail. You can turn it off with '/trail off'.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "You already have trail.");
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Nobody offered you trail.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/trail <accept/off>'.");
	}
}

void CGameContext::ConAccountInfo(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;


	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ Account Info ~~~");
	pSelf->SendChatTarget(pResult->m_ClientID, "*** register & login ***");
	pSelf->SendChatTarget(pResult->m_ClientID, "/register <name> <password> <password>");
	pSelf->SendChatTarget(pResult->m_ClientID, "/login <name> <password>");
	pSelf->SendChatTarget(pResult->m_ClientID, "*** other commands ***");
	pSelf->SendChatTarget(pResult->m_ClientID, "/acc_logout");
	pSelf->SendChatTarget(pResult->m_ClientID, "/changepassword");
	pSelf->SendChatTarget(pResult->m_ClientID, "-------------------");
	pSelf->SendChatTarget(pResult->m_ClientID, "Accounts are used to save your stats on this server.");
	//pSelf->SendChatTarget(pResult->m_ClientID, " ");
	//pSelf->SendChatTarget(pResult->m_ClientID, "Tipp: name and password shoudl be different");
}

void CGameContext::ConPoliceInfo(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	int page = pResult->GetInteger(0); //no parameter -> 0 -> page 1
	if (!page) { page = 1; }
	int pages = 4;
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "-- page %d/%d --", page, pages);

	if (page == 1)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"~~~ Police Info ~~~");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"[GENERAL INFORMATION]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Police can be bought in shop using '/buy police'.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"There are multiple police ranks, each cost 100 000 money.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"The policebot will help every police officer.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Every police rank will give you more benefits.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Use '/policeinfo <page>' to check out what other police ranks can do.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			aBuf);
	}
	else if (page == 2)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"~~~ Police Info ~~~");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"[POLICE 1]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Level needed to buy: [LVL 18]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Benefits:");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"- '/policechat'");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Use '/policeinfo <page>' to check out what other police ranks can do.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			aBuf);
	}
	else if (page == 3)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"~~~ Police Info ~~~");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"[POLICE 2]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Level needed to buy: [LVL 25]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Benefits:");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"- full access to '/jail' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"- '/policehelper'");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Use '/policeinfo <page>' to check out what other police ranks can do.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			aBuf);
	}
	else if (page == 4)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"~~~ Police Info ~~~");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"[POLICE 3]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Level needed to buy: [LVL 30]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Benefits:");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"- taser license ('/taser')");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Use '/policeinfo <page>' to check out what other police ranks can do.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			aBuf);
	}
	/*else if (page == 5)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"~~~ Police Info ~~~");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"[POLICE 4]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Level needed to buy: [LVL 40]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Benefits:");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"- PASTE FEATURES HERE");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Use '/policeinfo <page>' to check out what other police ranks can do.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			aBuf);
	}
	else if (page == 6)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"~~~ Police Info ~~~");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"[POLICE 5]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Level needed to buy: [LVL 50]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Benefits:");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"- PASTE FEATURES HERE");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Use '/policeinfo <page>' to check out what other police ranks can do.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			aBuf);
	}*/
	else
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "policeinfo",
			"Unknow page.");
	}
}


//void CGameContext::ConProfileInfo(IConsole::IResult *pResult, void *pUserData) //old
//{
//#if defined(CONF_DEBUG)
//#endif
//	CGameContext *pSelf = (CGameContext *)pUserData;
//	if (!CheckClientID(pResult->m_ClientID))
//		return;
//
//	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
//	if (!pPlayer)
//		return;
//
//	CCharacter* pChr = pPlayer->GetCharacter();
//	if (!pChr)
//		return;
//
//
//	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ Profile Info ~~~");
//	pSelf->SendChatTarget(pResult->m_ClientID, " ");
//	pSelf->SendChatTarget(pResult->m_ClientID, "VIEW PROFILES:");
//	pSelf->SendChatTarget(pResult->m_ClientID, "/profile view (playername)");
//	pSelf->SendChatTarget(pResult->m_ClientID, "Info: The player needs to be on the server and logged in");
//	pSelf->SendChatTarget(pResult->m_ClientID, " ");
//	pSelf->SendChatTarget(pResult->m_ClientID, "PROFILE SETTINGS:");
//	pSelf->SendChatTarget(pResult->m_ClientID, "/profile_style (style) - changes your profile style");
//	pSelf->SendChatTarget(pResult->m_ClientID, "/profile_status (status) - changes your status");
//	pSelf->SendChatTarget(pResult->m_ClientID, "/profile_skype (skype) - changes your skype");
//	pSelf->SendChatTarget(pResult->m_ClientID, "/profile_youtube (youtube) - changes your youtube");
//	pSelf->SendChatTarget(pResult->m_ClientID, "/profile_email (email) - changes your email");
//	pSelf->SendChatTarget(pResult->m_ClientID, "/profile_homepage (homepage) - changes your homepage");
//	pSelf->SendChatTarget(pResult->m_ClientID, "/profile_twitter (twitter) - changes your twitter");
//}

void CGameContext::ConTCMD3000(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[TCMD3000] you have to be alive to do tests");
	}

	char aBuf[128];

	str_format(aBuf, sizeof(aBuf), "Cucumber value: %d", pSelf->m_CucumberShareValue);
	//pSelf->SendChatTarget(pResult->m_ClientID, aBuf);


	if (g_Config.m_SvTestingCommands)
	{
		/*
        pSelf->m_MissionUnlockedLevel = pResult->GetInteger(0);
        pSelf->m_MissionCurrentLevel = pResult->GetInteger(1);
        str_format(aBuf, sizeof(aBuf), "updated level unlocked=%d current=%d", pSelf->m_MissionUnlockedLevel, pSelf->m_MissionCurrentLevel);
        pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
        pSelf->SaveSinglePlayer();

		pSelf->m_BlockWaveRound = pResult->GetInteger(0);
		str_format(aBuf, sizeof(aBuf), "set blockwave level %d", pSelf->m_BlockWaveRound);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		pSelf->BlockWaveWonRound();
		*/

		/*
		if (pResult->NumArguments() != 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Quest cheater: /tcmd3000 <quest> <level>");
		}

		pPlayer->m_QuestState = pResult->GetInteger(0);
		pPlayer->m_QuestStateLevel = pResult->GetInteger(1);
		pSelf->StartQuest(pPlayer->GetCID());
		*/

		//str_format(aBuf, sizeof(aBuf), "InJailReleaseArea=%d", pChr->m_InJailOpenArea);
		//pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

		//pSelf->ChillUpdateFileAcc(pResult->GetString(0), pResult->GetInteger(1), pResult->GetString(2), pResult->m_ClientID); //a fully working set all values of acc2 files but its a bit op maybe add it to the rcon api but not as normal admin cmd
	}
}

void CGameContext::ConAntiFlood(IConsole::IResult * pResult, void * pUserData)
{

	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (pPlayer->m_Authed != CServer::AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[FLOOD] Missing permission.");
		return;
	}

	pSelf->AbuseMotd(
"*~~~~* ANTI FLOOD configs *~~~~*\n\n\
\
sv_show_connection_msg\n\
(0=none 1=join 2=leave 3=join/leave/spec)\n\n\
\
sv_hide_connection_msg_pattern\n\
(hides connection pattern check '/regex')\n\n\
\
sv_register_human_level\n\
sv_login_human_level\n\
sv_chat_human_level\n\
(min '/humane_level' to chat/reg)\n\n\
\
*~~~* ANTI FLOOD rcon cmds *~~~*\n\n\
mute, namechange_mute,\n\
register_ban, login_ban\
"
	, pResult->m_ClientID);
}

void CGameContext::ConStockMarket(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	char aBuf[256];
	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if (!str_comp_nocase(aInput, "buy"))
	{
		if (pPlayer->GetMoney() < pSelf->m_CucumberShareValue)
		{
			str_format(aBuf, sizeof(aBuf), "You don't have enough money. You need %d money.", pSelf->m_CucumberShareValue);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		else
		{
			pPlayer->m_StockMarket_item_Cucumbers++;
			pPlayer->MoneyTransaction(-pSelf->m_CucumberShareValue, "bought 'cucumber stock'");


			pSelf->m_CucumberShareValue++; // push the gernerall share value
		}

	}
	else if (!str_comp_nocase(aInput, "sell"))
	{
		if (pPlayer->m_StockMarket_item_Cucumbers > 0)
		{
			pPlayer->m_StockMarket_item_Cucumbers--;
			pPlayer->MoneyTransaction(+pSelf->m_CucumberShareValue, "sold a 'cucumber stock'");

			pSelf->m_CucumberShareValue--; // pull the gernerall share value
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You dont have this stock");
		}
	}
	else if (!str_comp_nocase(aInput, "info"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "==== PUBLIC STOCK MARKET ====");
		str_format(aBuf, sizeof(aBuf), "Cucumbers %d money", pSelf->m_CucumberShareValue);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		pSelf->SendChatTarget(pResult->m_ClientID, "==== PERSONAL STATS ====");
		str_format(aBuf, sizeof(aBuf), "Cucumbers %d", pPlayer->m_StockMarket_item_Cucumbers);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/StockMarket <buy/sell/info>'.");
	}
}

void CGameContext::ConCaptcha(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;
	CCaptcha *pCap = pPlayer->m_pCaptcha;
	if (!pCap)
		return;

	pCap->Prompt(pResult->GetString(0));
}

void CGameContext::ConHumanLevel(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "[==== Human Level %d/9 ====]", pPlayer->m_PlayerHumanLevel);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	pSelf->SendChatTarget(pResult->m_ClientID, "\
		The server trys to detect if you are a active human or a robot.\
		Some things might be blocked if your human level is too low.\
	");
	pSelf->SendChatTarget(pResult->m_ClientID, "Play active use the chat and do quests ('/quest').");
	pSelf->SendChatTarget(pResult->m_ClientID, "Block others, login to your account and use the '/captcha' command.");
}

void CGameContext::ConPoop(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (pResult->NumArguments() != 2)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Throw shit at other players. Warning: You lose what you use.");
		pSelf->SendChatTarget(pResult->m_ClientID, "Use '/poop <amount> <player>'.");
		return;
	}

	char aBuf[512];
	int Amount;
	char aUsername[32];
	Amount = pResult->GetInteger(0);
	str_copy(aUsername, pResult->GetString(1), sizeof(aUsername));
	int PoopID = pSelf->GetCIDByName(aUsername);

	if (Amount > pPlayer->m_shit)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You don't have enough shit.");
		return;
	}

	if (Amount < 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You can't poop negative?! Imagine some1 is tring to push shit back in ur anus... wtf.");
		return;
	}
	if (Amount == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, ">ou don't have shit.");
		return;
	}

	if (PoopID == -1)
	{
		str_format(aBuf, sizeof(aBuf), "Can't find user with the name: '%s'.", aUsername);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
	else
	{
		if (!pSelf->m_apPlayers[PoopID]->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: This player is not logged in. More info '/accountinfo'.");
			return;
		}


		//player give
		str_format(aBuf, sizeof(aBuf), "You pooped %s %d times xd", aUsername, Amount);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "you lost %d shit ._.", Amount);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		pPlayer->m_shit -= Amount;

		//player get
		if (g_Config.m_SvPoopMSG == 1) //normal
		{
			str_format(aBuf, sizeof(aBuf), "'%s' threw %d shit at you o.O", pSelf->Server()->ClientName(pResult->m_ClientID), Amount);
			pSelf->SendChatTarget(PoopID, aBuf);
		}
		else if (g_Config.m_SvPoopMSG == 2) //extreme
		{
			for (int i = 0; i < Amount; i++)
			{
				str_format(aBuf, sizeof(aBuf), "'%s' threw shit at you o.O", pSelf->Server()->ClientName(pResult->m_ClientID));
				pSelf->SendChatTarget(PoopID, aBuf);

				if (i > 30) //poop blocker o.O 30 lines of poop is the whole chat. Poor server has enough
				{
					str_format(aBuf, sizeof(aBuf), "'%s' threw %d shit at you o.O", pSelf->Server()->ClientName(pResult->m_ClientID), Amount); //because it was more than the chatwindow can show inform the user how much poop it was
					pSelf->SendChatTarget(PoopID, aBuf);
					break;
				}
			}
		}
		pSelf->m_apPlayers[PoopID]->m_shit += Amount;
	}

}


void CGameContext::ConGive(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] You have to be alive to use this command.");
		return;
	}

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "~~~ GIVE COMMAND ~~~");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/give <extra>' to get it for yourself.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/give <extra> <playername>' to give to others.");
		pSelf->SendChatTarget(pResult->m_ClientID, "-- EXTRAS --");
		pSelf->SendChatTarget(pResult->m_ClientID, "rainbow, bloody, strong_bloody, trail, atom, spread_gun");
		return;
	}
	else if (pPlayer->m_IsBlockTourning)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] you can't use that command during block tournaments.");
		return;
	}
	else if (pPlayer->m_IsSurvivaling && pSelf->m_survivalgamestate != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] you can't use that command during survival games. (only in lobby)");
		return;
	}


	char aBuf[512];


	//Ranks sorted DESC by power
	//---> the highest rank gets triggerd

	//the ASC problem is if a SuperModerator is also rcon_mod he only has rcon_mod powerZ



	//COUDL DO:
	//Im unsure to check if GiveID is logged in. 
	//Pros:
	//- moderators can make random players happy and they dont have to spend time to login
	//Cons:
	//- missing motivation to create an account


	if (pPlayer->m_Authed == CServer::AUTHED_ADMIN)
	{
		if (pResult->NumArguments() == 1) //only item no player --> give it ur self
		{
			char aItem[64];
			str_copy(aItem, pResult->GetString(0), sizeof(aItem));
			if (!str_comp_nocase(aItem, "bloody"))
			{
				pPlayer->GetCharacter()->m_Bloody = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Bloody on.");
			}
			else if (!str_comp_nocase(aItem, "strong_bloody"))
			{
				pPlayer->GetCharacter()->m_StrongBloody = true;
				pPlayer->GetCharacter()->m_Bloody = false;
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Strong bloody on.");
			}
			else if (!str_comp_nocase(aItem, "rainbow"))
			{
				pPlayer->GetCharacter()->m_Rainbow = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Rainbow on.");
			}
			else if (!str_comp_nocase(aItem, "trail"))
			{
				pPlayer->GetCharacter()->m_Trail = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Trail on.");
			}
			else if (!str_comp_nocase(aItem, "atom"))
			{
				pPlayer->GetCharacter()->m_Atom = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Atom on.");
			}
			else if (!str_comp_nocase(aItem, "spread_gun"))
			{
				pChr->m_autospreadgun = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Spread gun on.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Unknown item.");
			}
		}
		else if (pResult->NumArguments() == 2) //give to other players
		{
			char aItem[64];
			char aUsername[32];
			str_copy(aItem, pResult->GetString(0), sizeof(aItem));
			str_copy(aUsername, pResult->GetString(1), sizeof(aUsername));

			int GiveID = -1;
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (pSelf->m_apPlayers[i])
				{
					if (!str_comp(pSelf->Server()->ClientName(i), aUsername))
					{
						GiveID = i;
						break;
					}
				}
			}

			if (GiveID != -1)
			{
				if (!str_comp_nocase(aItem, "bloody"))
				{
					if (pSelf->m_apPlayers[GiveID]->m_bloody_offer > 4)
					{
						pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Admins can't offer bloody to the same player more than 5 times.");
					}
					else 
					{
						str_format(aBuf, sizeof(aBuf), "[GIVE] Bloody offer sent to '%s'.", aUsername);
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

						pSelf->m_apPlayers[GiveID]->m_bloody_offer++;
						str_format(aBuf, sizeof(aBuf), "[GIVE] Bloody was offered to you by '%s'. Turn it on using '/bloody accept'.", pSelf->Server()->ClientName(pResult->m_ClientID));
						pSelf->SendChatTarget(pSelf->m_apPlayers[GiveID]->GetCID(), aBuf);
					}
				}
				else if (!str_comp_nocase(aItem, "strong_bloody"))
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
				}
				else if (!str_comp_nocase(aItem, "rainbow"))
				{
					if (pSelf->m_apPlayers[GiveID]->m_rainbow_offer > 19)
					{
						pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Admins can't offer rainbow to the same player more than 20 times.");
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "[GIVE] Rainbow offer sent to '%s'.", aUsername);
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

						pSelf->m_apPlayers[GiveID]->m_rainbow_offer++;
						str_format(aBuf, sizeof(aBuf), "[GIVE] Rainbow was offered to you by '%s'. Turn it on using '/rainbow accept'.", pSelf->Server()->ClientName(pResult->m_ClientID));
						pSelf->SendChatTarget(pSelf->m_apPlayers[GiveID]->GetCID(), aBuf);
					}
				}
				else if (!str_comp_nocase(aItem, "trail"))
				{
					if (pSelf->m_apPlayers[GiveID]->m_trail_offer > 9)
					{
						pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Admins can't offer trail to the same player more than 10 times.");
						return;
					}

					str_format(aBuf, sizeof(aBuf), "Trail offer sent to '%s'.", aUsername);
					pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

					pSelf->m_apPlayers[GiveID]->m_trail_offer++;
					str_format(aBuf, sizeof(aBuf), "[GIVE] Trail was offered to you by '%s'. Turn it on using '/trail accept'.", pSelf->Server()->ClientName(pResult->m_ClientID));
					pSelf->SendChatTarget(pSelf->m_apPlayers[GiveID]->GetCID(), aBuf);
				}
				else if (!str_comp_nocase(aItem, "atom"))
				{
					if (pSelf->m_apPlayers[GiveID]->m_atom_offer > 9)
					{
						pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Admins can't offer atom to the same player more than 10 times.");
						return;
					}

					str_format(aBuf, sizeof(aBuf), "[GIVE] Atom offer sent to '%s'.", aUsername);
					pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

					pSelf->m_apPlayers[GiveID]->m_atom_offer++;
					str_format(aBuf, sizeof(aBuf), "[GIVE] Atom was offered to you by '%s'. Turn it on using '/atom accept'.", pSelf->Server()->ClientName(pResult->m_ClientID));
					pSelf->SendChatTarget(pSelf->m_apPlayers[GiveID]->GetCID(), aBuf);
				}
				else if (!str_comp_nocase(aItem, "spread_gun"))
				{
					if (pSelf->m_apPlayers[GiveID]->m_autospreadgun_offer > 9)
					{
						pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Admins can't offer spread gun to the same player more than 10 times.");
						return;
					}

					str_format(aBuf, sizeof(aBuf), "[GIVE] Spread gun offer sent to '%s'.", aUsername);
					pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

					pSelf->m_apPlayers[GiveID]->m_autospreadgun_offer++;
					str_format(aBuf, sizeof(aBuf), "[GIVE] Spread gun was offered to you by '%s'. Turn it on using '/spread_gun accept'.", pSelf->Server()->ClientName(pResult->m_ClientID));
					pSelf->SendChatTarget(pSelf->m_apPlayers[GiveID]->GetCID(), aBuf);
				}
				else
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Unknown item.");
				}
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "[GIVE] Can't find playername: '%s'.", aUsername);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			}
		}
	}
	else if (pPlayer->m_IsSuperModerator)
	{
		if (pResult->NumArguments() == 1) //only item no player --> give it ur self
		{
			char aItem[64];
			str_copy(aItem, pResult->GetString(0), sizeof(aItem));
			if (!str_comp_nocase(aItem, "bloody"))
			{
				pPlayer->GetCharacter()->m_Bloody = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Bloody on.");
			}
			else if (!str_comp_nocase(aItem, "strong_bloody"))
			{
				pPlayer->GetCharacter()->m_StrongBloody = true;
				pPlayer->GetCharacter()->m_Bloody = false;
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Strong bloody on.");
			}
			else if (!str_comp_nocase(aItem, "rainbow"))
			{
				pPlayer->GetCharacter()->m_Rainbow = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Rainbow on.");
			}
			else if (!str_comp_nocase(aItem, "trail"))
			{
				pPlayer->GetCharacter()->m_Trail = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Trail on.");
			}
			else if (!str_comp_nocase(aItem, "atom"))
			{
				pPlayer->GetCharacter()->m_Atom = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Atom on.");
			}
			else if (!str_comp_nocase(aItem, "spread_gun"))
			{
				pChr->m_autospreadgun = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Spread gun on.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Unknown item.");
			}
		}
		else if (pResult->NumArguments() == 2) //give to other players
		{
			char aItem[64];
			char aUsername[32];
			str_copy(aItem, pResult->GetString(0), sizeof(aItem));
			str_copy(aUsername, pResult->GetString(1), sizeof(aUsername));

			int GiveID = -1;
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (pSelf->m_apPlayers[i])
				{
					if (!str_comp(pSelf->Server()->ClientName(i), aUsername))
					{
						GiveID = i;
						break;
					}
				}
			}

			if (GiveID != -1)
			{
				if (!str_comp_nocase(aItem, "bloody"))
				{
					if (pSelf->m_apPlayers[GiveID]->m_bloody_offer)
					{
						pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] VIP+ can't offer bloody to the same player more than once.");
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "[GIVE] Bloody offer sent to '%s'.", aUsername);
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

						pSelf->m_apPlayers[GiveID]->m_bloody_offer++;
						str_format(aBuf, sizeof(aBuf), "[GIVE] Bloody was offered to you by '%s'. Turn it on using '/bloody accept'.", pSelf->Server()->ClientName(pResult->m_ClientID));
						pSelf->SendChatTarget(pSelf->m_apPlayers[GiveID]->GetCID(), aBuf);
					}
				}
				else if (!str_comp_nocase(aItem, "strong_bloody"))
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
				}
				else if (!str_comp_nocase(aItem, "rainbow"))
				{
					if (pSelf->m_apPlayers[GiveID]->m_rainbow_offer > 9)
					{
						pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] VIP+ can't offer rainbow to the same player more than 10 times.");
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "[GIVE] Rainbow offer sent to '%s'.", aUsername);
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

						pSelf->m_apPlayers[GiveID]->m_rainbow_offer++;
						str_format(aBuf, sizeof(aBuf), "[GIVE] Rainbow was offered to you by '%s'. Turn it on using '/rainbow accept'.", pSelf->Server()->ClientName(pResult->m_ClientID));
						pSelf->SendChatTarget(pSelf->m_apPlayers[GiveID]->GetCID(), aBuf);
					}
				}
				else if (!str_comp_nocase(aItem, "trail"))
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
				}
				else if (!str_comp_nocase(aItem, "atom"))
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
				}
				else if (!str_comp_nocase(aItem, "spread_gun"))
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
				}
				else
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Unknown item.");
				}
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "[GIVE] Can't find player '%s'.", aUsername);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			}
		}
	}
	else if (pPlayer->m_IsModerator)
	{
		if (pResult->NumArguments() == 1) //only item no player --> give it ur self
		{
			char aItem[64];
			str_copy(aItem, pResult->GetString(0), sizeof(aItem));
			if (!str_comp_nocase(aItem, "bloody"))
			{
				pPlayer->GetCharacter()->m_Bloody = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Bloody on.");
			}
			else if (!str_comp_nocase(aItem, "strong_bloody"))
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
			}
			else if (!str_comp_nocase(aItem, "rainbow"))
			{
				pPlayer->GetCharacter()->m_Rainbow = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Rainbow on.");
			}
			else if (!str_comp_nocase(aItem, "trail"))
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
			}
			else if (!str_comp_nocase(aItem, "atom"))
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
			}
			else if (!str_comp_nocase(aItem, "spread_gun"))
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Unknown item.");
			}
		}
		else if (pResult->NumArguments() == 2) //give to other players
		{
			char aItem[64];
			char aUsername[32];
			str_copy(aItem, pResult->GetString(0), sizeof(aItem));
			str_copy(aUsername, pResult->GetString(1), sizeof(aUsername));

			int GiveID = -1;
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (pSelf->m_apPlayers[i])
				{
					if (!str_comp(pSelf->Server()->ClientName(i), aUsername))
					{
						GiveID = i;
						break;
					}
				}
			}

			if (GiveID != -1)
			{
				if (!str_comp_nocase(aItem, "bloody"))
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
				}
				else if (!str_comp_nocase(aItem, "strong_bloody"))
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
				}
				else if (!str_comp_nocase(aItem, "rainbow"))
				{
					if (pSelf->m_apPlayers[GiveID]->m_rainbow_offer)
					{
						pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] VIPs can't offer rainbow to the same player more than once.");
					}
					else
					{
						pSelf->m_apPlayers[GiveID]->m_rainbow_offer++;
						str_format(aBuf, sizeof(aBuf), "[GIVE] Rainbow offer sent to player: '%s'.", aUsername);
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
					}
				}
				else if (!str_comp_nocase(aItem, "trail"))
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
				}
				else if (!str_comp_nocase(aItem, "atom"))
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
				}
				else if (!str_comp_nocase(aItem, "spread_gun"))
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
				}
				else
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Unknown item.");
				}
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "[GIVE] Can't find playername: '%s'.", aUsername);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			}
		}
	}
	else if (pPlayer->m_Authed == CServer::AUTHED_MOD)
	{
		char aItem[64];
		str_copy(aItem, pResult->GetString(0), sizeof(aItem));
		if (!str_comp_nocase(aItem, "bloody"))
		{
			pPlayer->GetCharacter()->m_Bloody = true;
			pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Bloody on.");
		}
		else if (!str_comp_nocase(aItem, "strong_bloody"))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
		}
		else if (!str_comp_nocase(aItem, "rainbow"))
		{
			pPlayer->GetCharacter()->m_Rainbow = true;
			pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Rainbow on.");
		}
		else if (!str_comp_nocase(aItem, "trail"))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
		}
		else if (!str_comp_nocase(aItem, "atom"))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
		}
		else if (!str_comp_nocase(aItem, "spread_gun"))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Unknown item.");
		}
	}
	else //no rank at all
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[GIVE] Missing permission.");
	}
}

void CGameContext::ConBomb(IConsole::IResult *pResult, void *pUserData)
{

	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (!g_Config.m_SvAllowBomb)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Admin deactivated bomb games.");
		return;
	}

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Missing parameter. Check '/bomb help' for more help.");
		return;
	}


	char aBuf[512];

	char aCmd[64];

	str_copy(aCmd, pResult->GetString(0), sizeof(aCmd));

	if (!str_comp_nocase(aCmd, "create"))
	{
		if (pResult->NumArguments() < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: Missing money parameter ('/bomb create <money>').");
			return;
		}
		if (pSelf->m_BombGameState)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: There is already a bomb game. You can join it with '/bomb join'.");
			return;
		}
		if (!pPlayer->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You need to be logged in to create a bomb game. More info at '/accountinfo'.");
			return;
		}
		if (pPlayer->m_BombBanTime)
		{
			str_format(aBuf, sizeof(aBuf), "You are banned from bomb gamesfor %d second(s).", pPlayer->m_BombBanTime / 60);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}
		if (pSelf->IsMinigame(pResult->m_ClientID))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			return;
		}

		int BombMoney;
		BombMoney = pResult->GetInteger(1);

		if (BombMoney > pPlayer->GetMoney())
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: You don't have enough money.");
			return;
		}
		if (BombMoney < 0)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: Bomb reward has to be positive.");
			return;
		}

		if (pResult->NumArguments() > 2) //config map
		{
			char aConfig[32];
			str_copy(aConfig, pResult->GetString(2), sizeof(aConfig));

			if (!str_comp_nocase(aConfig, "NoArena"))
			{
				str_format(pSelf->m_BombMap, sizeof(pSelf->m_BombMap), "NoArena");
			}
			else if (!str_comp_nocase(aConfig, "Default"))
			{
				str_format(pSelf->m_BombMap, sizeof(pSelf->m_BombMap), "Default");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "--------[BOMB]--------");
				pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: unknown map. Aviable maps: ");
				pSelf->SendChatTarget(pResult->m_ClientID, "Default");
				pSelf->SendChatTarget(pResult->m_ClientID, "NoArena");
				pSelf->SendChatTarget(pResult->m_ClientID, "----------------------");
				return;
			}
		}
		else //no config --> Default
		{
			str_format(pSelf->m_BombMap, sizeof(pSelf->m_BombMap), "Default");
		}

		pSelf->m_apPlayers[pResult->m_ClientID]->m_BombTicksUnready = 0;
		pSelf->m_BombMoney = BombMoney;
		pSelf->m_BombGameState = 1;
		pChr->m_IsBombing = true;
		pPlayer->MoneyTransaction(-BombMoney, "bomb join");

		str_format(aBuf, sizeof(aBuf), "[BOMB] You have created a game lobby. Map: '%s'.", pSelf->m_BombMap);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), " -%d money for joining this bomb game.", BombMoney);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
	else if (!str_comp_nocase(aCmd, "join"))
	{
		if (!pPlayer->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You need to be logged in to join a bomb game. More info at '/accountinfo'.");
			return;
		}
		if (pPlayer->m_BombBanTime)
		{
			str_format(aBuf, sizeof(aBuf), "You are banned from bomb games for %d second(s).", pPlayer->m_BombBanTime / 60);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}
		if (pChr->m_IsBombing)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You are already in the bomb game.");
			return;
		}
		if (pSelf->IsMinigame(pResult->m_ClientID))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			return;
		}

		if (!pSelf->m_BombGameState)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "No bomb game running. You can create a new one with '/bomb create <money>'.");
			return;
		}
		else if (pSelf->m_BombGameState == 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "The bomb lobby is locked.");
			return;
		}
		else if (pSelf->m_BombGameState == 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "The bomb game is already running.");
			return;
		}
		else if (pSelf->m_BombGameState == 1)
		{
			if (pPlayer->GetMoney() < pSelf->m_BombMoney)
			{
				str_format(aBuf, sizeof(aBuf), "You need at least %d money to join this game.", pSelf->m_BombMoney);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}

			str_format(aBuf, sizeof(aBuf), "-%d money for joining this game. You don't want to risk that much money? Type '/bomb leave'", pSelf->m_BombMoney);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendChatTarget(pResult->m_ClientID, "You will die in this game! So better leave if you want to keep weapons and stuff.");
			pChr->m_IsBombing = true;
			pPlayer->MoneyTransaction(-pSelf->m_BombMoney, "bomb join");
			pPlayer->m_BombTicksUnready = 0;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Something went horrible wrong lol. Please contact an admin.");
			return;
		}
	}
	else if (!str_comp_nocase(aCmd, "leave"))
	{
		if (!pChr->m_IsBombing)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You are not in a bomb game.");
			return;
		}
		if (pChr->m_IsBombing && pSelf->m_BombGameState == 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You can't leave a running game. (If you disconnect, your money will be lost.)");
			return;
		}

		str_format(aBuf, sizeof(aBuf), "You left the bomb game. (+%d money)", pSelf->m_BombMoney);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		pPlayer->MoneyTransaction(pSelf->m_BombMoney, "bomb leave");
		pSelf->SendBroadcast("", pResult->m_ClientID);
		pChr->m_IsBombing = false;
		pChr->m_IsBomb = false;
		pChr->m_IsBombReady = false;
	}
	else if (!str_comp_nocase(aCmd, "start"))
	{
		if (!pChr->m_IsBombing)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You are not in a bomb game. Type '/bomb create <money>' or '/bomb join' first.");
			return;
		}
		if (pChr->m_IsBombReady)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You are already ready to start. (If you aren't ready anymore try '/bomb leave')");
			return;
		}
		if (pChr->m_IsBombing && pSelf->m_BombGameState == 3) //shoudl be never triggerd but yolo xd
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Game is already running...");
			return;
		}

		pSelf->SendChatTarget(pResult->m_ClientID, "You are now ready to play. Waiting for other players...");
		pChr->m_IsBombReady = true;
	}
	else if (!str_comp_nocase(aCmd, "lock")) 
	{
		if (!pChr->m_IsBombing)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You are not in a bomb game.");
			return;
		}
		if (pChr->m_IsBombing && pSelf->m_BombGameState == 3) 
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Running games are locked automatically.");
			return;
		}
		if (pSelf->CountBombPlayers() == 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "More bomb players needed to lock the lobby.");
			return;
		}
		if (g_Config.m_SvBombLockable == 0) //off
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Locking bomblobbys is deactivated.");
			return;
		}
		else if (g_Config.m_SvBombLockable == 1) //mods and higher
		{
			if (!pSelf->Server()->IsAuthed(pResult->m_ClientID))
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Only authed players can lock bomb games.");
				return;
			}
		}

		if (pSelf->m_BombGameState == 1) //unlocked --> lock
		{
			//lock it
			pSelf->m_BombGameState = 2;

			//send lock message to all bombers
			str_format(aBuf, sizeof(aBuf), "'%s' locked the bomb lobby.", pSelf->Server()->ClientName(pResult->m_ClientID));
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (pSelf->GetPlayerChar(i))
				{
					if (pSelf->GetPlayerChar(i)->m_IsBombing)
					{
						pSelf->SendChatTarget(i, aBuf);
					}
				}
			}
		}
		else if (pSelf->m_BombGameState == 2) //locked --> unlock
		{
			//unlock it
			pSelf->m_BombGameState = 1;

			//send unlock message to all bombers
			str_format(aBuf, sizeof(aBuf), "'%s' unlocked the bomb lobby.", pSelf->Server()->ClientName(pResult->m_ClientID));
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (pSelf->GetPlayerChar(i))
				{
					if (pSelf->GetPlayerChar(i)->m_IsBombing)
					{
						pSelf->SendChatTarget(i, aBuf);
					}
				}
			}
		}
	}
	else if (!str_comp_nocase(aCmd, "status"))
	{
		if (!pSelf->m_BombGameState) //offline
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Currently no bomb game running.");
			return;
		}
		else if (pSelf->m_BombGameState == 1) //lobby
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "------ Bomb Lobby ------");
			str_format(aBuf, sizeof(aBuf), "[%d/%d] players ready", pSelf->CountReadyBombPlayers(), pSelf->CountBombPlayers());
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendChatTarget(pResult->m_ClientID, "------------------------");
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (pSelf->GetPlayerChar(i) && pSelf->GetPlayerChar(i)->m_IsBombing)
				{
					if (pSelf->GetPlayerChar(i)->m_IsBombReady)
					{
						str_format(aBuf, sizeof(aBuf), "'%s' (ready)", pSelf->Server()->ClientName(i));
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "'%s'", pSelf->Server()->ClientName(i));
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
					}
				}
			}
			return;
		}
		else if (pSelf->m_BombGameState == 2) //lobby locked
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "------ Bomb Lobby (locked) ------");
			str_format(aBuf, sizeof(aBuf), "[%d/%d] players ready", pSelf->CountReadyBombPlayers(), pSelf->CountBombPlayers());
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendChatTarget(pResult->m_ClientID, "------------------------");
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (pSelf->GetPlayerChar(i) && pSelf->GetPlayerChar(i)->m_IsBombing)
				{
					if (pSelf->GetPlayerChar(i)->m_IsBombReady)
					{
						str_format(aBuf, sizeof(aBuf), "'%s' (ready)", pSelf->Server()->ClientName(i));
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "'%s'", pSelf->Server()->ClientName(i));
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
					}
				}
			}
			return;
		}
		else if (pSelf->m_BombGameState == 3) //ingame
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "------ Bomb (running game) ------");
			str_format(aBuf, sizeof(aBuf), "[%d/%d] players ready", pSelf->CountReadyBombPlayers(), pSelf->CountBombPlayers());
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendChatTarget(pResult->m_ClientID, "------------------------");
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (pSelf->GetPlayerChar(i) && pSelf->GetPlayerChar(i)->m_IsBombing)
				{
					if (pSelf->GetPlayerChar(i)->m_IsBombReady)
					{
						str_format(aBuf, sizeof(aBuf), "'%s' (ready)", pSelf->Server()->ClientName(i));
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "'%s'", pSelf->Server()->ClientName(i));
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
					}
				}
			}
			return;
		}
	}
	else if (!str_comp_nocase(aCmd, "ban"))
	{
		if (pResult->NumArguments() < 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: Stick to the following structure:");
			pSelf->SendChatTarget(pResult->m_ClientID, "'/bomb ban <seconds> <player>'");
			return;
		}

		if (pPlayer->m_Authed == CServer::AUTHED_ADMIN) //DESC power to use highest rank
		{
			//int Bantime = pResult->GetInteger(1) * pSelf->Server()->TickSpeed();
			int Bantime = pResult->GetInteger(1);
			char aBanname[32];
			str_copy(aBanname, pResult->GetString(2), sizeof(aBanname));
			int BanID = pSelf->GetCIDByName(aBanname);

			if (BanID == -1)
			{
				str_format(aBuf, sizeof(aBuf), "Can't find playername: '%s'.", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}

			if (Bantime > 900) //15 min ban limit
			{
				Bantime = 900;
				str_format(aBuf, sizeof(aBuf), "You banned '%s' for 15 minutes (max).", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

				//off all bomb stuff for the player
				pSelf->GetPlayerChar(BanID)->m_IsBombing = false;
				pSelf->GetPlayerChar(BanID)->m_IsBomb = false;
				pSelf->GetPlayerChar(BanID)->m_IsBombReady = false;

				//do the ban
				pSelf->m_apPlayers[BanID]->m_BombBanTime = Bantime * 60;
				str_format(aBuf, sizeof(aBuf), "[BOMB] You were banned by admin '%s' for %d seconds.", pSelf->Server()->ClientName(pResult->m_ClientID), Bantime);
				pSelf->SendChatTarget(BanID, aBuf);
			}
			else if (Bantime < 0)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Bantime has to be positive.");
				return;
			}
			else
			{
				//BANNED PLAYER

				//off all bomb stuff for the player
				pSelf->GetPlayerChar(BanID)->m_IsBombing = false;
				pSelf->GetPlayerChar(BanID)->m_IsBomb = false;
				pSelf->GetPlayerChar(BanID)->m_IsBombReady = false;

				//do the ban
				pSelf->m_apPlayers[BanID]->m_BombBanTime = Bantime * 60;
				str_format(aBuf, sizeof(aBuf), "[BOMB] You were banned by admin '%s' for %d seconds.", pSelf->Server()->ClientName(pResult->m_ClientID), Bantime);
				pSelf->SendChatTarget(BanID, aBuf);

				//BANNING PLAYER
				str_format(aBuf, sizeof(aBuf), "You banned '%s' for %d seconds from bomb games.", aBanname, Bantime);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

				//ADMIN CONSOLE (isnt admin console ._. itz chat :c)
				//str_format(aBuf, sizeof(aBuf), "'%s' were banned by admin '%s' for %d seconds.", aBanname, pSelf->Server()->ClientName(pResult->m_ClientID), Bantime);
				//pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bomb", aBuf);
			}
		}
		else if (pPlayer->m_IsSuperModerator)
		{
			int Bantime = pResult->GetInteger(1);
			char aBanname[32];
			str_copy(aBanname, pResult->GetString(2), sizeof(aBanname));
			int BanID = pSelf->GetCIDByName(aBanname);

			if (BanID == -1)
			{
				str_format(aBuf, sizeof(aBuf), "Can't find playername: '%s'.", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			if (pSelf->m_apPlayers[BanID]->m_Authed == CServer::AUTHED_ADMIN)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
				return;
			}

			if (Bantime > 300) //5 min ban limit
			{
				Bantime = 300;
				str_format(aBuf, sizeof(aBuf), "You banned '%s' for 5 minutes (max).", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

				//off all bomb stuff for the player
				pSelf->GetPlayerChar(BanID)->m_IsBombing = false;
				pSelf->GetPlayerChar(BanID)->m_IsBomb = false;
				pSelf->GetPlayerChar(BanID)->m_IsBombReady = false;

				//do the ban
				pSelf->m_apPlayers[BanID]->m_BombBanTime = Bantime * 60;
				str_format(aBuf, sizeof(aBuf), "[BOMB] You were banned by VIP+ '%s' for %d seconds.", pSelf->Server()->ClientName(pResult->m_ClientID), Bantime);
				pSelf->SendChatTarget(BanID, aBuf);
			}
			else if (Bantime < 0)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Bantime has to be positive.");
				return;
			}
			else
			{
				//BANNED PLAYER

				//off all bomb stuff for the player
				pSelf->GetPlayerChar(BanID)->m_IsBombing = false;
				pSelf->GetPlayerChar(BanID)->m_IsBomb = false;
				pSelf->GetPlayerChar(BanID)->m_IsBombReady = false;

				//do the ban
				pSelf->m_apPlayers[BanID]->m_BombBanTime = Bantime * 60;
				str_format(aBuf, sizeof(aBuf), "[BOMB] You were banned by VIP+ '%s' for %d seconds.", pSelf->Server()->ClientName(pResult->m_ClientID), Bantime);
				pSelf->SendChatTarget(BanID, aBuf);

				//BANNING PLAYER
				str_format(aBuf, sizeof(aBuf), "You banned '%s' for %d seconds from bomb games.", aBanname, Bantime);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

				//ADMIN CONSOLE (isnt admin console ._. itz chat :c)
				//str_format(aBuf, sizeof(aBuf), "'%s' were banned by supermoderator '%s' for %d seconds.", aBanname, pSelf->Server()->ClientName(pResult->m_ClientID), Bantime);
				//pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bomb", aBuf);
			}
		}
		else if (pPlayer->m_IsModerator)
		{
			int Bantime = pResult->GetInteger(1);
			char aBanname[32];
			str_copy(aBanname, pResult->GetString(2), sizeof(aBanname));
			int BanID = pSelf->GetCIDByName(aBanname);

			if (BanID == -1)
			{
				str_format(aBuf, sizeof(aBuf), "Can't find playername: '%s'.", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			if (pSelf->m_apPlayers[BanID]->m_Authed == CServer::AUTHED_ADMIN || pSelf->m_apPlayers[BanID]->m_IsSuperModerator)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission to kick this player.");
				return;
			}

			if (Bantime > 60) //1 min ban limit
			{
				Bantime = 60;
				str_format(aBuf, sizeof(aBuf), "You banned '%s' for 1 minute (max).", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

				//off all bomb stuff for the player
				pSelf->GetPlayerChar(BanID)->m_IsBombing = false;
				pSelf->GetPlayerChar(BanID)->m_IsBomb = false;
				pSelf->GetPlayerChar(BanID)->m_IsBombReady = false;

				//do the ban
				pSelf->m_apPlayers[BanID]->m_BombBanTime = Bantime * 60;
				str_format(aBuf, sizeof(aBuf), "[BOMB] You were banned by VIP '%s' for %d seconds.", pSelf->Server()->ClientName(pResult->m_ClientID), Bantime);
				pSelf->SendChatTarget(BanID, aBuf);
			}
			else if (Bantime < 0)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Bantime has to be positive.");
				return;
			}
			else
			{
				//BANNED PLAYER

				//off all bomb stuff for the player
				pSelf->GetPlayerChar(BanID)->m_IsBombing = false;
				pSelf->GetPlayerChar(BanID)->m_IsBomb = false;
				pSelf->GetPlayerChar(BanID)->m_IsBombReady = false;

				//do the ban
				pSelf->m_apPlayers[BanID]->m_BombBanTime = Bantime * 60;
				str_format(aBuf, sizeof(aBuf), "[BOMB] You were banned by VIP '%s' for %d seconds.", pSelf->Server()->ClientName(pResult->m_ClientID), Bantime);
				pSelf->SendChatTarget(BanID, aBuf);

				//BANNING PLAYER
				str_format(aBuf, sizeof(aBuf), "You banned '%s' for %d seconds from bomb games.", aBanname, Bantime);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

				//ADMIN CONSOLE (isnt admin console ._. itz chat :c)
				//str_format(aBuf, sizeof(aBuf), "'%s' were banned by vip '%s' for %d seconds.", aBanname, pSelf->Server()->ClientName(pResult->m_ClientID), Bantime);
				//pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bomb", aBuf);
			}
		}
		else if (pPlayer->m_Authed == CServer::AUTHED_MOD)
		{
			int Bantime = pResult->GetInteger(1);
			char aBanname[32];
			str_copy(aBanname, pResult->GetString(2), sizeof(aBanname));
			int BanID = pSelf->GetCIDByName(aBanname);

			if (BanID == -1)
			{
				str_format(aBuf, sizeof(aBuf), "Can't find playername: '%s'.", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}
			if (pSelf->m_apPlayers[BanID]->m_Authed == CServer::AUTHED_ADMIN || pSelf->m_apPlayers[BanID]->m_IsSuperModerator || pSelf->m_apPlayers[BanID]->m_IsModerator)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission to kick this player.");
				return;
			}

	
			if (!pSelf->GetPlayerChar(BanID)->m_IsBombing)
			{
				str_format(aBuf, sizeof(aBuf), "'%s' is not in a bomb game.", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}

			//KICKED PLAYER

			//off all bomb stuff for the player
			pSelf->GetPlayerChar(BanID)->m_IsBombing = false;
			pSelf->GetPlayerChar(BanID)->m_IsBomb = false;
			pSelf->GetPlayerChar(BanID)->m_IsBombReady = false;
			pSelf->SendBroadcast("", BanID);

			//do the kick
			pSelf->m_apPlayers[BanID]->m_BombBanTime = Bantime * 60;
			str_format(aBuf, sizeof(aBuf), "[BOMB] You were kicked by rcon_mod '%s'.", pSelf->Server()->ClientName(pResult->m_ClientID));
			pSelf->SendChatTarget(BanID, aBuf);

			//KICKING PLAYER
			str_format(aBuf, sizeof(aBuf), "You kicked '%s' from bomb games.", aBanname);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

			//ADMIN CONSOLE (isnt admin console ._. itz chat :c)
			//str_format(aBuf, sizeof(aBuf), "'%s' were kicked by rcon_mod '%s'.", aBanname, pSelf->Server()->ClientName(pResult->m_ClientID));
			//pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "bomb", aBuf);

		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
			return;
		}
	}
	else if (!str_comp_nocase(aCmd, "unban"))
	{
		if (pResult->NumArguments() < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Missing parameter: ClientID to unban ('/bomb banlist' for IDs).");
			pSelf->SendChatTarget(pResult->m_ClientID, "Command structure: '/bomb unban <ClientID>'.");
			pSelf->SendChatTarget(pResult->m_ClientID, "Unban all: '/bomb unban -1'.");
			return;
		}

		int UnbanID = pResult->GetInteger(1);

		if (UnbanID == -1) //unban all
		{
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->m_BombBanTime)
				{
					//UNBANNING PLAYER
					str_format(aBuf, sizeof(aBuf), "You unbanned '%s' (he had %d seconds bantime left).", pSelf->Server()->ClientName(i), pSelf->m_apPlayers[i]->m_BombBanTime / 60);
					pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

					//UNBANNED PLAYER
					pSelf->m_apPlayers[i]->m_BombBanTime = 0;
					str_format(aBuf, sizeof(aBuf), "You were unbanned from bomb games by '%s'.", pSelf->Server()->ClientName(pResult->m_ClientID));
					pSelf->SendChatTarget(pSelf->m_apPlayers[i]->GetCID(), aBuf);
				}
			}
			return;
		}


		if (pSelf->m_apPlayers[UnbanID])
		{
			if (pSelf->m_apPlayers[UnbanID]->m_BombBanTime)
			{
				//UNBANNING PLAYER
				str_format(aBuf, sizeof(aBuf), "You unbanned '%s' (he had %d seconds bantime left).", pSelf->Server()->ClientName(UnbanID), pSelf->m_apPlayers[UnbanID]->m_BombBanTime / 60);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

				//UNBANNED PLAYER
				pSelf->m_apPlayers[UnbanID]->m_BombBanTime = 0;
				str_format(aBuf, sizeof(aBuf), "You were unbanned from bomb games by '%s'.", pSelf->Server()->ClientName(pResult->m_ClientID));
				pSelf->SendChatTarget(pSelf->m_apPlayers[UnbanID]->GetCID(), aBuf);
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "'%s' is not banned from bomb games.", pSelf->Server()->ClientName(UnbanID));
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "There is no player with this ClientID.");
			return;
		}
	}
	else if (!str_comp_nocase(aCmd, "banlist")) //for now all dudes can see the banlist... not sure to make it rank only visible
	{
		if (!pSelf->CountBannedBombPlayers())
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "There are no banned bomb players, yay.");
			return;
		}


		//calcualte page amount
		//float f_pages = pSelf->CountBannedBombPlayers() / 5;

		//int pages = round(pSelf->CountBannedBombPlayers() / 5 + 0.5); //works as good as ++
		int pages = (int)((float)pSelf->CountBannedBombPlayers() / 5.0f + 0.999999f);
		int PlayersShownOnPage = 0;


		if (pResult->NumArguments() > 1) //show pages
		{
			int PlayersShownOnPreviousPages = 0;
			int page = pResult->GetInteger(1);
			if (page > pages)
			{
				if (pages == 1)
				{
					str_format(aBuf, sizeof(aBuf), "ERROR: There is only %d page.", pages);
					pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				}
				else
				{
					str_format(aBuf, sizeof(aBuf), "ERROR: There are only %d pages.", pages);
					pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				}
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "=== (%d) Banned Bombers (PAGE: %d/%d) ===", pSelf->CountBannedBombPlayers(), page, pages);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				//for (int i = page * 5; i < MAX_CLIENTS; i++) yes it is an minor performance improvement but fuck it (did that cuz something didnt work) ((would be -1 anyways because human page 2 is computer page 1))
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->m_BombBanTime)
					{
						if (PlayersShownOnPreviousPages >= (page - 1) * 5)
						{
							str_format(aBuf, sizeof(aBuf), "ID: %d '%s' (%d seconds)", pSelf->m_apPlayers[i]->GetCID(), pSelf->Server()->ClientName(i), pSelf->m_apPlayers[i]->m_BombBanTime / 60);
							pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
							PlayersShownOnPage++;
						}
						else
						{
							//str_format(aBuf, sizeof(aBuf), "No list cuz  NOT Previous: %d > (page - 1) * 5: %d", PlayersShownOnPreviousPages, (page - 1) * 5);
							//dbg_msg("bomb", aBuf);
							PlayersShownOnPreviousPages++;
						}
					}
					if (PlayersShownOnPage > 4) //show only 5 players on one site
					{
						//dbg_msg("bomb", "page limit reached");
						break;
					}
				}
				//str_format(aBuf, sizeof(aBuf), "Finished loop. Players on previous: %d", PlayersShownOnPreviousPages);
				//dbg_msg("bomb", aBuf);
			}
		}
		else //show page one
		{
			str_format(aBuf, sizeof(aBuf), "=== (%d) Banned Bombers (PAGE: %d/%d) ===", pSelf->CountBannedBombPlayers(), 1, pages);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->m_BombBanTime)
				{
					str_format(aBuf, sizeof(aBuf), "ID: %d '%s' (%d seconds)", pSelf->m_apPlayers[i]->GetCID(), pSelf->Server()->ClientName(i), pSelf->m_apPlayers[i]->m_BombBanTime / 60);
					pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
					PlayersShownOnPage++;
				}
				if (PlayersShownOnPage > 4) //show only 5 players on one site
				{
					break;
				}
			}
		}
	}
	else if (!str_comp_nocase(aCmd, "help") || !str_comp_nocase(aCmd, "info"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "===================");
		pSelf->SendChatTarget(pResult->m_ClientID, "->   B O M B   <-");
		pSelf->SendChatTarget(pResult->m_ClientID, "===================");
		pSelf->SendChatTarget(pResult->m_ClientID, "HOW DOES THE GAME WORK?");
		pSelf->SendChatTarget(pResult->m_ClientID, "One player gets choosen as bomb. Hammer other players to transfer bomb.");
		pSelf->SendChatTarget(pResult->m_ClientID, "If a player explodes as bomb he is out.");
		pSelf->SendChatTarget(pResult->m_ClientID, "Last man standing is the winner.");
		pSelf->SendChatTarget(pResult->m_ClientID, "HOW TO START?");
		pSelf->SendChatTarget(pResult->m_ClientID, "One player has to create a bomb lobby with '/bomb create <money>'.");
		pSelf->SendChatTarget(pResult->m_ClientID, "Players can join with '/bomb join'.");
		pSelf->SendChatTarget(pResult->m_ClientID, "All players who join the game have to pay <money> money and the winner gets it all.");
		pSelf->SendChatTarget(pResult->m_ClientID, "---------------");
		pSelf->SendChatTarget(pResult->m_ClientID, "More bomb commands at '/bomb cmdlist'.");
		return;
	}
	else if (!str_comp_nocase(aCmd, "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== BOMB COMMANDS ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bomb create <money>' to create a bomb game.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bomb create <money> <map>' also creates a bomb game.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bomb join' to join a bomb game.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bomb start' to start a game.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bomb lock' to lock a bomb lobby.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bomb status' to see some live bomb stats.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bomb ban <seconds> <name>' to ban players.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bomb unban <ClientID>' to unban players.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bomb banlist' to see all banned players.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bomb help' for help and info.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bomb cmdlist' for all bomb commands.");
		return;
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Unknown bomb command. More help at '/bomb help' or 'bomb cmdlist'.");
		return;
	}
}

void CGameContext::ConSurvival(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (!g_Config.m_SvAllowSurvival)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[SURVIVAL] command not allowed.");
		return;
	}

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[SURVIVAL] You have to be alive to use this command.");
		return;
	}

	//if (!pPlayer->IsLoggedIn()) //we want 10000 survival players so no annoying login
	//{
	//	pSelf->SendChatTarget(pResult->m_ClientID, "You have to be logged in to use this command. (type '/accountinfo' for more info)");
	//	return;
	//}


	if (pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=========> [SURVIVAL] <=========");
		pSelf->SendChatTarget(pResult->m_ClientID, "Don't die or you'r out!");
		pSelf->SendChatTarget(pResult->m_ClientID, "Last man standing wins the game.");
		pSelf->SendChatTarget(pResult->m_ClientID, "check '/survival cmdlist' for a list of all commands");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== SURVIVAL COMMANDS ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/survival join' to join the survival lobby");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/survival leave' to leave survival");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/survival status' to show current game status");
		//pSelf->SendChatTarget(pResult->m_ClientID, "'/survival stats' to show your stats");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "status"))
	{
		char aBuf[128];
		char aGameState[128];
		char aDM[16];
		char aTimeLimit[64];
		str_copy(aTimeLimit, "", sizeof(aTimeLimit));
		str_copy(aDM, "", sizeof(aDM));
		switch (pSelf->m_survivalgamestate) {
			case SURVIVAL_OFF:
			str_copy(aGameState, "off", sizeof(aGameState));
			break;
			case SURVIVAL_LOBBY:
			str_copy(aGameState, "in lobby phase", sizeof(aGameState));
			break;
			case SURVIVAL_DM:
			str_copy(aDM, "deathmatch", sizeof(aDM));
			// fall through
			case SURVIVAL_DM_COUNTDOWN:
			case SURVIVAL_INGAME:
			if (pSelf->m_survival_game_countdown != -1)
			{
				float time = pSelf->m_survival_game_countdown / pSelf->Server()->TickSpeed();
				str_format(aTimeLimit, sizeof(aTimeLimit),"(%d min %5.2f sec left max)", (int) time / 60, time - ((int) time / 60 * 60));
			}
			str_format(aGameState, sizeof(aGameState), "running %s %d/%d alive %s", aDM, pSelf->CountSurvivalPlayers(true), pSelf->m_survival_start_players, aTimeLimit);
			break;
			default:
			str_copy(aGameState, "unkown", sizeof(aGameState));
			break;
		}
		str_format(aBuf, sizeof(aBuf), "[SURVIVAL] Game is %s", aGameState);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
	else if (!str_comp_nocase(pResult->GetString(0), "leave"))
	{
		if (pPlayer->m_IsSurvivaling)
		{
			pChr->Die(pPlayer->GetCID(), WEAPON_SELF);
			pSelf->SetPlayerSurvival(pResult->m_ClientID, 0);
			pSelf->SendChatTarget(pResult->m_ClientID, "[SURVIVAL] you left the game. (bye c:)");
			pSelf->SendBroadcast("", pPlayer->GetCID(), 1); // survival broadcasts are importance lvl1 so lvl1 is needed to overwrite
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[SURVIVAL] you currently aren't playing survival.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "join"))
	{
		if (pSelf->IsMinigame(pResult->m_ClientID) == 4)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[SURVIVAL] you already joined the survival game.");
			return;
		}
		if (pSelf->IsMinigame(pResult->m_ClientID))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			return;
		}
		int spawns = pSelf->Collision()->CountSurvivalSpawns();
		int survivalplayers = pSelf->CountSurvivalPlayers();
		if (survivalplayers >= spawns)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[SURVIVAL] The survival lobby is full try agian later.");
			return;
		}


		vec2 SurvialLobbySpawnTile = pSelf->Collision()->GetRandomTile(TILE_SURVIVAL_LOBBY);

		if (SurvialLobbySpawnTile != vec2(-1, -1))
		{
			pSelf->m_apPlayers[pResult->m_ClientID]->GetCharacter()->SetPosition(SurvialLobbySpawnTile);
			pSelf->SendChatTarget(pResult->m_ClientID, "[SURVIVAL] You joined survival.");
			pSelf->SetPlayerSurvival(pResult->m_ClientID, 1);
		}
		else //no TestToTeleTile
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[SURVIVAL] No survival arena set.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Unknown survival paramter. Check '/survival cmdlist' for all commands.");
	}
}

void CGameContext::ConSpawn(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;
	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (!pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[SPAWN] you have to be logged in to use this command '/accountinfo'.");
		return;
	}
	if (pPlayer->GetMoney() < 1000000)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[SPAWN] you need at least 1 million money to use this command.");
		return;
	}
	if (pSelf->IsMinigame(pResult->m_ClientID))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[SPAWN] you can't use this command in minigames or jail.");
		return;
	}

	if (pChr->DDPP_Respawn())
		pPlayer->MoneyTransaction(-50000, "teleport to spawn");
	else
		pSelf->SendChatTarget(pResult->m_ClientID, "[SPAWN] teleport to spawn failed. Try again later.");
}

void CGameContext::ConRoom(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	char aBuf[128];
	char aCmd[64];
	str_copy(aCmd, pResult->GetString(0), sizeof(aCmd));

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Missing paramaters. Check '/room help'.");
		return;
	}


	if (!str_comp_nocase(aCmd, "help"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "*** ROOM ***");
		pSelf->SendChatTarget(pResult->m_ClientID, "The room is a special place on the map.");
		pSelf->SendChatTarget(pResult->m_ClientID, "Where special people can chill...");
		pSelf->SendChatTarget(pResult->m_ClientID, "This command allows VIP+ to invite tees to this room.");
		pSelf->SendChatTarget(pResult->m_ClientID, "*** USAGE ***");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/room invite <player>'.");
	}
	else if (!str_comp_nocase(aCmd, "invite"))
	{
		if (pResult->NumArguments() < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: Stick to construct: '/room invite <player>'.");
			return;
		}

		char aInviteName[32];
		str_copy(aInviteName, pResult->GetString(1), sizeof(aInviteName));
		int InviteID = pSelf->GetCIDByName(aInviteName);

		if (InviteID == -1)
		{
			str_format(aBuf, sizeof(aBuf), "Can't find playername: '%s'.", aInviteName);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}
		if (!pPlayer->m_IsSuperModerator)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
			return;
		}
		if (!pPlayer->m_BoughtRoom)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You need a roomkey to invite others. ('/buy room_key')");
			return;
		}
		if (pSelf->m_apPlayers[InviteID]->m_BoughtRoom)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "This player already has a room key.");
			return;
		}
		if (pSelf->GetPlayerChar(InviteID)->m_HasRoomKeyBySuperModerator)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "This player was already invited by a VIP+.");
			return;
		}
		if (!pSelf->GetPlayerChar(InviteID))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "This player is not alive... try again later.");
			return;
		}


		//GETTER
		pSelf->GetPlayerChar(InviteID)->m_HasRoomKeyBySuperModerator = true;
		str_format(aBuf, sizeof(aBuf), "'%s' invited you to the room c:.", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(InviteID, aBuf);
		//GIVER
		str_format(aBuf, sizeof(aBuf), "You invited '%s' to the room.", pSelf->Server()->ClientName(InviteID));
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
	else if (!str_comp_nocase(aCmd, "kick"))
	{
		if (pResult->NumArguments() < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: Stick construct: '/room kick <player>'.");
			return;
		}

		char aInviteName[32];
		str_copy(aInviteName, pResult->GetString(1), sizeof(aInviteName));
		int InviteID = pSelf->GetCIDByName(aInviteName);

		if (InviteID == -1)
		{
			str_format(aBuf, sizeof(aBuf), "Can't find playername: '%s'.", aInviteName);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}
		if (!pPlayer->m_IsSuperModerator)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
			return;
		}
		if (!pPlayer->m_BoughtRoom)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You need a roomkey to kick others. ('/buy room_key')");
			return;
		}
		if (pSelf->m_apPlayers[InviteID]->m_BoughtRoom)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: This player bought a room key.");
			return;
		}
		if (!pSelf->GetPlayerChar(InviteID)->m_HasRoomKeyBySuperModerator)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "This player wasn't given a key by a VIP+.");
			return;
		}
		if (!pSelf->GetPlayerChar(InviteID))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "This player is not alive... try agian later.");
			return;
		}


		//TAKEN
		pSelf->GetPlayerChar(InviteID)->m_HasRoomKeyBySuperModerator = false;
		str_format(aBuf, sizeof(aBuf), "'%s' kicked you out of room.", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		//TAKER
		str_format(aBuf, sizeof(aBuf), "You kicked '%s' out of room.", pSelf->Server()->ClientName(InviteID));
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Unknow command. Try '/room help' for more help.");
	}
}

void CGameContext::ConBank(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	char aBuf[256];

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "**** BANK ****");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bank close'");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bank open'");
		return;
	}

	if (!str_comp_nocase(pResult->GetString(0), "close"))
	{
		if (pPlayer->m_Authed != CServer::AUTHED_ADMIN) 
		{
			//pSelf->SendChatTarget(pResult->m_ClientID, "No such command: bank close.");
			pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
			return;
		}

		if (!pSelf->m_IsBankOpen)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Bank is already closed.");
			return;
		}

		pSelf->m_IsBankOpen = false;
		pSelf->SendChatTarget(pResult->m_ClientID, "<bank> bye world!");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "open"))
	{
		if (pPlayer->m_Authed != CServer::AUTHED_ADMIN)
		{
			//pSelf->SendChatTarget(pResult->m_ClientID, "No such command: bank open.");
			pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
			return;
		}

		if (pSelf->m_IsBankOpen)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Bank is already open.");
			return;
		}

		pSelf->m_IsBankOpen = true;
		pSelf->SendChatTarget(pResult->m_ClientID, "<bank> I'm open!");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "rob"))
	{
		if (!pChr->m_InBank)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You have to be in the bank.");
			return;
		}
		if (!pSelf->m_IsBankOpen)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Bank is closed.");
			return;
		}
		if (!pPlayer->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You are not logged in more info at '/accountinfo'.");
			return;
		}

		int policedudesfound = 0;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->m_PoliceRank && pSelf->m_apPlayers[i] != pPlayer)
			{
				policedudesfound++;
			}
		}

		if (!policedudesfound)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You can't rob the bank if there is no police on the server!");
			return;
		}

		pPlayer->m_EscapeTime += pSelf->Server()->TickSpeed() * 600; //+10 min
		//str_format(aBuf, sizeof(aBuf), "+%d bank robbery", 5 * policedudesfound);
		//pPlayer->MoneyTransaction(+5 * policedudesfound, aBuf);
		pPlayer->m_GangsterBagMoney += 5 * policedudesfound;
		str_format(aBuf, sizeof(aBuf), "You robbed the bank. (+%d money to your gangstabag)", 5 * policedudesfound);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Police will be hunting you for %d minutes.", (pPlayer->m_EscapeTime / pSelf->Server()->TickSpeed()) / 60);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

		str_format(aBuf, sizeof(aBuf), "'%s' robbed the bank.", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendAllPolice(aBuf);
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Unknown bank command check. '/bank' for more info.");
	}
}

void CGameContext::ConGangsterBag(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Real gangsters aren't dead or spectator.");
		return;
	}

	char aBuf[256];

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== Gangsterbag ===");
		str_format(aBuf, sizeof(aBuf), "You have got %d coins in your bag.", pPlayer->m_GangsterBagMoney);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		pSelf->SendChatTarget(pResult->m_ClientID, "Coins disappear upon disconnect.");
		pSelf->SendChatTarget(pResult->m_ClientID, "Real gangsters play 24/7 here and do illegal '/gangsterbag trade'!");
		return;
	}

	if (!str_comp_nocase(pResult->GetString(0), "trade"))
	{
		//todo: add trades with hammer to give gangsta coins to others

		// cant send yourself

		// can only trade if no escapetime

		// use brain to find bugsis

		if (!pPlayer->m_GangsterBagMoney)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You have no coins");
			return;
		}
		if (pResult->NumArguments() < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Try: '/trade <gangsta bro>'.");
			return;
		}
		if (pPlayer->m_EscapeTime)
		{
			str_format(aBuf, sizeof(aBuf), "You can't trade while escaping the police. You have to wait %d seconds...", pPlayer->m_EscapeTime / pSelf->Server()->TickSpeed());
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}

		int broID = pSelf->GetCIDByName(pResult->GetString(1));
		if (broID == -1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Getting crazy? Choose a real person...");
			return;
		}
		if (!pSelf->m_apPlayers[broID]->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Sure this is a trusty trade? He is not logged in...");
			return;
		}
		if (broID == pResult->m_ClientID)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You can only trade with other gangsters.");
			return;
		}
		char aOwnIP[128];
		char aBroIP[128];
		pSelf->Server()->GetClientAddr(pResult->m_ClientID, aOwnIP, sizeof(aOwnIP));
		pSelf->Server()->GetClientAddr(broID, aBroIP, sizeof(aBroIP));

		if (!str_comp_nocase(aOwnIP, aBroIP)) //send dummy money -> police traces ip -> dummy escape time 
		{
			//bro
			pSelf->m_apPlayers[broID]->m_GangsterBagMoney += pPlayer->m_GangsterBagMoney;
			pSelf->m_apPlayers[broID]->m_EscapeTime += pSelf->Server()->TickSpeed() * 180; // 180 secs == 3 minutes
			str_format(aBuf, sizeof(aBuf), "'%s' traded you %d gangster coins (police traced ip)", pSelf->Server()->ClientName(pResult->m_ClientID), pPlayer->m_GangsterBagMoney);
			pSelf->SendChatTarget(broID, aBuf);

			//trader
			pSelf->SendChatTarget(pResult->m_ClientID, "Police recognized the illegal trade... (ip traced)");
			pSelf->SendChatTarget(pResult->m_ClientID, "Your bro now has gangster coins and is getting hunted by police.");
			pPlayer->m_GangsterBagMoney = 0;
			pPlayer->m_EscapeTime += pSelf->Server()->TickSpeed() * 60; // +1 minutes for illegal trades
			return;
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "'%s' traded you %d money.", pSelf->Server()->ClientName(pResult->m_ClientID), pPlayer->m_GangsterBagMoney);
			pSelf->SendChatTarget(broID, aBuf);
			pSelf->m_apPlayers[broID]->MoneyTransaction(+pPlayer->m_GangsterBagMoney, "unkown source");

			pPlayer->m_GangsterBagMoney = 0;
			pSelf->SendChatTarget(pResult->m_ClientID, "You have traded coins!");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "clear"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Cleared gangsterbag ... rip coins.");
		pPlayer->m_GangsterBagMoney = 0;
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Try again with a real command.");
	}
}

void CGameContext::ConJailCode(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You have to be ingame to use this command.");
		return;
	}

	char aBuf[256];
	if (pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Usage: '/jail_code <playername>'");
		return;
	}
	if (pPlayer->m_PoliceRank < 2)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You need police rank 2 or higher.");
		return;
	}
	if (pPlayer->m_JailTime)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You are arrested.");
		return;
	}

	int jailedID = -1;
	jailedID = pSelf->GetCIDByName(pResult->GetString(0));
	if (jailedID == -1)
	{
		str_format(aBuf, sizeof(aBuf), "Can't find user '%s'", pResult->GetString(0));
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		return;
	}
	if (!pSelf->m_apPlayers[jailedID]->m_JailTime)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Player is not arrested.");
		return;
	}

	str_format(aBuf, sizeof(aBuf), "'%s' [%d]", pResult->GetString(0), pSelf->m_apPlayers[jailedID]->m_JailCode);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
}

void CGameContext::ConJail(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You have to be ingame to use this command.");
		return;
	}

	char aBuf[256];

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "---- JAIL ----");
		pSelf->SendChatTarget(pResult->m_ClientID, "The police brings all the gangster here.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/jail open <code> <player>' to open cells");
		//pSelf->SendChatTarget(pResult->m_ClientID, "'/jail list' list all jailed players"); //and for police2 list with codes
		pSelf->SendChatTarget(pResult->m_ClientID, "'/jail leave' to leave the jail");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/jail hammer' to config the police jail hammer");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/jail_code <player>' to show a certain jailcode");
		return;
	}

	if (!str_comp_nocase(pResult->GetString(0), "open"))
	{
		if (pResult->NumArguments() < 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Missing parameters. '/jail open <code> <player>'");
			return;
		}
		if (!pPlayer->GetCharacter()->m_InJailOpenArea)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Get closer to the cell.");
			return;
		}
		char aBuf[256];
		int jailedID = pSelf->GetCIDByName(pResult->GetString(2));
		if (!pSelf->m_apPlayers[jailedID])
		{
			str_format(aBuf, sizeof(aBuf), "'%s' is not online.", pResult->GetString(2));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}
		if (!pSelf->m_apPlayers[jailedID]->m_JailTime)
		{
			str_format(aBuf, sizeof(aBuf), "'%s' is not arrested.", pResult->GetString(2));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}
		if (pResult->GetInteger(1) != pSelf->m_apPlayers[jailedID]->m_JailCode)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Wrong cell code.");
			return;
		}

		str_format(aBuf, sizeof(aBuf), "You opened %s's cell.", pResult->GetString(2));
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		
		pSelf->m_apPlayers[jailedID]->m_IsJailDoorOpen = true;
		str_format(aBuf, sizeof(aBuf), "Your cell door was opened by '%s'.", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(jailedID, aBuf);
		pSelf->SendChatTarget(jailedID, "'/jail leave' to leave. (warning this might be illegal)");
	}
	//else if (!str_comp_nocase(pResult->GetString(0), "corrupt"))
	//{
	//	if (pResult->NumArguments() == 1)
	//	{
	//		pSelf->SendChatTarget(pResult->m_ClientID, "'/jail corrupt <money> <player>'");
	//		return;
	//	}

	//	int corruptID = pSelf->GetCIDByName(pResult->GetString(2));
	//	if (corruptID == -1)
	//	{
	//		str_format(aBuf, sizeof(aBuf), "'%s' is not online.", pResult->GetString(2));
	//		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	//		return;
	//	}
	//	if (!pSelf->m_apPlayers[corruptID]->m_PoliceRank)
	//	{
	//		str_format(aBuf, sizeof(aBuf), "'%s' is no police officer.", pResult->GetString(2));
	//		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	//		return;
	//	}

	//	str_format(aBuf, sizeof(aBuf), "you offered %s %d money to reduce your jailtime.", pResult->GetString(2), pResult->GetInteger(1));
	//	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

	//	str_format(aBuf, sizeof(aBuf), "'%s' would pay %d money if you help with an escape.", pResult->GetString(2), pResult->GetInteger(1));
	//	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	//	pSelf->SendChatTarget(pResult->m_ClientID, "'/jail release <jail code> %s' to take the money.");
	//}
	else if (!str_comp_nocase(pResult->GetString(0), "list")) //codes
	{
		if (pPlayer->m_JailTime || !pPlayer->m_PoliceRank)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
			return;
		}

		pSelf->SendChatTarget(pResult->m_ClientID, "coming soon");
		//list all jailed players with codes on several pages (steal bomb system)
	}
	else if (!str_comp_nocase(pResult->GetString(0), "leave"))
	{
		if (!pPlayer->m_JailTime)
		{
			//pSelf->SendChatTarget(pResult->m_ClientID, "you are not arrested.");
			return;
		}
		if (!pPlayer->m_IsJailDoorOpen)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Cell door is closed.");
			return;
		}

		pSelf->SendChatTarget(pResult->m_ClientID, "You escaped the jail, run! The police will be hunting you for 10 minutes.");
		pSelf->AddEscapeReason(pResult->m_ClientID,"jail escape");
		pPlayer->m_EscapeTime = pSelf->Server()->TickSpeed() * 600; // 10 minutes for escaping the jail
		pPlayer->m_JailTime = 0;
		pPlayer->m_IsJailDoorOpen = false;

		vec2 JailReleaseSpawn = pSelf->Collision()->GetRandomTile(TILE_JAILRELEASE);

		if (JailReleaseSpawn != vec2(-1, -1))
		{
			pChr->SetPosition(JailReleaseSpawn);
		}
		else //no jailrelease tile
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "No jailrelease tile on this map.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "hammer"))
	{
		if (!pPlayer->m_PoliceRank)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You have to be police to use this command.");
			return;
		}
		if (pResult->NumArguments() == 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "~~~ Jail Hammer ~~~");
			pSelf->SendChatTarget(pResult->m_ClientID, "Use this command to configurate your hammer.");
			pSelf->SendChatTarget(pResult->m_ClientID, "Use this hammer to move gangsters to jail.");
			pSelf->SendChatTarget(pResult->m_ClientID, "Simply activate it and hammer escaping gangsters.");
			pSelf->SendChatTarget(pResult->m_ClientID, "(you can only use it on freezed gangsters)");
			pSelf->SendChatTarget(pResult->m_ClientID, "If you are police 5 or higher then you can activate jail all hammer.");
			pSelf->SendChatTarget(pResult->m_ClientID, "Then you can jail also tees who are not known as gangsters.");
			pSelf->SendChatTarget(pResult->m_ClientID, "-- commands --");
			pSelf->SendChatTarget(pResult->m_ClientID, "'/jail hammer 1' to activate it.");
			pSelf->SendChatTarget(pResult->m_ClientID, "'/jail hammer 0' to deactivate it.");
			pSelf->SendChatTarget(pResult->m_ClientID, "'/jail hammer <seconds>' to activate jail all hammer.");
			return;
		}
		if (pResult->GetInteger(1) < 0)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "value has to be positive.");
			return;
		}
		if (pResult->GetInteger(1) == 0)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "jail hammer is now deactivated.");
			pPlayer->m_JailHammer = false;
			return;
		}
		if (pResult->GetInteger(1) == 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "jail hammer is now activated. (hammer frozen gangsters)");
			pPlayer->m_JailHammer = true;
			return;
		}
		if (pResult->GetInteger(1) > 1)
		{
			if (pPlayer->m_PoliceRank < 5)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "You have to be police rank 5 or higher to use this value.");
				return;
			}
			if (pResult->GetInteger(1) > 600)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "You can't arrest people for longer than 10 minutes.");
				return;
			}

			//pPlayer->m_JailHammerTime = pResult->GetInteger(1);
			pPlayer->m_JailHammer = pResult->GetInteger(1);
			str_format(aBuf, sizeof(aBuf), "You can now jail every freezed tee for %d seconds with your hammer.", pPlayer->m_JailHammer);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendChatTarget(pResult->m_ClientID, "You have to judge who is criminal and who is not.");
			pSelf->SendChatTarget(pResult->m_ClientID, "Much power brings much responsibility.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Unknown jail parameter check '/jail' for more info");
	}
}
void CGameContext::ConAscii(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	char aBuf[256];


	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "---- ascii art animation ----");
		pSelf->SendChatTarget(pResult->m_ClientID, "Create your own animation with this command.");
		pSelf->SendChatTarget(pResult->m_ClientID, "And publish it on your profile to share it.");
		pSelf->SendChatTarget(pResult->m_ClientID, "---- commands ----");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/ascii frame <frame number> <ascii art>' to edit a frame from 0-15.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/ascii speed <speed>' to change the animation speed.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/ascii profile <0/1>' private/publish animation on profile");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/ascii public <0/1>' private/publish animation.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/ascii view <client id>' to watch published animation.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/ascii view' to watch your animation.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/ascii stop' to stop running animation you'r watching.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/ascii stats' to see personal stats.");
		return;
	}

	if (!str_comp_nocase(pResult->GetString(0), "stats"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "---- ascii stats ----");
		str_format(aBuf, sizeof(aBuf), "views: %d", pPlayer->m_AsciiViewsDefault + pPlayer->m_AsciiViewsProfile);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		pSelf->SendChatTarget(pResult->m_ClientID, "---- specific ----");
		str_format(aBuf, sizeof(aBuf), "ascii views: %d", pPlayer->m_AsciiViewsDefault);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "ascii views (profile): %d", pPlayer->m_AsciiViewsProfile);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
	else if (!str_comp_nocase(pResult->GetString(0), "stop"))
	{
		if (pPlayer->m_AsciiWatchingID == -1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You are not watching an ascii animation.");
			return;
		}

		pPlayer->m_AsciiWatchingID = -1;
		pPlayer->m_AsciiWatchFrame = 0;
		pPlayer->m_AsciiWatchTicker = 0;
		pSelf->SendBroadcast("", pResult->m_ClientID);
	}
	else if (!str_comp_nocase(pResult->GetString(0), "profile"))
	{
		if (!pPlayer->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You have to be logged in to create ascii animations.");
			pSelf->SendChatTarget(pResult->m_ClientID, "Use '/accountinfo' for more help about accounts.");
			return;
		}

		if (pResult->NumArguments() != 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Type '/ascii profile <0/1>' to private/publish animation on yourprofile.");
			return;
		}

		if (pResult->GetInteger(1) == 0)
		{
			if (pPlayer->m_aAsciiPublishState[1] == '0')
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Your animation is already private.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Your animation is now private.");
				pSelf->SendChatTarget(pResult->m_ClientID, "It can no longer be viewed with '/profile view <you>'.");
				pPlayer->m_aAsciiPublishState[1] = '0';
			}
		}
		else if (pResult->GetInteger(1) == 1)
		{
			if (pPlayer->m_aAsciiPublishState[1] == '1')
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Your animation is already public on your profile.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Your animation is now public.");
				pSelf->SendChatTarget(pResult->m_ClientID, "It can be viewed with '/profile view <you>'.");
				pPlayer->m_aAsciiPublishState[1] = '1';
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Use 0 to make your animation private.");
			pSelf->SendChatTarget(pResult->m_ClientID, "Use 1 to make your animation public.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "public") || !str_comp_nocase(pResult->GetString(0), "publish"))
	{
		if (!pPlayer->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You have to be logged in to create ascii animations.");
			pSelf->SendChatTarget(pResult->m_ClientID, "Use '/accountinfo' for more help about accounts.");
			return;
		}

		if (pResult->NumArguments() != 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Type '/ascii public <0/1>' to private/publish animation.");
			return;
		}

		if (pResult->GetInteger(1) == 0)
		{
			if (pPlayer->m_aAsciiPublishState[0] == '0')
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Your animation is already private.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Your animation is now private.");
				pSelf->SendChatTarget(pResult->m_ClientID, "It can no longer be viewed with '/ascii view <your id>'");
				pPlayer->m_aAsciiPublishState[0] = '0';
			}
		}
		else if (pResult->GetInteger(1) == 1)
		{
			if (pPlayer->m_aAsciiPublishState[0] == '1')
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Your animation is already public.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Your animation is now public.");
				pSelf->SendChatTarget(pResult->m_ClientID, "It can be viewed with '/ascii view <your id>'");
				pPlayer->m_aAsciiPublishState[0] = '1';
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Use 0 to make your animation private.");
			pSelf->SendChatTarget(pResult->m_ClientID, "Use 1 to make your animation public.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "frame"))
	{
		if (!pPlayer->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You have to be logged in to create ascii animations.");
			pSelf->SendChatTarget(pResult->m_ClientID, "Use '/accountinfo' for more help about accounts.");
			return;
		}

		if (pResult->NumArguments() < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Type '/ascii frame <frame number> <ascii art>' to edit a frame from 0-15.");
			return;
		}

		if (pResult->GetInteger(1) == 0)
		{
			str_format(pPlayer->m_aAsciiFrame0, sizeof(pPlayer->m_aAsciiFrame0), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame0);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame0, pResult->m_ClientID);
		}
		else if (pResult->GetInteger(1) == 1)
		{
			str_format(pPlayer->m_aAsciiFrame1, sizeof(pPlayer->m_aAsciiFrame1), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame1);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame1, pResult->m_ClientID);
		}
		else if (pResult->GetInteger(1) == 2)
		{
			str_format(pPlayer->m_aAsciiFrame2, sizeof(pPlayer->m_aAsciiFrame2), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame2);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame2, pResult->m_ClientID);
		}
		else if (pResult->GetInteger(1) == 3)
		{
			str_format(pPlayer->m_aAsciiFrame3, sizeof(pPlayer->m_aAsciiFrame3), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame3);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame3, pResult->m_ClientID);
		}
		else if (pResult->GetInteger(1) == 4)
		{
			str_format(pPlayer->m_aAsciiFrame4, sizeof(pPlayer->m_aAsciiFrame4), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame4);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame4, pResult->m_ClientID);
		}
		else if (pResult->GetInteger(1) == 5)
		{
			str_format(pPlayer->m_aAsciiFrame5, sizeof(pPlayer->m_aAsciiFrame5), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame5);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame5, pResult->m_ClientID);
		}
		else if (pResult->GetInteger(1) == 6)
		{
			str_format(pPlayer->m_aAsciiFrame6, sizeof(pPlayer->m_aAsciiFrame6), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame6);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame6, pResult->m_ClientID);
		}
		else if (pResult->GetInteger(1) == 7)
		{
			str_format(pPlayer->m_aAsciiFrame7, sizeof(pPlayer->m_aAsciiFrame7), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame7);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame7, pResult->m_ClientID);
		}
		else if (pResult->GetInteger(1) == 8)
		{
			str_format(pPlayer->m_aAsciiFrame8, sizeof(pPlayer->m_aAsciiFrame8), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame8);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame8, pResult->m_ClientID);
		}
		else if (pResult->GetInteger(1) == 9)
		{
			str_format(pPlayer->m_aAsciiFrame9, sizeof(pPlayer->m_aAsciiFrame9), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame9);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame9, pResult->m_ClientID);
		}
		else if (pResult->GetInteger(1) == 10)
		{
			str_format(pPlayer->m_aAsciiFrame10, sizeof(pPlayer->m_aAsciiFrame10), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame10);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame10, pResult->m_ClientID);
		}
		else if (pResult->GetInteger(1) == 11)
		{
			str_format(pPlayer->m_aAsciiFrame11, sizeof(pPlayer->m_aAsciiFrame11), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame11);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame11, pResult->m_ClientID);
		}
		else if (pResult->GetInteger(1) == 12)
		{
			str_format(pPlayer->m_aAsciiFrame12, sizeof(pPlayer->m_aAsciiFrame12), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame12);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame12, pResult->m_ClientID);
		}
		else if (pResult->GetInteger(1) == 13)
		{
			str_format(pPlayer->m_aAsciiFrame13, sizeof(pPlayer->m_aAsciiFrame13), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame13);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame13, pResult->m_ClientID);
		}
		else if (pResult->GetInteger(1) == 14)
		{
			str_format(pPlayer->m_aAsciiFrame14, sizeof(pPlayer->m_aAsciiFrame14), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame14);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame14, pResult->m_ClientID);
		}
		else if (pResult->GetInteger(1) == 15)
		{
			str_format(pPlayer->m_aAsciiFrame15, sizeof(pPlayer->m_aAsciiFrame15), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_aAsciiFrame15);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pSelf->SendBroadcast(pPlayer->m_aAsciiFrame15, pResult->m_ClientID);
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "'%d' is no valid frame. Choose between 0 and 15.");
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "view") || !str_comp_nocase(pResult->GetString(0), "watch"))
	{
		if (pResult->NumArguments() == 1) //show own
		{
			pSelf->StartAsciiAnimation(pResult->m_ClientID, pResult->m_ClientID, -1);
			return;
		}

		pSelf->StartAsciiAnimation(pResult->m_ClientID, pResult->GetInteger(1), 0);
	}
	else if (!str_comp_nocase(pResult->GetString(0), "speed"))
	{
		if (!pPlayer->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You have to be logged in to create ascii animations.");
			pSelf->SendChatTarget(pResult->m_ClientID, "Use '/accountinfo' for more help about accounts.");
			return;
		}

		if (pResult->NumArguments() != 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Type '/ascii speed <speed>' to change the animation's speed.");
			return;
		}
		if (pResult->GetInteger(1) < 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Animation speed has to be 1 or higher.");
			return;
		}

		pPlayer->m_AsciiAnimSpeed = pResult->GetInteger(1);
		pSelf->SendChatTarget(pResult->m_ClientID, "Updated animation speed.");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Unknown ascii command. Type '/ascii' for command list.");
	}
}



void CGameContext::ConHook(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "===== hook =====");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/hook <power>'");
		pSelf->SendChatTarget(pResult->m_ClientID, "===== powers =====");
		pSelf->SendChatTarget(pResult->m_ClientID, "normal, rainbow, bloody");
		return;
	}


	if (!str_comp_nocase(pResult->GetString(0), "normal"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You got normal hook.");
		pPlayer->m_HookPower = 0;
	}
	else if (!str_comp_nocase(pResult->GetString(0), "rainbow"))
	{
		if (pPlayer->m_IsSuperModerator || pPlayer->m_IsModerator)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You got rainbow hook.");
			pPlayer->m_HookPower = 1;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "bloody"))
	{
		if (pPlayer->m_IsSuperModerator)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You got bloody hook.");
			pPlayer->m_HookPower = 2;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Unknown power. Type '/hook' for a list of all powers.");
	}
}

void CGameContext::ConReport(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	char aBuf[256];
	
	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "---- Report ----");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/report <reason> <player>'");
		pSelf->SendChatTarget(pResult->m_ClientID, "--- Reasons ---");
		pSelf->SendChatTarget(pResult->m_ClientID, "spawnblock, aimbot, flybot, spinbot, chat-spam, chat-insult");
		return;
	}

	if (pResult->NumArguments() == 2)
	{
		int repID = pSelf->GetCIDByName(pResult->GetString(1));
		if (repID == -1)
		{
			str_format(aBuf, sizeof(aBuf), "'%s' is not online.", pResult->GetString(1));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "use '/report <reason> <player>'");
		return;
	}

	if (!str_comp_nocase(pResult->GetString(0), "spawnblock"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "coming soon...");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "aimbot"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "coming soon...");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "flybot"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "coming soon...");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "spinbot"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "coming soon...");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "chat-spam"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "coming soon...");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "chat-insult"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "coming soon...");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Unknown reason. Check '/report' for all reasons.");
	}
}

void CGameContext::ConTaser(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info"))
	{
		if (pPlayer->m_TaserLevel == 0)
		{
			pPlayer->m_TaserPrice = 50000;
		}
		else if (pPlayer->m_TaserLevel == 1)
		{
			pPlayer->m_TaserPrice = 75000;
		}
		else if (pPlayer->m_TaserLevel == 2)
		{
			pPlayer->m_TaserPrice = 100000;
		}
		else if (pPlayer->m_TaserLevel == 3)
		{
			pPlayer->m_TaserPrice = 150000;
		}
		else if (pPlayer->m_TaserLevel == 4)
		{
			pPlayer->m_TaserPrice = 200000;
		}
		else if (pPlayer->m_TaserLevel == 5)
		{
			pPlayer->m_TaserPrice = 200000;
		}
		else if (pPlayer->m_TaserLevel == 6)
		{
			pPlayer->m_TaserPrice = 200000;
		}
		else
		{
			pPlayer->m_TaserPrice = 0;
		}

		char aBuf[256];

		pSelf->SendChatTarget(pResult->m_ClientID, "~~~ TASER INFO ~~~");
		pSelf->SendChatTarget(pResult->m_ClientID, "Police with rank 3 or higher are allowed to carry a taser.");
		pSelf->SendChatTarget(pResult->m_ClientID, "Taser makes rifle freeze players.");
		pSelf->SendChatTarget(pResult->m_ClientID, "~~~ YOUR TASER STATS ~~~");
		str_format(aBuf, sizeof(aBuf), "TaserLevel: %d/7", pPlayer->m_TaserLevel);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Price for the next level: %d", pPlayer->m_TaserPrice);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		//str_format(aBuf, sizeof(aBuf), "FreezeTime: %.4f seconds", (pPlayer->m_TaserLevel * 5) / pSelf->Server()->TickSpeed());
		str_format(aBuf, sizeof(aBuf), "FreezeTime: 0.%d seconds", pPlayer->m_TaserLevel * 10);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "FailRate: %d%", 0);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		pSelf->SendChatTarget(pResult->m_ClientID, "~~~ TASER COMMANDS ~~~");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/taser <on/off>' to activate/deactivate it.");
		//pSelf->SendChatTarget(pResult->m_ClientID, "'/taser <upgrade>' to level up you taser.");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "on"))
	{
		if (pPlayer->m_TaserLevel < 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You don't own a taser.");
			return;
		}
		
		pSelf->SendChatTarget(pResult->m_ClientID, "Taser activated. (Your rifle is now a taser)");
		pPlayer->m_TaserOn = true;
		return;
	}
	else if (!str_comp_nocase(pResult->GetString(0), "off"))
	{
		if (pPlayer->m_TaserLevel < 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You don't own a taser.");
			return;
		}

		pSelf->SendChatTarget(pResult->m_ClientID, "Taser deactivated. (Your rifle unfreezes again.)");
		pPlayer->m_TaserOn = false;
		return;
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid argument. Try '/taser info' for info about taser.");
	}


}

void CGameContext::ConSpawnWeaponsInfo(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	char aBuf[256];

	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ SPAWN WEAPONS INFO ~~~");
	pSelf->SendChatTarget(pResult->m_ClientID, "You can buy spawn weapons in the '/shop'.");
	pSelf->SendChatTarget(pResult->m_ClientID, "You will have the bought weapon on spawn.");
	pSelf->SendChatTarget(pResult->m_ClientID, "You can have max. 5 bullets per weapon.");
	pSelf->SendChatTarget(pResult->m_ClientID, "Each bullet costs 600.000 money.");
	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ YOUR SPAWN WEAPON STATS ~~~");
	str_format(aBuf, sizeof(aBuf), "Spawn shotgun bullets: %d", pPlayer->m_SpawnWeaponShotgun);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Spawn grenade bullets: %d", pPlayer->m_SpawnWeaponGrenade);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Spawn rifle bullets: %d", pPlayer->m_SpawnWeaponRifle);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ SPAWN WEAPON COMMANDS ~~~");
	pSelf->SendChatTarget(pResult->m_ClientID, "'/spawnweapons to activate/deactivate it.");
}

void CGameContext::ConSpookyGhostInfo(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ THE SPOOKY GHOST ~~~");
	pSelf->SendChatTarget(pResult->m_ClientID, "You can buy the spooky ghost in the '/shop'.");
	pSelf->SendChatTarget(pResult->m_ClientID, "The spooky ghost costs 1.000.000 money.");
	pSelf->SendChatTarget(pResult->m_ClientID, "If you don't have the ghost skin yet download it from 'https://ddnet.tw/skins/skin/ghost.png' and put it in your skins folder.");
	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ TOGGLE ON AND OFF ~~~");
	pSelf->SendChatTarget(pResult->m_ClientID, "You can activate and deactivate the spooky ghost by");
	pSelf->SendChatTarget(pResult->m_ClientID, "holding TAB and shooting 2 times with the pistol.");
}

void CGameContext::ConAdminChat(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (pPlayer->m_Authed != CServer::AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[ADMIN-CHAT] missing permission to use this command.");
		return;
	}
	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[ADMIN-CHAT] write a chat message that is only visible to admins.");
		return;
	}

	char aMsg[256];
	str_format(aMsg, sizeof(aMsg), "[%s]: %s", pSelf->Server()->ClientName(pResult->m_ClientID), pResult->GetString(0));
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->m_Authed == CServer::AUTHED_ADMIN) 
		{
			pSelf->SendChatTarget(i, aMsg);
		}
	}
}

void CGameContext::ConLive(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (pPlayer->m_Authed != CServer::AUTHED_ADMIN && !pPlayer->m_IsSupporter)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[LIVE] missing permission to use this command.");
		return;
	}

	char aBuf[128];

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "==== LIVE ====");
		pSelf->SendChatTarget(pResult->m_ClientID, "shows live stats about players");
		pSelf->SendChatTarget(pResult->m_ClientID, "usage: '/live (playername)'");
		return;
	}

	char aLiveName[32];
	str_format(aLiveName, sizeof(aLiveName), "%s", pResult->GetString(0));
	str_format(aBuf, sizeof(aBuf), "==== [LIVE] '%s' ====", aLiveName);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	int liveID = pSelf->GetCIDByName(aLiveName);
	if (liveID == -1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Status: OFFLINE");
		return;
	}
	pSelf->SendChatTarget(pResult->m_ClientID, "Status: ONLINE");
	CPlayer *pLive = pSelf->m_apPlayers[liveID];
	if (!pLive)
		return;

	str_format(aBuf, sizeof(aBuf), "Messages join=%s leave=%s team=%s",
		pSelf->ShowJoinMessage(pLive->GetCID()) ? "shown" : "hidden",
		pSelf->ShowLeaveMessage(pLive->GetCID()) ? "shown" : "hidden",
		pSelf->ShowTeamSwitchMessage(pLive->GetCID()) ? "shown" : "hidden");
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

	if (pLive->m_Authed)
	{
		str_format(aBuf, sizeof(aBuf), "Authed: %d", pLive->m_Authed);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
	if (pLive->IsLoggedIn())
	{
		str_format(aBuf, sizeof(aBuf), "Account: %s", pLive->m_aAccountLoginName);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "AccountID: %d", pLive->GetAccID());
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

		if (!pLive->IsQuesting())
		{
			//pSelf->SendChatTarget(pResult->m_ClientID, "Quest: FALSE"); //useless info
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "Quest: %d (level %d)", pLive->m_QuestState, pLive->m_QuestStateLevel);
			pSelf->SendChatTarget(pPlayer->GetCID(), aBuf);
			str_format(aBuf, sizeof(aBuf), "QuestStr: %s", pLive->m_aQuestString);
			pSelf->SendChatTarget(pPlayer->GetCID(), aBuf);
			if (pLive->m_aQuestProgress[0] != -1 && pLive->m_aQuestProgress[1] != -1)
			{
				str_format(aBuf, sizeof(aBuf), "QuestProgress: %d/%d", pLive->m_aQuestProgress[0], pLive->m_aQuestProgress[1]);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			}
			if (pLive->m_QuestFailed)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "QuestFailed: TRUE");
			}
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Account: NOT LOGGED IN");
	}

	if (!pLive->GetCharacter())
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Char: DEAD");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Char: ALIVE");
		if (pLive->GetCharacter()->m_DeepFreeze)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Frozen: DEEP");
		}
		else if (pLive->GetCharacter()->isFreezed)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Frozen: TRUE");
		}
		else if (pLive->GetCharacter()->m_FreezeTime)
		{
			str_format(aBuf, sizeof(aBuf), "Frozen: Freezetime: %d", pLive->GetCharacter()->m_FreezeTime);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Frozen: FALSE");
		}

		if (pLive->GetCharacter()->m_SuperJump)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "SuperJump: TRUE");
		}
		if (pLive->GetCharacter()->m_Jetpack)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Jetpack: TRUE");
		}
		if (pLive->GetCharacter()->m_EndlessHook)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Endless: TRUE");
		}
		pLive->GetCharacter()->BulletAmounts();
		str_format(aBuf, sizeof(aBuf), "Hammer[%d] Gun[%d] Ninja[%d]", pLive->GetCharacter()->HasWeapon(0), pLive->GetCharacter()->m_GunBullets, pLive->GetCharacter()->HasWeapon(5));
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "Shotgun[%d] Grenade[%d] Rifle[%d]", pLive->GetCharacter()->m_ShotgunBullets, pLive->GetCharacter()->m_GrenadeBullets, pLive->GetCharacter()->m_RifleBullets);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

		int viewers = 0;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->m_SpectatorID == liveID)
			{
				viewers++;
			}
		}
		if (viewers)
		{
			str_format(aBuf, sizeof(aBuf), "Viewers: %d", viewers);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		str_format(aBuf, sizeof(aBuf), "Position: (%.2f/%.2f)", pLive->GetCharacter()->GetPosition().x / 32, pLive->GetCharacter()->GetPosition().y / 32);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
}

void CGameContext::ConRegex(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	/*
		Since regex can be used as denial of service attack vector
		it is probably safer to make it staff only command
	*/
	if (pPlayer->m_Authed != CServer::AUTHED_ADMIN && !pPlayer->m_IsSupporter)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[REGEX] missing permission to use this command.");
		return;
	}

	if (pResult->NumArguments() != 2)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "==== Regex ====");
		pSelf->SendChatTarget(pResult->m_ClientID, "Train your POSIX regex skills or test patterns used for anti flood commands.");
		pSelf->SendChatTarget(pResult->m_ClientID, "usage: '/regex \"pattern\" \"string\"'");
		pSelf->SendChatTarget(pResult->m_ClientID, "example: '/regex \"[0-9]\" \"123\"' (match numeric)");
		pSelf->SendChatTarget(pResult->m_ClientID, "example: '/regex \"^[0-9]*x$\" \"123x\"' (match numbers followed by x)");
		return;
	}
#if defined(CONF_FAMILY_UNIX)
	int ret = regex_compile(pResult->GetString(0), pResult->GetString(1));
	if (ret == -1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[REGEX] Error: pattern compile failed.");
		return;
	}
	if (ret == 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[REGEX] (-) pattern does not match.");
		return;
	}
	if (ret == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[REGEX] (+) pattern matches.");
		return;
	}
	pSelf->SendChatTarget(pResult->m_ClientID, "[REGEX] Error: something went horribly wrong.");
#else
	pSelf->SendChatTarget(pResult->m_ClientID, "[REGEX] Error: not supported on your operating system.");
#endif
}

void CGameContext::ConMapsave(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (pPlayer->m_Authed != CServer::AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[ADMIN] Missing permission.");
		return;
	}

	char aCommand[32];
	str_copy(aCommand, pResult->GetString(0), sizeof(aCommand));

	if (pResult->NumArguments() == 0 || !str_comp_nocase(aCommand, "help"))
	{
		pSelf->SendChatTarget(ClientID, "--------  MAPSAVE  ----------");
		pSelf->SendChatTarget(ClientID, "Usage: '/mapsave [save|load|debug|players|check]'");
		pSelf->SendChatTarget(ClientID, "saves/loads state (position etc) of currently ingame players.");
		pSelf->SendChatTarget(ClientID, "Creates a binary file and it loads automatically (works with timeout codes).");
		pSelf->SendChatTarget(ClientID, "The load command is mostly for debugging because it should load from alone.");
		pSelf->SendChatTarget(ClientID, "----------------------------");
	}
	else if (!str_comp_nocase(aCommand, "load"))
	{
		pSelf->SendChatTarget(ClientID, "[MAPSAVE] loading map data...");
		pSelf->LoadMapPlayerData();
	}
	else if (!str_comp_nocase(aCommand, "save"))
	{
		pSelf->SendChatTarget(ClientID, "[MAPSAVE] saving map data...");
		pSelf->SaveMapPlayerData();
		pSelf->m_World.m_Paused = 1;
		pSelf->LogoutAllPlayersMessage();
	}
	else if (!str_comp_nocase(aCommand, "debug"))
	{
		pSelf->SendChatTarget(ClientID, "[MAPSAVE] reading map data debug (check logs)...");
		pSelf->ReadMapPlayerData(pResult->m_ClientID);
	}
	else if (!str_comp_nocase(aCommand, "players"))
	{
		pSelf->SendChatTarget(ClientID, "[MAPSAVE] listing player stats check rcon console...");
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			CPlayer *pPlayer = pSelf->m_apPlayers[i];
			if (!pPlayer)
				continue;

			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "%d:'%s' code=%s loaded=%d", i, pSelf->Server()->ClientName(i), pPlayer->m_TimeoutCode, pPlayer->m_MapSaveLoaded);
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mapsave", aBuf);
		}
	}
	else if (!str_comp_nocase(aCommand, "check"))
	{
		int NoCode = 0;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			CPlayer *pPlayer = pSelf->m_apPlayers[i];
			if (!pPlayer)
				continue;
			if (pPlayer->m_TimeoutCode[0])
				continue;

			NoCode++;
			pSelf->SendChatTarget(i, "[MAPSAVE] please type '/timeout (code)'.");
			pSelf->SendChatTarget(i, "[MAPSAVE] the admin wants to restart the server.");
			pSelf->SendChatTarget(i, "[MAPSAVE] create a code to load and save your stats.");
		}
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "[MAPSAVE] players without code: %d ('/mapsave players').", NoCode);
		pSelf->SendChatTarget(ClientID, aBuf);
	}
	else
	{
		pSelf->SendChatTarget(ClientID, "Usage: '/mapsave [save|load|debug|players|check]'");
	}
}

void CGameContext::ConShow(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	char aBuf[256];

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "====== SHOW ======");
		pSelf->SendChatTarget(pResult->m_ClientID, "Activates broadcasts.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/show <info>' to show an info.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/hide <info>' to hide an info.");
		pSelf->SendChatTarget(pResult->m_ClientID, "=== SHOWABLE INFO ===");
		if (!pPlayer->m_ShowBlockPoints)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "block_points");
		}
		if (pPlayer->m_HideBlockXp)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "block_xp");
		}
		if (!pPlayer->m_xpmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "xp");
		}
		if (pPlayer->m_hidejailmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "jail");
		}
		if (pPlayer->m_HideInsta1on1_killmessages)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "insta_killfeed");
		}
		if (pPlayer->m_HideQuestProgress)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "quest_progress");
		}
		if (pPlayer->m_HideQuestWarning)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "quest_warning");
		}
		if (!pPlayer->m_ShowInstaScoreBroadcast)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "insta_score");
		}
		return;
	}

	if (!str_comp_nocase(pResult->GetString(0), "block_points"))
	{
		if (!pPlayer->m_ShowBlockPoints)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "block_points are now activated.");
			pPlayer->m_ShowBlockPoints = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "block_points are already activated.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "block_xp"))
	{
		if (pPlayer->m_HideBlockXp)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "block_xp are now shown.");
			pPlayer->m_HideBlockXp = false;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "block_xp are already activated.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "xp"))
	{
		if (!pPlayer->m_xpmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "XP-messages are now activated.");
			pPlayer->m_xpmsg = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "XP-messages are already activated.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "jail"))
	{
		if (pPlayer->m_hidejailmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Jail-messages are now shown.");
			pPlayer->m_hidejailmsg = false;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Jail-messages are already shown.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "insta_killfeed"))
	{
		if (pPlayer->m_HideInsta1on1_killmessages)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "instagib kills are now shown.");
			pPlayer->m_HideInsta1on1_killmessages = false;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "instagib kills are already shown.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "quest_progress"))
	{
		if (pPlayer->m_HideQuestProgress)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "quest progress is now shown.");
			pPlayer->m_HideQuestProgress = false;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "quest progress is already shown.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "quest_warning"))
	{
		if (pPlayer->m_HideQuestWarning)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "quest warning is now shown.");
			pPlayer->m_HideQuestWarning = false;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "quest warning is already shown.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "insta_score"))
	{
		if (!pPlayer->m_ShowInstaScoreBroadcast)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "insta score is now shown.");
			pPlayer->m_ShowInstaScoreBroadcast = true;
			if (pPlayer->GetCharacter())
				pPlayer->GetCharacter()->m_UpdateInstaScoreBoard = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "insta score is already shown.");
		}
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "'%s' is not a valid info.", pResult->GetString(0));
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
}

void CGameContext::ConHide(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	char aBuf[256];
	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "====== HIDE ======");
		pSelf->SendChatTarget(pResult->m_ClientID, "Hides broadcasts.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/hide <info>' to hide an info.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/show <info>' to show an info.");
		pSelf->SendChatTarget(pResult->m_ClientID, "=== HIDEABLE INFO ===");
		if (pPlayer->m_ShowBlockPoints)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "block_points");
		}
		if (!pPlayer->m_HideBlockXp)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "block_xp");
		}
		if (pPlayer->m_xpmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "xp");
		}
		if (!pPlayer->m_hidejailmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "jail");
		}
		if (!pPlayer->m_HideInsta1on1_killmessages)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "insta_killfeed");
		}
		if (!pPlayer->m_HideQuestProgress)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "quest_progress");
		}
		if (!pPlayer->m_HideQuestWarning)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "quest_warning");
		}
		if (pPlayer->m_ShowInstaScoreBroadcast)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "insta_score");
		}
		return;
	}

	if (!str_comp_nocase(pResult->GetString(0), "block_points"))
	{
		if (pPlayer->m_ShowBlockPoints)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "block_points are now hidden.");
			pPlayer->m_ShowBlockPoints = false;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "block_points are already hidden.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "block_xp"))
	{
		if (!pPlayer->m_HideBlockXp)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "block_xp are now hidden.");
			pPlayer->m_HideBlockXp = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "block_xp are already hidden.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "xp"))
	{
		if (pPlayer->m_xpmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "XP-messages are now hidden.");
			pPlayer->m_xpmsg = false;
			pSelf->SendBroadcast("", pPlayer->GetCID(), 0); //send empty broadcast without importance to delete last xp message
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "XP-messages are already hidden.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "jail"))
	{
		if (!pPlayer->m_hidejailmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Jail-messages are now hidden.");
			pPlayer->m_hidejailmsg = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Jail-messages are already hidden.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "insta_killfeed"))
	{
		if (!pPlayer->m_HideInsta1on1_killmessages)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "instagib kills are now hidden.");
			pPlayer->m_HideInsta1on1_killmessages = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "instagib kills are already hidden.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "quest_progress"))
	{
		if (!pPlayer->m_HideQuestProgress)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "quest progress is now hidden.");
			pPlayer->m_HideQuestProgress = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "quest progress is already hidden.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "quest_warning"))
	{
		if (!pPlayer->m_HideQuestWarning)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "quest warning is now hidden.");
			pPlayer->m_HideQuestWarning = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "quest warning is already hidden.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "insta_score"))
	{
		if (pPlayer->m_ShowInstaScoreBroadcast)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "insta score is now hidden.");
			pPlayer->m_ShowInstaScoreBroadcast = false;
			pSelf->SendBroadcast("", pPlayer->GetCID(), 0);
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "insta score is already hidden.");
		}
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "'%s' is not a valid info.", pResult->GetString(0));
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
}

void CGameContext::ConQuest(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You have to be alive to use this command.");
		return;
	}

	/*
	if (pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You have to be logged in to use this command. (type '/accountinfo' for more info)");
		return;
	}
	*/

	char aBuf[128];

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "====== Q U E S T =====");
		if (!pPlayer->IsQuesting())
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "No running quest...");
			pSelf->SendChatTarget(pResult->m_ClientID, "Use '/quest start' to start one.");
		}
		else 
		{
			str_format(aBuf, sizeof(aBuf), "===QUEST[%d]==LEVEL[%d]===", pPlayer->m_QuestState, pPlayer->m_QuestStateLevel);
			pSelf->SendChatTarget(pPlayer->GetCID(), aBuf);
			pSelf->SendChatTarget(pResult->m_ClientID, pPlayer->m_aQuestString);
			if (pPlayer->m_aQuestProgress[0] != -1 && pPlayer->m_aQuestProgress[1] != -1)
			{
				str_format(aBuf, sizeof(aBuf), "Quest progress %d/%d completed.", pPlayer->m_aQuestProgress[0], pPlayer->m_aQuestProgress[1]);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			}
			if (pPlayer->m_QuestFailed)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "WARNING: Quest has failed. Start agian.");
			}
		}
		pSelf->SendChatTarget(pResult->m_ClientID, "========================");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/quest help' for more info.");
		return;
	}

	if (!str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info") || !str_comp_nocase(pResult->GetString(0), "cmdlist") || !str_comp_nocase(pResult->GetString(0), "man") || !str_comp_nocase(pResult->GetString(0), "?"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "====== Q U E S T =====");
		pSelf->SendChatTarget(pResult->m_ClientID, "Complete quests and get rewards.");
		pSelf->SendChatTarget(pResult->m_ClientID, "==== commands ====");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/quest' to get quest status.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/quest start' to start a quest.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/quest stop' to stop a quest.");
		//pSelf->SendChatTarget(pResult->m_ClientID, "'/quest skip' to skip a quest");
		//pSelf->SendChatTarget(pResult->m_ClientID, "'/quest level' to change difficulty");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "start") || !str_comp_nocase(pResult->GetString(0), "begin"))
	{
		if (pPlayer->IsQuesting())
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Quest is already running.");
			return;
		}
		pSelf->CheckConnectQuestBot();
		// load / activate QuestState
		if (pPlayer->m_QuestUnlocked) // if saved stats --> load stats
		{
			pPlayer->m_QuestState = pPlayer->m_QuestUnlocked;
		}
		else // no saved stats --> start quest1
		{
			pPlayer->m_QuestState = 1;
		}
		// load Quest Level
		pPlayer->m_QuestStateLevel = pPlayer->m_QuestLevelUnlocked;

		pSelf->SendChatTarget(pResult->m_ClientID, "[QUEST] started ..."); // print this before the actual start because the start can drop an error and we dont want this log : "[QUEST] ERROR STOPPED [QUEST] Started.." we want this log: "[QUEST] Started.. [QUEST] ERROR STOPPED"
		pSelf->StartQuest(pResult->m_ClientID);
	}
	else if (!str_comp_nocase(pResult->GetString(0), "stop"))
	{
		if (!pPlayer->IsQuesting())
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Quest already stopped.");
			return;
		}

		pPlayer->m_QuestState = CPlayer::QUEST_OFF;
		pPlayer->m_QuestStateLevel = 0;
		pSelf->SendChatTarget(pResult->m_ClientID, "[QUEST] Quest stopped.");
	}
	//else if (!str_comp_nocase(pResult->GetString(0), "level"))
	//{
	//	if (pPlayer->m_QuestLevelUnlocked < pResult->GetInteger(1))
	//	{
	//		str_format(aBuf, sizeof(aBuf), "Unlock this level first by completing level %d.", pResult->GetInteger(1) - 1);
	//		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	//		return;
	//	}
	//	if (pResult->GetInteger(1) < 0)
	//	{
	//		pSelf->SendChatTarget(pResult->m_ClientID, "You are to unexperienced to play these kind of quests...");
	//		return;
	//	}

	//	pPlayer->m_QuestState = pResult->GetInteger(1);
	//	str_format(aBuf, sizeof(aBuf), "Updated quest difficulty to %d.", pResult->GetInteger(1));
	//	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	//}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Unknown quest command. Type '/quest help' for more info.");
	}
}

void CGameContext::ConBounty(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	char aBuf[128];

	if (pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "----> Bounty <----");
		pSelf->SendChatTarget(pResult->m_ClientID, "Use the bounty command to pay money in a pricepool.");
		pSelf->SendChatTarget(pResult->m_ClientID, "And then choose a player.");
		pSelf->SendChatTarget(pResult->m_ClientID, "And the tee who blocks your choosen player");
		pSelf->SendChatTarget(pResult->m_ClientID, "Gets all the money from the pool.");
		pSelf->SendChatTarget(pResult->m_ClientID, "write '/bounty cmdlist' for all commands");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "==== BOUNTY COMMANDS ====");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bounty cmdlist' shows this list");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bounty help' shows some general info");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bounty add <amount> <player>' to add a bounty to a player");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bounty check <clientID>' to check the total bounty amount");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "add"))
	{
		if (pResult->NumArguments() == 3)
		{
			if (pPlayer->GetMoney() < pResult->GetInteger(1))
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[BOUNTY] You don't have that much money.");
				return;
			}

			if (pResult->GetInteger(1) < 200)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[BOUNTY] Minimum amount for bountys is 200.");
				return;
			}

			int ID = pSelf->GetCIDByName(pResult->GetString(2));
			if (ID == -1)
			{
				str_format(aBuf, sizeof(aBuf), "[BOUNTY] Player '%s' not found.", pResult->GetString(2));
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			}
			else if (pSelf->IsSameIP(ID, pResult->m_ClientID))
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[BOUNTY] You can't add bounty to your dummy.");
			}
			else if (pResult->GetInteger(1) < 1)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[BOUNTY] You can't add less than 1 money as bounty.");
			}
			else
			{
				pSelf->m_apPlayers[ID]->m_BlockBounty += pResult->GetInteger(1);
				str_format(aBuf, sizeof(aBuf), "-%d bounty (%s)", pResult->GetInteger(1), pSelf->Server()->ClientName(ID));
				pPlayer->MoneyTransaction(-pResult->GetInteger(1), aBuf);
				str_format(aBuf, sizeof(aBuf), "[BOUNTY] added %d money to '%s' (%d total)", pResult->GetInteger(1), pSelf->Server()->ClientName(ID), pSelf->m_apPlayers[ID]->m_BlockBounty);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				str_format(aBuf, sizeof(aBuf), "[BOUNTY] '%s' added %d money to '%s's bounty (%d total)", pSelf->Server()->ClientName(pResult->m_ClientID), pResult->GetInteger(1), pSelf->Server()->ClientName(ID), pSelf->m_apPlayers[ID]->m_BlockBounty);
				pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BOUNTY] Use: '/bounty add <amount> <player>'");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "check"))
	{
		if (pResult->NumArguments() == 2)
		{

			int ID = -1;
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (pSelf->m_apPlayers[i])
				{
					if (i == pResult->GetInteger(1))
					{
						ID = i;
						break;
					}
				}
			}

			if (ID == -1)
			{
				str_format(aBuf, sizeof(aBuf), "[BOUNTY] Player with id %d not found.", pResult->GetInteger(1));
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "[BOUNTY] '%s' has a bounty of %d money", pSelf->Server()->ClientName(ID), pSelf->m_apPlayers[ID]->m_BlockBounty);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BOUNTY] Use: '/bounty check <clientID>'");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[BOUNTY] Unknown command. write '/bounty cmdlist' for all commands");
	}
}

void CGameContext::ConDcDummy(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;
	if (pPlayer->m_Authed != CServer::AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[dummy] You have to be admin to use this command.");
		return;
	}

	int DummyID = pResult->GetInteger(0);
	char aBuf[128];
	CPlayer *pDummy = pSelf->m_apPlayers[DummyID];
	if (!pDummy)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[dummy] this id is not online.");
		return;
	}
	if (!pDummy->m_IsDummy)
	{
		str_format(aBuf, sizeof(aBuf), "[dummy] player '%s' is not a dummy.", pSelf->Server()->ClientName(DummyID));
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		return;
	}
	str_format(aBuf, sizeof(aBuf), "[dummy] dummy '%s' disconnected.", pSelf->Server()->ClientName(DummyID));
	pSelf->Server()->BotLeave(DummyID);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
}

void CGameContext::ConTROLL166(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	if (!pSelf->Server()->IsAuthed(pResult->m_ClientID))
		return;

	int VictimCID = pResult->GetVictim();
	CPlayer *pPlayer = pSelf->m_apPlayers[VictimCID];
	if (pPlayer) {
		pPlayer->m_TROLL166 = pResult->GetInteger(0);
	}
}

void CGameContext::ConTROLL420(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	if (!pSelf->Server()->IsAuthed(pResult->m_ClientID))
		return;

	int VictimCID = pResult->GetVictim();
	CPlayer *pPlayer = pSelf->m_apPlayers[VictimCID];
	if (pPlayer) {
		pPlayer->m_TROLL420 = pResult->GetInteger(0);
		pSelf->SendTuningParams(VictimCID);
	}
}

void CGameContext::ConTrade(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[TRADE] you have to be alive to use this command.");
		return;
	}

	if (pPlayer->m_SpookyGhostActive)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[TRADE] you can't trade as the spooky ghost.");
		return;
	}

	if (!g_Config.m_SvAllowTrade)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[TRADE] this command is deactivated by an administrator.");
		return;
	}

	if (pSelf->IsMinigame(pResult->m_ClientID))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[TRADE] you can't use this command in minigames or jail.");
		return;
	}

	char aBuf[256];

	if (pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== TRADE ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "Use this command to trade");
		pSelf->SendChatTarget(pResult->m_ClientID, "Weapons and items");
		pSelf->SendChatTarget(pResult->m_ClientID, "With other players on the server");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/trade cmdlist' for all commands");
		if (str_comp_nocase(pPlayer->m_aTradeOffer, "")) //not empty trade offer
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "=== latest trade ===");
			pSelf->SendChatTarget(pResult->m_ClientID, pPlayer->m_aTradeOffer);
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "==== TRADE commands ====");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/trade sell <item> <price> <player>' to send a player a trade offer");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/trade sell_public <item> <price>' to create a public sell offer everybody could accept");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/trade buy <item> <price> <player>' to accept a trade offer");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/tr <player>' fast accept latest trade (WARNING YOU COULD BE SCAMMED!)");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/trade items' for a full list of tradable items");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/trade cmdlist' shows this list");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "items"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== TRADE ITEMS ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "shotgun");
		pSelf->SendChatTarget(pResult->m_ClientID, "grenade");
		pSelf->SendChatTarget(pResult->m_ClientID, "rifle");
		pSelf->SendChatTarget(pResult->m_ClientID, "all_weapons");
		//pSelf->SendChatTarget(pResult->m_ClientID, "homing missiles ammo"); //coming soon...
	}
	else if (!str_comp_nocase(pResult->GetString(0), "sell_public"))
	{
		char aWeaponName[64];
		int weapon = pSelf->TradeItemToInt(pResult->GetString(1));
		int Price = pResult->GetInteger(2);
		str_format(aWeaponName, sizeof(aWeaponName), pResult->GetString(1));

		int TradeID = pSelf->TradePrepareSell(/*pToName=*/pResult->GetString(3), /*FromID=*/pResult->m_ClientID, aWeaponName, Price, /*Public=*/true);
		if (TradeID == -1) { return; }

		//send trade info to the invited player
		str_format(aBuf, sizeof(aBuf), "[TRADE] '%s' created a public offer [ %s ] for [ %d ] money (use '/trade' command to accept it)", pSelf->Server()->ClientName(pResult->m_ClientID), aWeaponName, pResult->GetInteger(2));
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		//and save it for all players so it can be seen later in '/trade'
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (pSelf->m_apPlayers[i])
			{
				if (i != pResult->m_ClientID) // don't offer own trade
				{
					str_format(pSelf->m_apPlayers[i]->m_aTradeOffer, sizeof(pSelf->m_apPlayers[i]->m_aTradeOffer), "[OPEN] '/trade buy %s %d %s'", aWeaponName, pResult->GetInteger(2), pSelf->Server()->ClientName(pResult->m_ClientID));
				}
			}
		}

		//send same info to the trading dude (he gets the public message)
		//str_format(aBuf, sizeof(aBuf), "[TRADE] you offered to all players [ %s ] for [ %d ] money", aWeaponName, pResult->GetInteger(2));
		//pSelf->SendChatTarget(pResult->m_ClientID, aBuf);

		//save trade to vars SELLER
		pPlayer->m_TradeItem = weapon;
		pPlayer->m_TradeMoney = pResult->GetInteger(2);
		pPlayer->m_TradeID = -1;
		pPlayer->m_TradeTick = pSelf->Server()->Tick() + pSelf->Server()->TickSpeed() * 2 * 60;
	}
	else if (!str_comp_nocase(pResult->GetString(0), "sell"))
	{
		char aWeaponName[64];
		int weapon = pSelf->TradeItemToInt(pResult->GetString(1));
		int Price = pResult->GetInteger(2);
		str_format(aWeaponName, sizeof(aWeaponName), pResult->GetString(1));

		int TradeID = pSelf->TradePrepareSell(/*pToName=*/pResult->GetString(3), /*FromID=*/pResult->m_ClientID, aWeaponName, Price, /*Public=*/false);
		if (TradeID == -1) { return; }

		//send trade info to the invited player
		str_format(aBuf, sizeof(aBuf), "[TRADE] '%s' offered you [ %s ] for [ %d ] money", pSelf->Server()->ClientName(pResult->m_ClientID), aWeaponName, pResult->GetInteger(2));
		pSelf->SendChatTarget(TradeID, aBuf);
		//and save it so it can be seen later in '/trade'
		str_format(pSelf->m_apPlayers[TradeID]->m_aTradeOffer, sizeof(pSelf->m_apPlayers[TradeID]->m_aTradeOffer), "[OPEN] '/trade buy %s %d %s'", aWeaponName, pResult->GetInteger(2), pSelf->Server()->ClientName(pResult->m_ClientID));

		//send same info to the trading dude
		str_format(aBuf, sizeof(aBuf), "[TRADE] you offered '%s' [ %s ] for [ %d ] money", pResult->GetString(3), aWeaponName, pResult->GetInteger(2));
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);


		//save trade to vars SELLER
		pPlayer->m_TradeItem = weapon;
		pPlayer->m_TradeMoney = pResult->GetInteger(2);
		pPlayer->m_TradeID = TradeID;
		pPlayer->m_TradeTick = pSelf->Server()->Tick() + pSelf->Server()->TickSpeed() * 1 * 60;

		//save trade to vars BUYER (makes no sense on multiple offers to one person. better check the command parameters of the buyer)
		//pSelf->m_apPlayers[TradeID]->m_TradeItem = weapon;
		//pSelf->m_apPlayers[TradeID]->m_TradeMoney = pResult->GetInteger(2);
	}
	else if (!str_comp_nocase(pResult->GetString(0), "buy"))
	{
		int TradeID = pSelf->GetCIDByName(pResult->GetString(3));
		int weapon = pSelf->TradeItemToInt(pResult->GetString(1));

		if (pSelf->TradePrepareBuy(pResult->m_ClientID, pResult->GetString(3), weapon)) { return; }

		if (pSelf->m_apPlayers[TradeID]->m_TradeItem != weapon ||
			pSelf->m_apPlayers[TradeID]->m_TradeMoney != pResult->GetInteger(2))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[TRADE] the trade you accepted doesn't exist.");
			return;
		}

		//##############
		// TRADE SUCCESS
		//##############

		//buyer
		str_format(aBuf, sizeof(aBuf), "[TRADE] you sucessfully bought [ %s ] for [ %d ] from player '%s'.", pResult->GetString(1), pSelf->m_apPlayers[TradeID]->m_TradeMoney, pSelf->Server()->ClientName(TradeID));
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "trade [%s] from [%s]", pResult->GetString(1), pSelf->Server()->ClientName(TradeID));
		pPlayer->MoneyTransaction(-pSelf->m_apPlayers[TradeID]->m_TradeMoney, aBuf);
		pPlayer->m_TradeItem = -1;
		pPlayer->m_TradeMoney = -1;
		pPlayer->m_TradeID = -1;
		str_copy(pPlayer->m_aTradeOffer, "", sizeof(pPlayer->m_aTradeOffer));
		if (weapon == 2)
		{
			pChr->GiveWeapon(2);
			pPlayer->m_SpawnShotgunActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_SHOTGUN] = false;
		}
		if (weapon == 3)
		{
			pChr->GiveWeapon(3);
			pPlayer->m_SpawnGrenadeActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_GRENADE] = false;
		}
		if (weapon == 4)
		{
			pChr->GiveWeapon(4);
			pPlayer->m_SpawnRifleActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_RIFLE] = false;
		}
		else if (weapon == 5)
		{
			pChr->GiveWeapon(2);
			pChr->GiveWeapon(3);
			pChr->GiveWeapon(4);
			pPlayer->m_SpawnShotgunActive = 0;
			pPlayer->m_SpawnGrenadeActive = 0;
			pPlayer->m_SpawnRifleActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_RIFLE] = false;
			pChr->m_aDecreaseAmmo[WEAPON_GRENADE] = false;
			pChr->m_aDecreaseAmmo[WEAPON_SHOTGUN] = false;
		}

		//seller
		str_format(aBuf, sizeof(aBuf), "[TRADE] you sucessfully sold [ %s ] for [ %d ] to player '%s'.", pResult->GetString(1), pSelf->m_apPlayers[TradeID]->m_TradeMoney, pSelf->Server()->ClientName(pPlayer->GetCID()));
		pSelf->SendChatTarget(TradeID, aBuf);
		str_format(aBuf, sizeof(aBuf), "trade [%s] to [%s]", pResult->GetString(1), pSelf->Server()->ClientName(pPlayer->GetCID()));
		pSelf->m_apPlayers[TradeID]->MoneyTransaction(+pSelf->m_apPlayers[TradeID]->m_TradeMoney, aBuf);
		pSelf->m_apPlayers[TradeID]->m_TradeItem = -1;
		pSelf->m_apPlayers[TradeID]->m_TradeMoney = -1;
		pSelf->m_apPlayers[TradeID]->m_TradeID = -1;
		if (weapon == 2 || weapon == 3 || weapon == 4)
		{
			pSelf->m_apPlayers[TradeID]->GetCharacter()->SetActiveWeapon(WEAPON_GUN);
			pSelf->m_apPlayers[TradeID]->GetCharacter()->SetWeaponGot(weapon, false);
		}
		else if (weapon == 5)
		{
			pSelf->m_apPlayers[TradeID]->GetCharacter()->SetActiveWeapon(WEAPON_GUN);
			pSelf->m_apPlayers[TradeID]->GetCharacter()->SetWeaponGot(2, false);
			pSelf->m_apPlayers[TradeID]->GetCharacter()->SetWeaponGot(3, false);
			pSelf->m_apPlayers[TradeID]->GetCharacter()->SetWeaponGot(4, false);
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[TRADE] unknown command. try '/trade cmdlist' for more help.");
	}
}

void CGameContext::ConTr(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[TRADE] you have to be alive to use this command.");
		return;
	}

	if (pPlayer->m_SpookyGhostActive)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[TRADE] you can't trade as the spooky ghost.");
		return;
	}

	if (!g_Config.m_SvAllowTrade)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[TRADE] this command is deactivated by an administrator.");
		return;
	}

	if (pSelf->IsMinigame(pResult->m_ClientID))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[TRADE] you can't use this command in minigames or jail.");
		return;
	}

	char aBuf[256];

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== TRADE (UNSAFE VERSION) ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "For information and the saver version");
		pSelf->SendChatTarget(pResult->m_ClientID, "use the '/trade' command");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/tr <player>' to accept the latest offer from the player");
		pSelf->SendChatTarget(pResult->m_ClientID, "WARNING the player could send a new offer with higher costs!");
		if (str_comp_nocase(pPlayer->m_aTradeOffer, "")) //not empty trade offer
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "=== latest trade ===");
			pSelf->SendChatTarget(pResult->m_ClientID, pPlayer->m_aTradeOffer);
		}
	}
	else if (pResult->NumArguments() == 1)
	{
		int TradeID = pSelf->GetCIDByName(pResult->GetString(0));
		if (TradeID == -1)
		{
			str_format(aBuf, sizeof(aBuf), "[TRADE] player '%s' is not online.", pResult->GetString(0));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}
		int weapon = pSelf->m_apPlayers[TradeID] ? pSelf->m_apPlayers[TradeID]->m_TradeItem : -1;
		char aWeaponName[64]; //calculate trading string
		str_format(aWeaponName, sizeof(aWeaponName), pSelf->TradeItemToStr(weapon));

		if (pSelf->TradePrepareBuy(pResult->m_ClientID, pResult->GetString(0), weapon)) { return; }

		if (pSelf->m_apPlayers[TradeID]->m_TradeMoney > 5000)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[TRADE] use the '/trade' command for trades with over 5000 money.");
			return;
		}

		//##############
		// TRADE SUCCESS
		//##############

		//buyer
		str_format(aBuf, sizeof(aBuf), "[TRADE] you sucessfully bought [ %s ] for [ %d ] from player '%s'.", aWeaponName, pSelf->m_apPlayers[TradeID]->m_TradeMoney, pSelf->Server()->ClientName(TradeID));
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "trade [%s] from [%s]", aWeaponName, pSelf->Server()->ClientName(TradeID));
		pPlayer->MoneyTransaction(-pSelf->m_apPlayers[TradeID]->m_TradeMoney, aBuf);
		pPlayer->m_TradeItem = -1;
		pPlayer->m_TradeMoney = -1;
		pPlayer->m_TradeID = -1;
		str_copy(pPlayer->m_aTradeOffer, "", sizeof(pPlayer->m_aTradeOffer));
		if (weapon == 2)
		{
			pChr->GiveWeapon(2);
			pPlayer->m_SpawnShotgunActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_SHOTGUN] = false;
		}
		if (weapon == 3)
		{
			pChr->GiveWeapon(3);
			pPlayer->m_SpawnGrenadeActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_GRENADE] = false;
		}
		if (weapon == 4)
		{
			pChr->GiveWeapon(4);
			pPlayer->m_SpawnRifleActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_RIFLE] = false;
		}
		else if (weapon == 5)
		{
			pChr->GiveWeapon(2);
			pChr->GiveWeapon(3);
			pChr->GiveWeapon(4);
			pPlayer->m_SpawnShotgunActive = 0;
			pPlayer->m_SpawnGrenadeActive = 0;
			pPlayer->m_SpawnRifleActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_RIFLE] = false;
			pChr->m_aDecreaseAmmo[WEAPON_GRENADE] = false;
			pChr->m_aDecreaseAmmo[WEAPON_SHOTGUN] = false;
		}

		//seller
		str_format(aBuf, sizeof(aBuf), "[TRADE] you sucessfully sold [ %s ] for [ %d ] to player '%s'.", aWeaponName, pSelf->m_apPlayers[TradeID]->m_TradeMoney, pSelf->Server()->ClientName(pPlayer->GetCID()));
		pSelf->SendChatTarget(TradeID, aBuf);
		str_format(aBuf, sizeof(aBuf), "trade [%s] to [%s]", aWeaponName, pSelf->Server()->ClientName(pPlayer->GetCID()));
		pSelf->m_apPlayers[TradeID]->MoneyTransaction(+pSelf->m_apPlayers[TradeID]->m_TradeMoney, aBuf);
		pSelf->m_apPlayers[TradeID]->m_TradeItem = -1;
		pSelf->m_apPlayers[TradeID]->m_TradeMoney = -1;
		pSelf->m_apPlayers[TradeID]->m_TradeID = -1;
		if (weapon == 2 || weapon == 3 || weapon == 4)
		{
			pSelf->m_apPlayers[TradeID]->GetCharacter()->SetActiveWeapon(WEAPON_GUN);
			pSelf->m_apPlayers[TradeID]->GetCharacter()->SetWeaponGot(weapon, false);
		}
		else if (weapon == 5)
		{
			pSelf->m_apPlayers[TradeID]->GetCharacter()->SetActiveWeapon(WEAPON_GUN);
			pSelf->m_apPlayers[TradeID]->GetCharacter()->SetWeaponGot(2, false);
			pSelf->m_apPlayers[TradeID]->GetCharacter()->SetWeaponGot(3, false);
			pSelf->m_apPlayers[TradeID]->GetCharacter()->SetWeaponGot(4, false);
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[TRADE] something really went wrong please contact an administrator.");
	}
}

void CGameContext::ConBlockWave(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[BlockWave] you have to be alive to use this command.");
		return;
	}

	if (!g_Config.m_SvAllowBlockWave)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[BlockWave] this command is disabled by an administator.");
		return;
	}

	if (!str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info") || pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== Block Wave ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "Block minigame by ChillerDragon.");
		pSelf->SendChatTarget(pResult->m_ClientID, "Survive waves of blocker bots.");
		pSelf->SendChatTarget(pResult->m_ClientID, "start with '/blockwave join'");
		pSelf->SendChatTarget(pResult->m_ClientID, "check all cmds with ''/blockwave cmdlist'");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== BlockWave cmds ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/blockwave help' for info and help");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/blockwave cmdlist' to show this list");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/blockwave join' to join the game");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/blockwave leave' to leave the game");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/blockwave status' to show cureent game status");
		//pSelf->SendChatTarget(pResult->m_ClientID, "'/blockwave stats' to check your stats"); //coming soon...
		//pSelf->SendChatTarget(pResult->m_ClientID, "'/blockwave shop' to show list of items"); //coming soon...
		//pSelf->SendChatTarget(pResult->m_ClientID, "'/blockwave buy <item>' to buy shop items"); //coming soon...
	}
	else if (!str_comp_nocase(pResult->GetString(0), "status"))
	{
		if (!pSelf->m_BlockWaveGameState)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BlockWave] No game running right now. Feel free to create one with '/blockwave join'");
		}
		else if (pSelf->m_BlockWaveGameState == 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BlockWave] Game starting right now. Feel free to join with '/blockwave join'");
		}
		else if (pSelf->m_BlockWaveGameState == 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BlockWave] Game running right now. Feel free to join with '/blockwave join'");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BlockWave] unknown status.");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "join"))
	{
		if (pPlayer->m_IsBlockWaving)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BlockWave] you already joined a game.");
			return;
		}

		if (pSelf->m_BlockWaveGameState == 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BlockWave] round running... you will join automatically when a new round starts.");
			pPlayer->m_IsBlockWaving = true;
			pPlayer->m_IsBlockWaveWaiting = true;
		}
		else if (pSelf->IsMinigame(pResult->m_ClientID))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BlockWave] error. maybe you are already in a minigame or jail. (check '/minigames status')");
		}
		else
		{
			if (!pSelf->m_BlockWaveGameState) //no game? --> start one
			{
				pSelf->StartBlockWaveGame();
			}
			pSelf->SendChatTarget(pResult->m_ClientID, "[BlockWave] joined the arena! hf & gl staying alive.");
			pPlayer->m_IsBlockWaving = true;
			pPlayer->m_IsBlockWaveWaiting = false;
			pChr->Die(pPlayer->GetCID(), WEAPON_SELF);
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "leave"))
	{
		if (pPlayer->m_IsBlockWaving)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BlockWave] you left the game.");
			pPlayer->m_IsBlockWaving = false;
			pChr->Die(pPlayer->GetCID(), WEAPON_SELF);
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[BlockWave] you currently aren't playing BlockWave.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[BlockWave] uknown parameter. check '/blockwave cmdlist'");
	}
}

void CGameContext::ConBroadcastServer(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	if (!pPlayer->m_IsSuperModerator)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission. You are not a VIP+.");
		return;
	}

	/*
	if (pSelf->m_iBroadcastDelay)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[broadcast_srv] could't sent broadcast because someone recived an important broadcast latley");
		return;
	}
	*/

	//str_format(pSelf->aBroadcastMSG, sizeof(pSelf->aBroadcastMSG), pResult->GetString(0));
	pSelf->SendBroadcastAll(pResult->GetString(0), 1, true); //send as important broadcast
}

void CGameContext::ConFng(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;


	if (pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== FNG INFO ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "Configurate some settings for the fng minigame.");
		pSelf->SendChatTarget(pResult->m_ClientID, "Use '/insta fng' or '/insta boomfng' to play fng.");
		pSelf->SendChatTarget(pResult->m_ClientID, "For all possible settings check '/fng cmdlist'");
	}
	else if (!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== FNG SETTINGS ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/fng autojoin <value>' 0=off 1=join fng 2=join boomfng on login");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/fng hammertune <value>' 0=vanilla 1=fng");
		if (pPlayer->m_Authed == CServer::AUTHED_ADMIN)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "'/fng bots <amount> <mode 4/5>' to connect bots for 4=grenade 5=rifle");
		}
	}
	else if (!str_comp_nocase(pResult->GetString(0), "bots"))
	{
		if (pPlayer->m_Authed != CServer::AUTHED_ADMIN)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[FNG] missing permission.");
			return;
		}
		if (pResult->NumArguments() != 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "[FNG] 3 arguments required.");
			return;
		}
		if (pResult->GetInteger(2) == 4)
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "[FNG] connecting %d grenade bots", pResult->GetInteger(1));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		else if (pResult->GetInteger(2) == 5)
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "[FNG] connecting %d rifle bots", pResult->GetInteger(1));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		else
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "[FNG] %d is an unspported mode (choose between 4 and 5)", pResult->GetInteger(2));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}
		pSelf->ConnectFngBots(pResult->GetInteger(1), pResult->GetInteger(2));
	}
	else if (!str_comp_nocase(pResult->GetString(0), "autojoin"))
	{
		if (pResult->NumArguments() > 1)
		{
			if (pResult->GetInteger(1) == 0)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[FNG] you are no longer joining games on account login.");
				pPlayer->m_aFngConfig[0] = '0';
				return;
			}
			else if (pResult->GetInteger(1) == 1)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[FNG] you are now automatically joining fng on account login.");
				pPlayer->m_aFngConfig[0] = '1';
				return;
			}
			else if (pResult->GetInteger(1) == 2)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[FNG] you are now automatically joining boomfng on account login.");
				pPlayer->m_aFngConfig[0] = '2';
				return;
			}
		}
		pSelf->SendChatTarget(pResult->m_ClientID, "[FNG] use '/fng autojoin <value>' and <value> has to be between 0 and 2");
		return;
	}
	else if (!str_comp_nocase(pResult->GetString(0), "hammertune"))
	{
		if (pResult->NumArguments() > 1)
		{
			if (pResult->GetInteger(1) == 0)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[FNG] you are now using VANILLA hammer.");
				pPlayer->m_aFngConfig[1] = '0';
				return;
			}
			else if (pResult->GetInteger(1) == 1)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "[FNG] you are now using FNG hammer.");
				pPlayer->m_aFngConfig[1] = '1';
				return;
			}
		}
		pSelf->SendChatTarget(pResult->m_ClientID, "[FNG] use '/fng hammertune <value>' and <value> has to be 1 or 0");
		return;
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[FNG] unknown command check '/fng help' or '/fng cmdlist'.");
	}
}



void CGameContext::ConSQLLogout(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (pPlayer->m_Authed == CServer::AUTHED_ADMIN)
	{
		//admins are allowed
	}
	else
	{
		if (g_Config.m_SvSupAccReset > 0) // 1 or 2 are allowed to use this
		{
			if (pPlayer->m_IsSupporter)
			{
				//supporters are allowed
			}
			else
			{
				pSelf->SendChatTarget(ClientID, "[SQL] Missing permission.");
				return;
			}
		}
		else
		{
			pSelf->SendChatTarget(ClientID, "[SQL] Missing permission.");
			return;
		}
	}

	if (pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "=== SQL logout ===");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/sql_logout <account_name>' to set acc logged out in db");
		pSelf->SendChatTarget(pResult->m_ClientID, "WARNING!!! this command can mess things up!");
		pSelf->SendChatTarget(pResult->m_ClientID, "try '/sql_logout_all' first because it is more save.");
		return;
	}

	char aUsername[32];
	str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
	pSelf->ExecuteSQLvf(pResult->m_ClientID, "UPDATE Accounts SET IsLoggedIn = 0 WHERE Username='%s'", aUsername);
}

void CGameContext::ConSQLLogoutAll(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;


	if (pPlayer->m_Authed == CServer::AUTHED_ADMIN)
	{
		//admins are allowed
	}
	else
	{
		if (g_Config.m_SvSupAccReset == 2)
		{
			if (pPlayer->m_IsSupporter)
			{
				//supporters are allowed
			}
			else
			{
				pSelf->SendChatTarget(ClientID, "[SQL] Missing permission.");
				return;
			}
		}
		else
		{
			pSelf->SendChatTarget(ClientID, "[SQL] Missing permission.");
			return;
		}
	}

	//for (int i = 0; i < MAX_CLIENTS; i++)
	//{
	//	if (pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->IsLoggedIn())
	//	{
	//		pSelf->m_aPlayers[i]->Logout(0);
	//		pSelf->SendChatTarget(i,"[ACC] you were logged out by an administrator. (logout all)");
	//	}
	//}

	pSelf->SQLcleanZombieAccounts(ClientID);
}

void CGameContext::ConWanted(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	char aBuf[128];
	int gangster = 0;

	pSelf->SendChatTarget(pResult->m_ClientID, "=== Wanted Players ===");
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->m_EscapeTime)
		{
			gangster++;
			str_format(aBuf, sizeof(aBuf), "'%s' reason [%s] seconds [%d]", pSelf->Server()->ClientName(i), pSelf->m_apPlayers[i]->m_aEscapeReason, pSelf->m_apPlayers[i]->m_EscapeTime / pSelf->Server()->TickSpeed());
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
	}
	str_format(aBuf, sizeof(aBuf), "=== %d gangster wanted ===", gangster);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
}

void CGameContext::ConViewers(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	char aBuf[32];
	char aMsg[128];
	int viewers = 0;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->m_SpectatorID == pResult->m_ClientID)
		{
			viewers++;
			if (viewers == 1)
			{
				str_format(aMsg, sizeof(aMsg), "'%s'", pSelf->Server()->ClientName(i));
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), ", '%s'", pSelf->Server()->ClientName(i));
				strcat(aMsg, aBuf);
			}
		}
	}

	if (viewers)
	{
		str_format(aBuf, sizeof(aBuf), "You have [%d] fangrills:", viewers);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		pSelf->SendChatTarget(pResult->m_ClientID, aMsg);
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "nobody is watching u ._.");
	}
}

void CGameContext::ConIp(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	NETADDR Addr;
	pSelf->Server()->GetClientAddr(ClientID, &Addr);
	char aAddrStr[NETADDR_MAXSTRSIZE];
	net_addr_str(&Addr, aAddrStr, sizeof(aAddrStr), true);
	char aBuf[32];
	str_format(aBuf, sizeof(aBuf), "your ip: %s", aAddrStr);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
}

void CGameContext::ConLogin2(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (g_Config.m_SvAccountStuff != 2)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Filebased accounts are turned off.");
		return;
	}

	if (pResult->NumArguments() != 2)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Use '/login2 <name> <password>'.");
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Use '/accountinfo' for help.");
		return;
	}

	if (pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] You are already logged in.");
		return;
	}

	char aUsername[32];
	char aPassword[MAX_PW_LEN+1];
	str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
	str_copy(aPassword, pResult->GetString(1), sizeof(aPassword));

	if (str_length(aUsername) > MAX_PW_LEN || str_length(aUsername) < MIN_PW_LEN)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Username is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if (str_length(aPassword) > MAX_PW_LEN || str_length(aPassword) < MIN_PW_LEN)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Password is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	//===========
	// FILE BASED
	//===========

	std::string data;
	char aData[32];
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s/%s.acc", g_Config.m_SvFileAccPath, aUsername);
	std::fstream Acc2File(aBuf);

	if (!std::ifstream(aBuf))
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] login failed.");
		Acc2File.close();
		return;
	}
	

	getline(Acc2File, data);
	str_copy(aData, data.c_str(), sizeof(aData));


	if (str_comp(aData, aPassword))
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] wrong password.");
		Acc2File.close();
		return;
	}

	getline(Acc2File, data);
	str_copy(aData, data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded login state '%s'", aData);

	if (aData[0] == '1')
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] error. Account is already logged in.");
		Acc2File.close();
		return;
	}

	getline(Acc2File, data);
	str_copy(aData, data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded port '%s'", aData);

	getline(Acc2File, data);
	str_copy(aData, data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded frozen state '%s'", aData);

	if (aData[0] == '1')
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] error. Account is frozen.");
		Acc2File.close();
		return;
	}

	//==============================
	//ALL CHECKS DONE --> load stats
	//==============================

	str_copy(pPlayer->m_aAccountLoginName, aUsername, sizeof(pPlayer->m_aAccountLoginName)); 
	str_copy(pPlayer->m_aAccountPassword, aPassword, sizeof(pPlayer->m_aAccountPassword));
	pPlayer->SetAccID(-1);
	pPlayer->m_IsFileAcc = true;

	getline(Acc2File, data);
	str_copy(aData, data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded vip '%d'", atoi(aData));
	pPlayer->m_IsModerator = atoi(aData);

	getline(Acc2File, data);
	str_copy(aData, data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded vip++ '%d'", atoi(aData));
	pPlayer->m_IsSuperModerator = atoi(aData);

	getline(Acc2File, data);
	str_copy(aData, data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded supporter '%d'", atoi(aData));
	pPlayer->m_IsSupporter = atoi(aData);

	getline(Acc2File, data);
	str_copy(aData, data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded money '%d'", atoi(aData));
	pPlayer->SetMoney(atoi(aData));

	getline(Acc2File, data);
	str_copy(aData, data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded level '%d'", atoi(aData));
	pPlayer->SetLevel(atoi(aData));

	getline(Acc2File, data);
	str_copy(aData, data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded xp '%d'", atoi(aData));
	pPlayer->SetXP(atoi(aData));

	getline(Acc2File, data);
	str_copy(aData, data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded shit '%d'", atoi(aData));
	pPlayer->m_shit = atoi(aData);

	getline(Acc2File, data);
	str_copy(aData, data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded policerank '%d'", atoi(aData));
	pPlayer->m_PoliceRank = atoi(aData);

	getline(Acc2File, data);
	str_copy(aData, data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded taserlevel '%d'", atoi(aData));
	pPlayer->m_TaserLevel = atoi(aData);

	pSelf->SendChatTarget(ClientID, "[ACCOUNT] logged in.");


	//save the acc with the new data and set islogged in to true
	pPlayer->SaveFileBased(1);
}

void CGameContext::ConRegister2(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (g_Config.m_SvAccountStuff != 2)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Filebased accounts are turned off.");
		return;
	}

	if (pResult->NumArguments() != 3)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Please use '/register2 <name> <password> <password>'.");
		return;
	}

	if (pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] You are already logged in.");
		return;
	}

	char aBuf[512];
	char aUsername[32];
	char aPassword[MAX_PW_LEN+1];
	char aPassword2[MAX_PW_LEN+1];
	str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
	str_copy(aPassword, pResult->GetString(1), sizeof(aPassword));
	str_copy(aPassword2, pResult->GetString(2), sizeof(aPassword2));

	if (str_length(aUsername) > MAX_PW_LEN || str_length(aUsername) < MIN_PW_LEN)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Username is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if ((str_length(aPassword) > MAX_PW_LEN || str_length(aPassword) < MIN_PW_LEN) || (str_length(aPassword2) > MAX_PW_LEN || str_length(aPassword2) < MIN_PW_LEN))
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Password is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if (str_comp_nocase(aPassword, aPassword2) != 0)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] Passwords need to be identical.");
		return;
	}

	//                                                                                                  \\ Escaping the escape seceqnze //unallow escape char because u can add newline i guess
	char aAllowedCharSet[128] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789&!?*.:+@/-_";
	bool EvilChar = false;

	for (int i = 0; i < str_length(aUsername); i++)
	{
		bool IsOk = false;

		for (int j = 0; j < str_length(aAllowedCharSet); j++)
		{
			if (aUsername[i] == aAllowedCharSet[j])
			{
				//dbg_msg("account","found valid char '%c' - '%c'", aUsername[i], aAllowedCharSet[j]);
				IsOk = true;
				break;
			}
		}

		if (!IsOk)
		{
			//dbg_msg("account", "found evil char '%c'", aUsername[i]);
			EvilChar = true;
			break;
		}
	}

	if (EvilChar)
	{
		str_format(aBuf, sizeof(aBuf), "[ACCOUNT] please use only the following characters in your username '%s'", aAllowedCharSet);
		pSelf->SendChatTarget(ClientID, aBuf);
		return;
	}


	//===========
	// FILE BASED
	//===========

	str_format(aBuf, sizeof(aBuf), "%s/%s.acc", g_Config.m_SvFileAccPath, aUsername);

	if (std::ifstream(aBuf))
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] username already exsists.");
		return;
	}
	std::ofstream Account2File(aBuf);
	if (!Account2File)
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] an error occured. pls report to an admin.");
		dbg_msg("acc2", "error1 writing file 'file_accounts/%s.acc'", aUsername);
		Account2File.close();
		return;
	}

	if (Account2File.is_open())
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] sucessfully registered an account.");
		Account2File
			<< aPassword << "\n"		/* 0 password */
			<< "0" << "\n"				/* 1 login state */
			<< "0" << "\n"				/* 2 last port */
			<< "0" << "\n"				/* 3 IsFrozen */
			<< "0" << "\n"				/* 4 IsModerator */
			<< "0" << "\n"				/* 5 IsSuperModerator */
			<< "0" << "\n"				/* 6 IsSupporter */
			<< "0" << "\n"				/* 7 money */
			<< "0" << "\n"				/* 8 level */
			<< "0" << "\n"				/* 9 xp */
			<< "0" << "\n"				/* 10 shit */
			<< "0" << "\n"				/* 11 policerank */
			<< "0" << "\n";				/* 12 taserlevel */
	}
	else
	{
		pSelf->SendChatTarget(ClientID, "[ACCOUNT] an error occured. pls report to an admin.");
		dbg_msg("acc2","error2 writing file 'file_accounts/%s.acc'", aUsername);
	}

	Account2File.close();
}

void CGameContext::ConACC2(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (g_Config.m_SvAccountStuff == 0)
	{
		pSelf->SendChatTarget(ClientID, "Account stuff is turned off.");
		return;
	}

	if (pPlayer->m_Authed != CServer::AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
		return;
	}

	char aBuf[128];
	char aCommand[32];
	char aName[32];
	str_copy(aCommand, pResult->GetString(0), sizeof(aCommand));
	str_copy(aName, pResult->GetString(1), sizeof(aName));

	if (pResult->NumArguments() == 0 || !str_comp_nocase(aCommand, "help"))
	{
		if (g_Config.m_SvAccountStuff != 2)
		{
			pSelf->SendChatTarget(ClientID, "---- COMMANDS [WARNING FILEBASED SYS IS DEACTIVATED]-----");
		}
		else
		{
			pSelf->SendChatTarget(ClientID, "---- COMMANDS -----");
		}
		pSelf->SendChatTarget(ClientID, "'/acc2 super_mod <name> <val>'");
		pSelf->SendChatTarget(ClientID, "'/acc2 mod <name> <val>'");
		pSelf->SendChatTarget(ClientID, "'/acc2 supporter <name> <val>'");
		pSelf->SendChatTarget(ClientID, "'/acc2 freeze_acc <name> <val>'");
		pSelf->SendChatTarget(ClientID, "----------------------");
		pSelf->SendChatTarget(ClientID, "'/acc_info <clientID>' additional info");
		pSelf->SendChatTarget(ClientID, "'/sql' similar cmd using sql acc sys");
		return;
	}


	if (!str_comp_nocase(aCommand, "supporter"))
	{
		if (pResult->NumArguments() < 3)
		{
			pSelf->SendChatTarget(ClientID, "Error: acc2 <command> <name> <value>");
			return;
		}
		int value;
		char str_value[16];
		value = pResult->GetInteger(2);
		str_format(str_value, sizeof(str_value), "%d", value);

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (pSelf->m_apPlayers[i])
			{
				if (pSelf->m_apPlayers[i]->IsLoggedIn() && !str_comp(pSelf->m_apPlayers[i]->m_aAccountLoginName, aName))
				{
					pSelf->m_apPlayers[i]->m_IsSupporter = value;
					if (value == 1)
					{
						pSelf->SendChatTarget(i, "[ACCOUNT] You are now Supporter.");
					}
					else
					{
						pSelf->SendChatTarget(i, "[ACCOUNT] You are no longer Supporter.");
					}
					str_format(aBuf, sizeof(aBuf), "UPDATED IsSupporter = %d (account is logged in)", value);
					pSelf->SendChatTarget(ClientID, aBuf);
					return;
				}
			}
		}

		//ONLY WRITE TO FILE IF ACCOUNT NOT LOGGED IN ON SERVER

		if (!pSelf->ChillUpdateFileAcc(aName, 6, str_value, pResult->m_ClientID))
		{
			str_format(aBuf, sizeof(aBuf), "[ACC2] UPDATED IsSupporter = %d (account is not logged in)", value);
			pSelf->SendChatTarget(ClientID, aBuf);
		}
		else
		{
			pSelf->SendChatTarget(ClientID, "[ACC2] command failed.");
		}
	}
	else if (!str_comp_nocase(aCommand, "super_mod"))
	{
		if (pResult->NumArguments() < 3)
		{
			pSelf->SendChatTarget(ClientID, "Error: acc2 <command> <name> <value>");
			return;
		}
		int value;
		char str_value[16];
		value = pResult->GetInteger(2);
		str_format(str_value, sizeof(str_value), "%d", value);

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (pSelf->m_apPlayers[i])
			{
				if (pSelf->m_apPlayers[i]->IsLoggedIn() && !str_comp(pSelf->m_apPlayers[i]->m_aAccountLoginName, aName))
				{
					pSelf->m_apPlayers[i]->m_IsSuperModerator = value;
					if (value == 1)
					{
						pSelf->SendChatTarget(i, "[ACCOUNT] You are now VIP+");
					}
					else
					{
						pSelf->SendChatTarget(i, "[ACCOUNT] You are no longer VIP+");
					}
					str_format(aBuf, sizeof(aBuf), "UPDATED IsSuperModerator = %d (account is logged in)", value);
					pSelf->SendChatTarget(ClientID, aBuf);
					return;
				}
			}
		}

		//ONLY WRITE TO FILE IF ACCOUNT NOT LOGGED IN ON SERVER

		if (!pSelf->ChillUpdateFileAcc(aName, 5, str_value, pResult->m_ClientID))
		{
			str_format(aBuf, sizeof(aBuf), "[ACC2] UPDATED IsSuperModerator = %d (account is not logged in)", value);
			pSelf->SendChatTarget(ClientID, aBuf);
		}
		else
		{
			pSelf->SendChatTarget(ClientID, "[ACC2] command failed.");
		}
	}
	else
	{
		pSelf->SendChatTarget(ClientID, "Unknown ACC2 command. Try '/acc2 help' for more help.");
	}
}

void CGameContext::ConAdmin(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (!(pPlayer->m_Authed == CServer::AUTHED_ADMIN || (pSelf->Server()->IsAuthed(ClientID) && pPlayer->m_IsSupporter)))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[ADMIN] Missing permission.");
		return;
	}

	char aCommand[32];
	char aName[32];
	str_copy(aCommand, pResult->GetString(0), sizeof(aCommand));
	str_copy(aName, pResult->GetString(1), sizeof(aName));

	if (pResult->NumArguments() == 0 || !str_comp_nocase(aCommand, "help"))
	{
		pSelf->SendChatTarget(ClientID, "---- COMMANDS -----");
		pSelf->SendChatTarget(ClientID, "'/admin vote_delay' reset vote delay to allow votes agian");
		pSelf->SendChatTarget(ClientID, "'/admin test' test DDNet++");
		pSelf->SendChatTarget(ClientID, "'/admin acc' test for illegal chars in account names");
		pSelf->SendChatTarget(ClientID, "'/admin cfg_tiles' info on configurable tiles");
		pSelf->SendChatTarget(ClientID, "--- SIMILAR COMMANDS ---");
		pSelf->SendChatTarget(ClientID, "'/flood' for flood protection commands");
		pSelf->SendChatTarget(ClientID, "----------------------");
		return;
	}

	if (!str_comp_nocase(aCommand, "vote_delay"))
	{
		pSelf->m_LastVoteCallAll = -9999999;
		pSelf->SendChatTarget(ClientID, "[ADMIN] votes can be used agian.");
	}
	else if (!str_comp_nocase(aCommand, "test"))
	{
		vec2 SurvivalGameSpawnTile = pSelf->Collision()->GetSurvivalSpawn(g_Config.m_SvMaxClients);
		vec2 SurvivalGameSpawnTile2 = pSelf->Collision()->GetSurvivalSpawn(MAX_CLIENTS);

		if (SurvivalGameSpawnTile == vec2(-1, -1))
		{
			pSelf->SendChatTarget(ClientID, "[ADMIN:Test] ERROR: not enough survival spawns (less survival spawns than slots)");
		}
		else if (SurvivalGameSpawnTile2 == vec2(-1, -1))
		{
			pSelf->SendChatTarget(ClientID, "[ADMIN:Test] WARNING: less survival spawns on map than slots possible in ddnet++ (no problem as long as slots stay how they are)");
		}
		else
		{
			pSelf->SendChatTarget(ClientID, "[ADMIN:Test] Test Finished. Everything looks good c:");
		}
	}
	else if (!str_comp_nocase(aCommand, "acc"))
	{
		if (pSelf->PrintSpecialCharUsers(ClientID) == 0)
		{
			pSelf->SendChatTarget(ClientID, "All users have allowed char set usernames.");
		}
	}
	else if (!str_comp_nocase(aCommand, "cfg_tiles") || !str_comp_nocase(aCommand, "cfg_tile") || !str_comp_nocase(aCommand, "config_tiles"))
	{
		pSelf->SendChatTarget(ClientID, "=== config tiles ===");
		pSelf->SendChatTarget(ClientID, "config tile 1 index=182");
		pSelf->SendChatTarget(ClientID, "config tile 2 index=183");
		pSelf->SendChatTarget(ClientID, "");
		pSelf->SendChatTarget(ClientID, "sv_cfg_tile_1, sv_cfg_tile_2:");
		pSelf->SendChatTarget(ClientID, "0=off");
		pSelf->SendChatTarget(ClientID, "1=freeze,2=unfreeze,3=deep,4=undeep");
		pSelf->SendChatTarget(ClientID, "5=hook,6=unhook,7=kill");
		pSelf->SendChatTarget(ClientID, "8=bloody,9=rainbow,10=spreadgun");
	}
	else
	{
		pSelf->SendChatTarget(ClientID, "[ADMIN] unknown parameter");
	}
}

void CGameContext::ConFNN(IConsole::IResult * pResult, void * pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (pPlayer->m_Authed != CServer::AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "[FNN] Missing permission.");
		return;
	}

	char aBuf[128];
	char aCommand[32];
	char aName[32];
	str_copy(aCommand, pResult->GetString(0), sizeof(aCommand));
	str_copy(aName, pResult->GetString(1), sizeof(aName));

	if (pResult->NumArguments() == 0 || !str_comp_nocase(aCommand, "help"))
	{
		pSelf->SendChatTarget(ClientID, "---- COMMANDS -----");
		pSelf->SendChatTarget(ClientID, "'/fnn init' loads the finish tile pos");
		pSelf->SendChatTarget(ClientID, "'/fnn train' does random moves and writes highscores to file");
		pSelf->SendChatTarget(ClientID, "'/fnn play_distance' replays the best distance");
		pSelf->SendChatTarget(ClientID, "'/fnn play_fitness' replays best distance in best time");
		pSelf->SendChatTarget(ClientID, "'/fnn play_distance_finish' replays best distance to finish");
		pSelf->SendChatTarget(ClientID, "'/fnn stats' prints current highscores");
		pSelf->SendChatTarget(ClientID, "----------------------");
		return;
	}

	if (!str_comp_nocase(aCommand, "init"))
	{
		pSelf->m_FinishTilePos = pSelf->GetFinishTile();
		str_format(aBuf, sizeof(aBuf), "[FNN] found finish tile at (%.2f/%.2f)", pSelf->m_FinishTilePos.x, pSelf->m_FinishTilePos.y);
		pSelf->SendChatTarget(ClientID, aBuf);
		return;
	}
	else if (pSelf->m_FinishTilePos.x == 0.000000f && pSelf->m_FinishTilePos.y == 0.000000f)
	{
		pSelf->SendChatTarget(ClientID, "[FNN] ERROR no finish tile loaded try '/fnn init'");
		return;
	}

	if (!str_comp_nocase(aCommand, "train"))
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->m_IsDummy && pSelf->m_apPlayers[i]->m_DummyMode == 25)
			{
				pSelf->m_apPlayers[i]->m_dmm25 = 0;
				str_format(aBuf, sizeof(aBuf), "[FNN] set submode to training for '%s'", pSelf->Server()->ClientName(i));
				pSelf->SendChatTarget(ClientID, aBuf);
			}
		}
	}
	else if (!str_comp_nocase(aCommand, "play_distance"))
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->m_IsDummy && pSelf->m_apPlayers[i]->m_DummyMode == 25)
			{
				pSelf->m_apPlayers[i]->m_dmm25 = 1; //load distance
				str_format(aBuf, sizeof(aBuf), "[FNN] set submode to play best distance for '%s'", pSelf->Server()->ClientName(i));
				pSelf->SendChatTarget(ClientID, aBuf);
				if (pSelf->m_apPlayers[i]->GetCharacter())
				{
					pSelf->m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_SELF);
				}
			}
		}
	}
	else if (!str_comp_nocase(aCommand, "play_fitness"))
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->m_IsDummy && pSelf->m_apPlayers[i]->m_DummyMode == 25)
			{
				pSelf->m_apPlayers[i]->m_dmm25 = 2; //load fitness
				str_format(aBuf, sizeof(aBuf), "[FNN] set submode to play best fitness for '%s'", pSelf->Server()->ClientName(i));
				pSelf->SendChatTarget(ClientID, aBuf);
				if (pSelf->m_apPlayers[i]->GetCharacter())
				{
					pSelf->m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_SELF);
				}
			}
		}
	}
	else if (!str_comp_nocase(aCommand, "play_distance_finish"))
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->m_IsDummy && pSelf->m_apPlayers[i]->m_DummyMode == 25)
			{
				pSelf->m_apPlayers[i]->m_dmm25 = 3; //load distance_finish
				str_format(aBuf, sizeof(aBuf), "[FNN] set submode to play best distance_finish for '%s'", pSelf->Server()->ClientName(i));
				pSelf->SendChatTarget(ClientID, aBuf);
				if (pSelf->m_apPlayers[i]->GetCharacter())
				{
					pSelf->m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_SELF);
				}
			}
		}
	}
	else if (!str_comp_nocase(aCommand, "stop"))
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->m_IsDummy && pSelf->m_apPlayers[i]->m_DummyMode == 25)
			{
				pSelf->m_apPlayers[i]->m_dmm25 = -2; //set to stop all
				str_format(aBuf, sizeof(aBuf), "[FNN] stopped '%s'", pSelf->Server()->ClientName(i));
				pSelf->SendChatTarget(ClientID, aBuf);
				if (pSelf->m_apPlayers[i]->GetCharacter())
				{
					pSelf->m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_SELF);
				}
			}
		}
	}
	else if (!str_comp_nocase(aCommand, "stats"))
	{
		pSelf->SendChatTarget(ClientID, "========== FNN Stats ==========");
		str_format(aBuf, sizeof(aBuf), "distance=%.2f", pSelf->m_FNN_best_distance);
		pSelf->SendChatTarget(ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "fitness=%.2f", pSelf->m_FNN_best_fitness);
		pSelf->SendChatTarget(ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "distance_finish=%.2f", pSelf->m_FNN_best_distance_finish);
		pSelf->SendChatTarget(ClientID, aBuf);
	}
	else
	{
		pSelf->SendChatTarget(ClientID, "[FNN] unknown parameter");
	}
}
