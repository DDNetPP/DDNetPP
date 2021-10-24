/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_SQLITE3_H
#define GAME_SERVER_SQLITE3_H
#include "base/system.h"
#include <engine/external/sqlite3/sqlite3.h>
#include <engine/server.h>
#include <queue>
#include <string>
#include <vector>

class CQuery
{
	friend class CSql;

private:
	std::string m_Query;
	sqlite3_stmt *m_pStatement;
	virtual void OnData();

public:
	bool Next();
	int GetColumnCount() { return sqlite3_column_count(m_pStatement); }
	const char *GetName(int i) { return sqlite3_column_name(m_pStatement, i); }
	int GetType(int i) { return sqlite3_column_type(m_pStatement, i); }

	int GetID(const char *pName);
	int GetInt(int i) { return sqlite3_column_int(m_pStatement, i); }
	int GetInt64(int i) { return sqlite3_column_int64(m_pStatement, i); }
	float GetFloat(int i) { return sqlite3_column_double(m_pStatement, i); }
	const char *GetText(int i) { return (const char *)sqlite3_column_text(m_pStatement, i); }
	const void *GetBlob(int i) { return sqlite3_column_blob(m_pStatement, i); }
	int GetSize(int i) { return sqlite3_column_bytes(m_pStatement, i); }

	class CSql *m_pDatabase;
	void Query(class CSql *pDatabase, char *pQuery);
	/*
		QueryBlocking

		made by ChillerDragon who is nobo so use is own risk
		this should be a blocking query and only return after it is executed
		so do not use this in gameticks or the server gets slowed down
	*/
	void QueryBlocking(class CSql *pDatabase, char *pQuery);
	virtual ~CQuery(){};
};

class CSql
{
private:
	void WorkerThread();
	static void InitWorker(void *pSelf);
	LOCK m_Lock;
	LOCK m_CallbackLock;

	// Queries which are not executed yet
	std::queue<CQuery *> m_lpQueries;

	// Queries which are executed but not processed yet
	std::queue<CQuery *> m_lpExecutedQueries;

	bool m_Running;

	sqlite3 *m_pDB;

public:
	CSql();
	~CSql();
	CQuery *Query(CQuery *pQuery, std::string QueryString);
	void QueryBlocking(CQuery *pQuery, std::string QueryString);

	void CreateDatabase();
	void Tick();
};

#endif
