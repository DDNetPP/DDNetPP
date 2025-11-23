// sample dummy mode

#include "sample.h"

#include <game/server/gamecontext.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos) * 32)

CDummySample::CDummySample(class CPlayer *pPlayer) :
	CDummyBase(pPlayer, DUMMYMODE_DEFAULT)
{
}

void CDummySample::OnTick()
{
	/********************************
	 *           weapons             *
	 *********************************/
	SetWeapon(WEAPON_HAMMER);
	SetWeapon(WEAPON_GUN);
	SetWeapon(WEAPON_SHOTGUN);
	SetWeapon(WEAPON_GRENADE);
	SetWeapon(WEAPON_LASER);

	/********************************
	 *           aiming              *
	 ********************************/
	// right and down:
	Aim(200, 200);

	// left and up:
	Aim(-200, -200);

	// up:
	// straight up would be (0, -200) but we better avoid using 0 (can cause bugs)
	Aim(1, -200);

	/********************************
	 *           moving              *
	 *********************************/
	Left();
	Right();
	StopMoving();
	Hook(); // hook
	Hook(0); // do not hook

	/********************************
	 *           shooting            *
	 *********************************/
	Fire(); // shoot
	Fire(0); // stop shooting

	/********************************
	 *           conditions          *
	 *********************************/
	if(GetPos().x < 10 * 32)
	{
		// this code gets executed if the bot is in the first 10 left tiles of the map
		// the bot walks to the right until he reaches the x coordinate 10
		Right();
	}
	else
	{
		// this code gets executed if the bot is at x == 10 or higher
		// the bot would go back to 10 if he is too far
		Left();
	}

	/******************************
	 *    advanced stuff          *
	 ******************************/

	// jump when grounded
	if(IsGrounded())
		Jump();

	// is the same as if(GetPos().x < 10 * 32)
	// use the X macro to reduce typing effort
	// when working whith whole tiles
	if(X < 10)
		Right();

	// Get the closest player to the bot
	// save it in the varialbe pChr
	CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), false, m_pCharacter);

	// Check wether there is a player
	// And if it is alive
	if(pChr && pChr->IsAlive())
	{
		// if there is a player
		// aim at him
		AimPos(pChr->GetPos());
	}
}
