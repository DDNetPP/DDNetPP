#include <base/log.h>
#include <base/system.h>
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
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

#include "one_vs_one_block.h"

bool COneVsOneBlock::IsActive(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	return pPlayer->m_IsBlockOneVsOneing;
}

void COneVsOneBlock::OnDeath(CCharacter *pChr, int Killer, int Weapon)
{
	CPlayer *pPlayer = pChr->GetPlayer();
	if(!pPlayer->m_IsBlockOneVsOneing)
		return;

	CGameState *pState = pPlayer->m_pBlockOneVsOneState;
	dbg_assert(pState, "1vs1 death without game state");

	CPlayer *pKiller = pState->OtherPlayer(pPlayer);
	if(pState->IsRunning() && Killer == pKiller->GetCid() && Weapon != WEAPON_GAME && Weapon != WEAPON_MINIGAME)
	{
		// only count a draw if the other player is frozen and also in a freeze tile
		// checking the "in freeze" state is important
		// because it can happen that both tees hit a freeze roof and fall down
		// but only one would fall into freeze on the floor and the other one would unfreeze eventually
		// in that case the player who will fall into the freeze floor should not be able to
		// trigger a draw by selfkilling before the other one unfreezes
		if(pKiller->GetCharacter() && pKiller->GetCharacter()->m_FreezeTime && pKiller->GetCharacter()->Core()->m_IsInFreeze)
			SendChat(pState, "[1vs1] draw");
		else
			pKiller->m_MinigameScore++;
		if(pKiller->GetCharacter() && pKiller->GetCharacter()->IsAlive())
		{
			pKiller->GetCharacter()->Die(pKiller->GetCid(), WEAPON_MINIGAME);
			pKiller->Respawn();
		}
	}
	PrintScoreBroadcast(pState);
}

void COneVsOneBlock::OnTeleportSuccess(CGameState *pGameState, CPlayer *pPlayer)
{
	pPlayer->m_BlockOneVsOneTeleported = true;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(pChr)
	{
		// force join team
		pChr->m_DDRaceState = ERaceState::NONE;
	}

	if(!pGameState->m_DDRaceTeam)
		pGameState->m_DDRaceTeam = GameServer()->m_pController->Teams().GetFirstEmptyTeam();

	const char *pError = nullptr;
	Controller()->Teams().SetTeamLock(pGameState->m_DDRaceTeam, false);
	Controller()->Teams().ChangeTeamState(pGameState->m_DDRaceTeam, ETeamState::OPEN);
	pError = Controller()->Teams().SetCharacterTeam(pPlayer->GetCid(), pGameState->m_DDRaceTeam);
	if(pError)
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "[1vs1] game aborted joining team failed: %s", pError);
		SendChatTarget(pPlayer->GetCid(), aBuf);
		SendChatTarget(pGameState->OtherPlayer(pPlayer)->GetCid(), aBuf);
		OnRoundEnd(pGameState);
		return;
	}
	Controller()->Teams().SetTeamLock(pGameState->m_DDRaceTeam, true);

	pGameState->m_NumTeleportedPlayers++;
	if(pGameState->m_NumTeleportedPlayers == 2)
	{
		pGameState->m_State = CGameState::EState::COUNTDOWN;
	}
}

// called after OnRoundStart()
void COneVsOneBlock::OnCountdownEnd(CGameState *pGameState)
{
	pGameState->m_State = CGameState::EState::RUNNING;
	PrintScoreBroadcast(pGameState);

	CPlayer *apPlayers[] = {pGameState->m_pPlayer1, pGameState->m_pPlayer2};
	for(CPlayer *pPlayer : apPlayers)
	{
		if(pPlayer->GetCharacter())
		{
			pPlayer->GetCharacter()->Die(pPlayer->GetCid(), WEAPON_MINIGAME);
			pPlayer->Respawn();
		}
	}
}

bool COneVsOneBlock::AllowSelfKill(int ClientId)
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

void COneVsOneBlock::PostSpawn(CCharacter *pChr)
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

// called before OnCountdownEnd()
void COneVsOneBlock::OnRoundStart(CPlayer *pPlayer1, CPlayer *pPlayer2)
{
	pPlayer1->m_BlockOneVsOneRequestedId = -1;
	pPlayer2->m_BlockOneVsOneRequestedId = -1;

	pPlayer1->m_BlockOneVsOneTeleported = false;
	pPlayer2->m_BlockOneVsOneTeleported = false;

	char aBuf[512];
	CCharacter *pChr1 = pPlayer1->GetCharacter();
	CCharacter *pChr2 = pPlayer2->GetCharacter();
	CCharacter *apCharacters[] = {pChr1, pChr2};
	CPlayer *apPlayers[] = {pPlayer1, pPlayer2};
	for(CPlayer *pPlayer : apPlayers)
	{
		CCharacter *pChr = pPlayer->GetCharacter();
		if(!pChr || !pChr->IsAlive())
		{
			// TODO: can this be annoying? We could also delay the round start until the other player spawned
			str_format(aBuf, sizeof(aBuf), "[1vs1] game aborted because '%s' died", Server()->ClientName(pPlayer->GetCid()));
			SendChatTarget(pPlayer1->GetCid(), aBuf);
			SendChatTarget(pPlayer2->GetCid(), aBuf);
			return;
		}
		if(pPlayer->GetTeam() != TEAM_RED)
		{
			str_format(aBuf, sizeof(aBuf), "[1vs1] game aborted because '%s' is not in game", Server()->ClientName(pPlayer->GetCid()));
			SendChatTarget(pPlayer1->GetCid(), aBuf);
			SendChatTarget(pPlayer2->GetCid(), aBuf);
			return;
		}
	}

	pPlayer1->m_IsBlockOneVsOneing = true;
	pPlayer2->m_IsBlockOneVsOneing = true;

	CGameState *pGameState = new CGameState(pPlayer1, pPlayer2);
	pGameState->m_State = CGameState::EState::WAITING_FOR_PLAYERS;
	pGameState->m_CountDownTicksLeft = Server()->TickSpeed() * 10;
	pGameState->m_NumTeleportedPlayers = 0;
	pPlayer1->m_pBlockOneVsOneState = pGameState;
	pPlayer2->m_pBlockOneVsOneState = pGameState;

	pPlayer1->m_MinigameScore = 0;
	pPlayer2->m_MinigameScore = 0;

	for(CCharacter *pChr : apCharacters)
	{
		pChr->RequestTeleToTile(TILE_BLOCK_DM_A1, pGameState->m_SpawnCounter++)
			.DelayInSeconds(3)
			.OnPreSuccess([=, this]() {
				if(pChr->GetPlayer()->m_pBlockOneVsOneState == pGameState)
					SavePosition(pChr->GetPlayer());
			})
			.OnPostSuccess([=, this]() {
				if(pChr->GetPlayer()->m_pBlockOneVsOneState == pGameState)
					OnTeleportSuccess(pGameState, pChr->GetPlayer());
			})
			.OnFailure([=, this](const char *pShort, const char *pLong) {
				char aError[512];
				str_format(aError, sizeof(aError), "[1vs1] game aborted '%s' failed to teleport (%s)", Server()->ClientName(pChr->GetPlayer()->GetCid()), pShort);
				SendChatTarget(pPlayer1->GetCid(), aError);
				SendChatTarget(pPlayer2->GetCid(), aError);

				// this is seriously FUCKED
				// the pGameState pointer can already be deleted
				// by the time the OnFailure callback is run
				//
				// uglies hack ever to check if the game is still running
				// the current character is guranteed to exist because its tick
				// is triggering the failure
				// and its player only ever points to valid game states
				//
				// NOBODY CAN EVER SEE THIS CODE
				if(pChr->GetPlayer()->m_pBlockOneVsOneState == pGameState)
				{
					OnRoundEnd(pGameState);
				}
			});
	}
}

void COneVsOneBlock::OnRoundEnd(CGameState *pGameState)
{
	pGameState->m_State = CGameState::EState::ROUND_END;

	CPlayer *pPlayer1 = pGameState->m_pPlayer1;
	CPlayer *pPlayer2 = pGameState->m_pPlayer2;

	pPlayer1->m_IsBlockOneVsOneing = false;
	pPlayer2->m_IsBlockOneVsOneing = false;
	Controller()->Teams().SetTeamLock(pGameState->m_DDRaceTeam, false);

	dbg_assert(pPlayer1->m_pBlockOneVsOneState, "OnRoundEnd() was called with invalid gamestate");
	delete pPlayer1->m_pBlockOneVsOneState;
	pPlayer1->m_pBlockOneVsOneState = nullptr;
	pPlayer2->m_pBlockOneVsOneState = nullptr;

	CPlayer *apPlayers[] = {pPlayer1, pPlayer2};
	for(CPlayer *pPlayer : apPlayers)
	{
		if(pPlayer->GetCharacter())
		{
			if(pPlayer->GetCharacter()->m_TeleRequest.IsActive())
			{
				SendChatTarget(pPlayer->GetCid(), "[1vs1] teleportation request aborted because of game end.");
				pPlayer->GetCharacter()->m_TeleRequest.Abort();
			}
			if(pPlayer->m_BlockOneVsOneTeleported)
			{
				pPlayer->GetCharacter()->Die(pPlayer->GetCid(), WEAPON_MINIGAME);
				pPlayer->Respawn();
			}
		}

		if(pPlayer->m_BlockOneVsOneTeleported)
			m_aRestorePos[pPlayer->GetCid()] = true;
	}
}

void COneVsOneBlock::OnRoundWin(CGameState *pGameState, CPlayer *pWinner, CPlayer *pLoser, const char *pMessage)
{
	GameServer()->SendChat(-1, TEAM_ALL, pMessage);
	OnRoundEnd(pGameState);
}

void COneVsOneBlock::OnGameAbort(CGameState *pGameState, CPlayer *pAbortingPlayer, const char *pReason)
{
	// Can not win a game twice
	if(!pGameState->IsRunning())
	{
		// ensure count down games are shutdown properly
		if(pGameState->m_State != CGameState::EState::ROUND_END)
		{
			SendChat(pGameState, "[1vs1] game aborted before it started");
			OnRoundEnd(pGameState);
		}
		return;
	}

	int Score1 = pGameState->m_pPlayer1->m_MinigameScore;
	int Score2 = pGameState->m_pPlayer2->m_MinigameScore;

	// if nobody did a score yet the game has no winner
	if(!Score1 && !Score2)
	{
		SendChat(pGameState, "[1vs1] game aborted. No winners.");
		OnRoundEnd(pGameState);
		return;
	}

	CPlayer *pWinner = pGameState->OtherPlayer(pAbortingPlayer);
	const char *pWinnerName = Server()->ClientName(pWinner->GetCid());
	const char *pLoserName = Server()->ClientName(pAbortingPlayer->GetCid());

	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "[1vs1] '%s' won vs '%s' (%s)", pWinnerName, pLoserName, pReason);
	OnRoundWin(pGameState, pWinner, pAbortingPlayer, aBuf);
}

int COneVsOneBlock::ScoreLimit(CPlayer *pPlayer)
{
	if(!pPlayer)
		return 10;
	if(!pPlayer->m_pBlockOneVsOneState)
		return 10;
	return pPlayer->m_pBlockOneVsOneState->ScoreLimit();
}

void COneVsOneBlock::DoWincheck(CGameState *pGameState)
{
	// Can not win a game twice
	if(!pGameState->IsRunning())
		return;

	int Score1 = pGameState->m_pPlayer1->m_MinigameScore;
	int Score2 = pGameState->m_pPlayer2->m_MinigameScore;

	// if nobody reached the scorelimit yet there is no winner
	if(Score1 < pGameState->ScoreLimit() && Score2 < pGameState->ScoreLimit())
		return;

	char aBuf[512];
	const char *pName1 = Server()->ClientName(pGameState->m_pPlayer1->GetCid());
	const char *pName2 = Server()->ClientName(pGameState->m_pPlayer2->GetCid());

	CPlayer *pWinner = nullptr;
	if(Score1 > Score2)
	{
		pWinner = pGameState->m_pPlayer1;
		str_format(aBuf, sizeof(aBuf), "[1vs1] '%s' won vs '%s' (%d:%d)", pName1, pName2, Score1, Score2);
	}
	else if(Score1 < Score2)
	{
		pWinner = pGameState->m_pPlayer2;
		str_format(aBuf, sizeof(aBuf), "[1vs1] '%s' won vs '%s' (%d:%d)", pName2, pName1, Score2, Score1);
	}
	else
	{
		pGameState->m_State = CGameState::EState::SUDDEN_DEATH;
		PrintScoreBroadcast(pGameState);
		return;
	}

	CPlayer *pLoser = pGameState->OtherPlayer(pWinner);
	OnRoundWin(pGameState, pWinner, pLoser, aBuf);
}

vec2 COneVsOneBlock::GetNextArenaSpawn(CGameState *pGameState)
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
		SendChat(pGameState, "[1vs1] no block arena found.");
		OnRoundEnd(pGameState);
	}
	return Spawn;
}

bool COneVsOneBlock::PickSpawn(vec2 *pPos, CPlayer *pPlayer)
{
	if(!pPlayer->m_IsBlockOneVsOneing)
		return false;
	if(!pPlayer->m_BlockOneVsOneTeleported)
		return false;

	vec2 Pos = GetNextArenaSpawn(pPlayer->m_pBlockOneVsOneState);
	if(Pos == vec2(-1, -1))
		return false;

	*pPos = Pos;
	return true;
}

void COneVsOneBlock::OnChatCmdInvite(CPlayer *pPlayer, const char *pInvitedName)
{
	int ClientId = pPlayer->GetCid();
	const char *pName = Server()->ClientName(ClientId);

	if(!g_Config.m_SvAllowBlockOneVsOne)
	{
		SendChatTarget(ClientId, "[1vs1] this command is disabled by an administator.");
		return;
	}

	if(pPlayer->m_pBlockOneVsOneState)
	{
		SendChatTarget(ClientId, "[1vs1] you are already in a round do /leave first.");
		return;
	}

	char aBuf[512];
	int InvitedId = GameServer()->GetCidByName(pInvitedName);
	if(InvitedId == -1)
	{
		str_format(aBuf, sizeof(aBuf), "[1vs1] '%s' is not online.", pInvitedName);
		SendChatTarget(ClientId, aBuf);
		return;
	}
	if(InvitedId == pPlayer->GetCid())
	{
		SendChatTarget(pPlayer->GetCid(), "[1vs1] you can't invite your self.");
		return;
	}

	if(pPlayer->m_BlockOneVsOneRequestedId != -1)
	{
		str_format(aBuf, sizeof(aBuf), "[1vs1] your old request to '%s' was canceled because you sent a new request.", Server()->ClientName(pPlayer->m_BlockOneVsOneRequestedId));
		SendChatTarget(ClientId, aBuf);
	}
	pPlayer->m_BlockOneVsOneRequestedId = InvitedId;

	CPlayer *pInvitedPlayer = GameServer()->m_apPlayers[InvitedId];
	if(pInvitedPlayer->m_BlockOneVsOneRequestedId == ClientId)
	{
		str_format(aBuf, sizeof(aBuf), "[1vs1] you accepted '%s' invite.", pInvitedName);
		SendChatTarget(ClientId, aBuf);

		str_format(aBuf, sizeof(aBuf), "[1vs1] '%s' accepted your invite.", pName);
		SendChatTarget(InvitedId, aBuf);

		OnRoundStart(pPlayer, pInvitedPlayer);
	}
	else
	{
		str_format(aBuf, sizeof(aBuf), "[1vs1] '%s' invited you to a duel accpet using the chat command: /1vs1 %s", pName, pName);
		SendChatTarget(InvitedId, aBuf);

		str_format(aBuf, sizeof(aBuf), "[1vs1] you invited '%s' to a duel waiting for accept.", pInvitedName);
		SendChatTarget(ClientId, aBuf);

		// 2 minutes time to accept invite
		pPlayer->m_BlockOneVsOneInviteExpire = time_get() + time_freq() * 120;
	}
}

void COneVsOneBlock::PrintScoreBroadcast(CGameState *pGameState)
{
	char aBuf[512];
	char aTopText[512];
	aTopText[0] = '\0';
	if(pGameState->State() == CGameState::EState::COUNTDOWN)
	{
		// TODO: we can also use the actual game timer here with a snap hook
		int Seconds = pGameState->m_CountDownTicksLeft / Server()->TickSpeed();
		str_format(aTopText, sizeof(aTopText), "START IN %d", Seconds);
	}
	else if(pGameState->State() == CGameState::EState::SUDDEN_DEATH)
	{
		str_copy(aTopText, "SUDDEN DEATH");
	}
	else if(pGameState->State() == CGameState::EState::WAITING_FOR_PLAYERS)
	{
		str_copy(aTopText, "WAITING FOR PLAYERS TO TELEPORT");
	}

	str_format(
		aBuf,
		sizeof(aBuf),
		"%s                                                                                                                                  \n"
		"%s: %d                                                                                                                              \n"
		"%s: %d                                                                                                                                ",
		aTopText,
		Server()->ClientName(pGameState->m_pPlayer1->GetCid()),
		pGameState->m_pPlayer1->m_MinigameScore,
		Server()->ClientName(pGameState->m_pPlayer2->GetCid()),
		pGameState->m_pPlayer2->m_MinigameScore);
	GameServer()->SendBroadcast(aBuf, pGameState->m_pPlayer1->GetCid());
	GameServer()->SendBroadcast(aBuf, pGameState->m_pPlayer2->GetCid());

	for(CPlayer *pPlayer : GameServer()->m_apPlayers)
	{
		if(!pPlayer)
			continue;
		if(!(pPlayer->IsPaused() || pPlayer->GetTeam() == TEAM_SPECTATORS))
			continue;
		if(pPlayer->m_SpectatorId == SPEC_FREEVIEW)
			continue;
		if(!(pPlayer->m_SpectatorId == pGameState->m_pPlayer1->GetCid() || pPlayer->m_SpectatorId == pGameState->m_pPlayer2->GetCid()))
			continue;

		GameServer()->SendBroadcast(aBuf, pPlayer->GetCid());
	}
}

void COneVsOneBlock::PlayerTick(CPlayer *pPlayer)
{
	if(!pPlayer->m_IsBlockOneVsOneing)
		return;

	// this is called twice a tick for each participant once
	// that is a bit cursed but it works because a round can only be ended once
	DoWincheck(pPlayer->m_pBlockOneVsOneState);

	// has to be checked again because DoWincheck could have already ended the game
	// then we can not tick anymore
	if(!pPlayer->m_IsBlockOneVsOneing)
		return;

	CGameState *pGameState = pPlayer->m_pBlockOneVsOneState;
	dbg_assert(pGameState, "1vs1 without state");

	// don't tick if the other player disconnected in that tick
	if(!pGameState->m_pPlayer2)
		return;

	// GameTick
	// hack to only tick once per tick per game
	// and not once per player
	if(pPlayer == pGameState->m_pPlayer1)
	{
		CCharacter *pChr1 = pGameState->m_pPlayer1->GetCharacter();
		CCharacter *pChr2 = pGameState->m_pPlayer2->GetCharacter();
		if(pChr1 && pChr1->IsAlive() && pChr2 && pChr2->IsAlive())
		{
			if(pChr1->FrozenSinceSeconds() > 5 && pChr2->FrozenSinceSeconds() > 5)
			{
				SendChat(pGameState, "[1vs1] draw");
				pChr1->Die(pChr1->GetPlayer()->GetCid(), WEAPON_MINIGAME);
				pChr2->Die(pChr2->GetPlayer()->GetCid(), WEAPON_MINIGAME);
			}
		}
	}

	CCharacter *pChr = pPlayer->GetCharacter();
	CCharacter *pOtherChr = pGameState->OtherPlayer(pPlayer)->GetCharacter();

	if(pChr && pChr->IsAlive() && pChr->FrozenSinceSeconds() > 10 && pOtherChr->FrozenSinceSeconds() == 0)
	{
		// SendChat(pGameState, "[1vs1] force killed after 10s freeze");
		pChr->Die(pPlayer->GetCid(), WEAPON_WORLD);
	}

	if(pChr && pChr->IsAlive() && pChr->HookingSinceSeconds() > g_Config.m_SvOneVsOneAntiGroundHook && pChr->Core()->HookedPlayer() == -1)
	{
		SendChatTarget(pPlayer->GetCid(), GameServer()->Loc("Frozen by anti ground hook", pPlayer->GetCid()));
		pChr->Freeze();
	}

	bool PrintHud = false;
	if(pGameState->m_State == CGameState::EState::COUNTDOWN)
	{
		if(pGameState->m_CountDownTicksLeft)
			pGameState->m_CountDownTicksLeft--;
		if(pGameState->m_CountDownTicksLeft < 1)
		{
			pGameState->m_CountDownTicksLeft = 0;
			OnCountdownEnd(pGameState);
		}
		else if(((pGameState->m_CountDownTicksLeft + 1) % Server()->TickSpeed()) == 0)
		{
			PrintHud = true;
		}
	}

	if(PrintHud || Server()->Tick() % 120 == 0)
	{
		PrintScoreBroadcast(pGameState);
	}
}

void COneVsOneBlock::PlayerSlowTick(CPlayer *pPlayer)
{
	int ClientId = pPlayer->GetCid();
	char aBuf[512];

	if(pPlayer->m_BlockOneVsOneRequestedId != -1 && pPlayer->m_BlockOneVsOneInviteExpire && time_get() > pPlayer->m_BlockOneVsOneInviteExpire)
	{
		const char *pName = Server()->ClientName(pPlayer->m_BlockOneVsOneRequestedId);
		str_format(aBuf, sizeof(aBuf), "[1vs1] your invite to '%s' expired.", pName);
		GameServer()->SendChatTarget(ClientId, aBuf);
		pPlayer->m_BlockOneVsOneRequestedId = -1;
	}
}

void COneVsOneBlock::SendChat(CGameState *pGameState, const char *pMessage)
{
	dbg_assert(pGameState, "missing gamestate");

	SendChatTarget(pGameState->m_pPlayer1->GetCid(), pMessage);
	SendChatTarget(pGameState->m_pPlayer2->GetCid(), pMessage);
}

CPlayer *COneVsOneBlock::GetInviteSender(const CPlayer *pPlayer)
{
	for(CPlayer *pInvitingPlayer : GameServer()->m_apPlayers)
	{
		if(!pInvitingPlayer)
			continue;
		if(pInvitingPlayer->m_BlockOneVsOneRequestedId == pPlayer->GetCid())
			return pInvitingPlayer;
	}
	return nullptr;
}

bool COneVsOneBlock::OnChatCmdLeave(CPlayer *pPlayer)
{
	dbg_assert(pPlayer->m_pBlockOneVsOneState, "tried to leave game without state");
	OnGameAbort(pPlayer->m_pBlockOneVsOneState, pPlayer, "left the 1vs1");
	return true;
}

void COneVsOneBlock::OnPlayerDisconnect(class CPlayer *pPlayer, const char *pReason)
{
	char aBuf[512];
	CPlayer *pInvitingPlayer = GetInviteSender(pPlayer);
	if(pInvitingPlayer)
	{
		pInvitingPlayer->m_BlockOneVsOneRequestedId = -1;
		str_format(aBuf, sizeof(aBuf), "[1vs1] your invite to '%s' expired.", Server()->ClientName(pPlayer->GetCid()));
		GameServer()->SendChatTarget(pInvitingPlayer->GetCid(), aBuf);
	}

	if(pPlayer->m_IsBlockOneVsOneing)
	{
		OnGameAbort(pPlayer->m_pBlockOneVsOneState, pPlayer, "left the server");
	}
}
