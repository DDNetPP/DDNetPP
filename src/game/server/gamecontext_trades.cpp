// gamecontext scoped trade ddnet++ methods

#include "gamecontext.h"

#include <engine/shared/config.h>

#include <cinttypes>

int CGameContext::TradePrepareSell(const char *pToName, int FromId, const char *pItemName, int Price, bool IsPublic)
{
	CPlayer *pPlayer = m_apPlayers[FromId];
	if(!pPlayer)
		return -1;

	CCharacter *pChr = GetPlayerChar(FromId);
	if(!pChr)
	{
		SendChatTarget(FromId, "[TRADE] you have to be alive to use this command.");
		return -1;
	}

	char aBuf[256];

	if(pPlayer->m_TradeTick > Server()->Tick())
	{
		int TimeLeft = (pPlayer->m_TradeTick - Server()->Tick()) / Server()->TickSpeed();
		str_format(aBuf, sizeof(aBuf), "[TRADE] delay: %02d:%02d", TimeLeft / 60, TimeLeft % 60);
		SendChatTarget(FromId, aBuf);
		return -1;
	}

	if(pPlayer->IsLoggedIn()) //LOGGED IN ???
	{
		SendChatTarget(FromId, "[TRADE] you have to be logged in to use this command. Check '/accountinfo'");
		return -1;
	}

	int Item = TradeItemToInt(pItemName); // ITEM EXIST ???
	if(Item == -1)
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] unknown item '%s' check '/trade items' for a full list.", pItemName);
		SendChatTarget(FromId, aBuf);
		return -1;
	}

	if(Item == 2 && pPlayer->m_SpawnShotgunActive) // are items spawn weapons?
	{
		SendChatTarget(FromId, "[TRADE] you can't trade your spawn shotgun.");
		return -1;
	}
	if(Item == 3 && pPlayer->m_SpawnGrenadeActive)
	{
		SendChatTarget(FromId, "[TRADE] you can't trade your spawn grenade.");
		return -1;
	}
	if(Item == 4 && pPlayer->m_SpawnRifleActive)
	{
		SendChatTarget(FromId, "[TRADE] you can't trade your spawn rifle.");
		return -1;
	}
	if(Item == 5 && (pPlayer->m_SpawnShotgunActive || pPlayer->m_SpawnGrenadeActive || pPlayer->m_SpawnRifleActive))
	{
		SendChatTarget(FromId, "[TRADE] you can't trade your spawn weapons.");
		return -1;
	}

	if(Item == 2 && pChr->m_aDecreaseAmmo[WEAPON_SHOTGUN]) // do items have infinite ammo? (not a pickep up spawn weapon)
	{
		SendChatTarget(FromId, "[TRADE] you can't trade if your weapon doesn't have infinite bullets.");
		return -1;
	}
	if(Item == 3 && pChr->m_aDecreaseAmmo[WEAPON_GRENADE])
	{
		SendChatTarget(FromId, "[TRADE] you can't trade if your weapon doesn't have infinite bullets.");
		return -1;
	}
	if(Item == 4 && pChr->m_aDecreaseAmmo[WEAPON_LASER])
	{
		SendChatTarget(FromId, "[TRADE] you can't trade if your weapon doesn't have infinite bullets.");
		return -1;
	}
	if(Item == 5 && (pChr->m_aDecreaseAmmo[WEAPON_SHOTGUN] || pChr->m_aDecreaseAmmo[WEAPON_GRENADE] || pChr->m_aDecreaseAmmo[WEAPON_LASER]))
	{
		SendChatTarget(FromId, "[TRADE] you can't trade if your weapons doesn't have infinite bullets.");
		return -1;
	}

	int HasItem = TradeHasItem(Item, FromId); // ITEM OWNED ???
	if(HasItem == -1)
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] you don't own the item [ %s ]", pItemName);
		SendChatTarget(FromId, aBuf);
		return -1;
	}

	if(Price < 1) // TRADE MONEY TOO LOW ???
	{
		SendChatTarget(FromId, "[TRADE] the trade price has to be higher than zer0.");
		return -1;
	}

	if(!IsPublic) // private trade
	{
		return TradeSellCheckUser(pToName, FromId); // DOES THE USER EXIST ??? AND IS HE LOGGED IN ???
	}

	return 1;
}

int CGameContext::TradeSellCheckUser(const char *pToName, int FromId)
{
	char aBuf[128];
	int TradeId = GetCidByName(pToName); //USER ONLINE ???
	if(TradeId == -1)
	{
		if(!str_comp_nocase(pToName, ""))
		{
			SendChatTarget(FromId, "[TRADE] Error: Missing username");
			return -1;
		}
		str_format(aBuf, sizeof(aBuf), "[TRADE] User '%s' not online", pToName);
		SendChatTarget(FromId, aBuf);
		return -1;
	}

	if(m_apPlayers[TradeId]->IsLoggedIn()) //USER LOGGED IN ???
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] player '%s' is not logged in.", pToName);
		SendChatTarget(FromId, aBuf);
		return -1;
	}
	return TradeId;
}

int CGameContext::TradePrepareBuy(int BuyerId, const char *pSellerName, int ItemId)
{
	CPlayer *pBPlayer = m_apPlayers[BuyerId]; // BUYER ONLINE ??
	if(!pBPlayer)
		return -1;

	char aBuf[128];
	int SellerId = GetCidByName(pSellerName); // SELLER ONLINE ??
	if(SellerId == -1)
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] User '%s' not online.", pSellerName);
		SendChatTarget(BuyerId, aBuf);
		return -1;
	}

	CPlayer *pSPlayer = m_apPlayers[SellerId];
	if(!pSPlayer)
		return -1;

	CCharacter *pBChr = GetPlayerChar(BuyerId);
	CCharacter *pSChr = GetPlayerChar(SellerId);

	if(pBPlayer->IsLoggedIn()) // BUYER LOGGED IN ??
	{
		SendChatTarget(BuyerId, "[TRADE] you have to be logged in to use this command. Check '/accountinfo'");
		return -1;
	}

	if(pSPlayer->IsLoggedIn()) // SELLER LOGGED IN ??
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] player '%s' is not logged in.", pSellerName);
		SendChatTarget(BuyerId, aBuf);
		return -1;
	}

	if(!pBChr || !pSChr) // BOTH ALIVE ??
	{
		SendChatTarget(BuyerId, "[TRADE] both players have to be alive.");
		return -1;
	}

	if(BuyerId == SellerId) // SAME TEE ??
	{
		SendChatTarget(BuyerId, "[TRADE] you can't trade alone, lol");
		return -1;
	}

	if(pSPlayer->m_TradeMoney > pBPlayer->GetMoney()) // ENOUGH MONEY ??
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] %" PRId64 "/%d money missing.", pBPlayer->GetMoney(), pSPlayer->m_TradeMoney);
		SendChatTarget(BuyerId, aBuf);
		return -1;
	}

	if(pSPlayer->m_TradeId != -1 && // PRIVATE TRADE ??
		pSPlayer->m_TradeId != BuyerId) // wrong private trade mate
	{
		SendChatTarget(BuyerId, "[TRADE] error, this trade is private.");
		return -1;
	}

	if(pSChr->HasWeapon(ItemId) || (ItemId == 5 && pSChr->HasWeapon(2) && pSChr->HasWeapon(3) && pSChr->HasWeapon(4)))
	{
		//has the weapons
	}
	else
	{
		SendChatTarget(BuyerId, "[TRADE] the seller doesn't own the item right now. try again later.");
		return -1;
	}

	if(IsMinigame(SellerId))
	{
		SendChatTarget(BuyerId, "[TRADE] trade failed because seller is in jail or minigame.");
		return -1;
	}

	if(IsMinigame(BuyerId))
	{
		SendChatTarget(BuyerId, "[TRADE] trade failed because you are in jail or minigame.");
		return -1;
	}

	if(pSPlayer->m_SpawnShotgunActive && ItemId == 2)
	{
		SendChatTarget(BuyerId, "[TRADE] the wanted weapon is a spawn weapon and can't be bought.");
		return -1;
	}

	if(pSPlayer->m_SpawnGrenadeActive && ItemId == 3)
	{
		SendChatTarget(BuyerId, "[TRADE] the wanted weapon is a spawn weapon and can't be bought.");
		return -1;
	}

	if(pSPlayer->m_SpawnRifleActive && ItemId == 4)
	{
		SendChatTarget(BuyerId, "[TRADE] the wanted weapon is a spawn weapon and can't be bought.");
		return -1;
	}

	if(pSChr->m_aDecreaseAmmo[WEAPON_SHOTGUN] && ItemId == 2)
	{
		SendChatTarget(BuyerId, "[TRADE] the wanted weapon doesn't have infinite bullets and can't be bought.");
		return -1;
	}

	if(pSChr->m_aDecreaseAmmo[WEAPON_GRENADE] && ItemId == 3)
	{
		SendChatTarget(BuyerId, "[TRADE] the wanted weapon doesn't have infinite bullets and can't be bought.");
		return -1;
	}

	if(pSChr->m_aDecreaseAmmo[WEAPON_LASER] && ItemId == 4)
	{
		SendChatTarget(BuyerId, "[TRADE] the wanted weapon doesn't have infinite bullets and can't be bought.");
		return -1;
	}

	return 0;
}

/*
int CGameContext::TradeSellCheckItem(const char *pItemName, int FromId)
{

	if (!str_comp_nocase(pItemName, "shotgun"))   // OWN TRADE ITEM ???
	{
		if (pChr->HasWeapon(2))
		{
			item = 2;
		}
		else
		{
			SendChatTarget(FromId, "[TRADE] you don't own this item.");
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
			SendChatTarget(FromId, "[TRADE] you don't own this item.");
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
			SendChatTarget(FromId, "[TRADE] you don't own this item.");
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
			SendChatTarget(FromId, "[TRADE] you don't own this item.");
			return -1;
		}
	}

	if (item == -1)
	{
		str_format(aBuf, sizeof(aBuf), "[TRADE] unknown item '%s' check '/trade items' for a full list.", pItemName);
		SendChatTarget(FromId, aBuf);
		return -1;
	}

	return item;
}
*/

int CGameContext::TradeItemToInt(const char *pItemName)
{
	int Item = -1;

	if(!str_comp_nocase(pItemName, "shotgun"))
	{
		Item = 2;
	}
	else if(!str_comp_nocase(pItemName, "grenade"))
	{
		Item = 3;
	}
	else if(!str_comp_nocase(pItemName, "rifle"))
	{
		Item = 4;
	}
	else if(!str_comp_nocase(pItemName, "all_weapons"))
	{
		Item = 5;
	}
	return Item;
}

const char *CGameContext::TradeItemToStr(int ItemId)
{
	if(ItemId == 2)
	{
		return "shotgun";
	}
	else if(ItemId == 3)
	{
		return "grenade";
	}
	else if(ItemId == 4)
	{
		return "rifle";
	}
	else if(ItemId == 5)
	{
		return "all_weapons";
	}
	return "(null)";
}

int CGameContext::TradeHasItem(int ItemId, int ClientId)
{
	CPlayer *pPlayer = m_apPlayers[ClientId];
	if(!pPlayer)
		return -1;

	CCharacter *pChr = GetPlayerChar(ClientId);
	if(!pChr)
		return -1;

	int Item = -1;

	if(ItemId == 2) // shotgun
	{
		if(pChr->HasWeapon(2))
		{
			Item = 2;
		}
	}
	else if(ItemId == 3) // grenade
	{
		if(pChr->HasWeapon(3))
		{
			Item = 3;
		}
	}
	else if(ItemId == 4) // rifle
	{
		if(pChr->HasWeapon(4))
		{
			Item = 4;
		}
	}
	else if(ItemId == 5) // all_weapons
	{
		if(pChr->HasWeapon(4) && pChr->HasWeapon(3) && pChr->HasWeapon(2))
		{
			Item = 5;
		}
	}

	return Item;
}
