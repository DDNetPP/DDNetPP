// gamecontext scoped trade ddnet++ methods

#include <engine/shared/config.h>

#include "gamecontext.h"

int CGameContext::TradePrepareSell(const char *pToName, int FromID, const char *pItemName, int Price, bool IsPublic)
{
	CPlayer *pPlayer = m_apPlayers[FromID];
	if(!pPlayer)
		return -1;

	CCharacter *pChr = GetPlayerChar(FromID);
	if(!pChr)
	{
		SendChatTarget(FromID, "[TRADE] you have to be alive to use this command.");
		return -1;
	}

	char aBuf[256];

	if(pPlayer->m_TradeTick > Server()->Tick())
	{
		int TimeLeft = (pPlayer->m_TradeTick - Server()->Tick()) / Server()->TickSpeed();
		str_format(aBuf, sizeof(aBuf), "[TRADE] delay: %02d:%02d", TimeLeft / 60, TimeLeft % 60);
		SendChatTarget(FromID, aBuf);
		return -1;
	}

	if(pPlayer->IsLoggedIn()) //LOGGED IN ???
	{
		SendChatTarget(FromID, "[TRADE] you have to be logged in to use this command. Check '/accountinfo'");
		return -1;
	}

	int item = TradeItemToInt(pItemName); // ITEM EXIST ???
	if(item == -1)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "[TRADE] unknown item '%s' check '/trade items' for a full list.", pItemName);
		SendChatTarget(FromID, aBuf);
		return -1;
	}

	if(item == 2 && pPlayer->m_SpawnShotgunActive) // are items spawn weapons?
	{
		SendChatTarget(FromID, "[TRADE] you can't trade your spawn shotgun.");
		return -1;
	}
	if(item == 3 && pPlayer->m_SpawnGrenadeActive)
	{
		SendChatTarget(FromID, "[TRADE] you can't trade your spawn grenade.");
		return -1;
	}
	if(item == 4 && pPlayer->m_SpawnRifleActive)
	{
		SendChatTarget(FromID, "[TRADE] you can't trade your spawn rifle.");
		return -1;
	}
	if(item == 5 && (pPlayer->m_SpawnShotgunActive || pPlayer->m_SpawnGrenadeActive || pPlayer->m_SpawnRifleActive))
	{
		SendChatTarget(FromID, "[TRADE] you can't trade your spawn weapons.");
		return -1;
	}

	if(item == 2 && pChr->m_aDecreaseAmmo[WEAPON_SHOTGUN]) // do items have infinite ammo? (not a pickep up spawn weapon)
	{
		SendChatTarget(FromID, "[TRADE] you can't trade if your weapon doesn't have infinite bullets.");
		return -1;
	}
	if(item == 3 && pChr->m_aDecreaseAmmo[WEAPON_GRENADE])
	{
		SendChatTarget(FromID, "[TRADE] you can't trade if your weapon doesn't have infinite bullets.");
		return -1;
	}
	if(item == 4 && pChr->m_aDecreaseAmmo[WEAPON_LASER])
	{
		SendChatTarget(FromID, "[TRADE] you can't trade if your weapon doesn't have infinite bullets.");
		return -1;
	}
	if(item == 5 && (pChr->m_aDecreaseAmmo[WEAPON_SHOTGUN] || pChr->m_aDecreaseAmmo[WEAPON_GRENADE] || pChr->m_aDecreaseAmmo[WEAPON_LASER]))
	{
		SendChatTarget(FromID, "[TRADE] you can't trade if your weapons doesn't have infinite bullets.");
		return -1;
	}

	int HasItem = TradeHasItem(item, FromID); // ITEM OWNED ???
	if(HasItem == -1)
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] you don't own the item [ %s ]", pItemName);
		SendChatTarget(FromID, aBuf);
		return -1;
	}

	if(Price < 1) // TRADE MONEY TOO LOW ???
	{
		SendChatTarget(FromID, "[TRADE] the trade price has to be higher than zer0.");
		return -1;
	}

	if(!IsPublic) // private trade
	{
		return TradeSellCheckUser(pToName, FromID); // DOES THE USER EXIST ??? AND IS HE LOGGED IN ???
	}

	return 1;
}

int CGameContext::TradeSellCheckUser(const char *pToName, int FromID)
{
	char aBuf[128];
	int TradeID = GetCIDByName(pToName); //USER ONLINE ???
	if(TradeID == -1)
	{
		if(!str_comp_nocase(pToName, ""))
		{
			SendChatTarget(FromID, "[TRADE] Error: Missing username");
			return -1;
		}
		str_format(aBuf, sizeof(aBuf), "[TRADE] User '%s' not online", pToName);
		SendChatTarget(FromID, aBuf);
		return -1;
	}

	if(m_apPlayers[TradeID]->IsLoggedIn()) //USER LOGGED IN ???
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] player '%s' is not logged in.", pToName);
		SendChatTarget(FromID, aBuf);
		return -1;
	}
	return TradeID;
}

int CGameContext::TradePrepareBuy(int BuyerID, const char *pSellerName, int ItemID)
{
	CPlayer *pBPlayer = m_apPlayers[BuyerID]; // BUYER ONLINE ??
	if(!pBPlayer)
		return -1;

	char aBuf[128];
	int SellerID = GetCIDByName(pSellerName); // SELLER ONLINE ??
	if(SellerID == -1)
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] User '%s' not online.", pSellerName);
		SendChatTarget(BuyerID, aBuf);
		return -1;
	}

	CPlayer *pSPlayer = m_apPlayers[SellerID];
	if(!pSPlayer)
		return -1;

	CCharacter *pBChr = GetPlayerChar(BuyerID);
	CCharacter *pSChr = GetPlayerChar(SellerID);

	if(pBPlayer->IsLoggedIn()) // BUYER LOGGED IN ??
	{
		SendChatTarget(BuyerID, "[TRADE] you have to be logged in to use this command. Check '/accountinfo'");
		return -1;
	}

	if(pSPlayer->IsLoggedIn()) // SELLER LOGGED IN ??
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] player '%s' is not logged in.", pSellerName);
		SendChatTarget(BuyerID, aBuf);
		return -1;
	}

	if(!pBChr || !pSChr) // BOTH ALIVE ??
	{
		SendChatTarget(BuyerID, "[TRADE] both players have to be alive.");
		return -1;
	}

	if(BuyerID == SellerID) // SAME TEE ??
	{
		SendChatTarget(BuyerID, "[TRADE] you can't trade alone, lol");
		return -1;
	}

	if(pSPlayer->m_TradeMoney > pBPlayer->GetMoney()) // ENOUGH MONEY ??
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] %lld/%d money missing.", pBPlayer->GetMoney(), pSPlayer->m_TradeMoney);
		SendChatTarget(BuyerID, aBuf);
		return -1;
	}

	if(pSPlayer->m_TradeID != -1 && // PRIVATE TRADE ??
		pSPlayer->m_TradeID != BuyerID) // wrong private trade mate
	{
		SendChatTarget(BuyerID, "[TRADE] error, this trade is private.");
		return -1;
	}

	if(pSChr->HasWeapon(ItemID) || (ItemID == 5 && pSChr->HasWeapon(2) && pSChr->HasWeapon(3) && pSChr->HasWeapon(4)))
	{
		//has the weapons
	}
	else
	{
		SendChatTarget(BuyerID, "[TRADE] the seller doesn't own the item right now. try agian later.");
		return -1;
	}

	if(IsMinigame(SellerID))
	{
		SendChatTarget(BuyerID, "[TRADE] trade failed because seller is in jail or minigame.");
		return -1;
	}

	if(IsMinigame(BuyerID))
	{
		SendChatTarget(BuyerID, "[TRADE] trade failed because you are in jail or minigame.");
		return -1;
	}

	if(pSPlayer->m_SpawnShotgunActive && ItemID == 2)
	{
		SendChatTarget(BuyerID, "[TRADE] the wanted weapon is a spawn weapon and can't be bought.");
		return -1;
	}

	if(pSPlayer->m_SpawnGrenadeActive && ItemID == 3)
	{
		SendChatTarget(BuyerID, "[TRADE] the wanted weapon is a spawn weapon and can't be bought.");
		return -1;
	}

	if(pSPlayer->m_SpawnRifleActive && ItemID == 4)
	{
		SendChatTarget(BuyerID, "[TRADE] the wanted weapon is a spawn weapon and can't be bought.");
		return -1;
	}

	if(pSChr->m_aDecreaseAmmo[WEAPON_SHOTGUN] && ItemID == 2)
	{
		SendChatTarget(BuyerID, "[TRADE] the wanted weapon doesn't have infinite bullets and can't be bought.");
		return -1;
	}

	if(pSChr->m_aDecreaseAmmo[WEAPON_GRENADE] && ItemID == 3)
	{
		SendChatTarget(BuyerID, "[TRADE] the wanted weapon doesn't have infinite bullets and can't be bought.");
		return -1;
	}

	if(pSChr->m_aDecreaseAmmo[WEAPON_LASER] && ItemID == 4)
	{
		SendChatTarget(BuyerID, "[TRADE] the wanted weapon doesn't have infinite bullets and can't be bought.");
		return -1;
	}

	return 0;
}

/*
int CGameContext::TradeSellCheckItem(const char *pItemName, int FromID)
{

	if (!str_comp_nocase(pItemName, "shotgun"))   // OWN TRADE ITEM ???
	{
		if (pChr->HasWeapon(2))
		{
			item = 2;
		}
		else
		{
			SendChatTarget(FromID, "[TRADE] you don't own this item.");
			return -1;
		}
	}
	else if (!str_comp_nocase(pItemName, "grenade"))
	{
		if (pChr->HasWeapon(3))
		{
			item = 3;
		}
		else
		{
			SendChatTarget(FromID, "[TRADE] you don't own this item.");
			return -1;
		}
	}
	else if (!str_comp_nocase(pItemName, "rifle"))
	{
		if (pChr->HasWeapon(4))
		{
			item = 4;
		}
		else
		{
			SendChatTarget(FromID, "[TRADE] you don't own this item.");
			return -1;
		}
	}
	else if (!str_comp_nocase(pItemName, "all_weapons"))
	{
		if (pChr->HasWeapon(4) && pChr->HasWeapon(3) && pChr->HasWeapon(2))
		{
			item = 5;
		}
		else
		{
			SendChatTarget(FromID, "[TRADE] you don't own this item.");
			return -1;
		}
	}

	if (item == -1)
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] unknown item '%s' check '/trade items' for a full list.", pItemName);
		SendChatTarget(FromID, aBuf);
		return -1;
	}

	return item;
}
*/

int CGameContext::TradeItemToInt(const char *pItemName)
{
	int item = -1;

	if(!str_comp_nocase(pItemName, "shotgun"))
	{
		item = 2;
	}
	else if(!str_comp_nocase(pItemName, "grenade"))
	{
		item = 3;
	}
	else if(!str_comp_nocase(pItemName, "rifle"))
	{
		item = 4;
	}
	else if(!str_comp_nocase(pItemName, "all_weapons"))
	{
		item = 5;
	}
	return item;
}

const char *CGameContext::TradeItemToStr(int ItemID)
{
	if(ItemID == 2)
	{
		return "shotgun";
	}
	else if(ItemID == 3)
	{
		return "grenade";
	}
	else if(ItemID == 4)
	{
		return "rifle";
	}
	else if(ItemID == 5)
	{
		return "all_weapons";
	}
	return "(null)";
}

int CGameContext::TradeHasItem(int ItemID, int ID)
{
	CPlayer *pPlayer = m_apPlayers[ID];
	if(!pPlayer)
		return -1;

	CCharacter *pChr = GetPlayerChar(ID);
	if(!pChr)
		return -1;

	int item = -1;

	if(ItemID == 2) // shotgun
	{
		if(pChr->HasWeapon(2))
		{
			item = 2;
		}
	}
	else if(ItemID == 3) // grenade
	{
		if(pChr->HasWeapon(3))
		{
			item = 3;
		}
	}
	else if(ItemID == 4) // rifle
	{
		if(pChr->HasWeapon(4))
		{
			item = 4;
		}
	}
	else if(ItemID == 5) // all_weapons
	{
		if(pChr->HasWeapon(4) && pChr->HasWeapon(3) && pChr->HasWeapon(2))
		{
			item = 5;
		}
	}

	return item;
}