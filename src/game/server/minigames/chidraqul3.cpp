// gamecontext scoped chodraqul3 ddnet++ methods

#include "../gamecontext.h"

#include <engine/shared/config.h>

#include <game/server/teams.h>

#include <cinttypes>
#include <cstring>

int CGameContext::C3_GetFreeSlots()
{
	int c = g_Config.m_SvChidraqulSlots;
	for(auto &Player : m_apPlayers)
		if(Player && Player->m_C3_GameState == 2)
			c--;
	return c;
}

int CGameContext::C3_GetOnlinePlayers()
{
	int c = 0;
	for(auto &Player : m_apPlayers)
		if(Player && Player->m_C3_GameState == 2)
			c++;
	return c;
}

void CGameContext::C3_MultiPlayer_GameTick(int id)
{
	if(m_apPlayers[id]->m_C3_UpdateFrame || Server()->Tick() % 120 == 0)
	{
		C3_RenderFrame();
		for(auto &Player : m_apPlayers)
		{
			if(Player)
			{
				Player->m_C3_UpdateFrame = false; //only render once a tick
			}
		}
	}
}

void CGameContext::C3_RenderFrame()
{
	char aBuf[128];
	char aHUD[64];
	char aWorld[64]; //max world size
	int players = C3_GetOnlinePlayers();

	//init world
	for(int i = 0; i < g_Config.m_SvChidraqulWorldX; i++)
	{
		aWorld[i] = '_';
	}

	//place players
	for(auto &Player : m_apPlayers)
	{
		if(Player && Player->m_C3_GameState == 2)
		{
			aWorld[Player->m_HashPos] = Player->m_HashSkin[0];
		}
	}

	//finish string
	aWorld[g_Config.m_SvChidraqulWorldX] = '\0';

	//add hud and send to players
	for(auto &Player : m_apPlayers)
	{
		if(Player && Player->m_C3_GameState == 2)
		{
			str_format(aHUD, sizeof(aHUD), "\n\nPos: %d Players: %d/%d", Player->m_HashPos, players, g_Config.m_SvChidraqulSlots);
			str_format(aBuf, sizeof(aBuf), "%s%s", aWorld, aHUD);

			//dbg_msg("debug", "printing: %s", aBuf);

			SendBroadcast(aBuf, Player->GetCid(), 0);
		}
	}
}
