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
		for(auto &aMessage : m_Data.m_aaMessages)
			aMessage[0] = 0;
		break;
	case BROADCAST:
		m_Data.m_aBroadcast[0] = 0;
		break;
	case LOGIN_INFO:
		// TODO: initalize all here
		m_Data.m_Info.m_AccountID = -1;
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

bool CAccounts::LoginThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize)
{
	const CSqlAccountRequest *pData = dynamic_cast<const CSqlAccountRequest *>(pGameData);
	CAccountResult *pResult = dynamic_cast<CAccountResult *>(pGameData->m_pResult.get());

	// check sort method
	char aBuf[512];
	str_copy(aBuf,
		"SELECT"
		"	Username, Password,"
		"	RegisterDate, IsLoggedIn, LastLoginPort"
		"	LastLogoutIGN1, LastLogoutIGN2, LastLogoutIGN3, LastLogoutIGN4, LastLogoutIGN5,"
		"	IP_1, IP_2, IP_3,"
		"	Clan1, Clan2, Clan3,"
		"	Skin,"
		"	Level, Money, Exp"
		"	Shit, LastGift,"
		"	PoliceRank,"
		"	JailTime, EscapeTime,"
		"	TaserLevel"
		"FROM Accounts"
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
		// pSqlServer->GetText(1); // username
		// pSqlServer->GetText(2); // password

		// float Time = pSqlServer->GetFloat(3);
		// str_time_float(Time, TIME_HOURS_CENTISECS, aBuf, sizeof(aBuf));
		// int Rank = pSqlServer->GetInt(4);
		// // CEIL and FLOOR are not supported in SQLite
		// int BetterThanPercent = std::floor(100.0 - 100.0 * pSqlServer->GetFloat(5));
		// CTeamrank Teamrank;
		// if(Teamrank.NextSqlResult(pSqlServer, &End, pError, ErrorSize))
		// {
		// 	return true;
		// }

		// char aFormattedNames[512] = "";
		// for(unsigned int Name = 0; Name < Teamrank.m_NumNames; Name++)
		// {
		// 	str_append(aFormattedNames, Teamrank.m_aaNames[Name], sizeof(aFormattedNames));

		// 	if(Name < Teamrank.m_NumNames - 2)
		// 		str_append(aFormattedNames, ", ", sizeof(aFormattedNames));
		// 	else if(Name < Teamrank.m_NumNames - 1)
		// 		str_append(aFormattedNames, " & ", sizeof(aFormattedNames));
		// }

		// if(g_Config.m_SvHideScore)
		// {
		// 	str_format(pResult->m_Data.m_aaMessages[0], sizeof(pResult->m_Data.m_aaMessages[0]),
		// 		"Your team time: %s, better than %d%%", aBuf, BetterThanPercent);
		// }
		// else
		// {
		// 	pResult->m_MessageKind = CScorePlayerResult::ALL;
		// 	str_format(pResult->m_Data.m_aaMessages[0], sizeof(pResult->m_Data.m_aaMessages[0]),
		// 		"%d. %s Team time: %s, better than %d%%, requested by %s",
		// 		Rank, aFormattedNames, aBuf, BetterThanPercent, pData->m_aRequestingPlayer);
		// }
	}
	else
	{
		str_copy(pResult->m_Data.m_aaMessages[0],
			"[ACCOUNT] Login failed. Wrong password or username.",
			sizeof(pResult->m_Data.m_aaMessages[0]));
	}
	return false;
}