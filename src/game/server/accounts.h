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

/*
	CAccountData

	Has an instance on every player object and one in the sql result
*/
struct CAccountData
{
	// meta
	int m_ClientID;

	int m_AccountID;
	char m_aUsername[64];
	char m_aPassword[64];
	char m_aAccountRegDate[64];

	// city
	int64_t m_level;
	int64_t m_xp;
	int64_t m_money;
	int m_shit;
	int m_GiftDelay;

	bool m_IsModerator;
	bool m_IsSuperModerator;
	bool m_IsSupporter;
	bool m_IsAccFrozen;
};

struct CAccountResult : ISqlResult
{
	CAccountResult();

	enum
	{
		MAX_MESSAGES = 10,
	};

	enum Variant
	{
		DIRECT,
		ALL,
		BROADCAST,
		LOGIN_INFO,
	} m_MessageKind;
	union
	{
		char m_aaMessages[MAX_MESSAGES][512];
		char m_aBroadcast[1024];
		CAccountData m_Account;
	} m_Data; // PLAYER_INFO

	void SetVariant(Variant v);
};

struct CSqlAccountRequest : ISqlData
{
	CSqlAccountRequest(std::shared_ptr<CAccountResult> pResult) :
		ISqlData(std::move(pResult))
	{
	}

	char m_aUsername[64];
	char m_aPassword[64];
};

// for server scoped querys (not per player)

// struct CScoreInitResult : ISqlResult
// {
// 	CScoreInitResult() :
// 		m_CurrentRecord(0)
// 	{
// 	}
// 	float m_CurrentRecord;
// };

class CAccounts
{
	CDbConnectionPool *m_pPool;
	CGameContext *GameServer() const { return m_pGameServer; }
	IServer *Server() const { return m_pServer; }
	CGameContext *m_pGameServer;
	IServer *m_pServer;

	static bool LoginThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);

	// returns new SqlResult bound to the player, if no current Thread is active for this player
	std::shared_ptr<CAccountResult> NewSqlAccountResult(int ClientID);
	// Creates for player database requests
	void ExecUserThread(
		bool (*pFuncPtr)(IDbConnection *, const ISqlData *, char *pError, int ErrorSize),
		const char *pThreadName,
		int ClientID,
		const char *pUsername,
		const char *pPassword);

public:
	CAccounts(CGameContext *pGameServer, CDbConnectionPool *pPool);
	~CAccounts() {}

	void Login(int ClientID, const char *pUsername, const char *pPassword);
};

#endif
