#include <base/ddpp_logs.h>
#include <base/system.h>
#include <engine/shared/config.h>
#include <engine/shared/protocol.h>
#include <game/generated/protocol.h>
#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/entities/flag.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/player.h>
#include <game/server/score.h>
#include <game/version.h>

#include "ddnetpp.h"

void CGameControllerDDNetPP::FlagTick()
{
	for(int Fi = 0; Fi < 2; Fi++)
	{
		CFlag *pFlag = m_apFlags[Fi];

		if(!pFlag)
			continue;

		// flag hits death-tile or left the game layer, reset it
		if(GameServer()->Collision()->GetCollisionAt(pFlag->m_Pos.x, pFlag->m_Pos.y) == TILE_DEATH || pFlag->GameLayerClipped(pFlag->m_Pos))
		{
			GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", "flag_return");
			if(g_Config.m_SvFlagSounds)
			{
				GameServer()->CreateSoundGlobal(SOUND_CTF_RETURN);
			}
			pFlag->Reset();
			continue;
		}

		//
		if(pFlag->m_pCarryingCharacter)
		{
			// update flag position
			pFlag->m_Pos = pFlag->m_pCarryingCharacter->m_Pos;

			if(pFlag->m_pCarryingCharacter->m_FirstFreezeTick != 0)
			{
				/*char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "Your freeze Time Amount is: %i AND %i",pFlag->m_pCarryingCharacter->m_FirstFreezeTick + Server()->TickSpeed()*8, Server()->Tick());
				GameServer()->SendChatTarget(pFlag->m_pCarryingCharacter->GetPlayer()->GetCid(), aBuf);*/

				if(Server()->Tick() > pFlag->m_pCarryingCharacter->m_FirstFreezeTick + Server()->TickSpeed() * 8)
				{
					/*pFlag->m_pCarryingCharacter->GetPlayer()->m_Rainbow = false;
					pFlag->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorBody = pFlag->m_pCarryingCharacter->GetPlayer()->m_ColorBodyOld;
					pFlag->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorFeet = pFlag->m_pCarryingCharacter->GetPlayer()->m_ColorFeetOld;*/
					if(m_apFlags[0] && m_apFlags[0]->m_pCarryingCharacter && m_apFlags[0]->m_pCarryingCharacter == pFlag->m_pCarryingCharacter->GetPlayer()->GetCharacter())
					{
						DropFlag(0, pFlag->m_pCarryingCharacter->GetPlayer()->GetCharacter()->GetAimDir()); //red
						//SendChatTarget(pFlag->m_pCarryingCharacter->GetPlayer()->GetCid(), "you dropped red flag");
					}
					else if(m_apFlags[1] && m_apFlags[1]->m_pCarryingCharacter && m_apFlags[1]->m_pCarryingCharacter == pFlag->m_pCarryingCharacter->GetPlayer()->GetCharacter())
					{
						DropFlag(1, pFlag->m_pCarryingCharacter->GetPlayer()->GetCharacter()->GetAimDir()); //blue
						//SendChatTarget(pFlag->m_pCarryingCharacter->GetPlayer()->GetCid(), "you dropped blue flag");
					}
				}
			}
		}
		else
		{
			if(GameServer()->Collision()->GetTileIndex(GameServer()->Collision()->GetMapIndex(pFlag->m_Pos)) == 95)
			{
				if(g_Config.m_SvFlagSounds)
				{
					GameServer()->CreateSoundGlobal(SOUND_CTF_RETURN);
				}
				pFlag->Reset();
			}

			CCharacter *apCloseCCharacters[MAX_CLIENTS];
			int Num = GameServer()->m_World.FindEntities(pFlag->m_Pos, CFlag::ms_PhysSize, (CEntity **)apCloseCCharacters, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
			for(int i = 0; i < Num; i++)
			{
				if(!apCloseCCharacters[i]->IsAlive() || apCloseCCharacters[i]->GetPlayer()->GetTeam() == TEAM_SPECTATORS || GameServer()->Collision()->IntersectLine(pFlag->m_Pos, apCloseCCharacters[i]->m_Pos, NULL, NULL))
					continue;
				if(m_apFlags[0] && m_apFlags[1])
				{
					if(m_apFlags[0]->m_pCarryingCharacter == apCloseCCharacters[i] || m_apFlags[1]->m_pCarryingCharacter == apCloseCCharacters[i] || (pFlag->m_pLastCarryingCharacter == apCloseCCharacters[i] && (pFlag->m_DropFreezeTick + Server()->TickSpeed() * 2) > Server()->Tick()))
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

				// only allow flag grabs in team 0
				if(apCloseCCharacters[i]->IsInDDraceTeam())
					continue;
				if(apCloseCCharacters[i]->Core()->m_Solo)
					continue;

				// take the flag
				if(pFlag->m_AtStand)
				{
					pFlag->m_GrabTick = Server()->Tick();
				}

				pFlag->m_AtStand = 0;
				pFlag->m_pCarryingCharacter = apCloseCCharacters[i];
				pFlag->m_pCarryingCharacter->GetPlayer()->m_ChangeTeamOnFlag = true;
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

						if(pPlayer->GetTeam() == TEAM_SPECTATORS && pPlayer->m_SpectatorId != SPEC_FREEVIEW && GameServer()->m_apPlayers[pPlayer->m_SpectatorId] && GameServer()->m_apPlayers[pPlayer->m_SpectatorId]->GetTeam() == Fi)
							GameServer()->CreateSoundGlobal(SOUND_CTF_GRAB_EN, c);
						else if(pPlayer->GetTeam() == Fi)
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

		if(!pFlag->m_AtStand)
		{
			if(pFlag->m_DropTick && Server()->Tick() > pFlag->m_DropTick + Server()->TickSpeed() * 90)
			{
				GameServer()->CreateSoundGlobal(SOUND_CTF_RETURN);
				pFlag->Reset();
			}
			else
			{
				//Friction
				pFlag->m_IsGrounded = false;
				if(GameServer()->Collision()->CheckPoint(pFlag->m_Pos.x + CFlag::ms_PhysSize / 2, pFlag->m_Pos.y + CFlag::ms_PhysSize / 2 + 5))
					pFlag->m_IsGrounded = true;
				if(GameServer()->Collision()->CheckPoint(pFlag->m_Pos.x - CFlag::ms_PhysSize / 2, pFlag->m_Pos.y + CFlag::ms_PhysSize / 2 + 5))
					pFlag->m_IsGrounded = true;

				if(pFlag->m_IsGrounded)
				{
					pFlag->m_Vel.x *= 0.75f;
				}
				else
				{
					pFlag->m_Vel.x *= 0.98f;
				}

				//Gravity
				pFlag->m_Vel.y += GameServer()->Tuning()->m_Gravity;

				//Speedups
				if(GameServer()->Collision()->IsSpeedup(GameServer()->Collision()->GetMapIndex(pFlag->m_Pos)))
				{
					int Force, MaxSpeed = 0;
					vec2 Direction, TempVel = pFlag->m_Vel;
					float TeeAngle, SpeederAngle, DiffAngle, SpeedLeft, TeeSpeed;
					GameServer()->Collision()->GetSpeedup(GameServer()->Collision()->GetMapIndex(pFlag->m_Pos), &Direction, &Force, &MaxSpeed);

					if(Force == 255 && MaxSpeed)
					{
						pFlag->m_Vel = Direction * (MaxSpeed / 5);
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
								SpeederAngle = atan(Direction.y / Direction.x) + 2.0f * std::asin(1.0f);
							else if(Direction.y > 0.0000001f)
								SpeederAngle = std::asin(1.0f);
							else
								SpeederAngle = std::asin(-1.0f);

							if(SpeederAngle < 0)
								SpeederAngle = 4.0f * std::asin(1.0f) + SpeederAngle;

							if(TempVel.x > 0.0000001f)
								TeeAngle = -atan(TempVel.y / TempVel.x);
							else if(TempVel.x < 0.0000001f)
								TeeAngle = atan(TempVel.y / TempVel.x) + 2.0f * std::asin(1.0f);
							else if(TempVel.y > 0.0000001f)
								TeeAngle = std::asin(1.0f);
							else
								TeeAngle = std::asin(-1.0f);

							if(TeeAngle < 0)
								TeeAngle = 4.0f * std::asin(1.0f) + TeeAngle;

							TeeSpeed = sqrt(pow(TempVel.x, 2) + pow(TempVel.y, 2));

							DiffAngle = SpeederAngle - TeeAngle;
							SpeedLeft = MaxSpeed / 5.0f - std::cos(DiffAngle) * TeeSpeed;
							//dbg_msg("speedup tile debug","MaxSpeed %i, TeeSpeed %f, SpeedLeft %f, SpeederAngle %f, TeeAngle %f", MaxSpeed, TeeSpeed, SpeedLeft, SpeederAngle, TeeAngle);
							if(std::abs(SpeedLeft) > Force && SpeedLeft > 0.0000001f)
								TempVel += Direction * Force;
							else if(std::abs(SpeedLeft) > Force)
								TempVel += Direction * -Force;
							else
								TempVel += Direction * SpeedLeft;
						}
						else
							TempVel += Direction * Force;
					}
					pFlag->m_Vel = TempVel;
				}

				GameServer()->Collision()->MoveBox(
					&pFlag->m_Pos,
					&pFlag->m_Vel,
					vec2(CFlag::ms_PhysSize, CFlag::ms_PhysSize),
					vec2(GameServer()->TuningList()[pFlag->m_TuneZone].m_GroundElasticityX, GameServer()->TuningList()[pFlag->m_TuneZone].m_GroundElasticityY));
			}
		}
	}
}

void CGameControllerDDNetPP::DropFlag(int FlagId, int Dir)
{
	CFlag *pFlag = m_apFlags[FlagId]; //red=0 blue=1
	if(!pFlag)
		return;

	if(g_Config.m_SvFlagSounds)
	{
		GameServer()->CreateSoundGlobal(SOUND_CTF_DROP);
	}
	if(pFlag->m_pCarryingCharacter && pFlag->m_pCarryingCharacter->GetPlayer())
	{
		pFlag->m_pCarryingCharacter->GetPlayer()->m_ChangeTeamOnFlag = true;
		pFlag->m_pLastCarryingCharacter = pFlag->m_pCarryingCharacter;
	}
	pFlag->m_DropTick = Server()->Tick();
	pFlag->m_DropFreezeTick = Server()->Tick();
	pFlag->m_pCarryingCharacter = 0;
	pFlag->m_Vel = vec2(5 * Dir, -5);
}

void CGameControllerDDNetPP::ChangeFlagOwner(int FlagId, int ClientId)
{
	dbg_assert(FlagId >= 0 && FlagId <= 1, "invalid flag id");
	if(ClientId < 0 || ClientId >= MAX_CLIENTS)
		return;
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return;
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	CFlag *pFlag = m_apFlags[FlagId];
	if((m_apFlags[0] && m_apFlags[0]->m_pCarryingCharacter == pChr) || (m_apFlags[1] && m_apFlags[1]->m_pCarryingCharacter == pChr))
	{
		// target already has a flag
		return;
	}

	if(g_Config.m_SvFlagSounds)
	{
		GameServer()->CreateSoundGlobal(SOUND_CTF_DROP);
	}

	pFlag->m_AtStand = 0;
	pFlag->m_pCarryingCharacter = pChr;
	pFlag->m_pCarryingCharacter->GetPlayer()->GetCharacter()->m_FirstFreezeTick = 0;
}

int CGameControllerDDNetPP::HasFlag(CCharacter *pChr)
{
	if(!pChr)
		return -1;

	for(CFlag *pFlag : m_apFlags)
		if(pFlag && pFlag->m_pCarryingCharacter == pChr)
			return pChr->GetPlayer()->GetCid();
	return -1;
}
