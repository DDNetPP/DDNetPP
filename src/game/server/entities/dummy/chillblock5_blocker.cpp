// MODE_CHILLBLOCK5_BLOCKER
// Mode 29
//
// based on mode 18 (tryhardwayblocker cb5)
// with some more human wayblock style not so try hard a cool chillblock5 blocker mode

#include "chillblock5_blocker.h"

#include "../character.h"

#include <base/math_ddpp.h>

#include <engine/shared/config.h>

#include <game/server/gamecontext.h>
#include <game/server/player.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos) * 32)

CDummyChillBlock5Blocker::CDummyChillBlock5Blocker(class CPlayer *pPlayer) :
	CDummyBase(pPlayer, DUMMYMODE_CHILLBLOCK5_BLOCKER)
{
	OnDeath();
}

void CDummyChillBlock5Blocker::OnDeath()
{
	m_DummyFreezeBlockTrick = 0;
	m_Dummy_trick_panic_check_delay = 0;
	m_Dummy_start_hook = false;
	m_Dummy_speedright = false;
	m_Dummy_trick3_panic_check = false;
	m_Dummy_trick3_panic = false;
	m_Dummy_trick3_start_count = false;
	m_Dummy_trick3_panic_left = false;
	m_Dummy_trick4_hasstartpos = false;
	m_Dummy_lock_bored = false;
	m_Dummy_doBalance = false;
	m_Dummy_AttackedOnSpawn = false;
	m_Dummy_bored_cuz_nothing_happens = false;
	m_Dummy_movement_to_block_area_style_window = false;
	m_Dummy_planned_movement = false;
	m_Dummy_jumped = false;
	m_Dummy_hooked = false;
	m_Dummy_moved_left = false;
	m_Dummy_hook_delay = false;
	m_Dummy_ruled = false;
	m_Dummy_pushing = false;
	m_Dummy_emergency = false;
	m_Dummy_wb_hooked = false;
	m_Dummy_left_freeze_full = false;
	m_Dummy_happy = false;
	m_Dummy_get_speed = false;
	m_Dummy_bored = false;
	m_Dummy_special_defend = false;
	m_Dummy_special_defend_attack = false;
	m_Dummy_bored_counter = 0;
	m_Dummy_mode18 = 0;
}

void CDummyChillBlock5Blocker::OnTick()
{
	Hook(0);
	Jump(0); //this is 29 only the mode 18 had no jump reset hope it works ... it should omg
	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "HALLO ICH BIN 29!");

	//Check ob dem bot langweilig geworden is :)

	if(m_Dummy_bored_counter > 2)
	{
		CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(GetPos(), true, m_pCharacter);
		if(pChr && pChr->IsAlive())
		{
		}
		else //no ruler alive
		{
			m_Dummy_lock_bored = true;
		}
	}
	else
	{
		m_Dummy_lock_bored = false;
	}

	if(m_Dummy_lock_bored)
	{
		if(GetPos().x < 429 * 32 && IsGrounded())
		{
			m_Dummy_bored = true;
			//static bool test = false;

			//if (!test)
			//{
			//	GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "bored");
			//	test = true;
			//}
		}
	}

	/*
	####################################################
	#    I M P O R T A N T    I N F O R M A T I O N    #
	####################################################

	DummyMode 29 is a very special mode cause it uses the mode18 as base
	and mode18 is for now the biggest mode ever made so this code is a huge mess
	so mode29 uses a lot of mode18 vars

	Mode18 is a tryhard ruler he wayblocks as good as he can and blocks if somebody manages to get in his area

	Mode29 is a blocker which is not dat tryhard he doesnt wayblock and does more random stuff and tries freezeblock tricks







	BRAND NEW STRUCTURE!
	WELCOME TO 18's SPECIAL MODE-CEPTION!

	dummymode 18 hase his own modes in the mode lol


	:::::::::::::::::::::
	dummymode29 modes
	:::::::::::::::::::::
	mode:         desc:
	0					Main mode
	1					attack mode (if ruler spot is ruled and bot is in tunnel)
	2                   different wayblock mode
	3					special defend mode
	4                   (PLANNED) 1on1 mode with counting in chat and helping




	dummymode29 code structure:
	- Check for activating other modes
	- big if clause with all modes

	*/

	//Check for activating other modes

	//Check mode 1 [Attack from tunnel wayblocker]
	//man könnte das auch mit m_Dummy_happy abfragen aber mich nich ganz so viel sinn
	if(GetPos().y > 214 * 32 && GetPos().x > 424 * 32)
	{
		CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerWB(GetPos(), true, m_pCharacter);
		if(pChr && pChr->IsAlive())
		{
			//Wenn der bot im tunnel ist und ein Gegner im RulerWB bereich
			m_Dummy_mode18 = 1;
			//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "Wayblocker gesichtet");
		}
	}
	else if(m_Dummy_bored)
	{
		m_Dummy_mode18 = 2;
	}
	else if(m_Dummy_special_defend) // Check mode 3 [Attack from tunnel wayblocker]
	{
		m_Dummy_mode18 = 3;
	}
	else
	{
		m_Dummy_mode18 = 0; // change to main mode
	}

	//Modes:

	if(m_Dummy_mode18 == 3) // special defend mode
	{
		// testy wenn der dummy in den special defend mode gesetzt wird pusht das sein adrenalin und ihm is nicht mehr lw
		m_Dummy_bored_counter = 0;

		CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(GetPos(), true, m_pCharacter);
		if(pChr && pChr->IsAlive())
		{
			AimX(pChr->GetPos().x - GetPos().x);
			AimY(pChr->GetPos().y - GetPos().y);

			// rest on tick
			Hook(0);
			Jump(0);
			StopMoving();
			Fire(0);
			SetWeapon(1); // gun verwenden damit auch erkannt wird wann der mode getriggert wird

			if(pChr->m_FreezeTime == 0)
			{
				// wenn der gegner doch irgendwie unfreeze wird übergib an den main mode und lass den notstand das regeln
				m_Dummy_special_defend = false;
				m_Dummy_special_defend_attack = false;
			}
			// mode18 sub mode 3
			// Main code:
			// warte bis der gegner auf den boden geklatscht ist
			// dann werf ihn rechts runter

			if(pChr->Core()->m_Vel.y > -0.9f && pChr->m_Pos.y > 211 * 32)
			{
				// wenn der gegner am boden liegt starte angriff
				m_Dummy_special_defend_attack = true;

				// start jump
				Jump();
			}

			if(m_Dummy_special_defend_attack)
			{
				// if the enemy is close enough do a double jump
				if(GetPos().x - pChr->m_Pos.x < 50)
				{
					Jump();
				}

				if(pChr->m_Pos.x < GetPos().x)
				{
					Hook();
				}
				else //wenn der gegner weiter rechts als der bot is lass los und übergib an main deine arbeit ist hier getahen
				{ //main mode wird evenetuell noch korrigieren mit schieben
					m_Dummy_special_defend = false;
					m_Dummy_special_defend_attack = false;
				}

				// the bot should be as much on the ride side as possible but not in the freeze

				if(GetPos().x < 427 * 32 + 15)
				{
					Right();
				}
				else
				{
					Left();
				}
			}
		}
		else // wenn kein gegner mehr im Ruler bereich is
		{
			m_Dummy_special_defend = false;
			m_Dummy_special_defend_attack = false;
		}
	}
	else if(m_Dummy_mode18 == 2) // different wayblock mode
	{
		//rest on tick
		Hook(0);
		Jump(0);
		StopMoving();
		Fire(0);
		if(Server()->Tick() % 30 == 0)
		{
			SetWeapon(0);
		}
		// if (Server()->Tick() % 5 == 0)
		// {
		// 	GameServer()->SendEmoticon(m_pPlayer->GetCid(), 7, -1);
		// }

		// Selfkills (bit random but they work)
		if(IsFrozen())
		{
			// wenn der bot freeze is warte erstmal n paar sekunden und dann kill dich
			if(Server()->Tick() % 300 == 0)
			{
				Die();
				m_Dummy_happy = false;
			}
		}

		CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler2(GetPos(), true, m_pCharacter);
		if(pChr && pChr->IsAlive())
		{
			// Check ob an notstand mode18 = 0 übergeben
			if(pChr->m_FreezeTime == 0)
			{
				m_Dummy_bored = false;
				m_Dummy_bored_counter = 0;
				m_Dummy_mode18 = 0;
			}

			AimX(pChr->GetPos().x - GetPos().x);
			AimY(pChr->GetPos().y - GetPos().y);

			Jump();

			// solange der bot über dem gegner ist (damit er wenn er ihn weg hammert nicht weiter hookt)
			if(pChr->GetPos().y > GetPos().y && pChr->GetPos().x > GetPos().x + 20)
			{
				Hook();
			}

			if(GetPos().x > 420 * 32)
			{
				Left();
			}

			if(pChr->GetPos().y < GetPos().y + 15)
			{
				Fire();
			}
		}
		else //lieblings position finden wenn nichts abgeht
		{
			//               old: 421 * 32
			if(GetPos().x < 423 * 32)
			{
				Right();
			}
			//                   old: 422 * 32 + 30
			else if(GetPos().x > 424 * 32 + 30)
			{
				Left();
			}
		}
	}
	else if(m_Dummy_mode18 == 1) //attack in tunnel
	{
		//Selfkills (bit random but they work)
		if(IsFrozen())
		{
			// wenn der bot freeze is warte erstmal n paar sekunden und dann kill dich
			if(Server()->Tick() % 300 == 0)
			{
				Die();
			}
		}

		//stay on position

		if(GetPos().x < 426 * 32 + 10) // zu weit links
		{
			Right(); //geh rechts
		}
		else if(GetPos().x > 428 * 32 - 10) //zu weit rechts
		{
			Left(); // geh links
		}
		else if(GetPos().x > 428 * 32 + 10) // viel zu weit rechts
		{
			Left(); // geh links
			Jump();
		}
		else
		{
			CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerWB(GetPos(), true, m_pCharacter);
			if(pChr && pChr->IsAlive())
			{
				if(pChr->GetPos().x < 436 * 32) //wenn er ganz weit über dem freeze auf der kante ist (hooke direkt)
				{
					AimX(pChr->GetPos().x - GetPos().x);
					AimY(pChr->GetPos().y - GetPos().y);
				}
				else //wenn der Gegner weiter hinter dem unhook ist (hook über den Gegner um ihn trozdem zu treffen und das unhook zu umgehen)
				{
					AimX(pChr->GetPos().x - GetPos().x - 50);
					AimY(pChr->GetPos().y - GetPos().y);
				}

				//char aBuf[256];
				//str_format(aBuf, sizeof(aBuf), "targX: %d = %d - %d", AimX(Chr->m_Pos.x, m_Pos.x));
				//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

				//Hook(0);
				CCharacter *pChrTunnel = GameServer()->m_World.ClosestCharTypeTunnel(GetPos(), true, m_pCharacter);
				if(pChrTunnel && pChrTunnel->IsAlive())
				{
					// wenn jemand im tunnel is check ob du nicht ausversehen den hookst anstatt des ziels in der WB area
					if(pChrTunnel->m_Pos.x < GetPos().x) //hooke nur wenn kein Gegner rechts von dem bot im tunnel is (da er sonst ziemlich wahrscheinlich den hooken würde)
					{
						Hook();
					}
				}
				else
				{
					// wenn eh keiner im tunnel is hau raus dat ding
					Hook();
				}

				// schau ob sich der gegner bewegt und der bot grad nicht mehr am angreifen iss dann resette falls er davor halt misshookt hat
				// geht nich -.-
				if(HookState() != HOOK_FLYING && HookState() != HOOK_GRABBED)
				{
					if(Server()->Tick() % 10 == 0)
					{
						Hook(0);
					}
				}

				if(GetVel().x > 3.0f)
				{
					Left();
				}
				else
				{
					StopMoving();
				}
			}
			else
			{
				m_Dummy_mode18 = 0;
			}
		}
	}
	else if(m_Dummy_mode18 == 0) //main mode
	{
		// if (mode18_main_init)
		// {
		// 	//initializing main mode...
		// 	//resetting stuff...
		// 	Hook(0);
		// }

		// Hook(0);
		// if (HookState() == HOOK_FLYING)
		// 	Hook();
		// else if (HookState() == HOOK_GRABBED)
		// 	Hook();
		// else
		// 	Hook(0);

		Jump(0);
		StopMoving();
		Fire(0);

		// char aBuf[256];
		// str_format(aBuf, sizeof(aBuf), "speed:  x: %f y: %f speed pChr:  x: %f y: %f", GetVel().x, GetVel().y);

		// GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

		if(1 == 2)
		{
			CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
			if(pChr && pChr->IsAlive())
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "speed pChr:  x: %f y: %f", pChr->Core()->m_Vel.x, pChr->Core()->m_Vel.y);

				//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
			}
		}

		//m_pPlayer->m_TeeInfos.m_Name = aBuf;

		if(GetVel().x > 1.0f)
		{
			//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "speed: schneller als 1");
		}

		//Check ob jemand in der linken freeze wand is

		CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerLeftFreeze(GetPos(), true, m_pCharacter); //wenn jemand rechts im freeze liegt
		if(pChr && pChr->IsAlive()) // wenn ein spieler rechts im freeze lebt
		{ //----> versuche im notstand nicht den gegner auch da rein zu hauen da ist ja jetzt voll
			// clang tidy redundant bool
			// m_Dummy_left_freeze_full = true;
			//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "Da liegt einer im freeze");
		}
		else // wenn da keiner is fülle diesen spot (linke freeze wand im ruler spot)
		{
			m_Dummy_left_freeze_full = false;
		}

		// hardcodet selfkill (moved in lower area only)
		// if (GetPos().x < 390 * 32 && GetPos().y > 214 * 32)  //Links am spawn runter
		// {
		// 	Die();
		// 	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "Links am spawn runter");
		// }
		// else if ((GetPos().y < 204 * 32 && GetPos().x < 415 * 32 && GetPos().x > 392 * 32 && GetPos().y > 190) || (GetPos().y < 204 * 32 && GetPos().x < 415 * 32 && GetPos().x < 390 * 32 && GetPos().y > 190)) //freeze decke am spawn
		// {
		// 	Die();
		// 	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze decke am spawn");
		// }
		// else if (GetPos().y > 218 * 32 + 31 /* für tee balance*/ && GetPos().x < 415 * 32) //freeze boden am spawn
		// {
		// 	Die();
		// 	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze boden am spawn");
		// }
		// else if (GetPos().y < 215 * 32 && GetPos().y > 213 * 32 && GetPos().x > 415 * 32 && GetPos().x < 428 * 32) //freeze decke im tunnel
		// {
		// 	Die();
		// 	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze decke im tunnel");
		// }
		// else if (GetPos().y > 222 * 32) //freeze becken unter area
		// {
		// 	Die();
		// 	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze becken unter area");
		// }
		// else if (GetPos().y > 213 * 32 && GetPos().x > 436 * 32) //freeze rechts neben freeze becken
		// {
		// 	//Die();
		// 	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze rechts neben freeze becken");
		// }
		// else if (GetPos().x > 469 * 32) //zu weit ganz rechts in der ruler area
		// {
		// 	//Die();
		// 	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "zu weit ganz rechts in der ruler area");
		// }
		// else if (GetPos().y > 211 * 32 + 34 && GetPos().x > 455 * 32) //alles freeze am boden rechts in der area
		// {
		// 	//Die();
		// 	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze boden rechts der area");
		// }

		// if (GetPos().y < 193 * 32 /*&& g_Config.m_SvChillBlock5Version == 1*/) //old spawn of unused version (this code makes no sense at all)
		// {
		// 	AimX(200);
		// 	AimY(-80);

		// 	//not falling in freeze is bad
		// 	if (GetVel().y < 0.01f && m_FreezeTime > 0)
		// 	{
		// 		if (Server()->Tick() % 40 == 0)
		// 		{
		// 			Die();
		// 		}
		// 	}
		// 	if (GetPos().y > 116 * 32 && GetPos().x > 394 * 32)
		// 	{
		// 		Die();
		// 	}

		// 	if (GetPos().x > 364 * 32 && GetPos().y < 126 * 32 && GetPos().y > 122 * 32 + 10)
		// 	{
		// 		if (GetVel().y > -1.0f)
		// 		{
		// 			Hook();
		// 		}
		// 	}

		// 	if (GetPos().y < 121 * 32 && GetPos().x > 369 * 32)
		// 	{
		// 		Left();
		// 	}
		// 	else
		// 	{
		// 		Right();
		// 	}
		// 	if (GetPos().y < 109 * 32 && GetPos().x > 377 * 32 && GetPos().x < 386 * 32)
		// 	{
		// 		Right();
		// 	}

		// 	if (GetPos().y > 128 * 32)
		// 	{
		// 		Jump();
		// 	}

		// 	//speeddown at end to avoid selfkill cuz to slow falling in freeze
		// 	if (GetPos().x > 384 * 32 && GetPos().y > 121 * 32)
		// 	{
		// 		AimX(200);
		// 		AimY(300);
		// 		Hook();
		// 	}
		// }
		// else //under 193 (above 193 is new spawn)

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
			else if(GetPos().x < 317 * 32) // top left spawn
			{
				if(GetPos().y < 158 * 32) // spawn area find down
				{
					// selfkill
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
		else
		{
			if(GetPos().x < 390 * 32 && GetPos().x > 325 * 32 && GetPos().y > 215 * 32) //Links am spawn runter
			{
				Die();
				//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "Links am spawn runter");
			}
			//else if ((GetPos().y < 204 * 32 && GetPos().x < 415 * 32 && GetPos().x > 392 * 32 && GetPos().y > 190) || (GetPos().y < 204 * 32 && GetPos().x < 415 * 32 && GetPos().x < 390 * 32 && GetPos().y > 190)) //freeze decke am old spawn
			//{
			//	Die();
			//	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze decke am old spawn");
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
				Die();
				//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze becken unter area");
			}
			else if(GetPos().y > 213 * 32 && GetPos().x > 436 * 32) //freeze rechts neben freeze becken
			{
				//Die();
				//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze rechts neben freeze becken");
			}
			else if(GetPos().x > 469 * 32) //zu weit ganz rechts in der ruler area
			{
				//Die();
				//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "zu weit ganz rechts in der ruler area");
			}
			else if(GetPos().y > 211 * 32 + 34 && GetPos().x > 455 * 32) //alles freeze am boden rechts in der area
			{
				//Die();
				//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze boden rechts der area");
			}

			if(GetPos().y < 220 * 32 && GetPos().x < 415 * 32 && m_pCharacter->m_FreezeTime > 1 && GetPos().x > 352 * 32) //always suicide on freeze if not reached the block area yet AND dont suicide in spawn area because new spawn sys can get pretty freezy
			{
				Die();
				//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze und links der block area");
			}

			// Movement
			/*
			NEW MOVEMENT TO BLOCK AREA STRUCTURE :)

			After spawning the bot thinks about what way he should choose.
			After he found one he stops thinking until he respawns again.

			if he thinks the tunnel is shit he goes trough the window

			*/

			//new spawn do something against hookers
			if(GetPos().x < 380 * 32 && GetPos().x > 322 * 32 && GetVel().x < -0.001f)
			{
				Hook();
				if((GetPos().x < 362 * 32 && IsGrounded()) || GetPos().x < 350 * 32)
				{
					if(Server()->Tick() % 10 == 0)
					{
						Jump();
						SetWeapon(0);
					}
				}
			}
			if(GetPos().x < 415 * 32)
			{
				// CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter); //hammer player up in freeze if in right pos
				if(pChr && pChr->IsAlive())
				{
					if(pChr->Core()->m_Pos.x > GetPos().x - 100 && pChr->Core()->m_Pos.x < GetPos().x + 100 && pChr->Core()->m_Pos.y > GetPos().y - 100 && pChr->Core()->m_Pos.y < GetPos().y + 100)
					{
						if(pChr->Core()->m_Vel.y < -1.5f) //only boost and use existing up speed
						{
							Fire();
						}
						if(Server()->Tick() % 3 == 0)
						{
							SetWeapon(0);
						}
					}
					//old spawn do something against way blockers (roof protection)
					if(GetPos().y > 206 * 32 + 4 && GetPos().y < 208 * 32 && GetVel().y < -4.4f)
					{
						Jump();
					}
					//old spawn roof protection / attack hook
					if(pChr->Core()->m_Pos.y > GetPos().y + 50)
					{
						Hook();
					}
				}
			}

			if(GetPos().x < 388 * 32 && GetPos().y > 213 * 32) //jump to old spawn
			{
				Jump();
				Fire();
				Hook();
				Aim(-200, 0);
			}

			if(!m_Dummy_planned_movement)
			{
				CCharacter *pChrTunnel = GameServer()->m_World.ClosestCharTypeTunnel(GetPos(), true, m_pCharacter);
				if(pChrTunnel && pChrTunnel->IsAlive())
				{
					if(pChrTunnel->Core()->m_Vel.x < 3.3f) //found a slow bob in tunnel
					{
						m_Dummy_movement_to_block_area_style_window = true;
					}
				}

				m_Dummy_planned_movement = true;
			}

			if(m_Dummy_movement_to_block_area_style_window)
			{
				if(GetPos().x < 415 * 32)
				{
					Right();

					if(GetPos().x > 404 * 32 && IsGrounded())
					{
						Jump();
					}
					if(GetPos().y < 208 * 32)
					{
						Jump();
					}

					if(GetPos().x > 410 * 32)
					{
						AimX(200);
						AimY(70);
						if(GetPos().x > 413 * 32)
						{
							Hook();
						}
					}
				}
				else //not needed but safety xD when the bot managed it to get into the ruler area change to old movement
				{
					m_Dummy_movement_to_block_area_style_window = false;
				}

				//something went wrong:
				if(GetPos().y > 214 * 32)
				{
					Jump();
					m_Dummy_movement_to_block_area_style_window = false;
				}
			}
			else //down way
			{
				//CheckFatsOnSpawn

				if(GetPos().x < 406 * 32)
				{
					// CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
					if(pChr && pChr->IsAlive())
					{
						AimX(pChr->GetPos().x - GetPos().x);
						AimY(pChr->GetPos().y - GetPos().y);

						if(pChr->GetPos().x < 407 * 32 && pChr->GetPos().y > 212 * 32 && pChr->GetPos().y < 215 * 32 && pChr->GetPos().x > GetPos().x) //wenn ein im weg stehender tee auf der spawn platform gefunden wurde
						{
							SetWeapon(0); //hol den hammer raus!
							if(pChr->GetPos().x - GetPos().x < 30) //wenn der typ nahe bei dem bot ist
							{
								if(m_pCharacter->Core()->m_FreezeStart == 0) //nicht rum schrein
								{
									Fire();
								}

								if(Server()->Tick() % 10 == 0)
								{
									GameServer()->SendEmoticon(m_pPlayer->GetCid(), 9, -1); //angry
								}
							}
						}
						else
						{
							if(Server()->Tick() % 20 == 0)
							{
								SetWeapon(1); //pack den hammer weg
							}
						}
					}
				}

				//Check attacked on spawn
				if(GetPos().x < 412 * 32 && GetPos().y > 217 * 32 && GetVel().x < -0.5f)
				{
					Jump();
					m_Dummy_AttackedOnSpawn = true;
				}
				if(IsGrounded())
				{
					m_Dummy_AttackedOnSpawn = false;
				}
				if(m_Dummy_AttackedOnSpawn)
				{
					if(Server()->Tick() % 100 == 0) //this shitty stuff can set it right after activation to false but i dont care
					{
						m_Dummy_AttackedOnSpawn = false;
					}
				}

				if(m_Dummy_AttackedOnSpawn)
				{
					int r = rand() % 89;

					if(r > 44)
					{
						Fire();
					}

					if(rand() % 1337 > 420)
					{
						SetWeapon(0);
					}

					// CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
					if(pChr && pChr->IsAlive())
					{
						r = rand() % 10 - 10;

						AimX(pChr->GetPos().x - GetPos().x);
						AimY(pChr->GetPos().y - GetPos().y - r);

						if(Server()->Tick() % 13 == 0)
						{
							GameServer()->SendEmoticon(m_pPlayer->GetCid(), 9, -1);
						}

						if(HookState() == HOOK_GRABBED || (GetPos().y < 216 * 32 && pChr->GetPos().x > 404 * 32) || (pChr->GetPos().x > 405 * 32 && GetPos().x > 404 * 32 + 20))
						{
							Hook();
							if(Server()->Tick() % 10 == 0)
							{
								int x = rand() % 20;
								int y = (rand() % 20) - 10;
								AimX(x);
								AimY(y);
							}
						}
					}
				}

				//CheckSlowDudesInTunnel

				if(GetPos().x > 415 * 32 && GetPos().y > 214 * 32) //wenn bot im tunnel ist
				{
					CCharacter *pChrTunnel = GameServer()->m_World.ClosestCharTypeTunnel(GetPos(), true, m_pCharacter);
					if(pChrTunnel && pChrTunnel->IsAlive())
					{
						if(pChrTunnel->Core()->m_Vel.x < 7.8f) //wenn der nächste spieler im tunnel ein slowdude is
						{
							//HauDenBau
							SetWeapon(0); //hol den hammer raus!

							AimX(pChrTunnel->GetPos().x - GetPos().x);
							AimY(pChrTunnel->GetPos().y - GetPos().y);

							if(m_pCharacter->Core()->m_FreezeStart == 0) //nicht rum schrein
							{
								Fire();
							}

							if(Server()->Tick() % 10 == 0) //angry emotes machen
							{
								GameServer()->SendEmoticon(m_pPlayer->GetCid(), 9, -1);
							}
						}
					}
				}

				//CheckSpeedInTunnel
				if(GetPos().x > 425 * 32 && GetPos().y > 214 * 32 && GetVel().x < 9.4f) //wenn nich genung speed zum wb spot springen
				{
					m_Dummy_get_speed = true;
				}

				if(m_Dummy_get_speed) //wenn schwung holen == true (tunnel)
				{
					if(GetPos().x > 422 * 32) //zu weit rechts
					{
						//---> hol schwung für den jump
						Left();

						//new hammer aggressive in the walkdirection to free the way
						if(!m_pCharacter->m_FreezeTime)
						{
							AimX(-200);
							AimY(-2);
							if(Server()->Tick() % 20 == 0)
							{
								SetWeapon(0);
							}
							if(Server()->Tick() % 25 == 0)
							{
								Fire();
							}
						}
					}
					else //wenn weit genung links
					{
						//dann kann das normale movement von dort aus genung schwung auf bauen
						m_Dummy_get_speed = false;
					}
				}
				else
				{
					if(GetPos().x < 415 * 32) //bis zum tunnel laufen
					{
						Right();
					}
					else if(GetPos().x < 440 * 32 && GetPos().y > 213 * 32) //im tunnel laufen
					{
						Right();
					}

					//externe if abfrage weil laufen während sprinegn xD

					if(GetPos().x > 413 * 32 && GetPos().x < 415 * 32) // in den tunnel springen
					{
						Jump();
						//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "triggerd");
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
						//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "triggerd");
					}

					//nochmal extern weil springen während springen

					if(GetPos().x > 430 * 32 && GetPos().y > 213 * 32) // im tunnel springen springen
					{
						Jump();
						//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "triggerd");
					}

					if(GetPos().x > 431 * 32 && GetPos().y > 213 * 32) //jump refillen für wayblock spot
					{
						Jump(0);
					}
				}
			}

			// *****************************************************
			// Way Block spot
			// *****************************************************
			// wayblockspot < 213 [y]

			//externer wayblockspot stuff

			//Checken ob der bot in seinem arial angegriffen wird obwohl kein nostand links ausgerufen wurde

			//wird nicht genutzt weil das preventive springen vom boden aus schluss endlich schlimmer ausgeht als der dj
			/*
			if (GetPos().y < 213 * 32 && GetPos().x > (427 * 32) - 20 && GetPos().x < (428 * 32) + 10) //wenn der bot sich an seinem ruler spot befindet
			{
			//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "Ich mag diesen ort :)");

			if (!m_Dummy_wb_hooked && !m_Dummy_emergency && !m_Dummy_pushing && GetVel().x > 0.90f) //wenn der bot sich auf das freeze zubewegt obwohl er nicht selber läuft
			{
			// --> er wird wahrscheinlich gehookt oder anderweitig extern angegriffen
			// --> schutzmaßnahmen treffen

			GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "AAAh ich werde angegriffen");
			Jump();
			}
			m_Dummy_pushing = false;
			m_Dummy_emergency = false;
			m_Dummy_wb_hooked = false;
			}
			*/

			//moved dynamic selfkills outside internal wb spot
			//self kill im freeze
			//New Testy selfkill kill if IsFrozen() and vel 0
			if(!IsFrozen() || GetVel().x < -0.5f || GetVel().x > 0.5f || GetVel().y != 0.000000f)
			{
				//mach nichts lol brauche nur das else is einfacher
			}
			else
			{
				if(Server()->Tick() % 150 == 0)
					Die();
			}

			//Bools zurueck setzten
			m_Dummy_pushing = false;
			m_Dummy_emergency = false;
			m_Dummy_wb_hooked = false;
			m_Dummy_happy = false;

			//normaler internen wb spot stuff

			//if (GetPos().y < 213 * 32) //old new added a x check idk why the was no
			if(GetPos().y < 213 * 32 && GetPos().x > 415 * 32)
			{
				//Old self kill kill if freeze
				//if (GetPos().y < 201 * 32) // decke
				//{
				//	Die();
				//	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "suicide reason: roof rulerspot");
				//}
				//else if (GetPos().x < 417 * 32 && GetPos().x > 414 * 32 + 17 && IsFrozen()) //linker streifen xD
				//{
				//	Die();
				//	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "suicide reason: left wall rulerspot");
				//}

				//New stuff RANDOM STOFF ROFL
				//if the bot is in his wb position an bored and there is no actual danger
				// ---> flick some aim and fire around xD

				//m_Dummy_bored_cuz_nothing_happens = true;

				//dont activate all the time and dunno how to make a cool activator code so fuck it rofl

				if(m_Dummy_bored_cuz_nothing_happens)
				{
					// CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
					if(pChr && pChr->IsAlive() && m_pCharacter->m_FreezeTime == 0)
					{
						if(pChr->GetPos().x < 429 * 32 && pChr->Core()->m_Vel.x < 4.3f)
						{
							int x = (rand() % 100) - 50;
							int y = rand() % 100;

							AimX(pChr->GetPos().x - GetPos().x + x);
							AimY(pChr->GetPos().y - GetPos().y + y);

							//doesnt work. i dont care. i dont fix. i just comment it out cuz fuck life is a bitch

							//int fr = rand() % 2000;
							//if (fr < 1300)
							//{
							//	m_Dummy_bored_shootin = true;
							//}

							//if (m_Dummy_bored_shootin)
							//{
							//	Fire();

							//	if (Server()->Tick() % 50 == 0)
							//	{
							//		m_Dummy_bored_shootin = false;
							//	}
							//}
						}
					}
				}

				//TODO(1): keep this structur in mind this makes not much sence
				// the bool m_Dummy_happy is just true if a enemy is in the ruler area because all code below depends on a enemy in ruler area
				// maybe rework this shit

				//
				//                                               --->   Ruler   <---    testy own class just search in ruler area

				CCharacter *pChrRuler = GameServer()->m_World.ClosestCharTypeRuler(GetPos(), true, m_pCharacter); //position anderer spieler mit pikus aimbot abfragen
				if(pChrRuler && pChrRuler->IsAlive() && pChr && pChr->IsAlive())
				{
					//sometimes walk to enemys.   to push them in freeze or super hammer them away
					if(pChrRuler->m_Pos.y < GetPos().y + 2 && pChrRuler->m_Pos.y > GetPos().y - 9)
					{
						if(pChrRuler->m_Pos.x > GetPos().x)
						{
							Right();
						}
						else
						{
							Left();
						}
					}

					if(pChrRuler->m_FreezeTime == 0) //if enemy in ruler spot is unfreeze -->notstand panic
					{
						//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "NOTSTAND");

						if(Server()->Tick() % 30 == 0) //angry emotes machen
							GameServer()->SendEmoticon(m_pPlayer->GetCid(), 9, -1);

						if(Server()->Tick() % 20 == 0)
							SetWeapon(0);

						AimX(pChrRuler->GetPos().x - GetPos().x);
						AimY(pChrRuler->GetPos().y - GetPos().y);

						if(m_pCharacter->m_FreezeTime == 0)
							Fire();

						//testy sollte eig auch am anfang des modes passen
						//StopMoving();

						//if (HookState() == HOOK_FLYING)
						//	Hook();
						//else if (HookState() == HOOK_GRABBED)
						//	Hook();
						//else
						//	Hook(0);

						//char aBuf[256];
						//str_format(aBuf, sizeof(aBuf), "hookstate: %x", m_Input.m_Hook);
						//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

						m_Dummy_emergency = true;

						if(!m_Dummy_left_freeze_full)
						{
							//                                                                                                        x > 5 = 3       <-- ignorieren xD

							//                                                                                                          hier wird das schieben an das andere schieben
							//                                                                                                    übergeben weil er hier von weiter weg anfängt zu schieben
							//                                                                                                und das kürzere schieben macht dann den ganzen stuff dass der bot nicht selber rein läuft
							//                                                                                                ja ich weiss das ist ziemlich umständlich xD
							//                                                                                                      aber das hat schon sinn das hier wird aufgerufen wenn der weit weg is und freezed und
							//                                                                                                  übergibt dann an die abfrage die auch aufgerufen wird wenn jemand unfreeze is jedoch nir auf kurze distanz

							//                                                                                                          tja aber das mit dem übergeben klappt ja nich wirklich

							//                                                                                                           Deswegen hab ich den code ganz gelöscht und nur ein teil als || in die "freeze protection & schieberrei" geklatscht
							//                                                                                                         ----> hier is ein berg an kommentaren zu nicht existentem code lol    gut das nur ich hier rum stüber hueueueu
							//start sequenz
							// Blocke spieler in die linke freeze wand

							if(!m_Dummy_jumped)
							{
								//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "boing!");
								Jump();
								m_Dummy_jumped = true;
							}
							else
							{
								Jump(0);
							}

							if(!m_Dummy_hooked)
							{
								if(Server()->Tick() % 30 == 0)
									m_Dummy_hook_delay = true;

								//testy removed hook here i dont know why but all works pretty good still xD
								if(m_Dummy_hook_delay)
									//Hook();

									if(Server()->Tick() % 200 == 0)
									{
										m_Dummy_hooked = true;
										Hook(0);
									}
							}

							if(!m_Dummy_moved_left)
							{
								if(GetPos().x > 419 * 32 + 20)
									Left();
								else
									Right();

								if(Server()->Tick() % 200 == 0)
								{
									m_Dummy_moved_left = true;
									StopMoving();
								}
							}
						}

						//Blocke gefreezte gegner für immer

						//TODO:
						//das is ein linke seite block wenn hier voll is sollte man anders vorgehen
						//                           früher war es y > 210   aber change weil er während er ihn hochzieht dann nicht mehr das hooken aufhört
						if(pChrRuler->m_FreezeTime > 0 && pChrRuler->m_Pos.y > 204 * 32 && pChrRuler->m_Pos.x > 422 * 32) //wenn ein gegner weit genung rechts freeze am boden liegt
						{
							// soll der bot sich einer position links des spielers nähern
							//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "hab n opfer gefunden");

							if(GetPos().x + (5 * 32 + 40) < pChrRuler->m_Pos.x) // er versucht 5 tiles links des gefreezten gegner zu kommen
							{
								Left();

								if(GetPos().x > pChrRuler->m_Pos.x && GetPos().x < pChrRuler->m_Pos.x + (4 * 32)) // wenn er 4 tiles rechts des gefreezten gegners is
								{
									Jump();
									//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "boing!");
								}
							}
							else //wenn der bot links des gefreezten spielers is
							{
								Jump();
								//echo jump
								//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "boing!");

								Left();

								if(GetPos().x < pChrRuler->m_Pos.x) //solange der bot weiter links is
								{
									Hook();
								}
								else
								{
									Hook(0);
								}
							}
						}

						//freeze protection & schieberrei
						//                                                                                                                                                                                                      old (417 * 32 - 60)
						if((pChrRuler->m_Pos.x + 10 < GetPos().x && pChrRuler->m_Pos.y > 211 * 32 && pChrRuler->m_Pos.x < 418 * 32) || (pChrRuler->m_FreezeTime > 0 && pChrRuler->m_Pos.y > 210 * 32 && pChrRuler->m_Pos.x < GetPos().x && pChrRuler->m_Pos.x > 417 * 32 - 60)) // wenn der spieler neben der linken wand linken freeze wand liegt schiebt ihn der bot rein
						{ // oder wenn der spieler weiter weg liegt aber freeze is

							if(!m_Dummy_left_freeze_full) //wenn da niemand is schieb den rein
							{
								// HIER TESTY TESTY CHANGES  211 * 32 + 40 stand hier
								if(pChrRuler->m_Pos.y > 211 * 32 + 40) // wenn der gegner wirklich ganz tief genung is
								{ //                          417 * 32 - 40
									if(GetPos().x > 418 * 32) // aber nicht selber ins freeze laufen
									{
										Left();

										//Check ob der gegener freeze is

										if(pChrRuler->m_FreezeTime > 0)
										{
											Fire(0);

											//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "haha owned");
										}

										//letzten stupser geben (sonst gibs bugs kb zu fixen)
										if(IsFrozen(pChrRuler)) //wenn er schon im freeze is
											Fire();
									}
									else
									{
										Right();
										//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "stop error code: 1");
										if(pChrRuler->m_FreezeTime > 0)
										{
											Fire(0);

											//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "haha owned");
										}
										//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "ich halte das auf.");
										//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "Ich will da nich rein laufen");
									}
								}
								else //wenn der gegner nicht tief genung ist
								{
									Right();

									if(pChrRuler->m_FreezeTime > 0)
									{
										Fire(0);

										//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "haha owned");
									}
								}
							}
							else //wenn da schon jemand liegt
							{
								// sag das mal xD
								//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "da liegt schon einer");
							}
						}
						else if(GetPos().x < 419 * 32 + 10) //sonst mehr abstand halten
						{
							Right();
							//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "stop error code: 2");
							//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "Ich will da nich rein laufen");
						}
						// else // wenn nichts los is erstmal stehen bleiben

						//freeze protection decke mit double jump wenn hammer

						if(GetVel().y < 20.0f && GetPos().y < 207 * 32) // wenn der tee nach oben gehammert wird
						{
							if(GetPos().y > 206 * 32) //ab 206 würd er so oder so ins freeze springen
								Jump();

							if(GetPos().y < pChr->m_Pos.y) //wenn der bot über dem spieler is soll er hooken
								Hook();
							else
								Hook(0);
						}

						//wenn der tee hcoh geschleudert wird
						//                 old 4 (macht aber im postiven bereich kein sinn aber hat geklappt)
						//                 HALLO HIER IST DEIN ICH AUS DER ZUKUNFT: du dummes kind wenn er in der luft hammert dann fliegt er doch nicht nach oben und gerade da musst du es ja perfekt haben ... low
						//if (GetVel().y < 4.0f && GetPos().y < pChr->m_Pos.y) //wenn er schneller als 4 nach oben fliegt und höher als der Gegener ist
						// lass das mit speed weg und mach lieber was mit höhe
						if(GetPos().y < 207 * 32 && GetPos().y < pChr->m_Pos.y)
						{
							//in hammer position bewegen
							if(GetPos().x > 418 * 32 + 20) //nicht ins freeze laufen
							{
								if(GetPos().x > pChr->m_Pos.x - 45) //zu weit rechts von hammer position
								{
									Left(); //gehe links
									//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "Ich will da nich rein laufen");
								}
								else if(GetPos().x < pChr->m_Pos.x - 39) // zu weit links von hammer position
								{
									Right();
									//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "stop error code: 3");
								}
								else //im hammer bereich
								{
									StopMoving(); //bleib da
								}
							}
						}

						//Check ob der gegener freeze is

						if(pChr->m_FreezeTime > 0 && pChr->m_Pos.y > 208 * 32 && !IsFrozen(pChr)) //wenn der Gegner tief und freeze is macht es wenig sinn den frei zu hammern
						{
							Fire(0);
							//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "haha owned");
						}

						//Hau den weg (wie dummymode 21)
						if(pChr->m_Pos.x > 418 * 32 && pChr->m_Pos.y > 209 * 32) //das ganze findet nur im bereich statt wo sonst eh nichts passiert
						{
							//wenn der bot den gegner nicht boosten würde hammer den auch nich weg
							Fire(0);

							if(pChr->Core()->m_Vel.y < -0.5f && GetPos().y + 15 > pChr->m_Pos.y) //wenn der dude speed nach oben hat
							{
								Jump();
								if(m_pCharacter->m_FreezeTime == 0)
								{
									Fire();
								}
							}
						}

						//TODO: FIX:
						//der bot unfreezed den gegner ab einer gewissen höhe wenn er rein gehammert wird schau das da was passiert

						//wenn ein tee freeze links neben dem bot liegt werf den einfach wieder ins freeze becken
						//das is bisher ja noch die einzige sicherheits lücke beim wayblocken
						//wenn man ein tee über den bot hammert

						if(pChr->m_Pos.x > 421 * 32 && pChr->Core()->m_FreezeStart > 0 && pChr->m_Pos.x < GetPos().x)
						{
							Jump();
							Hook();
						}

						//##############################
						// doggeystyle dogeing the freeze
						//##############################

						//during the fight dodge the freezefloor on the left with brain
						if(GetPos().x > 428 * 32 + 20 && GetPos().x < 438 * 32 - 20)
						{
							//very nobrain directions decision
							if(GetPos().x < 432 * 32) //left --> go left
							{
								Left();
							}
							else //right --> go right
							{
								Right();
							}

							//high speed left goto speed
							if(GetVel().x < -6.4f && GetPos().x < 434 * 32)
							{
								Left();
							}

							//low speed to the right
							if(GetPos().x > 431 * 32 + 20 && GetVel().x > 3.3f)
							{
								Right();
							}
						}

						//m_pPlayer->m_TeeInfos.m_ColorBody = (0 * 255 / 360);
						//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "Enemy in ruler spot found!");
					}
					else //sonst normal wayblocken und 427 aufsuchen
					{
						//m_Core.m_ActiveWeapon = WEAPON_GUN;
						SetWeapon(1);
						Jump(0);

						if(HookState() == HOOK_FLYING)
							Hook();
						else if(HookState() == HOOK_GRABBED)
							Hook();
						else
							Hook(0);

						//m_pPlayer->m_TeeInfos.m_ColorBody = (120 * 255 / 360);
						//positions check and correction 427

						m_Dummy_jumped = false;
						m_Dummy_hooked = false;
						m_Dummy_moved_left = false;

						if(GetPos().x > 428 * 32 + 15 && m_Dummy_ruled) //wenn viel zu weit ausserhalb der ruler area wo der bot nur hingehookt werden kann
						{
							Jump();
							Hook();
						}

						//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "Prüfe ob zu weit rechts");
						if(GetPos().x < (418 * 32) - 10) // zu weit links -> geh rechts
						{
							Right();
							//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "stop error code: 4");
						}
						else if(GetPos().x > (428 * 32) + 10) // zu weit rechts -> geh links
						{
							Left();
							//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "Ich bin zuweit rechts...");
						}
						else // im toleranz bereich -> stehen bleiben
						{
							m_Dummy_happy = true;
							m_Dummy_ruled = true;
							StopMoving();
							//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "toleranz bereich");
							//m_Input.m_LatestTargetX = 0;
							//m_Input.m_LatestTargetY = 0;

							//stuff im toleranz bereich doon

							// normal wayblock
							// CCharacter *pChrRuler = GameServer()->m_World.ClosestCharTypeRuler(GetPos(), true, m_pCharacter); //position anderer spieler mit pikus aimbot abfragen
							// if(pChrRuler && pChrRuler->IsAlive())
							{
								//AimX(pChrRuler->m_Pos.x - m_Pos.x);
								//AimY(pChrRuler->m_Pos.y - m_Pos.y);

								//AimX(1;//pChrRuler->m_Pos.x - m_Pos.x; //);
								//AimY(0;//pChrRuler->m_Pos.y - m_Pos.y; //);

								//i dunno why there is a if statement under this code and i dont wanna use it
								//so i made Trick[4] external (woo spagethii code)
								//Trick[4] clears the left freeze
								if(pChrRuler->m_Pos.x < 418 * 32 - 10 && pChrRuler->m_Pos.y > 210 * 32 && pChrRuler->m_Pos.y < 213 * 32 && IsFrozen(pChrRuler) && pChrRuler->Core()->m_Vel.y == 0.00f)
								{
									m_DummyFreezeBlockTrick = 4;
								}

								//                                                                                            old was: 418 * 32 + 20          and i have no fkin idea why so i changed to 17
								if(pChrRuler->m_Pos.y < 213 * 32 + 10 && pChrRuler->m_Pos.x < 430 * 32 && pChrRuler->m_Pos.y > 210 * 32 && pChrRuler->m_Pos.x > 417 * 32) // wenn ein spieler auf der linken seite in der ruler area is
								{
									//wenn ein gegner rechts des bots is prepare für trick[1]
									if(GetPos().x < pChrRuler->m_Pos.x && pChrRuler->m_Pos.x < 429 * 32 + 6)
									{
										Left();
										Jump(0);

										if(GetPos().x < 422 * 32)
										{
											Jump();
											m_DummyFreezeBlockTrick = 1;
											//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "trick1: hook to the left");
										}
									}
									//wenn ein gegner links des bots is prepare für tick[3]
									if(GetPos().x > pChrRuler->m_Pos.x && pChrRuler->m_Pos.x < 429 * 32)
									{
										Right();
										Jump(0);

										if(GetPos().x > 427 * 32 || GetPos().x > pChrRuler->m_Pos.x + (5 * 32))
										{
											Jump();
											m_DummyFreezeBlockTrick = 3;
											//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "trick3: hook to the right");
										}
									}
								}
								else // wenn spieler irgendwo anders is
								{
									//wenn ein spieler rechts des freezes is und freeze is Tric[2]
									if(pChrRuler->m_Pos.x > 433 * 32 && pChrRuler->m_FreezeTime > 0 && IsGrounded())
									{
										m_DummyFreezeBlockTrick = 2;
										//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "trick2: swinger");
									}
								}
							}
						}
					}
				}
				else // wenn kein lebender spieler im ruler spot ist
				{
					//Suche trozdem 427 auf

					if(GetPos().x < (427 * 32) - 20) // zu weit links -> geh rechts
					{
						Right();
						//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "stop error code: special");
					}
					else if(GetPos().x > (427 * 32) + 40) // zu weit rechts -> geh links
					{
						Left();
						//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "Ich bin zuweit rechts...");
					}
					//GameServer()->SendEmoticon(m_pPlayer->GetCid(), 1, -1);
				}

				// über das freeze springen wenn rechts der bevorzugenten camp stelle

				if(GetPos().x > 434 * 32)
				{
					Jump();
				}
				else if(GetPos().x > (434 * 32) - 20 && GetPos().x < (434 * 32) + 20) // bei flug über freeze jump wd holen
				{
					Jump(0);
					//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "refilling jump");
				}

				//new testy moved tricks into Wayblocker area (y < 213 && x > 215) (was external)
				//TRICKS
				if(1 == 1)
				{
					// CCharacter *pChrRuler = GameServer()->m_World.ClosestCharTypeRuler(GetPos(), true, m_pCharacter);
					if(pChrRuler && pChrRuler->IsAlive())
					{
						if(!m_Dummy_emergency && GetPos().x > 415 && GetPos().y < 213 * 32 && m_DummyFreezeBlockTrick != 0) //as long as no enemy is unfreeze in base --->  do some trickzz
						{
							//Trick reset all
							//resett in the tricks because trick1 doesnt want it
							//Hook(0);
							//Jump(0);
							//StopMoving();
							//Fire(0);

							//off tricks when not gud to make tricks#
							if(pChrRuler->m_FreezeTime == 0)
							{
								m_DummyFreezeBlockTrick = 0;
								m_Dummy_trick_panic_check_delay = 0;
								m_Dummy_trick3_panic_check = false;
								m_Dummy_trick3_start_count = false;
								m_Dummy_trick3_panic = false;
								m_Dummy_trick4_hasstartpos = false;
							}

							if(m_DummyFreezeBlockTrick == 1) //Tick[1] enemy on the right
							{
								if(IsFrozen(pChrRuler))
								{
									m_DummyFreezeBlockTrick = 0; //stop trick if enemy is in freeze
								}
								AimX(pChrRuler->GetPos().x - GetPos().x);
								AimY(pChrRuler->GetPos().y - GetPos().y);

								if(Server()->Tick() % 40 == 0)
								{
									SetWeapon(0);
								}

								if(pChrRuler->GetPos().x < GetPos().x && pChrRuler->GetPos().x > GetPos().x - 180) //if enemy is on the left in hammer distance
								{
									Fire();
								}

								if(GetPos().y < 210 * 32 + 10)
								{
									m_Dummy_start_hook = true;
								}

								if(m_Dummy_start_hook)
								{
									if(Server()->Tick() % 80 == 0 || pChrRuler->GetPos().x < GetPos().x + 22)
									{
										m_Dummy_start_hook = false;
									}
								}

								if(m_Dummy_start_hook)
								{
									Hook();
								}
								else
								{
									Hook(0);
								}
							}
							else if(m_DummyFreezeBlockTrick == 2) //enemy on the right plattform --> swing him away
							{
								Hook(0);
								Jump(0);
								StopMoving();
								Fire(0);

								if(Server()->Tick() % 50 == 0)
								{
									m_Dummy_bored_counter++;
									GameServer()->SendEmoticon(m_pPlayer->GetCid(), 7, -1);
								}

								if(GetPos().x < 438 * 32) //first go right
								{
									Right();
								}

								if(GetPos().x > 428 * 32 && GetPos().x < 430 * 32) //first jump
								{
									Jump();
								}

								if(GetPos().x > 427 * 32) //aim and swing
								{
									AimX(pChrRuler->GetPos().x - GetPos().x);
									AimY(pChrRuler->GetPos().y - GetPos().y);

									if(GetPos().x > 427 * 32 + 30 && pChrRuler->GetPos().y < 214 * 32)
									{
										Hook();
										if(Server()->Tick() % 10 == 0)
										{
											int x = (rand() % 100) - 50;
											int y = (rand() % 100) - 50;

											AimX(x);
											AimY(y);
										}
										// random shooting xD
										int r = (rand() % 200) + 10;
										if(Server()->Tick() % r == 0 && m_pCharacter->m_FreezeTime == 0)
										{
											Fire();
										}
									}
								}

								// also this trick needs some freeze dogining because sometime huge speed fucks the bot
								// and NOW THIS CODE is here to fuck the high speed
								// yo!
								if(GetPos().x > 440 * 32)
								{
									Left();
								}
								if(GetPos().x > 439 * 32 + 20 && GetPos().x < 440 * 32)
								{
									StopMoving();
								}
							}
							else if(m_DummyFreezeBlockTrick == 3) // enemy on the left swing him to the right
							{
								AimX(pChrRuler->GetPos().x - GetPos().x);
								AimY(pChrRuler->GetPos().y - GetPos().y);

								if(GetPos().y < 210 * 32 + 10)
								{
									m_Dummy_start_hook = true;
									m_Dummy_trick3_start_count = true;
								}

								if(m_Dummy_start_hook)
								{
									if(Server()->Tick() % 80 == 0 || pChrRuler->GetPos().x > GetPos().x - 22)
									{
										m_Dummy_start_hook = false;
									}
								}

								if(m_Dummy_start_hook)
								{
									Hook();
								}
								else
								{
									Hook(0);
								}

								if(GetPos().x < 429 * 32)
								{
									Right();
								}
								else
								{
									Left();
								}

								// STOPPER hook:
								// hook the tee if he flys to much to the right
								if(pChrRuler->m_Pos.x > 433 * 32 + 20)
								{
									Hook();
								}

								// Hook the tee again and go to the left -> drag him under block area
								// -->Trick 5
								if(pChrRuler->Core()->m_Vel.y > 8.1f && pChrRuler->m_Pos.x > 429 * 32 + 1 && pChrRuler->m_Pos.y > 209 * 32)
								{
									m_DummyFreezeBlockTrick = 5;
									Hook();
								}

								// if he lands on the right plattform switch trick xD
								// doesnt work anysways (now fixed by the stopper hook)
								if(pChrRuler->m_Pos.x > 433 * 32 && pChrRuler->Core()->m_Vel.y == 0.0f)
								{
									m_DummyFreezeBlockTrick = 2;
									// GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "trick gone wrong --> change trick");
								}

								// Check for trick went wrong --> trick3 panic activation
								if(m_Dummy_trick3_start_count)
								{
									m_Dummy_trick_panic_check_delay++;
								}
								if(m_Dummy_trick_panic_check_delay > 52)
								{
									m_Dummy_trick3_panic_check = true;
								}
								if(m_Dummy_trick3_panic_check)
								{
									if(pChrRuler->m_Pos.x < 430 * 32 && pChrRuler->m_Pos.x > 426 * 32 + 10 && pChrRuler->IsGrounded())
									{
										m_Dummy_trick3_panic = true;
										m_Dummy_trick3_panic_left = true;
									}
								}
								if(m_Dummy_trick3_panic)
								{
									//stuck --> go left and swing him down
									Right();
									if(GetPos().x < 425 * 32)
									{
										m_Dummy_trick3_panic_left = false;
									}

									if(m_Dummy_trick3_panic_left)
									{
										Left();
									}
									else
									{
										if(GetPos().y < 212 * 32 + 10)
										{
											Hook();
										}
									}
								}
							}
							else if(m_DummyFreezeBlockTrick == 4) // clear left freeze
							{
								Hook(0);
								Jump(0);
								StopMoving();
								Fire(0);

								if(!m_Dummy_trick4_hasstartpos)
								{
									if(GetPos().x < 423 * 32 - 10)
									{
										Right();
									}
									else if(GetPos().x > 424 * 32 - 20)
									{
										Left();
									}
									else
									{
										m_Dummy_trick4_hasstartpos = true;
									}
								}
								else //has start pos
								{
									AimX(-200);
									AimY(-2);
									if(IsFrozen(pChrRuler))
									{
										Hook();
									}
									else
									{
										Hook(0);
										m_DummyFreezeBlockTrick = 0; // fuck it too lazy normal stuff shoudl do the rest xD
									}
									if(Server()->Tick() % 7 == 0)
									{
										Hook(0);
									}
								}
							}
							else if(m_DummyFreezeBlockTrick == 5) // Hook under blockarea to the left (mostly the end of a trick)
							{
								//For now this trick only gets triggerd in trick 3 at the end

								//TODO: this trick needs a tick

								Hook();

								if(HookState() == HOOK_GRABBED)
								{
									Left();
								}
								else
								{
									if(GetPos().x < 428 * 32 + 20)
									{
										Right();
									}
								}
							}
						}
					}
					else // nobody alive in ruler area --> stop tricks
					{
						m_Dummy_trick4_hasstartpos = false;
						m_Dummy_trick3_panic = false;
						m_Dummy_trick3_start_count = false;
						m_Dummy_trick3_panic_check = false;
						m_Dummy_trick_panic_check_delay = 0;
						m_DummyFreezeBlockTrick = 0;
					}
				}
			}

			// ##################################
			//  29 only protections and doge moves
			// ##################################

			// Super last jumpy freeze protection o.O
			// saves alot bot live im very sure
			// #longlivesthebotrofl

			if(GetPos().x > 429 * 32 && GetPos().x < 436 * 32 && GetPos().y < 214 * 32) // dangerous area over the freeze
			{
				//first check! too low?
				if(GetPos().y > 211 * 32 + 10 && !IsGrounded())
				{
					Jump();
					Hook();
					if(Server()->Tick() % 4 == 0)
					{
						Jump(0);
					}
				}

				//second check! too speedy?
				if(GetPos().y > 209 * 32 && GetVel().y > 5.6f)
				{
					Jump();
					if(Server()->Tick() % 4 == 0)
					{
						Jump(0);
					}
				}
			}

			// survival moves above the second freeze in the ruler from the left
			//  ascii art shows where :
			//
			//                    |
			//                    |
			//                    v
			//                         --------
			// -----#####----###########-######
			// ###########-####################
			//            #
			//            #
			//            -#########################----------
			//            #--------------------------

			if(GetPos().x > 439 * 32 && GetPos().x < 449 * 32)
			{
				// low left lowspeed --> go left
				if(GetPos().x > 439 * 32 && GetPos().y > 209 * 32 && GetVel().x < 3.0f)
				{
					Left();
				}
				// low left highrightspeed --> go right with the speed and activate some random modes to keep the speed xD
				if(GetPos().x > 439 * 32 && GetPos().y > 209 * 32 && GetVel().x > 6.0f && Jumped() < 2)
				{
					Right();
					Jump();
					if(Server()->Tick() % 5 == 0)
					{
						Jump(0);
					}
					m_Dummy_speedright = true;
				}

				if(IsFrozen() || GetVel().x < 4.3f)
				{
					m_Dummy_speedright = false;
				}

				if(m_Dummy_speedright)
				{
					Right();
					AimX(200);
					int r = (rand() % 200) - 100;
					AimY(r);
					Hook();
					if(Server()->Tick() % 30 == 0 && HookState() != HOOK_GRABBED)
					{
						Hook(0);
					}
				}
			}
			else // out of the freeze area resett bools
			{
				m_Dummy_speedright = false;
			}

			// go down on plattform to get dj
			// bot always fails going back from right
			// because he doesnt refills his dj

			//             |
			//             |
			//             v
			//                         --------
			// -----#####----###########-######
			// ###########-####################
			//            #
			//            #
			//            -#########################----------
			//            #--------------------------

			if(GetPos().x > 433 * 32 + 20 && GetPos().x < 437 * 32 && Jumped() > 2)
			{
				Right();
			}

			// ##########################################
			//  S P E C I A L    S H I T ! ! !          #
			// ##########################################             agian...

			// woo special late important new stuff xD
			// reached hammerfly plattform --> get new movement skills
			// this area has his own extra codeblock with cool stuff

			if(GetPos().x > 448 * 32)
			{
				Hook(0);
				Jump(0);
				StopMoving();
				Fire(0);

				if(GetPos().x < 451 * 32 + 20 && !IsGrounded() && Jumped() > 2)
				{
					Right();
				}
				if(GetVel().x < -0.8f && GetPos().x < 450 * 32 && IsGrounded())
				{
					Jump();
				}
				// CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
				if(pChr && pChr->IsAlive())
				{
					if(GetPos().x < 451 * 32)
					{
						Right();
					}

					if(pChr->GetPos().x < GetPos().x - 7 * 32 && Jumped() < 2) // if enemy is on the left & bot has jump --> go left too
					{
						Left();
					}
					if(GetPos().x > 454 * 32)
					{
						Left();
						AimX(pChr->GetPos().x - GetPos().x);
						AimY(pChr->GetPos().y - GetPos().y);

						if(GetPos().y + 40 > pChr->GetPos().y)
						{
							Hook();
							if(Server()->Tick() % 50 == 0)
							{
								Hook(0);
							}
						}
					}

					//second freeze from the right --> goto singel palttform
					if(GetPos().x > 464 * 32 && Jumped() > 2 && GetPos().x < 468 * 32)
					{
						Right();
					}
					//go back
					if(GetPos().x < 468 * 32 && IsGrounded() && GetPos().x > 464 * 32)
					{
						Jump();
					}
					//balance
					if(GetPos().x > 460 * 32 && GetPos().x < 464 * 32 && GetPos().y > 210 * 32 + 10)
					{
						m_Dummy_doBalance = true;
					}
					if(IsGrounded() && IsFrozen())
					{
						m_Dummy_doBalance = false;
					}

					if(m_Dummy_doBalance)
					{
						if(GetPos().x > 463 * 32) //go right
						{
							if(GetPos().x > pChr->m_Pos.x - 4)
							{
								Left();
							}
							else if(GetPos().x > pChr->m_Pos.x)
							{
								Right();
							}

							if(GetPos().x < pChr->m_Pos.x)
							{
								AimX(5);
								AimY(200);
								if(GetPos().x - 1 < pChr->m_Pos.x)
								{
									Fire();
								}
							}
							else
							{
								//do the random flick
								int r = (rand() % 100) - 50;
								AimX(r);
								AimY(-200);
							}
							if(pChr->m_Pos.x > 465 * 32 - 10 && pChr->m_Pos.x < 469 * 32) //right enough go out
							{
								Right();
							}
						}
						else //go left
						{
							if(GetPos().x > pChr->m_Pos.x + 4)
							{
								Left();
							}
							else if(GetPos().x < pChr->m_Pos.x)
							{
								Right();
							}

							if(GetPos().x > pChr->m_Pos.x)
							{
								AimX(5);
								AimY(200);
								if(GetPos().x + 1 > pChr->m_Pos.x)
								{
									Fire();
								}
							}
							else
							{
								//do the random flick
								int r = (rand() % 100) - 50;
								AimX(r);
								AimY(-200);
							}
							if(pChr->m_Pos.x < 459 * 32) //left enough go out
							{
								Left();
							}
						}
					}
				}
				else //no close enemy alive
				{
					if(Jumped() < 2)
					{
						Left();
					}
				}

				//jump protection second freeze from the right

				//                                  |            -########
				//                                  |            -########
				//                                  v                    #
				//                                                       #
				//                -------------##########---##############
				// ...#####---#######-########------------ ---------------

				if(GetPos().x > 458 * 32 && GetPos().x < 466 * 32)
				{
					if(GetPos().y > 211 * 32 + 26)
					{
						Jump();
					}
					if(GetPos().y > 210 * 32 && GetVel().y > 5.4f)
					{
						Jump();
					}
				}

				//go home if its oky, oky?
				if(pChr && pChr->IsAlive())
				{
					if((GetPos().x < 458 * 32 && IsGrounded() && IsFrozen(pChr)) || (GetPos().x < 458 * 32 && IsGrounded() && pChr->m_Pos.x > GetPos().x + (10 * 32)))
					{
						Left();
					}
				}
				//keep going also in the air xD
				if(GetPos().x < 450 * 32 && GetVel().x < 1.1f && Jumped() < 2)
				{
					Left();
				}

				//                                           |   -########
				//                                           |   -########
				//                                           v           #
				//                                                       #
				//                -------------##########---##############
				// ...#####---#######-########------------ ---------------

				//go back if too far
				if(GetPos().x > 468 * 32 + 20)
				{
					Left();
				}
			}

			// shitty nub like jump correction because i am too lazy too fix bugsis
			// T0D0(done): fix this bugsis
			// the bot jumps somehow at spawn if a player is in the ruler area
			// i was working with dummybored and tricks
			// because i cant find the bug i set jump to 0 at spawn

			// here is ChillerDragon from ze future!
			//  FUCK YOU old ChillerDragon you just wasted my fucking time with this shitty line
			// im working on another movement where i need jumps at spawn and it took me 20 minutes to find this shitty line u faggot!
			// wow ofc the bot does shit at spawn because u only said the ruler area is (GetPos().y < 213 * 32) and no check on X omg!
			// hope this wont happen agian! (talking to you future dragon)

			// if (GetPos().x < 407 * 32)
			// {
			// 	Jump(0);
			// }
		}
	}
	else // Change to mode main and reset all
	{
		GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "EROOR!!!!");
		// RestOnChange (zuruecksetzten)
		Hook(0);
		Jump(0);
		StopMoving();
		Fire(0);

		m_Dummy_mode18 = 0;
	}
}
