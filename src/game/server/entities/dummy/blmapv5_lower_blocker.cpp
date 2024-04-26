// blmapv5

#include "blmapv5_lower_blocker.h"

#include "../character.h"
#include <base/math_ddpp.h>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos)*32)

CDummyBlmapV5LowerBlocker::CDummyBlmapV5LowerBlocker(class CPlayer *pPlayer) :
	CDummyBase(pPlayer, DUMMYMODE_BLMAPV5_LOWER_BLOCKER)
{
	OnDeath();
}

void CDummyBlmapV5LowerBlocker::OnDeath()
{
	m_angry = 0;
	m_rj_failed = false;
	m_panic_hook = false;
}

void CDummyBlmapV5LowerBlocker::OnTick()
{
	Jump(0);
	Fire(0);
	Hook(0);
	StopMoving();
	int offset_x = g_Config.m_SvDummyMapOffsetX * 32;
	//dbg_msg("debug","cfg=%d cfg*32=%d offset=%d mcore=%d offset+mcore=%d", g_Config.m_SvDummyMapOffsetX, g_Config.m_SvDummyMapOffsetX * 32, offset_x, GetPos().x, offset_x + GetPos().x);

	if(Server()->Tick() % 5000 == 0 && IsFrozen())
	{
		Die();
	}

	if(GetPos().y < 38 * 32) //spawn
	{
		if(GetPos().x + offset_x < 13 * 32 + 10)
		{
			Right();
		}
		else if(GetPos().x + offset_x > 14 * 32 + 16)
		{
			Left();
			//dbg_msg("debug", "walking lefte because %d > %d", offset_x + GetPos().x, 14 * 32 + 16);
		}
		else
		{
			if(GetPos().x + offset_x > 1.2f)
			{
				Left();
			}
			else if(GetPos().x + offset_x < -1.2f)
			{
				Right();
			}
		}
	}
	else if(GetPos().y < 46 * 32) //block area 1
	{
		if(IsGrounded() || GetPos().x + offset_x > 14 * 32 + 10)
		{
			Left();
		}
		if(GetPos().x + offset_x < 14 * 32 + 5 && GetPos().y < 40 * 32 + 30)
		{
			Right();
		}
		if(Server()->Tick() % 60 == 0)
		{
			SetWeapon(3); //switch to grenade
		}
	}
	else if(GetPos().y < 61 * 32) //block area 2
	{
		if(GetPos().y < 52 * 32)
		{
			Left();
		}
		else
		{
			Right();
			AimX(-100);
			AimY(19);
			if(m_pCharacter->m_FreezeTime == 0)
			{
				Fire();
			}
		}
	}
	else if(GetPos().y < 74 * 32) //block area 3
	{
		if(GetPos().x + offset_x > 16 * 32)
		{
			Left();
			AimX(100);
			AimY(19);
			if(m_pCharacter->m_FreezeTime == 0)
			{
				Fire();
			}
		}
		else
		{
			Right();
		}
	}
	else if(GetPos().y < 84 * 32) //block area 4
	{
		Right();
		AimX(-200);
		AimY(90);

		if(IsGrounded() && GetPos().x + offset_x > 23 * 32)
		{
			Jump();
		}
		if(GetVel().y < -0.05f && m_pCharacter->m_FreezeTime == 0)
		{
			Fire();
		}

		if(GetPos().x + offset_x > 22 * 32 && GetVel().x < 7.1f)
		{
			m_rj_failed = true;
		}

		if(m_rj_failed)
		{
			Left();
			if(GetPos().x + offset_x < 18 * 32)
			{
				m_rj_failed = false;
			}
		}
	}
	else //block area 5
	{
		if(GetPos().x + offset_x > 15 * 32 && GetPos().x + offset_x < 22 * 32) //never stay still over the middle freeze
		{
			m_panic_hook = true;
			if(GetPos().x + offset_x > 19 * 32)
			{
				Right();
			}
			else
			{
				Left();
				//dbg_msg("dummy","walk left to dodge the freeze");
			}
			if(GetPos().y > 92 * 32)
			{
				Jump();
				if(Server()->Tick() % 19 == 0)
				{
					Jump(0);
				}
			}
		}

		//block others:
		CCharacter *pChr = GameServer()->m_World.ClosestCharTypeUnfreezedArea5(GetPos(), false, m_pCharacter);
		if(pChr && pChr->IsAlive())
		{
			AimX(pChr->GetPos().x - GetPos().x);
			AimY(pChr->GetPos().y - GetPos().y);

			if((pChr->GetPos().y > GetPos().y + 64 || (m_panic_hook && pChr->GetPos().y < GetPos().y)) && //hook enemys up in freeze or hook enemys down to get speed up and avoid falkling in freeze
				!(GetPos().y < 88 * 32 && GetVel().y < -0.1f)) //but dont do it when the bot is looking but to do a roof nade
			{
				Hook(1);
			}

			if(m_angry)
			{
				if(pChr->GetPos().x + offset_x - 60 > GetPos().x + offset_x)
				{
					Right();
				}
				else if(pChr->GetPos().x + offset_x - 40 < GetPos().x + offset_x)
				{
					Left();
					//dbg_msg("dummy", "walk left to get better enemy positioning enemypos=%f", pChr->GetPos().x + offset_x);
				}
				else //near enough to hammer
				{
					if(Server()->Tick() % 8 == 0)
					{
						SetWeapon(0);
					}
					if(pChr->GetPos().y > GetPos().y && pChr->GetPos().y < GetPos().y + 50)
					{
						if(pChr->GetVel().y < -0.1f && m_pCharacter->m_FreezeTime == 0)
						{
							Fire();
						}
					}
				}

				if(IsGrounded())
				{
					int rand_val = rand() % 7;

					if(rand_val == 1)
					{
						Jump();
					}
					else if(rand_val == 2 || rand_val == 4 || rand_val == 6)
					{
						if(distance(pChr->GetPos(), GetPos()) < 60)
						{
							Fire();
						}
					}
				}
			}
		}

		m_panic_hook = false;

		//angry checker
		if(m_angry)
		{
			if(m_pCharacter->m_FreezeTime == 0)
			{
				m_angry--;
			}
			if(Server()->Tick() % 10 == 0) //angry emotes machen
			{
				GameServer()->SendEmoticon(m_pPlayer->GetCid(), 9, -1);
			}
		}
		if(GetVel().y < -1.2f && GetPos().y < 88 * 32)
		{
			if(Server()->Tick() % 10 == 0 && m_angry == 0)
			{
				m_angry = Server()->TickSpeed() * 20;
			}
		}

		if(GetPos().x + offset_x < 5 * 32) //lower left freeze
		{
			Right();
		}

		if(GetPos().y > 90 * 32 + 16 && GetVel().y < -6.4f) //slow down speed with dj when flying to fast up
		{
			Jump();
		}

		if(GetPos().y < 89 * 32 - 20) //high
		{
			if(Server()->Tick() % 3 == 0)
			{
				SetWeapon(3);
			}
			if(GetPos().y < 88 * 32 && GetVel().y < -0.1f) // near roof
			{
				AimX(1);
				AimY(-200);
				Fire();
			}
		}

		//dont enter the freeze exit on the right side
		if(GetPos().x + offset_x > 34 * 32 && GetPos().y < 91 * 32 && GetVel().x > 0.0f)
		{
			Left();
		}
	}
}