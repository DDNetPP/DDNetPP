// gamecontext scoped blockwave ddnet++ methods

#include <engine/shared/config.h>
#include <game/mapitems.h>
#include <game/server/teams.h>
#include <generated/protocol.h>

#include <cinttypes>
#include <cstring>

#include "../gamecontext.h"

#include "blockwave.h"

bool CBlockwave::IsActive(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	return pPlayer->m_IsBlockWaving;
}

void CGameContext::BlockWaveAddBots()
{
	int OccSlots = CountConnectedPlayers();
	int FreeSlots = MAX_CLIENTS - OccSlots;

	if(m_BlockWaveRound < 15 + 1) //max 15 bots
	{
		for(int i = 1; i < m_BlockWaveRound + 1; i++)
		{
			CreateNewDummy(DUMMYMODE_BLOCKWAVE, true);
			if(i > FreeSlots - 5) //always leave 5 slots free for people to join
			{
				dbg_msg("BlockWave", "Stopped connecting at %d/%d bots because server has only %d free slots", i, m_BlockWaveRound + 1, FreeSlots);
				break;
			}
		}
	}
	else
	{
		for(int i = 1; i < 15 + 1; i++)
		{
			CreateNewDummy(DUMMYMODE_BLOCKWAVE, true);
			if(i > FreeSlots - 5) //always leave 5 slots free for people to join
			{
				dbg_msg("BlockWave", "Stopped connecting at %d/15 + 1 bots because server has only %d free slots", i, FreeSlots);
				break;
			}
		}
	}
}

void CGameContext::BlockWaveRemoveBots()
{
	for(auto &Player : m_apPlayers)
	{
		if(Player)
		{
			if(Player->m_IsBlockWaving && Player->m_IsDummy) //disconnect dummys
			{
				Server()->BotLeave(Player->GetCid(), true);
			}
		}
	}
}

void CGameContext::BlockWaveWonRound()
{
	m_BlockWaveRound++;
	SendChatBlockWave("[BlockWave] round survived.");
	m_BlockWaveGameState = 1;

	//respawn all dead humans
	vec2 BlockWaveSpawnTile = Collision()->GetRandomTile(TILE_BLOCKWAVE_HUMAN);

	if(BlockWaveSpawnTile != vec2(-1, -1))
	{
		for(auto &Player : m_apPlayers)
		{
			if(Player && Player->m_IsBlockWaving)
			{
				if((!Player->m_IsDummy && Player->GetCharacter()) && (Player->GetCharacter()->m_FreezeTime || Player->m_IsBlockWaveWaiting)) //queue dudes waiting to join on new round or frozen ingames
				{
					Player->GetCharacter()->SetPosition(BlockWaveSpawnTile);
				}
				if(!Player->GetCharacter() || Player->m_IsBlockWaveWaiting) //if some queue dude is dead while waiting to join set him unqueue --> so on respawn he will enter the area
				{
					Player->m_IsBlockWaveWaiting = false;
				}
			}
		}
	}
	else //no BlockWaveSpawnTile
	{
		//GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[BlockWave] No arena set.");
		m_BlockWaveGameState = 0;
	}

	for(auto &Player : m_apPlayers)
	{
		if(Player)
		{
			Player->m_IsBlockWaveDead = false; //noboy is dead on new round
		}
	}
	BlockWaveRemoveBots();
}

void CGameContext::StartBlockWaveGame()
{
#if defined(CONF_DEBUG)
	dbg_msg("Blockwave", "Game started.");
#endif
	if(m_BlockWaveGameState)
	{
		return;
	} //no resatrt only start if not started yet
	if(!CountBlockWavePlayers())
	{
		StopBlockWaveGame();
		return;
	}
	m_BlockWaveGameState = 1;
	m_BlockWaveRound = 1; //reset rounds
	m_BlockWavePrepareDelay = (10 * Server()->TickSpeed());
	for(auto &Player : m_apPlayers)
	{
		if(Player)
		{
			Player->m_IsBlockWaveDead = false;
		}
	}
}

void CGameContext::StopBlockWaveGame()
{
	SendChat(-1, TEAM_ALL, "[BlockWave] game stopped.");
	BlockWaveRemoveBots();
	m_BlockWaveGameState = 0;
}

void CGameContext::BlockWaveGameTick()
{
	if(!m_BlockWaveGameState)
		return;
	if(!CountBlockWavePlayers())
	{
		StopBlockWaveGame();
		return;
	}

	char aBuf[256];

	if(m_BlockWaveGameState == 1)
	{
		m_BlockWavePrepareDelay--;
		if(m_BlockWavePrepareDelay % Server()->TickSpeed() == 0)
		{
			str_format(aBuf, sizeof(aBuf), "[BlockWave] round %d starts in %d seconds", m_BlockWaveRound, m_BlockWavePrepareDelay / Server()->TickSpeed());
			SendBroadcastBlockWave(aBuf);
		}
		if(m_BlockWavePrepareDelay < 0)
		{
			SendBroadcastBlockWave("[BlockWave] Have fun and good luck!");
			m_BlockWaveGameState = 2; //start round!
			m_BlockWavePrepareDelay = (10 * Server()->TickSpeed()); //could add a cfg var in secs instead of 10 here
			BlockWaveAddBots();
		}
	}
	else //running round
	{
		//check for rip round or win round
		if(Server()->Tick() % 60 == 0)
		{
			bool RipAll = true;
			bool Won = true;
			for(auto &Player : m_apPlayers)
			{
				if(Player && Player->m_IsBlockWaving && !Player->m_IsBlockWaveDead && !Player->m_IsDummy)
				{
					RipAll = false;
					break;
				}
			}
			for(auto &Player : m_apPlayers)
			{
				if(Player && Player->m_IsBlockWaving && !Player->m_IsBlockWaveDead && Player->m_IsDummy)
				{
					Won = false;
					break;
				}
			}
			if(RipAll)
			{
				BlockWaveStartNewGame();
			}
			if(Won)
			{
				BlockWaveWonRound();
			}
		}
	}
}

void CGameContext::BlockWaveEndGame()
{
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "[BlockWave] You lost! Survived %d rounds.", m_BlockWaveRound);
	SendChatBlockWave(aBuf);
}

void CGameContext::BlockWaveStartNewGame()
{
	BlockWaveEndGame(); //send message to all players
	m_BlockWaveGameState = 0; //end old game
	StartBlockWaveGame(); //start new game
	for(auto &Player : m_apPlayers)
	{
		if(Player && Player->m_IsBlockWaving && Player->GetCharacter())
		{
			Player->GetCharacter()->Die(Player->GetCid(), WEAPON_GAME);
			if(Player->m_IsDummy)
			{
				Server()->BotLeave(Player->GetCid(), true);
			}
		}
	}
}

int CGameContext::CountBlockWavePlayers()
{
	int c = 0;
	for(auto &Player : m_apPlayers)
	{
		if(Player && Player->m_IsBlockWaving)
		{
			c++;
		}
	}
	return c;
}

void CGameContext::SendBroadcastBlockWave(const char *pMsg)
{
	for(auto &Player : m_apPlayers)
	{
		if(Player && Player->m_IsBlockWaving)
		{
			SendBroadcast(pMsg, Player->GetCid());
		}
	}
}

void CGameContext::SendChatBlockWave(const char *pMsg)
{
	for(auto &Player : m_apPlayers)
	{
		if(Player && Player->m_IsBlockWaving)
		{
			SendChatTarget(Player->GetCid(), pMsg);
		}
	}
}
