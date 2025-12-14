#include <base/log.h>

#include <game/server/ddpp/accounts.h>
#include <game/server/gamemodes/ddnetpp/ddnetpp.h>

void CGameControllerDDNetPP::LogoutAllAccounts()
{
	for(CPlayer *pPlayer : GameServer()->m_apPlayers)
	{
		if(!pPlayer)
			continue;
		if(!pPlayer->IsLoggedIn())
			continue;

		GameServer()->SendChatTarget(pPlayer->GetCid(), "[ACCOUNT] you have been logged out due to changes in the system");
		pPlayer->Logout();
	}
}

bool CGameControllerDDNetPP::IsAccountRconCmdRatelimited(int ClientId, char *pReason, int ReasonSize)
{
	if(pReason)
		pReason[0] = '\0';

	CPlayer *pPlayer = GameServer()->GetPlayerOrNullptr(ClientId);
	if(!pPlayer)
	{
		if(pReason)
			str_copy(pReason, "invalid player", ReasonSize);
		return false;
	}

	bool PendingRconCmd = std::any_of(
		GameServer()->m_vAccountRconCmdQueryResults.begin(),
		GameServer()->m_vAccountRconCmdQueryResults.end(),
		[&](const std::shared_ptr<CAccountRconCmdResult> &Result) {
			if(!Result)
				return false;
			if(Result->m_Completed)
				return false;
			if(Result->m_UniqueClientId != pPlayer->GetUniqueCid())
				return false;
			return true;
		});
	if(PendingRconCmd)
	{
		if(pReason)
			str_copy(pReason, "already pending rcon cmd", ReasonSize);
		return true;
	}

	return false;
}

void CGameControllerDDNetPP::ProcessAccountRconCmdResult(CAccountRconCmdResult &Result)
{
	if(!Result.m_Success)
	{
		log_error("ddnet++", "Account rcon command failed. Check the server logs.");
		return;
	}

	CPlayer *pPlayer = GetPlayerByUniqueId(Result.m_UniqueClientId);

	switch(Result.m_MessageKind)
	{
	case CAccountRconCmdResult::FREEZE_ACC:
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "UPDATED IsAccFrozen = %d (account is not logged in on this server)", Result.m_State);
		CPlayer *pTarget = GameServer()->GetPlayerByAccountId(Result.m_TargetAccountId);

		if(pTarget)
		{
			pTarget->m_Account.m_IsAccFrozen = Result.m_State;
			// always logout and send you got frozen also if he gets unfreezed because if some1 gets unfreezed he is not logged in xd
			pTarget->Logout();
			str_format(aBuf, sizeof(aBuf), "%s. (Reason: Account frozen)", GameServer()->Loc("[ACCOUNT] Logged out", pTarget->GetCid()));
			GameServer()->SendChatTarget(pTarget->GetCid(), aBuf);
			str_format(aBuf, sizeof(aBuf), "UPDATED IsAccFrozen = %d (logged out %d:'%s')", Result.m_State, pTarget->GetCid(), Server()->ClientName(pTarget->GetCid()));
		}

		if(pPlayer)
			GameServer()->SendChatTarget(pPlayer->GetCid(), aBuf);
		else
			log_info("account", "%s", aBuf); // this is for econ
		break;
	}
	case CAccountRconCmdResult::MODERATOR:
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "UPDATED IsModerator = %d (account is not logged in)", Result.m_State);
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(!GameServer()->m_apPlayers[i])
				continue;

			if(GameServer()->m_apPlayers[i]->GetAccId() == Result.m_TargetAccountId)
			{
				GameServer()->m_apPlayers[i]->m_Account.m_IsModerator = Result.m_State;
				if(Result.m_State == 1)
					GameServer()->SendChatTarget(i, "[ACCOUNT] You are now VIP.");
				else
					GameServer()->SendChatTarget(i, "[ACCOUNT] You are no longer VIP.");
				str_format(aBuf, sizeof(aBuf), "UPDATED IsModerator = %d (%d:'%s')", Result.m_State, i, Server()->ClientName(i));
				break;
			}
		}
		if(pPlayer)
			GameServer()->SendChatTarget(pPlayer->GetCid(), aBuf);
		else
			log_info("account", "%s", aBuf); // this is for econ
		break;
	}
	case CAccountRconCmdResult::SUPER_MODERATOR:
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "UPDATED IsSuperModerator = %d (account is not logged in)", Result.m_State);
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(!GameServer()->m_apPlayers[i])
				continue;

			if(GameServer()->m_apPlayers[i]->GetAccId() == Result.m_TargetAccountId)
			{
				GameServer()->m_apPlayers[i]->m_Account.m_IsSuperModerator = Result.m_State;
				if(Result.m_State == 1)
				{
					GameServer()->SendChatTarget(i, "[ACCOUNT] You are now VIP+.");
					CCharacter *pChr = GameServer()->m_apPlayers[i]->GetCharacter();
					if(pChr)
					{
						pChr->Core()->m_DDNetPP.m_RestrictionData.m_CanEnterVipPlusOnly = true;
					}
				}
				else
				{
					GameServer()->SendChatTarget(i, "[ACCOUNT] You are no longer VIP+.");
					CCharacter *pChr = GameServer()->m_apPlayers[i]->GetCharacter();
					if(pChr)
					{
						pChr->Core()->m_DDNetPP.m_RestrictionData.m_CanEnterVipPlusOnly = false;
					}
				}
				str_format(aBuf, sizeof(aBuf), "UPDATED IsSuperModerator = %d (%d:'%s')", Result.m_State, i, Server()->ClientName(i));
				break;
			}
		}
		if(pPlayer)
			GameServer()->SendChatTarget(pPlayer->GetCid(), aBuf);
		else
			log_info("account", "%s", aBuf); // this is for econ
		break;
	}
	case CAccountRconCmdResult::SUPPORTER:
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "UPDATED IsSupporter = %d (account is not logged in)", Result.m_State);
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(!GameServer()->m_apPlayers[i])
				continue;

			if(GameServer()->m_apPlayers[i]->GetAccId() == Result.m_TargetAccountId)
			{
				GameServer()->m_apPlayers[i]->m_Account.m_IsSupporter = Result.m_State;
				if(Result.m_State == 1)
					GameServer()->SendChatTarget(i, "[ACCOUNT] You are now Supporter.");
				else
					GameServer()->SendChatTarget(i, "[ACCOUNT] You are no longer Supporter.");
				str_format(aBuf, sizeof(aBuf), "UPDATED IsSupporter = %d (%d:'%s')", Result.m_State, i, Server()->ClientName(i));
				break;
			}
		}
		if(pPlayer)
			GameServer()->SendChatTarget(pPlayer->GetCid(), aBuf);
		else
			log_info("account", "%s", aBuf); // this is for econ
		break;
	}
	case CAccountRconCmdResult::DIRECT:
		for(auto &aMessage : Result.m_aaMessages)
		{
			if(aMessage[0] == 0)
				break;

			if(pPlayer)
				GameServer()->SendChatTarget(pPlayer->GetCid(), aMessage);
			else
				log_info("account", "%s", aMessage); // this is for econ
		}
		break;
	case CAccountRconCmdResult::ALL:
	{
		bool PrimaryMessage = true;
		for(auto &aMessage : Result.m_aaMessages)
		{
			if(aMessage[0] == 0)
				break;

			if(pPlayer && GameServer()->ProcessSpamProtection(pPlayer->GetCid()) && PrimaryMessage)
				break;

			GameServer()->SendChat(-1, TEAM_ALL, aMessage, -1);
			PrimaryMessage = false;
		}
		break;
	}
	case CAccountRconCmdResult::BROADCAST:
		// if(Result.m_aBroadcast[0] != 0)
		// 	GameServer()->SendBroadcast(Result.m_aBroadcast, -1);
		break;
	case CAccountRconCmdResult::LOG_ONLY:
		for(auto &aMessage : Result.m_aaMessages)
		{
			if(aMessage[0] == 0)
				break;
			log_info("account", "%s", aMessage);
		}
		break;
	}
}
