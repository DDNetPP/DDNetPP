/* DDNet++ shop */

#include "../gamecontext.h"

#include "shop.h"

CShopItem::CShopItem(
	const char *pName,
	const char *pPrice,
	int Level,
	const char *pDescription,
	const char *pOwnedUntil) :
	m_NeededLevel(Level)
{
	m_aTitle[0] = '\0';
	str_copy(m_aName, pName, sizeof(m_aName));
	str_copy(m_aPriceStr, pPrice, sizeof(m_aPriceStr));
	str_copy(m_aDescription, pDescription, sizeof(m_aDescription));
	str_copy(m_aOwnUntil, pOwnedUntil, sizeof(m_aOwnUntil));
	Price(); // compute price int based on string
	Title(); // trigger length check assert on server start
	m_Active = true;
}

int CShopItem::Price()
{
	char aPrice[64] = { 0 };
	int i = 0;
	for(int k = 0; k < str_length(m_aPriceStr); k++)
	{
		char c = m_aPriceStr[k];
		if(c == ' ')
			continue;
		aPrice[i++] = c;
		++c;
	}
	m_Price = atoi(aPrice);
	return m_Price;
}

int CShopItemRoomKey::Price()
{
	str_copy(m_aPriceStr, g_Config.m_SvRoomPrice, sizeof(m_aPriceStr));
	return CShopItem::Price();
}

const char *CShopItem::NeededLevelStr(int ClientID)
{
	str_format(m_aNeededLevelStr, sizeof(m_aNeededLevelStr), "%d", NeededLevel());
	return m_aNeededLevelStr;
}

#define MAX_TITLE_LEN 36

const char *CShopItem::Title()
{
	int NameLen = str_length(m_aName) * 2 + 4;
	dbg_assert(NameLen, "shop item name too long to generate title");
	int Padding = (MAX_TITLE_LEN - NameLen) / 2;
	mem_zero(m_aTitle, sizeof(m_aTitle));
	int i = 0;
	for(i = 0; i < Padding; i++)
		m_aTitle[i] = ' ';
	m_aTitle[i++] = '~';
	m_aTitle[i++] = ' ';
	for(int k = 0; k < str_length(m_aName); k++)
	{
		char c = m_aName[k];
		if(c == '\0')
			break;
		if(c == '_')
			m_aTitle[i++] = ' ';
		else
			m_aTitle[i++] = str_uppercase(c);
		m_aTitle[i++] = ' ';
	}
	m_aTitle[i++] = '~';
	dbg_msg("shop", "%s", m_aTitle);
	return m_aTitle;
}

const char *CShopItem::OwnUntilLong()
{
	// TODO: print long descritption from character_ddpp.cpp
	return "";
}


bool CShopItem::CanBuy(int ClientID)
{
	// TODO: check price and level
	// TODO: send nice error message to ClientID if can not buy
	return false;
}

int CShopItemTaser::NeededPoliceRank(int ClientID)
{
	// TODO: this isnt always 3 is it?
	return 3;
}

const char *CShopItemTaser::NeededLevelStr(int ClientID)
{
	str_format(m_aNeededPoliceRank, sizeof(m_aNeededPoliceRank), "Police[%d]", NeededPoliceRank(ClientID));
	return m_aNeededPoliceRank;
}

bool CShopItemTaser::CanBuy(int ClientID)
{
	if(!CShopItem::CanBuy(ClientID))
		return false;
	// TODO: return true when police rank is high enough
	return false;
}

void CShop::OnInit()
{
	m_vItems.push_back(new CShopItem(
		"rainbow", "1 500", 5, "Rainbow will make your tee change the color very fast.", "dead"));
	m_vItems.push_back(new CShopItem(
		"bloody", "3 500", 15, "Bloody will give your tee a permanent kill effect.", "dead"));
	m_vItems.push_back(new CShopItem(
		"chidraqul", "250", 2, "Chidraqul is a minigame by ChillerDragon.\n"
						 "More information about this game coming soon.", "disconnect"));
	m_vItems.push_back(new CShopItem(
		"shit", "5", 0, "Shit is a fun item. You can use to '/poop' on other players.\n"
						 "You can also see your shit amount in your '/profile'.", "forever"));
	m_vItems.push_back(new CShopItemRoomKey(
		"room_key", g_Config.m_SvRoomPrice, 16, "If you have the room key you can enter the bank room.\n"
						 "It's under the spawn and there is a money tile.", "disconnect"));
	m_vItems.push_back(new CShopItem(
		"police", "100 000", 18, "Police officers get help from the police bot.\n"
						 "For more information about the specific police ranks\n"
						 "please visit '/policeinfo'.", "forever"));
	m_vItems.push_back(new CShopItemTaser(
		"taser", "50 000", -1, "Taser replaces your unfreeze rifle with a rifle that freezes\n"
						 "other tees. You can toggle it using '/taser <on/off>'.\n"
						 "For more information about the taser and your taser stats,\n"
						 "plase visit '/taser info'.", "forever"));
	m_vItems.push_back(new CShopItem(
		"pvp_arena_ticket", "150", 0, "You can join the pvp arena using '/pvp_arena join' if you have a ticket.", "forever"));
	m_vItems.push_back(new CShopItem(
		"ninjajetpack", "10 000", 21, "It will make your jetpack gun be a ninja.\n"
						 "Toggle it using '/ninjajetpack'.", "forever"));
	m_vItems.push_back(new CShopItem(
		"spawn_shotgun", "600 000", 33, "You will have shotgun if you respawn.\n"
						 "For more information about spawn weapons,\n"
						 "please visit '/spawnweaponsinfo'.", "forever"));
	m_vItems.push_back(new CShopItem(
		"spawn_grenade", "600 000", 33, "You will have grenade if you respawn.\n"
						 "For more information about spawn weapons,\n"
						 "please visit '/spawnweaponsinfo'.", "forever"));
	m_vItems.push_back(new CShopItem(
		"spawn_rifle", "600 000", 33, "You will have rifle if you respawn.\n"
						 "For more information about spawn weapons,\n"
						 "please visit '/spawnweaponsinfo'.", "forever"));
	m_vItems.push_back(new CShopItem(
		"spooky_ghost", "1 000 000", 1, "Using this item you can hide from other players behind bushes.\n"
						 "If your ghost is activated you will be able to shoot plasma\n"
						 "projectiles. For more information please visit '/spookyghostinfo'.", "forever"));
}

CShop::CShop(CGameContext *pGameContext)
{
	m_pGameContext = pGameContext;
	OnInit();
}

void CShop::ShowShopMotdCompressed(int ClientID)
{
	char aBuf[2048];
	str_copy(aBuf,
		"***************************\n"
		"        ~  S H O P  ~      \n"
		"***************************\n"
		"Usage: '/buy (itemname)'\n"
		"***************************\n"
		"Item | Price | Level | Owned until\n"
		"-------+------+--------+-------\n",
		sizeof(aBuf));
	for(auto &Item : m_vItems)
	{
		if(!Item->IsActive())
			continue;

		char aItem[128];
		str_format(
			aItem,
			sizeof(aItem),
			"%s | %s | %s | %s\n",
			Item->Name(),
			Item->PriceStr(),
			Item->NeededLevelStr(ClientID),
			Item->OwnUntil());
		str_append(aBuf, aItem, sizeof(aBuf));
	}
	m_pGameContext->AbuseMotd(aBuf, ClientID);
}
