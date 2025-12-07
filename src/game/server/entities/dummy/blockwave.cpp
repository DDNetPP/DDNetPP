// dummy mode -3
// BlockWave

#include "blockwave.h"

#include <game/server/gamecontext.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos) * 32)

CDummyBlockWave::CDummyBlockWave(class CPlayer *pPlayer) :
	CDummyBase(pPlayer, DUMMYMODE_BLOCKWAVE)
{
}

void CDummyBlockWave::OnTick()
{
	//rest dummy (zuruecksetzten)
	Hook(0);
	Jump(0);
	StopMoving();
	Fire(0);

	CCharacter *pChr = GameServer()->m_World.ClosestCharTypeNotInFreeze(GetPos(), true, m_pCharacter, false);
	if(pChr && pChr->IsAlive())
	{
		AimPos(pChr->GetPos());

		if(GetPos().y < pChr->GetPos().y - 60)
			Hook();

		if(GetPos().x < pChr->GetPos().x - 40)
			Right();
		else
			Left();

		if(GetPos().x > 411 * 32 && GetPos().x < 420 * 32) //hookbattle left entry
		{
			if(pChr->GetPos().x < GetPos().x + 8 * 32) //8 tiles hookrange
				Hook();

			if(HookState() == HOOK_GRABBED)
				Left();

			//rehook
			if(Server()->Tick() % 15 == 0 && HookState() != HOOK_GRABBED)
				Hook(0);
		}

		if(pChr->m_FreezeTime) //go full yolo and ignore all freeze just hook the enemy into freeze (kamikaze style)
		{
			Hook();

			if(GetPos().x > 446 * 32) //right side of main area and right spawn
				Right();
			else if(GetPos().x > 425 * 32 && GetPos().x < 434 * 32) //left side of the base prefer the freeze hole cuz its less kamikaze
				Right();
			else
				Left();

			if(Server()->Tick() % 9 == 0)
				Jump();

			if(Server()->Tick() % 15 == 0 && HookState() != HOOK_GRABBED)
				Hook(0);
		}
		else //no frozen enemy --> dont run into freeze
		{
			if(GetPos().x > 453 * 32 && GetPos().x < 457 * 32)
				Left(); //dont run into right entry of main area;
		}
	}

	//Care your bot mates c:
	CCharacter *pBot = GameServer()->m_World.ClosestCharTypeDummy(GetPos(), m_pCharacter);
	if(pBot && pBot->IsAlive())
	{
		//chill dont push all at once
		if(GetPos().x < 404 * 32 && GetPos().x > 393 * 32 && pBot->GetPos().x > GetPos().x + 5 && !IsFrozen(pBot)) //left side of map
			if(pBot->GetPos().x < GetPos().x + 8 * 32) //8 tiles distance
				Left();

		//get dj if mates block the fastlane on entering from left side
		if(GetPos().x > 414 * 32 && GetPos().x < 420 * 32 && !IsGrounded())
			if(pBot->GetPos().x > 420 * 32 && pBot->GetPos().x < 423 * 32 + 20 && pBot->m_FreezeTime)
				Left();
	}

	if(GetPos().y > 262 * 32 && GetPos().x > 404 * 32 && GetPos().x < 415 * 32 && !IsGrounded()) //Likely to fail in the leftest freeze becken
	{
		Jump();
		if(GetDirection() == 0) //never stand still here
		{
			if(GetPos().x > 410 * 32)
				Right();
			else
				Left();
		}
	}

	//basic map dodigen
	if(GetPos().x < 392 * 32) //dont walk in the freeze wall on the leftest side
		Right();

	if(GetPos().y < 257 * 32 && GetVel().y < -4.4f && GetPos().x < 456 * 32) //avoid hitting freeze roof
	{
		Jump();
		Hook();
	}

	if(GetPos().x > 428 * 32 && GetPos().x < 437 * 32) //freeze loch im main becken dodgen
	{
		if(GetPos().y > 263 * 32 && !IsGrounded())
			Jump();

		//position predefines
		if(GetPos().x > 423 * 32)
			Right();
		else
			Left();

		//velocity overrides
		if(GetVel().x > 2.6f)
			Right();
		if(GetVel().x < -2.6f)
			Left();
	}

	if(GetPos().x > 418 * 32 && GetPos().x < 422 * 32) //passing the freeze on the left side
	{
		if(GetPos().x < 420 * 32)
		{
			if(GetVel().x > 0.6f)
			{
				Jump();
			}
		}
		else
		{
			if(GetVel().x < -0.6f)
			{
				Jump();
			}
		}
	}

	if(GetPos().x > 457 * 32) //right spawn --->
	{
		Left();
		if(GetPos().x < 470 * 32 && IsGrounded()) //start border jump
			Jump();
		else if(GetPos().x < 470 * 32 && !IsGrounded() && GetPos().y > 260 * 32)
			Jump();
	}

	if(GetPos().y < 254 * 32 && GetPos().x < 401 * 32 && GetPos().x > 397 * 32) //left spawn upper right freeze wall
		Left();
}
