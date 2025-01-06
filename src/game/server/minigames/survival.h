// DDNet++ minigame

#ifndef GAME_SERVER_MINIGAMES_SURVIVAL_H
#define GAME_SERVER_MINIGAMES_SURVIVAL_H

#include "minigame_base.h"

class CSurvival : public CMinigame
{
public:
	using CMinigame::CMinigame;

	// void OnInit() override;
	// void Tick() override;
	// void SlowTick() override;
	// void PlayerTick(CPlayer *pPlayer) override;
	// void PlayerSlowTick(CPlayer *pPlayer) override;
	// void OnPlayerDisconnect(class CPlayer *pPlayer, const char *pReason) override;
	// void OnPlayerConnect(class CPlayer *pPlayer) override;
	// void CharacterTick(CCharacter *pChr) override;
	// bool AllowSelfKill(int ClientId) override;
	// void OnDeath(CCharacter *pChr, int Killer, int Weapon) override;
	// void PostSpawn(CCharacter *pChr) override;
	// bool PickSpawn(vec2 *pPos, CPlayer *pPlayer) override;
	// bool HandleCharacterTiles(CCharacter *pChr, int MapIndex) override;
	// bool OnChatCmdLeave(CPlayer *pPlayer) override;
	bool IsActive(int ClientId) override;

	// void Join(CPlayer *pPlayer);
	// void Leave(CPlayer *pPlayer);
};

#endif
