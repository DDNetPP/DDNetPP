#include <game/server/gamemodes/ddnetpp/ddnetpp.h>

bool CGameControllerDDNetPP::CanSpawn(int Team, vec2 *pOutPos, class CPlayer *pPlayer, int DDTeam)
{
	CSpawnEval Eval;

	if(pPlayer->m_IsInstaArena_gdm)
	{
		EvaluateSpawnType(&Eval, 1, DDTeam); //red (not bloody anymore)
	}
	else if(pPlayer->m_IsInstaArena_idm)
	{
		EvaluateSpawnType(&Eval, 2, DDTeam); //blue
	}
	else if(pPlayer->m_IsSurvivaling)
	{
		int Id = pPlayer->GetCid();
		Eval.m_Pos = pPlayer->m_IsSurvivalAlive ? GameServer()->GetNextSurvivalSpawn(Id) : GameServer()->GetSurvivalLobbySpawn(Id);
		if(Eval.m_Pos == vec2(-1, -1)) // fallback to ddr spawn if there is no arena
			EvaluateSpawnType(&Eval, 0, DDTeam); //default
		else
			Eval.m_Got = true;
	}
	else
	{
		bool IsMinigameSpawn = false;
		for(auto &Minigame : GameServer()->m_vMinigames)
		{
			if(Minigame->PickSpawn(&Eval.m_Pos, pPlayer))
			{
				Eval.m_Got = true;
				IsMinigameSpawn = true;
				break;
			}
		}
		if(!IsMinigameSpawn)
			EvaluateSpawnType(&Eval, 0, DDTeam); //default
	}

	if(Eval.m_Got)
	{
		*pOutPos = Eval.m_Pos;
		return true;
	}

	return CGameControllerDDRace::CanSpawn(Team, pOutPos, pPlayer, DDTeam);
}
