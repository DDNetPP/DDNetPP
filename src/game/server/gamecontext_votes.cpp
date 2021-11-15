// gamecontext scoped votes ddnet++ methods

#include "gamemodes/DDRace.h"
#include <engine/shared/config.h>
#include <game/server/entities/flag.h>

#include <game/server/ddpp/shop.h>

#include "gamecontext.h"

void CGameContext::VotedYes(CCharacter *pChr, CPlayer *pPlayer)
{
	// SendChatTarget(ClientID, "you pressed f3");
	if(!pChr)
		return;
	if(Shop()->VoteYes(pChr->GetPlayer()->GetCID()))
		return;
	IGameController *ControllerDDrace = pPlayer->GetCharacter()->GameServer()->m_pController;
	if(((CGameControllerDDRace *)ControllerDDrace)->m_apFlags[0])
	{
		if(((CGameControllerDDRace *)ControllerDDrace)->m_apFlags[0]->m_pCarryingCharacter == pChr)
		{
			((CGameControllerDDRace *)ControllerDDrace)->DropFlag(0, pChr->GetAimDir()); //red
			//SendChatTarget(ClientID, "you dropped red flag");
		}
	}
	if(((CGameControllerDDRace *)ControllerDDrace)->m_apFlags[1])
	{
		if(((CGameControllerDDRace *)ControllerDDrace)->m_apFlags[1]->m_pCarryingCharacter == pChr)
		{
			((CGameControllerDDRace *)ControllerDDrace)->DropFlag(1, pChr->GetAimDir()); //blue
			//SendChatTarget(ClientID, "you dropped blue flag");
		}
	}
}

void CGameContext::VotedNo(CCharacter *pChr)
{
	//SendChatTarget(ClientID, "you pressed f4");
	if(!pChr)
		return;
	if(Shop()->VoteNo(pChr->GetPlayer()->GetCID()))
		return;
	if(g_Config.m_SvAllowDroppingWeapons)
		pChr->DropWeapon(pChr->GetActiveWeapon()); // drop the weapon youre holding.
}
