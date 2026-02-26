#ifndef GAME_SERVER_DDNETPP_LUA_LUA_PLUGINS_H
#define GAME_SERVER_DDNETPP_LUA_LUA_PLUGINS_H

#ifdef CONF_LUA

#include <base/log.h>
#include <base/types.h>

#include <lua.hpp>
#include <vector>

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

class CLuaPlugin
{
public:
	CLuaPlugin(const char *pName, const char *pFullPath);
	~CLuaPlugin();

	lua_State *m_pLuaState = nullptr;
	char m_aName[IO_MAX_PATH_LENGTH] = "";
	char m_aFullPath[IO_MAX_PATH_LENGTH] = "";
	lua_State *LuaState() { return m_pLuaState; }
	const char *Name() const { return m_aName; }
	const char *FullPath() const { return m_aFullPath; }
};

class CLuaController
{
	CLuaGame m_Game;

	// TODO: i am too lazy to understand "rule 0/3/5"
	//       https://en.cppreference.com/w/cpp/language/rule_of_three.html
	//       initially i passed plugins by value but the vector was calling
	//       the destructor and messing up my lua state
	//       So I am now passing them as pointers so nothing unexpected happens
	//       apparently using "new" in modern C++ is bad style lol but whatever
	std::vector<CLuaPlugin *> m_vpPlugins;

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
	bool LoadPlugin(const char *pName, const char *pFilename);
	void ReloadPlugins();
};

#endif

#endif
