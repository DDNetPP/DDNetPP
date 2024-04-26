// gamecontext scoped minigame ddnet++ methods

#include <engine/shared/config.h>
#include <game/server/teams.h>

#include <cinttypes>
#include <cstring>

#include "gamecontext.h"

EScore CGameContext::MinigameScoreType(int ClientId)
{
	for(auto &Minigame : m_vMinigames)
	{
		if(!Minigame)
			continue;

		if(Minigame->IsActive(ClientId))
			return Minigame->ScoreType();
	}
	return SCORE_BLOCK;
}

int CGameContext::IsMinigame(int playerId) //if you update this function please also update the '/minigames' chat command
{
	CPlayer *pPlayer = m_apPlayers[playerId];
	if(!pPlayer)
		return 0;
	CCharacter *pChr = GetPlayerChar(playerId);

	if(pPlayer->m_Account.m_JailTime)
	{
		return -1;
	}
	if(pPlayer->m_IsInstaArena_gdm)
	{
		return 1;
	}
	if(pPlayer->m_IsInstaArena_idm)
	{
		return 2;
	}
	if(pPlayer->m_IsBalanceBatteling)
	{
		return 3;
	}
	if(pPlayer->m_IsSurvivaling)
	{
		return 4;
	}
	//if (pPlayer->m_Ischidraqul3) //dont return the broadcast only game because it doesnt make too much trouble. You can play chidraqul in jail or while being in insta no problem.
	//{
	//	return x;
	//}
	if(pChr)
	{
		if(pChr->m_IsBombing)
		{
			return 5;
		}
		if(pChr->m_IsPVParena)
		{
			return 6;
		}
	}
	if(pPlayer->m_IsBlockWaving)
	{
		return 7;
	}
	if(pPlayer->m_IsBlockTourning)
	{
		return 8;
	}
	if(pPlayer->m_IsBlockDeathmatch)
	{
		return 9;
	}

	return 0;
}

const char *CGameContext::GetBlockSkillGroup(int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return "error";

	if(pPlayer->m_Account.m_BlockSkill < 1000)
	{
		return "nameless tee";
	}
	else if(pPlayer->m_Account.m_BlockSkill < 3000)
	{
		return "brainless tee";
	}
	else if(pPlayer->m_Account.m_BlockSkill < 6000)
	{
		return "novice tee";
	}
	else if(pPlayer->m_Account.m_BlockSkill < 9000)
	{
		return "moderate tee";
	}
	else if(pPlayer->m_Account.m_BlockSkill < 15000)
	{
		return "brutal tee";
	}
	else if(pPlayer->m_Account.m_BlockSkill >= 20000)
	{
		return "insane tee";
	}
	else
	{
		return "unranked";
	}
}

int CGameContext::GetBlockSkillGroupInt(int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return -1;

	if(pPlayer->m_Account.m_BlockSkill < 1000)
	{
		return 1;
	}
	else if(pPlayer->m_Account.m_BlockSkill < 3000)
	{
		return 2;
	}
	else if(pPlayer->m_Account.m_BlockSkill < 6000)
	{
		return 3;
	}
	else if(pPlayer->m_Account.m_BlockSkill < 9000)
	{
		return 4;
	}
	else if(pPlayer->m_Account.m_BlockSkill < 15000)
	{
		return 5;
	}
	else if(pPlayer->m_Account.m_BlockSkill >= 20000)
	{
		return 6;
	}
	else
	{
		return 0;
	}
}

void CGameContext::UpdateBlockSkill(int value, int id)
{
	CPlayer *pPlayer = m_apPlayers[id];
	if(!pPlayer)
		return;

	int oldrank = GetBlockSkillGroupInt(id);
	pPlayer->m_Account.m_BlockSkill += value; //update skill
	if(pPlayer->m_Account.m_BlockSkill < 0)
	{
		pPlayer->m_Account.m_BlockSkill = 0; //never go less than zero
	}
	else if(pPlayer->m_Account.m_BlockSkill > 25000)
	{
		pPlayer->m_Account.m_BlockSkill = 25000; //max skill lvl
	}
	int newrank = GetBlockSkillGroupInt(id);
	if(newrank != oldrank)
	{
		char aBuf[128];
		if(newrank < oldrank) //downrank
		{
			str_format(aBuf, sizeof(aBuf), "[BLOCK] New skillgroup '%s' (downrank)", GetBlockSkillGroup(id));
			SendChatTarget(id, aBuf);
			UpdateBlockSkill(-590, id); //lower skill agian to not get an uprank too fast agian
		}
		else //uprank
		{
			str_format(aBuf, sizeof(aBuf), "[BLOCK] New skillgroup '%s' (uprank)", GetBlockSkillGroup(id));
			SendChatTarget(id, aBuf);
			UpdateBlockSkill(+590, id); //push skill agian to not get an downrank too fast agian
		}
	}
}
