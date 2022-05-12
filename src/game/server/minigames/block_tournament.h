// DDNet** block tournaments

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
	void OnDeath(CCharacter *pChr, int Killer) override;
	void PostSpawn(CCharacter *pChr, vec2 Pos) override;
	bool PickSpawn(vec2 *pPos, CPlayer *pPlayer) override;

	bool IsActive(int ClientID) override;

	// m_BlockTournaSpawnCounter
	int m_SpawnCounter; // is this generic enough for all games?
	int m_LobbyTick;
	int m_CoolDown; // MAKE PRIVTAE

	void Leave(CPlayer *pPlayer); // move to base?

private:
	void EndRound();
	vec2 GetNextArenaSpawn(int ClientID);

	/*
		m_StartPlayers

		The amount of players the current round started with
	*/
	int m_StartPlayers;

	// TODO: m_Tick does the same as int64_t CGameContext::m_BlockTournaStart;
	int m_Tick; // TODO: add minigame init and zero it there
};

#endif
