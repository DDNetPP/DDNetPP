// dummy that chills in the shop tiles

#include "shopbot.h"

#include "../character.h"
#include <base/math_ddpp.h>
#include <engine/shared/config.h>
#include <game/server/ddpp/shop.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>

#define X (GetPos().x / 32)
#define Y (GetPos().y / 32)
#define RAW(pos) ((pos)*32)

CDummyShopBot::CDummyShopBot(class CCharacter *pChr, class CPlayer *pPlayer) :
	CDummyBase(pChr, pPlayer)
{
}

void CDummyShopBot::OnTick()
{
	Jump(0);
	Fire(0);
	Hook(0);
	StopMoving();

	CCharacter *pChr = GameServer()->m_World.ClosestCharType(GetPos(), false, m_pCharacter);
	if(pChr && pChr->IsAlive() && GameServer()->Shop()->IsInShop(pChr->GetPlayer()->GetCID()))
		AimPos(pChr->GetPos());

	if(m_pCharacter->m_IsFreeShopBot)
		if(Server()->Tick() % 500 == 0 && !GameServer()->Shop()->IsInShop(m_pPlayer->GetCID()))
			Die();
}
