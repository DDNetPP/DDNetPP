/* DDNet++ shop */

#include <game/generated/protocol.h>

#include "../gamecontext.h"

#include "shop.h"

CShopItem::CShopItem(
	const char *pName,
	const char *pPrice,
	int Level,
	const char *pDescription,
	const char *pOwnedUntil,
	CGameContext *pGameContext) :
	m_pGameContext(pGameContext),
	m_NeededLevel(Level)
{
	m_aTitle[0] = '\0';
	str_copy(m_aName, pName, sizeof(m_aName));
	str_copy(m_aPriceStr, pPrice, sizeof(m_aPriceStr));
	str_copy(m_aDescription, pDescription, sizeof(m_aDescription));
	str_copy(m_aOwnUntil, pOwnedUntil, sizeof(m_aOwnUntil));
	Title(); // trigger length check assert on server start
	m_Active = true;
}

CGameContext *CShopItem::GameServer()
{
	return m_pGameContext;
}

int CShopItem::Price(int ClientId)
{
	char aPrice[64] = {0};
	int i = 0;
	for(int k = 0; k < str_length(PriceStr(ClientId)); k++)
	{
		char c = PriceStr(ClientId)[k];
		if(c < '0' || c > '9')
			continue;
		aPrice[i++] = c;
		++c;
	}
	m_Price = atoi(aPrice);
	return m_Price;
}

const char *CShopItem::NeededLevelStr(int ClientId)
{
	str_format(m_aNeededLevelStr, sizeof(m_aNeededLevelStr), "%d", NeededLevel(ClientId));
	return m_aNeededLevelStr;
}

#define MAX_TITLE_LEN 36

const char *CShopItem::Title()
{
	// only compute title once because names do not change
	if(m_aTitle[0] != '\0')
		return m_aTitle;

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
	return m_aTitle;
}

const char *CShopItem::OwnUntilLong()
{
	if(!str_comp(m_aOwnUntil, "dead"))
		return "You own this item until you die";
	if(!str_comp(m_aOwnUntil, "disconnect"))
		return "You own this item until\n"
		       "   you disconnect";
	if(!str_comp(m_aOwnUntil, "forever"))
		return "You own this item forever";
	return "";
}

bool CShopItem::CanBuy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	char aBuf[128];
	if(pPlayer->GetLevel() < NeededLevel(ClientId))
	{
		str_format(aBuf, sizeof(aBuf), "You need to be Level %d to buy '%s'", NeededLevel(ClientId), Name());
		GameServer()->SendChatTarget(ClientId, aBuf);
		return false;
	}
	if(pPlayer->m_Account.m_Money < Price(ClientId))
	{
		str_format(aBuf, sizeof(aBuf), "You don't have enough money! You need %s money.", PriceStr(ClientId));
		GameServer()->SendChatTarget(ClientId, aBuf);
		return false;
	}
	return true;
}

bool CShopItem::Buy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	if(!CanBuy(ClientId))
		return false;
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "bought '%s'", Name());
	pPlayer->MoneyTransaction(-Price(ClientId), aBuf);
	str_format(aBuf, sizeof(aBuf), "You bought %s!", Name());
	GameServer()->SendChatTarget(ClientId, aBuf);
	return true;
}

IServer *CShop::Server()
{
	return m_pGameContext->Server();
}

void CShop::OnInit()
{
	m_vItems.push_back(new CShopItemRainbow(
		"rainbow",
		"1 500",
		5,
		"Rainbow will make your tee change the color very fast.",
		"dead",
		m_pGameContext));
	m_vItems.push_back(new CShopItemBloody(
		"bloody",
		"3 500",
		15,
		"Bloody will give your tee a permanent kill effect.",
		"dead",
		m_pGameContext));
	m_vItems.push_back(new CShopItemChidraqul(
		"chidraqul",
		"250",
		2,
		"Chidraqul is a minigame by ChillerDragon.\n"
		"More information about this game coming soon.",
		"disconnect",
		m_pGameContext));
	m_vItems.push_back(new CShopItemShit(
		"shit",
		"5",
		0,
		"Shit is a fun item. You can use to '/poop' on other players.\n"
		"You can also see your shit amount in your '/profile'.",
		"forever",
		m_pGameContext));
	m_vItems.push_back(new CShopItemRoomKey(
		"room_key",
		g_Config.m_SvRoomPrice,
		16,
		"If you have the room key you can enter the bank room.\n"
		"It's under the spawn and there is a money tile.",
		"disconnect",
		m_pGameContext));
	m_vItems.push_back(new CShopItemPolice(
		"police",
		"100 000",
		18,
		"Police officers get help from the police bot.\n"
		"For more information about the specific police ranks\n"
		"please visit '/policeinfo'.",
		"forever",
		m_pGameContext));
	m_vItems.push_back(new CShopItemTaser(
		"taser",
		"50 000",
		-1,
		"Taser replaces your unfreeze rifle with a rifle that freezes\n"
		"other tees. You can toggle it using '/taser <on/off>'.\n"
		"For more information about the taser and your taser stats,\n"
		"plase visit '/taser info'.",
		"forever",
		m_pGameContext));
	m_vItems.push_back(new CShopItemPvpArenaTicket(
		"pvp_arena_ticket",
		"150",
		0,
		"You can join the pvp arena using '/pvp_arena join' if you have a ticket.",
		"forever",
		m_pGameContext));
	m_vItems.push_back(new CShopItemNinjaJetpack(
		"ninjajetpack",
		"10 000",
		21,
		"It will make your jetpack gun be a ninja.\n"
		"Toggle it using '/ninjajetpack'.",
		"forever",
		m_pGameContext));
	m_vItems.push_back(new CShopItemSpawnShotgun(
		"spawn_shotgun",
		"600 000",
		33,
		"You will have shotgun if you respawn.\n"
		"For more information about spawn weapons,\n"
		"please visit '/spawnweaponsinfo'.",
		"forever",
		m_pGameContext));
	m_vItems.push_back(new CShopItemSpawnGrenade(
		"spawn_grenade",
		"600 000",
		33,
		"You will have grenade if you respawn.\n"
		"For more information about spawn weapons,\n"
		"please visit '/spawnweaponsinfo'.",
		"forever",
		m_pGameContext));
	m_vItems.push_back(new CShopItemSpawnRifle(
		"spawn_rifle",
		"600 000",
		33,
		"You will have rifle if you respawn.\n"
		"For more information about spawn weapons,\n"
		"please visit '/spawnweaponsinfo'.",
		"forever",
		m_pGameContext));
	m_vItems.push_back(new CShopItemSpookyGhost(
		"spooky_ghost",
		"1 000 000",
		1,
		"Using this item you can hide from other players behind bushes.\n"
		"If your ghost is activated you will be able to shoot plasma\n"
		"projectiles. For more information please visit '/spookyghostinfo'.",
		"forever",
		m_pGameContext));
	m_vItems.push_back(new CShopItemShotgun(
		"shotgun",
		"1 000",
		5,
		"Gives you a regular shotgun weapon.\n",
		"dead",
		m_pGameContext));
	m_vItems.push_back(new CShopItemGrenade(
		"grenade",
		"1 000",
		5,
		"Gives you a regular grenade launcher.\n",
		"dead",
		m_pGameContext));
	m_vItems.push_back(new CShopItemLaser(
		"laser",
		"1 000",
		5,
		"Gives you a regular laser rifle weapon.\n",
		"dead",
		m_pGameContext));
	m_vItems.push_back(new CShopItemNinja(
		"ninja",
		"1 000",
		5,
		"Gives you a ninja weapon.\n",
		"dead",
		m_pGameContext));
}

bool CShopItemRainbow::Buy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return false;
	if(!CShopItem::CanBuy(ClientId))
		return false;
	CShopItem::Buy(ClientId);
	pChr->m_Rainbow = true;
	return true;
}

bool CShopItemBloody::Buy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return false;
	if(!CShopItem::Buy(ClientId))
		return false;
	pChr->m_Bloody = true;
	return true;
}

bool CShopItemChidraqul::Buy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	if(!CShopItem::Buy(ClientId))
		return false;
	if(pPlayer->m_BoughtGame)
	{
		GameServer()->SendChatLoc(ClientId, "You already own this item.");
		return false;
	}
	pPlayer->m_BoughtGame = true;
	return true;
}

bool CShopItemShit::Buy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	if(!CShopItem::Buy(ClientId))
		return false;
	pPlayer->m_Account.m_Shit++;
	return true;
}

bool CShopItemRoomKey::Buy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	if(pPlayer->m_BoughtRoom)
	{
		GameServer()->SendChatTarget(ClientId, "You already own this item.");
		return false;
	}
	if(g_Config.m_SvRoomState == 0)
	{
		GameServer()->SendChatTarget(ClientId, "Room has been turned off by admin.");
		return false;
	}
	if(!CShopItem::Buy(ClientId))
		return false;
	pPlayer->m_BoughtRoom = true;
	return true;
}

const char *CShopItemRoomKey::PriceStr(int ClientId)
{
	str_copy(m_aPriceStr, g_Config.m_SvRoomPrice, sizeof(m_aPriceStr));
	return m_aPriceStr;
}

bool CShopItemPolice::Buy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	if(pPlayer->m_Account.m_PoliceRank > 2)
	{
		GameServer()->SendChatTarget(ClientId, "You already bought maximum police level.");
		return false;
	}
	if(!CShopItem::Buy(ClientId))
		return false;
	pPlayer->m_Account.m_PoliceRank++;
	return true;
}

int CShopItemPolice::NeededLevel(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return 18;
	switch(pPlayer->m_Account.m_PoliceRank)
	{
	case 0:
		return 18;
	case 1:
		return 25;
	case 2:
		return 30;
	case 3:
		return 40;
	case 4:
		return 50;
	}
	return 18;
}

bool CShopItemTaser::Buy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	if(pPlayer->m_Account.m_TaserLevel > 6)
	{
		GameServer()->SendChatTarget(ClientId, "Taser already max level.");
		return false;
	}
	if(pPlayer->m_Account.m_PoliceRank < NeededPoliceRank(ClientId))
	{
		GameServer()->SendChatTarget(ClientId, "You don't have a weapon license.");
		return false;
	}
	if(!CShopItem::Buy(ClientId))
		return false;
	pPlayer->m_Account.m_TaserLevel++;
	if(pPlayer->m_Account.m_TaserLevel == 1)
		GameServer()->SendChatTarget(ClientId, "Type '/taser help' for all cmds");
	else
		GameServer()->SendChatTarget(ClientId, "Taser has been upgraded.");
	return true;
}

const char *CShopItemTaser::PriceStr(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return 0;
	switch(pPlayer->m_Account.m_TaserLevel)
	{
	case 0:
		str_copy(m_aPriceStr, "50 000", sizeof(m_aPriceStr));
		break;
	case 1:
		str_copy(m_aPriceStr, "75 000", sizeof(m_aPriceStr));
		break;
	case 2:
		str_copy(m_aPriceStr, "100 000", sizeof(m_aPriceStr));
		break;
	case 3:
		str_copy(m_aPriceStr, "150 000", sizeof(m_aPriceStr));
		break;
	case 4:
		str_copy(m_aPriceStr, "200 000", sizeof(m_aPriceStr));
		break;
	case 5:
		str_copy(m_aPriceStr, "200 000", sizeof(m_aPriceStr));
		break;
	case 6:
		str_copy(m_aPriceStr, "200 000", sizeof(m_aPriceStr));
		break;
	default: // max level
		str_copy(m_aPriceStr, "max", sizeof(m_aPriceStr));
		break;
	}
	return m_aPriceStr;
}

int CShopItemTaser::NeededPoliceRank(int ClientId)
{
	return 3;
}

const char *CShopItemTaser::NeededLevelStr(int ClientId)
{
	str_format(m_aNeededPoliceRank, sizeof(m_aNeededPoliceRank), "Police[%d]", NeededPoliceRank(ClientId));
	return m_aNeededPoliceRank;
}

bool CShopItemPvpArenaTicket::Buy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	if(!CShopItem::Buy(ClientId))
		return false;
	pPlayer->m_Account.m_PvpArenaTickets++;
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "You bought a pvp_arena_ticket. You have %d tickets.", pPlayer->m_Account.m_PvpArenaTickets);
	GameServer()->SendChatTarget(ClientId, aBuf);
	return true;
}

bool CShopItemNinjaJetpack::Buy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	if(pPlayer->m_Account.m_NinjaJetpackBought)
	{
		GameServer()->SendChatTarget(ClientId, "You bought ninjajetpack. Turn it on using '/ninjajetpack'.");
		return false;
	}
	if(!CShopItem::Buy(ClientId))
		return false;
	pPlayer->m_Account.m_NinjaJetpackBought = true;
	return true;
}

bool CShopItemSpawnShotgun::Buy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	if(pPlayer->m_Account.m_SpawnWeaponShotgun >= 5)
	{
		GameServer()->SendChatLoc(ClientId, "You already have maximum level for spawn shotgun.");
		return false;
	}
	if(!CShopItem::Buy(ClientId))
		return false;
	pPlayer->m_Account.m_SpawnWeaponShotgun++;
	GameServer()->SetSpawnweapons(true, ClientId);
	return true;
}

bool CShopItemSpawnGrenade::Buy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	if(pPlayer->m_Account.m_SpawnWeaponGrenade >= 5)
	{
		GameServer()->SendChatLoc(ClientId, "You already have maximum level for spawn grenade.");
		return false;
	}
	if(!CShopItem::Buy(ClientId))
		return false;
	pPlayer->m_Account.m_SpawnWeaponGrenade++;
	GameServer()->SetSpawnweapons(true, ClientId);
	return true;
}

bool CShopItemSpawnRifle::Buy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	if(pPlayer->m_Account.m_SpawnWeaponRifle >= 5)
	{
		GameServer()->SendChatLoc(ClientId, "You already have maximum level for spawn rifle.");
		return false;
	}
	if(!CShopItem::Buy(ClientId))
		return false;
	pPlayer->m_Account.m_SpawnWeaponRifle++;
	GameServer()->SetSpawnweapons(true, ClientId);
	return true;
}

bool CShopItemSpookyGhost::Buy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	if(pPlayer->m_Account.m_SpookyGhost >= 5)
	{
		GameServer()->SendChatLoc(ClientId, "You already have spooky ghost.");
		return false;
	}
	if(!CShopItem::Buy(ClientId))
		return false;
	pPlayer->m_Account.m_SpookyGhost++;
	return true;
}

bool CShopItemWeapon::Buy(int ClientId)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
	if(!pPlayer)
		return false;
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		GameServer()->SendChatLoc(ClientId, "You have to be alive to buy this item.");
		return false;
	}
	if(!CShopItem::Buy(ClientId))
		return false;
	pChr->GiveWeapon(Weapon());
	return true;
}

CShop::CShop(CGameContext *pGameContext)
{
	m_pGameContext = pGameContext;
	mem_zero(m_MotdTick, sizeof(m_MotdTick));
	mem_zero(m_Page, sizeof(m_Page));
	mem_zero(m_PurchaseState, sizeof(m_PurchaseState));
	mem_zero(m_ChangePage, sizeof(m_ChangePage));
	mem_zero(m_InShop, sizeof(m_InShop));
	OnInit();
}

CShop::~CShop()
{
	for(auto &Item : m_vItems)
		delete Item;
}

CGameContext *CShop::GameServer()
{
	return m_pGameContext;
}

void CShop::ShowShopMotdCompressed(int ClientId)
{
	char aBuf[2048];
	str_format(aBuf,
		sizeof(aBuf),
		"***************************\n"
		"        ~  %s  ~      \n" // S H O P
		"***************************\n"
		"%s: '/buy (%s)'\n" // usage: /buy itemname
		"***************************\n"
		"%s | %s | %s | %s\n" // Item | Price | Level | Owned until
		"-------+------+--------+-------\n",
		GameServer()->Loc("S H O P", ClientId),
		GameServer()->Loc("usage", ClientId),
		GameServer()->Loc("itemname", ClientId),
		GameServer()->Loc("Item", ClientId),
		GameServer()->Loc("Price", ClientId),
		GameServer()->Loc("Level", ClientId),
		GameServer()->Loc("Owned until", ClientId));
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
			Item->PriceStr(ClientId),
			Item->NeededLevelStr(ClientId),
			Item->OwnUntil());
		str_append(aBuf, aItem, sizeof(aBuf));
	}
	m_pGameContext->AbuseMotd(aBuf, ClientId);
}

void CShop::MotdTick(int ClientId)
{
	if(m_MotdTick[ClientId] < Server()->Tick())
	{
		m_Page[ClientId] = -1;
		m_PurchaseState[ClientId] = 0;
	}
}

void CShop::WillFireWeapon(int ClientId)
{
	if((m_Page[ClientId] != -1) && (m_PurchaseState[ClientId] == 1))
	{
		m_ChangePage[ClientId] = true;
	}
}

void CShop::FireWeapon(int Dir, int ClientId)
{
	if((m_ChangePage[ClientId]) && (m_Page[ClientId] != -1) && (m_PurchaseState[ClientId] == 1))
	{
		ShopWindow(Dir, ClientId);
		m_ChangePage[ClientId] = false;
	}
}

void CShop::LeaveShop(int ClientId)
{
	if(m_Page[ClientId] != -1)
		m_pGameContext->AbuseMotd("", ClientId);
	m_PurchaseState[ClientId] = 0;
	m_Page[ClientId] = -1;
	m_InShop[ClientId] = false;
}

void CShop::OnOpenScoreboard(int ClientId)
{
	m_MotdTick[ClientId] = 0;
}

void CShop::StartShop(int ClientId)
{
	if(!IsInShop(ClientId))
		return;
	if(m_PurchaseState[ClientId] == 2) // already in buy confirmation state
		return;
	if(m_Page[ClientId] != -1)
		return;

	ShopWindow(0, ClientId);
	m_PurchaseState[ClientId] = 1;
}

void CShop::ConfirmPurchase(int ClientId)
{
	if((m_Page[ClientId] == -1) || (m_Page[ClientId] == 0))
		return;

	char aBuf[256];
	str_format(aBuf,
		sizeof(aBuf),
		"***************************\n"
		"        ~  %s  ~      \n" // S H O P
		"***************************\n\n"
		"%s\n\n" // Are you sure you want to buy this item?
		"f3 - %s\n" // yes
		"f4 - %s\n\n" // no
		"***************************\n",
		GameServer()->Loc("S H O P", ClientId),
		GameServer()->Loc("Are you sure you want to buy this item?", ClientId),
		GameServer()->Loc("yes", ClientId),
		GameServer()->Loc("no", ClientId));

	m_pGameContext->AbuseMotd(aBuf, ClientId);
	m_PurchaseState[ClientId] = 2;
}

void CShop::PurchaseEnd(int ClientId, bool IsCancel)
{
	if(m_PurchaseState[ClientId] != 2) // nothing to end here
		return;

	char aResult[256];
	if(IsCancel)
	{
		char aBuf[256];
		str_copy(aResult, "You canceled the purchase.", sizeof(aResult));
		str_format(aBuf, sizeof(aBuf),
			"***************************\n"
			"        ~  %s  ~           \n" // S H O P
			"***************************\n\n"
			"%s\n\n"
			"***************************\n",
			GameServer()->Loc("S H O P", ClientId),
			aResult);

		m_pGameContext->AbuseMotd(aBuf, ClientId);
	}
	else
	{
		// TODO: performance go brrr make this take an id instead
		BuyItem(ClientId, GetItemNameById(m_Page[ClientId] - 1));
		ShopWindow(ClientId, 0);
	}

	m_PurchaseState[ClientId] = 1;
}

bool CShop::VoteNo(int ClientId)
{
	if(!IsInShop(ClientId))
		return false;

	if(m_PurchaseState[ClientId] == 2)
		PurchaseEnd(ClientId, true);
	else if(m_Page[ClientId] == -1)
		StartShop(ClientId);
	return true;
}

bool CShop::VoteYes(int ClientId)
{
	if(!IsInShop(ClientId))
		return false;

	if(m_PurchaseState[ClientId] == 1)
		ConfirmPurchase(ClientId);
	else if(m_PurchaseState[ClientId] == 2)
		PurchaseEnd(ClientId, false);
	return true;
}

const char *CShop::GetItemNameById(int ItemId)
{
	int Id = 0; // upper camel looks weird on id :D
	for(auto &Item : m_vItems)
		if(Item->IsActive())
			if(Id++ == ItemId)
				return Item->Name();
	return NULL;
}

int CShop::NumShopItems()
{
	int Total = 0;
	for(auto &Item : m_vItems)
		if(Item->IsActive())
			Total++;
	return Total;
}

void CShop::ShopWindow(int Dir, int ClientId)
{
	// motd is there for 10 sec
	m_MotdTick[ClientId] = Server()->Tick() + Server()->TickSpeed() * 10;

	// all active items plus the start page
	int MaxShopPages = NumShopItems();

	if(Dir == 0)
	{
		m_Page[ClientId] = 0;
	}
	else if(Dir == 1)
	{
		m_Page[ClientId]++;
		if(m_Page[ClientId] > MaxShopPages)
		{
			m_Page[ClientId] = 0;
		}
	}
	else if(Dir == -1)
	{
		m_Page[ClientId]--;
		if(m_Page[ClientId] < 0)
		{
			m_Page[ClientId] = MaxShopPages;
		}
	}

	char aMotd[2048];

	if(m_Page[ClientId] == 0)
	{
		str_format(aMotd,
			sizeof(aMotd),
			"***************************\n"
			"        ~  %s  ~           \n" // S H O P
			"***************************\n\n"
			"%s"
			"\n\n"
			"***************************\n"
			"%s", // If you want to buy an item press f3.
			GameServer()->Loc("S H O P", ClientId),
			GameServer()->Loc(
				"Welcome to the shop! If you need help, use '/shop help'.\n\n"
				"By shooting to the right you go one site forward,\n"
				"and by shooting left you go one site backwards.\n\n"
				"If you need more help, visit '/shop help'.",
				ClientId),
			GameServer()->Loc("If you want to buy an item press f3.", ClientId));
		m_pGameContext->AbuseMotd(aMotd, ClientId);
		return;
	}
	int ItemIndex = 0;
	for(auto &Item : m_vItems)
	{
		if(!Item->IsActive())
			continue;
		if(++ItemIndex != m_Page[ClientId])
			continue;

		str_format(
			aMotd,
			sizeof(aMotd),
			"***************************\n"
			"        ~  %s  ~           \n" // S H O P
			"***************************\n\n"
			"%s\n\n" // title
			"%s: %s\n" // Needed level
			"%s: %s\n" // Price
			"%s: %s\n\n" // Time
			"%s\n\n" // description
			"***************************\n"
			"%s\n\n\n" // If you want to buy an item press f3.
			"              ~ %d/%d ~              ",
			GameServer()->Loc("S H O P", ClientId),
			Item->Title(),
			GameServer()->Loc("Needed level", ClientId),
			Item->NeededLevelStr(ClientId),
			GameServer()->Loc("Price", ClientId),
			Item->PriceStr(ClientId),
			GameServer()->Loc("Time", ClientId),
			Item->OwnUntilLong(),
			Item->Description(),
			GameServer()->Loc("If you want to buy an item press f3.", ClientId),
			m_Page[ClientId], MaxShopPages);
	}
	m_pGameContext->AbuseMotd(aMotd, ClientId);
}

void CShop::BuyItem(int ClientId, const char *pItemName)
{
	if(!pItemName)
		return;

	if((g_Config.m_SvShopState == 1) && !IsInShop(ClientId))
	{
		GameServer()->SendChatLoc(ClientId, "You have to be in the shop to buy some items.");
		return;
	}
	for(auto &Item : m_vItems)
	{
		if(!Item->IsActive())
			continue;
		if(str_comp(Item->Name(), pItemName))
			continue;

		Item->Buy(ClientId);
		return;
	}
	GameServer()->SendChatLoc(ClientId, "No such item '%s' see '/shop' for a full list.", pItemName);
}
