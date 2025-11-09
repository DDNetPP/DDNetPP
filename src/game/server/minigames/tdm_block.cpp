#include "tdm_block.h"

#include <base/log.h>
#include <base/system.h>

#include <engine/shared/config.h>

#include <generated/protocol.h>

#include <game/mapitems_ddpp.h>
#include <game/race_state.h>
#include <game/server/ddpp/enums.h>
#include <game/server/ddpp/teleportation_request.h>
#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/player.h>
#include <game/server/teams.h>
#include <game/team_state.h>

bool CTdmBlock::IsActive(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	return pPlayer->m_IsBlockTdming;
}

void CTdmBlock::OnDeath(CCharacter *pChr, int Killer, int Weapon)
{
	CPlayer *pPlayer = pChr->GetPlayer();
	if(!pPlayer->m_IsBlockTdming)
		return;

	if(Weapon != WEAPON_GAME && Weapon != WEAPON_MINIGAME)
	{
		CPlayer *pKiller = GameServer()->GetPlayerOrNullptr(Killer);
		if(pKiller && pChr->GetId() != Killer)
			pKiller->m_MinigameScore++;
	}
}

bool CTdmBlock::AllowSelfKill(int ClientId)
{
	if(ClientId < 0 || ClientId > MAX_CLIENTS)
		return true;
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return true;
	if(!IsActive(ClientId))
		return true;
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return false;

	// avoid messing with the round start freeze
	int SecondsSinceSpawn = (Server()->Tick() - pChr->m_SpawnTick) / Server()->TickSpeed();
	if(SecondsSinceSpawn < 5)
		return false;
	return true;
}

void CTdmBlock::PostSpawn(CCharacter *pChr)
{
	if(!pChr)
		return;
	CPlayer *pPlayer = pChr->GetPlayer();
	if(!pPlayer)
		return;
	if(!IsActive(pPlayer->GetCid()))
		return;

	pChr->Freeze(3);
}

int CTdmBlock::ScoreLimit(CPlayer *pPlayer)
{
	if(!pPlayer)
		return 10;
	if(!GameState(pPlayer))
		return 10;
	return GameState(pPlayer)->ScoreLimit();
}

vec2 CTdmBlock::GetNextArenaSpawn(CGameState *pGameState)
{
	vec2 Spawn = Collision()->GetTileAtNum(TILE_BLOCK_DM_A1, pGameState->m_SpawnCounter++);

	// start re using spawns when there is not enough
	// but abort when we have to reuse in the beginning already
	if(Spawn == vec2(-1, -1) && pGameState->m_SpawnCounter > 1)
	{
		pGameState->m_SpawnCounter = 0;
		Spawn = Collision()->GetTileAtNum(TILE_BLOCK_DM_A1, pGameState->m_SpawnCounter++);
	}

	if(Spawn == vec2(-1, -1))
	{
		SendChat(pGameState, "[tdm] no block arena found.");
		OnRoundEnd(pGameState);
	}
	return Spawn;
}

bool CTdmBlock::PickSpawn(vec2 *pPos, CPlayer *pPlayer)
{
	if(!pPlayer->m_IsBlockTdming)
		return false;

	vec2 Pos = GetNextArenaSpawn(pPlayer->m_pBlockTdmState);
	if(Pos == vec2(-1, -1))
		return false;

	*pPos = Pos;
	return true;
}

void CTdmBlock::OnChatCmdTdm(CPlayer *pPlayer)
{
	if(IsActive(pPlayer->GetCid()))
	{
		SendChatTarget(pPlayer->GetCid(), "[tdm] you are already in a block team deathmatch.");
		return;
	}
	if(GameServer()->IsMinigaming(pPlayer->GetCid()))
	{
		SendChatTarget(pPlayer->GetCid(), "[tdm] you are already are in a minigame.");
		return;
	}

	Join(pPlayer);
	pPlayer->KillCharacter();
}

bool CTdmBlock::OnChatCmdLeave(CPlayer *pPlayer)
{
	if(!IsActive(pPlayer->GetCid()))
		return false;

	Leave(pPlayer);
	return true;
}

bool CTdmBlock::IsInLobby(CGameState *pGameState, CPlayer *pPlayer)
{
	if(!pPlayer)
		return false;
	if(!IsActive(pPlayer->GetCid()))
		return false;
	return pPlayer->m_pBlockTdmState == pGameState;
}

void CTdmBlock::OnRoundEnd(CGameState *pGameState)
{
	for(CPlayer *pPlayer : GameServer()->m_apPlayers)
	{
		if(!IsInLobby(pGameState, pPlayer))
			continue;

		Leave(pPlayer);
	}
}

void CTdmBlock::SendChat(CGameState *pGameState, const char *pMessage)
{
	for(CPlayer *pPlayer : GameServer()->m_apPlayers)
	{
		if(!IsInLobby(pGameState, pPlayer))
			continue;

		SendChatTarget(pPlayer->GetCid(), pMessage);
	}
}

void CTdmBlock::Join(CPlayer *pPlayer)
{
	// TODO: we could assert here that all these vars we init
	//       are not initied already to avoid logic issues of duped joins

	pPlayer->m_IsBlockTdming = true;
	pPlayer->m_MinigameScore = 0;

	// if we add multiple lobbies this has to create a new one
	pPlayer->m_pBlockTdmState = &m_GameState;
}

void CTdmBlock::Leave(CPlayer *pPlayer)
{
	pPlayer->m_IsBlockTdming = false;
	pPlayer->m_pBlockTdmState = nullptr;
}
