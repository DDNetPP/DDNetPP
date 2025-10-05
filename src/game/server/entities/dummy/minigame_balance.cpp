//Minigame the down left balance to secret moneyroom
// mode 30

#include "minigame_balance.h"

#include <base/math_ddpp.h>

#include <engine/shared/config.h>

#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos)*32)

CDummyMinigameBalance::CDummyMinigameBalance(class CPlayer *pPlayer) :
	CDummyBase(pPlayer, DUMMYMODE_MINIGAME_BALANCE1)
{
}

void CDummyMinigameBalance::OnTick()
{
	//rest dummy
	Hook(0);
	Jump(0);
	StopMoving();
	Fire(0);

	// could add a bit of random movement on initial fall here
	// and also a selfkill to avoid the balance battle taking forever
}
