// gamecontext scoped survival ddnet++ methods

#include "survival.h"

#include "../gamecontext.h"

#include <engine/shared/config.h>

#include <game/mapitems.h>

bool CSurvival::IsActive(int ClientId)
{
	CPlayer *pPlayer = GameServer()->GetPlayerOrNullptr(ClientId);
	if(!pPlayer)
		return false;
	return pPlayer->m_IsSurvivaling;
}

vec2 CGameContext::GetNextSurvivalSpawn(int ClientId)
{
	vec2 Spawn = Collision()->GetSurvivalSpawn(m_survival_spawn_counter++);
	if(Spawn == vec2(-1, -1))
	{
		SendChatTarget(ClientId, "[SURVIVAL] No arena set.");
		SurvivalSetGameState(SURVIVAL_OFF);
		return GetSurvivalLobbySpawn(ClientId);
	}
	return Spawn;
}

vec2 CGameContext::GetSurvivalLobbySpawn(int ClientId)
{
	vec2 Spawn = Collision()->GetRandomTile(TILE_SURVIVAL_LOBBY);
	if(Spawn == vec2(-1, -1))
	{
		SendChatTarget(ClientId, "[SURVIVAL] No lobby set.");
		SurvivalSetGameState(SURVIVAL_OFF);
	}
	return Spawn;
}

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
			SendBroadcastSurvival(aBuf);

			if(m_survivallobbycountdown == (Server()->TickSpeed() * 9)) //teleport winner in lobby on last 10 sec countdown
			{
				for(auto &Player : m_apPlayers)
				{
					if(Player && Player->GetCharacter() && Player->m_IsSurvivaling && Player->m_IsSurvivalWinner)
					{
						vec2 SurvivalLobbySpawnTile = Collision()->GetRandomTile(TILE_SURVIVAL_LOBBY);

						if(SurvivalLobbySpawnTile == vec2(-1, -1)) //no survival lobby
						{
							SendChatSurvival("[SURVIVAL] no survival lobby set.");
						}
						else
						{
							Player->GetCharacter()->SetPosition(SurvivalLobbySpawnTile);
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
			SendBroadcastSurvival(aBuf);
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
		SendBroadcastSurvival(aBuf);
	}
	else if(m_survival_dm_countdown % (Server()->TickSpeed() * 60) == 0 && m_survival_dm_countdown - 10 < (Server()->TickSpeed() * 60) * 5) //every minute if minutes < 5
	{
		str_format(aBuf, sizeof(aBuf), "[SURVIVAL] deathmatch starts in %d minutes", m_survival_dm_countdown / (Server()->TickSpeed() * 60));
		SendChatSurvival(aBuf);
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
			SendChatSurvival("[SURVIVAL] Nobody won.");
		}
		SurvivalSetGameState(SURVIVAL_LOBBY);
	}
	else if(AliveTees < g_Config.m_SvSurvivalDmPlayers)
	{
		SurvivalSetGameState(SURVIVAL_DM_COUNTDOWN);
		str_format(aBuf, sizeof(aBuf), "[SURVIVAL] deathmatch starts in %d minutes", m_survival_dm_countdown / (Server()->TickSpeed() * 60));
		SendChatSurvival(aBuf);
	}
}

void CGameContext::SurvivalStartGame()
{
	vec2 SurvivalGameSpawnTile = Collision()->GetRandomTile(TILE_SURVIVAL_SPAWN);

	if(SurvivalGameSpawnTile == vec2(-1, -1)) //no survival arena
	{
		SurvivalSetGameState(SURVIVAL_LOBBY);
		SendChatSurvival("[SURVIVAL] no survival arena set.");
		return;
	}
	else
	{
		SurvivalSetGameState(SURVIVAL_INGAME);
		SendChatSurvival("[SURVIVAL] GAME STARTED !!!");
		//SendBroadcastSurvival("STAY ALIVE!!!");
		SendBroadcastSurvival(""); // clear countdown
		SurvivalCheckWinnerAndDeathMatch();
	}
}

void CGameContext::SendChatSurvival(const char *pMsg)
{
	for(auto &Player : m_apPlayers)
	{
		if(Player)
		{
			if(Player->m_IsSurvivaling)
			{
				SendChatTarget(Player->GetCid(), pMsg);
			}
		}
	}
}

void CGameContext::SendBroadcastSurvival(const char *pMsg, int Importance)
{
	for(auto &Player : m_apPlayers)
	{
		if(Player)
		{
			if(Player->m_IsSurvivaling)
			{
				SendBroadcast(pMsg, Player->GetCid(), Importance);
			}
		}
	}
}

void CGameContext::SetPlayerSurvival(int ClientId, int Mode) //0=off 1=lobby 2=ingame 3=die
{
	if(!m_apPlayers[ClientId])
		return;

	if(Mode == SURVIVAL_OFF)
	{
		m_apPlayers[ClientId]->m_IsSurvivaling = false;
		m_apPlayers[ClientId]->m_IsVanillaDmg = false;
		m_apPlayers[ClientId]->m_IsVanillaWeapons = false;
		m_apPlayers[ClientId]->m_IsVanillaCompetetive = false;
		m_apPlayers[ClientId]->m_IsSurvivalAlive = false;
		m_apPlayers[ClientId]->Pause(CPlayer::PAUSE_NONE, true);
	}
	else if(Mode == SURVIVAL_LOBBY)
	{
		m_apPlayers[ClientId]->m_IsSurvivalAlive = false;
		m_apPlayers[ClientId]->m_IsSurvivaling = true;
		m_apPlayers[ClientId]->m_IsVanillaDmg = true;
		m_apPlayers[ClientId]->m_IsVanillaWeapons = true;
		m_apPlayers[ClientId]->m_IsVanillaCompetetive = true;
		m_apPlayers[ClientId]->m_IsSurvivalLobby = true;
		if(!m_survivalgamestate) //no game running --> start lobby
		{
			SurvivalSetGameState(SURVIVAL_LOBBY);
			dbg_msg("survival", "lobby started");
		}
	}
	else if(Mode == SURVIVAL_INGAME)
	{
		m_apPlayers[ClientId]->m_IsSurvivalAlive = true;
		m_apPlayers[ClientId]->m_IsSurvivaling = true;
		m_apPlayers[ClientId]->m_IsVanillaDmg = true;
		m_apPlayers[ClientId]->m_IsVanillaWeapons = true;
		m_apPlayers[ClientId]->m_IsVanillaCompetetive = true;
		m_apPlayers[ClientId]->m_IsSurvivalLobby = false;
		m_apPlayers[ClientId]->m_IsSurvivalWinner = false;
	}
	else if(Mode == SURVIVAL_DIE)
	{
		m_apPlayers[ClientId]->m_IsSurvivalAlive = false;
		m_apPlayers[ClientId]->m_IsSurvivalLobby = true;
		m_apPlayers[ClientId]->m_Account.m_SurvivalDeaths++;
	}
	else
	{
		dbg_msg("survival", "WARNING setted undefined mode %d", Mode);
	}
}

int CGameContext::SurvivalGetRandomAliveId(int NotThis)
{
	int RandPlayerIndex = rand() % CountSurvivalPlayers(true);
	int x = 0;
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;
		if(Player->GetCid() == NotThis)
			continue;

		if(Player->m_IsSurvivaling && Player->m_IsSurvivalAlive)
			if(x++ == RandPlayerIndex)
				return Player->GetCid();
	}
	return -1;
}

void CGameContext::SurvivalGetNextSpectator(int UpdateId, int KillerId)
{
	CPlayer *pPlayer = m_apPlayers[UpdateId];
	if(!pPlayer)
		return;

	int AliveTees = CountSurvivalPlayers(true);
	if(AliveTees > 1)
	{
		pPlayer->SetSpectatorId(UpdateId == KillerId ? SurvivalGetRandomAliveId() : KillerId);
		pPlayer->Pause(CPlayer::PAUSE_SPEC, true);
	}
	else
	{
		pPlayer->Pause(CPlayer::PAUSE_NONE, true);
	}
}

void CGameContext::SurvivalUpdateSpectators(int DiedId, int KillerId)
{
	for(auto &Player : m_apPlayers)
	{
		if(!Player || !Player->m_IsSurvivaling)
			continue;
		if(Player->SpectatorId() == DiedId)
		{
			SurvivalGetNextSpectator(Player->GetCid(), KillerId);
		}
	}
}

int CGameContext::CountSurvivalPlayers(bool Alive)
{
	int x = 0;
	for(auto &Player : m_apPlayers)
		if(Player)
			if(Player->m_IsSurvivaling && (!Alive || Player->m_IsSurvivalAlive))
				x++;
	return x;
}

void CGameContext::SurvivalSetGameState(int State)
{
	m_survivalgamestate = State;
	if(State == SURVIVAL_OFF)
	{
		for(auto &Player : m_apPlayers)
			if(Player)
				SetPlayerSurvival(Player->GetCid(), SURVIVAL_OFF);
	}
	else if(State == SURVIVAL_LOBBY)
	{
		m_survivallobbycountdown = Server()->TickSpeed() * g_Config.m_SvSurvivalLobbyDelay;
		for(auto &Player : m_apPlayers)
			if(Player && Player->m_IsSurvivaling)
				Player->Pause(CPlayer::PAUSE_NONE, true);
	}
	else if(State == SURVIVAL_INGAME)
	{
		m_survival_spawn_counter = 0;
		m_survival_game_countdown = g_Config.m_SvSurvivalMaxGameTime ? Server()->TickSpeed() * (g_Config.m_SvSurvivalMaxGameTime * 60) : -1;
		for(auto &Player : m_apPlayers)
		{
			if(Player && Player->m_IsSurvivaling)
			{
				if(Player->GetCharacter()) //only kill if isnt dead already or server crashes (he should respawn correctly anayways)
				{
					SaveCosmetics(Player->GetCid());
					Player->GetCharacter()->Die(Player->GetCid(), WEAPON_GAME);
				}
				Player->Pause(CPlayer::PAUSE_NONE, true);
				SetPlayerSurvival(Player->GetCid(), SURVIVAL_INGAME);
			}
		}
		m_survival_start_players = CountSurvivalPlayers(true); // all should be alive at game start. But in case we implment a afk state it should only count the active ones.
	}
	else if(State == SURVIVAL_DM_COUNTDOWN)
	{
		m_survival_dm_countdown = (Server()->TickSpeed() * 60) * g_Config.m_SvSurvivalDmDelay;
	}
	else if(State == SURVIVAL_DM)
	{
		SendChatSurvival("[SURVIVAL] teleporting survivors to deathmatch arena.");

		for(auto &Player : m_apPlayers)
		{
			if(Player && Player->GetCharacter() && Player->m_IsSurvivaling && Player->m_IsSurvivalAlive)
			{
				vec2 SurvivalSpawn = Collision()->GetRandomTile(TILE_SURVIVAL_DEATHMATCH);

				if(SurvivalSpawn != vec2(-1, -1))
				{
					Player->GetCharacter()->SetPosition(SurvivalSpawn);
				}
				else //no dm spawn tile
				{
					SendChatSurvival("[SURVIVAL] error no deathmatch arena found.");
					break;
				}
			}
		}
	}
}

bool CGameContext::SurvivalPickWinner()
{
	int WinnerId = -1;
	for(auto &Player : m_apPlayers)
	{
		if(Player && Player->m_IsSurvivaling && Player->m_IsSurvivalAlive)
		{
			WinnerId = Player->GetCid();
			break;
		}
	}
	if(WinnerId == -1)
	{
		return false;
	}
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "[SURVIVAL] '%s' won the game!", Server()->ClientName(WinnerId));
	SendChatSurvival(aBuf);
	SendBroadcastSurvival(aBuf);
	m_apPlayers[WinnerId]->m_IsSurvivalWinner = true;

	if(m_apPlayers[WinnerId]->IsLoggedIn())
	{
		SendChatTarget(WinnerId, "[SURVIVAL] you won! [+50xp] [+50money]");
		m_apPlayers[WinnerId]->MoneyTransaction(+50, "survival win");
		m_apPlayers[WinnerId]->GiveXP(50);
	}
	else
	{
		SendChatTarget(WinnerId, "[SURVIVAL] you won!");
	}

	str_copy(m_aLastSurvivalWinnerName, Server()->ClientName(WinnerId), sizeof(m_aLastSurvivalWinnerName));
	m_apPlayers[WinnerId]->m_Account.m_SurvivalWins++;
	m_apPlayers[WinnerId]->m_Account.m_SurvivalDeaths--; //hacky method too keep deaths the same (because they get incremented in the next step)
	SetPlayerSurvival(WinnerId, 3); //also set winner to dead now so that he can see names in lobby and respawns in lobby
	return true;
}
