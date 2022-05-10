// gamecontext scoped balance ddnet++ methods

#include <engine/shared/config.h>
#include <game/server/teams.h>

#include <cinttypes>
#include <cstring>

#include "../gamecontext.h"

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
				Server()->BotLeave(Player->GetCID(), true);
			}
		}
	}
	m_BalanceID1 = -1;
	m_BalanceID2 = -1;
	m_BalanceBattleState = 0; //set offline
}

void CGameContext::StartBalanceBattle(int ID1, int ID2)
{
	if(m_apPlayers[ID1] && !m_apPlayers[ID2])
	{
		SendChatTarget(ID1, "[balance] can't start a battle because your mate left.");
	}
	else if(!m_apPlayers[ID1] && m_apPlayers[ID2])
	{
		SendChatTarget(ID2, "[balance] can't start a battle because your mate left.");
	}
	else if(m_BalanceBattleState)
	{
		SendChatTarget(ID1, "[balance] can't start a battle because arena is full.");
		SendChatTarget(ID2, "[balance] can't start a battle because arena is full.");
	}
	else if(m_apPlayers[ID1] && m_apPlayers[ID2])
	{
		//moved to tick func
		//m_apPlayers[ID1]->m_IsBalanceBatteling = true;
		//m_apPlayers[ID2]->m_IsBalanceBatteling = true;
		//m_apPlayers[ID1]->m_IsBalanceBattlePlayer1 = true;
		//m_apPlayers[ID2]->m_IsBalanceBattlePlayer1 = false;
		//SendChatTarget(ID1, "[balance] BATTLE STARTED!");
		//SendChatTarget(ID2, "[balance] BATTLE STARTED!");
		//m_apPlayers[ID1]->GetCharacter()->Die(ID1, WEAPON_SELF);
		//m_apPlayers[ID2]->GetCharacter()->Die(ID2, WEAPON_SELF);

		m_BalanceDummyID1 = CreateNewDummy(-1, true);
		m_BalanceDummyID2 = CreateNewDummy(-2, true);
		m_BalanceID1 = ID1;
		m_BalanceID2 = ID2;
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
			SendBroadcast(aBuf, m_BalanceID1);
			SendBroadcast(aBuf, m_BalanceID2);
		}
		if(!m_BalanceBattleCountdown)
		{
			//move the dummys
			if(m_apPlayers[m_BalanceDummyID1] && m_apPlayers[m_BalanceDummyID1]->GetCharacter())
			{
				m_apPlayers[m_BalanceDummyID1]->GetCharacter()->MoveTee(-4, -2);
			}
			if(m_apPlayers[m_BalanceDummyID2] && m_apPlayers[m_BalanceDummyID2]->GetCharacter())
			{
				m_apPlayers[m_BalanceDummyID2]->GetCharacter()->MoveTee(-4, -2);
			}

			if(m_apPlayers[m_BalanceID1] && m_apPlayers[m_BalanceID2]) //both on server
			{
				m_apPlayers[m_BalanceID1]->m_IsBalanceBatteling = true;
				m_apPlayers[m_BalanceID2]->m_IsBalanceBatteling = true;
				m_apPlayers[m_BalanceID1]->m_IsBalanceBattlePlayer1 = true;
				m_apPlayers[m_BalanceID2]->m_IsBalanceBattlePlayer1 = false;
				SendChatTarget(m_BalanceID1, "[balance] BATTLE STARTED!");
				SendChatTarget(m_BalanceID2, "[balance] BATTLE STARTED!");
				m_apPlayers[m_BalanceID1]->GetCharacter()->Die(m_BalanceID1, WEAPON_SELF);
				m_apPlayers[m_BalanceID2]->GetCharacter()->Die(m_BalanceID2, WEAPON_SELF);
				SendBroadcast("[balance] BATTLE STARTED", m_BalanceID1);
				SendBroadcast("[balance] BATTLE STARTED", m_BalanceID2);
				m_BalanceBattleState = 2; //set ingame
			}
			else if(m_apPlayers[m_BalanceID1])
			{
				SendBroadcast("[balance] BATTLE STOPPED (because mate left)", m_BalanceID1);
				StopBalanceBattle();
			}
			else if(m_apPlayers[m_BalanceID2])
			{
				SendBroadcast("[balance] BATTLE STOPPED (because mate left)", m_BalanceID2);
				StopBalanceBattle();
			}
			else
			{
				StopBalanceBattle();
			}
		}
	}
	//else if (m_BalanceBattleState == 2) //ingame //moved to die(); because it is less ressource to avoid it in tick functions
	//{
	//	if (m_apPlayers[m_BalanceID1] && m_apPlayers[m_BalanceID2])
	//	{
	//		if (!m_apPlayers[m_BalanceID1]->GetCharacter())
	//		{
	//			SendChatTarget(m_BalanceID1, "[balance] you lost!");
	//			SendChatTarget(m_BalanceID2, "[balance] you won!");
	//		}
	//	}
	//	else if (!m_apPlayers[m_BalanceID1] && !m_apPlayers[m_BalanceID2]) //all lef --> close game
	//	{
	//		m_BalanceBattleState = 0;
	//	}
	//}
}
