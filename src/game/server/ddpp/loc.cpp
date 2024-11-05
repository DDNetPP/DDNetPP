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

const char *CLoc::DDPPLocalize(const char *pStr, int ClientId)
{
	// dbg_msg("lang", "id=%d str=%s", ClientId, pStr);
	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
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
		if(!str_comp("Kills", pStr))
			return "убийств";
		if(!str_comp("Owned until", pStr))
			return "Срок";
		if(pStr[0] == '[')
		{
			if(!str_comp("[ACCOUNT] Please use '/register <name> <password> <password>'.", pStr))
				return "[Аккаунт] Пожалуйста, используйте '/register <имя> <пароль> <пароль>'";
			if(!str_comp("[ACCOUNT] Use '/login <name> <password>'", pStr))
				return "[Аккаунт] Используйте '/login <имя> <пароль>'";
			if(!str_comp("[ACCOUNT] Use '/accountinfo' for help", pStr))
				return "[Аккаунт] Используйте '/accountinfo'";
		}
		if(pStr[0] == 'Y')
		{
			if(!str_comp("You need to be logged in to use moneytiles. \nGet an account with '/register <name> <pw> <pw>'", pStr))
				return "Tебе нужно войти в аккаунт. \nИспользуй '/register' или '/login'";
			if(!str_comp("You need to be logged in to play. \nGet an account with '/register <name> <pw> <pw>'", pStr))
				return "Чтобы играть, вам нужно войти в систему. \nИспользуй '/register' или '/login'";
		}
	}
	return pStr;
}
