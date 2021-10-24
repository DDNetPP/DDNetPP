// gamecontext scoped votes ddnet++ methods

#include "gamemodes/DDRace.h"
#include <engine/shared/config.h>
#include <game/server/entities/flag.h>

#include "gamecontext.h"

void CGameContext::VotedYes(CCharacter *pChr, CPlayer *pPlayer)
{
	// SendChatTarget(ClientID, "you pressed f3");

	if(pChr)
	{
		if(pChr->m_InShop)
		{
			if(pChr->m_PurchaseState == 1)
			{
				pChr->ConfirmPurchase();
			}
			else if(pChr->m_PurchaseState == 2)
			{
				pChr->PurchaseEnd(false);
			}
		}
		else
		{
			if(pChr)
			{
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
		}
	}
}

void CGameContext::VotedNo(CCharacter *pChr)
{
	//SendChatTarget(ClientID, "you pressed f4");

	if(pChr)
	{
		if(pChr->m_InShop)
		{
			if(pChr->m_PurchaseState == 2)
			{
				pChr->PurchaseEnd(true);
			}
			else if(pChr->m_ShopWindowPage == -1)
			{
				pChr->StartShop();
			}
		}
		else
		{
			if(g_Config.m_SvAllowDroppingWeapons)
			{
				pChr->DropWeapon(pChr->GetActiveWeapon()); // drop the weapon youre holding.
			}
		}
	}
}
