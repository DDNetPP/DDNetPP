﻿// gamecontext scoped balance ddnet++ methods

#include <engine/shared/config.h>
#include <game/server/gamecontroller.h>
#include <game/server/teams.h>

#include <cinttypes>
#include <cstring>

#include "../gamecontext.h"

#include "block_tournament.h"

bool CBlockTournament::IsActive(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
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
	pPlayer->m_IsBlockTourningInArena = false;
	m_aRestorePos[pPlayer->GetCID()] = true;
}

void CBlockTournament::Join(CPlayer *pPlayer)
{
	if(!pPlayer)
		return;

	pPlayer->m_IsBlockTourning = true;
	pPlayer->m_IsBlockTourningDead = false;
	pPlayer->m_IsBlockTourningInArena = false;
}

bool CBlockTournament::AllowSelfKill(int ClientID)
{
	if(ClientID < 0 || ClientID > MAX_CLIENTS)
		return true;
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return true;
	if(!IsActive(ClientID))
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
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, "[EVENT] Block Tournament уже начат.");
		return;
	}
	if(g_Config.m_SvAllowBlockTourna == 0)
	{
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, "[EVENT] Block Tournament выключен.");
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

	int Id = pPlayer->GetCID();
	vec2 Pos = GetNextArenaSpawn(Id);
	if(Pos == vec2(-1, -1)) // fallback to ddr spawn if there is no arena
		return false;

	*pPos = Pos;
	return true;
}

void CBlockTournament::PostSpawn(CCharacter *pChr, vec2 Pos)
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

vec2 CBlockTournament::GetNextArenaSpawn(int ClientID)
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
		SendChatAll("[EVENT] Ошибка: не найдена арена.");
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
	if(!IsActive(pPlayer->GetCID()))
		return;

	if(!pChr->m_FreezeTime)
	{
		pChr->m_BlockTournaDeadTicks = 0;
		return;
	}

	pChr->m_BlockTournaDeadTicks++;
	if(pChr->m_BlockTournaDeadTicks > 15 * Server()->TickSpeed())
		pChr->Die(pPlayer->GetCID(), WEAPON_SELF);
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
			Player->GetCharacter()->Die(Player->GetCID(), WEAPON_GAME);
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
						Player->GetCharacter()->Die(Player->GetCID(), WEAPON_GAME);
					}
				}
			}
			GameServer()->SendChat(-1, CGameContext::CHAT_ALL, "[EVENT] Время вышло!");
			m_State = STATE_OFF;
		}
	}
	else if(m_State == STATE_LOBBY)
	{
		m_LobbyTick--;
		if(m_LobbyTick % Server()->TickSpeed() == 0)
		{
			int blockers = CountAlive();
			if(blockers < 0)
			{
				blockers = 1;
			}
			str_format(aBuf,
				sizeof(aBuf),
				"Block Tournament через %d секунд\n"
				"[Участников: %d] /join чтобы войти",
				m_LobbyTick / Server()->TickSpeed(),
				blockers,
				g_Config.m_SvBlockTournaPlayers);
			GameServer()->SendBroadcastAll(aBuf, 2);
		}
		
		if(m_LobbyTick < 0)
		{
			m_StartPlayers = CountAlive();
			if(m_StartPlayers < g_Config.m_SvBlockTournaPlayers) //minimum x players needed to start a tourna
			{
				GameServer()->SendBroadcastAll("[EVENT] Ошибка! Недостаточно игроков.", 2);
				EndRound();
				return;
			}

			GameServer()->SendBroadcastAll("[EVENT] Приготовтесь!", 2);
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
						Player->GetCharacter()->Die(Player->GetCID(), WEAPON_GAME);
					}
		}
	}
	else if(State() == STATE_COOLDOWN)
	{
		m_CoolDown--;
		if(m_CoolDown % Server()->TickSpeed() == 0)
		{
			str_format(aBuf, sizeof(aBuf), "Начало через %d", m_CoolDown / Server()->TickSpeed());
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
		if(!IsActive(Player->GetCID()))
			continue;

		Leave(Player);
		if(Player->GetCharacter() && !Player->m_IsBlockTourningDead && Player->m_IsBlockTourningInArena)
			Player->GetCharacter()->Die(Player->GetCID(), WEAPON_GAME);
		else if(Player->GetCharacter())
			m_aRestorePos[Player->GetCID()] = false;
	}
}

int CBlockTournament::CountAlive()
{
	int c = 0;
	int id = -404;

	for(auto &Player : GameServer()->m_apPlayers)
	{
		if(Player)
		{
			if(Player->m_IsBlockTourning)
			{
				c++;
				id = Player->GetCID();
			}
		}
	}

	if(c == 1) //one alive? --> return his id negative
	{
		if(id == 0)
		{
			return -420;
		}
		else
		{
			return id * -1;
		}
	}

	return c;
}

void CBlockTournament::OnDeath(CCharacter *pChr, int Killer)
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
	int wonID = CountAlive();

	//update skill levels
	if(pPlayer->GetCID() == Killer) //selfkill
	{
		GameServer()->UpdateBlockSkill(-40, Killer);
	}
	else
	{
		int deadskill = pPlayer->m_Account.m_BlockSkill;
		int killskill = GameServer()->m_apPlayers[Killer]->m_Account.m_BlockSkill;
		int skilldiff = abs(deadskill - killskill);
		if(skilldiff < 1500) //pretty same skill lvl
		{
			if(deadskill < killskill) //the killer is better
			{
				GameServer()->UpdateBlockSkill(-29, pPlayer->GetCID()); //killed
				GameServer()->UpdateBlockSkill(+30, Killer); //killer
			}
			else //the killer is worse
			{
				GameServer()->UpdateBlockSkill(-40, pPlayer->GetCID()); //killed
				GameServer()->UpdateBlockSkill(+40, Killer); //killer
			}
		}
		else //unbalanced skill lvl --> punish harder and reward nicer
		{
			if(deadskill < killskill) //the killer is better
			{
				GameServer()->UpdateBlockSkill(-19, pPlayer->GetCID()); //killed
				GameServer()->UpdateBlockSkill(+20, Killer); //killer
			}
			else //the killer is worse
			{
				GameServer()->UpdateBlockSkill(-60, pPlayer->GetCID()); //killed
				GameServer()->UpdateBlockSkill(+60, Killer); //killer
			}
		}
	}

	if(wonID == -404)
	{
		str_format(aBuf, sizeof(aBuf), "[BT] Ошибка: %d", wonID);
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		m_State = STATE_OFF;
	}
	else if(wonID < 0)
	{
		if(wonID == -420)
			wonID = 0;
		wonID *= -1;
		str_format(aBuf, sizeof(aBuf), "[BT] '%s' выиграл.", Server()->ClientName(wonID), m_StartPlayers);
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		m_State = STATE_ENDING; //set end state

		//give price to the winner
		int xp_rew;
		int points_rew;
		int money_rew;
		int skill_rew;
		if(m_StartPlayers <= 5) //depending on how many tees participated
		{
			xp_rew = 200;
			points_rew = 3;
			money_rew = 100;
			skill_rew = 5;
		}
		else if(m_StartPlayers <= 10)
		{
			xp_rew = 500;
			points_rew = 5;
			money_rew = 500;
			skill_rew = 10;
		}
		else if(m_StartPlayers <= 15)
		{
			xp_rew = 1000;
			points_rew = 10;
			money_rew = 1000;
			skill_rew = 15;
		}
		else if(m_StartPlayers <= 32)
		{
			xp_rew = 2000;
			points_rew = 25;
			money_rew = 1500;
			skill_rew = 30;
		}
		else if(m_StartPlayers <= 44)
		{
			xp_rew = 3000;
			points_rew = 30;
			money_rew = 3000;
			skill_rew = 50;
		}
		else
		{
			xp_rew = 5000;
			points_rew = 100;
			money_rew = 4999;
			skill_rew = 80;
		}

		str_format(aBuf, sizeof(aBuf), "[BT] +%d XP", xp_rew);
		GameServer()->SendChatTarget(wonID, aBuf);
		str_format(aBuf, sizeof(aBuf), "[BT] +%d Баблишка", money_rew);
		GameServer()->SendChatTarget(wonID, aBuf);
		str_format(aBuf, sizeof(aBuf), "[BT] +%d Поинтов", points_rew);
		GameServer()->SendChatTarget(wonID, aBuf);

		GameServer()->m_apPlayers[wonID]->MoneyTransaction(+money_rew, "Block Tournament");
		GameServer()->m_apPlayers[wonID]->GiveXP(xp_rew);
		GameServer()->m_apPlayers[wonID]->GiveBlockPoints(points_rew);
		GameServer()->m_apPlayers[wonID]->m_IsBlockTourningInArena = false;
		GameServer()->UpdateBlockSkill(+skill_rew, wonID);
	}
	else if(wonID == 0)
	{
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, "[BT] nobody won the tournament");
		m_State = STATE_OFF;
	}
	else if(wonID > 1)
	{
		str_format(aBuf, sizeof(aBuf), "[BT] Ты сдох и занял %d место.", wonID + 1);
		GameServer()->SendChatTarget(pPlayer->GetCID(), aBuf);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "[BT] Ошибка: %d", wonID);
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		m_State = STATE_OFF;
	}
}
