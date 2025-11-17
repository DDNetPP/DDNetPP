// gamecontext scoped balance ddnet++ methods

#include "block_tournament.h"

#include "../gamecontext.h"

#include <engine/shared/config.h>
#include <engine/shared/protocol.h>

#include <game/server/gamecontroller.h>
#include <game/server/teams.h>

bool CBlockTournament::IsActive(int ClientId)
{
	CPlayer *pPlayer = GameServer()->GetPlayerOrNullptr(ClientId);
	if(!pPlayer)
		return false;
	return pPlayer->m_IsBlockTourning;
}

void CBlockTournament::OnInit()
{
	m_SpawnCounter = 0;
	m_LobbyTick = 0;
	m_Tick = 0;
	m_CoolDown = 0;
	m_StartPlayers = 0;
}

void CBlockTournament::Leave(CPlayer *pPlayer)
{
	if(!pPlayer)
		return;

	pPlayer->m_IsBlockTourning = false;
	m_aRestorePos[pPlayer->GetCid()] = true;
}

void CBlockTournament::Join(CPlayer *pPlayer)
{
	if(!pPlayer)
		return;

	pPlayer->m_IsBlockTourning = true;
	pPlayer->m_IsBlockTourningDead = false;
	pPlayer->m_IsBlockTourningInArena = false;
}

bool CBlockTournament::AllowSelfKill(int ClientId)
{
	if(ClientId < 0 || ClientId > MAX_CLIENTS)
		return true;
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return true;
	if(!IsActive(ClientId))
		return true;

	if(State() == STATE_IN_GAME || State() == STATE_COOLDOWN)
	{
		if(m_Tick < Server()->TickSpeed() * 10)
		{
			// silent abort selfkill first 10 secs of the tournament
			// to avoid accidental selfkill when it starts
			return false;
		}
	}
	return true;
}

void CGameContext::OnStartBlockTournament()
{
	m_pBlockTournament->StartRound();
}

void CBlockTournament::StartRound()
{
	if(State() != STATE_OFF)
	{
		GameServer()->SendChat(-1, TEAM_ALL, "[EVENT] error tournament already running.");
		return;
	}
	if(g_Config.m_SvAllowBlockTourna == 0)
	{
		GameServer()->SendChat(-1, TEAM_ALL, "[EVENT] error tournaments are deactivated by an admin.");
		return;
	}

	m_State = STATE_LOBBY;
	m_LobbyTick = g_Config.m_SvBlockTournaDelay * Server()->TickSpeed();
	m_CoolDown = BLOCKTOURNAMENT_COOLDOWN * Server()->TickSpeed();
}

bool CBlockTournament::PickSpawn(vec2 *pPos, CPlayer *pPlayer)
{
	if(!pPlayer->m_IsBlockTourning)
		return false;
	if(pPlayer->m_IsBlockTourningDead)
		return false;
	if(State() != STATE_COOLDOWN)
		return false;

	int Id = pPlayer->GetCid();
	vec2 Pos = GetNextArenaSpawn(Id);
	if(Pos == vec2(-1, -1)) // fallback to ddr spawn if there is no arena
		return false;

	*pPos = Pos;
	return true;
}

void CBlockTournament::PostSpawn(CCharacter *pChr)
{
	if(!pChr)
		return;
	CPlayer *pPlayer = pChr->GetPlayer();
	if(!pPlayer)
		return;
	if(!pPlayer->m_IsBlockTourning)
		return;
	if(pPlayer->m_IsBlockTourningDead)
		return;
	if(State() != STATE_COOLDOWN)
		return;

	pChr->Freeze(BLOCKTOURNAMENT_COOLDOWN);
	pPlayer->m_IsBlockTourningInArena = true;
	pChr->m_BlockTournaDeadTicks = 0;
}

vec2 CBlockTournament::GetNextArenaSpawn(int ClientId)
{
	vec2 Spawn = Collision()->GetBlockTournamentSpawn(m_SpawnCounter++);
	if(Spawn == vec2(-1, -1))
	{
		// start re using spawns when there is not enough
		// but abort when we have to reuse in the beginning already
		if(m_SpawnCounter > 1)
		{
			m_SpawnCounter = 0;
			Spawn = Collision()->GetBlockTournamentSpawn(m_SpawnCounter++);
			if(Spawn != vec2(-1, -1))
				return Spawn;
		}
		SendChatAll("[EVENT] No block tournament arena found.");
		EndRound();
	}
	return Spawn;
}

void CBlockTournament::CharacterTick(CCharacter *pChr)
{
	if(State() != STATE_IN_GAME)
		return;
	if(!pChr)
		return;
	CPlayer *pPlayer = pChr->GetPlayer();
	if(!pPlayer)
		return;
	if(!IsActive(pPlayer->GetCid()))
		return;

	if(!pChr->m_FreezeTime)
	{
		pChr->m_BlockTournaDeadTicks = 0;
		return;
	}

	pChr->m_BlockTournaDeadTicks++;
	if(pChr->m_BlockTournaDeadTicks > 15 * Server()->TickSpeed())
		pChr->Die(pPlayer->GetCid(), WEAPON_SELF);
}

void CBlockTournament::SlowTick()
{
	if(State() != CBlockTournament::STATE_ENDING)
		return;

	for(auto &Player : GameServer()->m_apPlayers)
	{
		if(!Player)
			continue;
		if(!Player->m_IsBlockTourning)
			continue;

		Leave(Player);
		if(Player->GetCharacter())
			Player->GetCharacter()->Die(Player->GetCid(), WEAPON_GAME);
	}
	State(CBlockTournament::STATE_OFF);
}

void CBlockTournament::Tick()
{
	if(State() == STATE_OFF)
		return;

	char aBuf[128];

	if(State() == STATE_IN_GAME) //ingame
	{
		m_Tick++;
		if(m_Tick > g_Config.m_SvBlockTournaGameTime * Server()->TickSpeed() * 60) //time over --> draw
		{
			//kill all tournas
			for(auto &Player : GameServer()->m_apPlayers)
			{
				if(Player && Player->m_IsBlockTourning)
				{
					Leave(Player);
					if(Player->GetCharacter() && !Player->m_IsBlockTourningDead)
					{
						Player->GetCharacter()->Die(Player->GetCid(), WEAPON_GAME);
					}
				}
			}
			GameServer()->SendChat(-1, TEAM_ALL, "[EVENT] Block tournament stopped because time was over.");
			m_State = STATE_OFF;
		}
	}
	else if(m_State == STATE_LOBBY)
	{
		m_LobbyTick--;
		if(m_LobbyTick % Server()->TickSpeed() == 0)
		{
			int Blockers = CountAlive();
			if(Blockers < 0)
			{
				Blockers = 1;
			}
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(!GameServer()->m_apPlayers[i])
					continue;

				const char *pFormat = GameServer()->Loc(
					"[EVENT] BLOCK IN %d SECONDS\n"
					"%d joined (min %d)\n"
					"/join to participate",
					i);

				str_format(aBuf,
					sizeof(aBuf),
					pFormat,
					m_LobbyTick / Server()->TickSpeed(),
					Blockers,
					g_Config.m_SvBlockTournaPlayers);
				GameServer()->SendBroadcast(aBuf, i, 2);
			}
		}

		if(m_LobbyTick < 0)
		{
			m_StartPlayers = CountAlive();
			if(m_StartPlayers < g_Config.m_SvBlockTournaPlayers) //minimum x players needed to start a tourna
			{
				GameServer()->SendBroadcastAll("[EVENT] Block tournament failed! Not enough players.", 2);
				EndRound();
				return;
			}

			GameServer()->SendBroadcastAll("[EVENT] Block tournament started!", 2);
			m_State = STATE_COOLDOWN;
			m_Tick = 0;
			m_CoolDown = BLOCKTOURNAMENT_COOLDOWN * Server()->TickSpeed();

			//ready all players
			m_SpawnCounter = 0;
			for(auto &Player : GameServer()->m_apPlayers)
				if(Player && Player->m_IsBlockTourning)
					if(Player->GetCharacter())
					{
						SavePosition(Player);
						Player->GetCharacter()->Die(Player->GetCid(), WEAPON_GAME);
					}
		}
	}
	else if(State() == STATE_COOLDOWN)
	{
		m_CoolDown--;
		if(m_CoolDown % Server()->TickSpeed() == 0)
		{
			str_format(aBuf, sizeof(aBuf), "Start in %d", m_CoolDown / Server()->TickSpeed());
			SendBroadcastAll(aBuf);
		}
		if(m_CoolDown < 0)
		{
			SendBroadcastAll("Last alive wins!");
			SendBroadcastAll(" ");
			m_State = STATE_IN_GAME;
			for(auto &Player : GameServer()->m_apPlayers)
			{
				if(!Player)
					continue;
				if(!Player->m_IsBlockTourning)
					continue;
				if(Player->m_IsBlockTourningDead)
					continue;

				CCharacter *pChr = Player->GetCharacter();
				if(!pChr)
					continue;

				pChr->UnFreeze();
			}
		}
	}
}

void CBlockTournament::EndRound()
{
	m_State = STATE_OFF;

	for(auto &Player : GameServer()->m_apPlayers)
	{
		if(!Player)
			continue;
		if(!IsActive(Player->GetCid()))
			continue;

		Leave(Player);
		if(Player->GetCharacter() && !Player->m_IsBlockTourningDead && Player->m_IsBlockTourningInArena)
			Player->GetCharacter()->Die(Player->GetCid(), WEAPON_GAME);
		else if(Player->GetCharacter())
			m_aRestorePos[Player->GetCid()] = false;
	}
}

int CBlockTournament::CountAlive()
{
	int c = 0;
	int Id = -404;

	for(auto &Player : GameServer()->m_apPlayers)
	{
		if(Player)
		{
			if(Player->m_IsBlockTourning)
			{
				c++;
				Id = Player->GetCid();
			}
		}
	}

	if(c == 1) // one alive? --> return his id negative
	{
		if(Id == 0)
		{
			return -420;
		}
		else
		{
			return Id * -1;
		}
	}

	return c;
}

void CBlockTournament::OnDeath(CCharacter *pChr, int Killer, int Weapon)
{
	if(!pChr)
		return;

	CPlayer *pPlayer = pChr->GetPlayer();
	if(!pPlayer)
		return;

	if(!pPlayer->m_IsBlockTourning)
		return;
	if(pPlayer->m_IsBlockTourningDead)
		return;
	if(!pPlayer->m_IsBlockTourningInArena)
		return;
	if(State() != STATE_IN_GAME && State() != STATE_COOLDOWN) //ingame
		return;

	char aBuf[128];
	//let him die and check for tourna win
	Leave(pPlayer);
	pPlayer->m_IsBlockTourningDead = true;
	pPlayer->m_IsBlockTourningInArena = false;
	int WonId = CountAlive();

	//update skill levels
	if(pPlayer->GetCid() == Killer) //selfkill
	{
		GameServer()->UpdateBlockSkill(-40, Killer);
	}
	else
	{
		int DeadSkill = pPlayer->m_Account.m_BlockSkill;
		int KillSkill = GameServer()->m_apPlayers[Killer]->m_Account.m_BlockSkill;
		int SkillDiff = abs(DeadSkill - KillSkill);
		if(SkillDiff < 1500) //pretty same skill lvl
		{
			if(DeadSkill < KillSkill) //the killer is better
			{
				GameServer()->UpdateBlockSkill(-29, pPlayer->GetCid()); //killed
				GameServer()->UpdateBlockSkill(+30, Killer); //killer
			}
			else //the killer is worse
			{
				GameServer()->UpdateBlockSkill(-40, pPlayer->GetCid()); //killed
				GameServer()->UpdateBlockSkill(+40, Killer); //killer
			}
		}
		else //unbalanced skill lvl --> punish harder and reward nicer
		{
			if(DeadSkill < KillSkill) //the killer is better
			{
				GameServer()->UpdateBlockSkill(-19, pPlayer->GetCid()); //killed
				GameServer()->UpdateBlockSkill(+20, Killer); //killer
			}
			else //the killer is worse
			{
				GameServer()->UpdateBlockSkill(-60, pPlayer->GetCid()); //killed
				GameServer()->UpdateBlockSkill(+60, Killer); //killer
			}
		}
	}

	if(WonId == -404)
	{
		str_format(aBuf, sizeof(aBuf), "[BLOCK] error %d", WonId);
		GameServer()->SendChat(-1, TEAM_ALL, aBuf);
		m_State = STATE_OFF;
	}
	else if(WonId < 0)
	{
		if(WonId == -420)
			WonId = 0;
		WonId *= -1;
		str_format(aBuf, sizeof(aBuf), "[BLOCK] '%s' won the tournament (%d players).", Server()->ClientName(WonId), m_StartPlayers);
		GameServer()->SendChat(-1, TEAM_ALL, aBuf);
		m_State = STATE_ENDING; //set end state

		//give price to the winner
		int XpRew;
		int PointsRew;
		int MoneyRew;
		int SkillRew;
		if(m_StartPlayers <= 5) //depending on how many tees participated
		{
			XpRew = 200;
			PointsRew = 3;
			MoneyRew = 100;
			SkillRew = 10;
		}
		else if(m_StartPlayers <= 10)
		{
			XpRew = 500;
			PointsRew = 5;
			MoneyRew = 500;
			SkillRew = 20;
		}
		else if(m_StartPlayers <= 15)
		{
			XpRew = 3000;
			PointsRew = 10;
			MoneyRew = 1000;
			SkillRew = 30;
		}
		else if(m_StartPlayers <= 32)
		{
			XpRew = 5000;
			PointsRew = 25;
			MoneyRew = 2000;
			SkillRew = 120;
		}
		else if(m_StartPlayers <= 44)
		{
			XpRew = 20000;
			PointsRew = 30;
			MoneyRew = 10000;
			SkillRew = 400;
		}
		else
		{
			XpRew = 25000;
			PointsRew = 100;
			MoneyRew = 50000;
			SkillRew = 900;
		}

		str_format(aBuf, sizeof(aBuf), "[BLOCK] +%d xp", XpRew);
		GameServer()->SendChatTarget(WonId, aBuf);
		str_format(aBuf, sizeof(aBuf), "[BLOCK] +%d money", MoneyRew);
		GameServer()->SendChatTarget(WonId, aBuf);
		str_format(aBuf, sizeof(aBuf), "[BLOCK] +%d points", PointsRew);
		GameServer()->SendChatTarget(WonId, aBuf);

		GameServer()->m_apPlayers[WonId]->MoneyTransaction(+MoneyRew, "block tournament");
		GameServer()->m_apPlayers[WonId]->GiveXP(XpRew);
		GameServer()->m_apPlayers[WonId]->GiveBlockPoints(PointsRew);
		GameServer()->UpdateBlockSkill(+SkillRew, WonId);
	}
	else if(WonId == 0)
	{
		GameServer()->SendChat(-1, TEAM_ALL, "[BLOCK] nobody won the tournament");
		m_State = STATE_OFF;
	}
	else if(WonId > 1)
	{
		str_format(aBuf, sizeof(aBuf), "[BLOCK] you died and placed as rank %d in the tournament", WonId + 1);
		GameServer()->SendChatTarget(pPlayer->GetCid(), aBuf);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "[BLOCK] error %d", WonId);
		GameServer()->SendChat(-1, TEAM_ALL, aBuf);
		m_State = STATE_OFF;
	}
}
