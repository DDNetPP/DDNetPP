#include <engine/shared/config.h>
#include <engine/shared/protocol.h>
#include <game/server/ddpp/enums.h>

#include "ddnetpp.h"

// SnappingClient - Client Id of the player that will receive the snapshot
// pPlayer - CPlayer that is being snapped
// DDRaceScore - Current value of the score set by the ddnet code
int CGameControllerDDNetPP::SnapPlayerScore(int SnappingClient, CPlayer *pPlayer, int DDRaceScore)
{
	if(SnappingClient < 0 || SnappingClient >= MAX_CLIENTS)
		return DDRaceScore;

	CPlayer *pSnapReceiver = GameServer()->m_apPlayers[SnappingClient];
	if(!pSnapReceiver)
		return DDRaceScore;

	if(pSnapReceiver->IsInstagibMinigame())
	{
		if(pPlayer->IsInstagibMinigame())
		{
			if(pPlayer->m_ScoreFixForDDNet)
				return pPlayer->m_InstaScore * 60;
			else
				return pPlayer->m_InstaScore;
		}
		else
			return -9999;
	}
	if(pSnapReceiver->m_IsSurvivaling)
	{
		if(pPlayer->m_IsSurvivaling)
		{
			if(pSnapReceiver->m_ScoreFixForDDNet)
				return pPlayer->m_Account.m_SurvivalKills * 60;
			else
				return pPlayer->m_Account.m_SurvivalKills;
		}
		else
			return -9999;
	}
	else if(pSnapReceiver->m_DisplayScore != SCORE_TIME)
	{
		if(pSnapReceiver->m_DisplayScore == SCORE_LEVEL)
		{
			if(pPlayer->IsLoggedIn())
			{
				if(pSnapReceiver->m_ScoreFixForDDNet)
					return pPlayer->GetLevel() * 60;
				else
					return pPlayer->GetLevel();
			}
			else if(pSnapReceiver->m_ScoreFixForDDNet)
				return -9999;
			else
				return 0;
		}
		else if(pSnapReceiver->m_DisplayScore == SCORE_BLOCK)
		{
			if(pPlayer->IsLoggedIn())
			{
				if(pSnapReceiver->m_ScoreFixForDDNet)
					return pPlayer->m_Account.m_BlockPoints * 60;
				else
					return pPlayer->m_Account.m_BlockPoints;
			}
			else if(pSnapReceiver->m_ScoreFixForDDNet)
				return -9999;
			else
				return 0;
		}
		else if(pSnapReceiver->m_DisplayScore == SCORE_CURRENT_SPREE)
		{
			if(pSnapReceiver->m_ScoreFixForDDNet)
				return pPlayer->m_KillStreak * 60;
			else
				return pPlayer->m_KillStreak;
		}
		else if(pSnapReceiver->m_DisplayScore == SCORE_KING_OF_THE_HILL)
		{
			if(pSnapReceiver->m_ScoreFixForDDNet)
				return pPlayer->m_KingOfTheHillScore * 60;
			else
				return pPlayer->m_KingOfTheHillScore;
		}
	}

	// TODO: watafak is this? can we please delete it?
	//       fully instagib mode servers should be deprecated
	//       and ddnet-insta should be used instead
	//       in ddnet++ there should only be minigames
	//       if you want to host a server that is only about one minigame
	//       then you need a regular ddrace lobby and a minigame join tile
	//       or some auto join config
	//       but then all the things like snapping score should be done by
	//       the minigame and not by the config that activates the minigame on spawn
	//
	//       if you want to mess with the global score type
	//       use sv_display_score
	if(g_Config.m_SvInstagibMode || !g_Config.m_SvDDPPscore)
		return pPlayer->m_MinigameScore;

	// this is a bit cursed
	// it is used for 1vs1 for now
	// but ideally it would also be used for survival where SCORE_BLOCK sounds a bit wrong
	CMinigame *pMinigame = GameServer()->GetMinigame(pSnapReceiver->GetCid());
	if(pMinigame && pMinigame->ScoreType() == SCORE_BLOCK)
		return pPlayer->m_MinigameScore;
	return DDRaceScore;
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
