// grenade fng

#include "rifle_fng.h"

#include <engine/shared/config.h>

#include <game/server/gamecontext.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos)*32)

CDummyRifleFng::CDummyRifleFng(class CPlayer *pPlayer) :
	CDummyBase(pPlayer, DUMMYMODE_RIFLE_FNG)
{
}

void CDummyRifleFng::OnTick()
{
	int OffsetX = g_Config.m_SvDummyMapOffsetX * 32; //offset for ChillBlock5 -667
	int OffsetY = g_Config.m_SvDummyMapOffsetY * 32;

	Hook(0);
	Jump(0);
	StopMoving();
	Fire(0);

	//attack enemys
	CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
	if(pChr && pChr->IsAlive())
	{
		AimPos(pChr->GetPos());

		if(pChr->m_FreezeTime < 1) //alive enemy --> attack
		{
			if(GetPos().x < pChr->GetPos().x)
			{
				Right();
			}
			else
			{
				Left();
			}

			if(Server()->Tick() % 100 == 0)
			{
				Fire();
			}
		}
		else //frozen enemy --> sacarfire
		{
		}
	}

	//don't fall in holes
	if(GetPos().x + OffsetX > 90 * 32 && GetPos().x + OffsetX < 180 * 32) //map middle (including 3 fall traps)
	{
		if(GetPos().y + OffsetY > 73 * 32)
		{
			Jump();
			//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "hopsa");
		}
	}

	//check for stucking in walls
	if(GetDirection() != 0 && GetVel().x == 0.0f)
	{
		if(Server()->Tick() % 60 == 0)
		{
			Jump();
		}
	}
}
