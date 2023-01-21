// ddnet++ 2021

#ifndef GAME_SERVER_DDPP_SHOP_H
#define GAME_SERVER_DDPP_SHOP_H

#include <engine/shared/config.h>

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
	virtual ~CShopItem(){};
	CGameContext *GameServer();

	/*
		Price(ClientID)

		Uses PriceStr() as source of truth.
		So if you want changing prices add that logic in PriceStr().
	*/
	virtual int Price(int ClientID = -1);
	virtual const char *PriceStr(int ClientID = -1) { return m_aPriceStr; };
	virtual int NeededLevel(int ClientID) { return m_NeededLevel; }
	virtual const char *NeededLevelStr(int ClientID);
	const char *Name() { return m_aName; }
	const char *Title();
	const char *Description() { return m_aDescription; }
	const char *OwnUntil() { return m_aOwnUntil; }
	const char *OwnUntilLong();

	bool IsActive() { return m_Active; }
	void Deactivate() { m_Active = false; }
	void Activate() { m_Active = true; }
	virtual bool CanBuy(int ClientID);
	virtual bool Buy(int ClientID);
};

class CShopItemRainbow : public CShopItem
{
public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
};

class CShopItemBloody : public CShopItem
{
public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
};

class CShopItemChidraqul : public CShopItem
{
public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
};

class CShopItemShit : public CShopItem
{
public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
};

class CShopItemRoomKey : public CShopItem
{
public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
	virtual const char *PriceStr(int ClientID = -1) override;
};

class CShopItemPolice : public CShopItem
{
public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
	virtual int NeededLevel(int ClientID) override;
};

class CShopItemTaser : public CShopItem
{
	char m_aNeededPoliceRank[64];

public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
	virtual const char *NeededLevelStr(int ClientID) override;
	virtual const char *PriceStr(int ClientID = -1) override;
	int NeededPoliceRank(int ClientID);
};

class CShopItemPvpArenaTicket : public CShopItem
{
public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
};

class CShopItemNinjaJetpack : public CShopItem
{
public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
};

class CShopItemSpawnShotgun : public CShopItem
{
public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
};

class CShopItemSpawnGrenade : public CShopItem
{
public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
};

class CShopItemSpawnRifle : public CShopItem
{
public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
};

class CShopItemSpookyGhost : public CShopItem
{
public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
};

class CShopItemGrenade : public CShopItem
{
public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
};
class CShopItemShotgun : public CShopItem
{
public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
};
class CShopItemLaser : public CShopItem
{
public:
	using CShopItem::CShopItem;

	virtual bool Buy(int ClientID) override;
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
	bool IsInShop(int ClientID) { return m_InShop[ClientID]; }

	void EnterShop(int ClientID) { m_InShop[ClientID] = true; }
	void LeaveShop(int ClientID);
	void SetMotdTick(int ClientID, int64_t Value) { m_MotdTick[ClientID] = Value; }
	void MotdTick(int ClientID);
	void StartShop(int ClientID);
	void ConfirmPurchase(int ClientID);
	void PurchaseEnd(int ClientID, bool IsCancel);
	bool VoteNo(int ClientID);
	bool VoteYes(int ClientID);
	void OnOpenScoreboard(int ClientID);

	void OnInit();
	void ShowShopMotdCompressed(int ClientID);
	void ShopWindow(int Dir, int ClientID);
	void FireWeapon(int Dir, int ClientID);
	void WillFireWeapon(int ClientID);
	void BuyItem(int ClientID, const char *pItemName);
	const char *GetItemNameById(int ItemID);
};

#endif
