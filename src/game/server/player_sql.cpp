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
		case CAccountResult::LOGIN_INFO:
			GameServer()->SendChatTarget(m_ClientID, "login  suscce");
			m_Account = Result.m_Account;
			break;
		}
	}
}
