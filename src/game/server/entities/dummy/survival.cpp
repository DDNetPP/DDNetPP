// generic survival minigame bot
// attacking and approaching close players

#include "survival.h"

#include "../character.h"

#include <base/math_ddpp.h>

#include <engine/shared/config.h>

#include <game/server/gamecontext.h>
#include <game/server/player.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos) * 32)

CDummySurvival::CDummySurvival(class CPlayer *pPlayer) :
	CDummyBase(pPlayer, DUMMYMODE_SURVIVAL)
{
	OnDeath();
}

void CDummySurvival::OnDeath()
{
	m_DummyDir = 0;
}

void CDummySurvival::OnTick()
{
	Jump(0);
	Fire(0);
	Hook(0);
	StopMoving();
	//m_pPlayer->m_TeeInfos.m_ColorBody = (0 * 255 / 360);

	CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), false, m_pCharacter);
	if(pChr && pChr->IsAlive())
	{
		AimPos(pChr->GetPos());

		if(Server()->Tick() % 10 == 0) //random aim strong every 10 secs
		{
			int RandXInp = pChr->GetPos().x - GetPos().x + (rand() % 200) - 100;
			int RandYInp = pChr->GetPos().y - GetPos().y + (rand() % 200) - 100;

			AimX(RandXInp);
			AimY(RandYInp);
		}
		else //aim normal bad
		{
			int RandXInp = pChr->GetPos().x - GetPos().x + (rand() % 20) - 10;
			int RandYInp = pChr->GetPos().y - GetPos().y + (rand() % 20) - 10;

			AimX(RandXInp);
			AimY(RandYInp);
		}

		//Fire();

		if(Server()->Tick() % 120 == 0)
		{
			SetWeapon(1); //randomly swap to gun
		}

		if(GetPos().x > pChr->GetPos().x - 80 && GetPos().x < pChr->GetPos().x + 80)
		{
			if(Server()->Tick() % 20 == 0)
			{
				SetWeapon(0); //switch hammer in close range
			}
			Fire();
		}
		else if(GetPos().x > pChr->GetPos().x)
		{
			Left();
		}
		else
		{
			Right();
		}

		if(Server()->Tick() % 20 == 0)
		{
			if(rand() % 20 > 8)
			{
				Fire();
			}
		}

		if(GetPos().y > pChr->GetPos().y - 5 * 32 && GetPos().y < pChr->GetPos().y + 5 * 32) //same height
		{
			if(GetPos().x > pChr->GetPos().x - 6 * 32 && GetPos().x < pChr->GetPos().x + 6 * 32) //hook range
			{
				if(Server()->Tick() % 10 == 0) //angry
				{
					GameServer()->SendEmoticon(m_pPlayer->GetCid(), 9, -1);
				}

				Hook();
				if(rand() % 6 == 0)
				{
					Hook(0);
				}
			}
			else if(GetPos().x > pChr->GetPos().x - 15 * 32 && GetPos().x < pChr->GetPos().x + 15 * 32) //combat range
			{
			}
			else if(GetPos().x > pChr->GetPos().x) //move towards enemy from to right side <---
			{
				int RandXInp = (rand() % 60 + 15) * -1;
				int RandYInp = rand() % 100 * -1;

				AimX(RandXInp);
				AimY(RandYInp);

				if(GetVel().y > -0.6f)
				{
					Hook();
				}
				if(IsGrounded())
				{
					Jump();
				}
			}
			else if(GetPos().x < pChr->GetPos().x) //move towards enemy from the left side ---->
			{
				int RandXInp = (rand() % 60) + 15;
				int RandYInp = rand() % 100 * -1;

				AimX(RandXInp);
				AimY(RandYInp);

				if(GetVel().y > -0.6f)
				{
					Hook();
				}
				if(IsGrounded())
				{
					Jump();
				}
			}
		}
		else if(GetPos().y > pChr->GetPos().y) //too low
		{
			int RandXInp = (rand() % 60) - 30;
			int RandYInp = rand() % 120 * -1;
			Hook();

			AimX(RandXInp);
			AimY(RandYInp);

			if(rand() % 3 == 1)
			{
				Hook(0);
			}

			if(GameServer()->Collision()->GetCollisionAt(GetPos().x, GetPos().y - 32) == 1) //collision in the way
			{
				Right();
				//m_pPlayer->m_TeeInfos.m_ColorBody = (120 * 255 / 360);
			}
		}
		else if(GetPos().y < pChr->GetPos().y) //too high
		{
			int RandXInp = (rand() % 60) - 30;
			int RandYInp = rand() % 120;
			Hook();

			AimX(RandXInp);
			AimY(RandYInp);

			if(rand() % 3 == 1)
			{
				Hook(0);
			}
		}

		//tile bypassing

		if(GetPos().y > pChr->GetPos().y + 50) //too low
		{
			if(GameServer()->Collision()->GetCollisionAt(GetPos().x, GetPos().y - 32) == 1 || GameServer()->Collision()->GetCollisionAt(GetPos().x, GetPos().y + 32) == 3) //collision in the way
			{
				SetDirection(m_DummyDir);
				//m_pPlayer->m_TeeInfos.m_ColorBody = (120 * 255 / 360);
			}
		}
		else if(GetPos().y < pChr->GetPos().y - 50) //high low
		{
			if(GameServer()->Collision()->GetCollisionAt(GetPos().x, GetPos().y + 32) == 1 || GameServer()->Collision()->GetCollisionAt(GetPos().x, GetPos().y + 32) == 3) //collision in the way
			{
				SetDirection(m_DummyDir);
				//m_pPlayer->m_TeeInfos.m_ColorBody = (120 * 255 / 360);
			}
		}
	}

	// avoid killtiles
	if(GameServer()->Collision()->GetCollisionAt(GetPos().x - 60, GetPos().y) == 2 || GameServer()->Collision()->GetCollisionAt(GetPos().x - 60, GetPos().y + 30) == 2)
	{
		Right();
	}
	else if(GameServer()->Collision()->GetCollisionAt(GetPos().x + 60, GetPos().y) == 2 || GameServer()->Collision()->GetCollisionAt(GetPos().x + 60, GetPos().y + 30) == 2)
	{
		Left();
	}
	// flybot for fly parts (working in 10% of the cases)
	if(!IsGrounded())
	{
		// falling on killtiles
		if(GameServer()->Collision()->GetCollisionAt(GetPos().x, GetPos().y + (5 * 32)) == 2)
		{
			AimX(2);
			AimY(-200);
			if(GetVel().y > 0.0f)
			{
				Hook();
			}
			else
			{
				Hook(0);
			}
		}
	}

	// check for stucking in walls
	if(GetDirection() != 0 && GetVel().x == 0.0f)
		if(Server()->Tick() % 60 == 0)
			Jump();

	// slow tick
	if(Server()->Tick() % 200 == 0)
	{
		// escape stuck hook
		Hook(0);

		// anti stuck whatever
		if(m_DummyDir == 1 && (GameServer()->Collision()->GetCollisionAt(GetPos().x + 20, GetPos().y) == 1 || GameServer()->Collision()->GetCollisionAt(GetPos().x + 20, GetPos().y) == 3))
		{
			m_DummyDir = -1;
		}
		else // if (m_DummyDir == -1 && (GameServer()->Collision()->GetCollisionAt(GetPos().x - 20, GetPos().y) == 1 || GameServer()->Collision()->GetCollisionAt(GetPos().x - 20, GetPos().y) == 3))
		{
			m_DummyDir = 1;
		}

		// end game if all humans dead
		if(GameServer()->m_survivalgamestate > 1) // survival game running
		{
			if(m_pPlayer->m_IsSurvivalAlive)
			{
				int AliveHumans = 0;
				for(auto &Player : GameServer()->m_apPlayers)
					if(Player && !Player->m_IsDummy) //check all humans
						if(Player->m_IsSurvivaling && Player->m_IsSurvivalAlive) // surival alive
							AliveHumans++;
				if(!AliveHumans) //all humans dead --> suicide to get new round running
				{
					Die();
					//dbg_msg("survival", "all humans dead suicide");
				}
				//else
				//{
				//	dbg_msg("survival","%d humans alive", AliveHumans);
				//}
			}
		}
	}
}
