// police bot on ChillBlock5
// guarding the station killing intruders
// and helping officers

#include "chillblock5_police.h"

#include "../character.h"

#include <base/math_ddpp.h>

#include <engine/shared/config.h>

#include <game/server/ddpp/shop.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos) * 32)

CDummyChillBlock5Police::CDummyChillBlock5Police(class CPlayer *pPlayer) :
	CDummyBase(pPlayer, DUMMYMODE_CHILLBLOCK5_POLICE)
{
	OnDeath();
}

void CDummyChillBlock5Police::OnDeath()
{
	m_Dummy_GotStuck = false;
	m_Dummy_GetSpeed = false;
	m_Dummy_ClosestPolice = false;
	m_Dummy_SpawnAnimation = false;
	m_Dummy_SpawnAnimation_delay = 0;
	m_Dummy_AttackMode = 0;
}

void CDummyChillBlock5Police::OnTick()
{
	//rest dummy
	Hook(0);
	Jump(0);
	StopMoving();
	Fire(0);

	//Basic Stuff:
	//tele from spawn into police base
	//if (GetPos().x < 410 * 32 && GetPos().x > 380 * 32 && GetPos().y < 219 * 32 && GetPos().y > 200 * 32) //spawn area
	if(GetPos().x < 460 * 32) //spawn
	{
		m_pCharacter->SetPosition(vec2(484 * 32, 234 * 32));
		m_Dummy_SpawnAnimation = true;
	}
	//do spawnanimation in police base
	if(m_Dummy_SpawnAnimation)
	{
		m_Dummy_SpawnAnimation_delay++;
		if(m_Dummy_SpawnAnimation_delay > 2)
		{
			GameServer()->CreatePlayerSpawn(GetPos());
			m_Dummy_SpawnAnimation = false;
		}
	}

	//selfkill
	//dyn
	if(GetVel().y == 0.000000f && GetVel().x < 0.01f && GetVel().x > -0.01f && IsFrozen())
	{
		if(Server()->Tick() % 20 == 0)
		{
			GameServer()->SendEmoticon(m_pPlayer->GetCid(), 3, -1);
		}

		if(Server()->Tick() % 200 == 0)
		{
			Die();
		}
	}

	//TODO:
	/*
	Check for an officär acteevated the "help me in base" bool
	m_IsHelpPolice = -1;


	for (int i = 0; i++; i < MAX_CLIENTS)
	{
	if (m_HelpPolice)
	m_IsHelpPolice = i; //return id
	}

	if (m_isHelpPolice)
	{

	//geh den suchen und hilf dem usw

	}


	*/

	CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
	if(pChr && pChr->IsAlive())
	{
		//for (int i = 0; i < MAX_CLIENTS; i++)
		//{
		//	if (p)
		//}
		m_Dummy_ClosestPolice = false;
		//If a policerank escapes from jail he is treated like a non police
		if((pChr->GetPlayer()->m_Account.m_PoliceRank > 0 && pChr->GetPlayer()->m_Account.m_EscapeTime == 0) || (pChr->GetPlayer()->m_PoliceHelper && pChr->GetPlayer()->m_Account.m_EscapeTime == 0))
		{
			//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "hello officär");
			m_Dummy_ClosestPolice = true;
			//if (pChr->isFreezed)
			//{
			//	m_Dummy_dmm31 = 2;
			//}
		}

		/*
		########################################

		m_Dummy_dmm31 - - - DUMMY MODE MODE [31]

		########################################

		Structure:

		* [STRUCT][1]: Check what sub-mode should be used

		* [STRUCT][2]: Do stuff depending on sub-modes

		* [STRUCT][3]: Do basic movement depending on sub-modes ( step 2 for all modes higher than 3)


		modes:

		0				LOCAL: NOTHING IS GOING ON
		1				LOCAL: ENEMY ATTACK
		2				LOCAL: POLICE HELP
		3				EXTERNAL: ENEMY ATTACK (right side / jail side)
		4				EXTERNAL: POLICE HELP (right side / jail side)

		*/

		//##############################################
		//[STRUCT][1]: Check what sub-mode should be used
		//##############################################
		if(m_Dummy_ClosestPolice) //police
		{
			if(pChr->m_FreezeTime > 0 && GetPos().x < 477 * 32)
			{
				m_Dummy_dmm31 = 2; // LOCAL: POLICE HELP
			}
			else
			{
				m_Dummy_dmm31 = 0; // LOCAL: NOTHING IS GOING ON
			}
		}
		else //not police
		{
			if(pChr->m_FreezeTime == 0)
			{
				if(pChr->GetPos().x > 481 * 32)
				{
					//m_Dummy_dmm31 = 3; //EXTERNAL: ENEMY ATTACK(right side / jail side)
				}
				else
				{
					m_Dummy_dmm31 = 1; //LOCAL: ENEMY ATTACK
				}
			}
			if(pChr->isFreezed)
			{
				m_Dummy_dmm31 = 0; //maybe add here a mode where the bot moves the nonPolices away to find failed polices
			}
		}

		//##############################################
		//[STRUCT][2]: Do stuff depending on sub - modes
		//##############################################

		if(m_Dummy_dmm31 == 0) //nothing is going on
		{
			AimPos(pChr->GetPos());
			if(Server()->Tick() % 90 == 0)
			{
				SetWeapon(1);
			}
		}
		else if(m_Dummy_dmm31 == 1) //Attack enemys
		{
			AimPos(pChr->GetPos());

			if(Server()->Tick() % 30 == 0)
			{
				SetWeapon(0);
			}

			if(m_pCharacter->m_FreezeTime == 0 && pChr->m_FreezeTime == 0 && pChr->GetVel().y < -0.5 && pChr->GetPos().x > GetPos().x - 3 * 32 && pChr->GetPos().x < GetPos().x + 3 * 32)
			{
				Fire();
			}

			m_Dummy_AttackMode = 0;
			if(GetPos().x < 466 * 32 + 20 && pChr->GetPos().x > 469 * 32 + 20) //hook enemy in air (rightside)
			{
				m_Dummy_AttackMode = 1;
			}

			if(m_Dummy_AttackMode == 0) //default mode
			{
				if(GetPos().x < 466 * 32 - 5) //only get bored on lovely place
				{
					SetDirection(rand() % 2);
					if(IsGrounded())
					{
						Jump(rand() % 2);
					}
					if(pChr->GetPos().y > GetPos().y)
					{
						Hook();
					}
				}
			}
			else if(m_Dummy_AttackMode == 1) //hook enemy escaping (rightside)
			{
				if(pChr->GetVel().x > 1.3f)
				{
					Hook();
				}
			}

			//Dont Hook enemys back in safety
			if((pChr->GetPos().x < 460 * 32 && pChr->GetPos().x > 457 * 32) || (pChr->GetPos().x < 469 * 32 && pChr->GetPos().x > 466 * 32))
			{
				Hook(0);
			}
		}
		else if(m_Dummy_dmm31 == 2) //help police dudes
		{
			AimPos(pChr->GetPos());

			if(pChr->GetPos().y > GetPos().y)
			{
				Hook();
			}
			if(Server()->Tick() % 40 == 0)
			{
				Hook(0);
				Jump(0);
			}
			if(IsGrounded() && pChr->isFreezed)
			{
				Jump();
			}

			if(pChr->isFreezed)
			{
				if(pChr->GetPos().x > GetPos().x)
				{
					Right();
				}
				else if(pChr->GetPos().x < GetPos().x)
				{
					Left();
				}
			}
			else
			{
				if(pChr->GetPos().x - 110 > GetPos().x)
				{
					Right();
				}
				else if(pChr->GetPos().x + 110 < GetPos().x)
				{
					Left();
				}
				else
				{
					if(Server()->Tick() % 10 == 0)
					{
						SetWeapon(0);
					}
					if(m_pCharacter->m_FreezeTime == 0 && pChr->m_FreezeTime > 0)
					{
						Fire();
					}
				}
			}

			//invert direction if hooked the player to add speed :)
			if(HookState() == HOOK_GRABBED)
			{
				if(pChr->GetPos().x > GetPos().x)
				{
					Left();
				}
				else if(pChr->GetPos().x < GetPos().x)
				{
					Right();
				}
			}

			//schleuderprotection   stop hook if mate is safe to prevemt blocking him to the other side
			if(pChr->GetPos().x > 460 * 32 + 10 && pChr->GetPos().x < 466 * 32)
			{
				Hook(0);
			}
		}
		else if(m_Dummy_dmm31 == 3) //EXTERNAL: Enemy attack (right side /jail side)
		{
			if(GetPos().x < 461 * 32)
			{
				Right();
			}
			else
			{
				if(GetPos().x < 484 * 32)
				{
					Right();
				}
				if(GetPos().x > 477 * 32 && !IsGrounded())
				{
					Hook();
				}
			}

			//jump all the time xD
			if(IsGrounded() && GetPos().x > 480 * 32)
			{
				Jump();
			}

			//Important jump protection
			if(GetPos().x > 466 * 32 && GetPos().y > 240 * 32 + 8 && GetPos().x < 483 * 32)
			{
				Jump();
			}
		}
		else //unknown dummymode
		{
			AimPos(pChr->GetPos());
			if(Server()->Tick() % 120 == 0)
			{
				//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "error: unknown sub-mode for this dummymode set.");
			}
		}
	}

	//##############################################
	//[STRUCT][3]: Do basic movement depending on sub - modes
	//##############################################

	//The basic movements depending on the dummysubmodes
	//but some submodes use the same thats why its listed external here
	//Movement
	//JailSpawn
	if(m_Dummy_dmm31 < 3)
	{
		if(GetPos().x > 482 * 32 + 20 && GetPos().y < 236 * 32)
		{
			if(GetVel().x > -8.2f && GetPos().x < 484 * 32 - 20)
			{
				m_Dummy_GetSpeed = true;
			}
			if(GetPos().x > 483 * 32 && !IsGrounded())
			{
				m_Dummy_GetSpeed = true;
			}
			if(GetVel().y > 5.3f)
			{
				m_Dummy_GetSpeed = true;
			}

			if(IsGrounded() && GetPos().x > 485 * 32)
			{
				m_Dummy_GetSpeed = false;
			}

			if(m_Dummy_GotStuck)
			{
				Left();
				if(Server()->Tick() % 33 == 0)
				{
					Jump();
				}
				if(Server()->Tick() % 20 == 0)
				{
					SetWeapon(0); //hammer
				}

				if(GetTargetX() < -20)
				{
					if(m_pCharacter->m_FreezeTime == 0)
					{
						Fire();
					}
				}
				else if(GetTargetX() > 20)
				{
					Hook();
					if(Server()->Tick() % 25 == 0)
					{
						Hook(0);
					}
				}

				//gets false in the big else m_Dummy_GotStuck = false;
			}
			else
			{
				if(m_Dummy_GetSpeed)
				{
					Right();
					if(Server()->Tick() % 90 == 0)
					{
						m_Dummy_GotStuck = true;
					}
				}
				else
				{
					Left();
					if(GetVel().x > -4.4f)
					{
						if(Server()->Tick() % 90 == 0)
						{
							m_Dummy_GotStuck = true;
						}
					}
				}
			}
		}
		else //not Jail spawn
		{
			m_Dummy_GotStuck = false;
			//TODO; add a dir 1 if he gets attacked
			if(GetPos().x > 464 * 32)
			{
				Left();
			}
			else if(GetPos().x < 461 * 32)
			{
				Right();
			}

			if(GetPos().x > 466 * 32 && GetPos().y > 240 * 32 + 8)
			{
				Jump();
			}
		}
	}
	else
	{
		//no basic moves for this submode
	}
}
