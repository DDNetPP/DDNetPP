// gamecontext scoped votes ddnet++ methods

#include "gamecontext.h"
#include "gamemodes/ddnet.h"

#include <engine/shared/config.h>

#include <game/server/ddpp/shop.h>
#include <game/server/entities/flag.h>

void CGameContext::VotedYes(CCharacter *pChr, CPlayer *pPlayer)
{
	// SendChatTarget(ClientId, "you pressed f3");
	if(!pChr)
		return;
	if(Shop()->VoteYes(pChr->GetPlayer()->GetCid()))
		return;

	IGameController *Controller = pPlayer->GetCharacter()->GameServer()->m_pController;
	if(Controller->CharacterDropFlag(pChr))
	{
		// SendChatTarget(ClientId, "you dropped the flag");
	}
}

void CGameContext::VotedNo(CCharacter *pChr)
{
	//SendChatTarget(ClientId, "you pressed f4");
	if(!pChr)
		return;
	if(Shop()->VoteNo(pChr->GetPlayer()->GetCid()))
		return;
	if(g_Config.m_SvAllowDroppingWeapons)
		pChr->DropWeapon(pChr->GetActiveWeapon()); // drop the weapon your holding.
}
