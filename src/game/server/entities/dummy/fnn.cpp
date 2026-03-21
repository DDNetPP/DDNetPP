/*
######################################################
#     DummyMode 25      The    FNN    Mode           #
#           [FNN] == [FAKENEURALNETWORK]             #
######################################################
ModeStructure:

* Spawn -> goto hardcodet spawn pos.x 353 * 32

* Check for human interactions and save them in the var m_Dummy_nn_touched_by_humans

*/

#include "fnn.h"

#include "../character.h"

#include <base/math_ddpp.h>
#include <base/system.h>

#include <engine/shared/config.h>

#include <game/mapitems.h>
#include <game/server/ddpp/shop.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>

#include <fstream>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos) * 32)

CDummyFNN::CDummyFNN(class CPlayer *pPlayer) :
	CDummyBase(pPlayer, DUMMYMODE_FNN)
{
	OnDeath();
}

void CDummyFNN::OnDeath()
{
	m_DummyNnStop = false;
	m_DummyNnReadyTime = 0;
	m_FnnStartServertick = 0;
	m_FnnStopServertick = 0;
	m_StartPos = vec2(0, 0);
}

void CDummyFNN::TakeDamage(vec2 Force, int Dmg, int From, int Weapon)
{
	// isTouched check
	if(From >= 0 && m_DummyNnReady && From != m_pPlayer->GetCid())
	{
		if((Weapon == WEAPON_GRENADE || Weapon == WEAPON_HAMMER || Weapon == WEAPON_SHOTGUN || Weapon == WEAPON_LASER) && GameServer()->m_apPlayers[From])
		{
			m_DummyNnTouchedByHumans = true;
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "[FNN] please stop shooting me %s", Server()->ClientName(From));
			GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, aBuf);
		}
	}
}

void CDummyFNN::OnTick()
{
	char aBuf[256];

	if(m_pPlayer->m_Dmm25 == -2) //stopped
	{
		Hook(0);
		Jump(0);
		StopMoving();
		Fire(0);
		m_pPlayer->m_TeeInfos.m_ColorBody = (180 * 255 / 260);
	}
	else if(!m_DummyNnReady) //first get the right start pos
	{
		Hook(0);
		Jump(0);
		StopMoving();
		Fire(0);

		if(GetPos().x > g_Config.m_SvFNNstartX * 32)
		{
			if(IsGrounded()) //only walk on ground because air is unpredictable
			{
				if(GetPos().x < g_Config.m_SvFNNstartX * 32 + 10) //in 1tile nähe langsam laufen
				{
					if(GetVel().x > -0.009f)
					{
						Left();
					}
				}
				else
				{
					Left();
				}
			}
		}
		else if(GetPos().x < g_Config.m_SvFNNstartX * 32)
		{
			if(IsGrounded()) //only walk on ground because air is unpredictable
			{
				if(GetPos().x > g_Config.m_SvFNNstartX * 32 - 10) //in 1tile nähe langsam laufen
				{
					if(GetVel().x < 0.009f)
					{
						Right();
					}
				}
				else
				{
					Right();
				}
			}
		}
		else //correct position
		{
			if(IsGrounded()) //only start on ground because air is unpredictable
			{
				m_DummyNnReady = true;
				m_StartPos = GetPos();
				dbg_msg("FNN", "Found start position (%.2f/%.2f) -> starting process", GetPos().x / 32, GetPos().y / 32);
			}
		}

		//Catch errors
		if(m_DummyNnReadyTime > 300)
		{
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", "Starting process failed. Get in start position took too long -> restarting...");
			Die();
		}
		m_DummyNnReadyTime++;
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

		//int m_aMoveId = -1;
		//int Hooked = false;
		for(auto &Player : GameServer()->m_apPlayers)
		{
			if(!Player)
				continue;

			CCharacter *pChar = Player->GetCharacter();

			if(!pChar || !pChar->IsAlive() || pChar == m_pCharacter)
				continue;

			if(pChar->Core()->HookedPlayer() == m_pPlayer->GetCid())
			{
				//Hooked = true;
				//m_aMoveId = i;
				m_DummyNnTouchedByHumans = true;
				GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "[FNN] DONT TOUCH ME HOOK WTF");
			}

			if(m_pCharacter->Core()->HookedPlayer() != -1)
			{
				str_format(aBuf, sizeof(aBuf), "[FNN] dont get in my hook %s", Server()->ClientName(m_pCharacter->Core()->HookedPlayer()));
				GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, aBuf);
			}
		}
		//if (m_Core.m_HookState == HOOK_GRABBED) //this includes normal collision hooks
		//{
		//	m_Dummy_nn_touched_by_humans = true;
		//	GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "[FNN] dont get in my hook -.-");
		//}
		//selfmade noob code check if pChr is too near and could touched the bot
		CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), true);
		if(pChr && pChr->IsAlive() && pChr != m_pCharacter)
		{
			if(pChr->GetPos().x < GetPos().x + 60 && pChr->GetPos().x > GetPos().x - 60 && pChr->GetPos().y < GetPos().y + 60 && pChr->GetPos().y > GetPos().y - 60)
			{
				m_DummyNnTouchedByHumans = true;
				GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "[FNN] dont touch my body with yours pls");
			}
		}

		//always set to black
		m_pPlayer->m_TeeInfos.m_ColorBody = (0 * 255 / 360);

		if(m_pPlayer->m_Dmm25 == -1) //error
		{
			Hook(0);
			Jump(0);
			StopMoving();
			Fire(0);
			m_pPlayer->m_TeeInfos.m_ColorBody = (180 * 255 / 260);
		}
		else if(m_pPlayer->m_Dmm25 == 0) //submode[0] write
		{
			// m_pPlayer->m_TeeInfos.m_Name = "writing...";
			// m_pPlayer->m_TeeInfos.m_ColorBody = (180 * 255 / 360);

			//m_pPlayer->m_Dummy_nn_time++; //maybe use it some day to analys each tick stuff or total training time idk

			// random inputs
			int RandFire = rand() % 2; // 1 0
			int RandJump = 0;
			if(rand() % 32 - (IsGrounded() * 6) == 0) // more likely to jump if grounded to avoid spamming dj
			{
				RandJump = 1;
			}
			int RandHook = rand() % 2;
			int RandWeapon = rand() % 4;
			int RandTargetX = (rand() % 401) - 200;
			int RandTargetY = (rand() % 401) - 200;
			if(Server()->Tick() % 77 == 0)
			{
				m_RandDirection = rand() % 3 - 1; //-1 0 1
				if(!(rand() % 3 == 0)) // increase chance of walking towards finish
				{
					// dbg_msg("fnn", "finish: %.2f pos: %.2f", GameServer()->m_FinishTilePos.x, GetPos().x / 32);
					if(GameServer()->m_FinishTilePos.x > GetPos().x / 32)
					{
						m_RandDirection = 1;
					}
					else
					{
						m_RandDirection = -1;
					}
				}
			}

			SetDirection(m_RandDirection);
			Jump(RandJump);
			Hook(RandHook);
			AimX(RandTargetX);
			AimY(RandTargetY);

			// read world inputs
			float Offset = 16.0f;
			int MapIndexL = GameServer()->Collision()->GetPureMapIndex(vec2(GetPos().x - (m_pCharacter->GetProximityRadius() / 2) - Offset, GetPos().y));
			int MapIndexR = GameServer()->Collision()->GetPureMapIndex(vec2(GetPos().x + (m_pCharacter->GetProximityRadius() / 2) + Offset, GetPos().y));
			int MapIndexB = GameServer()->Collision()->GetPureMapIndex(vec2(GetPos().x, GetPos().y + (m_pCharacter->GetProximityRadius() / 2) + Offset));
			int MapIndexT = GameServer()->Collision()->GetPureMapIndex(vec2(GetPos().x, GetPos().y - (m_pCharacter->GetProximityRadius() / 2) - Offset));
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
				Right();
				m_pPlayer->m_TeeInfos.m_ColorBody = (200 * 255 / 1);
			}
			else if(TileIndexR == TILE_FREEZE)
			{
				Left();
				m_pPlayer->m_TeeInfos.m_ColorBody = (200 * 255 / 1);
			}
			else
			{
				SetDirection(m_RandDirection);
			}
			if(TileIndexB == TILE_FREEZE)
			{
				Jump();
				m_pPlayer->m_TeeInfos.m_ColorBody = (200 * 255 / 1);
			}

			if(TileIndexB == TILE_NOHOOK)
			{
				m_pPlayer->m_TeeInfos.m_ColorBody = (100 * 255 / 1); // light red
			}
			// StopMoving();
			// Hook(0);
			// Jump(0);
			// rand_Fire = 0;

			if(m_FnnCurrentMoveIndex == 0)
			{
				m_FnnStartServertick = Server()->Tick();
				dbg_msg("FNN", "starting record on x=%f y=%f servertick=%d", GetPos().x, GetPos().y, m_FnnStartServertick);
			}

			//save values in array
			m_aRecMove[m_FnnCurrentMoveIndex] = GetDirection();
			// dbg_msg("fnn", "dir: %d", GetDirection());
			m_FnnCurrentMoveIndex++;
			m_aRecMove[m_FnnCurrentMoveIndex] = GetJump();
			// dbg_msg("fnn", "jump: %d", GetJump());
			m_FnnCurrentMoveIndex++;
			m_aRecMove[m_FnnCurrentMoveIndex] = GetHook();
			// dbg_msg("fnn", "hook: %d", GetHook());
			m_FnnCurrentMoveIndex++;
			m_aRecMove[m_FnnCurrentMoveIndex] = GetTargetX();
			// dbg_msg("fnn", "targetX: %d", GetTargetX());
			m_FnnCurrentMoveIndex++;
			m_aRecMove[m_FnnCurrentMoveIndex] = GetTargetY();
			// dbg_msg("fnn", "targetY: %d", GetTargetY());
			m_FnnCurrentMoveIndex++;

			if(RandWeapon == 0)
			{
				SetWeapon(0); //hammer
			}
			else if(RandWeapon == 1)
			{
				SetWeapon(1); //gun
			}
			else if(RandWeapon == 2)
			{
				if(m_pCharacter->GetWeaponGot(WEAPON_SHOTGUN))
				{
					SetWeapon(2); //shotgun
				}
			}
			else if(RandWeapon == 3)
			{
				if(m_pCharacter->GetWeaponGot(WEAPON_GRENADE))
				{
					SetWeapon(3); //grenade
				}
			}
			else if(RandWeapon == 4)
			{
				if(m_pCharacter->GetWeaponGot(WEAPON_LASER))
				{
					SetWeapon(4); //laser
				}
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "Error unknown weapon: %d", RandWeapon);
				GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", aBuf);
			}
			if(RandFire == 1 && m_pCharacter->m_FreezeTime == 0)
			{
				Fire();
			}

			if(m_FnnCurrentMoveIndex > FNN_MOVE_LEN - 12) //minus inputs because every tick the index gets incremented by 5
			{
				float NewestDistance = distance(m_StartPos, GetPos());
				vec2 CurrentPos(0, 0);
				CurrentPos.x = GetPos().x / 32;
				CurrentPos.y = GetPos().y / 32;
				float NewestDistanceFinish = distance(CurrentPos, GameServer()->m_FinishTilePos);
				float NewestFitness = NewestDistanceFinish / m_FnnCurrentMoveIndex;
				str_format(aBuf, sizeof(aBuf), "[FNN] ran out of memory ticks=%d distance=%.2f fitness=%.2f distance_finish=%.2f", m_FnnCurrentMoveIndex, NewestDistance, NewestFitness, NewestDistanceFinish);
				GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, aBuf);
				//Die();
				m_DummyNnStop = true;
			}

			if(g_Config.m_SvFNNtimeout && !m_DummyNnStop)
			{
				if(m_FnnCurrentMoveIndex > g_Config.m_SvFNNtimeout)
				{
					str_format(aBuf, sizeof(aBuf), "[FNN] timeouted after ticks=%d", m_FnnCurrentMoveIndex);
					GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, aBuf);
					m_DummyNnStop = true;
				}
			}

			if((GetVel().y == 0.000000f && GetVel().x < 0.01f && GetVel().x > -0.01f && IsFrozen()) || m_DummyNnStop)
			{
				if(Server()->Tick() % 10 == 0)
				{
					GameServer()->SendEmoticon(m_pPlayer->GetCid(), 3, -1);
				}
				if(Server()->Tick() % 40 == 0)
				{
					GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", "======================================");
					if(m_DummyNnTouchedByHumans)
					{
						str_format(aBuf, sizeof(aBuf), "Failed at (%.2f/%.2f) --> RESTARTING", GetPos().x / 32, GetPos().y / 32);
						GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", aBuf);
						GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", "Got touched by Humans --> DELETE RUN");
						Die();
					}
					else //no interaction with humans --> save normal
					{
						str_format(aBuf, sizeof(aBuf), "Failed at (%.2f/%.2f) --> RESTARTING", GetPos().x / 32, GetPos().y / 32);
						m_FnnStopServertick = Server()->Tick();
						dbg_msg("FNN", "stop servertick=%d totaltickdiff=%d", m_FnnStopServertick, m_FnnStopServertick - m_FnnStartServertick);
						GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", aBuf);
						//Die(); //moved to the end because we use character variables but im not sure.. maybe it had a sense its here

						//prepare data
						if(m_FnnCurrentMoveIndex > FNN_MOVE_LEN) //dont overload arraylength
						{
							m_FnnCurrentMoveIndex = FNN_MOVE_LEN;
						}

						float NewestDistance = distance(m_StartPos, GetPos());
						vec2 CurrentPos(0, 0);
						CurrentPos.x = GetPos().x / 32;
						CurrentPos.y = GetPos().y / 32;
						float NewestDistanceFinish = distance(CurrentPos, GameServer()->m_FinishTilePos);
						float NewestFitness = NewestDistanceFinish / m_FnnCurrentMoveIndex;
						dbg_msg("FNN", "distance=%.2f", NewestDistance);
						dbg_msg("FNN", "moveticks=%d", m_FnnCurrentMoveIndex);
						dbg_msg("FNN", "fitness=%.2f", NewestFitness);
						dbg_msg("FNN", "distance_finish=%.2f", NewestDistanceFinish);

						/***************************************
						 *                                      *
						 *                                      *
						 *         D I S T A N C E              *
						 *                                      *
						 *                                      *
						 ****************************************/

						if(NewestDistance > GameServer()->m_FnnBestDistance)
						{
							//saving the distance
							// dbg_msg("FNN","new distance highscore Old=%.2f -> New=%.2f", GameServer()->m_FNN_best_distance, newest_distance);
							str_format(aBuf, sizeof(aBuf), "[FNN] new distance highscore Old=%.2f -> New=%.2f", GameServer()->m_FnnBestDistance, NewestDistance);
							GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, aBuf);
							GameServer()->m_FnnBestDistance = NewestDistance;
							std::ofstream Statsfile;
							char aFilePath[512];
							str_copy(aFilePath, "FNN/move_stats.fnn", sizeof(aFilePath));
							Statsfile.open(aFilePath);
							if(Statsfile.is_open())
							{
								// statsfile << "-- total stats --";
								// statsfile << std::endl;
								Statsfile << NewestDistance; //distance
								Statsfile << '\n';
								Statsfile << GameServer()->m_FnnBestFitness; //fitness
								Statsfile << '\n';
								Statsfile << GameServer()->m_FnnBestDistanceFinish; //distance_finish
								Statsfile << '\n';
							}
							else
							{
								dbg_msg("FNN", "failed to update stats. failed to open file '%s'", aFilePath);
							}
							Statsfile.close();

							//saving the run
							std::ofstream Savefile;
							str_copy(aFilePath, "FNN/move_distance.fnn", sizeof(aFilePath));
							Savefile.open(aFilePath /*, std::ios::app*/); //dont append rewrite
							if(Savefile.is_open())
							{
								//first five lines are stats
								Savefile << "-- stats distance --";
								Savefile << '\n';
								Savefile << m_FnnCurrentMoveIndex; //moveticks
								Savefile << '\n';
								Savefile << NewestDistance; //distance
								Savefile << '\n';
								Savefile << NewestFitness; //fitness
								Savefile << '\n';
								Savefile << NewestDistanceFinish; //distance_finish
								Savefile << '\n';

								for(int i = 0; i < m_FnnCurrentMoveIndex; i++)
								{
									Savefile << m_aRecMove[i];
									Savefile << '\n';
								}
							}
							else
							{
								dbg_msg("FNN", "failed to save record. failed to open file '%s'", aFilePath);
							}
							Savefile.close();
						}

						/***************************************
						 *                                      *
						 *                                      *
						 *         F I T T N E S S              *
						 *                                      *
						 *                                      *
						 ****************************************/

						if(NewestFitness > GameServer()->m_FnnBestFitness)
						{
							//saving the fitness
							// dbg_msg("FNN", "new fitness highscore Old=%.2f -> New=%.2f", GameServer()->m_FNN_best_fitness, newest_fitness);
							str_format(aBuf, sizeof(aBuf), "[FNN] new fitness highscore Old=%.2f -> New=%.2f", GameServer()->m_FnnBestFitness, NewestFitness);
							GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, aBuf);
							GameServer()->m_FnnBestFitness = NewestFitness;
							std::ofstream StatsFile;
							char aFilePath[512];
							str_copy(aFilePath, "FNN/move_stats.fnn", sizeof(aFilePath));
							StatsFile.open(aFilePath);
							if(StatsFile.is_open())
							{
								// statsfile << "-- total stats --";
								// statsfile << std::endl;
								StatsFile << GameServer()->m_FnnBestDistance; //distance
								StatsFile << '\n';
								StatsFile << NewestFitness; //fitness
								StatsFile << '\n';
								StatsFile << GameServer()->m_FnnBestDistanceFinish; //distance_finish
								StatsFile << '\n';
							}
							else
							{
								dbg_msg("FNN", "failed to update stats. failed to open file '%s'", aFilePath);
							}
							StatsFile.close();

							//saving the run
							std::ofstream SaveFile;
							str_copy(aFilePath, "FNN/move_fitness.fnn", sizeof(aFilePath));
							SaveFile.open(aFilePath);
							if(SaveFile.is_open())
							{
								//first five lines are stats
								SaveFile << "-- stats fitness --";
								SaveFile << '\n';
								SaveFile << m_FnnCurrentMoveIndex; //moveticks
								SaveFile << '\n';
								SaveFile << NewestDistance; //distance
								SaveFile << '\n';
								SaveFile << NewestFitness; //fitness
								SaveFile << '\n';
								SaveFile << NewestDistanceFinish; //distance_finish
								SaveFile << '\n';

								for(int i = 0; i < m_FnnCurrentMoveIndex; i++)
								{
									SaveFile << m_aRecMove[i];
									SaveFile << '\n';
								}
							}
							else
							{
								dbg_msg("FNN", "failed to save record. failed to open file '%s'", aFilePath);
							}
							SaveFile.close();
						}

						/***************************************
						 *                                      *
						 *                                      *
						 *         D I S T A N C E              *
						 *         F I N I S H                  *
						 *                                      *
						 ****************************************/

						if(NewestDistanceFinish < GameServer()->m_FnnBestDistanceFinish)
						{
							//saving the distance_finish
							// dbg_msg("FNN", "new distance_finish highscore Old=%.2f -> New=%.2f", GameServer()->m_FNN_best_distance_finish, newest_distance_finish);
							str_format(aBuf, sizeof(aBuf), "[FNN] new distance_finish highscore Old=%.2f -> New=%.2f", GameServer()->m_FnnBestDistanceFinish, NewestDistanceFinish);
							GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, aBuf);
							GameServer()->m_FnnBestDistanceFinish = NewestDistanceFinish;
							std::ofstream StatsFile;
							char aFilePath[512];
							str_copy(aFilePath, "FNN/move_stats.fnn", sizeof(aFilePath));
							StatsFile.open(aFilePath);
							if(StatsFile.is_open())
							{
								// statsfile << "-- total stats --";
								// statsfile << std::endl;
								StatsFile << GameServer()->m_FnnBestDistance; //distance
								StatsFile << '\n';
								StatsFile << GameServer()->m_FnnBestFitness; //fitness
								StatsFile << '\n';
								StatsFile << NewestDistanceFinish; //distance_finish
								StatsFile << '\n';
							}
							else
							{
								dbg_msg("FNN", "failed to update stats. failed to open file '%s'", aFilePath);
							}
							StatsFile.close();

							//saving the run
							std::ofstream SaveFile;
							str_copy(aFilePath, "FNN/move_distance_finish.fnn", sizeof(aFilePath));
							SaveFile.open(aFilePath);
							if(SaveFile.is_open())
							{
								//first five lines are stats
								SaveFile << "-- stats distance finish --";
								SaveFile << '\n';
								SaveFile << m_FnnCurrentMoveIndex; //moveticks
								SaveFile << '\n';
								SaveFile << NewestDistance; //distance
								SaveFile << '\n';
								SaveFile << NewestFitness; //fitness
								SaveFile << '\n';
								SaveFile << NewestDistanceFinish; //distance_finish
								SaveFile << '\n';

								for(int i = 0; i < m_FnnCurrentMoveIndex; i++)
								{
									SaveFile << m_aRecMove[i];
									SaveFile << '\n';
								}
							}
							else
							{
								dbg_msg("FNN", "failed to save record. failed to open file '%s'", aFilePath);
							}
							SaveFile.close();
						}

						m_FnnCurrentMoveIndex = 0;
						Die();
					}
					GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", "======================================");
				}
			}
		}
		else if(m_pPlayer->m_Dmm25 == 1) //submode[1] read/load distance
		{
			GameServer()->FNN_LoadRun("FNN/move_distance.fnn", m_pPlayer->GetCid());
		}
		else if(m_pPlayer->m_Dmm25 == 2) //submode[2] read/load fitness
		{
			GameServer()->FNN_LoadRun("FNN/move_fitness.fnn", m_pPlayer->GetCid());
		}
		else if(m_pPlayer->m_Dmm25 == 3) //submode[3] read/load lowest distance_finish
		{
			GameServer()->FNN_LoadRun("FNN/move_distance_finish.fnn", m_pPlayer->GetCid());
		}
		else if(m_pPlayer->m_Dmm25 == 4) //submode[4] play loaded run
		{
			if(m_FnnCurrentMoveIndex == 0)
			{
				m_FnnStartServertick = Server()->Tick();
				dbg_msg("FNN", "starting play on x=%f y=%f servertick=%d", GetPos().x, GetPos().y, m_FnnStartServertick);
			}

			SetDirection(m_aRecMove[m_FnnCurrentMoveIndex]);
			// dbg_msg("fnn", "dir: %d", m_aRecMove[m_FNN_CurrentMoveIndex]);
			m_FnnCurrentMoveIndex++;
			Jump(m_aRecMove[m_FnnCurrentMoveIndex]);
			// dbg_msg("fnn", "jump: %d", m_aRecMove[m_FNN_CurrentMoveIndex]);
			m_FnnCurrentMoveIndex++;
			Hook(m_aRecMove[m_FnnCurrentMoveIndex]);
			// dbg_msg("fnn", "hook: %d", m_aRecMove[m_FNN_CurrentMoveIndex]);
			m_FnnCurrentMoveIndex++;
			AimX(m_aRecMove[m_FnnCurrentMoveIndex]);
			// dbg_msg("fnn", "targetX: %d", m_aRecMove[m_FNN_CurrentMoveIndex]);
			m_FnnCurrentMoveIndex++;
			AimY(m_aRecMove[m_FnnCurrentMoveIndex]);
			// dbg_msg("fnn", "targetY: %d", m_aRecMove[m_FNN_CurrentMoveIndex]);
			m_FnnCurrentMoveIndex++;

			// ignore latest input for now
			// dbg_msg("fnn", "r: %d", m_aRecMove[m_FNN_CurrentMoveIndex]);
			// m_FNN_CurrentMoveIndex++;
			// dbg_msg("fnn", "r: %d", m_aRecMove[m_FNN_CurrentMoveIndex]);
			// m_FNN_CurrentMoveIndex++;

			if(m_FnnCurrentMoveIndex >= m_FnnTicksLoadedRun)
			{
				m_pPlayer->m_Dmm25 = -1; //stop bot
				float NewestDistance = distance(m_StartPos, GetPos());
				vec2 CurrentPos(0, 0);
				CurrentPos.x = GetPos().x / 32;
				CurrentPos.y = GetPos().y / 32;
				float NewestDistanceFinish = distance(CurrentPos, GameServer()->m_FinishTilePos);
				float NewestFitness = NewestDistanceFinish / m_FnnCurrentMoveIndex;
				m_FnnStopServertick = Server()->Tick();
				dbg_msg("FNN", "stop servertick=%d totaltickdiff=%d", m_FnnStopServertick, m_FnnStopServertick - m_FnnStartServertick);
				dbg_msg("FNN", "distance=%.2f", NewestDistance);
				dbg_msg("FNN", "moveticks=%d", m_FnnCurrentMoveIndex);
				dbg_msg("FNN", "fitness=%.2f", NewestFitness);
				dbg_msg("FNN", "distance_finish=%.2f", NewestDistanceFinish);
				str_format(aBuf, sizeof(aBuf), "[FNN] finished replay ticks=%d distance=%.2f fitness=%.2f distance_finish=%.2f", m_FnnCurrentMoveIndex, NewestDistance, NewestFitness, NewestDistanceFinish);
				GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, aBuf);
			}
		}
	}
}
