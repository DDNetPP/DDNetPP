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
		if(!str_comp("Frozen by anti ground hook", pStr))
			return "Замороженный антизаземлитель (Frozen by anti ground hook)"; // TODO: this is deepl.com
		// TODO: this is only used in /stats for now but this could be used in a lot of places
		if(!str_comp("not logged in", pStr))
			return "Игрок не авторизован"; // player is not authorized
		if(pStr[0] == '\'')
		{
			if(!str_comp("'%s' is on a killing spree with %d kills!", pStr))
				return "'%s' совершил серию из %d убийств!";
			if(!str_comp("'%s's killing spree was ended by '%s' (%d kills)", pStr))
				return "Серия убийств игрока '%s' была завершена игроком '%s' (%d убийств)";
		}
		if(pStr[0] == '%')
		{
			if(!str_comp("%d players needed to start a spree.", pStr))
				return "Для начала серии нужно %d игроков.";
		}
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
			if(!str_comp("[JOIN] Block tournaments are deactivated by an admin", pStr))
				return "[JOIN] Турниры блоков отключены администратором";
			if(!str_comp("[JOIN] No block tournament running", pStr))
				return "[JOIN] Турнир блоков не проводится";
			if(!str_comp("[JOIN] You joined a block tournament", pStr))
				return "[JOIN] Вы присоединились к турниру блоков";
			if(!str_comp("[JOIN] You already joined the block tournament", pStr))
				return "[JOIN] Вы уже присоединились к турниру блоков";
			if(!str_comp("[EVENT] BLOCK IN %d SECONDS\n%d joined (min %d)\n/join to participate", pStr))
				return "[ИВЕНТ] БЛОК ТУРНИР ЧЕРЕЗ %d\n%d Зашли (мин %d)\nИспользуйте /join";
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
