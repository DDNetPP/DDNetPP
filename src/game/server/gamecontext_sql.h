/* sql classes used in gamecontext */

#include "db_sqlite3.h"

class CQueryPlayer : public CQuery
{
public:
	int m_ClientID;
	CGameContext *m_pGameServer;
};

class CQuerySQLstatus : public CQueryPlayer
{
	void OnData();
};
