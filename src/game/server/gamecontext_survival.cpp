// gamecontext scoped survival ddnet++ methods

#include <engine/shared/config.h>

#include "gamecontext.h"

void CGameContext::SurvivalLobbyTick()
{
	char aBuf[128];

	if(CountSurvivalPlayers() >= g_Config.m_SvSurvivalStartPlayers)
	{
		m_survivallobbycountdown--;
		if(m_survivallobbycountdown % Server()->TickSpeed() == 0 && m_survivallobbycountdown - 10 < Server()->TickSpeed() * 10) //only start to print last 10 seconds
		{
			if(!str_comp_nocase(m_aLastSurvivalWinnerName, ""))
			{
				str_format(aBuf, sizeof(aBuf), "survival game starts in %d seconds", m_survivallobbycountdown / Server()->TickSpeed());
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "Winner: %s\nsurvival game starts in %d seconds", m_aLastSurvivalWinnerName, m_survivallobbycountdown / Server()->TickSpeed());
			}
			SendSurvivalBroadcast(aBuf);

			if(m_survivallobbycountdown == (Server()->TickSpeed() * 9)) //teleport winner in lobby on last 10 sec countdown
			{
				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					if(m_apPlayers[i] && m_apPlayers[i]->GetCharacter() && m_apPlayers[i]->m_IsSurvivaling && m_apPlayers[i]->m_IsSurvivalWinner)
					{
						vec2 SurvivalLobbySpawnTile = Collision()->GetRandomTile(TILE_SURVIVAL_LOBBY);

						if(SurvivalLobbySpawnTile == vec2(-1, -1)) //no survival lobby
						{
							SendSurvivalChat("[SURVIVAL] no survival lobby set.");
						}
						else
						{
							m_apPlayers[i]->GetCharacter()->SetPosition(SurvivalLobbySpawnTile);
						}
					}
				}
			}
		}
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "[SURVIVAL] %d/%d players to start a game", CountSurvivalPlayers(), g_Config.m_SvSurvivalStartPlayers);
		if(Server()->Tick() % 30 == 0)
		{
			SendSurvivalBroadcast(aBuf);
		}
		m_survivallobbycountdown = Server()->TickSpeed() * g_Config.m_SvSurvivalLobbyDelay;
	}

	if(m_survivallobbycountdown < 1)
	{
		SurvivalStartGame();
	}
}

void CGameContext::SurvivalDeathmatchTick()
{
	char aBuf[256];
	m_survival_dm_countdown--;

	if(m_survival_dm_countdown % Server()->TickSpeed() == 0 && m_survival_dm_countdown - 10 < Server()->TickSpeed() * 10) //every second if seconds < 10
	{
		str_format(aBuf, sizeof(aBuf), "[SURVIVAL] deathmatch starts in %d seconds", m_survival_dm_countdown / Server()->TickSpeed());
		SendSurvivalBroadcast(aBuf);
	}
	else if(m_survival_dm_countdown % (Server()->TickSpeed() * 60) == 0 && m_survival_dm_countdown - 10 < (Server()->TickSpeed() * 60) * 5) //every minute if minutes < 5
	{
		str_format(aBuf, sizeof(aBuf), "[SURVIVAL] deathmatch starts in %d minutes", m_survival_dm_countdown / (Server()->TickSpeed() * 60));
		SendSurvivalChat(aBuf);
	}

	if(m_survival_dm_countdown < 1)
	{
		SurvivalSetGameState(SURVIVAL_DM);
	}
}

void CGameContext::SurvivalCheckWinnerAndDeathMatch()
{
	int AliveTees = CountSurvivalPlayers(true);
	char aBuf[128];
	if(AliveTees < 2) //could also be == 1 but i think < 2 is saver. Check for winning.                        (much wow sentence inc..) if 2 were alive and now only 1 players alive and one dies we have a winner
	{
		if(!SurvivalPickWinner())
		{
			SendSurvivalChat("[SURVIVAL] Nobody won.");
		}
		SurvivalSetGameState(SURVIVAL_LOBBY);
	}
	else if(AliveTees < g_Config.m_SvSurvivalDmPlayers)
	{
		SurvivalSetGameState(SURVIVAL_DM_COUNTDOWN);
		str_format(aBuf, sizeof(aBuf), "[SURVIVAL] deathmatch starts in %d minutes", m_survival_dm_countdown / (Server()->TickSpeed() * 60));
		SendSurvivalChat(aBuf);
	}
}

void CGameContext::SurvivalStartGame()
{
	vec2 SurvivalGameSpawnTile = Collision()->GetRandomTile(TILE_SURVIVAL_SPAWN);

	if(SurvivalGameSpawnTile == vec2(-1, -1)) //no survival arena
	{
		SurvivalSetGameState(SURVIVAL_LOBBY);
		SendSurvivalChat("[SURVIVAL] no survival arena set.");
		return;
	}
	else
	{
		SurvivalSetGameState(SURVIVAL_INGAME);
		SendSurvivalChat("[SURVIVAL] GAME STARTED !!!");
		//SendSurvivalBroadcast("STAY ALIVE!!!");
		SendSurvivalBroadcast(""); // clear countdown
		SurvivalCheckWinnerAndDeathMatch();
	}
}

void CGameContext::SendSurvivalChat(const char *pMsg)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			if(m_apPlayers[i]->m_IsSurvivaling)
			{
				SendChatTarget(i, pMsg);
			}
		}
	}
}

void CGameContext::SendSurvivalBroadcast(const char *pMsg, int Importance)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			if(m_apPlayers[i]->m_IsSurvivaling)
			{
				SendBroadcast(pMsg, i, Importance);
			}
		}
	}
}

void CGameContext::SetPlayerSurvival(int id, int mode) //0=off 1=lobby 2=ingame 3=die
{
	if(!m_apPlayers[id])
		return;

	if(mode == SURVIVAL_OFF)
	{
		m_apPlayers[id]->m_IsSurvivaling = false;
		m_apPlayers[id]->m_IsVanillaDmg = false;
		m_apPlayers[id]->m_IsVanillaWeapons = false;
		m_apPlayers[id]->m_IsVanillaCompetetive = false;
		m_apPlayers[id]->m_IsSurvivalAlive = false;
		m_apPlayers[id]->Pause(CPlayer::PAUSE_NONE, true);
	}
	else if(mode == SURVIVAL_LOBBY)
	{
		m_apPlayers[id]->m_IsSurvivalAlive = false;
		m_apPlayers[id]->m_IsSurvivaling = true;
		m_apPlayers[id]->m_IsVanillaDmg = true;
		m_apPlayers[id]->m_IsVanillaWeapons = true;
		m_apPlayers[id]->m_IsVanillaCompetetive = true;
		m_apPlayers[id]->m_IsSurvivalLobby = true;
		if(!m_survivalgamestate) //no game running --> start lobby
		{
			SurvivalSetGameState(SURVIVAL_LOBBY);
			dbg_msg("survival", "lobby started");
		}
	}
	else if(mode == SURVIVAL_INGAME)
	{
		m_apPlayers[id]->m_IsSurvivalAlive = true;
		m_apPlayers[id]->m_IsSurvivaling = true;
		m_apPlayers[id]->m_IsVanillaDmg = true;
		m_apPlayers[id]->m_IsVanillaWeapons = true;
		m_apPlayers[id]->m_IsVanillaCompetetive = true;
		m_apPlayers[id]->m_IsSurvivalLobby = false;
		m_apPlayers[id]->m_IsSurvivalWinner = false;
	}
	else if(mode == SURVIVAL_DIE)
	{
		m_apPlayers[id]->m_IsSurvivalAlive = false;
		m_apPlayers[id]->m_IsSurvivalLobby = true;
		m_apPlayers[id]->m_SurvivalDeaths++;
	}
	else
	{
		dbg_msg("survival", "WARNING setted undefined mode %d", mode);
	}
}

int CGameContext::SurvivalGetRandomAliveID(int NotThis)
{
	int r = rand() % CountSurvivalPlayers(true);
	int x = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(i == NotThis)
			continue;
		if(!m_apPlayers[i])
			continue;
		if(m_apPlayers[i]->m_IsSurvivaling && m_apPlayers[i]->m_IsSurvivalAlive)
		{
			if(x++ == r)
			{
				return i;
			}
		}
	}
	return -1;
}

void CGameContext::SurvivalGetNextSpectator(int UpdateID, int KillerID)
{
	CPlayer *pPlayer = m_apPlayers[UpdateID];
	if(!pPlayer)
		return;

	int AliveTees = CountSurvivalPlayers(true);
	if(AliveTees > 1)
	{
		pPlayer->m_SpectatorID = UpdateID == KillerID ? SurvivalGetRandomAliveID() : KillerID;
		pPlayer->Pause(CPlayer::PAUSE_SPEC, true);
	}
	else
	{
		pPlayer->Pause(CPlayer::PAUSE_NONE, true);
	}
}

void CGameContext::SurvivalUpdateSpectators(int DiedID, int KillerID)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i] || !m_apPlayers[i]->m_IsSurvivaling)
			continue;
		if(m_apPlayers[i]->m_SpectatorID == DiedID)
		{
			SurvivalGetNextSpectator(i, KillerID);
		}
	}
}

int CGameContext::CountSurvivalPlayers(bool Alive)
{
	int x = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i])
		{
			if(m_apPlayers[i]->m_IsSurvivaling && (!Alive || m_apPlayers[i]->m_IsSurvivalAlive))
			{
				x++;
			}
		}
	}
	return x;
}

void CGameContext::SurvivalSetGameState(int state)
{
	m_survivalgamestate = state;
	if(state == SURVIVAL_OFF)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_apPlayers[i])
			{
				SetPlayerSurvival(i, SURVIVAL_OFF);
			}
		}
	}
	else if(state == SURVIVAL_LOBBY)
	{
		m_survivallobbycountdown = Server()->TickSpeed() * g_Config.m_SvSurvivalLobbyDelay;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_apPlayers[i] && m_apPlayers[i]->m_IsSurvivaling)
			{
				m_apPlayers[i]->Pause(CPlayer::PAUSE_NONE, true);
			}
		}
	}
	else if(state == SURVIVAL_INGAME)
	{
		m_survival_spawn_counter = 0;
		m_survival_game_countdown = g_Config.m_SvSurvivalMaxGameTime ? Server()->TickSpeed() * (g_Config.m_SvSurvivalMaxGameTime * 60) : -1;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_apPlayers[i] && m_apPlayers[i]->m_IsSurvivaling)
			{
				if(m_apPlayers[i]->GetCharacter()) //only kill if isnt dead already or server crashes (he should respawn correctly anayways)
				{
					SaveCosmetics(i);
					m_apPlayers[i]->GetCharacter()->Die(i, WEAPON_GAME);
				}
				m_apPlayers[i]->Pause(CPlayer::PAUSE_NONE, true);
				SetPlayerSurvival(i, SURVIVAL_INGAME);
			}
		}
		m_survival_start_players = CountSurvivalPlayers(true); // all should be alive at game start. But in case we implment a afk state it should only count the active ones.
	}
	else if(state == SURVIVAL_DM_COUNTDOWN)
	{
		m_survival_dm_countdown = (Server()->TickSpeed() * 60) * g_Config.m_SvSurvivalDmDelay;
	}
	else if(state == SURVIVAL_DM)
	{
		SendSurvivalChat("[SURVIVAL] teleporting survivors to deathmatch arena.");

		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(m_apPlayers[i] && m_apPlayers[i]->GetCharacter() && m_apPlayers[i]->m_IsSurvivaling && m_apPlayers[i]->m_IsSurvivalAlive)
			{
				vec2 SurvivalSpawn = Collision()->GetRandomTile(TILE_SURVIVAL_DEATHMATCH);

				if(SurvivalSpawn != vec2(-1, -1))
				{
					m_apPlayers[i]->GetCharacter()->SetPosition(SurvivalSpawn);
				}
				else //no dm spawn tile
				{
					SendSurvivalChat("[SURVIVAL] error no deathmatch arena found.");
					break;
				}
			}
		}
	}
}

bool CGameContext::SurvivalPickWinner()
{
	int winnerID = -1;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_apPlayers[i] && m_apPlayers[i]->m_IsSurvivaling && m_apPlayers[i]->m_IsSurvivalAlive)
		{
			winnerID = i;
			break;
		}
	}
	if(winnerID == -1)
	{
		return false;
	}
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "[SURVIVAL] '%s' won the game!", Server()->ClientName(winnerID));
	SendSurvivalChat(aBuf);
	SendSurvivalBroadcast(aBuf);
	m_apPlayers[winnerID]->m_IsSurvivalWinner = true;

	if(m_apPlayers[winnerID]->IsLoggedIn())
	{
		SendChatTarget(winnerID, "[SURVIVAL] you won! [+50xp] [+50money]");
		m_apPlayers[winnerID]->MoneyTransaction(+50, "survival win");
		m_apPlayers[winnerID]->GiveXP(50);
	}
	else
	{
		SendChatTarget(winnerID, "[SURVIVAL] you won!");
	}

	str_copy(m_aLastSurvivalWinnerName, Server()->ClientName(winnerID), sizeof(m_aLastSurvivalWinnerName));
	m_apPlayers[winnerID]->m_SurvivalWins++;
	m_apPlayers[winnerID]->m_SurvivalDeaths--; //hacky method too keep deaths the same (because they get incremented in the next step)
	SetPlayerSurvival(winnerID, 3); //also set winner to dead now so that he can see names in lobby and respawns in lobby
	return true;
}