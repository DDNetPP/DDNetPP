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
	}
	return pStr;
}
