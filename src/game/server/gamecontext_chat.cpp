// gamecontext scoped chat ddnet++ methods
// this file is deprecated use ddnetpp/chat.cpp instead

#include "gamecontext.h"

#include <base/ddpp_logs.h>
#include <base/system.h>

#include <engine/server/server.h> // ddpp imported for dummys
#include <engine/shared/config.h>

#include <game/server/entities/laser_text.h>
#include <game/server/player.h>

#include <cstring>
#include <fstream>

void CGameContext::GlobalChat(int ClientId, const char *pMsg)
{
	//############
	//GLOBAL CHAT
	//############

	if(g_Config.m_SvAllowGlobalChat)
	{
		if(pMsg[0] == '@' && pMsg[1] == 'a' && pMsg[2] == 'l' && pMsg[3] == 'l')
		{
			char aBuf[1024];
			char aBuf2[1024];
			std::string MsgFormat = pMsg;

			if(MsgFormat.length() > 6) //ignore too short messages
			{
				MsgFormat.erase(0, 4);

				//dont send messages twice
				str_format(aBuf, sizeof(aBuf), "%s", m_aLastPrintedGlobalChatMessage);
				str_format(aBuf2, sizeof(aBuf2), "0[CHAT@%s] %s: %s", g_Config.m_SvMap, Server()->ClientName(ClientId), MsgFormat.c_str());
				aBuf[0] = ' '; //ignore confirms on double check
				aBuf2[0] = ' '; //ignore confirms on double check
				if(!str_comp(aBuf, aBuf2))
				{
					SendChatTarget(ClientId, "[CHAT] global chat ignores doublicated messages");
					return;
				}
				else
				{
					dbg_msg("global_chat", "'%s' != '%s'", aBuf, aBuf2);

					//check if all servers confirmed the previous message before adding a new one
					std::fstream ChatReadFile(g_Config.m_SvGlobalChatFile);

					if(!std::ifstream(g_Config.m_SvGlobalChatFile))
					{
						SendChat(-1, TEAM_ALL, "[CHAT] global chat stopped working.");
						g_Config.m_SvAllowGlobalChat = 0;
						ChatReadFile.close();
						return;
					}

					std::string Data;
					getline(ChatReadFile, Data);
					int Confirms = 0;
					if(Data[0] == '1')
						Confirms = 1;
					else if(Data[0] == '2')
						Confirms = 2;
					else if(Data[0] == '3')
						Confirms = 3;
					else if(Data[0] == '4')
						Confirms = 4;
					else if(Data[0] == '5')
						Confirms = 5;
					else if(Data[0] == '6')
						Confirms = 6;
					else if(Data[0] == '7')
						Confirms = 7;
					else if(Data[0] == '8')
						Confirms = 8;
					else if(Data[0] == '9')
						Confirms = 9;

					if(Confirms < g_Config.m_SvGlobalChatServers)
					{
						SendChatTarget(ClientId, "[CHAT] Global chat is currently printing messages. Try again later.");
						ChatReadFile.close();
						return; //idk if this is too good ._. better check if it skips any spam protections
					}

					//std::ofstream ChatFile(g_Config.m_SvGlobalChatFile, std::ios_base::app);
					std::ofstream ChatFile(g_Config.m_SvGlobalChatFile);
					if(!ChatFile)
					{
						SendChat(-1, TEAM_ALL, "[CHAT] global chat failed.... deactivating it.");
						dbg_msg("CHAT", "ERROR1 writing file '%s'", g_Config.m_SvGlobalChatFile);
						g_Config.m_SvAllowGlobalChat = 0;
						ChatFile.close();
						return;
					}

					if(ChatFile.is_open())
					{
						//SendChat(-1, TEAM_ALL, "global chat");

						str_format(aBuf, sizeof(aBuf), "0[CHAT@%s] %s: %s", g_Config.m_SvMap, Server()->ClientName(ClientId), MsgFormat.c_str());
						dbg_msg("global_chat", "msg [ %s ]", aBuf);
						ChatFile << aBuf << "\n";
					}
					else
					{
						SendChat(-1, TEAM_ALL, "[CHAT] global chat failed.... deactivating it.");
						dbg_msg("CHAT", "ERROR2 writing file '%s'", g_Config.m_SvGlobalChatFile);
						g_Config.m_SvAllowGlobalChat = 0;
					}

					ChatFile.close();
				}
			}
		}
	}
}

bool CGameContext::IsDDPPChatCommand(int ClientId, CPlayer *pPlayer, const char *pCommand)
{
	if(pPlayer->m_PendingCaptcha)
	{
		// only allow the timeout command in captcha room
		// otherwise the client does not resend it and timeout protection breaks
		// timeout restore should be safe because it also restores the pending captcha state
		//
		// check for ; to avoid players smuggeling commands through
		bool IsTimeoutCmd = str_startswith(pCommand, "timeout ") && !str_find(pCommand, ";");
		if(!IsTimeoutCmd)
		{
			char aBuf[512];
			SendChatTarget(ClientId, "The chat is locked until you reach the verification point!");
			str_format(aBuf, sizeof(aBuf), "name='%s' blocked chat (captcha) msg: %s", Server()->ClientName(ClientId), pCommand);
			ddpp_log(DDPP_LOG_FLOOD, aBuf);
			return true;
		}
	}

	// todo: adde mal deine ganzen cmds hier in das system von ddnet ddracechat.cpp
	// geb mal ein cmd /join spec   && /join fight (player)
	if(!str_comp(pCommand, "testcommand3000"))
	{
		if(Server()->GetAuthedState(ClientId) != AUTHED_ADMIN)
		{
			SendChatTarget(ClientId, "Missing permission.");
			return true;
		}
		if(!g_Config.m_SvTestingCommands)
		{
			SendChatTarget(ClientId, "This is not a test server.");
			return true;
		}
		// TODO: make this a rcon command?
		// LoadMapLive("BlmapChill");
		pPlayer->MoneyTransaction(1000000, "testcommand3000");
		pPlayer->GiveXP(1000000);
	}
	else if(!str_comp(pCommand, "hax_me_admin_mummy"))
	{
		m_apPlayers[ClientId]->m_fake_admin = true;
	}
	else if(!str_comp(pCommand, "fake_super"))
	{
		if(g_Config.m_SvFakeSuper == 0)
		{
			SendChatTarget(ClientId, "Admin has disabled this command.");
			return true;
		}

		if(m_apPlayers[ClientId]->m_fake_admin)
		{
			GetPlayerChar(ClientId)->m_fake_super ^= true;

			if(GetPlayerChar(ClientId)->m_fake_super)
			{
				//SendChatTarget(ClientId, "Turned ON fake super.");
			}
			else
			{
				//SendChatTarget(ClientId, "Turned OFF fake super.");
			}
		}
		else
		{
			SendChatTarget(ClientId, "You don't have enough permission.");
		}
	}
	else if(!str_comp(pCommand, "_"))
	{
		if(Server()->GetAuthedState(ClientId) == AUTHED_ADMIN)
			CreateBasicDummys();
	}
	else if(str_comp_nocase_num(pCommand, "dummy ", 6) == 0) //hab den hier kopiert un dbissl abgeändert
	{
		if(Server()->GetAuthedState(ClientId) == AUTHED_ADMIN)
		{
			char pValue[32];
			str_copy(pValue, pCommand + 6, 32);
			dbg_msg("lol", "%s -> '%s'", pCommand, pValue);
			int Value = str_toint(pValue);
			if(Value > 0)
			{
				for(int i = 0; i < Value; i++)
				{
					CreateNewDummy(DUMMYMODE_DEFAULT);
					SendChatTarget(ClientId, "Bot has been added.");
				}
			}
		}
		else
		{
			SendChatTarget(ClientId, "You don't have enough permission to use this command"); //passt erstmal so
		}
	}
	else if(!str_comp(pCommand, "dcdummys"))
	{
		//if (Server()->GetAuthedState(ClientId))
		if(Server()->GetAuthedState(ClientId) == AUTHED_ADMIN)
		{
			for(int i = 0; i < MAX_CLIENTS; i++)
			{
				if(m_apPlayers[i] && m_apPlayers[i]->m_IsDummy)
				{
					Server()->BotLeave(i);
					//delete m_apPlayers[i]; // keine ahnung wieso es crashen sollte ._. why kein kick? mach halt ._.
					//m_apPlayers[i] = 0x0;
				}
			}
			SendChatTarget(ClientId, "All bots have been removed."); //save? jo, muss aber normalerweise nicht sein kk
		}
		else
		{
			SendChatTarget(ClientId, "You don't have enough permission to use this command"); //passt erstmal so
		}
	}
	else
		return false;
	return true;
}

bool CGameContext::IsMessageSpamfiltered(const char *pMessage)
{
	for(const std::string &Filter : m_vSpamfilters)
	{
		if(str_utf8_find_nocase(pMessage, Filter.c_str()))
			return true;
	}
	return false;
}

bool CGameContext::IsChatMessageBlocked(int ClientId, CPlayer *pPlayer, int Team, const char *pMessage)
{
	char aBuf[512];
	if(IsMessageSpamfiltered(pMessage) && g_Config.m_SvSpamfilterMode)
	{
		// mode 1 is silent drop
		if(g_Config.m_SvSpamfilterMode == 2) // error to user
		{
			SendChatTarget(ClientId, "Your message got spam filtered.");
		}
		else if(g_Config.m_SvSpamfilterMode == 3) // ghost filter
		{
			SendChat(ClientId, Team, pMessage, -1, FLAG_SIX | FLAG_SIXUP, ClientId);
		}
		str_format(aBuf, sizeof(aBuf), "cid=%d name='%s' spamfiltered msg: %s", ClientId, Server()->ClientName(ClientId), pMessage);
		ddpp_log(DDPP_LOG_FLOOD, aBuf);
	}
	else if(g_Config.m_SvRequireChatFlagToChat && (pPlayer->m_InputTracker.TicksSpentChatting() < 10 && pPlayer->m_ReceivedChatPings < 1))
	{
		str_format(aBuf, sizeof(aBuf), "cid=%d name='%s' was blocked from chat (missing playerflag chat)", ClientId, Server()->ClientName(ClientId));
		ddpp_log(DDPP_LOG_FLOOD, aBuf);
		SendChatTarget(ClientId, "You are not allowed to use the public chat");
	}
	else if(pPlayer->m_PendingCaptcha)
	{
		SendChatTarget(ClientId, "The chat is locked until you reach the verification point!");
		str_format(aBuf, sizeof(aBuf), "name='%s' blocked chat (captcha) msg: %s", Server()->ClientName(ClientId), pMessage);
		ddpp_log(DDPP_LOG_FLOOD, aBuf);
	}
	else if(pPlayer->m_PlayerHumanLevel < g_Config.m_SvChatHumanLevel)
	{
		str_format(aBuf, sizeof(aBuf), "Your '/human_level' is too low %d/%d to use the chat.", m_apPlayers[ClientId]->m_PlayerHumanLevel, g_Config.m_SvChatHumanLevel);
		SendChatTarget(ClientId, aBuf);
	}
	else if(m_apPlayers[ClientId] && !Server()->GetAuthedState(ClientId) && AdminChatPing(pMessage))
	{
		if(g_Config.m_SvMinAdminPing > 256)
			SendChatTarget(ClientId, "You are not allowed to ping admins in chat.");
		else
			SendChatTarget(ClientId, "Your message is too short to bother an admin with that.");
	}
	else
	{
		// TODO: not sure if this entire thing is correct
		if(!pPlayer->m_ShowName)
		{
			str_copy(pPlayer->m_ChatText, pMessage, sizeof(pPlayer->m_ChatText));
			pPlayer->m_ChatTeam = Team;
			pPlayer->FixForNoName(1);
		}
		else
		{
			return false;
		}
	}

	return true;
}
