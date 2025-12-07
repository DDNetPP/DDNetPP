// mode 23
// Race mode ChillBlock5 with humans

#include "chillblock5_race.h"

#include "../character.h"

#include <base/math_ddpp.h>

#include <engine/shared/config.h>

#include <game/server/gamecontext.h>
#include <game/server/player.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos) * 32)

CDummyChillBlock5Race::CDummyChillBlock5Race(class CPlayer *pPlayer) :
	CDummyBase(pPlayer, DUMMYMODE_CHILLBLOCK5_RACE)
{
	OnDeath();
}

void CDummyChillBlock5Race::OnDeath()
{
	m_Dummy_help_m8_before_hf_hook = 0;
	m_Dummy_help_emergency = false;
	m_Dummy_help_no_emergency = false;
	m_Dummy_hook_mate_after_hammer = false;
	m_Dummy_help_before_fly = false;
	m_Dummy_2p_panic_while_helping = false;
	m_Dummy_panic_balance = false;
	m_Dummy_mate_failed = false;
	m_Dummy_hh_hook = false;
	m_Dummy_collected_weapons = false;
	m_Dummy_mate_collected_weapons = false;
	m_Dummy_rjumped2 = false;
	m_Dummy_dd_helphook = false;
	m_Dummy_2p_hook = false;
	m_Dummy_2p_state = 0;
	m_Dummy_mode23 = 0;
	m_Dummy_nothing_happens_counter = 0;
	m_Dummy_panic_weapon = 0;
	m_Dummy_sent_chat_msg = 0;
	m_Dummy_mate_help_mode = 0;
	m_Dummy_movement_mode23 = 0;
	m_DummyFreezed = false;
	m_EmoteTickNext = 0;
}

void CDummyChillBlock5Race::OnTick()
{
	//rest dummy (zuruecksetzten)
	Hook(0);
	Jump(0);
	StopMoving();
	Fire(0);
	// m_pPlayer->m_TeeInfos.m_ColorBody = (0 * 255 / 360); //remove this if u ever want to debug again xd

	/*
	Dummy23modes:
	0				Classic Main race mode.
	1				Tricky mode with tricky hammerfly and sensless harder hammerhit xD. (used for "Drag*" to fuck him in the race lol)
	2				ChillerDragon's mode just speedhammerfly.

	*/

	if(Server()->Tick() % 50 == 0)
	{
		m_Dummy_mode23 = 0;
		CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
		if(pChr && pChr->IsAlive())
		{
			if(
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "Starkiller") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "rqza") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "timakro") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "Nudelsaft c:") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "Destom") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "Ante") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "Ama") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "Forris") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "Aoe") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "Spyker") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "Waschlappen") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), ".:Mivv") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "nealson T'nP") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "ChillerDragon") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "ChillerDragon.*") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "Gwendal") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "Blue") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "Amol") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "ch1th шoymeн?") ||
				//!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "fokkonaut") ||
				!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "pro"))
			{
				m_Dummy_mode23 = 2;
			}
			if(!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "Drag*"))
			{
				m_Dummy_mode23 = 1;
			}
		}
	}

	if(GetPos().x > 241 * 32 && GetPos().x < 418 * 32 && GetPos().y > 121 * 32 && GetPos().y < 192 * 32) //new spawn ChillBlock5 (tourna edition (the on with the gores stuff))
	{
		//dieser code wird also nur ausgeführt wenn der bot gerade im neuen bereich ist
		if(GetPos().x > 319 * 32 && GetPos().y < 161 * 32) //top right spawn
		{
			//look up left
			if(GetPos().x < 372 * 32 && GetVel().y > 3.1f)
			{
				AimX(-30);
				AimY(-80);
			}
			else
			{
				AimX(-100);
				AimY(-80);
			}

			if(GetPos().x > 331 * 32 && IsFrozen())
			{
				Die();
			}

			if(GetPos().x < 327 * 32) //dont klatsch in ze wand
			{
				Right(); //nach rechts laufen
			}
			else
			{
				Left();
			}

			if(IsGrounded() && GetPos().x < 408 * 32) //initial jump from spawnplatform
			{
				Jump();
			}

			if(GetPos().x > 330 * 32) //only hook in tunnel and let fall at the end
			{
				if(GetPos().y > 147 * 32 || (GetPos().y > 143 * 32 && GetVel().y > 3.3f)) //gores pro hook up
				{
					Hook();
				}
				else if(GetPos().y < 143 * 32 && GetPos().x < 372 * 32) //hook down (if too high and in tunnel)
				{
					AimX(-42);
					AimY(100);
					Hook();
				}
			}
		}
		else if(GetPos().x < 317 * 32) //top left spawn
		{
			if(GetPos().y < 158 * 32) //spawn area find down
			{
				//selfkill
				if(IsFrozen())
				{
					Die();
				}

				if(GetPos().x < 276 * 32 + 20) //is die mitte von beiden linken spawns also da wo es runter geht
				{
					AimX(57);
					AimY(-100);
					Right();
				}
				else
				{
					AimX(-57);
					AimY(-100);
					Left();
				}

				if(GetPos().y > 147 * 32)
				{
					//dbg_msg("fok","will hooken");
					Hook();
					if(GetPos().x > 272 * 32 && GetPos().x < 279 * 32) //let fall in the hole
					{
						//dbg_msg("fok", "lass ma des");
						Hook(0);
						AimX(2);
						AimY(100);
					}
				}
			}
			else if(GetPos().y > 162 * 32) //managed it to go down --> go left
			{
				//selfkill
				if(IsFrozen())
				{
					Die();
				}

				if(GetPos().x < 283 * 32)
				{
					AimX(200);
					AimY(-136);
					if(GetPos().y > 164 * 32 + 10)
					{
						Hook();
					}
				}
				else
				{
					AimX(80);
					AimY(-100);
					if(GetPos().y > 171 * 32 - 10)
					{
						Hook();
					}
				}

				Right();
			}
			else //freeze unfreeze bridge only 2 tiles do nothing here
			{
			}
		}
		else //lower end area of new spawn --> entering old spawn by walling and walking right
		{
			Right();
			AimX(200);
			AimY(-84);

			//Selfkills
			if(IsFrozen() && IsGrounded()) //should never lie in freeze at the ground
			{
				Die();
			}

			if(GetPos().y < 166 * 32 - 20)
			{
				Hook();
			}

			if(GetPos().x > 332 * 32 && GetPos().x < 337 * 32 && GetPos().y > 182 * 32) //wont hit the wall --> jump
			{
				Jump();
			}

			if(GetPos().x > 336 * 32 + 20 && GetPos().y > 180 * 32) //stop moving if walled
			{
				StopMoving();
			}
		}
	}
	// else if(false && GetPos().y < 193 * 32 && GetPos().x < 450 * 32 /*&& g_Config.m_SvChillBlock5Version == 1*/) //new spawn
	// {
	// 	AimX(200);
	// 	AimY(-80);

	// 	//not falling in freeze is bad
	// 	if(GetVel().y < 0.01f && m_pCharacter->m_FreezeTime > 0)
	// 	{
	// 		if(Server()->Tick() % 40 == 0)
	// 		{
	// 			Die();
	// 		}
	// 	}
	// 	if(GetPos().y > 116 * 32 && GetPos().x > 394 * 32)
	// 	{
	// 		Die();
	// 	}

	// 	if(GetPos().x > 364 * 32 && GetPos().y < 126 * 32 && GetPos().y > 122 * 32 + 10)
	// 	{
	// 		if(GetVel().y > -1.0f)
	// 		{
	// 			Hook();
	// 		}
	// 	}

	// 	if(GetPos().y < 121 * 32 && GetPos().x > 369 * 32)
	// 	{
	// 		Left();
	// 	}
	// 	else
	// 	{
	// 		Right();
	// 	}
	// 	if(GetPos().y < 109 * 32 && GetPos().x > 377 * 32 && GetPos().x < 386 * 32)
	// 	{
	// 		Right();
	// 	}

	// 	if(GetPos().y > 128 * 32)
	// 	{
	// 		Jump();
	// 	}

	// 	//speeddown at end to avoid selfkill cuz to slow falling in freeze
	// 	if(GetPos().x > 384 * 32 && GetPos().y > 121 * 32)
	// 	{
	// 		AimX(200);
	// 		AimY(300);
	// 		Hook();
	// 	}
	// }
	else //under 193 (above 193 is new spawn)
	{
		//Selfkill

		//Checken ob der bot far im race ist
		if(m_Dummy_collected_weapons && GetPos().x > 470 * 32 && GetPos().y < 200 * 32)
		{
			//TODO:
			//schau wie weit der bot is wenn er weiter is als der ClosestCharTypeFarInRace bereich is schau das du rechtzeitig n anderen triggerst
			//wie zumbeispiel ClosestCharTypeFinish und das wird getriggert wenn der bot rechts des 2p parts is oder so

			CCharacter *pChr = GameServer()->m_World.ClosestCharTypeFarInRace(GetPos(), true, m_pCharacter);
			if(pChr && pChr->IsAlive())
			{
			}
			else
			{
				if(!IsFrozen() || GetVel().x < -0.5f || GetVel().x > 0.5f || GetVel().y != 0.000000f)
				{
					//mach nichts lol brauche nur das else is einfacher
				}
				else
				{
					if(Server()->Tick() % 370 == 0)
						Die();
				}
			}
		}
		else //sonst normal relative schnell killen
		{
			CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
			if(pChr && pChr->IsAlive())
			{
				if(!IsFrozen() || GetVel().x < -0.5f || GetVel().x > 0.5f || GetVel().y != 0.000000f)
				{
					//mach nichts lol brauche nur das else is einfacher
				}
				else
				{
					if(Server()->Tick() % 270 == 0)
						Die();
				}
			}
			else
			{
				if(IsFrozen() && GetVel().y == 0.000000f && GetVel().x < 0.1f && GetVel().x > -0.1f)
				{
					Die();
				}
			}
		}

		//instant self kills
		if(GetPos().x < 390 * 32 && GetPos().x > 325 * 32 && GetPos().y > 215 * 32) //Links am spawn runter
		{
			Die();
			//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "Links am spawn runter");
		}
		//else if ((GetPos().y < 204 * 32 && GetPos().x < 415 * 32 && GetPos().x > 392 * 32 && GetPos().y > 190) || (GetPos().y < 204 * 32 && GetPos().x < 415 * 32 && GetPos().x < 390 * 32 && GetPos().y > 190)) //freeze decke am spawn
		//{
		//	Die();
		//	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze decke am spawn");
		//}
		//else if (GetPos().y > 218 * 32 + 31 /* für tee balance*/ && GetPos().x < 415 * 32) //freeze boden am spawn
		//{
		//	Die();
		//	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze boden am spawn");
		//}
		else if(GetPos().y < 215 * 32 && GetPos().y > 213 * 32 && GetPos().x > 415 * 32 && GetPos().x < 428 * 32) //freeze decke im tunnel
		{
			Die();
			//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze decke im tunnel");
		}
		else if(GetPos().y > 222 * 32) //freeze becken unter area
		{
			//Die();
			//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze becken unter area");
		}

		if((GetPos().y < 220 * 32 && GetPos().x < 415 * 32 && m_pCharacter->m_FreezeTime > 1) && (GetPos().x > 350 * 32)) //always suicide on freeze if not reached the block area yet             (new) AND not coming from the new spawn and falling through the freeze
		{
			Die();
			//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze und links der block area");
		}

		//Movement bis zur ruler area:
		/*
		NEW! Movement modes for the basic move till hammerfly!
		m_Dummy_movement_mode23 is a int to check which movement style the bot should used

		0					normal old basic mode
		1					new mode jump left side up into ruler area [ALPHA]


		i dunno how to set the modes for now its hardcodet set to 1 maybe add a random switcher or depending on how frustrated the bot is

		*/
		m_Dummy_movement_mode23 = 0;

		if(GetPos().x < 388 * 32 && GetPos().y > 213 * 32) //jump to old spawn
		{
			Jump();
			Fire();
			Hook();
			AimX(-200);
			AimY(0);
		}

		if(m_Dummy_movement_mode23 == 0)
		{
			if(GetPos().x < 415 * 32) //bis zum tunnel laufen
			{
				Right();
			}
			else if(GetPos().x < 440 * 32 && GetPos().y > 213 * 32) //im tunnel laufen
			{
				Right();
				if(GetVel().x < 5.5f)
				{
					AimY(-3);
					AimX(200);

					if(Server()->Tick() % 30 == 0)
					{
						SetWeapon(0);
					}
					if(Server()->Tick() % 55 == 0)
					{
						if(m_pCharacter->m_FreezeTime == 0)
						{
							Fire();
						}
					}
					if(Server()->Tick() % 200 == 0)
					{
						Jump();
					}
				}
			}

			//externe if abfrage weil laufen während sprinegn xD
			if(GetPos().x > 413 * 32 && GetPos().x < 415 * 32) // in den tunnel springen
			{
				Jump();
				//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "triggered");
				//Jump(0);
			}
			else if(GetPos().x > 428 * 32 - 20 && GetPos().y > 213 * 32) // im tunnel springen
			{
				Jump();
			}

			// externen springen aufhören für dj
			if(GetPos().x > 428 * 32 && GetPos().y > 213 * 32) // im tunnel springen nicht mehr springen
			{
				Jump(0);
				//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "triggered");
			}

			//nochmal extern weil springen während springen
			if(GetPos().x > 430 * 32 && GetPos().y > 213 * 32) // im tunnel springen springen
			{
				Jump();
				//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "triggered");
			}

			if(GetPos().x > 431 * 32 && GetPos().y > 213 * 32) //jump refillen für wayblock spot
			{
				Jump(0);
			}
		}
		else if(m_Dummy_movement_mode23 == 1) //enter ruler area with a left jump
		{
			if(GetPos().x < 415 * 32) //bis zum tunnel laufen
			{
				Right();
			}
			else if(GetPos().x < 440 * 32 && GetPos().y > 213 * 32) //im tunnel laufen
			{
				Right();
			}

			//springen
			if(GetPos().x > 413 * 32 && GetPos().x < 415 * 32 && GetPos().y > 213 * 32) // in den tunnel springen
			{
				Jump();
			}
			else if(GetPos().x > 428 * 32 - 3 && GetPos().y > 217 * 32 && GetPos().y > 213 * 32) // im tunnel springen
			{
				Jump();
			}

			if(GetPos().x > 429 * 32 - 18)
			{
				Left();
			}

			//nochmal extern weil springen während springen
			if(GetPos().y < 217 * 32 && GetPos().x > 420 * 32 && GetPos().y > 213 * 32 + 20)
			{
				//Left();
				if(GetPos().y < 216 * 32) // im tunnel springen springen
				{
					Jump();
				}
			}
		}

		//MoVement ab dem ruler spot weiter laufen
		//NEW! wenn er links vom freeze becken im ruler spot is also beim wb spot des 18er modes dann jump übers freeze
		if(GetPos().y < 213 * 32 && GetPos().x > 428 * 32 && GetPos().x < 429 * 32)
		{
			Jump();
		}

		if(GetPos().x > 417 * 32 && GetPos().y < 213 * 32 && GetPos().x < 450 * 32) //vom ruler nach rechts nachm unfreeze werden
		{
			Right();
		}

		if(GetPos().x > 439 * 32 && GetPos().y < 213 * 32 && GetPos().x < 441 * 32) //über das freeze zum hf start springen
		{
			Jump();
		}

		if(GetPos().y > 200 * 32 && GetPos().x > 457 * 32)
		{
			Left();
		}

		//unnötiger dj
		//if (GetPos().x > 441 * 32 + 10 && GetPos().y < 213 * 32 && GetPos().x < 442 * 32)
		//{
		//	Jump();
		//}

		//TODO:
		//aufpassen dass er das ganze nur macht wenn er nicht schon beim 2p part ist
		if(m_Dummy_collected_weapons)
		{
			if(GetPos().x < 466 * 32)
			{
				SetWeapon(3);
			}
			//prepare for rocktjump

			if(GetPos().x < 451 * 32 + 1 && GetPos().y > 209 * 32) //wenn zu weit links für rj
			{
				Right();
			}
			else if(GetPos().x > 451 * 32 + 3 && GetPos().y > 209 * 32) //wenn zu weit links für rj
			{
				Left();
			}
			else
			{
				if(GetVel().x < 0.01f && GetVel().x > -0.01f) //nahezu stillstand
				{
					//ROCKETJUMP
					if(GetPos().x > 450 * 32 && GetPos().y > 209 * 32)
					{
						//Wenn der bot weit genung is und ne waffe hat und tief genung is
						// ---> bereit machen für rocketjump
						//damit der bot nicht ausm popo schiesst xD

						AimX(0);
						AimY(37);
					}

					if(GetPos().y > 210 * 32 + 30 && !IsFrozen()) //wenn der dummy auf dem boden steht und unfreeze is
					{
						if(GetVel().y == 0.000000f)
						{
							Jump();
						}
					}

					if(GetPos().y > 210 * 32 + 10 && GetVel().y < -0.9f && !IsFrozen()) //dann schiessen
					{
						Fire();
					}
				}
			}
		}

		if(GetPos().x > 448 * 32 && GetPos().x < 458 * 32 && GetPos().y > 209 * 32) //wenn der bot auf der platform is
		{
			//nicht zu schnell laufen
			if(Server()->Tick() % 3 == 0)
			{
				StopMoving();
				//GameServer()->SendEmoticon(m_pPlayer->GetCid(), 7, -1);
			}
		}

		//Rocketjump2 an der freeze wand
		//prepare aim!
		if(GetPos().y < 196 * 32)
		{
			AimX(-55);
			AimY(32);
		}

		if(GetPos().x < 452 * 32 && GetPos().y > 188 * 32 && GetPos().y < 192 * 32 && GetVel().y < 0.1f && m_Dummy_collected_weapons)
		{
			m_Dummy_rjumped2 = true;
			Fire();
		}

		//Fliegen nach rj2
		if(m_Dummy_rjumped2)
		{
			Right();

			if(GetPos().x > 461 * 32 && GetPos().y > 192 * 32 + 20)
			{
				Jump();
			}

			if(GetPos().x > 478 * 32 || GetPos().y > 196 * 32)
			{
				m_Dummy_rjumped2 = false;
			}
		}

		//Check ob der dummy schon waffen hat
		if(m_pCharacter->GetWeaponGot(3) && m_pCharacter->GetWeaponGot(2))
		{
			// m_Dummy_collected_weapons = true;
		}
		else //wenn er sie wd verliert zb durch shields
		{
			m_Dummy_collected_weapons = false;
		}

		if(1 == 0.5 + 0.5)
		{
			CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
			if(pChr && pChr->IsAlive())
			{
				if(pChr->GetPos().y < 165 * 32 && pChr->GetPos().x > 451 * 32 - 10 && pChr->GetPos().x < 454 * 32 + 10)
				{
					m_Dummy_mate_collected_weapons = true;
				}
			}
		}

		//Hammerfly
		if(GetPos().x > 447 * 32)
		{
			CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
			if(pChr && pChr->IsAlive())
			{
				//unfreezemates on platform

				//Get closer to the mate
				if(pChr->GetPos().y == GetPos().y && GetPos().x > 450 * 32 && GetPos().x < 457 * 32 && pChr->m_FreezeTime > 0)
				{
					if(pChr->GetPos().x > GetPos().x + 70) //if friend is on the right of the bot
					{
						Right();
					}
					else if(pChr->GetPos().x < GetPos().x - 70) //if firend is on the left of the bot
					{
						Left();
					}
				}

				//Hammer mate if near enough
				if(GetPos().x < 456 * 32 + 20 && pChr->m_FreezeTime > 0 && GetPos().y > 209 * 32 && pChr->GetPos().y > 209 * 32 && pChr->GetPos().x > 449 * 32 && pChr->GetPos().x < 457 * 32)
				{
					if(pChr->GetPos().x > GetPos().x - 60 && pChr->GetPos().x < GetPos().x + 60)
					{
						if(m_pCharacter->GetActiveWeapon() == WEAPON_HAMMER && m_pCharacter->m_FreezeTime == 0)
						{
							Fire();
							m_Dummy_hook_mate_after_hammer = true;
						}
					}
				}
				if(m_Dummy_hook_mate_after_hammer)
				{
					if(pChr->GetVel().x < -0.3f || pChr->GetVel().x > 0.3f)
					{
						Hook();
					}
					else
					{
						m_Dummy_hook_mate_after_hammer = false;
					}

					//stop this hook after some time to prevent nonstop hooking if something went wrong
					if(Server()->Tick() % 100 == 0)
					{
						m_Dummy_hook_mate_after_hammer = false;
					}
				}

				if(IsFrozen(pChr))
				{
					m_Dummy_help_before_fly = true;
				}
				if(pChr->m_FreezeTime == 0)
				{
					m_Dummy_help_before_fly = false;
				}
			}

			if(m_Dummy_help_before_fly)
			{
				if(!m_Dummy_collected_weapons)
				{
					if(Server()->Tick() % 20 == 0)
					{
						SetWeapon(0);
					}
					CCharacter *pChrFreeze = GameServer()->m_World.ClosestCharTypeFreeze(GetPos(), true, m_pCharacter); //only search freezed tees --> so even if others get closer he still has his mission
					if(pChrFreeze && pChrFreeze->IsAlive())
					{
						AimPos(pChrFreeze->GetPos());

						//GameServer()->SendEmoticon(m_pPlayer->GetCid(), 2, -1);

						//Check where help is needed
						if(pChrFreeze->GetPos().x > 457 * 32 + 10 && pChrFreeze->GetPos().x < 468 * 32 && pChrFreeze->GetPos().y < 213 * 32 + 5) //right freeze becken
						{
							//Get in help position:
							if(GetPos().x < 457 * 32 - 1)
							{
								Right();
							}
							else if(GetPos().x > 457 * 32 + 8)
							{
								Left();
							}

							//jump
							if(GetVel().y == 0.000000f && m_pCharacter->m_FreezeTime == 0 && GetPos().y > 209 * 32)
							{
								if(Server()->Tick() % 16 == 0)
								{
									Jump();
								}
							}

							//hook
							if(GetPos().y < pChrFreeze->GetPos().y - 60 && pChrFreeze->m_FreezeTime > 0)
							{
								Hook();
								if(GetPos().x > 454 * 32)
								{
									Left();
								}
							}

							//unfreezehammer
							if(pChrFreeze->GetPos().x < GetPos().x + 60 && pChrFreeze->GetPos().x > GetPos().x - 60 && pChrFreeze->GetPos().y < GetPos().y + 60 && pChrFreeze->GetPos().y > GetPos().y - 60)
							{
								if(m_pCharacter->m_FreezeTime == 0)
								{
									Fire();
								}
							}
						}
						else if(pChrFreeze->GetPos().x > 469 * 32 + 20 && pChrFreeze->GetPos().x < 480 * 32 && pChrFreeze->GetPos().y < 213 * 32 + 5 && pChrFreeze->GetPos().y > 202 * 32)
						{
							//Get in help position:
							if(GetPos().x < 467 * 32)
							{
								if(GetPos().x < 458 * 32)
								{
									if(GetVel().y == 0.000000f)
									{
										Right();
									}
								}
								else
								{
									Right();
								}

								if(GetVel().y > 0.2f || GetPos().y > 212 * 32)
								{
									Jump();
								}
							}
							if(GetPos().x > 469 * 32)
							{
								Left();
								if(GetVel().y > 0.2f || GetPos().y > 212 * 32)
								{
									Jump();
								}
							}

							//jump
							if(GetVel().y == 0.000000f && m_pCharacter->m_FreezeTime == 0 && GetPos().y > 209 * 32 && GetPos().x > 466 * 32)
							{
								if(Server()->Tick() % 16 == 0)
								{
									Jump();
								}
							}

							//hook
							if(GetPos().y < pChrFreeze->GetPos().y - 60 && pChrFreeze->m_FreezeTime > 0)
							{
								Hook();
								if(GetPos().x > 468 * 32)
								{
									Left();
								}
							}

							//unfreezehammer
							if(pChrFreeze->GetPos().x < GetPos().x + 60 && pChrFreeze->GetPos().x > GetPos().x - 60 && pChrFreeze->GetPos().y < GetPos().y + 60 && pChrFreeze->GetPos().y > GetPos().y - 60)
							{
								if(m_pCharacter->m_FreezeTime == 0)
								{
									Fire();
								}
							}
						}
						else if(pChrFreeze->GetPos().x > 437 * 32 && pChrFreeze->GetPos().x < 456 * 32 && pChrFreeze->GetPos().y < 219 * 32 && pChrFreeze->GetPos().y > 203 * 32) //left freeze becken
						{
							if(m_pCharacter->GetWeaponGot(2) && Server()->Tick() % 40 == 0)
							{
								SetWeapon(2);
							}

							if(Jumped() == 0) //has dj --> go left over the freeze and hook ze mate
							{
								Left();
							}
							else //no jump --> go back and get it
							{
								Right();
							}

							if(GetPos().y > 211 * 32 + 21)
							{
								Jump();
								m_Dummy_help_m8_before_hf_hook = true;
								if(m_pCharacter->GetWeaponGot(2) && m_pCharacter->m_FreezeTime == 0)
								{
									Fire();
								}

								//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "jump + hook");
							}

							if(m_Dummy_help_m8_before_hf_hook)
							{
								Hook();
								m_Dummy_help_m8_before_hf_hook++;
								if(m_Dummy_help_m8_before_hf_hook > 60 && HookState() != HOOK_GRABBED)
								{
									m_Dummy_help_m8_before_hf_hook = 0;
									//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "stopped hook");
								}
							}
						}
						else //unknown area
						{
							m_Dummy_help_before_fly = false;
						}
					}
					else //no freezed tee found
					{
						m_Dummy_help_before_fly = false;
					}
				}
			}
			//else  //old else new is if because the bot can stop helping if the closestplayer is in a unknown area fail
			if(!m_Dummy_help_before_fly)
			{
				//                                 fuck off mate i go solo fggt xD
				if(!m_Dummy_collected_weapons /*|| !m_Dummy_mate_collected_weapons*/)
				{
					if(Server()->Tick() % 20 == 0)
					{
						SetWeapon(0);
					}

					// CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
					if(pChr && pChr->IsAlive())
					{
						AimPos(pChr->GetPos());

						//Hammerfly normal way
						if(m_Dummy_mode23 == 0)
						{
							//shot schiessen schissen
							//im freeze nicht schiessen
							if(!m_DummyFreezed)
							{
								//schiess delay
								if(Server()->Tick() >= m_EmoteTickNext && pChr->GetPos().y < 212 * 32 - 5)
								{
									m_pPlayer->m_LastEmote = Server()->Tick();
									GameServer()->SendEmoticon(m_pPlayer->GetCid(), 7, -1);

									Fire();

									m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() / 2;
								}

								//wenn schon nah an der nade
								if(GetPos().y < 167 * 32)
								{
									Jump();

									if(GetPos().x < 453 * 32 - 8)
									{
										Right();
									}
									else if(GetPos().x > 454 * 32 + 8)
									{
										Left();
									}
								}
							}
							else if(m_DummyFreezed) //if (m_DummyFreezed == false)
							{
								Fire(0);
								m_DummyFreezed = false;
								//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "hey im freezded lul xD");
							}
							else
							{
								//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "iwas is maechtig flasch gelaufen du bob");
							}
						}
						else if(m_Dummy_mode23 == 2) //Speedhammerfly for ChillerDragon
						{
							//lauf zu dem hin
							//nur wenn der nächste spieler grad nach ooben fliegt oder auf der selben höhe ist
							if(pChr->GetPos().y == GetPos().y || pChr->GetVel().y < -0.4f)
							{
								if(pChr->GetPos().y >= GetPos().y)
								{
									if(pChr->GetPos().x + 1 < GetPos().x) //wenn zu weit rechts
									{
										if(GetPos().x > 452 * 32)
											Left();
									}
									else if(GetPos().x + 1 < pChr->GetPos().x) //wenn zu weit links
									{
										if(GetPos().x < 455 * 32)
											Right();
									}
								}
							}

							//und wenn der hoch springt
							//hau den weg xD
							if(pChr->GetVel().y < -0.5f && GetPos().y + 15 > pChr->GetPos().y) //wenn der dude speed nach oben hat
							{
								Jump();
								if(m_pCharacter->m_FreezeTime == 0)
								{
									Fire();
								}
							}
						}
						else
						{
							if(Server()->Tick() % 600 == 0)
							{
								GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "for this dummymode23 is no hammerflytype set :c");
							}
						}
					}
				}
			}
		}

		/*
		Struct:

		STRUCT[1]: Check if bot should change m_Dummy_2p_state

		STRUCT[2]: Let the bot do stuff depenging on m_Dummy_2p_state

		States:
		-2				Help pChr out of freeze
		-1				do nothing
		0				prepare for making the part (getting in the right position)
		1				starting to do the part -> walking left and hammerin'
		2				keep going doing the part -> hookin' and walking to the right
		3				final stage of doing the part -> jumpin' and unfreeze pChr with hammerhit
		4				jump in freeze and let the mate help

		5				go on edge if pChr dragged u through the part
		6				if on edge sg and unfreeze mate

		*/

		if(GetPos().y < 200 * 32)
		{
			//check ob der mate fail ist
			CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
			if(pChr && pChr->IsAlive())
			{
				if((pChr->GetPos().y > 198 * 32 + 10 && pChr->IsGrounded()) ||
					(pChr->GetPos().y < 198 * 32 + 10 && pChr->GetPos().x < 472 * 32 && pChr->IsGrounded()) || // recognize mates freeze in the freeze tunnel on the left
					(m_Dummy_mate_help_mode == 3)) // yolo hook swing mode handles mate as failed until he is unfreeze
				{
					if(IsFrozen(pChr))
						m_Dummy_mate_failed = true;
				}
				if(pChr->m_FreezeTime == 0)
				{
					m_Dummy_mate_failed = false;
					m_Dummy_mate_help_mode = 2; // set it to something not 3 because help mode 3 cant do the part (cant play if not failed)
				}
			}

			//schau ob der bot den part geschafft hat und auf state -1 gehen soll
			if(GetPos().x > 485 * 32)
			{
				m_Dummy_2p_state = -1; //part geschafft --> mach aus
			}

			if(GetPos().x > 466 * 32)
			{
				// CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
				if(pChr && pChr->IsAlive())
				{
					AimPos(pChr->GetPos());

					//holla
					//if (m_Dummy_collected_weapons && m_pCharacter->m_FreezeTime == 0 && GetPos().x > 478 * 32 && GetPos().x < 485 * 32 && pChr->GetPos().x > 476 * 32)
					if(m_Dummy_collected_weapons && m_pCharacter->m_FreezeTime == 0 && GetPos().x > 478 * 32 && GetPos().x < 492 * 32 + 10 && pChr->GetPos().x > 476 * 32) //new testy
					{
						//New direct in state
						//if (m_Dummy_2p_state == 1)
						//SetWeapon(0);

						//Reset Checkbools

						//if (!HookState() == HOOK_GRABBED && m_Dummy_2p_hook_grabbed) //wenn der bot denkt er grabbt ihn noch aber schon los gelassen hat --> fang von vorne an
						//{
						//	m_Dummy_2p_hook = false;
						//}

						if(pChr->GetPos().y > 198 * 32) //wenn pChr iwiw runter gefallen is dann mach den hook weg
						{
							m_Dummy_2p_hook = false;
						}

						//STRUCT[1]: Check if bot should change m_Dummy_2p_state
						if(GetPos().x < 477 * 32 || m_Dummy_mate_failed) //TODO: add if pChr wants to make the part
						{
							m_Dummy_2p_state = -1;
						}
						//                                                                                     || neu resette wenn der spieler kurz von der platform springt
						// NEW: added the bool to not start doing the part while helping
						if(GetPos().x > 477 * 32 && GetPos().x < 485 * 32 && GetPos().y < 195 * 32 /*|| pChr->GetPos().x < 476 * 32 - 11 || pChr->GetPos().y < 191 * 32*/) //alle states die mit anfangen zutuen haben nur wenn der bot auch in position steht den part zu machen
						{
							if(pChr->m_FreezeTime == 0 && m_pCharacter->m_FreezeTime == 0) //wenn beide unfreeze sind zeih auf
							{
								m_Dummy_2p_state = 0;
								//m_Dummy_2p_hook = false;
								//m_Dummy_2p_hook_grabbed = false;
							}
							//																								// NEW testy || stuff
							if((GetPos().x > pChr->GetPos().x && pChr->GetPos().y == GetPos().y && GetPos().x > 481 * 32) || (pChr->GetPos().x > 476 * 32 - 10 && GetPos().x > pChr->GetPos().x && pChr->GetPos().y > 191 * 32 - 10 && GetPos().x < 482 * 32 + 10))
							{
								m_Dummy_2p_state = 1; //starting to do the part->walking left and hammerin'
								if(Server()->Tick() % 30 == 0 && m_Dummy_nothing_happens_counter == 0)
								{
									SetWeapon(0);
								}
								//m_Dummy_2p_hammer1 = false;
							}
							//                                                                                 NEW TESTY || stuff     wenn der schonmal ausgelöst wurde bleib da bis der nexte ausgelöst wird oder pChr runter füllt
							if((m_Dummy_2p_state == 1 && pChr->GetVel().y > 0.5f && pChr->GetPos().x < 479 * 32) || m_Dummy_2p_hook)
							{
								m_Dummy_2p_state = 2; //keep going doing the part->hookin' and walking to the right
								m_Dummy_2p_hook = true;
								/*						if (HookState() == HOOK_GRABBED)
								{
								m_Dummy_2p_hook_grabbed = true;
								}*/
							}

							if(m_Dummy_2p_state == 2 && pChr->GetPos().x > 485 * 32 + 8)
							{
								m_Dummy_2p_state = 3; //final stage of doing the part->jumpin' and unfreeze pChr with hammerhit
							}

							//           NICHT NACH FREEZE ABRAGEN damit der bot auch ins freeze springt wenn das team fail ist und dann selfkill macht
							if(pChr->GetPos().x > 489 * 32 || (pChr->GetPos().x > 486 * 32 && pChr->GetPos().y < 186 * 32)) //Wenn grad gehammert und der tee weit genugn is spring rein
							{
								m_Dummy_2p_state = 4;
							}

							if(pChr->GetPos().y < 191 * 32 && pChr->GetPos().x < 486 * 32) //resette auf state=0 wenn pChr springt
							{
								//TODO:
								//das auch mal aus machen auch wenn nichts abbricht
								m_Dummy_2p_hook = false;
							}

							//testy set the bot to mode -1 if mate fails
							if(m_Dummy_mate_failed)
							{
								m_Dummy_2p_state = -1;
							}
						}

						//state=? 5 //extern weil der bot woanders is
						if(m_pCharacter->m_FreezeTime == 0 && GetPos().x > 485 * 32 && pChr->GetPos().x < 485 * 32) //wenn der bot rechts und unfreeze is und der mate noch links
						{
							m_Dummy_2p_state = 5;
							//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "set state 5");
						}

						if(m_pCharacter->m_FreezeTime == 0 && GetPos().x > 490 * 32 && pChr->m_FreezeTime > 0)
						{
							m_Dummy_2p_state = 6;
						}

						//STRUCT[2]: Let the bot do stuff depenging on m_Dummy_2p_state

						if(m_Dummy_2p_state == 0) //prepare doing the part (getting right pos)
						{
							Right(); //walking right until state 1 gets triggered
							//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "debug [1]");
						}
						else if(m_Dummy_2p_state == 1) //starting to do the part -> walking left and hammerin'
						{
							if(GetPos().x > 480 * 32 - 15) //lauf nach links bis zur hammer pos
							{
								Left();
							}

							if(pChr->GetPos().x < 480 * 32) //wenn pChr weit gwenung zum hammern is
							{
								Fire();
								//m_Dummy_2p_hammer1 = true;
							}

							//testy stop mate if hammer was too hard and mate fly to far
							if(pChr->GetPos().x < 478 * 32)
							{
								Hook();
							}
						}
						else if(m_Dummy_2p_state == 2) //keep going doing the part->hookin' and walking to the right
						{
							if(pChr->GetPos().y > 194 * 32 + 10)
								Hook();

							if(pChr->GetPos().y < 197 * 32)
								Right();
							//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "debug [2]");
						}
						else if(m_Dummy_2p_state == 3) //final stage of doing the part->jumpin' and unfreeze pChr with hammerhit
						{
							if(Server()->Tick() % 30 == 0)
							{
								SetWeapon(0); //hammer
							}

							if(pChr->m_FreezeTime > 0) //keep holding hook until pChr is unfreeze
							{
								Hook();
							}

							//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "debug [3]");
							Right();
							Jump();

							//Now tricky part the unfreeze hammer
							if(pChr->GetPos().y - GetPos().y < 7 && m_pCharacter->m_FreezeTime == 0) //wenn der abstand der beiden tees nach oben weniger is als 7 ^^
							{
								Fire();
							}
						}
						//MOVED TO EXTERN CUZ SPECIAL
						//else if (m_Dummy_2p_state == 4) //PART geschafft! spring ins freeze
						//{
						//	if (GetPos().y < 195 * 32 && GetPos().x > 478 * 32) //wenn der bot noch auf der platform is
						//	{
						//		Left(); //geh links bisse füllst
						//	}
						//	else //wenn de füllst
						//	{
						//		Right();
						//	}
						//}
					}

					//Mega externen stuff is der state4 weil der ausm gültigeitsbereich (platform) raus läuft und so der is halt was beonders deswegen steht der an einer besonder verwirrenden stelle -.-
					if(!m_Dummy_mate_failed && m_Dummy_2p_state == 4) //PART geschafft! spring ins freeze
					{
						//Shotgun boost xD
						SetWeapon(2);
						AimX(1);
						AimY(1);

						if(GetPos().y < 195 * 32 && GetPos().x > 478 * 32 - 15) //wenn der bot noch auf der platform is
						{
							if(GetPos().x < 480 * 32) //wenn er schon knapp an der kante is
							{
								//nicht zu schnell laufen
								if(Server()->Tick() % 5 == 0)
								{
									Left(); //geh links bisse füllst
								}
							}
							else
							{
								Left();
							}
						}
						else //wenn de füllst
						{
							Right();
							//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "debug [4]");
						}

						//DJ ins freeze
						if(GetPos().y > 195 * 32 + 10)
						{
							Jump();
						}

						if(GetDirection() == 1 && m_pCharacter->m_FreezeTime == 0)
						{
							Fire();
						}
					}

					if(!m_Dummy_mate_failed && m_Dummy_2p_state == 5) //made the part --> help mate
					{
						if(pChr->m_FreezeTime == 0 && pChr->GetPos().x > 485 * 32)
						{
							m_Dummy_2p_state = -1;
						}

						if(Jumped() > 1) //double jumped
						{
							if(GetPos().x < 492 * 32 - 30) //zu weit links
							{
								Right();
								//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "Direction = 1");

								if(GetPos().x > 488 * 32) //wenn schon knapp dran
								{
									//nur langsam laufen (bissl bremsen)
									if(GetVel().x < 2.3f)
									{
										StopMoving();
									}
								}
							}
						}

						//hold left wall till jump to always do same move with same distance and speed
						if(Jumped() < 2 && !IsGrounded())
						{
							Left();
						}

						if(GetPos().y > 195 * 32)
						{
							Jump();
						}

						if(GetPos().x > 492 * 32)
						{
							Left();
						}
					}
					//else if (m_Dummy_2p_state == -2) //auch extern weil der dummy vlt mal von der platform springt zum helfen
					//if (m_Dummy_mate_failed && m_Dummy_2p_state < 1)    <--- added m_Dummy_mate_failed to the state checks
					if(m_Dummy_mate_failed)
					{
						//The bot coudl fall of the plattform and hurt but this var helps to activate and accident
						//sometimes the special stage causes a jump on purpose and the var gets true so no emergency can be called
						//to make this possible again reset this var every tick here
						//m_Dummy_help_no_emergency is used to allow the emergency help
						m_Dummy_help_no_emergency = false;

						if(Server()->Tick() % 20 == 0)
						{
							GameServer()->SendEmoticon(m_pPlayer->GetCid(), 7, -1);
						}

						//Go on left edge to help:
						if(GetPos().x > 479 * 32 + 4) //to much right
						{
							if(GetPos().x < 480 * 32 - 25)
							{
								if(Server()->Tick() % 9 == 0)
								{
									Left();
								}
							}
							else
							{
								Left();
								AimX(300);
								AimY(-10);
							}

							if(GetVel().x < -1.5f && GetPos().x < 480 * 32)
							{
								StopMoving();
							}
						}
						else if(GetPos().x < 479 * 32 - 1)
						{
							Right();
							//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "debug [6]");
						}

						//Get mate with shotgun in right position:
						//if (pChr->GetPos().x < 479 * 32 + 6) //if the mate is left enough to get shotgunned from the edge
						if(pChr->GetPos().x < 478 * 32 && (m_Dummy_mate_help_mode != 3)) // if currently in yolo fly save mode -> goto else branch to keep yolo flying
						{
							if(Server()->Tick() % 30 == 0)
							{
								SetWeapon(2); //switch to sg
							}

							if(m_pCharacter->m_FreezeTime == 0 && pChr->GetVel().y == 0.000000f && pChr->GetVel().x < 0.007f && pChr->GetVel().x > -0.007f && GetPos().x < 480 * 32)
							{
								Fire();
							}
						}
						else //if right enough to stop sg
						{
							if(pChr->GetPos().x < 479 * 32 && (m_Dummy_mate_help_mode != 3))
							{
								if(pChr->GetPos().y > 194 * 32)
								{
									if(pChr->GetPos().y > 197 * 32)
									{
										Hook();
									}
									//reset hook if something went wrong
									if(Server()->Tick() % 90 == 0 && pChr->GetVel().y == 0.000000f) //if the bot should hook but the mate lays on the ground --> reset hook
									{
										Hook(0);
										m_Dummy_nothing_happens_counter++;
										if(m_Dummy_nothing_happens_counter > 2)
										{
											if(GetPos().x > 478 * 32 - 1 && Jumped() == 0)
											{
												Left();
											}
											AimX(GetTargetX() - 5);
										}
										if(m_Dummy_nothing_happens_counter > 4) //warning long time nothing happend! do crazy stuff
										{
											if(m_pCharacter->m_FreezeTime == 0)
											{
												Fire();
											}
										}
										if(m_Dummy_nothing_happens_counter > 5) //high warning mate coudl get bored --> switch through all weapons and move angel back
										{
											SetWeapon(m_Dummy_panic_weapon);
											m_Dummy_panic_weapon++;
											AimX(GetTargetX() + 1);
										}
									}
									if(pChr->GetVel().y != 0.000000f)
									{
										m_Dummy_nothing_happens_counter = 0;
									}
								}
							}
							else
							{
								if(Server()->Tick() % 50 == 0)
								{
									SetWeapon(2);
								}
								if(m_pCharacter->GetActiveWeapon() == WEAPON_SHOTGUN && m_pCharacter->m_FreezeTime == 0)
								{
									if(pChr->GetPos().y < 198 * 32 && (m_Dummy_mate_help_mode != 3)) //if mate is high enough and not in yolo hook help mode
									{
										AimX(-200);
										AimY(30);
										Fire();
									}
									else //if mate is too low --> change angel or move depending on the x position
									{
										if(pChr->GetPos().x < 481 * 32 - 4) //left enough to get him with other shotgun angels from the edge
										{
											//first go on the leftest possible pos on the edge
											if(GetVel().x > -0.0004f && GetPos().x > 478 * 32 - 2 && Jumped() == 0)
											{
												Left();
											}
											/*
											[PLANNED BUT NOT NEEDED]: add more help modes and switch trough them. for now just help mode 2 is used and the int to switch modes is useless
											Then start to help.
											There are different help modes to have some variations if nothing happens
											help modes:

											1				Old way of helping try to wallshot straight down (doesnt work)
											2				New alternative! wallshot the left wall while jumping
											3				2018 new yolo move with jumping in the air

											*/
											if(m_Dummy_mate_help_mode == 0) // start with good mode and increase chance of using it in the rand 0-3 range
											{
												m_Dummy_mate_help_mode = 3;
											}
											else if(Server()->Tick() % 400 == 0)
											{
												m_Dummy_mate_help_mode = rand() % 4;
											}

											if(m_Dummy_mate_help_mode == 3) // 2018 new yolo move with jumping in the air
											{
												if(Jumped() < 2)
												{
													if(GetPos().x > 476 * 32)
													{
														Left();
														if(IsGrounded() && GetPos().x < 481 * 32)
														{
															Jump();
														}

														if(GetPos().x < 477 * 32)
														{
															Hook(); // start hook
														}
														if(HookState() == HOOK_GRABBED)
														{
															Hook(); // hold hook
															Right();
															AimX(200);
															AimY(7);
															if(!m_pCharacter->m_FreezeTime)
															{
																Fire();
															}
														}
													}

													// anti stuck on edge
													if(Server()->Tick() % 80 == 0)
													{
														if(GetVel().x < 0.0f && GetVel().y < 0.0f)
														{
															Left();
															if(GetPos().y > 193 * 32 + 18) // don't jump in the freeze roof
															{
																Jump();
															}
															Hook();
															if(!m_pCharacter->m_FreezeTime)
															{
																Fire();
															}
														}
													}
												}
											}
											else if(m_Dummy_mate_help_mode == 2) //new (jump and wallshot the left wall)
											{
												if(GetPos().y > 193 * 32 && GetVel().y == 0.000000f)
												{
													if(Server()->Tick() % 30 == 0)
													{
														Jump();
													}
												}

												if(GetPos().y < 191 * 32) //prepare aim
												{
													AimX(-300);
													AimY(200);

													if(GetPos().y < 192 * 32 - 30) //shoot
													{
														if(m_pCharacter->m_FreezeTime == 0 && m_pCharacter->GetActiveWeapon() == WEAPON_SHOTGUN && GetVel().y < -0.5f)
														{
															Fire();
														}
													}
												}

												//Panic if fall of platform
												if(GetPos().y > 195 * 32 + 5)
												{
													Jump();
													Right();
													AimX(300);
													AimY(-2);
													m_Dummy_2p_panic_while_helping = true;
												}
												if((GetPos().x > 480 * 32 && m_pCharacter->m_FreezeTime == 0) || m_pCharacter->m_FreezeTime > 0) //stop this mode if the bot made it back to the platform or failed
												{
													m_Dummy_2p_panic_while_helping = false;
												}
												if(m_Dummy_2p_panic_while_helping)
												{
													Right();
													AimX(300);
													AimY(-2);
												}
											}
											else if(m_Dummy_mate_help_mode == 1) //old (shooting straight down from edge and try to wallshot)
											{
												AimX(15);
												AimY(300);
												if(GetVel().x > -0.1f && m_pCharacter->m_FreezeTime == 0)
												{
													Fire();
												}

												if(GetPos().y > 195 * 32 + 5)
												{
													Jump();
													StopMoving(); //old 1
													AimX(300);
													AimY(-2);
													m_Dummy_2p_panic_while_helping = true;
												}
												if((GetPos().x > 480 * 32 && m_pCharacter->m_FreezeTime == 0) || m_pCharacter->m_FreezeTime > 0) //stop this mode if the bot made it back to the platform or failed
												{
													m_Dummy_2p_panic_while_helping = false;
												}
												if(m_Dummy_2p_panic_while_helping)
												{
													if(GetPos().y < 196 * 32 - 8)
													{
														Right();
													}
													else
													{
														StopMoving();
													}
													AimX(300);
													AimY(-2);
												}
											}
										}
										else //if mate is far and dummy has to jump of the platform and shotgun him
										{
											//in this stage of helping the bot jumps of the platform on purpose
											//m_Dummy_help_no_emergency is used to prevent the an emergency because its planned
											m_Dummy_help_no_emergency = true;

											if(Server()->Tick() % 30 == 0)
											{
												SetWeapon(2);
											}
											//go down and jump
											if(Jumped() >= 2) //if bot has no jump
											{
												Right();
											}
											else
											{
												Left();

												if(GetPos().x < 477 * 32 && GetVel().x < -3.4f) //dont rush too hard intro nowehre
												{
													StopMoving();
												}

												if(GetPos().y > 195 * 32) //prepare aim
												{
													AimPos(pChr->GetPos());

													if(GetPos().y > 196 * 32 + 25 || GetPos().x < 475 * 32 + 15)
													{
														Fire();
														Jump();
													}

													if((pChr->GetPos().x < 486 * 32 && GetPos().y > 195 * 32 + 20) || (pChr->GetPos().x < 486 * 32 && GetPos().x < 477 * 32)) //if mate is in range add a hook
													{
														m_Dummy_dd_helphook = true;
													}
													if(GetPos().x > 479 * 32)
													{
														m_Dummy_dd_helphook = false;
													}

													if(m_Dummy_dd_helphook)
													{
														Hook();
													}
												}
											}
										}
									}
								}
							}
						}

						if(pChr->GetPos().x < 475 * 32) // mate failed in left tunnel
						{
							int dist = distance(pChr->GetPos(), GetPos());
							if(dist < 11 * 32)
							{
								Hook();
								m_Dummy_mate_help_mode = 3;
								if(Server()->Tick() % 100 == 0) // reset hook to not get stuck
								{
									Hook(0);
									if(IsGrounded())
									{
										Jump(); // idk do something
									}
								}
							}
						}

						if(GetPos().y < pChr->GetPos().y + 40 && pChr->GetPos().x < 479 * 32 + 10 && m_pCharacter->m_FreezeTime == 0) //if the mate is near enough to hammer
						{
							//dont switch to hammer because without delay it sucks
							//and with delay its too slow
							//the bot should anyways have a hammer ready in this situation
							// so ---> just shoot
							Fire();
						}

						//do something if nothing happens cuz the bot is stuck somehow
						if(Server()->Tick() % 100 == 0 && pChr->GetVel().y == 0.000000f && m_Dummy_nothing_happens_counter == 0) //if the mate stands still after 90secs the m_Dummy_nothing_happens_countershoudl get triggered. but if not this if function turns true
						{
							//[PLANNED]: this can cause an loop where nothing happens..
							//maybe add some weapon changes or change m_Input.m_TargetX a bit

							Left(); //ye just walk until an emergency gets called xD
							//ik pro trick but it moves the bot around
						}

						//Emergency takes over here if the bot got in a dangerous situation!
						//if (GetPos().y > 196 * 32 + 30) //+25 is used for the jump help and with 30 it shouldnt get any confusuion i hope
						if((GetPos().y > 195 * 32 && !m_Dummy_help_no_emergency)) //if the bot left the platform
						{
							m_Dummy_help_emergency = true;
						}

						if((GetPos().x > 479 * 32 && Jumped() == 0) || IsFrozen())
						{
							m_Dummy_help_emergency = false;
						}

						if(m_Dummy_help_emergency)
						{
							//reset all and let emergency control all xD
							Hook(0);
							Jump(0);
							StopMoving();
							Fire(0);

							if(Server()->Tick() % 20 == 0)
							{
								GameServer()->SendEmoticon(m_pPlayer->GetCid(), 1, -1);
							}

							AimX(0);
							AimY(-200);

							if(GetPos().y > 194 * 32 + 18)
							{
								Jump();
							}
							if(Jumped() >= 2)
							{
								Right();
							}
						}

						if(GetPos().x < 475 * 32 && GetVel().x < -2.2f)
						{
							if(GetVel().y > 1.1f) // falling -> sg roof
							{
								AimX(10);
								AimY(-120);
								if(!m_pCharacter->m_FreezeTime)
								{
									Fire();
								}
							}
							if(GetPos().y > 193 * 32 + 18) // don't jump in the freeze roof
							{
								Jump();
							}
							Right();
							m_Dummy_help_emergency = true;
						}
					} //dummy_mate_failed end

					if(m_Dummy_2p_state == 6) //extern af fuck the system
					{
						//m_pPlayer->m_TeeInfos.m_ColorBody = (255 * 255 / 360);

						StopMoving();
						Jump(0);
						//Jump();
						AimPos(pChr->GetPos());

						if(Server()->Tick() % 40 == 0 && m_pCharacter->GetWeaponGot(2))
						{
							SetWeapon(2);
						}

						if(m_pCharacter->m_FreezeTime == 0)
						{
							Fire();
						}
					}
				}
			}
			//Hammerhit with race mate till finish
			if(m_Dummy_mode23 == 0 || m_Dummy_mode23 == 2) //normal hammerhit
			{
				if(GetPos().x > 491 * 32)
				{
					if(pChr && pChr->IsAlive())
					{
						if(GetPos().x <= 514 * 32 - 5 && pChr->GetPos().y < 198 * 32)
						{
							SetWeapon(0);
						}
					}

					CCharacter *pChrFreeze = GameServer()->m_World.ClosestCharTypeFreeze(GetPos(), true, m_pCharacter); //new 11.11.2017 Updated from ClosestCharType to TypeFreeze
					if(pChrFreeze && pChrFreeze->IsAlive() && pChr && pChr->IsAlive())
					{
						//if (pChrFreeze->GetPos().x > 485 * 32) //newly added this to improve the 2p_state = 5 skills (go on edge if mate made the part)
						if(pChrFreeze->GetPos().x > 490 * 32 + 2) //newly added this to improve the 2p_state = 5 skills (go on edge if mate made the part)
						{
							AimPos(pChrFreeze->GetPos());

							//just do things if unffr
							//old shit cuz he cant rls because mate is unfreeze and dont check for later rlsing hook
							//if (m_pCharacter->m_FreezeTime == 0 && pChrFreeze->m_FreezeTime > 0) //und der mate auch freeze is (damit der nich von edges oder aus dem ziel gehookt wird)
							//                  fuck the edge only stop if finish lol
							if((m_pCharacter->m_FreezeTime == 0 && pChrFreeze->GetPos().x < 514 * 32 - 2) || (m_pCharacter->m_FreezeTime == 0 && pChrFreeze->GetPos().x > 521 * 32 + 2))
							{
								//get right hammer pos [rechte seite]
								if(pChrFreeze->GetPos().x < 515 * 32) //wenn der mate links des ziels ist
								{
									if(GetPos().x > pChrFreeze->GetPos().x + 45) //zu weit rechts von hammer position
									{
										Left(); //gehe links
									}
									else if(GetPos().x < pChrFreeze->GetPos().x + 39) // zu weit links von hammer position
									{
										Right();
									}
								}
								else //get right hammer pos [rechte seite] (wenn der mate rechts des ziels is)
								{
									if(GetPos().x > pChrFreeze->GetPos().x - 45) //zu weit links von hammer position
									{
										Left();
									}
									else if(GetPos().x < pChrFreeze->GetPos().x - 39) // zu weit rechts von hammer position
									{
										Right();
									}
								}

								//deactivate bool for hook if mate is high enough or bot is freezed (but freezed is checked somewerhe else)
								//                                                                                              NEW: just rls hook if mate is higher than bot (to prevent both falling added new ||)
								//                                                                                                                                                                                oder wenn der mate unter dem bot ist und unfreeze
								if((pChrFreeze->m_FreezeTime == 0 && pChrFreeze->GetVel().y > -1.5f && GetPos().y > pChrFreeze->GetPos().y - 15) || pChrFreeze->GetVel().y > 3.4f || (pChrFreeze->m_FreezeTime == 0 && pChrFreeze->GetPos().y + 38 > GetPos().y) || IsFrozen())
								{
									m_Dummy_hh_hook = false;
								}
								//activate bool for hook if mate stands still
								if(pChrFreeze->GetVel().y == 0.000000f /*|| pChrFreeze->GetVel().y < -4.5f*/) //wenn er am boden liegt anfangen oder wenn er zu schnell nach obenfliegt bremsen
								{
									m_Dummy_hh_hook = true;
								}

								if(m_Dummy_hh_hook)
								{
									Hook();
								}

								//jump if too low && if mate is freeze otherwise it would be annoying af
								if(GetPos().y > 191 * 32 && pChrFreeze->m_FreezeTime > 0)
								{
									Jump();
								}

								//Hammer
								//if (pChrFreeze->GetPos().y - GetPos().y < 7 && pChrFreeze->m_FreezeTime > 0) //wenn der abstand der beiden tees nach oben weniger is als 7 ^^
								if(pChrFreeze->m_FreezeTime > 0 && pChrFreeze->GetPos().y - GetPos().y < 18) //wenn der abstand kleiner als 10 is nach oben
								{
									Fire();
								}
							}
							else
							{
								m_Dummy_hh_hook = false; //reset hook if bot is freeze
							}

							//ReHook if mate flys to high
							if((pChr->GetPos().y < GetPos().y - 40 && pChr->GetVel().y < -4.4f) || pChr->GetPos().y < 183 * 32)
							{
								Hook();
							}

							//Check for panic balance cuz all went wrong lol
							//if dummy is too much left and has no dj PANIC!
							//                                                                                                                                           New Panic Trigger: if both fall fast an the bot is on top
							if((pChr->GetPos().x < 516 * 32 && Jumped() >= 2 && GetPos().x < pChr->GetPos().x - 5 && pChr->GetPos().y > GetPos().y && GetPos().y > 192 * 32 && IsFrozen(pChr)) ||
								(GetVel().y > 6.7f && pChr->GetVel().y > 7.4f && pChr->GetPos().y > GetPos().y && GetPos().y > 192 * 32 && pChr->GetPos().x < 516 * 32))
							{
								if(!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "ChillerDragon") || !str_comp(Server()->ClientName(pChr->GetPlayer()->GetCid()), "ChillerDragon.*")) //only chatflame debug while racing with ChillerDragon
								{
									if(m_Dummy_sent_chat_msg == 0 && !m_Dummy_panic_balance && m_pCharacter->m_FreezeTime == 0)
									{
										int r = rand() % 16; // 0-15

										if(r == 0)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "YOU SUCK LOL!");
										}
										else if(r == 1)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "what do you do?!");
										}
										else if(r == 2)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "O M G =!! what r u triin mate?");
										}
										else if(r == 3)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "you did shit. i didnt do shit because im a bot and i am perfect.");
										}
										else if(r == 4)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "what was your plan?");
										}
										else if(r == 5)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "this looks bad! i try to balance...");
										}
										else if(r == 6)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "i think we gonna die ....  lemme try to balance");
										}
										else if(r == 7)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "what was this?");
										}
										else if(r == 8)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "you fucked it up .. let me try to save us with my balance skills.");
										}
										else if(r == 9)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "do you have lags? i dont have lags.");
										}
										else if(r == 10)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "lol.");
										}
										else if(r == 11)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "lul");
										}
										else if(r == 12)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "wtf?");
										}
										else if(r == 13)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "OMG");
										}
										else if(r == 14)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "?!");
										}
										else if(r == 15)
										{
											GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "whats going on here?!");
										}

										//GameServer()->SendEmoticon(m_pPlayer->GetCid(), 4, -1);
										m_Dummy_sent_chat_msg = 1;
									}
								}

								m_Dummy_panic_balance = true;
							}
							if(pChr->m_FreezeTime == 0 || IsFrozen() || pChr->GetPos().x > 512 * 32 + 5) //if mate gets unfreezed or dummy freezed stop balance
							{
								m_Dummy_panic_balance = false;
							}

							if(m_Dummy_panic_balance)
							{
								if(GetPos().x < pChr->GetPos().x - 2) //Bot is too far left
								{
									Right();
								}
								else if(GetPos().x > pChr->GetPos().x) //Bot is too far right
								{
									Left();
								}

								if(GetPos().x > pChr->GetPos().x - 2 && GetPos().x < pChr->GetPos().x && GetVel().x > -0.3f && m_pCharacter->m_FreezeTime == 0)
								{
									Right();
									Fire();
								}
							}

							//Go in finish if near enough
							if((GetVel().y < 4.4f && GetPos().x > 511 * 32) || (GetVel().y < 8.4f && GetPos().x > 512 * 32))
							{
								if(GetPos().x < 514 * 32 && !m_Dummy_panic_balance)
								{
									Right();
								}
							}

							//If dummy made it till finish but mate is still freeze on the left side
							//he automaiclly help. BUT if he fails the hook reset it!
							//left side                                                                                      right side
							if((GetPos().x > 514 * 32 - 5 && m_pCharacter->m_FreezeTime == 0 && IsFrozen(pChr) && pChr->GetPos().x < 515 * 32) || (GetPos().x > 519 * 32 - 5 && m_pCharacter->m_FreezeTime == 0 && IsFrozen(pChr) && pChr->GetPos().x < 523 * 32))
							{
								if(Server()->Tick() % 70 == 0)
								{
									Hook(0);
									//GameServer()->SendEmoticon(m_pPlayer->GetCid(), 1, -1);
								}
							}
							//if mate is too far for hook --> shotgun him
							if(GetPos().x > 514 * 32 - 5 && GetPos().x > pChr->GetPos().x && GetPos().x - pChr->GetPos().x > 8 * 32)
							{
								SetWeapon(2); //shotgun
								if(pChr->m_FreezeTime > 0 && m_pCharacter->m_FreezeTime == 0 && pChr->GetVel().y == 0.000000f)
								{
									Fire();
								}
							}
							//another hook if normal hook doesnt work
							//to save mate if bot is finish
							if(GetHook() == 0)
							{
								if(pChr->m_FreezeTime > 0 && m_pCharacter->m_FreezeTime == 0 && GetPos().y < pChr->GetPos().y - 60 && GetPos().x > 514 * 32 - 5)
								{
									Hook();
									Fire();
									if(Server()->Tick() % 10 == 0)
									{
										GameServer()->SendEmoticon(m_pPlayer->GetCid(), 1, -1);
									}
								}
							}

							//Important dont walk of finish plattform check
							//if (GetVel().y < 6.4f) //Check if not falling to fast
							if(!m_Dummy_panic_balance)
							{
								if((GetVel().y < 6.4f && GetPos().x > 512 * 32 && GetPos().x < 515 * 32) || (GetPos().x > 512 * 32 + 30 && GetPos().x < 515 * 32)) //left side
								{
									Right();
								}

								if(GetVel().y < 6.4f && GetPos().x > 520 * 32 && GetPos().x < 524 * 32 /* || too lazy rarly needed*/) //right side
								{
									Left();
								}
							}
						}
					}
				}
			}
			else if(m_Dummy_mode23 == 1) //tricky hammerhit (harder)
			{
				if(GetPos().x > 491 * 32)
				{
					SetWeapon(0);
					// CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
					if(pChr && pChr->IsAlive())
					{
						AimPos(pChr->GetPos());

						//just do things if unffr
						if(m_pCharacter->m_FreezeTime == 0 && pChr->m_FreezeTime > 0) //und der mate auch freeze is (damit der nich von edges oder aus dem ziel gehookt wird)
						{
							//get right hammer pos [rechte seite]
							if(GetPos().x > pChr->GetPos().x + 45)
							{
								Left();
							}
							else if(GetPos().x < pChr->GetPos().x + 39)
							{
								Right();
							}

							//deactivate bool for hook if mate is high enough or bot is freezed (but freezed is checked somewerhe else)
							//                                                                                              NEW: just rls hook if mate is higher than bot (to prevent both falling added new ||)
							if(/*GetPos().y - pChr->GetPos().y > 15 ||*/ (pChr->m_FreezeTime == 0 && pChr->GetVel().y < -2.5f && pChr->GetPos().y < GetPos().y) || pChr->GetVel().y > 3.4f)
							{
								m_Dummy_hh_hook = false;
								//GameServer()->SendEmoticon(m_pPlayer->GetCid(), 1, -1);
							}
							//activate bool for hook if mate stands still
							if(pChr->GetVel().y == 0.000000f) //wenn er am boden liegt anfangen oder wenn er zu schnell nach obenfliegt bremsen
							{
								m_Dummy_hh_hook = true;
							}

							//jump if too low && if mate is freeze otherwise it would be annoying af
							if(GetPos().y > 191 * 32 && pChr->m_FreezeTime > 0)
							{
								Jump();
							}

							//Hammer
							//wenn der abstand der beiden tees nach oben weniger is als 7 ^^
							if(pChr->m_FreezeTime > 0 && pChr->GetPos().y - GetPos().y < 18) //wenn der abstand kleiner als 10 is nach oben
							{
								Fire();
							}
						}
						else
						{
							m_Dummy_hh_hook = false; //reset hook if bot is freeze
							//GameServer()->SendEmoticon(m_pPlayer->GetCid(), 7, -1);
						}
					}
				}

				if(m_Dummy_hh_hook)
				{
					Hook();
				}
			}
		}

		//General bug protection reset hook in freeze
		//if (IsFrozen())
		if(m_pCharacter->m_FreezeTime > 0)
		{
			Hook(0); //reset hook in freeze to prevent bugs with no hooking at last part
		}
	}

	//Leave THis LAST !!!
	//chat stuff

	if(m_Dummy_sent_chat_msg > 0 && m_Dummy_sent_chat_msg < 100)
	{
		m_Dummy_sent_chat_msg++;
	}
	else
	{
		m_Dummy_sent_chat_msg = 0;
	}
}
