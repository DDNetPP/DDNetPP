// dummymode -6
// blmapv3 1o1 mode
//
// blocker bot for the iconic v3 main area
// designed for maps that have the v3 area
// as a specific arena one can spawn into

#include "blmapv3_arena.h"

#include "../character.h"

#include <base/math_ddpp.h>

#include <engine/shared/config.h>

#include <game/server/gamecontext.h>
#include <game/server/player.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos) * 32)

CDummyBlmapV3Arena::CDummyBlmapV3Arena(class CPlayer *pPlayer) :
	CDummyBase(pPlayer, DUMMYMODE_BLMAPV3_ARENA)
{
}

void CDummyBlmapV3Arena::OnTick()
{
	//rest dummy
	Hook(0);
	Jump(0);
	StopMoving();
	Fire(0);

	CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
	if(pChr && pChr->IsAlive())
	{
		AimPos(pChr->GetPos());

		/*************************************************
		 *                                                *
		 *                A T T A C K                     *
		 *                                                *
		 **************************************************/

		//swing enemy up
		if(GetPos().y < pChr->GetPos().y - 20 && !IsGrounded() && !pChr->isFreezed)
		{
			Hook();
			float dist = distance(pChr->GetPos(), GetPos());
			if(dist < 250.f)
			{
				if(GetPos().x < pChr->GetPos().x)
				{
					Left();
				}
				else
				{
					Right();
				}
				if(dist < 80.f) // hammer dist
				{
					if(absolute(pChr->GetVel().x) > 2.6f)
					{
						if(m_pCharacter->m_FreezeTime == 0)
						{
							Fire();
						}
					}
				}
			}
		}

		//attack in mid
		if(pChr->GetPos().x > 393 * 32 - 7 + V3_OFFSET_X && pChr->GetPos().x < 396 * 32 + 7 + V3_OFFSET_X)
		{
			if(pChr->GetPos().x < GetPos().x) // bot on the left
			{
				if(pChr->GetVel().x < 0.0f)
				{
					Hook();
				}
				else
				{
					Hook(0);
				}
			}
			else // bot on the right
			{
				if(pChr->GetVel().x < 0.0f)
				{
					Hook(0);
				}
				else
				{
					Hook();
				}
			}

			//if (pChr->GetPos().y > 77 * 32 - 1 + V3_OFFSET_Y && pChr->IsGrounded() == false && pChr->isFreezed)
			//{
			//	Hook(0); //rekt -> let him fall
			//}
			if(pChr->isFreezed)
			{
				Hook(0);
			}
		}

		//attack bot in the middle and enemy in the air -> try to hook down
		/* quick ascii art xd
						#    #
					#   #    #   #
			enemy    #####    #####


					bot
					#####    #####
					#   #    #   #
						#
			###########################
		*/
		if(GetPos().y < 78 * 32 + V3_OFFSET_Y && GetPos().y > 70 * 32 + V3_OFFSET_Y && IsGrounded()) // if bot is in position
		{
			if(pChr->GetPos().x < 389 * 32 + V3_OFFSET_X || pChr->GetPos().x > 400 * 32 + V3_OFFSET_X) //enemy on the left side
			{
				if(pChr->GetPos().y < 76 * 32 + V3_OFFSET_Y && pChr->GetVel().y > 4.2f)
				{
					Hook();
				}
			}

			if(HookState() == HOOK_FLYING)
			{
				Hook();
			}
			else if(HookState() == HOOK_GRABBED)
			{
				Hook();
				//stay strong and walk agianst hook pull
				if(GetPos().x < 392 * 32 + V3_OFFSET_X) //left side
				{
					Right();
				}
				else if(GetPos().x > 397 * 32 + V3_OFFSET_X) //right side
				{
					Left();
				}
			}
		}

		// attack throw into left freeze wall
		if(GetPos().x < 383 * 32 + V3_OFFSET_X)
		{
			if(pChr->GetPos().y > GetPos().y + 190)
			{
				Hook();
			}
			else if(pChr->GetPos().y < GetPos().y - 190)
			{
				Hook();
			}
			else
			{
				if(pChr->GetVel().x < -1.6f)
				{
					if(pChr->GetPos().x < GetPos().x - 7 && pChr->GetPos().x > GetPos().x - 90) //enemy on the left side
					{
						if(pChr->GetPos().y < GetPos().y + 90 && pChr->GetPos().y > GetPos().y - 90)
						{
							if(m_pCharacter->m_FreezeTime == 0)
							{
								Fire();
							}
						}
					}
				}
			}
		}

		// attack throw into right freeze wall
		if(GetPos().x > 404 * 32 + V3_OFFSET_X)
		{
			if(pChr->GetPos().y > GetPos().y + 190)
			{
				Hook();
			}
			else if(pChr->GetPos().y < GetPos().y - 190)
			{
				Hook();
			}
			else
			{
				if(pChr->GetVel().x > 1.6f)
				{
					if(pChr->GetPos().x > GetPos().x + 7 && pChr->GetPos().x < GetPos().x + 90) //enemy on the right side
					{
						if(pChr->GetPos().y > GetPos().y - 90 && pChr->GetPos().y < GetPos().y + 90)
						{
							if(m_pCharacter->m_FreezeTime == 0)
							{
								Fire();
							}
						}
					}
				}
			}
		}

		/*************************************************
		 *                                                *
		 *                D E F E N D (move)              *
		 *                                                *
		 **************************************************/

		//########################################
		//Worst hammer switch code eu west rofl! #
		//########################################
		//switch to hammer if enemy is near enough
		if(pChr->GetPos().x > GetPos().x + 130)
		{
			//default is gun
			SetWeapon(1);
		}
		else if(pChr->GetPos().x < GetPos().x - 130)
		{
			//default is gun
			SetWeapon(1);
		}
		else
		{
			//switch to hammer if enemy is near enough
			if(pChr->GetPos().y > GetPos().y + 130)
			{
				//default is gun
				SetWeapon(1);
			}
			else if(pChr->GetPos().y < GetPos().y - 130)
			{
				//default is gun
				SetWeapon(1);
			}
			else
			{
				//near --> hammer
				SetWeapon(0);
			}
		}

		//Starty movement
		if(GetPos().x < 389 * 32 + V3_OFFSET_X && GetPos().y > 79 * 32 + V3_OFFSET_Y && pChr->GetPos().y > 79 * 32 + V3_OFFSET_Y && pChr->GetPos().x > 398 * 32 + V3_OFFSET_X && IsGrounded() && pChr->IsGrounded())
		{
			Jump();
		}
		if(GetPos().x < 389 * 32 + V3_OFFSET_X && pChr->GetPos().x > 307 * 32 + V3_OFFSET_X && pChr->GetPos().x > 398 * 32 + V3_OFFSET_X)
		{
			Right();
		}

		//important freeze doges leave them last!:

		//freeze prevention mainpart down right
		if(GetPos().x > 397 * 32 + V3_OFFSET_X && GetPos().x < 401 * 32 + V3_OFFSET_X && GetPos().y > 78 * 32 + V3_OFFSET_Y)
		{
			Jump();
			Hook();
			if(Server()->Tick() % 20 == 0)
			{
				Hook(0);
				Jump(0);
			}
			Right();
			AimX(200);
			AimY(80);
		}

		//freeze prevention mainpart down left
		if(GetPos().x > 387 * 32 + V3_OFFSET_X && GetPos().x < 391 * 32 + V3_OFFSET_X && GetPos().y > 78 * 32 + V3_OFFSET_Y)
		{
			Jump();
			Hook();
			if(Server()->Tick() % 20 == 0)
			{
				Hook(0);
				Jump(0);
			}
			Left();
			AimX(-200);
			AimY(80);
		}

		//Freeze prevention top left
		if(GetPos().x < 391 * 32 + V3_OFFSET_X && GetPos().y < 71 * 32 + V3_OFFSET_Y && GetPos().x > 387 * 32 - 10 + V3_OFFSET_X)
		{
			Left();
			Hook();
			if(Server()->Tick() % 20 == 0)
			{
				Hook(0);
			}
			AimX(-200);
			AimY(-87);
			if(GetPos().y > 19 * 32 + 20)
			{
				AimX(-200);
				AimY(-210);
			}
		}

		//Freeze prevention top right
		if(GetPos().x < 402 * 32 + 10 + V3_OFFSET_X && GetPos().y < 71 * 32 + V3_OFFSET_Y && GetPos().x > 397 * 32 + V3_OFFSET_X)
		{
			Right();
			Hook();
			if(Server()->Tick() % 20 == 0)
			{
				Hook(0);
			}
			AimX(200);
			AimY(-87);
			if(GetPos().y > 67 * 32 + 20 + V3_OFFSET_Y)
			{
				AimX(200);
				AimY(-210);
			}
		}

		//Freeze prevention mid
		if(GetPos().x > 393 * 32 - 7 + V3_OFFSET_X && GetPos().x < 396 * 32 + 7 + V3_OFFSET_X)
		{
			if(GetVel().x < 0.0f)
			{
				Left();
			}
			else
			{
				Right();
			}

			if(GetPos().y > 77 * 32 - 1 + V3_OFFSET_Y && !IsGrounded())
			{
				Jump();
				if(Jumped() > 2) //no jumps == rip   --> panic hook
				{
					Hook();
					if(Server()->Tick() % 15 == 0)
					{
						Hook(0);
					}
				}
			}
		}

		//Freeze prevention left
		if(GetPos().x < 380 * 32 + V3_OFFSET_X || (GetPos().x < 382 * 32 + V3_OFFSET_X && GetVel().x < -8.4f))
		{
			Right();
		}
		//Freeze prevention right
		if(GetPos().x > 408 * 32 + V3_OFFSET_X || (GetPos().x > 406 * 32 + V3_OFFSET_X && GetVel().x > 8.4f))
		{
			Left();
		}

		/*************************************************
		 *                                                *
		 *      ChillBlock5 v015 extensions around v3     *
		 *                                                *
		 **************************************************/

		// 3 tile freze pit on the floor
		// right side of v3 arena
		if(GetPos().x > 396 * 32 && GetPos().x < 402 * 32)
		{
			// TODO: this is not very skilled :D
			Left();

			if(GetPos().y > 80 * 32)
				Jump();
		}

		// 3 tile freze pit on the floor
		// left side of v3 arena
		if(GetPos().x > 364 * 32 && GetPos().x < 370 * 32)
		{
			// TODO: this is not very skilled :D
			Right();

			if(GetPos().y > 80 * 32)
				Jump();
		}
	}
}
