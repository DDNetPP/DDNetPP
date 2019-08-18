/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#include "gamecontext.h"
#include <engine/shared/config.h>
#include <engine/server/server.h>
#include <game/server/teams.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/version.h>
#include <game/generated/nethash.cpp>
#include <base/ddpp_logs.h>
#if defined(CONF_SQL)
#include <game/server/score/sql_score.h>
#endif

bool CheckClientID(int ClientID);

void CGameContext::ConGoLeft(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	pSelf->MoveCharacter(pResult->m_ClientID, -1, 0);
}


void CGameContext::ConGoRight(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	pSelf->MoveCharacter(pResult->m_ClientID, 1, 0);
}

void CGameContext::ConGoDown(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	pSelf->MoveCharacter(pResult->m_ClientID, 0, 1);
}

void CGameContext::ConGoUp(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	pSelf->MoveCharacter(pResult->m_ClientID, 0, -1);
}

void CGameContext::ConMove(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	pSelf->MoveCharacter(pResult->m_ClientID, pResult->GetInteger(0),
			pResult->GetInteger(1));
}

void CGameContext::ConMoveRaw(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	pSelf->MoveCharacter(pResult->m_ClientID, pResult->GetInteger(0),
			pResult->GetInteger(1), true);
}

void CGameContext::MoveCharacter(int ClientID, int X, int Y, bool Raw)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CCharacter* pChr = GetPlayerChar(ClientID);

	if (!pChr)
		return;

	pChr->Core()->m_Pos.x += ((Raw) ? 1 : 32) * X;
	pChr->Core()->m_Pos.y += ((Raw) ? 1 : 32) * Y;
	pChr->m_DDRaceState = DDRACE_CHEAT;
}

void CGameContext::ConKillPlayer(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	int Victim = pResult->GetVictim();

	if (pSelf->m_apPlayers[Victim])
	{
		//pSelf->m_apPlayers[Victim]->KillCharacter(WEAPON_GAME); //only survival is allowed to use weapon game o.O
		pSelf->m_apPlayers[Victim]->KillCharacter(WEAPON_WORLD);
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "%s was killed by %s",
				pSelf->Server()->ClientName(Victim),
				pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
	}
}

void CGameContext::ConNinja(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	pSelf->ModifyWeapons(pResult, pUserData, WEAPON_NINJA, false);
}

void CGameContext::Conheal(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// pSelf = GameContext()
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CCharacter* pChr = pSelf->GetPlayerChar(ClientID);
	if (pChr)
	{
		//allles müll lolololol
		//pChr->m_Health = 10;
		//GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCID()); //hier wird der effekt erstellt.
		//GameServer()->CreateDeath(Position, ClientID);
		pChr->IncreaseHealth(10);
		pChr->m_isHeal = true; //hier wird der heil effekt getriggat yuuu!

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "%s has been given full health.", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "Your health is now at 10HP! Say thanks to %s", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);


		str_format(aBuf, sizeof(aBuf), "%s was healed by %s", pSelf->Server()->ClientName(ClientID), pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);

	}
}

void CGameContext::Condummymode(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (pPlayer)
	{
		if (pResult->GetInteger(0) == 99 && pSelf->GetShopBot() != -1) // there can only be one shop bot
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "There is already a shop bot: '%s'", pSelf->Server()->ClientName(pSelf->GetShopBot()));
			pSelf->SendChatTarget(pResult->m_ClientID, aBuf);
			return;
		}
		else
			pPlayer->m_DummyMode = pResult->GetInteger(0);
	}
}

void CGameContext::ConDummyColor(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (pPlayer && pResult->GetInteger(0) && pPlayer->m_IsDummy)
	{
		pPlayer->m_TeeInfos.m_UseCustomColor = 1;
		pPlayer->m_TeeInfos.m_ColorBody = pResult->GetInteger(0);
		pPlayer->m_TeeInfos.m_ColorFeet = pResult->GetInteger(0);
	}
}

void CGameContext::ConDummySkin(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (pPlayer && pResult->GetString(0)[0] && pPlayer->m_IsDummy)
		str_copy(pPlayer->m_TeeInfos.m_SkinName, pResult->GetString(0), sizeof(pPlayer->m_TeeInfos.m_SkinName));
}

void CGameContext::ConForceColor(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (pPlayer && pResult->GetInteger(0) && !pPlayer->m_IsDummy)
	{
		pPlayer->m_TeeInfos.m_UseCustomColor = 1;
		pPlayer->m_TeeInfos.m_ColorBody = pResult->GetInteger(0);
		pPlayer->m_TeeInfos.m_ColorFeet = pResult->GetInteger(0);
	}
}

void CGameContext::ConForceSkin(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (pPlayer && pResult->GetString(0)[0] && !pPlayer->m_IsDummy)
		str_copy(pPlayer->m_TeeInfos.m_SkinName, pResult->GetString(0), sizeof(pPlayer->m_TeeInfos.m_SkinName));
}

void CGameContext::Condisarm(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (pPlayer)
	{
		pSelf->m_apPlayers[ClientID]->m_disarm ^= true;

//  kommentiert wegen disarm buggs xD




		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "%s has %s hammer for the next respawn", pPlayer->m_disarm ? "added" : "removed", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "Hammer has been %s for your next respawn by %s", pPlayer->m_disarm ? "given to you" : "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
		
	}
}

void CGameContext::Conninjasteam(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (pPlayer)
	{
		pPlayer->m_ninjasteam ^= true;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "ninjasteam has been %s for %s", pPlayer->m_ninjasteam ? "enabled" : "disabled", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "ninjasteam was %s by %s", pPlayer->m_ninjasteam ? "given to you" : "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}

void CGameContext::ConGodmode(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CCharacter* pChr = pSelf->GetPlayerChar(ClientID);
	if (pChr)
	{
		pChr->m_Godmode ^= true;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Godmode has been %s for %s", pChr->m_Godmode ? "enabled" : "disabled", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "Godmode was %s by %s", pChr->m_Godmode ? "given to you" : "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}

// cosmetics

void CGameContext::ConOldRainbow(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CCharacter* pChr = pSelf->GetPlayerChar(ClientID);
	if (pChr)
	{
		pChr->m_Rainbow ^= true;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Rainbow has been %s for %s", pChr->m_Rainbow ? "enabled" : "disabled", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "Rainbow was %s by %s", pChr->m_Rainbow ? "given to you" : "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}

void CGameContext::ConInfRainbow(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (pPlayer)
	{
		//pPlayer->m_freezeShotgun ^= true;
		pPlayer->m_InfRainbow ^= true;


		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Infinite Rainbow has been %s for %s", pPlayer->m_InfRainbow ? "enabled" : "disabled", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "Infinite Rainbow was %s by %s", pPlayer->m_InfRainbow ? "given to you": "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}

void CGameContext::ConOldBloody(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CCharacter* pChr = pSelf->GetPlayerChar(ClientID);
	if (pChr)
	{
		pChr->m_Bloody ^= true;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Bloody has been %s for %s", pChr->m_Bloody ? "enabled" : "disabled", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "Bloody was %s by %s", pChr->m_Bloody ? "given to you" : "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}

void CGameContext::ConInfBloody(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (pPlayer)
	{
		pPlayer->m_InfBloody ^= true;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Infinite Bloody has been %s for %s", pPlayer->m_InfBloody ? "enabled" : "disabled", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "Infinite Bloody was %s by %s", pPlayer->m_InfBloody ? "given to you" : "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}

void CGameContext::ConOldAtom(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CCharacter* pChr = pSelf->GetPlayerChar(ClientID);
	if (pChr)
	{
		pChr->m_Atom ^= true;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Atom has been %s for %s", pChr->m_Atom ? "enabled" : "disabled", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "Atom was %s by %s", pChr->m_Atom ? "given to you" : "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}

void CGameContext::ConInfAtom(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (pPlayer)
	{
		pPlayer->m_InfAtom ^= true;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Infinite Atom has been %s for %s", pPlayer->m_InfAtom ? "enabled" : "disabled", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "Infinite Atom was %s by %s", pPlayer->m_InfAtom ? "given to you" : "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}

void CGameContext::ConInfAutoSpreadGun(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (pPlayer)
	{
		pPlayer->m_InfAutoSpreadGun ^= true;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Infinite spread gun has been %s for %s", pPlayer->m_InfAutoSpreadGun ? "enabled" : "disabled", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "Infinite spread gun was %s by %s", pPlayer->m_InfAutoSpreadGun ? "given to you" : "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}

void CGameContext::ConOldAutoSpreadGun(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CCharacter* pChr = pSelf->GetPlayerChar(ClientID);
	if (pChr)
	{
		pChr->m_autospreadgun ^= true;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Spread gun has been %s for %s", pChr->m_autospreadgun ? "enabled" : "disabled", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "Spread Gun was %s by %s", pChr->m_autospreadgun ? "given to you" : "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}

void CGameContext::ConHomingMissile(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CCharacter* pChr = pSelf->GetPlayerChar(ClientID);
	if (pChr)
	{
		pChr->m_HomingMissile ^= true;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Homing Missile has been %s for %s", pChr->m_HomingMissile ? "enabled" : "disabled", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "Homing Missile was %s by %s", pChr->m_HomingMissile ? "given to you" : "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}

void CGameContext::ConPullhammer(IConsole::IResult *pResult, void *pUserData) // give/remove pullhammer
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	pSelf->SendChatTarget(pResult->m_ClientID, "deactivated command.");
	return;


	if (pSelf->m_apPlayers[ClientID])
	{
		pSelf->m_apPlayers[ClientID]->GetCharacter()->m_Pullhammer ^= 1;
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), pSelf->m_apPlayers[ClientID]->GetCharacter()->m_Pullhammer ? "%s gave you pullhammer!" : "%s removed your pullhammer!", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}

void CGameContext::ConOldTrail(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CCharacter* pChr = pSelf->GetPlayerChar(ClientID);
	if (pChr)
	{
		pChr->m_Trail ^= true;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Trail has been %s for %s", pChr->m_Trail ? "enabled" : "disabled", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "Trail was %s by %s", pChr->m_Trail ? "given to you" : "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}

void CGameContext::ConInfTrail(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (pPlayer)
	{
		pPlayer->m_InfTrail ^= true;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Infinite Trail has been %s for %s", pPlayer->m_InfTrail ? "enabled" : "disabled", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "Infinite Trail was %s by %s", pPlayer->m_InfTrail ? "given to you" : "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}


void CGameContext::ConForceJail(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();



	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (pPlayer)
	{
		pPlayer->JailPlayer(pResult->GetInteger(0));



		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "[JAIL] You were jailed by the evil admin '%s' for %d seconds.", pSelf->Server()->ClientName(pResult->m_ClientID), pResult->GetInteger(0));
		pSelf->SendChatTarget(ClientID, aBuf);

	}
}

void CGameContext::ConForceUnJail(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
	if (pPlayer)
	{
		pPlayer->m_JailTime = 0;

		if (pSelf->m_apPlayers[pResult->GetVictim()]->GetCharacter())
		{
			vec2 JailReleaseSpawn = pSelf->Collision()->GetRandomTile(TILE_JAILRELEASE);

			if (JailReleaseSpawn != vec2(-1, -1))
			{
				pSelf->m_apPlayers[pResult->GetVictim()]->GetCharacter()->SetPosition(JailReleaseSpawn);
			}
			else //no jailrelease
			{
				pSelf->SendChatTarget(pPlayer->GetCID(), "gibts nich");
			}
		}

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "You were released by the kind admin '%s'.", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);

	}
}

void CGameContext::ConDamage(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// pSelf = GameContext()
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CCharacter* pChr = pSelf->GetPlayerChar(ClientID);
	if (pChr)
	{
		pChr->m_isDmg ^= true;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Damage has been %s for %s.", pChr->m_isDmg ? "enabled" : "disabled", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "Damage was %s by %s.", pChr->m_isDmg ? "given to you" : "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}


void CGameContext::ConHammerfightMode(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// pSelf = GameContext()
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CCharacter* pChr = pSelf->GetPlayerChar(ClientID);
	if (pChr)
	{
		//allles müll lolololol
		//pChr->m_Health = 10;
		//GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCID()); //hier wird der effekt erstellt.
		//GameServer()->CreateDeath(Position, ClientID);
		pChr->IncreaseHealth(10);
		pChr->m_isHeal = true; //hier wird der heil effekt getriggat yuuu!
		pChr->m_isDmg ^= true;
		pChr->m_hammerfight ^= true;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "For %s hammerfight mode has been changed to: %s", pSelf->Server()->ClientName(ClientID), pChr->m_hammerfight ? "ON" : "OFF");
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "You were %s hammerfight-mode and your health was set to 10hp by %s", pChr->m_hammerfight ? "moved to" : "removed from", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);


		str_format(aBuf, sizeof(aBuf), "hammerfight-mode has been turned %s for %s by %s", pChr->m_hammerfight ? "ON" : "OFF", pSelf->Server()->ClientName(ClientID), pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChat(-1, CGameContext::CHAT_ALL, aBuf);

	}
}

void CGameContext::ConFreeze(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int Seconds = -1;
	int Victim = pResult->GetVictim();

	char aBuf[128];

	if (pResult->NumArguments() == 1)
		Seconds = clamp(pResult->GetInteger(0), -2, 9999);

	CCharacter* pChr = pSelf->GetPlayerChar(Victim);
	if (!pChr)
		return;

	if (pSelf->m_apPlayers[Victim])
	{
		pChr->Freeze(Seconds);
		pChr->GetPlayer()->m_RconFreeze = Seconds != -2;
		CServer* pServ = (CServer*)pSelf->Server();
		if (Seconds >= 0)
			str_format(aBuf, sizeof(aBuf), "'%s' ClientID=%d has been frozen for %d.", pServ->ClientName(Victim), Victim, Seconds);
		else if (Seconds == -2)
		{
			pChr->m_DeepFreeze = true;
			str_format(aBuf, sizeof(aBuf), "'%s' ClientID=%d has been Deep Frozen.", pServ->ClientName(Victim), Victim);
		}
		else
			str_format(aBuf, sizeof(aBuf), "'%s' ClientID=%d is frozen until you unfreeze him.", pServ->ClientName(Victim), Victim);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
	}

}

void CGameContext::ConUnFreeze(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int Victim = pResult->GetVictim();
	static bool Warning = false;
	char aBuf[128];
	CCharacter* pChr = pSelf->GetPlayerChar(Victim);
	if (!pChr)
		return;
	if (pChr->m_DeepFreeze && !Warning)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "warning", "This client is deeply frozen, repeat the command to defrost him.");
		Warning = true;
		return;
	}
	if (pChr->m_DeepFreeze && Warning)
	{
		pChr->m_DeepFreeze = false;
		Warning = false;
	}
	pChr->m_FreezeTime = 2;
	pChr->GetPlayer()->m_RconFreeze = false;
	CServer* pServ = (CServer*)pSelf->Server();
	str_format(aBuf, sizeof(aBuf), "'%s' ClientID=%d has been defrosted.", pServ->ClientName(Victim), Victim);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);
}

void CGameContext::ConfreezeShotgun(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// pSelf = GameContext()
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;

	int ClientID = pResult->GetVictim();

	CCharacter* pChr = pSelf->GetPlayerChar(ClientID);
	if (pChr)
	{
		pChr->m_freezeShotgun ^= true;
		pChr->m_isDmg ^= true;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "freezeShotgun has been %s for %s", pChr->m_freezeShotgun ? "enabled" : "disabled", pSelf->Server()->ClientName(ClientID));
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info", aBuf);

		str_format(aBuf, sizeof(aBuf), "freezeShotgun was %s by %s", pChr->m_freezeShotgun ? "given to you" : "removed", pSelf->Server()->ClientName(pResult->m_ClientID));
		pSelf->SendChatTarget(ClientID, aBuf);
	}
}
 
 

void CGameContext::ConSuper(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	CCharacter* pChr = pSelf->GetPlayerChar(pResult->m_ClientID);
	if (pChr && !pChr->m_Super)
	{
		pChr->m_Super = true;
		pChr->UnFreeze();
		pChr->m_TeamBeforeSuper = pChr->Team();
		pChr->Teams()->SetCharacterTeam(pResult->m_ClientID, TEAM_SUPER);
		pChr->m_DDRaceState = DDRACE_CHEAT;
	}
}

void CGameContext::ConUnSuper(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	CCharacter* pChr = pSelf->GetPlayerChar(pResult->m_ClientID);
	if (pChr && pChr->m_Super)
	{
		pChr->m_Super = false;
		pChr->Teams()->SetForceCharacterTeam(pResult->m_ClientID,
				pChr->m_TeamBeforeSuper);
	}
}

void CGameContext::ConUnSolo(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	CCharacter* pChr = pSelf->GetPlayerChar(pResult->m_ClientID);
	if (pChr)
		pChr->SetSolo(false);
}

void CGameContext::ConUnDeep(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	CCharacter* pChr = pSelf->GetPlayerChar(pResult->m_ClientID);
	if (pChr)
		pChr->m_DeepFreeze = false;
}

void CGameContext::ConShotgun(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	pSelf->ModifyWeapons(pResult, pUserData, WEAPON_SHOTGUN, false);
}

void CGameContext::ConGrenade(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	pSelf->ModifyWeapons(pResult, pUserData, WEAPON_GRENADE, false);
}

void CGameContext::ConRifle(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	pSelf->ModifyWeapons(pResult, pUserData, WEAPON_RIFLE, false);
}

void CGameContext::ConHammer(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	pSelf->ModifyWeapons(pResult, pUserData, WEAPON_HAMMER, false);
}

void CGameContext::ConWeapons(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	pSelf->ModifyWeapons(pResult, pUserData, -1, false);
}

void CGameContext::ConUnShotgun(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	pSelf->ModifyWeapons(pResult, pUserData, WEAPON_SHOTGUN, true);
}

void CGameContext::ConUnGrenade(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	pSelf->ModifyWeapons(pResult, pUserData, WEAPON_GRENADE, true);
}

void CGameContext::ConUnRifle(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	pSelf->ModifyWeapons(pResult, pUserData, WEAPON_RIFLE, true);
}

void CGameContext::ConUnWeapons(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	pSelf->ModifyWeapons(pResult, pUserData, -1, true);
}

void CGameContext::ConAddWeapon(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	pSelf->ModifyWeapons(pResult, pUserData, pResult->GetInteger(0), false);
}

void CGameContext::ConRemoveWeapon(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	pSelf->ModifyWeapons(pResult, pUserData, pResult->GetInteger(0), true);
}

//void CGameContext::ConHack(IConsole::IResult * pResult, void * pUserData)
//{
//	CGameContext *pSelf = (CGameContext *)pUserData;
//	if (!CheckClientID(pResult->m_ClientID))
//		return;
//
//	int ClientID = pResult->GetVictim();
//
//
//
//	CPlayer *pPlayer = pSelf->m_apPlayers[ClientID];
//	if (pPlayer)
//	{
//		pPlayer->m_xp += pResult->GetInteger(0);
//
//
//
//		char aBuf[256];
//		str_format(aBuf, sizeof(aBuf), "%s hacked you! +%d xp", pSelf->Server()->ClientName(pResult->m_ClientID), pResult->GetInteger(0));
//		pSelf->SendChatTarget(ClientID, aBuf);
//
//	}
//}

void CGameContext::ModifyWeapons(IConsole::IResult *pResult, void *pUserData, int Weapon, bool Remove)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	int ClientID = pResult->m_ClientID;
	if (clamp(Weapon, -1, NUM_WEAPONS - 1) != Weapon)
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info",
				"invalid weapon id");
		return;
	}

	CCharacter* pChr = GetPlayerChar(ClientID);
	if (!pChr)
		return;

	if (Weapon == -1)
	{
		if (Remove
				&& (pChr->GetActiveWeapon() == WEAPON_SHOTGUN
						|| pChr->GetActiveWeapon() == WEAPON_GRENADE
						|| pChr->GetActiveWeapon() == WEAPON_RIFLE))
			pChr->SetActiveWeapon(WEAPON_GUN);

		if (Remove)
		{
			pChr->SetWeaponGot(WEAPON_SHOTGUN, false);
			pChr->SetWeaponGot(WEAPON_GRENADE, false);
			pChr->SetWeaponGot(WEAPON_RIFLE, false);
		}
		else
			pChr->GiveAllWeapons();
	}
	else if (Weapon != WEAPON_NINJA)
	{
		if (Remove && pChr->GetActiveWeapon() == Weapon)
			pChr->SetActiveWeapon(WEAPON_GUN);

		if (Remove)
			pChr->SetWeaponGot(Weapon, false);
		else
			pChr->GiveWeapon(Weapon, -1);
	}
	else
	{
		if (Remove)
		{
			pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "info",
					"you can't remove ninja");
			return;
		}

		pChr->GiveNinja();
	}

	pChr->m_DDRaceState = DDRACE_CHEAT;
}

void CGameContext::ConToTeleporter(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	unsigned int TeleTo = pResult->GetInteger(0);

	if (((CGameControllerDDRace*)pSelf->m_pController)->m_TeleOuts[TeleTo-1].size())
	{
		int Num = ((CGameControllerDDRace*)pSelf->m_pController)->m_TeleOuts[TeleTo-1].size();
		vec2 TelePos = ((CGameControllerDDRace*)pSelf->m_pController)->m_TeleOuts[TeleTo-1][(!Num)?Num:rand() % Num];
		CCharacter* pChr = pSelf->GetPlayerChar(pResult->m_ClientID);
		if (pChr)
		{
			pChr->Core()->m_Pos = TelePos;
			pChr->m_Pos = TelePos;
			pChr->m_PrevPos = TelePos;
			pChr->m_DDRaceState = DDRACE_CHEAT;
		}
	}
}

void CGameContext::ConToCheckTeleporter(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	unsigned int TeleTo = pResult->GetInteger(0);

	if (((CGameControllerDDRace*)pSelf->m_pController)->m_TeleCheckOuts[TeleTo-1].size())
	{
		int Num = ((CGameControllerDDRace*)pSelf->m_pController)->m_TeleCheckOuts[TeleTo-1].size();
		vec2 TelePos = ((CGameControllerDDRace*)pSelf->m_pController)->m_TeleCheckOuts[TeleTo-1][(!Num)?Num:rand() % Num];
		CCharacter* pChr = pSelf->GetPlayerChar(pResult->m_ClientID);
		if (pChr)
		{
			pChr->Core()->m_Pos = TelePos;
			pChr->m_Pos = TelePos;
			pChr->m_PrevPos = TelePos;
			pChr->m_DDRaceState = DDRACE_CHEAT;
			pChr->m_TeleCheckpoint = TeleTo;
		}
	}
}

void CGameContext::ConTeleport(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	int TeleTo = pResult->GetInteger(0);
	int Tele = pResult->m_ClientID;
	if (pResult->NumArguments() > 0)
		Tele = pResult->GetVictim();

	if (pSelf->m_apPlayers[TeleTo])
	{
		CCharacter* pChr = pSelf->GetPlayerChar(Tele);
		if (pChr && pSelf->GetPlayerChar(TeleTo))
		{
			pChr->Core()->m_Pos = pSelf->m_apPlayers[TeleTo]->m_ViewPos;
			pChr->m_Pos = pSelf->m_apPlayers[TeleTo]->m_ViewPos;
			pChr->m_PrevPos = pSelf->m_apPlayers[TeleTo]->m_ViewPos;
			pChr->m_DDRaceState = DDRACE_CHEAT;
		}
	}
}

void CGameContext::ConKill(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	if (!CheckClientID(pResult->m_ClientID))
		return;
	CPlayer *pPlayer = pSelf->m_apPlayers[pResult->m_ClientID];

	if (!pPlayer
			|| (pPlayer->m_LastKill
					&& pPlayer->m_LastKill
					+ pSelf->Server()->TickSpeed()
					* g_Config.m_SvKillDelay
					> pSelf->Server()->Tick()))
		return;

	pPlayer->m_LastKill = pSelf->Server()->Tick();
	pPlayer->KillCharacter(WEAPON_SELF);
	//pPlayer->m_RespawnTick = pSelf->Server()->Tick() + pSelf->Server()->TickSpeed() * g_Config.m_SvSuicidePenalty;
}

void CGameContext::ConForcePause(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	CServer* pServ = (CServer*)pSelf->Server();
	int Victim = pResult->GetVictim();
	int Seconds = 0;
	if (pResult->NumArguments() > 0)
		Seconds = clamp(pResult->GetInteger(0), 0, 360);

	CPlayer *pPlayer = pSelf->m_apPlayers[Victim];
	if (!pPlayer)
		return;

	pPlayer->m_ForcePauseTime = Seconds*pServ->TickSpeed();
	pPlayer->m_Paused = CPlayer::PAUSED_FORCE;
}

void CGameContext::Mute(IConsole::IResult *pResult, NETADDR *Addr, int Secs, const char *pDisplayName)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[128];
	int Found = 0;
	// find a matching mute for this ip, update expiration time if found
	for (int i = 0; i < m_NumMutes; i++)
	{
		if (net_addr_comp(&m_aMutes[i].m_Addr, Addr) == 0)
		{
			m_aMutes[i].m_Expire = Server()->Tick()
							+ Secs * Server()->TickSpeed();
			Found = 1;
		}
	}

	if (!Found) // nothing found so far, find a free slot..
	{
		if (m_NumMutes < MAX_MUTES)
		{
			m_aMutes[m_NumMutes].m_Addr = *Addr;
			m_aMutes[m_NumMutes].m_Expire = Server()->Tick()
							+ Secs * Server()->TickSpeed();
			m_NumMutes++;
			Found = 1;
		}
	}
	if (Found)
	{
		str_format(aBuf, sizeof aBuf, "'%s' has been muted for %d seconds.",
				pDisplayName, Secs);
		SendChat(-1, CHAT_ALL, aBuf);
	}
	else // no free slot found
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mutes", "mute array is full");
}

void CGameContext::ConMute(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	pSelf->Console()->Print(
			IConsole::OUTPUT_LEVEL_STANDARD,
			"mutes",
			"Use either 'muteid <client_id> <seconds>' or 'muteip <ip> <seconds>'");
}

// mute through client id
void CGameContext::ConMuteID(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	int Victim = pResult->GetVictim();

	NETADDR Addr;
	pSelf->Server()->GetClientAddr(Victim, &Addr);

	pSelf->Mute(pResult, &Addr, clamp(pResult->GetInteger(0), 1, 86400),
			pSelf->Server()->ClientName(Victim));
}

// mute through ip, arguments reversed to workaround parsing
void CGameContext::ConMuteIP(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	NETADDR Addr;
	if (net_addr_from_str(&Addr, pResult->GetString(0)))
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mutes",
				"Invalid network address to mute");
	}
	pSelf->Mute(pResult, &Addr, clamp(pResult->GetInteger(1), 1, 86400),
			pResult->GetString(0));
}

// unmute by mute list index
void CGameContext::ConUnmute(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	char aIpBuf[64];
	char aBuf[64];
	int Victim = pResult->GetVictim();

	if (Victim < 0 || Victim >= pSelf->m_NumMutes)
		return;

	pSelf->m_NumMutes--;
	pSelf->m_aMutes[Victim] = pSelf->m_aMutes[pSelf->m_NumMutes];

	net_addr_str(&pSelf->m_aMutes[Victim].m_Addr, aIpBuf, sizeof(aIpBuf), false);
	str_format(aBuf, sizeof(aBuf), "Unmuted %s", aIpBuf);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mutes", aBuf);
}

// list mutes
void CGameContext::ConMutes(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	char aIpBuf[64];
	char aBuf[128];
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mutes",
			"Active mutes:");
	for (int i = 0; i < pSelf->m_NumMutes; i++)
	{
		net_addr_str(&pSelf->m_aMutes[i].m_Addr, aIpBuf, sizeof(aIpBuf), false);
		str_format(
				aBuf,
				sizeof aBuf,
				"%d: \"%s\", %d seconds left",
				i,
				aIpBuf,
				(pSelf->m_aMutes[i].m_Expire - pSelf->Server()->Tick())
				/ pSelf->Server()->TickSpeed());
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "mutes", aBuf);
	}
}

void CGameContext::ConRegisterBan(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	pSelf->Console()->Print(
			IConsole::OUTPUT_LEVEL_STANDARD,
			"RegisterBans",
			"Use either 'register_ban_id <client_id> <seconds>' or 'register_ban_ip <ip> <seconds>'");
}

// RegisterBan through client id
void CGameContext::ConRegisterBanID(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	int Victim = pResult->GetVictim();

	NETADDR Addr;
	pSelf->Server()->GetClientAddr(Victim, &Addr);

	pSelf->RegisterBan(&Addr, clamp(pResult->GetInteger(0), 1, 86400),
			pSelf->Server()->ClientName(Victim));
}

// RegisterBan through ip, arguments reversed to workaround parsing
void CGameContext::ConRegisterBanIP(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	NETADDR Addr;
	if (net_addr_from_str(&Addr, pResult->GetString(0)))
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "RegisterBans",
				"Invalid network address to RegisterBan");
	}
	pSelf->RegisterBan(&Addr, clamp(pResult->GetInteger(1), 1, 86400),
			pResult->GetString(0));
}

// unRegisterBan by RegisterBan list index
void CGameContext::ConUnRegisterBan(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	char aIpBuf[64];
	char aBuf[64];
	int Victim = pResult->GetVictim();

	if (Victim < 0 || Victim >= pSelf->m_NumRegisterBans)
		return;

	pSelf->m_NumRegisterBans--;
	pSelf->m_aRegisterBans[Victim] = pSelf->m_aRegisterBans[pSelf->m_NumRegisterBans];

	net_addr_str(&pSelf->m_aRegisterBans[Victim].m_Addr, aIpBuf, sizeof(aIpBuf), false);
	str_format(aBuf, sizeof(aBuf), "UnRegisterBand %s", aIpBuf);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "RegisterBans", aBuf);
}

// list RegisterBans
void CGameContext::ConRegisterBans(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	char aIpBuf[64];
	char aBuf[128];
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "RegisterBans",
			"Active RegisterBans:");
	for (int i = 0; i < pSelf->m_NumRegisterBans; i++)
	{
		net_addr_str(&pSelf->m_aRegisterBans[i].m_Addr, aIpBuf, sizeof(aIpBuf), false);
		str_format(
				aBuf,
				sizeof aBuf,
				"%d: \"%s\", %d seconds left",
				i,
				aIpBuf,
				(pSelf->m_aRegisterBans[i].m_Expire - pSelf->Server()->Tick())
				/ pSelf->Server()->TickSpeed());
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "RegisterBans", aBuf);
	}
}

void CGameContext::ConNameChangeMute(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	pSelf->Console()->Print(
			IConsole::OUTPUT_LEVEL_STANDARD,
			"NameChangeMutes",
			"Use either 'namechange_mute_id <client_id> <seconds>' or 'namechange_mute_ip <ip> <seconds>'");
}

// NameChangeMute through client id
void CGameContext::ConNameChangeMuteID(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	int Victim = pResult->GetVictim();

	NETADDR Addr;
	pSelf->Server()->GetClientAddr(Victim, &Addr);

	pSelf->NameChangeMute(&Addr, clamp(pResult->GetInteger(0), 1, 86400),
			pSelf->Server()->ClientName(Victim));
}

// NameChangeMute through ip, arguments reversed to workaround parsing
void CGameContext::ConNameChangeMuteIP(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	NETADDR Addr;
	if (net_addr_from_str(&Addr, pResult->GetString(0)))
	{
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "NameChangeMutes",
				"Invalid network address to NameChangeMute");
	}
	pSelf->NameChangeMute(&Addr, clamp(pResult->GetInteger(1), 1, 86400),
			pResult->GetString(0));
}

// unNameChangeMute by NameChangeMute list index
void CGameContext::ConNameChangeUnmute(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	char aIpBuf[64];
	char aBuf[64];
	int Victim = pResult->GetVictim();

	if (Victim < 0 || Victim >= pSelf->m_NumNameChangeMutes)
		return;

	pSelf->m_NumNameChangeMutes--;
	pSelf->m_aNameChangeMutes[Victim] = pSelf->m_aNameChangeMutes[pSelf->m_NumNameChangeMutes];

	net_addr_str(&pSelf->m_aNameChangeMutes[Victim].m_Addr, aIpBuf, sizeof(aIpBuf), false);
	str_format(aBuf, sizeof(aBuf), "UnNameChangeMuted %s", aIpBuf);
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "NameChangeMutes", aBuf);
}

// list NameChangeMutes
void CGameContext::ConNameChangeMutes(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	char aIpBuf[64];
	char aBuf[128];
	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "NameChangeMutes",
			"Active NameChangeMutes:");
	for (int i = 0; i < pSelf->m_NumNameChangeMutes; i++)
	{
		net_addr_str(&pSelf->m_aNameChangeMutes[i].m_Addr, aIpBuf, sizeof(aIpBuf), false);
		str_format(
				aBuf,
				sizeof aBuf,
				"%d: \"%s\", %d seconds left",
				i,
				aIpBuf,
				(pSelf->m_aNameChangeMutes[i].m_Expire - pSelf->Server()->Tick())
				/ pSelf->Server()->TickSpeed());
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "NameChangeMutes", aBuf);
	}
}

void CGameContext::ConList(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	int ClientID = pResult->m_ClientID;
	if(!CheckClientID(ClientID)) return;

	char zerochar = 0;
	if(pResult->NumArguments() > 0)
		pSelf->List(ClientID, pResult->GetString(0));
	else
		pSelf->List(ClientID, &zerochar);
}

void CGameContext::ConFreezeLaser(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();

	CCharacter* pChr = pSelf->GetPlayerChar(Victim);

	if (!pChr)
		return;

	char aBuf[128];
	str_format(aBuf, sizeof aBuf, "'%s' got freeze Laser!",
		pSelf->Server()->ClientName(Victim));
	pSelf->SendChat(-1, CHAT_ALL, aBuf);

	pChr->m_FreezeLaser = true;
}

void CGameContext::ConDestroyLaser(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	int Victim = pResult->GetVictim();

	CCharacter* pChr = pSelf->GetPlayerChar(Victim);

	if (!pChr)
		return;
	/*
	char aBuf[128];
	str_format(aBuf, sizeof aBuf, "'%s' got freeze hammer!",
		pSelf->Server()->ClientName(Victim));
	pSelf->SendChat(-1, CHAT_ALL, aBuf);
	*/
	pChr->m_DestroyLaser = true;
}

void CGameContext::ConFreezeHammer(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	int Victim = pResult->GetVictim();

	CCharacter* pChr = pSelf->GetPlayerChar(Victim);

	if (!pChr)
		return;

	char aBuf[128];
	str_format(aBuf, sizeof aBuf, "'%s' got freeze hammer!",
			pSelf->Server()->ClientName(Victim));
	pSelf->SendChat(-1, CHAT_ALL, aBuf);

	pChr->m_FreezeHammer = true;
}

void CGameContext::ConUnFreezeHammer(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;
	int Victim = pResult->GetVictim();

	CCharacter* pChr = pSelf->GetPlayerChar(Victim);

	if (!pChr)
		return;

	char aBuf[128];
	str_format(aBuf, sizeof aBuf, "'%s' lost freeze hammer!",
			pSelf->Server()->ClientName(Victim));
	pSelf->SendChat(-1, CHAT_ALL, aBuf);

	pChr->m_FreezeHammer = false;
}

void CGameContext::ConSQL_ADD(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	//CGameContext *pSelf = (CGameContext *)pUserData;
	//if (!CheckClientID(pResult->m_ClientID))
	//	return;

	//char aBuf[128];

}

//=============
// DDNet++
//=============

void CGameContext::ConDDPPLogs(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *) pUserData;

	if (pResult->NumArguments() != 1)
	{
		pSelf->Console()->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"ddpp_logs",
				"usage: logs (type)");
		pSelf->Console()->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"ddpp_logs",
				"types: mastersrv, wrong_rcon, rcon_auth");
		return;
	}
	if (!str_comp_nocase(pResult->GetString(0), "mastersrv"))
	{
		pSelf->m_pConsole->PrintDDPPLogs(DDPP_LOG_MASTER);
	}
	else if (!str_comp_nocase(pResult->GetString(0), "rcon_auth"))
	{
		pSelf->m_pConsole->PrintDDPPLogs(DDPP_LOG_AUTH_RCON);
	}
	else if (!str_comp_nocase(pResult->GetString(0), "wrong_rcon"))
	{
		pSelf->m_pConsole->PrintDDPPLogs(DDPP_LOG_WRONG_RCON);
	}
	else
	{
		pSelf->Console()->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"ddpp_logs",
				"Error: invalid type.");
		pSelf->Console()->Print(
				IConsole::OUTPUT_LEVEL_STANDARD,
				"ddpp_logs",
				"valid types: mastersrv, wrong_rcon, rcon_auth");
	}
}

void CGameContext::ConRconApiSayID(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
	{
		dbg_msg("RCON_API", "some ingame admin tried to abuse the api");
		return;
	}
	dbg_msg("RCON_API","some non client executed an api command");

	int ClientID = pResult->GetVictim();

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "[SERVER] %s", pResult->GetString(0));
	pSelf->SendChatTarget(ClientID, aBuf);
}

void CGameContext::ConRconApiAlterTable(IConsole::IResult *pResult, void *pUserData)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	CGameContext *pSelf = (CGameContext *)pUserData;
	if (!CheckClientID(pResult->m_ClientID))
	{
		dbg_msg("RCON_API", "some ingame admin tried to abuse the api");
		return;
	}
	dbg_msg("RCON_API", "some non client executed an api command");

	char aBuf[256];
	char aSQL[256];
	int Type = pResult->GetInteger(0);

	if (Type == 0)
	{
		str_format(aBuf, sizeof(aBuf), "added column '%s' of type INTEGER", pResult->GetString(1));
		str_format(aSQL, sizeof(aSQL), "ALTER TABLE `Accounts` ADD `%s` INTEGER DEFAULT 0", pResult->GetString(1));
	}
	else if (Type == 1)
	{
		str_format(aBuf, sizeof(aBuf), "added column '%s' of type VARCHAR(4)", pResult->GetString(1));
		str_format(aSQL, sizeof(aSQL), "ALTER TABLE `Accounts` ADD `%s` VARCHAR(4) DEFAULT ''", pResult->GetString(1));
	}
	else if (Type == 2)
	{
		str_format(aBuf, sizeof(aBuf), "added column '%s' of type VARCHAR(16)", pResult->GetString(1));
		str_format(aSQL, sizeof(aSQL), "ALTER TABLE `Accounts` ADD `%s` VARCHAR(16) DEFAULT ''", pResult->GetString(1));
	}
	else if (Type == 3)
	{
		str_format(aBuf, sizeof(aBuf), "added column '%s' of type VARCHAR(32)", pResult->GetString(1));
		str_format(aSQL, sizeof(aSQL), "ALTER TABLE `Accounts` ADD `%s` VARCHAR(32) DEFAULT ''", pResult->GetString(1));
	}
	else if (Type == 4)
	{
		str_format(aBuf, sizeof(aBuf), "added column '%s' of type VARCHAR(64)", pResult->GetString(1));
		str_format(aSQL, sizeof(aSQL), "ALTER TABLE `Accounts` ADD `%s` VARCHAR(64) DEFAULT ''", pResult->GetString(1));
	}
	else if (Type == 5)
	{
		str_format(aBuf, sizeof(aBuf), "added column '%s' of type VARCHAR(128)", pResult->GetString(1));
		str_format(aSQL, sizeof(aSQL), "ALTER TABLE `Accounts` ADD `%s` VARCHAR(128) DEFAULT ''", pResult->GetString(1));
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "failed to add column '%s' of unsupported type %d", pResult->GetString(1), Type);
		return;
	}

	pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "RCON_API", aBuf);
#if defined(CONF_DEBUG)
	dbg_msg("SQL", "RCON_API: %s", aSQL);
#endif
	pSelf->ExecuteSQLf(aSQL);
}