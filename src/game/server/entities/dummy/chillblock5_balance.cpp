// ChillBlock5 specific balancing mode
// not sure exactly what this is :D
// maybe its the taxi from the old tunnel?
//
// mode 30

#include "chillblock5_balance.h"

#include "../character.h"

#include <base/math_ddpp.h>

#include <engine/shared/config.h>

#include <game/server/gamecontext.h>
#include <game/server/player.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos)*32)

CDummyChillBlock5Balance::CDummyChillBlock5Balance(class CPlayer *pPlayer) :
	CDummyBase(pPlayer, DUMMYMODE_CHILLBLOCK5_BALANCE)
{
}

void CDummyChillBlock5Balance::OnTick()
{
	//rest dummy
	Hook(0);
	Jump(0);
	StopMoving();
	Fire(0);

	//hardcodetselfkills:
	if(GetPos().x > 404 * 32)
	{
		Die();
	}
	if(GetPos().y < 204 * 32)
	{
		Die();
	}
	if(GetPos().y < 215 * 32 && GetPos().x < 386 * 32 - 3)
	{
		Die();
	}
	//selfkill
	//dyn
	if(GetVel().y == 0.000000f && GetVel().x < 0.01f && GetVel().x > -0.01f && IsFrozen())
	{
		if(Server()->Tick() % 20 == 0)
		{
			GameServer()->SendEmoticon(m_pPlayer->GetCid(), 3, -1);
		}

		if(Server()->Tick() % 90 == 0)
		{
			Die();
		}
	}

	//balance
	CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
	if(pChr && pChr->IsAlive())
	{
		Aim(2, 200);

		if(pChr->GetPos().y > GetPos().y && GetPos().x > 310 * 32)
		{
			Right();
			if(pChr->GetPos().x < GetPos().x - 3)
			{
				Left();
			}
			if(pChr->GetPos().x > GetPos().x + 1)
			{
				Right();
			}
			if(GetPos().x > pChr->GetPos().x + 1 && GetPos().y > 238 * 32 && pChr->IsGrounded() && GetVel().x < -0.002f)
			{
				Fire();
				Left();
			}
		}
	}

	//movement going down#
	if(GetPos().y < 238 * 32 && GetPos().x > 344 * 32)
	{
		if(GetPos().x > 390 * 32)
		{
			Left();
		}
		if(GetPos().x < 388 * 32)
		{
			Right();
		}
	}
	else
	{
		if(Server()->Tick() % 40 == 0)
		{
			SetWeapon(0);
		}
	}

	if((GetPos().x < (314 * 32 - 10)) && GetVel().x < -0.001f)
	{
		// CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
		if(pChr && pChr->IsAlive())
		{
			AimPos(pChr->GetPos());

			if(GetPos().x > 310 * 32)
			{
				Jump();
			}
			if(GetPos().x > 305 * 32)
			{
				Left();
			}
			if(GetPos().x < 308 * 32 + 10 && pChr->m_FreezeTime > 0)
			{
				Hook();
			}
		}
	}
}
