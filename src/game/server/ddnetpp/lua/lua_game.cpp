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

void CLuaGame::SendRconCmdRem(int ClientId, const CLuaRconCommand *pCmd)
{
	CMsgPacker Msg(NETMSG_RCON_CMD_REM, true);
	Msg.AddString(pCmd->Name(), IConsole::TEMPCMD_NAME_LENGTH);
	Server()->SendMsg(&Msg, MSGFLAG_VITAL, ClientId);
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
