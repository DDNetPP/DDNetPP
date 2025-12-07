// gamecontext scoped balance ddnet++ methods

#include "balance.h"

#include "../gamecontext.h"

#include <engine/shared/config.h>

#include <game/mapitems.h>
#include <game/server/teams.h>

#include <cstring>

bool CBalance::IsActive(int ClientId)
{
	return ClientId == GameServer()->m_BalanceId1 || ClientId == GameServer()->m_BalanceId2;
}

bool CBalance::HandleCharacterTiles(CCharacter *pChr, int Index)
{
	CPlayer *pPlayer = pChr->GetPlayer();

	int TileIndex = GameServer()->Collision()->GetTileIndex(Index);
	int TileFIndex = GameServer()->Collision()->GetFrontTileIndex(Index);
	// freeze
	if((TileIndex == TILE_FREEZE) || (TileFIndex == TILE_FREEZE))
	{
		if((pPlayer->GetCid() == GameServer()->m_BalanceId1 || pPlayer->GetCid() == GameServer()->m_BalanceId2) && GameServer()->m_BalanceBattleState == 2)
		{
			pChr->Die(pPlayer->GetCid(), WEAPON_SELF);
			return true;
		}
	}
	return false;
}

void CGameContext::StopBalanceBattle()
{
	for(auto &Player : m_apPlayers)
	{
		if(Player)
		{
			if(Player->m_BalanceBattle_id != -1)
			{
				Player->m_BalanceBattle_id = -1;
			}
			if(Player->m_IsBalanceBattleDummy)
			{
				Server()->BotLeave(Player->GetCid(), true);
			}
		}
	}
	m_BalanceId1 = -1;
	m_BalanceId2 = -1;
	m_BalanceBattleState = 0; //set offline
}

void CGameContext::StartBalanceBattle(int Id1, int Id2)
{
	if(m_apPlayers[Id1] && !m_apPlayers[Id2])
	{
		SendChatTarget(Id1, "[balance] can't start a battle because your mate left.");
	}
	else if(!m_apPlayers[Id1] && m_apPlayers[Id2])
	{
		SendChatTarget(Id2, "[balance] can't start a battle because your mate left.");
	}
	else if(m_BalanceBattleState)
	{
		SendChatTarget(Id1, "[balance] can't start a battle because arena is full.");
		SendChatTarget(Id2, "[balance] can't start a battle because arena is full.");
	}
	else if(m_apPlayers[Id1] && m_apPlayers[Id2])
	{
		//moved to tick func
		//m_apPlayers[Id1]->m_IsBalanceBatteling = true;
		//m_apPlayers[Id2]->m_IsBalanceBatteling = true;
		//m_apPlayers[Id1]->m_IsBalanceBattlePlayer1 = true;
		//m_apPlayers[Id2]->m_IsBalanceBattlePlayer1 = false;
		//SendChatTarget(Id1, "[balance] BATTLE STARTED!");
		//SendChatTarget(Id2, "[balance] BATTLE STARTED!");
		//m_apPlayers[Id1]->GetCharacter()->Die(Id1, WEAPON_SELF);
		//m_apPlayers[Id2]->GetCharacter()->Die(Id2, WEAPON_SELF);

		m_BalanceDummyId1 = CreateNewDummy(DUMMYMODE_MINIGAME_BALANCE1, true);
		m_BalanceDummyId2 = CreateNewDummy(DUMMYMODE_MINIGAME_BALANCE2, true);
		m_BalanceId1 = Id1;
		m_BalanceId2 = Id2;
		m_BalanceBattleCountdown = Server()->TickSpeed() * 10;
		m_BalanceBattleState = 1; //set state to preparing
	}
}

void CGameContext::BalanceBattleTick()
{
	char aBuf[128];

	if(m_BalanceBattleState == 1) //preparing
	{
		m_BalanceBattleCountdown--;
		if(m_BalanceBattleCountdown % Server()->TickSpeed() == 0)
		{
			str_format(aBuf, sizeof(aBuf), "[balance] battle starts in %d seconds", m_BalanceBattleCountdown / Server()->TickSpeed());
			SendBroadcast(aBuf, m_BalanceId1);
			SendBroadcast(aBuf, m_BalanceId2);
		}
		if(!m_BalanceBattleCountdown)
		{
			//move the dummys
			if(m_apPlayers[m_BalanceDummyId1] && m_apPlayers[m_BalanceDummyId1]->GetCharacter())
			{
				m_apPlayers[m_BalanceDummyId1]->GetCharacter()->MoveTee(-4, -2);
			}
			if(m_apPlayers[m_BalanceDummyId2] && m_apPlayers[m_BalanceDummyId2]->GetCharacter())
			{
				m_apPlayers[m_BalanceDummyId2]->GetCharacter()->MoveTee(-4, -2);
			}

			if(m_apPlayers[m_BalanceId1] && m_apPlayers[m_BalanceId2]) //both on server
			{
				m_apPlayers[m_BalanceId1]->m_IsBalanceBatteling = true;
				m_apPlayers[m_BalanceId2]->m_IsBalanceBatteling = true;
				m_apPlayers[m_BalanceId1]->m_IsBalanceBattlePlayer1 = true;
				m_apPlayers[m_BalanceId2]->m_IsBalanceBattlePlayer1 = false;
				SendChatTarget(m_BalanceId1, "[balance] BATTLE STARTED!");
				SendChatTarget(m_BalanceId2, "[balance] BATTLE STARTED!");
				m_apPlayers[m_BalanceId1]->GetCharacter()->Die(m_BalanceId1, WEAPON_SELF);
				m_apPlayers[m_BalanceId2]->GetCharacter()->Die(m_BalanceId2, WEAPON_SELF);
				SendBroadcast("[balance] BATTLE STARTED", m_BalanceId1);
				SendBroadcast("[balance] BATTLE STARTED", m_BalanceId2);
				m_BalanceBattleState = 2; //set ingame
			}
			else if(m_apPlayers[m_BalanceId1])
			{
				SendBroadcast("[balance] BATTLE STOPPED (because mate left)", m_BalanceId1);
				StopBalanceBattle();
			}
			else if(m_apPlayers[m_BalanceId2])
			{
				SendBroadcast("[balance] BATTLE STOPPED (because mate left)", m_BalanceId2);
				StopBalanceBattle();
			}
			else
			{
				StopBalanceBattle();
			}
		}
	}
	//else if (m_BalanceBattleState == 2) //ingame //moved to die(); because it is less resource to avoid it in tick functions
	//{
	//	if (m_apPlayers[m_BalanceId1] && m_apPlayers[m_BalanceId2])
	//	{
	//		if (!m_apPlayers[m_BalanceId1]->GetCharacter())
	//		{
	//			SendChatTarget(m_BalanceId1, "[balance] you lost!");
	//			SendChatTarget(m_BalanceId2, "[balance] you won!");
	//		}
	//	}
	//	else if (!m_apPlayers[m_BalanceId1] && !m_apPlayers[m_BalanceId2]) //all lef --> close game
	//	{
	//		m_BalanceBattleState = 0;
	//	}
	//}
}
