// gamecontext scoped balance ddnet++ methods

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
}

bool CBlockTournament::PickSpawn(vec2 *pPos, CPlayer *pPlayer)
{
	if(!pPlayer->m_IsBlockTourning)
		return false;
	if(pPlayer->m_IsBlockTourningDead)
		return false;
	if(GameServer()->m_pBlockTournament->m_State != CGameContext::BLOCKTOURNA_IN_GAME)
		return false;

	int Id = pPlayer->GetCID();
	vec2 Pos = GameServer()->GetNextBlockTournaSpawn(Id);
	if(*pPos == vec2(-1, -1)) // fallback to ddr spawn if there is no arena
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
	if(GameServer()->m_pBlockTournament->m_State != CGameContext::BLOCKTOURNA_IN_GAME)
		return;

	pChr->Freeze(6);
	pPlayer->m_IsBlockTourningInArena = true;
}

vec2 CGameContext::GetNextBlockTournaSpawn(int ClientID)
{
	vec2 Spawn = Collision()->GetBlockTournamentSpawn(m_pBlockTournament->m_SpawnCounter++);
	if(Spawn == vec2(-1, -1))
	{
		SendChatTarget(ClientID, "[EVENT] No block tournament arena found.");
		EndBlockTourna();
	}
	return Spawn;
}

void CBlockTournament::Tick()
{
	if(!m_State)
		return;

	char aBuf[128];

	if(m_State == CGameContext::BLOCKTOURNA_IN_GAME) //ingame
	{
		m_Tick++;
		if(m_Tick > g_Config.m_SvBlockTournaGameTime * Server()->TickSpeed() * 60) //time over --> draw
		{
			//kill all tournas
			for(auto &Player : GameServer()->m_apPlayers)
			{
				if(Player && Player->m_IsBlockTourning)
				{
					Player->m_IsBlockTourning = false;
					if(Player->GetCharacter())
					{
						Player->GetCharacter()->Die(Player->GetCID(), WEAPON_GAME);
					}
				}
			}
			GameServer()->SendChat(-1, CGameContext::CHAT_ALL, "[EVENT] Block tournament stopped because time was over.");
			m_State = 0;
		}
	}
	else if(m_State == 1)
	{
		m_LobbyTick--;
		if(m_LobbyTick % Server()->TickSpeed() == 0)
		{
			int blockers = GameServer()->CountBlockTournaAlive();
			if(blockers < 0)
			{
				blockers = 1;
			}
			str_format(aBuf, sizeof(aBuf), "[EVENT] BLOCK IN %d SECONDS\n[%d/%d] '/join'ed already", m_LobbyTick / Server()->TickSpeed(), blockers, g_Config.m_SvBlockTournaPlayers);
			GameServer()->SendBroadcastAll(aBuf, 2);
		}

		if(m_LobbyTick < 0)
		{
			GameServer()->m_BlockTournaStartPlayers = GameServer()->CountBlockTournaAlive();
			if(GameServer()->m_BlockTournaStartPlayers < g_Config.m_SvBlockTournaPlayers) //minimum x players needed to start a tourna
			{
				GameServer()->SendBroadcastAll("[EVENT] Block tournament failed! Not enough players.", 2);
				GameServer()->EndBlockTourna();
				return;
			}

			GameServer()->SendBroadcastAll("[EVENT] Block tournament started!", 2);
			m_State = 2;
			m_Tick = 0;
			GameServer()->m_BlockTournaStart = time_get();

			//ready all players
			m_SpawnCounter = 0;
			for(auto &Player : GameServer()->m_apPlayers)
				if(Player && Player->m_IsBlockTourning)
					if(Player->GetCharacter()) // TODO: use CSaveTee to restore state after tournament
						Player->GetCharacter()->Die(Player->GetCID(), WEAPON_GAME);
		}
	}
}

void CGameContext::EndBlockTourna()
{
	m_pBlockTournament->m_State = 0;

	for(auto &Player : m_apPlayers)
		if(Player)
			Player->m_IsBlockTourning = false;
}

int CGameContext::CountBlockTournaAlive()
{
	int c = 0;
	int id = -404;

	for(auto &Player : m_apPlayers)
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
	if(m_State != CGameContext::BLOCKTOURNA_IN_GAME) //ingame
		return;

	char aBuf[128];
	//let him die and check for tourna win
	pPlayer->m_IsBlockTourning = false;
	pPlayer->m_IsBlockTourningDead = true;
	pPlayer->m_IsBlockTourningInArena = false;
	int wonID = GameServer()->CountBlockTournaAlive();

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
		str_format(aBuf, sizeof(aBuf), "[BLOCK] error %d", wonID);
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		m_State = 0;
	}
	else if(wonID < 0)
	{
		if(wonID == -420)
			wonID = 0;
		wonID *= -1;
		str_format(aBuf, sizeof(aBuf), "[BLOCK] '%s' won the tournament (%d players).", Server()->ClientName(wonID), GameServer()->m_BlockTournaStartPlayers);
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		m_State = 3; //set end state

		//give price to the winner
		int xp_rew;
		int points_rew;
		int money_rew;
		int skill_rew;
		if(GameServer()->m_BlockTournaStartPlayers <= 5) //depending on how many tees participated
		{
			xp_rew = 100;
			points_rew = 3;
			money_rew = 50;
			skill_rew = 10;
		}
		else if(GameServer()->m_BlockTournaStartPlayers <= 10)
		{
			xp_rew = 150;
			points_rew = 5;
			money_rew = 100;
			skill_rew = 20;
		}
		else if(GameServer()->m_BlockTournaStartPlayers <= 15)
		{
			xp_rew = 300;
			points_rew = 10;
			money_rew = 200;
			skill_rew = 30;
		}
		else if(GameServer()->m_BlockTournaStartPlayers <= 32)
		{
			xp_rew = 700;
			points_rew = 25;
			money_rew = 500;
			skill_rew = 120;
		}
		else if(GameServer()->m_BlockTournaStartPlayers <= 44)
		{
			xp_rew = 1200;
			points_rew = 30;
			money_rew = 1000;
			skill_rew = 400;
		}
		else
		{
			xp_rew = 25000;
			points_rew = 100;
			money_rew = 15000;
			skill_rew = 900;
		}

		str_format(aBuf, sizeof(aBuf), "[BLOCK] +%d xp", xp_rew);
		GameServer()->SendChatTarget(wonID, aBuf);
		str_format(aBuf, sizeof(aBuf), "[BLOCK] +%d money", money_rew);
		GameServer()->SendChatTarget(wonID, aBuf);
		str_format(aBuf, sizeof(aBuf), "[BLOCK] +%d points", points_rew);
		GameServer()->SendChatTarget(wonID, aBuf);

		GameServer()->m_apPlayers[wonID]->MoneyTransaction(+money_rew, "block tournament");
		GameServer()->m_apPlayers[wonID]->GiveXP(xp_rew);
		GameServer()->m_apPlayers[wonID]->GiveBlockPoints(points_rew);
		GameServer()->UpdateBlockSkill(+skill_rew, wonID);
	}
	else if(wonID == 0)
	{
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, "[BLOCK] nobody won the tournament");
		m_State = 0;
	}
	else if(wonID > 1)
	{
		str_format(aBuf, sizeof(aBuf), "[BLOCK] you died and placed as rank %d in the tournament", wonID + 1);
		GameServer()->SendChatTarget(pPlayer->GetCID(), aBuf);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "[BLOCK] error %d", wonID);
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		m_State = 0;
	}
}
