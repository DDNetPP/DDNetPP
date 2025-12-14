#ifndef GAME_SERVER_DDPP_ACCOUNTS_H
#define GAME_SERVER_DDPP_ACCOUNTS_H

#include "../save.h"

#include <engine/map.h>
#include <engine/server/databases/connection_pool.h>

#include <game/prng.h>
#include <game/voting.h>

#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct ISqlData;
class IDbConnection;
class IServer;
class CGameContext;

#define MAX_ASCII_FRAMES 16
#define MAX_SQL_ID_LENGTH 8

/*
	CAccountData

	Has an instance on every player object and one in the sql result
*/
struct CAccountData
{
	CAccountData()
	{
		m_ClientId = -1;

		m_Id = 0;
		m_aUsername[0] = '\0';
		m_aPassword[0] = '\0';
		m_aRegisterDate[0] = '\0';
		m_IsLoggedIn = 0;
		m_LastLoginPort = 0;
		m_LastLogoutIGN1[0] = '\0';
		m_LastLogoutIGN2[0] = '\0';
		m_LastLogoutIGN3[0] = '\0';
		m_LastLogoutIGN4[0] = '\0';
		m_LastLogoutIGN5[0] = '\0';
		m_aIp_1[0] = '\0';
		m_aIp_2[0] = '\0';
		m_aIp_3[0] = '\0';
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

		m_ProfileStyle = 0;
		m_ProfileViews = 0;
		m_ProfileStatus[0] = '\0';
		m_ProfileSkype[0] = '\0';
		m_ProfileYoutube[0] = '\0';
		m_ProfileEmail[0] = '\0';
		m_ProfileHomepage[0] = '\0';
		m_ProfileTwitter[0] = '\0';

		m_HomingMissilesAmmo = 0;

		m_BlockPoints = 0;
		m_BlockPoints_Kills = 0;
		m_BlockPoints_Deaths = 0;
		m_BlockSkill = 0;

		m_IsModerator = false;
		m_IsSuperModerator = false;
		m_IsSupporter = false;
		m_IsAccFrozen = false;

		m_BombGamesPlayed = 0;
		m_BombGamesWon = 0;
		m_BombBanTime = 0;

		m_GrenadeKills = 0;
		m_GrenadeDeaths = 0;
		m_GrenadeSpree = 0;
		m_GrenadeShots = 0;
		m_GrenadeShotsNoRJ = 0;
		m_GrenadeWins = 0;
		m_RifleKills = 0;
		m_RifleDeaths = 0;
		m_RifleSpree = 0;
		m_RifleShots = 0;
		m_RifleWins = 0;

		str_copy(m_aFngConfig, "000", sizeof(m_aFngConfig));
		str_copy(m_aShowHideConfig, "0010000000", sizeof(m_aShowHideConfig));

		m_SurvivalKills = 0;
		m_SurvivalDeaths = 0;
		m_SurvivalWins = 0;

		m_NinjaJetpackBought = 0;
		m_SpookyGhost = 0;
		m_UseSpawnWeapons = false;
		m_SpawnWeaponShotgun = 0;
		m_SpawnWeaponGrenade = 0;
		m_SpawnWeaponRifle = 0;

		str_copy(m_aAsciiPublishState, "000", sizeof(m_aAsciiPublishState));
		m_AsciiViewsDefault = 0;
		m_AsciiViewsProfile = 0;
		for(auto &aAsciiFrame : m_aAsciiFrame)
			aAsciiFrame[0] = '\0';
	}

	// meta
	int m_ClientId;

	int m_Id;
	char m_aUsername[64];
	char m_aPassword[64];
	char m_aRegisterDate[64];
	int m_IsLoggedIn;
	int m_LastLoginPort;
	char m_LastLogoutIGN1[32];
	char m_LastLogoutIGN2[32];
	char m_LastLogoutIGN3[32];
	char m_LastLogoutIGN4[32];
	char m_LastLogoutIGN5[32];
	char m_aIp_1[32];
	char m_aIp_2[32];
	char m_aIp_3[32];
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

	/*
		m_ProfileStyle

		0=default 1=shit 2=social 3=show-off 4=pvp 5=bomber
		TODO: create enum
	*/
	int m_ProfileStyle;
	int m_ProfileViews;
	char m_ProfileStatus[50];
	char m_ProfileSkype[50];
	char m_ProfileYoutube[50];
	char m_ProfileEmail[50];
	char m_ProfileHomepage[50];
	char m_ProfileTwitter[50];

	int m_HomingMissilesAmmo;

	int m_BlockPoints; //KILLS + other stuff like block tournaments won
	int m_BlockPoints_Kills; //Block points (blocked others)
	int m_BlockPoints_Deaths; //Block -points (blocked by others)
	int m_BlockSkill;

	bool m_IsModerator;
	bool m_IsSuperModerator;
	bool m_IsSupporter;
	bool m_IsAccFrozen;

	int m_BombGamesPlayed;
	int m_BombGamesWon;
	int m_BombBanTime;

	int m_GrenadeKills;
	int m_GrenadeDeaths;
	int m_GrenadeSpree;
	int m_GrenadeShots;
	int m_GrenadeShotsNoRJ;
	int m_GrenadeWins;
	int m_RifleKills;
	int m_RifleDeaths;
	int m_RifleSpree;
	int m_RifleShots;
	int m_RifleWins;

	/*
		m_aFngConfig

		[0] = autojoin
		[1] = hammertune
		[2] = coming soon
		[3] = nullbyte
	*/
	char m_aFngConfig[4];

	/*
		m_aShowHideConfig

		[0]=blockpoints
		[1]=blockxp
		[2]=xp
		[3]=jail
		[4]=instafeed(1n1)
		[5]=questprogress
		[6]=questwarning
		[7]=instabroadcast
	*/
	char m_aShowHideConfig[16];

	int m_SurvivalKills;
	int m_SurvivalDeaths;
	int m_SurvivalWins;

	int m_NinjaJetpackBought;
	int m_SpookyGhost;
	bool m_UseSpawnWeapons;
	int m_SpawnWeaponShotgun;
	int m_SpawnWeaponGrenade;
	int m_SpawnWeaponRifle;

	/*
		m_aAsciiPublishState

		4 digit int str

		0 = off 1 = on and each digit stands for different stuff.  1=visible at all 2=profile 3=not used yet 4=nullbyte
	*/
	char m_aAsciiPublishState[4];
	int m_AsciiViewsDefault;
	int m_AsciiViewsProfile;
	char m_aAsciiFrame[MAX_ASCII_FRAMES][64];
};

struct CAccountRconCmdResult : ISqlResult
{
	// @param UniqueClientId the unique client id of the admin who launched the request
	CAccountRconCmdResult(uint32_t UniqueClientId);

	enum
	{
		MAX_MESSAGES = 10,
	};

	enum Variant
	{
		DIRECT,
		ALL,
		BROADCAST,
		FREEZE_ACC,
		MODERATOR,
		SUPER_MODERATOR,
		SUPPORTER,
		LOG_ONLY,
	} m_MessageKind;

	// admin that ran the rcon command
	// not a regular client id but a unique id
	uint32_t m_UniqueClientId = 0;

	// the client id of the player that ran the command
	int m_AdminClientId;

	// the sql database id of the account that
	// is being operated on
	int m_TargetAccountId;

	char m_aaMessages[MAX_MESSAGES][512];
	char m_aBroadcast[1024];
	char m_aUsername[64];
	char m_aPassword[64];
	int m_State;
	Variant m_Type;

	void SetVariant(Variant v, const struct CSqlAdminCommandRequest *pRequest);
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
		LOGGED_IN_ALREADY,
		LOGIN_WRONG_PASS,
		LOGIN_INFO,
		REGISTER,
		LOG_ONLY,
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

	CAccountData m_AccountData;
	char m_aUsername[64];
	char m_aPassword[64];
	char m_aNewPassword[64];
	char m_aServerIp[64];
	int m_ServerPort;
};

struct CSqlAdminCommandRequest : ISqlData
{
	CSqlAdminCommandRequest(std::shared_ptr<CAccountRconCmdResult> pResult) :
		ISqlData(std::move(pResult))
	{
	}

	char m_aQuery[128 + (MAX_CLIENTS * (MAX_SQL_ID_LENGTH + 1))];
	int m_AdminClientId;
	int m_TargetAccountId;
	char m_aUsername[64];
	char m_aPassword[64];
	int m_State;
	CAccountRconCmdResult::Variant m_Type;
};

struct CSqlSetLoginData : ISqlData
{
	CSqlSetLoginData() :
		ISqlData(nullptr)
	{
	}

	int m_AccountId;
	int m_LoggedIn;
	char m_aServerIp[64];
	int m_ServerPort;
};

struct CSqlCleanZombieAccountsData : ISqlData
{
	CSqlCleanZombieAccountsData() :
		ISqlData(nullptr)
	{
	}

	char m_aQuery[128 + (MAX_CLIENTS * (MAX_SQL_ID_LENGTH + 1))];
	int m_ClientId;
	char m_aServerIp[64];
	int m_ServerPort;
};

struct CSqlStringData : ISqlData
{
	CSqlStringData() :
		ISqlData(nullptr)
	{
	}

	char m_aString[1024];
};

struct CSqlCreateTableRequest : ISqlData
{
	CSqlCreateTableRequest() :
		ISqlData(nullptr)
	{
	}
	char m_aaMessages[10][512];
};

// for server scoped queries (not per player)

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

	// per player queries user
	static bool LoginThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	static bool RegisterThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	static bool SaveThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	static bool ChangePasswordThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);
	static bool AdminSetPasswordThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);

	// per player queries admin
	static bool UpdateAccountStateThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize);

	// non ratelimited server side queries
	static bool CreateTableThread(IDbConnection *pSqlServer, const ISqlData *pGameData, Write w, char *pError, int ErrorSize);
	static bool SetLoggedInThread(IDbConnection *pSqlServer, const ISqlData *pGameData, Write w, char *pError, int ErrorSize);
	static bool CleanZombieAccountsThread(IDbConnection *pSqlServer, const ISqlData *pGameData, Write w, char *pError, int ErrorSize);
	static bool LogoutUsernameThread(IDbConnection *pSqlServer, const ISqlData *pGameData, Write w, char *pError, int ErrorSize);
	static bool ExecuteSqlThread(IDbConnection *pSqlServer, const ISqlData *pGameData, Write w, char *pError, int ErrorSize);

	// returns new SqlResult bound to the player, if no current Thread is active for this player
	std::shared_ptr<CAccountResult> NewSqlAccountResult(int ClientId);
	void ExecUserThread(
		bool (*pFuncPtr)(IDbConnection *, const ISqlData *, char *pError, int ErrorSize),
		const char *pThreadName,
		int ClientId,
		const char *pUsername,
		const char *pPassword,
		const char *pNewPassword,
		CAccountData *pAccountData);
	void ExecAdminThread(
		bool (*pFuncPtr)(IDbConnection *, const ISqlData *, char *pError, int ErrorSize),
		const char *pThreadName,
		int AdminClientId,
		int TargetAccountId,
		int State,
		CAccountRconCmdResult::Variant Type,
		const char *pUsername,
		const char *pPassword,
		const char *pQuery);

public:
	CAccounts(CGameContext *pGameServer, CDbConnectionPool *pPool);
	~CAccounts() = default;

	/*
		Function:
			Save

		Remarks:
			Shares a ratelimit lock with Login(), Register() and ChangePassword()
			so account saves will not execute if the player
			is currently executing a login query or changing his password
	*/
	void Save(int ClientId, CAccountData *pAccountData);
	void Login(int ClientId, const char *pUsername, const char *pPassword);
	void Register(int ClientId, const char *pUsername, const char *pPassword);
	void ChangePassword(int ClientId, const char *pUsername, const char *pOldPassword, const char *pNewPassword);
	void AdminSetPassword(int ClientId, const char *pUsername, const char *pPassword);

	void UpdateAccountState(int AdminClientId, int TargetAccountId, int State, CAccountRconCmdResult::Variant Type, const char *pQuery);

	void CreateDatabase();
	void SetLoggedIn(int ClientId, int LoggedIn, int AccountId, int Port);
	void CleanZombieAccounts(int ClientId, int Port, const char *pQuery);
	void LogoutUsername(const char *pUsername);
	void ExecuteSql(const char *pQuery);
};

#endif
