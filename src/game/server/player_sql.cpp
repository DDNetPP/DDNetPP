﻿/* CPlayer related sql ddnet++ methods */

#include "gamemodes/DDRace.h"
#include <engine/shared/config.h>

#include <fstream>
#include <limits>

#include "gamecontext.h"

#include "player.h"

#include <game/mapitems.h>

void CPlayer::OnLogin()
{
	//================================
	// ABORT LOGIN AFTER LOADING DATA
	//================================

	if(m_Account.m_IsAccFrozen)
	{
		GameServer()->SendChatTarget(m_ClientID, "[ACCOUNT] Ошибка: Аккаунт заморожен.");
		Logout();
		return;
	}

	//==========================
	// Done loading stuff
	// Start to do something...
	//==========================

	GameServer()->SendChatTarget(m_ClientID, "[ACCOUNT] Вы успешно авторизовались.");

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
		GameServer()->SendChatTarget(m_ClientID, "[NoboSpawn] Real spawn unlocked due to login.");
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
				GameServer()->SendChatTarget(m_ClientID, "No jail set.");
			}
		}
	}

	//auto joins
	if(m_Account.m_aFngConfig[0] == '1') //auto fng join
	{
		if(!g_Config.m_SvAllowInsta)
		{
			GameServer()->SendChatTarget(m_ClientID, "[INSTA] fng autojoin failed because fng is deactivated by an admin.");
		}
		else
		{
			GameServer()->SendChatTarget(m_ClientID, "[INSTA] you automatically joined an fng game. (use '/fng' to change this setting)");
			GameServer()->JoinInstagib(5, true, m_ClientID);
		}
	}
	else if(m_Account.m_aFngConfig[0] == '2') //auto boomfng join
	{
		if(!g_Config.m_SvAllowInsta)
		{
			GameServer()->SendChatTarget(m_ClientID, "[INSTA] boomfng autojoin failed because fng is deactivated by an admin.");
		}
		else
		{
			GameServer()->SendChatTarget(m_ClientID, "[INSTA] you automatically joined an boomfng game. (use '/fng' to change this setting)");
			GameServer()->JoinInstagib(4, true, m_ClientID);
		}
	}

	// account pass reset info
	if(!str_comp(m_Account.m_ProfileEmail, "") && !str_comp(m_Account.m_ProfileSkype, ""))
		GameServer()->SendChatTarget(m_ClientID, "[ACCOUNT] set an '/profile email' or '/profile skype' to restore your password if you forget it.");

	//========================================
	// LEAVE THIS CODE LAST!!!!
	// multiple server account protection stuff
	//========================================
	GameServer()->Accounts()->SetLoggedIn(m_ClientID, 1, m_Account.m_ID, g_Config.m_SvPort);
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
		case CAccountResult::LOGIN_WRONG_PASS:
			GameServer()->SendChatTarget(m_ClientID, "[ACCOUNT] Login failed. Wrong password or username.");
			char aBuf[1024];
			str_format(
				aBuf,
				sizeof(aBuf),
				"[%s] '%s' '%s'",
				GameServer()->Server()->ClientName(m_ClientID), Result.m_Account.m_aUsername, Result.m_Account.m_aPassword);
			GameServer()->SaveWrongLogin(aBuf);
			GameServer()->LoginBanCheck(m_ClientID);
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
				GameServer()->SendChatTarget(m_ClientID, aMessage);
			}
			GameServer()->RegisterBanCheck(m_ClientID);
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
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(!GameServer()->m_apPlayers[i])
					continue;

				if(GameServer()->m_apPlayers[i]->GetAccID() == Result.m_TargetAccountID)
				{
					GameServer()->m_apPlayers[i]->m_Account.m_IsAccFrozen = Result.m_State;
					// always logout and send you got frozen also if he gets unfreezed because if some1 gets unfreezed he is not logged in xd
					GameServer()->m_apPlayers[i]->Logout();
					GameServer()->SendChatTarget(i, "Logged out. (Reason: Account frozen)");
					str_format(aBuf, sizeof(aBuf), "UPDATED IsAccFrozen = %d (logged out %d:'%s')", Result.m_State, i, Server()->ClientName(i));
					break;
				}
			}
			GameServer()->SendChatTarget(m_ClientID, aBuf);
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

				if(GameServer()->m_apPlayers[i]->GetAccID() == Result.m_TargetAccountID)
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
			GameServer()->SendChatTarget(m_ClientID, aBuf);
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

				if(GameServer()->m_apPlayers[i]->GetAccID() == Result.m_TargetAccountID)
				{
					GameServer()->m_apPlayers[i]->m_Account.m_IsSuperModerator = Result.m_State;
					if(Result.m_State == 1)
						GameServer()->SendChatTarget(i, "[ACCOUNT] You are now VIP+.");
					else
						GameServer()->SendChatTarget(i, "[ACCOUNT] You are no longer VIP+.");
					str_format(aBuf, sizeof(aBuf), "UPDATED IsSuperModerator = %d (%d:'%s')", Result.m_State, i, Server()->ClientName(i));
					break;
				}
			}
			GameServer()->SendChatTarget(m_ClientID, aBuf);
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

				if(GameServer()->m_apPlayers[i]->GetAccID() == Result.m_TargetAccountID)
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
			GameServer()->SendChatTarget(m_ClientID, aBuf);
			break;
		}
		case CAdminCommandResult::DIRECT:
			for(auto &aMessage : Result.m_aaMessages)
			{
				if(aMessage[0] == 0)
					break;
				GameServer()->SendChatTarget(m_ClientID, aMessage);
			}
			break;
		case CAdminCommandResult::ALL:
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
