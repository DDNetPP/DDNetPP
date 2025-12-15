#ifndef GAME_SERVER_DDNETPP_DB_BANS_H
#define GAME_SERVER_DDNETPP_DB_BANS_H

#include <engine/map.h>
#include <engine/server/databases/connection_pool.h>

struct ISqlData;
class IDbConnection;
class IServer;
class CGameContext;

class CDbBans
{
	CDbConnectionPool *m_pPool;
	CGameContext *GameServer() const { return m_pGameServer; }
	IServer *Server() const { return m_pServer; }
	CGameContext *m_pGameServer;
	IServer *m_pServer;

	// non ratelimited server side queries
	static bool CreateTableThread(IDbConnection *pSqlServer, const ISqlData *pGameData, Write w, char *pError, int ErrorSize);

public:
	CDbBans(CGameContext *pGameServer, CDbConnectionPool *pPool);
	~CDbBans() = default;

	void CreateDatabase();
};

#endif
