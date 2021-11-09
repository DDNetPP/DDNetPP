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
	CAccountData()
	{
		m_ClientID = -1;

		m_ID = 0;
		m_aUsername[0] = '\0';
		m_aPassword[0] = '\0';
		m_aRegisterDate[0] = '\0';
		m_LastLogoutIGN1[0] = '\0';
		m_LastLogoutIGN2[0] = '\0';
		m_LastLogoutIGN3[0] = '\0';
		m_LastLogoutIGN4[0] = '\0';
		m_LastLogoutIGN5[0] = '\0';
		m_aIP_1[0] = '\0';
		m_aIP_2[0] = '\0';
		m_aIP_3[0] = '\0';
		m_aClan1[0] = '\0';
		m_aClan2[0] = '\0';
		m_aClan3[0] = '\0';
		m_aSkin[0] = '\0';

		m_Level = 0;
		m_Money = 0;
		m_XP = 0;
		m_Shit = 0;
		m_GiftDelay = 0;

		m_PoliceRank = 0;
		m_JailTime = 0;
		m_EscapeTime = 0;
		m_TaserLevel = 0;

		m_PvpArenaTickets = 0;
		m_PvpArenaGamesPlayed = 0;
		m_PvpArenaKills = 0;
		m_PvpArenaDeaths = 0;

		m_IsModerator = false;
		m_IsSuperModerator = false;
		m_IsSupporter = false;
		m_IsAccFrozen = false;
	}

	// meta
	int m_ClientID;

	int m_ID;
	char m_aUsername[64];
	char m_aPassword[64];
	char m_aRegisterDate[64];
	char m_LastLogoutIGN1[32];
	char m_LastLogoutIGN2[32];
	char m_LastLogoutIGN3[32];
	char m_LastLogoutIGN4[32];
	char m_LastLogoutIGN5[32];
	char m_aIP_1[32];
	char m_aIP_2[32];
	char m_aIP_3[32];
	char m_aClan1[32];
	char m_aClan2[32];
	char m_aClan3[32];
	char m_aSkin[32];

	// city
	int64_t m_Level;
	int64_t m_Money;
	int64_t m_XP;
	int m_Shit;
	int m_GiftDelay;

	int m_PoliceRank;
	int64_t m_JailTime;
	int64_t m_EscapeTime;
	int m_TaserLevel;

	int m_PvpArenaTickets;
	int m_PvpArenaGamesPlayed;
	int m_PvpArenaKills;
	int m_PvpArenaDeaths;

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

	char m_aaMessages[MAX_MESSAGES][512];
	char m_aBroadcast[1024];
	CAccountData m_Account;

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
