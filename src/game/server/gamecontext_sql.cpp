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

void CQuerySetPassword::OnData()
{
	if(Next())
	{
		//send acc infos on found
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "[SQL] Updated '%s's password to [ %s ]", m_pGameServer->m_apPlayers[m_ClientID]->m_aSQLNameName, m_pGameServer->m_apPlayers[m_ClientID]->m_aSetPassword);
		m_pGameServer->SendChatTarget(m_ClientID, aBuf);

		//do the actual sql update
		char *pQueryBuf = sqlite3_mprintf("UPDATE Accounts SET Password='%q' WHERE Username='%q'", m_pGameServer->m_apPlayers[m_ClientID]->m_aSetPassword, m_pGameServer->m_apPlayers[m_ClientID]->m_aSQLNameName);
		CQuery *pQuery = new CQuery();
		pQuery->Query(m_pDatabase, pQueryBuf);
		sqlite3_free(pQueryBuf);
	}
	else
	{
		m_pGameServer->SendChatTarget(m_ClientID, "[ACCOUNT] Invalid account.");
	}
}

void CGameContext::SQLcleanZombieAccounts(int ClientID)
{
	/*
		support up to 99 999 999 (8 digit long) registered accounts
		if more accounts are registered the system breaks :c

		related issue https://github.com/DDNetPP/DDNetPP/issues/279
	*/
	static const int MAX_SQL_ID_LENGTH = 8;
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
	str_format(aBuf, sizeof(aBuf), "UPDATE Accounts SET IsLoggedIn = 0 WHERE LastLoginPort = '%i' ", g_Config.m_SvPort);
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
	ExecuteSQLvf(ClientID, aBuf);
}

void CGameContext::SQLaccount(int mode, int ClientID, const char *pUsername, const char *pPassword)
{
	if(mode == SQL_SET_PASSWORD)
	{
		char *pQueryBuf = sqlite3_mprintf("SELECT * FROM Accounts WHERE Username='%q'", pUsername);
		CQuerySetPassword *pQuery = new CQuerySetPassword();
		pQuery->m_ClientID = ClientID;
		pQuery->m_pGameServer = this;
		pQuery->Query(m_Database, pQueryBuf);
		sqlite3_free(pQueryBuf);
	}
}

void CGameContext::ExecuteSQLBlockingf(const char *pSQL, ...)
{
	va_list ap;
	va_start(ap, pSQL);
	char *pQueryBuf = sqlite3_vmprintf(pSQL, ap);
	va_end(ap);
	CQuery *pQuery = new CQuery();
	pQuery->QueryBlocking(m_Database, pQueryBuf);
	sqlite3_free(pQueryBuf);
	delete pQuery;
	dbg_msg("blocking-sql", "should be last...");
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

void CGameContext::ExecuteSQLvf(int VerboseID, const char *pSQL, ...)
{
	va_list ap;
	va_start(ap, pSQL);
	char *pQueryBuf = sqlite3_vmprintf(pSQL, ap);
	va_end(ap);
	if(VerboseID != -1)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "[SQL] executing: %s", pQueryBuf);
		SendChatTarget(VerboseID, aBuf);
	}
	CQuerySQLstatus *pQuery;
	pQuery = new CQuerySQLstatus();
	pQuery->m_ClientID = VerboseID;
	pQuery->m_pGameServer = this;
	pQuery->Query(m_Database, pQueryBuf);
	sqlite3_free(pQueryBuf);
}
