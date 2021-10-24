#include "db_sqlite3.h"
#include <engine/shared/config.h>

bool CQuery::Next()
{
	int Ret = sqlite3_step(m_pStatement);
	return Ret == SQLITE_ROW;
}

void CQuery::Query(CSql *pDatabase, char *pQuery)
{
#if defined(CONF_DEBUG)
	if(!pQuery)
	{
		dbg_msg("SQLite", "[WARNING] no pQuery found in CQuery::Query()");
	}
#endif
	m_pDatabase = pDatabase;
	m_pDatabase->Query(this, pQuery);
}

void CQuery::QueryBlocking(CSql *pDatabase, char *pQueryStr)
{
	pDatabase->QueryBlocking(this, pQueryStr);
}

void CSql::QueryBlocking(CQuery *pQuery, std::string QueryString)
{
	int Ret;
	Ret = sqlite3_prepare_v2(m_pDB, QueryString.c_str(), -1, &pQuery->m_pStatement, 0);
	if(Ret != SQLITE_OK)
	{
		dbg_msg("SQLite", "QueryBlockingError: %s", sqlite3_errmsg(m_pDB));
		return;
	}
	Ret = sqlite3_finalize(pQuery->m_pStatement);
	if(Ret != SQLITE_OK)
	{
		dbg_msg("SQLite", "QueryBlockingFinalizeError: %s", sqlite3_errmsg(m_pDB));
		return;
	}
	dbg_msg("SQLite", "finished QueryBlocking function");
}

void CQuery::OnData()
{
	Next();
}
int CQuery::GetID(const char *pName)
{
	for(int i = 0; i < GetColumnCount(); i++)
	{
		if(str_comp(GetName(i), pName) == 0)
			return i;
	}
	return -1;
}

void CSql::WorkerThread()
{
	while(m_Running)
	{
		lock_wait(m_Lock); //lock queue
		if(m_lpQueries.size() > 0)
		{
			CQuery *pQuery = m_lpQueries.front();
			m_lpQueries.pop();
			lock_unlock(m_Lock); //unlock queue

			int Ret;
			Ret = sqlite3_prepare_v2(m_pDB, pQuery->m_Query.c_str(), -1, &pQuery->m_pStatement, 0);
			if(Ret == SQLITE_OK)
			{
				if(!m_Running) //last check
					break;

				// pQuery->OnData()

				lock_wait(m_CallbackLock);
				m_lpExecutedQueries.push(pQuery);
				lock_unlock(m_CallbackLock);

				// sqlite3_finalize(pQuery->m_pStatement);
			}
			else
			{
				dbg_msg("SQLite", "WorkerError: %s", sqlite3_errmsg(m_pDB));
			}

			// delete pQuery;
		}
		else
		{
			thread_sleep(100);
			lock_unlock(m_Lock); //unlock queue
		}

		thread_sleep(10);
	}
}

void CSql::Tick()
{
	while(true)
	{
		lock_wait(m_CallbackLock);
		if(!m_lpExecutedQueries.size())
		{
			lock_unlock(m_CallbackLock);
			break;
		}

		CQuery *pQuery = m_lpExecutedQueries.front();
		m_lpExecutedQueries.pop();

		pQuery->OnData();

		sqlite3_finalize(pQuery->m_pStatement);

		lock_unlock(m_CallbackLock);
		delete pQuery;
	}
}

void CSql::InitWorker(void *pUser)
{
	CSql *pSelf = (CSql *)pUser;
	pSelf->WorkerThread();
}

CQuery *CSql::Query(CQuery *pQuery, std::string QueryString)
{
#if defined(CONF_DEBUG)
	if(!pQuery)
	{
		dbg_msg("SQLite", "[WARNING] no pQuery found in CQuery *CSql::Query(CQuery *pQuery, std::string QueryString)");
		return NULL;
	}
#endif

	pQuery->m_Query = QueryString;

	lock_wait(m_Lock);
	m_lpQueries.push(pQuery);
	lock_unlock(m_Lock);

	return pQuery;
}

CSql::CSql()
{
}

void CSql::CreateDatabase()
{
	dbg_msg("SQLite", "connecting to '%s' ", g_Config.m_SvDatabasePath);
	int rc = sqlite3_open(g_Config.m_SvDatabasePath, &m_pDB);
	if(rc)
	{
		dbg_msg("SQLite", "Can't open database error: %d", rc);
		sqlite3_close(m_pDB);
		return;
		// the return; used to make trouble but I think it makes sense...
		// the comment that reported the trouble:
		// causes broken db idk why it said "no such column: UseSpawnWeapons" i have no idea why it picked this one and not the first or last
	}

	char const *Query = "CREATE TABLE IF NOT EXISTS Accounts (\n\
		ID							INTEGER			PRIMARY KEY		AUTOINCREMENT,\n\
		Username					VARCHAR(32)		NOT NULL,\n\
		Password					VARCHAR(128)	NOT NULL,\n\
		RegisterDate				VARCHAR(32)		DEFAULT '',\n\
		IsLoggedIn					INTEGER			DEFAULT 0,\n\
		LastLoginPort				INTEGER			DEFAULT 0,\n\
		LastLogoutIGN1				VARCHAR(32)		DEFAULT '',\n\
		LastLogoutIGN2				VARCHAR(32)		DEFAULT '',\n\
		LastLogoutIGN3				VARCHAR(32)		DEFAULT '',\n\
		LastLogoutIGN4				VARCHAR(32)		DEFAULT '',\n\
		LastLogoutIGN5				VARCHAR(32)		DEFAULT '',\n\
		IP_1						VARCHAR(32)		DEFAULT '',\n\
		IP_2						VARCHAR(32)		DEFAULT '',\n\
		IP_3						VARCHAR(32)		DEFAULT '',\n\
		Clan1						VARCHAR(32)		DEFAULT '',\n\
		Clan2						VARCHAR(32)		DEFAULT '',\n\
		Clan3						VARCHAR(32)		DEFAULT '',\n\
		Skin						VARCHAR(32)		DEFAULT '',\n\
		Level						INTEGER			DEFAULT 0,\n\
		Money						INTEGER			DEFAULT 0,\n\
		Exp							INTEGER			DEFAULT 0,\n\
		Shit						INTEGER			DEFAULT 0,\n\
		LastGift					INTEGER			DEFAULT 0,\n\
		PoliceRank					INTEGER			DEFAULT 0,\n\
		JailTime					INTEGER			DEFAULT 0,\n\
		EscapeTime					INTEGER			DEFAULT 0,\n\
		TaserLevel					INTEGER			DEFAULT 0,\n\
		PvPArenaTickets				INTEGER			DEFAULT 0,\n\
		PvPArenaGames				INTEGER			DEFAULT 0,\n\
		PvPArenaKills				INTEGER			DEFAULT 0,\n\
		PvPArenaDeaths				INTEGER			DEFAULT 0,\n\
		ProfileStyle				INTEGER			DEFAULT 0,\n\
		ProfileViews				INTEGER			DEFAULT 0,\n\
		ProfileStatus				VARCHAR(128)	DEFAULT '',\n\
		ProfileSkype				VARCHAR(128)	DEFAULT '',\n\
		ProfileYoutube				VARCHAR(128)	DEFAULT '',\n\
		ProfileEmail				VARCHAR(128)	DEFAULT '',\n\
		ProfileHomepage				VARCHAR(128)	DEFAULT '',\n\
		ProfileTwitter				VARCHAR(128)	DEFAULT '',\n\
		HomingMissiles				INTEGER			DEFAULT 0,\n\
		BlockPoints					INTEGER			DEFAULT 0,\n\
		BlockKills					INTEGER			DEFAULT 0,\n\
		BlockDeaths					INTEGER			DEFAULT 0,\n\
		BlockSkill					INTEGER			DEFAULT 0,\n\
		IsModerator					INTEGER			DEFAULT 0,\n\
		IsSuperModerator			INTEGER			DEFAULT 0,\n\
		IsSupporter					INTEGER			DEFAULT 0,\n\
		IsAccFrozen					INTEGER			DEFAULT 0,\n\
		BombGamesPlayed				INTEGER			DEFAULT 0,\n\
		BombGamesWon				INTEGER			DEFAULT 0,\n\
		BombBanTime					INTEGER			DEFAULT 0,\n\
		GrenadeKills				INTEGER			DEFAULT 0,\n\
		GrenadeDeaths				INTEGER			DEFAULT 0,\n\
		GrenadeSpree				INTEGER			DEFAULT 0,\n\
		GrenadeShots				INTEGER			DEFAULT 0,\n\
		GrenadeShotsNoRJ			INTEGER			DEFAULT 0,\n\
		GrenadeWins					INTEGER			DEFAULT 0,\n\
		RifleKills					INTEGER			DEFAULT 0,\n\
		RifleDeaths					INTEGER			DEFAULT 0,\n\
		RifleSpree					INTEGER			DEFAULT 0,\n\
		RifleShots					INTEGER			DEFAULT 0,\n\
		RifleWins					INTEGER			DEFAULT 0,\n\
		FngConfig					VARCHAR(4)		DEFAULT '',\n\
		ShowHideConfig				VARCHAR(16)		DEFAULT '0010000000',\n\
		SurvivalKills				INTEGER			DEFAULT 0,\n\
		SurvivalDeaths				INTEGER			DEFAULT 0,\n\
		SurvivalWins				INTEGER			DEFAULT 0,\n\
		NinjaJetpackBought			INTEGER			DEFAULT 0,\n\
		SpookyGhost					INTEGER			DEFAULT 0,\n\
		UseSpawnWeapons				INTEGER			DEFAULT 0,\n\
		SpawnWeaponShotgun			INTEGER			DEFAULT 0,\n\
		SpawnWeaponGrenade 			INTEGER			DEFAULT 0,\n\
		SpawnWeaponRifle			INTEGER			DEFAULT 0,\n\
		AsciiState					VARCHAR(4)		DEFAULT '',\n\
		AsciiViewsDefault			INTEGER			DEFAULT 0,\n\
		AsciiViewsProfile			INTEGER			DEFAULT 0,\n\
		AsciiFrame0					VARCHAR(64)		DEFAULT '',\n\
		AsciiFrame1					VARCHAR(64)		DEFAULT '',\n\
		AsciiFrame2					VARCHAR(64)		DEFAULT '',\n\
		AsciiFrame3					VARCHAR(64)		DEFAULT '',\n\
		AsciiFrame4					VARCHAR(64)		DEFAULT '',\n\
		AsciiFrame5					VARCHAR(64)		DEFAULT '',\n\
		AsciiFrame6					VARCHAR(64)		DEFAULT '',\n\
		AsciiFrame7					VARCHAR(64)		DEFAULT '',\n\
		AsciiFrame8					VARCHAR(64)		DEFAULT '',\n\
		AsciiFrame9					VARCHAR(64)		DEFAULT '',\n\
		AsciiFrame10				VARCHAR(64)		DEFAULT '',\n\
		AsciiFrame11				VARCHAR(64)		DEFAULT '',\n\
		AsciiFrame12				VARCHAR(64)		DEFAULT '',\n\
		AsciiFrame13				VARCHAR(64)		DEFAULT '',\n\
		AsciiFrame14				VARCHAR(64)		DEFAULT '',\n\
		AsciiFrame15				VARCHAR(64)		DEFAULT '');";

	sqlite3_exec(m_pDB, Query, 0, 0, 0);

	m_Lock = lock_create();
	m_CallbackLock = lock_create();
	m_Running = true;
	thread_init(InitWorker, this, "sql init");
}

CSql::~CSql()
{
	m_Running = false;
	lock_wait(m_Lock);
	while(m_lpQueries.size())
	{
		CQuery *pQuery = m_lpQueries.front();
		m_lpQueries.pop();
		delete pQuery;
	}

	lock_unlock(m_Lock);
	lock_destroy(m_Lock);

	lock_wait(m_CallbackLock);

	while(m_lpExecutedQueries.size())
	{
		CQuery *pQuery = m_lpExecutedQueries.front();
		m_lpExecutedQueries.pop();
		delete pQuery;
	}

	lock_unlock(m_CallbackLock);
	lock_destroy(m_CallbackLock);
}
