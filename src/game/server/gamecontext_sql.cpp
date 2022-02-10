// gamecontext scoped sql ddnet++ methods

#include <engine/shared/config.h>

#include <cstring>

#include "gamecontext.h"
#include "gamecontext_sql.h"

void CQuerySQLstatus::OnData()
{
	int n = Next();
	if(m_ClientID == -1)
		return;
	if(n)
		m_pGameServer->SendChatTarget(m_ClientID, "[SQL] result: got rows.");
	else
		m_pGameServer->SendChatTarget(m_ClientID, "[SQL] result: no rows.");
}

void CGameContext::SQLcleanZombieAccounts(int ClientID)
{
	/*
		support up to 99 999 999 (8 digit long) registered accounts
		if more accounts are registered the system breaks :c

		related issue https://github.com/DDNetPP/DDNetPP/issues/279
	*/
	char aBuf[128 + (MAX_CLIENTS * (MAX_SQL_ID_LENGTH + 1))];
	bool IsLoggedIns = false;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_apPlayers[i])
			continue;
		if(m_apPlayers[i]->IsLoggedIn())
		{
			IsLoggedIns = true;
			break;
		}
	}
	str_copy(aBuf, "UPDATE Accounts SET IsLoggedIn = 0 WHERE LastLoginPort = ? ", sizeof(aBuf));
	if(IsLoggedIns)
	{
		str_append(aBuf, " AND ID NOT IN (", sizeof(aBuf));
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(!m_apPlayers[i])
				continue;
			if(!m_apPlayers[i]->IsLoggedIn())
				continue;
			char aBufBuf[MAX_SQL_ID_LENGTH + 2]; // max supported id len + comma + nullterm
			str_format(aBufBuf, sizeof(aBufBuf), "%d,", m_apPlayers[i]->GetAccID());
			str_append(aBuf, aBufBuf, sizeof(aBuf));
		}
		aBuf[strlen(aBuf) - 1] = '\0'; // chop of the last comma
		str_append(aBuf, ")", sizeof(aBuf));
	}
	dbg_msg("accounts", "clean broken accounts: %s", aBuf);
	m_pAccounts->CleanZombieAccounts(ClientID, g_Config.m_SvPort, aBuf);
}

void CGameContext::ExecuteSQLf(const char *pSQL, ...)
{
	va_list ap;
	va_start(ap, pSQL);
	char *pQueryBuf = sqlite3_vmprintf(pSQL, ap);
	va_end(ap);
	CQuery *pQuery = new CQuery();
	pQuery->Query(m_Database, pQueryBuf);
	sqlite3_free(pQueryBuf);
}
