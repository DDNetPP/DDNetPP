// ddnet++ 2021

#ifndef GAME_SERVER_DDPP_SHOP_H
#define GAME_SERVER_DDPP_SHOP_H

#include <vector>

class CShopItem
{
	int m_Price;
	int m_NeededLevel;

public:
	CShopItem();

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

	int Price() { return m_Price; }
	int NeededLevel() { return m_NeededLevel; }
};

class CGameContext;

class CShop
{
	std::vector<CShopItem> m_vItems;
	CGameContext *m_pGameContext;

public:
	CShop(class CGameContext *pGameContext);

	void OnInit();
	void ShowShopMotdCompressed(int ClientID);
};

#endif
