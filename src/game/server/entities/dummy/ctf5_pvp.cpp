// ctf5

#include "ctf5_pvp.h"

#include "../character.h"

#include <base/math_ddpp.h>

#include <engine/shared/config.h>

#include <game/server/gamecontext.h>
#include <game/server/player.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos) * 32)

CDummyCtf5Pvp::CDummyCtf5Pvp(class CPlayer *pPlayer) :
	CDummyBase(pPlayer, DUMMYMODE_CTF5_PVP)
{
}

void CDummyCtf5Pvp::OnTick()
{
	Jump(0);
	Fire(0);
	Hook(0);
	StopMoving();

	CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), false, m_pCharacter);
	if(pChr && pChr->IsAlive())
	{
		vec2 Enemy = pChr->GetPos();
		Enemy.y -= 20;
		AimPos(Enemy);
		Fire();

		if(GetPos().y - 4 * 32 > pChr->GetPos().y)
			if(Server()->Tick() % 20 == 0)
				Jump();
		if(GetPos().x > pChr->GetPos().x)
		{
			Left();
			if(GetVel().x == 0.0f)
				if(Server()->Tick() % 20 == 0)
					Jump();
		}
		else
		{
			Right();
			if(GetVel().x == 0.0f)
				if(Server()->Tick() % 20 == 0)
					Jump();
		}
	}
}
