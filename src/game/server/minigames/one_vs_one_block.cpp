#include <base/system.h>
#include <engine/shared/config.h>
#include <game/mapitems_ddpp.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/player.h>

#include "one_vs_one_block.h"

bool COneVsOneBlock::IsActive(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	return pPlayer->m_IsBlockOneVsOneing;
}

void COneVsOneBlock::OnDeath(CCharacter *pChr, int Killer)
{
	CPlayer *pPlayer = pChr->GetPlayer();
	if(!pPlayer->m_IsBlockOneVsOneing)
		return;

	CGameState *pState = pPlayer->m_pBlockOneVsOneState;
	dbg_assert(pState, "1vs1 death without game state");

	// TODO: is every death a score for the opponent?
	//       What about untouched unfrozen selfkills?

	CPlayer *pKiller = pState->OtherPlayer(pPlayer);
	if(pState->IsRunning())
		pKiller->m_MinigameScore++;
	PrintScoreBroadcast(pState);
}

// called after OnRoundStart()
void COneVsOneBlock::OnCountdownEnd(CGameState *pGameState)
{
	pGameState->m_State = CGameState::EState::RUNNING;
	PrintScoreBroadcast(pGameState);
}

// called before OnCountdownEnd()
void COneVsOneBlock::OnRoundStart(CPlayer *pPlayer1, CPlayer *pPlayer2)
{
	pPlayer1->m_BlockOneVsOneRequestedId = -1;
	pPlayer2->m_BlockOneVsOneRequestedId = -1;

	pPlayer1->m_IsBlockOneVsOneing = true;
	pPlayer2->m_IsBlockOneVsOneing = true;

	CGameState *pGameState = new CGameState(pPlayer1, pPlayer2);
	pGameState->m_State = CGameState::EState::COUNTDOWN;
	pGameState->m_CountDownTicksLeft = Server()->TickSpeed() * 10;
	pPlayer1->m_pBlockOneVsOneState = pGameState;
	pPlayer2->m_pBlockOneVsOneState = pGameState;

	pPlayer1->m_MinigameScore = 0;
	pPlayer2->m_MinigameScore = 0;

	// TODO: this needs a bunch of checks if we can even join the team
	//       for example ddnet only allows it to non spectator alive players
	int Team = GameServer()->m_pController->Teams().GetFirstEmptyTeam();

	const char *pError = nullptr;
	pError = Controller()->Teams().SetCharacterTeam(pPlayer1->GetCid(), Team);
	pError = Controller()->Teams().SetCharacterTeam(pPlayer2->GetCid(), Team);
	if(pError)
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "[1vs1] game aborted joining team failed: %s", pError);
		SendChatTarget(pPlayer1->GetCid(), aBuf);
		SendChatTarget(pPlayer2->GetCid(), aBuf);
		OnRoundEnd(pGameState);
		return;
	}
	Controller()->Teams().SetTeamLock(Team, true);
	pGameState->m_DDRaceTeam = Team;

	pPlayer1->KillCharacter();
	pPlayer2->KillCharacter();

	// set ddrace teams
	// make sure the players are not spectators and can be respawned
	// respawn them and let the spawn code handle teleportation
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

	pPlayer1->KillCharacter();
	pPlayer2->KillCharacter();
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
		return;

	int Score1 = pGameState->m_pPlayer1->m_MinigameScore;
	int Score2 = pGameState->m_pPlayer1->m_MinigameScore;

	// if nobody did a score yet the game has no winner
	if(!Score1 && !Score2)
	{
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
		SendChatTarget(pGameState->m_pPlayer1->GetCid(), "[1vs1] no block arena found.");
		SendChatTarget(pGameState->m_pPlayer2->GetCid(), "[1vs1] no block arena found.");
		OnRoundEnd(pGameState);
	}
	return Spawn;
}

bool COneVsOneBlock::PickSpawn(vec2 *pPos, CPlayer *pPlayer)
{
	if(!pPlayer->m_IsBlockOneVsOneing)
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

	if(pPlayer->m_BlockOneVsOneInviteExpire && time_get() > pPlayer->m_BlockOneVsOneInviteExpire)
	{
		pPlayer->m_BlockOneVsOneRequestedId = -1;
		const char *pName = Server()->ClientName(pPlayer->m_BlockOneVsOneRequestedId);
		str_format(aBuf, sizeof(aBuf), "[1vs1] your invite to '%s' expired.", pName);
		GameServer()->SendChatTarget(ClientId, aBuf);
	}
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
