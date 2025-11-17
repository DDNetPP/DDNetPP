// gamecontext scoped bomb ddnet++ methods

#include "bomb.h"

#include "../gamecontext.h"

#include <engine/shared/config.h>

#include <game/server/teams.h>

#include <cinttypes>

bool CBomb::IsActive(int ClientId)
{
	CPlayer *pPlayer = GameServer()->GetPlayerOrNullptr(ClientId);
	if(!pPlayer)
		return false;
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return false;
	return pChr->m_IsBombing;
}

void CGameContext::EndBombGame(int WinnerId)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GetPlayerChar(i))
		{
			if(GetPlayerChar(i)->m_IsBombing)
			{
				GetPlayerChar(i)->m_IsBombing = false;
			}
			if(GetPlayerChar(i)->m_IsBomb)
			{
				GetPlayerChar(i)->m_IsBomb = false;
			}
			if(GetPlayerChar(i)->m_IsBombReady)
			{
				GetPlayerChar(i)->m_IsBombReady = false;
			}
		}
	}
	m_BombGameState = 0;
	m_BombTick = g_Config.m_SvBombTicks;

	if(WinnerId == -1)
	{
		return;
	}

	//winner private
	char aBuf[128];
	m_apPlayers[WinnerId]->MoneyTransaction(m_BombMoney * m_BombStartPlayers, "won bomb");
	str_format(aBuf, sizeof(aBuf), "[BOMB] You won the bomb game. +%" PRId64 " money.", m_BombMoney * m_BombStartPlayers);
	SendChatTarget(WinnerId, aBuf);
	m_apPlayers[WinnerId]->m_Account.m_BombGamesWon++;
	m_apPlayers[WinnerId]->m_Account.m_BombGamesPlayed++;
	if(!str_comp_nocase(m_BombMap, "NoArena"))
	{
		//GetPlayerChar(i)->ChillTelePortTile(GetPlayerChar(i)->m_BombPosX, GetPlayerChar(i)->m_BombPosY); //dont tele back in no arena
	}
	else
	{
		GetPlayerChar(WinnerId)->ChillTelePortTile(GetPlayerChar(WinnerId)->m_BombPosX, GetPlayerChar(WinnerId)->m_BombPosY); //tele on pos where game started
	}

	//winner public
	str_format(aBuf, sizeof(aBuf), "[BOMB] '%s' won and got %" PRId64 " money!", Server()->ClientName(WinnerId), m_BombMoney * m_BombStartPlayers);
	SendChat(-1, TEAM_ALL, aBuf);
}

void CGameContext::CheckStartBomb()
{
	char aBuf[128];
	bool AllReady = true;
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;

		int PlayerId = Player->GetCid();
		if(!GetPlayerChar(PlayerId))
			continue;

		if(GetPlayerChar(PlayerId)->m_IsBombing && !GetPlayerChar(PlayerId)->m_IsBombReady)
		{
			AllReady = false;
			//break; //back in the times this was an performance improvement but nowerdays we need all id's of the unready players to kick em

			//Kick unready players
			Player->m_BombTicksUnready++;
			if(Player->m_BombTicksUnready + 500 == g_Config.m_SvBombUnreadyKickDelay)
			{
				SendChatTarget(PlayerId, "[BOMB] WARNING! Type '/bomb start' or you will be kicked out of the bomb game.");
			}
			if(Player->m_BombTicksUnready > g_Config.m_SvBombUnreadyKickDelay)
			{
				SendBroadcast("", PlayerId); //send empty broadcast to signalize lobby leave
				SendChatTarget(PlayerId, "[BOMB] you got kicked out of lobby. (Reason: too late '/bomb start')");

				GetPlayerChar(PlayerId)->m_IsBombing = false;
				GetPlayerChar(PlayerId)->m_IsBomb = false;
				GetPlayerChar(PlayerId)->m_IsBombReady = false;
			}
		}
	}
	//if (CountReadyBombPlayers() == CountBombPlayers()) //eats more ressources than the other way
	if(AllReady)
	{
		if(m_BombStartCountDown > 1)
		{
			if(Server()->Tick() % 40 == 0)
			{
				for(int i = 0; i < MAX_CLIENTS; i++)
				{
					if(m_apPlayers[i] && GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing)
					{
						str_format(aBuf, sizeof(aBuf), "[BOMB] game starts in %d ...", m_BombStartCountDown);
						SendBroadcast(aBuf, i);
					}
				}
				m_BombStartCountDown--;
			}
		}
		else
		{
			m_BombStartPlayers = CountBombPlayers();
			m_BombGameState = 3;
			m_BombStartCountDown = g_Config.m_SvBombStartDelay;
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(m_apPlayers[i] && GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing)
				{
					if(!str_comp_nocase(m_BombMap, "Default"))
					{
						//GetPlayerChar(i)->m_Pos.x = g_Config.m_SvBombSpawnX + m_apPlayers[i]->GetCid() * 2; //spread the spawns round the cfg var depending on cid max distance is 63 * 2 = 126 = almost 4 tiles
						//GetPlayerChar(i)->m_Pos.x = g_Config.m_SvBombSpawnX;
						//GetPlayerChar(i)->m_Pos.y = g_Config.m_SvBombSpawnY;
						GetPlayerChar(i)->ChillTelePort((g_Config.m_SvBombSpawnX * 32) + m_apPlayers[i]->GetCid() * 2, g_Config.m_SvBombSpawnY * 32);
						//GetPlayerChar(i)->m_Pos = vec2(g_Config.m_SvBombSpawnX + m_apPlayers[i]->GetCid() * 2, g_Config.m_SvBombSpawnY); //doesnt tele but would freeze the tees (which could be nice but idk ... its scary)
					}
					str_format(aBuf, sizeof(aBuf), "Bomb game has started! +%" PRId64 " money for the winner!", m_BombMoney * m_BombStartPlayers);
					SendBroadcast(aBuf, i);
					GetPlayerChar(i)->m_BombPosX = GetPlayerChar(i)->m_Pos.x / 32;
					GetPlayerChar(i)->m_BombPosY = GetPlayerChar(i)->m_Pos.y / 32;
				}
			}
		}
	}
}

void CGameContext::BombTick()
{
	char aBuf[512];

	//bomb tickin'
	m_BombTick--;
	if(m_BombTick == 0) //time over --> kill the bomb (bomb explode)
	{
		m_BombTick = g_Config.m_SvBombTicks;
		for(auto &Player : m_apPlayers)
		{
			if(!Player)
				continue;

			int PlayerId = Player->GetCid();
			if(GetPlayerChar(PlayerId))
			{
				if(GetPlayerChar(PlayerId)->m_IsBomb)
				{
					Player->m_Account.m_BombGamesPlayed++;
					CreateExplosion(GetPlayerChar(PlayerId)->m_Pos, PlayerId, WEAPON_GRENADE, false, 0, GetPlayerChar(PlayerId)->Teams()->TeamMask(0)); //bomb explode! (think this explosion is always team 0 but yolo)
					str_format(aBuf, sizeof(aBuf), "'%s' exploded as bomb", Server()->ClientName(PlayerId));
					Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "bomb", aBuf);
					GetPlayerChar(PlayerId)->Die(PlayerId, WEAPON_GAME);
					break;
				}
			}
		}
	}

	//check start game
	if(m_BombGameState < 3) //not ingame
	{
		for(auto &Player : m_apPlayers)
		{
			if(!Player)
				continue;

			CCharacter *pChr = Player->GetCharacter();
			if(!pChr)
				continue;

			if(pChr->m_IsBombing)
			{
				if(Server()->Tick() % 40 == 0)
				{
					if(pChr->m_IsBombReady)
					{
						str_format(
							aBuf,
							sizeof(aBuf),
							"--== Bomb Lobby ==--\n"
							"[%d/%d] players ready\n"
							"Map: %s   Money: %" PRId64,
							CountReadyBombPlayers(),
							CountBombPlayers(),
							m_BombMap,
							m_BombMoney);
					}
					else
					{
						str_format(
							aBuf,
							sizeof(aBuf),
							"--== Bomb Lobby ==--\n"
							"[%d/%d] players ready\n"
							"Map: %s   Money: %" PRId64 "\n\n\n"
							"Type '/bomb start' to start.",
							CountReadyBombPlayers(),
							CountBombPlayers(),
							m_BombMap,
							m_BombMoney);
					}
					SendBroadcast(aBuf, Player->GetCid());
				}
			}
		}
		if(CountBombPlayers() > 1) //2+ tees required to start a game
		{
			CheckStartBomb();
		}
		else
		{
			m_BombGameState = 1; //unlock bomb lobbys with only 1 tee
		}
	}

	//check end game (no players)
	if(!CountBombPlayers())
	{
		EndBombGame(-1);
	}

	//check end game (only 1 player -> winner)
	if(CountBombPlayers() == 1 && m_BombGameState == 3)
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GetPlayerChar(i))
			{
				if(GetPlayerChar(i)->m_IsBombing)
				{
					EndBombGame(i);
					break;
				}
			}
		}
	}

	//check for missing bomb
	if(m_BombGameState == 3)
	{
		bool BombFound = false;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GetPlayerChar(i))
			{
				if(GetPlayerChar(i)->m_IsBomb)
				{
					BombFound = true;
					break;
				}
			}
		}
		if(!BombFound) //nobody bomb? -> pick new1
		{
			m_BombTick = g_Config.m_SvBombTicks;
			m_BombFinalColor = 180;

			//str_format(aBuf, sizeof(aBuf), "Bombfound: %d", FindNextBomb());
			//Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "bomb", aBuf);

			if(FindNextBomb() != -1)
			{
				GetPlayerChar(FindNextBomb())->m_IsBomb = true;
				SendChatTarget(FindNextBomb(), "The server has picked you as bomb.");
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "Failed to pick new bomb. Bombfound: %d", FindNextBomb());
				Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "bomb", aBuf);
			}
		}
	}
}

int CGameContext::FindNextBomb()
{
	//Check who has the furthest distance to all other players (no average middle needed)
	//New version with pythagoras
	int MaxDist = 0;
	int NextBombId = -1;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing)
		{
			int Dist = 0;
			for(int IComp = 0; IComp < MAX_CLIENTS; IComp++)
			{
				if(GetPlayerChar(IComp) && GetPlayerChar(IComp)->m_IsBombing)
				{
					int a = GetPlayerChar(i)->m_Pos.x - GetPlayerChar(IComp)->m_Pos.x;
					int b = GetPlayerChar(i)->m_Pos.y - GetPlayerChar(IComp)->m_Pos.y;

					//|a| |b|
					a = abs(a);
					b = abs(b);

					int c = sqrt((double)(a + b)); //pythagoras rocks
					Dist += c; //store all distances to all players
				}
			}
			if(Dist > MaxDist)
			{
				MaxDist = Dist;
				NextBombId = i;
			}
		}
	}
	return NextBombId;
}

int CGameContext::CountBannedBombPlayers()
{
	int BannedPlayers = 0;

	for(auto &Player : m_apPlayers)
		if(Player && Player->m_Account.m_BombBanTime)
			BannedPlayers++;

	return BannedPlayers;
}

int CGameContext::CountBombPlayers()
{
	int BombPlayers = 0;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GetPlayerChar(i))
		{
			if(GetPlayerChar(i)->m_IsBombing)
			{
				BombPlayers++;
			}
		}
	}
	return BombPlayers;
}

int CGameContext::CountReadyBombPlayers()
{
	int RdyPlrs = 0;

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GetPlayerChar(i) && GetPlayerChar(i)->m_IsBombing && GetPlayerChar(i)->m_IsBombReady)
		{
			RdyPlrs++;
		}
	}
	return RdyPlrs;
}

void CGameContext::SendBroadcastBomb(const char *pMsg)
{
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;

		CCharacter *pChr = Player->GetCharacter();
		if(!pChr)
			continue;
		if(!pChr->m_IsBombing)
			continue;

		SendBroadcast(pMsg, Player->GetCid());
	}
}

void CGameContext::SendChatBomb(const char *pMsg)
{
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;

		CCharacter *pChr = Player->GetCharacter();
		if(!pChr)
			continue;
		if(!pChr->m_IsBombing)
			continue;

		SendChatTarget(Player->GetCid(), pMsg);
	}
}
