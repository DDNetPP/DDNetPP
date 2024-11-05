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
		if(!str_comp("Level", pStr))
			return "Уровень";
		if(!str_comp("Kills", pStr))
			return "убийств";
		if(!str_comp("Deaths", pStr))
			return "Смертей";
		if(!str_comp("BLOCK", pStr))
			return "БЛОК";
		if(!str_comp("Owned until", pStr))
			return "Срок";
		if(!str_comp("stats", pStr))
			return "статистика";
		if(pStr[0] == '[')
		{
			if(!str_comp("[ACCOUNT] Please use '/register <name> <password> <password>'.", pStr))
				return "[АККАУНТ] Пожалуйста, используйте '/register <имя> <пароль> <пароль>'";
			if(!str_comp("[ACCOUNT] Use '/login <name> <password>'", pStr))
				return "[АККАУНТ] Используйте '/login <имя> <пароль>'";
			if(!str_comp("[ACCOUNT] Use '/accountinfo' for help", pStr))
				return "[АККАУНТ] Используйте '/accountinfo'";
			if(!str_comp("[ACCOUNT] You are already logged in", pStr))
				return "[АККАУНТ] Вы уже вошли в систему";
			if(!str_comp("[ACCOUNT] Logged out", pStr))
				return "[АККАУНТ] Вы вышли из системы";
			if(!str_comp("[ACCOUNT] Account has been registered", pStr))
				return "[АККАУНТ] Аккаунт зарегистрирован";
			if(!str_comp("[ACCOUNT] Login with: /login <name> <pass>", pStr))
				return "[АККАУНТ] Войдите с помощью: /login <имя> <пароль>";
		}
		if(pStr[0] == 'P')
		{
			if(!str_comp("Price", pStr))
				return "Цена";
			if(!str_comp("Points", pStr))
				return "Поинтов";
			if(!str_comp("PvP-Arena Tickets", pStr))
				return "Билеты на ПВП Арену";
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
