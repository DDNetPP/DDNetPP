/* DDNet++ shop */

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

int CShopItem::Price(int ClientID)
{
	char aPrice[64] = {0};
	int i = 0;
	for(int k = 0; k < str_length(PriceStr(ClientID)); k++)
	{
		char c = PriceStr(ClientID)[k];
		if(c == ' ')
			continue;
		aPrice[i++] = c;
		++c;
	}
	m_Price = atoi(aPrice);
	return m_Price;
}

const char *CShopItem::NeededLevelStr(int ClientID)
{
	str_format(m_aNeededLevelStr, sizeof(m_aNeededLevelStr), "%d", NeededLevel(ClientID));
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
		return "Пока не сдохнешь";
	if(!str_comp(m_aOwnUntil, "disconnect"))
		return "Пока не ливнешь\n";
	if(!str_comp(m_aOwnUntil, "forever"))
		return "Навсегда";
	return "";
}

bool CShopItem::CanBuy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	char aBuf[128];
	if(pPlayer->GetLevel() < NeededLevel(ClientID))
	{
		str_format(aBuf, sizeof(aBuf), "Тебе нужен %d уровень что бы купить '%s'", NeededLevel(ClientID), Name());
		GameServer()->SendChatTarget(ClientID, aBuf);
		return false;
	}
	if(pPlayer->m_Account.m_Money < Price(ClientID))
	{
		str_format(aBuf, sizeof(aBuf), "У тебя недостаточно средств! Твой баланс %s.", PriceStr(ClientID));
		GameServer()->SendChatTarget(ClientID, aBuf);
		return false;
	}
	return true;
}

bool CShopItem::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(!CanBuy(ClientID))
		return false;
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "bought '%s'", Name());
	pPlayer->MoneyTransaction(-Price(ClientID), aBuf);
	str_format(aBuf, sizeof(aBuf), "You bought %s!", Name());
	GameServer()->SendChatTarget(ClientID, aBuf);
	return true;
}

IServer *CShop::Server()
{
	return m_pGameContext->Server();
}

void CShop::OnInit()
{
	m_vItems.push_back(new CShopItemSuperHammer(
		"SuperHammer",
		"10 000",
		50,
		"Ну очень пиздатый молот.",
		"disconnect",
		m_pGameContext));

	m_vItems.push_back(new CShopItemGrenade(
		"Grenade",
		"1000",
		15,
		"Выдаёт Grenade прямо тебе в руки.",
		"dead",
		m_pGameContext));

	m_vItems.push_back(new CShopItemShotgun(
		"Shotgun",
		"1000",
		15,
		"Выдаёт Shotgun прямо тебе в руки.",
		"dead",
		m_pGameContext));

	m_vItems.push_back(new CShopItemLaser(
		"Laser",
		"1000",
		15,
		"Выдаёт Laser прямо тебе в руки.",
		"dead",
		m_pGameContext));
}

bool CShopItemRainbow::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return false;
	if(!CShopItem::CanBuy(ClientID))
		return false;
	CShopItem::Buy(ClientID);
	pChr->m_Rainbow = true;
	return true;
}

bool CShopItemBloody::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return false;
	if(!CShopItem::Buy(ClientID))
		return false;
	pChr->m_Bloody = true;
	return true;
}

bool CShopItemChidraqul::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(!CShopItem::Buy(ClientID))
		return false;
	if(pPlayer->m_BoughtGame)
	{
		GameServer()->SendChatTarget(ClientID, "You already own this item.");
		return false;
	}
	pPlayer->m_BoughtGame = true;
	return true;
}

bool CShopItemShit::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(!CShopItem::Buy(ClientID))
		return false;
	pPlayer->m_Account.m_Shit++;
	return true;
}

bool CShopItemRoomKey::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(pPlayer->m_BoughtRoom)
	{
		GameServer()->SendChatTarget(ClientID, "You already own this item.");
		return false;
	}
	if(g_Config.m_SvRoomState == 0)
	{
		GameServer()->SendChatTarget(ClientID, "Room has been turned off by admin.");
		return false;
	}
	if(!CShopItem::Buy(ClientID))
		return false;
	pPlayer->m_BoughtRoom = true;
	return true;
}

const char *CShopItemRoomKey::PriceStr(int ClientID)
{
	str_copy(m_aPriceStr, g_Config.m_SvRoomPrice, sizeof(m_aPriceStr));
	return m_aPriceStr;
}

bool CShopItemPolice::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(pPlayer->m_Account.m_PoliceRank > 2)
	{
		GameServer()->SendChatTarget(ClientID, "You already bought maximum police level.");
		return false;
	}
	if(!CShopItem::Buy(ClientID))
		return false;
	pPlayer->m_Account.m_PoliceRank++;
	return true;
}

int CShopItemPolice::NeededLevel(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
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

bool CShopItemTaser::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(pPlayer->m_Account.m_TaserLevel > 6)
	{
		GameServer()->SendChatTarget(ClientID, "Шокер уже максимального уровня.");
		return false;
	}
	if(pPlayer->m_Account.m_PoliceRank < NeededPoliceRank(ClientID))
	{
		GameServer()->SendChatTarget(ClientID, "You don't have a weapon license.");
		return false;
	}
	if(!CShopItem::Buy(ClientID))
		return false;
	pPlayer->m_Account.m_TaserLevel++;
	if(pPlayer->m_Account.m_TaserLevel == 1)
		GameServer()->SendChatTarget(ClientID, "Type '/taser help' for all cmds");
	else
		GameServer()->SendChatTarget(ClientID, "Вы прокачали Шокер.");
	return true;
}

const char *CShopItemTaser::PriceStr(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
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

int CShopItemTaser::NeededPoliceRank(int ClientID)
{
	return 3;
}

const char *CShopItemTaser::NeededLevelStr(int ClientID)
{
	str_format(m_aNeededPoliceRank, sizeof(m_aNeededPoliceRank), "Police[%d]", NeededPoliceRank(ClientID));
	return m_aNeededPoliceRank;
}

bool CShopItemPvpArenaTicket::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(!CShopItem::Buy(ClientID))
		return false;
	pPlayer->m_Account.m_PvpArenaTickets++;
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "You bought a pvp_arena_ticket. You have %d tickets.", pPlayer->m_Account.m_PvpArenaTickets);
	GameServer()->SendChatTarget(ClientID, aBuf);
	return true;
}

bool CShopItemNinjaJetpack::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(pPlayer->m_Account.m_NinjaJetpackBought)
	{
		GameServer()->SendChatTarget(ClientID, "You bought ninjajetpack. Turn it on using '/ninjajetpack'.");
		return false;
	}
	if(!CShopItem::Buy(ClientID))
		return false;
	pPlayer->m_Account.m_NinjaJetpackBought = true;
	return true;
}

bool CShopItemSpawnShotgun::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(pPlayer->m_Account.m_SpawnWeaponShotgun >= 5)
	{
		GameServer()->SendChatTarget(ClientID, "You already have maximum level for spawn shotgun.");
		return false;
	}
	if(!CShopItem::Buy(ClientID))
		return false;
	pPlayer->m_Account.m_SpawnWeaponShotgun++;
	GameServer()->SetSpawnweapons(true, ClientID);
	return true;
}

bool CShopItemSpawnGrenade::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(pPlayer->m_Account.m_SpawnWeaponGrenade >= 5)
	{
		GameServer()->SendChatTarget(ClientID, "You already have maximum level for spawn grenade.");
		return false;
	}
	if(!CShopItem::Buy(ClientID))
		return false;
	pPlayer->m_Account.m_SpawnWeaponGrenade++;
	GameServer()->SetSpawnweapons(true, ClientID);
	return true;
}

bool CShopItemSpawnRifle::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(pPlayer->m_Account.m_SpawnWeaponRifle >= 5)
	{
		GameServer()->SendChatTarget(ClientID, "You already have maximum level for spawn rifle.");
		return false;
	}
	if(!CShopItem::Buy(ClientID))
		return false;
	pPlayer->m_Account.m_SpawnWeaponRifle++;
	GameServer()->SetSpawnweapons(true, ClientID);
	return true;
}

bool CShopItemSpookyGhost::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(pPlayer->m_Account.m_SpookyGhost >= 5)
	{
		GameServer()->SendChatTarget(ClientID, "You already have spooky ghost.");
		return false;
	}
	if(!CShopItem::Buy(ClientID))
		return false;
	pPlayer->m_Account.m_SpookyGhost++;
	return true;
}

bool CShopItemGrenade::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(pPlayer->GetCharacter()->Core()->m_aWeapons[WEAPON_GRENADE].m_Got)
	{
		GameServer()->SendChatTarget(ClientID, "У тебя уже есть Grenade.");
		return false;
	}
	if(!CShopItem::Buy(ClientID))
		return false;
	pPlayer->GetCharacter()->GiveWeapon(WEAPON_GRENADE, false);
	return true;
}

bool CShopItemSuperHammer::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(pPlayer->m_SuperHammer)
	{
		GameServer()->SendChatTarget(ClientID, "У тебя уже есть ну очень пиздатый молот.");
		return false;
	}
	if(!CShopItem::Buy(ClientID))
		return false;
	pPlayer->m_SuperHammer = true;
	return true;
}

bool CShopItemShotgun::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(pPlayer->GetCharacter()->Core()->m_aWeapons[WEAPON_SHOTGUN].m_Got)
	{
		GameServer()->SendChatTarget(ClientID, "У тебя уже есть Shotgun.");
		return false;
	}
	if(!CShopItem::Buy(ClientID))
		return false;
	pPlayer->GetCharacter()->GiveWeapon(WEAPON_SHOTGUN, false);
	return true;
}

bool CShopItemLaser::Buy(int ClientID)
{
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return false;
	if(pPlayer->GetCharacter()->Core()->m_aWeapons[WEAPON_LASER].m_Got)
	{
		GameServer()->SendChatTarget(ClientID, "У тебя уже есть Laser.");
		return false;
	}
	if(!CShopItem::Buy(ClientID))
		return false;
	pPlayer->GetCharacter()->GiveWeapon(WEAPON_LASER, false);
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

void CShop::ShowShopMotdCompressed(int ClientID)
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
		GameServer()->Loc("S H O P", ClientID),
		GameServer()->Loc("usage", ClientID),
		GameServer()->Loc("itemname", ClientID),
		GameServer()->Loc("Item", ClientID),
		GameServer()->Loc("Price", ClientID),
		GameServer()->Loc("Level", ClientID),
		GameServer()->Loc("Owned until", ClientID));
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
			Item->PriceStr(ClientID),
			Item->NeededLevelStr(ClientID),
			Item->OwnUntil());
		str_append(aBuf, aItem, sizeof(aBuf));
	}
	m_pGameContext->AbuseMotd(aBuf, ClientID);
}

void CShop::MotdTick(int ClientID)
{
	if(m_MotdTick[ClientID] < Server()->Tick())
	{
		m_Page[ClientID] = -1;
		m_PurchaseState[ClientID] = 0;
	}
}

void CShop::WillFireWeapon(int ClientID)
{
	if((m_Page[ClientID] != -1) && (m_PurchaseState[ClientID] == 1))
	{
		m_ChangePage[ClientID] = true;
	}
}

void CShop::FireWeapon(int Dir, int ClientID)
{
	if((m_ChangePage[ClientID]) && (m_Page[ClientID] != -1) && (m_PurchaseState[ClientID] == 1))
	{
		ShopWindow(Dir, ClientID);
		m_ChangePage[ClientID] = false;
	}
}

void CShop::LeaveShop(int ClientID)
{
	if(m_Page[ClientID] != -1)
		m_pGameContext->AbuseMotd("", ClientID);
	m_PurchaseState[ClientID] = 0;
	m_Page[ClientID] = -1;
	m_InShop[ClientID] = false;
}

void CShop::OnOpenScoreboard(int ClientID)
{
	m_MotdTick[ClientID] = 0;
}

void CShop::StartShop(int ClientID)
{
	if(!IsInShop(ClientID))
		return;
	if(m_PurchaseState[ClientID] == 2) // already in buy confirmation state
		return;
	if(m_Page[ClientID] != -1)
		return;

	ShopWindow(0, ClientID);
	m_PurchaseState[ClientID] = 1;
}

void CShop::ConfirmPurchase(int ClientID)
{
	if((m_Page[ClientID] == -1) || (m_Page[ClientID] == 0))
		return;

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf),
		"************************************\n"
		"        ~  Магазин Ашотика  ~      \n"
		"************************************\n\n"
		"Ты точно хочешь купить эту хуйню?\n\n"
		"f3 - Да\n"
		"f4 - Нет\n\n"
		"***************************\n");

	m_pGameContext->AbuseMotd(aBuf, ClientID);
	m_PurchaseState[ClientID] = 2;
}

void CShop::PurchaseEnd(int ClientID, bool IsCancel)
{
	if(m_PurchaseState[ClientID] != 2) // nothing to end here
		return;

	char aResult[256];
	if(IsCancel)
	{
		char aBuf[256];
		str_format(aResult, sizeof(aResult), "You canceled the purchase.");
		str_format(aBuf, sizeof(aBuf),
			"************************************\n"
			"        ~  Магазин Ашотика  ~      \n"
			"************************************\n\n"
			"%s\n\n"
			"***************************\n",
			aResult);

		m_pGameContext->AbuseMotd(aBuf, ClientID);
	}
	else
	{
		// TODO: performance go brrr make this take an id instead
		BuyItem(ClientID, GetItemNameById(m_Page[ClientID] - 1));
		ShopWindow(ClientID, 0);
	}

	m_PurchaseState[ClientID] = 1;
}

bool CShop::VoteNo(int ClientID)
{
	if(!IsInShop(ClientID))
		return false;

	if(m_PurchaseState[ClientID] == 2)
		PurchaseEnd(ClientID, true);
	else if(m_Page[ClientID] == -1)
		StartShop(ClientID);
	return true;
}

bool CShop::VoteYes(int ClientID)
{
	if(!IsInShop(ClientID))
		return false;

	if(m_PurchaseState[ClientID] == 1)
		ConfirmPurchase(ClientID);
	else if(m_PurchaseState[ClientID] == 2)
		PurchaseEnd(ClientID, false);
	return true;
}

const char *CShop::GetItemNameById(int ItemID)
{
	int Id = 0; // upper camel looks weird on id :D
	for(auto &Item : m_vItems)
		if(Item->IsActive())
			if(Id++ == ItemID)
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

void CShop::ShopWindow(int Dir, int ClientID)
{
	// motd is there for 10 sec
	m_MotdTick[ClientID] = Server()->Tick() + Server()->TickSpeed() * 10;

	// all active items plus the start page
	int MaxShopPages = NumShopItems();

	if(Dir == 0)
	{
		m_Page[ClientID] = 0;
	}
	else if(Dir == 1)
	{
		m_Page[ClientID]++;
		if(m_Page[ClientID] > MaxShopPages)
		{
			m_Page[ClientID] = 0;
		}
	}
	else if(Dir == -1)
	{
		m_Page[ClientID]--;
		if(m_Page[ClientID] < 0)
		{
			m_Page[ClientID] = MaxShopPages;
		}
	}

	char aMotd[2048];

	if(m_Page[ClientID] == 0)
	{
		str_copy(aMotd,
			"************************************\n"
			"        ~  Магазин Ашотика  ~      \n"
			"************************************\n\n"
			"Ас-саляму алейкум брат мой!\n\n"
			"Для того что бы переключаться между товарами, стреляй вправо или влево\n"
			"\n\n"
			"***************************\n"
			"Купить предмет - F3.",
			sizeof(aMotd));
		m_pGameContext->AbuseMotd(aMotd, ClientID);
		return;
	}
	int ItemIndex = 0;
	for(auto &Item : m_vItems)
	{
		if(!Item->IsActive())
			continue;
		if(++ItemIndex != m_Page[ClientID])
			continue;

		str_format(aMotd, sizeof(aMotd),
			"************************************\n"
			"        ~  Магазин Ашотика  ~      \n"
			"************************************\n\n"
			"%s\n\n"
			"Уровень: %s\n"
			"Цена: %s\n"
			"Срок: %s\n\n"
			"%s\n\n"
			"***************************\n"
			"Купить - F3.\n\n\n"
			"              ~ %d/%d ~              ",
			Item->Title(),
			Item->NeededLevelStr(ClientID),
			Item->PriceStr(ClientID),
			Item->OwnUntilLong(),
			Item->Description(),
			m_Page[ClientID], MaxShopPages);
	}
	m_pGameContext->AbuseMotd(aMotd, ClientID);
}

void CShop::BuyItem(int ClientID, const char *pItemName)
{
	if(!pItemName)
		return;

	if((g_Config.m_SvShopState == 1) && !IsInShop(ClientID))
	{
		m_pGameContext->SendChatTarget(ClientID, "You have to be in the shop to buy some items.");
		return;
	}
	char aBuf[512];
	for(auto &Item : m_vItems)
	{
		if(!Item->IsActive())
			continue;
		if(str_comp(Item->Name(), pItemName))
			continue;

		Item->Buy(ClientID);
		return;
	}
	str_format(aBuf, sizeof(aBuf), "No such item '%s' see '/shop' for a full list.", pItemName);
	m_pGameContext->SendChatTarget(ClientID, aBuf);
}
