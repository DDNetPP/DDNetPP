// ddnet++ accounts
#include <engine/server/databases/connection.h>

#include "gamecontext.h"

#include "accounts.h"

CAccountResult::CAccountResult()
{
	SetVariant(Variant::DIRECT);
}

void CAccountResult::SetVariant(Variant v)
{
	m_MessageKind = v;
	switch(v)
	{
	case DIRECT:
	case ALL:
		for(auto &aMessage : m_aaMessages)
			aMessage[0] = 0;
		break;
	case BROADCAST:
		m_aBroadcast[0] = 0;
		break;
	case LOGIN_INFO:
		// initialized in account data constructor
		break;
	}
}

CAccounts::CAccounts(CGameContext *pGameServer, CDbConnectionPool *pPool) :
	m_pPool(pPool),
	m_pGameServer(pGameServer),
	m_pServer(pGameServer->Server())
{
}

std::shared_ptr<CAccountResult> CAccounts::NewSqlAccountResult(int ClientID)
{
	CPlayer *pCurPlayer = GameServer()->m_apPlayers[ClientID];
	if(pCurPlayer->m_AccountQueryResult != nullptr) // TODO: send player a message: "too many requests"
		return nullptr;
	pCurPlayer->m_AccountQueryResult = std::make_shared<CAccountResult>();
	return pCurPlayer->m_AccountQueryResult;
}

void CAccounts::ExecUserThread(
	bool (*pFuncPtr)(IDbConnection *, const ISqlData *, char *pError, int ErrorSize),
	const char *pThreadName,
	int ClientID,
	const char *pUsername,
	const char *pPassword)
{
	auto pResult = NewSqlAccountResult(ClientID);
	if(pResult == nullptr)
		return;
	auto Tmp = std::unique_ptr<CSqlAccountRequest>(new CSqlAccountRequest(pResult));
	str_copy(Tmp->m_aUsername, pUsername, sizeof(Tmp->m_aUsername));
	str_copy(Tmp->m_aPassword, pPassword, sizeof(Tmp->m_aPassword));

	m_pPool->Execute(pFuncPtr, std::move(Tmp), pThreadName);
}

void CAccounts::Login(int ClientID, const char *pUsername, const char *pPassword)
{
	ExecUserThread(LoginThread, "login user", ClientID, pUsername, pPassword);
}

bool CAccounts::LoginThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize)
{
	const CSqlAccountRequest *pData = dynamic_cast<const CSqlAccountRequest *>(pGameData);
	CAccountResult *pResult = dynamic_cast<CAccountResult *>(pGameData->m_pResult.get());

	char aBuf[512];
	str_copy(aBuf,
		"SELECT "
		"	ID,"
		/*  2         3 */
		"	Username, Password,"
		/*  4             5           6 */
		"	RegisterDate, IsLoggedIn, LastLoginPort,"
		/*  7               8               9               10              11 */
		"	LastLogoutIGN1, LastLogoutIGN2, LastLogoutIGN3, LastLogoutIGN4, LastLogoutIGN5,"
		/*  12    13    14 */
		"	IP_1, IP_2, IP_3,"
		/*  15     16     17 */
		"	Clan1, Clan2, Clan3,"
		"	Skin," // 18
		/*  19     20     21 */
		"	Level, Money, Exp,"
		/*  22    23 */
		"	Shit, LastGift,"
		"	PoliceRank," // 24
		"	JailTime, EscapeTime,"
		"	TaserLevel, "
		"	PvPArenaTickets, PvPArenaGames, PvPArenaKills, PvPArenaDeaths, "
		"	ProfileStyle, ProfileViews, ProfileStatus,"
		"	ProfileSkype, ProfileYoutube, ProfileEmail, ProfileHomepage, ProfileTwitter"
		"	HomingMissiles,"
		"	BlockPoints, BlockKills, BlockDeaths, BlockSkill,"
		"	IsModerator, IsSuperModerator, IsSupporter, IsAccFrozen,"
		"	BombGamesPlayed, BombGamesWon, BombBanTime,"
		"	GrenadeKills, GrenadeDeaths, GrenadeSpree,"
		"	GrenadeShots, GrenadeShotsNoRJ, GrenadeWins,"
		"	RifleKills, RifleDeaths, RifleSpree,"
		"	RifleShots, RifleWins,"
		"	FngConfig, ShowHideConfig,"
		"	SurvivalKills, SurvivalDeaths, SurvivalWins,"
		"	NinjaJetpackBought, SpookyGhost,"
		"	UseSpawnWeapons,"
		"	SpawnWeaponShotgun, SpawnWeaponGrenade, SpawnWeaponRifle,"
		"	AsciiState, AsciiViewsDefault, AsciiViewsProfile,"
		"	AsciiFrame0,"
		"	AsciiFrame1, AsciiFrame2, AsciiFrame3, AsciiFrame4, AsciiFrame5,"
		"	AsciiFrame6, AsciiFrame7, AsciiFrame8, AsciiFrame9, AsciiFrame10,"
		"	AsciiFrame11, AsciiFrame12, AsciiFrame13, AsciiFrame14, AsciiFrame15"
		"FROM Accounts "
		"WHERE Username = ? AND Password = ?;",
		sizeof(aBuf));

	if(pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
	{
		return true;
	}
	pSqlServer->BindString(1, pData->m_aUsername);
	pSqlServer->BindString(2, pData->m_aPassword);

	bool End;
	if(pSqlServer->Step(&End, pError, ErrorSize))
	{
		return true;
	}
	if(!End)
	{
		if(pSqlServer->GetInt(5)) // IsLoggedIn
		{
			pResult->SetVariant(CAccountResult::DIRECT);
			str_copy(pResult->m_aaMessages[0],
				"[ACCOUNT] Login failed. This account is logged in already.",
				sizeof(pResult->m_aaMessages[0]));
			return false;
		}
		int Index = 1;
		pResult->SetVariant(CAccountResult::LOGIN_INFO);
		pResult->m_Account.m_ID = pSqlServer->GetInt(Index++); // 1
		pSqlServer->GetString(Index++, pResult->m_Account.m_aUsername, sizeof(pResult->m_Account.m_aUsername));
		pSqlServer->GetString(Index++, pResult->m_Account.m_aPassword, sizeof(pResult->m_Account.m_aPassword));
		pSqlServer->GetString(Index++, pResult->m_Account.m_aRegisterDate, sizeof(pResult->m_Account.m_aRegisterDate));
		pResult->m_Account.m_IsLoggedIn = pSqlServer->GetInt(Index++);
		Index++; // 6 LastLoginPort (not needed)
		pSqlServer->GetString(Index++, pResult->m_Account.m_LastLogoutIGN1, sizeof(pResult->m_Account.m_LastLogoutIGN1));
		pSqlServer->GetString(Index++, pResult->m_Account.m_LastLogoutIGN2, sizeof(pResult->m_Account.m_LastLogoutIGN2));
		pSqlServer->GetString(Index++, pResult->m_Account.m_LastLogoutIGN3, sizeof(pResult->m_Account.m_LastLogoutIGN3));
		pSqlServer->GetString(Index++, pResult->m_Account.m_LastLogoutIGN4, sizeof(pResult->m_Account.m_LastLogoutIGN4));
		pSqlServer->GetString(Index++, pResult->m_Account.m_LastLogoutIGN5, sizeof(pResult->m_Account.m_LastLogoutIGN5));
		pSqlServer->GetString(Index++, pResult->m_Account.m_aIP_1, sizeof(pResult->m_Account.m_aIP_1));
		pSqlServer->GetString(Index++, pResult->m_Account.m_aIP_2, sizeof(pResult->m_Account.m_aIP_2));
		pSqlServer->GetString(Index++, pResult->m_Account.m_aIP_3, sizeof(pResult->m_Account.m_aIP_3));
		pSqlServer->GetString(Index++, pResult->m_Account.m_aClan1, sizeof(pResult->m_Account.m_aClan1));
		pSqlServer->GetString(Index++, pResult->m_Account.m_aClan2, sizeof(pResult->m_Account.m_aClan2));
		pSqlServer->GetString(Index++, pResult->m_Account.m_aClan3, sizeof(pResult->m_Account.m_aClan3));
		pSqlServer->GetString(Index++, pResult->m_Account.m_aSkin, sizeof(pResult->m_Account.m_aSkin));
		pResult->m_Account.m_Level = pSqlServer->GetInt64(Index++); // 19
		pResult->m_Account.m_Money = pSqlServer->GetInt64(Index++);
		pResult->m_Account.m_XP = pSqlServer->GetInt64(Index++);
		pResult->m_Account.m_Shit = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_GiftDelay = pSqlServer->GetInt(Index++); // 23
		pResult->m_Account.m_PoliceRank = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_JailTime = pSqlServer->GetInt64(Index++);
		pResult->m_Account.m_EscapeTime = pSqlServer->GetInt64(Index++);
		pResult->m_Account.m_TaserLevel = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_PvpArenaTickets = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_PvpArenaGamesPlayed = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_PvpArenaKills = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_PvpArenaDeaths = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_ProfileStyle = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_ProfileViews = pSqlServer->GetInt(Index++);
		pSqlServer->GetString(Index++, pResult->m_Account.m_ProfileStatus, sizeof(pResult->m_Account.m_ProfileStatus));
		pSqlServer->GetString(Index++, pResult->m_Account.m_ProfileSkype, sizeof(pResult->m_Account.m_ProfileSkype));
		pSqlServer->GetString(Index++, pResult->m_Account.m_ProfileYoutube, sizeof(pResult->m_Account.m_ProfileYoutube));
		pSqlServer->GetString(Index++, pResult->m_Account.m_ProfileEmail, sizeof(pResult->m_Account.m_ProfileEmail));
		pSqlServer->GetString(Index++, pResult->m_Account.m_ProfileHomepage, sizeof(pResult->m_Account.m_ProfileHomepage));
		pSqlServer->GetString(Index++, pResult->m_Account.m_ProfileTwitter, sizeof(pResult->m_Account.m_ProfileTwitter));
		pResult->m_Account.m_HomingMissilesAmmo = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_BlockPoints = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_BlockPoints_Kills = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_BlockPoints_Deaths = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_BlockSkill = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_IsModerator = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_IsSuperModerator = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_IsSupporter = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_IsAccFrozen = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_BombGamesPlayed = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_BombGamesWon = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_BombBanTime = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_GrenadeKills = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_GrenadeDeaths = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_GrenadeSpree = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_GrenadeShots = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_GrenadeWins = pSqlServer->GetInt(Index++);
		pSqlServer->GetString(Index++, pResult->m_Account.m_aFngConfig, sizeof(pResult->m_Account.m_aFngConfig));
		pSqlServer->GetString(Index++, pResult->m_Account.m_aShowHideConfig, sizeof(pResult->m_Account.m_aShowHideConfig));
		pResult->m_Account.m_SurvivalKills = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_SurvivalDeaths = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_SurvivalWins = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_NinjaJetpackBought = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_SpookyGhost = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_UseSpawnWeapons = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_SpawnWeaponShotgun = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_SpawnWeaponGrenade = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_SpawnWeaponRifle = pSqlServer->GetInt(Index++);
		pSqlServer->GetString(Index++, pResult->m_Account.m_aAsciiPublishState, sizeof(pResult->m_Account.m_aAsciiPublishState));
		pResult->m_Account.m_AsciiViewsDefault = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_AsciiViewsProfile = pSqlServer->GetInt(Index++);
		for(int i = 0; i < MAX_ASCII_FRAMES; i++)
		{
			pSqlServer->GetString(Index, pResult->m_Account.m_aAsciiFrame[Index], sizeof(pResult->m_Account.m_aAsciiFrame[Index]));
			Index++;
		}
	}
	else
	{
		pResult->SetVariant(CAccountResult::DIRECT);
		str_copy(pResult->m_aaMessages[0],
			"[ACCOUNT] Login failed. Wrong password or username.",
			sizeof(pResult->m_aaMessages[0]));
	}
	return false;
}

void CAccounts::SetLoggedIn(int ClientID, int LoggedIn, int AccountID, int Port)
{
	auto Tmp = std::unique_ptr<CSqlSetLoginData>(new CSqlSetLoginData());
	Tmp->m_Port = Port;
	Tmp->m_LoggedIn = LoggedIn;
	Tmp->m_AccountID = AccountID;

	m_pPool->ExecuteWrite(SetLoggedInThread, std::move(Tmp), "set logged in");
}

bool CAccounts::SetLoggedInThread(IDbConnection *pSqlServer, const ISqlData *pGameData, bool Failure, char *pError, int ErrorSize)
{
	const CSqlSetLoginData *pData = dynamic_cast<const CSqlSetLoginData *>(pGameData);

	char aBuf[512];
	str_copy(aBuf,
		"UPDATE Accounts SET IsLoggedIn = ?, LastLoginPort = ? WHERE ID = ?;",
		sizeof(aBuf));

	if(pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
	{
		return true;
	}
	pSqlServer->BindInt(1, pData->m_LoggedIn);
	pSqlServer->BindInt(2, pData->m_Port);
	pSqlServer->BindInt(3, pData->m_AccountID);

	bool End;
	if(pSqlServer->Step(&End, pError, ErrorSize))
	{
		dbg_assert(false, "SetLoggedInThread did not step");
		return true;
	}
	return End;
}
