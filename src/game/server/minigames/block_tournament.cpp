// gamecontext scoped balance ddnet++ methods

#include <engine/shared/config.h>
#include <game/server/teams.h>

#include <cinttypes>
#include <cstring>

#include "../gamecontext.h"

void CGameContext::BlockTournaTick()
{
	char aBuf[128];

	if(m_BlockTournaState == 2) //ingame
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

			//ready all players
			for(auto &Player : m_apPlayers)
			{
				if(Player && Player->m_IsBlockTourning)
				{
					if(Player->GetCharacter())
					{
						//delete weapons
						Player->GetCharacter()->SetActiveWeapon(WEAPON_GUN);
						Player->GetCharacter()->SetWeaponGot(2, false);
						Player->GetCharacter()->SetWeaponGot(3, false);
						Player->GetCharacter()->SetWeaponGot(4, false);

						//delete cosmentics (they are not competetive)
						DeleteCosmetics(Player->GetCID());

						//delete "cheats" from the race
						Player->GetCharacter()->m_Jetpack = false;
						Player->GetCharacter()->m_EndlessHook = false;
						Player->GetCharacter()->m_SuperJump = false;

						//kill speed
						Player->GetCharacter()->KillSpeed();

						//teleport
						vec2 BlockPlayerSpawn = Collision()->GetRandomTile(TILE_BLOCK_TOURNA_SPAWN);

						if(BlockPlayerSpawn != vec2(-1, -1))
						{
							Player->GetCharacter()->SetPosition(BlockPlayerSpawn);
						}
						else //no tile found
						{
							SendBroadcastAll("[EVENT] Block tournament failed! No spawntiles found.", 2);
							EndBlockTourna();
							return;
						}

						//freeze to get a fair start nobody should be surprised
						Player->GetCharacter()->UnFreeze();
						Player->GetCharacter()->Freeze(6);
					}
					else
					{
						Player->m_IsBlockTourning = false;
						SendChatTarget(Player->GetCID(), "[BLOCK] you didn't join because you were dead on tournament start.");
					}
				}
			}
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
