// DummyMode 18 the tryhard wayblocker
// this mode <3 18 reks em all
// pathlaufing system (become ze ruler xD)
//
// You probably also want to check out mode 29
// which is a bit less tryhard and more fun version of this

#include "chillblock5_blocker_tryhard.h"

#include "../character.h"

#include <base/math_ddpp.h>

#include <engine/shared/config.h>

#include <game/server/gamecontext.h>
#include <game/server/player.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos) * 32)

CDummyChillBlock5BlockerTryHard::CDummyChillBlock5BlockerTryHard(class CPlayer *pPlayer) :
	CDummyBase(pPlayer, DUMMYMODE_CHILLBLOCK5_BLOCKER_TRYHARD)
{
	OnDeath();
}

void CDummyChillBlock5BlockerTryHard::OnDeath()
{
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
	m_EmoteTickNext = 0;
}

void CDummyChillBlock5BlockerTryHard::OnTick()
{
	Hook(0);
	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "HALLO ICH BIN 18!");

	//Check ob dem bot langweilig geworden is :)

	//m_Dummy_bored_counter = 5;

	if(m_Dummy_bored_counter > 2)
	{
		m_Dummy_bored = true;
	}

	/*
	BRAND NEW STRUCTURE!
	WELCOME TO 18's SPECIAL MODE-CEPTION!

	dummymode 18 hase his own modes in the mode lol


	:::::::::::::::::::::
	dummymode18 modes
	:::::::::::::::::::::
	mode:         desc:
	0					Main mode
	1					attack mode (if ruler spot is ruled and bot is in tunnel)
	2                   different wayblock mode
	3                   (PLANNED) 1on1 mode with counting in chat and helping




	dummymode18 code structure:
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
	else if(m_Dummy_happy && m_Dummy_bored)
	{
		m_Dummy_mode18 = 2;
	}
	else if(m_Dummy_special_defend) //Check mode 3 [Attack from tunnel wayblocker]
	{
		m_Dummy_mode18 = 3;
	}
	else
	{
		m_Dummy_mode18 = 0; //change to main mode
	}

	//Modes:

	if(m_Dummy_mode18 == 3) //special defend mode
	{
		//testy wenn der dummy in den special defend mode gesetzt wird pusht das sein adrenalin und ihm is nicht mehr lw
		m_Dummy_bored_counter = 0;

		CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(GetPos(), true, m_pCharacter);
		if(pChr && pChr->IsAlive())
		{
			AimPos(pChr->GetPos());

			//rest on tick
			Hook(0);
			Jump(0);
			StopMoving();
			Fire(0);
			SetWeapon(1); //gun verwenden damit auch erkannt wird wann der mode getriggert wird

			if(pChr->m_FreezeTime == 0)
			{
				//wenn der gegner doch irgendwie unfreeze wird übergib an den main mode und lass den notstand das regeln
				m_Dummy_special_defend = false;
				m_Dummy_special_defend_attack = false;
			}
			//mode18 sub mode 3
			//Main code:
			//warte bis der gegner auf den boden geklatscht ist
			//dann werf ihn rechts runter

			if(pChr->GetVel().y > -0.9f && pChr->GetPos().y > 211 * 32)
			{
				//wenn der gegner am boden liegt starte angriff
				m_Dummy_special_defend_attack = true;

				//start jump
				Jump();
			}

			if(m_Dummy_special_defend_attack)
			{
				if(GetPos().x - pChr->GetPos().x < 50) //wenn der gegner nah genung is mach dj
				{
					Jump();
				}

				if(pChr->GetPos().x < GetPos().x)
				{
					Hook();
				}
				else //wenn der gegner weiter rechts als der bot is lass los und übergib an main deine arbeit ist hier getahen
				{ //main mode wird evenetuell noch korrigieren mit schieben
					m_Dummy_special_defend = false;
					m_Dummy_special_defend_attack = false;
				}

				//Der bot sollte möglichst weit nach rechts gehen aber natürlich nicht ins freeze

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
		else //wenn kein gegner mehr im Ruler bereich is
		{
			m_Dummy_special_defend = false;
			m_Dummy_special_defend_attack = false;
		}
	}
	else if(m_Dummy_mode18 == 2) //different wayblock mode
	{
		//rest on tick
		Hook(0);
		Jump(0);
		StopMoving();
		Fire(0);
		SetWeapon(0);

		//Selfkills (bit random but they work)
		if(IsFrozen())
		{
			//wenn der bot freeze is warte erstmal n paar sekunden und dann kill dich
			if(Server()->Tick() % 300 == 0)
			{
				Die();
				m_Dummy_happy = false;
			}
		}

		CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler2(GetPos(), true, m_pCharacter);
		if(pChr && pChr->IsAlive())
		{
			//Check ob an notstand mode18 = 0 übergeben
			if(pChr->m_FreezeTime == 0)
			{
				m_Dummy_bored = false;
				m_Dummy_bored_counter = 0;
				m_Dummy_mode18 = 0;
			}

			AimPos(pChr->GetPos());

			Jump();

			if(pChr->GetPos().y > GetPos().y) //solange der bot über dem gegner ist (damit er wenn er ihn weg hammert nicht weiter hookt)
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
			if(GetPos().x < 421 * 32)
			{
				Right();
			}
			else if(GetPos().x > 422 * 32 + 30)
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
			//wenn der bot freeze is warte erstmal n paar sekunden und dann kill dich
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
					AimPos(pChr->GetPos());
				}
				else // wenn der Gegner weiter hinter dem unhook ist (hook über den Gegner um ihn trozdem zu treffen und das unhook zu umgehen)
				{
					AimPos(vec2(pChr->GetPos().x - 50, pChr->GetPos().y));
				}

				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "targX: %d = %.2f - %.2f", GetTargetX(), pChr->GetPos().x, GetPos().x);
				// GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

				// Hook(0);
				CCharacter *pChrTunnel = GameServer()->m_World.ClosestCharTypeTunnel(GetPos(), true, m_pCharacter);
				if(pChrTunnel && pChrTunnel->IsAlive())
				{
					// wenn jemand im tunnel is check ob du nicht ausversehen den hookst anstatt des ziels in der WB area
					if(pChrTunnel->GetPos().x < GetPos().x) //hooke nur wenn kein Gegner rechts von dem bot im tunnel is (da er sonst ziemlich wahrscheinlich den hooken würde)
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
				/*	if (!HookState() == HOOK_FLYING && !HookState() == HOOK_GRABBED)
				{
				if (Server()->Tick() % 10 == 0)
				Hook(0);
				}*/

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
		//if (mode18_main_init)
		//{
		//	//initialzing main mode...
		//	//resetting stuff...
		//	Hook(0);
		//}

		//Hook(0);
		//if (HookState() == HOOK_FLYING)
		//	Hook();
		//else if (HookState() == HOOK_GRABBED)
		//	Hook();
		//else
		//	Hook(0);

		Jump(0);
		StopMoving();
		Fire(0);

		//char aBuf[256];
		//str_format(aBuf, sizeof(aBuf), "speed:  x: %f y: %f speed pChr:  x: %f y: %f", GetVel().x, GetVel().y);

		//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

		//if (1 == 1)
		//{
		//	CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
		//	if (pChr && pChr->IsAlive())
		//	{
		//		char aBuf[256];
		//		str_format(aBuf, sizeof(aBuf), "speed pChr:  x: %f y: %f", pChr->GetVel().x, pChr->GetVel().y);

		//		//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
		//	}
		//}

		//m_pPlayer->m_TeeInfos.m_Name = aBuf;

		if(GetVel().x > 1.0f)
		{
			//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "speed: schneller als 1");
		}

		//Check ob jemand in der linken freeze wand is

		{
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
		}

		//Selfkill

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

		if(GetPos().y < 220 * 32 && GetPos().x < 415 * 32 && m_pCharacter->m_FreezeTime > 1) //always suicide on freeze if not reached teh block area yet
		{
			Die();
			//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "freeze und links der block area");
		}

		if(GetPos().x < 388 * 32 && GetPos().y > 213 * 32) //jump to old spawn
		{
			Jump();
			Fire();
			Hook();
			AimX(-200);
			AimY(0);
		}

		// Movement
		//CheckFatsOnSpawn

		if(GetPos().x < 406 * 32)
		{
			CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
			if(pChr && pChr->IsAlive())
			{
				AimX(pChr->GetPos().x - GetPos().x);
				AimY(pChr->GetPos().y - GetPos().y);

				if(pChr->GetPos().x < 407 * 32 && pChr->GetPos().y > 212 * 32 && pChr->GetPos().y < 215 * 32 && pChr->GetPos().x > GetPos().x) //wenn ein im weg stehender tee auf der spawn plattform gefunden wurde
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
					SetWeapon(1); //pack den hammer weg
				}
			}
		}

		//CheckSlowDudesInTunnel

		if(GetPos().x > 415 * 32 && GetPos().y > 214 * 32) //wenn bot im tunnel ist
		{
			CCharacter *pChr = GameServer()->m_World.ClosestCharTypeTunnel(GetPos(), true, m_pCharacter);
			if(pChr && pChr->IsAlive())
			{
				if(pChr->GetVel().x < 7.8f) //wenn der nächste spieler im tunnel ein slowdude is
				{
					//HauDenBau
					SetWeapon(0); //hol den hammer raus!

					AimX(pChr->GetPos().x - GetPos().x);
					AimY(pChr->GetPos().y - GetPos().y);

					if(m_pCharacter->Core()->m_FreezeStart == 0) //nicht rum schrein
					{
						Fire();
					}

					if(Server()->Tick() % 10 == 0) //do angry emotes
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

		// *****************************************************
		// Way Block spot (Main Spot) 29 29 29 29
		// *****************************************************
		// wayblockspot < 213

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

		//Bools zurueck setzten
		m_Dummy_pushing = false;
		m_Dummy_emergency = false;
		m_Dummy_wb_hooked = false;
		m_Dummy_happy = false;

		//normaler internen wb spot stuff

		if(GetPos().y < 213 * 32)
		{
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

			//because shitty structure (more infos at TODO(1)) check here for enemys attacking from outside of the ruler area

			//Checken Ob ein potentieller Gegner auf der edge unter dem WBbereich ist
			//Falls diesem so sei --> mach den da weg
			//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "debug");
			if(1 == 1)
			{
				//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "debug");
				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerWBBottom(GetPos(), true, m_pCharacter);
				if(pChr && pChr->IsAlive() && !IsFrozen(pChr)) //wenn jemand da so im bereich lebt und unfreeze ist
				{
					AimPos(pChr->GetPos());

					//erkenne gefahr
					// --> treffe gegen maßnahmen

					//lauf rum rand (bereit machen zum hooken)
					if(GetPos().x < 428 * 32 + 6) //wenn zu weit links um in dem winkel zu hooken
					{
						Right();
					}
					else if(GetPos().x > 428 * 32 + 28)
					{
						Left();
					}

					//hooke
					CCharacter *pChrClosest = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
					if(pChrClosest && pChrClosest->IsAlive())
					{
						//Wenn der nächste spieler unter der wb area ist hook
						//damit er wenn er einen falschen spieler gehookt hat oder sonst wie den nicht hochzieht
						if(pChrClosest->GetPos().y > 213 * 32 && GetPos().x > 427 * 32 + 3)
						{
							Hook();
						}
					}
				}
			}

			//TODO(1): keep this structur in mind this makes not much sence
			// the bool m_Dummy_happy is just true if a enemy is in the ruler area because all code below depends on a enemy in ruler area
			// maybe rework this shit
			//
			//                                               --->   Ruler   <---    testy own class just search in ruler area

			CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(GetPos(), true, m_pCharacter); //position anderer spieler mit pikus aimbot abfragen
			if(pChr && pChr->IsAlive())
			{
				//                                         old: 417 * 32
				//                                                      Old: tee<bot      New: tee<pos.x
				if((pChr->GetPos().y < 213 * 32 && pChr->GetPos().x > 417 * 32 - 5 /* && pChr->GetPos().x < GetPos().x*/ && pChr->GetPos().x < 428 * 32 && GetPos().x < 429 * 32 && GetPos().x > 415 * 32 && GetPos().y < 213 * 32) || //wenn ein tee weiter links ist als der bot && der bot links vom mittelfreeze im rulerspot steht
					(pChr->GetPos().y < 213 * 32 && pChr->GetPos().x > 417 * 32 - 5 /* && pChr->GetPos().x < GetPos().x*/ && pChr->GetPos().x < 444 * 32 && GetPos().x < 429 * 32 && GetPos().x > 415 * 32 && GetPos().y < 213 * 32 && pChr->m_FreezeTime == 0)) //oder der tee auch rechts vom bot ist aber unfreeze
				//wenn dies ist -> notstand links ausrufen und versuchen gegner zu blocken
				{
					//m_Core.m_ActiveWeapon = WEAPON_HAMMER;
					SetWeapon(0);

					//testy sollte eig auch am anfang des modes passen
					//StopMoving();

					//if (HookState() == HOOK_FLYING)
					//	Hook();
					//else if (HookState() == HOOK_GRABBED)
					//	Hook();
					//else
					//	Hook(0);

					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "hookstate: %x", GetHook());
					//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

					m_Dummy_emergency = true;

					if(!m_Dummy_left_freeze_full)
					{
						//                                                                                                        x > 5 = 3       <-- ignorieren xD

						//                                                                                                          hier wird das schieben an das andere schieben
						//                                                                                                    übergeben weil er hier von weiter weg anfängt zu schieben
						//                                                                                                und das kürzere schieben macht dann den ganzen stuff das der bot nicht selber rein läuft
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

					//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "NOTSTAND");

					if(Server()->Tick() % 30 == 0) //angry emotes machen
					{
						GameServer()->SendEmoticon(m_pPlayer->GetCid(), 9, -1);
					}

					AimPos(pChr->GetPos());

					//schiess delay
					if(Server()->Tick() >= m_EmoteTickNext)
					{
						m_pPlayer->m_LastEmote = Server()->Tick();

						//GameServer()->SendEmoticon(m_pPlayer->GetCid(), 7, -1);

						if(m_pCharacter->Core()->m_FreezeStart == 0) //nicht rum schrein
						{
							Fire();
						}

						m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() / 4;
					}

					//Blocke gefreezte gegner für immer

					//TODO:
					//das is ein linke seite block wenn hier voll is sollte man anders vorgehen
					//                           früher war es y > 210   aber change weil er während er ihn hochzieht dann nicht mehr das hooken aufhört
					if(pChr->m_FreezeTime > 0 && pChr->GetPos().y > 204 * 32 && pChr->GetPos().x > 422 * 32) //wenn ein gegner weit genung rechts freeze am boden liegt
					{
						// soll der bot sich einer position links des spielers nähern
						//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "hab n opfer gefunden");

						if(GetPos().x + (5 * 32 + 40) < pChr->GetPos().x) // er versucht 5 tiles links des gefreezten gegner zu kommen
						{
							Left();

							if(GetPos().x > pChr->GetPos().x && GetPos().x < pChr->GetPos().x + (4 * 32)) // wenn er 4 tiles rechts des gefreezten gegners is
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

							if(GetPos().x < pChr->GetPos().x) //solange der bot weiter links is
							{
								Hook();
							}
							else
							{
								Hook(0);
							}
						}
					}

					// freeze protection & schieberrei (pushing)
					//                                                                                                                                                                                                      old (417 * 32 - 60)
					if((pChr->GetPos().x + 10 < GetPos().x && pChr->GetPos().y > 211 * 32 && pChr->GetPos().x < 418 * 32) || (pChr->m_FreezeTime > 0 && pChr->GetPos().y > 210 * 32 && pChr->GetPos().x < GetPos().x && pChr->GetPos().x > 417 * 32 - 60)) // wenn der spieler neben der linken wand linken freeze wand liegt schiebt ihn der bot rein
					{ // oder wenn der spieler weiter weg liegt aber freeze is

						if(!m_Dummy_left_freeze_full) //wenn da niemand is schieb den rein
						{
							// HIER TESTY TESTY CHANGES  211 * 32 + 40 stand hier
							if(pChr->GetPos().y > 211 * 32 + 40) // wenn der gegner wirklich ganz tief genung is
							{ //                          417 * 32 - 40
								if(GetPos().x > 418 * 32) // aber nicht selber ins freeze laufen
								{
									Left();

									//Check ob der gegener freeze is

									if(pChr->m_FreezeTime > 0)
									{
										Fire(0);

										//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "haha owned");
									}

									//letzten stupser geben (sonst gibs bugs kb zu fixen)
									if(IsFrozen(pChr)) //wenn er schon im freeze is
									{
										Fire();
									}
								}
								else
								{
									Right();
									//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "stop error code: 1");
									if(pChr->m_FreezeTime > 0)
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

								if(pChr->m_FreezeTime > 0)
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

						if(GetPos().y < pChr->GetPos().y) //wenn der bot über dem spieler is soll er hooken
							Hook();
						else
							Hook(0);
					}

					//wenn der tee hcoh geschleudert wird
					//                 old 4 (macht aber im postiven bereich kein sinn aber hat geklappt)
					//                 HALLO HIER IST DEIN ICH AUS DER ZUKUNFT: du dummes kind wenn er in der luft hammert dann fliegt er doch nicht nach oben und gerade da musst du es ja perfekt haben ... low
					//if (GetVel().y < 4.0f && GetPos().y < pChr->GetPos().y) //wenn er schneller als 4 nach oben fliegt und höher als der Gegener ist
					// lass das mit speed weg und mach lieber was mit höhe
					if(GetPos().y < 207 * 32 && GetPos().y < pChr->GetPos().y)
					{
						//in hammer position bewegen
						if(GetPos().x > 418 * 32 + 20) //nicht ins freeze laufen
						{
							if(GetPos().x > pChr->GetPos().x - 45) //zu weit rechts von hammer position
							{
								Left(); //gehe links
								//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "Ich will da nich rein laufen");
							}
							else if(GetPos().x < pChr->GetPos().x - 39) // zu weit links von hammer position
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

					if(pChr->m_FreezeTime > 0 && pChr->GetPos().y > 208 * 32 && !IsFrozen(pChr)) //wenn der Gegner tief und freeze is macht es wenig sinn den frei zu hammern
					{
						Fire(0);
						//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "haha owned");
					}

					//Hau den weg (wie dummymode 21)
					if(pChr->GetPos().x > 418 * 32 && pChr->GetPos().y > 209 * 32) //das ganze findet nur im bereich statt wo sonst eh nichts passiert
					{
						//wenn der bot den gegner nicht boosten würde hammer den auch nich weg
						Fire(0);

						if(pChr->GetVel().y < -0.5f && GetPos().y + 15 > pChr->GetPos().y) //wenn der dude speed nach oben hat
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

					if(pChr->GetPos().x > 421 * 32 && pChr->Core()->m_FreezeStart > 0 && pChr->GetPos().x < GetPos().x)
					{
						Jump();
						Hook();
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
					if(GetPos().x < (427 * 32) - 20) // zu weit links -> geh rechts
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
						// CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter); //position anderer spieler mit pikus aimbot abfragen
						if(pChr && pChr->IsAlive())
						{
							//Check ob jemand special angeflogen kommt dann mode18 = 3 starten
							//Check ob special_defend aktiviert werden sollte
							if(pChr->GetPos().x < 431 * 32 && pChr->GetVel().y < -12.5f && pChr->GetVel().x < -7.4f)
							{
								m_Dummy_special_defend = true;
							}

							//debuggn special_defend
							//char aBuf[256];
							//str_format(aBuf, sizeof(aBuf), "speed pChr:  x: %f y: %f", pChr->GetVel().x, pChr->GetVel().y);
							//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

							//AimX(pChr->GetPos().x - GetPos().x);
							//AimY(pChr->GetPos().y - GetPos().y);

							//AimX(1;//pChr->GetPos().x - GetPos().x; //);
							//AimY(0;//pChr->GetPos().y - GetPos().y; //);

							if(pChr->GetPos().y < 213 * 32 + 10 && pChr->GetPos().x < 430 * 32 && pChr->GetPos().y > 210 * 32 && pChr->GetPos().x > 416 * 32 + 32) // wenn ein spieler auf der linken seite in der ruler area is
							{
								//wenn der typ über dem freze irgendwo rum fliegt

								if(pChr->GetPos().y < 212 * 32 - 10) //wenn er jedoch zu hoch zum schieben ist
								{
									//mach dich bereit zu schieben und geh nach links (aufziehen)
									Left();
								}
								else //wenn er tief genung zum schieben ist
								{
									if(GetPos().x < 428 * 32 + 10) //bei (429 * 32) gibts voll jiggle xD
									{
										Right(); //schieb ihn runter
										m_Dummy_pushing = true;
									}
									else
									{
										StopMoving(); // aber nicht zu weit
										//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "ja ich bin das!");
									}
								}
							}
							else // wenn spieler irgendwo anders is
							{
								//CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(GetPos(), true, m_pCharacter);  //wenn jemand oben is
								//if (pChr && pChr->IsAlive())
								//{
								//		Hook();
								//		m_Dummy_wb_hooked = true;
								//		GameServer()->SendEmoticon(m_pPlayer->GetCid(), 7, -1);

								//}

								if(!m_Dummy_emergency) //wayblock stuff hook ja nein pipapo nicht im notfall
								{
									CCharacter *pChrRulerWB = GameServer()->m_World.ClosestCharTypeRulerWB(GetPos(), true, m_pCharacter); //wenn jemand oben is (nur im wb bereich)
									if(pChrRulerWB && pChrRulerWB->IsAlive())
									{ // und er nicht zu tief ist (das is um unnötiges festgehalte zu verhindern) (bringt eh nichts weil das hier dann nicht mehr aufgerufen wird weil der dann nicht mehr in ClosesestCharTypeRulerWB ist -.-)
										AimX(pChrRulerWB->GetPos().x - GetPos().x);
										AimY(pChrRulerWB->GetPos().y - GetPos().y);

										//GameServer()->SendEmoticon(m_pPlayer->GetCid(), 8, -1);
										//                             noch eine vel abfrage weil der bot sonst daneben hookt
										if(pChrRulerWB->GetPos().y > 211 * 32 && pChrRulerWB->GetVel().y > -1.0f && pChrRulerWB->GetVel().y < 2.0f && pChrRulerWB->GetPos().x > 435 * 32 /*&& pChrRulerWB->GetPos().y < 213 * 32*/) //wenn er nich zu schnell nach oben fliegt und zu weit oben ist
										{
											//AimX(pChrRulerWB->GetPos().x - GetPos().x; //);
											//AimY(pChrRulerWB->GetPos().y - GetPos().y; //);

											Hook(0);
											Hook();
											m_Dummy_wb_hooked = true;
											if(Server()->Tick() % 30 == 0) //nicht zu hart spammen wenn iwas abgeht
											{
												GameServer()->SendEmoticon(m_pPlayer->GetCid(), 7, -1);
												m_Dummy_bored_counter++; //zähl mal mit wie lange der bot hier rum gurkt und wieviele spieler er so wayblockt
												char aBuf[256];
												str_format(aBuf, sizeof(aBuf), "dummy_bored_count: %d", m_Dummy_bored_counter);
												GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
											}
										}
									}
									else
									{
										if(Server()->Tick() % 10 == 0) //nicht zu hart spammen wenn iwas abgeht
											GameServer()->SendEmoticon(m_pPlayer->GetCid(), 1, -15);
										Hook(0);
										//GameServer()->SendEmoticon(m_pPlayer->GetCid(), 3, -1);
									}
								}
								//unnötiges festgehalte unten verhindern
								if(!m_Dummy_emergency) //auch hier wieder nur wenn kein notfall is
								{
									// CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true, m_pCharacter);
									if(pChr && pChr->IsAlive())
									{
										if(pChr->GetPos().y > 213 * 32 - 5 && HookState() == HOOK_GRABBED && pChr->GetPos().y < 213 * 32 + 5)
										{
											//Wenn folgendes:
											// kein notstand
											// der bot wenn am haken hat
											// der nächste spieler aber unter der ruler area ist (genauer gesagt gerade im freeze eingetaucht)
											// --> vermute ich mal das er genau diesen spieler hookt
											// --> den los lassen da dieser sowieso keien gefahr mehr ist damit ein neuer gegner schneller geblockt werden kann
											if(Server()->Tick() % 10 == 0) //nicht zu hart spammen wenn iwas abgeht
												GameServer()->SendEmoticon(m_pPlayer->GetCid(), 5, -1);

											Hook(0);
										}
									}
								}

								//if (Server()->Tick() % 50 == 0) //hook ihn xD
								//{
								//	//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "bin im bereich");
								//	if (m_Input.m_Hook == 0)
								//	{
								//		Hook();
								//		m_Dummy_wb_hooked = true;
								//		//GameServer()->SendEmoticon(m_pPlayer->GetCid(), 1, -1);
								//		//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "huke");
								//		//schiess delay
								//		if (Server()->Tick() >= m_EmoteTickNext)
								//		{
								//			m_pPlayer->m_LastEmote = Server()->Tick();

								//			//GameServer()->SendEmoticon(m_pPlayer->GetCid(), 7, -1);

								//			Fire();

								//			m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() / 4;
								//		}
								//		else
								//		{
								//			Fire(0);
								//		}
								//	}
								//	else
								//	{
								//		Hook(0);
								//		GameServer()->SendEmoticon(m_pPlayer->GetCid(), 7, -1);
								//		//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "huke nich");

								//	}
								//}
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
		}

		//testy vel

		/*

		if (GetVel().x == 16 * 32)
		{
		GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "speed: 16");
		}

		*/

		//ganz am ende nochmal checken ob man nicht gerade einen spieler wieder aus dem freeze zieht
		//TODO: wenn hier irgendwann eine protection kommt das der spieler nicht ganz an der linken wand sein sollte
		// muss das hier geändert wwwerden

		if(1 < 10)
		{
			CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerLeftFreeze(GetPos(), true, m_pCharacter);
			if(pChr && pChr->IsAlive())
			{
				if(IsFrozen(pChr) && pChr->GetPos().x < 417 * 32 - 30) //wenn ein gegner in der linken wand is
				{
					Hook(0); //hook den da mal wd nich raus
				}
			}
		}

		//das selbe auch rechts

		if(1 < 10)
		{
			CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(GetPos(), true, m_pCharacter);
			if(pChr && pChr->IsAlive())
			{ // heieiie aber natürlich auch hoch genung is das der bot noch wayblocken kann
				if(pChr->m_FreezeTime > 0 && pChr->GetPos().x > 428 * 32 + 40 && pChr->GetPos().y < 211 * 32) //wenn ein gegner in der linken wand is
				{
					Hook(0); //hook den da mal wd nich raus
				}
			}
		}

		//TESTY to prevent bugs
		//wenn kein notfall is und der bot glücklich mit seiner position ist
		//dann sollte er auch nicht springen und somit irgendwie spielern helfen die er gerade hookt

		if(!m_Dummy_emergency && m_Dummy_happy)
		{
			Jump(0);
		}
	}
	else //Change to mode main and reset all
	{
		GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "EROOR!!!!");
		//RestOnChange (zuruecksetzten)
		Hook(0);
		Jump(0);
		StopMoving();
		Fire(0);

		m_Dummy_mode18 = 0;
	}
}
