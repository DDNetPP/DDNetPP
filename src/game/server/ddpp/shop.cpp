/* DDNet++ shop */

#include "../gamecontext.h"

#include "shop.h"

CShopItem::CShopItem(
	const char *pTitle,
	const char *pName,
	int Price,
	int Level,
	const char *pDescription,
	const char *pOwnedUntil) :
	m_Price(Price),
	m_NeededLevel(Level)
{
	str_copy(m_aTitle, pTitle, sizeof(m_aTitle));
	str_copy(m_aName, pName, sizeof(m_aName));
	str_copy(m_aDescription, pDescription, sizeof(m_aDescription));
	str_copy(m_aOwnUntil, pOwnedUntil, sizeof(m_aOwnUntil));
}

const char *CShopItem::PriceStr()
{
	str_format(m_aPriceStr, sizeof(m_aPriceStr), "%d", Price());
	return m_aPriceStr;
}

const char *CShopItem::NeededLevelStr(int ClientID)
{
	str_format(m_aNeededLevelStr, sizeof(m_aNeededLevelStr), "%d", NeededLevel());
	return m_aNeededLevelStr;
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
	// TODO: remove title from constructor and generate it based on name
	m_vItems.push_back(new CShopItem(
		"        ~  R A I N B O W  ~      ", "rainbow", 1500, 5, "test desc", "dead"));
	m_vItems.push_back(new CShopItem(
		"        ~  B L O O D Y  ~      ", "bloody", 3500, 15, "test desc", "dead"));
	m_vItems.push_back(new CShopItem(
		"        ~  C H I D R A Q U L  ~      ", "chidraqul", 250, 2, "test desc", "disconnect"));
	m_vItems.push_back(new CShopItem(
		"        ~  S H I T  ~      ", "shit", 5, 0, "desc", "forever"));
	m_vItems.push_back(new CShopItemRoomKey(
		"        ~  R O O M K E Y  ~      ", "room_key", g_Config.m_SvRoomPrice, 16, "test desc", "disconnect"));
	m_vItems.push_back(new CShopItem(
		"        ~  P O L I C E  ~      ", "police", 100000, 18, "desc", "forever"));
	m_vItems.push_back(new CShopItemTaser(
		"        ~  T A S E R  ~      ", "taser", 50000, -1, "desc", "forever"));
	m_vItems.push_back(new CShopItem(
		"    ~  P V P A R E N A T I C K E T  ~  ", "pvp_arena_ticket", 150, 0, "desc", "forever"));
	m_vItems.push_back(new CShopItem(
		"       ~  N I N J A J E T P A C K  ~     ", "ninjajetpack", 10000, 21, "desc", "forever"));
	m_vItems.push_back(new CShopItem(
		"     ~  S P A W N S H O T G U N  ~   ", "spawn_shotgun", 600000, 33, "desc", "forever"));
	m_vItems.push_back(new CShopItem(
		"      ~  S P A W N G R E N A D E  ~    ", "spawn_grenade", 600000, 33, "desc", "forever"));
	m_vItems.push_back(new CShopItem(
		"       ~  S P A W N R I F L E  ~       ", "spawn_rifle", 600000, 33, "desc", "forever"));
	m_vItems.push_back(new CShopItem(
		"       ~  S P O O K Y G H O S T  ~     ", "spooky_ghost", 1000000, 1, "desc", "forever"));
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
