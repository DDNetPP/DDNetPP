#ifndef GAME_SERVER_DDNETPP_LUA_LUA_PLUGINS_H
#define GAME_SERVER_DDNETPP_LUA_LUA_PLUGINS_H

#ifdef CONF_LUA

#include <base/log.h>

#include <lua.hpp>

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

class CLuaController
{
	CLuaGame m_Game;

	lua_State *m_pLuaState = nullptr;
	lua_State *LuaState() { return m_pLuaState; }

	IGameController *m_pController = nullptr;
	CGameContext *m_pGameServer = nullptr;

	static int FsListPluginCallback(const char *pFilename, int IsDir, int DirType, void *pUser);

public:
	const IGameController *Controller() const { return m_pController; }
	IGameController *Controller() { return m_pController; }
	const CGameContext *GameServer() const { return m_pGameServer; }
	CGameContext *GameServer() { return m_pGameServer; }

	void Init(IGameController *pController, CGameContext *pGameServer);
	~CLuaController();

	void OnTick();

private:
	bool LoadPlugin(const char *pFilename);
	void ReloadPlugins();
};

#endif

#endif
