/* CPlayer related sql ddnet++ methods */

#include "gamemodes/DDRace.h"
#include <engine/shared/config.h>

#include <fstream>
#include <limits>

#include "gamecontext.h"
#include "gamecontext_sql.h"

#include "player.h"

void CPlayer::DDPPProcessScoreResult(CAccountResult &Result)
{
	if(Result.m_Success) // SQL request was successful
	{
		switch(Result.m_MessageKind)
		{
		case CAccountResult::DIRECT:
			for(auto &aMessage : Result.m_aaMessages)
			{
				if(aMessage[0] == 0)
					break;
				GameServer()->SendChatTarget(m_ClientID, aMessage);
			}
			break;
		case CAccountResult::ALL:
		{
			bool PrimaryMessage = true;
			for(auto &aMessage : Result.m_aaMessages)
			{
				if(aMessage[0] == 0)
					break;

				if(GameServer()->ProcessSpamProtection(m_ClientID) && PrimaryMessage)
					break;

				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aMessage, -1);
				PrimaryMessage = false;
			}
			break;
		}
		case CAccountResult::BROADCAST:
			// if(Result.m_aBroadcast[0] != 0)
			// 	GameServer()->SendBroadcast(Result.m_aBroadcast, -1);
			break;
		case CAccountResult::LOGGED_IN_ALREADY:
			if(GameServer()->CheckAccounts(Result.m_Account.m_ID))
				GameServer()->SendChatTarget(m_ClientID, "[ACCOUNT] This account is already logged in on this server.");
			else
				GameServer()->SendChatTarget(m_ClientID, "[ACCOUNT] Login failed. This account is already logged in on another server.");
			dbg_msg("account", "sql id=%d is already logged in.", Result.m_Account.m_ID);
			break;
		case CAccountResult::LOGIN_INFO:
			GameServer()->SendChatTarget(m_ClientID, "login  suscce");
			m_Account = Result.m_Account;
			GameServer()->Accounts()->SetLoggedIn(m_ClientID, 1, m_Account.m_ID, g_Config.m_SvPort);
			break;
		case CAccountResult::LOG_ONLY:
			for(auto &aMessage : Result.m_aaMessages)
			{
				if(aMessage[0] == 0)
					break;
				dbg_msg("account", "%s", aMessage);
			}
			break;
		}
	}
}
