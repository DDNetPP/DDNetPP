#include <engine/shared/config.h>
#include <game/server/score.h>

#include "gamecontroller.h"

int IGameController::SnapPlayerScore(int SnappingClient, CPlayer *pPlayer, int DDRaceScore)
{
	if(Server()->IsSixup(SnappingClient))
	{
		// Times are in milliseconds for 0.7
		return pPlayer->m_Score.has_value() ? GameServer()->Score()->PlayerData(pPlayer->GetCid())->m_BestTime * 1000 : -1;
	}

	return DDRaceScore;
}

int IGameController::SnapScoreLimit(int SnappingClient)
{
	return g_Config.m_SvScorelimit;
}

int IGameController::ServerInfoClientScoreValue(CPlayer *pPlayer)
{
	return pPlayer->m_Score.value_or(-9999);
}
