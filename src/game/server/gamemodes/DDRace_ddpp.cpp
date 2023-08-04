/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
/* Based on Race mod stuff and tweaked by GreYFoX@GTi and others to fit our DDRace needs. */
#include "DDRace.h"
#include <engine/server.h>
#include <engine/shared/config.h>
#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/entities/flag.h>
#include <game/server/gamecontext.h>

void CGameControllerDDRace::FlagTick()
{
	for(int fi = 0; fi < 2; fi++)
	{
		CFlag *F = m_apFlags[fi];

		if(!F)
			continue;

		// flag hits death-tile or left the game layer, reset it
		if(GameServer()->Collision()->GetCollisionAt(F->m_Pos.x, F->m_Pos.y) == TILE_DEATH || F->GameLayerClipped(F->m_Pos))
		{
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", "flag_return");
			if(g_Config.m_SvFlagSounds)
			{
				GameServer()->CreateSoundGlobal(SOUND_CTF_RETURN);
			}
			F->Reset();
			continue;
		}

		//
		if(F->m_pCarryingCharacter)
		{
			// update flag position
			F->m_Pos = F->m_pCarryingCharacter->m_Pos;

			if(F->m_pCarryingCharacter->m_FirstFreezeTick != 0)
			{
				/*char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "Your freeze Time Amount is: %i AND %i",F->m_pCarryingCharacter->m_FirstFreezeTick + Server()->TickSpeed()*8, Server()->Tick());
				GameServer()->SendChatTarget(F->m_pCarryingCharacter->GetPlayer()->GetCID(), aBuf);*/

				if(Server()->Tick() > F->m_pCarryingCharacter->m_FirstFreezeTick + Server()->TickSpeed() * 8)
				{
					/*F->m_pCarryingCharacter->GetPlayer()->m_Rainbow = false;
					F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorBody = F->m_pCarryingCharacter->GetPlayer()->m_ColorBodyOld;
					F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorFeet = F->m_pCarryingCharacter->GetPlayer()->m_ColorFeetOld;*/
					if(m_apFlags[0] && m_apFlags[0]->m_pCarryingCharacter && m_apFlags[0]->m_pCarryingCharacter == F->m_pCarryingCharacter->GetPlayer()->GetCharacter())
					{
						DropFlag(0, F->m_pCarryingCharacter->GetPlayer()->GetCharacter()->GetAimDir()); //red
						//SendChatTarget(F->m_pCarryingCharacter->GetPlayer()->GetCID(), "you dropped red flag");
					}
					else if(m_apFlags[1] && m_apFlags[1]->m_pCarryingCharacter && m_apFlags[1]->m_pCarryingCharacter == F->m_pCarryingCharacter->GetPlayer()->GetCharacter())
					{
						DropFlag(1, F->m_pCarryingCharacter->GetPlayer()->GetCharacter()->GetAimDir()); //blue
						//SendChatTarget(F->m_pCarryingCharacter->GetPlayer()->GetCID(), "you dropped blue flag");
					}
				}
			}
		}
		else
		{
			if(GameServer()->Collision()->GetTileIndex(GameServer()->Collision()->GetMapIndex(F->m_Pos)) == 95)
			{
				if(g_Config.m_SvFlagSounds)
				{
					GameServer()->CreateSoundGlobal(SOUND_CTF_RETURN);
				}
				F->Reset();
			}

			CCharacter *apCloseCCharacters[MAX_CLIENTS];
			int Num = GameServer()->m_World.FindEntities(F->m_Pos, CFlag::ms_PhysSize, (CEntity **)apCloseCCharacters, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
			for(int i = 0; i < Num; i++)
			{
				if(!apCloseCCharacters[i]->IsAlive() || apCloseCCharacters[i]->GetPlayer()->GetTeam() == TEAM_SPECTATORS || GameServer()->Collision()->IntersectLine(F->m_Pos, apCloseCCharacters[i]->m_Pos, NULL, NULL))
					continue;
				if(m_apFlags[0] && m_apFlags[1])
				{
					if(m_apFlags[0]->m_pCarryingCharacter == apCloseCCharacters[i] || m_apFlags[1]->m_pCarryingCharacter == apCloseCCharacters[i] || (F->m_pLastCarryingCharacter == apCloseCCharacters[i] && (F->m_DropFreezeTick + Server()->TickSpeed() * 2) > Server()->Tick()))
						continue;
				}
				else
				{
					if(m_apFlags[0])
					{
						if(m_apFlags[0]->m_pCarryingCharacter == apCloseCCharacters[i] || (m_apFlags[0]->m_pLastCarryingCharacter == apCloseCCharacters[i] && (m_apFlags[0]->m_DropFreezeTick + Server()->TickSpeed() * 2) > Server()->Tick()))
							continue;
					}
					if(m_apFlags[1])
					{
						if(m_apFlags[1]->m_pCarryingCharacter == apCloseCCharacters[i] || (m_apFlags[1]->m_pLastCarryingCharacter == apCloseCCharacters[i] && (m_apFlags[1]->m_DropFreezeTick + Server()->TickSpeed() * 2) > Server()->Tick()))
							continue;
					}
				}

				// take the flag
				if(F->m_AtStand)
				{
					F->m_GrabTick = Server()->Tick();
				}

				F->m_AtStand = 0;
				F->m_pCarryingCharacter = apCloseCCharacters[i];
				F->m_pCarryingCharacter->GetPlayer()->m_ChangeTeamOnFlag = true;
				/*if (!apCloseCCharacters[i]->GetPlayer()->m_Rainbow){
					apCloseCCharacters[i]->GetPlayer()->m_ColorBodyOld = apCloseCCharacters[i]->GetPlayer()->m_TeeInfos.m_ColorBody;
					apCloseCCharacters[i]->GetPlayer()->m_ColorFeetOld = apCloseCCharacters[i]->GetPlayer()->m_TeeInfos.m_ColorFeet;
					}
					apCloseCCharacters[i]->GetPlayer()->m_Rainbow = RAINBOW_BLACKWHITE;*/
				apCloseCCharacters[i]->m_FirstFreezeTick = 0;

				if(g_Config.m_SvFlagSounds)
				{
					for(int c = 0; c < MAX_CLIENTS; c++)
					{
						CPlayer *pPlayer = GameServer()->m_apPlayers[c];
						if(!pPlayer)
							continue;

						if(pPlayer->GetTeam() == TEAM_SPECTATORS && pPlayer->m_SpectatorID != SPEC_FREEVIEW && GameServer()->m_apPlayers[pPlayer->m_SpectatorID] && GameServer()->m_apPlayers[pPlayer->m_SpectatorID]->GetTeam() == fi)
							GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, c);
						else if(pPlayer->GetTeam() == fi)
							GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, c);
						else
							GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_PL, c);
					}
				}
				// demo record entry
				GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, -2);
				break;
			}
		}

		if(!F->m_AtStand)
		{
			if(F->m_DropTick && Server()->Tick() > F->m_DropTick + Server()->TickSpeed() * 90)
			{
				GameServer()->CreateSoundGlobal(SOUND_CTF_RETURN);
				F->Reset();
			}
			else
			{
				//Friction
				m_IsGrounded = false;
				if(GameServer()->Collision()->CheckPoint(F->m_Pos.x + CFlag::ms_PhysSize / 2, F->m_Pos.y + CFlag::ms_PhysSize / 2 + 5))
					m_IsGrounded = true;
				if(GameServer()->Collision()->CheckPoint(F->m_Pos.x - CFlag::ms_PhysSize / 2, F->m_Pos.y + CFlag::ms_PhysSize / 2 + 5))
					m_IsGrounded = true;

				if(m_IsGrounded)
				{
					F->m_Vel.x *= 0.75f;
				}
				else
				{
					F->m_Vel.x *= 0.98f;
				}

				//Gravity
				F->m_Vel.y += GameServer()->Tuning()->m_Gravity;

				//Speedups
				if(GameServer()->Collision()->IsSpeedup(GameServer()->Collision()->GetMapIndex(F->m_Pos)))
				{
					int Force, MaxSpeed = 0;
					vec2 Direction, TempVel = F->m_Vel;
					float TeeAngle, SpeederAngle, DiffAngle, SpeedLeft, TeeSpeed;
					GameServer()->Collision()->GetSpeedup(GameServer()->Collision()->GetMapIndex(F->m_Pos), &Direction, &Force, &MaxSpeed);

					if(Force == 255 && MaxSpeed)
					{
						F->m_Vel = Direction * (MaxSpeed / 5);
					}

					else
					{
						if(MaxSpeed > 0 && MaxSpeed < 5)
							MaxSpeed = 5;
						//dbg_msg("speedup tile start","Direction %f %f, Force %d, Max Speed %d", (Direction).x,(Direction).y, Force, MaxSpeed);
						if(MaxSpeed > 0)
						{
							if(Direction.x > 0.0000001f)
								SpeederAngle = -atan(Direction.y / Direction.x);
							else if(Direction.x < 0.0000001f)
								SpeederAngle = atan(Direction.y / Direction.x) + 2.0f * asin(1.0f);
							else if(Direction.y > 0.0000001f)
								SpeederAngle = asin(1.0f);
							else
								SpeederAngle = asin(-1.0f);

							if(SpeederAngle < 0)
								SpeederAngle = 4.0f * asin(1.0f) + SpeederAngle;

							if(TempVel.x > 0.0000001f)
								TeeAngle = -atan(TempVel.y / TempVel.x);
							else if(TempVel.x < 0.0000001f)
								TeeAngle = atan(TempVel.y / TempVel.x) + 2.0f * asin(1.0f);
							else if(TempVel.y > 0.0000001f)
								TeeAngle = asin(1.0f);
							else
								TeeAngle = asin(-1.0f);

							if(TeeAngle < 0)
								TeeAngle = 4.0f * asin(1.0f) + TeeAngle;

							TeeSpeed = sqrt(pow(TempVel.x, 2) + pow(TempVel.y, 2));

							DiffAngle = SpeederAngle - TeeAngle;
							SpeedLeft = MaxSpeed / 5.0f - cos(DiffAngle) * TeeSpeed;
							//dbg_msg("speedup tile debug","MaxSpeed %i, TeeSpeed %f, SpeedLeft %f, SpeederAngle %f, TeeAngle %f", MaxSpeed, TeeSpeed, SpeedLeft, SpeederAngle, TeeAngle);
							if(abs(SpeedLeft) > Force && SpeedLeft > 0.0000001f)
								TempVel += Direction * Force;
							else if(abs(SpeedLeft) > Force)
								TempVel += Direction * -Force;
							else
								TempVel += Direction * SpeedLeft;
						}
						else
							TempVel += Direction * Force;
					}
					F->m_Vel = TempVel;
				}

				GameServer()->Collision()->MoveBox(
					&F->m_Pos,
					&F->m_Vel,
					vec2(F->ms_PhysSize, F->ms_PhysSize),
					vec2(GameServer()->TuningList()[F->m_TuneZone].m_GroundElasticityX, GameServer()->TuningList()[F->m_TuneZone].m_GroundElasticityY)
				);
			}
		}
	}
}

bool CGameControllerDDRace::OnEntity(int Index, int x, int y, int Layer, int Flags, bool Initial, int Number)
{
	IGameController::OnEntity(Index, x, y, Layer, Flags, Initial, Number);

	const vec2 Pos(x * 32.0f + 16.0f, y * 32.0f + 16.0f);
	int Team = -1;
	if(Index == ENTITY_FLAGSTAND_RED)
		Team = TEAM_RED;
	if(Index == ENTITY_FLAGSTAND_BLUE)
		Team = TEAM_BLUE;
	if(Team == -1 || m_apFlags[Team])
		return false;

	CFlag *F = new CFlag(&GameServer()->m_World, Team);
	F->m_StandPos = Pos;
	F->m_Pos = Pos;
	m_apFlags[Team] = F;
	GameServer()->m_World.InsertEntity(F);
	return true;
}

int CGameControllerDDRace::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int WeaponID)
{
	IGameController::OnCharacterDeath(pVictim, pKiller, WeaponID);
	int HadFlag = 0;

	// drop flags
	for(auto &Flag : m_apFlags)
	{
		if(!Flag)
			continue;

		if(pKiller && pKiller->GetCharacter() && Flag->m_pCarryingCharacter == pKiller->GetCharacter())
			HadFlag |= 2;
		if(Flag->m_pCarryingCharacter == pVictim)
		{
			if(g_Config.m_SvFlagSounds)
			{
				GameServer()->CreateSoundGlobal(SOUND_CTF_DROP);
			}
			/*pVictim->GetPlayer()->m_Rainbow = false;
			pVictim->GetPlayer()->m_TeeInfos.m_ColorBody = pVictim->GetPlayer()->m_ColorBodyOld;
			pVictim->GetPlayer()->m_TeeInfos.m_ColorFeet = pVictim->GetPlayer()->m_ColorFeetOld;*/
			Flag->m_DropTick = Server()->Tick();
			Flag->m_pCarryingCharacter = 0;
			Flag->m_Vel = vec2(0, 0);

			HadFlag |= 1;
		}
		if(Flag->m_pLastCarryingCharacter == pVictim)
			Flag->m_pLastCarryingCharacter = 0;
	}

	return HadFlag;
}

void CGameControllerDDRace::ChangeFlagOwner(int id, int character)
{
	CFlag *F = m_apFlags[id];
	if((m_apFlags[0] && m_apFlags[0]->m_pCarryingCharacter == GameServer()->GetPlayerChar(character)) || (m_apFlags[1] && m_apFlags[1]->m_pCarryingCharacter == GameServer()->GetPlayerChar(character)))
	{
	}
	else
	{
		F->m_AtStand = 0;

		if(g_Config.m_SvFlagSounds)
		{
			GameServer()->CreateSoundGlobal(SOUND_CTF_DROP);
		}
		/*
		F->m_pCarryingCharacter->GetPlayer()->m_Rainbow = false;
		F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorBody = F->m_pCarryingCharacter->GetPlayer()->m_ColorBodyOld;
		F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorFeet = F->m_pCarryingCharacter->GetPlayer()->m_ColorFeetOld;*/

		if(GameServer()->m_apPlayers[character] && GameServer()->m_apPlayers[character]->GetCharacter()) //ChillerDragon's crashbug protection //didnt understand the bug didnt test the portection better comment it out //uncommented agian yolo
		{
			F->m_pCarryingCharacter = GameServer()->m_apPlayers[character]->GetCharacter();
			/*
			if (!F->m_pCarryingCharacter->GetPlayer()->m_Rainbow){
			F->m_pCarryingCharacter->GetPlayer()->m_ColorBodyOld = F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorBody;
			F->m_pCarryingCharacter->GetPlayer()->m_ColorFeetOld = F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorFeet;
			}
			F->m_pCarryingCharacter->GetPlayer()->m_Rainbow = RAINBOW_BLACKWHITE;*/
			F->m_pCarryingCharacter->GetPlayer()->GetCharacter()->m_FirstFreezeTick = 0;
		}
	}
}

int CGameControllerDDRace::HasFlag(CCharacter *pChr)
{
	if(!pChr)
		return -1;

	for(auto &Flag : m_apFlags)
		if(Flag && Flag->m_pCarryingCharacter == pChr)
			return pChr->GetPlayer()->GetCID();
	return -1;
}

void CGameControllerDDRace::DropFlag(int id, int dir)
{
	CFlag *F = m_apFlags[id]; //red=0 blue=1
	if(!F)
		return;

	if(g_Config.m_SvFlagSounds)
	{
		GameServer()->CreateSoundGlobal(SOUND_CTF_DROP);
	}
	if(F->m_pCarryingCharacter && F->m_pCarryingCharacter->GetPlayer())
	{
		/*F->m_pCarryingCharacter->GetPlayer()->m_Rainbow = false;
		F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorBody = F->m_pCarryingCharacter->GetPlayer()->m_ColorBodyOld;
		F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorFeet = F->m_pCarryingCharacter->GetPlayer()->m_ColorFeetOld;*/
		F->m_pCarryingCharacter->GetPlayer()->m_ChangeTeamOnFlag = true;
		F->m_pLastCarryingCharacter = F->m_pCarryingCharacter;
	}
	F->m_DropTick = Server()->Tick();
	F->m_DropFreezeTick = Server()->Tick();
	F->m_pCarryingCharacter = 0;
	F->m_Vel = vec2(5 * dir, -5);
}

void CGameControllerDDRace::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	int FlagCarrierRed = FLAG_MISSING;
	if(m_apFlags[TEAM_RED])
	{
		if(m_apFlags[TEAM_RED]->m_AtStand)
			FlagCarrierRed = FLAG_ATSTAND;
		else if(m_apFlags[TEAM_RED]->GetCarrier() && m_apFlags[TEAM_RED]->GetCarrier()->GetPlayer())
			FlagCarrierRed = m_apFlags[TEAM_RED]->GetCarrier()->GetPlayer()->GetCID();
		else
			FlagCarrierRed = FLAG_TAKEN;
	}

	int FlagCarrierBlue = FLAG_MISSING;
	if(m_apFlags[TEAM_BLUE])
	{
		if(m_apFlags[TEAM_BLUE]->m_AtStand)
			FlagCarrierBlue = FLAG_ATSTAND;
		else if(m_apFlags[TEAM_BLUE]->GetCarrier() && m_apFlags[TEAM_BLUE]->GetCarrier()->GetPlayer())
			FlagCarrierBlue = m_apFlags[TEAM_BLUE]->GetCarrier()->GetPlayer()->GetCID();
		else
			FlagCarrierBlue = FLAG_TAKEN;
	}

	if(Server()->IsSixup(SnappingClient))
	{
		protocol7::CNetObj_GameDataFlag *pGameDataObj = static_cast<protocol7::CNetObj_GameDataFlag *>(Server()->SnapNewItem(-protocol7::NETOBJTYPE_GAMEDATAFLAG, 0, sizeof(protocol7::CNetObj_GameDataFlag)));
		if(!pGameDataObj)
			return;

		pGameDataObj->m_FlagCarrierRed = FlagCarrierRed;
		pGameDataObj->m_FlagCarrierBlue = FlagCarrierBlue;
	}
	else
	{
		CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
		if(!pGameDataObj)
			return;

		pGameDataObj->m_FlagCarrierRed = FlagCarrierRed;
		pGameDataObj->m_FlagCarrierBlue = FlagCarrierBlue;
	}
}

void CGameControllerDDRace::HandleCharacterTilesDDPP(class CCharacter *pChr, int m_TileIndex, int m_TileFIndex, int Tile1, int Tile2, int Tile3, int Tile4, int FTile1, int FTile2, int FTile3, int FTile4, int PlayerDDRaceState)
{
	int ClientID = pChr->GetPlayer()->GetCID();
	// start
	if(((m_TileIndex == TILE_START) || (m_TileFIndex == TILE_START) || FTile1 == TILE_START || FTile2 == TILE_START || FTile3 == TILE_START || FTile4 == TILE_START || Tile1 == TILE_START || Tile2 == TILE_START || Tile3 == TILE_START || Tile4 == TILE_START) && (PlayerDDRaceState == DDRACE_NONE || PlayerDDRaceState == DDRACE_FINISHED || (PlayerDDRaceState == DDRACE_STARTED && !GetPlayerTeam(ClientID) && g_Config.m_SvTeam != 3)))
	{
		pChr->GetPlayer()->m_MoneyTilePlus = true;
		if(pChr->GetPlayer()->m_QuestState == CPlayer::QUEST_RACE)
		{
			if((pChr->GetPlayer()->m_QuestStateLevel == 3 || pChr->GetPlayer()->m_QuestStateLevel == 8) && pChr->GetPlayer()->m_QuestProgressValue)
			{
				GameServer()->QuestAddProgress(pChr->GetPlayer()->GetCID(), 2);
			}
			else if(pChr->GetPlayer()->m_QuestStateLevel == 9 && pChr->GetPlayer()->m_QuestFailed)
			{
				// GameServer()->SendChatTarget(pChr->GetPlayer()->GetCID(), "[QUEST] running agian.");
				pChr->GetPlayer()->m_QuestFailed = false;
			}
		}
		pChr->m_DDPP_Finished = false;
	}
	// finish
	if(((m_TileIndex == TILE_FINISH) || (m_TileFIndex == TILE_FINISH) || FTile1 == TILE_FINISH || FTile2 == TILE_FINISH || FTile3 == TILE_FINISH || FTile4 == TILE_FINISH || Tile1 == TILE_FINISH || Tile2 == TILE_FINISH || Tile3 == TILE_FINISH || Tile4 == TILE_FINISH) && PlayerDDRaceState == DDRACE_STARTED)
	{
		if(pChr->GetPlayer()->m_QuestState == CPlayer::QUEST_RACE)
		{
			if(pChr->GetPlayer()->m_QuestStateLevel == 5)
			{
				if(HasFlag(pChr) != -1) //has flag
				{
					GameServer()->QuestCompleted(pChr->GetPlayer()->GetCID());
				}
				else
				{
					GameServer()->QuestFailed(pChr->GetPlayer()->GetCID());
				}
			}
			else if(pChr->GetPlayer()->m_QuestStateLevel == 9)
			{
				if(!pChr->GetPlayer()->m_QuestFailed)
				{
					GameServer()->QuestCompleted(pChr->GetPlayer()->GetCID());
				}
			}
		}

		pChr->m_DummyFinished = true;
		pChr->m_DummyFinishes++;

		/*
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "xp [%d/1000]", pChr->GetPlayer()->GetXP());
		GameServer()->SendBroadcast(aBuf, pChr->GetPlayer()->GetCID(), 0);
		*/
	}
}
