// made by fokkonaut and ChillerDragon

#include "blmapchill_police.h"

#include "../character.h"
#include <game/server/player.h>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos) * 32)

CDummyBlmapChillPolice::CDummyBlmapChillPolice(class CCharacter *pChr, class CPlayer *pPlayer)
: CDummyBase(pChr, pPlayer)
{
	m_LovedX = 0;
	m_LovedY = 0;
	m_LowerPanic = 0;
	m_Speed = 70;
	m_HelpMode = 0;
	m_GrenadeJump = 0;
	m_SpawnTeleporter = 0;
	m_FailedAttempts = 0;
	m_Confused = 0;
	m_Sad = 0;

	m_IsHelpHook = false;
	m_IsClosestPolice = false;
	m_DidRocketjump = false;
	m_HasTouchedGround = false;
	m_HasAlreadyBeenHere = false;
	m_HasStartGrenade = false;
	m_IsDJUsed = false;
	m_HasReachedCinemaEntrance = false;

	m_LastStuckCheckPos = vec2(0, 0);
}

bool CDummyBlmapChillPolice::CheckStuck()
{
	bool IsStuck = false;
	if (TicksPassed(400) && (GetDirection() == DIRECTION_NONE || GetVel().x == 0.0f))
	{
		if (distance(m_LastStuckCheckPos, GetPos()) < 20 * 32)
		{
			IsStuck = true;
			m_Confused++;
			if (m_Confused > 7)
				m_Sad++;
		}
		m_LastStuckCheckPos = GetPos();
	}
	if (m_Confused < 7) // confused
	{
		if (IsStuck && m_Confused > 1)
			GameServer()->SendEmoticon(m_pPlayer->GetCID(), EMOTICON_QUESTION);
		if (m_Confused == 3)
			Jump();
		else if (m_Confused == 4)
		{
			Fire();
			AvoidFreeze();
		}
		else if (m_Confused == 5)
		{
			Right();
			AvoidFreeze();
		}
		else if (m_Confused == 6)
		{
			Left();
			AvoidFreeze();
		}
	}
	else // too confused -> sad
	{
		if (IsStuck)
			GameServer()->SendEmoticon(m_pPlayer->GetCID(), EMOTICON_DROP);
		if (m_Sad > 3)
			m_Sad % 2 ? Left() : Right();
		if (m_Sad > 6)
		{
			Jump();
			if (TicksPassed(_random(10, 400)))
				Jump(false);
		}
		if (m_Sad > 7)
		{
			Hook();
			if (TicksPassed(_random(10, 400)))
				Hook(false);
		}
		if (m_Sad > 8 && (IsStuck || m_Sad > 9))
			Aim(_random(-100, 100), _random(-100, 100));
		if (m_Sad > 20)
		{
			Die();
			return true;
		}
	}
	if (m_Confused > 3)
		AvoidDeath();
	return false;
}

void CDummyBlmapChillPolice::OldPoliceMoves()
{
	if (X < 397 && Y > 436 && X > 388) // on the money tile jump loop, to prevent blocking flappy there
	{
		Jump(0);
		if (TicksPassed(20))
			Jump();
	}
	// detect lower panic (accedentally fall into the lower police base 
	if (!m_LowerPanic && Y > 437 && GetPos().y > m_LovedY)
	{
		m_LowerPanic = 1;
		GameServer()->SendEmoticon(m_pPlayer->GetCID(), EMOTICON_SPLATTEE);
	}

	if (m_LowerPanic)
	{
		// check for end panic
		if (Y < 434)
		{
			if (IsGrounded())
				m_LowerPanic = 0; // made it up yay
		}

		if (m_LowerPanic == 1) // position to jump on stairs
		{
			if (X < 400)
				Jump();
			else if (X > 401)
				Left();
			else
				m_LowerPanic = 2;
		}
		else if (m_LowerPanic == 2) // jump on the left starblock element
		{
			if (IsGrounded())
			{
				Jump();
				if (TicksPassed(20))
					Jump(0);
			}

			// navigate to platform
			if (GetPos().y < RAW(435) - 10)
			{
				Left();
				if (Y < 433)
				{
					if (GetVel().y > 0.01f && m_IsDJUsed == false)
					{
						Jump(); // double jump
						if (!IsGrounded()) // this dummyuseddj is for only using default 2 jumps even if 5 jump is on
							m_IsDJUsed = true;
					}
				}
				if (m_IsDJUsed == true && IsGrounded())
					m_IsDJUsed = false;
			}

			else if (Y < 438) // only if high enough focus on the first lower platform
			{
				if (X < 403)
					Right();
				else if (GetPos().x > (404 * 32) + 20)
					Left();
			}

			// check for fail position
			if ((GetPos().y > (441 * 32) + 10 && (X > 402 || GetPos().x < (399 * 32) + 10)) || IsFrozen())
				m_LowerPanic = 1; // lower panic mode to reposition
		}
	}
	else // no dummy lower panic
	{
		m_HelpMode = 0;
		// check if officer needs help
		CCharacter *pChr = GameWorld()->ClosestCharTypePoliceFreezeHole(GetPos(), true, m_pCharacter);
		if (pChr && pChr->IsAlive())
		{
			if (Y > 435) // setting the destination of dummy to top left police entry bcs otherwise bot fails when trying to help --> walks into jail wall xd
			{
				m_LovedX = (392 + rand() % 2) * 32;
				m_LovedY = 430 * 32;
			}
			AimPos(pChr->GetPos());

			m_IsClosestPolice = false;

			if (pChr->GetPlayer()->m_PoliceHelper || pChr->GetPlayer()->m_PoliceRank)
				m_IsClosestPolice = true;

			if (pChr->Core()->m_Pos.x > (444 * 32) - 10) // police dude failed too far --> to be reached by hook (set too help mode extream to leave save area)
			{
				m_HelpMode = 2;
				if (Jumped() > 1 && X > 431) // double jumped and above the freeze
					Left();
				else
					Right();
				// doublejump before falling in freeze
				if ((X > 432 && Y > 432) || X > 437) // over the freeze and too low
				{
					Jump();
					m_IsHelpHook = true;
				}
				if (IsGrounded() && X > 430 && TicksPassed(60))
					Jump();
			}
			else
				m_HelpMode = 1;

			if (m_HelpMode == 1 && GetPos().x > (431 * 32) + 10)
				Left();
			else if (m_HelpMode == 1 && X < 430)
				Right();
			else
			{
				if (!m_IsHelpHook && m_IsClosestPolice)
				{
					if (m_HelpMode == 2) // police dude failed too far --> to be reached by hook
					{
						//if (X > 435) //moved too double jump
						//{
						//	m_IsHelpHook = true;
						//}
					}
					else if (pChr->Core()->m_Pos.x > (439 * 32)) // police dude in the middle
					{
						if (IsGrounded())
						{
							m_IsHelpHook = true;
							Jump();
							Hook();
						}
					}
					else // police dude failed too near to hook from ground
					{
						if (GetVel().y < -4.20f && Y < 431)
						{
							m_IsHelpHook = true;
							Jump();
							Hook();
						}
					}
				}
				if (TicksPassed(8))
					Right();
			}

			if (m_IsHelpHook)
			{
				Hook();
				if (TicksPassed(200))
				{
					m_IsHelpHook = false; // timeout hook maybe failed
					Hook(0);
					Right();
				}
			}

			// dont wait on ground
			if (IsGrounded() && TicksPassed(900))
				Jump();
			// backup reset jump
			if (TicksPassed(1337))
				Jump(0);
		}

		if (!m_HelpMode)
		{
			//==============
			//NOTHING TO DO
			//==============
			// basic walk to destination
			if (GetPos().x < m_LovedX - 32)
				Right();
			else if (GetPos().x > m_LovedX + 32 && X > 384)
				Left();

			// change changing speed
			if (TicksPassed(m_Speed))
			{
				if (rand() % 2 == 0)
					m_Speed = rand() % 10000 + 420;
			}

			// choose beloved destination
			if (TicksPassed(m_Speed))
			{
				if ((rand() % 2) == 0)
				{
					if ((rand() % 3) == 0)
					{
						m_LovedX = RAW(420) + rand() % 69;
						m_LovedY = RAW(430);
						GameServer()->SendEmoticon(m_pPlayer->GetCID(), EMOTICON_GHOST);
					}
					else
					{
						m_LovedX = RAW(392 + rand() % 2);
						m_LovedY = RAW(430);
					}
					if ((rand() % 2) == 0)
					{
						m_LovedX = RAW(384) + rand() % 128;
						m_LovedY = RAW(430);
						GameServer()->SendEmoticon(m_pPlayer->GetCID(), EMOTICON_MUSIC);
					}
					else
					{
						if (rand() % 3 == 0)
						{
							m_LovedX = RAW(420) + rand() % 128;
							m_LovedY = RAW(430);
							GameServer()->SendEmoticon(m_pPlayer->GetCID(), EMOTICON_SUSHI);
						}
						else if (rand() % 4 == 0)
						{
							m_LovedX = RAW(429) + rand() % 64;
							m_LovedY = RAW(430);
							GameServer()->SendEmoticon(m_pPlayer->GetCID(), EMOTICON_SUSHI);
						}
					}
					if (rand() % 5 == 0) //lower middel base
					{
						m_LovedX = RAW(410) + rand() % 64;
						m_LovedY = RAW(443);
					}
				}
				else
					GameServer()->SendEmoticon(m_pPlayer->GetCID(), EMOTICON_EXCLAMATION);
			}
		}
	}

	//dont walk into the lower police base entry freeze
	if (X > 425 && X < 429) //right side
	{
		if (GetVel().x < -0.02f && IsGrounded())
			Jump();
	}
	else if (X > 389 && X < 391) //left side
	{
		if (GetVel().x > 0.02f && IsGrounded())
			Jump();
	}

	//jump over the police underground from entry to enty
	if (GetPos().y > m_LovedY) //only if beloved place is an upper one
	{
		if (X > 415 && X < 418) //right side
		{
			if (GetVel().x < -0.02f && IsGrounded())
			{
				Jump();
				if (TicksPassed(5))
					Jump(0);
			}
		}
		else if (X > 398 && X < 401) //left side
		{
			if (GetVel().x > 0.02f && IsGrounded())
			{
				Jump();
				if (TicksPassed(5))
					Jump(0);
			}
		}

		//do the doublejump
		if (GetVel().y > 6.9f && Y > 430 && X < 433  && m_IsDJUsed == false) //falling and not too high to hit roof with head
		{
			Jump();
			if (!IsGrounded()) // this dummyuseddj is for only using default 2 jumps even if 5 jump is on
				m_IsDJUsed = true;
		}
		if (m_IsDJUsed == true && IsGrounded())
			m_IsDJUsed = false;
	}
	// left side of police the freeze pit
	if (Y > 380 && X < 381 && X > 363)
	{
		Right();
		if (X > 367 && X < 368 && IsGrounded())
			Jump();
		if (Y > 433.7f)
			Jump();
	}
}

void CDummyBlmapChillPolice::OnTick()
{
	if (X > 451 && X < 472 && Y > 74 && Y < 85) // new spawn area, walk into the left SPAWN teleporter
	{
		Left();
		// jump into tele on spawn or jump onto edge after getting 5 jumps
		if (X > 454 && X < 462) // left side of new spawn area
		{
			Jump();
			if (TicksPassed(10))
				Jump(0);
		}
	}
	else if (X < 240 && Y < 36) // the complete zone in the map intselfs. its for resetting the dummy when he is back in spawn using tp
	{
		if (IsFrozen() && X > 32)
		{
			if (TicksPassed(60))
				GameServer()->SendEmoticon(m_pPlayer->GetCID(), EMOTICON_DROP); // tear emote before killing
			if (TicksPassed(500) && IsGrounded()) //kill when freeze
			{
				Die();
				return;
			}
		}
		if (X < 24 && Y < 14 && X > 23) // looking for tp and setting different aims for the swing
			m_SpawnTeleporter = 1;
		if (X < 25 && Y < 14 && X > 24) // looking for tp and setting different aims for the swing
			m_SpawnTeleporter = 2;
		if (X < 26 && Y < 14 && X > 25) // looking for tp and setting different aims for the swing
			m_SpawnTeleporter = 3;
		if (Y > 25 && X > 43 && Y < 35) // kill
		{
			Die();
			return;
		}
		if (Y > 35 && X < 43) // area bottom right from spawn, if he fall, he will kill
		{
			Die();
			return;
		}
		if (X < 16) // area left of old spawn, he will kill too
		{
			Die();
			return;
		}
		if (Y > 25) // after unfreeze hold hook to the right and walk right.
		{
			Aim(100, 1);
			Right();
			Hook();
			if (X > 25 && X < 33 && Y > 30) //set weapon and aim down
			{
				if (TicksPassed(5))
				{
					SetWeapon(WEAPON_GRENADE);
				}
				if (m_SpawnTeleporter == 1)
				{
					Aim(190, 100);
				}
				else if (m_SpawnTeleporter == 2)
				{
					Aim(205, 100);
				}
				else if (m_SpawnTeleporter == 3)
				{
					Aim(190, 100);
				}
				if (X > 31)
				{
					Jump();
					Fire();
				}
			}
			// fix getting stuck in the unfreeze and hooking the wall
			if (X < 33 && X > 31 && Y < 29)
				Hook(0);
		}
		else if (X > 33 && X < 50 && Y > 18)
		{
			Right();
			if (X > 33 && X < 42 && Y > 20 && Y < 25)
			{
				GameServer()->SendEmoticon(m_pPlayer->GetCID(), EMOTICON_EYES); //happy emote when successfully did the grenaede jump
			}
			if (X < 50)
			{
				Jump();
				if (TicksPassed(20))
					Jump(0);
				if (Y < 19 && m_pCharacter->m_FreezeTime == 0)
					Fire();
				Aim(-200, 200);
			}
			// don't walk into freeze
			if (X > 47 && X < 50 && Y > 17)
			{
				Left();
			}
		}
		else if (Y < 16 && X < 75 && X > 40) // walking right on it
		{
			Right();
			Jump(0);
			if (Y > 10 && X > 55 && X < 56) //jumping over gap
				Jump();
		}
		if (Y > 15 && X > 55 && X < 65)
		{
			Right();
			if (X > 63)
				Jump();
		}
		// fallen into lower right area of the top block area before gores
		if (Y > 20 && Y < 25)
		{
			Aim(X > 67 ? -20 : 20, 100);
			if (X > 69)
				Left();
			if (IsGrounded())
				Jump();
			if (GetPos().y < RAW(23) + 20 && GetVel().y < -1.1f)
				Fire();
		}
		else if (X > 75 && X < 135) //gores stuff (the thign with freeze spikes from top and bottom)
		{
			Jump(0);
			Right();
			// start jump into gores
			if (X > 77 && Y > 13 && X < 80)
				Jump();
			// nade boost on roof
			if ((X > 80 && X < 90) || (X > 104 && X < 117))
			{
				if (Y < 10)
					Fire();
				Aim(GetVel().x > 12.0f ? -10 : -100, -180);
			}
			if (X > 92 && Y > 12.5f) // hooking stuff from now on
			{
				Aim(100, -100);
				Hook();
				if (Y < 14 && X > 100 && X < 110)
					Hook(0);
			}
			// Don't swing into roof when boost worked
			if (X > 127 && X < 138 && GetVel().x > 12.f && GetVel().y < -1.5f)
				Hook(0);
			if (X > 120 && Y < 13)
				Hook(0);
		}
		else if (X > 135)
		{
			Right();
			// gores (the long way to 5 jumps)
			if (X < 220)
			{
				if ((GetPos().y > RAW(12) + 10 && GetVel().y > 4.1f) || (GetPos().y > RAW(12) + 30 && GetVel().y > -1.1f))
				{
					if (HookState() == HOOK_FLYING || HookState() == HOOK_GRABBED)
					{
						Aim(-100, 100);
						Fire();
					}
					else
					{
						Aim(100, -200);
					}
					Hook();
					if (Y < 12)
					{
						Hook(0);
					}
					if (X > 212)
					{
						Hook();
						Aim(100, -75);
						Jump();
					}
				}
			}
			// 5 jumps area
			if (X > 222)
			{
				Jump(0);
				if (Jumps() < 5)
				{
					if (X > 227)
						Left();
					else
						Right();
				}
				else // got 5 jumps go into tele
				{
					Aim(-200, 150);
					if (X < 229)
					{
						if (m_pCharacter->m_FreezeTime == 0)
							Fire();
					}
					else // on right side of platform (do rj here)
					{
						if (IsGrounded() && !m_pCharacter->GetReloadTimer())
						{
							if (X > 231)
							{
								m_DidRocketjump = true;
								Fire();
							}
						}
						else
						{
							if (GetPos().x > RAW(229) + 10 && !m_DidRocketjump)
								Left();
						}
					}
				}
			}
		}
	}
	if (IsFrozen() && Y < 410) // kills when in freeze and not in policebase
	{
		if (TicksPassed(60))
			GameServer()->SendEmoticon(m_pPlayer->GetCID(), EMOTICON_DROP); // tear emote before killing
		if (TicksPassed(500) && IsGrounded()) // kill when freeze
		{
			Die();
			return;
		}
	}
	if (IsFrozen() && X < 41 && X > 33 && Y < 10) // kills when on speedup right next to the newtee spawn to prevent infinite flappy blocking
	{
		if (TicksPassed(500)) // kill when freeze
		{
			Die();
			return;
		}
	}
	// new spawn going left and hopping over the gap under the CFRM.
	// (the jump over the freeze gap before falling down is not here, its in line 38 (search for comment 'jump onto edge after getting 5 jumps'))
	if (m_GrenadeJump == 4 || (X > 368 && Y < 340))
	{
		// change to gun
		if (TicksPassed(3) && X > 497)
			SetWeapon(WEAPON_GUN);
		// if bot gets under the table he will go right and jump out of the gap under the table
		LeftAntiStuck();
		// jump out of the chair room
		if (X < 497 && X > 496)
			Jump();
		// fallen too low backup jump
		if (X > 469 && Y > 74)
		{
			Jump();
			if (TicksPassed(15))
				Jump(0);
		}

		// above new spawn about to jump through freeze
		if (Y < 75)
		{
			// Slow down to hit ground before jumping through freeze
			if (X > 465 && X < 469)
			{
				if (!IsGrounded())
					Right();
			}
			// Too slow to jump through freeze -> go back get speed
			if (X > 455)
			{
				if (X < 461 && GetVel().x > -9.f && !m_GetSpeed)
				{
					m_GetSpeed = true;
					m_FailedAttempts++;
				}
				if (X > 465 || GetVel().x < -10.f)
					m_GetSpeed = false;
				if (m_GetSpeed)
				{
					Right();
					Aim(200, 100);
					Hook();
					if (TicksPassed(15))
						Hook(0);
				}
			}
			// count not succeding for a long time as fail
			if (TicksPassed(200))
				m_FailedAttempts++;
		}
		// somebody is blocking flappy intentionally
		if (m_FailedAttempts > 4 && X > 452 && X < 505)
		{
			// don't aim for edge and rather go full speed to bypass the blocker
			Left();
			Hook(0);
			Aim(100, 100);
			if (TicksPassed(15))
				SetWeapon(WEAPON_GRENADE);
			if (GetVel().y < -0.2f)
				Fire();
			if (IsGrounded())
				Jump();
			if (X < 456)
				Jump();
			if (TicksPassed(15))
				Jump(0);
		}

		// rocket jump from new spawn edge to old map entry
		if (X < 453 && Y < 80)
		{
			StopMoving();
			if (TicksPassed(10)) // change to grenade
				SetWeapon(WEAPON_GRENADE);

			if (!m_pCharacter->m_FreezeTime && IsGrounded() && m_GrenadeJump == 0) // shoot up
			{
				Jump();
				Aim(1, -100);
				Fire();
				m_GrenadeJump = 1;
			}
			else if (GetVel().y > -7.6f && m_GrenadeJump == 1) // jump in air // basically a timer for when the grenade comes down
			{
				Jump();
				m_GrenadeJump = 2;
			}
			if (m_GrenadeJump == 2 || m_GrenadeJump == 3) // double grenade
			{
				if (IsGrounded())
					Left();
				if (GetVel().y < 0.09f && GetVel().x < -0.1f)
				{
					Jump();
					Aim(100, 150);
					Fire();
					m_GrenadeJump = 4;
				}
			}
			if (m_GrenadeJump == 4)
			{
				Left();
				if (GetVel().y > 4.4f)
					Jump();
				if (Y > 85)
					m_GrenadeJump = 0; // something went wrong abort and try fallback grenade jump
			}
		}
		else // Reset rj vars for fallback grenade jump and other reuse
		{
			m_GrenadeJump = 0;
		}
	}
	if (X > 370 && Y < 340 && Y > 310) // bottom going left to the grenade jump
	{
		Left();
		// bottom jump over the hole to police station
		if (X < 422 && X > 421)
			Jump();
		// using 5jump from now on
		if (X < 406 && X > 405)
			Jump();
		if (X < 397 && X > 396)
			Jump();
		if (X < 387 && X > 386)
			Jump();
		// last jump from the 5 jump
		if (X < 377 && X > 376)
			Jump();
		// recover from uncontrolled long fall
		if (X > 435 && Y > 327 && GetVel().y > 20.0f)
			Jump();
		if (Y > 339) // if he falls into the hole to police station he will kill
		{
			Die();
			return;
		}
	}
	else if (Y > 296 && X < 370 && X > 350 && Y < 418) // getting up to the grenade jump part
	{
		if (IsGrounded())
		{
			m_HasReachedCinemaEntrance = true;
			Jump();
		}
		else if (Y < 313 && Y > 312 && X < 367)
			Right();
		else if (X > 367)
			Left();
		if (!m_HasReachedCinemaEntrance && X < 370)
			StopMoving();
		if (GetVel().y > 0.0000001f && Y < 310)
			Jump();
	}
	else if (GetVel().y > 0.001f && Y < 293 && X > 366.4f && X < 370)
	{
		Left();
		if (TicksPassed(1))
			SetWeapon(WEAPON_GRENADE);
	}
	else if (X > 325 && X < 366 && Y < 295 && Y > 59) // insane grenade jump
	{
		if (IsGrounded() && m_GrenadeJump == 0) // shoot up
		{
			Jump();
			Aim(0, -100);
			Fire();
			m_GrenadeJump = 1;
		}
		else if (GetVel().y > -7.6f && m_GrenadeJump == 1) // jump in air // basically a timer for when the grenade comes down
		{
			Jump();
			m_GrenadeJump = 2;
		}
		if (m_GrenadeJump == 2 || m_GrenadeJump == 3) // double grenade
		{
			if (Y > 58)
			{
				if (IsGrounded())
					m_HasTouchedGround = true;
				if (m_HasTouchedGround == true)
					Left();
				if (GetVel().y > 0.1f && IsGrounded())
				{
					Jump();
					Aim(100, 150);
					Fire();
					m_GrenadeJump = 3;
				}
				if (m_GrenadeJump == 3)
				{
					if (X < 344 && X > 343 && Y > 250) // air grenade for double wall grnade
					{
						Aim(-100, -100);
						Fire();
					}
				}
			}
		}
		if (X < 330 && GetVel().x == 0.0f && Y > 59) // if on wall jump and shoot
		{
			if (Y > 250 && GetVel().y > 6.0f)
			{
				Jump();
				m_HasStartGrenade = true;
			}
			if (m_HasStartGrenade == true)
			{
				Aim(-100, 170);
				Fire();
			}
			if (Y < 130 && Y > 131)
				Jump();
			if (GetVel().y > 0.001f && Y < 150)
				m_HasStartGrenade = false;
			if (GetVel().y > 2.0f && Y < 150)
			{
				Jump();
				m_HasStartGrenade = true;
			}
		}
	}
	if (Y < 60 && X < 337 && X > 325 && Y > 53) // top of the grenade jump // shoot left to get to the right 
	{
		Right();
		Aim(-100, 0);
		Fire();
		Jump(0);
		if (X > 333 && Y < 335) // hook up and get into the tunnel thingy
		{
			Jump();
			if (Y < 55)
				Right();
		}
	}
	if (X > 337 && X < 400 && Y < 60 && Y > 40) // hook thru the hookthru
	{
		Aim(0, -1);
		Hook();
	}
	if (X > 339.5f && X < 345 && Y < 51)
		Left();
	if (X < 339 && X > 315 && Y > 40 && Y < 53) // top of grenade jump the thing to go through the wall
	{
		Hook(0);
		Aim(100, 50);
		if (m_HasAlreadyBeenHere == false)
		{
			if (X < 339)
			{
				Right();
				if (X > 338 && X < 339 && Y > 51)
					m_HasAlreadyBeenHere = true;
			}
		}
		if (m_HasAlreadyBeenHere == true) //using grenade to get throug the freeze in this tunnel thingy
		{
			Left();
			if (X < 338)
				Fire();
		}
		if (X < 328 && Y < 60)
			Jump();
	}
	// Stuck on the outside of the clu spike thing
	else if (Y > 120 && Y < 185 && X > 233 && X < 300)
	{
		if (X < 272)
			Right();
		/*
		##    <- deep and spikes and clu skip
		#
	######
	######
	##      <-- stuck here
	######
	######
		#
		#  <-- or stuck here
		##
		 #    police entrance
	######      |
	#           v
		*/
	}
	// jumping over the big freeze to get into the tunnel
	else if (Y > 260 && X < 325 && Y < 328 && X > 275)
	{
		Left();
		Jump(0);
		if (Y > 280 && Y < 285)
			Jump();
		if (TicksPassed(5))
			SetWeapon(WEAPON_GUN);
	}
	// after grenade jump and being down going into the tunnel to police staion
	else if (Y > 328 && Y < 346 && X > 236 && X < 365)
	{
		Right();
		if (X > 265 && X < 267)
			Jump();
		if (X > 282 && X < 284)
			Jump();
		// walk left in air to get on the little block
		if (Y > 337.4f && X > 295)
			Left();
		// fix someone blocking flappy, he would just keep moving left into the wall and do nothing there
		if (X > 294 && X < 297 && Y > 343 && Y < 345 && (IsGrounded() || GetVel().y < -1.1f))
			m_GetSpeed = true;
		if (m_GetSpeed)
		{
			if (X < 290)
				m_GetSpeed = false;
			Left();
			if (Y > 337)
			{
				// rj the wall up
				Aim(-200, 280);
				if (TicksPassed(10))
					SetWeapon(WEAPON_GRENADE);
				if (GetVel().y > -4.1f && IsGrounded())
					Jump(rand() % 3);
				else if (GetVel().y > -1.5f)
					Fire();
			}
			else
			{
				// avoid roof freeze
				Aim(1, -200);
				if (Y < 331)
					Fire();
			}
		}
		// reset get speed when falling through the freeze for next part
		if (IsFrozen())
			m_GetSpeed = false;
	}
	else if (Y < 361 && Y > 346)
	{
		if (TicksPassed(10))
			SetWeapon(WEAPON_GRENADE);
		Right();
		// slow down and go back to enter the 2 tiles wide hole
		if (X > 321)
			Left();
		if (X > 317 && GetVel().x > 5.5f)
			StopMoving();
		if (X > 316 && GetVel().x > 9.8f)
			Left();
		// Get enough speed before the rj
		if (X < 297 && X > 296)
			if (GetVel().x < 9.9f)
				m_GetSpeed = true;
		if (m_GetSpeed)
		{
			Left();
			if ((X < 294 && IsGrounded()) || X < 280)
				m_GetSpeed = false;
		}
		Aim(-50, 100);
		if (X < 303)
		{
			if (X > 296)
				Fire();
		}
		else
		{
			AimX(50);
			if (X > 310 && X < 312)
				Fire();
		}
		Jump(0);
		if (GetVel().y > 0.0000001f && Y > 352.6f && X < 315) // jump in air to get to the right
			Jump();
	}
	else if (X > 180 && X < 450 && Y < 450 && Y > 358) // wider police area with left entrance
	{
		// kills when in freeze in policebase or left of it (takes longer that he kills bcs the way is so long he wait a bit longer for help)
		if (IsFrozen())
		{
			if (TicksPassed(60))
				GameServer()->SendEmoticon(m_pPlayer->GetCID(), EMOTICON_DROP); // tear emote before killing
			if (TicksPassed(3000) && (IsGrounded() || X > 430)) // kill when freeze
			{
				Die();
				return;
			}
		}
		m_Sad = 0; // so nice in police area
		m_Confused = 0;
		if (Y < 408)
			if (TicksPassed(10))
				SetWeapon(WEAPON_GUN);
		// walking right again to get into the tunnel at the bottom
		if (X < 363)
			Right();
		// do not enter in pvp area or bank
		if (X > 324 && Y < 408)
		{
			Left();
			if (X > 330)
				LeftAntiStuck();
		}
		// police area entrance tunnel (left side)
		if (X > 316 && X < 370 && Y > 416)
			RightThroughFreeze();
		/* * * * * * * *
		 * police area *
		 * * * * * * * */
		if (X > 363 && X < 450 && Y < 450 && Y > 380)
		{
			NewPoliceMoves();
			//OldPoliceMoves();
		}
	}
	else
	{
		// not in police area check if stuck somewhere
		if (CheckStuck())
			return;
	}
}

void CDummyBlmapChillPolice::NewPoliceMoves()
{
	if (TicksPassed(30))
	{
		m_WantedWeapon = WEAPON_GUN;
		Aim(100, 10);
	}
	AvoidFreeze();
	AvoidFreezeWeapons();
	// freeze pit on left side of police
	if (X > 366 && X < 382 && !IsGrounded())
	{
		if (GetVel().x < -6.6f && X < 372)
			Left();
		else
			Right();
	}
	// freeze pit on right side of police
	if (X > 430)
		Left();
	if (!HelpOfficerRight())
		HelpOfficerLeft();
}

void CDummyBlmapChillPolice::WalkPoliceDir(int Direction)
{
	// fallen down
	if (Y > 437)
	{
		m_WantedWeapon = WEAPON_GRENADE;
		if (X < 399)
			Right();
		else if (X > 402 && X < 406)
			Right();
		else if (X > 412)
			Left();
		else
		{
			Aim(0, 200);
			Fire();
			if (IsGrounded())
				Jump(_random(3));
		}
	}
	// when high enough stay there and move on the upper area
	else
	{
		SetDirection(Direction);
		if (GetVel().y > 2.2f && Y > 431 && X > 400 && X < 417)
			Jump();
		// grenade boosts floor
		if (IsGrounded())
		{
			Aim(100 * -Direction, 20);
			if (TicksPassed(10))
				m_WantedWeapon = WEAPON_GRENADE;
			if (m_pCharacter->GetActiveWeapon() == WEAPON_GRENADE)
				Fire();
		}
		// grenade boosts roof
		if (GetVel().y < 0.01 && GameServer()->Collision()->GetTile(GetPos().x, GetPos().y - 32) == TILE_NOHOOK)
		{
			Aim(100 * -Direction, -25);
			if (TicksPassed(10))
				m_WantedWeapon = WEAPON_GRENADE;
			if (m_pCharacter->GetActiveWeapon() == WEAPON_GRENADE)
				Fire();
		}
	}
}

bool CDummyBlmapChillPolice::HelpOfficerLeft()
{
	// check if officer needs help
	CCharacter *pChr = GameWorld()->ClosestCharTypePoliceFreezePitLeft(GetPos(), m_pCharacter);
	if (!pChr || !pChr->IsAlive())
		return true;
	if (X > 383)
		WalkPoliceLeft();
	else
	{
		AimPos(pChr->GetPos());
		LeftThroughFreeze();
		if (JumpedTotal() > 2)
			Right();
		Hook(0);
		float DistToOfficer = distance(GetPos(), pChr->GetPos());
		if (HookState() == HOOK_FLYING || HookState() == HOOK_GRABBED || DistToOfficer < 10 * 32)
			Hook(1);
		if (HookState() == HOOK_GRABBED)
		{
			Right();
			int HookedID = m_pCharacter->Core()->m_HookedPlayer;
			CPlayer *pHooked = GameServer()->m_apPlayers[HookedID];
			CCharacter *pCharHooked = pHooked->GetCharacter();
			if (pCharHooked)
			{
				// hooked a non police tee -> try to get rid of it
				if (!IsPolice(pCharHooked))
				{
					// hook random to the left
					Left();
					if (JumpedTotal() > 3)
						Hook(0);
					if (pCharHooked->Core()->m_Vel.x < -8.8f)
						Hook(0);
				}
			}
		}
		if (JumpedTotal() > 3)
			Right();
		if (X < 366)
			Right();
		if (X < 369)
		{
			CCharacter *pClosestChr = GameWorld()->ClosestCharacterNoRange(GetPos(), m_pCharacter);
			// if on left side and closes char is not police
			// make sure that boi is blocked first otherwise he is just in the way
			if (!IsPolice(pClosestChr) && !pClosestChr->isFreezed)
			{
				AimPos(pClosestChr->GetPos());
				// push into freeze when both grounded
				if (IsGrounded() && pClosestChr->IsGrounded())
				{
					// avoid yeeting away when pushing
					Jump(0);
					if (pClosestChr->GetPos().x < GetPos().x)
						Left();
					else
						Right();
				}
				// if enemy other side of freeze just hook em in
				if (pClosestChr->GetPos().x < RAW(363))
				{
					Hook(1);
					Right();
					if (X > 367)
						Left();
				}
				// make sure hook does not get stuck
				if (HookState() != HOOK_FLYING && HookState() != HOOK_GRABBED && TicksPassed(60))
					Hook(0);
				// do not walk into left freeze
				if (X < 364)
					Right();
			}
			// release hook on enemy when he is frozen on the left side
			if (!IsPolice(pClosestChr) && pClosestChr->isFreezed)
				if (pClosestChr->GetPos().x < RAW(364))
					if (m_pCharacter->Core()->m_HookedPlayer == pClosestChr->GetPlayer()->GetCID())
						Hook(0);
		}
		if (TicksPassed(30))
			m_WantedWeapon = WEAPON_HAMMER;
		if (DistToOfficer < 80 && pChr->m_FreezeTime && m_pCharacter->GetActiveWeapon() == WEAPON_HAMMER)
			Fire();
	}
	return false;
}

bool CDummyBlmapChillPolice::HelpOfficerRight()
{
	// check if officer needs help
	CCharacter *pChr = GameWorld()->ClosestCharTypePoliceFreezeHole(GetPos(), true, m_pCharacter);
	if (!pChr || !pChr->IsAlive())
		return false;
	if (X < 422)
		WalkPoliceRight();
	else
	{
		AimPos(pChr->GetPos());
		RightThroughFreeze();
		if (JumpedTotal() > 2)
			Left();
		Hook(0);
		float DistToOfficer = distance(GetPos(), pChr->GetPos());
		if (HookState() == HOOK_FLYING || HookState() == HOOK_GRABBED || DistToOfficer < 10 * 32)
			Hook(1);
		if (HookState() == HOOK_GRABBED)
		{
			Left();
			int HookedID = m_pCharacter->Core()->m_HookedPlayer;
			CCharacter *pCharHooked = GameServer()->GetPlayerChar(HookedID);
			if (pCharHooked)
			{
				// hooked a non police tee -> try to get rid of it
				if (!IsPolice(pCharHooked))
				{
					// officer on right side -> hook random to the left
					if (pChr->GetPos().x > RAW(442))
					{
						if (!pCharHooked->isFreezed)
							Hook(0);
					}
					// officer on left side -> hook random to the right
					else
					{
						Right();
						if (JumpedTotal() > 3)
							Hook(0);
						if (pCharHooked->Core()->m_Vel.x > 8.8f)
							Hook(0);
					}
				}
			}
		}
		if (JumpedTotal() > 3)
			Left();
		if (X > 441)
			Left();
		if (TicksPassed(30))
			m_WantedWeapon = WEAPON_HAMMER;
		if (DistToOfficer < 80 && pChr->m_FreezeTime && m_pCharacter->GetActiveWeapon() == WEAPON_HAMMER)
			Fire();
	}
	return true;
}
