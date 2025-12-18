/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#include "gamecontext.h"

#include <base/log.h>
#include <base/system.h>
#include <base/system_ddpp.h>
#include <base/types.h>

#include <engine/external/sqlite3/sqlite3.h>
#include <engine/server/server.h>
#include <engine/shared/config.h>
#include <engine/shared/protocol.h>

#include <game/mapitems.h>
#include <game/mapitems_ddpp.h>
#include <game/server/captcha.h>
#include <game/server/ddnetpp/db/accounts.h>
#include <game/server/ddpp/enums.h>
#include <game/server/ddpp/shop.h>
#include <game/server/entities/laser_text.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/minigames/minigame_base.h>
#include <game/server/player.h>
#include <game/server/teams.h>
#include <game/version.h>

#include <cinttypes>
#include <cstdio> //acc2 to_str()
#include <cstdlib> //acc2 to_str()
#include <ctime> //ChillerDragon
#include <fstream> //ChillerDragon acc sys2

bool CheckClientId(int ClientId); //TODO: whats this ? xd

bool CGameContext::DDPPCredits()
{
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
		"ChillerDragon's block mod DDNet++ " DDNETPP_VERSIONSTR);
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
		"Created by ChillerDragon, timakro, FruchtiHD, fokkonaut, ReiTW, Henritees");
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
		"heinrich5991, QshaR, Teecloud, noby, SarKro, Pikotee, toast");
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
		"Blue, Zwelf");
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
		"Based on DDNet.");
	return true;
}

bool CGameContext::DDPPInfo()
{
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
		"ChillerDragon's DDNet++ " DDNETPP_VERSIONSTR " (more info '/changelog')");
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
		"Based on DDNet Version: " GAME_VERSION);
	if(GIT_SHORTREV_HASH)
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "Git revision hash: %s", GIT_SHORTREV_HASH);
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp", aBuf);
	}
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
		"Official site: DDNet.tw");
	Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
		"For more info: /cmdlist");
	return true;
}

bool CGameContext::DDPPPoints(IConsole::IResult *pResult, void *pUserData)
{
	if(g_Config.m_SvPointsMode == 1) //ddnet
		return false;

	if(g_Config.m_SvPointsMode == 2) //ddpp (blockpoints)
	{
		CGameContext *pSelf = (CGameContext *)pUserData;
		if(!CheckClientId(pResult->m_ClientId))
			return true;
		CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
		if(!pPlayer)
			return true;

		char aBuf[256];

		if(pResult->NumArguments() > 0) //show others
		{
			int pointsId = pSelf->GetCidByName(pResult->GetString(0));
			if(pointsId == -1)
			{
				str_format(aBuf, sizeof(aBuf), "'%s' is not online", pResult->GetString(0));
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return true;
			}
			str_format(aBuf, sizeof(aBuf), "'%s' points[%d] kills[%d] deaths[%d]", pResult->GetString(0), pSelf->m_apPlayers[pointsId]->m_Account.m_BlockPoints, pSelf->m_apPlayers[pointsId]->m_Account.m_BlockPoints_Kills, pSelf->m_apPlayers[pointsId]->m_Account.m_BlockPoints_Deaths);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
		else //show own
		{
			str_format(aBuf, sizeof(aBuf), "'%s' points[%d] kills[%d] deaths[%d]", pSelf->Server()->ClientName(pResult->m_ClientId), pPlayer->m_Account.m_BlockPoints, pPlayer->m_Account.m_BlockPoints_Kills, pPlayer->m_Account.m_BlockPoints_Deaths);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
	}
	else //points deactivated
	{
		CGameContext *pSelf = (CGameContext *)pUserData;
		if(!CheckClientId(pResult->m_ClientId))
			return true;
		CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
		if(!pPlayer)
			return true;

		pSelf->Console()->Print(
			IConsole::OUTPUT_LEVEL_STANDARD,
			"points",
			"Showing points is deactivated on this DDNet++ server.");
	}
	return true;
}

void CGameContext::ConToggleSpawn(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	if(!pPlayer->m_Account.m_IsSuperModerator)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission. You are not a VIP+.");
		return;
	}

	pPlayer->m_IsSuperModSpawn ^= true;

	if(pPlayer->m_IsSuperModSpawn)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "VIP+ Spawn activated");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "VIP+ Spawn deactivated");
	}
}

void CGameContext::ConSpawnWeapons(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	pSelf->SetSpawnweapons(!pPlayer->m_Account.m_UseSpawnWeapons, pResult->m_ClientId);
}

void CGameContext::ConSayServer(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;
	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	if(!pPlayer->m_Account.m_IsSuperModerator && !pPlayer->m_Account.m_IsModerator)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[SAY] Missing permission.");
		return;
	}

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "[SERVER] %s", pResult->GetString(0));
	pSelf->SendChat(-1, TEAM_ALL, aBuf);
}

void CGameContext::ConPolicehelper(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	char aBuf[128];

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "**** POLICEHELPER ****");
		pSelf->SendChatTarget(pResult->m_ClientId, "Police[2] can add/remove policehelpers with:");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/policehelper add/remove <playername>'.");
		pSelf->SendChatTarget(pResult->m_ClientId, "Police-Bots help policehelpers.");
		pSelf->SendChatTarget(pResult->m_ClientId, "*** Personal Stats ***");
		if(pPlayer->m_Account.m_PoliceRank > 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Police[2]: true");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Police[2]: false");
		}
		if(pPlayer->m_PoliceHelper)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Policehelper: true");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Policehelper: false");
		}
		return;
	}

	if(pPlayer->m_Account.m_PoliceRank < 2)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[POLICE] You have to be atleast Police[2] to use this command. Check '/policehelper' for more info.");
		return;
	}
	if(pResult->NumArguments() == 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[POLICE] Missing parameter: <player>");
		return;
	}

	int HelperId = pSelf->GetCidByName(pResult->GetString(1));
	if(HelperId == -1)
	{
		str_format(aBuf, sizeof(aBuf), "[POLICE] Player '%s' is not online.", pResult->GetString(1));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		return;
	}

	char aPara[32];
	str_format(aPara, sizeof(aPara), "%s", pResult->GetString(0));
	if(!str_comp_nocase(aPara, "add"))
	{
		if(pSelf->m_apPlayers[HelperId])
		{
			if(pSelf->m_apPlayers[HelperId]->m_PoliceHelper)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[POLICE] This player is already a policehelper.");
				return;
			}

			pSelf->m_apPlayers[HelperId]->m_PoliceHelper = true;
			str_format(aBuf, sizeof(aBuf), "[POLICE] '%s' promoted you to policehelper.", pSelf->Server()->ClientName(pResult->m_ClientId));
			pSelf->SendChatTarget(HelperId, aBuf);

			str_format(aBuf, sizeof(aBuf), "[POLICE] '%s' is now a policehelper.", pSelf->Server()->ClientName(HelperId));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
	}
	else if(!str_comp_nocase(aPara, "remove"))
	{
		if(pSelf->m_apPlayers[HelperId])
		{
			if(!pSelf->m_apPlayers[HelperId]->m_PoliceHelper)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[POLICE] This player is not a policehelper.");
				return;
			}

			pSelf->m_apPlayers[HelperId]->m_PoliceHelper = false;
			str_format(aBuf, sizeof(aBuf), "[POLICE] '%s' removed your policehelper rank.", pSelf->Server()->ClientName(pResult->m_ClientId));
			pSelf->SendChatTarget(HelperId, aBuf);

			str_format(aBuf, sizeof(aBuf), "[POLICE] '%s' is no longer a policehelper.", pSelf->Server()->ClientName(HelperId));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[POLICE] Unknown parameter. Check '/policehelper' for help.");
	}
}

//void CGameContext::ConTaserinfo(IConsole::IResult *pResult, void *pUserData)
//{
//#if defined(CONF_DEBUG)
//#endif
//	CGameContext *pSelf = (CGameContext *)pUserData;
//	if (!CheckClientId(pResult->m_ClientId))
//		return;
//
//	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
//	if (!pPlayer)
//		return;
//
//	CCharacter* pChr = pPlayer->GetCharacter();
//	if (!pChr)
//		return;
//
//
//	if (pPlayer->m_Account.m_TaserLevel == 0)
//	{
//		pPlayer->m_TaserPrice = 50000;
//	}
//	else if (pPlayer->m_Account.m_TaserLevel == 1)
//	{
//		pPlayer->m_TaserPrice = 75000;
//	}
//	else if (pPlayer->m_Account.m_TaserLevel == 2)
//	{
//		pPlayer->m_TaserPrice = 100000;
//	}
//	else if (pPlayer->m_Account.m_TaserLevel == 3)
//	{
//		pPlayer->m_TaserPrice = 150000;
//	}
//	else if (pPlayer->m_Account.m_TaserLevel == 4)
//	{
//		pPlayer->m_TaserPrice = 200000;
//	}
//	else if (pPlayer->m_Account.m_TaserLevel == 5)
//	{
//		pPlayer->m_TaserPrice = 200000;
//	}
//	else if (pPlayer->m_Account.m_TaserLevel == 6)
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
//	pSelf->SendChatTarget(pResult->m_ClientId, "~~~ TASER INFO ~~~");
//	pSelf->SendChatTarget(pResult->m_ClientId, "Police Ranks 3 or higher are allowed to carry a taser.");
//	pSelf->SendChatTarget(pResult->m_ClientId, "Use the taser to fight gangsters.");
//	pSelf->SendChatTarget(pResult->m_ClientId, "~~~ YOUR TASER STATS ~~~");
//	str_format(aBuf, sizeof(aBuf), "TaserLevel: %d/7", pPlayer->m_Account.m_TaserLevel);
//	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
//	str_format(aBuf, sizeof(aBuf), "Price for the next level: %d", pPlayer->m_TaserPrice);
//	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
//	str_format(aBuf, sizeof(aBuf), "FreezeTime: 0.%d seconds", pPlayer->m_Account.m_TaserLevel * 5);
//	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
//	str_format(aBuf, sizeof(aBuf), "FailRate: %d%", 0);
//	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
//}

void CGameContext::ConOfferInfo(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	char aBuf[256];

	pSelf->SendChatTarget(pResult->m_ClientId, "~~~ OFFER INFO ~~~");
	pSelf->SendChatTarget(pResult->m_ClientId, "Users can accept offers with '/<extra> <accept>'");
	pSelf->SendChatTarget(pResult->m_ClientId, "VIPs can give all players one rainbow offer.");
	pSelf->SendChatTarget(pResult->m_ClientId, "VIP+ can give all players more rainbow offers and one bloody.");
	//pSelf->SendChatTarget(pResult->m_ClientId, "Admins can give all players much more of everything."); //admins can't do shit lul
	pSelf->SendChatTarget(pResult->m_ClientId, "~~~ YOUR OFFER STATS (Extras) ~~~");
	str_format(aBuf, sizeof(aBuf), "Rainbow: %d", pPlayer->m_rainbow_offer);
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Bloody: %d", pPlayer->m_bloody_offer);
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Trail: %d", pPlayer->m_trail_offer);
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Atom: %d", pPlayer->m_atom_offer);
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Spread gun: %d", pPlayer->m_autospreadgun_offer);
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
}

void CGameContext::ConChangelog(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	// RELEASE NOTES:
	// 25.9.2024  RELEASED v.0.1.1
	// 31.10.2023 RELEASED v.0.1.0
	// 20.9.2019  RELEASED v.0.0.7
	// 7.9.2018   RELEASED v.0.0.6
	// 7.10.2017  RELEASED v.0.0.3
	// 25.5.2017  RELEASED v.0.0.2
	// 9.4.2017   RELEASED v.0.0.1

	int PageCounter = 1;
	int Page = pResult->GetInteger(0); //no parameter -> 0 -> page 1
	int Pages = 10;
	Page = std::clamp(Page, 1, Pages);
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "page %d/%d '/changelog <page>'", Page, Pages);

	if(Page == PageCounter++)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"=== Changelog (DDNet++ dev) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"* rename chat command '/acc_logout' to '/logout'");
	}
	else if(Page == PageCounter++)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"=== Changelog (DDNet++ v.0.1.1) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"* update to ddnet 18.5.1");
	}
	else if(Page == PageCounter++)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"=== Changelog (DDNet++ v.0.1.0) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"* update to ddnet 17.2.1");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ add '/lasertext' command for VIP+");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ add '/lang' command for translations");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ add xp rewards for kills/blocks");
	}
	else if(Page == PageCounter++)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"=== Changelog (DDNet++ v.0.0.7) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"* fix flag crashbug");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"* fix account system (database) on windows");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"* finally own gametype name");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ add new shop (map/motd/dummy)");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ add '/score' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ add '/drop_armor' and '/drop_health' commands");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ add '/spawn' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ add '/survival' minigame");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ add '/regex' staff command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ add '/mapsave' staff command");
	}
	else if(Page == PageCounter++)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"=== Changelog (DDNet++ v.0.0.6) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"* fix tons of bugs");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"* improved trade command and added public trades");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"* new shop");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"* upgraded fng (multis/onfire-mode/fng only mode)");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added spawnweapons to shop");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added drop weapons (on vote no key)");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added vanilla/ddrace mode tiles");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added spooky_ghost to shop");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added nobo spawn");
	}
	else if(Page == PageCounter++)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"=== Changelog (DDNet++ v.0.0.5) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"* drop flags in aim direction");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added cosmetic tiles");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"* fixed the chidraqul minigame");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added dummymodes for BlmapChill and blmapV5");
	}
	else if(Page == PageCounter++)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"=== Changelog (DDNet++ v.0.0.4) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added spreading guns for VIP+");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added global chat (@all)");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added block tournaments");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added fng settings (check '/fng')");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added '/wanted' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added '/viewers' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added '/blockwave' minigame");
	}
	else if(Page == PageCounter++)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"=== Changelog (DDNet++ v.0.0.3) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added minigames overview (check '/minigames')");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added balance battles (check '/balance')");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added new '/insta' commands and gametypes");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added new '/bounty' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added new '/trade' command");
	}
	else if(Page == PageCounter++)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"=== Changelog (DDNet++ v.0.0.2) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added '/ascii' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added block points check '/points'");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added '/hook <power>' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added '/hide' and '/show' commands");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added quests ('/quest' for more info)");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added '/insta gdm' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"* improved the racer bot");
	}
	else if(Page == PageCounter++)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"=== Changelog (DDNet++ v.0.0.1) ===");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added VIP+");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added VIP");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added VIP+ Spawn");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added '/acc_logout' command (now just '/logout')");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added '/changepassword <old> <new> <new>' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added '/poop <amount> <player>' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added '/pay <amount> <player>' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added '/policeinfo' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added '/bomb <command>' command more info '/bomb help'");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"+ added instagib modes (gdm, idm, gSurvival and iSurvival)");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"* dummys now join automatically on server start");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"* improved the blocker bot");
	}
	else
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"unknown page.");
		return;
	}
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
		"------------------------");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
		aBuf);
}

void CGameContext::ConScore(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	if(pResult->NumArguments() == 0)
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "Your current display score type is %s.", display_score_to_str(pPlayer->m_DisplayScore));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		pSelf->SendChatTarget(pResult->m_ClientId, "You can change it to any of these: " DISPLAY_SCORE_VALUES);
		return;
	}

	if(str_to_display_score(pResult->GetString(0), &pPlayer->m_DisplayScore))
		pSelf->SendChatTarget(pResult->m_ClientId, "Updated display score.");
	else
		pSelf->SendChatTarget(pResult->m_ClientId, "Invalid score name pick one of those: " DISPLAY_SCORE_VALUES);
}

void CGameContext::ConShop(IConsole::IResult *pResult, void *pUserData)
{
	// if you add something to the shop make sure to also add extend the list here and add a page to ShopWindow() and BuyItem() in character.cpp

	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(!str_comp_nocase(pResult->GetString(0), "help"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "~~~ SHOP HELP ~~~");
		pSelf->SendChatTarget(pResult->m_ClientId, "If you're in the shop you can open the shop menu using f4.");
		pSelf->SendChatTarget(pResult->m_ClientId, "By shooting to the right you go one site forward,");
		pSelf->SendChatTarget(pResult->m_ClientId, "and by shooting left you go one site backwards.");
		pSelf->SendChatTarget(pResult->m_ClientId, "If you want to buy an item you have to press f3.");
		pSelf->SendChatTarget(pResult->m_ClientId, "Then a confirmation will pop up and you have to press f3 again to confirm.");
		pSelf->SendChatTarget(pResult->m_ClientId, "NOTICE: f3 and f4 may not work for you, you have to press VOTE YES for f3 and VOTE NO for f4.");

		if(pSelf->GetShopBot() != -1)
		{
			char aShopBot[128];
			str_format(aShopBot, sizeof(aShopBot), "If you want to see the shop, watch '%s' in '/pause'.", pSelf->Server()->ClientName(pSelf->GetShopBot()));
			pSelf->SendChatTarget(pResult->m_ClientId, "-----------------------------");
			pSelf->SendChatTarget(pResult->m_ClientId, aShopBot);
		}
	}
	else
	{
		pSelf->Shop()->ShowShopMotdCompressed(pResult->m_ClientId);
	}
}

void CGameContext::ConPoliceChat(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];

	char aBuf[256 + 24];

	str_format(aBuf, 256 + 24, "[%x][Police]%s: %s",
		pPlayer->m_Account.m_PoliceRank,
		pSelf->Server()->ClientName(pResult->m_ClientId),
		pResult->GetString(0));
	if(pPlayer->m_Account.m_PoliceRank > 0)
		pSelf->SendChat(-2, TEAM_ALL, aBuf, pResult->m_ClientId);
	else
		pSelf->Console()->Print(
			IConsole::OUTPUT_LEVEL_STANDARD,
			"Police",
			"You are not police.");
}

void CGameContext::ConBuy(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;
	if(pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Unknown item. Type '/buy <itemname>' use '/shop' to see the full itemlist.");
		return;
	}
	pSelf->Shop()->BuyItem(pResult->m_ClientId, pResult->GetString(0));
}

void CGameContext::ConRegister(IConsole::IResult *pResult, void *pUserData)
{
	char aBuf[512];
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	if(g_Config.m_SvAccounts == 0)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Accounts are turned off.");
		return;
	}
	if(g_Config.m_SvAccounts == 2) //filebased
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] SQLite accounts are turned off. (try '/register2')");
		return;
	}

	if(pResult->NumArguments() != 3)
	{
		pSelf->SendChatTarget(ClientId, pSelf->Loc("[ACCOUNT] Please use '/register <name> <password> <password>'.", ClientId));
		return;
	}

	if(pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(ClientId, pSelf->Loc("[ACCOUNT] You are already logged in", ClientId));
		return;
	}

	// 20 ticks is quite fast
	// if it takes you less than 20 ticks to type register you are a crazy fast typer
	// it can happen
	// but is quite unrealistic because you also have to
	// choose a name and password and they have to match
	// and there is just a high chance that the final register command is not
	// the first message you sent
	//
	// but this will block legit headless clients such as term-ux
	if(pPlayer->m_InputTracker.TicksSpentChatting() < 20 && g_Config.m_SvRequireChatFlagToRegister)
	{
		// security through obscurity in open source lmao
		// do not give bots creating accounts a hint on how to bypass it xd
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] something went wrong please try again.");
		return;
	}

	if(pPlayer->m_PlayerHumanLevel < g_Config.m_SvRegisterHumanLevel)
	{
		str_format(aBuf, sizeof(aBuf), "[ACCOUNT] your '/human_level' is too low %d/%d to use this command.", pPlayer->m_PlayerHumanLevel, g_Config.m_SvRegisterHumanLevel);
		pSelf->SendChatTarget(ClientId, aBuf);
		return;
	}

	const NETADDR *pAddr = pSelf->Server()->ClientAddr(ClientId);
	int RegBanned = 0;

	for(int i = 0; i < pSelf->m_NumRegisterBans && !RegBanned; i++)
	{
		if(!net_addr_comp_noport(pAddr, &pSelf->m_aRegisterBans[i].m_Addr))
			RegBanned = (pSelf->m_aRegisterBans[i].m_Expire - pSelf->Server()->Tick()) / pSelf->Server()->TickSpeed();
	}

	if(RegBanned > 0)
	{
		str_format(aBuf, sizeof aBuf, "[ACCOUNT] you have to wait %d seconds before you can register again.", RegBanned);
		pSelf->SendChatTarget(ClientId, aBuf);
		return;
	}

	char aUsername[32];
	char aPassword[MAX_PW_LEN + 1];
	char aPassword2[MAX_PW_LEN + 1];
	str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
	str_copy(aPassword, pResult->GetString(1), sizeof(aPassword));
	str_copy(aPassword2, pResult->GetString(2), sizeof(aPassword2));

	if(str_length(aUsername) > 20 || str_length(aUsername) < 3)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Username is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if((str_length(aPassword) > MAX_PW_LEN || str_length(aPassword) < MIN_PW_LEN) || (str_length(aPassword2) > MAX_PW_LEN || str_length(aPassword2) < MIN_PW_LEN))
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Password is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if(str_comp_nocase(aPassword, aPassword2) != 0)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Passwords need to be identical.");
		return;
	}

	//if (EvilChar)
	if(!pSelf->IsAllowedCharSet(aUsername))
	{
		str_format(aBuf, sizeof(aBuf), "[ACCOUNT] please use only the following characters in your username '%s'", pSelf->m_aAllowedCharSet);
		pSelf->SendChatTarget(ClientId, aBuf);
		return;
	}

	pSelf->Accounts()->Register(ClientId, aUsername, aPassword);
}

void CGameContext::ConSqlName(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int ClientId = pResult->m_ClientId;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(g_Config.m_SvAccounts == 0)
	{
		pSelf->ChatrespLocSys(ClientId, "SQL", "The account system is turned off.");
		return;
	}

	if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) != AUTHED_ADMIN)
	{
		pSelf->ChatrespLocSys(ClientId, "SQL", "Missing permission.");
		return;
	}

	if(pResult->NumArguments() == 0)
	{
		log_info("chatresp", "---- COMMANDS -----");
		log_info("chatresp", "'/sql_name super_mod <acc_name> <on/off>'");
		// log_info("chatresp", "'/sql_name mod <acc_name> <true/false>'"); //coming soon...
		// log_info("chatresp", "'/sql_name freeze_acc <acc_name> <true/false>'"); //coming soon...
		log_info("chatresp", "'/sql_name set_passwd <acc_name> <passwd>' to reset password");
		log_info("chatresp", "----------------------");
		log_info("chatresp", "'/acc_info <name>' additional info");
		log_info("chatresp", "'/sql' similar command using sql ids");
		return;
	}

	const char *pUsername = pResult->GetString(1);

	if(!str_comp_nocase(pResult->GetString(0), "super_mod"))
	{
		if(pResult->NumArguments() != 3)
		{
			log_info("chatresp", "usage: /sql_name super_mod <acc_name> <on/off>");
			return;
		}
		bool Value = str_to_bool(pResult->GetString(2));
		const char *pQuery = "UPDATE Accounts SET IsSuperModerator = ? WHERE Username = ?;";
		pSelf->m_pAccounts->UpdateAccountStateByUsername(ClientId, pUsername, Value, CAccountRconCmdResult::SUPER_MODERATOR, pQuery);
	}
	else if(!str_comp_nocase(pResult->GetString(0), "set_passwd"))
	{
		if(pResult->NumArguments() != 3)
		{
			pSelf->SendChatTarget(ClientId, "usage: /sql_name set_passwd <acc_name> <passwd>");
			return;
		}

		if((str_length(pResult->GetString(2)) > MAX_PW_LEN || str_length(pResult->GetString(2)) < MIN_PW_LEN) || (str_length(pResult->GetString(2)) > MAX_PW_LEN || str_length(pResult->GetString(2)) < MIN_PW_LEN))
		{
			pSelf->SendChatTarget(ClientId, "[ACCOUNT] Password is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
			return;
		}
		pSelf->m_pAccounts->AdminSetPassword(ClientId, pUsername, pResult->GetString(2));
	}
	else
	{
		pSelf->SendChatTarget(ClientId, "Unknown command try '/sql_name' for full list.");
	}
}

static bool BlockAccountRconCmd(CGameContext *pSelf, int ClientId, const char *pOperation)
{
	if(!pSelf->m_pController)
	{
		log_error("cahtresp", "something went wrong with this rcon command");
		return true;
	}

	// allow admins to reset account passwords even if accounts are off
	// if(!g_Config.m_SvAccounts)
	// {
	// 	log_error("chatresp", "accounts are turned off");
	// 	return true;
	// }

	char aReason[512];
	if(pSelf->m_pController->IsAccountRconCmdRatelimited(ClientId, aReason, sizeof(aReason)))
	{
		log_error("chatresp", "%s failed because of: %s", pOperation, aReason);
		return true;
	}
	return false;
}

void CGameContext::ConSql(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int ClientId = pResult->m_ClientId;
	if(BlockAccountRconCmd(pSelf, pResult->m_ClientId, "sql"))
		return;

	if(g_Config.m_SvAccounts == 0)
	{
		pSelf->ChatrespLocSys(ClientId, "SQL", "The account system is turned off.");
		return;
	}

	if(pSelf->Server()->GetAuthedState(ClientId) != AUTHED_ADMIN)
	{
		pSelf->ChatrespLocSys(ClientId, "SQL", "Missing permission.");
		return;
	}

	if(pResult->NumArguments() == 0)
	{
		log_info("chatresp", "---- COMMANDS -----");
		log_info("chatresp", "'/sql getid <clientid>' to get sql id");
		log_info("chatresp", "'/sql super_mod <sqlid> <val>'");
		log_info("chatresp", "'/sql mod <sqlid> <val>'");
		log_info("chatresp", "'/sql supporter <sqlid> <val>'");
		log_info("chatresp", "'/sql freeze_acc <sqlid> <val>'");
		log_info("chatresp", "----------------------");
		log_info("chatresp", "'/acc_info <player name>' additional info");
		log_info("chatresp", "'/sql_name' similar command using account names");
		log_info("chatresp", "'/sql_logout <playername>' sets logout state (risky)");
		log_info("chatresp", "'/sql_logout_all' sets logout state only for current port (save)");
		return;
	}

	if(pResult->NumArguments() < 2)
	{
		log_info("chatresp", "[SQL] need at least two arguments, or pass none to show help");
		return;
	}

	const char *pCommand = pResult->GetString(0);
	int SqlId = pResult->GetInteger(1);

	if(!str_comp_nocase(pCommand, "getid"))
	{
		int VictimId = SqlId;
		if(!CheckClientId(VictimId))
		{
			log_error("chatresp", "[SQL] invalid client id %d", VictimId);
			return;
		}
		if(!pSelf->m_apPlayers[VictimId])
		{
			log_error("chatresp", "[SQL] Can't find player with Id: %d.", VictimId);
			return;
		}

		if(!pSelf->m_apPlayers[VictimId]->IsLoggedIn())
		{
			log_error("chatresp", "[SQL] Player '%s' is not logged in.", pSelf->Server()->ClientName(VictimId));
			return;
		}

		log_info("chatresp", "[SQL] '%s' SQL-Id: %d", pSelf->Server()->ClientName(VictimId), pSelf->m_apPlayers[VictimId]->GetAccId());
	}
	else if(!str_comp_nocase(pCommand, "supporter"))
	{
		if(pResult->NumArguments() < 3)
		{
			log_error("chatresp", "[SQL] Error: sql <command> <id> <value>");
			return;
		}

		int Value = pResult->GetInteger(2);
		pSelf->m_pAccounts->UpdateAccountState(ClientId, SqlId, Value, CAccountRconCmdResult::SUPPORTER, "UPDATE Accounts SET IsSupporter = ? WHERE Id = ?;");
	}
	else if(!str_comp_nocase(pCommand, "super_mod"))
	{
		if(pResult->NumArguments() < 3)
		{
			log_error("chatresp", "[SQL] Error: sql <command> <id> <value>");
			return;
		}

		int Value = pResult->GetInteger(2);
		pSelf->m_pAccounts->UpdateAccountState(ClientId, SqlId, Value, CAccountRconCmdResult::SUPER_MODERATOR, "UPDATE Accounts SET IsSuperModerator = ? WHERE Id = ?;");
	}
	else if(!str_comp_nocase(pCommand, "mod"))
	{
		if(pResult->NumArguments() < 3)
		{
			log_error("chatresp", "[SQL] Error: sql <command> <id> <value>");
			return;
		}

		int Value = pResult->GetInteger(2);
		pSelf->m_pAccounts->UpdateAccountState(ClientId, SqlId, Value, CAccountRconCmdResult::MODERATOR, "UPDATE Accounts SET IsModerator = ? WHERE Id = ?;");
	}
	else if(!str_comp_nocase(pCommand, "freeze_acc"))
	{
		if(pResult->NumArguments() < 3)
		{
			log_error("chatresp", "[SQL] Error: sql <command> <id> <value>");
			return;
		}

		int Value = pResult->GetInteger(2);
		pSelf->m_pAccounts->UpdateAccountState(ClientId, SqlId, Value, CAccountRconCmdResult::FREEZE_ACC, "UPDATE Accounts SET IsAccFrozen = ? WHERE Id = ?;");
	}
	else
	{
		log_info("chatresp", "Unknown SQL command. Try just '/sql' for more help.");
	}
}

void CGameContext::ConAcc_Info(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	if(g_Config.m_SvAccounts == 0)
	{
		pSelf->SendChatLocSys(ClientId, "ACCOUNT", "The account system is turned off.");
		return;
	}

	if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) != AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(ClientId, "[SQL] Missing permission.");
		return;
	}

	if(pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(ClientId, "[SQL] Use '/acc_info <name>'.");
		return;
	}

	char aUsername[32];
	str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
	int InfoId = pSelf->GetCidByName(aUsername);

	if(InfoId > -1)
	{
		if(!pSelf->m_apPlayers[InfoId]->IsLoggedIn())
		{
			pSelf->SendChatTarget(ClientId, "[SQL] This player is not logged in.");
			return;
		}

		char aBuf[512];
		str_format(
			aBuf,
			sizeof(aBuf),
			"==== '%s' %s ====",
			pSelf->Server()->ClientName(pSelf->m_apPlayers[InfoId]->GetCid()),
			pSelf->Loc("Account Info", ClientId));
		pSelf->SendChatTarget(ClientId, aBuf);
		str_format(aBuf, sizeof(aBuf), "Register date [%s]", pSelf->m_apPlayers[InfoId]->m_Account.m_aRegisterDate);
		pSelf->SendChatTarget(ClientId, aBuf);
		str_format(aBuf, sizeof(aBuf), "==== Username: '%s' SQL: %d ====", pSelf->m_apPlayers[InfoId]->m_Account.m_aUsername, pSelf->m_apPlayers[InfoId]->GetAccId());
		pSelf->SendChatTarget(ClientId, aBuf);
		pSelf->SendChatTarget(ClientId, pSelf->m_apPlayers[InfoId]->m_Account.m_LastLogoutIGN1);
		pSelf->SendChatTarget(ClientId, pSelf->m_apPlayers[InfoId]->m_Account.m_LastLogoutIGN2);
		pSelf->SendChatTarget(ClientId, pSelf->m_apPlayers[InfoId]->m_Account.m_LastLogoutIGN3);
		pSelf->SendChatTarget(ClientId, pSelf->m_apPlayers[InfoId]->m_Account.m_LastLogoutIGN4);
		pSelf->SendChatTarget(ClientId, pSelf->m_apPlayers[InfoId]->m_Account.m_LastLogoutIGN5);
		pSelf->SendChatTarget(ClientId, "======== IP ========");
		pSelf->SendChatTarget(ClientId, pSelf->m_apPlayers[InfoId]->m_Account.m_aIp_1);
		pSelf->SendChatTarget(ClientId, pSelf->m_apPlayers[InfoId]->m_Account.m_aIp_2);
		pSelf->SendChatTarget(ClientId, pSelf->m_apPlayers[InfoId]->m_Account.m_aIp_3);
		pSelf->SendChatTarget(ClientId, "======== Clan ========");
		pSelf->SendChatTarget(ClientId, pSelf->m_apPlayers[InfoId]->m_Account.m_aClan1);
		pSelf->SendChatTarget(ClientId, pSelf->m_apPlayers[InfoId]->m_Account.m_aClan2);
		pSelf->SendChatTarget(ClientId, pSelf->m_apPlayers[InfoId]->m_Account.m_aClan3);
		str_format(aBuf, sizeof(aBuf), "========= Skin '%s' =========", pSelf->m_apPlayers[InfoId]->m_Account.m_aSkin);
		pSelf->SendChatTarget(ClientId, aBuf);
	}
	else
	{
		pSelf->SendChatTarget(ClientId, "[SQL] Unknown player name.");
	}
}

void CGameContext::ConStats(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	int StatsId = ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	char aBuf[512];

	if(pResult->NumArguments() > 0) //other players stats
	{
		char aStatsName[32];
		str_copy(aStatsName, pResult->GetString(0), sizeof(aStatsName));
		StatsId = pSelf->GetCidByName(aStatsName);
		if(StatsId == -1)
		{
			str_format(aBuf, sizeof(aBuf), "[STATS] Can't find user '%s'", aStatsName);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			return;
		}
	}

	if(pPlayer->m_IsInstaArena_idm || pPlayer->m_IsInstaArena_gdm || g_Config.m_SvInstagibMode)
		pSelf->ShowInstaStats(ClientId, StatsId);
	else if(pPlayer->m_IsSurvivaling)
		pSelf->ShowSurvivalStats(ClientId, StatsId);
	else // blockcity stats
		pSelf->ShowDDPPStats(ClientId, StatsId);
}

void CGameContext::ConProfile(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "~~~ Profile help ~~~");
		pSelf->SendChatTarget(pResult->m_ClientId, "Profiles are connected to your account.");
		pSelf->SendChatTarget(pResult->m_ClientId, "More info about accounts with '/accountinfo'.");
		pSelf->SendChatTarget(pResult->m_ClientId, "--------------------");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/profile cmdlist' for command list.");
		return;
	}

	char aBuf[512];
	char aPara0[32];
	char aPara1[32];
	str_copy(aPara0, pResult->GetString(0), sizeof(aPara0));
	str_copy(aPara1, pResult->GetString(1), sizeof(aPara1));
	int ViewId = pSelf->GetCidByName(aPara1);

	if(!str_comp_nocase(aPara0, "help"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "--- Profile help ---");
		pSelf->SendChatTarget(pResult->m_ClientId, "Profiles are connected with your account.");
		pSelf->SendChatTarget(pResult->m_ClientId, "More info about accounts with '/accountinfo'.");
		pSelf->SendChatTarget(pResult->m_ClientId, "--------------------");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/profile cmdlist' for command list.");
	}
	else if(!str_comp_nocase(aPara0, "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "--- Profile Commands ---");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/profile view <playername>' to view a players profile.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/profile style <style>' to change profile style.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/profile status <status> to change status.'");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/profile email <e-mail>' to change e-mail.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/profile homepage <homepage>' to change homepage.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/profile youtube <youtube>' to change youtube.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/profile skype <skype>' to change skype.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/profile twitter <twitter>' to change twitter.");
	}
	else if(!str_comp_nocase(aPara0, "view") || !str_comp_nocase(aPara0, "watch"))
	{
		if(pResult->NumArguments() < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Missing parameters. Stick to struct: 'profile view <playername>'.");
			return;
		}

		if(ViewId == -1)
		{
			str_format(aBuf, sizeof(aBuf), "Can't find user: '%s'", aPara1);
			return;
		}

		pSelf->ShowProfile(pResult->m_ClientId, ViewId);
	}
	else
	{
		if(!pPlayer->IsLoggedIn()) //also gets triggered on unknown commands but whatever if logged in all works fine
		{
			//pSelf->SendChatTarget(pResult->m_ClientId, "Unknown command or:");
			pSelf->SendChatTarget(pResult->m_ClientId, "You have to be logged in to use this command.");
			pSelf->SendChatTarget(pResult->m_ClientId, "All info about accounts: '/accountinfo'");
			return;
		}

		if(!str_comp_nocase(aPara0, "style"))
		{
			if(!str_comp_nocase(aPara1, "default"))
			{
				pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileStyle = 0;
				pSelf->SendChatTarget(pResult->m_ClientId, "Changed profile-style to: default");
			}
			else if(!str_comp_nocase(aPara1, "shit"))
			{
				pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileStyle = 1;
				pSelf->SendChatTarget(pResult->m_ClientId, "Changed profile-style to: shit");
			}
			else if(!str_comp_nocase(aPara1, "social"))
			{
				pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileStyle = 2;
				pSelf->SendChatTarget(pResult->m_ClientId, "Changed profile-style to: social");
			}
			else if(!str_comp_nocase(aPara1, "show-off"))
			{
				pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileStyle = 3;
				pSelf->SendChatTarget(pResult->m_ClientId, "Changed profile-style to: show-off");
			}
			else if(!str_comp_nocase(aPara1, "pvp"))
			{
				pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileStyle = 4;
				pSelf->SendChatTarget(pResult->m_ClientId, "Changed profile-style to: pvp");
			}
			else if(!str_comp_nocase(aPara1, "bomber"))
			{
				pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileStyle = 5;
				pSelf->SendChatTarget(pResult->m_ClientId, "Changed profile-style to: bomber");
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "error: '%s' is not a profile style. Choose between the following: default, shit, social, show-off, pvp, bomber", aPara1);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			}
		}
		else if(!str_comp_nocase(aPara0, "status"))
		{
			if(pResult->NumArguments() < 2)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] status: %s", pPlayer->m_Account.m_ProfileStatus);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}
			if(!pSelf->IsAllowedCharSet(aPara1))
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] please use only the following characters in your status '%s'", pSelf->m_aAllowedCharSet);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}

			str_copy(pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileStatus, aPara1, sizeof(pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileStatus));
			str_format(aBuf, sizeof(aBuf), "Updated your profile status: %s", pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileStatus);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
		else if(!str_comp_nocase(aPara0, "skype"))
		{
			if(pResult->NumArguments() < 2)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] skype: %s", pPlayer->m_Account.m_ProfileSkype);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}
			if(!pSelf->IsAllowedCharSet(aPara1))
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] please use only the following characters in your skype '%s'", pSelf->m_aAllowedCharSet);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}

			str_copy(pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileSkype, aPara1, sizeof(pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileSkype));
			str_format(aBuf, sizeof(aBuf), "Updated your profile skype: %s", pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileSkype);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
		else if(!str_comp_nocase(aPara0, "youtube"))
		{
			if(pResult->NumArguments() < 2)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] youtube: %s", pPlayer->m_Account.m_ProfileYoutube);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}
			if(!pSelf->IsAllowedCharSet(aPara1))
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] please use only the following characters in your youtube '%s'", pSelf->m_aAllowedCharSet);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}

			str_copy(pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileYoutube, aPara1, sizeof(pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileYoutube));
			str_format(aBuf, sizeof(aBuf), "Updated your profile youtube: %s", pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileYoutube);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
		else if(!str_comp_nocase(aPara0, "email") || !str_comp_nocase(aPara0, "e-mail"))
		{
			if(pResult->NumArguments() < 2)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] email: %s", pPlayer->m_Account.m_ProfileEmail);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}
			if(!pSelf->IsAllowedCharSet(aPara1))
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] please use only the following characters in your email '%s'", pSelf->m_aAllowedCharSet);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}

			str_copy(pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileEmail, aPara1, sizeof(pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileEmail));
			str_format(aBuf, sizeof(aBuf), "Updated your profile e-mail: %s", pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileEmail);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
		else if(!str_comp_nocase(aPara0, "homepage") || !str_comp_nocase(aPara0, "website"))
		{
			if(pResult->NumArguments() < 2)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] homepage: %s", pPlayer->m_Account.m_ProfileHomepage);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}
			if(!pSelf->IsAllowedCharSet(aPara1))
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] please use only the following characters in your homepage '%s'", pSelf->m_aAllowedCharSet);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}

			str_copy(pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileHomepage, aPara1, sizeof(pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileHomepage));
			str_format(aBuf, sizeof(aBuf), "Updated your profile homepage: %s", pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileHomepage);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
		else if(!str_comp_nocase(aPara0, "twitter"))
		{
			if(pResult->NumArguments() < 2)
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] twitter: %s", pPlayer->m_Account.m_ProfileTwitter);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}
			if(!pSelf->IsAllowedCharSet(aPara1))
			{
				str_format(aBuf, sizeof(aBuf), "[PROFILE] please use only the following characters in your twitter '%s'", pSelf->m_aAllowedCharSet);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}

			str_copy(pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileTwitter, aPara1, sizeof(pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileTwitter));
			str_format(aBuf, sizeof(aBuf), "Updated your profile twitter: %s", pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_ProfileTwitter);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Unknown command. '/profile cmdlist' or '/profile help' might help.");
		}
	}
}

void CGameContext::ConLogin(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	if(g_Config.m_SvAccounts == 0)
	{
		pSelf->SendChatLocSys(ClientId, "ACCOUNT", "The account system is turned off.");
		return;
	}
	if(g_Config.m_SvAccounts == 2) //filebased
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] SQLite accounts are turned off. (try '/login2')");
		return;
	}

	if(pPlayer->m_Account.m_JailTime)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] You can't login in jail.");
		return;
	}

	if(pPlayer->m_PlayerHumanLevel < g_Config.m_SvLoginHumanLevel)
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "[ACCOUNT] Your '/human_level' is too low %d/%d to use this command.", pPlayer->m_PlayerHumanLevel, g_Config.m_SvLoginHumanLevel);
		pSelf->SendChatTarget(ClientId, aBuf);
		return;
	}

	const NETADDR *pAddr = pSelf->Server()->ClientAddr(ClientId);
	int Banned = 0;

	for(int i = 0; i < pSelf->m_NumLoginBans && !Banned; i++)
	{
		if(!net_addr_comp_noport(pAddr, &pSelf->m_aLoginBans[i].m_Addr))
			Banned = (pSelf->m_aLoginBans[i].m_Expire - pSelf->Server()->Tick()) / pSelf->Server()->TickSpeed();
	}

	if(Banned > 0)
	{
		char aBuf[128];
		str_format(aBuf, sizeof aBuf, "[ACCOUNT] You have to wait %d seconds before you can login again.", Banned);
		pSelf->SendChatTarget(ClientId, aBuf);
		return;
	}

	char aUsername[32];
	char aPassword[MAX_PW_LEN + 1];

	if(pResult->NumArguments() == 1)
	{
		str_copy(aUsername, pSelf->Server()->ClientName(ClientId), sizeof(aUsername));
		str_copy(aPassword, pResult->GetString(0), sizeof(aPassword));
		//char aBuf[128];
		//pSelf->SendChatTarget(ClientId, aBuf);
		//str_format(aBuf, sizeof(aBuf), "[ACCOUNT] WARNING no username given. (trying '%s')", aUsername);
	}
	else if(pResult->NumArguments() == 2)
	{
		str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
		str_copy(aPassword, pResult->GetString(1), sizeof(aPassword));
	}
	else
	{
		pSelf->SendChatTarget(ClientId, pSelf->Loc("[ACCOUNT] Use '/login <name> <password>'", ClientId));
		pSelf->SendChatTarget(ClientId, pSelf->Loc("[ACCOUNT] Use '/accountinfo' for help", ClientId));
		return;
	}

	if(pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(ClientId, pSelf->Loc("[ACCOUNT] You are already logged in", ClientId));
		return;
	}

	if(str_length(aUsername) > MAX_PW_LEN || str_length(aUsername) < MIN_PW_LEN)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Username is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if(str_length(aPassword) > MAX_PW_LEN || str_length(aPassword) < MIN_PW_LEN)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Password is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	pSelf->Accounts()->Login(ClientId, aUsername, aPassword);
}

void CGameContext::ConChangePassword(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	if(!pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "You are not logged in. (More info '/accountinfo')");
		return;
	}
	if(pResult->NumArguments() != 3)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Please use: '/changepassword <oldpw> <newpw> <repeat_newpw>'");
		return;
	}

	char aOldPass[MAX_PW_LEN + 1];
	char aNewPass[MAX_PW_LEN + 1];
	char aNewPass2[MAX_PW_LEN + 1];
	str_copy(aOldPass, pResult->GetString(0), sizeof(aOldPass));
	str_copy(aNewPass, pResult->GetString(1), sizeof(aNewPass));
	str_copy(aNewPass2, pResult->GetString(2), sizeof(aNewPass2));

	if(str_length(aOldPass) > MAX_PW_LEN || str_length(aOldPass) < MIN_PW_LEN)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Your old password is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if((str_length(aNewPass) > MAX_PW_LEN || str_length(aNewPass) < MIN_PW_LEN) || (str_length(aNewPass2) > MAX_PW_LEN || str_length(aNewPass2) < MIN_PW_LEN))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Your password is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if(str_comp_nocase(aNewPass, aNewPass2) != 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "New passwords have to be identical.");
		return;
	}

	if(str_comp(aOldPass, pPlayer->m_Account.m_aPassword))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Wrong old password.");
		return;
	}

	// even if the sql query fails passwords are not saved on logout
	// but this makes sure that if a password is changed multiple times its compared against the new one
	str_copy(pPlayer->m_Account.m_aPassword, aNewPass, sizeof(pPlayer->m_Account.m_aPassword));
	pSelf->Accounts()->ChangePassword(pResult->m_ClientId, pPlayer->m_Account.m_aUsername, pPlayer->m_Account.m_aPassword, aNewPass);
}

void CGameContext::ConAccLogout(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	if(g_Config.m_SvAccounts == 0)
	{
		pSelf->SendChatLocSys(ClientId, "ACCOUNT", "The account system is turned off.");
		return;
	}
	/*
	if (g_Config.m_SvAccounts == 2) //filebased
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] SQLite accounts are turned off.");
		return;
	}
	*/

	CCharacter *pChr = pPlayer->GetCharacter();
	if(pChr)
	{
		if(pChr->m_FreezeTime)
		{
			pSelf->SendChatLocSys(ClientId, "ACCOUNT", "You can't logout while being frozen.");
			return;
		}
	}

	if(pPlayer->m_Account.m_JailTime)
	{
		pSelf->SendChatLocSys(ClientId, "ACCOUNT", "You can't logout in jail.");
		return;
	}

	if(!pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] You are not logged in.");
		return;
	}

	if(pPlayer->m_Insta1on1_id != -1)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] You can't logout in 1on1. ('/insta leave' to leave)");
		return;
	}

	if(pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] You can't logout in insta matches. ('/insta leave' to leave)");
		return;
	}

	if(pSelf->IsMinigame(pResult->m_ClientId) > 0) //all minigames no jail returns bigger than zero
	{
		pSelf->SendChatLocSys(ClientId, "ACCOUNT", "You can't logout during minigames try '/leave'");
		return;
	}

	if(pPlayer->GetCharacter())
	{
		if(pPlayer->GetCharacter()->m_IsBombing)
		{
			pSelf->SendChatTarget(ClientId, "[ACCOUNT] You can't logout in bomb games. ('/bomb leave' to leave)");
			return;
		}
		if(pPlayer->GetCharacter()->m_IsPvpArenaing)
		{
			pSelf->SendChatTarget(ClientId, "[ACCOUNT] You can't logout in pvp_arena. ('/pvp_arena leave' to leave)");
			return;
		}
	}

	pPlayer->Logout();
	pSelf->SendChatTarget(ClientId, pSelf->Loc("[ACCOUNT] Logged out", ClientId));
}

void CGameContext::ConChidraqul(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	char aCommand[64];
	str_copy(aCommand, pResult->GetString(0), sizeof(aCommand));

	if(!str_comp_nocase(aCommand, "info") || !str_comp_nocase(aCommand, "help"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "==== chidraqul3 ====");
		pSelf->SendChatTarget(pResult->m_ClientId, "The chidraqul minigame in his third generation.");
		pSelf->SendChatTarget(pResult->m_ClientId, "Buy the game in '/shop' with '/buy chidraqul' and you can use it until disconnect.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/chidraqul cmdlist' for a list of all commands");
	}
	else if(!str_comp_nocase(aCommand, "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== chirdraqul3 commands ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/chidraqul start' to start the game");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/chidraqul stop' to stop the game");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/chidraqul r' to move right");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/chidraqul l' to move left");
	}
	else if(!str_comp_nocase(aCommand, "start"))
	{
		if(pPlayer->m_BoughtGame)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[chidraqul] started.");
			str_format(pPlayer->m_HashSkin, sizeof(pPlayer->m_HashSkin), "%s", g_Config.m_SvChidraqulDefaultSkin);
			pPlayer->m_C3_GameState = 1; //singleplayer
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You don't have this game. You can buy it with '/buy chidraqul'");
		}
	}
	else if(!str_comp_nocase(aCommand, "stop") || !str_comp_nocase(aCommand, "quit"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[chidraqul] stopped.");
		pSelf->m_apPlayers[pResult->m_ClientId]->m_C3_GameState = false;
		pSelf->SendBroadcast(" ", pResult->m_ClientId);
	}
	else if(!str_comp_nocase(aCommand, "r"))
	{
		if(pPlayer->m_HashPos < g_Config.m_SvChidraqulWorldX - 1) //space for the string delimiter
		{
			pPlayer->m_HashPos++;
			pPlayer->m_C3_UpdateFrame = true;
		}
	}
	else if(!str_comp_nocase(aCommand, "l"))
	{
		if(pPlayer->m_HashPos > 0)
		{
			pPlayer->m_HashPos--;
			pPlayer->m_C3_UpdateFrame = true;
		}
	}
	else if(!str_comp_nocase(aCommand, "multiplayer"))
	{
		if(!pPlayer->m_BoughtGame)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You don't have this game. You can buy it with '/buy chidraqul'");
			return;
		}

		pPlayer->JoinMultiplayer();
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[chidraqul] Unknown command. try '/chidraqul help'");
	}
}

void CGameContext::ConMinigames(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "You have to be ingame to use this command.");
		return;
	}

	if(pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== MINIGAMES ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "This command gives you overview");
		pSelf->SendChatTarget(pResult->m_ClientId, "off all minigames!");
		pSelf->SendChatTarget(pResult->m_ClientId, "Btw we also have quests. Check '/quest'.");
		pSelf->SendChatTarget(pResult->m_ClientId, "");
		pSelf->SendChatTarget(pResult->m_ClientId, "check '/minigames cmdlist' for all commands");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== MINIGAMES COMMANDS ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/minigames status' gives life minigame status");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/minigames list' lists all minigames");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/minigames info' shows some info about the command");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "list"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=========================");
		pSelf->SendChatTarget(pResult->m_ClientId, "===== LIST OF ALL MINIGAMES =====");
		pSelf->SendChatTarget(pResult->m_ClientId, "=========================");
		pSelf->SendChatTarget(pResult->m_ClientId, "GAME:             COMMAND:");
		pSelf->SendChatTarget(pResult->m_ClientId, "[INSTAGIB]        '/insta'"); // 1=grenade 2=rifle
		pSelf->SendChatTarget(pResult->m_ClientId, "[BALANCE]        '/balance'"); // 3
		pSelf->SendChatTarget(pResult->m_ClientId, "[SURVIVAL]       '/survival'"); // 4
		pSelf->SendChatTarget(pResult->m_ClientId, "[BOMB]             '/bomb'"); // 5
		pSelf->SendChatTarget(pResult->m_ClientId, "[PVP]                 '/pvp_arena'"); // 6
		pSelf->SendChatTarget(pResult->m_ClientId, "[BLOCKWAVE]   '/blockwave'"); // 7
		// block tournament										/join						// 8
		pSelf->SendChatTarget(pResult->m_ClientId, "[BLOCK]            '/block'"); // 9
		pSelf->SendChatTarget(pResult->m_ClientId, "[1vs1]              '/1vs1'"); // 10
	}
	else if(!str_comp_nocase(pResult->GetString(0), "status"))
	{
		int GameId = pSelf->IsMinigame(pResult->m_ClientId);

		pSelf->SendChatTarget(pResult->m_ClientId, "===== MINIGAME STATUS =====");

		if(!GameId)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You aren't currently minigaming.");
		}
		else if(GameId == -1)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You are not allowed to play mingames.");
			pSelf->SendChatTarget(pResult->m_ClientId, "because you are jailed.");
		}
		else if(GameId == 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTAGIB] gdm (check '/insta' for more info)");
		}
		else if(GameId == 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTAGIB] idm (check '/insta' for more info)");
		}
		else if(GameId == 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BALANCE] (check '/balance' for more info)");
		}
		else if(GameId == 4)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[SURVIVAL] (check '/survival' for more info)");
		}
		else if(GameId == 5)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BOMB] (check '/bomb' for more info)");
		}
		else if(GameId == 6)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[PVP] (check '/pvp_arena' for more info)");
		}
		else if(GameId == 7)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BLOCKWAVE] (check '/blockwave' for more info)");
		}
		else if(GameId == 8)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[EVENT] Block (check '/event' for more info)");
		}
		else if(GameId == 9)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BLOCK] '/block' to join");
			pSelf->SendChatTarget(pResult->m_ClientId, "[BLOCK] '/leave' to leave block deathmatch");
		}
		else if(GameId == 10)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[1vs1] '/1vs1' to join");
			pSelf->SendChatTarget(pResult->m_ClientId, "[1vs1] '/leave' to leave block 1vs1");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[UNKNOWN] you are playing an unknown game.");
		}

		pSelf->SendChatTarget(pResult->m_ClientId, "======================");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Unknown minigames argument (Check '/minigames cmdlist').");
	}
}

void CGameContext::ConCC(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) == AUTHED_ADMIN)
	{
		pSelf->SendChat(-1, TEAM_ALL, "'namless rofl' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'(1)namless tee' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'einFISCH' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'GAGAGA' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'Steve-' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'Gro�erDBC' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'cB | Bashcord' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'LIVUS BAGGUGE' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'BoByBANK' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'noobson tnP' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'vali' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'ChiliDreghugn' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'Stahkilla' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'Detztin' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'pikutee <3' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'namless tee' has left the game");
		pSelf->SendChat(-1, TEAM_ALL, "'BoByBANK' has left the game");
		pSelf->SendChat(-1, TEAM_ALL, "'Ubu' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'Magnet' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'Jambi*' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'HurricaneZ' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'Sonix' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'darkdragonovernoob' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'Ubu' has left the game");
		pSelf->SendChat(-1, TEAM_ALL, "'deen' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'fuck me soon' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'fik you!' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'fuckmeson' has left the game");
		pSelf->SendChat(-1, TEAM_ALL, "'(1)' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'Noved' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'Aoe' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'artkis' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'namless brain' entered and joined the game");
		pSelf->SendChat(-1, TEAM_ALL, "'(1)ChillerDrago' has left the game");
		pSelf->SendChat(-1, TEAM_ALL, "'HurricaneZ' has left the game");
		pSelf->SendChat(-1, TEAM_ALL, "'(2)ChillerDrago' has left the game");
		pSelf->SendChat(-1, TEAM_ALL, "'hax0r' has left the game");
		pSelf->SendChat(-1, TEAM_ALL, "'(3)ChillerDrago' has left the game");
		pSelf->SendChat(-1, TEAM_ALL, "'SarKro' has left the game");
	}
	else
	{
		//pSelf->SendChatTarget(pResult->m_ClientId, "No such command: %s.");
		pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission.");
	}
}

void CGameContext::ConBalance(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[BALANCE] You have to be ingame to use this command.");
		return;
	}

	if(!g_Config.m_SvAllowBalance)
	{
		pSelf->SendChatLocSys(pResult->m_ClientId, "BALANCE", "%s", "this command is deactivated by an administrator.");
		return;
	}

	char aBuf[128];

	if(pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== BALANCE HELP ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "Battle friends in a tee balance 1on1");
		pSelf->SendChatTarget(pResult->m_ClientId, "check '/balance cmdlist' for a list of all commands");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== INSTAGIB COMMANDS ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/balance battle <player>' to invite a player");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/balance accept <player>' to accept a battle");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/balance help' to show some help and info");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "battle"))
	{
		vec2 BalanceBattleSpawn1 = pSelf->Collision()->GetRandomTile(TILE_BALANCE_BATTLE_1);
		vec2 BalanceBattleSpawn2 = pSelf->Collision()->GetRandomTile(TILE_BALANCE_BATTLE_2);
		int MateId = pSelf->GetCidByName(pResult->GetString(1));
		if(MateId == -1)
		{
			str_format(aBuf, sizeof(aBuf), "[BALANCE] Can't find the user '%s'", pResult->GetString(1));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			return;
		}
		else if(MateId == pResult->m_ClientId)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BALANCE] You can't invite your self.");
			return;
		}
		else if(BalanceBattleSpawn1 == vec2(-1, -1) || BalanceBattleSpawn2 == vec2(-1, -1))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BALANCE] no battle arena found.");
			return;
		}
		else if(pSelf->IsMinigame(pResult->m_ClientId))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Error: maybe you are already in a minigame or jail. (check '/minigames status')");
		}
		//else if (!pSelf->m_apPlayers[mateId]->IsLoggedIn())
		//{
		//	pSelf->SendChatTarget(pResult->m_ClientId, "This player is not logged in.");
		//	return;
		//}
		//else if (pPlayer->GetMoney() < 10)
		//{
		//	pSelf->SendChatTarget(pResult->m_ClientId, "You don't have 10 money to start a game.");
		//	return;
		//}
		else
		{
			pPlayer->m_BalanceBattle_id = MateId;
			str_format(aBuf, sizeof(aBuf), "[BALANCE] Invited '%s' to a battle", pResult->GetString(1));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

			str_format(aBuf, sizeof(aBuf), "[BALANCE] '%s' invited you to a battle.", pSelf->Server()->ClientName(pResult->m_ClientId));
			pSelf->SendChatTarget(MateId, aBuf);
			str_format(aBuf, sizeof(aBuf), "('/balance accept %s' to accept)", pSelf->Server()->ClientName(pResult->m_ClientId));
			pSelf->SendChatTarget(MateId, aBuf);
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "accept"))
	{
		int MateId = pSelf->GetCidByName(pResult->GetString(1));
		if(MateId == -1)
		{
			str_format(aBuf, sizeof(aBuf), "[BALANCE] Can't find the user '%s'", pResult->GetString(1));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			return;
		}
		else if(pSelf->m_apPlayers[MateId]->m_BalanceBattle_id != pResult->m_ClientId)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BALANCE] This player didn't invite you.");
			return;
		}
		else if(pSelf->IsMinigame(pResult->m_ClientId))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Error: maybe you are already in a minigame or jail. (check '/minigames status')");
		}
		else
		{
			pSelf->StartBalanceBattle(MateId, pResult->m_ClientId);
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[BALANCE] Unknown parameter. Check '/balance cmdlist' for all commands.");
	}
}

void CGameContext::ConInsta(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	if(!g_Config.m_SvAllowInsta)
	{
		pSelf->SendChatLocSys(pResult->m_ClientId, "INSTA", "%s", "this command is deactivated by an administrator.");
		return;
	}

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You have to be ingame to use this command.");
		return;
	}

	if(!pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You are not logged in. Use  '/accountinfo' or more info.");
		return;
	}

	char aBuf[256];

	if(pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== INSTAGIB HELP ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "Kill people with one shot in a special arena.");
		pSelf->SendChatTarget(pResult->m_ClientId, "Check '/insta cmdlist' for a list of all commands.");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== INSTAGIB COMMANDS ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/insta leave' to leave any kind of instagib game");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/insta gdm' to join grenade deathmatch instagib game");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/insta boomfng' to join grenade fng game");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/insta idm' to join rifle deathmatch instagib game");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/insta fng' to join rifle fng game");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/insta 1on1 <mode> <player>' to 1on1 <player> (+100 money for the winner)");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/insta stats' to show game statistics");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/insta help' for help and info");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "leave"))
	{
		pSelf->m_pInstagib->Leave(pPlayer);
	}
	else if(!str_comp_nocase(pResult->GetString(0), "stats"))
	{
		pSelf->ShowInstaStats(pResult->m_ClientId, pResult->m_ClientId);
	}
	else if(!str_comp_nocase(pResult->GetString(0), "gdm"))
	{
		if(pPlayer->m_IsInstaArena_gdm)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You are already in a grenade game. ('/insta leave' to leave)");
		}
		else if(pPlayer->m_IsInstaArena_idm)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You are already in a rifle game. ('/insta leave' to leave)");
		}
		else if(pSelf->IsMinigame(pResult->m_ClientId))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
		}
		else if(!pSelf->CanJoinInstaArena(true, false))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Arena is full.");
		}
		else if(g_Config.m_SvAllowGrenade == 2 || g_Config.m_SvAllowGrenade == 0)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] gdm is currently deactivated by an admin.");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You joined a grenade game.");
			pSelf->m_pInstagib->Join(pPlayer, WEAPON_GRENADE, false);
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "idm"))
	{
		if(pPlayer->m_IsInstaArena_gdm)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You are already in a grenade game. ('/insta leave' to leave)");
		}
		else if(pPlayer->m_IsInstaArena_idm)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You are already in a rifle game. ('/insta leave' to leave)");
		}
		else if(pSelf->IsMinigame(pResult->m_ClientId))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
		}
		else if(!pSelf->CanJoinInstaArena(false, false))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Arena is full.");
		}
		else if(g_Config.m_SvAllowRifle == 2 || g_Config.m_SvAllowRifle == 0)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] idm is currently deactivated by an admin.");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You joined a rifle game.");
			pSelf->m_pInstagib->Join(pPlayer, WEAPON_LASER, false);
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "boomfng"))
	{
		if(pPlayer->m_IsInstaArena_gdm)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You are already in a grenade game. ('/insta leave' to leave)");
		}
		else if(pPlayer->m_IsInstaArena_idm)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You are already in a rifle game. ('/insta leave' to leave)");
		}
		else if(pSelf->IsMinigame(pResult->m_ClientId))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
		}
		else if(!pSelf->CanJoinInstaArena(true, false))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Arena is full.");
		}
		else if(g_Config.m_SvAllowGrenade == 0 || g_Config.m_SvAllowGrenade == 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] boomfng is currently deactivated by an admin.");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You joined a boomfng game.");
			pSelf->m_pInstagib->Join(pPlayer, WEAPON_GRENADE, true);
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "fng"))
	{
		if(pPlayer->m_IsInstaArena_gdm)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You are already in a grenade game. ('/insta leave' to leave)");
		}
		else if(pPlayer->m_IsInstaArena_idm)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You are already in a rifle game. ('/insta leave' to leave)");
		}
		else if(pSelf->IsMinigame(pResult->m_ClientId))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
		}
		else if(!pSelf->CanJoinInstaArena(false, false))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Arena is full.");
		}
		else if(g_Config.m_SvAllowRifle == 0 || g_Config.m_SvAllowRifle == 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] fng is currently deactivated by an admin.");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You joined a fng game.");
			pSelf->m_pInstagib->Join(pPlayer, WEAPON_LASER, true);
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "1on1"))
	{
		if(pResult->NumArguments() == 1 || !str_comp_nocase(pResult->GetString(1), "help") || !str_comp_nocase(pResult->GetString(1), "info"))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "=== Insta 1on1 ===");
			pSelf->SendChatTarget(pResult->m_ClientId, "Battle 1 vs 1 in gdm, idm, fng or boomfng.");
			pSelf->SendChatTarget(pResult->m_ClientId, "The winner gets 100 money from the looser.");
			pSelf->SendChatTarget(pResult->m_ClientId, "=== commands ===");
			pSelf->SendChatTarget(pResult->m_ClientId, "'/insta 1on1 gdm <player>'");
			pSelf->SendChatTarget(pResult->m_ClientId, "'/insta 1on1 idm <player>'");
			pSelf->SendChatTarget(pResult->m_ClientId, "'/insta 1on1 boomfng <player>'");
			pSelf->SendChatTarget(pResult->m_ClientId, "'/insta 1on1 fng <player>'");
			pSelf->SendChatTarget(pResult->m_ClientId, "'/insta 1on1 accept <player>'");

			//description is too long and goes newline --> lukz ugly af

			//pSelf->SendChatTarget(pResult->m_ClientId, "'/insta 1on1 gdm <player>' to send a gdm 1on1 request to <player>");
			//pSelf->SendChatTarget(pResult->m_ClientId, "'/insta 1on1 idm <player>' to send a idm 1on1 request to <player>");
			//pSelf->SendChatTarget(pResult->m_ClientId, "'/insta 1on1 boomfng <player>' to send a boomfng 1on1 request to <player>");
			//pSelf->SendChatTarget(pResult->m_ClientId, "'/insta 1on1 fng <player>' to send a fng 1on1 request to <player>");
			//pSelf->SendChatTarget(pResult->m_ClientId, "'/insta 1on1 accept <player>' to accept <player>'s 1on1 request");
		}
		else if(!str_comp_nocase(pResult->GetString(1), "gdm"))
		{
			int MateId = pSelf->GetCidByName(pResult->GetString(2));
			if(MateId == -1)
			{
				str_format(aBuf, sizeof(aBuf), "[INSTA] Can't find playername: '%s'.", pResult->GetString(2));
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}
			else if(pSelf->IsMinigame(pResult->m_ClientId))
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			}
			else if(MateId == pResult->m_ClientId)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You can't invite yourself.");
				return;
			}
			else if(!pSelf->m_apPlayers[MateId]->IsLoggedIn())
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] This player is not logged in.");
				return;
			}
			else if(pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You can't invite while being ingame. Do '/insta leave' first.");
				return;
			}
			else if(pPlayer->GetMoney() < 100)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You need at least 100 money to start a game.");
				return;
			}

			pPlayer->m_Insta1on1_id = MateId; //set this id to -1 if you join any kind of insta game which is not 1on1
			pPlayer->m_Insta1on1_mode = 0; //gdm
			str_format(aBuf, sizeof(aBuf), "[INSTA] Invited '%s' to a gdm 1on1.", pResult->GetString(2));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

			str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' invited you to a gdm 1on1. ('/insta 1on1 accept %s' to accept)", pSelf->Server()->ClientName(pResult->m_ClientId), pSelf->Server()->ClientName(pResult->m_ClientId));
			pSelf->SendChatTarget(MateId, aBuf);
		}
		else if(!str_comp_nocase(pResult->GetString(1), "idm"))
		{
			int MateId = pSelf->GetCidByName(pResult->GetString(2));
			if(MateId == -1)
			{
				str_format(aBuf, sizeof(aBuf), "[INSTA] Can't find playername: '%s'.", pResult->GetString(2));
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}
			else if(pSelf->IsMinigame(pResult->m_ClientId))
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			}
			else if(MateId == pResult->m_ClientId)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You can't invite yourself.");
				return;
			}
			else if(!pSelf->m_apPlayers[MateId]->IsLoggedIn())
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] This player is not logged in.");
				return;
			}
			else if(pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You can't invite while being ingame. Do '/insta leave' first.");
				return;
			}
			else if(pPlayer->GetMoney() < 100)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You need at least 100 money to start a game.");
				return;
			}

			pPlayer->m_Insta1on1_id = MateId; //set this id to -1 if you join any kind of insta game which is not 1on1
			pPlayer->m_Insta1on1_mode = 1; //idm
			str_format(aBuf, sizeof(aBuf), "[INSTA] Invited '%s' to a idm 1on1.", pResult->GetString(2));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

			str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' invited you to a idm 1on1. ('/insta 1on1 accept %s' to accept)", pSelf->Server()->ClientName(pResult->m_ClientId), pSelf->Server()->ClientName(pResult->m_ClientId));
			pSelf->SendChatTarget(MateId, aBuf);
		}
		else if(!str_comp_nocase(pResult->GetString(1), "boomfng"))
		{
			int MateId = pSelf->GetCidByName(pResult->GetString(2));
			if(MateId == -1)
			{
				str_format(aBuf, sizeof(aBuf), "[INSTA] Can't find playername: '%s'.", pResult->GetString(2));
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}
			else if(pSelf->IsMinigame(pResult->m_ClientId))
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			}
			else if(MateId == pResult->m_ClientId)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You can't invite yourself.");
				return;
			}
			else if(!pSelf->m_apPlayers[MateId]->IsLoggedIn())
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] This player is not logged in.");
				return;
			}
			else if(pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You can't invite while being ingame. Do '/insta leave' first.");
				return;
			}
			else if(pPlayer->GetMoney() < 100)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You need at least 100 money to start a game.");
				return;
			}

			pPlayer->m_Insta1on1_id = MateId; //set this id to -1 if you join any kind of insta game which is not 1on1
			pPlayer->m_Insta1on1_mode = 2; //boomfng
			str_format(aBuf, sizeof(aBuf), "[INSTA] Invited '%s' to a boomfng 1on1.", pResult->GetString(2));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

			str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' invited you to a boomfng 1on1. ('/insta 1on1 accept %s' to accept)", pSelf->Server()->ClientName(pResult->m_ClientId), pSelf->Server()->ClientName(pResult->m_ClientId));
			pSelf->SendChatTarget(MateId, aBuf);
		}
		else if(!str_comp_nocase(pResult->GetString(1), "fng"))
		{
			int MateId = pSelf->GetCidByName(pResult->GetString(2));
			if(MateId == -1)
			{
				str_format(aBuf, sizeof(aBuf), "[INSTA] Can't find playername: '%s'.", pResult->GetString(2));
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}
			else if(pSelf->IsMinigame(pResult->m_ClientId))
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			}
			else if(MateId == pResult->m_ClientId)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You can't invite yourself.");
				return;
			}
			else if(!pSelf->m_apPlayers[MateId]->IsLoggedIn())
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] This player is not logged in.");
				return;
			}
			else if(pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You can't invite while being ingame. Do '/insta leave' first.");
				return;
			}
			else if(pPlayer->GetMoney() < 100)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You don't have 100 money to start a game.");
				return;
			}

			pPlayer->m_Insta1on1_id = MateId; //set this id to -1 if you join any kind of insta game which is not 1on1
			pPlayer->m_Insta1on1_mode = 3; //fng
			str_format(aBuf, sizeof(aBuf), "[INSTA] Invited '%s' to a fng 1on1.", pResult->GetString(2));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

			str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' invited you to a fng 1on1. ('/insta 1on1 accept %s' to accept)", pSelf->Server()->ClientName(pResult->m_ClientId), pSelf->Server()->ClientName(pResult->m_ClientId));
			pSelf->SendChatTarget(MateId, aBuf);
		}
		else if(!str_comp_nocase(pResult->GetString(1), "accept"))
		{
			int MateId = pSelf->GetCidByName(pResult->GetString(2));
			if(MateId == -1)
			{
				str_format(aBuf, sizeof(aBuf), "[INSTA] Can't find playername: '%s'.", pResult->GetString(2));
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			}
			else if(pSelf->IsMinigame(pResult->m_ClientId))
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			}
			else if(pSelf->m_apPlayers[MateId]->m_Insta1on1_id != pResult->m_ClientId)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] This player didn't invite you.");
			}
			else if(pPlayer->m_IsInstaArena_gdm || pPlayer->m_IsInstaArena_idm)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You can't accept while being ingame. Do '/insta leave' first.");
				return;
			}
			else if(pPlayer->GetMoney() < 100)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You need at least 100 money to start a game.");
				return;
			}
			else if(pSelf->m_apPlayers[MateId]->GetMoney() < 100)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Your 1on1 mate doesn't have enough money to start a game.");
				return;
			}
			else
			{
				if(pSelf->m_apPlayers[MateId]->m_Insta1on1_mode == 0 || pSelf->m_apPlayers[MateId]->m_Insta1on1_mode == 2) //grenade
				{
					if(!pSelf->CanJoinInstaArena(true, true))
					{
						pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA]Arena is too full to start 1on1.");
					}
					else //everything succeeded! yay --> start 1on1
					{
						if(pSelf->m_apPlayers[MateId]->m_Insta1on1_mode == 0)
						{
							pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You joined a gdm 1on1 (-100 money)");
							pSelf->SendChatTarget(MateId, "[INSTA] You joined a gdm 1on1 (-100 money)");
						}
						else
						{
							pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You joined a boomfng 1on1 (-100 money)");
							pPlayer->m_IsInstaArena_fng = true;

							pSelf->SendChatTarget(MateId, "[INSTA] You joined a boomfng 1on1 (-100 money)");
							pSelf->m_apPlayers[MateId]->m_IsInstaArena_fng = true;
						}

						pSelf->m_apPlayers[MateId]->m_IsInstaArena_gdm = true;
						pSelf->m_apPlayers[MateId]->m_Insta1on1_score = 0;
						pSelf->m_apPlayers[MateId]->MoneyTransaction(-100, "join insta 1on1");
						pSelf->m_apPlayers[MateId]->GetCharacter()->Die(MateId, WEAPON_SELF);

						pPlayer->m_IsInstaArena_gdm = true;
						pPlayer->m_Insta1on1_score = 0;
						pPlayer->m_Insta1on1_id = MateId;
						pPlayer->MoneyTransaction(-100, "join insta 1on1");
						pChr->Die(pPlayer->GetCid(), WEAPON_SELF);
					}
				}
				else //rifle
				{
					if(!pSelf->CanJoinInstaArena(false, true))
					{
						pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Arena is too full to start 1on1.");
					}
					else //everything succeeded! yay --> start 1on1
					{
						if(pSelf->m_apPlayers[MateId]->m_Insta1on1_mode == 1)
						{
							pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You joined a idm 1on1 (-100 money)");
							pSelf->SendChatTarget(MateId, "[INSTA] You joined a idm 1on1 (-100 money)");
						}
						else
						{
							pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] You joined a fng 1on1 (-100 money)");
							pPlayer->m_IsInstaArena_fng = true;

							pSelf->SendChatTarget(MateId, "[INSTA] You joined a fng 1on1 (-100 money)");
							pSelf->m_apPlayers[MateId]->m_IsInstaArena_fng = true;
						}

						pSelf->m_apPlayers[MateId]->m_IsInstaArena_idm = true;
						pSelf->m_apPlayers[MateId]->m_Insta1on1_score = 0;
						pSelf->m_apPlayers[MateId]->MoneyTransaction(-100, "join insta 1on1");
						pSelf->m_apPlayers[MateId]->GetCharacter()->Die(MateId, WEAPON_SELF);

						pPlayer->m_IsInstaArena_idm = true;
						pPlayer->m_Insta1on1_score = 0;
						pPlayer->m_Insta1on1_id = MateId;
						pPlayer->MoneyTransaction(-100, "join insta 1on1");
						pChr->Die(pPlayer->GetCid(), WEAPON_SELF);
					}
				}
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Unknown 1on1 parameter. Check '/insta 1on1 help' for more help");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[INSTA] Unknown parameter. Check '/insta cmdlist' for all commands");
	}
}

void CGameContext::ConJoin(IConsole::IResult *pResult, void *pUserData) //this command joins the currently running event... for now only Block tournaments
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[JOIN] you have to be alive to use this command");
		return;
	}

	/***********************************
	 *                                  *
	 *          BLOCK TOURNAMENT        *
	 *                                  *
	 ************************************/

	if(!g_Config.m_SvAllowBlockTourna)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, pSelf->Loc("[JOIN] Block tournaments are deactivated by an admin", pResult->m_ClientId));
		return;
	}
	if(pPlayer->m_IsBlockTourning)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, pSelf->Loc("[JOIN] You already joined the block tournament", pResult->m_ClientId));
		return;
	}

	const NETADDR *pOwnAddr = pSelf->Server()->ClientAddr(pResult->m_ClientId);
	int NumJoins = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!pSelf->m_apPlayers[i])
			continue;
		if(i == pResult->m_ClientId)
			continue;
		if(!pSelf->m_apPlayers[i]->m_IsBlockTourning)
			continue;

		const NETADDR *pAddr = pSelf->Server()->ClientAddr(i);
		if(!net_addr_comp_noport(pAddr, pOwnAddr))
		{
			NumJoins++;
		}
	}
	if(NumJoins >= g_Config.m_SvBlockTournaMaxPerIp)
	{
		pSelf->SendChatLoc(pResult->m_ClientId, "[JOIN] Only %d players per ip are allowed to join", g_Config.m_SvBlockTournaMaxPerIp);
		return;
	}

	if(pSelf->GetDDRaceTeam(pResult->m_ClientId) != 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[JOIN] You can not join while being in a ddrace team. Do '/team 0' first.");
		return;
	}

	if(pSelf->IsMinigame(pResult->m_ClientId))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[JOIN] This command is not allowed in jail or minigames. try '/leave' first.");
		return;
	}
	else if(g_Config.m_SvAllowBlockTourna == 2 && !pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[JOIN] You have to be logged in to join block tournaments.");
		return;
	}
	else if(pSelf->m_pBlockTournament->State() == CBlockTournament::STATE_IN_GAME)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[JOIN] Block tournament is already running please wait until its finished.");
		return;
	}
	else if(pSelf->m_pBlockTournament->State() == CBlockTournament::STATE_OFF)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, pSelf->Loc("[JOIN] No block tournament running", pResult->m_ClientId));
		return;

		//pSelf->SendChatTarget(pResult->m_ClientId, "[JOIN] you started a block tournament.");
		//pPlayer->m_IsBlockTourning = true;
		//pSelf->m_pBlockTournament->State() = 1;
		//pSelf->m_BlockTournaLobbyTick = g_Config.m_SvBlockTournaDelay * pSelf->Server()->TickSpeed();
		//return;
	}
	else if(pSelf->m_pBlockTournament->State() == CBlockTournament::STATE_LOBBY)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, pSelf->Loc("[JOIN] You joined a block tournament", pResult->m_ClientId));
		pSelf->m_pBlockTournament->Join(pPlayer);
		return;
	}
}

void CGameContext::ConBlock(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[BLOCK] you have to be alive to use this command");
		return;
	}

	if(pPlayer->m_IsBlockDeathmatch)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[BLOCK] you are already in deathmatch mode.");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[BLOCK] you joined the deathmatch arena!");
		pSelf->SendChatTarget(pResult->m_ClientId, "[BLOCK] type /leave to leave");
		pChr->Die(pPlayer->GetCid(), WEAPON_SELF);
		pPlayer->m_IsBlockDeathmatch = true;
	}
}

void CGameContext::ConPvpArena(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[PVP] You have to be alive to use this command.");
		return;
	}

	if(pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[PVP] Invalid. Type '/pvp_arena <join/leave>'.");
		return;
	}

	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if(!str_comp_nocase(aInput, "join"))
	{
		pSelf->m_pPvpArena->Join(pPlayer);
	}
	else if(!str_comp_nocase(aInput, "leave"))
	{
		pSelf->m_pPvpArena->Leave(pPlayer);
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[PVP] Invalid. Type '/pvp_arena <join/leave>'.");
	}
}

void CGameContext::ConMoney(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	char aBuf[256];

	pSelf->SendChatTarget(pResult->m_ClientId, "~~~~~~~~~~");
	str_format(aBuf, sizeof(aBuf), "Money: %" PRId64, pPlayer->GetMoney());
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	pSelf->SendChatTarget(pResult->m_ClientId, "~~~~~~~~~~");
	pSelf->SendChatTarget(pResult->m_ClientId, pPlayer->m_money_transaction0);
	pSelf->SendChatTarget(pResult->m_ClientId, pPlayer->m_money_transaction1);
	pSelf->SendChatTarget(pResult->m_ClientId, pPlayer->m_money_transaction2);
	pSelf->SendChatTarget(pResult->m_ClientId, pPlayer->m_money_transaction3);
	pSelf->SendChatTarget(pResult->m_ClientId, pPlayer->m_money_transaction4);
	pSelf->SendChatTarget(pResult->m_ClientId, pPlayer->m_money_transaction5);
	pSelf->SendChatTarget(pResult->m_ClientId, pPlayer->m_money_transaction6);
	pSelf->SendChatTarget(pResult->m_ClientId, pPlayer->m_money_transaction7);
	pSelf->SendChatTarget(pResult->m_ClientId, pPlayer->m_money_transaction8);
	pSelf->SendChatTarget(pResult->m_ClientId, pPlayer->m_money_transaction9);
	str_format(aBuf, sizeof(aBuf), "+%d (moneytiles)", pPlayer->m_MoneyTilesMoney);
	if(pPlayer->m_MoneyTilesMoney > 0)
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	pSelf->SendChatTarget(pResult->m_ClientId, "~~~~~~~~~~");
}

void CGameContext::ConPay(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	if(pResult->NumArguments() != 2)
	{
		pSelf->SendChatLoc(pResult->m_ClientId, "Use '/pay <amount> <player>' to send money to other players'");
		return;
	}

	char aBuf[512];
	int Amount;
	char aUsername[32];
	Amount = pResult->GetInteger(0);
	str_copy(aUsername, pResult->GetString(1), sizeof(aUsername));
	int PayId = pSelf->GetCidByName(aUsername);

	// COULD DO:
	// add a blocker to pay money to ur self... but me funny mede it pozzible

	if(Amount > pPlayer->GetMoney())
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "You don't have enough money.");
		return;
	}

	if(Amount < 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Are you trying to steal money?");
		return;
	}

	if(Amount == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "You paid nothing.");
		return;
	}

	if(PayId == -1)
	{
		str_format(aBuf, sizeof(aBuf), "Can't find playername: '%s'.", aUsername);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	}
	else
	{
		if(!pSelf->m_apPlayers[PayId]->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "ERROR: This player is not logged in. More info: '/accountinfo'.");
			return;
		}

		//player give
		str_format(aBuf, sizeof(aBuf), "You paid %d money to the player '%s'", Amount, aUsername);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		str_format(aBuf, sizeof(aBuf), "paid to '%s'", aUsername);
		pPlayer->MoneyTransaction(-Amount, aBuf);
		//dbg_msg("pay", "survived give"); //survives

		//player get
		str_format(aBuf, sizeof(aBuf), "'%s' paid you %d money", pSelf->Server()->ClientName(pResult->m_ClientId), Amount);
		pSelf->SendChatTarget(PayId, aBuf);
		str_format(aBuf, sizeof(aBuf), "paid by '%s'", pSelf->Server()->ClientName(pResult->m_ClientId));
		pSelf->m_apPlayers[PayId]->MoneyTransaction(Amount, aBuf);
		dbg_msg("pay", "survived get");
	}
}

void CGameContext::ConGift(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	char aBuf[256];

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "**** GIFT INFO ****");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/gift <player>' to send someone 150 money.");
		pSelf->SendChatTarget(pResult->m_ClientId, "You don't lose this money. It is coming from the server.");
		pSelf->SendChatTarget(pResult->m_ClientId, "**** GIFT STATUS ****");
		if(pPlayer->m_Account.m_GiftDelay)
		{
			str_format(aBuf, sizeof(aBuf), "[GIFT] You can send gifts in %d seconds.", pPlayer->m_Account.m_GiftDelay / pSelf->Server()->TickSpeed());
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[GIFT] You can send gifts.");
		}
		return;
	}

	int GiftId = pSelf->GetCidByName(pResult->GetString(0));

	if(GiftId == -1)
	{
		str_format(aBuf, sizeof(aBuf), "[GIFT] '%s' is not online.", pResult->GetString(0));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		return;
	}
	if(pPlayer->m_Account.m_GiftDelay)
	{
		str_format(aBuf, sizeof(aBuf), "[GIFT] You can send gifts in %d seconds.", pPlayer->m_Account.m_GiftDelay / pSelf->Server()->TickSpeed());
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		return;
	}
	if(pPlayer->GetLevel() < 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[GIFT] You have to be at least lvl 1 to use gifts.");
		return;
	}

	if(pSelf->m_apPlayers[GiftId])
	{
		const NETADDR *pOwnAddr = pSelf->Server()->ClientAddr(pResult->m_ClientId);
		const NETADDR *pReceiverAddr = pSelf->Server()->ClientAddr(GiftId);

		if(!net_addr_comp_noport(pOwnAddr, pReceiverAddr))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[GIFT] You can't give money to your dummy.");
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "gift (%s)", pSelf->Server()->ClientName(pResult->m_ClientId));
			pSelf->m_apPlayers[GiftId]->MoneyTransaction(+150, aBuf);
			str_format(aBuf, sizeof(aBuf), "[GIFT] You gave '%s' 150 money!", pSelf->Server()->ClientName(GiftId));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

			str_format(aBuf, sizeof(aBuf), "[GIFT] '%s' has gifted you 150 money. (more info: '/gift')", pSelf->Server()->ClientName(pResult->m_ClientId));
			pSelf->SendChatTarget(GiftId, aBuf);

			pSelf->m_apPlayers[pResult->m_ClientId]->m_Account.m_GiftDelay = pSelf->Server()->TickSpeed() * 180; //180 seconds == 3 minutes
		}
	}
}

void CGameContext::ConEvent(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	bool IsEvent = false;

	pSelf->SendChatTarget(pResult->m_ClientId, "###########################");
	if(g_Config.m_SvFinishEvent == 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "~~~ Race Event ~~~");
		pSelf->SendChatTarget(pResult->m_ClientId, "Info: You get more xp for finishing the map!");
		IsEvent = true;
	}
	if(pSelf->m_pBlockTournament->State())
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "~~~ Block Event ~~~");
		pSelf->SendChatTarget(pResult->m_ClientId, "Info: last ma standing fight in a block tournament use '/join' to join");
		IsEvent = true;
	}

	if(!IsEvent)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "No events running at the moment...");
	}

	pSelf->SendChatTarget(pResult->m_ClientId, "###########################");
}

// accept/turn-off cosmetic features

void CGameContext::ConRainbow(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	if(pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Invalid. Type '/rainbow <accept/off>'.");
		return;
	}

	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if(!str_comp_nocase(aInput, "off"))
	{
		pPlayer->GetCharacter()->m_Rainbow = false;
		pPlayer->m_InfRainbow = false;
		pSelf->SendChatTarget(pResult->m_ClientId, "Rainbow turned off.");
	}
	else if(!str_comp_nocase(aInput, "accept"))
	{
		if(pPlayer->m_rainbow_offer > 0)
		{
			if(!pPlayer->GetCharacter()->m_Rainbow)
			{
				pPlayer->GetCharacter()->m_Rainbow = true;
				pPlayer->m_rainbow_offer--;
				pSelf->SendChatTarget(pResult->m_ClientId, "You accepted rainbow. You can turn it off with '/rainbow off'.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "You already have rainbow.");
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Nobody offered you rainbow.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Invalid. Type '/rainbow <accept/off>'.");
	}
}

void CGameContext::ConBloody(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	if(pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Invalid. Type '/bloody <accept/off>'.");
		return;
	}

	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if(!str_comp_nocase(aInput, "off"))
	{
		pPlayer->GetCharacter()->m_Bloody = false;
		pPlayer->GetCharacter()->m_StrongBloody = false;
		pPlayer->m_InfBloody = false;
		pSelf->SendChatTarget(pResult->m_ClientId, "Bloody turned off.");
	}
	else if(!str_comp_nocase(aInput, "accept"))
	{
		if(pPlayer->m_bloody_offer > 0)
		{
			if(!pPlayer->GetCharacter()->m_Bloody)
			{
				pPlayer->GetCharacter()->m_Bloody = true;
				pPlayer->m_bloody_offer--;
				pSelf->SendChatTarget(pResult->m_ClientId, "You accepted bloody. You can turn it off with '/bloody off'.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "You already have bloody.");
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Nobody offered you bloody.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Invalid. Type '/bloody <accept/off>'.");
	}
}

void CGameContext::ConAtom(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	if(pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Invalid. Type '/atom <accept/off>'.");
		return;
	}

	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if(!str_comp_nocase(aInput, "off"))
	{
		pPlayer->GetCharacter()->m_Atom = false;
		pPlayer->m_InfAtom = false;
		pSelf->SendChatTarget(pResult->m_ClientId, "Atom turned off.");
	}
	else if(!str_comp_nocase(aInput, "accept"))
	{
		if(pPlayer->m_atom_offer > 0)
		{
			if(!pPlayer->GetCharacter()->m_Atom)
			{
				pPlayer->GetCharacter()->m_Atom = true;
				pPlayer->m_atom_offer--;
				pSelf->SendChatTarget(pResult->m_ClientId, "You accepted atom. You can turn it off with '/atom off'.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "You already have atom.");
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Nobody offered you atom.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Invalid. Type '/atom <accept/off>'.");
	}
}

void CGameContext::ConAutoSpreadGun(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	if(pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Invalid. Type '/spread_gun <accept/off>'.");
		return;
	}

	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if(!str_comp_nocase(aInput, "off"))
	{
		pPlayer->GetCharacter()->m_autospreadgun = false;
		pPlayer->m_InfAutoSpreadGun = false;
		pSelf->SendChatTarget(pResult->m_ClientId, "Spread gun turned off.");
	}
	else if(!str_comp_nocase(aInput, "accept"))
	{
		if(pPlayer->m_autospreadgun_offer > 0)
		{
			if(!pPlayer->GetCharacter()->m_autospreadgun)
			{
				pPlayer->GetCharacter()->m_autospreadgun = true;
				pPlayer->m_autospreadgun_offer--;
				pSelf->SendChatTarget(pResult->m_ClientId, "You accepted spread gun. You can turn it off with '/spread_gun off'.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "You already have spread gun.");
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Nobody offered you spread gun.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Invalid. Type '/spread_gun <accept/off>'.");
	}
}

void CGameContext::ConDropHealth(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	if(!pPlayer->m_Account.m_IsSuperModerator && pSelf->Server()->GetAuthedState(pResult->m_ClientId) != AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[DROP] Missing permission.");
		return;
	}
	if(pPlayer->m_IsBlockTourning)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[DROP] you can't use that command during block tournaments.");
		return;
	}
	else if(pPlayer->m_IsSurvivaling && pSelf->m_survivalgamestate != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[DROP] you can't use that command during survival games. (only in lobby)");
		return;
	}
	else if(pSelf->m_survivalgamestate > 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[DROP] you can't use that command while a survival game is running.");
		return;
	}

	int amount = 1;
	if(pResult->NumArguments() > 0)
		amount = pResult->GetInteger(0);
	pChr->DropHealth(amount);
}

void CGameContext::ConDropArmor(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	if(!pPlayer->m_Account.m_IsSuperModerator && pSelf->Server()->GetAuthedState(pResult->m_ClientId) != AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[DROP] Missing permission.");
		return;
	}
	if(pPlayer->m_IsBlockTourning)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[DROP] you can't use that command during block tournaments.");
		return;
	}
	else if(pPlayer->m_IsSurvivaling && pSelf->m_survivalgamestate != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[DROP] you can't use that command during survival games. (only in lobby)");
		return;
	}
	else if(pSelf->m_survivalgamestate > 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[DROP] you can't use that command while a survival game is running.");
		return;
	}

	int amount = 1;
	if(pResult->NumArguments() > 0)
		amount = pResult->GetInteger(0);
	pChr->DropArmor(amount);
}

void CGameContext::ConTrail(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	if(pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Invalid. Type '/trail <accept/off>'.");
		return;
	}

	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if(!str_comp_nocase(aInput, "off"))
	{
		pPlayer->GetCharacter()->m_Trail = false;
		pPlayer->m_InfTrail = false;
		pSelf->SendChatTarget(pResult->m_ClientId, "Trail turned off.");
	}
	else if(!str_comp_nocase(aInput, "accept"))
	{
		if(pPlayer->m_trail_offer > 0)
		{
			if(!pPlayer->GetCharacter()->m_Trail)
			{
				pPlayer->GetCharacter()->m_Trail = true;
				pPlayer->m_trail_offer--;
				pSelf->SendChatTarget(pResult->m_ClientId, "You accepted trail. You can turn it off with '/trail off'.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "You already have trail.");
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Nobody offered you trail.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Invalid. Type '/trail <accept/off>'.");
	}
}

void CGameContext::ConAccountInfo(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "~~~ %s ~~~", pSelf->Loc("Account Info", pResult->m_ClientId));
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	pSelf->SendChatTarget(pResult->m_ClientId, "/register <name> <password> <password>");
	pSelf->SendChatTarget(pResult->m_ClientId, "/login <name> <password>");
	pSelf->SendChatTarget(pResult->m_ClientId, "/logout");
	pSelf->SendChatTarget(pResult->m_ClientId, "/changepassword");
	pSelf->SendChatTarget(pResult->m_ClientId, "-------------------");
	pSelf->SendChatLoc(pResult->m_ClientId, "Accounts are used to save your stats on this server.");
}

void CGameContext::ConPoliceInfo(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;

	int page = pResult->GetInteger(0); //no parameter -> 0 -> page 1
	if(!page)
	{
		page = 1;
	}
	int pages = 4;
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "-- page %d/%d --", page, pages);

	if(page == 1)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"~~~ Police Info ~~~");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"[GENERAL INFORMATION]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Police can be bought in shop using '/buy police'.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"There are multiple police ranks, each cost 100 000 money.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"The policebot will help every police officer.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Every police rank will give you more benefits.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Use '/policeinfo <page>' to check out what other police ranks can do.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			aBuf);
	}
	else if(page == 2)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"~~~ Police Info ~~~");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"[POLICE 1]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Level needed to buy: [LVL 18]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Benefits:");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"- '/policechat'");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Use '/policeinfo <page>' to check out what other police ranks can do.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			aBuf);
	}
	else if(page == 3)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"~~~ Police Info ~~~");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"[POLICE 2]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Level needed to buy: [LVL 25]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Benefits:");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"- full access to '/jail' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"- '/policehelper'");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Use '/policeinfo <page>' to check out what other police ranks can do.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			aBuf);
	}
	else if(page == 4)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"~~~ Police Info ~~~");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"[POLICE 3]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Level needed to buy: [LVL 30]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Benefits:");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"- taser license ('/taser')");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Use '/policeinfo <page>' to check out what other police ranks can do.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			aBuf);
	}
	/*else if (page == 5)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"~~~ Police Info ~~~");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"[POLICE 4]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Level needed to buy: [LVL 40]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Benefits:");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"- PASTE FEATURES HERE");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Use '/policeinfo <page>' to check out what other police ranks can do.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			aBuf);
	}
	else if (page == 6)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"~~~ Police Info ~~~");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"[POLICE 5]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Level needed to buy: [LVL 50]");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Benefits:");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"- PASTE FEATURES HERE");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Use '/policeinfo <page>' to check out what other police ranks can do.");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			aBuf);
	}*/
	else
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp",
			"Unknown page.");
	}
}

//void CGameContext::ConProfileInfo(IConsole::IResult *pResult, void *pUserData) //old
//{
//#if defined(CONF_DEBUG)
//#endif
//	CGameContext *pSelf = (CGameContext *)pUserData;
//	if (!CheckClientId(pResult->m_ClientId))
//		return;
//
//	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
//	if (!pPlayer)
//		return;
//
//	CCharacter* pChr = pPlayer->GetCharacter();
//	if (!pChr)
//		return;
//
//
//	pSelf->SendChatTarget(pResult->m_ClientId, "~~~ Profile Info ~~~");
//	pSelf->SendChatTarget(pResult->m_ClientId, " ");
//	pSelf->SendChatTarget(pResult->m_ClientId, "VIEW PROFILES:");
//	pSelf->SendChatTarget(pResult->m_ClientId, "/profile view (playername)");
//	pSelf->SendChatTarget(pResult->m_ClientId, "Info: The player needs to be on the server and logged in");
//	pSelf->SendChatTarget(pResult->m_ClientId, " ");
//	pSelf->SendChatTarget(pResult->m_ClientId, "PROFILE SETTINGS:");
//	pSelf->SendChatTarget(pResult->m_ClientId, "/profile_style (style) - changes your profile style");
//	pSelf->SendChatTarget(pResult->m_ClientId, "/profile_status (status) - changes your status");
//	pSelf->SendChatTarget(pResult->m_ClientId, "/profile_skype (skype) - changes your skype");
//	pSelf->SendChatTarget(pResult->m_ClientId, "/profile_youtube (youtube) - changes your youtube");
//	pSelf->SendChatTarget(pResult->m_ClientId, "/profile_email (email) - changes your email");
//	pSelf->SendChatTarget(pResult->m_ClientId, "/profile_homepage (homepage) - changes your homepage");
//	pSelf->SendChatTarget(pResult->m_ClientId, "/profile_twitter (twitter) - changes your twitter");
//}

void CGameContext::ConTCMD3000(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[TCMD3000] you have to be alive to do tests");
	}

	char aBuf[128];

	str_format(aBuf, sizeof(aBuf), "Cucumber value: %d", pSelf->m_CucumberShareValue);
	//pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

	if(g_Config.m_SvTestingCommands)
	{
		// pPlayer->m_QuestState = pResult->GetInteger(0);
		// pPlayer->m_QuestStateLevel = pResult->GetInteger(1);
		// pSelf->StartQuest(pPlayer->GetCid());
		/*
	pSelf->m_MissionUnlockedLevel = pResult->GetInteger(0);
	pSelf->m_MissionCurrentLevel = pResult->GetInteger(1);
	str_format(aBuf, sizeof(aBuf), "updated level unlocked=%d current=%d", pSelf->m_MissionUnlockedLevel, pSelf->m_MissionCurrentLevel);
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	pSelf->SaveSinglePlayer();

		pSelf->m_BlockWaveRound = pResult->GetInteger(0);
		str_format(aBuf, sizeof(aBuf), "set blockwave level %d", pSelf->m_BlockWaveRound);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		pSelf->BlockWaveWonRound();
		*/

		/*
		if (pResult->NumArguments() != 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Quest cheater: /tcmd3000 <quest> <level>");
		}

		*/

		//pSelf->ChillUpdateFileAcc(pResult->GetString(0), pResult->GetInteger(1), pResult->GetString(2), pResult->m_ClientId); //a fully working set all values of acc2 files but its a bit op maybe add it to the rcon api but not as normal admin cmd
	}
}

void CGameContext::ConAntiFlood(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) != AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[FLOOD] Missing permission.");
		return;
	}

	pSelf->AbuseMotd(
		"*~~~~* ANTI FLOOD configs *~~~~*\n\n\
\
sv_spamfilter_mode, sv_captcha_room\n\
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
register_ban, login_ban, reload_censorlist, \n\
add_spamfilter, reload_spamfilters\
",
		pResult->m_ClientId);
}

void CGameContext::ConStockMarket(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	char aBuf[256];
	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if(!str_comp_nocase(aInput, "buy"))
	{
		if(pPlayer->GetMoney() < pSelf->m_CucumberShareValue)
		{
			str_format(aBuf, sizeof(aBuf), "You don't have enough money. You need %d money.", pSelf->m_CucumberShareValue);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "Purchased cucumber for %d money.", pSelf->m_CucumberShareValue);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pPlayer->m_StockMarket_item_Cucumbers++;
			pPlayer->MoneyTransaction(-pSelf->m_CucumberShareValue, "bought 'cucumber stock'");

			pSelf->m_CucumberShareValue++; // push the gernerall share value
		}
	}
	else if(!str_comp_nocase(aInput, "sell"))
	{
		if(pPlayer->m_StockMarket_item_Cucumbers > 0)
		{
			str_format(aBuf, sizeof(aBuf), "Sold cucumber for %d money.", pSelf->m_CucumberShareValue);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pPlayer->m_StockMarket_item_Cucumbers--;
			pPlayer->MoneyTransaction(+pSelf->m_CucumberShareValue, "sold a 'cucumber stock'");

			pSelf->m_CucumberShareValue--; // pull the gernerall share value
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You dont have this stock");
		}
	}
	else if(!str_comp_nocase(aInput, "info"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "==== PUBLIC STOCK MARKET ====");
		str_format(aBuf, sizeof(aBuf), "Cucumbers %d money", pSelf->m_CucumberShareValue);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		pSelf->SendChatTarget(pResult->m_ClientId, "==== PERSONAL STATS ====");
		str_format(aBuf, sizeof(aBuf), "Cucumbers %d", pPlayer->m_StockMarket_item_Cucumbers);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Invalid. Type '/market <buy/sell/info>'.");
	}
}

void CGameContext::ConCaptcha(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;
	CCaptcha *pCap = pPlayer->m_pCaptcha;
	if(!pCap)
		return;

	pCap->Prompt(pResult->GetString(0));
}

void CGameContext::ConLang(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;
	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;
	pPlayer->SetLanguage(pResult->GetString(0));
	pSelf->RefreshExtraVoteMenu(pPlayer->GetCid());
}

void CGameContext::ConHumanLevel(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "[==== Human Level %d/9 ====]", pPlayer->m_PlayerHumanLevel);
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	pSelf->SendChatTarget(pResult->m_ClientId, "\
		The server tries to detect if you are a active human or a robot.\
		Some things might be blocked if your human level is too low.\
	");
	pSelf->SendChatTarget(pResult->m_ClientId, "Play active use the chat and do quests ('/quest').");
	pSelf->SendChatTarget(pResult->m_ClientId, "Block others, login to your account and use the '/captcha' command.");
}

void CGameContext::ConPoop(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	if(pResult->NumArguments() != 2)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Throw shit at other players. Warning: You lose what you use.");
		pSelf->SendChatTarget(pResult->m_ClientId, "Use '/poop <amount> <player>'.");
		return;
	}

	char aBuf[512];
	int Amount;
	char aUsername[32];
	Amount = pResult->GetInteger(0);
	str_copy(aUsername, pResult->GetString(1), sizeof(aUsername));
	int PoopId = pSelf->GetCidByName(aUsername);

	if(Amount > pPlayer->m_Account.m_Shit)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "You don't have enough shit.");
		return;
	}

	if(Amount < 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "You can't poop negative?! Imagine some1 is trying to push shit back in ur anus... wtf.");
		return;
	}
	if(Amount == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, ">ou don't have shit.");
		return;
	}

	if(PoopId == -1)
	{
		str_format(aBuf, sizeof(aBuf), "Can't find user with the name: '%s'.", aUsername);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	}
	else
	{
		if(!pSelf->m_apPlayers[PoopId]->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "ERROR: This player is not logged in. More info '/accountinfo'.");
			return;
		}

		//player give
		str_format(aBuf, sizeof(aBuf), "You pooped %s %d times xd", aUsername, Amount);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		str_format(aBuf, sizeof(aBuf), "you lost %d shit ._.", Amount);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		pPlayer->m_Account.m_Shit -= Amount;

		//player get
		if(g_Config.m_SvPoopMSG == 1) //normal
		{
			str_format(aBuf, sizeof(aBuf), "'%s' threw %d shit at you o.O", pSelf->Server()->ClientName(pResult->m_ClientId), Amount);
			pSelf->SendChatTarget(PoopId, aBuf);
		}
		else if(g_Config.m_SvPoopMSG == 2) //extreme
		{
			for(int i = 0; i < Amount; i++)
			{
				str_format(aBuf, sizeof(aBuf), "'%s' threw shit at you o.O", pSelf->Server()->ClientName(pResult->m_ClientId));
				pSelf->SendChatTarget(PoopId, aBuf);

				if(i > 30) //poop blocker o.O 30 lines of poop is the whole chat. Poor server has enough
				{
					str_format(aBuf, sizeof(aBuf), "'%s' threw %d shit at you o.O", pSelf->Server()->ClientName(pResult->m_ClientId), Amount); //because it was more than the chatwindow can show inform the user how much poop it was
					pSelf->SendChatTarget(PoopId, aBuf);
					break;
				}
			}
		}
		pSelf->m_apPlayers[PoopId]->m_Account.m_Shit += Amount;
	}
}

void CGameContext::ConGive(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] You have to be alive to use this command.");
		return;
	}

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "~~~ GIVE COMMAND ~~~");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/give <extra>' to get it for yourself.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/give <extra> <playername>' to give to others.");
		pSelf->SendChatTarget(pResult->m_ClientId, "-- EXTRAS --");
		pSelf->SendChatTarget(pResult->m_ClientId, "rainbow, bloody, strong_bloody, trail, atom, spread_gun");
		return;
	}
	else if(pPlayer->m_IsBlockTourning)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] you can't use that command during block tournaments.");
		return;
	}
	else if(pPlayer->m_IsSurvivaling && pSelf->m_survivalgamestate != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] you can't use that command during survival games. (only in lobby)");
		return;
	}

	char aBuf[512];

	//Ranks sorted DESC by power
	//---> the highest rank gets triggered

	//the ASC problem is if a SuperModerator is also rcon_mod he only has rcon_mod powerZ

	// COULD DO:
	// Im unsure to check if GiveId is logged in.
	// Pros:
	// - moderators can make random players happy and they dont have to spend time to login
	// Cons:
	// - missing motivation to create an account

	if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) == AUTHED_ADMIN)
	{
		if(pResult->NumArguments() == 1) //only item no player --> give it ur self
		{
			char aItem[64];
			str_copy(aItem, pResult->GetString(0), sizeof(aItem));
			if(!str_comp_nocase(aItem, "bloody"))
			{
				pPlayer->GetCharacter()->m_Bloody = true;
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Bloody on.");
			}
			else if(!str_comp_nocase(aItem, "strong_bloody"))
			{
				pPlayer->GetCharacter()->m_StrongBloody = true;
				pPlayer->GetCharacter()->m_Bloody = false;
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Strong bloody on.");
			}
			else if(!str_comp_nocase(aItem, "rainbow"))
			{
				pPlayer->GetCharacter()->m_Rainbow = true;
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Rainbow on.");
			}
			else if(!str_comp_nocase(aItem, "trail"))
			{
				pPlayer->GetCharacter()->m_Trail = true;
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Trail on.");
			}
			else if(!str_comp_nocase(aItem, "atom"))
			{
				pPlayer->GetCharacter()->m_Atom = true;
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Atom on.");
			}
			else if(!str_comp_nocase(aItem, "spread_gun"))
			{
				pChr->m_autospreadgun = true;
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Spread gun on.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Unknown item.");
			}
		}
		else if(pResult->NumArguments() == 2) //give to other players
		{
			char aItem[64];
			char aUsername[32];
			str_copy(aItem, pResult->GetString(0), sizeof(aItem));
			str_copy(aUsername, pResult->GetString(1), sizeof(aUsername));

			int GiveId = pSelf->GetCidByName(aUsername);

			if(GiveId != -1)
			{
				if(!str_comp_nocase(aItem, "bloody"))
				{
					if(pSelf->m_apPlayers[GiveId]->m_bloody_offer > 4)
					{
						pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Admins can't offer bloody to the same player more than 5 times.");
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "[GIVE] Bloody offer sent to '%s'.", aUsername);
						pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

						pSelf->m_apPlayers[GiveId]->m_bloody_offer++;
						str_format(aBuf, sizeof(aBuf), "[GIVE] Bloody was offered to you by '%s'. Turn it on using '/bloody accept'.", pSelf->Server()->ClientName(pResult->m_ClientId));
						pSelf->SendChatTarget(pSelf->m_apPlayers[GiveId]->GetCid(), aBuf);
					}
				}
				else if(!str_comp_nocase(aItem, "strong_bloody"))
				{
					pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission.");
				}
				else if(!str_comp_nocase(aItem, "rainbow"))
				{
					if(pSelf->m_apPlayers[GiveId]->m_rainbow_offer > 19)
					{
						pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Admins can't offer rainbow to the same player more than 20 times.");
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "[GIVE] Rainbow offer sent to '%s'.", aUsername);
						pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

						pSelf->m_apPlayers[GiveId]->m_rainbow_offer++;
						str_format(aBuf, sizeof(aBuf), "[GIVE] Rainbow was offered to you by '%s'. Turn it on using '/rainbow accept'.", pSelf->Server()->ClientName(pResult->m_ClientId));
						pSelf->SendChatTarget(pSelf->m_apPlayers[GiveId]->GetCid(), aBuf);
					}
				}
				else if(!str_comp_nocase(aItem, "trail"))
				{
					if(pSelf->m_apPlayers[GiveId]->m_trail_offer > 9)
					{
						pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Admins can't offer trail to the same player more than 10 times.");
						return;
					}

					str_format(aBuf, sizeof(aBuf), "Trail offer sent to '%s'.", aUsername);
					pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

					pSelf->m_apPlayers[GiveId]->m_trail_offer++;
					str_format(aBuf, sizeof(aBuf), "[GIVE] Trail was offered to you by '%s'. Turn it on using '/trail accept'.", pSelf->Server()->ClientName(pResult->m_ClientId));
					pSelf->SendChatTarget(pSelf->m_apPlayers[GiveId]->GetCid(), aBuf);
				}
				else if(!str_comp_nocase(aItem, "atom"))
				{
					if(pSelf->m_apPlayers[GiveId]->m_atom_offer > 9)
					{
						pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Admins can't offer atom to the same player more than 10 times.");
						return;
					}

					str_format(aBuf, sizeof(aBuf), "[GIVE] Atom offer sent to '%s'.", aUsername);
					pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

					pSelf->m_apPlayers[GiveId]->m_atom_offer++;
					str_format(aBuf, sizeof(aBuf), "[GIVE] Atom was offered to you by '%s'. Turn it on using '/atom accept'.", pSelf->Server()->ClientName(pResult->m_ClientId));
					pSelf->SendChatTarget(pSelf->m_apPlayers[GiveId]->GetCid(), aBuf);
				}
				else if(!str_comp_nocase(aItem, "spread_gun"))
				{
					if(pSelf->m_apPlayers[GiveId]->m_autospreadgun_offer > 9)
					{
						pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Admins can't offer spread gun to the same player more than 10 times.");
						return;
					}

					str_format(aBuf, sizeof(aBuf), "[GIVE] Spread gun offer sent to '%s'.", aUsername);
					pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

					pSelf->m_apPlayers[GiveId]->m_autospreadgun_offer++;
					str_format(aBuf, sizeof(aBuf), "[GIVE] Spread gun was offered to you by '%s'. Turn it on using '/spread_gun accept'.", pSelf->Server()->ClientName(pResult->m_ClientId));
					pSelf->SendChatTarget(pSelf->m_apPlayers[GiveId]->GetCid(), aBuf);
				}
				else
				{
					pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Unknown item.");
				}
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "[GIVE] Can't find playername: '%s'.", aUsername);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			}
		}
	}
	else if(pPlayer->m_Account.m_IsSuperModerator)
	{
		if(pResult->NumArguments() == 1) //only item no player --> give it ur self
		{
			char aItem[64];
			str_copy(aItem, pResult->GetString(0), sizeof(aItem));
			if(!str_comp_nocase(aItem, "bloody"))
			{
				pPlayer->GetCharacter()->m_Bloody = true;
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Bloody on.");
			}
			else if(!str_comp_nocase(aItem, "strong_bloody"))
			{
				pPlayer->GetCharacter()->m_StrongBloody = true;
				pPlayer->GetCharacter()->m_Bloody = false;
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Strong bloody on.");
			}
			else if(!str_comp_nocase(aItem, "rainbow"))
			{
				pPlayer->GetCharacter()->m_Rainbow = true;
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Rainbow on.");
			}
			else if(!str_comp_nocase(aItem, "trail"))
			{
				pPlayer->GetCharacter()->m_Trail = true;
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Trail on.");
			}
			else if(!str_comp_nocase(aItem, "atom"))
			{
				pPlayer->GetCharacter()->m_Atom = true;
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Atom on.");
			}
			else if(!str_comp_nocase(aItem, "spread_gun"))
			{
				pChr->m_autospreadgun = true;
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Spread gun on.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Unknown item.");
			}
		}
		else if(pResult->NumArguments() == 2) //give to other players
		{
			char aItem[64];
			char aUsername[32];
			str_copy(aItem, pResult->GetString(0), sizeof(aItem));
			str_copy(aUsername, pResult->GetString(1), sizeof(aUsername));

			int GiveId = pSelf->GetCidByName(aUsername);

			if(GiveId != -1)
			{
				if(!str_comp_nocase(aItem, "bloody"))
				{
					if(pSelf->m_apPlayers[GiveId]->m_bloody_offer)
					{
						pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] VIP+ can't offer bloody to the same player more than once.");
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "[GIVE] Bloody offer sent to '%s'.", aUsername);
						pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

						pSelf->m_apPlayers[GiveId]->m_bloody_offer++;
						str_format(aBuf, sizeof(aBuf), "[GIVE] Bloody was offered to you by '%s'. Turn it on using '/bloody accept'.", pSelf->Server()->ClientName(pResult->m_ClientId));
						pSelf->SendChatTarget(pSelf->m_apPlayers[GiveId]->GetCid(), aBuf);
					}
				}
				else if(!str_comp_nocase(aItem, "strong_bloody"))
				{
					pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
				}
				else if(!str_comp_nocase(aItem, "rainbow"))
				{
					if(pSelf->m_apPlayers[GiveId]->m_rainbow_offer > 9)
					{
						pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] VIP+ can't offer rainbow to the same player more than 10 times.");
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "[GIVE] Rainbow offer sent to '%s'.", aUsername);
						pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

						pSelf->m_apPlayers[GiveId]->m_rainbow_offer++;
						str_format(aBuf, sizeof(aBuf), "[GIVE] Rainbow was offered to you by '%s'. Turn it on using '/rainbow accept'.", pSelf->Server()->ClientName(pResult->m_ClientId));
						pSelf->SendChatTarget(pSelf->m_apPlayers[GiveId]->GetCid(), aBuf);
					}
				}
				else if(!str_comp_nocase(aItem, "trail"))
				{
					pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
				}
				else if(!str_comp_nocase(aItem, "atom"))
				{
					pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
				}
				else if(!str_comp_nocase(aItem, "spread_gun"))
				{
					pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
				}
				else
				{
					pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Unknown item.");
				}
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "[GIVE] Can't find player '%s'.", aUsername);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			}
		}
	}
	else if(pPlayer->m_Account.m_IsModerator)
	{
		if(pResult->NumArguments() == 1) //only item no player --> give it ur self
		{
			char aItem[64];
			str_copy(aItem, pResult->GetString(0), sizeof(aItem));
			if(!str_comp_nocase(aItem, "bloody"))
			{
				pPlayer->GetCharacter()->m_Bloody = true;
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Bloody on.");
			}
			else if(!str_comp_nocase(aItem, "strong_bloody"))
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
			}
			else if(!str_comp_nocase(aItem, "rainbow"))
			{
				pPlayer->GetCharacter()->m_Rainbow = true;
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Rainbow on.");
			}
			else if(!str_comp_nocase(aItem, "trail"))
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
			}
			else if(!str_comp_nocase(aItem, "atom"))
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
			}
			else if(!str_comp_nocase(aItem, "spread_gun"))
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Unknown item.");
			}
		}
		else if(pResult->NumArguments() == 2) //give to other players
		{
			char aItem[64];
			char aUsername[32];
			str_copy(aItem, pResult->GetString(0), sizeof(aItem));
			str_copy(aUsername, pResult->GetString(1), sizeof(aUsername));

			int GiveId = pSelf->GetCidByName(aUsername);

			if(GiveId != -1)
			{
				if(!str_comp_nocase(aItem, "bloody"))
				{
					pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
				}
				else if(!str_comp_nocase(aItem, "strong_bloody"))
				{
					pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
				}
				else if(!str_comp_nocase(aItem, "rainbow"))
				{
					if(pSelf->m_apPlayers[GiveId]->m_rainbow_offer)
					{
						pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] VIPs can't offer rainbow to the same player more than once.");
					}
					else
					{
						pSelf->m_apPlayers[GiveId]->m_rainbow_offer++;
						str_format(aBuf, sizeof(aBuf), "[GIVE] Rainbow offer sent to player: '%s'.", aUsername);
						pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
					}
				}
				else if(!str_comp_nocase(aItem, "trail"))
				{
					pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
				}
				else if(!str_comp_nocase(aItem, "atom"))
				{
					pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
				}
				else if(!str_comp_nocase(aItem, "spread_gun"))
				{
					pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
				}
				else
				{
					pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Unknown item.");
				}
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "[GIVE] Can't find playername: '%s'.", aUsername);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			}
		}
	}
	else if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) == AUTHED_MOD)
	{
		char aItem[64];
		str_copy(aItem, pResult->GetString(0), sizeof(aItem));
		if(!str_comp_nocase(aItem, "bloody"))
		{
			pPlayer->GetCharacter()->m_Bloody = true;
			pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Bloody on.");
		}
		else if(!str_comp_nocase(aItem, "strong_bloody"))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
		}
		else if(!str_comp_nocase(aItem, "rainbow"))
		{
			pPlayer->GetCharacter()->m_Rainbow = true;
			pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Rainbow on.");
		}
		else if(!str_comp_nocase(aItem, "trail"))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
		}
		else if(!str_comp_nocase(aItem, "atom"))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
		}
		else if(!str_comp_nocase(aItem, "spread_gun"))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Unknown item.");
		}
	}
	else //no rank at all
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[GIVE] Missing permission.");
	}
}

void CGameContext::ConBomb(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	if(!g_Config.m_SvAllowBomb)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Admin deactivated bomb games.");
		return;
	}

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Missing parameter. Check '/bomb help' for more help.");
		return;
	}

	char aBuf[512];

	char aCmd[64];

	str_copy(aCmd, pResult->GetString(0), sizeof(aCmd));

	if(!str_comp_nocase(aCmd, "create"))
	{
		if(pResult->NumArguments() < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "ERROR: Missing money parameter ('/bomb create <money>').");
			return;
		}
		if(pSelf->m_BombGameState)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "ERROR: There is already a bomb game. You can join it with '/bomb join'.");
			return;
		}
		if(!pPlayer->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You need to be logged in to create a bomb game. More info at '/accountinfo'.");
			return;
		}
		if(pPlayer->m_Account.m_BombBanTime)
		{
			str_format(aBuf, sizeof(aBuf), "You are banned from bomb gamesfor %d second(s).", pPlayer->m_Account.m_BombBanTime / 60);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			return;
		}
		if(pSelf->IsMinigame(pResult->m_ClientId))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			return;
		}

		int BombMoney;
		BombMoney = pResult->GetInteger(1);

		if(BombMoney > pPlayer->GetMoney())
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "ERROR: You don't have enough money.");
			return;
		}
		if(BombMoney < 0)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "ERROR: Bomb reward has to be positive.");
			return;
		}

		if(pResult->NumArguments() > 2) //config map
		{
			char aConfig[32];
			str_copy(aConfig, pResult->GetString(2), sizeof(aConfig));

			if(!str_comp_nocase(aConfig, "NoArena"))
			{
				str_copy(pSelf->m_BombMap, "NoArena", sizeof(pSelf->m_BombMap));
			}
			else if(!str_comp_nocase(aConfig, "Default"))
			{
				str_copy(pSelf->m_BombMap, "Default", sizeof(pSelf->m_BombMap));
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "--------[BOMB]--------");
				pSelf->SendChatTarget(pResult->m_ClientId, "ERROR: unknown map. Available maps: ");
				pSelf->SendChatTarget(pResult->m_ClientId, "Default");
				pSelf->SendChatTarget(pResult->m_ClientId, "NoArena");
				pSelf->SendChatTarget(pResult->m_ClientId, "----------------------");
				return;
			}
		}
		else //no config --> Default
		{
			str_copy(pSelf->m_BombMap, "Default", sizeof(pSelf->m_BombMap));
		}

		pSelf->m_apPlayers[pResult->m_ClientId]->m_BombTicksUnready = 0;
		pSelf->m_BombMoney = BombMoney;
		pSelf->m_BombGameState = 1;
		pChr->m_IsBombing = true;
		pPlayer->MoneyTransaction(-BombMoney, "bomb join");

		str_format(aBuf, sizeof(aBuf), "[BOMB] You have created a game lobby. Map: '%s'.", pSelf->m_BombMap);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		str_format(aBuf, sizeof(aBuf), " -%d money for joining this bomb game.", BombMoney);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	}
	else if(!str_comp_nocase(aCmd, "join"))
	{
		if(!pPlayer->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You need to be logged in to join a bomb game. More info at '/accountinfo'.");
			return;
		}
		if(pPlayer->m_Account.m_BombBanTime)
		{
			str_format(aBuf, sizeof(aBuf), "You are banned from bomb games for %d second(s).", pPlayer->m_Account.m_BombBanTime / 60);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			return;
		}
		if(pChr->m_IsBombing)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You are already in the bomb game.");
			return;
		}
		if(pSelf->IsMinigame(pResult->m_ClientId))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			return;
		}

		if(!pSelf->m_BombGameState)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "No bomb game running. You can create a new one with '/bomb create <money>'.");
			return;
		}
		else if(pSelf->m_BombGameState == 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "The bomb lobby is locked.");
			return;
		}
		else if(pSelf->m_BombGameState == 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "The bomb game is already running.");
			return;
		}
		else if(pSelf->m_BombGameState == 1)
		{
			if(pPlayer->GetMoney() < pSelf->m_BombMoney)
			{
				str_format(aBuf, sizeof(aBuf), "You need at least %" PRId64 " money to join this game.", pSelf->m_BombMoney);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}

			str_format(aBuf, sizeof(aBuf), "-%" PRId64 " money for joining this game. You don't want to risk that much money? Type '/bomb leave'", pSelf->m_BombMoney);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendChatTarget(pResult->m_ClientId, "You will die in this game! So better leave if you want to keep weapons and stuff.");
			pChr->m_IsBombing = true;
			pPlayer->MoneyTransaction(-pSelf->m_BombMoney, "bomb join");
			pPlayer->m_BombTicksUnready = 0;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Something went horrible wrong lol. Please contact an admin.");
			return;
		}
	}
	else if(!str_comp_nocase(aCmd, "leave"))
	{
		if(!pChr->m_IsBombing)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You are not in a bomb game.");
			return;
		}
		if(pChr->m_IsBombing && pSelf->m_BombGameState == 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You can't leave a running game. (If you disconnect, your money will be lost.)");
			return;
		}

		str_format(aBuf, sizeof(aBuf), "You left the bomb game. (+%" PRId64 " money)", pSelf->m_BombMoney);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		pPlayer->MoneyTransaction(pSelf->m_BombMoney, "bomb leave");
		pSelf->SendBroadcast("", pResult->m_ClientId);
		pChr->m_IsBombing = false;
		pChr->m_IsBomb = false;
		pChr->m_IsBombReady = false;
	}
	else if(!str_comp_nocase(aCmd, "start"))
	{
		if(!pChr->m_IsBombing)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You are not in a bomb game. Type '/bomb create <money>' or '/bomb join' first.");
			return;
		}
		if(pChr->m_IsBombReady)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You are already ready to start. (If you aren't ready anymore try '/bomb leave')");
			return;
		}
		if(pChr->m_IsBombing && pSelf->m_BombGameState == 3) //should be never triggered but yolo xd
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Game is already running...");
			return;
		}

		pSelf->SendChatTarget(pResult->m_ClientId, "You are now ready to play. Waiting for other players...");
		pChr->m_IsBombReady = true;
	}
	else if(!str_comp_nocase(aCmd, "lock"))
	{
		if(!pChr->m_IsBombing)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You are not in a bomb game.");
			return;
		}
		if(pChr->m_IsBombing && pSelf->m_BombGameState == 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Running games are locked automatically.");
			return;
		}
		if(pSelf->CountBombPlayers() == 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "More bomb players needed to lock the lobby.");
			return;
		}
		if(g_Config.m_SvBombLockable == 0) //off
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Locking bomblobbys is deactivated.");
			return;
		}
		else if(g_Config.m_SvBombLockable == 1) //mods and higher
		{
			if(!pSelf->Server()->GetAuthedState(pResult->m_ClientId))
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "Only authed players can lock bomb games.");
				return;
			}
		}

		if(pSelf->m_BombGameState == 1) //unlocked --> lock
		{
			//lock it
			pSelf->m_BombGameState = 2;

			//send lock message to all bombers
			str_format(aBuf, sizeof(aBuf), "'%s' locked the bomb lobby.", pSelf->Server()->ClientName(pResult->m_ClientId));
			pSelf->SendChatBomb(aBuf);
		}
		else if(pSelf->m_BombGameState == 2) //locked --> unlock
		{
			//unlock it
			pSelf->m_BombGameState = 1;

			//send unlock message to all bombers
			str_format(aBuf, sizeof(aBuf), "'%s' unlocked the bomb lobby.", pSelf->Server()->ClientName(pResult->m_ClientId));
			pSelf->SendChatBomb(aBuf);
		}
	}
	else if(!str_comp_nocase(aCmd, "status"))
	{
		if(!pSelf->m_BombGameState) //offline
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Currently no bomb game running.");
			return;
		}
		else if(pSelf->m_BombGameState == 1) //lobby
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "------ Bomb Lobby ------");
			str_format(aBuf, sizeof(aBuf), "[%d/%d] players ready", pSelf->CountReadyBombPlayers(), pSelf->CountBombPlayers());
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendChatTarget(pResult->m_ClientId, "------------------------");
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(pSelf->GetPlayerChar(i) && pSelf->GetPlayerChar(i)->m_IsBombing)
				{
					if(pSelf->GetPlayerChar(i)->m_IsBombReady)
					{
						str_format(aBuf, sizeof(aBuf), "'%s' (ready)", pSelf->Server()->ClientName(i));
						pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "'%s'", pSelf->Server()->ClientName(i));
						pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
					}
				}
			}
			return;
		}
		else if(pSelf->m_BombGameState == 2) //lobby locked
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "------ Bomb Lobby (locked) ------");
			str_format(aBuf, sizeof(aBuf), "[%d/%d] players ready", pSelf->CountReadyBombPlayers(), pSelf->CountBombPlayers());
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendChatTarget(pResult->m_ClientId, "------------------------");
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(pSelf->GetPlayerChar(i) && pSelf->GetPlayerChar(i)->m_IsBombing)
				{
					if(pSelf->GetPlayerChar(i)->m_IsBombReady)
					{
						str_format(aBuf, sizeof(aBuf), "'%s' (ready)", pSelf->Server()->ClientName(i));
						pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "'%s'", pSelf->Server()->ClientName(i));
						pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
					}
				}
			}
			return;
		}
		else if(pSelf->m_BombGameState == 3) //ingame
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "------ Bomb (running game) ------");
			str_format(aBuf, sizeof(aBuf), "[%d/%d] players ready", pSelf->CountReadyBombPlayers(), pSelf->CountBombPlayers());
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendChatTarget(pResult->m_ClientId, "------------------------");
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(pSelf->GetPlayerChar(i) && pSelf->GetPlayerChar(i)->m_IsBombing)
				{
					if(pSelf->GetPlayerChar(i)->m_IsBombReady)
					{
						str_format(aBuf, sizeof(aBuf), "'%s' (ready)", pSelf->Server()->ClientName(i));
						pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "'%s'", pSelf->Server()->ClientName(i));
						pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
					}
				}
			}
			return;
		}
	}
	else if(!str_comp_nocase(aCmd, "ban"))
	{
		if(pResult->NumArguments() < 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "ERROR: Stick to the following structure:");
			pSelf->SendChatTarget(pResult->m_ClientId, "'/bomb ban <seconds> <player>'");
			return;
		}

		if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) == AUTHED_ADMIN) //DESC power to use highest rank
		{
			//int Bantime = pResult->GetInteger(1) * pSelf->Server()->TickSpeed();
			int Bantime = pResult->GetInteger(1);
			char aBanname[32];
			str_copy(aBanname, pResult->GetString(2), sizeof(aBanname));
			int BanId = pSelf->GetCidByName(aBanname);

			if(BanId == -1)
			{
				str_format(aBuf, sizeof(aBuf), "Can't find playername: '%s'.", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}

			if(Bantime > 900) //15 min ban limit
			{
				Bantime = 900;
				str_format(aBuf, sizeof(aBuf), "You banned '%s' for 15 minutes (max).", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

				//off all bomb stuff for the player
				pSelf->GetPlayerChar(BanId)->m_IsBombing = false;
				pSelf->GetPlayerChar(BanId)->m_IsBomb = false;
				pSelf->GetPlayerChar(BanId)->m_IsBombReady = false;

				//do the ban
				pSelf->m_apPlayers[BanId]->m_Account.m_BombBanTime = Bantime * 60;
				str_format(aBuf, sizeof(aBuf), "[BOMB] You were banned by admin '%s' for %d seconds.", pSelf->Server()->ClientName(pResult->m_ClientId), Bantime);
				pSelf->SendChatTarget(BanId, aBuf);
			}
			else if(Bantime < 0)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "Bantime has to be positive.");
				return;
			}
			else
			{
				//BANNED PLAYER

				//off all bomb stuff for the player
				pSelf->GetPlayerChar(BanId)->m_IsBombing = false;
				pSelf->GetPlayerChar(BanId)->m_IsBomb = false;
				pSelf->GetPlayerChar(BanId)->m_IsBombReady = false;

				//do the ban
				pSelf->m_apPlayers[BanId]->m_Account.m_BombBanTime = Bantime * 60;
				str_format(aBuf, sizeof(aBuf), "[BOMB] You were banned by admin '%s' for %d seconds.", pSelf->Server()->ClientName(pResult->m_ClientId), Bantime);
				pSelf->SendChatTarget(BanId, aBuf);

				//BANNING PLAYER
				str_format(aBuf, sizeof(aBuf), "You banned '%s' for %d seconds from bomb games.", aBanname, Bantime);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

				//ADMIN CONSOLE (isnt admin console ._. itz chat :c)
				//str_format(aBuf, sizeof(aBuf), "'%s' were banned by admin '%s' for %d seconds.", aBanname, pSelf->Server()->ClientName(pResult->m_ClientId), Bantime);
				//pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp", aBuf);
			}
		}
		else if(pPlayer->m_Account.m_IsSuperModerator)
		{
			int Bantime = pResult->GetInteger(1);
			char aBanname[32];
			str_copy(aBanname, pResult->GetString(2), sizeof(aBanname));
			int BanId = pSelf->GetCidByName(aBanname);

			if(BanId == -1)
			{
				str_format(aBuf, sizeof(aBuf), "Can't find playername: '%s'.", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}
			if(pSelf->Server()->GetAuthedState(BanId) == AUTHED_ADMIN)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission.");
				return;
			}

			if(Bantime > 300) //5 min ban limit
			{
				Bantime = 300;
				str_format(aBuf, sizeof(aBuf), "You banned '%s' for 5 minutes (max).", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

				//off all bomb stuff for the player
				pSelf->GetPlayerChar(BanId)->m_IsBombing = false;
				pSelf->GetPlayerChar(BanId)->m_IsBomb = false;
				pSelf->GetPlayerChar(BanId)->m_IsBombReady = false;

				//do the ban
				pSelf->m_apPlayers[BanId]->m_Account.m_BombBanTime = Bantime * 60;
				str_format(aBuf, sizeof(aBuf), "[BOMB] You were banned by VIP+ '%s' for %d seconds.", pSelf->Server()->ClientName(pResult->m_ClientId), Bantime);
				pSelf->SendChatTarget(BanId, aBuf);
			}
			else if(Bantime < 0)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "Bantime has to be positive.");
				return;
			}
			else
			{
				//BANNED PLAYER

				//off all bomb stuff for the player
				pSelf->GetPlayerChar(BanId)->m_IsBombing = false;
				pSelf->GetPlayerChar(BanId)->m_IsBomb = false;
				pSelf->GetPlayerChar(BanId)->m_IsBombReady = false;

				//do the ban
				pSelf->m_apPlayers[BanId]->m_Account.m_BombBanTime = Bantime * 60;
				str_format(aBuf, sizeof(aBuf), "[BOMB] You were banned by VIP+ '%s' for %d seconds.", pSelf->Server()->ClientName(pResult->m_ClientId), Bantime);
				pSelf->SendChatTarget(BanId, aBuf);

				//BANNING PLAYER
				str_format(aBuf, sizeof(aBuf), "You banned '%s' for %d seconds from bomb games.", aBanname, Bantime);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

				//ADMIN CONSOLE (isnt admin console ._. itz chat :c)
				//str_format(aBuf, sizeof(aBuf), "'%s' were banned by supermoderator '%s' for %d seconds.", aBanname, pSelf->Server()->ClientName(pResult->m_ClientId), Bantime);
				//pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp", aBuf);
			}
		}
		else if(pPlayer->m_Account.m_IsModerator)
		{
			int Bantime = pResult->GetInteger(1);
			char aBanname[32];
			str_copy(aBanname, pResult->GetString(2), sizeof(aBanname));
			int BanId = pSelf->GetCidByName(aBanname);

			if(BanId == -1)
			{
				str_format(aBuf, sizeof(aBuf), "Can't find playername: '%s'.", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}
			if(pSelf->Server()->GetAuthedState(BanId) == AUTHED_ADMIN || pSelf->m_apPlayers[BanId]->m_Account.m_IsSuperModerator)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission to kick this player.");
				return;
			}

			if(Bantime > 60) //1 min ban limit
			{
				Bantime = 60;
				str_format(aBuf, sizeof(aBuf), "You banned '%s' for 1 minute (max).", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

				//off all bomb stuff for the player
				pSelf->GetPlayerChar(BanId)->m_IsBombing = false;
				pSelf->GetPlayerChar(BanId)->m_IsBomb = false;
				pSelf->GetPlayerChar(BanId)->m_IsBombReady = false;

				//do the ban
				pSelf->m_apPlayers[BanId]->m_Account.m_BombBanTime = Bantime * 60;
				str_format(aBuf, sizeof(aBuf), "[BOMB] You were banned by VIP '%s' for %d seconds.", pSelf->Server()->ClientName(pResult->m_ClientId), Bantime);
				pSelf->SendChatTarget(BanId, aBuf);
			}
			else if(Bantime < 0)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "Bantime has to be positive.");
				return;
			}
			else
			{
				//BANNED PLAYER

				//off all bomb stuff for the player
				pSelf->GetPlayerChar(BanId)->m_IsBombing = false;
				pSelf->GetPlayerChar(BanId)->m_IsBomb = false;
				pSelf->GetPlayerChar(BanId)->m_IsBombReady = false;

				//do the ban
				pSelf->m_apPlayers[BanId]->m_Account.m_BombBanTime = Bantime * 60;
				str_format(aBuf, sizeof(aBuf), "[BOMB] You were banned by VIP '%s' for %d seconds.", pSelf->Server()->ClientName(pResult->m_ClientId), Bantime);
				pSelf->SendChatTarget(BanId, aBuf);

				//BANNING PLAYER
				str_format(aBuf, sizeof(aBuf), "You banned '%s' for %d seconds from bomb games.", aBanname, Bantime);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

				//ADMIN CONSOLE (isnt admin console ._. itz chat :c)
				//str_format(aBuf, sizeof(aBuf), "'%s' were banned by vip '%s' for %d seconds.", aBanname, pSelf->Server()->ClientName(pResult->m_ClientId), Bantime);
				//pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp", aBuf);
			}
		}
		else if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) == AUTHED_MOD)
		{
			int Bantime = pResult->GetInteger(1);
			char aBanname[32];
			str_copy(aBanname, pResult->GetString(2), sizeof(aBanname));
			int BanId = pSelf->GetCidByName(aBanname);

			if(BanId == -1)
			{
				str_format(aBuf, sizeof(aBuf), "Can't find playername: '%s'.", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}
			if(pSelf->Server()->GetAuthedState(BanId) == AUTHED_ADMIN || pSelf->m_apPlayers[BanId]->m_Account.m_IsSuperModerator || pSelf->m_apPlayers[BanId]->m_Account.m_IsModerator)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission to kick this player.");
				return;
			}

			if(!pSelf->GetPlayerChar(BanId)->m_IsBombing)
			{
				str_format(aBuf, sizeof(aBuf), "'%s' is not in a bomb game.", aBanname);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				return;
			}

			// KICKED PLAYER

			// off all bomb stuff for the player
			pSelf->GetPlayerChar(BanId)->m_IsBombing = false;
			pSelf->GetPlayerChar(BanId)->m_IsBomb = false;
			pSelf->GetPlayerChar(BanId)->m_IsBombReady = false;
			pSelf->SendBroadcast("", BanId);

			// do the kick
			pSelf->m_apPlayers[BanId]->m_Account.m_BombBanTime = Bantime * 60;
			str_format(aBuf, sizeof(aBuf), "[BOMB] You were kicked by rcon_mod '%s'.", pSelf->Server()->ClientName(pResult->m_ClientId));
			pSelf->SendChatTarget(BanId, aBuf);

			// KICKING PLAYER
			str_format(aBuf, sizeof(aBuf), "You kicked '%s' from bomb games.", aBanname);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

			// ADMIN CONSOLE (isnt admin console ._. itz chat :c)
			// str_format(aBuf, sizeof(aBuf), "'%s' were kicked by rcon_mod '%s'.", aBanname, pSelf->Server()->ClientName(pResult->m_ClientId));
			// pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp", aBuf);
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission.");
			return;
		}
	}
	else if(!str_comp_nocase(aCmd, "unban"))
	{
		if(pResult->NumArguments() < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Missing parameter: ClientId to unban ('/bomb banlist' for IDs).");
			pSelf->SendChatTarget(pResult->m_ClientId, "Command structure: '/bomb unban <ClientId>'.");
			pSelf->SendChatTarget(pResult->m_ClientId, "Unban all: '/bomb unban -1'.");
			return;
		}

		int UnbanId = pResult->GetInteger(1);

		if(UnbanId == -1) // unban all
		{
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(pSelf->m_apPlayers[i] && pSelf->m_apPlayers[i]->m_Account.m_BombBanTime)
				{
					// UNBANNING PLAYER
					str_format(aBuf, sizeof(aBuf), "You unbanned '%s' (he had %d seconds bantime left).", pSelf->Server()->ClientName(i), pSelf->m_apPlayers[i]->m_Account.m_BombBanTime / 60);
					pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

					// UNBANNED PLAYER
					pSelf->m_apPlayers[i]->m_Account.m_BombBanTime = 0;
					str_format(aBuf, sizeof(aBuf), "You were unbanned from bomb games by '%s'.", pSelf->Server()->ClientName(pResult->m_ClientId));
					pSelf->SendChatTarget(pSelf->m_apPlayers[i]->GetCid(), aBuf);
				}
			}
			return;
		}

		if(pSelf->m_apPlayers[UnbanId])
		{
			if(pSelf->m_apPlayers[UnbanId]->m_Account.m_BombBanTime)
			{
				// UNBANNING PLAYER
				str_format(aBuf, sizeof(aBuf), "You unbanned '%s' (he had %d seconds bantime left).", pSelf->Server()->ClientName(UnbanId), pSelf->m_apPlayers[UnbanId]->m_Account.m_BombBanTime / 60);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

				// UNBANNED PLAYER
				pSelf->m_apPlayers[UnbanId]->m_Account.m_BombBanTime = 0;
				str_format(aBuf, sizeof(aBuf), "You were unbanned from bomb games by '%s'.", pSelf->Server()->ClientName(pResult->m_ClientId));
				pSelf->SendChatTarget(pSelf->m_apPlayers[UnbanId]->GetCid(), aBuf);
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "'%s' is not banned from bomb games.", pSelf->Server()->ClientName(UnbanId));
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "There is no player with this ClientId.");
			return;
		}
	}
	else if(!str_comp_nocase(aCmd, "banlist")) // for now all dudes can see the banlist... not sure to make it rank only visible
	{
		if(!pSelf->CountBannedBombPlayers())
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "There are no banned bomb players, yay.");
			return;
		}

		// calculate page amount
		// float f_pages = pSelf->CountBannedBombPlayers() / 5;

		// int pages = round(pSelf->CountBannedBombPlayers() / 5 + 0.5); //works as good as ++
		int Pages = (int)(((float)pSelf->CountBannedBombPlayers() / 5.0f) + 0.999999f);
		int PlayersShownOnPage = 0;

		if(pResult->NumArguments() > 1) //show pages
		{
			int PlayersShownOnPreviousPages = 0;
			int Page = pResult->GetInteger(1);
			if(Page > Pages)
			{
				if(Pages == 1)
				{
					str_format(aBuf, sizeof(aBuf), "ERROR: There is only %d page.", Pages);
					pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				}
				else
				{
					str_format(aBuf, sizeof(aBuf), "ERROR: There are only %d pages.", Pages);
					pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				}
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "=== (%d) Banned Bombers (PAGE: %d/%d) ===", pSelf->CountBannedBombPlayers(), Page, Pages);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				//for (int i = page * 5; i < MAX_CLIENTS; i++) yes it is an minor performance improvement but fuck it (did that cuz something didnt work) ((would be -1 anyways because human page 2 is computer page 1))
				for(auto &Player : pSelf->m_apPlayers)
				{
					if(!Player)
						continue;
					if(!Player->m_Account.m_BombBanTime)
						continue;

					if(PlayersShownOnPreviousPages >= (Page - 1) * 5)
					{
						str_format(
							aBuf,
							sizeof(aBuf),
							"Id: %d '%s' (%d seconds)",
							Player->GetCid(),
							pSelf->Server()->ClientName(Player->GetCid()),
							Player->m_Account.m_BombBanTime / 60);
						pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
						PlayersShownOnPage++;
					}
					else
					{
						//str_format(aBuf, sizeof(aBuf), "No list cuz  NOT Previous: %d > (page - 1) * 5: %d", PlayersShownOnPreviousPages, (page - 1) * 5);
						//dbg_msg("bomb", aBuf);
						PlayersShownOnPreviousPages++;
					}
					if(PlayersShownOnPage > 4) //show only 5 players on one site
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
			str_format(aBuf, sizeof(aBuf), "=== (%d) Banned Bombers (PAGE: %d/%d) ===", pSelf->CountBannedBombPlayers(), 1, Pages);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			for(auto &Player : pSelf->m_apPlayers)
			{
				if(!Player)
					continue;
				if(!Player->m_Account.m_BombBanTime)
					continue;

				str_format(
					aBuf,
					sizeof(aBuf),
					"Id: %d '%s' (%d seconds)",
					Player->GetCid(),
					pSelf->Server()->ClientName(Player->GetCid()),
					Player->m_Account.m_BombBanTime / 60);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				PlayersShownOnPage++;
				if(PlayersShownOnPage > 4) //show only 5 players on one site
				{
					break;
				}
			}
		}
	}
	else if(!str_comp_nocase(aCmd, "help") || !str_comp_nocase(aCmd, "info"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "===================");
		pSelf->SendChatTarget(pResult->m_ClientId, "->   B O M B   <-");
		pSelf->SendChatTarget(pResult->m_ClientId, "===================");
		pSelf->SendChatTarget(pResult->m_ClientId, "HOW DOES THE GAME WORK?");
		pSelf->SendChatTarget(pResult->m_ClientId, "One player gets chosen as bomb. Hammer other players to transfer bomb.");
		pSelf->SendChatTarget(pResult->m_ClientId, "If a player explodes as bomb he is out.");
		pSelf->SendChatTarget(pResult->m_ClientId, "Last man standing is the winner.");
		pSelf->SendChatTarget(pResult->m_ClientId, "HOW TO START?");
		pSelf->SendChatTarget(pResult->m_ClientId, "One player has to create a bomb lobby with '/bomb create <money>'.");
		pSelf->SendChatTarget(pResult->m_ClientId, "Players can join with '/bomb join'.");
		pSelf->SendChatTarget(pResult->m_ClientId, "All players who join the game have to pay <money> money and the winner gets it all.");
		pSelf->SendChatTarget(pResult->m_ClientId, "---------------");
		pSelf->SendChatTarget(pResult->m_ClientId, "More bomb commands at '/bomb cmdlist'.");
		return;
	}
	else if(!str_comp_nocase(aCmd, "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== BOMB COMMANDS ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bomb create <money>' to create a bomb game.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bomb create <money> <map>' also creates a bomb game.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bomb join' to join a bomb game.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bomb start' to start a game.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bomb lock' to lock a bomb lobby.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bomb status' to see some live bomb stats.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bomb ban <seconds> <name>' to ban players.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bomb unban <ClientId>' to unban players.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bomb banlist' to see all banned players.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bomb help' for help and info.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bomb cmdlist' for all bomb commands.");
		return;
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Unknown bomb command. More help at '/bomb help' or 'bomb cmdlist'.");
		return;
	}
}

void CGameContext::ConSurvival(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	if(!g_Config.m_SvAllowSurvival)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[SURVIVAL] command not allowed.");
		return;
	}

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[SURVIVAL] You have to be alive to use this command.");
		return;
	}

	//if (!pPlayer->IsLoggedIn()) //we want 10000 survival players so no annoying login
	//{
	//	pSelf->SendChatTarget(pResult->m_ClientId, "You have to be logged in to use this command. (type '/accountinfo' for more info)");
	//	return;
	//}

	if(pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=========> [SURVIVAL] <=========");
		pSelf->SendChatTarget(pResult->m_ClientId, "Don't die or you'r out!");
		pSelf->SendChatTarget(pResult->m_ClientId, "Last man standing wins the game.");
		pSelf->SendChatTarget(pResult->m_ClientId, "check '/survival cmdlist' for a list of all commands");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== SURVIVAL COMMANDS ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/survival join' to join the survival lobby");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/survival leave' to leave survival");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/survival status' to show current game status");
		//pSelf->SendChatTarget(pResult->m_ClientId, "'/survival stats' to show your stats");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "status"))
	{
		char aBuf[128];
		char aGameState[128];
		char aDM[16];
		char aTimeLimit[64];
		str_copy(aTimeLimit, "", sizeof(aTimeLimit));
		str_copy(aDM, "", sizeof(aDM));
		switch(pSelf->m_survivalgamestate)
		{
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
			if(pSelf->m_survival_game_countdown != -1)
			{
				float Time = pSelf->m_survival_game_countdown / pSelf->Server()->TickSpeed();
				str_format(aTimeLimit, sizeof(aTimeLimit), "(%d min %5.2f sec left max)", (int)Time / 60, Time - ((int)Time / 60 * 60));
			}
			str_format(aGameState, sizeof(aGameState), "running %s %d/%d alive %s", aDM, pSelf->CountSurvivalPlayers(true), pSelf->m_survival_start_players, aTimeLimit);
			break;
		default:
			str_copy(aGameState, "unknown", sizeof(aGameState));
			break;
		}
		str_format(aBuf, sizeof(aBuf), "[SURVIVAL] Game is %s", aGameState);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	}
	else if(!str_comp_nocase(pResult->GetString(0), "leave"))
	{
		if(pPlayer->m_IsSurvivaling)
		{
			pChr->Die(pPlayer->GetCid(), WEAPON_SELF);
			pSelf->SetPlayerSurvival(pResult->m_ClientId, 0);
			pSelf->SendChatTarget(pResult->m_ClientId, "[SURVIVAL] you left the game. (bye c:)");
			pSelf->SendBroadcast("", pPlayer->GetCid(), 1); // survival broadcasts are importance lvl1 so lvl1 is needed to overwrite
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[SURVIVAL] you currently aren't playing survival.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "join"))
	{
		if(pSelf->IsMinigame(pResult->m_ClientId) == 4)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[SURVIVAL] you already joined the survival game.");
			return;
		}
		if(pSelf->IsMinigame(pResult->m_ClientId))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Error: maybe you are already in a minigame or jail. (check '/minigames status')");
			return;
		}
		int Spawns = pSelf->Collision()->CountSurvivalSpawns();
		int SurvivalPlayers = pSelf->CountSurvivalPlayers();
		if(SurvivalPlayers >= Spawns)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[SURVIVAL] The survival lobby is full try again later.");
			return;
		}

		vec2 SurvialLobbySpawnTile = pSelf->Collision()->GetRandomTile(TILE_SURVIVAL_LOBBY);

		if(SurvialLobbySpawnTile != vec2(-1, -1))
		{
			pSelf->m_apPlayers[pResult->m_ClientId]->GetCharacter()->SetPosition(SurvialLobbySpawnTile);
			pSelf->SendChatTarget(pResult->m_ClientId, "[SURVIVAL] You joined survival.");
			pSelf->SetPlayerSurvival(pResult->m_ClientId, 1);
		}
		else //no TestToTeleTile
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[SURVIVAL] No survival arena set.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Unknown survival parameter. Check '/survival cmdlist' for all commands.");
	}
}

void CGameContext::ConSpawn(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	if(!pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[SPAWN] you have to be logged in to use this command '/accountinfo'.");
		return;
	}
	if(pPlayer->GetMoney() < 1000000)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[SPAWN] you need at least 1 million money to use this command.");
		return;
	}
	if(pSelf->IsMinigame(pResult->m_ClientId))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[SPAWN] you can't use this command in minigames or jail.");
		return;
	}

	if(pChr->DDPP_Respawn())
		pPlayer->MoneyTransaction(-50000, "teleport to spawn");
	else
		pSelf->SendChatTarget(pResult->m_ClientId, "[SPAWN] teleport to spawn failed. Try again later.");
}

void CGameContext::ConRoom(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	char aBuf[128];
	char aCmd[64];
	str_copy(aCmd, pResult->GetString(0), sizeof(aCmd));

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Missing parameters. Check '/room help'.");
		return;
	}

	if(!str_comp_nocase(aCmd, "help"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "*** ROOM ***");
		pSelf->SendChatTarget(pResult->m_ClientId, "The room is a special place on the map.");
		pSelf->SendChatTarget(pResult->m_ClientId, "Where special people can chill...");
		pSelf->SendChatTarget(pResult->m_ClientId, "This command allows VIP+ to invite tees to this room.");
		pSelf->SendChatTarget(pResult->m_ClientId, "*** USAGE ***");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/room invite <player>'.");
	}
	else if(!str_comp_nocase(aCmd, "invite"))
	{
		if(pResult->NumArguments() < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "ERROR: Stick to construct: '/room invite <player>'.");
			return;
		}

		char aInviteName[32];
		str_copy(aInviteName, pResult->GetString(1), sizeof(aInviteName));
		int InviteId = pSelf->GetCidByName(aInviteName);

		if(InviteId == -1)
		{
			str_format(aBuf, sizeof(aBuf), "Can't find playername: '%s'.", aInviteName);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			return;
		}
		if(!pPlayer->m_Account.m_IsSuperModerator)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission.");
			return;
		}
		if(!pPlayer->m_BoughtRoom)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You need a roomkey to invite others. ('/buy room_key')");
			return;
		}
		if(pSelf->m_apPlayers[InviteId]->m_BoughtRoom)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "This player already has a room key.");
			return;
		}
		if(pSelf->GetPlayerChar(InviteId)->m_HasRoomKeyBySuperModerator)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "This player was already invited by a VIP+.");
			return;
		}
		if(!pSelf->GetPlayerChar(InviteId))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "This player is not alive... try again later.");
			return;
		}

		//GETTER
		pSelf->GetPlayerChar(InviteId)->m_HasRoomKeyBySuperModerator = true;
		str_format(aBuf, sizeof(aBuf), "'%s' invited you to the room c:.", pSelf->Server()->ClientName(pResult->m_ClientId));
		pSelf->SendChatTarget(InviteId, aBuf);
		//GIVER
		str_format(aBuf, sizeof(aBuf), "You invited '%s' to the room.", pSelf->Server()->ClientName(InviteId));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	}
	else if(!str_comp_nocase(aCmd, "kick"))
	{
		if(pResult->NumArguments() < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "ERROR: Stick construct: '/room kick <player>'.");
			return;
		}

		char aInviteName[32];
		str_copy(aInviteName, pResult->GetString(1), sizeof(aInviteName));
		int InviteId = pSelf->GetCidByName(aInviteName);

		if(InviteId == -1)
		{
			str_format(aBuf, sizeof(aBuf), "Can't find playername: '%s'.", aInviteName);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			return;
		}
		if(!pPlayer->m_Account.m_IsSuperModerator)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission.");
			return;
		}
		if(!pPlayer->m_BoughtRoom)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You need a roomkey to kick others. ('/buy room_key')");
			return;
		}
		if(pSelf->m_apPlayers[InviteId]->m_BoughtRoom)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "ERROR: This player bought a room key.");
			return;
		}
		if(!pSelf->GetPlayerChar(InviteId)->m_HasRoomKeyBySuperModerator)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "This player wasn't given a key by a VIP+.");
			return;
		}
		if(!pSelf->GetPlayerChar(InviteId))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "This player is not alive... try again later.");
			return;
		}

		//TAKEN
		pSelf->GetPlayerChar(InviteId)->m_HasRoomKeyBySuperModerator = false;
		str_format(aBuf, sizeof(aBuf), "'%s' kicked you out of room.", pSelf->Server()->ClientName(pResult->m_ClientId));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		//TAKER
		str_format(aBuf, sizeof(aBuf), "You kicked '%s' out of room.", pSelf->Server()->ClientName(InviteId));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Unknown command. Try '/room help' for more help.");
	}
}

void CGameContext::ConBank(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	char aBuf[256];

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "**** BANK ****");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bank close'");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bank open'");
		return;
	}

	if(!str_comp_nocase(pResult->GetString(0), "close"))
	{
		if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) != AUTHED_ADMIN)
		{
			//pSelf->SendChatTarget(pResult->m_ClientId, "No such command: bank close.");
			pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission.");
			return;
		}

		if(!pSelf->m_IsBankOpen)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Bank is already closed.");
			return;
		}

		pSelf->m_IsBankOpen = false;
		pSelf->SendChatTarget(pResult->m_ClientId, "<bank> bye world!");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "open"))
	{
		if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) != AUTHED_ADMIN)
		{
			//pSelf->SendChatTarget(pResult->m_ClientId, "No such command: bank open.");
			pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission.");
			return;
		}

		if(pSelf->m_IsBankOpen)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Bank is already open.");
			return;
		}

		pSelf->m_IsBankOpen = true;
		pSelf->SendChatTarget(pResult->m_ClientId, "<bank> I'm open!");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "rob"))
	{
		if(!pChr->m_InBank)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You have to be in the bank.");
			return;
		}
		if(!pSelf->m_IsBankOpen)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Bank is closed.");
			return;
		}
		if(!pPlayer->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You are not logged in more info at '/accountinfo'.");
			return;
		}

		int policedudesfound = 0;
		for(auto &Player : pSelf->m_apPlayers)
			if(Player && Player->m_Account.m_PoliceRank && Player != pPlayer)
				policedudesfound++;

		if(!policedudesfound)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You can't rob the bank if there is no police on the server!");
			return;
		}

		pPlayer->m_Account.m_EscapeTime += pSelf->Server()->TickSpeed() * 600; //+10 min
		//str_format(aBuf, sizeof(aBuf), "+%d bank robbery", 5 * policedudesfound);
		//pPlayer->MoneyTransaction(+5 * policedudesfound, aBuf);
		pPlayer->m_GangsterBagMoney += 5 * policedudesfound;
		str_format(aBuf, sizeof(aBuf), "You robbed the bank. (+%d money to your gangstabag)", 5 * policedudesfound);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		str_format(aBuf, sizeof(aBuf), "Police will be hunting you for %" PRId64 " minutes.", (pPlayer->m_Account.m_EscapeTime / pSelf->Server()->TickSpeed()) / 60);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

		str_format(aBuf, sizeof(aBuf), "'%s' robbed the bank.", pSelf->Server()->ClientName(pResult->m_ClientId));
		pSelf->SendAllPolice(aBuf);
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Unknown bank command check. '/bank' for more info.");
	}
}

void CGameContext::ConGangsterBag(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Real gangsters aren't dead or spectator.");
		return;
	}

	char aBuf[256];

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== Gangsterbag ===");
		str_format(aBuf, sizeof(aBuf), "You have got %d coins in your bag.", pPlayer->m_GangsterBagMoney);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		pSelf->SendChatTarget(pResult->m_ClientId, "Coins disappear upon disconnect.");
		pSelf->SendChatTarget(pResult->m_ClientId, "Real gangsters play 24/7 here and do illegal '/gangsterbag trade'!");
		return;
	}

	if(!str_comp_nocase(pResult->GetString(0), "trade"))
	{
		//todo: add trades with hammer to give gangsta coins to others

		// cant send yourself

		// can only trade if no escapetime

		// use brain to find bugsis

		if(!pPlayer->m_GangsterBagMoney)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You have no coins");
			return;
		}
		if(pResult->NumArguments() < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Try: '/trade <gangsta bro>'.");
			return;
		}
		if(pPlayer->m_Account.m_EscapeTime)
		{
			str_format(aBuf, sizeof(aBuf), "You can't trade while escaping the police. You have to wait %" PRId64 " seconds...", pPlayer->m_Account.m_EscapeTime / pSelf->Server()->TickSpeed());
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			return;
		}

		int BroId = pSelf->GetCidByName(pResult->GetString(1));
		if(BroId == -1)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Getting crazy? Choose a real person...");
			return;
		}
		if(!pSelf->m_apPlayers[BroId]->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Sure this is a trusty trade? He is not logged in...");
			return;
		}
		if(BroId == pResult->m_ClientId)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You can only trade with other gangsters.");
			return;
		}
		const NETADDR *pOwnAddr = pSelf->Server()->ClientAddr(pResult->m_ClientId);
		const NETADDR *pBroAddr = pSelf->Server()->ClientAddr(BroId);

		if(!net_addr_comp_noport(pOwnAddr, pBroAddr)) //send dummy money -> police traces ip -> dummy escape time
		{
			//bro
			pSelf->m_apPlayers[BroId]->m_GangsterBagMoney += pPlayer->m_GangsterBagMoney;
			pSelf->m_apPlayers[BroId]->m_Account.m_EscapeTime += pSelf->Server()->TickSpeed() * 180; // 180 secs == 3 minutes
			str_format(aBuf, sizeof(aBuf), "'%s' traded you %d gangster coins (police traced ip)", pSelf->Server()->ClientName(pResult->m_ClientId), pPlayer->m_GangsterBagMoney);
			pSelf->SendChatTarget(BroId, aBuf);

			//trader
			pSelf->SendChatTarget(pResult->m_ClientId, "Police recognized the illegal trade... (ip traced)");
			pSelf->SendChatTarget(pResult->m_ClientId, "Your bro now has gangster coins and is getting hunted by police.");
			pPlayer->m_GangsterBagMoney = 0;
			pPlayer->m_Account.m_EscapeTime += pSelf->Server()->TickSpeed() * 60; // +1 minutes for illegal trades
			return;
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "'%s' traded you %d money.", pSelf->Server()->ClientName(pResult->m_ClientId), pPlayer->m_GangsterBagMoney);
			pSelf->SendChatTarget(BroId, aBuf);
			pSelf->m_apPlayers[BroId]->MoneyTransaction(+pPlayer->m_GangsterBagMoney, "unknown source");

			pPlayer->m_GangsterBagMoney = 0;
			pSelf->SendChatTarget(pResult->m_ClientId, "You have traded coins!");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "clear"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Cleared gangsterbag ... rip coins.");
		pPlayer->m_GangsterBagMoney = 0;
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Try again with a real command.");
	}
}

void CGameContext::ConJailCode(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "You have to be ingame to use this command.");
		return;
	}

	char aBuf[256];
	if(pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Usage: '/jail_code <playername>'");
		return;
	}
	if(pPlayer->m_Account.m_PoliceRank < 2)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "You need police rank 2 or higher.");
		return;
	}
	if(pPlayer->m_Account.m_JailTime)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "You are arrested.");
		return;
	}

	int JailedId = -1;
	JailedId = pSelf->GetCidByName(pResult->GetString(0));
	if(JailedId == -1)
	{
		str_format(aBuf, sizeof(aBuf), "Can't find user '%s'", pResult->GetString(0));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		return;
	}
	if(!pSelf->m_apPlayers[JailedId]->m_Account.m_JailTime)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Player is not arrested.");
		return;
	}

	str_format(aBuf, sizeof(aBuf), "'%s' [%d]", pResult->GetString(0), pSelf->m_apPlayers[JailedId]->m_JailCode);
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
}

void CGameContext::ConJail(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "You have to be ingame to use this command.");
		return;
	}

	char aBuf[256];

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "---- JAIL ----");
		pSelf->SendChatTarget(pResult->m_ClientId, "The police brings all the gangster here.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/jail open <code> <player>' to open cells");
		//pSelf->SendChatTarget(pResult->m_ClientId, "'/jail list' list all jailed players"); //and for police2 list with codes
		pSelf->SendChatTarget(pResult->m_ClientId, "'/jail leave' to leave the jail");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/jail hammer' to config the police jail hammer");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/jail_code <player>' to show a certain jailcode");
		return;
	}

	if(!str_comp_nocase(pResult->GetString(0), "open"))
	{
		if(pResult->NumArguments() < 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Missing parameters. '/jail open <code> <player>'");
			return;
		}
		if(!pPlayer->GetCharacter()->IsOnTile(TILE_JAILRELEASE))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Get closer to the cell.");
			return;
		}
		int JailedId = pSelf->GetCidByName(pResult->GetString(2));
		if(!pSelf->m_apPlayers[JailedId])
		{
			str_format(aBuf, sizeof(aBuf), "'%s' is not online.", pResult->GetString(2));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			return;
		}
		if(!pSelf->m_apPlayers[JailedId]->m_Account.m_JailTime)
		{
			str_format(aBuf, sizeof(aBuf), "'%s' is not arrested.", pResult->GetString(2));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			return;
		}
		if(pResult->GetInteger(1) != pSelf->m_apPlayers[JailedId]->m_JailCode)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Wrong cell code.");
			return;
		}

		str_format(aBuf, sizeof(aBuf), "You opened %s's cell.", pResult->GetString(2));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

		pSelf->m_apPlayers[JailedId]->m_IsJailDoorOpen = true;
		str_format(aBuf, sizeof(aBuf), "Your cell door was opened by '%s'.", pSelf->Server()->ClientName(pResult->m_ClientId));
		pSelf->SendChatTarget(JailedId, aBuf);
		pSelf->SendChatTarget(JailedId, "'/jail leave' to leave. (warning this might be illegal)");
	}
	//else if (!str_comp_nocase(pResult->GetString(0), "corrupt"))
	//{
	//	if (pResult->NumArguments() == 1)
	//	{
	//		pSelf->SendChatTarget(pResult->m_ClientId, "'/jail corrupt <money> <player>'");
	//		return;
	//	}

	//	int corruptId = pSelf->GetCidByName(pResult->GetString(2));
	//	if (corruptId == -1)
	//	{
	//		str_format(aBuf, sizeof(aBuf), "'%s' is not online.", pResult->GetString(2));
	//		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	//		return;
	//	}
	//	if (!pSelf->m_apPlayers[corruptId]->m_Account.m_PoliceRank)
	//	{
	//		str_format(aBuf, sizeof(aBuf), "'%s' is no police officer.", pResult->GetString(2));
	//		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	//		return;
	//	}

	//	str_format(aBuf, sizeof(aBuf), "you offered %s %d money to reduce your jailtime.", pResult->GetString(2), pResult->GetInteger(1));
	//	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

	//	str_format(aBuf, sizeof(aBuf), "'%s' would pay %d money if you help with an escape.", pResult->GetString(2), pResult->GetInteger(1));
	//	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	//	pSelf->SendChatTarget(pResult->m_ClientId, "'/jail release <jail code> %s' to take the money.");
	//}
	else if(!str_comp_nocase(pResult->GetString(0), "list")) //codes
	{
		if(pPlayer->m_Account.m_JailTime || !pPlayer->m_Account.m_PoliceRank)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission.");
			return;
		}

		pSelf->SendChatTarget(pResult->m_ClientId, "coming soon");
		//list all jailed players with codes on several pages (steal bomb system)
	}
	else if(!str_comp_nocase(pResult->GetString(0), "leave"))
	{
		if(!pPlayer->m_Account.m_JailTime)
		{
			//pSelf->SendChatTarget(pResult->m_ClientId, "you are not arrested.");
			return;
		}
		if(!pPlayer->m_IsJailDoorOpen)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Cell door is closed.");
			return;
		}

		pSelf->SendChatTarget(pResult->m_ClientId, "You escaped the jail, run! The police will be hunting you for 10 minutes.");
		pSelf->AddEscapeReason(pResult->m_ClientId, "jail escape");
		pPlayer->m_Account.m_EscapeTime = pSelf->Server()->TickSpeed() * 600; // 10 minutes for escaping the jail
		pPlayer->m_Account.m_JailTime = 0;
		pPlayer->m_IsJailDoorOpen = false;

		vec2 JailReleaseSpawn = pSelf->Collision()->GetRandomTile(TILE_JAILRELEASE);

		if(JailReleaseSpawn != vec2(-1, -1))
		{
			pChr->SetPosition(JailReleaseSpawn);
		}
		else //no jailrelease tile
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "No jailrelease tile on this map.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "hammer"))
	{
		if(!pPlayer->m_Account.m_PoliceRank)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You have to be police to use this command.");
			return;
		}
		if(pResult->NumArguments() == 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "~~~ Jail Hammer ~~~");
			pSelf->SendChatTarget(pResult->m_ClientId, "Use this command to configurate your hammer.");
			pSelf->SendChatTarget(pResult->m_ClientId, "Use this hammer to move gangsters to jail.");
			pSelf->SendChatTarget(pResult->m_ClientId, "Simply activate it and hammer escaping gangsters.");
			pSelf->SendChatTarget(pResult->m_ClientId, "(you can only use it on freezed gangsters)");
			pSelf->SendChatTarget(pResult->m_ClientId, "If you are police 5 or higher then you can activate jail all hammer.");
			pSelf->SendChatTarget(pResult->m_ClientId, "Then you can jail also tees who are not known as gangsters.");
			pSelf->SendChatTarget(pResult->m_ClientId, "-- commands --");
			pSelf->SendChatTarget(pResult->m_ClientId, "'/jail hammer 1' to activate it.");
			pSelf->SendChatTarget(pResult->m_ClientId, "'/jail hammer 0' to deactivate it.");
			pSelf->SendChatTarget(pResult->m_ClientId, "'/jail hammer <seconds>' to activate jail all hammer.");
			return;
		}
		if(pResult->GetInteger(1) < 0)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "value has to be positive.");
			return;
		}
		if(pResult->GetInteger(1) == 0)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "jail hammer is now deactivated.");
			pPlayer->m_JailHammer = false;
			return;
		}
		if(pResult->GetInteger(1) == 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "jail hammer is now activated. (hammer frozen gangsters)");
			pPlayer->m_JailHammer = true;
			return;
		}
		if(pResult->GetInteger(1) > 1)
		{
			if(pPlayer->m_Account.m_PoliceRank < 5)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "You have to be police rank 5 or higher to use this value.");
				return;
			}
			if(pResult->GetInteger(1) > 600)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "You can't arrest people for longer than 10 minutes.");
				return;
			}

			//pPlayer->m_JailHammerTime = pResult->GetInteger(1);
			pPlayer->m_JailHammer = pResult->GetInteger(1);
			str_format(aBuf, sizeof(aBuf), "You can now jail every freezed tee for %d seconds with your hammer.", pPlayer->m_JailHammer);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendChatTarget(pResult->m_ClientId, "You have to judge who is criminal and who is not.");
			pSelf->SendChatTarget(pResult->m_ClientId, "Much power brings much responsibility.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Unknown jail parameter check '/jail' for more info");
	}
}
void CGameContext::ConAscii(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	char aBuf[256];

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "---- ascii art animation ----");
		pSelf->SendChatTarget(pResult->m_ClientId, "Create your own animation with this command.");
		pSelf->SendChatTarget(pResult->m_ClientId, "And publish it on your profile to share it.");
		pSelf->SendChatTarget(pResult->m_ClientId, "---- commands ----");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/ascii frame <frame number> <ascii art>' to edit a frame from 0-15.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/ascii speed <speed>' to change the animation speed.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/ascii profile <0/1>' private/publish animation on profile");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/ascii public <0/1>' private/publish animation.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/ascii view <client id>' to watch published animation.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/ascii view' to watch your animation.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/ascii stop' to stop running animation you'r watching.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/ascii stats' to see personal stats.");
		return;
	}

	if(!str_comp_nocase(pResult->GetString(0), "stats"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "---- ascii stats ----");
		str_format(aBuf, sizeof(aBuf), "views: %d", pPlayer->m_AsciiViewsDefault + pPlayer->m_AsciiViewsProfile);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		pSelf->SendChatTarget(pResult->m_ClientId, "---- specific ----");
		str_format(aBuf, sizeof(aBuf), "ascii views: %d", pPlayer->m_AsciiViewsDefault);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		str_format(aBuf, sizeof(aBuf), "ascii views (profile): %d", pPlayer->m_AsciiViewsProfile);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	}
	else if(!str_comp_nocase(pResult->GetString(0), "stop"))
	{
		if(pPlayer->m_AsciiWatchingId == -1)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You are not watching an ascii animation.");
			return;
		}

		pPlayer->m_AsciiWatchingId = -1;
		pPlayer->m_AsciiWatchFrame = 0;
		pPlayer->m_AsciiWatchTicker = 0;
		pSelf->SendBroadcast("", pResult->m_ClientId);
	}
	else if(!str_comp_nocase(pResult->GetString(0), "profile"))
	{
		if(!pPlayer->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You have to be logged in to create ascii animations.");
			pSelf->SendChatTarget(pResult->m_ClientId, "Use '/accountinfo' for more help about accounts.");
			return;
		}

		if(pResult->NumArguments() != 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Type '/ascii profile <0/1>' to private/publish animation on yourprofile.");
			return;
		}

		if(pResult->GetInteger(1) == 0)
		{
			if(pPlayer->m_Account.m_aAsciiPublishState[1] == '0')
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "Your animation is already private.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "Your animation is now private.");
				pSelf->SendChatTarget(pResult->m_ClientId, "It can no longer be viewed with '/profile view <you>'.");
				pPlayer->m_Account.m_aAsciiPublishState[1] = '0';
			}
		}
		else if(pResult->GetInteger(1) == 1)
		{
			if(pPlayer->m_Account.m_aAsciiPublishState[1] == '1')
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "Your animation is already public on your profile.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "Your animation is now public.");
				pSelf->SendChatTarget(pResult->m_ClientId, "It can be viewed with '/profile view <you>'.");
				pPlayer->m_Account.m_aAsciiPublishState[1] = '1';
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Use 0 to make your animation private.");
			pSelf->SendChatTarget(pResult->m_ClientId, "Use 1 to make your animation public.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "public") || !str_comp_nocase(pResult->GetString(0), "publish"))
	{
		if(!pPlayer->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You have to be logged in to create ascii animations.");
			pSelf->SendChatTarget(pResult->m_ClientId, "Use '/accountinfo' for more help about accounts.");
			return;
		}

		if(pResult->NumArguments() != 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Type '/ascii public <0/1>' to private/publish animation.");
			return;
		}

		if(pResult->GetInteger(1) == 0)
		{
			if(pPlayer->m_Account.m_aAsciiPublishState[0] == '0')
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "Your animation is already private.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "Your animation is now private.");
				pSelf->SendChatTarget(pResult->m_ClientId, "It can no longer be viewed with '/ascii view <your id>'");
				pPlayer->m_Account.m_aAsciiPublishState[0] = '0';
			}
		}
		else if(pResult->GetInteger(1) == 1)
		{
			if(pPlayer->m_Account.m_aAsciiPublishState[0] == '1')
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "Your animation is already public.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "Your animation is now public.");
				pSelf->SendChatTarget(pResult->m_ClientId, "It can be viewed with '/ascii view <your id>'");
				pPlayer->m_Account.m_aAsciiPublishState[0] = '1';
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Use 0 to make your animation private.");
			pSelf->SendChatTarget(pResult->m_ClientId, "Use 1 to make your animation public.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "frame"))
	{
		if(!pPlayer->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You have to be logged in to create ascii animations.");
			pSelf->SendChatTarget(pResult->m_ClientId, "Use '/accountinfo' for more help about accounts.");
			return;
		}

		if(pResult->NumArguments() < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Type '/ascii frame <frame number> <ascii art>' to edit a frame from 0-15.");
			return;
		}

		if(pResult->GetInteger(1) == 0)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[0], sizeof(pPlayer->m_Account.m_aAsciiFrame[0]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[0]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[0], pResult->m_ClientId);
		}
		else if(pResult->GetInteger(1) == 1)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[1], sizeof(pPlayer->m_Account.m_aAsciiFrame[1]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[1]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[1], pResult->m_ClientId);
		}
		else if(pResult->GetInteger(1) == 2)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[2], sizeof(pPlayer->m_Account.m_aAsciiFrame[2]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[2]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[2], pResult->m_ClientId);
		}
		else if(pResult->GetInteger(1) == 3)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[3], sizeof(pPlayer->m_Account.m_aAsciiFrame[3]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[3]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[3], pResult->m_ClientId);
		}
		else if(pResult->GetInteger(1) == 4)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[4], sizeof(pPlayer->m_Account.m_aAsciiFrame[4]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[4]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[4], pResult->m_ClientId);
		}
		else if(pResult->GetInteger(1) == 5)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[5], sizeof(pPlayer->m_Account.m_aAsciiFrame[5]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[5]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[5], pResult->m_ClientId);
		}
		else if(pResult->GetInteger(1) == 6)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[6], sizeof(pPlayer->m_Account.m_aAsciiFrame[6]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[6]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[6], pResult->m_ClientId);
		}
		else if(pResult->GetInteger(1) == 7)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[7], sizeof(pPlayer->m_Account.m_aAsciiFrame[7]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[7]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[7], pResult->m_ClientId);
		}
		else if(pResult->GetInteger(1) == 8)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[8], sizeof(pPlayer->m_Account.m_aAsciiFrame[8]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[8]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[8], pResult->m_ClientId);
		}
		else if(pResult->GetInteger(1) == 9)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[9], sizeof(pPlayer->m_Account.m_aAsciiFrame[9]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[9]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[9], pResult->m_ClientId);
		}
		else if(pResult->GetInteger(1) == 10)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[10], sizeof(pPlayer->m_Account.m_aAsciiFrame[10]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[10]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[10], pResult->m_ClientId);
		}
		else if(pResult->GetInteger(1) == 11)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[11], sizeof(pPlayer->m_Account.m_aAsciiFrame[11]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[11]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[11], pResult->m_ClientId);
		}
		else if(pResult->GetInteger(1) == 12)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[12], sizeof(pPlayer->m_Account.m_aAsciiFrame[12]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[12]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[12], pResult->m_ClientId);
		}
		else if(pResult->GetInteger(1) == 13)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[13], sizeof(pPlayer->m_Account.m_aAsciiFrame[13]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[13]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[13], pResult->m_ClientId);
		}
		else if(pResult->GetInteger(1) == 14)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[14], sizeof(pPlayer->m_Account.m_aAsciiFrame[14]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[14]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[14], pResult->m_ClientId);
		}
		else if(pResult->GetInteger(1) == 15)
		{
			str_format(pPlayer->m_Account.m_aAsciiFrame[15], sizeof(pPlayer->m_Account.m_aAsciiFrame[15]), "%s", pResult->GetString(2));
			str_format(aBuf, sizeof(aBuf), "updated frame[%d]: %s", pResult->GetInteger(1), pPlayer->m_Account.m_aAsciiFrame[15]);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			pSelf->SendBroadcast(pPlayer->m_Account.m_aAsciiFrame[15], pResult->m_ClientId);
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "'%d' is no valid frame. Choose between 0 and 15.", pResult->GetInteger(1));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "view") || !str_comp_nocase(pResult->GetString(0), "watch"))
	{
		if(pResult->NumArguments() == 1) //show own
		{
			pSelf->StartAsciiAnimation(pResult->m_ClientId, pResult->m_ClientId, -1);
			return;
		}

		pSelf->StartAsciiAnimation(pResult->m_ClientId, pResult->GetInteger(1), 0);
	}
	else if(!str_comp_nocase(pResult->GetString(0), "speed"))
	{
		if(!pPlayer->IsLoggedIn())
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You have to be logged in to create ascii animations.");
			pSelf->SendChatTarget(pResult->m_ClientId, "Use '/accountinfo' for more help about accounts.");
			return;
		}

		if(pResult->NumArguments() != 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Type '/ascii speed <speed>' to change the animation's speed.");
			return;
		}
		if(pResult->GetInteger(1) < 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Animation speed has to be 1 or higher.");
			return;
		}

		pPlayer->m_AsciiAnimSpeed = pResult->GetInteger(1);
		pSelf->SendChatTarget(pResult->m_ClientId, "Updated animation speed.");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Unknown ascii command. Type '/ascii' for command list.");
	}
}

void CGameContext::ConHook(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "===== hook =====");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/hook <power>'");
		pSelf->SendChatTarget(pResult->m_ClientId, "===== powers =====");
		pSelf->SendChatTarget(pResult->m_ClientId, "normal, rainbow, bloody");
		return;
	}

	if(!str_comp_nocase(pResult->GetString(0), "normal"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "You got normal hook.");
		pPlayer->m_HookPower = 0;
	}
	else if(!str_comp_nocase(pResult->GetString(0), "rainbow"))
	{
		if(pPlayer->m_Account.m_IsSuperModerator || pPlayer->m_Account.m_IsModerator)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You got rainbow hook.");
			pPlayer->m_HookPower = 1;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "bloody"))
	{
		if(pPlayer->m_Account.m_IsSuperModerator)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You got bloody hook.");
			pPlayer->m_HookPower = 2;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Unknown power. Type '/hook' for a list of all powers.");
	}
}

void CGameContext::ConReport(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	char aBuf[256];

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "---- Report ----");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/report <reason> <player>'");
		pSelf->SendChatTarget(pResult->m_ClientId, "--- Reasons ---");
		pSelf->SendChatTarget(pResult->m_ClientId, "spawnblock, aimbot, flybot, spinbot, chat-spam, chat-insult");
		return;
	}

	if(pResult->NumArguments() == 2)
	{
		int RepId = pSelf->GetCidByName(pResult->GetString(1));
		if(RepId == -1)
		{
			str_format(aBuf, sizeof(aBuf), "'%s' is not online.", pResult->GetString(1));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			return;
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "use '/report <reason> <player>'");
		return;
	}

	if(!str_comp_nocase(pResult->GetString(0), "spawnblock"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "coming soon...");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "aimbot"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "coming soon...");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "flybot"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "coming soon...");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "spinbot"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "coming soon...");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "chat-spam"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "coming soon...");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "chat-insult"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "coming soon...");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Unknown reason. Check '/report' for all reasons.");
	}
}

void CGameContext::ConTaser(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	if(pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info"))
	{
		if(pPlayer->m_Account.m_TaserLevel == 0)
		{
			pPlayer->m_TaserPrice = 50000;
		}
		else if(pPlayer->m_Account.m_TaserLevel == 1)
		{
			pPlayer->m_TaserPrice = 75000;
		}
		else if(pPlayer->m_Account.m_TaserLevel == 2)
		{
			pPlayer->m_TaserPrice = 100000;
		}
		else if(pPlayer->m_Account.m_TaserLevel == 3)
		{
			pPlayer->m_TaserPrice = 150000;
		}
		else if(pPlayer->m_Account.m_TaserLevel == 4)
		{
			pPlayer->m_TaserPrice = 200000;
		}
		else if(pPlayer->m_Account.m_TaserLevel == 5)
		{
			pPlayer->m_TaserPrice = 200000;
		}
		else if(pPlayer->m_Account.m_TaserLevel == 6)
		{
			pPlayer->m_TaserPrice = 200000;
		}
		else
		{
			pPlayer->m_TaserPrice = 0;
		}

		char aBuf[256];

		pSelf->SendChatTarget(pResult->m_ClientId, "~~~ TASER INFO ~~~");
		pSelf->SendChatTarget(pResult->m_ClientId, "Police with rank 3 or higher are allowed to carry a taser.");
		pSelf->SendChatTarget(pResult->m_ClientId, "Taser makes rifle freeze players.");
		pSelf->SendChatTarget(pResult->m_ClientId, "~~~ YOUR TASER STATS ~~~");
		str_format(aBuf, sizeof(aBuf), "TaserLevel: %d/7", pPlayer->m_Account.m_TaserLevel);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		str_format(aBuf, sizeof(aBuf), "Price for the next level: %d", pPlayer->m_TaserPrice);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		//str_format(aBuf, sizeof(aBuf), "FreezeTime: %.4f seconds", (pPlayer->m_Account.m_TaserLevel * 5) / pSelf->Server()->TickSpeed());
		str_format(aBuf, sizeof(aBuf), "FreezeTime: %.2f seconds", pPlayer->TaserFreezeTime());
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		str_format(aBuf, sizeof(aBuf), "FailRate: %d%%", 0);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		pSelf->SendChatTarget(pResult->m_ClientId, "~~~ TASER COMMANDS ~~~");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/taser <on/off>' to activate/deactivate it.");
		//pSelf->SendChatTarget(pResult->m_ClientId, "'/taser <upgrade>' to level up you taser.");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "on"))
	{
		if(pPlayer->m_Account.m_TaserLevel < 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You don't own a taser.");
			return;
		}

		pSelf->SendChatTarget(pResult->m_ClientId, "Taser activated. (Your rifle is now a taser)");
		pPlayer->m_TaserOn = true;
		return;
	}
	else if(!str_comp_nocase(pResult->GetString(0), "off"))
	{
		if(pPlayer->m_Account.m_TaserLevel < 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "You don't own a taser.");
			return;
		}

		pSelf->SendChatTarget(pResult->m_ClientId, "Taser deactivated. (Your rifle unfreezes again.)");
		pPlayer->m_TaserOn = false;
		return;
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Invalid argument. Try '/taser info' for info about taser.");
	}
}

void CGameContext::ConSpawnWeaponsInfo(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	char aBuf[256];

	pSelf->SendChatTarget(pResult->m_ClientId, "~~~ SPAWN WEAPONS INFO ~~~");
	pSelf->SendChatTarget(pResult->m_ClientId, "You can buy spawn weapons in the '/shop'.");
	pSelf->SendChatTarget(pResult->m_ClientId, "You will have the bought weapon on spawn.");
	pSelf->SendChatTarget(pResult->m_ClientId, "You can have max. 5 bullets per weapon.");
	pSelf->SendChatTarget(pResult->m_ClientId, "Each bullet costs 600.000 money.");
	pSelf->SendChatTarget(pResult->m_ClientId, "~~~ YOUR SPAWN WEAPON STATS ~~~");
	str_format(aBuf, sizeof(aBuf), "Spawn shotgun bullets: %d", pPlayer->m_Account.m_SpawnWeaponShotgun);
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Spawn grenade bullets: %d", pPlayer->m_Account.m_SpawnWeaponGrenade);
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	str_format(aBuf, sizeof(aBuf), "Spawn rifle bullets: %d", pPlayer->m_Account.m_SpawnWeaponRifle);
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	pSelf->SendChatTarget(pResult->m_ClientId, "~~~ SPAWN WEAPON COMMANDS ~~~");
	pSelf->SendChatTarget(pResult->m_ClientId, "'/spawnweapons to activate/deactivate it.");
}

void CGameContext::ConSpookyGhostInfo(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	pSelf->SendChatTarget(pResult->m_ClientId, "~~~ THE SPOOKY GHOST ~~~");
	pSelf->SendChatTarget(pResult->m_ClientId, "You can buy the spooky ghost in the '/shop'.");
	pSelf->SendChatTarget(pResult->m_ClientId, "The spooky ghost costs 1.000.000 money.");
	pSelf->SendChatTarget(pResult->m_ClientId, "If you don't have the ghost skin yet download it from 'https://ddnet.tw/skins/skin/ghost.png' and put it in your skins folder.");
	pSelf->SendChatTarget(pResult->m_ClientId, "~~~ TOGGLE ON AND OFF ~~~");
	pSelf->SendChatTarget(pResult->m_ClientId, "You can activate and deactivate the spooky ghost by");
	pSelf->SendChatTarget(pResult->m_ClientId, "holding TAB and shooting 2 times with the pistol.");
}

void CGameContext::ConAdminChat(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) != AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[ADMIN-CHAT] missing permission to use this command.");
		return;
	}
	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[ADMIN-CHAT] write a chat message that is only visible to admins.");
		return;
	}

	char aMsg[256];
	str_format(aMsg, sizeof(aMsg), "[%s]: %s", pSelf->Server()->ClientName(pResult->m_ClientId), pResult->GetString(0));

	for(auto &Player : pSelf->m_apPlayers)
		if(Player && pSelf->Server()->GetAuthedState(Player->GetCid()) == AUTHED_ADMIN)
			pSelf->SendChatTarget(Player->GetCid(), aMsg);
}

void CGameContext::ConLive(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) != AUTHED_ADMIN && !pPlayer->m_Account.m_IsSupporter)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[LIVE] missing permission to use this command.");
		return;
	}

	char aBuf[128];

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "==== LIVE ====");
		pSelf->SendChatTarget(pResult->m_ClientId, "shows live stats about players");
		pSelf->SendChatTarget(pResult->m_ClientId, "usage: '/live (playername)'");
		return;
	}

	char aLiveName[32];
	str_format(aLiveName, sizeof(aLiveName), "%s", pResult->GetString(0));
	str_format(aBuf, sizeof(aBuf), "==== [LIVE] '%s' ====", aLiveName);
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	int LiveId = pSelf->GetCidByName(aLiveName);
	if(LiveId == -1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Status: OFFLINE");
		return;
	}
	pSelf->SendChatTarget(pResult->m_ClientId, "Status: ONLINE");
	CPlayer *pLive = pSelf->m_apPlayers[LiveId];
	if(!pLive)
		return;

	str_format(aBuf, sizeof(aBuf), "Messages join=%s leave=%s team=%s",
		pSelf->ShowJoinMessage(pLive->GetCid()) ? "shown" : "hidden",
		pSelf->ShowLeaveMessage(pLive->GetCid()) ? "shown" : "hidden",
		pSelf->ShowTeamSwitchMessage(pLive->GetCid()) ? "shown" : "hidden");
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

	if(pSelf->Server()->GetAuthedState(pLive->GetCid()))
	{
		str_format(aBuf, sizeof(aBuf), "Authed: %d", pSelf->Server()->GetAuthedState(pLive->GetCid()));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	}
	if(pLive->IsLoggedIn())
	{
		str_format(aBuf, sizeof(aBuf), "Account: %s", pLive->m_Account.m_aUsername);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		str_format(aBuf, sizeof(aBuf), "AccountId: %d", pLive->GetAccId());
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

		if(!pLive->IsQuesting())
		{
			//pSelf->SendChatTarget(pResult->m_ClientId, "Quest: FALSE"); //useless info
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "Quest: %d (level %d)", pLive->m_QuestState, pLive->m_QuestStateLevel);
			pSelf->SendChatTarget(pPlayer->GetCid(), aBuf);
			str_format(aBuf, sizeof(aBuf), "QuestStr: %s", pLive->m_aQuestString);
			pSelf->SendChatTarget(pPlayer->GetCid(), aBuf);
			if(pLive->m_aQuestProgress[0] != -1 && pLive->m_aQuestProgress[1] != -1)
			{
				str_format(aBuf, sizeof(aBuf), "QuestProgress: %d/%d", pLive->m_aQuestProgress[0], pLive->m_aQuestProgress[1]);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			}
			if(pLive->m_QuestFailed)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "QuestFailed: TRUE");
			}
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Account: NOT LOGGED IN");
	}

	if(!pLive->GetCharacter())
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Char: DEAD");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Char: ALIVE");
		if(pLive->GetCharacter()->Core()->m_DeepFrozen)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Frozen: DEEP");
		}
		else if(pLive->GetCharacter()->isFreezed)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Frozen: TRUE");
		}
		else if(pLive->GetCharacter()->m_FreezeTime)
		{
			str_format(aBuf, sizeof(aBuf), "Frozen: Freezetime: %d", pLive->GetCharacter()->m_FreezeTime);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Frozen: FALSE");
		}

		if(pLive->GetCharacter()->Core()->m_EndlessJump)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "SuperJump: TRUE");
		}
		if(pLive->GetCharacter()->Core()->m_Jetpack)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Jetpack: TRUE");
		}
		if(pLive->GetCharacter()->Core()->m_EndlessHook)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Endless: TRUE");
		}
		pLive->GetCharacter()->BulletAmounts();
		str_format(aBuf, sizeof(aBuf), "Hammer[%d] Gun[%d] Ninja[%d]", pLive->GetCharacter()->HasWeapon(0), pLive->GetCharacter()->m_GunBullets, pLive->GetCharacter()->HasWeapon(5));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		str_format(
			aBuf,
			sizeof(aBuf),
			"Shotgun[%d] Grenade[%d] Rifle[%d]",
			pLive->GetCharacter()->m_ShotgunBullets,
			pLive->GetCharacter()->m_GrenadeBullets,
			pLive->GetCharacter()->m_RifleBullets);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

		int viewers = 0;
		for(auto &Player : pSelf->m_apPlayers)
			if(Player && Player->SpectatorId() == LiveId)
				viewers++;
		if(viewers)
		{
			str_format(aBuf, sizeof(aBuf), "Viewers: %d", viewers);
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
		str_format(aBuf, sizeof(aBuf), "Position: (%.2f/%.2f)", pLive->GetCharacter()->GetPosition().x / 32, pLive->GetCharacter()->GetPosition().y / 32);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	}
}

void CGameContext::ConRegex(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	/*
		Since regex can be used as denial of service attack vector
		it is probably safer to make it staff only command
	*/
	if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) != AUTHED_ADMIN && !pPlayer->m_Account.m_IsSupporter)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[REGEX] missing permission to use this command.");
		return;
	}

	if(pResult->NumArguments() != 2)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "==== Regex ====");
		pSelf->SendChatTarget(pResult->m_ClientId, "Train your POSIX regex skills or test patterns used for anti flood commands.");
		pSelf->SendChatTarget(pResult->m_ClientId, "usage: '/regex \"pattern\" \"string\"'");
		pSelf->SendChatTarget(pResult->m_ClientId, "example: '/regex \"[0-9]\" \"123\"' (match numeric)");
		pSelf->SendChatTarget(pResult->m_ClientId, "example: '/regex \"^[0-9]*x$\" \"123x\"' (match numbers followed by x)");
		return;
	}
#if defined(CONF_FAMILY_UNIX)
	int ret = regex_compile(pResult->GetString(0), pResult->GetString(1));
	if(ret == -1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[REGEX] Error: pattern compile failed.");
		return;
	}
	if(ret == 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[REGEX] (-) pattern does not match.");
		return;
	}
	if(ret == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[REGEX] (+) pattern matches.");
		return;
	}
	pSelf->SendChatTarget(pResult->m_ClientId, "[REGEX] Error: something went horribly wrong.");
#else
	pSelf->SendChatTarget(pResult->m_ClientId, "[REGEX] Error: not supported on your operating system.");
#endif
}

void CGameContext::ConMapsave(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) != AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[ADMIN] Missing permission.");
		return;
	}

	char aCommand[32];
	str_copy(aCommand, pResult->GetString(0), sizeof(aCommand));

	if(pResult->NumArguments() == 0 || !str_comp_nocase(aCommand, "help"))
	{
		pSelf->SendChatTarget(ClientId, "--------  MAPSAVE  ----------");
		pSelf->SendChatTarget(ClientId, "Usage: '/mapsave [save|load|debug|players|check]'");
		pSelf->SendChatTarget(ClientId, "saves/loads state (position etc) of currently ingame players.");
		pSelf->SendChatTarget(ClientId, "Creates a binary file and it loads automatically (works with timeout codes).");
		pSelf->SendChatTarget(ClientId, "The load command is mostly for debugging because it should load from alone.");
		pSelf->SendChatTarget(ClientId, "----------------------------");
	}
	else if(!str_comp_nocase(aCommand, "load"))
	{
		pSelf->SendChatTarget(ClientId, "[MAPSAVE] loading map data...");
		pSelf->LoadMapPlayerData();
	}
	else if(!str_comp_nocase(aCommand, "save"))
	{
		pSelf->SendChatTarget(ClientId, "[MAPSAVE] saving map data...");
		pSelf->SaveMapPlayerData();
		pSelf->m_World.m_Paused = true;
		pSelf->LogoutAllPlayersMessage();
	}
	else if(!str_comp_nocase(aCommand, "debug"))
	{
		pSelf->SendChatTarget(ClientId, "[MAPSAVE] reading map data debug (check logs)...");
		pSelf->ReadMapPlayerData(pResult->m_ClientId);
	}
	else if(!str_comp_nocase(aCommand, "players"))
	{
		pSelf->SendChatTarget(ClientId, "[MAPSAVE] listing player stats check rcon console...");
		for(auto &Player : pSelf->m_apPlayers)
		{
			if(!Player)
				continue;

			char aBuf[128];
			str_format(
				aBuf,
				sizeof(aBuf),
				"%d:'%s' code=%s loaded=%d",
				Player->GetCid(),
				pSelf->Server()->ClientName(Player->GetCid()),
				Player->m_aTimeoutCode,
				Player->m_MapSaveLoaded);
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "chatresp", aBuf);
		}
	}
	else if(!str_comp_nocase(aCommand, "check"))
	{
		int NoCode = 0;
		for(auto &Player : pSelf->m_apPlayers)
		{
			if(!Player)
				continue;
			if(Player->m_aTimeoutCode[0])
				continue;

			NoCode++;
			pSelf->SendChatTarget(Player->GetCid(), "[MAPSAVE] please type '/timeout (code)'.");
			pSelf->SendChatTarget(Player->GetCid(), "[MAPSAVE] the admin wants to restart the server.");
			pSelf->SendChatTarget(Player->GetCid(), "[MAPSAVE] create a code to load and save your stats.");
		}
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "[MAPSAVE] players without code: %d ('/mapsave players').", NoCode);
		pSelf->SendChatTarget(ClientId, aBuf);
	}
	else
	{
		pSelf->SendChatTarget(ClientId, "Usage: '/mapsave [save|load|debug|players|check]'");
	}
}

void CGameContext::ConShow(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	char aBuf[256];

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "====== SHOW ======");
		pSelf->SendChatTarget(pResult->m_ClientId, "Activates broadcasts.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/show <info>' to show an info.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/hide <info>' to hide an info.");
		pSelf->SendChatTarget(pResult->m_ClientId, "=== SHOWABLE INFO ===");
		if(!pPlayer->m_ShowBlockPoints)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "block_points");
		}
		if(pPlayer->m_HideBlockXp)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "block_xp");
		}
		if(!pPlayer->m_xpmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "xp");
		}
		if(pPlayer->m_hidejailmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "jail");
		}
		if(pPlayer->m_HideInsta1on1_killmessages)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "insta_killfeed");
		}
		if(pPlayer->m_HideQuestProgress)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "quest_progress");
		}
		if(pPlayer->m_HideQuestWarning)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "quest_warning");
		}
		if(!pPlayer->m_ShowInstaScoreBroadcast)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "insta_score");
		}
		return;
	}

	if(!str_comp_nocase(pResult->GetString(0), "block_points"))
	{
		if(!pPlayer->m_ShowBlockPoints)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "block_points are now activated.");
			pPlayer->m_ShowBlockPoints = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "block_points are already activated.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "block_xp"))
	{
		if(pPlayer->m_HideBlockXp)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "block_xp are now shown.");
			pPlayer->m_HideBlockXp = false;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "block_xp are already activated.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "xp"))
	{
		if(!pPlayer->m_xpmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "XP-messages are now activated.");
			pPlayer->m_xpmsg = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "XP-messages are already activated.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "jail"))
	{
		if(pPlayer->m_hidejailmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Jail-messages are now shown.");
			pPlayer->m_hidejailmsg = false;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Jail-messages are already shown.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "insta_killfeed"))
	{
		if(pPlayer->m_HideInsta1on1_killmessages)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "instagib kills are now shown.");
			pPlayer->m_HideInsta1on1_killmessages = false;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "instagib kills are already shown.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "quest_progress"))
	{
		if(pPlayer->m_HideQuestProgress)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "quest progress is now shown.");
			pPlayer->m_HideQuestProgress = false;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "quest progress is already shown.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "quest_warning"))
	{
		if(pPlayer->m_HideQuestWarning)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "quest warning is now shown.");
			pPlayer->m_HideQuestWarning = false;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "quest warning is already shown.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "insta_score"))
	{
		if(!pPlayer->m_ShowInstaScoreBroadcast)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "insta score is now shown.");
			pPlayer->m_ShowInstaScoreBroadcast = true;
			if(pPlayer->GetCharacter())
				pPlayer->GetCharacter()->m_UpdateInstaScoreBoard = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "insta score is already shown.");
		}
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "'%s' is not a valid info.", pResult->GetString(0));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	}
}

void CGameContext::ConHide(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	char aBuf[256];
	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "====== HIDE ======");
		pSelf->SendChatTarget(pResult->m_ClientId, "Hides broadcasts.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/hide <info>' to hide an info.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/show <info>' to show an info.");
		pSelf->SendChatTarget(pResult->m_ClientId, "=== HIDEABLE INFO ===");
		if(pPlayer->m_ShowBlockPoints)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "block_points");
		}
		if(!pPlayer->m_HideBlockXp)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "block_xp");
		}
		if(pPlayer->m_xpmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "xp");
		}
		if(!pPlayer->m_hidejailmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "jail");
		}
		if(!pPlayer->m_HideInsta1on1_killmessages)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "insta_killfeed");
		}
		if(!pPlayer->m_HideQuestProgress)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "quest_progress");
		}
		if(!pPlayer->m_HideQuestWarning)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "quest_warning");
		}
		if(pPlayer->m_ShowInstaScoreBroadcast)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "insta_score");
		}
		return;
	}

	if(!str_comp_nocase(pResult->GetString(0), "block_points"))
	{
		if(pPlayer->m_ShowBlockPoints)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "block_points are now hidden.");
			pPlayer->m_ShowBlockPoints = false;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "block_points are already hidden.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "block_xp"))
	{
		if(!pPlayer->m_HideBlockXp)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "block_xp are now hidden.");
			pPlayer->m_HideBlockXp = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "block_xp are already hidden.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "xp"))
	{
		if(pPlayer->m_xpmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "XP-messages are now hidden.");
			pPlayer->m_xpmsg = false;
			pSelf->SendBroadcast("", pPlayer->GetCid(), 0); //send empty broadcast without importance to delete last xp message
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "XP-messages are already hidden.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "jail"))
	{
		if(!pPlayer->m_hidejailmsg)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Jail-messages are now hidden.");
			pPlayer->m_hidejailmsg = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Jail-messages are already hidden.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "insta_killfeed"))
	{
		if(!pPlayer->m_HideInsta1on1_killmessages)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "instagib kills are now hidden.");
			pPlayer->m_HideInsta1on1_killmessages = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "instagib kills are already hidden.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "quest_progress"))
	{
		if(!pPlayer->m_HideQuestProgress)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "quest progress is now hidden.");
			pPlayer->m_HideQuestProgress = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "quest progress is already hidden.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "quest_warning"))
	{
		if(!pPlayer->m_HideQuestWarning)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "quest warning is now hidden.");
			pPlayer->m_HideQuestWarning = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "quest warning is already hidden.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "insta_score"))
	{
		if(pPlayer->m_ShowInstaScoreBroadcast)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "insta score is now hidden.");
			pPlayer->m_ShowInstaScoreBroadcast = false;
			pSelf->SendBroadcast("", pPlayer->GetCid(), 0);
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "insta score is already hidden.");
		}
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "'%s' is not a valid info.", pResult->GetString(0));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	}
}

void CGameContext::ConQuest(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "You have to be alive to use this command.");
		return;
	}

	/*
	if (pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "You have to be logged in to use this command. (type '/accountinfo' for more info)");
		return;
	}
	*/

	char aBuf[128];

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "====== Q U E S T =====");
		if(!pPlayer->IsQuesting())
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "No running quest...");
			pSelf->SendChatTarget(pResult->m_ClientId, "Use '/quest start' to start one.");
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "===QUEST[%d]==LEVEL[%d]===", pPlayer->m_QuestState, pPlayer->m_QuestStateLevel);
			pSelf->SendChatTarget(pPlayer->GetCid(), aBuf);
			pSelf->SendChatTarget(pResult->m_ClientId, pPlayer->m_aQuestString);
			if(pPlayer->m_aQuestProgress[0] != -1 && pPlayer->m_aQuestProgress[1] != -1)
			{
				str_format(aBuf, sizeof(aBuf), "Quest progress %d/%d completed.", pPlayer->m_aQuestProgress[0], pPlayer->m_aQuestProgress[1]);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			}
			if(pPlayer->m_QuestFailed)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "WARNING: Quest has failed. Start again.");
			}
		}
		pSelf->SendChatTarget(pResult->m_ClientId, "========================");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/quest help' for more info.");
		return;
	}

	if(!str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info") || !str_comp_nocase(pResult->GetString(0), "cmdlist") || !str_comp_nocase(pResult->GetString(0), "man") || !str_comp_nocase(pResult->GetString(0), "?"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "====== Q U E S T =====");
		pSelf->SendChatTarget(pResult->m_ClientId, "Complete quests and get rewards.");
		pSelf->SendChatTarget(pResult->m_ClientId, "==== commands ====");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/quest' to get quest status.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/quest start' to start a quest.");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/quest stop' to stop a quest.");
		//pSelf->SendChatTarget(pResult->m_ClientId, "'/quest skip' to skip a quest");
		//pSelf->SendChatTarget(pResult->m_ClientId, "'/quest level' to change difficulty");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "start") || !str_comp_nocase(pResult->GetString(0), "begin"))
	{
		if(pPlayer->IsQuesting())
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Quest is already running.");
			return;
		}
		pSelf->CheckConnectQuestBot();
		// load / activate QuestState
		if(pPlayer->m_QuestUnlocked) // if saved stats --> load stats
		{
			pPlayer->m_QuestState = pPlayer->m_QuestUnlocked;
		}
		else // no saved stats --> start quest1
		{
			pPlayer->m_QuestState = 1;
		}
		// load Quest Level
		pPlayer->m_QuestStateLevel = pPlayer->m_QuestLevelUnlocked;

		pSelf->SendChatTarget(pResult->m_ClientId, "[QUEST] started ..."); // print this before the actual start because the start can drop an error and we dont want this log : "[QUEST] ERROR STOPPED [QUEST] Started.." we want this log: "[QUEST] Started.. [QUEST] ERROR STOPPED"
		pSelf->StartQuest(pResult->m_ClientId);
	}
	else if(!str_comp_nocase(pResult->GetString(0), "stop"))
	{
		if(!pPlayer->IsQuesting())
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "Quest already stopped.");
			return;
		}

		pPlayer->m_QuestState = CPlayer::QUEST_OFF;
		pPlayer->m_QuestStateLevel = 0;
		pSelf->SendChatTarget(pResult->m_ClientId, "[QUEST] Quest stopped.");
	}
	//else if (!str_comp_nocase(pResult->GetString(0), "level"))
	//{
	//	if (pPlayer->m_QuestLevelUnlocked < pResult->GetInteger(1))
	//	{
	//		str_format(aBuf, sizeof(aBuf), "Unlock this level first by completing level %d.", pResult->GetInteger(1) - 1);
	//		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	//		return;
	//	}
	//	if (pResult->GetInteger(1) < 0)
	//	{
	//		pSelf->SendChatTarget(pResult->m_ClientId, "You are to unexperienced to play these kind of quests...");
	//		return;
	//	}

	//	pPlayer->m_QuestState = pResult->GetInteger(1);
	//	str_format(aBuf, sizeof(aBuf), "Updated quest difficulty to %d.", pResult->GetInteger(1));
	//	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	//}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Unknown quest command. Type '/quest help' for more info.");
	}
}

void CGameContext::ConBounty(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	char aBuf[128];

	if(pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "----> Bounty <----");
		pSelf->SendChatTarget(pResult->m_ClientId, "Use the bounty command to pay money in a pricepool.");
		pSelf->SendChatTarget(pResult->m_ClientId, "And then choose a player.");
		pSelf->SendChatTarget(pResult->m_ClientId, "And the tee who blocks your chosen player");
		pSelf->SendChatTarget(pResult->m_ClientId, "Gets all the money from the pool.");
		pSelf->SendChatTarget(pResult->m_ClientId, "write '/bounty cmdlist' for all commands");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "==== BOUNTY COMMANDS ====");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bounty cmdlist' shows this list");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bounty help' shows some general info");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bounty add <amount> <player>' to add a bounty to a player");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/bounty check <clientId>' to check the total bounty amount");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "add"))
	{
		if(pResult->NumArguments() == 3)
		{
			if(pPlayer->GetMoney() < pResult->GetInteger(1))
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[BOUNTY] You don't have that much money.");
				return;
			}

			if(pResult->GetInteger(1) < 200)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[BOUNTY] Minimum amount for bountys is 200.");
				return;
			}

			int Id = pSelf->GetCidByName(pResult->GetString(2));
			if(Id == -1)
			{
				str_format(aBuf, sizeof(aBuf), "[BOUNTY] Player '%s' not found.", pResult->GetString(2));
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			}
			else if(pSelf->IsSameIp(Id, pResult->m_ClientId))
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[BOUNTY] You can't add bounty to your dummy.");
			}
			else if(pResult->GetInteger(1) < 1)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[BOUNTY] You can't add less than 1 money as bounty.");
			}
			else
			{
				pSelf->m_apPlayers[Id]->m_BlockBounty += pResult->GetInteger(1);
				str_format(aBuf, sizeof(aBuf), "-%d bounty (%s)", pResult->GetInteger(1), pSelf->Server()->ClientName(Id));
				pPlayer->MoneyTransaction(-pResult->GetInteger(1), aBuf);
				str_format(aBuf, sizeof(aBuf), "[BOUNTY] added %d money to '%s' (%d total)", pResult->GetInteger(1), pSelf->Server()->ClientName(Id), pSelf->m_apPlayers[Id]->m_BlockBounty);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
				str_format(aBuf, sizeof(aBuf), "[BOUNTY] '%s' added %d money to '%s's bounty (%d total)", pSelf->Server()->ClientName(pResult->m_ClientId), pResult->GetInteger(1), pSelf->Server()->ClientName(Id), pSelf->m_apPlayers[Id]->m_BlockBounty);
				pSelf->SendChat(-1, TEAM_ALL, aBuf);
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BOUNTY] Use: '/bounty add <amount> <player>'");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "check"))
	{
		if(pResult->NumArguments() == 2)
		{
			int Id = -1;
			for(auto &Player : pSelf->m_apPlayers)
			{
				if(Player)
				{
					if(Player->GetCid() == pResult->GetInteger(1))
					{
						Id = Player->GetCid();
						break;
					}
				}
			}

			if(Id == -1)
			{
				str_format(aBuf, sizeof(aBuf), "[BOUNTY] Player with id %d not found.", pResult->GetInteger(1));
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "[BOUNTY] '%s' has a bounty of %d money", pSelf->Server()->ClientName(Id), pSelf->m_apPlayers[Id]->m_BlockBounty);
				pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			}
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BOUNTY] Use: '/bounty check <clientId>'");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[BOUNTY] Unknown command. write '/bounty cmdlist' for all commands");
	}
}

void CGameContext::ConDcDummy(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;
	if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) != AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[dummy] You have to be admin to use this command.");
		return;
	}

	int DummyId = pResult->GetInteger(0);
	char aBuf[128];
	CPlayer *pDummy = pSelf->m_apPlayers[DummyId];
	if(!pDummy)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[dummy] this id is not online.");
		return;
	}
	if(!pDummy->m_IsDummy)
	{
		str_format(aBuf, sizeof(aBuf), "[dummy] player '%s' is not a dummy.", pSelf->Server()->ClientName(DummyId));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		return;
	}
	str_format(aBuf, sizeof(aBuf), "[dummy] dummy '%s' disconnected.", pSelf->Server()->ClientName(DummyId));
	pSelf->Server()->BotLeave(DummyId);
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
}

void CGameContext::ConTROLL166(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(!pSelf->Server()->GetAuthedState(pResult->m_ClientId))
		return;

	int VictimCid = pResult->GetVictim();
	CPlayer *pPlayer = pSelf->m_apPlayers[VictimCid];
	if(pPlayer)
	{
		pPlayer->m_TROLL166 = pResult->GetInteger(0);
	}
}

void CGameContext::ConTROLL420(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	if(!pSelf->Server()->GetAuthedState(pResult->m_ClientId))
		return;

	int VictimCid = pResult->GetVictim();
	CPlayer *pPlayer = pSelf->m_apPlayers[VictimCid];
	if(pPlayer)
	{
		pPlayer->m_TROLL420 = pResult->GetInteger(0);
		pSelf->SendTuningParams(VictimCid);
	}
}

void CGameContext::ConTrade(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[TRADE] you have to be alive to use this command.");
		return;
	}

	if(pPlayer->m_SpookyGhostActive)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[TRADE] you can't trade as the spooky ghost.");
		return;
	}

	if(!g_Config.m_SvAllowTrade)
	{
		pSelf->SendChatLocSys(pResult->m_ClientId, "TRADE", "%s", "this command is deactivated by an administrator.");
		return;
	}

	if(pSelf->IsMinigame(pResult->m_ClientId))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[TRADE] you can't use this command in minigames or jail.");
		return;
	}

	char aBuf[256];

	if(pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== TRADE ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "Use this command to trade");
		pSelf->SendChatTarget(pResult->m_ClientId, "Weapons and items");
		pSelf->SendChatTarget(pResult->m_ClientId, "With other players on the server");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/trade cmdlist' for all commands");
		if(str_comp_nocase(pPlayer->m_aTradeOffer, "")) //not empty trade offer
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "=== latest trade ===");
			pSelf->SendChatTarget(pResult->m_ClientId, pPlayer->m_aTradeOffer);
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "==== TRADE commands ====");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/trade sell <item> <price> <player>' to send a player a trade offer");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/trade sell_public <item> <price>' to create a public sell offer everybody could accept");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/trade buy <item> <price> <player>' to accept a trade offer");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/tr <player>' fast accept latest trade (WARNING YOU COULD BE SCAMMED!)");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/trade items' for a full list of tradable items");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/trade cmdlist' shows this list");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "items"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== TRADE ITEMS ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "shotgun");
		pSelf->SendChatTarget(pResult->m_ClientId, "grenade");
		pSelf->SendChatTarget(pResult->m_ClientId, "rifle");
		pSelf->SendChatTarget(pResult->m_ClientId, "all_weapons");
		//pSelf->SendChatTarget(pResult->m_ClientId, "homing missiles ammo"); //coming soon...
	}
	else if(!str_comp_nocase(pResult->GetString(0), "sell_public"))
	{
		char aWeaponName[64];
		int Weapon = pSelf->TradeItemToInt(pResult->GetString(1));
		int Price = pResult->GetInteger(2);
		str_copy(aWeaponName, pResult->GetString(1), sizeof(aWeaponName));

		int TradeId = pSelf->TradePrepareSell(/*pToName=*/pResult->GetString(3), /*FromId=*/pResult->m_ClientId, aWeaponName, Price, /*IsPublic=*/true);
		if(TradeId == -1)
		{
			return;
		}

		//send trade info to the invited player
		str_format(aBuf, sizeof(aBuf), "[TRADE] '%s' created a public offer [ %s ] for [ %d ] money (use '/trade' command to accept it)", pSelf->Server()->ClientName(pResult->m_ClientId), aWeaponName, pResult->GetInteger(2));
		pSelf->SendChat(-1, TEAM_ALL, aBuf);
		//and save it for all players so it can be seen later in '/trade'
		for(auto &Player : pSelf->m_apPlayers)
		{
			if(!Player)
				continue;
			if(Player->GetCid() == pResult->m_ClientId) // don't offer own trade
				continue;

			str_format(
				Player->m_aTradeOffer,
				sizeof(Player->m_aTradeOffer),
				"[OPEN] '/trade buy %s %d %s'",
				aWeaponName,
				pResult->GetInteger(2),
				pSelf->Server()->ClientName(pResult->m_ClientId));
		}

		//send same info to the trading dude (he gets the public message)
		//str_format(aBuf, sizeof(aBuf), "[TRADE] you offered to all players [ %s ] for [ %d ] money", aWeaponName, pResult->GetInteger(2));
		//pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

		//save trade to vars SELLER
		pPlayer->m_TradeItem = Weapon;
		pPlayer->m_TradeMoney = pResult->GetInteger(2);
		pPlayer->m_TradeId = -1;
		pPlayer->m_TradeTick = pSelf->Server()->Tick() + pSelf->Server()->TickSpeed() * 2 * 60;
	}
	else if(!str_comp_nocase(pResult->GetString(0), "sell"))
	{
		char aWeaponName[64];
		int Weapon = pSelf->TradeItemToInt(pResult->GetString(1));
		int Price = pResult->GetInteger(2);
		str_copy(aWeaponName, pResult->GetString(1), sizeof(aWeaponName));

		int TradeId = pSelf->TradePrepareSell(/*pToName=*/pResult->GetString(3), /*FromId=*/pResult->m_ClientId, aWeaponName, Price, /*IsPublic=*/false);
		if(TradeId == -1)
		{
			return;
		}

		//send trade info to the invited player
		str_format(aBuf, sizeof(aBuf), "[TRADE] '%s' offered you [ %s ] for [ %d ] money", pSelf->Server()->ClientName(pResult->m_ClientId), aWeaponName, pResult->GetInteger(2));
		pSelf->SendChatTarget(TradeId, aBuf);
		//and save it so it can be seen later in '/trade'
		str_format(pSelf->m_apPlayers[TradeId]->m_aTradeOffer, sizeof(pSelf->m_apPlayers[TradeId]->m_aTradeOffer), "[OPEN] '/trade buy %s %d %s'", aWeaponName, pResult->GetInteger(2), pSelf->Server()->ClientName(pResult->m_ClientId));

		//send same info to the trading dude
		str_format(aBuf, sizeof(aBuf), "[TRADE] you offered '%s' [ %s ] for [ %d ] money", pResult->GetString(3), aWeaponName, pResult->GetInteger(2));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);

		//save trade to vars SELLER
		pPlayer->m_TradeItem = Weapon;
		pPlayer->m_TradeMoney = pResult->GetInteger(2);
		pPlayer->m_TradeId = TradeId;
		pPlayer->m_TradeTick = pSelf->Server()->Tick() + pSelf->Server()->TickSpeed() * 1 * 60;

		//save trade to vars BUYER (makes no sense on multiple offers to one person. better check the command parameters of the buyer)
		//pSelf->m_apPlayers[TradeId]->m_TradeItem = weapon;
		//pSelf->m_apPlayers[TradeId]->m_TradeMoney = pResult->GetInteger(2);
	}
	else if(!str_comp_nocase(pResult->GetString(0), "buy"))
	{
		int TradeId = pSelf->GetCidByName(pResult->GetString(3));
		int Weapon = pSelf->TradeItemToInt(pResult->GetString(1));

		if(pSelf->TradePrepareBuy(pResult->m_ClientId, pResult->GetString(3), Weapon))
		{
			return;
		}

		if(pSelf->m_apPlayers[TradeId]->m_TradeItem != Weapon ||
			pSelf->m_apPlayers[TradeId]->m_TradeMoney != pResult->GetInteger(2))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[TRADE] the trade you accepted doesn't exist.");
			return;
		}

		//##############
		// TRADE SUCCESS
		//##############

		//buyer
		str_format(aBuf, sizeof(aBuf), "[TRADE] you successfully bought [ %s ] for [ %d ] from player '%s'.", pResult->GetString(1), pSelf->m_apPlayers[TradeId]->m_TradeMoney, pSelf->Server()->ClientName(TradeId));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		str_format(aBuf, sizeof(aBuf), "trade [%s] from [%s]", pResult->GetString(1), pSelf->Server()->ClientName(TradeId));
		pPlayer->MoneyTransaction(-pSelf->m_apPlayers[TradeId]->m_TradeMoney, aBuf);
		pPlayer->m_TradeItem = -1;
		pPlayer->m_TradeMoney = -1;
		pPlayer->m_TradeId = -1;
		str_copy(pPlayer->m_aTradeOffer, "", sizeof(pPlayer->m_aTradeOffer));
		if(Weapon == 2)
		{
			pChr->GiveWeapon(2);
			pPlayer->m_SpawnShotgunActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_SHOTGUN] = false;
		}
		if(Weapon == 3)
		{
			pChr->GiveWeapon(3);
			pPlayer->m_SpawnGrenadeActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_GRENADE] = false;
		}
		if(Weapon == 4)
		{
			pChr->GiveWeapon(4);
			pPlayer->m_SpawnRifleActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_LASER] = false;
		}
		else if(Weapon == 5)
		{
			pChr->GiveWeapon(2);
			pChr->GiveWeapon(3);
			pChr->GiveWeapon(4);
			pPlayer->m_SpawnShotgunActive = 0;
			pPlayer->m_SpawnGrenadeActive = 0;
			pPlayer->m_SpawnRifleActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_LASER] = false;
			pChr->m_aDecreaseAmmo[WEAPON_GRENADE] = false;
			pChr->m_aDecreaseAmmo[WEAPON_SHOTGUN] = false;
		}

		//seller
		str_format(aBuf, sizeof(aBuf), "[TRADE] you successfully sold [ %s ] for [ %d ] to player '%s'.", pResult->GetString(1), pSelf->m_apPlayers[TradeId]->m_TradeMoney, pSelf->Server()->ClientName(pPlayer->GetCid()));
		pSelf->SendChatTarget(TradeId, aBuf);
		str_format(aBuf, sizeof(aBuf), "trade [%s] to [%s]", pResult->GetString(1), pSelf->Server()->ClientName(pPlayer->GetCid()));
		pSelf->m_apPlayers[TradeId]->MoneyTransaction(+pSelf->m_apPlayers[TradeId]->m_TradeMoney, aBuf);
		pSelf->m_apPlayers[TradeId]->m_TradeItem = -1;
		pSelf->m_apPlayers[TradeId]->m_TradeMoney = -1;
		pSelf->m_apPlayers[TradeId]->m_TradeId = -1;
		if(Weapon == 2 || Weapon == 3 || Weapon == 4)
		{
			pSelf->m_apPlayers[TradeId]->GetCharacter()->SetActiveWeapon(WEAPON_GUN);
			pSelf->m_apPlayers[TradeId]->GetCharacter()->SetWeaponGot(Weapon, false);
		}
		else if(Weapon == 5)
		{
			pSelf->m_apPlayers[TradeId]->GetCharacter()->SetActiveWeapon(WEAPON_GUN);
			pSelf->m_apPlayers[TradeId]->GetCharacter()->SetWeaponGot(2, false);
			pSelf->m_apPlayers[TradeId]->GetCharacter()->SetWeaponGot(3, false);
			pSelf->m_apPlayers[TradeId]->GetCharacter()->SetWeaponGot(4, false);
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[TRADE] unknown command. try '/trade cmdlist' for more help.");
	}
}

void CGameContext::ConTr(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[TRADE] you have to be alive to use this command.");
		return;
	}

	if(pPlayer->m_SpookyGhostActive)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[TRADE] you can't trade as the spooky ghost.");
		return;
	}

	if(!g_Config.m_SvAllowTrade)
	{
		pSelf->SendChatLocSys(pResult->m_ClientId, "TRADE", "%s", "this command is deactivated by an administrator.");
		return;
	}

	if(pSelf->IsMinigame(pResult->m_ClientId))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[TRADE] you can't use this command in minigames or jail.");
		return;
	}

	char aBuf[256];

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== TRADE (UNSAFE VERSION) ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "For information and the saver version");
		pSelf->SendChatTarget(pResult->m_ClientId, "use the '/trade' command");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/tr <player>' to accept the latest offer from the player");
		pSelf->SendChatTarget(pResult->m_ClientId, "WARNING the player could send a new offer with higher costs!");
		if(str_comp_nocase(pPlayer->m_aTradeOffer, "")) //not empty trade offer
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "=== latest trade ===");
			pSelf->SendChatTarget(pResult->m_ClientId, pPlayer->m_aTradeOffer);
		}
	}
	else if(pResult->NumArguments() == 1)
	{
		int TradeId = pSelf->GetCidByName(pResult->GetString(0));
		if(TradeId == -1)
		{
			str_format(aBuf, sizeof(aBuf), "[TRADE] player '%s' is not online.", pResult->GetString(0));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			return;
		}
		int Weapon = pSelf->m_apPlayers[TradeId] ? pSelf->m_apPlayers[TradeId]->m_TradeItem : -1;
		char aWeaponName[64]; //calculate trading string
		str_copy(aWeaponName, pSelf->TradeItemToStr(Weapon), sizeof(aWeaponName));

		if(pSelf->TradePrepareBuy(pResult->m_ClientId, pResult->GetString(0), Weapon))
		{
			return;
		}

		if(pSelf->m_apPlayers[TradeId]->m_TradeMoney > 5000)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[TRADE] use the '/trade' command for trades with over 5000 money.");
			return;
		}

		//##############
		// TRADE SUCCESS
		//##############

		//buyer
		str_format(aBuf, sizeof(aBuf), "[TRADE] you successfully bought [ %s ] for [ %d ] from player '%s'.", aWeaponName, pSelf->m_apPlayers[TradeId]->m_TradeMoney, pSelf->Server()->ClientName(TradeId));
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		str_format(aBuf, sizeof(aBuf), "trade [%s] from [%s]", aWeaponName, pSelf->Server()->ClientName(TradeId));
		pPlayer->MoneyTransaction(-pSelf->m_apPlayers[TradeId]->m_TradeMoney, aBuf);
		pPlayer->m_TradeItem = -1;
		pPlayer->m_TradeMoney = -1;
		pPlayer->m_TradeId = -1;
		str_copy(pPlayer->m_aTradeOffer, "", sizeof(pPlayer->m_aTradeOffer));
		if(Weapon == 2)
		{
			pChr->GiveWeapon(2);
			pPlayer->m_SpawnShotgunActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_SHOTGUN] = false;
		}
		if(Weapon == 3)
		{
			pChr->GiveWeapon(3);
			pPlayer->m_SpawnGrenadeActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_GRENADE] = false;
		}
		if(Weapon == 4)
		{
			pChr->GiveWeapon(4);
			pPlayer->m_SpawnRifleActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_LASER] = false;
		}
		else if(Weapon == 5)
		{
			pChr->GiveWeapon(2);
			pChr->GiveWeapon(3);
			pChr->GiveWeapon(4);
			pPlayer->m_SpawnShotgunActive = 0;
			pPlayer->m_SpawnGrenadeActive = 0;
			pPlayer->m_SpawnRifleActive = 0;
			pChr->m_aDecreaseAmmo[WEAPON_LASER] = false;
			pChr->m_aDecreaseAmmo[WEAPON_GRENADE] = false;
			pChr->m_aDecreaseAmmo[WEAPON_SHOTGUN] = false;
		}

		//seller
		str_format(aBuf, sizeof(aBuf), "[TRADE] you successfully sold [ %s ] for [ %d ] to player '%s'.", aWeaponName, pSelf->m_apPlayers[TradeId]->m_TradeMoney, pSelf->Server()->ClientName(pPlayer->GetCid()));
		pSelf->SendChatTarget(TradeId, aBuf);
		str_format(aBuf, sizeof(aBuf), "trade [%s] to [%s]", aWeaponName, pSelf->Server()->ClientName(pPlayer->GetCid()));
		pSelf->m_apPlayers[TradeId]->MoneyTransaction(+pSelf->m_apPlayers[TradeId]->m_TradeMoney, aBuf);
		pSelf->m_apPlayers[TradeId]->m_TradeItem = -1;
		pSelf->m_apPlayers[TradeId]->m_TradeMoney = -1;
		pSelf->m_apPlayers[TradeId]->m_TradeId = -1;
		if(Weapon == 2 || Weapon == 3 || Weapon == 4)
		{
			pSelf->m_apPlayers[TradeId]->GetCharacter()->SetActiveWeapon(WEAPON_GUN);
			pSelf->m_apPlayers[TradeId]->GetCharacter()->SetWeaponGot(Weapon, false);
		}
		else if(Weapon == 5)
		{
			pSelf->m_apPlayers[TradeId]->GetCharacter()->SetActiveWeapon(WEAPON_GUN);
			pSelf->m_apPlayers[TradeId]->GetCharacter()->SetWeaponGot(2, false);
			pSelf->m_apPlayers[TradeId]->GetCharacter()->SetWeaponGot(3, false);
			pSelf->m_apPlayers[TradeId]->GetCharacter()->SetWeaponGot(4, false);
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[TRADE] something really went wrong please contact an administrator.");
	}
}

void CGameContext::ConBlockWave(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[BlockWave] you have to be alive to use this command.");
		return;
	}

	if(!g_Config.m_SvAllowBlockWave)
	{
		pSelf->SendChatLocSys(pResult->m_ClientId, "BlockWave", "%s", "this command is deactivated by an administrator.");
		return;
	}

	if(!str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info") || pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== Block Wave ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "Block minigame by ChillerDragon.");
		pSelf->SendChatTarget(pResult->m_ClientId, "Survive waves of blocker bots.");
		pSelf->SendChatTarget(pResult->m_ClientId, "start with '/blockwave join'");
		pSelf->SendChatTarget(pResult->m_ClientId, "check all cmds with ''/blockwave cmdlist'");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== BlockWave cmds ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/blockwave help' for info and help");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/blockwave cmdlist' to show this list");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/blockwave join' to join the game");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/blockwave leave' to leave the game");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/blockwave status' to show cureent game status");
		//pSelf->SendChatTarget(pResult->m_ClientId, "'/blockwave stats' to check your stats"); //coming soon...
		//pSelf->SendChatTarget(pResult->m_ClientId, "'/blockwave shop' to show list of items"); //coming soon...
		//pSelf->SendChatTarget(pResult->m_ClientId, "'/blockwave buy <item>' to buy shop items"); //coming soon...
	}
	else if(!str_comp_nocase(pResult->GetString(0), "status"))
	{
		if(!pSelf->m_BlockWaveGameState)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BlockWave] No game running right now. Feel free to create one with '/blockwave join'");
		}
		else if(pSelf->m_BlockWaveGameState == 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BlockWave] Game starting right now. Feel free to join with '/blockwave join'");
		}
		else if(pSelf->m_BlockWaveGameState == 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BlockWave] Game running right now. Feel free to join with '/blockwave join'");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BlockWave] unknown status.");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "join"))
	{
		if(pPlayer->m_IsBlockWaving)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BlockWave] you already joined a game.");
			return;
		}

		if(pSelf->m_BlockWaveGameState == 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BlockWave] round running... you will join automatically when a new round starts.");
			pPlayer->m_IsBlockWaving = true;
			pPlayer->m_IsBlockWaveWaiting = true;
		}
		else if(pSelf->IsMinigame(pResult->m_ClientId))
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BlockWave] error. maybe you are already in a minigame or jail. (check '/minigames status')");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BlockWave] joined the arena! hf & gl staying alive.");
			pPlayer->m_IsBlockWaving = true;
			pPlayer->m_IsBlockWaveWaiting = false;
			if(!pSelf->m_BlockWaveGameState) //no game? --> start one
			{
				pSelf->StartBlockWaveGame();
			}
			pChr->Die(pPlayer->GetCid(), WEAPON_SELF);
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "leave"))
	{
		if(pPlayer->m_IsBlockWaving)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BlockWave] you left the game.");
			pPlayer->m_IsBlockWaving = false;
			pChr->Die(pPlayer->GetCid(), WEAPON_SELF);
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[BlockWave] you currently aren't playing BlockWave.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[BlockWave] unknown parameter. check '/blockwave cmdlist'");
	}
}

void CGameContext::ConLeave(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int ClientId = pResult->m_ClientId;
	if(!CheckClientId(ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	for(CMinigame *pMinigame : pSelf->m_vMinigames)
		if(pMinigame->IsActive(ClientId))
			if(pMinigame->OnChatCmdLeave(pPlayer))
				return; // can only leave one minigame at a time

	// TODO: move to minigame callback
	if(pPlayer->m_IsBlockDeathmatch)
	{
		pSelf->SendChatTarget(ClientId, "[BLOCK] you left the deathmatch arena!");
		pSelf->SendChatTarget(ClientId, "[BLOCK] now kys :p");
		pPlayer->m_IsBlockDeathmatch = false;
		return;
	}

	// TODO: remove this message but first all minigames need /leave support
	pSelf->SendChatTarget(ClientId, "leave what? xd");
	pSelf->SendChatTarget(ClientId, "Do you want to leave the minigame you are playing?");
	pSelf->SendChatTarget(ClientId, "then type '/<minigame> leave'");
	pSelf->SendChatTarget(ClientId, "check '/minigames status' for the minigame command you need");
}

void CGameContext::ConOneVsOneBlock(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int ClientId = pResult->m_ClientId;
	if(!CheckClientId(ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	pSelf->m_pOneVsOneBlock->OnChatCmdInvite(pPlayer, pResult->GetString(0));
}

void CGameContext::ConTdm(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	int ClientId = pResult->m_ClientId;
	if(!CheckClientId(ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	pSelf->m_pTdmBlock->OnChatCmdTdm(pPlayer);
}

void CGameContext::ConBroadcastServer(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	if(!pPlayer->m_Account.m_IsSuperModerator)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission. You are not a VIP+.");
		return;
	}

	/*
	if (pSelf->m_iBroadcastDelay)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[broadcast_srv] could't sent broadcast because someone received an important broadcast lately");
		return;
	}
	*/

	//str_format(pSelf->aBroadcastMSG, sizeof(pSelf->aBroadcastMSG), pResult->GetString(0));
	pSelf->SendBroadcastAll(pResult->GetString(0), 1, true); //send as important broadcast
}

void CGameContext::ConLaserText(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	if(!pPlayer->m_Account.m_IsSuperModerator)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[lasertext] Missing permission. You are not a VIP+.");
		return;
	}

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[lasertext] you have to be alive to run this command");
		return;
	}
	new CLaserText(
		&pSelf->m_World,
		vec2(pChr->GetPos().x, pChr->GetPos().y - (5 * 32)),
		pSelf->Server()->TickSpeed() * 3,
		pResult->GetString(0));
}

void CGameContext::ConFng(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientId];
	if(!pPlayer)
		return;

	if(pResult->NumArguments() == 0 || !str_comp_nocase(pResult->GetString(0), "help") || !str_comp_nocase(pResult->GetString(0), "info"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== FNG INFO ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "Configurate some settings for the fng minigame.");
		pSelf->SendChatTarget(pResult->m_ClientId, "Use '/insta fng' or '/insta boomfng' to play fng.");
		pSelf->SendChatTarget(pResult->m_ClientId, "For all possible settings check '/fng cmdlist'");
	}
	else if(!str_comp_nocase(pResult->GetString(0), "cmdlist"))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== FNG SETTINGS ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/fng autojoin <value>' 0=off 1=join fng 2=join boomfng on login");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/fng hammertune <value>' 0=vanilla 1=fng");
		if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) == AUTHED_ADMIN)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "'/fng bots <amount> <mode 4/5>' to connect bots for 4=grenade 5=rifle");
		}
	}
	else if(!str_comp_nocase(pResult->GetString(0), "bots"))
	{
		if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) != AUTHED_ADMIN)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[FNG] missing permission.");
			return;
		}
		if(pResult->NumArguments() != 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientId, "[FNG] 3 arguments required.");
			return;
		}
		if(pResult->GetInteger(2) == 4)
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "[FNG] connecting %d grenade bots", pResult->GetInteger(1));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
		else if(pResult->GetInteger(2) == 5)
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "[FNG] connecting %d rifle bots", pResult->GetInteger(1));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		}
		else
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "[FNG] %d is an unsupported mode (choose between 4 and 5)", pResult->GetInteger(2));
			pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
			return;
		}
		pSelf->ConnectFngBots(pResult->GetInteger(1), pResult->GetInteger(2));
	}
	else if(!str_comp_nocase(pResult->GetString(0), "autojoin"))
	{
		if(pResult->NumArguments() > 1)
		{
			if(pResult->GetInteger(1) == 0)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[FNG] you are no longer joining games on account login.");
				pPlayer->m_Account.m_aFngConfig[0] = '0';
				return;
			}
			else if(pResult->GetInteger(1) == 1)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[FNG] you are now automatically joining fng on account login.");
				pPlayer->m_Account.m_aFngConfig[0] = '1';
				return;
			}
			else if(pResult->GetInteger(1) == 2)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[FNG] you are now automatically joining boomfng on account login.");
				pPlayer->m_Account.m_aFngConfig[0] = '2';
				return;
			}
		}
		pSelf->SendChatTarget(pResult->m_ClientId, "[FNG] use '/fng autojoin <value>' and <value> has to be between 0 and 2");
		return;
	}
	else if(!str_comp_nocase(pResult->GetString(0), "hammertune"))
	{
		if(pResult->NumArguments() > 1)
		{
			if(pResult->GetInteger(1) == 0)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[FNG] you are now using VANILLA hammer.");
				pPlayer->m_Account.m_aFngConfig[1] = '0';
				return;
			}
			else if(pResult->GetInteger(1) == 1)
			{
				pSelf->SendChatTarget(pResult->m_ClientId, "[FNG] you are now using FNG hammer.");
				pPlayer->m_Account.m_aFngConfig[1] = '1';
				return;
			}
		}
		pSelf->SendChatTarget(pResult->m_ClientId, "[FNG] use '/fng hammertune <value>' and <value> has to be 1 or 0");
		return;
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[FNG] unknown command check '/fng help' or '/fng cmdlist'.");
	}
}

void CGameContext::ConSqlLogout(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) == AUTHED_ADMIN)
	{
		//admins are allowed
	}
	else
	{
		if(g_Config.m_SvSupAccReset > 0) // 1 or 2 are allowed to use this
		{
			if(pPlayer->m_Account.m_IsSupporter)
			{
				//supporters are allowed
			}
			else
			{
				pSelf->SendChatTarget(ClientId, "[SQL] Missing permission.");
				return;
			}
		}
		else
		{
			pSelf->SendChatTarget(ClientId, "[SQL] Missing permission.");
			return;
		}
	}

	if(pResult->NumArguments() == 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "=== SQL logout ===");
		pSelf->SendChatTarget(pResult->m_ClientId, "'/sql_logout <account_name>' to set acc logged out in db");
		pSelf->SendChatTarget(pResult->m_ClientId, "WARNING!!! this command can mess things up!");
		pSelf->SendChatTarget(pResult->m_ClientId, "try '/sql_logout_all' first because it is more save.");
		return;
	}

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "[SQL] UPDATE Accounts SET IsLoggedIn = 0 WHERE Username = '%s'", pResult->GetString(0));
	pSelf->SendChatTarget(ClientId, aBuf);
	pSelf->m_pAccounts->LogoutUsername(pResult->GetString(0));
}

void CGameContext::ConSqlLogoutAll(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) == AUTHED_ADMIN)
	{
		//admins are allowed
	}
	else
	{
		if(g_Config.m_SvSupAccReset == 2)
		{
			if(pPlayer->m_Account.m_IsSupporter)
			{
				//supporters are allowed
			}
			else
			{
				pSelf->SendChatTarget(ClientId, "[SQL] Missing permission.");
				return;
			}
		}
		else
		{
			pSelf->SendChatTarget(ClientId, "[SQL] Missing permission.");
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

	pSelf->SQLcleanZombieAccounts(ClientId);
}

void CGameContext::ConWanted(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	char aBuf[128];
	int Gangster = 0;

	pSelf->SendChatTarget(pResult->m_ClientId, "=== Wanted Players ===");
	for(auto &Player : pSelf->m_apPlayers)
	{
		if(!Player)
			continue;
		if(!Player->m_Account.m_EscapeTime)
			continue;

		Gangster++;
		str_format(
			aBuf,
			sizeof(aBuf),
			"'%s' reason [%s] seconds [%" PRId64 "]",
			pSelf->Server()->ClientName(Player->GetCid()),
			Player->m_aEscapeReason,
			Player->m_Account.m_EscapeTime / pSelf->Server()->TickSpeed());
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
	}
	str_format(aBuf, sizeof(aBuf), "=== %d gangster wanted ===", Gangster);
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
}

void CGameContext::ConViewers(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	char aBuf[32];
	char aMsg[128];
	int viewers = 0;

	for(auto &Player : pSelf->m_apPlayers)
	{
		if(Player && Player->SpectatorId() == pResult->m_ClientId)
		{
			viewers++;
			if(viewers == 1)
			{
				str_format(aMsg, sizeof(aMsg), "'%s'", pSelf->Server()->ClientName(Player->GetCid()));
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), ", '%s'", pSelf->Server()->ClientName(Player->GetCid()));
				str_append(aMsg, aBuf, sizeof(aMsg));
			}
		}
	}

	if(viewers)
	{
		str_format(aBuf, sizeof(aBuf), "You have [%d] fangrills:", viewers);
		pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
		pSelf->SendChatTarget(pResult->m_ClientId, aMsg);
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "nobody is watching u ._.");
	}
}

void CGameContext::ConIp(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	const NETADDR *pAddr = pSelf->Server()->ClientAddr(ClientId);
	char aAddrStr[NETADDR_MAXSTRSIZE];
	net_addr_str(pAddr, aAddrStr, sizeof(aAddrStr), true);
	char aBuf[32];
	str_format(aBuf, sizeof(aBuf), "your ip: %s", aAddrStr);
	pSelf->SendChatTarget(pResult->m_ClientId, aBuf);
}

void CGameContext::ConLogin2(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	if(g_Config.m_SvAccounts != 2)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Filebased accounts are turned off.");
		return;
	}

	if(pResult->NumArguments() != 2)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Use '/login2 <name> <password>'.");
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Use '/accountinfo' for help.");
		return;
	}

	if(pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(ClientId, pSelf->Loc("[ACCOUNT] You are already logged in", ClientId));
		return;
	}

	char aUsername[32];
	char aPassword[MAX_PW_LEN + 1];
	str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
	str_copy(aPassword, pResult->GetString(1), sizeof(aPassword));

	if(str_length(aUsername) > MAX_PW_LEN || str_length(aUsername) < MIN_PW_LEN)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Username is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if(str_length(aPassword) > MAX_PW_LEN || str_length(aPassword) < MIN_PW_LEN)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Password is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	//===========
	// FILE BASED
	//===========

	std::string Data;
	char aData[32];
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s/%s.acc", g_Config.m_SvFileAccPath, aUsername);
	std::fstream Acc2File(aBuf);

	if(!std::ifstream(aBuf))
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] login failed.");
		Acc2File.close();
		return;
	}

	getline(Acc2File, Data);
	str_copy(aData, Data.c_str(), sizeof(aData));

	if(str_comp(aData, aPassword))
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] wrong password.");
		Acc2File.close();
		return;
	}

	getline(Acc2File, Data);
	str_copy(aData, Data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded login state '%s'", aData);

	if(aData[0] == '1')
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] error. Account is already logged in.");
		Acc2File.close();
		return;
	}

	getline(Acc2File, Data);
	str_copy(aData, Data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded port '%s'", aData);

	getline(Acc2File, Data);
	str_copy(aData, Data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded frozen state '%s'", aData);

	if(aData[0] == '1')
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] error. Account is frozen.");
		Acc2File.close();
		return;
	}

	//==============================
	//ALL CHECKS DONE --> load stats
	//==============================

	str_copy(pPlayer->m_Account.m_aUsername, aUsername, sizeof(pPlayer->m_Account.m_aUsername));
	str_copy(pPlayer->m_Account.m_aPassword, aPassword, sizeof(pPlayer->m_Account.m_aPassword));
	pPlayer->SetAccId(-1);
	pPlayer->m_IsFileAcc = true;

	getline(Acc2File, Data);
	str_copy(aData, Data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded vip '%d'", atoi(aData));
	pPlayer->m_Account.m_IsModerator = atoi(aData);

	getline(Acc2File, Data);
	str_copy(aData, Data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded vip++ '%d'", atoi(aData));
	pPlayer->m_Account.m_IsSuperModerator = atoi(aData);

	getline(Acc2File, Data);
	str_copy(aData, Data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded supporter '%d'", atoi(aData));
	pPlayer->m_Account.m_IsSupporter = atoi(aData);

	getline(Acc2File, Data);
	str_copy(aData, Data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded money '%d'", atoi(aData));
	pPlayer->SetMoney(atoi(aData));

	getline(Acc2File, Data);
	str_copy(aData, Data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded level '%d'", atoi(aData));
	pPlayer->SetLevel(atoi(aData));

	getline(Acc2File, Data);
	str_copy(aData, Data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded xp '%d'", atoi(aData));
	pPlayer->SetXP(atoi(aData));

	getline(Acc2File, Data);
	str_copy(aData, Data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded shit '%d'", atoi(aData));
	pPlayer->m_Account.m_Shit = atoi(aData);

	getline(Acc2File, Data);
	str_copy(aData, Data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded policerank '%d'", atoi(aData));
	pPlayer->m_Account.m_PoliceRank = atoi(aData);

	getline(Acc2File, Data);
	str_copy(aData, Data.c_str(), sizeof(aData));
	dbg_msg("acc2", "loaded taserlevel '%d'", atoi(aData));
	pPlayer->m_Account.m_TaserLevel = atoi(aData);

	pSelf->SendChatTarget(ClientId, "[ACCOUNT] logged in.");

	//save the acc with the new data and set islogged in to true
	pPlayer->m_Account.m_IsLoggedIn = 1;
	pPlayer->SaveFileBased();
}

void CGameContext::ConRegister2(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	if(g_Config.m_SvAccounts != 2)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Filebased accounts are turned off.");
		return;
	}

	if(pResult->NumArguments() != 3)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Please use '/register2 <name> <password> <password>'.");
		return;
	}

	if(pPlayer->IsLoggedIn())
	{
		pSelf->SendChatTarget(ClientId, pSelf->Loc("[ACCOUNT] You are already logged in", ClientId));
		return;
	}

	char aBuf[512];
	char aUsername[32];
	char aPassword[MAX_PW_LEN + 1];
	char aPassword2[MAX_PW_LEN + 1];
	str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
	str_copy(aPassword, pResult->GetString(1), sizeof(aPassword));
	str_copy(aPassword2, pResult->GetString(2), sizeof(aPassword2));

	if(str_length(aUsername) > MAX_PW_LEN || str_length(aUsername) < MIN_PW_LEN)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Username is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if((str_length(aPassword) > MAX_PW_LEN || str_length(aPassword) < MIN_PW_LEN) || (str_length(aPassword2) > MAX_PW_LEN || str_length(aPassword2) < MIN_PW_LEN))
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Password is too long or too short. Max. length " MAX_PW_LEN_STR ", min. length " MIN_PW_LEN_STR);
		return;
	}

	if(str_comp_nocase(aPassword, aPassword2) != 0)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] Passwords need to be identical.");
		return;
	}

	//                                                                                                  \\ Escaping the escape seceqnze //unallow escape char because u can add newline i guess
	char aAllowedCharSet[128] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789&!?*.:+@/-_";
	bool EvilChar = false;

	for(int i = 0; i < str_length(aUsername); i++)
	{
		bool IsOk = false;

		for(int j = 0; j < str_length(aAllowedCharSet); j++)
		{
			if(aUsername[i] == aAllowedCharSet[j])
			{
				//dbg_msg("account","found valid char '%c' - '%c'", aUsername[i], aAllowedCharSet[j]);
				IsOk = true;
				break;
			}
		}

		if(!IsOk)
		{
			//dbg_msg("account", "found evil char '%c'", aUsername[i]);
			EvilChar = true;
			break;
		}
	}

	if(EvilChar)
	{
		str_format(aBuf, sizeof(aBuf), "[ACCOUNT] please use only the following characters in your username '%s'", aAllowedCharSet);
		pSelf->SendChatTarget(ClientId, aBuf);
		return;
	}

	//===========
	// FILE BASED
	//===========

	str_format(aBuf, sizeof(aBuf), "%s/%s.acc", g_Config.m_SvFileAccPath, aUsername);

	if(std::ifstream(aBuf))
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] username already exists.");
		return;
	}
	std::ofstream Account2File(aBuf);
	if(!Account2File)
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] an error occurred. pls report to an admin.");
		dbg_msg("acc2", "error1 writing file 'file_accounts/%s.acc'", aUsername);
		Account2File.close();
		return;
	}

	if(Account2File.is_open())
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] successfully registered an account.");
		Account2File
			<< aPassword << "\n" /* 0 password */
			<< "0"
			<< "\n" /* 1 login state */
			<< "0"
			<< "\n" /* 2 last port */
			<< "0"
			<< "\n" /* 3 IsFrozen */
			<< "0"
			<< "\n" /* 4 IsModerator */
			<< "0"
			<< "\n" /* 5 IsSuperModerator */
			<< "0"
			<< "\n" /* 6 IsSupporter */
			<< "0"
			<< "\n" /* 7 money */
			<< "0"
			<< "\n" /* 8 level */
			<< "0"
			<< "\n" /* 9 xp */
			<< "0"
			<< "\n" /* 10 shit */
			<< "0"
			<< "\n" /* 11 policerank */
			<< "0"
			<< "\n"; /* 12 taserlevel */
	}
	else
	{
		pSelf->SendChatTarget(ClientId, "[ACCOUNT] an error occurred. pls report to an admin.");
		dbg_msg("acc2", "error2 writing file 'file_accounts/%s.acc'", aUsername);
	}

	Account2File.close();
}

void CGameContext::ConACC2(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	if(g_Config.m_SvAccounts == 0)
	{
		pSelf->SendChatLocSys(ClientId, "ACCOUNT", "The account system is turned off.");
		return;
	}

	if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) != AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "Missing permission.");
		return;
	}

	char aBuf[128];
	char aCommand[32];
	char aName[32];
	str_copy(aCommand, pResult->GetString(0), sizeof(aCommand));
	str_copy(aName, pResult->GetString(1), sizeof(aName));

	if(pResult->NumArguments() == 0 || !str_comp_nocase(aCommand, "help"))
	{
		if(g_Config.m_SvAccounts != 2)
		{
			pSelf->SendChatTarget(ClientId, "---- COMMANDS [WARNING FILEBASED SYS IS DEACTIVATED]-----");
		}
		else
		{
			pSelf->SendChatTarget(ClientId, "---- COMMANDS -----");
		}
		pSelf->SendChatTarget(ClientId, "'/acc2 super_mod <name> <val>'");
		pSelf->SendChatTarget(ClientId, "'/acc2 mod <name> <val>'");
		pSelf->SendChatTarget(ClientId, "'/acc2 supporter <name> <val>'");
		pSelf->SendChatTarget(ClientId, "'/acc2 freeze_acc <name> <val>'");
		pSelf->SendChatTarget(ClientId, "----------------------");
		pSelf->SendChatTarget(ClientId, "'/acc_info <player name>' additional info");
		pSelf->SendChatTarget(ClientId, "'/sql' similar cmd using sql acc sys");
		return;
	}

	if(!str_comp_nocase(aCommand, "supporter"))
	{
		if(pResult->NumArguments() < 3)
		{
			pSelf->SendChatTarget(ClientId, "Error: acc2 <command> <name> <value>");
			return;
		}
		int value;
		char aValueStr[16];
		value = pResult->GetInteger(2);
		str_format(aValueStr, sizeof(aValueStr), "%d", value);

		for(auto &Player : pSelf->m_apPlayers)
		{
			if(Player)
			{
				if(Player->IsLoggedIn() && !str_comp(Player->m_Account.m_aUsername, aName))
				{
					Player->m_Account.m_IsSupporter = value;
					if(value == 1)
					{
						pSelf->SendChatTarget(Player->GetCid(), "[ACCOUNT] You are now Supporter.");
					}
					else
					{
						pSelf->SendChatTarget(Player->GetCid(), "[ACCOUNT] You are no longer Supporter.");
					}
					str_format(aBuf, sizeof(aBuf), "UPDATED IsSupporter = %d (account is logged in)", value);
					pSelf->SendChatTarget(ClientId, aBuf);
					return;
				}
			}
		}

		//ONLY WRITE TO FILE IF ACCOUNT NOT LOGGED IN ON SERVER

		if(!pSelf->ChillUpdateFileAcc(aName, 6, aValueStr, pResult->m_ClientId))
		{
			str_format(aBuf, sizeof(aBuf), "[ACC2] UPDATED IsSupporter = %d (account is not logged in)", value);
			pSelf->SendChatTarget(ClientId, aBuf);
		}
		else
		{
			pSelf->SendChatTarget(ClientId, "[ACC2] command failed.");
		}
	}
	else if(!str_comp_nocase(aCommand, "super_mod"))
	{
		if(pResult->NumArguments() < 3)
		{
			pSelf->SendChatTarget(ClientId, "Error: acc2 <command> <name> <value>");
			return;
		}
		int value;
		char aValueStr[16];
		value = pResult->GetInteger(2);
		str_format(aValueStr, sizeof(aValueStr), "%d", value);

		for(auto &Player : pSelf->m_apPlayers)
		{
			if(!Player)
				continue;

			if(Player->IsLoggedIn() && !str_comp(Player->m_Account.m_aUsername, aName))
			{
				Player->m_Account.m_IsSuperModerator = value;
				if(value == 1)
				{
					pSelf->SendChatTarget(Player->GetCid(), "[ACCOUNT] You are now VIP+");
				}
				else
				{
					pSelf->SendChatTarget(Player->GetCid(), "[ACCOUNT] You are no longer VIP+");
				}
				str_format(aBuf, sizeof(aBuf), "UPDATED IsSuperModerator = %d (account is logged in)", value);
				pSelf->SendChatTarget(ClientId, aBuf);
				return;
			}
		}

		//ONLY WRITE TO FILE IF ACCOUNT NOT LOGGED IN ON SERVER

		if(!pSelf->ChillUpdateFileAcc(aName, 5, aValueStr, pResult->m_ClientId))
		{
			str_format(aBuf, sizeof(aBuf), "[ACC2] UPDATED IsSuperModerator = %d (account is not logged in)", value);
			pSelf->SendChatTarget(ClientId, aBuf);
		}
		else
		{
			pSelf->SendChatTarget(ClientId, "[ACC2] command failed.");
		}
	}
	else
	{
		pSelf->SendChatTarget(ClientId, "Unknown ACC2 command. Try '/acc2 help' for more help.");
	}
}

void CGameContext::ConAdmin(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	if(!(pSelf->Server()->GetAuthedState(pResult->m_ClientId) == AUTHED_ADMIN || (pSelf->Server()->GetAuthedState(ClientId) && pPlayer->m_Account.m_IsSupporter)))
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[ADMIN] Missing permission.");
		return;
	}

	char aCommand[32];
	char aName[32];
	str_copy(aCommand, pResult->GetString(0), sizeof(aCommand));
	str_copy(aName, pResult->GetString(1), sizeof(aName));

	if(pResult->NumArguments() == 0 || !str_comp_nocase(aCommand, "help"))
	{
		pSelf->SendChatTarget(ClientId, "---- COMMANDS -----");
		pSelf->SendChatTarget(ClientId, "'/admin vote_delay' reset vote delay to allow votes again");
		pSelf->SendChatTarget(ClientId, "'/admin test' test DDNet++");
		pSelf->SendChatTarget(ClientId, "'/admin acc' test for illegal chars in account names");
		pSelf->SendChatTarget(ClientId, "'/admin cfg_tiles' info on configurable tiles");
		pSelf->SendChatTarget(ClientId, "--- SIMILAR COMMANDS ---");
		pSelf->SendChatTarget(ClientId, "'/flood' for flood protection commands");
		pSelf->SendChatTarget(ClientId, "----------------------");
		return;
	}

	if(!str_comp_nocase(aCommand, "vote_delay"))
	{
		pSelf->m_LastVoteCallAll = -9999999;
		pSelf->SendChatTarget(ClientId, "[ADMIN] votes can be used again.");
	}
	else if(!str_comp_nocase(aCommand, "test"))
	{
		vec2 SurvivalGameSpawnTile = pSelf->Collision()->GetSurvivalSpawn(g_Config.m_SvMaxClients);
		vec2 SurvivalGameSpawnTile2 = pSelf->Collision()->GetSurvivalSpawn(MAX_CLIENTS);

		if(SurvivalGameSpawnTile == vec2(-1, -1))
		{
			pSelf->SendChatTarget(ClientId, "[ADMIN:Test] ERROR: not enough survival spawns (less survival spawns than slots)");
		}
		else if(SurvivalGameSpawnTile2 == vec2(-1, -1))
		{
			pSelf->SendChatTarget(ClientId, "[ADMIN:Test] WARNING: less survival spawns on map than slots possible in ddnet++ (no problem as long as slots stay how they are)");
		}
		else
		{
			pSelf->SendChatTarget(ClientId, "[ADMIN:Test] Test Finished. Everything looks good c:");
		}
	}
	else if(!str_comp_nocase(aCommand, "acc"))
	{
		if(pSelf->PrintSpecialCharUsers(ClientId) == 0)
		{
			pSelf->SendChatTarget(ClientId, "All users have allowed char set usernames.");
		}
	}
	else if(!str_comp_nocase(aCommand, "cfg_tiles") || !str_comp_nocase(aCommand, "cfg_tile") || !str_comp_nocase(aCommand, "config_tiles"))
	{
		pSelf->SendChatTarget(ClientId, "=== config tiles ===");
		pSelf->SendChatTarget(ClientId, "config tile 1 index=182");
		pSelf->SendChatTarget(ClientId, "config tile 2 index=183");
		pSelf->SendChatTarget(ClientId, "");
		pSelf->SendChatTarget(ClientId, "sv_cfg_tile_1, sv_cfg_tile_2:");
		pSelf->SendChatTarget(ClientId, "0=off");
		pSelf->SendChatTarget(ClientId, "1=freeze,2=unfreeze,3=deep,4=undeep");
		pSelf->SendChatTarget(ClientId, "5=hook,6=unhook,7=kill");
		pSelf->SendChatTarget(ClientId, "8=bloody,9=rainbow,10=spreadgun");
	}
	else
	{
		pSelf->SendChatTarget(ClientId, "[ADMIN] unknown parameter");
	}
}

void CGameContext::ConFNN(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientId(pResult->m_ClientId))
		return;

	int ClientId = pResult->m_ClientId;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	if(pSelf->Server()->GetAuthedState(pResult->m_ClientId) != AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(pResult->m_ClientId, "[FNN] Missing permission.");
		return;
	}

	char aBuf[128];
	char aCommand[32];
	char aName[32];
	str_copy(aCommand, pResult->GetString(0), sizeof(aCommand));
	str_copy(aName, pResult->GetString(1), sizeof(aName));

	if(pResult->NumArguments() == 0 || !str_comp_nocase(aCommand, "help"))
	{
		pSelf->SendChatTarget(ClientId, "---- COMMANDS -----");
		pSelf->SendChatTarget(ClientId, "'/fnn init' loads the finish tile pos");
		pSelf->SendChatTarget(ClientId, "'/fnn train' does random moves and writes highscores to file");
		pSelf->SendChatTarget(ClientId, "'/fnn play_distance' replays the best distance");
		pSelf->SendChatTarget(ClientId, "'/fnn play_fitness' replays best distance in best time");
		pSelf->SendChatTarget(ClientId, "'/fnn play_distance_finish' replays best distance to finish");
		pSelf->SendChatTarget(ClientId, "'/fnn stats' prints current highscores");
		pSelf->SendChatTarget(ClientId, "----------------------");
		return;
	}

	if(!str_comp_nocase(aCommand, "init"))
	{
		pSelf->m_FinishTilePos = pSelf->GetFinishTile();
		str_format(aBuf, sizeof(aBuf), "[FNN] found finish tile at (%.2f/%.2f)", pSelf->m_FinishTilePos.x, pSelf->m_FinishTilePos.y);
		pSelf->SendChatTarget(ClientId, aBuf);
		return;
	}
	else if(pSelf->m_FinishTilePos.x == 0.000000f && pSelf->m_FinishTilePos.y == 0.000000f)
	{
		pSelf->SendChatTarget(ClientId, "[FNN] ERROR no finish tile loaded try '/fnn init'");
		return;
	}

	if(!str_comp_nocase(aCommand, "train"))
	{
		for(auto &Player : pSelf->m_apPlayers)
		{
			if(Player && Player->m_IsDummy && Player->DummyMode() == DUMMYMODE_FNN)
			{
				Player->m_dmm25 = 0;
				str_format(aBuf, sizeof(aBuf), "[FNN] set submode to training for '%s'", pSelf->Server()->ClientName(Player->GetCid()));
				pSelf->SendChatTarget(ClientId, aBuf);
			}
		}
	}
	else if(!str_comp_nocase(aCommand, "play_distance"))
	{
		for(auto &Player : pSelf->m_apPlayers)
		{
			if(Player && Player->m_IsDummy && Player->DummyMode() == DUMMYMODE_FNN)
			{
				Player->m_dmm25 = 1; //load distance
				str_format(aBuf, sizeof(aBuf), "[FNN] set submode to play best distance for '%s'", pSelf->Server()->ClientName(Player->GetCid()));
				pSelf->SendChatTarget(ClientId, aBuf);
				if(Player->GetCharacter())
				{
					Player->GetCharacter()->Die(Player->GetCid(), WEAPON_SELF);
				}
			}
		}
	}
	else if(!str_comp_nocase(aCommand, "play_fitness"))
	{
		for(auto &Player : pSelf->m_apPlayers)
		{
			if(Player && Player->m_IsDummy && Player->DummyMode() == DUMMYMODE_FNN)
			{
				Player->m_dmm25 = 2; //load fitness
				str_format(aBuf, sizeof(aBuf), "[FNN] set submode to play best fitness for '%s'", pSelf->Server()->ClientName(Player->GetCid()));
				pSelf->SendChatTarget(ClientId, aBuf);
				if(Player->GetCharacter())
				{
					Player->GetCharacter()->Die(Player->GetCid(), WEAPON_SELF);
				}
			}
		}
	}
	else if(!str_comp_nocase(aCommand, "play_distance_finish"))
	{
		for(auto &Player : pSelf->m_apPlayers)
		{
			if(Player && Player->m_IsDummy && Player->DummyMode() == DUMMYMODE_FNN)
			{
				Player->m_dmm25 = 3; //load distance_finish
				str_format(aBuf, sizeof(aBuf), "[FNN] set submode to play best distance_finish for '%s'", pSelf->Server()->ClientName(Player->GetCid()));
				pSelf->SendChatTarget(ClientId, aBuf);
				if(Player->GetCharacter())
				{
					Player->GetCharacter()->Die(Player->GetCid(), WEAPON_SELF);
				}
			}
		}
	}
	else if(!str_comp_nocase(aCommand, "stop"))
	{
		for(auto &Player : pSelf->m_apPlayers)
		{
			if(Player && Player->m_IsDummy && Player->DummyMode() == DUMMYMODE_FNN)
			{
				Player->m_dmm25 = -2; //set to stop all
				str_format(aBuf, sizeof(aBuf), "[FNN] stopped '%s'", pSelf->Server()->ClientName(Player->GetCid()));
				pSelf->SendChatTarget(ClientId, aBuf);
				if(Player->GetCharacter())
				{
					Player->GetCharacter()->Die(Player->GetCid(), WEAPON_SELF);
				}
			}
		}
	}
	else if(!str_comp_nocase(aCommand, "stats"))
	{
		pSelf->SendChatTarget(ClientId, "========== FNN Stats ==========");
		str_format(aBuf, sizeof(aBuf), "distance=%.2f", pSelf->m_FNN_best_distance);
		pSelf->SendChatTarget(ClientId, aBuf);
		str_format(aBuf, sizeof(aBuf), "fitness=%.2f", pSelf->m_FNN_best_fitness);
		pSelf->SendChatTarget(ClientId, aBuf);
		str_format(aBuf, sizeof(aBuf), "distance_finish=%.2f", pSelf->m_FNN_best_distance_finish);
		pSelf->SendChatTarget(ClientId, aBuf);
	}
	else
	{
		pSelf->SendChatTarget(ClientId, "[FNN] unknown parameter");
	}
}
