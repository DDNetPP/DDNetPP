#ifndef GAME_SERVER_MINIGAMES_TDM_BLOCK_H
#define GAME_SERVER_MINIGAMES_TDM_BLOCK_H

// DDNet++ team deathmatch block minigame

#include "minigame_base.h"

#include <generated/protocol.h>

class CPlayer;
class CCharacter;
class CGameContext;

class CTdmBlock : public CMinigame
{
public:
	CTdmBlock(CGameContext *pGameContext);

	bool IsActive(int ClientId) override;
	bool AllowSelfKill(int ClientId) override;
	void OnDeath(CCharacter *pChr, int Killer, int Weapon) override;
	bool PickSpawn(vec2 *pPos, CPlayer *pPlayer) override;
	int ScoreLimit(CPlayer *pPlayer) override;
	void Snap(int SnappingClient) override;
	void SnapGameInfo(CPlayer *pPlayer, CNetObj_GameInfo *pGameInfo) override;
	void SnapGameData(CPlayer *pPlayer, CNetObj_GameData *pGameData) override;
	void OnPlayerDisconnect(class CPlayer *pPlayer, const char *pReason) override;
	void Tick() override;

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

		int m_aTeamscore[NUM_TEAMS] = {0};

		int m_DDRaceTeam = 0;
		int m_SpawnCounter = 0;
	};
	CGameState m_GameState;

	// this is for potential multi lobby support in the future
	std::vector<CGameState *> m_vpGameStates;

	// the pPlayer argument does not make much sense
	// but this is here in case in the future there will be multiple
	// tdm games at once and we need a player instance to find the
	// matching gamestate
	CGameState *GameState(const CPlayer *pPlayer) { return &m_GameState; }

	void Tick(CGameState *pGameState);

	void OnChatCmdTdm(CPlayer *pPlayer);
	bool OnChatCmdLeave(CPlayer *pPlayer) override;

	// find next spawn position
	// making sure players dont share spawns
	vec2 GetNextArenaSpawn(CGameState *pGameState);

	// returns true if pPlayer is in the lobby pGameState
	// can be used as IsActive alternative
	// and is a preparation for multi lobby support
	bool IsInLobby(CGameState *pGameState, CPlayer *pPlayer);

	// called when the round ends
	// used to cleanup all the game state
	void OnRoundEnd(CGameState *pGameState);

	void OnRoundStart(CGameState *pGameState);

	// send a chat message to all players in the tdm lobby
	void SendChat(CGameState *pGameState, const char *pMessage);

	// prints current game state info to all participants
	void PrintHudBroadcast(CGameState *pGameState);

	void Join(CPlayer *pPlayer);
	void Leave(CPlayer *pPlayer);
};

#endif
