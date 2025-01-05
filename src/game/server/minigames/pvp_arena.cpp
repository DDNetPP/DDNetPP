#include <game/server/gamecontext.h>

#include "pvp_arena.h"

bool CPvpArena::IsActive(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return false;

	return pChr->m_IsPvpArenaing;
}

bool CPvpArena::OnChatCmdLeave(CPlayer *pPlayer)
{
	Leave(pPlayer);
	return true;
}

void CPvpArena::Leave(CPlayer *pPlayer)
{
	if(!IsActive(pPlayer->GetCid()))
	{
		SendChatTarget(pPlayer->GetCid(), "[PVP] You are not in an arena.");
		return;
	}

	SendChatTarget(pPlayer->GetCid(), "[PVP] Teleport request sent. Don't move for 6 seconds.");
	pPlayer->GetCharacter()->m_pvp_arena_tele_request_time = Server()->TickSpeed() * 6;
	pPlayer->GetCharacter()->m_pvp_arena_exit_request = true;
}

void CPvpArena::PlayerTick(CPlayer *pPlayer)
{
}

void CPvpArena::OnDeath(CCharacter *pChr, int Killer, int Weapon)
{
	if(!IsActive(pChr->GetPlayer()->GetCid()))
		return;

	CPlayer *pPlayer = pChr->GetPlayer();
	char aBuf[512];
	if(GameServer()->m_apPlayers[Killer])
	{
		if(GameServer()->GetPlayerChar(Killer) && Weapon != WEAPON_GAME && Weapon != WEAPON_SELF)
		{
			//GameServer()->GetPlayerChar(Killer)->m_Bloody = true;

			if(GameServer()->GetPlayerChar(Killer)->m_IsPvpArenaing)
			{
				if(GameServer()->m_apPlayers[Killer]->IsMaxLevel() ||
					GameServer()->IsSameIp(Killer, pPlayer->GetCid()) || // dont give xp on dummy kill
					GameServer()->IsSameIp(pPlayer->GetCid(), GameServer()->m_apPlayers[Killer]->m_pvp_arena_last_kill_id) // dont give xp on killing same ip twice in a row
				)
				{
					GameServer()->m_apPlayers[Killer]->MoneyTransaction(+150, "pvp_arena kill");
					GameServer()->m_apPlayers[Killer]->m_Account.m_PvpArenaKills++;

					str_format(aBuf, sizeof(aBuf), "[PVP] +150 money for killing %s", Server()->ClientName(pPlayer->GetCid()));
					GameServer()->SendChatTarget(Killer, aBuf);
				}
				else
				{
					GameServer()->m_apPlayers[Killer]->MoneyTransaction(+150, "pvp_arena kill");
					GameServer()->m_apPlayers[Killer]->GiveXP(100);
					GameServer()->m_apPlayers[Killer]->m_Account.m_PvpArenaKills++;

					str_format(aBuf, sizeof(aBuf), "[PVP] +100 xp +150 money for killing %s", Server()->ClientName(pPlayer->GetCid()));
					GameServer()->SendChatTarget(Killer, aBuf);
				}

				int r = rand() % 100;
				if(r > 92)
				{
					GameServer()->m_apPlayers[Killer]->m_Account.m_PvpArenaTickets++;
					GameServer()->SendChatTarget(Killer, "[PVP] +1 pvp_arena_ticket        (special random drop for kill)");
				}
				GameServer()->m_apPlayers[Killer]->m_pvp_arena_last_kill_id = pPlayer->GetCid();
			}
		}
	}

	pPlayer->m_Account.m_PvpArenaDeaths++;

	str_format(aBuf, sizeof(aBuf), "[PVP] You lost the arena-fight because you were killed by %s.", Server()->ClientName(Killer));
	GameServer()->SendChatTarget(pPlayer->GetCid(), aBuf);
}
