// gamecontext scoped balance ddnet++ methods

#include <engine/shared/config.h>
#include <game/server/teams.h>

#include <cinttypes>
#include <cstring>

#include "../gamecontext.h"

#include "block_tournament.h"

void CBlockTournament::Tick()
{
	// TODO: copy code in here
	m_pGameServer->BlockTournaTick();
}

vec2 CGameContext::GetNextBlockTournaSpawn(int ClientID)
{
	vec2 Spawn = Collision()->GetBlockTournamentSpawn(m_BlockTournaSpawnCounter++);
	if(Spawn == vec2(-1, -1))
	{
		SendChatTarget(ClientID, "[EVENT] No block tournament arena found.");
		EndBlockTourna();
	}
	return Spawn;
}

void CGameContext::BlockTournaTick()
{
	if(!m_BlockTournaState)
		return;

	char aBuf[128];

	if(m_BlockTournaState == BLOCKTOURNA_IN_GAME) //ingame
	{
		m_BlockTournaTick++;
		if(m_BlockTournaTick > g_Config.m_SvBlockTournaGameTime * Server()->TickSpeed() * 60) //time over --> draw
		{
			//kill all tournas
			for(auto &Player : m_apPlayers)
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
			SendChat(-1, CGameContext::CHAT_ALL, "[EVENT] Block tournament stopped because time was over.");
			m_BlockTournaState = 0;
		}
	}
	else if(m_BlockTournaState == 1)
	{
		m_BlockTournaLobbyTick--;
		if(m_BlockTournaLobbyTick % Server()->TickSpeed() == 0)
		{
			int blockers = CountBlockTournaAlive();
			if(blockers < 0)
			{
				blockers = 1;
			}
			str_format(aBuf, sizeof(aBuf), "[EVENT] BLOCK IN %d SECONDS\n[%d/%d] '/join'ed already", m_BlockTournaLobbyTick / Server()->TickSpeed(), blockers, g_Config.m_SvBlockTournaPlayers);
			SendBroadcastAll(aBuf, 2);
		}

		if(m_BlockTournaLobbyTick < 0)
		{
			m_BlockTournaStartPlayers = CountBlockTournaAlive();
			if(m_BlockTournaStartPlayers < g_Config.m_SvBlockTournaPlayers) //minimum x players needed to start a tourna
			{
				SendBroadcastAll("[EVENT] Block tournament failed! Not enough players.", 2);
				EndBlockTourna();
				return;
			}

			SendBroadcastAll("[EVENT] Block tournament started!", 2);
			m_BlockTournaState = 2;
			m_BlockTournaTick = 0;
			m_BlockTournaStart = time_get();

			//ready all players
			m_BlockTournaSpawnCounter = 0;
			for(auto &Player : m_apPlayers)
				if(Player && Player->m_IsBlockTourning)
					if(Player->GetCharacter()) // TODO: use CSaveTee to restore state after tournament 
						Player->GetCharacter()->Die(Player->GetCID(), WEAPON_GAME);
		}
	}
}

void CGameContext::EndBlockTourna()
{
	m_BlockTournaState = 0;

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


void CCharacter::BlockTourna_Die(int Killer)
{
	if(!m_pPlayer->m_IsBlockTourning)
		return;
	if(m_pPlayer->m_IsBlockTourningDead)
		return;
	if(!m_pPlayer->m_IsBlockTourningInArena)
		return;
	if(GameServer()->m_BlockTournaState != CGameContext::BLOCKTOURNA_IN_GAME) //ingame
		return;

	char aBuf[128];
	//let him die and check for tourna win
	m_pPlayer->m_IsBlockTourning = false;
	m_pPlayer->m_IsBlockTourningDead = true;
	m_pPlayer->m_IsBlockTourningInArena = false;
	int wonID = GameServer()->CountBlockTournaAlive();

	//update skill levels
	if(m_pPlayer->GetCID() == Killer) //selfkill
	{
		GameServer()->UpdateBlockSkill(-40, Killer);
	}
	else
	{
		int deadskill = m_pPlayer->m_Account.m_BlockSkill;
		int killskill = GameServer()->m_apPlayers[Killer]->m_Account.m_BlockSkill;
		int skilldiff = abs(deadskill - killskill);
		if(skilldiff < 1500) //pretty same skill lvl
		{
			if(deadskill < killskill) //the killer is better
			{
				GameServer()->UpdateBlockSkill(-29, m_pPlayer->GetCID()); //killed
				GameServer()->UpdateBlockSkill(+30, Killer); //killer
			}
			else //the killer is worse
			{
				GameServer()->UpdateBlockSkill(-40, m_pPlayer->GetCID()); //killed
				GameServer()->UpdateBlockSkill(+40, Killer); //killer
			}
		}
		else //unbalanced skill lvl --> punish harder and reward nicer
		{
			if(deadskill < killskill) //the killer is better
			{
				GameServer()->UpdateBlockSkill(-19, m_pPlayer->GetCID()); //killed
				GameServer()->UpdateBlockSkill(+20, Killer); //killer
			}
			else //the killer is worse
			{
				GameServer()->UpdateBlockSkill(-60, m_pPlayer->GetCID()); //killed
				GameServer()->UpdateBlockSkill(+60, Killer); //killer
			}
		}
	}

	if(wonID == -404)
	{
		str_format(aBuf, sizeof(aBuf), "[BLOCK] error %d", wonID);
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		GameServer()->m_BlockTournaState = 0;
	}
	else if(wonID < 0)
	{
		if(wonID == -420)
			wonID = 0;
		wonID *= -1;
		str_format(aBuf, sizeof(aBuf), "[BLOCK] '%s' won the tournament (%d players).", Server()->ClientName(wonID), GameServer()->m_BlockTournaStartPlayers);
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		GameServer()->m_BlockTournaState = 3; //set end state

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
		GameServer()->m_BlockTournaState = 0;
	}
	else if(wonID > 1)
	{
		str_format(aBuf, sizeof(aBuf), "[BLOCK] you died and placed as rank %d in the tournament", wonID + 1);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "[BLOCK] error %d", wonID);
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		GameServer()->m_BlockTournaState = 0;
	}
}
