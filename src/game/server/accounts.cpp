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

void CAccounts::Login(int ClientID, const char *pUsername, const char *pPassword)
{
	ExecUserThread(LoginThread, "login user", ClientID, pUsername, pPassword);
}

bool CAccounts::LoginThread(IDbConnection *pSqlServer, const ISqlData *pGameData, char *pError, int ErrorSize)
{
	const CSqlAccountRequest *pData = dynamic_cast<const CSqlAccountRequest *>(pGameData);
	CAccountResult *pResult = dynamic_cast<CAccountResult *>(pGameData->m_pResult.get());

	// check sort method
	char aBuf[512];
	str_copy(aBuf,
		"SELECT "
		"	Username, Password,"
		"	RegisterDate, IsLoggedIn, LastLoginPort,"
		"	LastLogoutIGN1, LastLogoutIGN2, LastLogoutIGN3, LastLogoutIGN4, LastLogoutIGN5,"
		"	IP_1, IP_2, IP_3,"
		"	Clan1, Clan2, Clan3,"
		"	Skin," // 17
		"	Level, Money, Exp,"
		"	Shit, LastGift,"
		"	PoliceRank,"
		"	JailTime, EscapeTime,"
		"	TaserLevel "
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
		// pSqlServer->GetText(1); // username
		// pSqlServer->GetText(2); // password
		pResult->SetVariant(CAccountResult::LOGIN_INFO);
		pResult->m_Data.m_Info.m_level = pSqlServer->GetInt(18);
		pResult->m_Data.m_Info.m_money = pSqlServer->GetInt(19);
	}
	else
	{
		pResult->SetVariant(CAccountResult::DIRECT);
		str_copy(pResult->m_Data.m_aaMessages[0],
			"[ACCOUNT] Login failed. Wrong password or username.",
			sizeof(pResult->m_Data.m_aaMessages[0]));
	}
	return false;
}