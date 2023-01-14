#include "loc.h"

#include <base/system.h>

#include <game/server/gamecontext.h>
#include <game/server/player.h>

CLoc::CLoc(CGameContext *pGameContext)
{
	m_pGameContext = pGameContext;
}

CGameContext *CLoc::GameServer()
{
	return m_pGameContext;
}

const char *CLoc::DDPPLocalize(const char *pStr, int ClientID)
{
	// dbg_msg("lang", "id=%d str=%s", ClientID, pStr);
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	if(!pPlayer)
		return pStr;
	if(pPlayer->Language() == LANG_EN)
		return pStr;
	if(pPlayer->Language() == LANG_RU)
	{
		if(!str_comp("Money", pStr))
			return "Деньги";
		if(!str_comp("S H O P", pStr))
			return "Магазин Ашота";
		if(!str_comp("usage", pStr))
			return "Используй";
		if(!str_comp("itemname", pStr))
			return "предмет";
		if(!str_comp("Item", pStr))
			return "Предмет";
		if(!str_comp("Price", pStr))
			return "Цена";
		if(!str_comp("Level", pStr))
			return "Уровень";
		if(!str_comp("Owned until", pStr))
			return "Срок";
	}
	return pStr;
}
