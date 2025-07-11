#include <engine/shared/config.h>
#include <engine/shared/protocol.h>
#include <game/server/ddpp/enums.h>

#include "ddnetpp.h"

// SnappingClient - Client Id of the player that will receive the snapshot
// pPlayer - CPlayer that is being snapped
// DDRaceScore - Current value of the score set by the ddnet code
int CGameControllerDDNetPP::SnapPlayerScore(int SnappingClient, CPlayer *pPlayer, int DDRaceScore)
{
	return pPlayer->GetScoreValue(SnappingClient);
}

int CGameControllerDDNetPP::SnapScoreLimit(int SnappingClient)
{
	if(SnappingClient < 0 || SnappingClient >= MAX_CLIENTS)
		return 0;

	CPlayer *pSnapReceiver = GameServer()->m_apPlayers[SnappingClient];
	if(!pSnapReceiver)
		return CGameControllerDDRace::SnapScoreLimit(SnappingClient);

	if(pSnapReceiver->IsInstagibMinigame())
	{
		if(pSnapReceiver->m_IsInstaMode_fng)
		{
			if(pSnapReceiver->m_IsInstaMode_idm)
				return g_Config.m_SvRifleScorelimit;
			else if(pSnapReceiver->m_IsInstaMode_gdm)
				return g_Config.m_SvGrenadeScorelimit;
		}
	}

	CMinigame *pMinigame = GameServer()->GetMinigame(SnappingClient);
	if(pMinigame)
		return pMinigame->ScoreLimit(pSnapReceiver);

	return CGameControllerDDRace::SnapScoreLimit(SnappingClient);
}

// SnappingClient - Client Id of the player that will receive the snapshot
// pPlayer - CPlayer that is being snapped
// pClientInfo - (in and output) info that is being snappend which is already pre filled by ddnet and can be altered.
// pPlayerInfo - (in and output) info that is being snappend which is already pre filled by ddnet and can be altered.
void CGameControllerDDNetPP::SnapPlayer6(int SnappingClient, CPlayer *pPlayer, CNetObj_ClientInfo *pClientInfo, CNetObj_PlayerInfo *pPlayerInfo)
{
	// hide players in the captcha room from the scoreboard
	// this is 0.6 only so 0.7 players see all players at all times (too lazy to fix)
	if(pPlayer->m_PendingCaptcha)
		pPlayerInfo->m_Team = TEAM_BLUE;
}

// SnappingClient - Client Id of the player that will receive the snapshot
// pPlayer - CPlayer that is being snapped
// PlayerFlags7 - the flags that were already set for that player by ddnet
int CGameControllerDDNetPP::SnapPlayerFlags7(int SnappingClient, CPlayer *pPlayer, int PlayerFlags7)
{
	return PlayerFlags7;
}
