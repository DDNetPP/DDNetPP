/* sql classes used in gamecontext */

#include "db_sqlite3.h"

class CQueryPlayer : public CQuery
{
public:
	int m_ClientID;
	CGameContext *m_pGameServer;
};

// TODO: remove
class CQueryChangePassword : public CQueryPlayer
{
	void OnData();

public:
};

// TODO: remove
class CQuerySetPassword : public CQueryPlayer
{
	void OnData();

public:
};

class CQuerySQLstatus : public CQueryPlayer
{
	void OnData();
};

// TODO: remove
class CQueryRegister : public CQueryPlayer
{
	void OnData();

public:
	std::string m_Name;
	std::string m_Password;
	std::string m_Date;
};

// TODO: remove
class CQueryLogin : public CQueryPlayer
{
	void OnData();

public:
};

// TODO: remove
class CQueryLoginThreaded : public CQueryPlayer
{
	void OnData();

public:
};
