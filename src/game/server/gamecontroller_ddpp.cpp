#include "gamecontroller.h"

#include <engine/shared/config.h>

#include <game/server/score.h>

int IGameController::SnapScoreLimit(int SnappingClient)
{
	return g_Config.m_SvScorelimit;
}

int IGameController::ServerInfoClientScoreValue(CPlayer *pPlayer)
{
	return pPlayer->m_Score.value_or(-9999);
}
