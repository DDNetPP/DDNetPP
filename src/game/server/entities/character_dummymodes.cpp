// Hardcoded serverside bots madness
// created by yung ChillerDragon xd

#include "character.h"

#include <engine/server/server.h>
#include <engine/shared/config.h>
#include <fstream> //ChillerDragon saving bot move records
#include <game/server/gamecontext.h>
#include <string> //ChillerDragon std::getline

void CCharacter::Fire(bool Fire)
{
	if(Fire)
	{
		m_LatestInput.m_Fire++;
		m_Input.m_Fire++;
	}
	else
	{
		m_LatestInput.m_Fire = 0;
		m_Input.m_Fire = 0;
	}
}

void CCharacter::DummyTick()
{
	if(m_pPlayer->m_IsDummy)
	{
		if((m_pPlayer->m_rainbow_offer != m_pPlayer->m_DummyRainbowOfferAmount) && !m_Rainbow)
		{
			m_Rainbow = true;
			m_pPlayer->m_rainbow_offer = 0;
			m_pPlayer->m_DummyRainbowOfferAmount = m_pPlayer->m_rainbow_offer;
		}
		else if((m_pPlayer->m_rainbow_offer != m_pPlayer->m_DummyRainbowOfferAmount) && m_Rainbow)
		{
			m_Rainbow = false;
			m_pPlayer->m_rainbow_offer = 0;
			m_pPlayer->m_DummyRainbowOfferAmount = m_pPlayer->m_rainbow_offer;
		}

		if(m_pPlayer->m_DummyMode == DUMMYMODE_DEFAULT) // 0
		{
			/********************************
			*           weapons            *
			********************************/
			//SetWeapon(0); // hammer
			//SetWeapon(1); // gun
			//SetWeapon(2); // shotgun
			//SetWeapon(3); // grenade
			//SetWeapon(4); // rifle

			/********************************
			*           aiming             *
			********************************/
			//right and down:
			//m_Input.m_TargetX = 200;
			//m_Input.m_TargetY = 200;
			//m_LatestInput.m_TargetX = 200;
			//m_LatestInput.m_TargetY = 200;

			//left and up:
			//m_Input.m_TargetX = -200;
			//m_Input.m_TargetY = -200;
			//m_LatestInput.m_TargetX = -200;
			//m_LatestInput.m_TargetY = -200;

			//up:
			//m_Input.m_TargetX = 1; // straight up would be 0 but we better avoid using 0 (can cause bugs)
			//m_Input.m_TargetY = -200;
			//m_LatestInput.m_TargetX = 1;
			//m_LatestInput.m_TargetY = -200;

			/********************************
			*           moving             *
			********************************/
			//m_Input.m_Hook = 0; //don't hook
			//m_Input.m_Hook = 1; //hook
			//m_Input.m_Jump = 0; //don't jump
			//m_Input.m_Jump = 1; //jump
			//m_Input.m_Direction = 0; //don't walk
			//m_Input.m_Direction = -1; //walk left
			//m_Input.m_Direction = 1; //walk right

			/********************************
			*           shooting           *
			********************************/
			//m_LatestInput.m_Fire++; //fire!
			//m_Input.m_Fire++; //fire!
			//m_LatestInput.m_Fire = 0; //stop fire
			//m_Input.m_Fire = 0; //stop fire

			//

			/********************************
			*           conditions         *
			********************************/
			if(m_Core.m_Pos.x < 10 * 32)
			{
				//this code gets executed if the bot is in the first 10 left tiles of the map
				//m_Input.m_Direction = 1; //the bot walks to the right until he reaches the x coordinate 10
			}
			else
			{
				//this code gets executed if the bot is at x == 10 or higher
				//m_Input.m_Direction = -1; //the bot would go back to 10 if he is too far
			}
		}
		else if(m_pPlayer->m_DummyMode == 1)
		{
			m_Input.m_TargetX = 2;
			m_Input.m_TargetY = 2;
			m_LatestInput.m_TargetX = 2;
			m_LatestInput.m_TargetY = 2;
			if(Server()->Tick() % 100 == 0)
				SetWeapon(3);
			m_LatestInput.m_Fire++;
			m_Input.m_Fire++;
		}
		else if(m_pPlayer->m_DummyMode == 2)
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			//Selfkills
			if(isFreezed)
			{
				if(Server()->Tick() % 300 == 0)
				{
					//Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}
			}

			char aBuf[256];
			//str_format(aBuf, sizeof(aBuf), "speed:  x: %f y: %f", m_Core.m_Vel.x, m_Core.m_Vel.y);
			//str_format(aBuf, sizeof(aBuf), "target:  x: %d y: %d", m_Input.m_TargetX, m_Input.m_TargetY);
			str_format(aBuf, sizeof(aBuf), "pos.x %.2f pos.y %.2f", m_Core.m_Pos.x, m_Core.m_Pos.y);
			//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

			if(1 == 2) //just for debuggin
			{
				CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
				if(pChr && pChr->IsAlive())
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				}
			}
			else //normal
			{
				float Angle = m_AngleTo;

				if(Server()->Tick() > m_AngleTickStart + m_AngleTickTime)
				{
					if(Server()->Tick() >= m_AngleTickNext)
					{
						m_AngleFrom = m_AngleTo;

						m_AngleTo += (rand() % 360) - 180;
						m_AngleTickTime = Server()->TickSpeed() / 2 + (rand() % (Server()->TickSpeed() / 2));

						m_AngleTickStart = Server()->Tick();
						m_AngleTickNext = m_AngleTickStart + m_AngleTickTime + Server()->TickSpeed() * 2 + Server()->TickSpeed() / 2 * (rand() % 10);
					}
				}
				else
				{
					Angle = m_AngleFrom + (m_AngleTo - m_AngleFrom) * (Server()->Tick() - m_AngleTickStart) / m_AngleTickTime;
				}

				float AngleRad = Angle * pi / 180.f;
				m_Input.m_TargetX = cosf(AngleRad) * 100;
				m_Input.m_TargetY = sinf(AngleRad) * 100;
			}

			if(Server()->Tick() >= m_EmoteTickNext)
			{
				m_pPlayer->m_LastEmote = Server()->Tick();
				int r = rand() % 100;
				GameServer()->SendEmoticon(m_pPlayer->GetCID(), r < 10 ? 5 : r < 55 ? 2 :
                                                                                                      7);

				m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() * 10 + Server()->TickSpeed() * (rand() % 21);
			}
		}
		else if(m_pPlayer->m_DummyMode == -3)
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			CCharacter *pChr = GameServer()->m_World.ClosestCharTypeNotInFreeze(m_Pos, true, this, false);
			if(pChr && pChr->IsAlive())
			{
				m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				if(m_Core.m_Pos.y < pChr->m_Core.m_Pos.y - 60)
				{
					m_Input.m_Hook = 1;
				}

				if(m_Core.m_Pos.x < pChr->m_Core.m_Pos.x - 40)
				{
					m_Input.m_Direction = 1;
				}
				else
				{
					m_Input.m_Direction = -1;
				}

				if(m_Core.m_Pos.x > 411 * 32 && m_Core.m_Pos.x < 420 * 32) //hookbattle left entry
				{
					if(pChr->m_Core.m_Pos.x < m_Core.m_Pos.x + 8 * 32) //8 tiles hookrange
					{
						m_Input.m_Hook = 1;
					}

					if(m_Core.m_HookState == HOOK_GRABBED)
					{
						m_Input.m_Direction = -1;
					}

					//rehook
					if(Server()->Tick() % 15 == 0 && m_Core.m_HookState != HOOK_GRABBED)
					{
						m_Input.m_Hook = 0;
					}
				}

				if(pChr->m_FreezeTime) //go full yolo and ignore all freeze just hook the enemy into freeze (kamikaze style)
				{
					m_Input.m_Hook = 1;

					if(m_Core.m_Pos.x > 446 * 32) //right side of main area and right spawn
					{
						m_Input.m_Direction = 1;
					}
					else if(m_Core.m_Pos.x > 425 * 32 && m_Core.m_Pos.x < 434 * 32) //left side of the base prefer the freeze hole cuz its less kamikaze
					{
						m_Input.m_Direction = 1;
					}
					else
					{
						m_Input.m_Direction = -1;
					}

					if(Server()->Tick() % 9 == 0)
					{
						m_Input.m_Jump = 1;
					}

					if(Server()->Tick() % 15 == 0 && m_Core.m_HookState != HOOK_GRABBED)
					{
						m_Input.m_Hook = 0;
					}
				}
				else //no frozen enemy --> dont run into freeze
				{
					if(m_Core.m_Pos.x > 453 * 32 && m_Core.m_Pos.x < 457 * 32)
					{
						m_Input.m_Direction = -1; //dont run into right entry of main area
					}
				}
			}

			//Care your bot mates c:
			CCharacter *pBot = GameServer()->m_World.ClosestCharTypeDummy(m_Pos, this);
			if(pBot && pBot->IsAlive())
			{
				//chill dont push all at once
				if(m_Core.m_Pos.x < 404 * 32 && m_Core.m_Pos.x > 393 * 32 && pBot->m_Core.m_Pos.x > m_Core.m_Pos.x + 5 && !pBot->isFreezed) //left side of map
				{
					if(pBot->m_Core.m_Pos.x < m_Core.m_Pos.x + 8 * 32) //8 tiles distance
					{
						m_Input.m_Direction = -1;
					}
				}

				//get dj if mates block the fastlane on entering from left side
				if(m_Core.m_Pos.x > 414 * 32 && m_Core.m_Pos.x < 420 * 32 && !IsGrounded())
				{
					if(pBot->m_Core.m_Pos.x > 420 * 32 && pBot->m_Core.m_Pos.x < 423 * 32 + 20 && pBot->m_FreezeTime)
					{
						m_Input.m_Direction = -1;
					}
				}
			}

			if(m_Core.m_Pos.y > 262 * 32 && m_Core.m_Pos.x > 404 * 32 && m_Core.m_Pos.x < 415 * 32 && !IsGrounded()) //Likely to fail in the leftest freeze becken
			{
				m_Input.m_Jump = 1;
				if(m_Input.m_Direction == 0) //never stand still here
				{
					if(m_Core.m_Pos.x > 410 * 32)
					{
						m_Input.m_Direction = 1;
					}
					else
					{
						m_Input.m_Direction = -1;
					}
				}
			}

			//basic map dodigen
			if(m_Core.m_Pos.x < 392 * 32) //dont walk in the freeze wall on the leftest side
			{
				m_Input.m_Direction = 1;
			}

			if(m_Core.m_Pos.y < 257 * 32 && m_Core.m_Vel.y < -4.4f && m_Core.m_Pos.x < 456 * 32) //avoid hitting freeze roof
			{
				m_Input.m_Jump = 1;
				m_Input.m_Hook = 1;
			}

			if(m_Core.m_Pos.x > 428 * 32 && m_Core.m_Pos.x < 437 * 32) //freeze loch im main becken dodgen
			{
				if(m_Core.m_Pos.y > 263 * 32 && !IsGrounded())
				{
					m_Input.m_Jump = 1;
				}

				//position predefines
				if(m_Core.m_Pos.x > 423 * 32)
				{
					m_Input.m_Direction = 1;
				}
				else
				{
					m_Input.m_Direction = -1;
				}

				//velocity ovrrides
				if(m_Core.m_Vel.x > 2.6f)
				{
					m_Input.m_Direction = 1;
				}
				if(m_Core.m_Vel.x < -2.6f)
				{
					m_Input.m_Direction = -1;
				}
			}

			if(m_Core.m_Pos.x > 418 * 32 && m_Core.m_Pos.x < 422 * 32) //passing the freeze on the left side
			{
				if(m_Core.m_Pos.x < 420 * 32)
				{
					if(m_Core.m_Vel.x > 0.6f)
					{
						m_Input.m_Jump = 1;
					}
				}
				else
				{
					if(m_Core.m_Vel.x < -0.6f)
					{
						m_Input.m_Jump = 1;
					}
				}
			}

			if(m_Core.m_Pos.x > 457 * 32) //right spawn --->
			{
				m_Input.m_Direction = -1;
				if(m_Core.m_Pos.x < 470 * 32 && IsGrounded()) //start border jump
				{
					m_Input.m_Jump = 1;
				}
				else if(m_Core.m_Pos.x < 470 * 32 && !IsGrounded() && m_Core.m_Pos.y > 260 * 32)
				{
					m_Input.m_Jump = 1;
				}
			}

			if(m_Core.m_Pos.y < 254 * 32 && m_Core.m_Pos.x < 401 * 32 && m_Core.m_Pos.x > 397 * 32) //left spawn upper right freeze wall
			{
				m_Input.m_Direction = -1;
			}
		}
		else if(m_pPlayer->m_DummyMode == -4) //rifle fng
		{
			int offset_x = g_Config.m_SvDummyMapOffsetX * 32; //offset for ChillBlock5 -667
			int offset_y = g_Config.m_SvDummyMapOffsetY * 32;

			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			//attack enemys
			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
			if(pChr && pChr->IsAlive())
			{
				m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				if(pChr->m_FreezeTime < 1) //alive enemy --> attack
				{
					if(m_Core.m_Pos.x < pChr->m_Core.m_Pos.x)
					{
						m_Input.m_Direction = 1;
					}
					else
					{
						m_Input.m_Direction = -1;
					}

					if(Server()->Tick() % 100 == 0)
					{
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
					}
				}
				else //frozen enemy --> sacarfire
				{
				}
			}

			//don't fall in holes
			if(m_Core.m_Pos.x + offset_x > 90 * 32 && m_Core.m_Pos.x + offset_x < 180 * 32) //map middle (including 3 fall traps)
			{
				if(m_Core.m_Pos.y + offset_y > 73 * 32)
				{
					m_Input.m_Jump = 1;
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hopsa");
				}
			}

			//check for stucking in walls
			if(m_Input.m_Direction != 0 && m_Core.m_Vel.x == 0.0f)
			{
				if(Server()->Tick() % 60 == 0)
				{
					m_Input.m_Jump = 1;
				}
			}
		}
		else if(m_pPlayer->m_DummyMode == -5) //grenade fng
		{
			int offset_x = 0; //offset for ChillBlock5 667 (for rifle)
			int offset_y = 0;

			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			//attack enemys
			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
			if(pChr && pChr->IsAlive())
			{
				m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				if(m_Core.m_Pos.x < pChr->m_Core.m_Pos.x)
				{
					m_Input.m_Direction = 1;
				}
				else
				{
					m_Input.m_Direction = -1;
				}

				if(Server()->Tick() % 100 == 0)
				{
					m_Input.m_Fire++;
					m_LatestInput.m_Fire++;
				}
			}

			//don't fall in holes
			if(m_Core.m_Pos.x + offset_x > 90 * 32 && m_Core.m_Pos.x + offset_x < 180 * 32) //map middle (including 3 fall traps)
			{
				if(m_Core.m_Pos.y + offset_y > 73 * 32)
				{
					m_Input.m_Jump = 1;
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hopsa");
				}
			}

			//check for stucking in walls
			if(m_Input.m_Direction != 0 && m_Core.m_Vel.x == 0.0f)
			{
				if(Server()->Tick() % 60 == 0)
				{
					m_Input.m_Jump = 1;
				}
			}
		}
		else if(m_pPlayer->m_DummyMode == -6) //ChillBlock5 blmapv3 1o1 mode
		{
			//rest dummy
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
			if(pChr && pChr->IsAlive())
			{
				m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				/*************************************************
				*                                                *
				*                A T T A C K                     *
				*                                                *
				**************************************************/

				//swing enemy up
				if(m_Core.m_Pos.y < pChr->m_Pos.y - 20 && !IsGrounded() && !pChr->isFreezed)
				{
					m_Input.m_Hook = 1;
					float dist = distance(pChr->m_Pos, m_Core.m_Pos);
					if(dist < 250.f)
					{
						if(m_Core.m_Pos.x < pChr->m_Pos.x)
						{
							m_Input.m_Direction = -1;
						}
						else
						{
							m_Input.m_Direction = 1;
						}
						if(dist < 80.f) // hammer dist
						{
							if(absolute(pChr->m_Core.m_Vel.x) > 2.6f)
							{
								if(m_FreezeTime == 0)
								{
									m_LatestInput.m_Fire++;
									m_Input.m_Fire++;
								}
							}
						}
					}
				}

				//attack in mid
				if(pChr->m_Pos.x > 393 * 32 - 7 + V3_OFFSET_X && pChr->m_Pos.x < 396 * 32 + 7 + V3_OFFSET_X)
				{
					if(pChr->m_Pos.x < m_Core.m_Pos.x) // bot on the left
					{
						if(pChr->m_Core.m_Vel.x < 0.0f)
						{
							m_Input.m_Hook = 1;
						}
						else
						{
							m_Input.m_Hook = 0;
						}
					}
					else // bot on the right
					{
						if(pChr->m_Core.m_Vel.x < 0.0f)
						{
							m_Input.m_Hook = 0;
						}
						else
						{
							m_Input.m_Hook = 1;
						}
					}

					//if (pChr->m_Pos.y > 77 * 32 - 1 + V3_OFFSET_Y && pChr->IsGrounded() == false && pChr->isFreezed)
					//{
					//	m_Input.m_Hook = 0; //rekt -> let him fall
					//}
					if(pChr->isFreezed)
					{
						m_Input.m_Hook = 0;
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
				if(m_Core.m_Pos.y < 78 * 32 + V3_OFFSET_Y && m_Core.m_Pos.y > 70 * 32 + V3_OFFSET_Y && IsGrounded()) // if bot is in position
				{
					if(pChr->m_Pos.x < 389 * 32 + V3_OFFSET_X || pChr->m_Pos.x > 400 * 32 + V3_OFFSET_X) //enemy on the left side
					{
						if(pChr->m_Pos.y < 76 * 32 + V3_OFFSET_Y && pChr->m_Core.m_Vel.y > 4.2f)
						{
							m_Input.m_Hook = 1;
						}
					}

					if(m_Core.m_HookState == HOOK_FLYING)
					{
						m_Input.m_Hook = 1;
					}
					else if(m_Core.m_HookState == HOOK_GRABBED)
					{
						m_Input.m_Hook = 1;
						//stay strong and walk agianst hook pull
						if(m_Core.m_Pos.x < 392 * 32 + V3_OFFSET_X) //left side
						{
							m_Input.m_Direction = 1;
						}
						else if(m_Core.m_Pos.x > 397 * 32 + V3_OFFSET_X) //right side
						{
							m_Input.m_Direction = -1;
						}
					}
				}

				// attack throw into left freeze wall
				if(m_Core.m_Pos.x < 383 * 32 + V3_OFFSET_X)
				{
					if(pChr->m_Pos.y > m_Core.m_Pos.y + 190)
					{
						m_Input.m_Hook = 1;
					}
					else if(pChr->m_Pos.y < m_Core.m_Pos.y - 190)
					{
						m_Input.m_Hook = 1;
					}
					else
					{
						if(pChr->m_Core.m_Vel.x < -1.6f)
						{
							if(pChr->m_Pos.x < m_Core.m_Pos.x - 7 && pChr->m_Pos.x > m_Core.m_Pos.x - 90) //enemy on the left side
							{
								if(pChr->m_Pos.y < m_Core.m_Pos.y + 90 && pChr->m_Pos.y > m_Core.m_Pos.y - 90)
								{
									if(m_FreezeTime == 0)
									{
										m_LatestInput.m_Fire++;
										m_Input.m_Fire++;
									}
								}
							}
						}
					}
				}

				// attack throw into right freeze wall
				if(m_Core.m_Pos.x > 404 * 32 + V3_OFFSET_X)
				{
					if(pChr->m_Pos.y > m_Core.m_Pos.y + 190)
					{
						m_Input.m_Hook = 1;
					}
					else if(pChr->m_Pos.y < m_Core.m_Pos.y - 190)
					{
						m_Input.m_Hook = 1;
					}
					else
					{
						if(pChr->m_Core.m_Vel.x > 1.6f)
						{
							if(pChr->m_Pos.x > m_Core.m_Pos.x + 7 && pChr->m_Pos.x < m_Core.m_Pos.x + 90) //enemy on the right side
							{
								if(pChr->m_Pos.y > m_Core.m_Pos.y - 90 && pChr->m_Pos.y < m_Core.m_Pos.y + 90)
								{
									if(m_FreezeTime == 0)
									{
										m_LatestInput.m_Fire++;
										m_Input.m_Fire++;
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
				if(pChr->m_Pos.x > m_Core.m_Pos.x + 130)
				{
					//default is gun
					SetWeapon(1);
				}
				else if(pChr->m_Pos.x < m_Core.m_Pos.x - 130)
				{
					//default is gun
					SetWeapon(1);
				}
				else
				{
					//switch to hammer if enemy is near enough
					if(pChr->m_Pos.y > m_Core.m_Pos.y + 130)
					{
						//default is gun
						SetWeapon(1);
					}
					else if(pChr->m_Pos.y < m_Core.m_Pos.y - 130)
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
				if(m_Core.m_Pos.x < 389 * 32 + V3_OFFSET_X && m_Core.m_Pos.y > 79 * 32 + V3_OFFSET_Y && pChr->m_Pos.y > 79 * 32 + V3_OFFSET_Y && pChr->m_Pos.x > 398 * 32 + V3_OFFSET_X && IsGrounded() && pChr->IsGrounded())
				{
					m_Input.m_Jump = 1;
				}
				if(m_Core.m_Pos.x < 389 * 32 + V3_OFFSET_X && pChr->m_Pos.x > 307 * 32 + V3_OFFSET_X && pChr->m_Pos.x > 398 * 32 + V3_OFFSET_X)
				{
					m_Input.m_Direction = 1;
				}

				//important freeze doges leave them last!:

				//freeze prevention mainpart down right
				if(m_Core.m_Pos.x > 397 * 32 + V3_OFFSET_X && m_Core.m_Pos.x < 401 * 32 + V3_OFFSET_X && m_Core.m_Pos.y > 78 * 32 + V3_OFFSET_Y)
				{
					m_Input.m_Jump = 1;
					m_Input.m_Hook = 1;
					if(Server()->Tick() % 20 == 0)
					{
						m_Input.m_Hook = 0;
						m_Input.m_Jump = 0;
					}
					m_Input.m_Direction = 1;
					m_Input.m_TargetX = 200;
					m_Input.m_TargetY = 80;
				}

				//freeze prevention mainpart down left
				if(m_Core.m_Pos.x > 387 * 32 + V3_OFFSET_X && m_Core.m_Pos.x < 391 * 32 + V3_OFFSET_X && m_Core.m_Pos.y > 78 * 32 + V3_OFFSET_Y)
				{
					m_Input.m_Jump = 1;
					m_Input.m_Hook = 1;
					if(Server()->Tick() % 20 == 0)
					{
						m_Input.m_Hook = 0;
						m_Input.m_Jump = 0;
					}
					m_Input.m_Direction = -1;
					m_Input.m_TargetX = -200;
					m_Input.m_TargetY = 80;
				}

				//Freeze prevention top left
				if(m_Core.m_Pos.x < 391 * 32 + V3_OFFSET_X && m_Core.m_Pos.y < 71 * 32 + V3_OFFSET_Y && m_Core.m_Pos.x > 387 * 32 - 10 + V3_OFFSET_X)
				{
					m_Input.m_Direction = -1;
					m_Input.m_Hook = 1;
					if(Server()->Tick() % 20 == 0)
					{
						m_Input.m_Hook = 0;
					}
					m_Input.m_TargetX = -200;
					m_Input.m_TargetY = -87;
					if(m_Core.m_Pos.y > 19 * 32 + 20)
					{
						m_Input.m_TargetX = -200;
						m_Input.m_TargetY = -210;
					}
				}

				//Freeze prevention top right
				if(m_Core.m_Pos.x < 402 * 32 + 10 + V3_OFFSET_X && m_Core.m_Pos.y < 71 * 32 + V3_OFFSET_Y && m_Core.m_Pos.x > 397 * 32 + V3_OFFSET_X)
				{
					m_Input.m_Direction = 1;
					m_Input.m_Hook = 1;
					if(Server()->Tick() % 20 == 0)
					{
						m_Input.m_Hook = 0;
					}
					m_Input.m_TargetX = 200;
					m_Input.m_TargetY = -87;
					if(m_Core.m_Pos.y > 67 * 32 + 20 + V3_OFFSET_Y)
					{
						m_Input.m_TargetX = 200;
						m_Input.m_TargetY = -210;
					}
				}

				//Freeze prevention mid
				if(m_Core.m_Pos.x > 393 * 32 - 7 + V3_OFFSET_X && m_Core.m_Pos.x < 396 * 32 + 7 + V3_OFFSET_X)
				{
					if(m_Core.m_Vel.x < 0.0f)
					{
						m_Input.m_Direction = -1;
					}
					else
					{
						m_Input.m_Direction = 1;
					}

					if(m_Core.m_Pos.y > 77 * 32 - 1 + V3_OFFSET_Y && IsGrounded() == false)
					{
						m_Input.m_Jump = 1;
						if(m_Core.m_Jumped > 2) //no jumps == rip   --> panic hook
						{
							m_Input.m_Hook = 1;
							if(Server()->Tick() % 15 == 0)
							{
								m_Input.m_Hook = 0;
							}
						}
					}
				}

				//Freeze prevention left
				if(m_Core.m_Pos.x < 380 * 32 + V3_OFFSET_X || (m_Core.m_Pos.x < 382 * 32 + V3_OFFSET_X && m_Core.m_Vel.x < -8.4f))
				{
					m_Input.m_Direction = 1;
				}
				//Freeze prevention right
				if(m_Core.m_Pos.x > 408 * 32 + V3_OFFSET_X || (m_Core.m_Pos.x > 406 * 32 + V3_OFFSET_X && m_Core.m_Vel.x > 8.4f))
				{
					m_Input.m_Direction = -1;
				}
			}
		}
		else if(m_pPlayer->m_DummyMode == DUMMYMODE_ADVENTURE) // -7
		{
			m_Input.m_Jump = 0;
			m_Input.m_Fire = 0;
			m_LatestInput.m_Fire = 0;
			// m_Input.m_Hook = 0;
			m_Input.m_Direction = 0;

			if(rand() % 20 > 17)
				m_Input.m_Direction = rand() % 3 - 1;
			if(rand() % 660 > 656)
				m_Input.m_Jump = 1;
			if(rand() % 256 > 244)
				m_Input.m_Hook = rand() % 2;
			if(rand() % 1000 > 988)
			{
				m_Input.m_Fire++;
				m_LatestInput.m_Fire++;
			}

			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this, false);
			if(pChr && pChr->IsAlive())
			{
				static bool IsAimbot = false;
				if(IsAimbot)
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y - 20;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y - 20;

					// don't shoot walls
					if(!GameServer()->Collision()->IntersectLine(m_Pos, pChr->m_Pos, 0x0, 0))
					{
						if(Server()->Tick() % 77 == 0 && m_FreezeTime < 1)
						{
							m_Input.m_Fire++;
							m_LatestInput.m_Fire++;
						}
					}
					IsAimbot = rand() % 50 > 40;
				}
				else // no aimbot
				{
					IsAimbot = rand() % 3000 > 2789;
					if(pChr->m_Pos.x < m_Pos.x)
					{
						m_Input.m_Direction = -1;
					}
					else
					{
						m_Input.m_Direction = 1;
					}
				}
			}
		}
		else if(m_pPlayer->m_DummyMode == 3)
		{
			//rest dummy (zuruecksetzten)
			//m_Input.m_Hook = 0;
			//m_Input.m_Jump = 0;
			//m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			//m_Core.m_ActiveWeapon = WEAPON_HAMMER;

			//happy finish
			if(m_DummyFinished == true)
			{
				if(m_DummyFinishes == 1)
				{
					m_DummyShowRank = true;
					m_Input.m_Jump = 1;
					m_Input.m_Jump = 0;
					m_Input.m_Jump = 1;
					m_Input.m_Jump = 0;
					GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3);
					GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "GoodGame :)! Thank you team, for helping me finish this cool map!");
					m_DummyFinished = false;
				}
				else if(m_DummyFinishes == 2)
				{
					GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Yay! Finished again!");
					m_DummyFinished = false;
				}
				else
				{
					GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "gg :)");
					m_DummyFinished = false;
				}
			}
			//showing rank
			if(m_DummyShowRank == true)
			{
				if(frandom() * 100 < 2)
				{
					GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Look at my nice rank!");
					GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Type /rank");
					m_DummyShowRank = false;
				}
			}

			//weapon switch old
			/*
			if (Server()->Tick() >= m_EmoteTickNext)
			{
			m_pPlayer->m_LastEmote = Server()->Tick();
			//m_Core.m_ActiveWeapon = WEAPON_GUN;
			DoWeaponSwitch();

			m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() / 10 + Server()->TickSpeed() * (rand() % 21);
			}
			else
			{
			m_Core.m_ActiveWeapon = WEAPON_HAMMER;
			}
			*/

			//emotes fast
			if(Server()->Tick() >= m_EmoteTickNext)
			{
				m_pPlayer->m_LastEmote = Server()->Tick();

				int r = rand() % 100;
				GameServer()->SendEmoticon(m_pPlayer->GetCID(), r < 10 ? 5 : r < 55 ? 2 :
                                                                                                      7);

				m_DummyHammer ^= true;

				m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() / 6;
			}

			//weapons
			if(m_FreezeTime == 0)
			{
				if(m_DummyHammer) //bubub
				{
					//m_Core.m_ActiveWeapon = WEAPON_HAMMER;
					SetWeapon(0);
				}
				else
				{
					SetWeapon(1);
					//m_Core.m_ActiveWeapon = WEAPON_GUN;
				}
			}

			// moving eyes
			float Angle = m_AngleTo;

			if(Server()->Tick() > m_AngleTickStart + m_AngleTickTime)
			{
				if(Server()->Tick() >= m_AngleTickNext)
				{
					m_AngleFrom = m_AngleTo;
					m_AngleTo += (rand() % 360) - 180;
					m_AngleTickTime = Server()->TickSpeed() / 15 + (rand() % (Server()->TickSpeed() / 15));
					m_AngleTickStart = Server()->Tick() / 10;
					m_AngleTickNext = m_AngleTickStart + m_AngleTickTime + Server()->TickSpeed() * 2 + Server()->TickSpeed() / 2 * (rand() % 10);
				}
			}
			else
			{
				Angle = m_AngleFrom + (m_AngleTo - m_AngleFrom) * (Server()->Tick() - m_AngleTickStart) / m_AngleTickTime;
			}

			float AngleRad = Angle * pi / 180.f;
			m_Input.m_TargetX = cosf(AngleRad) * 100;
			m_Input.m_TargetY = sinf(AngleRad) * 100;

			if(m_FreezeTime == 0)
			{
				m_LatestInput.m_TargetX = cosf(AngleRad) * 100;
				m_LatestInput.m_TargetY = sinf(AngleRad) * 100;
				m_LatestInput.m_Fire++;
				m_Input.m_Fire++;
				//FireWeapon(true);
				//m_Input.m_Jump = 1;
			}

			// hook
			if(m_Core.m_HookState == HOOK_FLYING)
				m_Input.m_Hook = 1;
			else if(m_Core.m_HookState == HOOK_GRABBED)
			{
				if(frandom() * 250 < 1)
				{
					if(frandom() * 250 == 1)
						m_Input.m_Hook = 0;
					else
						m_Input.m_Hook = 1;
				}
				else
				{
					if(frandom() * 250 < 1)
						m_Input.m_Hook = 0;
					else
						m_Input.m_Hook = 1;
				}
			}
			else
			{
				if(frandom() * 250 < 3)
				{
					m_Input.m_Hook = 1;
				}
				else
					m_Input.m_Hook = 0;
			}

			// walk
			if(m_StopMoveTick && Server()->Tick() >= m_StopMoveTick)
			{
				m_LastMoveDirection = m_Input.m_Direction;
				m_Input.m_Direction = 0;
				m_StopMoveTick = 0;
			}
			if(Server()->Tick() >= m_MoveTick)
			{
				int Direction = rand() % 6;
				if(m_LastMoveDirection == -1)
				{
					if(Direction < 2)
						m_Input.m_Direction = 1;
					else
						m_Input.m_Direction = -1;
				}
				else
				{
					if(Direction < 2)
						m_Input.m_Direction = -1;
					else
						m_Input.m_Direction = 1;
				}
				m_StopMoveTick = Server()->Tick() + Server()->TickSpeed();
				m_MoveTick = Server()->Tick() + Server()->TickSpeed() * 3 + Server()->TickSpeed() * (rand() % 6);
			}
		}
		else if(m_pPlayer->m_DummyMode == 4) // mode 3 + selfkill
		{
			//rest dummy (zuruecksetzten)
			//m_Input.m_Hook = 0;
			//m_Input.m_Jump = 0;
			//m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			//m_Core.m_ActiveWeapon = WEAPON_HAMMER;
			SetWeapon(0);

			//freeze selfkilll

			if(m_DummyFreezed)
			{
				Die(m_pPlayer->GetCID(), WEAPON_SELF);
				m_DummyFreezed = false;
			}

			//weapon switch
			if(Server()->Tick() >= m_EmoteTickNext)
			{
				m_pPlayer->m_LastEmote = Server()->Tick();
				//m_Core.m_ActiveWeapon = WEAPON_GUN;
				//DoWeaponSwitch();

				m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() / 10 + Server()->TickSpeed() * (rand() % 21);
			}
			else
			{
				//m_Core.m_ActiveWeapon = WEAPON_HAMMER;
				SetWeapon(0);
			}

			// emotes

			if(Server()->Tick() >= m_EmoteTickNext)
			{
				m_pPlayer->m_LastEmote = Server()->Tick();
				int r = rand() % 100;
				GameServer()->SendEmoticon(m_pPlayer->GetCID(), r < 10 ? 5 : r < 55 ? 2 :
                                                                                                      7);

				m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() * 5 + Server()->TickSpeed() * (rand() % 21);
			}

			// moving eyes
			float Angle = m_AngleTo;

			if(Server()->Tick() > m_AngleTickStart + m_AngleTickTime)
			{
				if(Server()->Tick() >= m_AngleTickNext)
				{
					m_AngleFrom = m_AngleTo;
					m_AngleTo += (rand() % 360) - 180;
					m_AngleTickTime = Server()->TickSpeed() / 15 + (rand() % (Server()->TickSpeed() / 15));
					m_AngleTickStart = Server()->Tick() / 10;
					m_AngleTickNext = m_AngleTickStart + m_AngleTickTime + Server()->TickSpeed() * 2 + Server()->TickSpeed() / 2 * (rand() % 10);
				}
			}
			else
			{
				Angle = m_AngleFrom + (m_AngleTo - m_AngleFrom) * (Server()->Tick() - m_AngleTickStart) / m_AngleTickTime;
			}

			float AngleRad = Angle * pi / 180.f;
			m_Input.m_TargetX = cosf(AngleRad) * 100;
			m_Input.m_TargetY = sinf(AngleRad) * 100;

			// fire
			if(frandom() * 25 < 3) // probier mal jetzt ob sich was ändert
			{
				m_LatestInput.m_TargetX = cosf(AngleRad) * 100;
				m_LatestInput.m_TargetY = sinf(AngleRad) * 100;
				m_LatestInput.m_Fire++;
				m_Input.m_Fire++;
				//FireWeapon(true);
				//m_Input.m_Jump = 1;
			}

			// hook
			if(m_Core.m_HookState == HOOK_FLYING)
				m_Input.m_Hook = 1;
			else if(m_Core.m_HookState == HOOK_GRABBED)
			{
				if(frandom() * 250 < 1)
				{
					if(frandom() * 250 == 1)
						m_Input.m_Hook = 0;
					else
						m_Input.m_Hook = 1;
				}
				else
				{
					if(frandom() * 250 < 1)
						m_Input.m_Hook = 0;
					else
						m_Input.m_Hook = 1;
				}
			}
			else
			{
				if(frandom() * 250 < 3)
				{
					m_Input.m_Hook = 1;
				}
				else
					m_Input.m_Hook = 0;
			}

			// walk
			if(m_StopMoveTick && Server()->Tick() >= m_StopMoveTick)
			{
				m_LastMoveDirection = m_Input.m_Direction;
				m_Input.m_Direction = 0;
				m_StopMoveTick = 0;
			}
			if(Server()->Tick() >= m_MoveTick)
			{
				int Direction = rand() % 6;
				if(m_LastMoveDirection == -1)
				{
					if(Direction < 2)
						m_Input.m_Direction = 1;
					else
						m_Input.m_Direction = -1;
				}
				else
				{
					if(Direction < 2)
						m_Input.m_Direction = -1;
					else
						m_Input.m_Direction = 1;
				}
				m_StopMoveTick = Server()->Tick() + Server()->TickSpeed();
				m_MoveTick = Server()->Tick() + Server()->TickSpeed() * 3 + Server()->TickSpeed() * (rand() % 6);
			}
		}
		else if(m_pPlayer->m_DummyMode == 9) //move left
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			m_Input.m_Direction = -1;
		}
		else if(m_pPlayer->m_DummyMode == 10) //move right
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			m_Input.m_Direction = 1;
		}
		else if(m_pPlayer->m_DummyMode == 11) //move Jump
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "speed:  x: %f y: %f", m_Core.m_Vel.x, m_Core.m_Vel.y);
			//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			m_Input.m_Jump = 1;
		}
		else if(m_pPlayer->m_DummyMode == 12) //Damage dude
		{
			m_isDmg = true;

			if(Server()->Tick() >= m_EmoteTickNext)
			{
				m_pPlayer->m_LastEmote = Server()->Tick();
				int r = rand() % 100;
				GameServer()->SendEmoticon(m_pPlayer->GetCID(), r < 10 ? 5 : r < 55 ? 2 :
                                                                                                      7);

				m_Input.m_Jump = 1;

				m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() + Server()->TickSpeed() * (rand() % 4);
			}
			else
			{
				m_Input.m_Jump = 0;
			}

			//laufen (move xD)
			if(m_StopMoveTick && Server()->Tick() >= m_StopMoveTick)
			{
				m_LastMoveDirection = m_Input.m_Direction;
				m_Input.m_Direction = 0;
				m_StopMoveTick = 0;
			}
			if(Server()->Tick() >= m_MoveTick)
			{
				int Direction = rand() % 6;
				if(m_LastMoveDirection == -1)
				{
					if(Direction < 2)
						m_Input.m_Direction = 1;
					else
						m_Input.m_Direction = -1;
				}
				else
				{
					if(Direction < 2)
						m_Input.m_Direction = -1;
					else
						m_Input.m_Direction = 1;
				}
				m_StopMoveTick = Server()->Tick() + Server()->TickSpeed() * 10 / Server()->TickSpeed() * (rand() % 60);
				m_MoveTick = Server()->Tick() + Server()->TickSpeed() * 3 + Server()->TickSpeed() * (rand() % 6);
			}
		}
		else if(m_pPlayer->m_DummyMode == 13) // fly bot
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			m_Input.m_TargetX = 0;
			m_Input.m_TargetY = -1;
			m_LatestInput.m_TargetX = 0;
			m_LatestInput.m_TargetY = -1;

			if(m_Core.m_Vel.y > 0)
			{
				m_Input.m_Hook = 1;
			}
			else
			{
				m_Input.m_Hook = 0;
			}
		}
		else if(m_pPlayer->m_DummyMode == 14) // fly bot left
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = -1;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			m_Input.m_TargetX = 0;
			m_Input.m_TargetY = -1;

			if(m_Core.m_Vel.y > 0)
				m_Input.m_Hook = 1;
			else
				m_Input.m_Hook = 0;
		}
		else if(m_pPlayer->m_DummyMode == 15) // fly bot right
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 1;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			m_Input.m_TargetX = 0;
			m_Input.m_TargetY = -1;

			if(m_Core.m_Vel.y > 0)
				m_Input.m_Hook = 1;
			else
				m_Input.m_Hook = 0;
		} //this mode <3 18 reks em all
		else if(m_pPlayer->m_DummyMode == 18) //pathlaufing system (become ze ruler xD) // ich bin der mode um den es hier geht
		{
			m_Input.m_Hook = 0;
			//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "HALLO ICH BIN 18!");

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
			if(m_Core.m_Pos.y > 214 * 32 && m_Core.m_Pos.x > 424 * 32)
			{
				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerWB(m_Pos, true, this);
				if(pChr && pChr->IsAlive())
				{
					//Wenn der bot im tunnel ist und ein Gegner im RulerWB bereich
					m_Dummy_mode18 = 1;
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Wayblocker gesichtet");
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

				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this);
				if(pChr && pChr->IsAlive())
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					//rest on tick
					m_Input.m_Hook = 0;
					m_Input.m_Jump = 0;
					m_Input.m_Direction = 0;
					m_LatestInput.m_Fire = 0;
					m_Input.m_Fire = 0;
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

					if(pChr->m_Core.m_Vel.y > -0.9f && pChr->m_Pos.y > 211 * 32)
					{
						//wenn der gegner am boden liegt starte angriff
						m_Dummy_special_defend_attack = true;

						//start jump
						m_Input.m_Jump = 1;
					}

					if(m_Dummy_special_defend_attack)
					{
						if(m_Core.m_Pos.x - pChr->m_Pos.x < 50) //wenn der gegner nah genung is mach dj
						{
							m_Input.m_Jump = 1;
						}

						if(pChr->m_Pos.x < m_Core.m_Pos.x)
						{
							m_Input.m_Hook = 1;
						}
						else //wenn der gegner weiter rechts als der bot is lass los und übergib an main deine arbeit ist hier getahen
						{ //main mode wird evenetuell noch korrigieren mit schieben
							m_Dummy_special_defend = false;
							m_Dummy_special_defend_attack = false;
						}

						//Der bot sollte möglichst weit nach rechts gehen aber natürlich nicht ins freeze

						if(m_Core.m_Pos.x < 427 * 32 + 15)
						{
							m_Input.m_Direction = 1;
						}
						else
						{
							m_Input.m_Direction = -1;
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
				m_Input.m_Hook = 0;
				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;
				SetWeapon(0);

				//Selfkills (bit random but they work)
				if(isFreezed)
				{
					//wenn der bot freeze is warte erstmal n paar sekunden und dann kill dich
					if(Server()->Tick() % 300 == 0)
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
						m_Dummy_happy = false;
					}
				}

				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler2(m_Pos, true, this);
				if(pChr && pChr->IsAlive())
				{
					//Check ob an notstand mode18 = 0 übergeben
					if(pChr->m_FreezeTime == 0)
					{
						m_Dummy_bored = false;
						m_Dummy_bored_counter = 0;
						m_Dummy_mode18 = 0;
					}

					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					m_Input.m_Jump = 1;

					if(pChr->m_Pos.y > m_Core.m_Pos.y) //solange der bot über dem gegner ist (damit er wenn er ihn weg hammert nicht weiter hookt)
					{
						m_Input.m_Hook = 1;
					}

					if(m_Core.m_Pos.x > 420 * 32)
					{
						m_Input.m_Direction = -1;
					}

					if(pChr->m_Pos.y < m_Core.m_Pos.y + 15)
					{
						m_LatestInput.m_Fire++;
						m_Input.m_Fire++;
					}
				}
				else //lieblings position finden wenn nichts abgeht
				{
					if(m_Core.m_Pos.x < 421 * 32)
					{
						m_Input.m_Direction = 1;
					}
					else if(m_Core.m_Pos.x > 422 * 32 + 30)
					{
						m_Input.m_Direction = -1;
					}
				}
			}
			else if(m_Dummy_mode18 == 1) //attack in tunnel
			{
				//Selfkills (bit random but they work)
				if(isFreezed)
				{
					//wenn der bot freeze is warte erstmal n paar sekunden und dann kill dich
					if(Server()->Tick() % 300 == 0)
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}
				}

				//stay on position

				if(m_Core.m_Pos.x < 426 * 32 + 10) // zu weit links
				{
					m_Input.m_Direction = 1; //geh rechts
				}
				else if(m_Core.m_Pos.x > 428 * 32 - 10) //zu weit rechts
				{
					m_Input.m_Direction = -1; // geh links
				}
				else if(m_Core.m_Pos.x > 428 * 32 + 10) // viel zu weit rechts
				{
					m_Input.m_Direction = -1; // geh links
					m_Input.m_Jump = 1;
				}
				else
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerWB(m_Pos, true, this);
					if(pChr && pChr->IsAlive())
					{
						if(pChr->m_Pos.x < 436 * 32) //wenn er ganz weit über dem freeze auf der kante ist (hooke direkt)
						{
							m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
						}
						else // wenn der Gegner weiter hinter dem unhook ist (hook über den Gegner um ihn trozdem zu treffen und das unhook zu umgehen)
						{
							m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x - 50;
							m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x - 50;
							m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
						}

						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "targX: %d = %.2f - %.2f", m_Input.m_TargetX, pChr->m_Pos.x, m_Pos.x);
						// GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

						// m_Input.m_Hook = 0;
						CCharacter *pChr = GameServer()->m_World.ClosestCharTypeTunnel(m_Pos, true, this);
						if(pChr && pChr->IsAlive())
						{
							// wenn jemand im tunnel is check ob du nicht ausversehen den hookst anstatt des ziels in der WB area
							if(pChr->m_Pos.x < m_Core.m_Pos.x) //hooke nur wenn kein Gegner rechts von dem bot im tunnel is (da er sonst ziemlich wahrscheinlich den hooken würde)
							{
								m_Input.m_Hook = 1;
							}
						}
						else
						{
							// wenn eh keiner im tunnel is hau raus dat ding
							m_Input.m_Hook = 1;
						}

						// schau ob sich der gegner bewegt und der bot grad nicht mehr am angreifen iss dann resette falls er davor halt misshookt hat
						// geht nich -.-
						/*	if (!m_Core.m_HookState == HOOK_FLYING && !m_Core.m_HookState == HOOK_GRABBED)
						{
						if (Server()->Tick() % 10 == 0)
						m_Input.m_Hook = 0;
						}*/

						if(m_Core.m_Vel.x > 3.0f)
						{
							m_Input.m_Direction = -1;
						}
						else
						{
							m_Input.m_Direction = 0;
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
				//	m_Input.m_Hook = 0;
				//}

				//m_Input.m_Hook = 0;
				//if (m_Core.m_HookState == HOOK_FLYING)
				//	m_Input.m_Hook = 1;
				//else if (m_Core.m_HookState == HOOK_GRABBED)
				//	m_Input.m_Hook = 1;
				//else
				//	m_Input.m_Hook = 0;

				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;

				//char aBuf[256];
				//str_format(aBuf, sizeof(aBuf), "speed:  x: %f y: %f speed pChr:  x: %f y: %f", m_Core.m_Vel.x, m_Core.m_Vel.y);

				//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

				//if (1 == 1)
				//{
				//	CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
				//	if (pChr && pChr->IsAlive())
				//	{
				//		char aBuf[256];
				//		str_format(aBuf, sizeof(aBuf), "speed pChr:  x: %f y: %f", pChr->m_Core.m_Vel.x, pChr->m_Core.m_Vel.y);

				//		//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
				//	}
				//}

				//m_pPlayer->m_TeeInfos.m_Name = aBuf;

				if(m_Core.m_Vel.x > 1.0f)
				{
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "speed: schneller als 1");
				}

				//Check ob jemand in der linken freeze wand is

				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerLeftFreeze(m_Pos, true, this); //wenn jemand rechts im freeze liegt
				if(pChr && pChr->IsAlive()) // wenn ein spieler rechts im freeze lebt
				{ //----> versuche im notstand nicht den gegner auch da rein zu hauen da ist ja jetzt voll

					m_Dummy_left_freeze_full = true;
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Da liegt einer im freeze");
				}
				else // wenn da keiner is fülle diesen spot (linke freeze wand im ruler spot)
				{
					m_Dummy_left_freeze_full = false;
				}

				//Selfkill

				if(m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.x > 325 * 32 && m_Core.m_Pos.y > 215 * 32) //Links am spawn runter
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Links am spawn runter");
				}
				//else if ((m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x > 392 * 32 && m_Core.m_Pos.y > 190) || (m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.y > 190)) //freeze decke am spawn
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke am spawn");
				//}
				//else if (m_Core.m_Pos.y > 218 * 32 + 31 /* für tee balance*/ && m_Core.m_Pos.x < 415 * 32) //freeze boden am spawn
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze boden am spawn");
				//}
				else if(m_Core.m_Pos.y < 215 * 32 && m_Core.m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.x < 428 * 32) //freeze decke im tunnel
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke im tunnel");
				}
				else if(m_Core.m_Pos.y > 222 * 32) //freeze becken unter area
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze becken unter area");
				}
				else if(m_Core.m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 436 * 32) //freeze rechts neben freeze becken
				{
					//Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze rechts neben freeze becken");
				}
				else if(m_Core.m_Pos.x > 469 * 32) //zu weit ganz rechts in der ruler area
				{
					//Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "zu weit ganz rechts in der ruler area");
				}
				else if(m_Core.m_Pos.y > 211 * 32 + 34 && m_Core.m_Pos.x > 455 * 32) //alles freeze am boden rechts in der area
				{
					//Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze boden rechts der area");
				}

				if(m_Core.m_Pos.y < 220 * 32 && m_Core.m_Pos.x < 415 * 32 && m_FreezeTime > 1) //always suicide on freeze if not reached teh block area yet
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze und links der block area");
				}

				if(m_Core.m_Pos.x < 388 * 32 && m_Core.m_Pos.y > 213 * 32) //jump to old spawn
				{
					m_Input.m_Jump = 1;
					m_Input.m_Fire++;
					m_LatestInput.m_Fire++;
					m_Input.m_Hook = 1;
					m_Input.m_TargetX = -200;
					m_Input.m_TargetY = 0;
				}

				// Movement
				//CheckFatsOnSpawn

				if(m_Core.m_Pos.x < 406 * 32)
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
					if(pChr && pChr->IsAlive())
					{
						m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
						m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

						m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
						m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

						if(pChr->m_Pos.x < 407 * 32 && pChr->m_Pos.y > 212 * 32 && pChr->m_Pos.y < 215 * 32 && pChr->m_Pos.x > m_Core.m_Pos.x) //wenn ein im weg stehender tee auf der spawn plattform gefunden wurde
						{
							SetWeapon(0); //hol den hammer raus!
							if(pChr->m_Pos.x - m_Core.m_Pos.x < 30) //wenn der typ nahe bei dem bot ist
							{
								if(m_FreezeTick == 0) //nicht rum schrein
								{
									m_LatestInput.m_Fire++;
									m_Input.m_Fire++;
								}

								if(Server()->Tick() % 10 == 0)
								{
									GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9); //angry
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

				if(m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.y > 214 * 32) //wenn bot im tunnel ist
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharTypeTunnel(m_Pos, true, this);
					if(pChr && pChr->IsAlive())
					{
						if(pChr->m_Core.m_Vel.x < 7.8f) //wenn der nächste spieler im tunnel ein slowdude is
						{
							//HauDenBau
							SetWeapon(0); //hol den hammer raus!

							m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							if(m_FreezeTick == 0) //nicht rum schrein
							{
								m_LatestInput.m_Fire++;
								m_Input.m_Fire++;
							}

							if(Server()->Tick() % 10 == 0) //do angry emotes
							{
								GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9);
							}
						}
					}
				}

				//CheckSpeedInTunnel
				if(m_Core.m_Pos.x > 425 * 32 && m_Core.m_Pos.y > 214 * 32 && m_Core.m_Vel.x < 9.4f) //wenn nich genung speed zum wb spot springen
				{
					m_Dummy_get_speed = true;
				}

				if(m_Dummy_get_speed) //wenn schwung holen == true (tunnel)
				{
					if(m_Core.m_Pos.x > 422 * 32) //zu weit rechts
					{
						//---> hol schwung für den jump
						m_Input.m_Direction = -1;
					}
					else //wenn weit genung links
					{
						//dann kann das normale movement von dort aus genung schwung auf bauen
						m_Dummy_get_speed = false;
					}
				}
				else
				{
					if(m_Core.m_Pos.x < 415 * 32) //bis zum tunnel laufen
					{
						m_Input.m_Direction = 1;
					}
					else if(m_Core.m_Pos.x < 440 * 32 && m_Core.m_Pos.y > 213 * 32) //im tunnel laufen
					{
						m_Input.m_Direction = 1;
					}

					//externe if abfrage weil laufen während sprinegn xD
					if(m_Core.m_Pos.x > 413 * 32 && m_Core.m_Pos.x < 415 * 32) // in den tunnel springen
					{
						m_Input.m_Jump = 1;
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
						//m_Input.m_Jump = 0;
					}
					else if(m_Core.m_Pos.x > 428 * 32 - 20 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen
					{
						m_Input.m_Jump = 1;
					}

					// externen springen aufhören für dj
					if(m_Core.m_Pos.x > 428 * 32 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen nicht mehr springen
					{
						m_Input.m_Jump = 0;
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
					}

					//nochmal extern weil springen während springen
					if(m_Core.m_Pos.x > 430 * 32 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen springen
					{
						m_Input.m_Jump = 1;
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
					}

					if(m_Core.m_Pos.x > 431 * 32 && m_Core.m_Pos.y > 213 * 32) //jump refillen für wayblock spot
					{
						m_Input.m_Jump = 0;
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
				if (m_Core.m_Pos.y < 213 * 32 && m_Core.m_Pos.x > (427 * 32) - 20 && m_Core.m_Pos.x < (428 * 32) + 10) //wenn der bot sich an seinem ruler spot befindet
				{
				//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich mag diesen ort :)");

				if (!m_Dummy_wb_hooked && !m_Dummy_emergency && !m_Dummy_pushing && m_Core.m_Vel.x > 0.90f) //wenn der bot sich auf das freeze zubewegt obwohl er nicht selber läuft
				{
				// --> er wird wahrscheinlich gehookt oder anderweitig extern angegriffen
				// --> schutzmaßnahmen treffen

				GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "AAAh ich werde angegriffen");
				m_Input.m_Jump = 1;
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

				if(m_Core.m_Pos.y < 213 * 32)
				{
					//self kill im freeze
					//New Testy selfkill kill if isFreezed and vel 0
					if(!isFreezed || m_Core.m_Vel.x < -0.5f || m_Core.m_Vel.x > 0.5f || m_Core.m_Vel.y != 0.000000f)
					{
						//mach nichts lol brauche nur das else is einfacher
					}
					else
					{
						if(Server()->Tick() % 150 == 0)
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}

					//Old self kill kill if freeze
					//if (m_Core.m_Pos.y < 201 * 32) // decke
					//{
					//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "suicide reason: roof rulerspot");
					//}
					//else if (m_Core.m_Pos.x < 417 * 32 && m_Core.m_Pos.x > 414 * 32 + 17 && isFreezed) //linker streifen xD
					//{
					//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "suicide reason: left wall rulerspot");
					//}

					//because shitty structure (more infos at TODO(1)) check here for enemys attacking from outside of the ruler area

					//Checken Ob ein potentieller Gegner auf der edge unter dem WBbereich ist
					//Falls diesem so sei --> mach den da weg
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "debug");
					if(1 == 1)
					{
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "debug");
						CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerWBBottom(m_Pos, true, this);
						if(pChr && pChr->IsAlive() && !pChr->isFreezed) //wenn jemand da so im bereich lebt und unfreeze ist
						{
							m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							//erkenne gefahr
							// --> treffe gegen maßnahmen

							//lauf rum rand (bereit machen zum hooken)
							if(m_Core.m_Pos.x < 428 * 32 + 6) //wenn zu weit links um in dem winkel zu hooken
							{
								m_Input.m_Direction = 1;
							}
							else if(m_Core.m_Pos.x > 428 * 32 + 28)
							{
								m_Input.m_Direction = -1;
							}

							//hooke
							CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
							if(pChr && pChr->IsAlive())
							{
								//Wenn der nächste spieler unter der wb area ist hook
								//damit er wenn er einen falschen spieler gehookt hat oder sonst wie den nicht hochzieht
								if(pChr->m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 427 * 32 + 3)
								{
									m_Input.m_Hook = 1;
								}
							}
						}
					}

					//TODO(1): keep this structur in mind this makes not much sence
					// the bool m_Dummy_happy is just true if a enemy is in the ruler area because all code below depends on a enemy in ruler area
					// maybe rework this shit
					//
					//                                               --->   Ruler   <---    testy own class just search in ruler area

					CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this); //position anderer spieler mit pikus aimbot abfragen
					if(pChr && pChr->IsAlive())
					{
						//                                         old: 417 * 32
						//                                                      Old: tee<bot      New: tee<pos.x
						if((pChr->m_Pos.y < 213 * 32 && pChr->m_Pos.x > 417 * 32 - 5 /* && pChr->m_Pos.x < m_Core.m_Pos.x*/ && pChr->m_Pos.x < 428 * 32 && m_Core.m_Pos.x < 429 * 32 && m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.y < 213 * 32) || //wenn ein tee weiter links ist als der bot && der bot links vom mittelfreeze im rulerspot steht
							(pChr->m_Pos.y < 213 * 32 && pChr->m_Pos.x > 417 * 32 - 5 /* && pChr->m_Pos.x < m_Core.m_Pos.x*/ && pChr->m_Pos.x < 444 * 32 && m_Core.m_Pos.x < 429 * 32 && m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.y < 213 * 32 && pChr->m_FreezeTime == 0)) //oder der tee auch rechts vom bot ist aber unfreeze
						//wenn dies ist -> notstand links ausrufen und versuchen gegner zu blocken
						{
							//m_Core.m_ActiveWeapon = WEAPON_HAMMER;
							SetWeapon(0);

							//testy sollte eig auch am anfang des modes passen
							//m_Input.m_Direction = 0;

							//if (m_Core.m_HookState == HOOK_FLYING)
							//	m_Input.m_Hook = 1;
							//else if (m_Core.m_HookState == HOOK_GRABBED)
							//	m_Input.m_Hook = 1;
							//else
							//	m_Input.m_Hook = 0;

							char aBuf[256];
							str_format(aBuf, sizeof(aBuf), "hookstate: %x", m_Input.m_Hook);
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
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "boing!");
									m_Input.m_Jump = 1;
									m_Dummy_jumped = true;
								}
								else
								{
									m_Input.m_Jump = 0;
								}

								if(!m_Dummy_hooked)
								{
									if(Server()->Tick() % 30 == 0)
										m_Dummy_hook_delay = true;

									//testy removed hook here i dont know why but all works pretty good still xD
									if(m_Dummy_hook_delay)
										//m_Input.m_Hook = 1;

										if(Server()->Tick() % 200 == 0)
										{
											m_Dummy_hooked = true;
											m_Input.m_Hook = 0;
										}
								}

								if(!m_Dummy_moved_left)
								{
									if(m_Core.m_Pos.x > 419 * 32 + 20)
										m_Input.m_Direction = -1;
									else
										m_Input.m_Direction = 1;

									if(Server()->Tick() % 200 == 0)
									{
										m_Dummy_moved_left = true;
										m_Input.m_Direction = 0;
									}
								}
							}

							//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "NOTSTAND");

							if(Server()->Tick() % 30 == 0) //angry emotes machen
							{
								GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9);
							}

							CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this); //aimbot + hammerspam
							if(pChr && pChr->IsAlive())
							{
								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								//schiess delay
								if(Server()->Tick() >= m_EmoteTickNext)
								{
									m_pPlayer->m_LastEmote = Server()->Tick();

									//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);

									if(m_FreezeTick == 0) //nicht rum schrein
									{
										m_LatestInput.m_Fire++;
										m_Input.m_Fire++;
									}

									m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() / 4;
								}
							}

							//Blocke gefreezte gegner für immer

							//TODO:
							//das is ein linke seite block wenn hier voll is sollte man anders vorgehen
							//                           früher war es y > 210   aber change weil er während er ihn hochzieht dann nicht mehr das hooken aufhört
							if(pChr->m_FreezeTime > 0 && pChr->m_Pos.y > 204 * 32 && pChr->m_Pos.x > 422 * 32) //wenn ein gegner weit genung rechts freeze am boden liegt
							{
								// soll der bot sich einer position links des spielers nähern
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hab n opfer gefunden");

								if(m_Core.m_Pos.x + (5 * 32 + 40) < pChr->m_Pos.x) // er versucht 5 tiles links des gefreezten gegner zu kommen
								{
									m_Input.m_Direction = -1;

									if(m_Core.m_Pos.x > pChr->m_Pos.x && m_Core.m_Pos.x < pChr->m_Pos.x + (4 * 32)) // wenn er 4 tiles rechts des gefreezten gegners is
									{
										m_Input.m_Jump = 1;
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "boing!");
									}
								}
								else //wenn der bot links des gefreezten spielers is
								{
									m_Input.m_Jump = 1;
									//echo jump
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "boing!");

									m_Input.m_Direction = -1;

									if(m_Core.m_Pos.x < pChr->m_Pos.x) //solange der bot weiter links is
									{
										m_Input.m_Hook = 1;
									}
									else
									{
										m_Input.m_Hook = 0;
									}
								}
							}

							// freeze protection & schieberrei (pushing)
							//                                                                                                                                                                                                      old (417 * 32 - 60)
							if((pChr->m_Pos.x + 10 < m_Core.m_Pos.x && pChr->m_Pos.y > 211 * 32 && pChr->m_Pos.x < 418 * 32) || (pChr->m_FreezeTime > 0 && pChr->m_Pos.y > 210 * 32 && pChr->m_Pos.x < m_Core.m_Pos.x && pChr->m_Pos.x > 417 * 32 - 60)) // wenn der spieler neben der linken wand linken freeze wand liegt schiebt ihn der bot rein
							{ // oder wenn der spieler weiter weg liegt aber freeze is

								if(!m_Dummy_left_freeze_full) //wenn da niemand is schieb den rein
								{
									// HIER TESTY TESTY CHANGES  211 * 32 + 40 stand hier
									if(pChr->m_Pos.y > 211 * 32 + 40) // wenn der gegner wirklich ganz tief genung is
									{ //                          417 * 32 - 40
										if(m_Core.m_Pos.x > 418 * 32) // aber nicht selber ins freeze laufen
										{
											m_Input.m_Direction = -1;

											//Check ob der gegener freeze is

											if(pChr->m_FreezeTime > 0)
											{
												m_LatestInput.m_Fire = 0; //nicht schiessen ofc xD (doch is schon besser xD)
												m_Input.m_Fire = 0;

												//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
											}

											//letzten stupser geben (sonst gibs bugs kb zu fixen)
											if(pChr->isFreezed) //wenn er schon im freeze is
											{
												m_LatestInput.m_Fire = 1; //hau ihn an die wand
												m_Input.m_Fire = 1;
											}
										}
										else
										{
											m_Input.m_Direction = 1;
											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 1");
											if(pChr->m_FreezeTime > 0)
											{
												m_LatestInput.m_Fire = 0; //nicht schiessen ofc xD (doch is schon besser xD)
												m_Input.m_Fire = 0;

												//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
											}
											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "ich halte das auf.");
											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich will da nich rein laufen");
										}
									}
									else //wenn der gegner nicht tief genung ist
									{
										m_Input.m_Direction = 1;

										if(pChr->m_FreezeTime > 0)
										{
											m_LatestInput.m_Fire = 0; //nicht schiessen ofc xD (doch is schon besser xD)
											m_Input.m_Fire = 0;

											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
										}
									}
								}
								else //wenn da schon jemand liegt
								{
									// sag das mal xD
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "da liegt schon einer");
								}
							}
							else if(m_Core.m_Pos.x < 419 * 32 + 10) //sonst mehr abstand halten
							{
								m_Input.m_Direction = 1;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 2");
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich will da nich rein laufen");
							}
							// else // wenn nichts los is erstmal stehen bleiben

							//freeze protection decke mit double jump wenn hammer

							if(m_Core.m_Vel.y < 20.0f && m_Core.m_Pos.y < 207 * 32) // wenn der tee nach oben gehammert wird
							{
								if(m_Core.m_Pos.y > 206 * 32) //ab 206 würd er so oder so ins freeze springen
									m_Input.m_Jump = 1;

								if(m_Core.m_Pos.y < pChr->m_Pos.y) //wenn der bot über dem spieler is soll er hooken
									m_Input.m_Hook = 1;
								else
									m_Input.m_Hook = 0;
							}

							//wenn der tee hcoh geschleudert wird
							//                 old 4 (macht aber im postiven bereich kein sinn aber hat geklappt)
							//                 HALLO HIER IST DEIN ICH AUS DER ZUKUNFT: du dummes kind wenn er in der luft hammert dann fliegt er doch nicht nach oben und gerade da musst du es ja perfekt haben ... low
							//if (m_Core.m_Vel.y < 4.0f && m_Core.m_Pos.y < pChr->m_Pos.y) //wenn er schneller als 4 nach oben fliegt und höher als der Gegener ist
							// lass das mit speed weg und mach lieber was mit höhe
							if(m_Core.m_Pos.y < 207 * 32 && m_Core.m_Pos.y < pChr->m_Pos.y)
							{
								//in hammer position bewegen
								if(m_Core.m_Pos.x > 418 * 32 + 20) //nicht ins freeze laufen
								{
									if(m_Core.m_Pos.x > pChr->m_Pos.x - 45) //zu weit rechts von hammer position
									{
										m_Input.m_Direction = -1; //gehe links
											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich will da nich rein laufen");
									}
									else if(m_Core.m_Pos.x < pChr->m_Pos.x - 39) // zu weit links von hammer position
									{
										m_Input.m_Direction = 1;
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 3");
									}
									else //im hammer bereich
									{
										m_Input.m_Direction = 0; //bleib da
									}
								}
							}

							//Check ob der gegener freeze is

							if(pChr->m_FreezeTime > 0 && pChr->m_Pos.y > 208 * 32 && !pChr->isFreezed) //wenn der Gegner tief und freeze is macht es wenig sinn den frei zu hammern
							{
								m_LatestInput.m_Fire = 0; //nicht schiessen
								m_Input.m_Fire = 0;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
							}

							//Hau den weg (wie dummymode 21)
							if(pChr->m_Pos.x > 418 * 32 && pChr->m_Pos.y > 209 * 32) //das ganze findet nur im bereich statt wo sonst eh nichts passiert
							{
								//wenn der bot den gegner nicht boosten würde hammer den auch nich weg
								m_LatestInput.m_Fire = 0;
								m_Input.m_Fire = 0;

								if(pChr->m_Core.m_Vel.y < -0.5f && m_Core.m_Pos.y + 15 > pChr->m_Pos.y) //wenn der dude speed nach oben hat
								{
									m_Input.m_Jump = 1;
									if(m_FreezeTime == 0)
									{
										m_LatestInput.m_Fire++;
										m_Input.m_Fire++;
									}
								}
							}

							//TODO: FIX:
							//der bot unfreezed den gegner ab einer gewissen höhe wenn er rein gehammert wird schau das da was passiert

							//wenn ein tee freeze links neben dem bot liegt werf den einfach wieder ins freeze becken
							//das is bisher ja noch die einzige sicherheits lücke beim wayblocken
							//wenn man ein tee über den bot hammert

							if(pChr->m_Pos.x > 421 * 32 && pChr->m_FreezeTick > 0 && pChr->m_Pos.x < m_Core.m_Pos.x)
							{
								m_Input.m_Jump = 1;
								m_Input.m_Hook = 1;
							}

							//m_pPlayer->m_TeeInfos.m_ColorBody = (0 * 255 / 360);
							//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Enemy in ruler spot found!");
						}
						else //sonst normal wayblocken und 427 aufsuchen
						{
							//m_Core.m_ActiveWeapon = WEAPON_GUN;
							SetWeapon(1);
							m_Input.m_Jump = 0;

							if(m_Core.m_HookState == HOOK_FLYING)
								m_Input.m_Hook = 1;
							else if(m_Core.m_HookState == HOOK_GRABBED)
								m_Input.m_Hook = 1;
							else
								m_Input.m_Hook = 0;

							//m_pPlayer->m_TeeInfos.m_ColorBody = (120 * 255 / 360);
							//positions check and correction 427

							m_Dummy_jumped = false;
							m_Dummy_hooked = false;
							m_Dummy_moved_left = false;

							if(m_Core.m_Pos.x > 428 * 32 + 15 && m_Dummy_ruled) //wenn viel zu weit ausserhalb der ruler area wo der bot nur hingehookt werden kann
							{
								m_Input.m_Jump = 1;
								m_Input.m_Hook = 1;
							}

							//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Prüfe ob zu weit rechts");
							if(m_Core.m_Pos.x < (427 * 32) - 20) // zu weit links -> geh rechts
							{
								m_Input.m_Direction = 1;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 4");
							}
							else if(m_Core.m_Pos.x > (428 * 32) + 10) // zu weit rechts -> geh links
							{
								m_Input.m_Direction = -1;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich bin zuweit rechts...");
							}
							else // im toleranz bereich -> stehen bleiben
							{
								m_Dummy_happy = true;
								m_Dummy_ruled = true;
								m_Input.m_Direction = 0;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "toleranz bereich");
								//m_Input.m_LatestTargetX = 0;
								//m_Input.m_LatestTargetY = 0;

								//stuff im toleranz bereich doon

								// normal wayblock
								CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this); //position anderer spieler mit pikus aimbot abfragen
								if(pChr && pChr->IsAlive())
								{
									//Check ob jemand special angeflogen kommt dann mode18 = 3 starten
									//Check ob special_defend aktiviert werden sollte
									if(pChr->m_Pos.x < 431 * 32 && pChr->m_Core.m_Vel.y < -12.5f && pChr->m_Core.m_Vel.x < -7.4f)
									{
										m_Dummy_special_defend = true;
									}

									//debuggn special_defend
									//char aBuf[256];
									//str_format(aBuf, sizeof(aBuf), "speed pChr:  x: %f y: %f", pChr->m_Core.m_Vel.x, pChr->m_Core.m_Vel.y);
									//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

									//m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
									//m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

									//m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
									//m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

									//m_Input.m_TargetX = 1;//pChr->m_Pos.x - m_Pos.x; //1
									//m_Input.m_TargetY = 0;//pChr->m_Pos.y - m_Pos.y; //0

									//m_LatestInput.m_TargetX = 1;//pChr->m_Pos.x - m_Pos.x;
									//m_LatestInput.m_TargetY = 0;//pChr->m_Pos.y - m_Pos.y;

									if(pChr->m_Pos.y < 213 * 32 + 10 && pChr->m_Pos.x < 430 * 32 && pChr->m_Pos.y > 210 * 32 && pChr->m_Pos.x > 416 * 32 + 32) // wenn ein spieler auf der linken seite in der ruler area is
									{
										//wenn der typ über dem freze irgendwo rum fliegt

										if(pChr->m_Pos.y < 212 * 32 - 10) //wenn er jedoch zu hoch zum schieben ist
										{
											//mach dich bereit zu schieben und geh nach links (aufziehen)
											m_Input.m_Direction = -1;
										}
										else //wenn er tief genung zum schieben ist
										{
											if(m_Core.m_Pos.x < 428 * 32 + 10) //bei (429 * 32) gibts voll jiggle xD
											{
												m_Input.m_Direction = 1; //schieb ihn runter
												m_Dummy_pushing = true;
											}
											else
											{
												m_Input.m_Direction = 0; // aber nicht zu weit
													//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "ja ich bin das!");
											}
										}
									}
									else // wenn spieler irgendwo anders is
									{
										//CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this);  //wenn jemand oben is
										//if (pChr && pChr->IsAlive())
										//{
										//		m_Input.m_Hook = 1;
										//		m_Dummy_wb_hooked = true;
										//		GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);

										//}

										if(!m_Dummy_emergency) //wayblock stuff hook ja nein pipapo nicht im notfall
										{
											CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerWB(m_Pos, true, this); //wenn jemand oben is (nur im wb bereich)
											if(pChr && pChr->IsAlive())
											{ // und er nicht zu tief ist (das is um unnötiges festgehalte zu verhindern) (bringt eh nichts weil das hier dann nicht mehr aufgerufen wird weil der dann nicht mehr in ClosesestCharTypeRulerWB ist -.-)
												m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
												m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

												m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
												m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
												//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 8);
												//                             noch eine vel abfrage weil der bot sonst daneben hookt
												if(pChr->m_Pos.y > 211 * 32 && pChr->m_Core.m_Vel.y > -1.0f && pChr->m_Core.m_Vel.y < 2.0f && pChr->m_Pos.x > 435 * 32 /*&& pChr->m_Pos.y < 213 * 32*/) //wenn er nich zu schnell nach oben fliegt und zu weit oben ist
												{
													//m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x; //1
													//m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y; //0

													m_Input.m_Hook = 0;
													m_Input.m_Hook = 1;
													m_Dummy_wb_hooked = true;
													if(Server()->Tick() % 30 == 0) //nicht zu hart spammen wenn iwas abgeht
													{
														GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
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
													GameServer()->SendEmoticon(m_pPlayer->GetCID(), 15);
												m_Input.m_Hook = 0;
												//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3);
											}
										}
										//unnötiges festgehalte unten verhindern
										if(!m_Dummy_emergency) //auch hier wieder nur wenn kein notfall is
										{
											CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
											if(pChr && pChr->IsAlive())
											{
												if(pChr->m_Pos.y > 213 * 32 - 5 && m_Core.m_HookState == HOOK_GRABBED && pChr->m_Pos.y < 213 * 32 + 5)
												{
													//Wenn folgendes:
													// kein notstand
													// der bot wenn am haken hat
													// der nächste spieler aber unter der ruler area ist (genauer gesagt gerade im freeze eingetaucht)
													// --> vermute ich mal das er genau diesen spieler hookt
													// --> den los lassen da dieser sowieso keien gefahr mehr ist damit ein neuer gegner schneller geblockt werden kann
													if(Server()->Tick() % 10 == 0) //nicht zu hart spammen wenn iwas abgeht
														GameServer()->SendEmoticon(m_pPlayer->GetCID(), 5);

													m_Input.m_Hook = 0;
												}
											}
										}

										//if (Server()->Tick() % 50 == 0) //hook ihn xD
										//{
										//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "bin im bereich");
										//	if (m_Input.m_Hook == 0)
										//	{
										//		m_Input.m_Hook = 1;
										//		m_Dummy_wb_hooked = true;
										//		//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
										//		//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "huke");
										//		//schiess delay
										//		if (Server()->Tick() >= m_EmoteTickNext)
										//		{
										//			m_pPlayer->m_LastEmote = Server()->Tick();

										//			//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);

										//			m_LatestInput.m_Fire++;
										//			m_Input.m_Fire++;

										//			m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() / 4;
										//		}
										//		else
										//		{
										//			m_LatestInput.m_Fire = 0;
										//			m_Input.m_Fire = 0;
										//		}
										//	}
										//	else
										//	{
										//		m_Input.m_Hook = 0;
										//		GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
										//		//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "huke nich");

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

						if(m_Core.m_Pos.x < (427 * 32) - 20) // zu weit links -> geh rechts
						{
							m_Input.m_Direction = 1;
							//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: special");
						}
						else if(m_Core.m_Pos.x > (427 * 32) + 40) // zu weit rechts -> geh links
						{
							m_Input.m_Direction = -1;
							//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich bin zuweit rechts...");
						}
						//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
					}

					// über das freeze springen wenn rechts der bevorzugenten camp stelle

					if(m_Core.m_Pos.x > 434 * 32)
					{
						m_Input.m_Jump = 1;
					}
					else if(m_Core.m_Pos.x > (434 * 32) - 20 && m_Core.m_Pos.x < (434 * 32) + 20) // bei flug über freeze jump wd holen
					{
						m_Input.m_Jump = 0;
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "refilling jump");
					}
				}

				//testy vel

				/*

				if (m_Core.m_Vel.x == 16 * 32)
				{
				GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "speed: 16");
				}

				*/

				//ganz am ende nochmal checken ob man nicht gerade einen spieler wieder aus dem freeze zieht
				//TODO: wenn hier irgendwann eine protection kommt das der spieler nicht ganz an der linken wand sein sollte
				// muss das hier geändert wwwerden

				if(1 < 10)
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerLeftFreeze(m_Pos, true, this);
					if(pChr && pChr->IsAlive())
					{
						if(pChr->isFreezed && pChr->m_Pos.x < 417 * 32 - 30) //wenn ein gegner in der linken wand is
						{
							m_Input.m_Hook = 0; //hook den da mal wd nich raus
						}
					}
				}

				//das selbe auch rechts

				if(1 < 10)
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this);
					if(pChr && pChr->IsAlive())
					{ // heieiie aber natürlich auch hoch genung is das der bot noch wayblocken kann
						if(pChr->m_FreezeTime > 0 && pChr->m_Pos.x > 428 * 32 + 40 && pChr->m_Pos.y < 211 * 32) //wenn ein gegner in der linken wand is
						{
							m_Input.m_Hook = 0; //hook den da mal wd nich raus
						}
					}
				}

				//TESTY to prevent bugs
				//wenn kein notfall is und der bot glücklich mit seiner position ist
				//dann sollte er auch nicht springen und somit irgendwie spielern helfen die er gerade hookt

				if(!m_Dummy_emergency && m_Dummy_happy)
				{
					m_Input.m_Jump = 0;
				}
			}
			else //Change to mode main and reset all
			{
				GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "EROOR!!!!");
				//RestOnChange (zuruecksetzten)
				m_Input.m_Hook = 0;
				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;

				m_Dummy_mode18 = 0;
			}
		}
		else if(m_pPlayer->m_DummyMode == 23) //Race mode ChillBlock5 with humans
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;
			// m_pPlayer->m_TeeInfos.m_ColorBody = (0 * 255 / 360); //remove this if u ever want to debug agian xd

			/*
			Dummy23modes:
			0				Classic Main race mode.
			1				Tricky mode with tricky hammerfly and sensless harder hammerhit xD. (used for "Drag*" to fuck him in the race lol)
			2				ChillerDragon's mode just speedhammerfly.

			*/

			if(Server()->Tick() % 50 == 0)
			{
				m_Dummy_mode23 = 0;
				CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
				if(pChr && pChr->IsAlive())
				{
					if(
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Starkiller") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "rqza") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "timakro") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Nudelsaft c:") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Destom") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Ante") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Ama") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Forris") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Aoe") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Spyker") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Waschlappen") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), ".:Mivv") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "nealson T'nP") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "ChillerDragon") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "ChillerDragon.*") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Gwendal") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Blue") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Amol") ||
						//!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "fokkonaut") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "pro"))
					{
						m_Dummy_mode23 = 2;
					}
					if(!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Drag*"))
					{
						m_Dummy_mode23 = 1;
					}
				}
			}

			if(m_Core.m_Pos.x > 241 * 32 && m_Core.m_Pos.x < 418 * 32 && m_Core.m_Pos.y > 121 * 32 && m_Core.m_Pos.y < 192 * 32) //new spawn ChillBlock5 (tourna edition (the on with the gores stuff))
			{
				//dieser code wird also nur ausgeführt wenn der bot gerade im neuen bereich ist
				if(m_Core.m_Pos.x > 319 * 32 && m_Core.m_Pos.y < 161 * 32) //top right spawn
				{
					//look up left
					if(m_Core.m_Pos.x < 372 * 32 && m_Core.m_Vel.y > 3.1f)
					{
						m_Input.m_TargetX = -30;
						m_Input.m_TargetY = -80;
					}
					else
					{
						m_Input.m_TargetX = -100;
						m_Input.m_TargetY = -80;
					}

					if(m_Core.m_Pos.x > 331 * 32 && isFreezed)
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}

					if(m_Core.m_Pos.x < 327 * 32) //dont klatsch in ze wand
					{
						m_Input.m_Direction = 1; //nach rechts laufen
					}
					else
					{
						m_Input.m_Direction = -1;
					}

					if(IsGrounded() && m_Core.m_Pos.x < 408 * 32) //initial jump from spawnplatform
					{
						m_Input.m_Jump = 1;
					}

					if(m_Core.m_Pos.x > 330 * 32) //only hook in tunnel and let fall at the end
					{
						if(m_Core.m_Pos.y > 147 * 32 || (m_Core.m_Pos.y > 143 * 32 && m_Core.m_Vel.y > 3.3f)) //gores pro hook up
						{
							m_Input.m_Hook = 1;
						}
						else if(m_Core.m_Pos.y < 143 * 32 && m_Core.m_Pos.x < 372 * 32) //hook down (if too high and in tunnel)
						{
							m_Input.m_TargetX = -42;
							m_Input.m_TargetY = 100;
							m_Input.m_Hook = 1;
						}
					}
				}
				else if(m_Core.m_Pos.x < 317 * 32) //top left spawn
				{
					if(m_Core.m_Pos.y < 158 * 32) //spawn area find down
					{
						//selfkill
						if(isFreezed)
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}

						if(m_Core.m_Pos.x < 276 * 32 + 20) //is die mitte von beiden linken spawns also da wo es runter geht
						{
							m_Input.m_TargetX = 57;
							m_Input.m_TargetY = -100;
							m_Input.m_Direction = 1;
						}
						else
						{
							m_Input.m_TargetX = -57;
							m_Input.m_TargetY = -100;
							m_Input.m_Direction = -1;
						}

						if(m_Core.m_Pos.y > 147 * 32)
						{
							//dbg_msg("fok","will hooken");
							m_Input.m_Hook = 1;
							if(m_Core.m_Pos.x > 272 * 32 && m_Core.m_Pos.x < 279 * 32) //let fall in the hole
							{
								//dbg_msg("fok", "lass ma des");
								m_Input.m_Hook = 0;
								m_Input.m_TargetX = 2;
								m_Input.m_TargetY = 100;
							}
						}
					}
					else if(m_Core.m_Pos.y > 162 * 32) //managed it to go down --> go left
					{
						//selfkill
						if(isFreezed)
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}

						if(m_Core.m_Pos.x < 283 * 32)
						{
							m_Input.m_TargetX = 200;
							m_Input.m_TargetY = -136;
							if(m_Core.m_Pos.y > 164 * 32 + 10)
							{
								m_Input.m_Hook = 1;
							}
						}
						else
						{
							m_Input.m_TargetX = 80;
							m_Input.m_TargetY = -100;
							if(m_Core.m_Pos.y > 171 * 32 - 10)
							{
								m_Input.m_Hook = 1;
							}
						}

						m_Input.m_Direction = 1;
					}
					else //freeze unfreeze bridge only 2 tiles do nothing here
					{
					}
				}
				else //lower end area of new spawn --> entering old spawn by walling and walking right
				{
					m_Input.m_Direction = 1;
					m_Input.m_TargetX = 200;
					m_Input.m_TargetY = -84;

					//Selfkills
					if(isFreezed && IsGrounded()) //should never lie in freeze at the ground
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}

					if(m_Core.m_Pos.y < 166 * 32 - 20)
					{
						m_Input.m_Hook = 1;
					}

					if(m_Core.m_Pos.x > 332 * 32 && m_Core.m_Pos.x < 337 * 32 && m_Core.m_Pos.y > 182 * 32) //wont hit the wall --> jump
					{
						m_Input.m_Jump = 1;
					}

					if(m_Core.m_Pos.x > 336 * 32 + 20 && m_Core.m_Pos.y > 180 * 32) //stop moving if walled
					{
						m_Input.m_Direction = 0;
					}
				}
			}
			else if(false && m_Core.m_Pos.y < 193 * 32 && m_Core.m_Pos.x < 450 * 32 /*&& g_Config.m_SvChillBlock5Version == 1*/) //new spawn
			{
				m_Input.m_TargetX = 200;
				m_Input.m_TargetY = -80;

				//not falling in freeze is bad
				if(m_Core.m_Vel.y < 0.01f && m_FreezeTime > 0)
				{
					if(Server()->Tick() % 40 == 0)
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}
				}
				if(m_Core.m_Pos.y > 116 * 32 && m_Core.m_Pos.x > 394 * 32)
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}

				if(m_Core.m_Pos.x > 364 * 32 && m_Core.m_Pos.y < 126 * 32 && m_Core.m_Pos.y > 122 * 32 + 10)
				{
					if(m_Core.m_Vel.y > -1.0f)
					{
						m_Input.m_Hook = 1;
					}
				}

				if(m_Core.m_Pos.y < 121 * 32 && m_Core.m_Pos.x > 369 * 32)
				{
					m_Input.m_Direction = -1;
				}
				else
				{
					m_Input.m_Direction = 1;
				}
				if(m_Core.m_Pos.y < 109 * 32 && m_Core.m_Pos.x > 377 * 32 && m_Core.m_Pos.x < 386 * 32)
				{
					m_Input.m_Direction = 1;
				}

				if(m_Core.m_Pos.y > 128 * 32)
				{
					m_Input.m_Jump = 1;
				}

				//speeddown at end to avoid selfkill cuz to slow falling in freeze
				if(m_Core.m_Pos.x > 384 * 32 && m_Core.m_Pos.y > 121 * 32)
				{
					m_Input.m_TargetX = 200;
					m_Input.m_TargetY = 300;
					m_Input.m_Hook = 1;
				}
			}
			else //under 193 (above 193 is new spawn)
			{
				//Selfkill

				//Checken ob der bot far im race ist
				if(m_Dummy_collected_weapons && m_Core.m_Pos.x > 470 * 32 && m_Core.m_Pos.y < 200 * 32)
				{
					//TODO:
					//schau wie weit der bot is wenn er weiter is als der ClosestCharTypeFarInRace bereich is schau das du rechtzeitig n anderen triggerst
					//wie zumbeispiel ClosestCharTypeFinish und das wird getriggert wenn der bot rechts des 2p parts is oder so

					CCharacter *pChr = GameServer()->m_World.ClosestCharTypeFarInRace(m_Pos, true, this);
					if(pChr && pChr->IsAlive())
					{
					}
					else
					{
						if(!isFreezed || m_Core.m_Vel.x < -0.5f || m_Core.m_Vel.x > 0.5f || m_Core.m_Vel.y != 0.000000f)
						{
							//mach nichts lol brauche nur das else is einfacher
						}
						else
						{
							if(Server()->Tick() % 370 == 0)
								Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}
					}
				}
				else //sonst normal relativ schnell killen
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
					if(pChr && pChr->IsAlive())
					{
						if(!isFreezed || m_Core.m_Vel.x < -0.5f || m_Core.m_Vel.x > 0.5f || m_Core.m_Vel.y != 0.000000f)
						{
							//mach nichts lol brauche nur das else is einfacher
						}
						else
						{
							if(Server()->Tick() % 270 == 0)
								Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}
					}
					else
					{
						if(isFreezed && m_Core.m_Vel.y == 0.000000f && m_Core.m_Vel.x < 0.1f && m_Core.m_Vel.x > -0.1f)
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}
					}
				}

				//instant self kills
				if(m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.x > 325 * 32 && m_Core.m_Pos.y > 215 * 32) //Links am spawn runter
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Links am spawn runter");
				}
				//else if ((m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x > 392 * 32 && m_Core.m_Pos.y > 190) || (m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.y > 190)) //freeze decke am spawn
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke am spawn");
				//}
				//else if (m_Core.m_Pos.y > 218 * 32 + 31 /* für tee balance*/ && m_Core.m_Pos.x < 415 * 32) //freeze boden am spawn
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze boden am spawn");
				//}
				else if(m_Core.m_Pos.y < 215 * 32 && m_Core.m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.x < 428 * 32) //freeze decke im tunnel
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke im tunnel");
				}
				else if(m_Core.m_Pos.y > 222 * 32) //freeze becken unter area
				{
					//Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze becken unter area");
				}

				if((m_Core.m_Pos.y < 220 * 32 && m_Core.m_Pos.x < 415 * 32 && m_FreezeTime > 1) && (m_Core.m_Pos.x > 350 * 32)) //always suicide on freeze if not reached teh block area yet             (new) AND not coming from the new spawn and falling through the freeze
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze und links der block area");
				}

				//Movement bis zur ruler area:
				/*
				NEW! Movement modes for the basic move till hammerfly!
				m_Dummy_movement_mode23 is a int to check which movement style the bot shoudl used

				0					normal old basic mode
				1					new mode jump left side up into ruler area [ALPHA]


				i dunno how to set the modes for now its hardcodet set to 1 maybe add a random switcher or depending on how frustrated the bot is

				*/
				m_Dummy_movement_mode23 = 0;

				if(m_Core.m_Pos.x < 388 * 32 && m_Core.m_Pos.y > 213 * 32) //jump to old spawn
				{
					m_Input.m_Jump = 1;
					m_Input.m_Fire++;
					m_LatestInput.m_Fire++;
					m_Input.m_Hook = 1;
					m_Input.m_TargetX = -200;
					m_Input.m_TargetY = 0;
				}

				if(m_Dummy_movement_mode23 == 0)
				{
					if(m_Core.m_Pos.x < 415 * 32) //bis zum tunnel laufen
					{
						m_Input.m_Direction = 1;
					}
					else if(m_Core.m_Pos.x < 440 * 32 && m_Core.m_Pos.y > 213 * 32) //im tunnel laufen
					{
						m_Input.m_Direction = 1;
						if(m_Core.m_Vel.x < 5.5f)
						{
							m_Input.m_TargetY = -3;
							m_Input.m_TargetX = 200;
							m_LatestInput.m_TargetY = -3;
							m_LatestInput.m_TargetX = 200;

							if(Server()->Tick() % 30 == 0)
							{
								SetWeapon(0);
							}
							if(Server()->Tick() % 55 == 0)
							{
								if(m_FreezeTime == 0)
								{
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
								}
							}
							if(Server()->Tick() % 200 == 0)
							{
								m_Input.m_Jump = 1;
							}
						}
					}

					//externe if abfrage weil laufen während sprinegn xD
					if(m_Core.m_Pos.x > 413 * 32 && m_Core.m_Pos.x < 415 * 32) // in den tunnel springen
					{
						m_Input.m_Jump = 1;
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
						//m_Input.m_Jump = 0;
					}
					else if(m_Core.m_Pos.x > 428 * 32 - 20 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen
					{
						m_Input.m_Jump = 1;
					}

					// externen springen aufhören für dj
					if(m_Core.m_Pos.x > 428 * 32 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen nicht mehr springen
					{
						m_Input.m_Jump = 0;
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
					}

					//nochmal extern weil springen während springen
					if(m_Core.m_Pos.x > 430 * 32 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen springen
					{
						m_Input.m_Jump = 1;
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
					}

					if(m_Core.m_Pos.x > 431 * 32 && m_Core.m_Pos.y > 213 * 32) //jump refillen für wayblock spot
					{
						m_Input.m_Jump = 0;
					}
				}
				else if(m_Dummy_movement_mode23 == 1) //enter ruler area with a left jump
				{
					if(m_Core.m_Pos.x < 415 * 32) //bis zum tunnel laufen
					{
						m_Input.m_Direction = 1;
					}
					else if(m_Core.m_Pos.x < 440 * 32 && m_Core.m_Pos.y > 213 * 32) //im tunnel laufen
					{
						m_Input.m_Direction = 1;
					}

					//springen
					if(m_Core.m_Pos.x > 413 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.y > 213 * 32) // in den tunnel springen
					{
						m_Input.m_Jump = 1;
					}
					else if(m_Core.m_Pos.x > 428 * 32 - 3 && m_Core.m_Pos.y > 217 * 32 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen
					{
						m_Input.m_Jump = 1;
					}

					if(m_Core.m_Pos.x > 429 * 32 - 18)
					{
						m_Input.m_Direction = -1;
					}

					//nochmal extern weil springen während springen
					if(m_Core.m_Pos.y < 217 * 32 && m_Core.m_Pos.x > 420 * 32 && m_Core.m_Pos.y > 213 * 32 + 20)
					{
						//m_Input.m_Direction = -1;
						if(m_Core.m_Pos.y < 216 * 32) // im tunnel springen springen
						{
							m_Input.m_Jump = 1;
						}
					}
				}

				//MoVement ab dem ruler spot weiter laufen
				//NEW! wenn er links vom freeze becken im ruler spot is also beim wb spot des 18er modes dann jump übers freeze
				if(m_Core.m_Pos.y < 213 * 32 && m_Core.m_Pos.x > 428 * 32 && m_Core.m_Pos.x < 429 * 32)
				{
					m_Input.m_Jump = 1;
				}

				if(m_Core.m_Pos.x > 417 * 32 && m_Core.m_Pos.y < 213 * 32 && m_Core.m_Pos.x < 450 * 32) //vom ruler nach rechts nachm unfreeze werden
				{
					m_Input.m_Direction = 1;
				}

				if(m_Core.m_Pos.x > 439 * 32 && m_Core.m_Pos.y < 213 * 32 && m_Core.m_Pos.x < 441 * 32) //über das freeze zum hf start springen
				{
					m_Input.m_Jump = 1;
				}

				if(m_Core.m_Pos.y > 200 * 32 && m_Core.m_Pos.x > 457 * 32)
				{
					m_Input.m_Direction = -1;
				}

				//unnötiger dj
				//if (m_Core.m_Pos.x > 441 * 32 + 10 && m_Core.m_Pos.y < 213 * 32 && m_Core.m_Pos.x < 442 * 32)
				//{
				//	m_Input.m_Jump = 1;
				//}

				//TODO:
				//aufpassen dass er das ganze nur macht wenn er nicht schon beim 2p part ist
				if(m_Dummy_collected_weapons)
				{
					if(m_Core.m_Pos.x < 466 * 32)
					{
						SetWeapon(3);
					}
					//prepare for rocktjump

					if(m_Core.m_Pos.x < 451 * 32 + 1 && m_Core.m_Pos.y > 209 * 32) //wenn zu weit links für rj
					{
						m_Input.m_Direction = 1;
					}
					else if(m_Core.m_Pos.x > 451 * 32 + 3 && m_Core.m_Pos.y > 209 * 32) //wenn zu weit links für rj
					{
						m_Input.m_Direction = -1;
					}
					else
					{
						if(m_Core.m_Vel.x < 0.01f && m_Core.m_Vel.x > -0.01f) //nahezu stillstand
						{
							//ROCKETJUMP
							if(m_Core.m_Pos.x > 450 * 32 && m_Core.m_Pos.y > 209 * 32)
							{
								//Wenn der bot weit genung is und ne waffe hat und tief genung is
								// ---> bereit machen für rocketjump
								//damit der bot nicht ausm popo schiesst xD

								m_Input.m_TargetX = 0;
								m_Input.m_TargetY = 37;
								m_LatestInput.m_TargetX = 0;
								m_LatestInput.m_TargetY = 37;
							}

							if(m_Core.m_Pos.y > 210 * 32 + 30 && !isFreezed) //wenn der dummy auf dem boden steht und unfreeze is
							{
								if(m_Core.m_Vel.y == 0.000000f)
								{
									m_Input.m_Jump = 1;
								}
							}

							if(m_Core.m_Pos.y > 210 * 32 + 10 && m_Core.m_Vel.y < -0.9f && !isFreezed) //dann schiessen
							{
								//m_LatestInput.m_TargetX = 0;
								//m_LatestInput.m_TargetY = 10;
								m_LatestInput.m_Fire++;
								m_Input.m_Fire++;
							}
						}
					}
				}

				if(m_Core.m_Pos.x > 448 * 32 && m_Core.m_Pos.x < 458 * 32 && m_Core.m_Pos.y > 209 * 32) //wenn der bot auf der platform is
				{
					//nicht zu schnell laufen
					if(Server()->Tick() % 3 == 0)
					{
						m_Input.m_Direction = 0;
						//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
					}
				}

				//Rocketjump2 an der freeze wand
				//prepare aim!
				if(m_Core.m_Pos.y < 196 * 32)
				{
					m_Input.m_TargetX = -55;
					m_Input.m_TargetY = 32;
					m_LatestInput.m_TargetX = -55;
					m_LatestInput.m_TargetY = 32;
				}

				if(m_Core.m_Pos.x < 452 * 32 && m_Core.m_Pos.y > 188 * 32 && m_Core.m_Pos.y < 192 * 32 && m_Core.m_Vel.y < 0.1f && m_Dummy_collected_weapons)
				{
					m_Dummy_rjumped2 = true;
					m_LatestInput.m_Fire++;
					m_Input.m_Fire++;
				}

				//Fliegen nach rj2
				if(m_Dummy_rjumped2)
				{
					m_Input.m_Direction = 1;

					if(m_Core.m_Pos.x > 461 * 32 && m_Core.m_Pos.y > 192 * 32 + 20)
					{
						m_Input.m_Jump = 1;
					}

					if(m_Core.m_Pos.x > 478 * 32 || m_Core.m_Pos.y > 196 * 32)
					{
						m_Dummy_rjumped2 = false;
					}
				}

				//Check ob der dummy schon waffen hat
				if(m_aWeapons[3].m_Got && m_aWeapons[2].m_Got)
				{
					m_Dummy_collected_weapons = true;
				}
				else //wenn er sie wd verliert zb durch shields
				{
					m_Dummy_collected_weapons = false;
				}

				if(1 == 0.5 + 0.5)
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
					if(pChr && pChr->IsAlive())
					{
						if(pChr->m_Pos.y < 165 * 32 && pChr->m_Pos.x > 451 * 32 - 10 && pChr->m_Pos.x < 454 * 32 + 10)
						{
							m_Dummy_mate_collected_weapons = true;
						}
					}
				}

				//Hammerfly
				if(m_Core.m_Pos.x > 447 * 32)
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
					if(pChr && pChr->IsAlive())
					{
						//unfreezemates on platform

						//Get closer to the mate
						if(pChr->m_Pos.y == m_Core.m_Pos.y && m_Core.m_Pos.x > 450 * 32 && m_Core.m_Pos.x < 457 * 32 && pChr->m_FreezeTime > 0)
						{
							if(pChr->m_Pos.x > m_Core.m_Pos.x + 70) //if friend is on the right of the bot
							{
								m_Input.m_Direction = 1;
							}
							else if(pChr->m_Pos.x < m_Core.m_Pos.x - 70) //if firend is on the left of the bot
							{
								m_Input.m_Direction = -1;
							}
						}

						//Hammer mate if near enough
						if(m_Core.m_Pos.x < 456 * 32 + 20 && pChr->m_FreezeTime > 0 && m_Core.m_Pos.y > 209 * 32 && pChr->m_Pos.y > 209 * 32 && pChr->m_Pos.x > 449 * 32 && pChr->m_Pos.x < 457 * 32)
						{
							if(pChr->m_Pos.x > m_Core.m_Pos.x - 60 && pChr->m_Pos.x < m_Core.m_Pos.x + 60)
							{
								if(m_Core.m_ActiveWeapon == WEAPON_HAMMER && m_FreezeTime == 0)
								{
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
									m_Dummy_hook_mate_after_hammer = true;
								}
							}
						}
						if(m_Dummy_hook_mate_after_hammer)
						{
							if(pChr->m_Core.m_Vel.x < -0.3f || pChr->m_Core.m_Vel.x > 0.3f)
							{
								m_Input.m_Hook = 1;
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

						if(pChr->isFreezed)
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
							CCharacter *pChr = GameServer()->m_World.ClosestCharTypeFreeze(m_Pos, true, this); //only search freezed tees --> so even if others get closer he still has his mission
							if(pChr && pChr->IsAlive())
							{
								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 2);

								//Check where help is needed
								if(pChr->m_Pos.x > 457 * 32 + 10 && pChr->m_Pos.x < 468 * 32 && pChr->m_Pos.y < 213 * 32 + 5) //right freeze becken
								{
									//Get in help position:
									if(m_Core.m_Pos.x < 457 * 32 - 1)
									{
										m_Input.m_Direction = 1;
									}
									else if(m_Core.m_Pos.x > 457 * 32 + 8)
									{
										m_Input.m_Direction = -1;
									}

									//jump
									if(m_Core.m_Vel.y == 0.000000f && m_FreezeTime == 0 && m_Core.m_Pos.y > 209 * 32)
									{
										if(Server()->Tick() % 16 == 0)
										{
											m_Input.m_Jump = 1;
										}
									}

									//hook
									if(m_Core.m_Pos.y < pChr->m_Pos.y - 60 && pChr->m_FreezeTime > 0)
									{
										m_Input.m_Hook = 1;
										if(m_Core.m_Pos.x > 454 * 32)
										{
											m_Input.m_Direction = -1;
										}
									}

									//unfreezehammer
									if(pChr->m_Pos.x < m_Core.m_Pos.x + 60 && pChr->m_Pos.x > m_Core.m_Pos.x - 60 && pChr->m_Pos.y < m_Core.m_Pos.y + 60 && pChr->m_Pos.y > m_Core.m_Pos.y - 60)
									{
										if(m_FreezeTime == 0)
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}
									}
								}
								else if(pChr->m_Pos.x > 469 * 32 + 20 && pChr->m_Pos.x < 480 * 32 && pChr->m_Pos.y < 213 * 32 + 5 && pChr->m_Pos.y > 202 * 32)
								{
									//Get in help position:
									if(m_Core.m_Pos.x < 467 * 32)
									{
										if(m_Core.m_Pos.x < 458 * 32)
										{
											if(m_Core.m_Vel.y == 0.000000f)
											{
												m_Input.m_Direction = 1;
											}
										}
										else
										{
											m_Input.m_Direction = 1;
										}

										if(m_Core.m_Vel.y > 0.2f || m_Core.m_Pos.y > 212 * 32)
										{
											m_Input.m_Jump = 1;
										}
									}
									if(m_Core.m_Pos.x > 469 * 32)
									{
										m_Input.m_Direction = -1;
										if(m_Core.m_Vel.y > 0.2f || m_Core.m_Pos.y > 212 * 32)
										{
											m_Input.m_Jump = 1;
										}
									}

									//jump
									if(m_Core.m_Vel.y == 0.000000f && m_FreezeTime == 0 && m_Core.m_Pos.y > 209 * 32 && m_Core.m_Pos.x > 466 * 32)
									{
										if(Server()->Tick() % 16 == 0)
										{
											m_Input.m_Jump = 1;
										}
									}

									//hook
									if(m_Core.m_Pos.y < pChr->m_Pos.y - 60 && pChr->m_FreezeTime > 0)
									{
										m_Input.m_Hook = 1;
										if(m_Core.m_Pos.x > 468 * 32)
										{
											m_Input.m_Direction = -1;
										}
									}

									//unfreezehammer
									if(pChr->m_Pos.x < m_Core.m_Pos.x + 60 && pChr->m_Pos.x > m_Core.m_Pos.x - 60 && pChr->m_Pos.y < m_Core.m_Pos.y + 60 && pChr->m_Pos.y > m_Core.m_Pos.y - 60)
									{
										if(m_FreezeTime == 0)
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}
									}
								}
								else if(pChr->m_Pos.x > 437 * 32 && pChr->m_Pos.x < 456 * 32 && pChr->m_Pos.y < 219 * 32 && pChr->m_Pos.y > 203 * 32) //left freeze becken
								{
									if(m_aWeapons[2].m_Got && Server()->Tick() % 40 == 0)
									{
										SetWeapon(2);
									}

									if(m_Core.m_Jumped == 0) //has dj --> go left over the freeze and hook ze mate
									{
										m_Input.m_Direction = -1;
									}
									else //no jump --> go back and get it
									{
										m_Input.m_Direction = 1;
									}

									if(m_Core.m_Pos.y > 211 * 32 + 21)
									{
										m_Input.m_Jump = 1;
										m_Dummy_help_m8_before_hf_hook = true;
										if(m_aWeapons[2].m_Got && m_FreezeTime == 0)
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}

										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "jump + hook");
									}

									if(m_Dummy_help_m8_before_hf_hook)
									{
										m_Input.m_Hook = 1;
										m_Dummy_help_m8_before_hf_hook++;
										if(m_Dummy_help_m8_before_hf_hook > 60 && m_Core.m_HookState != HOOK_GRABBED)
										{
											m_Dummy_help_m8_before_hf_hook = 0;
											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stopped hook");
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

							CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
							if(pChr && pChr->IsAlive())
							{
								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								//Hammerfly normal way
								if(m_Dummy_mode23 == 0)
								{
									//shot schiessen schissen
									//im freeze nicht schiessen
									if(m_DummyFreezed == false)
									{
										m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
										m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

										//schiess delay
										if(Server()->Tick() >= m_EmoteTickNext && pChr->m_Pos.y < 212 * 32 - 5)
										{
											m_pPlayer->m_LastEmote = Server()->Tick();
											GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);

											m_LatestInput.m_Fire++;
											m_Input.m_Fire++;

											m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() / 2;
										}

										//wenn schon nah an der nade
										if(m_Core.m_Pos.y < 167 * 32)
										{
											m_Input.m_Jump = 1;

											if(m_Core.m_Pos.x < 453 * 32 - 8)
											{
												m_Input.m_Direction = 1;
											}
											else if(m_Core.m_Pos.x > 454 * 32 + 8)
											{
												m_Input.m_Direction = -1;
											}
										}
									}
									else if(m_DummyFreezed == true) //if (m_DummyFreezed == false)
									{
										m_LatestInput.m_Fire = 0;
										m_Input.m_Fire = 0;
										m_DummyFreezed = false;
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hey im freezded lul xD");
									}
									else
									{
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "iwas is maechtig flasch gelaufen du bob");
									}
								}
								else if(m_Dummy_mode23 == 2) //Speedhammerfly for ChillerDragon
								{
									//lauf zu dem hin
									//nur wenn der nächste spieler grad nach ooben fliegt oder auf der selben höhe ist
									if(pChr->m_Pos.y == m_Core.m_Pos.y || pChr->m_Core.m_Vel.y < -0.4f)
									{
										if(pChr->m_Pos.y >= m_Core.m_Pos.y)
										{
											if(pChr->m_Pos.x + 1 < m_Core.m_Pos.x) //wenn zu weit rechts
											{
												if(m_Core.m_Pos.x > 452 * 32)
													m_Input.m_Direction = -1;
											}
											else if(m_Core.m_Pos.x + 1 < pChr->m_Pos.x) //wenn zu weit links
											{
												if(m_Core.m_Pos.x < 455 * 32)
													m_Input.m_Direction = 1;
											}
										}
									}

									//und wenn der hoch springt
									//hau den weg xD
									if(pChr->m_Core.m_Vel.y < -0.5f && m_Core.m_Pos.y + 15 > pChr->m_Pos.y) //wenn der dude speed nach oben hat
									{
										m_Input.m_Jump = 1;
										if(m_FreezeTime == 0)
										{
											m_LatestInput.m_Fire++;
											m_Input.m_Fire++;
										}
									}
								}
								else
								{
									if(Server()->Tick() % 600 == 0)
									{
										GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "for this dummymode23 is no hammerflytype set :c");
									}
								}
							}
						}
					}
				}

				//Der krasse shit!
				//Der bot macht den 2Player part o.O
				//erstmal schauen wann der bot den 2player part machen soll

				//if (m_Core.m_Pos.x > 466 * 32)
				//{
				//	CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
				//	if (pChr && pChr->IsAlive())
				//	{
				//		m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				//		m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				//		m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				//		m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				//		if (m_Dummy_collected_weapons && m_FreezeTime == 0 && m_Core.m_Pos.x > 478 * 32 && m_Core.m_Pos.x < 485 * 32 && pChr->m_Pos.x > 476 * 32)
				//		{
				//			SetWeapon(0);

				//			if (pChr->m_Pos.x < m_Core.m_Pos.x && m_Core.m_Pos.x > 481 * 32 && pChr->m_Pos.y < 195 * 32) //wenn sich der racemate zum schieben eignet schieb ihn zum abgrund um ihn dort dann später zu hammern
				//			{
				//				m_Input.m_Direction = -1;
				//			}

				//			if (pChr->m_Pos.x < m_Core.m_Pos.x && m_Core.m_Pos.x - pChr->m_Pos.x < 8 && m_Core.m_Pos.x > 481 * 32 - 1) //wenn der racemate links des bots bereit um ins freeze gehammert zu werden liegt hau raus
				//			{
				//				m_Input.m_Fire++;
				//				m_LatestInput.m_Fire++;
				//			}

				//			if (pChr->m_Pos.y > 194 * 32 && m_Core.m_Pos.x < 481 * 32 && pChr->m_Pos.x < m_Core.m_Pos.x && pChr->m_Core.m_Vel.y > -0.5f) //wenn der racemate unter der platform ist und der bot geeigent zum draggen is ---> gogo
				//			{
				//				m_Dummy_dd_hook = true;
				//			}

				//			//TODO:
				//			//abfrage die m_Dummy_dd_hook wieder auf false setzt (wenn pChr zu tief is oder zu weit rechts oder der hammer gehittet hat)

				//			if (m_Dummy_dd_hook)
				//			{
				//				m_Input.m_Hook = 1;
				//				m_Input.m_Direction = 1;

				//				if (pChr->m_Pos.x > 485 * 32)
				//				{
				//					m_Input.m_Jump = true;
				//				}
				//			}
				//		}
				//	}
				//}

				/*
				New Stuff:
				commented out the whole old system

				Struct:

				STRUCT[1]: Check if bot shoudl change m_Dummy_2p_state

				STRUCT[2]: Let the bot do stuff depenging on m_Dummy_2p_state

				States:
				-2				Help pChr out of freeze
				-1				do nothing
				0				prepare for making the part (gettin in the right position)
				1				starting to do the part -> walking left and hammerin'
				2				keep going doing the part -> hookin' and walking to the right
				3				final stage of doing the part -> jumpin' and unfreeze pChr with hammerhit
				4				jump in freeze and let the mate help

				5				go on edge if pChr dragged u through the part 
				6				if on edge sg and unfreeze mate

				*/

				if(m_Core.m_Pos.y < 200 * 32)
				{
					//check ob der mate fail ist
					CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
					if(pChr && pChr->IsAlive())
					{
						if((pChr->m_Pos.y > 198 * 32 + 10 && pChr->IsGrounded()) ||
							(pChr->m_Pos.y < 198 * 32 + 10 && pChr->m_Pos.x < 472 * 32 && pChr->IsGrounded()) || // recognize mates freeze in the freeze tunnel on the left
							(m_Dummy_mate_help_mode == 3)) // yolo hook swing mode handles mate as failed until he is unfreeze
						{
							if(pChr->isFreezed)
								m_Dummy_mate_failed = true;
						}
						if(pChr->m_FreezeTime == 0)
						{
							m_Dummy_mate_failed = false;
							m_Dummy_mate_help_mode = 2; // set it to something not 3 because help mode 3 cant do the part (cant play if not failed)
						}
					}

					//schau ob der bot den part geschafft hat und auf state -1 gehen soll
					if(m_Core.m_Pos.x > 485 * 32)
					{
						m_Dummy_2p_state = -1; //part geschafft --> mach aus
					}

					if(m_Core.m_Pos.x > 466 * 32)
					{
						CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
						if(pChr && pChr->IsAlive())
						{
							m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
							m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							//holla
							//if (m_Dummy_collected_weapons && m_FreezeTime == 0 && m_Core.m_Pos.x > 478 * 32 && m_Core.m_Pos.x < 485 * 32 && pChr->m_Pos.x > 476 * 32)
							if(m_Dummy_collected_weapons && m_FreezeTime == 0 && m_Core.m_Pos.x > 478 * 32 && m_Core.m_Pos.x < 492 * 32 + 10 && pChr->m_Pos.x > 476 * 32) //new testy
							{
								//New direct in state
								//if (m_Dummy_2p_state == 1)
								//SetWeapon(0);

								//Reset Checkbools

								//if (!m_Core.m_HookState == HOOK_GRABBED && m_Dummy_2p_hook_grabbed) //wenn der bot denkt er grabbt ihn noch aber schon los gelassen hat --> fang von vorne an
								//{
								//	m_Dummy_2p_hook = false;
								//}

								if(pChr->m_Pos.y > 198 * 32) //wenn pChr iwiw runter gefallen is dann mach den hook weg
								{
									m_Dummy_2p_hook = false;
								}

								//STRUCT[1]: Check if bot shoudl change m_Dummy_2p_state
								if(m_Core.m_Pos.x < 477 * 32 || m_Dummy_mate_failed) //TODO: add if pChr wants to make the part
								{
									m_Dummy_2p_state = -1;
								}
								//                                                                                     || neu resette wenn der spieler kurz von der platform springt
								// NEW: added the bool to not start doing the part while helping
								if(m_Core.m_Pos.x > 477 * 32 && m_Core.m_Pos.x < 485 * 32 && m_Core.m_Pos.y < 195 * 32 /*|| pChr->m_Pos.x < 476 * 32 - 11 || pChr->m_Pos.y < 191 * 32*/) //alle states die mit anfangen zutuen haben nur wenn der bot auch in position steht den part zu machen
								{
									if(pChr->m_FreezeTime == 0 && m_FreezeTime == 0) //wenn beide unfreeze sind zeih auf
									{
										m_Dummy_2p_state = 0;
										//m_Dummy_2p_hook = false;
										//m_Dummy_2p_hook_grabbed = false;
									}
									//																								// NEW testy || stuff
									if((m_Core.m_Pos.x > pChr->m_Pos.x && pChr->m_Pos.y == m_Core.m_Pos.y && m_Core.m_Pos.x > 481 * 32) || (pChr->m_Pos.x > 476 * 32 - 10 && m_Core.m_Pos.x > pChr->m_Pos.x && pChr->m_Pos.y > 191 * 32 - 10 && m_Core.m_Pos.x < 482 * 32 + 10))
									{
										m_Dummy_2p_state = 1; //starting to do the part->walking left and hammerin'
										if(Server()->Tick() % 30 == 0 && m_Dummy_nothing_happens_counter == 0)
										{
											SetWeapon(0);
										}
										//m_Dummy_2p_hammer1 = false;
									}
									//                                                                                 NEW TESTY || stuff     wenn der schonmal ausgelöst wurde bleib da bis der nexte ausgelöst wird oder pChr runter füllt
									if((m_Dummy_2p_state == 1 && pChr->m_Core.m_Vel.y > 0.5f && pChr->m_Pos.x < 479 * 32) || m_Dummy_2p_hook)
									{
										m_Dummy_2p_state = 2; //keep going doing the part->hookin' and walking to the right
										m_Dummy_2p_hook = true;
										/*						if (m_Core.m_HookState == HOOK_GRABBED)
										{
										m_Dummy_2p_hook_grabbed = true;
										}*/
									}

									if(m_Dummy_2p_state == 2 && pChr->m_Pos.x > 485 * 32 + 8)
									{
										m_Dummy_2p_state = 3; //final stage of doing the part->jumpin' and unfreeze pChr with hammerhit
									}

									//           NICHT NACH FREEZE ABRAGEN damit der bot auch ins freeze springt wenn das team fail ist und dann selfkill macht
									if(pChr->m_Pos.x > 489 * 32 || (pChr->m_Pos.x > 486 * 32 && pChr->m_Pos.y < 186 * 32)) //Wenn grad gehammert und der tee weit genugn is spring rein
									{
										m_Dummy_2p_state = 4;
									}

									if(pChr->m_Pos.y < 191 * 32 && pChr->m_Pos.x < 486 * 32) //resette auf state=0 wenn pChr springt
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
								if(m_FreezeTime == 0 && m_Core.m_Pos.x > 485 * 32 && pChr->m_Pos.x < 485 * 32) //wenn der bot rechts und unfreeze is und der mate noch links
								{
									m_Dummy_2p_state = 5;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "set state 5");
								}

								if(m_FreezeTime == 0 && m_Core.m_Pos.x > 490 * 32 && pChr->m_FreezeTime > 0)
								{
									m_Dummy_2p_state = 6;
								}

								//STRUCT[2]: Let the bot do stuff depenging on m_Dummy_2p_state

								if(m_Dummy_2p_state == 0) //prepare doing the part (gettin right pos)
								{
									m_Input.m_Direction = 1; //walking right until state 1 gets triggerd
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "debug [1]");
								}
								else if(m_Dummy_2p_state == 1) //starting to do the part -> walking left and hammerin'
								{
									if(m_Core.m_Pos.x > 480 * 32 - 15) //lauf nach links bis zur hammer pos
									{
										m_Input.m_Direction = -1;
									}

									if(pChr->m_Pos.x < 480 * 32) //wenn pChr weit gwenung zum hammern is
									{
										m_Input.m_Fire++;
										m_LatestInput.m_Fire++;
										//m_Dummy_2p_hammer1 = true;
									}

									//testy stop mate if hammer was too hard and mate fly to far
									if(pChr->m_Pos.x < 478 * 32)
									{
										m_Input.m_Hook = 1;
									}
								}
								else if(m_Dummy_2p_state == 2) //keep going doing the part->hookin' and walking to the right
								{
									if(pChr->m_Pos.y > 194 * 32 + 10)
										m_Input.m_Hook = 1;

									if(pChr->m_Pos.y < 197 * 32)
										m_Input.m_Direction = 1;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "debug [2]");
								}
								else if(m_Dummy_2p_state == 3) //final stage of doing the part->jumpin' and unfreeze pChr with hammerhit
								{
									if(Server()->Tick() % 30 == 0)
									{
										SetWeapon(0); //hammer
									}

									if(pChr->m_FreezeTime > 0) //keep holding hook untill pChr is unfreeze
									{
										m_Input.m_Hook = 1;
									}

									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "debug [3]");
									m_Input.m_Direction = 1;
									m_Input.m_Jump = 1;

									//Now tricky part the unfreeze hammer
									if(pChr->m_Pos.y - m_Core.m_Pos.y < 7 && m_FreezeTime == 0) //wenn der abstand der beiden tees nach oben weniger is als 7 ^^
									{
										m_Input.m_Fire++;
										m_LatestInput.m_Fire++;
									}
								}
								//MOVED TO EXTERN CUZ SPECIAL
								//else if (m_Dummy_2p_state == 4) //PART geschafft! spring ins freeze
								//{
								//	if (m_Core.m_Pos.y < 195 * 32 && m_Core.m_Pos.x > 478 * 32) //wenn der bot noch auf der plattform is
								//	{
								//		m_Input.m_Direction = -1; //geh links bisse füllst
								//	}
								//	else //wenn de füllst
								//	{
								//		m_Input.m_Direction = 1;
								//	}
								//}
							}

							//Mega externen stuff is der state4 weil der ausm gültigeitsbereich (platform) raus läuft und so der is halt was beonders deswegen steht der an einer besonder verwirrenden stelle -.-
							if(!m_Dummy_mate_failed && m_Dummy_2p_state == 4) //PART geschafft! spring ins freeze
							{
								//Shotgun boost xD
								SetWeapon(2);
								m_Input.m_TargetX = 1;
								m_Input.m_TargetY = 1;
								m_LatestInput.m_TargetX = 1;
								m_LatestInput.m_TargetY = 1;

								if(m_Core.m_Pos.y < 195 * 32 && m_Core.m_Pos.x > 478 * 32 - 15) //wenn der bot noch auf der plattform is
								{
									if(m_Core.m_Pos.x < 480 * 32) //wenn er schon knapp an der kante is
									{
										//nicht zu schnell laufen
										if(Server()->Tick() % 5 == 0)
										{
											m_Input.m_Direction = -1; //geh links bisse füllst
										}
									}
									else
									{
										m_Input.m_Direction = -1;
									}
								}
								else //wenn de füllst
								{
									m_Input.m_Direction = 1;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "debug [4]");
								}

								//DJ ins freeze
								if(m_Core.m_Pos.y > 195 * 32 + 10)
								{
									m_Input.m_Jump = 1;
								}

								if(m_Input.m_Direction == 1 && m_FreezeTime == 0)
								{
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
								}
							}

							if(!m_Dummy_mate_failed && m_Dummy_2p_state == 5) //made the part --> help mate
							{
								if(pChr->m_FreezeTime == 0 && pChr->m_Pos.x > 485 * 32)
								{
									m_Dummy_2p_state = -1;
								}

								if(m_Core.m_Jumped > 1) //double jumped
								{
									if(m_Core.m_Pos.x < 492 * 32 - 30) //zu weit links
									{
										m_Input.m_Direction = 1;
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Direction = 1");

										if(m_Core.m_Pos.x > 488 * 32) //wenn schon knapp dran
										{
											//nur langsam laufen (bissl bremsen)
											if(m_Core.m_Vel.x < 2.3f)
											{
												m_Input.m_Direction = 0;
											}
										}
									}
								}

								//hold left wall till jump to always do same move with same distance and speed
								if(m_Core.m_Jumped < 2 && !IsGrounded())
								{
									m_Input.m_Direction = -1;
								}

								if(m_Core.m_Pos.y > 195 * 32)
								{
									m_Input.m_Jump = 1;
								}

								if(m_Core.m_Pos.x > 492 * 32)
								{
									m_Input.m_Direction = -1;
								}
							}
							//else if (m_Dummy_2p_state == -2) //auch extern weil der dummy vlt mal von der platform springt zum helfen
							//if (m_Dummy_mate_failed && m_Dummy_2p_state < 1)    <--- added m_Dummy_mate_failed to the state checks
							if(m_Dummy_mate_failed)
							{
								//The bot coudl fall of the plattform and hurt but this var helps to activate and accident
								//sometimes the special stage causes a jump on purpose and the var gets true so no emergency can be called
								//to make this possible agian reset this var every tick here
								//m_Dummy_help_no_emergency is used to allow the emergency help
								m_Dummy_help_no_emergency = false;

								if(Server()->Tick() % 20 == 0)
								{
									GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
								}

								//Go on left edge to help:
								if(m_Core.m_Pos.x > 479 * 32 + 4) //to much right
								{
									if(m_Core.m_Pos.x < 480 * 32 - 25)
									{
										if(Server()->Tick() % 9 == 0)
										{
											m_Input.m_Direction = -1;
										}
									}
									else
									{
										m_Input.m_Direction = -1;
										m_Input.m_TargetX = 300;
										m_Input.m_TargetY = -10;
										m_LatestInput.m_TargetX = 300;
										m_LatestInput.m_TargetY = -10;
									}

									if(m_Core.m_Vel.x < -1.5f && m_Core.m_Pos.x < 480 * 32)
									{
										m_Input.m_Direction = 0;
									}
								}
								else if(m_Core.m_Pos.x < 479 * 32 - 1)
								{
									m_Input.m_Direction = 1;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "debug [6]");
								}

								//Get mate with shotgun in right position:
								//if (pChr->m_Pos.x < 479 * 32 + 6) //if the mate is left enough to get shotgunned from the edge
								if(pChr->m_Pos.x < 478 * 32 && (m_Dummy_mate_help_mode != 3)) // if currently in yolo fly save mode -> goto else branch to keep yolo flying
								{
									if(Server()->Tick() % 30 == 0)
									{
										SetWeapon(2); //switch to sg
									}

									if(m_FreezeTime == 0 && pChr->m_Core.m_Vel.y == 0.000000f && pChr->m_Core.m_Vel.x < 0.007f && pChr->m_Core.m_Vel.x > -0.007f && m_Core.m_Pos.x < 480 * 32)
									{
										m_Input.m_Fire++;
										m_LatestInput.m_Fire++;
									}
								}
								else //if right enough to stop sg
								{
									if(pChr->m_Pos.x < 479 * 32 && (m_Dummy_mate_help_mode != 3))
									{
										if(pChr->m_Pos.y > 194 * 32)
										{
											if(pChr->m_Core.m_Pos.y > 197 * 32)
											{
												m_Input.m_Hook = 1;
											}
											//reset hook if something went wrong
											if(Server()->Tick() % 90 == 0 && pChr->m_Core.m_Vel.y == 0.000000f) //if the bot shoudl hook but the mate lays on the ground --> resett hook
											{
												m_Input.m_Hook = 0;
												m_Dummy_nothing_happens_counter++;
												if(m_Dummy_nothing_happens_counter > 2)
												{
													if(m_Core.m_Pos.x > 478 * 32 - 1 && m_Core.m_Jumped == 0)
													{
														m_Input.m_Direction = -1;
													}
													m_Input.m_TargetX = m_Input.m_TargetX - 5;
													m_LatestInput.m_TargetX = m_LatestInput.m_TargetX - 5;
												}
												if(m_Dummy_nothing_happens_counter > 4) //warning long time nothing happend! do crazy stuff
												{
													if(m_FreezeTime == 0)
													{
														m_Input.m_Fire++;
														m_LatestInput.m_Fire++;
													}
												}
												if(m_Dummy_nothing_happens_counter > 5) //high warning mate coudl get bored --> swtich through all weapons and move angel back
												{
													SetWeapon(m_Dummy_panic_weapon);
													m_Dummy_panic_weapon++;
													m_Input.m_TargetX++;
													m_LatestInput.m_TargetX++;
												}
											}
											if(pChr->m_Core.m_Vel.y != 0.000000f)
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
										if(m_Core.m_ActiveWeapon == WEAPON_SHOTGUN && m_FreezeTime == 0)
										{
											if(pChr->m_Pos.y < 198 * 32 && (m_Dummy_mate_help_mode != 3)) //if mate is high enough and not in yolo hook help mode
											{
												m_Input.m_TargetX = -200;
												m_Input.m_TargetY = 30;
												m_LatestInput.m_TargetX = -200;
												m_LatestInput.m_TargetY = 30;
												m_Input.m_Fire++;
												m_LatestInput.m_Fire++;
											}
											else //if mate is too low --> change angel or move depnding on the x position
											{
												if(pChr->m_Pos.x < 481 * 32 - 4) //left enough to get him with other shotgun angels from the edge
												{
													//first go on the leftest possible pos on the edge
													if(m_Core.m_Vel.x > -0.0004f && m_Core.m_Pos.x > 478 * 32 - 2 && m_Core.m_Jumped == 0)
													{
														m_Input.m_Direction = -1;
													}
													/*
													[PLANNED BUT NOT NEEDED]: add more help modes and switch trough them. for now just help mode 2 is used and the int to swtich modes is usless
													Then start to help.
													There are different help modes to have some variations if nothing happens
													help modes:

													1				Old way of helping try to wallshot staright down (doesnt work)
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
														if(m_Core.m_Jumped < 2)
														{
															if(m_Core.m_Pos.x > 476 * 32)
															{
																m_Input.m_Direction = -1;
																if(IsGrounded() && m_Core.m_Pos.x < 481 * 32)
																{
																	m_Input.m_Jump = 1;
																}

																if(m_Core.m_Pos.x < 477 * 32)
																{
																	m_Input.m_Hook = 1; // start hook
																}
																if(m_Core.m_HookState == HOOK_GRABBED)
																{
																	m_Input.m_Hook = 1; // hold hook
																	m_Input.m_Direction = 1;
																	m_Input.m_TargetX = 200;
																	m_Input.m_TargetY = 7;
																	if(!m_FreezeTime)
																	{
																		m_Input.m_Fire++;
																		m_LatestInput.m_Fire++;
																	}
																}
															}

															// anti stuck on edge
															if(Server()->Tick() % 80 == 0)
															{
																if(m_Core.m_Vel.x < 0.0f && m_Core.m_Vel.y < 0.0f)
																{
																	m_Input.m_Direction = -1;
																	if(m_Core.m_Pos.y > 193 * 32 + 18) // don't jump in the freeze roof
																	{
																		m_Input.m_Jump = 1;
																	}
																	m_Input.m_Hook = 1;
																	if(!m_FreezeTime)
																	{
																		m_Input.m_Fire++;
																		m_LatestInput.m_Fire++;
																	}
																}
															}
														}
													}
													else if(m_Dummy_mate_help_mode == 2) //new (jump and wallshot the left wall)
													{
														if(m_Core.m_Pos.y > 193 * 32 && m_Core.m_Vel.y == 0.000000f)
														{
															if(Server()->Tick() % 30 == 0)
															{
																m_Input.m_Jump = 1;
															}
														}

														if(m_Core.m_Pos.y < 191 * 32) //prepare aim
														{
															m_Input.m_TargetX = -300;
															m_Input.m_TargetY = 200;
															m_LatestInput.m_TargetX = -300;
															m_LatestInput.m_TargetY = 200;

															if(m_Core.m_Pos.y < 192 * 32 - 30) //shoot
															{
																if(m_FreezeTime == 0 && m_Core.m_ActiveWeapon == WEAPON_SHOTGUN && m_Core.m_Vel.y < -0.5f)
																{
																	m_Input.m_Fire++;
																	m_LatestInput.m_Fire++;
																}
															}
														}

														//Panic if fall of platform
														if(m_Core.m_Pos.y > 195 * 32 + 5)
														{
															m_Input.m_Jump = 1;
															m_Input.m_Direction = 1;
															m_Input.m_TargetX = 300;
															m_Input.m_TargetY = -2;
															m_LatestInput.m_TargetX = 300;
															m_LatestInput.m_TargetY = -2;
															m_Dummy_2p_panic_while_helping = true;
														}
														if((m_Core.m_Pos.x > 480 * 32 && m_FreezeTime == 0) || m_FreezeTime > 0) //stop this mode if the bot made it back to the platform or failed
														{
															m_Dummy_2p_panic_while_helping = false;
														}
														if(m_Dummy_2p_panic_while_helping)
														{
															m_Input.m_Direction = 1;
															m_Input.m_TargetX = 300;
															m_Input.m_TargetY = -2;
															m_LatestInput.m_TargetX = 300;
															m_LatestInput.m_TargetY = -2;
														}
													}
													else if(m_Dummy_mate_help_mode == 1) //old (shooting straight down from edge and try to wallshot)
													{
														m_Input.m_TargetX = 15;
														m_Input.m_TargetY = 300;
														m_LatestInput.m_TargetX = 15;
														m_LatestInput.m_TargetY = 300;
														if(m_Core.m_Vel.x > -0.1f && m_FreezeTime == 0)
														{
															m_Input.m_Fire++;
															m_LatestInput.m_Fire++;
														}

														if(m_Core.m_Pos.y > 195 * 32 + 5)
														{
															m_Input.m_Jump = 1;
															m_Input.m_Direction = 0; //old 1
															m_Input.m_TargetX = 300;
															m_Input.m_TargetY = -2;
															m_LatestInput.m_TargetX = 300;
															m_LatestInput.m_TargetY = -2;
															m_Dummy_2p_panic_while_helping = true;
														}
														if((m_Core.m_Pos.x > 480 * 32 && m_FreezeTime == 0) || m_FreezeTime > 0) //stop this mode if the bot made it back to the platform or failed
														{
															m_Dummy_2p_panic_while_helping = false;
														}
														if(m_Dummy_2p_panic_while_helping)
														{
															if(m_Core.m_Pos.y < 196 * 32 - 8)
															{
																m_Input.m_Direction = 1;
															}
															else
															{
																m_Input.m_Direction = 0;
															}
															m_Input.m_TargetX = 300;
															m_Input.m_TargetY = -2;
															m_LatestInput.m_TargetX = 300;
															m_LatestInput.m_TargetY = -2;
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
													if(m_Core.m_Jumped >= 2) //if bot has no jump
													{
														m_Input.m_Direction = 1;
													}
													else
													{
														m_Input.m_Direction = -1;

														if(m_Core.m_Pos.x < 477 * 32 && m_Core.m_Vel.x < -3.4f) //dont rush too hard intro nowehre
														{
															m_Input.m_Direction = 0;
														}

														if(m_Core.m_Pos.y > 195 * 32) //prepare aim
														{
															m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
															m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
															m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
															m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

															if(m_Core.m_Pos.y > 196 * 32 + 25 || m_Core.m_Pos.x < 475 * 32 + 15)
															{
																m_Input.m_Fire++;
																m_LatestInput.m_Fire++;
																m_Input.m_Jump = 1;
															}

															if((pChr->m_Pos.x < 486 * 32 && m_Core.m_Pos.y > 195 * 32 + 20) || (pChr->m_Pos.x < 486 * 32 && m_Core.m_Pos.x < 477 * 32)) //if mate is in range add a hook
															{
																m_Dummy_dd_helphook = true;
															}
															if(m_Core.m_Pos.x > 479 * 32)
															{
																m_Dummy_dd_helphook = false;
															}

															if(m_Dummy_dd_helphook)
															{
																m_Input.m_Hook = 1;
															}
														}
													}
												}
											}
										}
									}
								}

								if(pChr->m_Pos.x < 475 * 32) // mate failed in left tunnel
								{
									int dist = distance(pChr->m_Pos, m_Core.m_Pos);
									if(dist < 11 * 32)
									{
										m_Input.m_Hook = 1;
										m_Dummy_mate_help_mode = 3;
										if(Server()->Tick() % 100 == 0) // reset hook to not get stuck
										{
											m_Input.m_Hook = 0;
											if(IsGrounded())
											{
												m_Input.m_Jump = 1; // idk do something
											}
										}
									}
								}

								if(m_Core.m_Pos.y < pChr->m_Pos.y + 40 && pChr->m_Pos.x < 479 * 32 + 10 && m_FreezeTime == 0) //if the mate is near enough to hammer
								{
									//dont switch to hammer because without delay it sucks
									//and with delay its too slow
									//the bot shoudl anyways have a hammer ready in this situation
									// so ---> just shoot
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
								}

								//do something if nothing happens cuz the bot is stuck somehow
								if(Server()->Tick() % 100 == 0 && pChr->m_Core.m_Vel.y == 0.000000f && m_Dummy_nothing_happens_counter == 0) //if the mate stands still after 90secs the m_Dummy_nothing_happens_countershoudl get triggerd. but if not this if function turns true
								{
									//[PLANNED]: this can cause an loop wehre nothing happens..
									//maybe add some weapon changes or change m_Input.m_TargetX a bit

									m_Input.m_Direction = -1; //ye just walk untill an emergency gets called xD
										//ik pro trick but it moves the bot around
								}

								//Emergency takes over here if the bot got in a dangerous situation!
								//if (m_Core.m_Pos.y > 196 * 32 + 30) //+25 is used for the jump help and with 30 it shoudlnt get any confusuion i hope
								if((m_Core.m_Pos.y > 195 * 32 && !m_Dummy_help_no_emergency)) //if the bot left the platform
								{
									m_Dummy_help_emergency = true;
								}

								if((m_Core.m_Pos.x > 479 * 32 && m_Core.m_Jumped == 0) || isFreezed)
								{
									m_Dummy_help_emergency = false;
								}

								if(m_Dummy_help_emergency)
								{
									//resett all and let emergency control all xD
									m_Input.m_Hook = 0;
									m_Input.m_Jump = 0;
									m_Input.m_Direction = 0;
									m_LatestInput.m_Fire = 0;
									m_Input.m_Fire = 0;

									if(Server()->Tick() % 20 == 0)
									{
										GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
									}

									m_Input.m_TargetX = 0;
									m_Input.m_TargetY = -200;
									m_LatestInput.m_TargetX = 0;
									m_LatestInput.m_TargetY = -200;

									if(m_Core.m_Pos.y > 194 * 32 + 18)
									{
										m_Input.m_Jump = 1;
									}
									if(m_Core.m_Jumped >= 2)
									{
										m_Input.m_Direction = 1;
									}
								}

								if(m_Core.m_Pos.x < 475 * 32 && m_Core.m_Vel.x < -2.2f)
								{
									if(m_Core.m_Vel.y > 1.1f) // falling -> sg roof
									{
										m_Input.m_TargetX = 10;
										m_Input.m_TargetY = -120;
										if(!m_FreezeTime)
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}
									}
									if(m_Core.m_Pos.y > 193 * 32 + 18) // don't jump in the freeze roof
									{
										m_Input.m_Jump = 1;
									}
									m_Input.m_Direction = 1;
									m_Dummy_help_emergency = true;
								}
							} //dummy_mate_failed end

							if(m_Dummy_2p_state == 6) //extern af fuck the system
							{
								//m_pPlayer->m_TeeInfos.m_ColorBody = (255 * 255 / 360);

								m_Input.m_Direction = 0;
								m_Input.m_Jump = 0;
								//m_Input.m_Jump = 1;
								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								if(Server()->Tick() % 40 == 0 && m_aWeapons[2].m_Got)
								{
									SetWeapon(2);
								}

								if(m_FreezeTime == 0)
								{
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
								}
							}
						}
					}
					//Hammerhit with race mate till finish
					if(m_Dummy_mode23 == 0 || m_Dummy_mode23 == 2) //normal hammerhit
					{
						if(m_Core.m_Pos.x > 491 * 32)
						{
							if(m_Core.m_Pos.x <= 514 * 32 - 5 && pChr->m_Pos.y < 198 * 32)
							{
								SetWeapon(0);
							}

							CCharacter *pChr = GameServer()->m_World.ClosestCharTypeFreeze(m_Pos, true, this); //new 11.11.2017 Updated from ClosestCharType to TypeFreeze
							if(pChr && pChr->IsAlive())
							{
								//if (pChr->m_Pos.x > 485 * 32) //newly added this to improve the 2p_state = 5 skills (go on edge if mate made the part)
								if(pChr->m_Pos.x > 490 * 32 + 2) //newly added this to improve the 2p_state = 5 skills (go on edge if mate made the part)
								{
									m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
									m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
									m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
									m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

									//just do things if unffr
									//old shit cuz he cant rls because mate is unfreeze and dont check for later rlsing hook
									//if (m_FreezeTime == 0 && pChr->m_FreezeTime > 0) //und der mate auch freeze is (damit der nich von edges oder aus dem ziel gehookt wird)
									//                  fuck the edge only stop if finish lol
									if((m_FreezeTime == 0 && pChr->m_Pos.x < 514 * 32 - 2) || (m_FreezeTime == 0 && pChr->m_Pos.x > 521 * 32 + 2))
									{
										//get right hammer pos [rechte seite]
										if(pChr->m_Pos.x < 515 * 32) //wenn der mate links des ziels ist
										{
											if(m_Core.m_Pos.x > pChr->m_Pos.x + 45) //zu weit rechts von hammer position
											{
												m_Input.m_Direction = -1; //gehe links
											}
											else if(m_Core.m_Pos.x < pChr->m_Pos.x + 39) // zu weit links von hammer position
											{
												m_Input.m_Direction = 1;
											}
										}
										else //get right hammer pos [rechte seite] (wenn der mate rechts des ziels is)
										{
											if(m_Core.m_Pos.x > pChr->m_Pos.x - 45) //zu weit links von hammer position
											{
												m_Input.m_Direction = -1;
											}
											else if(m_Core.m_Pos.x < pChr->m_Pos.x - 39) // zu weit rechts von hammer position
											{
												m_Input.m_Direction = 1;
											}
										}

										//deactivate bool for hook if mate is high enough or bot is freezed (but freezed is checked somewerhe else)
										//                                                                                              NEW: just rls hook if mate is higher than bot (to prevent both falling added new ||)
										//                                                                                                                                                                                oder wenn der mate unter dem bot ist und unfreeze
										if((pChr->m_FreezeTime == 0 && pChr->m_Core.m_Vel.y > -1.5f && m_Core.m_Pos.y > pChr->m_Pos.y - 15) || pChr->m_Core.m_Vel.y > 3.4f || (pChr->m_FreezeTime == 0 && pChr->m_Pos.y + 38 > m_Core.m_Pos.y) || isFreezed)
										{
											m_Dummy_hh_hook = false;
										}
										//activate bool for hook if mate stands still
										if(pChr->m_Core.m_Vel.y == 0.000000f /*|| pChr->m_Core.m_Vel.y < -4.5f*/) //wenn er am boden liegt anfangen oder wenn er zu schnell nach obenfliegt bremsen
										{
											m_Dummy_hh_hook = true;
										}

										if(m_Dummy_hh_hook)
										{
											m_Input.m_Hook = 1;
										}

										//jump if too low && if mate is freeze otherwise it woudl be annoying af
										if(m_Core.m_Pos.y > 191 * 32 && pChr->m_FreezeTime > 0)
										{
											m_Input.m_Jump = 1;
										}

										//Hammer
										//if (pChr->m_Pos.y - m_Core.m_Pos.y < 7 && pChr->m_FreezeTime > 0) //wenn der abstand der beiden tees nach oben weniger is als 7 ^^
										if(pChr->m_FreezeTime > 0 && pChr->m_Pos.y - m_Core.m_Pos.y < 18) //wenn der abstand kleiner als 10 is nach oben
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}
									}
									else
									{
										m_Dummy_hh_hook = false; //reset hook if bot is freeze
									}

									//ReHook if mate flys to high
									if((pChr->m_Pos.y < m_Core.m_Pos.y - 40 && pChr->m_Core.m_Vel.y < -4.4f) || pChr->m_Pos.y < 183 * 32)
									{
										m_Input.m_Hook = 1;
									}

									//Check for panic balance cuz all went wrong lol
									//if dummy is too much left and has no dj PANIC!
									//                                                                                                                                           New Panic Trigger: if both fall fast an the bot is on top
									if((pChr->m_Pos.x < 516 * 32 && m_Core.m_Jumped >= 2 && m_Core.m_Pos.x < pChr->m_Pos.x - 5 && pChr->m_Pos.y > m_Core.m_Pos.y && m_Core.m_Pos.y > 192 * 32 && pChr->isFreezed) ||
										(m_Core.m_Vel.y > 6.7f && pChr->m_Core.m_Vel.y > 7.4f && pChr->m_Pos.y > m_Core.m_Pos.y && m_Core.m_Pos.y > 192 * 32 && pChr->m_Pos.x < 516 * 32))
									{
										if(!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "ChillerDragon") || !str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "ChillerDragon.*")) //only chatflame debug while racing with ChillerDragon
										{
											if(m_Dummy_sent_chat_msg == 0 && !m_Dummy_panic_balance && m_FreezeTime == 0)
											{
												int r = rand() % 16; // 0-15

												if(r == 0)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "YOU SUCK LOL!");
												}
												else if(r == 1)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "what do you do?!");
												}
												else if(r == 2)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "O M G =!! what r u triin mate?");
												}
												else if(r == 3)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "you did shit. i didnt do shit because im a bot and i am perfect.");
												}
												else if(r == 4)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "what was your plan?");
												}
												else if(r == 5)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "this looks bad! i try to balance...");
												}
												else if(r == 6)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "i think we gonna die ....  lemme try to balance");
												}
												else if(r == 7)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "what was this?");
												}
												else if(r == 8)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "you fucked it up .. let me try to save us with my balance skills.");
												}
												else if(r == 9)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "do you have lags? i dont have lags.");
												}
												else if(r == 10)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "lol.");
												}
												else if(r == 11)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "lul");
												}
												else if(r == 12)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "wtf?");
												}
												else if(r == 13)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "OMG");
												}
												else if(r == 14)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "?!");
												}
												else if(r == 15)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "whats going on here?!");
												}

												//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 4);
												m_Dummy_sent_chat_msg = 1;
											}
										}

										m_Dummy_panic_balance = true;
									}
									if(pChr->m_FreezeTime == 0 || isFreezed || pChr->m_Pos.x > 512 * 32 + 5) //if mate gets unfreezed or dummy freezed stop balance
									{
										m_Dummy_panic_balance = false;
									}

									if(m_Dummy_panic_balance)
									{
										if(m_Core.m_Pos.x < pChr->m_Pos.x - 2) //Bot is too far left
										{
											m_Input.m_Direction = 1;
										}
										else if(m_Core.m_Pos.x > pChr->m_Pos.x) //Bot is too far right
										{
											m_Input.m_Direction = -1;
										}

										if(m_Core.m_Pos.x > pChr->m_Pos.x - 2 && m_Core.m_Pos.x < pChr->m_Pos.x && m_Core.m_Vel.x > -0.3f && m_FreezeTime == 0)
										{
											m_Input.m_Direction = 1;
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}
									}

									//Go in finish if near enough
									if((m_Core.m_Vel.y < 4.4f && m_Core.m_Pos.x > 511 * 32) || (m_Core.m_Vel.y < 8.4f && m_Core.m_Pos.x > 512 * 32))
									{
										if(m_Core.m_Pos.x < 514 * 32 && !m_Dummy_panic_balance)
										{
											m_Input.m_Direction = 1;
										}
									}

									//If dummy made it till finish but mate is still freeze on the left side
									//he automaiclly help. BUT if he fails the hook resett it!
									//left side                                                                                      right side
									if((m_Core.m_Pos.x > 514 * 32 - 5 && m_FreezeTime == 0 && pChr->isFreezed && pChr->m_Pos.x < 515 * 32) || (m_Core.m_Pos.x > 519 * 32 - 5 && m_FreezeTime == 0 && pChr->isFreezed && pChr->m_Pos.x < 523 * 32))
									{
										if(Server()->Tick() % 70 == 0)
										{
											m_Input.m_Hook = 0;
											//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
										}
									}
									//if mate is too far for hook --> shotgun him
									if(m_Core.m_Pos.x > 514 * 32 - 5 && m_Core.m_Pos.x > pChr->m_Pos.x && m_Core.m_Pos.x - pChr->m_Pos.x > 8 * 32)
									{
										SetWeapon(2); //shotgun
										if(pChr->m_FreezeTime > 0 && m_FreezeTime == 0 && pChr->m_Core.m_Vel.y == 0.000000f)
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}
									}
									//another hook if normal hook doesnt work
									//to save mate if bot is finish
									if(m_Input.m_Hook == 0)
									{
										if(pChr->m_FreezeTime > 0 && m_FreezeTime == 0 && m_Core.m_Pos.y < pChr->m_Pos.y - 60 && m_Core.m_Pos.x > 514 * 32 - 5)
										{
											m_Input.m_Hook = 1;
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
											if(Server()->Tick() % 10 == 0)
											{
												GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
											}
										}
									}

									//Important dont walk of finish plattform check
									//if (m_Core.m_Vel.y < 6.4f) //Check if not falling to fast
									if(!m_Dummy_panic_balance)
									{
										if((m_Core.m_Vel.y < 6.4f && m_Core.m_Pos.x > 512 * 32 && m_Core.m_Pos.x < 515 * 32) || (m_Core.m_Pos.x > 512 * 32 + 30 && m_Core.m_Pos.x < 515 * 32)) //left side
										{
											m_Input.m_Direction = 1;
										}

										if(m_Core.m_Vel.y < 6.4f && m_Core.m_Pos.x > 520 * 32 && m_Core.m_Pos.x < 524 * 32 /* || too lazy rarly needed*/) //right side
										{
											m_Input.m_Direction = -1;
										}
									}
								}
							}
						}
					}
					else if(m_Dummy_mode23 == 1) //tricky hammerhit (harder)
					{
						if(m_Core.m_Pos.x > 491 * 32)
						{
							SetWeapon(0);
							CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
							if(pChr && pChr->IsAlive())
							{
								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								//just do things if unffr
								if(m_FreezeTime == 0 && pChr->m_FreezeTime > 0) //und der mate auch freeze is (damit der nich von edges oder aus dem ziel gehookt wird)
								{
									//get right hammer pos [rechte seite]
									if(m_Core.m_Pos.x > pChr->m_Pos.x + 45)
									{
										m_Input.m_Direction = -1;
									}
									else if(m_Core.m_Pos.x < pChr->m_Pos.x + 39)
									{
										m_Input.m_Direction = 1;
									}

									//deactivate bool for hook if mate is high enough or bot is freezed (but freezed is checked somewerhe else)
									//                                                                                              NEW: just rls hook if mate is higher than bot (to prevent both falling added new ||)
									if(/*m_Core.m_Pos.y - pChr->m_Pos.y > 15 ||*/ (pChr->m_FreezeTime == 0 && pChr->m_Core.m_Vel.y < -2.5f && pChr->m_Pos.y < m_Core.m_Pos.y) || pChr->m_Core.m_Vel.y > 3.4f)
									{
										m_Dummy_hh_hook = false;
										//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
									}
									//activate bool for hook if mate stands still
									if(pChr->m_Core.m_Vel.y == 0.000000f) //wenn er am boden liegt anfangen oder wenn er zu schnell nach obenfliegt bremsen
									{
										m_Dummy_hh_hook = true;
									}

									//jump if too low && if mate is freeze otherwise it woudl be annoying af
									if(m_Core.m_Pos.y > 191 * 32 && pChr->m_FreezeTime > 0)
									{
										m_Input.m_Jump = 1;
									}

									//Hammer
									//wenn der abstand der beiden tees nach oben weniger is als 7 ^^
									if(pChr->m_FreezeTime > 0 && pChr->m_Pos.y - m_Core.m_Pos.y < 18) //wenn der abstand kleiner als 10 is nach oben
									{
										m_Input.m_Fire++;
										m_LatestInput.m_Fire++;
									}
								}
								else
								{
									m_Dummy_hh_hook = false; //reset hook if bot is freeze
										//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
								}
							}
						}

						if(m_Dummy_hh_hook)
						{
							m_Input.m_Hook = 1;
						}
					}
				}

				//General bug protection resett hook in freeze
				//if (isFreezed)
				if(m_FreezeTime > 0)
				{
					m_Input.m_Hook = 0; //resett hook in freeze to prevent bugs with no hooking at last part
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
		else if(m_pPlayer->m_DummyMode == 25) //ChillerDraguns testy fake nural network lol (2.0 revived)
		{
			/*
			######################################################
			#     DummyMode 25      The    FNN    Mode           #
			#           [FNN] == [FAKENEURALNETWORK]             #
			######################################################
			ModeStructure:
			
			* Spawn -> goto hardcodet spawn pos.x 353 * 32
			
			* Check for human interactions and save them in the var m_Dummy_nn_touched_by_humans
	
			*/
			char aBuf[256];

			if(m_pPlayer->m_dmm25 == -2) //stopped
			{
				m_Input.m_Hook = 0;
				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;
				m_pPlayer->m_TeeInfos.m_ColorBody = (180 * 255 / 260);
			}
			else if(!m_Dummy_nn_ready) //first get the right start pos
			{
				m_Input.m_Hook = 0;
				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;

				if(m_Core.m_Pos.x > g_Config.m_SvFNNstartX * 32)
				{
					if(IsGrounded()) //only walk on ground because air is unpredictable
					{
						if(m_Core.m_Pos.x < g_Config.m_SvFNNstartX * 32 + 10) //in 1tile nähe langsam laufen
						{
							if(m_Core.m_Vel.x > -0.009f)
							{
								m_Input.m_Direction = -1;
							}
						}
						else
						{
							m_Input.m_Direction = -1;
						}
					}
				}
				else if(m_Core.m_Pos.x < g_Config.m_SvFNNstartX * 32)
				{
					if(IsGrounded()) //only walk on ground because air is unpredictable
					{
						if(m_Core.m_Pos.x > g_Config.m_SvFNNstartX * 32 - 10) //in 1tile nähe langsam laufen
						{
							if(m_Core.m_Vel.x < 0.009f)
							{
								m_Input.m_Direction = 1;
							}
						}
						else
						{
							m_Input.m_Direction = 1;
						}
					}
				}
				else //correct position
				{
					if(IsGrounded()) //only start on ground because air is unpredictable
					{
						m_Dummy_nn_ready = true;
						m_StartPos = m_Core.m_Pos;
						dbg_msg("FNN", "Found start position (%.2f/%.2f) -> starting process", m_Core.m_Pos.x / 32, m_Core.m_Pos.y / 32);
					}
				}

				//Catch errors
				if(m_Dummy_nn_ready_time > 300)
				{
					GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", "Starting process failed. Get in start position took too long -> restarting...");
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}
				m_Dummy_nn_ready_time++;
				//char aBuf[256];
				//str_format(aBuf, sizeof(aBuf), "time: %d", m_Dummy_nn_ready_time);
				//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", aBuf);
			}
			else //if the bot has the right start pos start doing random stuff
			{
				/*


				dummy sub mode structure:

				old used a bool (m_Dummy_nn_write) which has to be changed manually in the source.

				new system uses chat command /dmm25 = dummmymodemode25 to choose submodes.
				submodes:
				-2					stop all
				-1					error/offline

				0					write
				1					read/load highest distance
				2					read/load highest fitness
				3					read/load lowest distance_finish

				4					play loaded run

				*/

				//int m_aMoveID = -1;
				//int Hooked = false;
				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					CCharacter *pChar = GameServer()->GetPlayerChar(i);

					if(!pChar || !pChar->IsAlive() || pChar == this)
						continue;

					if(pChar->Core()->m_HookedPlayer == m_pPlayer->GetCID())
					{
						//Hooked = true;
						//m_aMoveID = i;
						m_Dummy_nn_touched_by_humans = true;
						GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "[FNN] DONT TOUCH ME HOOK WTF");
					}

					if(Core()->m_HookedPlayer != -1)
					{
						str_format(aBuf, sizeof(aBuf), "[FNN] dont get in my hook %s", Server()->ClientName(Core()->m_HookedPlayer));
						GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, aBuf);
					}
				}
				//if (m_Core.m_HookState == HOOK_GRABBED) //this includes normal collision hooks
				//{
				//	m_Dummy_nn_touched_by_humans = true;
				//	GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "[FNN] dont get in my hook -.-");
				//}
				//selfmade noob code check if pChr is too near and coudl touched the bot
				CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true);
				if(pChr && pChr->IsAlive() && pChr != this)
				{
					if(pChr->m_Pos.x < m_Core.m_Pos.x + 60 && pChr->m_Pos.x > m_Core.m_Pos.x - 60 && pChr->m_Pos.y < m_Core.m_Pos.y + 60 && pChr->m_Pos.y > m_Core.m_Pos.y - 60)
					{
						m_Dummy_nn_touched_by_humans = true;
						GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "[FNN] dont touch my body with yours pls");
					}
				}

				//always set to black
				m_pPlayer->m_TeeInfos.m_ColorBody = (0 * 255 / 360);

				if(m_pPlayer->m_dmm25 == -1) //error
				{
					m_Input.m_Hook = 0;
					m_Input.m_Jump = 0;
					m_Input.m_Direction = 0;
					m_LatestInput.m_Fire = 0;
					m_Input.m_Fire = 0;
					m_pPlayer->m_TeeInfos.m_ColorBody = (180 * 255 / 260);
				}
				else if(m_pPlayer->m_dmm25 == 0) //submode[0] write
				{
					// m_pPlayer->m_TeeInfos.m_Name = "writing...";
					// m_pPlayer->m_TeeInfos.m_ColorBody = (180 * 255 / 360);

					//m_pPlayer->m_Dummy_nn_time++; //maybe use it some day to analys each tick stuff or total trainign time idk

					// random inputs
					int rand_Fire = rand() % 2; // 1 0
					int rand_Jump = 0;
					if(rand() % 32 - (IsGrounded() * 6) == 0) // more likley to jump if grounded to avoid spamming dj
					{
						rand_Jump = 1;
					}
					int rand_Hook = rand() % 2;
					int rand_Weapon = rand() % 4;
					int rand_TargetX = rand() % 401 - 200;
					int rand_TargetY = rand() % 401 - 200;
					static int rand_Direction = rand() % 3 - 1; //-1 0 1
					if(Server()->Tick() % 77 == 0)
					{
						rand_Direction = rand() % 3 - 1; //-1 0 1
						if(!(rand() % 3 == 0)) // increase chance of walking towards finish
						{
							// dbg_msg("fnn", "finish: %.2f pos: %.2f", GameServer()->m_FinishTilePos.x, m_Core.m_Pos.x / 32);
							if(GameServer()->m_FinishTilePos.x > m_Core.m_Pos.x / 32)
							{
								rand_Direction = 1;
							}
							else
							{
								rand_Direction = -1;
							}
						}
					}

					m_Input.m_Direction = rand_Direction;
					m_Input.m_Jump = rand_Jump;
					m_Input.m_Hook = rand_Hook;
					m_Input.m_TargetX = rand_TargetX;
					m_Input.m_TargetY = rand_TargetY;
					m_LatestInput.m_TargetX = rand_TargetX;
					m_LatestInput.m_TargetY = rand_TargetY;

					// read world inputs
					float Offset = 16.0f;
					int MapIndexL = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x - (m_ProximityRadius / 2) - Offset, m_Pos.y));
					int MapIndexR = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x + (m_ProximityRadius / 2) + Offset, m_Pos.y));
					int MapIndexB = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x, m_Pos.y + (m_ProximityRadius / 2) + Offset));
					int MapIndexT = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x, m_Pos.y - (m_ProximityRadius / 2) - Offset));
					//dbg_msg("","N%d L%d R%d B%d T%d",MapIndex,MapIndexL,MapIndexR,MapIndexB,MapIndexT);
					int TileIndexL = GameServer()->Collision()->GetTileIndex(MapIndexL);
					// int TileFlagsL = GameServer()->Collision()->GetTileFlags(MapIndexL);
					int TileIndexR = GameServer()->Collision()->GetTileIndex(MapIndexR);
					// int TileFlagsR = GameServer()->Collision()->GetTileFlags(MapIndexR);
					int TileIndexB = GameServer()->Collision()->GetTileIndex(MapIndexB);
					// int TileFlagsB = GameServer()->Collision()->GetTileFlags(MapIndexB);
					int TileIndexT = GameServer()->Collision()->GetTileIndex(MapIndexT);

					// if (Server()->Tick() % 100 == 0)
					// {
					// 	dbg_msg("fnn-debug", "------ TEST --------");
					// 	dbg_msg("fnn-debug", "left: %d mapindex: %d", TileIndexL, MapIndexL);
					// 	dbg_msg("fnn-debug", "right: %d mapindex: %d", TileIndexR, MapIndexR);
					dbg_msg("fnn-debug", "up: %d mapindex: %d", TileIndexT, MapIndexT);
					// 	dbg_msg("fnn-debug", "down: %d mapindex: %d", TileIndexB, MapIndexB);
					// }

					if(TileIndexL == TILE_FREEZE)
					{
						m_Input.m_Direction = 1;
						m_pPlayer->m_TeeInfos.m_ColorBody = (200 * 255 / 1);
					}
					else if(TileIndexR == TILE_FREEZE)
					{
						m_Input.m_Direction = -1;
						m_pPlayer->m_TeeInfos.m_ColorBody = (200 * 255 / 1);
					}
					else
					{
						m_Input.m_Direction = rand_Direction;
					}
					if(TileIndexB == TILE_FREEZE)
					{
						m_Input.m_Jump = 1;
						m_pPlayer->m_TeeInfos.m_ColorBody = (200 * 255 / 1);
					}

					if(TileIndexB == TILE_NOHOOK)
					{
						m_pPlayer->m_TeeInfos.m_ColorBody = (100 * 255 / 1); // light red
					}
					// m_Input.m_Direction = 0;
					// m_Input.m_Hook = 0;
					// m_Input.m_Jump = 0;
					// rand_Fire = 0;

					if(m_FNN_CurrentMoveIndex == 0)
					{
						m_FNN_start_servertick = Server()->Tick();
						dbg_msg("FNN", "starting record on x=%f y=%f servertick=%d", m_Core.m_Pos.x, m_Core.m_Pos.y, m_FNN_start_servertick);
					}

					//save values in array
					m_aRecMove[m_FNN_CurrentMoveIndex] = m_Input.m_Direction;
					// dbg_msg("fnn", "dir: %d", m_Input.m_Direction);
					m_FNN_CurrentMoveIndex++;
					m_aRecMove[m_FNN_CurrentMoveIndex] = m_Input.m_Jump;
					// dbg_msg("fnn", "jump: %d", m_Input.m_Jump);
					m_FNN_CurrentMoveIndex++;
					m_aRecMove[m_FNN_CurrentMoveIndex] = m_Input.m_Hook;
					// dbg_msg("fnn", "hook: %d", m_Input.m_Hook);
					m_FNN_CurrentMoveIndex++;
					m_aRecMove[m_FNN_CurrentMoveIndex] = m_Input.m_TargetX;
					// dbg_msg("fnn", "targetX: %d", m_Input.m_TargetX);
					m_FNN_CurrentMoveIndex++;
					m_aRecMove[m_FNN_CurrentMoveIndex] = m_Input.m_TargetY;
					// dbg_msg("fnn", "targetY: %d", m_Input.m_TargetY);
					m_FNN_CurrentMoveIndex++;

					if(rand_Weapon == 0)
					{
						SetWeapon(0); //hammer
					}
					else if(rand_Weapon == 1)
					{
						SetWeapon(1); //gun
					}
					else if(rand_Weapon == 2)
					{
						if(m_aWeapons[WEAPON_SHOTGUN].m_Got)
						{
							SetWeapon(2); //shotgun
						}
					}
					else if(rand_Weapon == 3)
					{
						if(m_aWeapons[WEAPON_GRENADE].m_Got)
						{
							SetWeapon(3); //grenade
						}
					}
					else if(rand_Weapon == 4)
					{
						if(m_aWeapons[WEAPON_LASER].m_Got)
						{
							SetWeapon(4); //laser
						}
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "Error unknown weapon: %d", rand_Weapon);
						GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", aBuf);
					}
					if(rand_Fire == 1 && m_FreezeTime == 0)
					{
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
					}

					if(m_FNN_CurrentMoveIndex > FNN_MOVE_LEN - 12) //minus inputs because every tick the index gets incremented by 5
					{
						float newest_distance = distance(m_StartPos, m_Core.m_Pos);
						vec2 current_pos(0, 0);
						current_pos.x = m_Core.m_Pos.x / 32;
						current_pos.y = m_Core.m_Pos.y / 32;
						float newest_distance_finish = distance(current_pos, GameServer()->m_FinishTilePos);
						float newest_fitness = newest_distance_finish / m_FNN_CurrentMoveIndex;
						str_format(aBuf, sizeof(aBuf), "[FNN] ran out of memory ticks=%d distance=%.2f fitness=%.2f distance_finish=%.2f", m_FNN_CurrentMoveIndex, newest_distance, newest_fitness, newest_distance_finish);
						GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, aBuf);
						//Die(m_pPlayer->GetCID(), WEAPON_SELF);
						m_Dummy_nn_stop = true;
					}

					if(g_Config.m_SvFNNtimeout && !m_Dummy_nn_stop)
					{
						if(m_FNN_CurrentMoveIndex > g_Config.m_SvFNNtimeout)
						{
							str_format(aBuf, sizeof(aBuf), "[FNN] timeouted after ticks=%d", m_FNN_CurrentMoveIndex);
							GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, aBuf);
							m_Dummy_nn_stop = true;
						}
					}

					if((m_Core.m_Vel.y == 0.000000f && m_Core.m_Vel.x < 0.01f && m_Core.m_Vel.x > -0.01f && isFreezed) || m_Dummy_nn_stop)
					{
						if(Server()->Tick() % 10 == 0)
						{
							GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3);
						}
						if(Server()->Tick() % 40 == 0)
						{
							GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", "======================================");
							if(m_Dummy_nn_touched_by_humans)
							{
								str_format(aBuf, sizeof(aBuf), "Failed at (%.2f/%.2f) --> RESTARTING", m_Core.m_Pos.x / 32, m_Core.m_Pos.y / 32);
								GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", aBuf);
								GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", "Got touched by Humans --> DELETE RUN");
								Die(m_pPlayer->GetCID(), WEAPON_SELF);
							}
							else //no interaction with humans --> save normal
							{
								str_format(aBuf, sizeof(aBuf), "Failed at (%.2f/%.2f) --> RESTARTING", m_Core.m_Pos.x / 32, m_Core.m_Pos.y / 32);
								m_FNN_stop_servertick = Server()->Tick();
								dbg_msg("FNN", "stop servertick=%d totaltickdiff=%d", m_FNN_stop_servertick, m_FNN_stop_servertick - m_FNN_start_servertick);
								GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", aBuf);
								//Die(m_pPlayer->GetCID(), WEAPON_SELF); //moved to the end because we use character variables but im not sure.. maybe it had a sense its here

								//prepare data
								if(m_FNN_CurrentMoveIndex > FNN_MOVE_LEN) //dont overload arraylength
								{
									m_FNN_CurrentMoveIndex = FNN_MOVE_LEN;
								}

								float newest_distance = distance(m_StartPos, m_Core.m_Pos);
								vec2 current_pos(0, 0);
								current_pos.x = m_Core.m_Pos.x / 32;
								current_pos.y = m_Core.m_Pos.y / 32;
								float newest_distance_finish = distance(current_pos, GameServer()->m_FinishTilePos);
								float newest_fitness = newest_distance_finish / m_FNN_CurrentMoveIndex;
								dbg_msg("FNN", "distance=%.2f", newest_distance);
								dbg_msg("FNN", "moveticks=%d", m_FNN_CurrentMoveIndex);
								dbg_msg("FNN", "fitness=%.2f", newest_fitness);
								dbg_msg("FNN", "distance_finish=%.2f", newest_distance_finish);

								/***************************************
								*                                      *
								*                                      *
								*         D I S T A N C E              *
								*                                      *
								*                                      *
								****************************************/

								if(newest_distance > GameServer()->m_FNN_best_distance)
								{
									//saving the distance
									// dbg_msg("FNN","new distance highscore Old=%.2f -> New=%.2f", GameServer()->m_FNN_best_distance, newest_distance);
									str_format(aBuf, sizeof(aBuf), "[FNN] new distance highscore Old=%.2f -> New=%.2f", GameServer()->m_FNN_best_distance, newest_distance);
									GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, aBuf);
									GameServer()->m_FNN_best_distance = newest_distance;
									std::ofstream statsfile;
									char aFilePath[512];
									str_format(aFilePath, sizeof(aFilePath), "FNN/move_stats.fnn");
									statsfile.open(aFilePath);
									if(statsfile.is_open())
									{
										// statsfile << "-- total stats --";
										// statsfile << std::endl;
										statsfile << newest_distance; //distance
										statsfile << std::endl;
										statsfile << GameServer()->m_FNN_best_fitness; //fitness
										statsfile << std::endl;
										statsfile << GameServer()->m_FNN_best_distance_finish; //distance_finish
										statsfile << std::endl;
									}
									else
									{
										dbg_msg("FNN", "failed to update stats. failed to open file '%s'", aFilePath);
									}
									statsfile.close();

									//saving the run
									std::ofstream savefile;
									str_format(aFilePath, sizeof(aFilePath), "FNN/move_distance.fnn");
									savefile.open(aFilePath /*, std::ios::app*/); //dont append rewrite
									if(savefile.is_open())
									{
										//first five lines are stats
										savefile << "-- stats distance --";
										savefile << std::endl;
										savefile << m_FNN_CurrentMoveIndex; //moveticks
										savefile << std::endl;
										savefile << newest_distance; //distance
										savefile << std::endl;
										savefile << newest_fitness; //fitness
										savefile << std::endl;
										savefile << newest_distance_finish; //distance_finish
										savefile << std::endl;

										for(int i = 0; i < m_FNN_CurrentMoveIndex; i++)
										{
											savefile << m_aRecMove[i];
											savefile << std::endl;
										}
									}
									else
									{
										dbg_msg("FNN", "failed to save record. failed to open file '%s'", aFilePath);
									}
									savefile.close();
								}

								/***************************************
								*                                      *
								*                                      *
								*         F I T T N E S S              *
								*                                      *
								*                                      *
								****************************************/

								if(newest_fitness > GameServer()->m_FNN_best_fitness)
								{
									//saving the fitness
									// dbg_msg("FNN", "new fitness highscore Old=%.2f -> New=%.2f", GameServer()->m_FNN_best_fitness, newest_fitness);
									str_format(aBuf, sizeof(aBuf), "[FNN] new fitness highscore Old=%.2f -> New=%.2f", GameServer()->m_FNN_best_fitness, newest_fitness);
									GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, aBuf);
									GameServer()->m_FNN_best_fitness = newest_fitness;
									std::ofstream statsfile;
									char aFilePath[512];
									str_format(aFilePath, sizeof(aFilePath), "FNN/move_stats.fnn");
									statsfile.open(aFilePath);
									if(statsfile.is_open())
									{
										// statsfile << "-- total stats --";
										// statsfile << std::endl;
										statsfile << GameServer()->m_FNN_best_distance; //distance
										statsfile << std::endl;
										statsfile << newest_fitness; //fitness
										statsfile << std::endl;
										statsfile << GameServer()->m_FNN_best_distance_finish; //distance_finish
										statsfile << std::endl;
									}
									else
									{
										dbg_msg("FNN", "failed to update stats. failed to open file '%s'", aFilePath);
									}
									statsfile.close();

									//saving the run
									std::ofstream savefile;
									str_format(aFilePath, sizeof(aFilePath), "FNN/move_fitness.fnn");
									savefile.open(aFilePath);
									if(savefile.is_open())
									{
										//first five lines are stats
										savefile << "-- stats fitness --";
										savefile << std::endl;
										savefile << m_FNN_CurrentMoveIndex; //moveticks
										savefile << std::endl;
										savefile << newest_distance; //distance
										savefile << std::endl;
										savefile << newest_fitness; //fitness
										savefile << std::endl;
										savefile << newest_distance_finish; //distance_finish
										savefile << std::endl;

										for(int i = 0; i < m_FNN_CurrentMoveIndex; i++)
										{
											savefile << m_aRecMove[i];
											savefile << std::endl;
										}
									}
									else
									{
										dbg_msg("FNN", "failed to save record. failed to open file '%s'", aFilePath);
									}
									savefile.close();
								}

								/***************************************
								*                                      *
								*                                      *
								*         D I S T A N C E              *
								*         F I N I S H                  *
								*                                      *
								****************************************/

								if(newest_distance_finish < GameServer()->m_FNN_best_distance_finish)
								{
									//saving the distance_finish
									// dbg_msg("FNN", "new distance_finish highscore Old=%.2f -> New=%.2f", GameServer()->m_FNN_best_distance_finish, newest_distance_finish);
									str_format(aBuf, sizeof(aBuf), "[FNN] new distance_finish highscore Old=%.2f -> New=%.2f", GameServer()->m_FNN_best_distance_finish, newest_distance_finish);
									GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, aBuf);
									GameServer()->m_FNN_best_distance_finish = newest_distance_finish;
									std::ofstream statsfile;
									char aFilePath[512];
									str_format(aFilePath, sizeof(aFilePath), "FNN/move_stats.fnn");
									statsfile.open(aFilePath);
									if(statsfile.is_open())
									{
										// statsfile << "-- total stats --";
										// statsfile << std::endl;
										statsfile << GameServer()->m_FNN_best_distance; //distance
										statsfile << std::endl;
										statsfile << GameServer()->m_FNN_best_fitness; //fitness
										statsfile << std::endl;
										statsfile << newest_distance_finish; //distance_finish
										statsfile << std::endl;
									}
									else
									{
										dbg_msg("FNN", "failed to update stats. failed to open file '%s'", aFilePath);
									}
									statsfile.close();

									//saving the run
									std::ofstream savefile;
									str_format(aFilePath, sizeof(aFilePath), "FNN/move_distance_finish.fnn");
									savefile.open(aFilePath);
									if(savefile.is_open())
									{
										//first five lines are stats
										savefile << "-- stats distance finish --";
										savefile << std::endl;
										savefile << m_FNN_CurrentMoveIndex; //moveticks
										savefile << std::endl;
										savefile << newest_distance; //distance
										savefile << std::endl;
										savefile << newest_fitness; //fitness
										savefile << std::endl;
										savefile << newest_distance_finish; //distance_finish
										savefile << std::endl;

										for(int i = 0; i < m_FNN_CurrentMoveIndex; i++)
										{
											savefile << m_aRecMove[i];
											savefile << std::endl;
										}
									}
									else
									{
										dbg_msg("FNN", "failed to save record. failed to open file '%s'", aFilePath);
									}
									savefile.close();
								}

								m_FNN_CurrentMoveIndex = 0;
								Die(m_pPlayer->GetCID(), WEAPON_SELF);
							}
							GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", "======================================");
						}
					}
				}
				else if(m_pPlayer->m_dmm25 == 1) //submode[1] read/load distance
				{
					GameServer()->FNN_LoadRun("FNN/move_distance.fnn", m_pPlayer->GetCID());
				}
				else if(m_pPlayer->m_dmm25 == 2) //submode[2] read/load fitness
				{
					GameServer()->FNN_LoadRun("FNN/move_fitness.fnn", m_pPlayer->GetCID());
				}
				else if(m_pPlayer->m_dmm25 == 3) //submode[3] read/load lowest distance_finish
				{
					GameServer()->FNN_LoadRun("FNN/move_distance_finish.fnn", m_pPlayer->GetCID());
				}
				else if(m_pPlayer->m_dmm25 == 4) //submode[4] play loaded run
				{
					if(m_FNN_CurrentMoveIndex == 0)
					{
						m_FNN_start_servertick = Server()->Tick();
						dbg_msg("FNN", "starting play on x=%f y=%f servertick=%d", m_Core.m_Pos.x, m_Core.m_Pos.y, m_FNN_start_servertick);
					}

					m_Input.m_Direction = m_aRecMove[m_FNN_CurrentMoveIndex];
					// dbg_msg("fnn", "dir: %d", m_aRecMove[m_FNN_CurrentMoveIndex]);
					m_FNN_CurrentMoveIndex++;
					m_Input.m_Jump = m_aRecMove[m_FNN_CurrentMoveIndex];
					// dbg_msg("fnn", "jump: %d", m_aRecMove[m_FNN_CurrentMoveIndex]);
					m_FNN_CurrentMoveIndex++;
					m_Input.m_Hook = m_aRecMove[m_FNN_CurrentMoveIndex];
					// dbg_msg("fnn", "hook: %d", m_aRecMove[m_FNN_CurrentMoveIndex]);
					m_FNN_CurrentMoveIndex++;
					m_Input.m_TargetX = m_aRecMove[m_FNN_CurrentMoveIndex];
					// dbg_msg("fnn", "targetX: %d", m_aRecMove[m_FNN_CurrentMoveIndex]);
					m_FNN_CurrentMoveIndex++;
					m_Input.m_TargetY = m_aRecMove[m_FNN_CurrentMoveIndex];
					// dbg_msg("fnn", "targetY: %d", m_aRecMove[m_FNN_CurrentMoveIndex]);
					m_FNN_CurrentMoveIndex++;

					// ignore latest input for now
					// m_LatestInput.m_TargetX = m_aRecMove[m_FNN_CurrentMoveIndex];
					// dbg_msg("fnn", "r: %d", m_aRecMove[m_FNN_CurrentMoveIndex]);
					// m_FNN_CurrentMoveIndex++;
					// m_LatestInput.m_TargetY = m_aRecMove[m_FNN_CurrentMoveIndex];
					// dbg_msg("fnn", "r: %d", m_aRecMove[m_FNN_CurrentMoveIndex]);
					// m_FNN_CurrentMoveIndex++;

					if(m_FNN_CurrentMoveIndex >= m_FNN_ticks_loaded_run)
					{
						m_pPlayer->m_dmm25 = -1; //stop bot
						float newest_distance = distance(m_StartPos, m_Core.m_Pos);
						vec2 current_pos(0, 0);
						current_pos.x = m_Core.m_Pos.x / 32;
						current_pos.y = m_Core.m_Pos.y / 32;
						float newest_distance_finish = distance(current_pos, GameServer()->m_FinishTilePos);
						float newest_fitness = newest_distance_finish / m_FNN_CurrentMoveIndex;
						m_FNN_stop_servertick = Server()->Tick();
						dbg_msg("FNN", "stop servertick=%d totaltickdiff=%d", m_FNN_stop_servertick, m_FNN_stop_servertick - m_FNN_start_servertick);
						dbg_msg("FNN", "distance=%.2f", newest_distance);
						dbg_msg("FNN", "moveticks=%d", m_FNN_CurrentMoveIndex);
						dbg_msg("FNN", "fitness=%.2f", newest_fitness);
						dbg_msg("FNN", "distance_finish=%.2f", newest_distance_finish);
						str_format(aBuf, sizeof(aBuf), "[FNN] finished replay ticks=%d distance=%.2f fitness=%.2f distance_finish=%.2f", m_FNN_CurrentMoveIndex, newest_distance, newest_fitness, newest_distance_finish);
						GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, aBuf);
					}
				}
			}
		}
		else if(m_pPlayer->m_DummyMode == 27) //BlmapChill police
		{
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			//selfkills
			if(m_FreezeTime)
			{
				if(m_Core.m_Pos.y < 338 * 32) //upper map area all over the bottom floor and pvp arena
				{
					if(Server()->Tick() % 20 == 0)
					{
						if(m_Core.m_Vel.y == 0.000f && m_Core.m_Vel.x < 0.02f && m_Core.m_Vel.x > -0.02f)
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}
					}
				}
				else if(m_Core.m_Pos.y < 376 * 32) //pvp arena and jail entry
				{
					if(Server()->Tick() % 300 == 0)
					{
						if(IsGrounded())
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}
					}
				}
			}
			if(m_Core.m_Pos.x > 448 * 32 && m_Core.m_Pos.x < 480 * 32 && m_Core.m_Pos.y > 60 * 32 && m_Core.m_Pos.y < 85 * 32) //the whole spawn area
			{
				if(m_pPlayer->m_DummyModeSpawn == 32)
				{
					m_pPlayer->m_DummyMode = 32;
				}
				m_Input.m_Direction = -1;
				if(((m_Core.m_Vel.y > 0.01f) || //jump when falling down
					   (m_Core.m_Vel.x > -0.1f && IsGrounded() && Server()->Tick() % 26 == 0)) && //getting stuck
					!(m_Core.m_Pos.x < 469 * 32)) //but not if too far left (at spawn spawn and before jump <--)
				{
					m_Input.m_Jump = 1;
				}
				if(m_Core.m_Pos.y < 75 * 32) //upper area above spawn
				{
					//aim on floor to get speed
					m_Input.m_TargetX = -90;
					m_Input.m_TargetY = 100;

					if(m_Core.m_Pos.x < 459 * 32) //jump through the freeze to fall of down the left side
					{
						m_Input.m_Jump = 1;
					}
					if(m_Core.m_Pos.x < 458 * 32 && m_Core.m_Pos.y < 71 * 32 - 10) //last boost hook before jump
					{
						m_Input.m_Hook = 1;
					}
				}

				m_Dummy27_loved_x = 0; //clean brain at spawn
				m_Dummy27_speed = 70;
			}

			/*
			//bad code hooking from the left side
			if (m_Core.m_Pos.x > 451 * 32 && m_Core.m_Pos.x < 473 * 32 + 10 && m_Core.m_Pos.y > 77 * 32 && m_Core.m_Pos.y < 85 * 32) //spawn cave
			{
				//prepare for the up swing
				m_Input.m_TargetX = 86;
				m_Input.m_TargetY = -200;

				m_Input.m_Direction = 1; //walk --> at spawn
				if (m_Core.m_Pos.x > 468 * 32 && IsGrounded()) //jump if far enough -->
				{
					m_Input.m_Jump = 1;
				}

				if (m_Core.m_Pos.x > 469 * 32 && m_Core.m_Pos.y > 80 * 32 + 10) //far enough --> swing hook up
				{
					m_Input.m_Hook = 1;
				}
			}
			*/

			//hook from the right side
			if(m_Core.m_Pos.x > 451 * 32 && m_Core.m_Pos.x < 500 * 32 && m_Core.m_Pos.y > 77 * 32 && m_Core.m_Pos.y < 85 * 32) //spawn cave + a lot space to the left (until the entry to the other cave)
			{
				//prepare for the up swing
				m_Input.m_TargetX = -90;
				m_Input.m_TargetY = -170;
				if(m_Core.m_Pos.x > 474 * 32)
				{
					m_Input.m_TargetX = -90;
					m_Input.m_TargetY = -140;
				}

				m_Input.m_Direction = 1; //walk --> at spawn
				if(m_Core.m_Pos.x > 468 * 32 && IsGrounded()) //jump if far enough -->
				{
					m_Input.m_Jump = 1;
				}

				if(m_Core.m_Pos.x > 472 * 32 && m_Core.m_Pos.y > 80 * 32 + 10) //far enough --> swing hook up
				{
					m_Input.m_Hook = 1;
					if(Server()->Tick() % 20 == 0)
					{
						m_Input.m_Hook = 0;
					}
				}
				if(m_Core.m_Pos.x > 476 * 32) //left the spawn and too far -->
				{
					m_Input.m_Direction = -1;
				}
			}

			if(m_Core.m_Pos.y < 338 * 32 && m_Core.m_Pos.y > 90 * 32) //falling down area
			{
				if(m_Core.m_Pos.x > 406 * 32)
				{
					m_Input.m_Direction = -1;
				}
				else
				{
					m_Input.m_Direction = 1;
				}

				if(m_Core.m_Vel.y > 0.04f && m_Core.m_Vel.y < 16.6f && m_Core.m_Pos.x < 420 * 32) //slowly falling (sliding of platforms)
				{
					m_Input.m_Jump = 1;
				}

				if(!m_Dummy27_loved_x) //think of a destination while falling
				{
					if(rand() % 2 == 0)
					{
						m_Dummy27_loved_x = 420 * 32;
						m_Dummy27_loved_y = 430 * 32;
					}
					else
					{
						m_Dummy27_loved_x = 394 * 32;
						m_Dummy27_loved_y = 430 * 32;
					}
				}
			}
			else if(m_Core.m_Pos.y > 338 * 32 && m_Core.m_Pos.y < 380 * 32) //entering the police hole area (pvp arena)
			{
				if(!m_Dummy27_IsReadyToEnterPolice)
				{
					m_Input.m_Direction = -1;
				}
				else
				{
					m_Input.m_Direction = 1;
				}

				if(m_Core.m_Pos.x < 395 * 32)
				{
					m_Dummy27_IsReadyToEnterPolice = true;
				}

				if(m_Core.m_Pos.x > 402 * 32 && IsGrounded()) //jump into the tunnel -->
				{
					m_Input.m_Jump = 1;
				}
				if(m_Core.m_Pos.x > 402 * 32 && m_Core.m_Pos.y > 371 * 32)
				{
					m_Input.m_Jump = 1;
				}
			}
			else if(m_Core.m_Pos.x > 380 * 32 && m_Core.m_Pos.x < 450 * 32 && m_Core.m_Pos.y < 450 * 32 && m_Core.m_Pos.y > 380 * 32) //police area // 27
			{
				if(m_Core.m_Pos.x < 397 * 32 && m_Core.m_Pos.y > 436 * 32 && m_Core.m_Pos.x > 388 * 32) // on the money tile jump loop, to prevent blocking flappy there
				{
					m_Input.m_Jump = 0;
					if(Server()->Tick() % 20 == 0)
					{
						m_Input.m_Jump = 1;
					}
				}
				//detect lower panic (accedentally fall into the lower police base
				if(!m_Dummy27_lower_panic && m_Core.m_Pos.y > 437 * 32 && m_Core.m_Pos.y > m_Dummy27_loved_y)
				{
					m_Dummy27_lower_panic = 1;
					GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9); //angry emote
				}

				if(m_Dummy27_lower_panic)
				{
					//Check for end panic
					if(m_Core.m_Pos.y < 434 * 32)
					{
						if(IsGrounded())
						{
							m_Dummy27_lower_panic = 0; //made it up yay
						}
					}

					if(m_Dummy27_lower_panic == 1) //position to jump on stairs
					{
						if(m_Core.m_Pos.x < 400 * 32)
						{
							m_Input.m_Direction = 1;
						}
						else if(m_Core.m_Pos.x > 401 * 32)
						{
							m_Input.m_Direction = -1;
						}
						else
						{
							m_Dummy27_lower_panic = 2;
						}
					}
					else if(m_Dummy27_lower_panic == 2) //jump on the left starblock element
					{
						if(IsGrounded())
						{
							m_Input.m_Jump = 1;
							if(Server()->Tick() % 20 == 0)
							{
								m_Input.m_Jump = 0;
							}
						}

						//navigate to platform
						if(m_Core.m_Pos.y < 435 * 32 - 10)
						{
							m_Input.m_Direction = -1;
							if(m_Core.m_Pos.y < 433 * 32)
							{
								if(m_Core.m_Vel.y > 0.01f && m_DummyUsedDJ == false)
								{
									m_Input.m_Jump = 1; //double jump
									if(!IsGrounded()) // this dummyuseddj is for only using default 2 jumps even if 5 jump is on
									{
										m_DummyUsedDJ = true;
									}
								}
							}
							if(m_DummyUsedDJ == true && IsGrounded())
							{
								m_DummyUsedDJ = false;
							}
						}

						else if(m_Core.m_Pos.y < 438 * 32) //only if high enough focus on the first lower platform
						{
							if(m_Core.m_Pos.x < 403 * 32)
							{
								m_Input.m_Direction = 1;
							}
							else if(m_Core.m_Pos.x > 404 * 32 + 20)
							{
								m_Input.m_Direction = -1;
							}
						}

						if((m_Core.m_Pos.y > 441 * 32 + 10 && (m_Core.m_Pos.x > 402 * 32 || m_Core.m_Pos.x < 399 * 32 + 10)) || isFreezed) //check for fail position
						{
							m_Dummy27_lower_panic = 1; //lower panic mode to reposition
						}
					}
				}
				else //no dummy lower panic
				{
					m_Dummy27_help_mode = 0;
					//check if officer needs help
					CCharacter *pChr = GameServer()->m_World.ClosestCharTypePoliceFreezeHole(m_Pos, true, this);
					if(pChr && pChr->IsAlive())
					{
						if(m_Core.m_Pos.y > 435 * 32) // setting the destination of dummy to top left police entry bcs otherwise bot fails when trying to help --> walks into jail wall xd
						{
							m_Dummy27_loved_x = (392 + rand() % 2) * 32;
							m_Dummy27_loved_y = 430 * 32;
						}
						//aimbot on heuzeueu
						m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
						m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
						m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
						m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

						m_Dummy_ClosestPolice = false;
						//If a policerank escapes from jail he is treated like a non police
						if((pChr->m_pPlayer->m_PoliceRank > 0 && pChr->m_pPlayer->m_EscapeTime == 0) || (pChr->m_pPlayer->m_PoliceHelper && pChr->m_pPlayer->m_EscapeTime == 0))
						{
							m_Dummy_ClosestPolice = true;
						}

						if(pChr->m_Pos.x > 444 * 32 - 10) //police dude failed too far --> to be reached by hook (set too help mode extream to leave save area)
						{
							m_Dummy27_help_mode = 2;
							if(m_Core.m_Jumped > 1 && m_Core.m_Pos.x > 431 * 32) //double jumped and above the freeze
							{
								m_Input.m_Direction = -1;
							}
							else
							{
								m_Input.m_Direction = 1;
							}
							//doublejump before falling in freeze
							if((m_Core.m_Pos.x > 432 * 32 && m_Core.m_Pos.y > 432 * 32) || m_Core.m_Pos.x > 437 * 32) //over the freeze and too low
							{
								m_Input.m_Jump = 1;
								m_Dummy27_help_hook = true;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "catch hook jump!");
							}
							if(IsGrounded() && m_Core.m_Pos.x > 430 * 32 && Server()->Tick() % 60 == 0)
							{
								m_Input.m_Jump = 1;
								//if (Server()->Tick() % 60 == 0)
								//{
								//	m_Input.m_Jump = 0;
								//}
							}
						}
						else
						{
							m_Dummy27_help_mode = 1;
						}

						if(m_Dummy27_help_mode == 1 && m_Core.m_Pos.x > 431 * 32 + 10)
						{
							m_Input.m_Direction = -1;
						}
						else if(m_Dummy27_help_mode == 1 && m_Core.m_Pos.x < 430 * 32)
						{
							m_Input.m_Direction = 1;
						}
						else
						{
							if(!m_Dummy27_help_hook && m_Dummy_ClosestPolice)
							{
								if(m_Dummy27_help_mode == 2) //police dude failed too far --> to be reached by hook
								{
									//if (m_Core.m_Pos.x > 435 * 32) //moved too double jump
									//{
									//	m_Dummy27_help_hook = true;
									//}
								}
								else if(pChr->m_Pos.x > 439 * 32) //police dude in the middle
								{
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "middle");
									if(IsGrounded())
									{
										m_Dummy27_help_hook = true;
										m_Input.m_Jump = 1;
										m_Input.m_Hook = 1;
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "HOOOK");
									}
								}
								else //police dude failed too near to hook from ground
								{
									if(m_Core.m_Vel.y < -4.20f && m_Core.m_Pos.y < 431 * 32)
									{
										m_Dummy27_help_hook = true;
										m_Input.m_Jump = 1;
										m_Input.m_Hook = 1;
									}
								}
							}
							if(Server()->Tick() % 8 == 0)
							{
								m_Input.m_Direction = 1;
							}
						}

						if(m_Dummy27_help_hook)
						{
							m_Input.m_Hook = 1;
							if(Server()->Tick() % 200 == 0)
							{
								m_Dummy27_help_hook = false; //timeout hook maybe failed
								m_Input.m_Hook = 0;
								m_Input.m_Direction = 1;
							}
						}

						//dont wait on ground
						if(IsGrounded() && Server()->Tick() % 900 == 0)
						{
							m_Input.m_Jump = 1;
						}
						//backup reset jump
						if(Server()->Tick() % 1337 == 0)
						{
							m_Input.m_Jump = 0;
						}
					}

					if(!m_Dummy27_help_mode)
					{
						//==============
						//NOTHING TO DO
						//==============
						//basic walk to destination
						if(m_Core.m_Pos.x < m_Dummy27_loved_x - 32)
						{
							m_Input.m_Direction = 1;
						}
						else if(m_Core.m_Pos.x > m_Dummy27_loved_x + 32 && m_Core.m_Pos.x > 384 * 32)
						{
							m_Input.m_Direction = -1;
						}

						//change changing speed
						if(Server()->Tick() % m_Dummy27_speed == 0)
						{
							if(rand() % 2 == 0)
							{
								m_Dummy27_speed = rand() % 10000 + 420;
							}
						}

						//choose beloved destination
						if(Server()->Tick() % m_Dummy27_speed == 0)
						{
							if((rand() % 2) == 0)
							{
								if((rand() % 3) == 0)
								{
									m_Dummy27_loved_x = 420 * 32 + rand() % 69;
									m_Dummy27_loved_y = 430 * 32;
									GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
								}
								else
								{
									m_Dummy27_loved_x = (392 + rand() % 2) * 32;
									m_Dummy27_loved_y = 430 * 32;
								}
								if((rand() % 2) == 0)
								{
									m_Dummy27_loved_x = 384 * 32 + rand() % 128;
									m_Dummy27_loved_y = 430 * 32;
									GameServer()->SendEmoticon(m_pPlayer->GetCID(), 5);
								}
								else
								{
									if(rand() % 3 == 0)
									{
										m_Dummy27_loved_x = 420 * 32 + rand() % 128;
										m_Dummy27_loved_y = 430 * 32;
										GameServer()->SendEmoticon(m_pPlayer->GetCID(), 8);
									}
									else if(rand() % 4 == 0)
									{
										m_Dummy27_loved_x = 429 * 32 + rand() % 64;
										m_Dummy27_loved_y = 430 * 32;
										GameServer()->SendEmoticon(m_pPlayer->GetCID(), 8);
									}
								}
								if(rand() % 5 == 0) //lower middel base
								{
									m_Dummy27_loved_x = 410 * 32 + rand() % 64;
									m_Dummy27_loved_y = 443 * 32;
								}
							}
							else
							{
								GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
							}
						}
					}
				}

				//=====================
				// all importat backups
				// all dodges and moves
				// which are needed all
				// the time
				// police base only
				//=====================

				//dont walk into the lower police base entry freeze
				if(m_Core.m_Pos.x > 425 * 32 && m_Core.m_Pos.x < 429 * 32) //right side
				{
					if(m_Core.m_Vel.x < -0.02f && IsGrounded())
					{
						m_Input.m_Jump = 1;
					}
				}
				else if(m_Core.m_Pos.x > 389 * 32 && m_Core.m_Pos.x < 391 * 32) //left side
				{
					if(m_Core.m_Vel.x > 0.02f && IsGrounded())
					{
						m_Input.m_Jump = 1;
					}
				}

				//jump over the police underground from entry to enty
				if(m_Core.m_Pos.y > m_Dummy27_loved_y) //only if beloved place is an upper one
				{
					if(m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.x < 418 * 32) //right side
					{
						if(m_Core.m_Vel.x < -0.02f && IsGrounded())
						{
							m_Input.m_Jump = 1;
							if(Server()->Tick() % 5 == 0)
							{
								m_Input.m_Jump = 0;
							}
						}
					}
					else if(m_Core.m_Pos.x > 398 * 32 && m_Core.m_Pos.x < 401 * 32) //left side
					{
						if(m_Core.m_Vel.x > 0.02f && IsGrounded())
						{
							m_Input.m_Jump = 1;
							if(Server()->Tick() % 5 == 0)
							{
								m_Input.m_Jump = 0;
							}
						}
					}

					//do the doublejump
					if(m_Core.m_Vel.y > 6.9f && m_Core.m_Pos.y > 430 * 32 && m_Core.m_Pos.x < 433 * 32 && m_DummyUsedDJ == false) //falling and not too high to hit roof with head
					{
						m_Input.m_Jump = 1;
						//m_LatestInput.m_Fire++;
						//m_Input.m_Fire++;
						if(!IsGrounded()) // this dummyuseddj is for only using default 2 jumps even if 5 jump is on
						{
							m_DummyUsedDJ = true;
						}
					}
					if(m_DummyUsedDJ == true && IsGrounded())
					{
						m_DummyUsedDJ = false;
					}
				}
			}
			if(m_Core.m_Pos.y > 380 * 32 && m_Core.m_Pos.x < 363 * 32) // walking right again to get into the tunnel at the bottom
			{
				m_Input.m_Direction = 1;
				if(IsGrounded())
				{
					m_Input.m_Jump = 1;
				}
			}
			if(m_Core.m_Pos.y > 380 * 32 && m_Core.m_Pos.x < 381 * 32 && m_Core.m_Pos.x > 363 * 32)
			{
				m_Input.m_Direction = 1;
				if(m_Core.m_Pos.x > 367 * 32 && m_Core.m_Pos.x < 368 * 32 && IsGrounded())
				{
					m_Input.m_Jump = 1;
				}
				if(m_Core.m_Pos.y > 433.7 * 32)
				{
					m_Input.m_Jump = 1;
				}
			}
			if(m_pPlayer->m_DummyModeSpawn == 32)
			{
				if(m_Core.m_Pos.x > 290 * 32 && m_Core.m_Pos.x < 450 * 32 && m_Core.m_Pos.y > 415 * 32 && m_Core.m_Pos.y < 450 * 32)
				{
					if(isFreezed) // kills when in freeze in policebase or left of it (takes longer that he kills bcs the way is so long he wait a bit longer for help)
					{
						if(Server()->Tick() % 60 == 0)
						{
							GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3); // tear emote before killing
						}
						if(Server()->Tick() % 3000 == 0 && (IsGrounded() || m_Core.m_Pos.x > 430 * 32)) // kill when freeze
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}
					}
				}
			}
			else //unknown area //it isnt it is spawn area and stuff not a random area
			{
				////walk towards police station
				//if (m_Core.m_Pos.x < 400 * 32)
				//{
				//	m_Input.m_Direction = 1;
				//}
				//else
				//{
				//	m_Input.m_Direction = -1;
				//}

				//if (Server()->Tick() % 420 == 0)
				//{
				//	m_Input.m_Jump = 1;
				//	//ask public chat for help
				//	if (rand() % 2 == 0)
				//	{
				//		int rand_message = rand() % 10;
				//		if (rand_message == 0)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "where am i?");
				//		}
				//		else if (rand_message == 1)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hello is there anybody?");
				//		}
				//		else if (rand_message == 2)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "guys! i need help!!!11");
				//		}
				//		else if (rand_message == 3)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "I think i am lost.");
				//		}
				//		else if (rand_message == 4)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "officer needs help!");
				//		}
				//		else if (rand_message == 5)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Where is the police base?");
				//		}
				//		else if (rand_message == 6)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Can someone bring me to the police base?");
				//		}
				//		else if (rand_message == 7)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "I AM LOST!");
				//		}
				//		else if (rand_message == 8)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "where the fuck am i?");
				//		}
				//		else if (rand_message == 9)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hello sir? can you bring me to the police base?");
				//		}
				//	}
				//}

				//if (Server()->Tick() % 2000 == 0 && isFreezed && IsGrounded())
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//}
				//if (Server()->Tick() % 9000 == 0)
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//}
			}

			//reset in freeze ( W A R N I N G!!! Keep this code last in dummymode)
			if(m_FreezeTime)
			{
				m_Input.m_Hook = 0;
				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;
			}
		}
		else if(m_pPlayer->m_DummyMode == 28) // some BlmapChill
		{
			//RestOnChange (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			if(m_Core.m_Pos.x > 451 * 32 && m_Core.m_Pos.x < 472 * 32 && m_Core.m_Pos.y > 74 * 32 && m_Core.m_Pos.y < 85 * 32) //spawn bereich
			{
				m_Input.m_Direction = -1;

				if(m_Core.m_Pos.x > 454 * 32 && m_Core.m_Pos.x < 458 * 32) //linke seite des spawn bereiches
				{
					m_Input.m_Jump = 1;
					if(Server()->Tick() % 10 == 0) //is sone kleine glock die alle was weiss ich Zeiteinheiten true ist
					{
						m_Input.m_Jump = 0; //auch wenn davor Jump = 1 gestezt wurde es zählt immer der letzte also wird er wenn iwi 10 sec oder was auch immer vergangen sind nochmal kurz den jump loslassen
					}
				}
			}
			else if(m_Core.m_Pos.x > 30 * 32 && m_Core.m_Pos.x < 32 * 32 && m_Core.m_Pos.y > 25 * 32 && m_Core.m_Pos.y < 32 * 32) // if right wall on newteespawn go a bit left to not just fall into freeze
			{
				m_Input.m_Direction = -1;
				m_Input.m_Jump = 1;
			}
			else if(m_Core.m_Pos.x > 15 * 32 && m_Core.m_Pos.x < 33 * 32 && m_Core.m_Pos.y > 33 * 32 && m_Core.m_Pos.y < 35 * 32) //newtee spawn
			{
				m_Input.m_Direction = 1;

				if(m_Core.m_Pos.x > 30 * 32 && m_Core.m_Pos.x < 32 * 32 && m_Core.m_Pos.y > 32 * 32 && m_Core.m_Pos.y < 35 * 32) // right side newetee jump through freeze
				{
					m_Input.m_Jump = 1;
				}
			}
			else if(m_Core.m_Pos.x > 20 * 32 && m_Core.m_Pos.x < 105 * 32 && m_Core.m_Pos.y > 30 * 32 && m_Core.m_Pos.y < 44 * 32) // under newtee spawn walk to the right
			{
				m_Input.m_Direction = 1;

				if(m_Core.m_Pos.x > 41 * 32 && m_Core.m_Pos.x < 43 * 32) // jump over frz in ground
				{
					m_Input.m_Jump = 1;
				}
				else if(m_Core.m_Pos.y > 42 * 32 && m_Core.m_Pos.y < 44 * 32 && m_Core.m_Pos.x > 55 * 32 && m_Core.m_Pos.x < 58 * 32) // jump over 2nd freeze
				{
					m_Input.m_Jump = 1;
				}
				else if(m_Core.m_Pos.x > 75 * 32 && m_Core.m_Pos.x < 76 * 32 && m_Core.m_Pos.y > 30 * 32 && m_Core.m_Pos.y < 44 * 32) // jump over the rotating thing
				{
					m_Input.m_Jump = 1;
				}
				else if((m_Core.m_Vel.y > 0.01) && m_Core.m_Pos.x > 77 * 32 && m_Core.m_Pos.x < 90 * 32) //dj in mid air over the rotating thing
				{
					m_Input.m_Jump = 1;
				}
			}
			else if(m_Core.m_Pos.x > 95 * 32 && m_Core.m_Pos.x < 121 * 32) // already fell down and now going right
			{
				m_Input.m_Direction = 1;
				if(m_Core.m_Pos.x > 110 * 32 && m_Core.m_Pos.x < 115 * 32) // and jump to be ready for wallhammer
				{
					m_Input.m_Jump = 1;
				}
			}
		}
		else if(m_pPlayer->m_DummyMode == 29) //mode 18 (tryhardwayblocker cb5)    with some more human wayblock style not so try hard a cool chillblock5 blocker mode
		{
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0; //this is 29 only the mode 18 had no jump resett hope it works ... it shoudl omg
				//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "HALLO ICH BIN 29!");

			//Check ob dem bot langweilig geworden is :)

			if(m_Dummy_bored_counter > 2)
			{
				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this);
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
				if(m_Core.m_Pos.x < 429 * 32 && IsGrounded())
				{
					m_Dummy_bored = true;
					//static bool test = false;

					//if (!test)
					//{
					//	GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "bored");
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

			Mode29 is a blocker which is not dat tryhard he doesnt wayblock and does more random stuff and trys freezeblock tricks







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
			if(m_Core.m_Pos.y > 214 * 32 && m_Core.m_Pos.x > 424 * 32)
			{
				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerWB(m_Pos, true, this);
				if(pChr && pChr->IsAlive())
				{
					//Wenn der bot im tunnel ist und ein Gegner im RulerWB bereich
					m_Dummy_mode18 = 1;
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Wayblocker gesichtet");
				}
			}
			else if(m_Dummy_bored)
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

				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this);
				if(pChr && pChr->IsAlive())
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					//rest on tick
					m_Input.m_Hook = 0;
					m_Input.m_Jump = 0;
					m_Input.m_Direction = 0;
					m_LatestInput.m_Fire = 0;
					m_Input.m_Fire = 0;
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

					if(pChr->m_Core.m_Vel.y > -0.9f && pChr->m_Pos.y > 211 * 32)
					{
						//wenn der gegner am boden liegt starte angriff
						m_Dummy_special_defend_attack = true;

						//start jump
						m_Input.m_Jump = 1;
					}

					if(m_Dummy_special_defend_attack)
					{
						if(m_Core.m_Pos.x - pChr->m_Pos.x < 50) //wenn der gegner nah genung is mach dj
						{
							m_Input.m_Jump = 1;
						}

						if(pChr->m_Pos.x < m_Core.m_Pos.x)
						{
							m_Input.m_Hook = 1;
						}
						else //wenn der gegner weiter rechts als der bot is lass los und übergib an main deine arbeit ist hier getahen
						{ //main mode wird evenetuell noch korrigieren mit schieben
							m_Dummy_special_defend = false;
							m_Dummy_special_defend_attack = false;
						}

						//Der bot sollte möglichst weit nach rechts gehen aber natürlich nicht ins freeze

						if(m_Core.m_Pos.x < 427 * 32 + 15)
						{
							m_Input.m_Direction = 1;
						}
						else
						{
							m_Input.m_Direction = -1;
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
				m_Input.m_Hook = 0;
				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;
				if(Server()->Tick() % 30 == 0)
				{
					SetWeapon(0);
				}
				//if (Server()->Tick() % 5 == 0)
				//{
				//	GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
				//}

				//Selfkills (bit random but they work)
				if(isFreezed)
				{
					//wenn der bot freeze is warte erstmal n paar sekunden und dann kill dich
					if(Server()->Tick() % 300 == 0)
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
						m_Dummy_happy = false;
					}
				}

				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler2(m_Pos, true, this);
				if(pChr && pChr->IsAlive())
				{
					//Check ob an notstand mode18 = 0 übergeben
					if(pChr->m_FreezeTime == 0)
					{
						m_Dummy_bored = false;
						m_Dummy_bored_counter = 0;
						m_Dummy_mode18 = 0;
					}

					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					m_Input.m_Jump = 1;

					if(pChr->m_Pos.y > m_Core.m_Pos.y && pChr->m_Pos.x > m_Core.m_Pos.x + 20) //solange der bot über dem gegner ist (damit er wenn er ihn weg hammert nicht weiter hookt)
					{
						m_Input.m_Hook = 1;
					}

					if(m_Core.m_Pos.x > 420 * 32)
					{
						m_Input.m_Direction = -1;
					}

					if(pChr->m_Pos.y < m_Core.m_Pos.y + 15)
					{
						m_LatestInput.m_Fire++;
						m_Input.m_Fire++;
					}
				}
				else //lieblings position finden wenn nichts abgeht
				{
					//               old: 421 * 32
					if(m_Core.m_Pos.x < 423 * 32)
					{
						m_Input.m_Direction = 1;
					}
					//                   old: 422 * 32 + 30
					else if(m_Core.m_Pos.x > 424 * 32 + 30)
					{
						m_Input.m_Direction = -1;
					}
				}
			}
			else if(m_Dummy_mode18 == 1) //attack in tunnel
			{
				//Selfkills (bit random but they work)
				if(isFreezed)
				{
					//wenn der bot freeze is warte erstmal n paar sekunden und dann kill dich
					if(Server()->Tick() % 300 == 0)
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}
				}

				//stay on position

				if(m_Core.m_Pos.x < 426 * 32 + 10) // zu weit links
				{
					m_Input.m_Direction = 1; //geh rechts
				}
				else if(m_Core.m_Pos.x > 428 * 32 - 10) //zu weit rechts
				{
					m_Input.m_Direction = -1; // geh links
				}
				else if(m_Core.m_Pos.x > 428 * 32 + 10) // viel zu weit rechts
				{
					m_Input.m_Direction = -1; // geh links
					m_Input.m_Jump = 1;
				}
				else
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerWB(m_Pos, true, this);
					if(pChr && pChr->IsAlive())
					{
						if(pChr->m_Pos.x < 436 * 32) //wenn er ganz weit über dem freeze auf der kante ist (hooke direkt)
						{
							m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
						}
						else //wenn der Gegner weiter hinter dem unhook ist (hook über den Gegner um ihn trozdem zu treffen und das unhook zu umgehen)
						{
							m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x - 50;
							m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x - 50;
							m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
						}

						//char aBuf[256];
						//str_format(aBuf, sizeof(aBuf), "targX: %d = %d - %d", m_Input.m_TargetX, pChr->m_Pos.x, m_Pos.x);
						//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

						//m_Input.m_Hook = 0;
						CCharacter *pChr = GameServer()->m_World.ClosestCharTypeTunnel(m_Pos, true, this);
						if(pChr && pChr->IsAlive())
						{
							//wenn jemand im tunnel is check ob du nicht ausversehen den hookst anstatt des ziels in der WB area
							if(pChr->m_Pos.x < m_Core.m_Pos.x) //hooke nur wenn kein Gegner rechts von dem bot im tunnel is (da er sonst ziemlich wahrscheinlich den hooken würde)
							{
								m_Input.m_Hook = 1;
							}
						}
						else
						{
							//wenn eh keiner im tunnel is hau raus dat ding
							m_Input.m_Hook = 1;
						}

						//schau ob sich der gegner bewegt und der bot grad nicht mehr am angreifen iss dann resette falls er davor halt misshookt hat
						//geht nich -.-
						if(m_Core.m_HookState != HOOK_FLYING && m_Core.m_HookState != HOOK_GRABBED)
						{
							if(Server()->Tick() % 10 == 0)
							{
								m_Input.m_Hook = 0;
							}
						}

						if(m_Core.m_Vel.x > 3.0f)
						{
							m_Input.m_Direction = -1;
						}
						else
						{
							m_Input.m_Direction = 0;
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
				//	m_Input.m_Hook = 0;
				//}

				//m_Input.m_Hook = 0;
				//if (m_Core.m_HookState == HOOK_FLYING)
				//	m_Input.m_Hook = 1;
				//else if (m_Core.m_HookState == HOOK_GRABBED)
				//	m_Input.m_Hook = 1;
				//else
				//	m_Input.m_Hook = 0;

				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;

				//char aBuf[256];
				//str_format(aBuf, sizeof(aBuf), "speed:  x: %f y: %f speed pChr:  x: %f y: %f", m_Core.m_Vel.x, m_Core.m_Vel.y);

				//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

				if(1 == 2)
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
					if(pChr && pChr->IsAlive())
					{
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "speed pChr:  x: %f y: %f", pChr->m_Core.m_Vel.x, pChr->m_Core.m_Vel.y);

						//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
					}
				}

				//m_pPlayer->m_TeeInfos.m_Name = aBuf;

				if(m_Core.m_Vel.x > 1.0f)
				{
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "speed: schneller als 1");
				}

				//Check ob jemand in der linken freeze wand is

				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerLeftFreeze(m_Pos, true, this); //wenn jemand rechts im freeze liegt
				if(pChr && pChr->IsAlive()) // wenn ein spieler rechts im freeze lebt
				{ //----> versuche im notstand nicht den gegner auch da rein zu hauen da ist ja jetzt voll

					m_Dummy_left_freeze_full = true;
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Da liegt einer im freeze");
				}
				else // wenn da keiner is fülle diesen spot (linke freeze wand im ruler spot)
				{
					m_Dummy_left_freeze_full = false;
				}

				//hardcodet selfkill (moved in lower area only)
				//if (m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.y > 214 * 32)  //Links am spawn runter
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Links am spawn runter");
				//}
				//else if ((m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x > 392 * 32 && m_Core.m_Pos.y > 190) || (m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.y > 190)) //freeze decke am spawn
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke am spawn");
				//}
				//else if (m_Core.m_Pos.y > 218 * 32 + 31 /* für tee balance*/ && m_Core.m_Pos.x < 415 * 32) //freeze boden am spawn
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze boden am spawn");
				//}
				//else if (m_Core.m_Pos.y < 215 * 32 && m_Core.m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.x < 428 * 32) //freeze decke im tunnel
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke im tunnel");
				//}
				//else if (m_Core.m_Pos.y > 222 * 32) //freeze becken unter area
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze becken unter area");
				//}
				//else if (m_Core.m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 436 * 32) //freeze rechts neben freeze becken
				//{
				//	//Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze rechts neben freeze becken");
				//}
				//else if (m_Core.m_Pos.x > 469 * 32) //zu weit ganz rechts in der ruler area
				//{
				//	//Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "zu weit ganz rechts in der ruler area");
				//}
				//else if (m_Core.m_Pos.y > 211 * 32 + 34 && m_Core.m_Pos.x > 455 * 32) //alles freeze am boden rechts in der area
				//{
				//	//Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze boden rechts der area");
				//}

				//if (m_Core.m_Pos.y < 193 * 32 /*&& g_Config.m_SvChillBlock5Version == 1*/) //old spawn of unsued version (this code makes no sense at all)
				//{
				//	m_Input.m_TargetX = 200;
				//	m_Input.m_TargetY = -80;

				//	//not falling in freeze is bad
				//	if (m_Core.m_Vel.y < 0.01f && m_FreezeTime > 0)
				//	{
				//		if (Server()->Tick() % 40 == 0)
				//		{
				//			Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//		}
				//	}
				//	if (m_Core.m_Pos.y > 116 * 32 && m_Core.m_Pos.x > 394 * 32)
				//	{
				//		Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	}

				//	if (m_Core.m_Pos.x > 364 * 32 && m_Core.m_Pos.y < 126 * 32 && m_Core.m_Pos.y > 122 * 32 + 10)
				//	{
				//		if (m_Core.m_Vel.y > -1.0f)
				//		{
				//			m_Input.m_Hook = 1;
				//		}
				//	}

				//	if (m_Core.m_Pos.y < 121 * 32 && m_Core.m_Pos.x > 369 * 32)
				//	{
				//		m_Input.m_Direction = -1;
				//	}
				//	else
				//	{
				//		m_Input.m_Direction = 1;
				//	}
				//	if (m_Core.m_Pos.y < 109 * 32 && m_Core.m_Pos.x > 377 * 32 && m_Core.m_Pos.x < 386 * 32)
				//	{
				//		m_Input.m_Direction = 1;
				//	}

				//	if (m_Core.m_Pos.y > 128 * 32)
				//	{
				//		m_Input.m_Jump = 1;
				//	}

				//	//speeddown at end to avoid selfkill cuz to slow falling in freeze
				//	if (m_Core.m_Pos.x > 384 * 32 && m_Core.m_Pos.y > 121 * 32)
				//	{
				//		m_Input.m_TargetX = 200;
				//		m_Input.m_TargetY = 300;
				//		m_Input.m_Hook = 1;
				//	}
				//}
				//else //under 193 (above 193 is new spawn)

				if(m_Core.m_Pos.x > 241 * 32 && m_Core.m_Pos.x < 418 * 32 && m_Core.m_Pos.y > 121 * 32 && m_Core.m_Pos.y < 192 * 32) //new spawn ChillBlock5 (tourna edition (the on with the gores stuff))
				{
					//dieser code wird also nur ausgeführt wenn der bot gerade im neuen bereich ist
					if(m_Core.m_Pos.x > 319 * 32 && m_Core.m_Pos.y < 161 * 32) //top right spawn
					{
						//look up left
						if(m_Core.m_Pos.x < 372 * 32 && m_Core.m_Vel.y > 3.1f)
						{
							m_Input.m_TargetX = -30;
							m_Input.m_TargetY = -80;
						}
						else
						{
							m_Input.m_TargetX = -100;
							m_Input.m_TargetY = -80;
						}

						if(m_Core.m_Pos.x > 331 * 32 && isFreezed)
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}

						if(m_Core.m_Pos.x < 327 * 32) //dont klatsch in ze wand
						{
							m_Input.m_Direction = 1; //nach rechts laufen
						}
						else
						{
							m_Input.m_Direction = -1;
						}

						if(IsGrounded() && m_Core.m_Pos.x < 408 * 32) //initial jump from spawnplatform
						{
							m_Input.m_Jump = 1;
						}

						if(m_Core.m_Pos.x > 330 * 32) //only hook in tunnel and let fall at the end
						{
							if(m_Core.m_Pos.y > 147 * 32 || (m_Core.m_Pos.y > 143 * 32 && m_Core.m_Vel.y > 3.3f)) //gores pro hook up
							{
								m_Input.m_Hook = 1;
							}
							else if(m_Core.m_Pos.y < 143 * 32 && m_Core.m_Pos.x < 372 * 32) //hook down (if too high and in tunnel)
							{
								m_Input.m_TargetX = -42;
								m_Input.m_TargetY = 100;
								m_Input.m_Hook = 1;
							}
						}
					}
					else if(m_Core.m_Pos.x < 317 * 32) //top left spawn
					{
						if(m_Core.m_Pos.y < 158 * 32) //spawn area find down
						{
							//selfkill
							if(isFreezed)
							{
								Die(m_pPlayer->GetCID(), WEAPON_SELF);
							}

							if(m_Core.m_Pos.x < 276 * 32 + 20) //is die mitte von beiden linken spawns also da wo es runter geht
							{
								m_Input.m_TargetX = 57;
								m_Input.m_TargetY = -100;
								m_Input.m_Direction = 1;
							}
							else
							{
								m_Input.m_TargetX = -57;
								m_Input.m_TargetY = -100;
								m_Input.m_Direction = -1;
							}

							if(m_Core.m_Pos.y > 147 * 32)
							{
								//dbg_msg("fok","will hooken");
								m_Input.m_Hook = 1;
								if(m_Core.m_Pos.x > 272 * 32 && m_Core.m_Pos.x < 279 * 32) //let fall in the hole
								{
									//dbg_msg("fok", "lass ma des");
									m_Input.m_Hook = 0;
									m_Input.m_TargetX = 2;
									m_Input.m_TargetY = 100;
								}
							}
						}
						else if(m_Core.m_Pos.y > 162 * 32) //managed it to go down --> go left
						{
							//selfkill
							if(isFreezed)
							{
								Die(m_pPlayer->GetCID(), WEAPON_SELF);
							}

							if(m_Core.m_Pos.x < 283 * 32)
							{
								m_Input.m_TargetX = 200;
								m_Input.m_TargetY = -136;
								if(m_Core.m_Pos.y > 164 * 32 + 10)
								{
									m_Input.m_Hook = 1;
								}
							}
							else
							{
								m_Input.m_TargetX = 80;
								m_Input.m_TargetY = -100;
								if(m_Core.m_Pos.y > 171 * 32 - 10)
								{
									m_Input.m_Hook = 1;
								}
							}

							m_Input.m_Direction = 1;
						}
						else //freeze unfreeze bridge only 2 tiles do nothing here
						{
						}
					}
					else //lower end area of new spawn --> entering old spawn by walling and walking right
					{
						m_Input.m_Direction = 1;
						m_Input.m_TargetX = 200;
						m_Input.m_TargetY = -84;

						//Selfkills
						if(isFreezed && IsGrounded()) //should never lie in freeze at the ground
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}

						if(m_Core.m_Pos.y < 166 * 32 - 20)
						{
							m_Input.m_Hook = 1;
						}

						if(m_Core.m_Pos.x > 332 * 32 && m_Core.m_Pos.x < 337 * 32 && m_Core.m_Pos.y > 182 * 32) //wont hit the wall --> jump
						{
							m_Input.m_Jump = 1;
						}

						if(m_Core.m_Pos.x > 336 * 32 + 20 && m_Core.m_Pos.y > 180 * 32) //stop moving if walled
						{
							m_Input.m_Direction = 0;
						}
					}
				}
				else
				{
					if(m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.x > 325 * 32 && m_Core.m_Pos.y > 215 * 32) //Links am spawn runter
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Links am spawn runter");
					}
					//else if ((m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x > 392 * 32 && m_Core.m_Pos.y > 190) || (m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.y > 190)) //freeze decke am old spawn
					//{
					//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke am old spawn");
					//}
					//else if (m_Core.m_Pos.y > 218 * 32 + 31 /* für tee balance*/ && m_Core.m_Pos.x < 415 * 32) //freeze boden am spawn
					//{
					//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze boden am spawn");
					//}
					else if(m_Core.m_Pos.y < 215 * 32 && m_Core.m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.x < 428 * 32) //freeze decke im tunnel
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke im tunnel");
					}
					else if(m_Core.m_Pos.y > 222 * 32) //freeze becken unter area
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze becken unter area");
					}
					else if(m_Core.m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 436 * 32) //freeze rechts neben freeze becken
					{
						//Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze rechts neben freeze becken");
					}
					else if(m_Core.m_Pos.x > 469 * 32) //zu weit ganz rechts in der ruler area
					{
						//Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "zu weit ganz rechts in der ruler area");
					}
					else if(m_Core.m_Pos.y > 211 * 32 + 34 && m_Core.m_Pos.x > 455 * 32) //alles freeze am boden rechts in der area
					{
						//Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze boden rechts der area");
					}

					if(m_Core.m_Pos.y < 220 * 32 && m_Core.m_Pos.x < 415 * 32 && m_FreezeTime > 1 && m_Core.m_Pos.x > 352 * 32) //always suicide on freeze if not reached teh block area yet AND dont suicide in spawn area because new spawn sys can get pretty freezy
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze und links der block area");
					}

					// Movement
					/*
					NEW MOVEMENT TO BLOCK AREA STRUCTURE :)

					After spawning the bot thinks about what way he shoudl choose.
					After he found one he stopps thinking until he respawns agian.

					if he thinks the tunnel is shit he goes trough the window

					*/

					//new spawn do something agianst hookers
					if(m_Core.m_Pos.x < 380 * 32 && m_Core.m_Pos.x > 322 * 32 && m_Core.m_Vel.x < -0.001f)
					{
						m_Input.m_Hook = 1;
						if((m_Core.m_Pos.x < 362 * 32 && IsGrounded()) || m_Core.m_Pos.x < 350 * 32)
						{
							if(Server()->Tick() % 10 == 0)
							{
								m_Input.m_Jump = 1;
								SetWeapon(0);
							}
						}
					}
					if(m_Core.m_Pos.x < 415 * 32)
					{
						CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this); //hammer player up in freeze if in right pos
						if(pChr && pChr->IsAlive())
						{
							if(pChr->m_Core.m_Pos.x > m_Core.m_Pos.x - 100 && pChr->m_Core.m_Pos.x < m_Core.m_Pos.x + 100 && pChr->m_Core.m_Pos.y > m_Core.m_Pos.y - 100 && pChr->m_Core.m_Pos.y < m_Core.m_Pos.y + 100)
							{
								if(pChr->m_Core.m_Vel.y < -1.5f) //only boost and use existing up speed
								{
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
								}
								if(Server()->Tick() % 3 == 0)
								{
									SetWeapon(0);
								}
							}
							//old spawn do something agianst way blockers (roof protection)
							if(m_Core.m_Pos.y > 206 * 32 + 4 && m_Core.m_Pos.y < 208 * 32 && m_Core.m_Vel.y < -4.4f)
							{
								m_Input.m_Jump = 1;
							}
							//old spawn roof protection / attack hook
							if(pChr->m_Core.m_Pos.y > m_Core.m_Pos.y + 50)
							{
								m_Input.m_Hook = 1;
							}
						}
					}

					if(m_Core.m_Pos.x < 388 * 32 && m_Core.m_Pos.y > 213 * 32) //jump to old spawn
					{
						m_Input.m_Jump = 1;
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
						m_Input.m_Hook = 1;
						m_Input.m_TargetX = -200;
						m_Input.m_TargetY = 0;
					}

					if(!m_Dummy_planned_movment)
					{
						CCharacter *pChr = GameServer()->m_World.ClosestCharTypeTunnel(m_Pos, true, this);
						if(pChr && pChr->IsAlive())
						{
							if(pChr->m_Core.m_Vel.x < 3.3f) //found a slow bob in tunnel
							{
								m_Dummy_movement_to_block_area_style_window = true;
							}
						}

						m_Dummy_planned_movment = true;
					}

					if(m_Dummy_movement_to_block_area_style_window)
					{
						if(m_Core.m_Pos.x < 415 * 32)
						{
							m_Input.m_Direction = 1;

							if(m_Core.m_Pos.x > 404 * 32 && IsGrounded())
							{
								m_Input.m_Jump = 1;
							}
							if(m_Core.m_Pos.y < 208 * 32)
							{
								m_Input.m_Jump = 1;
							}

							if(m_Core.m_Pos.x > 410 * 32)
							{
								m_Input.m_TargetX = 200;
								m_Input.m_TargetY = 70;
								if(m_Core.m_Pos.x > 413 * 32)
								{
									m_Input.m_Hook = 1;
								}
							}
						}
						else //not needed but safty xD when the bot managed it to get into the ruler area change to old movement
						{
							m_Dummy_movement_to_block_area_style_window = false;
						}

						//something went wrong:
						if(m_Core.m_Pos.y > 214 * 32)
						{
							m_Input.m_Jump = 1;
							m_Dummy_movement_to_block_area_style_window = false;
						}
					}
					else //down way
					{
						//CheckFatsOnSpawn

						if(m_Core.m_Pos.x < 406 * 32)
						{
							CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
							if(pChr && pChr->IsAlive())
							{
								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								if(pChr->m_Pos.x < 407 * 32 && pChr->m_Pos.y > 212 * 32 && pChr->m_Pos.y < 215 * 32 && pChr->m_Pos.x > m_Core.m_Pos.x) //wenn ein im weg stehender tee auf der spawn plattform gefunden wurde
								{
									SetWeapon(0); //hol den hammer raus!
									if(pChr->m_Pos.x - m_Core.m_Pos.x < 30) //wenn der typ nahe bei dem bot ist
									{
										if(m_FreezeTick == 0) //nicht rum schrein
										{
											m_LatestInput.m_Fire++;
											m_Input.m_Fire++;
										}

										if(Server()->Tick() % 10 == 0)
										{
											GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9); //angry
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
						if(m_Core.m_Pos.x < 412 * 32 && m_Core.m_Pos.y > 217 * 32 && m_Core.m_Vel.x < -0.5f)
						{
							m_Input.m_Jump = 1;
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
							int r = rand() % 88; // #noRACISMIM   hitler was fggt    but just because he claimed this number i wont stop using it fuck him and his claims i dont care about him i use this number as my number. It is a statement agianst his usage! we have to fight!

							if(r > 44)
							{
								m_Input.m_Fire++;
							}

							int duNIPPEL = rand() % 1337;
							if(duNIPPEL > 420)
							{
								SetWeapon(0);
							}

							CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
							if(pChr && pChr->IsAlive())
							{
								int r = rand() % 10 - 10;

								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y - r;

								if(Server()->Tick() % 13 == 0)
								{
									GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9);
								}

								if(m_Core.m_HookState == HOOK_GRABBED || (m_Core.m_Pos.y < 216 * 32 && pChr->m_Pos.x > 404 * 32) || (pChr->m_Pos.x > 405 * 32 && m_Core.m_Pos.x > 404 * 32 + 20))
								{
									m_Input.m_Hook = 1;
									if(Server()->Tick() % 10 == 0)
									{
										int x = rand() % 20;
										int y = rand() % 20 - 10;
										m_Input.m_TargetX = x;
										m_Input.m_TargetY = y;
									}
								}
							}
						}

						//CheckSlowDudesInTunnel

						if(m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.y > 214 * 32) //wenn bot im tunnel ist
						{
							CCharacter *pChr = GameServer()->m_World.ClosestCharTypeTunnel(m_Pos, true, this);
							if(pChr && pChr->IsAlive())
							{
								if(pChr->m_Core.m_Vel.x < 7.8f) //wenn der nächste spieler im tunnel ein slowdude is
								{
									//HauDenBau
									SetWeapon(0); //hol den hammer raus!

									m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
									m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

									m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
									m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

									if(m_FreezeTick == 0) //nicht rum schrein
									{
										m_LatestInput.m_Fire++;
										m_Input.m_Fire++;
									}

									if(Server()->Tick() % 10 == 0) //angry emotes machen
									{
										GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9);
									}
								}
							}
						}

						//CheckSpeedInTunnel
						if(m_Core.m_Pos.x > 425 * 32 && m_Core.m_Pos.y > 214 * 32 && m_Core.m_Vel.x < 9.4f) //wenn nich genung speed zum wb spot springen
						{
							m_Dummy_get_speed = true;
						}

						if(m_Dummy_get_speed) //wenn schwung holen == true (tunnel)
						{
							if(m_Core.m_Pos.x > 422 * 32) //zu weit rechts
							{
								//---> hol schwung für den jump
								m_Input.m_Direction = -1;

								//new hammer agressive in the walkdirection to free the way
								if(m_FreezeTime == 0)
								{
									m_Input.m_TargetX = -200;
									m_Input.m_TargetY = -2;
									if(Server()->Tick() % 20 == 0)
									{
										SetWeapon(0);
									}
									if(Server()->Tick() % 25 == 0)
									{
										m_Input.m_Fire++;
										m_LatestInput.m_Fire++;
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
							if(m_Core.m_Pos.x < 415 * 32) //bis zum tunnel laufen
							{
								m_Input.m_Direction = 1;
							}
							else if(m_Core.m_Pos.x < 440 * 32 && m_Core.m_Pos.y > 213 * 32) //im tunnel laufen
							{
								m_Input.m_Direction = 1;
							}

							//externe if abfrage weil laufen während sprinegn xD

							if(m_Core.m_Pos.x > 413 * 32 && m_Core.m_Pos.x < 415 * 32) // in den tunnel springen
							{
								m_Input.m_Jump = 1;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
								//m_Input.m_Jump = 0;
							}
							else if(m_Core.m_Pos.x > 428 * 32 - 20 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen
							{
								m_Input.m_Jump = 1;
							}

							// externen springen aufhören für dj

							if(m_Core.m_Pos.x > 428 * 32 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen nicht mehr springen
							{
								m_Input.m_Jump = 0;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
							}

							//nochmal extern weil springen während springen

							if(m_Core.m_Pos.x > 430 * 32 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen springen
							{
								m_Input.m_Jump = 1;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
							}

							if(m_Core.m_Pos.x > 431 * 32 && m_Core.m_Pos.y > 213 * 32) //jump refillen für wayblock spot
							{
								m_Input.m_Jump = 0;
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
					if (m_Core.m_Pos.y < 213 * 32 && m_Core.m_Pos.x > (427 * 32) - 20 && m_Core.m_Pos.x < (428 * 32) + 10) //wenn der bot sich an seinem ruler spot befindet
					{
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich mag diesen ort :)");

					if (!m_Dummy_wb_hooked && !m_Dummy_emergency && !m_Dummy_pushing && m_Core.m_Vel.x > 0.90f) //wenn der bot sich auf das freeze zubewegt obwohl er nicht selber läuft
					{
					// --> er wird wahrscheinlich gehookt oder anderweitig extern angegriffen
					// --> schutzmaßnahmen treffen

					GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "AAAh ich werde angegriffen");
					m_Input.m_Jump = 1;
					}
					m_Dummy_pushing = false;
					m_Dummy_emergency = false;
					m_Dummy_wb_hooked = false;
					}
					*/

					//moved dynamic selfkills outside internal wb spot
					//self kill im freeze
					//New Testy selfkill kill if isFreezed and vel 0
					if(!isFreezed || m_Core.m_Vel.x < -0.5f || m_Core.m_Vel.x > 0.5f || m_Core.m_Vel.y != 0.000000f)
					{
						//mach nichts lol brauche nur das else is einfacher
					}
					else
					{
						if(Server()->Tick() % 150 == 0)
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}

					//Bools zurueck setzten
					m_Dummy_pushing = false;
					m_Dummy_emergency = false;
					m_Dummy_wb_hooked = false;
					m_Dummy_happy = false;

					//normaler internen wb spot stuff

					//if (m_Core.m_Pos.y < 213 * 32) //old new added a x check idk why the was no
					if(m_Core.m_Pos.y < 213 * 32 && m_Core.m_Pos.x > 415 * 32)
					{
						//Old self kill kill if freeze
						//if (m_Core.m_Pos.y < 201 * 32) // decke
						//{
						//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "suicide reason: roof rulerspot");
						//}
						//else if (m_Core.m_Pos.x < 417 * 32 && m_Core.m_Pos.x > 414 * 32 + 17 && isFreezed) //linker streifen xD
						//{
						//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "suicide reason: left wall rulerspot");
						//}

						//New stuff RANDOM STOFF ROFL
						//if the bot is in his wb position an bored and there is no actual danger
						// ---> flick some aim and fire around xD

						//m_Dummy_bored_cuz_nothing_happens = true;

						//dont activate all the time and dunno how to make a cool activator code so fuck it rofl

						if(m_Dummy_bored_cuz_nothing_happens)
						{
							CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
							if(pChr && pChr->IsAlive() && m_FreezeTime == 0)
							{
								if(pChr->m_Pos.x < 429 * 32 && pChr->m_Core.m_Vel.x < 4.3f)
								{
									int x = rand() % 100 - 50;
									int y = rand() % 100;

									m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x + x;
									m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y + y;

									//doesnt work. i dont care. i dont fix. i just comment it out cuz fuck life is a bitch

									//int fr = rand() % 2000;
									//if (fr < 1300)
									//{
									//	m_Dummy_bored_shootin = true;
									//}

									//if (m_Dummy_bored_shootin)
									//{
									//	m_Input.m_Fire++;
									//	m_LatestInput.m_Fire++;

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

						CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this); //position anderer spieler mit pikus aimbot abfragen
						if(pChr && pChr->IsAlive())
						{
							//sometimes walk to enemys.   to push them in freeze or super hammer them away
							if(pChr->m_Pos.y < m_Core.m_Pos.y + 2 && pChr->m_Pos.y > m_Core.m_Pos.y - 9)
							{
								if(pChr->m_Pos.x > m_Core.m_Pos.x)
								{
									m_Input.m_Direction = 1;
								}
								else
								{
									m_Input.m_Direction = -1;
								}
							}

							if(pChr->m_FreezeTime == 0) //if enemy in ruler spot is unfreeze -->notstand panic
							{
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "NOTSTAND");

								if(Server()->Tick() % 30 == 0) //angry emotes machen
								{
									GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9);
								}

								if(Server()->Tick() % 20 == 0)
								{
									SetWeapon(0);
								}

								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								if(m_FreezeTime == 0)
								{
									m_LatestInput.m_Fire++;
									m_Input.m_Fire++;
								}

								//testy sollte eig auch am anfang des modes passen
								//m_Input.m_Direction = 0;

								//if (m_Core.m_HookState == HOOK_FLYING)
								//	m_Input.m_Hook = 1;
								//else if (m_Core.m_HookState == HOOK_GRABBED)
								//	m_Input.m_Hook = 1;
								//else
								//	m_Input.m_Hook = 0;

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
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "boing!");
										m_Input.m_Jump = 1;
										m_Dummy_jumped = true;
									}
									else
									{
										m_Input.m_Jump = 0;
									}

									if(!m_Dummy_hooked)
									{
										if(Server()->Tick() % 30 == 0)
											m_Dummy_hook_delay = true;

										//testy removed hook here i dont know why but all works pretty good still xD
										if(m_Dummy_hook_delay)
											//m_Input.m_Hook = 1;

											if(Server()->Tick() % 200 == 0)
											{
												m_Dummy_hooked = true;
												m_Input.m_Hook = 0;
											}
									}

									if(!m_Dummy_moved_left)
									{
										if(m_Core.m_Pos.x > 419 * 32 + 20)
											m_Input.m_Direction = -1;
										else
											m_Input.m_Direction = 1;

										if(Server()->Tick() % 200 == 0)
										{
											m_Dummy_moved_left = true;
											m_Input.m_Direction = 0;
										}
									}
								}

								//Blocke gefreezte gegner für immer

								//TODO:
								//das is ein linke seite block wenn hier voll is sollte man anders vorgehen
								//                           früher war es y > 210   aber change weil er während er ihn hochzieht dann nicht mehr das hooken aufhört
								if(pChr->m_FreezeTime > 0 && pChr->m_Pos.y > 204 * 32 && pChr->m_Pos.x > 422 * 32) //wenn ein gegner weit genung rechts freeze am boden liegt
								{
									// soll der bot sich einer position links des spielers nähern
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hab n opfer gefunden");

									if(m_Core.m_Pos.x + (5 * 32 + 40) < pChr->m_Pos.x) // er versucht 5 tiles links des gefreezten gegner zu kommen
									{
										m_Input.m_Direction = -1;

										if(m_Core.m_Pos.x > pChr->m_Pos.x && m_Core.m_Pos.x < pChr->m_Pos.x + (4 * 32)) // wenn er 4 tiles rechts des gefreezten gegners is
										{
											m_Input.m_Jump = 1;
											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "boing!");
										}
									}
									else //wenn der bot links des gefreezten spielers is
									{
										m_Input.m_Jump = 1;
										//echo jump
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "boing!");

										m_Input.m_Direction = -1;

										if(m_Core.m_Pos.x < pChr->m_Pos.x) //solange der bot weiter links is
										{
											m_Input.m_Hook = 1;
										}
										else
										{
											m_Input.m_Hook = 0;
										}
									}
								}

								//freeze protection & schieberrei
								//                                                                                                                                                                                                      old (417 * 32 - 60)
								if((pChr->m_Pos.x + 10 < m_Core.m_Pos.x && pChr->m_Pos.y > 211 * 32 && pChr->m_Pos.x < 418 * 32) || (pChr->m_FreezeTime > 0 && pChr->m_Pos.y > 210 * 32 && pChr->m_Pos.x < m_Core.m_Pos.x && pChr->m_Pos.x > 417 * 32 - 60)) // wenn der spieler neben der linken wand linken freeze wand liegt schiebt ihn der bot rein
								{ // oder wenn der spieler weiter weg liegt aber freeze is

									if(!m_Dummy_left_freeze_full) //wenn da niemand is schieb den rein
									{
										// HIER TESTY TESTY CHANGES  211 * 32 + 40 stand hier
										if(pChr->m_Pos.y > 211 * 32 + 40) // wenn der gegner wirklich ganz tief genung is
										{ //                          417 * 32 - 40
											if(m_Core.m_Pos.x > 418 * 32) // aber nicht selber ins freeze laufen
											{
												m_Input.m_Direction = -1;

												//Check ob der gegener freeze is

												if(pChr->m_FreezeTime > 0)
												{
													m_LatestInput.m_Fire = 0; //nicht schiessen ofc xD (doch is schon besser xD)
													m_Input.m_Fire = 0;

													//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
												}

												//letzten stupser geben (sonst gibs bugs kb zu fixen)
												if(pChr->isFreezed) //wenn er schon im freeze is
												{
													m_LatestInput.m_Fire = 1; //hau ihn an die wand
													m_Input.m_Fire = 1;
												}
											}
											else
											{
												m_Input.m_Direction = 1;
												//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 1");
												if(pChr->m_FreezeTime > 0)
												{
													m_LatestInput.m_Fire = 0; //nicht schiessen ofc xD (doch is schon besser xD)
													m_Input.m_Fire = 0;

													//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
												}
												//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "ich halte das auf.");
												//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich will da nich rein laufen");
											}
										}
										else //wenn der gegner nicht tief genung ist
										{
											m_Input.m_Direction = 1;

											if(pChr->m_FreezeTime > 0)
											{
												m_LatestInput.m_Fire = 0; //nicht schiessen ofc xD (doch is schon besser xD)
												m_Input.m_Fire = 0;

												//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
											}
										}
									}
									else //wenn da schon jemand liegt
									{
										// sag das mal xD
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "da liegt schon einer");
									}
								}
								else if(m_Core.m_Pos.x < 419 * 32 + 10) //sonst mehr abstand halten
								{
									m_Input.m_Direction = 1;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 2");
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich will da nich rein laufen");
								}
								// else // wenn nichts los is erstmal stehen bleiben

								//freeze protection decke mit double jump wenn hammer

								if(m_Core.m_Vel.y < 20.0f && m_Core.m_Pos.y < 207 * 32) // wenn der tee nach oben gehammert wird
								{
									if(m_Core.m_Pos.y > 206 * 32) //ab 206 würd er so oder so ins freeze springen
										m_Input.m_Jump = 1;

									if(m_Core.m_Pos.y < pChr->m_Pos.y) //wenn der bot über dem spieler is soll er hooken
										m_Input.m_Hook = 1;
									else
										m_Input.m_Hook = 0;
								}

								//wenn der tee hcoh geschleudert wird
								//                 old 4 (macht aber im postiven bereich kein sinn aber hat geklappt)
								//                 HALLO HIER IST DEIN ICH AUS DER ZUKUNFT: du dummes kind wenn er in der luft hammert dann fliegt er doch nicht nach oben und gerade da musst du es ja perfekt haben ... low
								//if (m_Core.m_Vel.y < 4.0f && m_Core.m_Pos.y < pChr->m_Pos.y) //wenn er schneller als 4 nach oben fliegt und höher als der Gegener ist
								// lass das mit speed weg und mach lieber was mit höhe
								if(m_Core.m_Pos.y < 207 * 32 && m_Core.m_Pos.y < pChr->m_Pos.y)
								{
									//in hammer position bewegen
									if(m_Core.m_Pos.x > 418 * 32 + 20) //nicht ins freeze laufen
									{
										if(m_Core.m_Pos.x > pChr->m_Pos.x - 45) //zu weit rechts von hammer position
										{
											m_Input.m_Direction = -1; //gehe links
												//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich will da nich rein laufen");
										}
										else if(m_Core.m_Pos.x < pChr->m_Pos.x - 39) // zu weit links von hammer position
										{
											m_Input.m_Direction = 1;
											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 3");
										}
										else //im hammer bereich
										{
											m_Input.m_Direction = 0; //bleib da
										}
									}
								}

								//Check ob der gegener freeze is

								if(pChr->m_FreezeTime > 0 && pChr->m_Pos.y > 208 * 32 && !pChr->isFreezed) //wenn der Gegner tief und freeze is macht es wenig sinn den frei zu hammern
								{
									m_LatestInput.m_Fire = 0; //nicht schiessen
									m_Input.m_Fire = 0;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
								}

								//Hau den weg (wie dummymode 21)
								if(pChr->m_Pos.x > 418 * 32 && pChr->m_Pos.y > 209 * 32) //das ganze findet nur im bereich statt wo sonst eh nichts passiert
								{
									//wenn der bot den gegner nicht boosten würde hammer den auch nich weg
									m_LatestInput.m_Fire = 0;
									m_Input.m_Fire = 0;

									if(pChr->m_Core.m_Vel.y < -0.5f && m_Core.m_Pos.y + 15 > pChr->m_Pos.y) //wenn der dude speed nach oben hat
									{
										m_Input.m_Jump = 1;
										if(m_FreezeTime == 0)
										{
											m_LatestInput.m_Fire++;
											m_Input.m_Fire++;
										}
									}
								}

								//TODO: FIX:
								//der bot unfreezed den gegner ab einer gewissen höhe wenn er rein gehammert wird schau das da was passiert

								//wenn ein tee freeze links neben dem bot liegt werf den einfach wieder ins freeze becken
								//das is bisher ja noch die einzige sicherheits lücke beim wayblocken
								//wenn man ein tee über den bot hammert

								if(pChr->m_Pos.x > 421 * 32 && pChr->m_FreezeTick > 0 && pChr->m_Pos.x < m_Core.m_Pos.x)
								{
									m_Input.m_Jump = 1;
									m_Input.m_Hook = 1;
								}

								//##############################
								// doggeystyle dogeing the freeze
								//##############################

								//during the fight dodge the freezefloor on the left with brain
								if(m_Core.m_Pos.x > 428 * 32 + 20 && m_Core.m_Pos.x < 438 * 32 - 20)
								{
									//very nobrain directions decision
									if(m_Core.m_Pos.x < 432 * 32) //left --> go left
									{
										m_Input.m_Direction = -1;
									}
									else //right --> go right
									{
										m_Input.m_Direction = 1;
									}

									//high speed left goto speed
									if(m_Core.m_Vel.x < -6.4f && m_Core.m_Pos.x < 434 * 32)
									{
										m_Input.m_Direction = -1;
									}

									//low speed to the right
									if(m_Core.m_Pos.x > 431 * 32 + 20 && m_Core.m_Vel.x > 3.3f)
									{
										m_Input.m_Direction = 1;
									}
								}

								//m_pPlayer->m_TeeInfos.m_ColorBody = (0 * 255 / 360);
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Enemy in ruler spot found!");
							}
							else //sonst normal wayblocken und 427 aufsuchen
							{
								//m_Core.m_ActiveWeapon = WEAPON_GUN;
								SetWeapon(1);
								m_Input.m_Jump = 0;

								if(m_Core.m_HookState == HOOK_FLYING)
									m_Input.m_Hook = 1;
								else if(m_Core.m_HookState == HOOK_GRABBED)
									m_Input.m_Hook = 1;
								else
									m_Input.m_Hook = 0;

								//m_pPlayer->m_TeeInfos.m_ColorBody = (120 * 255 / 360);
								//positions check and correction 427

								m_Dummy_jumped = false;
								m_Dummy_hooked = false;
								m_Dummy_moved_left = false;

								if(m_Core.m_Pos.x > 428 * 32 + 15 && m_Dummy_ruled) //wenn viel zu weit ausserhalb der ruler area wo der bot nur hingehookt werden kann
								{
									m_Input.m_Jump = 1;
									m_Input.m_Hook = 1;
								}

								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Prüfe ob zu weit rechts");
								if(m_Core.m_Pos.x < (418 * 32) - 10) // zu weit links -> geh rechts
								{
									m_Input.m_Direction = 1;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 4");
								}
								else if(m_Core.m_Pos.x > (428 * 32) + 10) // zu weit rechts -> geh links
								{
									m_Input.m_Direction = -1;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich bin zuweit rechts...");
								}
								else // im toleranz bereich -> stehen bleiben
								{
									m_Dummy_happy = true;
									m_Dummy_ruled = true;
									m_Input.m_Direction = 0;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "toleranz bereich");
									//m_Input.m_LatestTargetX = 0;
									//m_Input.m_LatestTargetY = 0;

									//stuff im toleranz bereich doon

									// normal wayblock
									CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this); //position anderer spieler mit pikus aimbot abfragen
									if(pChr && pChr->IsAlive())
									{
										//m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
										//m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

										//m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
										//m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

										//m_Input.m_TargetX = 1;//pChr->m_Pos.x - m_Pos.x; //1
										//m_Input.m_TargetY = 0;//pChr->m_Pos.y - m_Pos.y; //0

										//m_LatestInput.m_TargetX = 1;//pChr->m_Pos.x - m_Pos.x;
										//m_LatestInput.m_TargetY = 0;//pChr->m_Pos.y - m_Pos.y;

										//i dunno why there is a if statement under this code and i dont wanna use it
										//so i made Trick[4] external (woo spagethii code)
										//Trick[4] clears the left freeze
										if(pChr->m_Pos.x < 418 * 32 - 10 && pChr->m_Pos.y > 210 * 32 && pChr->m_Pos.y < 213 * 32 && pChr->isFreezed && pChr->m_Core.m_Vel.y == 0.00f)
										{
											m_DummyFreezeBlockTrick = 4;
										}

										//                                                                                            old was: 418 * 32 + 20          and i have no fkin idea why so i changed to 17
										if(pChr->m_Pos.y < 213 * 32 + 10 && pChr->m_Pos.x < 430 * 32 && pChr->m_Pos.y > 210 * 32 && pChr->m_Pos.x > 417 * 32) // wenn ein spieler auf der linken seite in der ruler area is
										{
											//wenn ein gegner rechts des bots is prepare für trick[1]
											if(m_Core.m_Pos.x < pChr->m_Pos.x && pChr->m_Pos.x < 429 * 32 + 6)
											{
												m_Input.m_Direction = -1;
												m_Input.m_Jump = 0;

												if(m_Core.m_Pos.x < 422 * 32)
												{
													m_Input.m_Jump = 1;
													m_DummyFreezeBlockTrick = 1;
													//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "trick1: hook to the left");
												}
											}
											//wenn ein gegner links des bots is prepare für tick[3]
											if(m_Core.m_Pos.x > pChr->m_Pos.x && pChr->m_Pos.x < 429 * 32)
											{
												m_Input.m_Direction = 1;
												m_Input.m_Jump = 0;

												if(m_Core.m_Pos.x > 427 * 32 || m_Core.m_Pos.x > pChr->m_Pos.x + (5 * 32))
												{
													m_Input.m_Jump = 1;
													m_DummyFreezeBlockTrick = 3;
													//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "trick3: hook to the right");
												}
											}
										}
										else // wenn spieler irgendwo anders is
										{
											//wenn ein spieler rechts des freezes is und freeze is Tric[2]
											if(pChr->m_Pos.x > 433 * 32 && pChr->m_FreezeTime > 0 && IsGrounded())
											{
												m_DummyFreezeBlockTrick = 2;
												//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "trick2: swinger");
											}
										}
									}
								}
							}
						}
						else // wenn kein lebender spieler im ruler spot ist
						{
							//Suche trozdem 427 auf

							if(m_Core.m_Pos.x < (427 * 32) - 20) // zu weit links -> geh rechts
							{
								m_Input.m_Direction = 1;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: special");
							}
							else if(m_Core.m_Pos.x > (427 * 32) + 40) // zu weit rechts -> geh links
							{
								m_Input.m_Direction = -1;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich bin zuweit rechts...");
							}
							//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
						}

						// über das freeze springen wenn rechts der bevorzugenten camp stelle

						if(m_Core.m_Pos.x > 434 * 32)
						{
							m_Input.m_Jump = 1;
						}
						else if(m_Core.m_Pos.x > (434 * 32) - 20 && m_Core.m_Pos.x < (434 * 32) + 20) // bei flug über freeze jump wd holen
						{
							m_Input.m_Jump = 0;
							//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "refilling jump");
						}

						//new testy moved tricks into Wayblocker area (y < 213 && x > 215) (was external)
						//TRICKS
						if(1 == 1)
						{
							CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this);
							if(pChr && pChr->IsAlive())
							{
								if(!m_Dummy_emergency && m_Core.m_Pos.x > 415 && m_Core.m_Pos.y < 213 * 32 && m_DummyFreezeBlockTrick != 0) //as long as no enemy is unfreeze in base --->  do some trickzz
								{
									//Trick reset all
									//resett in the tricks because trick1 doesnt want it
									//m_Input.m_Hook = 0;
									//m_Input.m_Jump = 0;
									//m_Input.m_Direction = 0;
									//m_LatestInput.m_Fire = 0;
									//m_Input.m_Fire = 0;

									//off tricks when not gud to make tricks#
									if(pChr->m_FreezeTime == 0)
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
										if(pChr->isFreezed)
										{
											m_DummyFreezeBlockTrick = 0; //stop trick if enemy is in freeze
										}
										m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
										m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
										m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
										m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

										if(Server()->Tick() % 40 == 0)
										{
											SetWeapon(0);
										}

										if(pChr->m_Pos.x < m_Core.m_Pos.x && pChr->m_Pos.x > m_Core.m_Pos.x - 180) //if enemy is on the left in hammer distance
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}

										if(m_Core.m_Pos.y < 210 * 32 + 10)
										{
											m_Dummy_start_hook = true;
										}

										if(m_Dummy_start_hook)
										{
											if(Server()->Tick() % 80 == 0 || pChr->m_Pos.x < m_Core.m_Pos.x + 22)
											{
												m_Dummy_start_hook = false;
											}
										}

										if(m_Dummy_start_hook)
										{
											m_Input.m_Hook = 1;
										}
										else
										{
											m_Input.m_Hook = 0;
										}
									}
									else if(m_DummyFreezeBlockTrick == 2) //enemy on the right plattform --> swing him away
									{
										m_Input.m_Hook = 0;
										m_Input.m_Jump = 0;
										m_Input.m_Direction = 0;
										m_LatestInput.m_Fire = 0;
										m_Input.m_Fire = 0;

										if(Server()->Tick() % 50 == 0)
										{
											m_Dummy_bored_counter++;
											GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
										}

										if(m_Core.m_Pos.x < 438 * 32) //first go right
										{
											m_Input.m_Direction = 1;
										}

										if(m_Core.m_Pos.x > 428 * 32 && m_Core.m_Pos.x < 430 * 32) //first jump
										{
											m_Input.m_Jump = 1;
										}

										if(m_Core.m_Pos.x > 427 * 32) //aim and swing
										{
											m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
											m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
											m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
											m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

											if(m_Core.m_Pos.x > 427 * 32 + 30 && pChr->m_Pos.y < 214 * 32)
											{
												m_Input.m_Hook = 1;
												if(Server()->Tick() % 10 == 0)
												{
													int x = rand() % 100 - 50;
													int y = rand() % 100 - 50;

													m_Input.m_TargetX = x;
													m_Input.m_TargetY = y;
												}
												//random shooting xD
												int r = rand() % 200 + 10;
												if(Server()->Tick() % r == 0 && m_FreezeTime == 0)
												{
													m_Input.m_Fire++;
													m_LatestInput.m_Fire++;
												}
											}
										}

										//also this trick needs some freeze dogining because sometime huge speed fucks the bot
										//and NOW THIS CODE is here to fuck the high speed
										// yo!
										if(m_Core.m_Pos.x > 440 * 32)
										{
											m_Input.m_Direction = -1;
										}
										if(m_Core.m_Pos.x > 439 * 32 + 20 && m_Core.m_Pos.x < 440 * 32)
										{
											m_Input.m_Direction = 0;
										}
									}
									else if(m_DummyFreezeBlockTrick == 3) //enemy on the left swing him to the right
									{
										m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
										m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
										m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
										m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

										if(m_Core.m_Pos.y < 210 * 32 + 10)
										{
											m_Dummy_start_hook = true;
											m_Dummy_trick3_start_count = true;
										}

										if(m_Dummy_start_hook)
										{
											if(Server()->Tick() % 80 == 0 || pChr->m_Pos.x > m_Core.m_Pos.x - 22)
											{
												m_Dummy_start_hook = false;
											}
										}

										if(m_Dummy_start_hook)
										{
											m_Input.m_Hook = 1;
										}
										else
										{
											m_Input.m_Hook = 0;
										}

										if(m_Core.m_Pos.x < 429 * 32)
										{
											m_Input.m_Direction = 1;
										}
										else
										{
											m_Input.m_Direction = -1;
										}

										//STOPPER hook:
										//hook the tee if he flys to much to the right
										if(pChr->m_Pos.x > 433 * 32 + 20)
										{
											m_Input.m_Hook = 1;
										}

										//Hook the tee agian and go to the left -> drag him under block area
										//-->Trick 5
										if(pChr->m_Core.m_Vel.y > 8.1f && pChr->m_Pos.x > 429 * 32 + 1 && pChr->m_Pos.y > 209 * 32)
										{
											m_DummyFreezeBlockTrick = 5;
											m_Input.m_Hook = 1;
										}

										//if he lands on the right plattform switch trick xD
										//doesnt work anysways (now fixed by the stopper hook)
										if(pChr->m_Pos.x > 433 * 32 && pChr->m_Core.m_Vel.y == 0.0f)
										{
											m_DummyFreezeBlockTrick = 2;
											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "trick gone wrong --> change trick");
										}

										//Check for trick went wrong --> trick3 panic activation
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
											if(pChr->m_Pos.x < 430 * 32 && pChr->m_Pos.x > 426 * 32 + 10 && pChr->IsGrounded())
											{
												m_Dummy_trick3_panic = true;
												m_Dummy_trick3_panic_left = true;
											}
										}
										if(m_Dummy_trick3_panic)
										{
											//stuck --> go left and swing him down
											m_Input.m_Direction = 1;
											if(m_Core.m_Pos.x < 425 * 32)
											{
												m_Dummy_trick3_panic_left = false;
											}

											if(m_Dummy_trick3_panic_left)
											{
												m_Input.m_Direction = -1;
											}
											else
											{
												if(m_Core.m_Pos.y < 212 * 32 + 10)
												{
													m_Input.m_Hook = 1;
												}
											}
										}
									}
									else if(m_DummyFreezeBlockTrick == 4) //clear left freeze
									{
										m_Input.m_Hook = 0;
										m_Input.m_Jump = 0;
										m_Input.m_Direction = 0;
										m_LatestInput.m_Fire = 0;
										m_Input.m_Fire = 0;

										if(!m_Dummy_trick4_hasstartpos)
										{
											if(m_Core.m_Pos.x < 423 * 32 - 10)
											{
												m_Input.m_Direction = 1;
											}
											else if(m_Core.m_Pos.x > 424 * 32 - 20)
											{
												m_Input.m_Direction = -1;
											}
											else
											{
												m_Dummy_trick4_hasstartpos = true;
											}
										}
										else //has start pos
										{
											m_Input.m_TargetX = -200;
											m_Input.m_TargetY = -2;
											if(pChr->isFreezed)
											{
												m_Input.m_Hook = 1;
											}
											else
											{
												m_Input.m_Hook = 0;
												m_DummyFreezeBlockTrick = 0; //fuck it too lazy normal stuff shoudl do the rest xD
											}
											if(Server()->Tick() % 7 == 0)
											{
												m_Input.m_Hook = 0;
											}
										}
									}
									else if(m_DummyFreezeBlockTrick == 5) //Hook under blockarea to the left (mostly the end of a trick)
									{
										//For now this trick only gets triggerd in trick 3 at the end

										//TODO: this trick needs a tick

										m_Input.m_Hook = 1;

										if(m_Core.m_HookState == HOOK_GRABBED)
										{
											m_Input.m_Direction = -1;
										}
										else
										{
											if(m_Core.m_Pos.x < 428 * 32 + 20)
											{
												m_Input.m_Direction = 1;
											}
										}
									}
								}
							}
							else //nobody alive in ruler area --> stop tricks
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

					//##################################
					// 29 only protections and doge moves
					//##################################

					//Super last jumpy freeze protection o.O
					//saves alot bot live im very sure
					//#longlivesthebotrofl

					if(m_Core.m_Pos.x > 429 * 32 && m_Core.m_Pos.x < 436 * 32 && m_Core.m_Pos.y < 214 * 32) //dangerous area over the freeze
					{
						//first check! too low?
						if(m_Core.m_Pos.y > 211 * 32 + 10 && IsGrounded() == false)
						{
							m_Input.m_Jump = 1;
							m_Input.m_Hook = 1;
							if(Server()->Tick() % 4 == 0)
							{
								m_Input.m_Jump = 0;
							}
						}

						//second check! too speedy?
						if(m_Core.m_Pos.y > 209 * 32 && m_Core.m_Vel.y > 5.6f)
						{
							m_Input.m_Jump = 1;
							if(Server()->Tick() % 4 == 0)
							{
								m_Input.m_Jump = 0;
							}
						}
					}

					//survival moves above the second freeze in the ruler from the left
					// ascii art shows where :
					//
					//                   |
					//                   |
					//                   v
					//                        --------
					//-----#####----###########-######
					//###########-####################
					//           #
					//           #
					//           -#########################----------
					//           #--------------------------

					if(m_Core.m_Pos.x > 439 * 32 && m_Core.m_Pos.x < 449 * 32)
					{
						//low left lowspeed --> go left
						if(m_Core.m_Pos.x > 439 * 32 && m_Core.m_Pos.y > 209 * 32 && m_Core.m_Vel.x < 3.0f)
						{
							m_Input.m_Direction = -1;
						}
						//low left highrightspeed --> go right with the speed and activate some random modes to keep the speed xD
						if(m_Core.m_Pos.x > 439 * 32 && m_Core.m_Pos.y > 209 * 32 && m_Core.m_Vel.x > 6.0f && m_Core.m_Jumped < 2)
						{
							m_Input.m_Direction = 1;
							m_Input.m_Jump = 1;
							if(Server()->Tick() % 5 == 0)
							{
								m_Input.m_Jump = 0;
							}
							m_Dummy_speedright = true;
						}

						if(isFreezed || m_Core.m_Vel.x < 4.3f)
						{
							m_Dummy_speedright = false;
						}

						if(m_Dummy_speedright)
						{
							m_Input.m_Direction = 1;
							m_Input.m_TargetX = 200;
							int r = rand() % 200 - 100;
							m_Input.m_TargetY = r;
							m_Input.m_Hook = 1;
							if(Server()->Tick() % 30 == 0 && m_Core.m_HookState != HOOK_GRABBED)
							{
								m_Input.m_Hook = 0;
							}
						}
					}
					else //out of the freeze area resett bools
					{
						m_Dummy_speedright = false;
					}

					//go down on plattform to get dj
					//bot always fails going back from right
					//because he doesnt refills his dj

					//            |
					//            |
					//            v
					//                        --------
					//-----#####----###########-######
					//###########-####################
					//           #
					//           #
					//           -#########################----------
					//           #--------------------------

					if(m_Core.m_Pos.x > 433 * 32 + 20 && m_Core.m_Pos.x < 437 * 32 && m_Core.m_Jumped > 2)
					{
						m_Input.m_Direction = 1;
					}

					//##########################################
					// S P E C I A L    S H I T ! ! !          #
					//##########################################             agian...

					//woo special late important new stuff xD
					//reached hammerfly plattform --> get new movement skills
					//this area has his own extra codeblock with cool stuff

					if(m_Core.m_Pos.x > 448 * 32)
					{
						m_Input.m_Hook = 0;
						m_Input.m_Jump = 0;
						m_Input.m_Direction = 0;
						m_LatestInput.m_Fire = 0;
						m_Input.m_Fire = 0;

						if(m_Core.m_Pos.x < 451 * 32 + 20 && IsGrounded() == false && m_Core.m_Jumped > 2)
						{
							m_Input.m_Direction = 1;
						}
						if(m_Core.m_Vel.x < -0.8f && m_Core.m_Pos.x < 450 * 32 && IsGrounded())
						{
							m_Input.m_Jump = 1;
						}
						CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
						if(pChr && pChr->IsAlive())
						{
							if(m_Core.m_Pos.x < 451 * 32)
							{
								m_Input.m_Direction = 1;
							}

							if(pChr->m_Pos.x < m_Core.m_Pos.x - 7 * 32 && m_Core.m_Jumped < 2) //if enemy is on the left & bot has jump --> go left too
							{
								m_Input.m_Direction = -1;
							}
							if(m_Core.m_Pos.x > 454 * 32)
							{
								m_Input.m_Direction = -1;
								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								if(m_Core.m_Pos.y + 40 > pChr->m_Pos.y)
								{
									m_Input.m_Hook = 1;
									if(Server()->Tick() % 50 == 0)
									{
										m_Input.m_Hook = 0;
									}
								}
							}

							//second freeze from the right --> goto singel palttform
							if(m_Core.m_Pos.x > 464 * 32 && m_Core.m_Jumped > 2 && m_Core.m_Pos.x < 468 * 32)
							{
								m_Input.m_Direction = 1;
							}
							//go back
							if(m_Core.m_Pos.x < 468 * 32 && IsGrounded() && m_Core.m_Pos.x > 464 * 32)
							{
								m_Input.m_Jump = 1;
							}
							//balance
							if(m_Core.m_Pos.x > 460 * 32 && m_Core.m_Pos.x < 464 * 32 && m_Core.m_Pos.y > 210 * 32 + 10)
							{
								m_Dummy_doBalance = true;
							}
							if(IsGrounded() && isFreezed)
							{
								m_Dummy_doBalance = false;
							}

							if(m_Dummy_doBalance)
							{
								if(m_Core.m_Pos.x > 463 * 32) //go right
								{
									if(m_Core.m_Pos.x > pChr->m_Pos.x - 4)
									{
										m_Input.m_Direction = -1;
									}
									else if(m_Core.m_Pos.x > pChr->m_Pos.x)
									{
										m_Input.m_Direction = 1;
									}

									if(m_Core.m_Pos.x < pChr->m_Pos.x)
									{
										m_Input.m_TargetX = 5;
										m_Input.m_TargetY = 200;
										if(m_Core.m_Pos.x - 1 < pChr->m_Pos.x)
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}
									}
									else
									{
										//do the random flick
										int r = rand() % 100 - 50;
										m_Input.m_TargetX = r;
										m_Input.m_TargetY = -200;
									}
									if(pChr->m_Pos.x > 465 * 32 - 10 && pChr->m_Pos.x < 469 * 32) //right enough go out
									{
										m_Input.m_Direction = 1;
									}
								}
								else //go left
								{
									if(m_Core.m_Pos.x > pChr->m_Pos.x + 4)
									{
										m_Input.m_Direction = -1;
									}
									else if(m_Core.m_Pos.x < pChr->m_Pos.x)
									{
										m_Input.m_Direction = 1;
									}

									if(m_Core.m_Pos.x > pChr->m_Pos.x)
									{
										m_Input.m_TargetX = 5;
										m_Input.m_TargetY = 200;
										if(m_Core.m_Pos.x + 1 > pChr->m_Pos.x)
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}
									}
									else
									{
										//do the random flick
										int r = rand() % 100 - 50;
										m_Input.m_TargetX = r;
										m_Input.m_TargetY = -200;
									}
									if(pChr->m_Pos.x < 459 * 32) //left enough go out
									{
										m_Input.m_Direction = -1;
									}
								}
							}
						}
						else //no close enemy alive
						{
							if(m_Core.m_Jumped < 2)
							{
								m_Input.m_Direction = -1;
							}
						}

						//jump protection second freeze from the right

						//                                  |            -########
						//                                  |            -########
						//                                  v                    #
						//                                                       #
						//                -------------##########---##############
						// ...#####---#######-########------------ ---------------

						if(m_Core.m_Pos.x > 458 * 32 && m_Core.m_Pos.x < 466 * 32)
						{
							if(m_Core.m_Pos.y > 211 * 32 + 26)
							{
								m_Input.m_Jump = 1;
							}
							if(m_Core.m_Pos.y > 210 * 32 && m_Core.m_Vel.y > 5.4f)
							{
								m_Input.m_Jump = 1;
							}
						}

						//go home if its oky, oky?
						if((m_Core.m_Pos.x < 458 * 32 && IsGrounded() && pChr->isFreezed) || (m_Core.m_Pos.x < 458 * 32 && IsGrounded() && pChr->m_Pos.x > m_Core.m_Pos.x + (10 * 32)))
						{
							m_Input.m_Direction = -1;
						}
						//keep going also in the air xD
						if(m_Core.m_Pos.x < 450 * 32 && m_Core.m_Vel.x < 1.1f && m_Core.m_Jumped < 2)
						{
							m_Input.m_Direction = -1;
						}

						//                                           |   -########
						//                                           |   -########
						//                                           v           #
						//                                                       #
						//                -------------##########---##############
						// ...#####---#######-########------------ ---------------

						//go back if too far
						if(m_Core.m_Pos.x > 468 * 32 + 20)
						{
							m_Input.m_Direction = -1;
						}
					}

					//shitty nub like jump correction because i am too lazy too fix bugsis
					//T0D0(done): fix this bugsis
					//the bot jumps somehow at spawn if a player is in the ruler area
					//i was working with dummybored and tricks
					//because i cant find the bug i set jump to 0 at spawn

					//here is ChillerDragon from ze future!
					// FUCK YOU old ChillerDragon you just wasted my fucking time with this shitty line
					//im working on another movement where i need jumps at spawn and it took me 20 minutes to find this shitty line u faggot!
					//wow ofc the bot does shit at spawn because u only said the ruler area is (m_Core.m_Pos.y < 213 * 32) and no check on X omg!
					//hope this wont happen agian! (talking to you future dragon)

					//if (m_Core.m_Pos.x < 407 * 32)
					//{
					//	m_Input.m_Jump = 0;
					//}
				}
			}
			else //Change to mode main and reset all
			{
				GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "EROOR!!!!");
				//RestOnChange (zuruecksetzten)
				m_Input.m_Hook = 0;
				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;

				m_Dummy_mode18 = 0;
			}
		}
		else if(m_pPlayer->m_DummyMode == 30) //ChillBlock5 the down left balance to secret moneyroom
		{
			//rest dummy
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			//hardcodetselfkills:
			if(m_Core.m_Pos.x > 404 * 32)
			{
				Die(m_pPlayer->GetCID(), WEAPON_SELF);
			}
			if(m_Core.m_Pos.y < 204 * 32)
			{
				Die(m_pPlayer->GetCID(), WEAPON_SELF);
			}
			if(m_Core.m_Pos.y < 215 * 32 && m_Core.m_Pos.x < 386 * 32 - 3)
			{
				Die(m_pPlayer->GetCID(), WEAPON_SELF);
			}
			//selfkill
			//dyn
			if(m_Core.m_Vel.y == 0.000000f && m_Core.m_Vel.x < 0.01f && m_Core.m_Vel.x > -0.01f && isFreezed)
			{
				if(Server()->Tick() % 20 == 0)
				{
					GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3);
				}

				if(Server()->Tick() % 90 == 0)
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}
			}

			//balance
			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
			if(pChr && pChr->IsAlive())
			{
				//m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				//m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				//m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				//m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				m_Input.m_TargetX = 2;
				m_Input.m_TargetY = 200;
				m_LatestInput.m_TargetX = 2;
				m_LatestInput.m_TargetY = 200;

				if(pChr->m_Pos.y > m_Core.m_Pos.y && m_Core.m_Pos.x > 310 * 32)
				{
					m_Input.m_Direction = 1;
					if(pChr->m_Pos.x < m_Core.m_Pos.x - 3)
					{
						m_Input.m_Direction = -1;
					}
					if(pChr->m_Pos.x > m_Core.m_Pos.x + 1)
					{
						m_Input.m_Direction = 1;
					}
					if(m_Core.m_Pos.x > pChr->m_Pos.x + 1 && m_Core.m_Pos.y > 238 * 32 && pChr->IsGrounded() && m_Core.m_Vel.x < -0.002f)
					{
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
						m_Input.m_Direction = -1;
					}
				}
			}

			//movement going down#
			if(m_Core.m_Pos.y < 238 * 32 && m_Core.m_Pos.x > 344 * 32)
			{
				if(m_Core.m_Pos.x > 390 * 32)
				{
					m_Input.m_Direction = -1;
				}
				if(m_Core.m_Pos.x < 388 * 32)
				{
					m_Input.m_Direction = 1;
				}
			}
			else
			{
				if(Server()->Tick() % 40 == 0)
				{
					SetWeapon(0);
				}
			}

			if(m_Core.m_Pos.x < 314 * 32 - 10 && m_Core.m_Vel.x < -0.001f)
			{
				CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
				if(pChr && pChr->IsAlive())
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					if(m_Core.m_Pos.x > 310 * 32)
					{
						m_Input.m_Jump = 1;
					}
					if(m_Core.m_Pos.x > 305 * 32)
					{
						m_Input.m_Direction = -1;
					}
					if(m_Core.m_Pos.x < 308 * 32 + 10 && pChr->m_FreezeTime > 0)
					{
						m_Input.m_Hook = 1;
					}
				}
			}
		}
		else if(m_pPlayer->m_DummyMode == 31) //ChillBlock5 Police Guard
		{
			//rest dummy
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			//Basic Stuff:
			//tele from spawn into police base
			//if (m_Core.m_Pos.x < 410 * 32 && m_Core.m_Pos.x > 380 * 32 && m_Core.m_Pos.y < 219 * 32 && m_Core.m_Pos.y > 200 * 32) //spawn area
			if(m_Core.m_Pos.x < 460 * 32) //spawn
			{
				m_Core.m_Pos.x = 484 * 32;
				m_Core.m_Pos.y = 234 * 32;
				m_Dummy_SpawnAnimation = true;
			}
			//do spawnanimation in police base
			if(m_Dummy_SpawnAnimation)
			{
				m_Dummy_SpawnAnimation_delay++;
				if(m_Dummy_SpawnAnimation_delay > 2)
				{
					GameServer()->CreatePlayerSpawn(m_Pos);
					m_Dummy_SpawnAnimation = false;
				}
			}

			//selfkill
			//dyn
			if(m_Core.m_Vel.y == 0.000000f && m_Core.m_Vel.x < 0.01f && m_Core.m_Vel.x > -0.01f && isFreezed)
			{
				if(Server()->Tick() % 20 == 0)
				{
					GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3);
				}

				if(Server()->Tick() % 200 == 0)
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
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

			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
			if(pChr && pChr->IsAlive())
			{
				//for (int i = 0; i < MAX_CLIENTS; i++)
				//{
				//	if (p)
				//}
				m_Dummy_ClosestPolice = false;
				//If a policerank escapes from jail he is treated like a non police
				if((pChr->m_pPlayer->m_PoliceRank > 0 && pChr->m_pPlayer->m_EscapeTime == 0) || (pChr->m_pPlayer->m_PoliceHelper && pChr->m_pPlayer->m_EscapeTime == 0))
				{
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hello officär");
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

				* [STRUC][1]: Check what sub-mode shoudl be used

				* [STRUC][2]: Do stuff depending on sub-modes

				* [STRUC][3]: Do basic movement depending on sub-modes ( step 2 for all modes higher than 3)


				modes:

				0				LOCAL: NOTHING IS GOING ON
				1				LOCAL: ENEMY ATTACK
				2				LOCAL: POLICE HELP
				3				EXTERNAL: ENEMY ATTACK (right side / jail side)
				4				EXTERNAL: POLICE HELP (right side / jail side)

				*/

				//##############################################
				//[STRUC][1]: Check what sub-mode shoudl be used
				//##############################################
				if(m_Dummy_ClosestPolice) //police
				{
					if(pChr->m_FreezeTime > 0 && m_Core.m_Pos.x < 477 * 32)
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
						if(pChr->m_Pos.x > 481 * 32)
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
				//[STRUC][2]: Do stuff depending on sub - modes
				//##############################################

				if(m_Dummy_dmm31 == 0) //nothing is going on
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					if(Server()->Tick() % 90 == 0)
					{
						SetWeapon(1);
					}
				}
				else if(m_Dummy_dmm31 == 1) //Attack enemys
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					if(Server()->Tick() % 30 == 0)
					{
						SetWeapon(0);
					}

					if(m_FreezeTime == 0 && pChr->m_FreezeTime == 0 && pChr->m_Core.m_Vel.y < -0.5 && pChr->m_Pos.x > m_Core.m_Pos.x - 3 * 32 && pChr->m_Pos.x < m_Core.m_Pos.x + 3 * 32)
					{
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
					}

					m_Dummy_AttackMode = 0;
					if(m_Core.m_Pos.x < 466 * 32 + 20 && pChr->m_Pos.x > 469 * 32 + 20) //hook enemy in air (rightside)
					{
						m_Dummy_AttackMode = 1;
					}

					if(m_Dummy_AttackMode == 0) //default mode
					{
						if(m_Core.m_Pos.x < 466 * 32 - 5) //only get bored on lovley place
						{
							m_Input.m_Direction = rand() % 2;
							if(IsGrounded())
							{
								m_Input.m_Jump = rand() % 2;
							}
							if(pChr->m_Pos.y > m_Core.m_Pos.y)
							{
								m_Input.m_Hook = 1;
							}
						}
					}
					else if(m_Dummy_AttackMode == 1) //hook enemy escaping (rightside)
					{
						if(pChr->m_Core.m_Vel.x > 1.3f)
						{
							m_Input.m_Hook = 1;
						}
					}

					//Dont Hook enemys back in safety
					if((pChr->m_Pos.x < 460 * 32 && pChr->m_Pos.x > 457 * 32) || (pChr->m_Pos.x < 469 * 32 && pChr->m_Pos.x > 466 * 32))
					{
						m_Input.m_Hook = 0;
					}
				}
				else if(m_Dummy_dmm31 == 2) //help police dudes
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					if(pChr->m_Pos.y > m_Core.m_Pos.y)
					{
						m_Input.m_Hook = 1;
					}
					if(Server()->Tick() % 40 == 0)
					{
						m_Input.m_Hook = 0;
						m_Input.m_Jump = 0;
					}
					if(IsGrounded() && pChr->isFreezed)
					{
						m_Input.m_Jump = 1;
					}

					if(pChr->isFreezed)
					{
						if(pChr->m_Pos.x > m_Core.m_Pos.x)
						{
							m_Input.m_Direction = 1;
						}
						else if(pChr->m_Pos.x < m_Core.m_Pos.x)
						{
							m_Input.m_Direction = -1;
						}
					}
					else
					{
						if(pChr->m_Pos.x - 110 > m_Core.m_Pos.x)
						{
							m_Input.m_Direction = 1;
						}
						else if(pChr->m_Pos.x + 110 < m_Core.m_Pos.x)
						{
							m_Input.m_Direction = -1;
						}
						else
						{
							if(Server()->Tick() % 10 == 0)
							{
								SetWeapon(0);
							}
							if(m_FreezeTime == 0 && pChr->m_FreezeTime > 0)
							{
								m_Input.m_Fire++;
								m_LatestInput.m_Fire++;
							}
						}
					}

					//invert direction if hooked the player to add speed :)
					if(m_Core.m_HookState == HOOK_GRABBED)
					{
						if(pChr->m_Pos.x > m_Core.m_Pos.x)
						{
							m_Input.m_Direction = -1;
						}
						else if(pChr->m_Pos.x < m_Core.m_Pos.x)
						{
							m_Input.m_Direction = 1;
						}
					}

					//schleuderprotection   stop hook if mate is safe to prevemt blocking him to the other side
					if(pChr->m_Pos.x > 460 * 32 + 10 && pChr->m_Pos.x < 466 * 32)
					{
						m_Input.m_Hook = 0;
					}
				}
				else if(m_Dummy_dmm31 == 3) //EXTERNAL: Enemy attack (rigt side /jail side)
				{
					if(m_Core.m_Pos.x < 461 * 32)
					{
						m_Input.m_Direction = 1;
					}
					else
					{
						if(m_Core.m_Pos.x < 484 * 32)
						{
							m_Input.m_Direction = 1;
						}
						if(m_Core.m_Pos.x > 477 * 32 && !IsGrounded())
						{
							m_Input.m_Hook = 1;
						}
					}

					//jump all the time xD
					if(IsGrounded() && m_Core.m_Pos.x > 480 * 32)
					{
						m_Input.m_Jump = 1;
					}

					//Important jump protection
					if(m_Core.m_Pos.x > 466 * 32 && m_Core.m_Pos.y > 240 * 32 + 8 && m_Core.m_Pos.x < 483 * 32)
					{
						m_Input.m_Jump = 1;
					}
				}
				else //unknown dummymode
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					if(Server()->Tick() % 120 == 0)
					{
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "error: unknow sub-mode for this dummymode set.");
					}
				}
			}

			//##############################################
			//[STRUC][3]: Do basic movement depending on sub - modes
			//##############################################

			//The basic movements depending on the dummysubmodes
			//but some submodes use the same thats why its listed external here
			//Movement
			//JailSpawn
			if(m_Dummy_dmm31 < 3)
			{
				if(m_Core.m_Pos.x > 482 * 32 + 20 && m_Core.m_Pos.y < 236 * 32)
				{
					if(m_Core.m_Vel.x > -8.2f && m_Core.m_Pos.x < 484 * 32 - 20)
					{
						m_Dummy_GetSpeed = true;
					}
					if(m_Core.m_Pos.x > 483 * 32 && !IsGrounded())
					{
						m_Dummy_GetSpeed = true;
					}
					if(m_Core.m_Vel.y > 5.3f)
					{
						m_Dummy_GetSpeed = true;
					}

					if(IsGrounded() && m_Core.m_Pos.x > 485 * 32)
					{
						m_Dummy_GetSpeed = false;
					}

					if(m_Dummy_GotStuck)
					{
						m_Input.m_Direction = -1;
						if(Server()->Tick() % 33 == 0)
						{
							m_Input.m_Jump = 1;
						}
						if(Server()->Tick() % 20 == 0)
						{
							SetWeapon(0); //hammer
						}

						if(m_Input.m_TargetX < -20)
						{
							if(m_FreezeTime == 0)
							{
								m_Input.m_Fire++;
								m_LatestInput.m_Fire++;
							}
						}
						else if(m_Input.m_TargetX > 20)
						{
							m_Input.m_Hook = 1;
							if(Server()->Tick() % 25 == 0)
							{
								m_Input.m_Hook = 0;
							}
						}

						//gets false in the big esle m_Dummy_GotStuck = false;
					}
					else
					{
						if(m_Dummy_GetSpeed)
						{
							m_Input.m_Direction = 1;
							if(Server()->Tick() % 90 == 0)
							{
								m_Dummy_GotStuck = true;
							}
						}
						else
						{
							m_Input.m_Direction = -1;
							if(m_Core.m_Vel.x > -4.4f)
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
					if(m_Core.m_Pos.x > 464 * 32)
					{
						m_Input.m_Direction = -1;
					}
					else if(m_Core.m_Pos.x < 461 * 32)
					{
						m_Input.m_Direction = 1;
					}

					if(m_Core.m_Pos.x > 466 * 32 && m_Core.m_Pos.y > 240 * 32 + 8)
					{
						m_Input.m_Jump = 1;
					}
				}
			}
			else
			{
				//no basic moves for this submode
			}
		}
		else if(m_pPlayer->m_DummyMode == 32) // solo police base bot using 5 jumps and insane grenade jump
		{
			m_pDummyBlmapChillPolice->Tick();
		}
		else if(m_pPlayer->m_DummyMode == 33) //Chillintelligenz
		{
			//########################################
			// YOUTUBE
			// 800 likes top comment omg!
			// https://www.youtube.com/watch?v=xjgd8dki_V4&google_comment_id=z13vxrwiqriytlq3023exlkr5n3sydpvs
			// 100 likes top comment omg!
			// https://www.youtube.com/watch?v=oDTO8j12CBI&google_comment_id=z12ojtqx3tmsxtq1u23exlkr5n3sydpvs
			//#######################################
			// American Ultra
			// He never died
			// watchmen (die Wächter)
			// deadpool
			// Logan (The Wolverine)
			// Hardcore (henry)
			// Scott Pilgrim (gegen den Rest der Welt)
			CITick();
		}
		else if(m_pPlayer->m_DummyMode == 34) //survival
		{
			m_Input.m_Jump = 0;
			m_Input.m_Fire = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Hook = 0;
			m_Input.m_Direction = 0;
			//m_pPlayer->m_TeeInfos.m_ColorBody = (0 * 255 / 360);

			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, false, this);
			if(pChr && pChr->IsAlive())
			{
				m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				if(Server()->Tick() % 10 == 0) //random aim strong every 10 secs
				{
					int rand_x_inp = pChr->m_Pos.x - m_Pos.x + rand() % 200 - 100;
					int rand_y_inp = pChr->m_Pos.y - m_Pos.y + rand() % 200 - 100;

					m_Input.m_TargetX = rand_x_inp;
					m_Input.m_TargetY = rand_y_inp;
					m_LatestInput.m_TargetX = rand_x_inp;
					m_LatestInput.m_TargetY = rand_y_inp;
				}
				else //aim normal bad
				{
					int rand_x_inp = pChr->m_Pos.x - m_Pos.x + rand() % 20 - 10;
					int rand_y_inp = pChr->m_Pos.y - m_Pos.y + rand() % 20 - 10;

					m_Input.m_TargetX = rand_x_inp;
					m_Input.m_TargetY = rand_y_inp;
					m_LatestInput.m_TargetX = rand_x_inp;
					m_LatestInput.m_TargetY = rand_y_inp;
				}

				//m_Input.m_Fire++;
				//m_LatestInput.m_Fire++;

				if(Server()->Tick() % 120 == 0)
				{
					SetWeapon(1); //randomly swap to gun
				}

				if(m_Core.m_Pos.x > pChr->m_Pos.x - 80 && m_Core.m_Pos.x < pChr->m_Pos.x + 80)
				{
					if(Server()->Tick() % 20 == 0)
					{
						SetWeapon(0); //switch hammer in close range
					}
					m_Input.m_Fire++;
					m_LatestInput.m_Fire++;
				}
				else if(m_Core.m_Pos.x > pChr->m_Pos.x)
				{
					m_Input.m_Direction = -1;
				}
				else
				{
					m_Input.m_Direction = 1;
				}

				if(Server()->Tick() % 20 == 0)
				{
					if(rand() % 20 > 8)
					{
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
					}
				}

				if(m_Core.m_Pos.y > pChr->m_Pos.y - 5 * 32 && m_Core.m_Pos.y < pChr->m_Pos.y + 5 * 32) //same height
				{
					if(m_Core.m_Pos.x > pChr->m_Pos.x - 6 * 32 && m_Core.m_Pos.x < pChr->m_Pos.x + 6 * 32) //hook range
					{
						if(Server()->Tick() % 10 == 0) //angry
						{
							GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9);
						}

						m_Input.m_Hook = 1;
						if(rand() % 6 == 0)
						{
							m_Input.m_Hook = 0;
						}
					}
					else if(m_Core.m_Pos.x > pChr->m_Pos.x - 15 * 32 && m_Core.m_Pos.x < pChr->m_Pos.x + 15 * 32) //combat range
					{
					}
					else if(m_Core.m_Pos.x > pChr->m_Pos.x) //move towards enemy from to right side <---
					{
						int rand_x_inp = (rand() % 60 + 15) * -1;
						int rand_y_inp = rand() % 100 * -1;

						m_Input.m_TargetX = rand_x_inp;
						m_Input.m_TargetY = rand_y_inp;
						m_LatestInput.m_TargetX = rand_x_inp;
						m_LatestInput.m_TargetY = rand_y_inp;

						if(m_Core.m_Vel.y > -0.6f)
						{
							m_Input.m_Hook = 1;
						}
						if(IsGrounded())
						{
							m_Input.m_Jump = 1;
						}
					}
					else if(m_Core.m_Pos.x < pChr->m_Pos.x) //move towards enemy from the left side ---->
					{
						int rand_x_inp = rand() % 60 + 15;
						int rand_y_inp = rand() % 100 * -1;

						m_Input.m_TargetX = rand_x_inp;
						m_Input.m_TargetY = rand_y_inp;
						m_LatestInput.m_TargetX = rand_x_inp;
						m_LatestInput.m_TargetY = rand_y_inp;

						if(m_Core.m_Vel.y > -0.6f)
						{
							m_Input.m_Hook = 1;
						}
						if(IsGrounded())
						{
							m_Input.m_Jump = 1;
						}
					}
				}
				else if(m_Core.m_Pos.y > pChr->m_Pos.y) //too low
				{
					int rand_x_inp = rand() % 60 - 30;
					int rand_y_inp = rand() % 120 * -1;
					m_Input.m_Hook = 1;

					m_Input.m_TargetX = rand_x_inp;
					m_Input.m_TargetY = rand_y_inp;
					m_LatestInput.m_TargetX = rand_x_inp;
					m_LatestInput.m_TargetY = rand_y_inp;

					if(rand() % 3 == 1)
					{
						m_Input.m_Hook = 0;
					}

					if(GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x, m_Core.m_Pos.y - 32) == 1) //collsion in the way
					{
						m_Input.m_Direction = 1;
						//m_pPlayer->m_TeeInfos.m_ColorBody = (120 * 255 / 360);
					}
				}
				else if(m_Core.m_Pos.y < pChr->m_Pos.y) //too high
				{
					int rand_x_inp = rand() % 60 - 30;
					int rand_y_inp = rand() % 120;
					m_Input.m_Hook = 1;

					m_Input.m_TargetX = rand_x_inp;
					m_Input.m_TargetY = rand_y_inp;
					m_LatestInput.m_TargetX = rand_x_inp;
					m_LatestInput.m_TargetY = rand_y_inp;

					if(rand() % 3 == 1)
					{
						m_Input.m_Hook = 0;
					}
				}

				//tile bypassing

				if(m_Core.m_Pos.y > pChr->m_Pos.y + 50) //too low
				{
					if(GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x, m_Core.m_Pos.y - 32) == 1 || GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x, m_Core.m_Pos.y + 32) == 3) //collsion in the way
					{
						m_Input.m_Direction = m_DummyDir;
						//m_pPlayer->m_TeeInfos.m_ColorBody = (120 * 255 / 360);
					}
				}
				else if(m_Core.m_Pos.y < pChr->m_Pos.y - 50) //high low
				{
					if(GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x, m_Core.m_Pos.y + 32) == 1 || GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x, m_Core.m_Pos.y + 32) == 3) //collsion in the way
					{
						m_Input.m_Direction = m_DummyDir;
						//m_pPlayer->m_TeeInfos.m_ColorBody = (120 * 255 / 360);
					}
				}
			}

			//avoid killtiles
			if(GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x - 60, m_Core.m_Pos.y) == 2 || GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x - 60, m_Core.m_Pos.y + 30) == 2) //deathtiles on the left side
			{
				m_Input.m_Direction = 1;
			}
			else if(GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x + 60, m_Core.m_Pos.y) == 2 || GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x + 60, m_Core.m_Pos.y + 30) == 2) //deathtiles on the right side
			{
				m_Input.m_Direction = -1;
			}
			if(!IsGrounded()) //flybot for fly parts (working in 10% of the cases)
			{
				if(GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x, m_Core.m_Pos.y + 5 * 32) == 2) //falling on killtiles
				{
					m_Input.m_TargetX = 2;
					m_LatestInput.m_TargetX = 2;
					m_Input.m_TargetY = -200;
					m_LatestInput.m_TargetY = -200;
					if(m_Core.m_Vel.y > 0.0f)
					{
						m_Input.m_Hook = 1;
					}
					else
					{
						m_Input.m_Hook = 0;
					}
				}
			}

			//check for stucking in walls
			if(m_Input.m_Direction != 0 && m_Core.m_Vel.x == 0.0f)
			{
				if(Server()->Tick() % 60 == 0)
				{
					m_Input.m_Jump = 1;
				}
			}

			//slow tick
			if(Server()->Tick() % 200 == 0)
			{
				//escape stuck hook
				m_Input.m_Hook = 0;

				//anti stuck whatever
				if(m_DummyDir == 1 && (GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x + 20, m_Core.m_Pos.y) == 1 || GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x + 20, m_Core.m_Pos.y) == 3))
				{
					m_DummyDir = -1;
				}
				else // if (m_DummyDir == -1 && (GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x - 20, m_Core.m_Pos.y) == 1 || GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x - 20, m_Core.m_Pos.y) == 3))
				{
					m_DummyDir = 1;
				}

				//end game if all humans dead
				if(GameServer()->m_survivalgamestate > 1) //survival game running
				{
					if(m_pPlayer->m_IsSurvivalAlive)
					{
						int AliveHumans = 0;
						for(int i = 0; i < MAX_CLIENTS; i++)
						{
							if(GameServer()->m_apPlayers[i] && !GameServer()->m_apPlayers[i]->m_IsDummy) //check all humans
							{
								if(GameServer()->m_apPlayers[i]->m_IsSurvivaling && GameServer()->m_apPlayers[i]->m_IsSurvivalAlive) // surival alive
								{
									AliveHumans++;
								}
							}
						}
						if(!AliveHumans) //all humans dead --> suicide to get new round running
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
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
		else if(m_pPlayer->m_DummyMode == 35) // weak singleplayer guardian
		{
			m_Input.m_Jump = 0;
			m_Input.m_Fire = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Hook = 0;
			m_Input.m_Direction = 0;

			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, false, this);
			if(pChr && pChr->IsAlive())
			{
				m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y - 20;
				m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y - 20;

				// don't shoot walls
				if(!GameServer()->Collision()->IntersectLine(m_Pos, pChr->m_Pos, 0x0, 0))
				{
					if(Server()->Tick() % 77 == 0 && m_FreezeTime < 1)
					{
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
					}
				}
			}
		}
		else if(m_pPlayer->m_DummyMode == DUMMYMODE_QUEST) // 36
		{
			m_Input.m_Jump = 0;
			m_Input.m_Fire = 0;
			m_LatestInput.m_Fire = 0;
			// m_Input.m_Hook = 0;
			m_Input.m_Direction = 0;

			if(rand() % 20 > 17)
				m_Input.m_Direction = rand() % 3 - 1;
			if(rand() % 660 > 656)
				m_Input.m_Jump = 1;
			if(rand() % 256 > 244)
				m_Input.m_Hook = rand() % 2;
			if(rand() % 1000 > 988)
			{
				m_Input.m_Fire++;
				m_LatestInput.m_Fire++;
			}

			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, false, this);
			if(pChr && pChr->IsAlive())
			{
				static bool IsAimbot = false;
				if(IsAimbot)
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y - 20;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y - 20;

					// don't shoot walls
					if(!GameServer()->Collision()->IntersectLine(m_Pos, pChr->m_Pos, 0x0, 0))
					{
						if(Server()->Tick() % 77 == 0 && m_FreezeTime < 1)
						{
							m_Input.m_Fire++;
							m_LatestInput.m_Fire++;
						}
					}
					IsAimbot = rand() % 50 > 40;
				}
				else // no aimbot
				{
					IsAimbot = rand() % 3000 > 2789;
				}
			}

			if(Server()->Tick() % 500 == 0 && isFreezed)
				Die(m_pPlayer->GetCID(), WEAPON_SELF);
			if(Server()->Tick() % 5000 == 0) // spawn is probably most reachable spot so go back there here and then
				Die(m_pPlayer->GetCID(), WEAPON_SELF);
		}
		else if(m_pPlayer->m_DummyMode == 99) // shop bot
		{
			m_Input.m_Jump = 0;
			m_Input.m_Fire = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Hook = 0;
			m_Input.m_Direction = 0;

			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, false, this);
			if(pChr && pChr->IsAlive() && pChr->m_InShop)
			{
				m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
			}

			if(m_IsFreeShopBot)
			{
				if(Server()->Tick() % 500 == 0 && !m_InShop)
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}
			}
		}
		else if(m_pPlayer->m_DummyMode == 103) //ctf5 pvp
		{
			m_Input.m_Jump = 0;
			m_Input.m_Fire = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Hook = 0;
			m_Input.m_Direction = 0;

			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, false, this);
			if(pChr && pChr->IsAlive())
			{
				m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y - 20;
				m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y - 20;

				m_Input.m_Fire++;
				m_LatestInput.m_Fire++;

				if(m_Core.m_Pos.y - 4 * 32 > pChr->m_Pos.y)
				{
					if(Server()->Tick() % 20 == 0)
					{
						m_Input.m_Jump = 1;
					}
				}
				if(m_Core.m_Pos.x > pChr->m_Pos.x)
				{
					m_Input.m_Direction = -1;
					if(m_Core.m_Vel.x == 0.0f)
					{
						if(Server()->Tick() % 20 == 0)
						{
							m_Input.m_Jump = 1;
						}
					}
				}
				else
				{
					m_Input.m_Direction = 1;
					if(m_Core.m_Vel.x == 0.0f)
					{
						if(Server()->Tick() % 20 == 0)
						{
							m_Input.m_Jump = 1;
						}
					}
				}
			}
		}
		else if(m_pPlayer->m_DummyMode == 104) //blmapV5 lower areas blocker
		{
			m_Input.m_Jump = 0;
			m_Input.m_Fire = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Hook = 0;
			m_Input.m_Direction = 0;
			int offset_x = g_Config.m_SvDummyMapOffsetX * 32;
			//dbg_msg("debug","cfg=%d cfg*32=%d offset=%d mcore=%d offset+mcore=%d", g_Config.m_SvDummyMapOffsetX, g_Config.m_SvDummyMapOffsetX * 32, offset_x, m_Core.m_Pos.x, offset_x + m_Core.m_Pos.x);

			if(Server()->Tick() % 5000 == 0 && isFreezed)
			{
				Die(m_pPlayer->GetCID(), WEAPON_SELF);
			}

			if(m_Core.m_Pos.y < 38 * 32) //spawn
			{
				if(m_Core.m_Pos.x + offset_x < 13 * 32 + 10)
				{
					m_Input.m_Direction = 1;
				}
				else if(m_Core.m_Pos.x + offset_x > 14 * 32 + 16)
				{
					m_Input.m_Direction = -1;
					//dbg_msg("debug", "walking lefte because %d > %d", offset_x + m_Core.m_Pos.x, 14 * 32 + 16);
				}
				else
				{
					if(m_Core.m_Pos.x + offset_x > 1.2f)
					{
						m_Input.m_Direction = -1;
					}
					else if(m_Core.m_Pos.x + offset_x < -1.2f)
					{
						m_Input.m_Direction = 1;
					}
				}
			}
			else if(m_Core.m_Pos.y < 46 * 32) //block area 1
			{
				if(IsGrounded() || m_Core.m_Pos.x + offset_x > 14 * 32 + 10)
				{
					m_Input.m_Direction = -1;
				}
				if(m_Core.m_Pos.x + offset_x < 14 * 32 + 5 && m_Core.m_Pos.y < 40 * 32 + 30)
				{
					m_Input.m_Direction = 1;
				}
				if(Server()->Tick() % 60 == 0)
				{
					SetWeapon(3); //switch to grenade
				}
			}
			else if(m_Core.m_Pos.y < 61 * 32) //block area 2
			{
				if(m_Core.m_Pos.y < 52 * 32)
				{
					m_Input.m_Direction = -1;
				}
				else
				{
					m_Input.m_Direction = 1;
					m_Input.m_TargetX = -100;
					m_Input.m_TargetY = 19;
					m_LatestInput.m_TargetX = -100;
					m_LatestInput.m_TargetY = 19;
					if(m_FreezeTime == 0)
					{
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
					}
				}
			}
			else if(m_Core.m_Pos.y < 74 * 32) //block area 3
			{
				if(m_Core.m_Pos.x + offset_x > 16 * 32)
				{
					m_Input.m_Direction = -1;
					m_Input.m_TargetX = 100;
					m_Input.m_TargetY = 19;
					m_LatestInput.m_TargetX = 100;
					m_LatestInput.m_TargetY = 19;
					if(m_FreezeTime == 0)
					{
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
					}
				}
				else
				{
					m_Input.m_Direction = 1;
				}
			}
			else if(m_Core.m_Pos.y < 84 * 32) //block area 4
			{
				m_Input.m_Direction = 1;
				m_Input.m_TargetX = -200;
				m_Input.m_TargetY = 90;
				m_LatestInput.m_TargetX = -200;
				m_LatestInput.m_TargetY = 90;

				if(IsGrounded() && m_Core.m_Pos.x + offset_x > 23 * 32)
				{
					m_Input.m_Jump = 1;
				}
				if(m_Core.m_Vel.y < -0.05f && m_FreezeTime == 0)
				{
					m_Input.m_Fire++;
					m_LatestInput.m_Fire++;
				}

				if(m_Core.m_Pos.x + offset_x > 22 * 32 && m_Core.m_Vel.x < 7.1f)
				{
					m_Dummy104_rj_failed = true;
				}

				if(m_Dummy104_rj_failed)
				{
					m_Input.m_Direction = -1;
					if(m_Core.m_Pos.x + offset_x < 18 * 32)
					{
						m_Dummy104_rj_failed = false;
					}
				}
			}
			else //block area 5
			{
				if(m_Core.m_Pos.x + offset_x > 15 * 32 && m_Core.m_Pos.x + offset_x < 22 * 32) //never stay still over the middle freeze
				{
					m_Dummy104_panic_hook = true;
					if(m_Core.m_Pos.x + offset_x > 19 * 32)
					{
						m_Input.m_Direction = 1;
					}
					else
					{
						m_Input.m_Direction = -1;
						//dbg_msg("dummy","walk left to dodge the freeze");
					}
					if(m_Core.m_Pos.y > 92 * 32)
					{
						m_Input.m_Jump = 1;
						if(Server()->Tick() % 19 == 0)
						{
							m_Input.m_Jump = 0;
						}
					}
				}

				//block others:
				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeUnfreezedArea5(m_Pos, false, this);
				if(pChr && pChr->IsAlive())
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					if((pChr->m_Core.m_Pos.y > m_Core.m_Pos.y + 64 || (m_Dummy104_panic_hook && pChr->m_Core.m_Pos.y < m_Core.m_Pos.y)) && //hook enemys up in freeze or hook enemys down to get speed up and avoid falkling in freeze
						!(m_Core.m_Pos.y < 88 * 32 && m_Core.m_Vel.y < -0.1f)) //but dont do it when the bot is looking but to do a roof nade
					{
						m_Input.m_Hook = 1;
					}

					if(m_Dummy104_angry)
					{
						if(pChr->m_Core.m_Pos.x + offset_x - 60 > m_Core.m_Pos.x + offset_x)
						{
							m_Input.m_Direction = 1;
						}
						else if(pChr->m_Core.m_Pos.x + offset_x - 40 < m_Core.m_Pos.x + offset_x)
						{
							m_Input.m_Direction = -1;
							//dbg_msg("dummy", "walk left to get better enemy positioning enemypos=%f", pChr->m_Core.m_Pos.x + offset_x);
						}
						else //near enough to hammer
						{
							if(Server()->Tick() % 8 == 0)
							{
								SetWeapon(0);
							}
							if(pChr->m_Core.m_Pos.y > m_Core.m_Pos.y && pChr->m_Core.m_Pos.y < m_Core.m_Pos.y + 50)
							{
								if(pChr->m_Core.m_Vel.y < -0.1f && m_FreezeTime == 0)
								{
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
								}
							}
						}

						if(IsGrounded())
						{
							int rand_val = rand() % 7;

							if(rand_val == 1)
							{
								m_Input.m_Jump = 1;
							}
							else if(rand_val == 2 || rand_val == 4 || rand_val == 6)
							{
								if(distance(pChr->m_Core.m_Pos, m_Core.m_Pos) < 60)
								{
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
								}
							}
						}
					}
				}

				m_Dummy104_panic_hook = false;

				//angry checker
				if(m_Dummy104_angry)
				{
					if(m_FreezeTime == 0)
					{
						m_Dummy104_angry--;
					}
					if(Server()->Tick() % 10 == 0) //angry emotes machen
					{
						GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9);
					}
				}
				if(m_Core.m_Vel.y < -1.2f && m_Core.m_Pos.y < 88 * 32)
				{
					if(Server()->Tick() % 10 == 0 && m_Dummy104_angry == 0)
					{
						m_Dummy104_angry = Server()->TickSpeed() * 20;
					}
				}

				if(m_Core.m_Pos.x + offset_x < 5 * 32) //lower left freeze
				{
					m_Input.m_Direction = 1;
				}

				if(m_Core.m_Pos.y > 90 * 32 + 16 && m_Core.m_Vel.y < -6.4f) //slow down speed with dj when flying to fast up
				{
					m_Input.m_Jump = 1;
				}

				if(m_Core.m_Pos.y < 89 * 32 - 20) //high
				{
					if(Server()->Tick() % 3 == 0)
					{
						SetWeapon(3);
					}
					if(m_Core.m_Pos.y < 88 * 32 && m_Core.m_Vel.y < -0.1f) // near roof
					{
						m_Input.m_TargetX = 1;
						m_Input.m_TargetY = -200;
						m_LatestInput.m_TargetX = 1;
						m_LatestInput.m_TargetY = -200;
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
					}
				}

				//dont enter the freeze exit on the right side
				if(m_Core.m_Pos.x + offset_x > 34 * 32 && m_Core.m_Pos.y < 91 * 32 && m_Core.m_Vel.x > 0.0f)
				{
					m_Input.m_Direction = -1;
				}
			}
		}
		else if(m_pPlayer->m_DummyMode == 105) //blmapV5 upper blocker
		{
			m_Input.m_Jump = 0;
			m_Input.m_Fire = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Hook = 0;
			m_Input.m_Direction = 0;
			m_Input.m_TargetX = 45;
			m_Input.m_TargetY = 45;
			m_LatestInput.m_TargetX = 45;
			m_LatestInput.m_TargetY = 45;
			int bot_x = m_Core.m_Pos.x + g_Config.m_SvDummyMapOffsetX * 32;

			if(m_Core.m_Pos.y > 38 * 32) //too low
			{
				Die(m_pPlayer->GetCID(), WEAPON_SELF);
			}

			if(bot_x > 16 * 32 && m_Core.m_Pos.y < 27 * 32) //lovley wayblock spot
			{
			}
			else //not at lovley wayblock spot
			{
				if(Server()->Tick() % 420 == 0)
				{
					if(m_FreezeTime && IsGrounded()) //stuck ?
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}
				}

				if(m_Dummy105_move_left)
				{
					m_Input.m_Direction = -1;

					//failed?
					if(bot_x < 22 * 32)
					{
						m_Dummy105_move_left = false;
					}
				}
				else
				{
					m_Input.m_Direction = 1;

					if(bot_x > 10 * 32 && bot_x < 17 * 32) //jump in tunnel
					{
						m_Input.m_Jump = 1;
						if(Server()->Tick() % 20 == 0)
						{
							SetWeapon(3);
						}
					}

					if(bot_x > 28 * 32 && IsGrounded())
					{
						m_Input.m_Jump = 1;
					}

					if(m_Core.m_Pos.y < 31 * 32 - 10) //dont touch the roof
					{
						m_Input.m_Hook = 1;
					}
					if(bot_x > 35 * 32 && m_Core.m_Vel.x == 0.000000f) //hit the rocketjump wall
					{
						m_Input.m_Jump = 1;
						if(m_Core.m_Vel.y < -1.1f && m_FreezeTime == 0)
						{
							m_Input.m_Fire++;
							m_LatestInput.m_Fire++;
							m_Dummy105_move_left = true;
						}
					}
				}
			}
		}
		else // dummymode == end dummymode == last
		{
			m_pPlayer->m_DummyMode = 0;
		} //DUMMY END
	}
}
