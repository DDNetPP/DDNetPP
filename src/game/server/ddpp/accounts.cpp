// ddnet++ accounts
#include <engine/server/databases/connection.h>

#include "../gamecontext.h"

#include "accounts.h"

CAdminCommandResult::CAdminCommandResult()
{
	SetVariant(Variant::DIRECT, NULL);
}

// TODO: remove SetVariant and use constructor instead.
//       we are not doing any union magic in the CAdminCommandResult
void CAdminCommandResult::SetVariant(Variant v, const CSqlAdminCommandRequest *pRequest)
{
	if(pRequest)
	{
		m_AdminClientID = pRequest->m_AdminClientID;
		m_TargetAccountID = pRequest->m_TargetAccountID;
		m_State = pRequest->m_State;
		m_Type = pRequest->m_Type;
		str_copy(m_aUsername, pRequest->m_aUsername, sizeof(m_aUsername));
		str_copy(m_aPassword, pRequest->m_aPassword, sizeof(m_aPassword));
	}
	else
	{
		m_AdminClientID = -1;
		m_TargetAccountID = -1;
		m_State = -1;
		m_Type = DIRECT;
		m_aUsername[0] = '\0';
		m_aPassword[0] = '\0';
	}
	m_MessageKind = v;
	switch(v)
	{
	case FREEZE_ACC:
	case MODERATOR:
	case SUPER_MODERATOR:
	case SUPPORTER:
	case DIRECT:
	case ALL:
		for(auto &aMessage : m_aaMessages)
			aMessage[0] = 0;
		break;
	case BROADCAST:
		m_aBroadcast[0] = 0;
		break;
	case LOG_ONLY:
		break;
	}
}

CAccountResult::CAccountResult()
{
	SetVariant(Variant::DIRECT);
}

void CAccountResult::SetVariant(Variant v)
{
	m_MessageKind = v;
	switch(v)
	{
	case REGISTER:
	case DIRECT:
	case ALL:
		for(auto &aMessage : m_aaMessages)
			aMessage[0] = 0;
		break;
	case BROADCAST:
		m_aBroadcast[0] = 0;
		break;
	case LOGGED_IN_ALREADY:
		break;
	case LOGIN_WRONG_PASS:
	case LOGIN_INFO:
		m_Account = CAccountData();
		break;
	case LOG_ONLY:
		break;
	}
}

CAccounts::CAccounts(CGameContext *pGameServer, CDbConnectionPool *pPool) :
	m_pPool(pPool),
	m_pGameServer(pGameServer),
	m_pServer(pGameServer->Server())
{
}

std::shared_ptr<CAdminCommandResult> CAccounts::NewSqlAdminCommandResult(int ClientID)
{
	CPlayer *pCurPlayer = GameServer()->m_apPlayers[ClientID];
	if(pCurPlayer->m_AdminCommandQueryResult != nullptr) // TODO: send player a message: "too many requests"
		return nullptr;
	pCurPlayer->m_AdminCommandQueryResult = std::make_shared<CAdminCommandResult>();
	return pCurPlayer->m_AdminCommandQueryResult;
}

void CAccounts::ExecAdminThread(
	bool (*pFuncPtr)(IDbConnection *, const ISqlData *, char *pError, int ErrorSize),
	const char *pThreadName,
	int AdminClientID,
	int TargetAccountID,
	int State,
	CAdminCommandResult::Variant Type,
	const char *pUsername,
	const char *pPassword,
	const char *pQuery)
{
	auto pResult = NewSqlAdminCommandResult(AdminClientID);
	if(pResult == nullptr)
		return;
	auto Tmp = std::make_unique<CSqlAdminCommandRequest>(pResult);
	Tmp->m_AdminClientID = AdminClientID;
	Tmp->m_TargetAccountID = TargetAccountID;
	Tmp->m_State = State;
	Tmp->m_Type = Type;
	str_copy(Tmp->m_aUsername, pUsername, sizeof(Tmp->m_aUsername));
	str_copy(Tmp->m_aPassword, pPassword, sizeof(Tmp->m_aPassword));
	str_copy(Tmp->m_aQuery, pQuery, sizeof(Tmp->m_aQuery));

	m_pPool->Execute(pFuncPtr, std::move(Tmp), pThreadName);
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
	const char *pPassword,
	const char *pNewPassword,
	CAccountData *pAccountData)
{
	auto pResult = NewSqlAccountResult(ClientID);
	if(pResult == nullptr)
		return;
	auto Tmp = std::make_unique<CSqlAccountRequest>(pResult);
	str_copy(Tmp->m_aUsername, pUsername, sizeof(Tmp->m_aUsername));
	str_copy(Tmp->m_aPassword, pPassword, sizeof(Tmp->m_aPassword));
	str_copy(Tmp->m_aNewPassword, pNewPassword, sizeof(Tmp->m_aNewPassword));
	if(pAccountData)
		Tmp->m_AccountData = *pAccountData;
	else
		Tmp->m_AccountData = CAccountData();

	m_pPool->Execute(pFuncPtr, std::move(Tmp), pThreadName);
}

void CAccounts::Save(int ClientID, CAccountData *pAccountData)
{
	ExecUserThread(SaveThread, "save user", ClientID, "", "", "", pAccountData);
}

bool CAccounts::SaveThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize)
{
	const CSqlAccountRequest *pData = dynamic_cast<const CSqlAccountRequest *>(pGameData);
	CAccountResult *pResult = dynamic_cast<CAccountResult *>(pGameData->m_pResult.get());
	pResult->SetVariant(CAccountResult::LOG_ONLY);

	char aBuf[2048];
	str_copy(aBuf,
		"UPDATE Accounts SET "
		"	IsLoggedIn = ?,"
		"	LastLogoutIGN1 = ?, LastLogoutIGN2 = ?, LastLogoutIGN3 = ?, LastLogoutIGN4 = ?, LastLogoutIGN5 = ?,"
		"	IP_1 = ?, IP_2 = ?, IP_3 = ?,"
		"	Clan1 = ?, Clan2 = ?, Clan3 = ?,"
		"	Skin = ?,"
		"	Level = ?, Money = ?, Exp = ?,"
		"	Shit = ?, LastGift = ?,"
		"	PoliceRank = ?,"
		"	JailTime = ?, EscapeTime = ?,"
		"	TaserLevel = ?,"
		"	PvPArenaTickets = ?, PvPArenaGames = ?, PvPArenaKills = ?, PvPArenaDeaths = ?,"
		"	ProfileStyle = ?, ProfileViews = ?, ProfileStatus = ?,"
		"	ProfileSkype = ?, ProfileYoutube = ?, ProfileEmail = ?, ProfileHomepage = ?, ProfileTwitter = ?,"
		"	HomingMissiles = ?,"
		"	BlockPoints = ?, BlockKills = ?, BlockDeaths = ?, BlockSkill = ?,"
		"	IsModerator = ?, IsSuperModerator = ?, IsSupporter = ?, IsAccFrozen = ?,"
		"	BombGamesPlayed = ?, BombGamesWon = ?, BombBanTime = ?,"
		"	GrenadeKills = ?, GrenadeDeaths = ?, GrenadeSpree = ?,"
		"	GrenadeShots = ?, GrenadeShotsNoRJ = ?, GrenadeWins = ?,"
		"	RifleKills = ?, RifleDeaths = ?, RifleSpree = ?,"
		"	RifleShots = ?, RifleWins = ?,"
		"	FngConfig = ?, ShowHideConfig = ?,"
		"	SurvivalKills = ?, SurvivalDeaths = ?, SurvivalWins = ?,"
		"	NinjaJetpackBought = ?, SpookyGhost = ?,"
		"	UseSpawnWeapons = ?,"
		"	SpawnWeaponShotgun = ?, SpawnWeaponGrenade = ?, SpawnWeaponRifle = ?,"
		"	AsciiState = ?, AsciiViewsDefault = ?, AsciiViewsProfile = ?,"
		"	AsciiFrame0 = ?,"
		"	AsciiFrame1 = ?, AsciiFrame2 = ?, AsciiFrame3 = ?, AsciiFrame4 = ?, AsciiFrame5 = ?,"
		"	AsciiFrame6 = ?, AsciiFrame7 = ?, AsciiFrame8 = ?, AsciiFrame9 = ?, AsciiFrame10 = ?,"
		"	AsciiFrame11 = ?, AsciiFrame12 = ?, AsciiFrame13 = ?, AsciiFrame14 = ?, AsciiFrame15 = ?"
		"	WHERE ID = ?;",
		sizeof(aBuf));

	if(pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
	{
		return true;
	}
	const CAccountData *pAcc = &pData->m_AccountData;
	int Index = 1;
	pSqlServer->BindInt(Index++, pAcc->m_IsLoggedIn);
	pSqlServer->BindString(Index++, pAcc->m_LastLogoutIGN1);
	pSqlServer->BindString(Index++, pAcc->m_LastLogoutIGN2);
	pSqlServer->BindString(Index++, pAcc->m_LastLogoutIGN3);
	pSqlServer->BindString(Index++, pAcc->m_LastLogoutIGN4);
	pSqlServer->BindString(Index++, pAcc->m_LastLogoutIGN5);
	pSqlServer->BindString(Index++, pAcc->m_aIP_1);
	pSqlServer->BindString(Index++, pAcc->m_aIP_2);
	pSqlServer->BindString(Index++, pAcc->m_aIP_3);
	pSqlServer->BindString(Index++, pAcc->m_aClan1);
	pSqlServer->BindString(Index++, pAcc->m_aClan2);
	pSqlServer->BindString(Index++, pAcc->m_aClan3);
	pSqlServer->BindString(Index++, pAcc->m_aSkin);
	pSqlServer->BindInt64(Index++, pAcc->m_Level);
	pSqlServer->BindInt64(Index++, pAcc->m_Money);
	pSqlServer->BindInt64(Index++, pAcc->m_XP);
	pSqlServer->BindInt(Index++, pAcc->m_Shit);
	pSqlServer->BindInt(Index++, pAcc->m_GiftDelay);
	pSqlServer->BindInt(Index++, pAcc->m_PoliceRank);
	pSqlServer->BindInt64(Index++, pAcc->m_JailTime);
	pSqlServer->BindInt64(Index++, pAcc->m_EscapeTime);
	pSqlServer->BindInt(Index++, pAcc->m_TaserLevel);
	pSqlServer->BindInt(Index++, pAcc->m_PvpArenaTickets);
	pSqlServer->BindInt(Index++, pAcc->m_PvpArenaGamesPlayed);
	pSqlServer->BindInt(Index++, pAcc->m_PvpArenaKills);
	pSqlServer->BindInt(Index++, pAcc->m_PvpArenaDeaths);
	pSqlServer->BindInt(Index++, pAcc->m_ProfileStyle);
	pSqlServer->BindInt(Index++, pAcc->m_ProfileViews);
	pSqlServer->BindString(Index++, pAcc->m_ProfileStatus);
	pSqlServer->BindString(Index++, pAcc->m_ProfileSkype);
	pSqlServer->BindString(Index++, pAcc->m_ProfileYoutube);
	pSqlServer->BindString(Index++, pAcc->m_ProfileEmail);
	pSqlServer->BindString(Index++, pAcc->m_ProfileHomepage);
	pSqlServer->BindString(Index++, pAcc->m_ProfileTwitter);
	pSqlServer->BindInt(Index++, pAcc->m_HomingMissilesAmmo);
	pSqlServer->BindInt(Index++, pAcc->m_BlockPoints);
	pSqlServer->BindInt(Index++, pAcc->m_BlockPoints_Kills);
	pSqlServer->BindInt(Index++, pAcc->m_BlockPoints_Deaths);
	pSqlServer->BindInt(Index++, pAcc->m_BlockSkill);
	pSqlServer->BindInt(Index++, pAcc->m_IsModerator);
	pSqlServer->BindInt(Index++, pAcc->m_IsSuperModerator);
	pSqlServer->BindInt(Index++, pAcc->m_IsSupporter);
	pSqlServer->BindInt(Index++, pAcc->m_IsAccFrozen);
	pSqlServer->BindInt(Index++, pAcc->m_BombGamesPlayed);
	pSqlServer->BindInt(Index++, pAcc->m_BombGamesWon);
	pSqlServer->BindInt(Index++, pAcc->m_BombBanTime);
	pSqlServer->BindInt(Index++, pAcc->m_GrenadeKills);
	pSqlServer->BindInt(Index++, pAcc->m_GrenadeDeaths);
	pSqlServer->BindInt(Index++, pAcc->m_GrenadeSpree);
	pSqlServer->BindInt(Index++, pAcc->m_GrenadeShots);
	pSqlServer->BindInt(Index++, pAcc->m_GrenadeShotsNoRJ);
	pSqlServer->BindInt(Index++, pAcc->m_GrenadeWins);
	pSqlServer->BindInt(Index++, pAcc->m_RifleKills);
	pSqlServer->BindInt(Index++, pAcc->m_RifleDeaths);
	pSqlServer->BindInt(Index++, pAcc->m_RifleSpree);
	pSqlServer->BindInt(Index++, pAcc->m_RifleShots);
	pSqlServer->BindInt(Index++, pAcc->m_RifleWins);
	pSqlServer->BindString(Index++, pAcc->m_aFngConfig);
	pSqlServer->BindString(Index++, pAcc->m_aShowHideConfig);
	pSqlServer->BindInt(Index++, pAcc->m_SurvivalKills);
	pSqlServer->BindInt(Index++, pAcc->m_SurvivalDeaths);
	pSqlServer->BindInt(Index++, pAcc->m_SurvivalWins);
	pSqlServer->BindInt(Index++, pAcc->m_NinjaJetpackBought);
	pSqlServer->BindInt(Index++, pAcc->m_SpookyGhost);
	pSqlServer->BindInt(Index++, pAcc->m_UseSpawnWeapons);
	pSqlServer->BindInt(Index++, pAcc->m_SpawnWeaponShotgun);
	pSqlServer->BindInt(Index++, pAcc->m_SpawnWeaponGrenade);
	pSqlServer->BindInt(Index++, pAcc->m_SpawnWeaponRifle);
	pSqlServer->BindString(Index++, pAcc->m_aAsciiPublishState);
	pSqlServer->BindInt(Index++, pAcc->m_AsciiViewsDefault);
	pSqlServer->BindInt(Index++, pAcc->m_AsciiViewsProfile);
	for(const auto &AsciiFrame : pAcc->m_aAsciiFrame)
		pSqlServer->BindString(Index++, AsciiFrame);
	pSqlServer->BindInt(Index++, pAcc->m_ID);

	bool End;
	if(pSqlServer->Step(&End, pError, ErrorSize))
	{
		return true;
	}
	if(!End)
	{
		str_format(pResult->m_aaMessages[0],
			sizeof(pResult->m_aaMessages[0]),
			"save ID=%d finished with status=fail",
			pData->m_AccountData.m_ID);
		return true;
	}
	str_format(pResult->m_aaMessages[0],
		sizeof(pResult->m_aaMessages[0]),
		"save ID=%d finished with status=success",
		pData->m_AccountData.m_ID);
	return false;
}

void CAccounts::Login(int ClientID, const char *pUsername, const char *pPassword)
{
	ExecUserThread(LoginThread, "login user", ClientID, pUsername, pPassword, "", NULL);
}

bool CAccounts::LoginThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize)
{
	const CSqlAccountRequest *pData = dynamic_cast<const CSqlAccountRequest *>(pGameData);
	CAccountResult *pResult = dynamic_cast<CAccountResult *>(pGameData->m_pResult.get());

	char aBuf[2048];
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
		"	TaserLevel,"
		"	PvPArenaTickets, PvPArenaGames, PvPArenaKills, PvPArenaDeaths,"
		"	ProfileStyle, ProfileViews, ProfileStatus,"
		"	ProfileSkype, ProfileYoutube, ProfileEmail, ProfileHomepage, ProfileTwitter,"
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
		"	AsciiFrame11, AsciiFrame12, AsciiFrame13, AsciiFrame14, AsciiFrame15 "
		"	FROM Accounts "
		"	WHERE Username = ? AND Password = ?;",
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
			pResult->SetVariant(CAccountResult::LOGGED_IN_ALREADY);
			pResult->m_Account.m_ID = pSqlServer->GetInt(1);
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
		pResult->m_Account.m_GrenadeShotsNoRJ = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_GrenadeWins = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_RifleKills = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_RifleDeaths = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_RifleSpree = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_RifleShots = pSqlServer->GetInt(Index++);
		pResult->m_Account.m_RifleWins = pSqlServer->GetInt(Index++);
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
		for(auto &AsciiFrame : pResult->m_Account.m_aAsciiFrame)
		{
			pSqlServer->GetString(Index, AsciiFrame, sizeof(AsciiFrame));
			Index++;
		}
	}
	else
	{
		pResult->SetVariant(CAccountResult::LOGIN_WRONG_PASS);
		str_copy(pResult->m_Account.m_aPassword, pData->m_aPassword, sizeof(pResult->m_Account.m_aPassword));
		str_copy(pResult->m_Account.m_aUsername, pData->m_aUsername, sizeof(pResult->m_Account.m_aUsername));
	}
	return false;
}

void CAccounts::UpdateAccountState(int AdminClientID, int TargetAccountID, int State, CAdminCommandResult::Variant Type, const char *pQuery)
{
	ExecAdminThread(UpdateAccountStateThread, "update account state", AdminClientID, TargetAccountID, State, Type, "", "", pQuery);
}

bool CAccounts::UpdateAccountStateThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize)
{
	const CSqlAdminCommandRequest *pData = dynamic_cast<const CSqlAdminCommandRequest *>(pGameData);
	CAdminCommandResult *pResult = dynamic_cast<CAdminCommandResult *>(pGameData->m_pResult.get());
	pResult->SetVariant(pData->m_Type, pData);

	// char aBuf[2048];
	// str_copy(aBuf,
	// 	"UPDATE Accounts SET "
	// 	"	IsFrozen = ?"
	// 	"	WHERE ID = ?;",
	// 	sizeof(aBuf));

	if(pSqlServer->PrepareStatement(pData->m_aQuery, pError, ErrorSize))
	{
		return true;
	}
	pSqlServer->BindInt(1, pData->m_State);
	pSqlServer->BindInt(2, pData->m_TargetAccountID);

	bool End;
	if(pSqlServer->Step(&End, pError, ErrorSize))
	{
		return true;
	}
	if(!End)
	{
		str_copy(pResult->m_aaMessages[0],
			"[ACCOUNT] Update state failed.",
			sizeof(pResult->m_aaMessages[0]));
	}
	else
	{
		str_copy(pResult->m_aaMessages[0],
			"[ACCOUNT] Successfully updated account state.",
			sizeof(pResult->m_aaMessages[0]));
	}
	return false;
}

void CAccounts::AdminSetPassword(int ClientID, const char *pUsername, const char *pPassword)
{
	ExecUserThread(AdminSetPasswordThread, "admin set password", ClientID, pUsername, pPassword, "", NULL);
}

bool CAccounts::AdminSetPasswordThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize)
{
	const CSqlAccountRequest *pData = dynamic_cast<const CSqlAccountRequest *>(pGameData);
	CAccountResult *pResult = dynamic_cast<CAccountResult *>(pGameData->m_pResult.get());
	pResult->SetVariant(CAccountResult::DIRECT);

	char aBuf[2048];
	str_copy(aBuf,
		"SELECT ID FROM Accounts WHERE Username = ?;",
		sizeof(aBuf));

	if(pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
		return true;

	pSqlServer->BindString(1, pData->m_aUsername);

	bool End;
	if(pSqlServer->Step(&End, pError, ErrorSize))
		return true;

	if(End)
	{
		str_copy(pResult->m_aaMessages[0],
			"[SQL] Username not found.",
			sizeof(pResult->m_aaMessages[0]));
	}
	else
	{
		str_copy(aBuf,
			"UPDATE Accounts SET "
			"	Password = ?"
			"	WHERE Username = ?;",
			sizeof(aBuf));

		if(pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
		{
			return true;
		}
		pSqlServer->BindString(1, pData->m_aPassword);
		pSqlServer->BindString(2, pData->m_aUsername);

		if(pSqlServer->Step(&End, pError, ErrorSize))
		{
			return true;
		}
		if(!End)
		{
			str_copy(pResult->m_aaMessages[0],
				"[SQL] Password set failed.",
				sizeof(pResult->m_aaMessages[0]));
		}
		else
		{
			str_copy(pResult->m_aaMessages[0],
				"[SQL] Successfully updated password.",
				sizeof(pResult->m_aaMessages[0]));
		}
	}
	return false;
}

void CAccounts::ChangePassword(int ClientID, const char *pUsername, const char *pOldPassword, const char *pNewPassword)
{
	ExecUserThread(ChangePasswordThread, "change password", ClientID, pUsername, pOldPassword, pNewPassword, NULL);
}

bool CAccounts::ChangePasswordThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize)
{
	const CSqlAccountRequest *pData = dynamic_cast<const CSqlAccountRequest *>(pGameData);
	CAccountResult *pResult = dynamic_cast<CAccountResult *>(pGameData->m_pResult.get());
	pResult->SetVariant(CAccountResult::DIRECT);

	char aBuf[2048];
	str_copy(aBuf,
		"UPDATE Accounts SET "
		"	Password = ?"
		"	WHERE Username = ?;",
		sizeof(aBuf));

	if(pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
	{
		return true;
	}
	pSqlServer->BindString(1, pData->m_aUsername);
	pSqlServer->BindString(2, pData->m_aNewPassword);

	bool End;
	if(pSqlServer->Step(&End, pError, ErrorSize))
	{
		return true;
	}
	if(!End)
	{
		str_copy(pResult->m_aaMessages[0],
			"[ACCOUNT] Password change failed.",
			sizeof(pResult->m_aaMessages[0]));
	}
	else
	{
		str_copy(pResult->m_aaMessages[0],
			"[ACCOUNT] Successfully changed your password.",
			sizeof(pResult->m_aaMessages[0]));
	}
	return false;
}

void CAccounts::ExecuteSQL(const char *pQuery)
{
	auto Tmp = std::make_unique<CSqlStringData>();
	str_copy(Tmp->m_aString, pQuery, sizeof(Tmp->m_aString));

	m_pPool->ExecuteWrite(ExecuteSQLThread, std::move(Tmp), "add table column");
}

bool CAccounts::ExecuteSQLThread(IDbConnection *pSqlServer, const ISqlData *pGameData, Write w, char *pError, int ErrorSize)
{
	if(w != Write::NORMAL && w != Write::NORMAL_FAILED)
	{
		dbg_assert(false, "ExecuteSQLThread failed to write");
		return true;
	}

	const CSqlStringData *pData = dynamic_cast<const CSqlStringData *>(pGameData);

	// char aBuf[512];
	// str_copy(aBuf,
	// 	"ALTER TABLE 'Accounts' ADD ColName VARCHAR(64) DEFAULT ''",
	// 	sizeof(aBuf));

	if(pSqlServer->PrepareStatement(pData->m_aString, pError, ErrorSize))
	{
		return true;
	}

	bool End;
	if(pSqlServer->Step(&End, pError, ErrorSize))
	{
		dbg_assert(false, "ExecuteSQLThread did not step");
		return true;
	}
	return !End;
}

void CAccounts::LogoutUsername(const char *pUsername)
{
	auto Tmp = std::make_unique<CSqlStringData>();
	str_copy(Tmp->m_aString, pUsername, sizeof(Tmp->m_aString));

	m_pPool->ExecuteWrite(LogoutUsernameThread, std::move(Tmp), "logout username");
}

bool CAccounts::LogoutUsernameThread(IDbConnection *pSqlServer, const ISqlData *pGameData, Write w, char *pError, int ErrorSize)
{
	if(w != Write::NORMAL && w != Write::NORMAL_FAILED)
	{
		dbg_assert(false, "LogoutUsernameThread failed to write");
		return true;
	}
	const CSqlStringData *pData = dynamic_cast<const CSqlStringData *>(pGameData);

	char aBuf[512];
	str_copy(aBuf,
		"UPDATE Accounts SET IsLoggedIn = 0 WHERE Username = ?;",
		sizeof(aBuf));

	if(pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
	{
		return true;
	}
	pSqlServer->BindString(1, pData->m_aString);

	bool End;
	if(pSqlServer->Step(&End, pError, ErrorSize))
	{
		dbg_assert(false, "LogoutUsernameThread did not step");
		return true;
	}
	return !End;
}

void CAccounts::CleanZombieAccounts(int ClientID, int Port, const char *pQuery)
{
	auto Tmp = std::make_unique<CSqlCleanZombieAccountsData>();
	Tmp->m_ClientID = ClientID;
	Tmp->m_Port = Port;
	str_copy(Tmp->m_aQuery, pQuery, sizeof(Tmp->m_aQuery));

	m_pPool->ExecuteWrite(CleanZombieAccountsThread, std::move(Tmp), "clean zombies");
}

bool CAccounts::CleanZombieAccountsThread(IDbConnection *pSqlServer, const ISqlData *pGameData, Write w, char *pError, int ErrorSize)
{
	if(w != Write::NORMAL && w != Write::NORMAL_FAILED)
	{
		dbg_assert(false, "CleanZombieAccountsThread failed to write");
		return true;
	}
	const CSqlCleanZombieAccountsData *pData = dynamic_cast<const CSqlCleanZombieAccountsData *>(pGameData);

	// char aBuf[512];
	// str_copy(aBuf,
	// 	"UPDATE Accounts SET IsLoggedIn = 0, LastLoginPort = ?;",
	// 	sizeof(aBuf));

	if(pSqlServer->PrepareStatement(pData->m_aQuery, pError, ErrorSize))
	{
		return true;
	}
	pSqlServer->BindInt(1, pData->m_Port);

	bool End;
	if(pSqlServer->Step(&End, pError, ErrorSize))
	{
		dbg_assert(false, "CleanZombieAccountsThread did not step");
		return true;
	}
	return !End;
}

void CAccounts::SetLoggedIn(int ClientID, int LoggedIn, int AccountID, int Port)
{
	auto Tmp = std::make_unique<CSqlSetLoginData>();
	Tmp->m_Port = Port;
	Tmp->m_LoggedIn = LoggedIn;
	Tmp->m_AccountID = AccountID;

	m_pPool->ExecuteWrite(SetLoggedInThread, std::move(Tmp), "set logged in");
}

bool CAccounts::SetLoggedInThread(IDbConnection *pSqlServer, const ISqlData *pGameData, Write w, char *pError, int ErrorSize)
{
	if(w != Write::NORMAL && w != Write::NORMAL_FAILED)
	{
		dbg_assert(false, "SetLoggedInThread failed to write");
		return true;
	}
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
	return !End;
}

void CAccounts::Register(int ClientID, const char *pUsername, const char *pPassword)
{
	ExecUserThread(RegisterThread, "register user", ClientID, pUsername, pPassword, "", NULL);
}

bool CAccounts::RegisterThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize)
{
	const CSqlAccountRequest *pData = dynamic_cast<const CSqlAccountRequest *>(pGameData);
	CAccountResult *pResult = dynamic_cast<CAccountResult *>(pGameData->m_pResult.get());

	char aBuf[2048];
	str_copy(aBuf,
		"SELECT ID FROM Accounts WHERE Username = ?;",
		sizeof(aBuf));

	if(pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
		return true;

	pSqlServer->BindString(1, pData->m_aUsername);

	bool End;
	if(pSqlServer->Step(&End, pError, ErrorSize))
		return true;

	if(!End)
	{
		pResult->SetVariant(CAccountResult::DIRECT);
		str_copy(pResult->m_aaMessages[0],
			"[ACCOUNT] Username already exists.",
			sizeof(pResult->m_aaMessages[0]));
	}
	else
	{
		pResult->SetVariant(CAccountResult::REGISTER);
		char aDate[64];
		time_t RawTime;
		struct tm *TimeInfo;
		time(&RawTime);
		TimeInfo = localtime(&RawTime);
		str_format(aDate,
			sizeof(aDate),
			"%d-%02d-%02d_%02d:%02d:%02d",
			TimeInfo->tm_year + 1900,
			TimeInfo->tm_mon + 1,
			TimeInfo->tm_mday,
			TimeInfo->tm_hour,
			TimeInfo->tm_min,
			TimeInfo->tm_sec);
		str_copy(aBuf,
			"INSERT INTO Accounts "
			"  (Username, Password, RegisterDate) "
			"  VALUES "
			"  (?, ?, ?);",
			sizeof(aBuf));

		if(pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
			return true;

		pSqlServer->BindString(1, pData->m_aUsername);
		pSqlServer->BindString(2, pData->m_aPassword);
		pSqlServer->BindString(3, aDate);

		if(pSqlServer->Step(&End, pError, ErrorSize))
			return true;

		if(!End)
		{
			str_copy(pResult->m_aaMessages[0],
				"[ACCOUNT] Something went wrong.",
				sizeof(pResult->m_aaMessages[0]));
		}
		else
		{
			str_copy(pResult->m_aaMessages[0],
				"[ACCOUNT] Account has been registered.",
				sizeof(pResult->m_aaMessages[0]));
			str_copy(pResult->m_aaMessages[1],
				"[ACCOUNT] Login with: /login <name> <pass>",
				sizeof(pResult->m_aaMessages[1]));
		}
	}
	return false;
}

void CAccounts::CreateDatabase()
{
	auto Tmp = std::make_unique<CSqlCreateTableRequest>();
	m_pPool->ExecuteWrite(CreateTableThread, std::move(Tmp), "create table");
}

bool CAccounts::CreateTableThread(IDbConnection *pSqlServer, const ISqlData *pGameData, Write w, char *pError, int ErrorSize)
{
	if(w != Write::NORMAL && w != Write::NORMAL_FAILED)
	{
		dbg_assert(false, "CreateTableThread failed to write");
		return true;
	}
	// CSqlCreateTableRequest *pResult = dynamic_cast<CSqlCreateTableRequest *>(pGameData->m_pResult.get());

	char aBuf[4096];
	str_copy(aBuf,
		"CREATE TABLE IF NOT EXISTS Accounts ("
		"  ID					INTEGER			PRIMARY KEY		AUTOINCREMENT,"
		"  Username				VARCHAR(32)		NOT NULL,"
		"  Password				VARCHAR(128)	NOT NULL,"
		"  RegisterDate			VARCHAR(32)		DEFAULT '',"
		"  IsLoggedIn			INTEGER			DEFAULT 0,"
		"  LastLoginPort		INTEGER			DEFAULT 0,"
		"  LastLogoutIGN1		VARCHAR(32)		DEFAULT '',"
		"  LastLogoutIGN2		VARCHAR(32)		DEFAULT '',"
		"  LastLogoutIGN3		VARCHAR(32)		DEFAULT '',"
		"  LastLogoutIGN4		VARCHAR(32)		DEFAULT '',"
		"  LastLogoutIGN5		VARCHAR(32)		DEFAULT '',"
		"  IP_1					VARCHAR(32)		DEFAULT '',"
		"  IP_2					VARCHAR(32)		DEFAULT '',"
		"  IP_3					VARCHAR(32)		DEFAULT '',"
		"  Clan1				VARCHAR(32)		DEFAULT '',"
		"  Clan2				VARCHAR(32)		DEFAULT '',"
		"  Clan3				VARCHAR(32)		DEFAULT '',"
		"  Skin					VARCHAR(32)		DEFAULT '',"
		"  Level				INTEGER			DEFAULT 0,"
		"  Money				INTEGER			DEFAULT 0,"
		"  Exp					INTEGER			DEFAULT 0,"
		"  Shit					INTEGER			DEFAULT 0,"
		"  LastGift				INTEGER			DEFAULT 0,"
		"  PoliceRank			INTEGER			DEFAULT 0,"
		"  JailTime				INTEGER			DEFAULT 0,"
		"  EscapeTime			INTEGER			DEFAULT 0,"
		"  TaserLevel			INTEGER			DEFAULT 0,"
		"  PvPArenaTickets		INTEGER			DEFAULT 0,"
		"  PvPArenaGames		INTEGER			DEFAULT 0,"
		"  PvPArenaKills		INTEGER			DEFAULT 0,"
		"  PvPArenaDeaths		INTEGER			DEFAULT 0,"
		"  ProfileStyle			INTEGER			DEFAULT 0,"
		"  ProfileViews			INTEGER			DEFAULT 0,"
		"  ProfileStatus		VARCHAR(128)	DEFAULT '',"
		"  ProfileSkype			VARCHAR(128)	DEFAULT '',"
		"  ProfileYoutube		VARCHAR(128)	DEFAULT '',"
		"  ProfileEmail			VARCHAR(128)	DEFAULT '',"
		"  ProfileHomepage		VARCHAR(128)	DEFAULT '',"
		"  ProfileTwitter		VARCHAR(128)	DEFAULT '',"
		"  HomingMissiles		INTEGER			DEFAULT 0,"
		"  BlockPoints			INTEGER			DEFAULT 0,"
		"  BlockKills			INTEGER			DEFAULT 0,"
		"  BlockDeaths			INTEGER			DEFAULT 0,"
		"  BlockSkill			INTEGER			DEFAULT 0,"
		"  IsModerator			INTEGER			DEFAULT 0,"
		"  IsSuperModerator		INTEGER			DEFAULT 0,"
		"  IsSupporter			INTEGER			DEFAULT 0,"
		"  IsAccFrozen			INTEGER			DEFAULT 0,"
		"  BombGamesPlayed		INTEGER			DEFAULT 0,"
		"  BombGamesWon			INTEGER			DEFAULT 0,"
		"  BombBanTime			INTEGER			DEFAULT 0,"
		"  GrenadeKills			INTEGER			DEFAULT 0,"
		"  GrenadeDeaths		INTEGER			DEFAULT 0,"
		"  GrenadeSpree			INTEGER			DEFAULT 0,"
		"  GrenadeShots			INTEGER			DEFAULT 0,"
		"  GrenadeShotsNoRJ		INTEGER			DEFAULT 0,"
		"  GrenadeWins			INTEGER			DEFAULT 0,"
		"  RifleKills			INTEGER			DEFAULT 0,"
		"  RifleDeaths			INTEGER			DEFAULT 0,"
		"  RifleSpree			INTEGER			DEFAULT 0,"
		"  RifleShots			INTEGER			DEFAULT 0,"
		"  RifleWins			INTEGER			DEFAULT 0,"
		"  FngConfig			VARCHAR(4)		DEFAULT '000',"
		"  ShowHideConfig		VARCHAR(16)		DEFAULT '0010000000',"
		"  SurvivalKills		INTEGER			DEFAULT 0,"
		"  SurvivalDeaths		INTEGER			DEFAULT 0,"
		"  SurvivalWins			INTEGER			DEFAULT 0,"
		"  NinjaJetpackBought	INTEGER			DEFAULT 0,"
		"  SpookyGhost			INTEGER			DEFAULT 0,"
		"  UseSpawnWeapons		INTEGER			DEFAULT 0,"
		"  SpawnWeaponShotgun	INTEGER			DEFAULT 0,"
		"  SpawnWeaponGrenade	INTEGER			DEFAULT 0,"
		"  SpawnWeaponRifle		INTEGER			DEFAULT 0,"
		"  AsciiState			VARCHAR(4)		DEFAULT '',"
		"  AsciiViewsDefault	INTEGER			DEFAULT 0,"
		"  AsciiViewsProfile	INTEGER			DEFAULT 0,"
		"  AsciiFrame0			VARCHAR(64)		DEFAULT '',"
		"  AsciiFrame1			VARCHAR(64)		DEFAULT '',"
		"  AsciiFrame2			VARCHAR(64)		DEFAULT '',"
		"  AsciiFrame3			VARCHAR(64)		DEFAULT '',"
		"  AsciiFrame4			VARCHAR(64)		DEFAULT '',"
		"  AsciiFrame5			VARCHAR(64)		DEFAULT '',"
		"  AsciiFrame6			VARCHAR(64)		DEFAULT '',"
		"  AsciiFrame7			VARCHAR(64)		DEFAULT '',"
		"  AsciiFrame8			VARCHAR(64)		DEFAULT '',"
		"  AsciiFrame9			VARCHAR(64)		DEFAULT '',"
		"  AsciiFrame10			VARCHAR(64)		DEFAULT '',"
		"  AsciiFrame11			VARCHAR(64)		DEFAULT '',"
		"  AsciiFrame12			VARCHAR(64)		DEFAULT '',"
		"  AsciiFrame13			VARCHAR(64)		DEFAULT '',"
		"  AsciiFrame14			VARCHAR(64)		DEFAULT '',"
		"  AsciiFrame15			VARCHAR(64)		DEFAULT '');",
		sizeof(aBuf));

	if(pSqlServer->PrepareStatement(aBuf, pError, ErrorSize))
	{
		return true;
	}

	bool End;
	if(pSqlServer->Step(&End, pError, ErrorSize))
	{
		return true;
	}
	if(!End)
	{
		// str_copy(pResult->m_aaMessages[0],
		// 	"create table finished with status=fail",
		// 	sizeof(pResult->m_aaMessages[0]));
		return true;
	}
	// str_copy(pResult->m_aaMessages[0],
	// 	"create table finished with status=success",
	// 	sizeof(pResult->m_aaMessages[0]));
	return false;
}
