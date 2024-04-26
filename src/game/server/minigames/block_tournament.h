// DDNet++ block tournaments

#ifndef GAME_SERVER_MINIGAMES_BLOCK_TOURNAMENT_H
#define GAME_SERVER_MINIGAMES_BLOCK_TOURNAMENT_H

#include "minigame_base.h"

#define BLOCKTOURNAMENT_COOLDOWN 6

class CBlockTournament : public CMinigame
{
public:
	using CMinigame::CMinigame;

	void OnInit() override;
	void Tick() override;
	void SlowTick() override;
	void CharacterTick(CCharacter *pChr) override;
	bool AllowSelfKill(int ClientId) override;
	void OnDeath(CCharacter *pChr, int Killer) override;
	void PostSpawn(CCharacter *pChr, vec2 Pos) override;
	bool PickSpawn(vec2 *pPos, CPlayer *pPlayer) override;

	bool IsActive(int ClientId) override;
	void Leave(CPlayer *pPlayer); // move to base?
	void Join(CPlayer *pPlayer);

	enum
	{
		STATE_OFF = 0,
		STATE_LOBBY,
		STATE_COOLDOWN,
		STATE_IN_GAME,
		STATE_ENDING,
	};
	void StartRound();

private:
	int CountAlive();
	void EndRound();
	vec2 GetNextArenaSpawn(int ClientId);

	/*
		m_StartPlayers

		The amount of players the current round started with
	*/
	int m_StartPlayers;

	int m_CoolDown;
	int m_Tick;
	int m_SpawnCounter; // is this generic enough for all games?
	int m_LobbyTick;
};

#endif
