#ifndef GAME_SERVER_ACCOUNTS_H
#define GAME_SERVER_ACCOUNTS_H

#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <engine/map.h>
#include <engine/server/databases/connection_pool.h>
#include <game/prng.h>
#include <game/voting.h>

#include "save.h"

struct ISqlData;
class IDbConnection;
class IServer;
class CGameContext;

class CAccounts
{
	CDbConnectionPool *m_pPool;
	CGameContext *GameServer() const { return m_pGameServer; }
	IServer *Server() const { return m_pServer; }
	CGameContext *m_pGameServer;
	IServer *m_pServer;

public:
	CAccounts(CGameContext *pGameServer, CDbConnectionPool *pPool);
	~CAccounts() {}
};

#endif
