// DDNet++ balance battles minigame

#ifndef GAME_SERVER_MINIGAMES_BALANCE_H
#define GAME_SERVER_MINIGAMES_BALANCE_H

#include "minigame_base.h"

class CBalance : public CMinigame
{
public:
	using CMinigame::CMinigame;

	// void OnInit() override;
	// void Tick() override;
	// void SlowTick() override;
	// void CharacterTick(CCharacter *pChr) override;
	// bool AllowSelfKill(int ClientId) override;
	// void OnDeath(CCharacter *pChr, int Killer) override;
	// void PostSpawn(CCharacter *pChr, vec2 Pos) override;
	// bool PickSpawn(vec2 *pPos, CPlayer *pPlayer) override;
	bool HandleCharacterTiles(CCharacter *pChr, int MapIndex) override;

	bool IsActive(int ClientId) override;
};

#endif
