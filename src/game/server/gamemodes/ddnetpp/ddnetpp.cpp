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

CGameControllerDDNetPP::CGameControllerDDNetPP(class CGameContext *pGameServer) :
	CGameControllerDDRace(pGameServer)
{
	m_pGameType = g_Config.m_SvTestingCommands ? g_Config.m_SvGameTypeNameTest : g_Config.m_SvGameTypeName;
	m_GameFlags = GAMEFLAG_FLAGS;
}

CGameControllerDDNetPP::~CGameControllerDDNetPP() = default;

void CGameControllerDDNetPP::Tick()
{
	CGameControllerDDRace::Tick();

	FlagTick();
	DetectReconnectFlood();
}

void CGameControllerDDNetPP::SetArmorProgress(CCharacter *pCharacer, int Progress)
{
	if(!pCharacer->GetPlayer()->m_IsVanillaDmg)
		CGameControllerDDRace::SetArmorProgress(pCharacer, Progress);
}

bool CGameControllerDDNetPP::CanJoinTeam(int Team, int NotThisId, char *pErrorReason, int ErrorReasonSize)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[NotThisId];
	if(!pPlayer)
		return CGameControllerDDRace::CanJoinTeam(Team, NotThisId, pErrorReason, ErrorReasonSize);

	if(g_Config.m_SvRequireLogin && g_Config.m_SvAccountStuff)
	{
		if(!pPlayer->IsLoggedIn())
		{
			const char *pReason = GameServer()->Loc("You need to be logged in to play. \nGet an account with '/register <name> <pw> <pw>'", pPlayer->GetCid());
			str_copy(pErrorReason, pReason, ErrorReasonSize);
			return false;
		}
	}

	return CGameControllerDDRace::CanJoinTeam(Team, NotThisId, pErrorReason, ErrorReasonSize);
}

void CGameControllerDDNetPP::PrintJoinMessage(CPlayer *pPlayer)
{
	if(!pPlayer->m_PendingJoinMessage)
		return;

	pPlayer->m_PendingJoinMessage = false;

	char aBuf[512];
	int ClientId = pPlayer->GetCid();
	str_format(aBuf, sizeof(aBuf), "'%s' entered and joined the %s", Server()->ClientName(ClientId), GetTeamName(pPlayer->GetTeam()));
	GameServer()->SendChat(-1, TEAM_ALL, aBuf, -1, CGameContext::FLAG_SIX);
}

// full overwrites ddnet's OnPlayerConnect!
void CGameControllerDDNetPP::OnPlayerConnect(class CPlayer *pPlayer)
{
	// this code has to be manually kept in sync with CGameControllerDDrace::OnPlayerConnect()
	IGameController::OnPlayerConnect(pPlayer);
	int ClientId = pPlayer->GetCid();

	// init the player
	Score()->PlayerData(ClientId)->Reset();

	// Can't set score here as LoadScore() is threaded, run it in
	// LoadScoreThreaded() instead
	Score()->LoadPlayerData(ClientId);

	if(!Server()->ClientPrevIngame(ClientId))
	{
		char aBuf[512];
		if(!pPlayer->m_SilentJoinMessage)
		{
			if(GameServer()->ShowJoinMessage(ClientId))
			{
				PrintJoinMessage(pPlayer);
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "'%s' entered and joined the %s (message hidden)", Server()->ClientName(ClientId), GetTeamName(pPlayer->GetTeam()));
				ddpp_log(DDPP_LOG_FLOOD, aBuf);
			}
		}
		if(g_Config.m_SvInstagibMode)
		{
			GameServer()->SendChatTarget(ClientId, "DDNet++ Instagib Mod (" DDNETPP_VERSIONSTR ") based on DDNet " GAME_RELEASE_VERSION);
		}
		else
		{
			char aWelcome[128];
			char aSubGameType[128];
			aSubGameType[0] = '\0';
			if(g_Config.m_SvDDPPgametype[0])
				str_format(aSubGameType, sizeof(aSubGameType), "(%s) ", g_Config.m_SvDDPPgametype);
			str_format(aWelcome, sizeof(aWelcome), "DDNet++ %s%s based on DDNet " GAME_RELEASE_VERSION, aSubGameType, DDNETPP_VERSIONSTR);
			GameServer()->SendChatTarget(ClientId, aWelcome);
		}
	}

	m_NumConnectionsInTheLastMinute++;
	m_NumConnectionsInTheLast10Minutes++;

	if(g_Config.m_SvRequireLogin && g_Config.m_SvAccountStuff)
	{
		if(!pPlayer->IsLoggedIn())
		{
			const char *pReason = GameServer()->Loc("You need to be logged in to play. \nGet an account with '/register <name> <pw> <pw>'", pPlayer->GetCid());
			GameServer()->SendBroadcast(pReason, pPlayer->GetCid());
			GameServer()->SendChatTarget(pPlayer->GetCid(), pReason);
		}
	}
}

void CGameControllerDDNetPP::OnPlayerDisconnect(class CPlayer *pPlayer, const char *pReason, bool Silent)
{
	if(pPlayer->m_PendingJoinMessage)
		Silent = true;
	CGameControllerDDRace::OnPlayerDisconnect(pPlayer, pReason, Silent);
}

void CGameControllerDDNetPP::DoTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg)
{
	if(!GameServer()->ShowJoinMessage(pPlayer->GetCid()))
		DoChatMsg = false;
	CGameControllerDDRace::DoTeamChange(pPlayer, Team, DoChatMsg);
}

int CGameControllerDDNetPP::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int WeaponId)
{
	CGameControllerDDRace::OnCharacterDeath(pVictim, pKiller, WeaponId);
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

void CGameControllerDDNetPP::DetectReconnectFlood()
{
	int Threshold = 0;
	if(GameServer()->CountConnectedHumans() > 10)
		Threshold = 10;

	bool HitThreshold = false;

	if(m_NumConnectionsInTheLastMinute > 10 + Threshold)
		HitThreshold = true;

	// 2 connections per minute on average seems fine
	// but if that constantly happens over a span of 10 minutes
	// it will get annoying!
	if(m_NumConnectionsInTheLast10Minutes > 20 + Threshold)
		HitThreshold = true;

	// activate
	if(HitThreshold)
	{
		m_LastConnectionSpamThresholdHit = time_get();
		if(!GameServer()->ReconnectFlood())
		{
			ddpp_log(DDPP_LOG_FLOOD, "activate anti reconnect flood");
			GameServer()->SetReconnectFlood(true);
		}
	}
	// deactivate
	else if(GameServer()->ReconnectFlood() && m_LastConnectionSpamThresholdHit)
	{
		int SecondsSinceLastHit = (time_get() - m_LastConnectionSpamThresholdHit) / time_freq();
		if(SecondsSinceLastHit > 1000)
		{
			ddpp_log(DDPP_LOG_FLOOD, "deactivate anti reconnect flood");
			GameServer()->SetReconnectFlood(false);
		}
	}

	if(m_NextMinuteReset < time_get())
	{
		m_NextMinuteReset = time_get() + time_freq() * 60;
		m_NumConnectionsInTheLastMinute = 0;
	}

	if(m_Next10MinutesReset < time_get())
	{
		m_NextMinuteReset = time_get() + time_freq() * 60 * 10;
		m_NumConnectionsInTheLast10Minutes = 0;
	}
}

void CGameControllerDDNetPP::FlagTick()
{
	for(int Fi = 0; Fi < 2; Fi++)
	{
		CFlag *F = m_apFlags[Fi];

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
				GameServer()->SendChatTarget(F->m_pCarryingCharacter->GetPlayer()->GetCid(), aBuf);*/

				if(Server()->Tick() > F->m_pCarryingCharacter->m_FirstFreezeTick + Server()->TickSpeed() * 8)
				{
					/*F->m_pCarryingCharacter->GetPlayer()->m_Rainbow = false;
					F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorBody = F->m_pCarryingCharacter->GetPlayer()->m_ColorBodyOld;
					F->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorFeet = F->m_pCarryingCharacter->GetPlayer()->m_ColorFeetOld;*/
					if(m_apFlags[0] && m_apFlags[0]->m_pCarryingCharacter && m_apFlags[0]->m_pCarryingCharacter == F->m_pCarryingCharacter->GetPlayer()->GetCharacter())
					{
						DropFlag(0, F->m_pCarryingCharacter->GetPlayer()->GetCharacter()->GetAimDir()); //red
						//SendChatTarget(F->m_pCarryingCharacter->GetPlayer()->GetCid(), "you dropped red flag");
					}
					else if(m_apFlags[1] && m_apFlags[1]->m_pCarryingCharacter && m_apFlags[1]->m_pCarryingCharacter == F->m_pCarryingCharacter->GetPlayer()->GetCharacter())
					{
						DropFlag(1, F->m_pCarryingCharacter->GetPlayer()->GetCharacter()->GetAimDir()); //blue
						//SendChatTarget(F->m_pCarryingCharacter->GetPlayer()->GetCid(), "you dropped blue flag");
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

				// only allow flag grabs in team 0
				if(apCloseCCharacters[i]->IsInDDraceTeam())
					continue;
				if(apCloseCCharacters[i]->Core()->m_Solo)
					continue;

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
					F->m_Vel = TempVel;
				}

				GameServer()->Collision()->MoveBox(
					&F->m_Pos,
					&F->m_Vel,
					vec2(CFlag::ms_PhysSize, CFlag::ms_PhysSize),
					vec2(GameServer()->TuningList()[F->m_TuneZone].m_GroundElasticityX, GameServer()->TuningList()[F->m_TuneZone].m_GroundElasticityY));
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
		/*F->m_pCarryingCharacter->GetPlayer()->m_Rainbow = false;
		pFlag->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorBody = F->m_pCarryingCharacter->GetPlayer()->m_ColorBodyOld;
		pFlag->m_pCarryingCharacter->GetPlayer()->m_TeeInfos.m_ColorFeet = F->m_pCarryingCharacter->GetPlayer()->m_ColorFeetOld;*/
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

// TODO: move to gamecontext because thats probably useful everywhere for example the extra vote menu
const char *CGameControllerDDNetPP::CommandByVoteMsg(const CNetMsg_Cl_CallVote *pMsg)
{
	if(str_comp_nocase(pMsg->m_pType, "option"))
		return "";

	CVoteOptionServer *pOption = GameServer()->m_pVoteOptionFirst;
	while(pOption)
	{
		if(str_comp_nocase(pMsg->m_pValue, pOption->m_aDescription) == 0)
			return pOption->m_aCommand;
		pOption = pOption->m_pNext;
	}
	return "";
}

// return true to drop the vote
bool CGameControllerDDNetPP::OnCallVoteNetMessage(const CNetMsg_Cl_CallVote *pMsg, int ClientId)
{
	if(GameServer()->m_VotingBlockedUntil == -1 || (GameServer()->m_VotingBlockedUntil > time_get()))
	{
		if(str_comp(CommandByVoteMsg(pMsg), "unblock_votes"))
		{
			// dbg_msg("ddnet++", "blocked vote '%s' != '%s'", pMsg->m_pType, "unblock_votes");
			if(GameServer()->m_VotingBlockedUntil == -1)
			{
				GameServer()->SendChatTarget(ClientId, "votes are currently blocked by a vote");
			}
			else
			{
				char aBuf[512];
				int Seconds = (GameServer()->m_VotingBlockedUntil - time_get()) / time_freq();
				int Minutes = Seconds / 60;
				if(Seconds > 60)
					str_format(aBuf, sizeof(aBuf), "votes are blocked for %d more minute%s", Minutes, Minutes == 1 ? "" : "s");
				else
					str_format(aBuf, sizeof(aBuf), "votes are blocked for %d more seconds", Seconds);
				GameServer()->SendChatTarget(ClientId, aBuf);
			}
			return true;
		}
	}
	return false;
}
