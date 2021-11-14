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
		"rainbow", "1 500", 5, "test desc", "dead"));
	m_vItems.push_back(new CShopItem(
		"bloody", "3 500", 15, "test desc", "dead"));
	m_vItems.push_back(new CShopItem(
		"chidraqul", "250", 2, "test desc", "disconnect"));
	m_vItems.push_back(new CShopItem(
		"shit", "5", 0, "desc", "forever"));
	m_vItems.push_back(new CShopItemRoomKey(
		"room_key", g_Config.m_SvRoomPrice, 16, "test desc", "disconnect"));
	m_vItems.push_back(new CShopItem(
		"police", "100 000", 18, "desc", "forever"));
	m_vItems.push_back(new CShopItemTaser(
		"taser", "50 000", -1, "desc", "forever"));
	m_vItems.push_back(new CShopItem(
		"pvp_arena_ticket", "150", 0, "desc", "forever"));
	m_vItems.push_back(new CShopItem(
		"ninjajetpack", "10 000", 21, "desc", "forever"));
	m_vItems.push_back(new CShopItem(
		"spawn_shotgun", "600 000", 33, "desc", "forever"));
	m_vItems.push_back(new CShopItem(
		"spawn_grenade", "600 000", 33, "desc", "forever"));
	m_vItems.push_back(new CShopItem(
		"spawn_rifle", "600 000", 33, "desc", "forever"));
	m_vItems.push_back(new CShopItem(
		"spooky_ghost", "1 000 000", 1, "desc", "forever"));
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
