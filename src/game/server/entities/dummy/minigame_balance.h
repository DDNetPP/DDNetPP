#ifndef GAME_SERVER_ENTITIES_DUMMY_MINIGAME_BALANCE_H
#define GAME_SERVER_ENTITIES_DUMMY_MINIGAME_BALANCE_H

#include "dummybase.h"

#include <base/vmath.h>

class CDummyMinigameBalance : public CDummyBase
{
public:
	CDummyMinigameBalance(class CPlayer *pPlayer);
	void OnTick() override;
	~CDummyMinigameBalance() override = default;
	const char *ModeStr() override { return "Minigame Balance"; }
};

#endif
