#include <engine/shared/protocol.h>

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

	// DDNet++ score mod
	if(g_Config.m_SvInstagibMode || !g_Config.m_SvDDPPscore)
		return pPlayer->m_MinigameScore;

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

	return DDRaceScore;
}
