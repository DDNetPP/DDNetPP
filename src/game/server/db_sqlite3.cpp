#include "db_sqlite3.h"

bool CQuery::Next()
{
	int Ret = sqlite3_step(m_pStatement);
	return Ret == SQLITE_ROW;
}
void CQuery::Query(CSql *pDatabase, char *pQuery)
{
	m_pDatabase = pDatabase;
	m_pDatabase->Query(this, pQuery);
}
void CQuery::OnData()
{
	Next();
}
int CQuery::GetID(const char *pName)
{
	for (int i = 0; i < GetColumnCount(); i++)
	{
		if (str_comp(GetName(i), pName) == 0)
			return i;
	}
	return -1;
}

void CSql::WorkerThread()
{
	while (m_Running)
	{
		lock_wait(m_Lock); //lock queue
		if (m_lpQueries.size() > 0)
		{
			CQuery *pQuery = m_lpQueries.front();
			m_lpQueries.pop();
			lock_unlock(m_Lock); //unlock queue

			int Ret;
			Ret = sqlite3_prepare_v2(m_pDB, pQuery->m_Query.c_str(), -1, &pQuery->m_pStatement, 0);
			if (Ret == SQLITE_OK)
			{
				if (!m_Running) //last check
					break;
				pQuery->OnData();

				sqlite3_finalize(pQuery->m_pStatement);
			}
			else
				dbg_msg("SQLite", "%s", sqlite3_errmsg(m_pDB));

			delete pQuery;
		}
		else
		{
			thread_sleep(100);
			lock_unlock(m_Lock); //unlock queue
		}

		thread_sleep(10);
	}
}

void CSql::InitWorker(void *pUser)
{
	CSql *pSelf = (CSql *)pUser;
	pSelf->WorkerThread();
}

CQuery *CSql::Query(CQuery *pQuery, std::string QueryString)
{
	pQuery->m_Query = QueryString;

	lock_wait(m_Lock);
	m_lpQueries.push(pQuery);
	lock_unlock(m_Lock);

	return pQuery;
}

CSql::CSql()
{
	sqlite3 *test;
	int rc = sqlite3_open("accounts.db", &m_pDB);
	if (rc)
	{
		dbg_msg("SQLite", "Can't open database");
		sqlite3_close(m_pDB);
	}

	char *Query = "CREATE TABLE IF NOT EXISTS Accounts (" \
		"ID							INTEGER			PRIMARY KEY		AUTOINCREMENT," \
		"Username					VARCHAR(32)		NOT NULL," \
		"Password					VARCHAR(128)	NOT NULL," \
		"Level						INTEGER			DEFAULT 0," \
		"Money						INTEGER			DEFAULT 0," \
		/* "Neededxp                   INTEGER         DEFAULT 5000," \
		"Plusxp                     INTEGER         DEFAULT 1000," \ */
		"Exp						INTEGER			DEFAULT 0," \
		"Shit						INTEGER			DEFAULT 0," \
		"LastGift					INTEGER			DEFAULT 0," \
		"PoliceRank					INTEGER			DEFAULT 0," \
		"JailTime					INTEGER			DEFAULT 0," \
		"EscapeTime					INTEGER			DEFAULT 0," \
		"TaserLevel					INTEGER			DEFAULT 0);";

	sqlite3_exec(m_pDB, Query, 0, 0, 0);

	m_Lock = lock_create();
	m_Running = true;
	thread_init(InitWorker, this);
}

CSql::~CSql()
{
	m_Running = false;
	lock_wait(m_Lock);
	while (m_lpQueries.size())
	{
		CQuery *pQuery = m_lpQueries.front();
		m_lpQueries.pop();
		delete pQuery;
	}
	lock_unlock(m_Lock);
	lock_destroy(m_Lock);
}
