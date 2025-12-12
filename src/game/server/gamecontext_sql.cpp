// gamecontext scoped sql ddnet++ methods

#include "gamecontext.h"

#include <engine/shared/config.h>

#include <cstring>

CPlayer *CGameContext::GetPlayerByAccountId(int AccountId)
{
	for(auto &pPlayer : m_apPlayers)
	{
		if(!pPlayer)
			continue;

		if(pPlayer->GetAccId() == AccountId)
			return pPlayer;
	}
	return nullptr;
}

void CGameContext::SQLcleanZombieAccounts(int ClientId)
{
	/*
		support up to 99 999 999 (8 digit long) registered accounts
		if more accounts are registered the system breaks :c

		related issue https://github.com/DDNetPP/DDNetPP/issues/279
	*/
	char aBuf[128 + (MAX_CLIENTS * (MAX_SQL_ID_LENGTH + 1))];
	bool IsLoggedIns = false;
	for(auto &Player : m_apPlayers)
	{
		if(!Player)
			continue;
		if(Player->IsLoggedIn())
		{
			IsLoggedIns = true;
			break;
		}
	}
	str_copy(aBuf, "UPDATE Accounts SET IsLoggedIn = 0 WHERE IsLoggedIn = 1 AND server_ip = ? AND LastLoginPort = ? ", sizeof(aBuf));
	if(IsLoggedIns)
	{
		str_append(aBuf, " AND ID NOT IN (", sizeof(aBuf));
		for(auto &Player : m_apPlayers)
		{
			if(!Player)
				continue;
			if(!Player->IsLoggedIn())
				continue;
			char aBufBuf[MAX_SQL_ID_LENGTH + 2]; // max supported id len + comma + nullterm
			str_format(aBufBuf, sizeof(aBufBuf), "%d,", Player->GetAccId());
			str_append(aBuf, aBufBuf, sizeof(aBuf));
		}
		aBuf[strlen(aBuf) - 1] = '\0'; // chop of the last comma
		str_append(aBuf, ")", sizeof(aBuf));
	}
	dbg_msg("accounts", "clean broken accounts: %s", aBuf);
	m_pAccounts->CleanZombieAccounts(ClientId, g_Config.m_SvPort, aBuf);
}
