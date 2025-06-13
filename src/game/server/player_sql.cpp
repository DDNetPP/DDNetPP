/* CPlayer related sql ddnet++ methods */

#include "gamecontext.h"
#include "player.h"

#include <base/system.h>

#include <engine/shared/config.h>

#include <generated/protocol.h>

#include <game/mapitems.h>

#include <fstream>
#include <limits>

void CPlayer::OnLogin()
{
	//================================
	// ABORT LOGIN AFTER LOADING DATA
	//================================

	if(m_Account.m_IsAccFrozen)
	{
		GameServer()->SendChatTarget(m_ClientId, "[ACCOUNT] Login failed: Account is frozen.");
		Logout();
		return;
	}

	//==========================
	// Done loading stuff
	// Start to do something...
	//==========================

	GameServer()->SendChatTarget(m_ClientId, "[ACCOUNT] Login successful.");
	GameServer()->RefreshExtraVoteMenu(m_ClientId);

	if(g_Config.m_SvRequireLoginToJoin && g_Config.m_SvAccounts)
	{
		if(GetTeam() == TEAM_SPECTATORS)
			SetTeam(TEAM_RED);
	}

	// load scoreboard scores
	if(g_Config.m_SvInstaScore)
	{
		if(g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2) //gdm & zCatch grenade
			m_Score = m_Account.m_GrenadeKills;
		else if(g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4) // idm & zCatch rifle
			m_Score = m_Account.m_RifleKills;
	}

	// nobo spawn
	if(m_NoboSpawnStop > GameServer()->Server()->Tick())
	{
		m_IsNoboSpawn = false;
		m_NoboSpawnStop = 0;
		GameServer()->SendChatTarget(m_ClientId, "[NoboSpawn] Real spawn unlocked due to login.");
	}

	//jail
	if(m_Account.m_JailTime)
	{
		if(GetCharacter())
		{
			vec2 JailPlayerSpawn = GameServer()->Collision()->GetRandomTile(TILE_JAIL);

			if(JailPlayerSpawn != vec2(-1, -1))
			{
				GetCharacter()->SetPosition(JailPlayerSpawn);
			}
			else //no jailplayer
			{
				GameServer()->SendChatTarget(m_ClientId, "No jail set.");
			}
		}
	}

	//auto joins
	if(m_Account.m_aFngConfig[0] == '1') //auto fng join
	{
		if(!g_Config.m_SvAllowInsta)
		{
			GameServer()->SendChatTarget(m_ClientId, "[INSTA] fng autojoin failed because fng is deactivated by an admin.");
		}
		else
		{
			GameServer()->SendChatTarget(m_ClientId, "[INSTA] you automatically joined an fng game. (use '/fng' to change this setting)");
			GameServer()->m_pInstagib->Join(this, WEAPON_LASER, true);
		}
	}
	else if(m_Account.m_aFngConfig[0] == '2') //auto boomfng join
	{
		if(!g_Config.m_SvAllowInsta)
		{
			GameServer()->SendChatTarget(m_ClientId, "[INSTA] boomfng autojoin failed because fng is deactivated by an admin.");
		}
		else
		{
			GameServer()->SendChatTarget(m_ClientId, "[INSTA] you automatically joined an boomfng game. (use '/fng' to change this setting)");
			GameServer()->m_pInstagib->Join(this, WEAPON_GRENADE, true);
		}
	}

	// account pass reset info
	if(!str_comp(m_Account.m_ProfileEmail, "") && !str_comp(m_Account.m_ProfileSkype, ""))
		GameServer()->SendChatTarget(m_ClientId, "[ACCOUNT] set an '/profile email' or '/profile skype' to restore your password if you forget it.");

	CCharacter *pChr = GetCharacter();
	if(pChr)
	{
		pChr->Core()->m_DDNetPP.m_RestrictionData.m_CanEnterVipPlusOnly = m_Account.m_IsSuperModerator;
	}

	//========================================
	// LEAVE THIS CODE LAST!!!!
	// multiple server account protection stuff
	//========================================
	// GameServer()->Accounts()->SetLoggedIn(m_ClientId, 1, m_Account.m_Id, g_Config.m_SvPort);
}

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
				GameServer()->SendChatTarget(m_ClientId, aMessage);
			}
			break;
		case CAccountResult::ALL:
		{
			bool PrimaryMessage = true;
			for(auto &aMessage : Result.m_aaMessages)
			{
				if(aMessage[0] == 0)
					break;

				if(GameServer()->ProcessSpamProtection(m_ClientId) && PrimaryMessage)
					break;

				GameServer()->SendChat(-1, TEAM_ALL, aMessage, -1);
				PrimaryMessage = false;
			}
			break;
		}
		case CAccountResult::BROADCAST:
			// if(Result.m_aBroadcast[0] != 0)
			// 	GameServer()->SendBroadcast(Result.m_aBroadcast, -1);
			break;
		case CAccountResult::LOGGED_IN_ALREADY:
			if(GameServer()->CheckAccounts(Result.m_Account.m_Id))
				GameServer()->SendChatTarget(m_ClientId, "[ACCOUNT] This account is already logged in on this server.");
			else
				GameServer()->SendChatTarget(m_ClientId, "[ACCOUNT] Login failed. This account is already logged in on another server.");
			dbg_msg("account", "sql id=%d is already logged in.", Result.m_Account.m_Id);
			break;
		case CAccountResult::LOGIN_WRONG_PASS:
			GameServer()->SendChatTarget(m_ClientId, "[ACCOUNT] Login failed. Wrong password or username.");
			GameServer()->WriteWrongLoginJson(m_ClientId, Result.m_Account.m_aUsername, Result.m_Account.m_aPassword);
			GameServer()->LoginBanCheck(m_ClientId);
			break;
		case CAccountResult::LOGIN_INFO:
			m_Account = Result.m_Account;
			OnLogin();
			break;
		case CAccountResult::REGISTER:
			for(auto &aMessage : Result.m_aaMessages)
			{
				if(aMessage[0] == 0)
					continue;

				// localizing here is a bit cursed but it works lol
				GameServer()->SendChatTarget(m_ClientId, GameServer()->Loc(aMessage, m_ClientId));
			}
			GameServer()->RegisterBanCheck(m_ClientId);
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

void CPlayer::DDPPProcessAdminCommandResult(CAdminCommandResult &Result)
{
	if(Result.m_Success) // SQL request was successful
	{
		switch(Result.m_MessageKind)
		{
		case CAdminCommandResult::FREEZE_ACC:
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

			GameServer()->SendChatTarget(m_ClientId, aBuf);
			break;
		}
		case CAdminCommandResult::MODERATOR:
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
			GameServer()->SendChatTarget(m_ClientId, aBuf);
			break;
		}
		case CAdminCommandResult::SUPER_MODERATOR:
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
						CCharacter *pChr = GetCharacter();
						if(pChr)
						{
							pChr->Core()->m_DDNetPP.m_RestrictionData.m_CanEnterVipPlusOnly = true;
						}
					}
					else
					{
						GameServer()->SendChatTarget(i, "[ACCOUNT] You are no longer VIP+.");
						CCharacter *pChr = GetCharacter();
						if(pChr)
						{
							pChr->Core()->m_DDNetPP.m_RestrictionData.m_CanEnterVipPlusOnly = false;
						}
					}
					str_format(aBuf, sizeof(aBuf), "UPDATED IsSuperModerator = %d (%d:'%s')", Result.m_State, i, Server()->ClientName(i));
					break;
				}
			}
			GameServer()->SendChatTarget(m_ClientId, aBuf);
			break;
		}
		case CAdminCommandResult::SUPPORTER:
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
			GameServer()->SendChatTarget(m_ClientId, aBuf);
			break;
		}
		case CAdminCommandResult::DIRECT:
			for(auto &aMessage : Result.m_aaMessages)
			{
				if(aMessage[0] == 0)
					break;
				GameServer()->SendChatTarget(m_ClientId, aMessage);
			}
			break;
		case CAdminCommandResult::ALL:
		{
			bool PrimaryMessage = true;
			for(auto &aMessage : Result.m_aaMessages)
			{
				if(aMessage[0] == 0)
					break;

				if(GameServer()->ProcessSpamProtection(m_ClientId) && PrimaryMessage)
					break;

				GameServer()->SendChat(-1, TEAM_ALL, aMessage, -1);
				PrimaryMessage = false;
			}
			break;
		}
		case CAdminCommandResult::BROADCAST:
			// if(Result.m_aBroadcast[0] != 0)
			// 	GameServer()->SendBroadcast(Result.m_aBroadcast, -1);
			break;
		case CAdminCommandResult::LOG_ONLY:
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
