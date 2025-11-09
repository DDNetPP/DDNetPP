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
#include <game/server/minigames/minigame_base.h>
#include <game/server/player.h>
#include <game/server/teams.h>
#include <game/team_state.h>

CTdmBlock::CTdmBlock(CGameContext *pGameContext) :
	CMinigame(pGameContext)
{
	// we only ever have one lobby at a time for now
	m_vpGameStates.emplace_back(&m_GameState);
}

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
			pKiller->m_Minigame.m_Score++;
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

int CTdmBlock::ScoreLimit(CPlayer *pPlayer)
{
	if(!pPlayer)
		return 10;
	if(!GameState(pPlayer))
		return 10;
	return GameState(pPlayer)->ScoreLimit();
}

void CTdmBlock::OnPlayerDisconnect(class CPlayer *pPlayer, const char *pReason)
{
	Leave(pPlayer);
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

void CTdmBlock::Tick()
{
	for(CGameState *pGameState : m_vpGameStates)
		Tick(pGameState);
}

void CTdmBlock::Tick(CGameState *pGameState)
{
	switch(pGameState->m_State)
	{
	case CGameState::EState::WAITING_FOR_PLAYERS:
		// randomly hardcodet min amount of 2 players to start a round xd
		if(pGameState->m_NumTeleportedPlayers > 1)
		{
			pGameState->m_State = CGameState::EState::COUNTDOWN;
			pGameState->m_CountDownTicksLeft = Server()->TickSpeed() * 3;
		}
		break;
	case CGameState::EState::COUNTDOWN:
		pGameState->m_CountDownTicksLeft--;
		if(pGameState->m_CountDownTicksLeft < 1)
		{
			OnRoundStart(pGameState);
		}
		break;
	case CGameState::EState::RUNNING:
	case CGameState::EState::SUDDEN_DEATH:
	case CGameState::EState::ROUND_END:
		// we do nothing on tick for these
		break;
	}

	PrintHudBroadcast(pGameState);
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

void CTdmBlock::OnRoundStart(CGameState *pGameState)
{
	pGameState->m_State = CGameState::EState::RUNNING;
	for(CPlayer *pPlayer : GameServer()->m_apPlayers)
	{
		if(!IsInLobby(pGameState, pPlayer))
			continue;

		pPlayer->m_Minigame.Reset();
		pPlayer->m_FreezeOnSpawn = 3;
		pPlayer->KillCharacter();
		SendChatTarget(pPlayer->GetCid(), "[tdm] round is starting!");
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

void CTdmBlock::PrintHudBroadcast(CGameState *pGameState)
{
	char aBuf[512] = "";

	switch(pGameState->m_State)
	{
	case CGameState::EState::WAITING_FOR_PLAYERS:
		str_copy(aBuf, "Waiting for more players");
		break;
	case CGameState::EState::COUNTDOWN:
		// TODO: do not use broadcast for that, send a proper game timer
		str_format(aBuf, sizeof(aBuf), "Game starting in %.2f", (float)pGameState->m_CountDownTicksLeft / Server()->TickSpeed());
		break;
	case CGameState::EState::RUNNING:
	case CGameState::EState::SUDDEN_DEATH:
	case CGameState::EState::ROUND_END:
		// lets not spam hud all the time
		break;
	}

	if(aBuf[0] == '\0')
		return;

	for(CPlayer *pPlayer : GameServer()->m_apPlayers)
	{
		if(!IsInLobby(pGameState, pPlayer))
			continue;

		GameServer()->SendBroadcast(aBuf, pPlayer->GetCid());
	}
}

void CTdmBlock::Join(CPlayer *pPlayer)
{
	// TODO: we could assert here that all these vars we init
	//       are not initied already to avoid logic issues of duped joins

	pPlayer->m_IsBlockTdming = true;
	pPlayer->m_Minigame.Reset();

	// if we add multiple lobbies this has to create a new one
	pPlayer->m_pBlockTdmState = &m_GameState;

	// TODO: this should actually be only incremented after teleport
	//       but we do not teleport yet with cooldown we just respawn into the area
	//       if this is changed also on leave we need to only decrement if teleport happend
	pPlayer->m_pBlockTdmState->m_NumTeleportedPlayers++;
}

void CTdmBlock::Leave(CPlayer *pPlayer)
{
	if(!IsActive(pPlayer->GetCid()))
		return;

	pPlayer->m_pBlockTdmState->m_NumTeleportedPlayers--;
	pPlayer->m_IsBlockTdming = false;
	pPlayer->m_pBlockTdmState = nullptr;
}
