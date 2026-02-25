#ifndef GAME_SERVER_DDNETPP_LUA_LUA_PLUGINS_H
#define GAME_SERVER_DDNETPP_LUA_LUA_PLUGINS_H

#ifdef CONF_LUA

#include <base/log.h>

class IGameController;
class CGameContext;

class CLuaBridge
{
	IGameController *m_pController = nullptr;
	CGameContext *m_pGameServer = nullptr;

public:
	const IGameController *Controller() const { return m_pController; }
	IGameController *Controller() { return m_pController; }
	const CGameContext *GameServer() const { return m_pGameServer; }
	CGameContext *GameServer() { return m_pGameServer; }
	void Init(IGameController *pController, CGameContext *pGameServer);
	void Foo() { log_info("lua", "FOOOOO"); }

	void SendChat(const char *pMessage);
};

class CLuaController
{
	// TODO: name this the same way as the lua binding
	//       so something like game?
	CLuaBridge m_Bridge;

public:
	void Init(IGameController *pController, CGameContext *pGameServer);
};

#endif

#endif
