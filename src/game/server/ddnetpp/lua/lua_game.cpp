#ifdef CONF_LUA
#include "lua_game.h"

#include <base/dbg.h>
#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

#include <game/server/ddnetpp/lua/lua_plugin.h>
#include <game/server/gamecontext.h>

const IServer *CLuaGame::Server() const
{
	return GameServer()->Server();
}

IServer *CLuaGame::Server()
{
	return GameServer()->Server();
}

const CCollision *CLuaGame::Collision() const
{
	return m_pGameServer->Collision();
}

CCollision *CLuaGame::Collision()
{
	return GameServer()->Collision();
}

void CLuaGame::Init(IGameController *pController, CGameContext *pGameServer)
{
	m_pController = pController;
	m_pGameServer = pGameServer;
}

void CLuaGame::SendRconCmdAdd(int ClientId, const CLuaRconCommand *pCmd)
{
	CMsgPacker Msg(NETMSG_RCON_CMD_ADD, true);
	Msg.AddString(pCmd->Name(), IConsole::TEMPCMD_NAME_LENGTH);
	Msg.AddString(pCmd->Help(), IConsole::TEMPCMD_HELP_LENGTH);
	Msg.AddString(pCmd->Params(), IConsole::TEMPCMD_PARAMS_LENGTH);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientId);
}

void CLuaGame::SendRconCmdRem(int ClientId, const char *pCmd)
{
	CMsgPacker Msg(NETMSG_RCON_CMD_REM, true);
	Msg.AddString(pCmd, IConsole::TEMPCMD_NAME_LENGTH);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientId);
}

void CLuaGame::SendChatCmdAdd(int ClientId, const CLuaRconCommand *pCmd)
{
	if(Server()->IsSixup(ClientId))
	{
		protocol7::CNetMsg_Sv_CommandInfo Msg;
		Msg.m_pName = pCmd->Name();
		Msg.m_pArgsFormat = pCmd->Params();
		Msg.m_pHelpText = pCmd->Help();
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, ClientId);
	}
	else
	{
		CNetMsg_Sv_CommandInfo Msg;
		Msg.m_pName = pCmd->Name();
		Msg.m_pArgsFormat = pCmd->Params();
		Msg.m_pHelpText = pCmd->Help();
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, ClientId);
	}
}

void CLuaGame::SendChatCmdRem(int ClientId, const char *pCmd)
{
	if(Server()->IsSixup(ClientId))
	{
		protocol7::CNetMsg_Sv_CommandInfoRemove Msg;
		Msg.m_pName = pCmd;
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, ClientId);
	}
	else
	{
		CNetMsg_Sv_CommandInfoRemove Msg;
		Msg.m_pName = pCmd;
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, ClientId);
	}
}

void CLuaGame::SendRconCmdGroupStart(int ClientId, int Length)
{
	CMsgPacker Msg(NETMSG_RCON_CMD_GROUP_START, true);
	Msg.AddInt(Length);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientId);
}

void CLuaGame::SendRconCmdGroupEnd(int ClientId)
{
	CMsgPacker Msg(NETMSG_RCON_CMD_GROUP_END, true);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientId);
}

void CLuaGame::SendChatCmdGroupStart(int ClientId, int Length)
{
	CNetMsg_Sv_CommandInfoGroupStart Msg;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, ClientId);
}

void CLuaGame::SendChatCmdGroupEnd(int ClientId)
{
	CNetMsg_Sv_CommandInfoGroupEnd Msg;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL | MSGFLAG_NORECORD, ClientId);
}

void CLuaGame::SendChat(const char *pMessage)
{
	GameServer()->SendChat(-1, TEAM_ALL, pMessage);
}

void CLuaGame::SendVoteClearOptions(int ClientId)
{
	CNetMsg_Sv_VoteClearOptions Msg;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientId);
}

void CLuaGame::SendVoteOptionAdd(int ClientId, const char *pDescription)
{
	CNetMsg_Sv_VoteOptionAdd Msg;
	Msg.m_pDescription = pDescription;
	Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, ClientId);
}

#endif
