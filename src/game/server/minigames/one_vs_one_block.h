// DDNet++ minigame

#ifndef GAME_SERVER_MINIGAMES_ONE_VS_ONE_BLOCK_H
#define GAME_SERVER_MINIGAMES_ONE_VS_ONE_BLOCK_H

#include "minigame_base.h"

class COneVsOneBlock : public CMinigame
{
public:
	using CMinigame::CMinigame;

	// void OnInit() override;
	// void Tick() override;
	// void SlowTick() override;
	void PlayerTick(CPlayer *pPlayer) override;
	void PlayerSlowTick(CPlayer *pPlayer) override;
	void OnPlayerDisconnect(class CPlayer *pPlayer, const char *pReason) override;
	// void OnPlayerConnect(class CPlayer *pPlayer) override;
	// void CharacterTick(CCharacter *pChr) override;
	bool AllowSelfKill(int ClientId) override;
	void OnDeath(CCharacter *pChr, int Killer, int Weapon) override;
	void PostSpawn(CCharacter *pChr) override;
	bool PickSpawn(vec2 *pPos, CPlayer *pPlayer) override;
	// bool HandleCharacterTiles(CCharacter *pChr, int MapIndex) override;
	int ScoreLimit(CPlayer *pPlayer) override;

	// score is counted in CPlayer::m_Minigame::m_Score
	class CGameState
	{
	public:
		enum class EState
		{
			// waiting for both players to teleport into the area
			WAITING_FOR_PLAYERS,

			// give players time to prepare
			// scores will be reset on countdown end
			// will change to RUNNING when m_CountDownTicksLeft reaches 0
			COUNTDOWN,

			// game is running and scores can be made
			RUNNING,

			// both players reached the scorelimit
			// in the same tick
			//
			// next score wins
			SUDDEN_DEATH,

			// the game is ending the winners were already announced
			ROUND_END,
		};
		EState m_State = EState::WAITING_FOR_PLAYERS;
		EState State() const { return m_State; }
		bool IsRunning() const { return m_State == EState::RUNNING || m_State == EState::SUDDEN_DEATH; }
		int ScoreLimit() { return 10; }
		int m_CountDownTicksLeft = 0;
		int m_NumTeleportedPlayers = 0;

		int m_DDRaceTeam = 0;
		int m_SpawnCounter = 0;

		CPlayer *m_pPlayer1 = nullptr;
		CPlayer *m_pPlayer2 = nullptr;

		CGameState(CPlayer *pPlayer1, CPlayer *pPlayer2)
		{
			m_pPlayer1 = pPlayer1;
			m_pPlayer2 = pPlayer2;
		}

		CPlayer *OtherPlayer(CPlayer *pPlayer)
		{
			return pPlayer == m_pPlayer1 ? m_pPlayer2 : m_pPlayer1;
		}
	};

	// send a chat message to both players
	void SendChat(CGameState *pGameState, const char *pMessage);

	// send a chat message to both players, localized
	// also prepends the "[1vs1]" prefix
	void SendChatLoc(CGameState *pGameState, const char *pMessage);

	// given the player that received the invite
	// find the player that sent the invite
	CPlayer *GetInviteSender(const CPlayer *pPlayer);

	// pPlayer is the player that ran the /1vs1 chat command
	// and pInvitedName is the argument passed to the chat command
	void OnChatCmdInvite(CPlayer *pPlayer, const char *pInvitedName);

	bool OnChatCmdLeave(CPlayer *pPlayer) override;

	// prints the broadcast with the current score
	// to both participants in the pGameState
	void PrintScoreBroadcast(CGameState *pGameState);

	// player teleported into the arena
	void OnTeleportSuccess(CGameState *pGameState, CPlayer *pPlayer);

	// game will start running and scores can be made
	void OnCountdownEnd(CGameState *pGameState);

	// called when a 1vs1 is starting
	// used to init all the game state
	// only enters the countdown phase
	// the real game starts after that
	// if you need the real game start
	// look into `OnCountdownEnd()`
	void OnRoundStart(CPlayer *pPlayer1, CPlayer *pPlayer2);

	// called when the round ends
	// used to cleanup all the game state
	void OnRoundEnd(CGameState *pGameState);

	// Checks if someone won and then ends the game
	// and announces the winner
	// can be called multiple times per tick because it only fires once
	void DoWincheck(CGameState *pGameState);

	// Is not called if the game ends with 0:0
	// In all other cases a winner is picked.
	// This function then announces the winner in the chat and rewards him.
	// Here we could also post the stats to discord.
	void OnRoundWin(CGameState *pGameState, CPlayer *pWinner, CPlayer *pLoser, const char *pMessage);

	// Alternative end to scorelimit reach caught by `DoWincheck()`
	// Will be called when one player leaves the game.
	void OnGameAbort(CGameState *pGameState, CPlayer *pAbortingPlayer, const char *pReason);

	// find next spawn position
	// making sure players dont share spawns
	vec2 GetNextArenaSpawn(CGameState *pGameState);

	bool IsActive(int ClientId) override;
};

#endif
