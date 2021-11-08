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

	// check sort method
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
		if(pSqlServer->GetInt(5)) // IsLoggedIn
		{
			pResult->SetVariant(CAccountResult::DIRECT);
			str_copy(pResult->m_aaMessages[0],
				"[ACCOUNT] Login failed. This account is logged in already.",
				sizeof(pResult->m_aaMessages[0]));
			return false;
		}
		pResult->SetVariant(CAccountResult::LOGIN_INFO);
		pResult->m_Account.m_ID = pSqlServer->GetInt(1);
		// pSqlServer->GetText(2); // username
		// pSqlServer->GetText(3); // password
		pSqlServer->GetString(4, pResult->m_Account.m_aRegisterDate, sizeof(pResult->m_Account.m_aRegisterDate));
		// 5 IsLoggedIn (not needed here)
		// 6 LastLoginPort (not needed)
		// pResult->m_Account.m_level = pSqlServer->GetInt64(19);
		// pResult->m_Account.m_money = pSqlServer->GetInt64(20);
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