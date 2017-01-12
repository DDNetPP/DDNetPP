/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#include "gamecontext.h"
#include <engine/shared/config.h>
#include <engine/shared/protocol.h>
#include <engine/server/server.h>
#include <game/server/teams.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/version.h>
#include <game/generated/nethash.cpp>
#include <time.h>          //ChillerDragon
#if defined(CONF_SQL)
#include <game/server/score/sql_score.h>
#endif

bool CheckClientID(int ClientID);

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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
		pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission. You are not a SuperModerator.");
		return;
	}

	pPlayer->m_IsSuperModSpawn ^= true;

	if (pPlayer->m_IsSuperModSpawn)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "SuperModSpawn activated");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "SuperModSpawn deactivated");
	}
}

void CGameContext::ConToggleXpMsg(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	pPlayer->m_xpmsg ^= true;
	pSelf->SendBroadcast(" ", pResult->m_ClientID);
}

void CGameContext::ConTaserinfo(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;


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
	pSelf->SendChatTarget(pResult->m_ClientID, "Police Ranks 3 or higher are allowed to carry a taser.");
	pSelf->SendChatTarget(pResult->m_ClientID, "Use the taser to fight gangsters.");
	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ YOUR TASER STATS ~~~");
	str_format(aBuf, sizeof(aBuf), "TaserLevel: %d/7", pPlayer->m_TaserLevel);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Price for the next level: %d", pPlayer->m_TaserPrice);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	str_format(aBuf, sizeof(aBuf), "FreezeTime: %d seconds", pPlayer->m_TaserLevel * 5);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	str_format(aBuf, sizeof(aBuf), "FailRate: %d%", 0);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
}

void CGameContext::ConOfferInfo(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
	pSelf->SendChatTarget(pResult->m_ClientID, "Moderators can give all players one rainbow offer.");
	pSelf->SendChatTarget(pResult->m_ClientID, "SuperModerators can give all players more rainbow offers and one bloody.");
	//pSelf->SendChatTarget(pResult->m_ClientID, "Admins can give all players much more of everything."); //admins can't do shit lul
	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ YOUR OFFER STATS ~~~");
	str_format(aBuf, sizeof(aBuf), "Rainbow: %d", pPlayer->m_rainbow_offer);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Bloody: %d", pPlayer->m_bloody_offer);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Trail. %d", pPlayer->m_trail_offer);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	str_format(aBuf, sizeof(aBuf), "Atom: %d", pPlayer->m_atom_offer);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
}

void CGameContext::ConChangelog(IConsole::IResult * pResult, void * pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (pResult->NumArguments() == 0)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"DDraceNetworkPlusPlus v.0.0.1");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added SuperModerator");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added Moderator");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added SuperModerator Spawn");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/logout' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/poop <amount> <player>' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"+ added '/pay <amount> <player>' command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"* dummys now join automaticlly on server start");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"* improved the blocker bot");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"------------------------");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
			"page 1/2     '/changelog <page>'");
	}
	else
	{
		int page = 0;
		page = pResult->GetInteger(0);

		if (page == 1)
		{
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
				"DDraceNetworkPlusPlus v.0.0.1");
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
				"+ added SuperModerator");
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
				"+ added Moderator");
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
				"+ added SuperModerator Spawn");
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
				"+ added '/logout' command");
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
				"* dummys now join automatically on server start");
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
				"* improved the blocker bot");
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
				"------------------------");
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
				"page 1/2     '/changelog <page>'");
		}
		else if (page == 2)
		{
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
				"DDraceNetworkPlusPlus v.0.0.0");
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
				"page 2/2     '/changelog <page>'");
		}
		else
		{
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "changelog",
				"unknow page.");
		}
	}
}


void CGameContext::ConMinigameinfo(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;

	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Info",
		"Minigame made by ChillerDragon.");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Info",
		"If you buy the game you can play it until disconnect");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Info",
		"Controls & Commands:");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Info",
		"'/start_minigame'      Starts the game...");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Info",
		"'/stop_minigame'      Stops the game...");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Info",
		"'/Minigameleft'           move left.");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Info",
		"'/Minigameright'           move right.");
}

void CGameContext::ConShop(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "room_key     %d money | lvl16 | disconnect", g_Config.m_SvRoomPrice);



	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"***************************");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"          ~ SHOP ~");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"***************************");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"type /buy <itemname>");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"***************************");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"ItemName | Price | Needed Level | OwnTime:");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"rainbow       1 500 money | lvl8 | dead");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"bloody         3 500 money | lvl3 | dead");
	/*pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"atom         3 500 money | lvl3 | dead");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"trail         3 500 money | lvl3 | dead");*/
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"minigame     250 money | lvl2 | disconnect");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"shit              5 money | lvl0 | forever");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		aBuf);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"police          100 000 money | lvl18 | forever");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"taser          50 000 money | police3 | forever");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "Shop",
		"pvp_arena_ticket     150 money | lvl0 | 1 use");
}

void CGameContext::ConPoliceChat(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;

	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "credit",
		"ChillerDragon's Block mod.");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "credit",
		"Created by ChillerDragon, timakro, FruchtiHD, Pikotee, \\toast & Blue");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "credit",
		"Based on DDRaceNetwork.");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "credit",
		"DDRaceNetwork is maintained by deen.More infos on ddnet.tw");
}

void CGameContext::ConInfo(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;

	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "credit",
		"ChillerDragon's Block mod. v.0.0.1 (more infos '/changelog')");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info",
			"Based on Teeworlds DDraceNetwork Version: " GAME_VERSION);
//#if defined( GIT_SHORTREV_HASH )
//	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info",
//			"Git revision hash: " GIT_SHORTREV_HASH);
//#endif
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info",
			"Official DDraceNetwork site: ddnet.tw");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info",
			"For more Info /cmdlist");
	//pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info",
	//	    "ChillerDragon's Website: www.chillerdragon.weebly.com");
	//pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info",
	//		"ChillerDragon's Youtube Channels: ChillerDragon & TeeTower");
}
/*
void CGameContext::ConPlayerinfo(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info",
		"##################################");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info",
		"Informations baut a player");
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info",
		"##################################");
}
*/
void CGameContext::ConHelp(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;

	if (pResult->NumArguments() == 0)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "help",
				"/cmdlist will show a list of all chat commands");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "help",
				"/help + any command will show you the help for this command");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "help",
				"Example /help settings will display the help about ");
	}
	else
	{
		const char *pArg = pResult->GetString(0);
		const IConsole::CCommandInfo *pCmdInfo =
				pSelf->Console()->GetCommandInfo(pArg, CFGFLAG_SERVER, false);
		if (pCmdInfo && pCmdInfo->m_pHelp)
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "help",
					pCmdInfo->m_pHelp);
		else
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"help",
					"Command is either unknown or you have given a blank command without any parameters.");
	}
}

void CGameContext::ConSettings(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;

	if (pResult->NumArguments() == 0)
	{
		pSelf->Console()->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"setting",
				"to check a server setting say /settings and setting's name, setting names are:");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "setting",
				"teams, cheats, collision, hooking, endlesshooking, me, ");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "setting",
				"hitting, oldlaser, timeout, votes, pause and scores");
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
							"Players Can't collide on this server");
		}
		else if (str_comp(pArg, "hooking") == 0)
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"settings",
					HookTemp ?
							"Players can hook each other on this server" :
							"Players Can't hook each other on this server");
		}
		else if (str_comp(pArg, "endlesshooking") == 0)
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"settings",
					g_Config.m_SvEndlessDrag ?
							"Players can hook time is unlimited" :
							"Players can hook time is limited");
		}
		else if (str_comp(pArg, "hitting") == 0)
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"settings",
					g_Config.m_SvHit ?
							"Player's weapons affect each other" :
							"Player's weapons has no affect on each other");
		}
		else if (str_comp(pArg, "oldlaser") == 0)
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"settings",
					g_Config.m_SvOldLaser ?
							"Lasers can hit you if you shot them and that they pull you towards the bounce origin (Like DDRace Beta)" :
							"Lasers can't hit you if you shot them, and they pull others towards the shooter");
		}
		else if (str_comp(pArg, "me") == 0)
		{
			pSelf->Console()->Print(
					IConsole::OUTPUT_LEVEL_STANDARD,
					"settings",
					g_Config.m_SvSlashMe ?
							"Players can use /me commands the famous IRC Command" :
							"Players Can't use the /me command");
		}
		else if (str_comp(pArg, "timeout") == 0)
		{
			str_format(aBuf, sizeof(aBuf),
					"The Server Timeout is currently set to %d seconds",
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
							"Players Can't use the Callvote menu tab to kick offenders");
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
									:"/pause is NOT available on this server");
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	bool Printed = false;
	if (g_Config.m_SvDDRaceRules)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "rules",
				"Be nice.");
		Printed = true;
	}
	if (g_Config.m_SvRulesLine1[0])
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "rules",
				g_Config.m_SvRulesLine1);
		Printed = true;
	}
	if (g_Config.m_SvRulesLine2[0])
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "rules",
				g_Config.m_SvRulesLine2);
		Printed = true;
	}
	if (g_Config.m_SvRulesLine3[0])
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "rules",
				g_Config.m_SvRulesLine3);
		Printed = true;
	}
	if (g_Config.m_SvRulesLine4[0])
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "rules",
				g_Config.m_SvRulesLine4);
		Printed = true;
	}
	if (g_Config.m_SvRulesLine5[0])
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "rules",
				g_Config.m_SvRulesLine5);
		Printed = true;
	}
	if (g_Config.m_SvRulesLine6[0])
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "rules",
				g_Config.m_SvRulesLine6);
		Printed = true;
	}
	if (g_Config.m_SvRulesLine7[0])
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "rules",
				g_Config.m_SvRulesLine7);
		Printed = true;
	}
	if (g_Config.m_SvRulesLine8[0])
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "rules",
				g_Config.m_SvRulesLine8);
		Printed = true;
	}
	if (g_Config.m_SvRulesLine9[0])
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "rules",
				g_Config.m_SvRulesLine9);
		Printed = true;
	}
	if (g_Config.m_SvRulesLine10[0])
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "rules",
				g_Config.m_SvRulesLine10);
		Printed = true;
	}
	if (!Printed)
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "rules",
				"No Rules Defined, Kill em all!!");
}

void CGameContext::ConToggleSpec(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
	"You can't spec while you are dead/a spectator.");
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

	pPlayer->m_Paused = (pPlayer->m_Paused == CPlayer::PAUSED_PAUSED) ? CPlayer::PAUSED_NONE : CPlayer::PAUSED_PAUSED;
}

void CGameContext::ConTogglePause(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if(!CheckClientID(pResult->m_ClientID)) return;
	char aBuf[128];

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if(!pPlayer)
		return;

	if (pPlayer->GetCharacter() == 0)
	{
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "pause",
	"You can't pause while you are dead/a spectator.");
	return;
	}

	if(pPlayer->m_Paused == CPlayer::PAUSED_FORCE)
	{
		str_format(aBuf, sizeof(aBuf), "You are force-paused. %ds left.", pPlayer->m_ForcePauseTime/pSelf->Server()->TickSpeed());
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "pause", aBuf);
		return;
	}

	pPlayer->m_Paused = (pPlayer->m_Paused == CPlayer::PAUSED_SPEC) ? CPlayer::PAUSED_NONE : CPlayer::PAUSED_SPEC;
}

void CGameContext::ConTeamTop5(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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

	if (pResult->NumArguments() > 0 && pResult->GetInteger(0) >= 0)
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "top5",
				"Showing the top 5 is not allowed on this server.");
		return;
	}

	if (pResult->NumArguments() > 0 && pResult->GetInteger(0) >= 0)
		pSelf->Score()->ShowTop5(pResult, pResult->m_ClientID, pUserData,
				pResult->GetInteger(0));
	else
		pSelf->Score()->ShowTop5(pResult, pResult->m_ClientID, pUserData);

#if defined(CONF_SQL)
	if(pSelf->m_apPlayers[pResult->m_ClientID] && g_Config.m_SvUseSQL)
		pSelf->m_apPlayers[pResult->m_ClientID]->m_LastSQLQuery = pSelf->Server()->Tick();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "dnd", "You will not receive any further global chat and server messages");
	}
}

void CGameContext::ConMap(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "map", "Example: /map adr3 to call vote for Adrenaline 3. This means that the map name must start with 'a' and contain the characters 'd', 'r' and '3' in that order");
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
			((CServer *)pSelf->Server())->DelClientCallback(pResult->m_ClientID, "Timeout Protection used", ((CServer *)pSelf->Server()));
			((CServer *)pSelf->Server())->m_aClients[i].m_Authed = CServer::AUTHED_NO;
			if (pSelf->m_apPlayers[i]->GetCharacter())
				((CGameContext *)(((CServer *)pSelf->Server())->GameServer()))->SendTuningParams(i, pSelf->m_apPlayers[i]->GetCharacter()->m_TuneZone);
			return;
		}
	}

	((CServer *)pSelf->Server())->m_NetServer.SetTimeoutProtected(pResult->m_ClientID);
	str_copy(pPlayer->m_TimeoutCode, pResult->GetString(0), sizeof(pPlayer->m_TimeoutCode));
}

void CGameContext::ConSave(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
	char aCountry[4];
	if(str_length(pCode) > 3 && pCode[0] >= 'A' && pCode[0] <= 'Z' && pCode[1] >= 'A'
		&& pCode[1] <= 'Z' && pCode[2] >= 'A' && pCode[2] <= 'Z' && pCode[3] == ' ')
	{
		str_copy(aCountry, pCode, 4);
		pCode = pCode + 4;
	}
	else
	{
		str_copy(aCountry, g_Config.m_SvSqlServerName, 4);
	}

	pSelf->Score()->SaveTeam(Team, pCode, pResult->m_ClientID, aCountry);

	if(g_Config.m_SvUseSQL)
		pPlayer->m_LastSQLQuery = pSelf->Server()->Tick();
#endif
}

void CGameContext::ConLoad(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
	if(g_Config.m_SvUseSQL)
		pPlayer->m_LastSQLQuery = pSelf->Server()->Tick();
#endif
}



//moved to gamecontext.cpp cuz me knoop!
//void CGameContext::ConAddPolicehelper(IConsole::IResult *pResult, void *pUserData)
//{
//	CGameContext *pSelf = (CGameContext *)pUserData;
//	if (!CheckClientID(pResult->m_ClientID))
//		return;
//
//	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
//	if (!pPlayer)
//		return;
//
//
//
//	if (pResult->NumArguments() > 0)
//		if (!g_Config.m_SvHideScore)
//			pSelf->Score()->ShowRank(pResult->m_ClientID, pResult->GetString(0),
//				true);
//		else
//			pSelf->Console()->Print(
//				IConsole::OUTPUT_LEVEL_STANDARD,
//				"rank",
//				"Showing the rank of other players is not allowed on this server.");
//	else
//		pSelf->Console()->Print(
//			IConsole::OUTPUT_LEVEL_STANDARD,
//			"cmd-info",
//			"Error: Missing Arguments. Use following structure: /add_policehelper (player_name)");
//
//}


void CGameContext::ConLockTeam(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
				"You must join a team and play with somebody or else you can\'t play");
		pPlayer->GetCharacter()->m_LastStartWarning = pSelf->Server()->Tick();
	}

	if (pResult->NumArguments() > 0)
	{
		if (pPlayer->GetCharacter() == 0)
		{
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "join",
					"You can't change teams while you are dead/a spectator.");
		}
		else
		{
			if (pPlayer->m_Last_Team
					+ pSelf->Server()->TickSpeed()
					* g_Config.m_SvTeamChangeDelay
					> pSelf->Server()->Tick())
			{
				pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "join",
						"You can\'t change teams that fast!");
			}
			else if(pResult->GetInteger(0) > 0 && pResult->GetInteger(0) < MAX_CLIENTS && ((CGameControllerDDRace*) pSelf->m_pController)->m_Teams.TeamLocked(pResult->GetInteger(0)))
			{
				pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "join",
						"This team is locked using /lock. Only members of the team can unlock it using /lock.");
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
						"You cannot join this team at this time");
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
					"You can't check your team while you are dead/a spectator.");
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// This will never be called
}

void CGameContext::ConWhisper(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// This will never be called
}

void CGameContext::ConSetEyeEmote(IConsole::IResult *pResult,
		void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (g_Config.m_SvEmotionalTees == -1)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "emote",
				"Server admin disabled emotes.");
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;
	if (pResult->NumArguments())
		pPlayer->m_NinjaJetpack = pResult->GetInteger(0);
	else
		pPlayer->m_NinjaJetpack = !pPlayer->m_NinjaJetpack;
}

void CGameContext::ConShowOthers(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	dbg_assert(ClientID >= 0 || ClientID < MAX_CLIENTS,
			"The Client ID is wrong");
	if (ClientID < 0 || ClientID >= MAX_CLIENTS)
		return false;
	return true;
}

void CGameContext::ConSayTime(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
		pSelf->SendChatTarget(pPlayer->GetCID(), "Rescue is not enabled on this server");
		return;
	}

	pChr->Rescue();
}

void CGameContext::ConProtectedKill(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_SQL)
void CGameContext::ConPoints(IConsole::IResult *pResult, void *pUserData)
{
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	if(pSelf->m_apPlayers[pResult->m_ClientID] && g_Config.m_SvUseSQL)
		if(pSelf->m_apPlayers[pResult->m_ClientID]->m_LastSQLQuery + pSelf->Server()->TickSpeed() >= pSelf->Server()->Tick())
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

	if(pSelf->m_apPlayers[pResult->m_ClientID] && g_Config.m_SvUseSQL)
		pSelf->m_apPlayers[pResult->m_ClientID]->m_LastSQLQuery = pSelf->Server()->Tick();
}
#endif

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


void CGameContext::ConPolicetaser(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (pPlayer->m_TaserLevel < 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You don't own a taser.");
		return;
	}

	if (pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/policetaser on' or '/policetaser off'");
		return;
	}

	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if (!str_comp_nocase(aInput, "on"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Taser activated. (Your rifle is now a taser)");
		pPlayer->m_TaserOn = true;
		return;
	}
	else if (!str_comp_nocase(aInput, "off"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Taser deactivated. (Your rifle unfreezes people)");
		pPlayer->m_TaserOn = false;
		return;
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/policetaser on' or '/policetaser off'");
		return;
	}
}

void CGameContext::ConBuy(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
		pSelf->SendChatTarget(pResult->m_ClientID, "Unknown item. Type '/buy <itemname>' use '/shop' to see the full itemlist.");
		return;
	}


	char aBuf[256];
	char aItem[32];
	str_copy(aItem, pResult->GetString(0), 32);

	if (!str_comp_nocase(aItem, "police"))
	{
		//pSelf->SendChatTarget(pResult->m_ClientID, "PoliceRank isnt available yet.");
		//return;

		if (pPlayer->m_PoliceRank == 0)
		{
			if (pPlayer->m_level < 18)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Your level is too low! You need level 18 to buy police.");
				return;
			}
		}
		else if (pPlayer->m_PoliceRank == 1)
		{
			if (pPlayer->m_level < 25)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Your level is too low! You need level 25 to upgrade police to level 2.");
				return;
			}
		}
		else if (pPlayer->m_PoliceRank == 2)
		{
			if (pPlayer->m_level < 30)
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Your level is too low! You need level 30 to upgrade police to level 3.");
				return;
			}
		}


		if (pPlayer->m_PoliceRank > 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You already bought maximum police lvl.");
			return;
		}

		if (pPlayer->m_money >= 100000)
		{
			pPlayer->MoneyTransaction(-100000, "-100000 you bought shit");
			pPlayer->m_PoliceRank++;
			str_format(aBuf, sizeof(aBuf), "You bought PoliceRank[%d]!", pPlayer->m_PoliceRank);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You don't have enough money! You need 100 000.");
		}
	}
	else if (!str_comp_nocase(aItem, "taser"))
	{
		//TODO / coming soon...:
		//add taser levels...
		//make taster shitty and sometimes dont work and higher levels lower the shitty risk

		//pSelf->SendChatTarget(pResult->m_ClientID, "coming soon...");
		//return;


		if (pPlayer->m_PoliceRank < 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You don't own a weapon license.");
			return;
		}

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
			pSelf->SendChatTarget(pResult->m_ClientID, "You already bought maximum taser level.");
			return;
		}

		if (pPlayer->m_money < pPlayer->m_TaserPrice)
		{
			str_format(aBuf, sizeof(aBuf), "You don't have enough money to upgrade your taser. You need %d money", pPlayer->m_TaserPrice);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}

		str_format(aBuf, sizeof(aBuf), "-%d bought taser", pPlayer->m_TaserPrice);
		pPlayer->MoneyTransaction(-pPlayer->m_TaserPrice, aBuf);


		pPlayer->m_TaserLevel++;
		if (pPlayer->m_TaserLevel == 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You bought a taser. (use '/policetaser on' to activate it)");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You upgraded your taser.");
		}
	}
	else if (!str_comp_nocase(aItem, "shit"))
	{
		if (pPlayer->m_money >= 5)
		{
			pPlayer->MoneyTransaction(-5, "-5 you bought shit");

			pPlayer->m_shit++;
			pSelf->SendChatTarget(pResult->m_ClientID, "You bought shit.");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You don't have enough money!");
		}
	}
	else if (!str_comp_nocase(aItem, "room_key"))
	{
		if (pPlayer->m_level < 16)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You need level 16 or more to buy a key.");
			return;
		}
		if (pPlayer->m_BoughtRoom)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You already own this item.");
			return;
		}
		if (g_Config.m_SvRoomState == 0)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "An admin has turned off the room.");
			return;
		}
		if (pPlayer->m_money >= g_Config.m_SvRoomPrice)
		{
			str_format(aBuf, sizeof(aBuf), "-%d bought room_key", g_Config.m_SvRoomPrice);
			pPlayer->MoneyTransaction(-g_Config.m_SvRoomPrice, aBuf);
			pPlayer->m_BoughtRoom = true;
			pSelf->SendChatTarget(pResult->m_ClientID, "You bought a key. You can now enter the bankroom until disconnect.");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You don't have enough money! You need 5 000.");
		}
	}
	else if (!str_comp_nocase(aItem, "minigame"))
	{
		if (pPlayer->m_level < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You need level 2 or more to buy a minigame.");
			return;
		}


		if (pPlayer->m_BoughtGame)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You already own this game.");
			return;
		}


		if (pPlayer->m_money >= 250)
		{
			pPlayer->MoneyTransaction(-250, "-250 bought minigame.");
			pPlayer->m_BoughtGame = true;
			pSelf->SendChatTarget(pResult->m_ClientID, "You bought the minigame until disconnect. Check '/minigameinfo' for more information.");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You don't have enough money! You need 250.");
		}
	}
	// buy cosmetic feature
	else if (!str_comp_nocase(aItem, "rainbow"))
	{
		if (pPlayer->GetCharacter()->m_Rainbow)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You already own rainbow");
			return;
		}

		if (pPlayer->m_level < 5)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Your level is too low! You need level 5 to buy rainbow.");
		}
		else
		{
			if (pPlayer->m_money >= 1500)
			{
				pPlayer->MoneyTransaction(-1500, "-1500 bought rainbow");
				pPlayer->GetCharacter()->m_Rainbow = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "You bought rainbow until death.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "You don't have enough money! You need 1 500.");
			}
		}
	}
	else if (!str_comp_nocase(aItem, "bloody"))
	{
		if (pPlayer->GetCharacter()->m_Bloody)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You already own bloody");
			return;
		}

		if (pPlayer->m_level < 15)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Your level is too low! You need level 15 to buy bloody.");
		}
		else
		{
			if (pPlayer->m_money >= 3500)
			{
				pPlayer->MoneyTransaction(-3500, "-3500 bought bloody");
				pPlayer->GetCharacter()->m_Bloody = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "You bought bloody until death.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "You don't have enough money! You need 3 500.");
			}
		}
	}
	/*else if (!str_comp_nocase(aItem, "atom"))
	{
		if (pPlayer->GetCharacter()->m_Atom)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "you already own atom");
			return;
		}

		if (pPlayer->m_level < 15)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "your level is too low! you need level 15 to buy atom.");
		}
		else
		{
			if (pPlayer->m_money >= 3500)
			{
				pPlayer->MoneyTransaction(-3500, "-3500 bought pvp_arena_ticket");
				pPlayer->GetCharacter()->m_Atom = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "you bought atom until death.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "you don't have enough money! You need 3 500.");
			}
		}
	}
	else if (!str_comp_nocase(aItem, "trail"))
	{
		if (pPlayer->GetCharacter()->m_Trail)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "you already own trail");
			return;
		}

		if (pPlayer->m_level < 15)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "your level is too low! you need level 15 to buy trail.");
		}
		else
		{
			if (pPlayer->m_money >= 3500)
			{
				pPlayer->MoneyTransaction(-3500, "-3500 bought pvp_arena_ticket");
				pPlayer->GetCharacter()->m_Trail = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "you bought trail until death.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "you don't have enough money! You need 3 500.");
			}
		}
	}*/
	else if (!str_comp_nocase(aItem, "pvp_arena_ticket"))
	{
		if (pPlayer->m_money >= 150)
		{
			pPlayer->MoneyTransaction(-150, "-150 bought pvp_arena_ticket");
			pPlayer->m_pvp_arena_tickets++;

			str_format(aBuf, sizeof(aBuf), "You bought a pvp_arena_ticket. You have %d tickets.", pPlayer->m_pvp_arena_tickets);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You don't have enough money! You need 150.");
		}
	}
	//else if (!str_comp_nocase(aItem, "bloody"))
	//{
	//	if (pChr->m_BoughtBloody)
	//		pSelf->SendChatTarget(pResult->m_ClientID, "You already bought bloody");
	//	else if (pPlayer->m_money >= 100)
	//	{
	//		pSelf->SendChatTarget(pResult->m_ClientID, "you bought bloody until death.");
	//		pPlayer->m_money -= 100;
	//		pChr->m_BoughtBloody = true;
	//	}
	//	else
	//	{
	//		pSelf->SendChatTarget(pResult->m_ClientID, "you don't have enough money!");
	//	}
	//}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid shop item. Choose another one.");
	}

}

void CGameContext::ConRegister(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (g_Config.m_SvAccountStuff == 0)
	{
		pSelf->SendChatTarget(ClientID, "Accounts are turned off on this server.");
		return;
	}


	if (pResult->NumArguments() != 3)
	{
		pSelf->SendChatTarget(ClientID, "Please use '/register <name> <password> <password>'.");
		return;
	}

	if (pPlayer->m_AccountID > 0)
	{
		pSelf->SendChatTarget(ClientID, "You are already logged in.");
		return;
	}

	char aUsername[32];
	char aPassword[128];
	char aPassword2[128];
	str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
	str_copy(aPassword, pResult->GetString(1), sizeof(aPassword));
	str_copy(aPassword2, pResult->GetString(2), sizeof(aPassword2));

	if (str_length(aUsername) > 20 || str_length(aUsername) < 3)
	{
		pSelf->SendChatTarget(ClientID, "Your Username is too long or too short. Max length 20 Min length 3");
		return;
	}

	if ((str_length(aPassword) > 20 || str_length(aPassword) < 3) || (str_length(aPassword2) > 20 || str_length(aPassword2) < 3))
	{
		pSelf->SendChatTarget(ClientID, "Your Password is too long or too short. Max length 20 Min length 3");
		return;
	}

	if (str_comp_nocase(aPassword, aPassword2) != 0)
	{
		pSelf->SendChatTarget(ClientID, "Passwords needs to be the same.");
		return;
	}

	char *pQueryBuf = sqlite3_mprintf("SELECT * FROM Accounts WHERE Username='%q'", aUsername);
	CQueryRegister *pQuery = new CQueryRegister();
	pQuery->m_ClientID = ClientID;
	pQuery->m_pGameServer = pSelf;
	pQuery->m_Name = aUsername;
	pQuery->m_Password = aPassword;
	pQuery->Query(pSelf->m_Database, pQueryBuf);
	sqlite3_free(pQueryBuf);
}

void CGameContext::ConSQL(IConsole::IResult * pResult, void * pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (g_Config.m_SvAccountStuff == 0)
	{
		pSelf->SendChatTarget(ClientID, "account stuff is turned off on this server.");
		return;
	}

	if (pResult->NumArguments() < 2)
	{
		pSelf->SendChatTarget(ClientID, "Error: si?i");
		return;
	}

	if (pPlayer->m_Authed != CServer::AUTHED_ADMIN) //after Arguments check to troll curious users
	{
		pSelf->SendChatTarget(ClientID, "missing permission to use this command.");
		return;
	}

	char aCommand[32];
	int SQL_ID;
	str_copy(aCommand, pResult->GetString(0), sizeof(aCommand));
	SQL_ID = pResult->GetInteger(1);


	if (!str_comp_nocase(aCommand, "getid")) //2 argument commands
	{
		pSelf->SendChatTarget(ClientID, "coming soon...");
	}
	else //3 argument commands
	{
		if (pResult->NumArguments() < 3)
		{
			pSelf->SendChatTarget(ClientID, "Error: sql <command> <id> <value>");
			return;
		}
		int value;
		value = pResult->GetInteger(2);


		if (!str_comp_nocase(aCommand, "super_mod"))
		{
			char *pQueryBuf = sqlite3_mprintf("UPDATE Accounts SET IsSuperModerator='%d' WHERE ID='%d'", value, SQL_ID);

			CQuery *pQuery = new CQuery();
			pQuery->Query(pSelf->m_Database, pQueryBuf);
			sqlite3_free(pQueryBuf);

			pSelf->SendChatTarget(ClientID, "UPDATED value... warning: if the player is logged in he has to relog to get the update");
		}
		else if (!str_comp_nocase(aCommand, "mod"))
		{
			char *pQueryBuf = sqlite3_mprintf("UPDATE Accounts SET IsModerator='%d' WHERE ID='%d'", value, SQL_ID);

			CQuery *pQuery = new CQuery();
			pQuery->Query(pSelf->m_Database, pQueryBuf);
			sqlite3_free(pQueryBuf);

			pSelf->SendChatTarget(ClientID, "UPDATED value... warning: if the player is logged in he has to relog to get the update");
		}
		else if (!str_comp_nocase(aCommand, "freeze_acc"))
		{
			char *pQueryBuf = sqlite3_mprintf("UPDATE Accounts SET IsAccFrozen='%d' WHERE ID='%d'", value, SQL_ID);

			CQuery *pQuery = new CQuery();
			pQuery->Query(pSelf->m_Database, pQueryBuf);
			sqlite3_free(pQueryBuf);

			pSelf->SendChatTarget(ClientID, "UPDATED value... warning: if the player is logged in he has to relog to get the update");
		}
		else
		{
			pSelf->SendChatTarget(ClientID, "unknown command.");
		}
	}

}

void CGameContext::ConAcc_Info(IConsole::IResult * pResult, void * pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (g_Config.m_SvAccountStuff == 0)
	{
		pSelf->SendChatTarget(ClientID, "account stuff is turned off on this server.");
		return;
	}


	if (pPlayer->m_Authed != CServer::AUTHED_ADMIN)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
		return;
	}

	if (pResult->NumArguments() != 1)
	{
		pSelf->SendChatTarget(ClientID, "Please use '/acc_info <name>'.");
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
		if (pSelf->m_apPlayers[InfoID]->m_AccountID <= 0)
		{
			pSelf->SendChatTarget(ClientID, "This player is not logged in.");
			return;
		}

		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "==== Name: %s SQL: %d ====", pSelf->Server()->ClientName(pSelf->m_apPlayers[InfoID]->GetCID()), pSelf->m_apPlayers[InfoID]->m_AccountID);
		pSelf->SendChatTarget(ClientID, aBuf);
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_LastLogoutIGN1);
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_LastLogoutIGN2);
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_LastLogoutIGN3);
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_LastLogoutIGN4);
		pSelf->SendChatTarget(ClientID, pSelf->m_apPlayers[InfoID]->m_LastLogoutIGN5);
	}
	else
	{
		pSelf->SendChatTarget(ClientID, "Unkown player name.");
	}
}

void CGameContext::ConLogin(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;
	
	if (g_Config.m_SvAccountStuff == 0)
	{
		pSelf->SendChatTarget(ClientID, "account stuff is turned off on this server.");
		return;
	}

	if (pResult->NumArguments() != 2)
	{
		pSelf->SendChatTarget(ClientID, "Please use '/login <name> <password>'.");
		return;
	}

	if (pPlayer->m_AccountID > 0)
	{
		pSelf->SendChatTarget(ClientID, "You are already logged in.");
		return;
	}

	char aUsername[32];
	char aPassword[128];
	str_copy(aUsername, pResult->GetString(0), sizeof(aUsername));
	str_copy(aPassword, pResult->GetString(1), sizeof(aPassword));

	if (str_length(aUsername) > 20 || str_length(aUsername) < 3)
	{
		pSelf->SendChatTarget(ClientID, "Your Username is too long or too short. Max length 20 Min length 3");
		return;
	}

	if (str_length(aPassword) > 20 || str_length(aPassword) < 3)
	{
		pSelf->SendChatTarget(ClientID, "Your Password is too long or too short. Max length 20 Min length 3");
		return;
	}

	char *pQueryBuf = sqlite3_mprintf("SELECT * FROM Accounts WHERE Username='%q' AND Password='%q'", aUsername, aPassword);
	CQueryLogin *pQuery = new CQueryLogin();
	pQuery->m_ClientID = ClientID;
	pQuery->m_pGameServer = pSelf;
	pQuery->Query(pSelf->m_Database, pQueryBuf);
	sqlite3_free(pQueryBuf);
}

void CGameContext::ConLogout(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->m_ClientID;
	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (!pPlayer)
		return;

	if (g_Config.m_SvAccountStuff == 0)
	{
		pSelf->SendChatTarget(ClientID, "account stuff is turned off on this server.");
		return;
	}


	if (pPlayer->m_AccountID <= 0)
	{
		pSelf->SendChatTarget(ClientID, "You are not logged in.");
		return;
	}

	pPlayer->Logout();
	pSelf->SendChatTarget(ClientID, "Logged out.");
}

void CGameContext::ConTogglejailmsg(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	pPlayer->m_hidejailmsg ^= true;
	pSelf->SendBroadcast(" ", pResult->m_ClientID);
}

void CGameContext::ConMinigameLeft(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;


	if (pPlayer->m_IsMinigame)
	{
		if (pPlayer->m_HashPos > 0)
		{
			pPlayer->m_HashPos--;
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You need to start a minigame first with '/start_minigame' to use the '/Minigameleft' command");
	}

}

void CGameContext::ConMinigameRight(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;


	if (pPlayer->m_IsMinigame)
	{
		if (g_Config.m_SvAllowMinigame == 2)
		{
			if (pPlayer->m_HashPos < 10)
			{
				pPlayer->m_HashPos++;
			}
		}
		else
		{
			if (pPlayer->m_HashPos < pPlayer->m_Minigameworld_size_x - 1)
			{
				pPlayer->m_HashPos++;
			}
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You need to start a minigame first with '/start_minigame' to use the '/Minigameright' command");
	}

}

void CGameContext::ConMinigameUp(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;


	if (pPlayer->m_IsMinigame)
	{
		if (g_Config.m_SvAllowMinigame == 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Admin disableld the /up movement.");
		}
		else
		{
			if (pPlayer->m_HashPosY < 1)
			{
				pPlayer->m_HashPosY++;
			}
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You need to start a minigame first with '/start_minigame' to use the '/Minigameup' command");
	}

}

void CGameContext::ConMinigameDown(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;


	if (pPlayer->m_IsMinigame)
	{
		if (g_Config.m_SvAllowMinigame == 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Admin disabled the /down movement.");
		}
		else
		{
			if (pPlayer->m_HashPosY > 0)
			{
				pPlayer->m_HashPosY--;
			}
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You need to start a minigame first with '/start_minigame' to use the '/Minigamedown' command");
	}

}

void CGameContext::ConCC(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	if (pPlayer->m_Authed == CServer::AUTHED_ADMIN)
	{

		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'namless rofl' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'(1)namless tee' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'einFISCH' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'GAGAGA' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'Steve-' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'GroßerDBC' entered and joined the game");
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
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'ChilligerDrago' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'(1)ChillerDrago' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'(2)ChillerDrago' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'(3)ChillerDrago' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'Noved' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'Aoe' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'artkis' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'namless brain' entered and joined the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'(1)ChillerDrago' has left the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'HurricaneZ' has left the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'(2)ChillerDrago' has left the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'hax0r' has left the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'(3)ChillerDrago' has left the game");
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, "'Destin' has left the game");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "You don't have enough permissions to use this command.");
	}
}

void CGameContext::ConPvpArena(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/pvp_arena ' + 'join' or 'leave'");
		return;
	}


	char aBuf[256];
	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if (!str_comp_nocase(aInput, "join"))
	{
		if (g_Config.m_SvPvpArenaState)
		{
			if (pPlayer->m_pvp_arena_tickets > 0)
			{
				if (!pPlayer->GetCharacter()->m_IsPVParena)
				{
					pPlayer->m_pvp_arena_tickets--;
					pPlayer->m_pvp_arena_games_played++;
					pPlayer->GetCharacter()->m_IsPVParena = true;
					pPlayer->GetCharacter()->m_isDmg = true;
					pSelf->SendChatTarget(pResult->m_ClientID, "Teleporting to arena... good luck and have fun!");
				}
				else
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "You are already in the PvP-arena");
				}
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "You don't have a ticket. Buy ticket first with '/buy pvp_arena_ticket'");
			}
		}
		else //no arena configurated
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "No pvp-arena found.");
		}
	}
	else if (!str_comp_nocase(aInput, "leave"))
	{
		if (pPlayer->GetCharacter()->m_IsPVParena)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Teleport request sent. Don't move for 6 seconds.");
			pPlayer->GetCharacter()->m_pvp_arena_exit_request_time = pSelf->Server()->TickSpeed() * 6; //6 sekunden
			pPlayer->GetCharacter()->m_pvp_arena_exit_request = true;
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "You are not in an arena.");
		}
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/pvp_arena ' + 'join' or 'leave'");
	}

}

void CGameContext::ConMoney(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
	str_format(aBuf, sizeof(aBuf), "Money: %d", pPlayer->m_money);
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
	pSelf->SendChatTarget(pResult->m_ClientID, "~~~~~~~~~~");
}

void CGameContext::ConPay(IConsole::IResult * pResult, void * pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	char aBuf[512];
	int Amount;
	char aUsername[32];
	Amount = pResult->GetInteger(0);
	str_copy(aUsername, pResult->GetString(1), sizeof(aUsername));
	int PayID = pSelf->GetCIDByName(aUsername);


	//COUDL DO:
	// add a blocker to pay money to ur self... but me funny mede it pozzible


	if (Amount > pPlayer->m_money)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "you don't have that much money mate -.-");
		return;
	}

	if (Amount < 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "lel are u triin' to steal money?");
		return;
	}

	if (PayID == -1)
	{
		str_format(aBuf, sizeof(aBuf), "Can't find a user with the name: %s", aUsername);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
	else
	{
		if (pSelf->m_apPlayers[PayID]->m_AccountID < 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: This player is not logged in. More info '/accountinfo'");
			return;
		}


		//player give
		str_format(aBuf, sizeof(aBuf), "You paid %d money to the player '%s'", Amount, aUsername);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "-%d paid to '%s'", Amount, aUsername);
		pPlayer->MoneyTransaction(-Amount, aBuf);

		//player get
		str_format(aBuf, sizeof(aBuf), "You paid %d money to the player '%s'", Amount, aUsername);
		pSelf->SendChatTarget(PayID, aBuf);
		str_format(aBuf, sizeof(aBuf), "+%d paid by '%s'", Amount, aUsername);
		pSelf->m_apPlayers[PayID]->MoneyTransaction(Amount, aBuf);
	}

}

void CGameContext::ConEvent(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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

	pSelf->SendChatTarget(pResult->m_ClientID, "###########################");
	if (g_Config.m_SvFinishEvent == 1)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "~~~ Race Event ~~~");
		pSelf->SendChatTarget(pResult->m_ClientID, "Info: You get more xp for finishing the map!");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "no events running at the moment...");
	}

	pSelf->SendChatTarget(pResult->m_ClientID, "###########################");
}


// accept/turn-off cosmetic features

void CGameContext::ConRainbow(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/rainbow ' + 'accept' or 'off'");
		return;
	}


	char aBuf[256];
	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if (!str_comp_nocase(aInput, "off"))
	{
		pPlayer->GetCharacter()->m_Rainbow = false;
		pPlayer->m_InfRainbow = false;
		pSelf->SendChatTarget(pResult->m_ClientID, "Rainbow turned off");
	}
	else if (!str_comp_nocase(aInput, "accept"))
	{
		if (pPlayer->m_rainbow_offer > 0)
		{
			if (!pPlayer->GetCharacter()->m_Rainbow)
			{
				pPlayer->GetCharacter()->m_Rainbow = true;
				pPlayer->m_rainbow_offer--;
				pSelf->SendChatTarget(pResult->m_ClientID, "You accepted rainbow. You can turn off rainbow with '/rainbow off'");
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
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/rainbow ' + 'accept' or 'off'");
	}
}

void CGameContext::ConBloody(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/bloody ' + 'accept' or 'off'");
		return;
	}


	char aBuf[256];
	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if (!str_comp_nocase(aInput, "off"))
	{
			pPlayer->GetCharacter()->m_Bloody = false;
			pPlayer->m_InfBloody = false;
			pSelf->SendChatTarget(pResult->m_ClientID, "bloody turned off");
	}
	else if (!str_comp_nocase(aInput, "accept"))
	{
		if (pPlayer->m_bloody_offer > 0)
		{
			if (!pPlayer->GetCharacter()->m_Bloody)
			{
				pPlayer->GetCharacter()->m_Bloody = true;
				pPlayer->m_bloody_offer--;
				pSelf->SendChatTarget(pResult->m_ClientID, "You accepted bloody. You can turn off bloody with '/bloody off'");
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
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/bloody ' + 'accept' or 'off'");
	}
}

void CGameContext::ConAtom(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/atom ' + 'accept' or 'off'");
		return;
	}


	char aBuf[256];
	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if (!str_comp_nocase(aInput, "off"))
	{
			pPlayer->GetCharacter()->m_Atom = false;
			pPlayer->m_InfAtom = false;
			pSelf->SendChatTarget(pResult->m_ClientID, "atom turned off");
	}
	else if (!str_comp_nocase(aInput, "accept"))
	{
		if (pPlayer->m_atom_offer > 0)
		{
			if (!pPlayer->GetCharacter()->m_Atom)
			{
				pPlayer->GetCharacter()->m_Atom = true;
				pPlayer->m_atom_offer--;
				pSelf->SendChatTarget(pResult->m_ClientID, "You accepted atom. You can turn off atom with '/atom off'");
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
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/atom ' + 'accept' or 'off'");
	}
}

void CGameContext::ConTrail(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/trail ' + 'accept' or 'off'");
		return;
	}


	char aBuf[256];
	char aInput[32];
	str_copy(aInput, pResult->GetString(0), 32);

	if (!str_comp_nocase(aInput, "off"))
	{
			pPlayer->GetCharacter()->m_Trail = false;
			pPlayer->m_InfTrail = false;
			pSelf->SendChatTarget(pResult->m_ClientID, "trail turned off");
	}
	else if (!str_comp_nocase(aInput, "accept"))
	{
		if (pPlayer->m_trail_offer > 0)
		{
			if (!pPlayer->GetCharacter()->m_Trail)
			{
				pPlayer->GetCharacter()->m_Trail = true;
				pPlayer->m_trail_offer--;
				pSelf->SendChatTarget(pResult->m_ClientID, "You accepted trail. You can turn off trail with '/trail off'");
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
		pSelf->SendChatTarget(pResult->m_ClientID, "Invalid. Type '/trail ' + 'accept' or 'off'");
	}
}

void CGameContext::ConAccountInfo(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;


	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ Account Info ~~~");
	pSelf->SendChatTarget(pResult->m_ClientID, "How to register?");
	pSelf->SendChatTarget(pResult->m_ClientID, "/register <name> <password> <password>");
	pSelf->SendChatTarget(pResult->m_ClientID, "How to login?");
	pSelf->SendChatTarget(pResult->m_ClientID, "/login <name> <password>");
	//pSelf->SendChatTarget(pResult->m_ClientID, " ");
	//pSelf->SendChatTarget(pResult->m_ClientID, "Tipp: name and password shoudl be different");
}

void CGameContext::ConPoliceInfo(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;


	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ Policeinfo ~~~");
	pSelf->SendChatTarget(pResult->m_ClientID, "What are the police benefits?");
	pSelf->SendChatTarget(pResult->m_ClientID, "The police can write in '/policechat' to get extra attention from players, announce policehelpers, get extra money per policerank from money-tiles and has a policetaser. ");
	pSelf->SendChatTarget(pResult->m_ClientID, "How many policeranks are there?");
	pSelf->SendChatTarget(pResult->m_ClientID, "Currently there are 3 policeranks, you get +1 extra money for each rank.");
	pSelf->SendChatTarget(pResult->m_ClientID, "How to become police?");
	pSelf->SendChatTarget(pResult->m_ClientID, "When you hit level 18, you can buy the first policerank in '/shop'");
	pSelf->SendChatTarget(pResult->m_ClientID, "For more information about the policetaser, type '/taserinfo");

}


void CGameContext::ConProfileInfo(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;


	pSelf->SendChatTarget(pResult->m_ClientID, "~~~ Profile Info ~~~");
	pSelf->SendChatTarget(pResult->m_ClientID, " ");
	pSelf->SendChatTarget(pResult->m_ClientID, "VIEW PROFILES:");
	pSelf->SendChatTarget(pResult->m_ClientID, "/profile (playername)");
	pSelf->SendChatTarget(pResult->m_ClientID, "Info: The player needs to be on the server and logged in");
	pSelf->SendChatTarget(pResult->m_ClientID, " ");
	pSelf->SendChatTarget(pResult->m_ClientID, "PROFILE SETTINGS:");
	pSelf->SendChatTarget(pResult->m_ClientID, "/profile_style (style) - changes your profile style");
	pSelf->SendChatTarget(pResult->m_ClientID, "/profile_status (status) - changes your status");
	pSelf->SendChatTarget(pResult->m_ClientID, "/profile_skype (skype) - changes your skype");
	pSelf->SendChatTarget(pResult->m_ClientID, "/profile_youtube (youtube) - changes your youtube");
	pSelf->SendChatTarget(pResult->m_ClientID, "/profile_email (email) - changes your email");
	pSelf->SendChatTarget(pResult->m_ClientID, "/profile_homepage (homepage) - changes your homepage");
	pSelf->SendChatTarget(pResult->m_ClientID, "/profile_twitter (twitter) - changes your twitter");
}

void CGameContext::ConTCMD3000(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	char aBuf[128];

	str_format(aBuf, sizeof(aBuf), "Cucumber value: %d", pSelf->m_CucumberShareValue);
	pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
}

void CGameContext::ConStockMarket(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
		if (pPlayer->m_money < pSelf->m_CucumberShareValue)
		{
			str_format(aBuf, sizeof(aBuf), "You don't have enough money. You need %d money.", pSelf->m_CucumberShareValue);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		}
		else
		{
			pPlayer->m_StockMarket_item_Cucumbers++;

			str_format(aBuf, sizeof(aBuf), "-%d bought a cucumber stock", pSelf->m_CucumberShareValue);
			pPlayer->MoneyTransaction(-pSelf->m_CucumberShareValue, aBuf);


			pSelf->m_CucumberShareValue++; //push the gernerall share value
		}

	}
	else if (!str_comp_nocase(aInput, "sell"))
	{
		if (pPlayer->m_StockMarket_item_Cucumbers > 0)
		{
			pPlayer->m_StockMarket_item_Cucumbers--;


			str_format(aBuf, sizeof(aBuf), "+%d sold a cucumber stock", pSelf->m_CucumberShareValue);
			pPlayer->MoneyTransaction(+pSelf->m_CucumberShareValue, aBuf);

			pSelf->m_CucumberShareValue--; //pull the gernerall share value
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "you dont have this stock");
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
		pSelf->SendChatTarget(pResult->m_ClientID, "error. Type '/StockMarket ' + 'sell' or 'buy' or 'info'");
	}
}

void CGameContext::ConPoop(IConsole::IResult * pResult, void * pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;

	char aBuf[512];
	int Amount;
	char aUsername[32];
	Amount = pResult->GetInteger(0);
	str_copy(aUsername, pResult->GetString(1), sizeof(aUsername));
	int PoopID = pSelf->GetCIDByName(aUsername);


	//COUDL DO:
	// add a blocker to poop ur self... but me funny mede it pozzible


	if (Amount > pPlayer->m_shit)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "you don't have shit");
		return;
	}

	if (Amount < 0)
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "you can't poop negative?! Imagine some1 is tring to push shit back in ur anus ... wtf");
		return;
	}

	if (PoopID == -1)
	{
		str_format(aBuf, sizeof(aBuf), "Can't find a user with the name: %s", aUsername);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
	}
	else
	{
		if (pSelf->m_apPlayers[PoopID]->m_AccountID < 1)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: This player is not logged in. More info '/accountinfo'");
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
			str_format(aBuf, sizeof(aBuf), "%s threw %d shit at you o.O", aUsername, Amount);
			pSelf->SendChatTarget(PoopID, aBuf);
		}
		else if (g_Config.m_SvPoopMSG == 2) //extreme
		{
			for (int i = 0; i < Amount; i++)
			{
				str_format(aBuf, sizeof(aBuf), "%s threw shit at you o.O", aUsername);
				pSelf->SendChatTarget(PoopID, aBuf);

				if (i > 30) //poop blocker o.O 30 lines of poop is the whole chat. Poor server has enough
				{
					str_format(aBuf, sizeof(aBuf), "%s threw %d shit at you o.O", aUsername, Amount); //because it was more than the chatwindow can show inform the user how much poop it was
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;


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
				pSelf->SendChatTarget(pResult->m_ClientID, "Bloody on.");
			}
			else if (!str_comp_nocase(aItem, "rainbow"))
			{
				pPlayer->GetCharacter()->m_Rainbow = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "Rainbow on.");
			}
			else if (!str_comp_nocase(aItem, "trail"))
			{
				pPlayer->GetCharacter()->m_Trail = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "Trail on.");
			}
			else if (!str_comp_nocase(aItem, "atom"))
			{
				pPlayer->GetCharacter()->m_Atom = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "Atom on.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Unknown item.");
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
						pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission. Admins can't give more than 5 bloody offer to the same player.");
					}
					else
					{
						pSelf->m_apPlayers[GiveID]->m_bloody_offer++;
						str_format(aBuf, sizeof(aBuf), "Bloody offer given to the user: %s", aUsername);
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
					}
				}
				else if (!str_comp_nocase(aItem, "rainbow"))
				{
					if (pSelf->m_apPlayers[GiveID]->m_rainbow_offer > 19)
					{
						pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission. Admins can't give more than 20 rainbow offers to the same player.");
					}
					else
					{
						pSelf->m_apPlayers[GiveID]->m_rainbow_offer++;
						str_format(aBuf, sizeof(aBuf), "Rainbow offer given to the user: %s", aUsername);
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
					}
				}
				else if (!str_comp_nocase(aItem, "trail"))
				{
					if (pSelf->m_apPlayers[GiveID]->m_trail_offer > 9)
					{
						pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission. Admins can't give more than 10 trail offers to the same player.");
						return;
					}

					pSelf->m_apPlayers[GiveID]->m_trail_offer++;
					str_format(aBuf, sizeof(aBuf), "Trail offer given to the user: %s", aUsername);
					pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				}
				else if (!str_comp_nocase(aItem, "atom"))
				{
					if (pSelf->m_apPlayers[GiveID]->m_atom_offer > 9)
					{
						pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission. Admins can't give more than 10 atom offers to the same player.");
						return;
					}

					pSelf->m_apPlayers[GiveID]->m_atom_offer++;
					str_format(aBuf, sizeof(aBuf), "Atom offer given to the user: %s", aUsername);
					pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				}
				else
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "Unknown item.");
				}
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "Can't find a user with the name: %s", aUsername);
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
				pSelf->SendChatTarget(pResult->m_ClientID, "Bloody on.");
			}
			else if (!str_comp_nocase(aItem, "rainbow"))
			{
				pPlayer->GetCharacter()->m_Rainbow = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "Rainbow on.");
			}
			else if (!str_comp_nocase(aItem, "trail"))
			{
				pPlayer->GetCharacter()->m_Trail = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "Trail on.");
			}
			else if (!str_comp_nocase(aItem, "atom"))
			{
				pPlayer->GetCharacter()->m_Atom = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "Atom on.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Unknown item.");
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
						pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission. SuperModerators can't give more than one bloody offer to the same player.");
					}
					else
					{
						pSelf->m_apPlayers[GiveID]->m_bloody_offer++;
						str_format(aBuf, sizeof(aBuf), "Bloody offer given to the user: %s", aUsername);
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
					}
				}
				else if (!str_comp_nocase(aItem, "rainbow"))
				{
					if (pSelf->m_apPlayers[GiveID]->m_rainbow_offer > 9)
					{
						pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission. SuperModerators can't give more than 10 rainbow offers to the same player.");
					}
					else
					{
						pSelf->m_apPlayers[GiveID]->m_rainbow_offer++;
						str_format(aBuf, sizeof(aBuf), "Rainbow offer given to the user: %s", aUsername);
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
					}
				}
				else if (!str_comp_nocase(aItem, "trail"))
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
				}
				else if (!str_comp_nocase(aItem, "atom"))
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
				}
				else
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "Unknown item.");
				}
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "Can't find a user with the name: %s", aUsername);
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
				pSelf->SendChatTarget(pResult->m_ClientID, "Bloody on.");
			}
			else if (!str_comp_nocase(aItem, "rainbow"))
			{
				pPlayer->GetCharacter()->m_Rainbow = true;
				pSelf->SendChatTarget(pResult->m_ClientID, "Rainbow on.");
			}
			else if (!str_comp_nocase(aItem, "trail"))
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
			}
			else if (!str_comp_nocase(aItem, "atom"))
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
			}
			else
			{
				pSelf->SendChatTarget(pResult->m_ClientID, "Unknown item.");
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
					pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
				}
				else if (!str_comp_nocase(aItem, "rainbow"))
				{
					if (pSelf->m_apPlayers[GiveID]->m_rainbow_offer)
					{
						pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission. Moderators can't give more than one offer to the same player.");
					}
					else
					{
						pSelf->m_apPlayers[GiveID]->m_rainbow_offer++;
						str_format(aBuf, sizeof(aBuf), "Rainbow offer given to the user: %s", aUsername);
						pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
					}
				}
				else if (!str_comp_nocase(aItem, "trail"))
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
				}
				else if (!str_comp_nocase(aItem, "atom"))
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
				}
				else
				{
					pSelf->SendChatTarget(pResult->m_ClientID, "Unknown item.");
				}
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "Can't find a user with the name: %s", aUsername);
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
			pSelf->SendChatTarget(pResult->m_ClientID, "Bloody on.");
		}
		else if (!str_comp_nocase(aItem, "rainbow"))
		{
			pPlayer->GetCharacter()->m_Rainbow = true;
			pSelf->SendChatTarget(pResult->m_ClientID, "Rainbow on.");
		}
		else if (!str_comp_nocase(aItem, "trail"))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
		}
		else if (!str_comp_nocase(aItem, "atom"))
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Unknown item.");
		}
	}
	else //no rank at all
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "Missing permission.");
	}
}

void CGameContext::ConBomb(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];
	if (!pPlayer)
		return;

	CCharacter* pChr = pPlayer->GetCharacter();
	if (!pChr)
		return;


	char aBuf[512];

	char aCmd[64];

	str_copy(aCmd, pResult->GetString(0), sizeof(aCmd));

	if (!str_comp_nocase(aCmd, "create"))
	{
		if (pResult->NumArguments() < 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: missing money parameter ('/bomb create <money>')");
			return;
		}
		if (pSelf->m_BombGameState)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: there is already a bomb game. you can join it with '/bomb join'");
			return;
		}

		int BombMoney;
		BombMoney = pResult->GetInteger(1);

		if (BombMoney > pPlayer->m_money)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: you don't have that much money.");
			return;
		}
		if (BombMoney < 0)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "ERROR: bomb reward has to be positive.");
			return;
		}


		pSelf->m_BombMoney = BombMoney;
		pSelf->m_BombGameState = 1;
		pPlayer->m_IsBombing = true;
		str_format(aBuf, sizeof(aBuf), "-%d bomb (join)", BombMoney);
		pPlayer->MoneyTransaction(-BombMoney, aBuf);
	}
	else if (!str_comp_nocase(aCmd, "join"))
	{
		if (!pSelf->m_BombGameState)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "No bomb game running. You can create a new one with '/bomb create <money>'");
			return;
		}
		else if (pSelf->m_BombGameState == 2)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "this bomb lobby is locked.");
			return;
		}
		else if (pSelf->m_BombGameState == 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "this bomb game is already running.");
			return;
		}
		else if (pSelf->m_BombGameState == 1)
		{
			if (pPlayer->m_money < pSelf->m_BombMoney)
			{
				str_format(aBuf, sizeof(aBuf), "you need atleast %d money to join this game.", pSelf->m_BombMoney);
				pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
				return;
			}

			str_format(aBuf, sizeof(aBuf), "-%d money for joining this game. You don't wanna risk that much money? -> '/bomb leave'", pSelf->m_BombMoney);
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			pPlayer->m_IsBombing = true;
			str_format(aBuf, sizeof(aBuf), "-%d bomb (join)", pSelf->m_BombMoney);
			pPlayer->MoneyTransaction(-pSelf->m_BombMoney, aBuf);
		}
		else
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "something went horrible wrong lol. pls contact an admin.");
			return;
		}
	}
	else if (!str_comp_nocase(aCmd, "leave"))
	{
		if (!pPlayer->m_IsBombing)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "you are not in a bomb game.");
			return;
		}
		if (pPlayer->m_IsBombing && pSelf->m_BombGameState == 3)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "you can't leave a running game. (if you disconnect your money will be lost)");
			return;
		}

		str_format(aBuf, sizeof(aBuf), "you left the bomb game. (+%d money)", pSelf->m_BombMoney);
		pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
		str_format(aBuf, sizeof(aBuf), "+%d bomb (leave)", pSelf->m_BombMoney);
		pPlayer->MoneyTransaction(pSelf->m_BombMoney, aBuf);
		pPlayer->m_IsBombing = false;
	}
	else if (!str_comp_nocase(aCmd, "start"))
	{
		if (!pPlayer->m_IsBombing)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "you are not in a bomb game.");
			return;
		}
		if (pPlayer->m_IsBombReady)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "you are already ready to start. (if you aren't ready anymore try '/bomb leave')");
			return;
		}
		if (pPlayer->m_IsBombing && pSelf->m_BombGameState == 3) //shoudl be never triggerd but yolo xd
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "game already running...");
			return;
		}

		pSelf->SendChatTarget(pResult->m_ClientID, "You are now ready to play. Wainting for others...");
		pPlayer->m_IsBombReady = true;
	}
	else if (!str_comp_nocase(aCmd, "lock")) 
	{
		if (!pPlayer->m_IsBombing)
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "you are not in a bomb game.");
			return;
		}
		if (pPlayer->m_IsBombing && pSelf->m_BombGameState == 3) 
		{
			pSelf->SendChatTarget(pResult->m_ClientID, "Running games are locked automatically");
			return;
		}


		if (pSelf->m_BombGameState == 1) //unlocked --> lock
		{
			//lock it
			pSelf->m_BombGameState = 2;

			//send lock message to all bombers
			str_format(aBuf, sizeof(aBuf), "'%s' locked the bomb lobby.", pSelf->Server()->ClientName(pResult->m_ClientID));
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (pSelf->m_apPlayers[i])
				{
					if (pSelf->m_apPlayers[i]->m_IsBombing)
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
				if (pSelf->m_apPlayers[i])
				{
					if (pSelf->m_apPlayers[i]->m_IsBombing)
					{
						pSelf->SendChatTarget(i, aBuf);
					}
				}
			}
		}
	}
	else if (!str_comp_nocase(aCmd, "help"))
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "===================");
		pSelf->SendChatTarget(pResult->m_ClientID, "->   B O M B   <-");
		pSelf->SendChatTarget(pResult->m_ClientID, "===================");
		pSelf->SendChatTarget(pResult->m_ClientID, "*** HOW? ***");
		pSelf->SendChatTarget(pResult->m_ClientID, "hammer others if ur the bomb");
		pSelf->SendChatTarget(pResult->m_ClientID, "*** Commands? ***");
		pSelf->SendChatTarget(pResult->m_ClientID, "join, leave, start, lock and help.");
		pSelf->SendChatTarget(pResult->m_ClientID, "'/bomb create <money>' to create a game with <money> price for each player. The winner gets it all.");
	}
	else
	{
		pSelf->SendChatTarget(pResult->m_ClientID, "unknow bomb command. more help at '/bomb help'");
	}
}