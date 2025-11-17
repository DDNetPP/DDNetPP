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
	CPlayer *pPlayer = GameServer()->GetPlayerOrNullptr(ClientId);
	if(!pPlayer)
		return false;
	return pPlayer->m_IsBlockTdming;
}

void CTdmBlock::OnDeath(CCharacter *pChr, int Killer, int Weapon)
{
	CPlayer *pPlayer = pChr->GetPlayer();
	if(!IsActive(pPlayer->GetCid()))
		return;

	if(Weapon != WEAPON_GAME && Weapon != WEAPON_MINIGAME)
	{
		CPlayer *pKiller = GameServer()->GetPlayerOrNullptr(Killer);
		if(pKiller && pChr->GetId() != Killer)
		{
			pKiller->m_Minigame.m_Score++;
			pPlayer->m_pBlockTdmState->m_aTeamscore[pKiller->m_Minigame.m_Team]++;
		}
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

void CTdmBlock::SnapGameInfo(CPlayer *pPlayer, CNetObj_GameInfo *pGameInfo)
{
	pGameInfo->m_GameFlags |= GAMEFLAG_TEAMS;

	CGameState *pGameState = pPlayer->m_pBlockTdmState;
	if(pGameState->m_State == CGameState::EState::ROUND_END)
		pGameInfo->m_GameStateFlags |= GAMESTATEFLAG_GAMEOVER;
	if(pGameState->m_State == CGameState::EState::SUDDEN_DEATH)
		pGameInfo->m_GameStateFlags |= GAMESTATEFLAG_SUDDENDEATH;
}

void CTdmBlock::Snap(int SnappingClient)
{
	if(!IsActive(SnappingClient))
		return;
	CPlayer *pPlayer = GameServer()->GetPlayerOrNullptr(SnappingClient);
	if(!pPlayer)
		return;

	CGameState *pGameState = pPlayer->m_pBlockTdmState;
	if(Server()->IsSixup(SnappingClient))
	{
		protocol7::CNetObj_GameDataTeam *pGameDataTeam = static_cast<protocol7::CNetObj_GameDataTeam *>(Server()->SnapNewItem(-protocol7::NETOBJTYPE_GAMEDATATEAM, 0, sizeof(protocol7::CNetObj_GameDataTeam)));
		if(!pGameDataTeam)
			return;

		pGameDataTeam->m_TeamscoreRed = pGameState->m_aTeamscore[TEAM_RED];
		pGameDataTeam->m_TeamscoreBlue = pGameState->m_aTeamscore[TEAM_BLUE];
	}
}

void CTdmBlock::SnapGameData(CPlayer *pPlayer, CNetObj_GameData *pGameData)
{
	CGameState *pGameState = pPlayer->m_pBlockTdmState;
	pGameData->m_TeamscoreRed = pGameState->m_aTeamscore[TEAM_RED];
	pGameData->m_TeamscoreBlue = pGameState->m_aTeamscore[TEAM_BLUE];
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
	case CGameState::EState::ROUND_END:
		pGameState->m_CountDownTicksLeft--;
		if(pGameState->m_CountDownTicksLeft < 1)
		{
			// TODO: should clear scores here
			pGameState->m_State = CGameState::EState::WAITING_FOR_PLAYERS;
		}
	case CGameState::EState::SUDDEN_DEATH:
	case CGameState::EState::RUNNING:
		// we do nothing on tick for these
		break;
	}

	PrintHudBroadcast(pGameState);
	DoWincheck(pGameState);
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
	mem_zero(pGameState->m_aTeamscore, sizeof(pGameState->m_aTeamscore));
	for(CPlayer *pPlayer : GameServer()->m_apPlayers)
	{
		if(!IsInLobby(pGameState, pPlayer))
			continue;

		pPlayer->m_Minigame.m_Score = 0;
		pPlayer->m_FreezeOnSpawn = 3;
		pPlayer->KillCharacter();
		SendChatTarget(pPlayer->GetCid(), "[tdm] round is starting!");
	}
}

void CTdmBlock::DoWincheck(CGameState *pGameState)
{
	if(!pGameState->IsRunning())
		return;

	// TODO: support draw/sudden death
	//       can happen if both teams do the final score point
	//       in the same tick
	//       right now we just let red win in that rare edge case :D

	for(int Team = TEAM_RED; Team < NUM_TEAMS; Team++)
	{
		if(pGameState->m_aTeamscore[Team] >= pGameState->ScoreLimit())
		{
			OnWin(pGameState, Team);
			return;
		}
	}
}

void CTdmBlock::OnWin(CGameState *pGameState, int WinnerTeam)
{
	if(pGameState->m_State == CGameState::EState::ROUND_END)
		return;

	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "[tdm] %s team won!", WinnerTeam == TEAM_RED ? "red" : "blue");
	SendChat(pGameState, aBuf);

	pGameState->m_State = CGameState::EState::ROUND_END;
	pGameState->m_CountDownTicksLeft = Server()->TickSpeed() * 5;
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

	// if we add multiple lobbies this has to create a new one
	pPlayer->m_pBlockTdmState = &m_GameState;

	// TODO: this should actually be only incremented after teleport
	//       but we do not teleport yet with cooldown we just respawn into the area
	//       if this is changed also on leave we need to only decrement if teleport happend
	pPlayer->m_pBlockTdmState->m_NumTeleportedPlayers++;

	pPlayer->m_IsBlockTdming = true;
	pPlayer->m_Minigame.Reset();
	pPlayer->m_Minigame.m_Team = pPlayer->m_pBlockTdmState->m_NumTeleportedPlayers % 2 ? TEAM_RED : TEAM_BLUE;
}

void CTdmBlock::Leave(CPlayer *pPlayer)
{
	if(!IsActive(pPlayer->GetCid()))
		return;

	pPlayer->m_pBlockTdmState->m_NumTeleportedPlayers--;
	pPlayer->m_IsBlockTdming = false;
	pPlayer->m_pBlockTdmState = nullptr;
}
