#include <base/system.h>
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/server/entities/character.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/player.h>
#include <game/server/score.h>
#include <game/version.h>

#include "ddnetpp.h"

CGameControllerDDNetPP::CGameControllerDDNetPP(class CGameContext *pGameServer) :
	CGameControllerDDRace(pGameServer)
{
	m_pGameType = g_Config.m_SvTestingCommands ? g_Config.m_SvGameTypeNameTest : g_Config.m_SvGameTypeName;
	m_GameFlags = GAMEFLAG_FLAGS;
}

CGameControllerDDNetPP::~CGameControllerDDNetPP() = default;

void CGameControllerDDNetPP::SetArmorProgress(CCharacter *pCharacer, int Progress)
{
	if(!pCharacer->GetPlayer()->m_IsVanillaDmg)
		CGameControllerDDRace::SetArmorProgress(pCharacer, Progress);
}

bool CGameControllerDDNetPP::CanJoinTeam(int Team, int NotThisId, char *pErrorReason, int ErrorReasonSize)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[NotThisId];
	if(!pPlayer)
		return CGameControllerDDRace::CanJoinTeam(Team, NotThisId, pErrorReason, ErrorReasonSize);

	if(g_Config.m_SvRequireLogin && g_Config.m_SvAccountStuff)
	{
		if(!pPlayer->IsLoggedIn())
		{
			const char *pReason = GameServer()->Loc("You need to be logged in to play. \nGet an account with '/register <name> <pw> <pw>'", pPlayer->GetCid());
			str_copy(pErrorReason, pReason, ErrorReasonSize);
			return false;
		}
	}

	return CGameControllerDDRace::CanJoinTeam(Team, NotThisId, pErrorReason, ErrorReasonSize);
}

// full overwrites ddnet's OnPlayerConnect!
void CGameControllerDDNetPP::OnPlayerConnect(class CPlayer *pPlayer)
{
	// this code has to be manually kept in sync with CGameControllerDDrace::OnPlayerConnect()
	IGameController::OnPlayerConnect(pPlayer);
	int ClientId = pPlayer->GetCid();

	// init the player
	Score()->PlayerData(ClientId)->Reset();

	// Can't set score here as LoadScore() is threaded, run it in
	// LoadScoreThreaded() instead
	Score()->LoadPlayerData(ClientId);

	if(!Server()->ClientPrevIngame(ClientId))
	{
		char aBuf[512];
		if(!pPlayer->m_SilentJoinMessage)
		{
			if(GameServer()->ShowJoinMessage(ClientId))
			{
				str_format(aBuf, sizeof(aBuf), "'%s' entered and joined the %s", Server()->ClientName(ClientId), GetTeamName(pPlayer->GetTeam()));
				GameServer()->SendChat(-1, TEAM_ALL, aBuf, -1, CGameContext::FLAG_SIX);
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "'%s' entered and joined the %s (message hidden)", Server()->ClientName(ClientId), GetTeamName(pPlayer->GetTeam()));
				GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "server", aBuf);
			}
		}
		if(g_Config.m_SvInstagibMode)
		{
			GameServer()->SendChatTarget(ClientId, "DDNet++ Instagib Mod (" DDNETPP_VERSIONSTR ") based on DDNet " GAME_RELEASE_VERSION);
		}
		else
		{
			char aWelcome[128];
			char aSubGameType[128];
			aSubGameType[0] = '\0';
			if(g_Config.m_SvDDPPgametype[0])
				str_format(aSubGameType, sizeof(aSubGameType), "(%s) ", g_Config.m_SvDDPPgametype);
			str_format(aWelcome, sizeof(aWelcome), "DDNet++ %s%s based on DDNet " GAME_RELEASE_VERSION, aSubGameType, DDNETPP_VERSIONSTR);
			GameServer()->SendChatTarget(ClientId, aWelcome);
		}
	}

	if(g_Config.m_SvRequireLogin && g_Config.m_SvAccountStuff)
	{
		if(!pPlayer->IsLoggedIn())
		{
			const char *pReason = GameServer()->Loc("You need to be logged in to play. \nGet an account with '/register <name> <pw> <pw>'", pPlayer->GetCid());
			GameServer()->SendBroadcast(pReason, pPlayer->GetCid());
			GameServer()->SendChatTarget(pPlayer->GetCid(), pReason);
		}
	}
}

void CGameControllerDDNetPP::DoTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg)
{
	if(!GameServer()->ShowJoinMessage(pPlayer->GetCid()))
		DoChatMsg = false;
	CGameControllerDDRace::DoTeamChange(pPlayer, Team, DoChatMsg);
}

// TODO: move to gamecontext because thats probably useful everywhere for example the extra vote menu
const char *CGameControllerDDNetPP::CommandByVoteMsg(const CNetMsg_Cl_CallVote *pMsg)
{
	if(str_comp_nocase(pMsg->m_pType, "option"))
		return "";

	CVoteOptionServer *pOption = GameServer()->m_pVoteOptionFirst;
	while(pOption)
	{
		if(str_comp_nocase(pMsg->m_pValue, pOption->m_aDescription) == 0)
			return pOption->m_aCommand;
		pOption = pOption->m_pNext;
	}
	return "";
}

// return true to drop the vote
bool CGameControllerDDNetPP::OnCallVoteNetMessage(const CNetMsg_Cl_CallVote *pMsg, int ClientId)
{
	if(GameServer()->m_VotingBlockedUntil == -1 || (GameServer()->m_VotingBlockedUntil > time_get()))
	{
		if(str_comp(CommandByVoteMsg(pMsg), "unblock_votes"))
		{
			// dbg_msg("ddnet++", "blocked vote '%s' != '%s'", pMsg->m_pType, "unblock_votes");
			if(GameServer()->m_VotingBlockedUntil == -1)
			{
				GameServer()->SendChatTarget(ClientId, "votes are currently blocked by a vote");
			}
			else
			{
				char aBuf[512];
				int Seconds = (GameServer()->m_VotingBlockedUntil - time_get()) / time_freq();
				int Minutes = Seconds / 60;
				if(Seconds > 60)
					str_format(aBuf, sizeof(aBuf), "votes are blocked for %d more minute%s", Minutes, Minutes == 1 ? "" : "s");
				else
					str_format(aBuf, sizeof(aBuf), "votes are blocked for %d more seconds", Seconds);
				GameServer()->SendChatTarget(ClientId, aBuf);
			}
			return true;
		}
	}
	return false;
}
