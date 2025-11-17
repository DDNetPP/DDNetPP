#include "pvp_arena.h"

#include <engine/shared/config.h>

#include <game/mapitems_ddpp.h>
#include <game/server/gamecontext.h>

bool CPvpArena::IsActive(int ClientId)
{
	CPlayer *pPlayer = GameServer()->GetPlayerOrNullptr(ClientId);
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

void CPvpArena::Join(CPlayer *pPlayer)
{
	int ClientId = pPlayer->GetCid();
	if(GameServer()->IsMinigame(ClientId))
	{
		SendChatTarget(ClientId, "[PVP] You can't join becasue your are in another minigame or jail (check '/minigames status')");
		return;
	}

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	if(pPlayer->m_Account.m_PvpArenaTickets < 1)
	{
		SendChatTarget(ClientId, "[PVP] You don't have a ticket. Buy a ticket first with '/buy pvp_arena_ticket'");
		return;
	}
	if(pPlayer->GetCharacter()->m_IsPvpArenaing)
	{
		SendChatTarget(ClientId, "[PVP] You are already in the PvP-arena");
		return;
	}
	// we do not call LoadPosition()
	// but do a manual tele request back to GetSavePosition()
	SavePosition(pPlayer);

	if(g_Config.m_SvTeleportationDelay)
		SendChatTarget(ClientId, "[PVP] Teleport request sent. Don't move for 4 seconds.");

	pChr->RequestTeleToTile(TILE_PVP_ARENA_SPAWN)
		.DelayInSeconds(4)
		.OnPreSuccess([=, this]() {
			SavePosition(pChr->GetPlayer());
		})
		.OnPostSuccess([=, this]() {
			pPlayer->m_Account.m_PvpArenaTickets--;
			pPlayer->m_Account.m_PvpArenaGamesPlayed++;
			pChr->m_IsPvpArenaing = true;
			pChr->m_isDmg = true;
			GameServer()->SendChatTarget(pPlayer->GetCid(), "[PVP] Teleporting to arena... good luck and have fun!");
		})
		.OnFailure([=, this](const char *pShort, const char *pLong) {
			char aError[512];
			str_format(aError, sizeof(aError), "[PVP] %s", pLong);
			SendChatTarget(pChr->GetPlayer()->GetCid(), aError);
		});
}

void CPvpArena::Leave(CPlayer *pPlayer)
{
	if(!IsActive(pPlayer->GetCid()))
	{
		SendChatTarget(pPlayer->GetCid(), "[PVP] You are not in an arena.");
		return;
	}

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	vec2 OldPos;
	if(!GetSavedPosition(pPlayer, &OldPos))
	{
		SendChatTarget(pPlayer->GetCid(), "[PVP] Your old position was lost. This really should not have happend sorry you are stuck.");
		return;
	}

	if(g_Config.m_SvTeleportationDelay)
		SendChatTarget(pPlayer->GetCid(), "[PVP] Teleport request sent. Don't move for 6 seconds.");

	pChr->RequestTeleToPos(OldPos)
		.DelayInSeconds(6)
		.OnPostSuccess([=, this]() {
			ClearSavedPosition(pPlayer);

			pPlayer->m_Account.m_PvpArenaTickets++;
			pChr->SetHealth(10);
			pChr->m_IsPvpArenaing = false;
			pChr->m_isDmg = false;

			GameServer()->SendChatTarget(pPlayer->GetCid(), "[PVP] Successfully teleported out of arena.");
			GameServer()->SendChatTarget(pPlayer->GetCid(), "[PVP] You got your ticket back because you have survived.");
		})
		.OnFailure([=, this](const char *pShort, const char *pLong) {
			char aError[512];
			str_format(aError, sizeof(aError), "[PVP] %s", pLong);
			SendChatTarget(pChr->GetPlayer()->GetCid(), aError);
		});
}

void CPvpArena::CharacterTick(CCharacter *pChr)
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
