#ifndef GAME_SERVER_DDNETPP_LUA_LUA_GAME_H
#define GAME_SERVER_DDNETPP_LUA_LUA_GAME_H

#ifdef CONF_LUA

#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

class IGameController;
class CGameContext;

/// represents the "Game" table in lua
///
/// example lua script
///
/// ```lua
/// Game:send_chat("hello world")
/// ```
///
/// Sends a chat message
class CLuaGame
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

#endif

#endif
