// ddnet++ 2021

#ifndef GAME_SERVER_DDPP_SHOP_H
#define GAME_SERVER_DDPP_SHOP_H

#include <engine/server.h>
#include <engine/shared/config.h>
#include <generated/protocol.h>

#include <vector>

class CGameContext;

class CShopItem
{
	CGameContext *m_pGameContext;

protected:
	int m_Price;
	int m_NeededLevel;

	/*
        m_Active

        Shop items can be deactivated in rcon
        then they wont be listed in the shop
    */
	bool m_Active;

	/*
        m_aTitle

        fancy formatted title shown in the motd
        shop menu
    */
	char m_aTitle[64];

	/*
        m_aName

        Short name shown the compressed shop
        and used for /buy commands
    */
	char m_aName[64];

	/*
        m_aDescription

        Description shown in the motd shop menu
    */
	char m_aDescription[1024];

	/*
        m_aOwnUntil

        String holding the time as human readable string
        for how long the shop item can be used.

        values:

            dead
            disconnect
            forever
    */
	char m_aOwnUntil[64];

	/*
        m_aPriceStr

        Holding the m_Price as string.
        Make it more readable by splitting every 3 digits.
    */
	char m_aPriceStr[64];

	/*
        m_aNeededLevelStr

        Holding the m_NeededLevel as string.

        Can also hold a police level as value in the format of
        Police[level]
    */
	char m_aNeededLevelStr[64];

public:
	CShopItem(
		const char *pName,
		const char *pPrice,
		int Level,
		const char *pDescription,
		const char *pOwnedUntil,
		CGameContext *pGameContext);
	virtual ~CShopItem() = default;
	CGameContext *GameServer();

	/*
		Price(ClientId)

		Uses PriceStr() as source of truth.
		So if you want changing prices add that logic in PriceStr().
	*/
	virtual int Price(int ClientId = -1);
	virtual const char *PriceStr(int ClientId = -1) { return m_aPriceStr; };
	void SetPrice(const char *pPrice) { str_copy(m_aPriceStr, pPrice, sizeof(m_aPriceStr)); }
	void SetDescription(const char *pDescription) { str_copy(m_aDescription, pDescription, sizeof(m_aDescription)); }
	void SetNeededLevel(int Level) { m_NeededLevel = Level; }
	virtual int NeededLevel(int ClientId) { return m_NeededLevel; }
	virtual const char *NeededLevelStr(int ClientId);
	const char *Name() { return m_aName; }
	const char *Title();
	const char *Description() { return m_aDescription; }
	const char *OwnUntil() { return m_aOwnUntil; }
	const char *OwnUntilLong();

	bool IsActive() const { return m_Active; }
	void Deactivate() { m_Active = false; }
	void Activate() { m_Active = true; }
	virtual bool CanBuy(int ClientId);
	virtual bool Buy(int ClientId);
};

class CShopItemRainbow : public CShopItem
{
public:
	using CShopItem::CShopItem;

	bool Buy(int ClientId) override;
};

class CShopItemBloody : public CShopItem
{
public:
	using CShopItem::CShopItem;

	bool Buy(int ClientId) override;
};

class CShopItemChidraqul : public CShopItem
{
public:
	using CShopItem::CShopItem;

	bool Buy(int ClientId) override;
};

class CShopItemShit : public CShopItem
{
public:
	using CShopItem::CShopItem;

	bool Buy(int ClientId) override;
};

class CShopItemRoomKey : public CShopItem
{
public:
	using CShopItem::CShopItem;

	bool Buy(int ClientId) override;
	const char *PriceStr(int ClientId = -1) override;
};

class CShopItemPolice : public CShopItem
{
public:
	using CShopItem::CShopItem;

	bool Buy(int ClientId) override;
	int NeededLevel(int ClientId) override;
};

class CShopItemTaser : public CShopItem
{
	char m_aNeededPoliceRank[64];

public:
	using CShopItem::CShopItem;

	bool Buy(int ClientId) override;
	const char *NeededLevelStr(int ClientId) override;
	const char *PriceStr(int ClientId = -1) override;
	int NeededPoliceRank(int ClientId);
};

class CShopItemPvpArenaTicket : public CShopItem
{
public:
	using CShopItem::CShopItem;

	bool Buy(int ClientId) override;
};

class CShopItemNinjaJetpack : public CShopItem
{
public:
	using CShopItem::CShopItem;

	bool Buy(int ClientId) override;
};

class CShopItemSpawnShotgun : public CShopItem
{
public:
	using CShopItem::CShopItem;

	bool Buy(int ClientId) override;
};

class CShopItemSpawnGrenade : public CShopItem
{
public:
	using CShopItem::CShopItem;

	bool Buy(int ClientId) override;
};

class CShopItemSpawnRifle : public CShopItem
{
public:
	using CShopItem::CShopItem;

	bool Buy(int ClientId) override;
};

class CShopItemSpookyGhost : public CShopItem
{
public:
	using CShopItem::CShopItem;

	bool Buy(int ClientId) override;
};

class CShopItemWeapon : public CShopItem
{
public:
	using CShopItem::CShopItem;

	bool Buy(int ClientId) override;

	virtual int Weapon() { return WEAPON_GUN; }
};

class CShopItemShotgun : public CShopItemWeapon
{
public:
	using CShopItemWeapon::CShopItemWeapon;

	int Weapon() override { return WEAPON_SHOTGUN; }
};

class CShopItemGrenade : public CShopItemWeapon
{
public:
	using CShopItemWeapon::CShopItemWeapon;

	int Weapon() override { return WEAPON_GRENADE; }
};

class CShopItemLaser : public CShopItemWeapon
{
public:
	using CShopItemWeapon::CShopItemWeapon;

	int Weapon() override { return WEAPON_LASER; }
};

class CShopItemNinja : public CShopItemWeapon
{
public:
	using CShopItemWeapon::CShopItemWeapon;

	int Weapon() override { return WEAPON_NINJA; }
};

class CShop
{
	CGameContext *m_pGameContext;
	int64_t m_MotdTick[MAX_CLIENTS];
	int m_Page[MAX_CLIENTS];
	int m_PurchaseState[MAX_CLIENTS];
	bool m_ChangePage[MAX_CLIENTS];
	bool m_InShop[MAX_CLIENTS];
	IServer *Server();

public:
	CShop(class CGameContext *pGameContext);
	~CShop();

	CGameContext *GameServer();
	std::vector<CShopItem *> m_vItems;

	int NumShopItems();
	bool IsInShop(int ClientId) { return m_InShop[ClientId]; }

	void EnterShop(int ClientId) { m_InShop[ClientId] = true; }
	void LeaveShop(int ClientId);
	void SetMotdTick(int ClientId, int64_t Value) { m_MotdTick[ClientId] = Value; }
	void MotdTick(int ClientId);
	void StartShop(int ClientId);
	void ConfirmPurchase(int ClientId);
	void PurchaseEnd(int ClientId, bool IsCancel);
	bool VoteNo(int ClientId);
	bool VoteYes(int ClientId);
	void OnOpenScoreboard(int ClientId);

	void OnInit();
	void ShowShopMotdCompressed(int ClientId);
	void ShopWindow(int Dir, int ClientId);
	void FireWeapon(int Dir, int ClientId);
	void WillFireWeapon(int ClientId);
	void BuyItem(int ClientId, const char *pItemName);
	const char *GetItemNameById(int ItemId);
};

#endif
