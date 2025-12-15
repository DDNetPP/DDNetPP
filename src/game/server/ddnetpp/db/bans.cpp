// ddnet++ bans stored in database
#include <base/log.h>
#include <base/system.h>

#include <engine/server/databases/connection.h>
#include <engine/server/databases/connection_pool.h>
#include <engine/shared/config.h>
#include <engine/shared/protocol.h>

#include <game/server/ddnetpp/db/bans.h>
#include <game/server/gamecontext.h>

CDbBans::CDbBans(CGameContext *pGameServer, CDbConnectionPool *pPool) :
	m_pPool(pPool),
	m_pGameServer(pGameServer),
	m_pServer(pGameServer->Server())
{
}

void CDbBans::CreateDatabase()
{
	auto Tmp = std::make_unique<CSqlCreateTableRequest>();
	m_pPool->ExecuteWrite(CreateTableThread, std::move(Tmp), "create table");
}

bool CDbBans::CreateTableThread(IDbConnection *pSqlServer, const ISqlData *pGameData, Write w, char *pError, int ErrorSize)
{
	if(w == Write::NORMAL_FAILED)
	{
		dbg_assert_failed("CreateTableThread failed to write");
		return false;
	}

	// mysql and sqlite3 compat
	const char *pAutoincrement = "AUTOINCREMENT";
	if(g_Config.m_SvUseMysqlForAccounts && w == Write::NORMAL)
		pAutoincrement = "AUTO_INCREMENT";

	const char *pTableName = "bans";
	char aBuf[4096];
	str_format(aBuf,
		sizeof(aBuf),
		"CREATE TABLE IF NOT EXISTS %s ("
		"  id            INTEGER         PRIMARY KEY %s,"
		"  created_at    TIMESTAMP       NOT NULL DEFAULT CURRENT_TIMESTAMP,"
		"  expire_date   TIMESTAMP       NOT NULL,"
		"  victim_name   VARCHAR(%d)     COLLATE %s NOT NULL,"
		"  victim_ip     VARCHAR(64)     NOT NULL,"
		"  server_ip     VARCHAR(64)     NOT NULL,"
		"  server_port   INTEGER         NOT NULL,"
		"  admin_name    VARCHAR(64)     COLLATE %s NOT NULL,"
		"  reason        VARCHAR(64)     COLLATE %s NOT NULL);",
		pTableName,
		pAutoincrement,
		MAX_NAME_LENGTH,
		pSqlServer->BinaryCollate(),
		pSqlServer->BinaryCollate(),
		pSqlServer->BinaryCollate());

	if(!pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
	{
		return false;
	}
	int NumInserted;
	if(!pSqlServer->ExecuteUpdate(&NumInserted, pError, ErrorSize))
	{
		return false;
	}

	return true;
}
