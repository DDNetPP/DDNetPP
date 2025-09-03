#include <base/ddpp_logs.h>
#include <base/system.h>
#include <engine/shared/config.h>
#include <engine/shared/protocol.h>
#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/entities/flag.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/player.h>
#include <game/server/score.h>
#include <game/version.h>
#include <generated/protocol.h>

#include "ddnetpp.h"

CGameControllerDDNetPP::CGameControllerDDNetPP(class CGameContext *pGameServer) :
	CGameControllerDDRace(pGameServer)
{
	m_pGameType = g_Config.m_SvTestingCommands ? g_Config.m_SvGameTypeNameTest : g_Config.m_SvGameTypeName;
	m_GameFlags = GAMEFLAG_FLAGS;
}

CGameControllerDDNetPP::~CGameControllerDDNetPP() = default;

void CGameControllerDDNetPP::Tick()
{
	CGameControllerDDRace::Tick();

	FlagTick();
	DetectReconnectFlood();

	if(GameServer()->m_TicksUntilDefer > 0)
	{
		GameServer()->m_TicksUntilDefer--;
		if(!GameServer()->m_TicksUntilDefer)
			GameServer()->RunDeferredCommands();
	}
}

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

	if(g_Config.m_SvRequireLoginToJoin && g_Config.m_SvAccounts)
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

int CGameControllerDDNetPP::GetAutoTeam(int NotThisId)
{
	if(NotThisId < 0 || NotThisId > MAX_CLIENTS)
		return CGameControllerDDRace::GetAutoTeam(NotThisId);

	CPlayer *pPlayer = GameServer()->m_apPlayers[NotThisId];
	if(!pPlayer)
		return CGameControllerDDRace::GetAutoTeam(NotThisId);

	if(GameServer()->m_insta_survival_gamestate)
		return TEAM_SPECTATORS;

	return CGameControllerDDRace::GetAutoTeam(NotThisId);
}

void CGameControllerDDNetPP::PrintJoinMessage(CPlayer *pPlayer)
{
	if(!pPlayer->m_PendingJoinMessage)
		return;

	pPlayer->m_PendingJoinMessage = false;

	char aBuf[512];
	int ClientId = pPlayer->GetCid();
	str_format(aBuf, sizeof(aBuf), "'%s' entered and joined the %s", Server()->ClientName(ClientId), GetTeamName(pPlayer->GetTeam()));
	GameServer()->SendChat(-1, TEAM_ALL, aBuf, -1, CGameContext::FLAG_SIX);
}

// full overwrites ddnet's OnPlayerConnect!
void CGameControllerDDNetPP::OnPlayerConnect(class CPlayer *pPlayer)
{
	// this code has to be manually kept in sync with CGameControllerDDRace::OnPlayerConnect()
	IGameController::OnPlayerConnect(pPlayer);

	for(CMinigame *pMinigame : GameServer()->m_vMinigames)
		pMinigame->OnPlayerConnect(pPlayer);

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
				PrintJoinMessage(pPlayer);
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "'%s' entered and joined the %s (message hidden)", Server()->ClientName(ClientId), GetTeamName(pPlayer->GetTeam()));
				ddpp_log(DDPP_LOG_FLOOD, aBuf);
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
	else
	{
		// do not send delayed join messages on map change
		// if the player was in game the regular join message will not be printed
		pPlayer->m_PendingJoinMessage = false;
	}

	if(g_Config.m_SvRequireLoginToJoin && g_Config.m_SvAccounts)
	{
		if(!pPlayer->IsLoggedIn())
		{
			const char *pReason = GameServer()->Loc("You need to be logged in to play. \nGet an account with '/register <name> <pw> <pw>'", pPlayer->GetCid());
			GameServer()->SendBroadcast(pReason, pPlayer->GetCid());
			GameServer()->SendChatTarget(pPlayer->GetCid(), pReason);
		}
	}
}

void CGameControllerDDNetPP::OnPlayerDisconnect(class CPlayer *pPlayer, const char *pReason, bool Silent)
{
	for(CMinigame *pMinigame : GameServer()->m_vMinigames)
	{
		pMinigame->ClearSavedPosition(pPlayer);
		pMinigame->OnPlayerDisconnect(pPlayer, pReason);
	}

	if(pPlayer->m_PendingJoinMessage)
		Silent = true;

	int64_t TicksOnline = Server()->Tick() - pPlayer->m_JoinTick;
	int64_t SecondsOnline = TicksOnline / Server()->TickSpeed();

	if(SecondsOnline < 60)
	{
		m_NumShortConnectionsInTheLastMinute++;
		m_NumShortConnectionsInTheLast10Minutes++;
	}

	CGameControllerDDRace::OnPlayerDisconnect(pPlayer, pReason, Silent);
}

void CGameControllerDDNetPP::DoTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg)
{
	if(!GameServer()->ShowJoinMessage(pPlayer->GetCid()))
		DoChatMsg = false;
	CGameControllerDDRace::DoTeamChange(pPlayer, Team, DoChatMsg);
}

int CGameControllerDDNetPP::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int WeaponId)
{
	CGameControllerDDRace::OnCharacterDeath(pVictim, pKiller, WeaponId);
	int HadFlag = 0;

	// drop flags
	for(auto &Flag : m_apFlags)
	{
		if(!Flag)
			continue;

		if(pKiller && pKiller->GetCharacter() && Flag->GetCarrier() == pKiller->GetCharacter())
			HadFlag |= 2;
		if(Flag->GetCarrier() == pVictim)
		{
			if(g_Config.m_SvFlagSounds)
			{
				GameServer()->CreateSoundGlobal(SOUND_CTF_DROP);
			}
			/*pVictim->GetPlayer()->m_Rainbow = false;
			pVictim->GetPlayer()->m_TeeInfos.m_ColorBody = pVictim->GetPlayer()->m_ColorBodyOld;
			pVictim->GetPlayer()->m_TeeInfos.m_ColorFeet = pVictim->GetPlayer()->m_ColorFeetOld;*/
			Flag->m_DropTick = Server()->Tick();
			Flag->SetCarrier(nullptr);
			Flag->m_Vel = vec2(0, 0);

			HadFlag |= 1;
		}
		if(Flag->GetLastCarrier() == pVictim)
			Flag->SetCarrier(nullptr);
	}

	return HadFlag;
}

int CGameControllerDDNetPP::ServerInfoClientScoreValue(CPlayer *pPlayer)
{
	// can happen during map change smh
	if(!pPlayer)
		return 0;
	return pPlayer->GetScoreValue(-1);
}

const char *CGameControllerDDNetPP::ServerInfoClientScoreKind()
{
	switch(GameServer()->m_DisplayScore)
	{
	case EDisplayScore::TIME: return "time";
	case EDisplayScore::LEVEL: return "points";
	case EDisplayScore::BLOCK: return "points";
	case EDisplayScore::CURRENT_SPREE: return "points";
	case EDisplayScore::KING_OF_THE_HILL: return "points";
	case EDisplayScore::NUM_SCORES: return "points";
	}
	return "time";
}

void CGameControllerDDNetPP::DetectReconnectFlood()
{
	int Threshold = 0;
	if(GameServer()->CountConnectedHumans() > 10)
		Threshold = 10;

	bool HitThreshold = false;

	if(m_NumShortConnectionsInTheLastMinute > 10 + Threshold)
		HitThreshold = true;

	// 2 connections per minute on average seems fine
	// but if that constantly happens over a span of 10 minutes
	// it will get annoying!
	if(m_NumShortConnectionsInTheLast10Minutes > 20 + Threshold)
		HitThreshold = true;

	// activate
	if(HitThreshold)
	{
		m_LastConnectionSpamThresholdHit = time_get();
		if(!GameServer()->ReconnectFlood() && g_Config.m_SvAutoAntiReconnectFlood)
		{
			ddpp_log(DDPP_LOG_FLOOD, "activate anti reconnect flood");
			GameServer()->SetReconnectFlood(true);
		}
	}
	// deactivate
	else if(GameServer()->ReconnectFlood() && m_LastConnectionSpamThresholdHit)
	{
		int SecondsSinceLastHit = (time_get() - m_LastConnectionSpamThresholdHit) / time_freq();
		if(SecondsSinceLastHit > 1000)
		{
			ddpp_log(DDPP_LOG_FLOOD, "deactivate anti reconnect flood");
			GameServer()->SetReconnectFlood(false);
		}
	}

	if(m_NextMinuteReset < time_get())
	{
		m_NextMinuteReset = time_get() + time_freq() * 60;
		m_NumShortConnectionsInTheLastMinute = 0;
	}

	if(m_Next10MinutesReset < time_get())
	{
		m_NextMinuteReset = time_get() + time_freq() * 60 * 10;
		m_NumShortConnectionsInTheLast10Minutes = 0;
	}
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
	dbg_assert(ClientId >= 0 && ClientId < MAX_CLIENTS, "Invalid client id");
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];

	if(g_Config.m_SvRequireLoginToVote && !pPlayer->IsLoggedIn())
	{
		GameServer()->SendChatTarget(ClientId, "You need to be logged in to vote. Use the /login chat command.");
		return true;
	}

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

// return true to drop the vote
bool CGameControllerDDNetPP::OnVoteNetMessage(const CNetMsg_Cl_Vote *pMsg, int ClientId)
{
	dbg_assert(ClientId >= 0 && ClientId < MAX_CLIENTS, "Invalid client id");
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];

	CCharacter *pChr = pPlayer->GetCharacter();
	if(pMsg->m_Vote == 1) //vote yes (f3)
		GameServer()->VotedYes(pChr, pPlayer);
	else if(pMsg->m_Vote == -1) //vote no (f4)
		GameServer()->VotedNo(pChr);

	if(g_Config.m_SvRequireLoginToVote && !pPlayer->IsLoggedIn())
	{
		GameServer()->SendChatTarget(ClientId, "You need to be logged in to vote. Use the /login chat command.");
		return true;
	}

	return false;
}
