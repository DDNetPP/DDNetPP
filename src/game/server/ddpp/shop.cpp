/* DDNet++ shop */

#include "../gamecontext.h"
#include <engine/shared/config.h>

#include "shop.h"

void CShop::OnInit()
{
}

CShop::CShop(CGameContext *pGameContext)
{
	m_pGameContext = pGameContext;
}

void CShop::ShowShopMotdCompressed(int ClientID)
{
	char aBuf[2048];
	char aRoomKey[512];
	str_format(aRoomKey, sizeof(aRoomKey), "room_key | %d | 16 | disconnect", g_Config.m_SvRoomPrice);
	str_format(aBuf, sizeof(aBuf),
		"***************************\n"
		"        ~  S H O P  ~      \n"
		"***************************\n"
		"Usage: '/buy (itemname)'\n"
		"***************************\n"
		"Item | Price | Level | Time:\n"
		"-------+------+--------+-------\n"
		"rainbow  | 1 500 | 5 | dead\n"
		"bloody    | 3 500 | 15 | dead\n"
		"chidraqul | 250 | 2 | disconnect\n"
		"shit   | 5 | 0 | forever\n"
		"%s\n"
		"police | 100 000 | 18 | forever\n"
		"taser | 50 000 | Police[3] | forever\n"
		"pvp_arena_ticket | 150 | 0 | forever\n"
		"ninjajetpack | 10 000 | 21 | forever\n"
		"spawn_shotgun | 600 000 | 33 | forever\n"
		"spawn_grenade | 600 000 | 33 | forever\n"
		"spawn_rifle | 600 000 | 33 | forever\n"
		"spooky_ghost | 1 000 000 | 1 | forever\n",
		aRoomKey);
	m_pGameContext->AbuseMotd(aBuf, ClientID);
}
