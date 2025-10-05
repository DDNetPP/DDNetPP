// dummymode -7
// DUMMYMODE_ADVENTURE
//
// vanilla area pvp bot
// set the config sv_adventure_bots to the amount of bots you want
// they will spawn at TILE_BOTSPAWN_1

#include "adventure.h"

#include "../character.h"

#include <base/math_ddpp.h>

#include <engine/shared/config.h>

#include <game/server/ddpp/shop.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos)*32)

CDummyAdventure::CDummyAdventure(class CPlayer *pPlayer) :
	CDummyBase(pPlayer, DUMMYMODE_ADVENTURE)
{
}

void CDummyAdventure::OnTick()
{
	Jump(0);
	Fire(0);
	StopMoving();

	if(rand() % 20 > 17)
		SetDirection(rand() % 3 - 1);
	if(rand() % 660 > 656)
		Jump();
	if(rand() % 256 > 244)
		Hook(rand() % 2);
	if(rand() % 1000 > 988)
		Fire();

	CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), false, m_pCharacter);
	if(pChr && pChr->IsAlive())
	{
		if(m_IsAimbot)
		{
			vec2 EnemyPos = pChr->GetPos();
			EnemyPos.y -= 20;
			AimPos(EnemyPos);

			// don't shoot walls
			if(!GameServer()->Collision()->IntersectLine(GetPos(), pChr->GetPos(), 0x0, 0))
				if(Server()->Tick() % 77 == 0 && m_pCharacter->m_FreezeTime < 1)
					Fire();
			m_IsAimbot = rand() % 50 > 40;
		}
		else // no aimbot
		{
			m_IsAimbot = rand() % 3000 > 2789;
		}
	}

	if(Server()->Tick() % 500 == 0 && IsFrozen())
		Die();
}
