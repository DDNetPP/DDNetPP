#ifndef GAME_SERVER_DDNETPP_LUA_LUA_CONTROLLER_H
#define GAME_SERVER_DDNETPP_LUA_LUA_CONTROLLER_H

#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

#include <game/server/ddnetpp/lua/lua_plugin.h>

#include <vector>

class IGameController;
class CGameContext;

class CLuaController
{
#ifdef CONF_LUA
	CLuaGame m_Game;

	// TODO: i am too lazy to understand "rule 0/3/5"
	//       https://en.cppreference.com/w/cpp/language/rule_of_three.html
	//       initially i passed plugins by value but the vector was calling
	//       the destructor and messing up my lua state
	//       So I am now passing them as pointers so nothing unexpected happens
	//       apparently using "new" in modern C++ is bad style lol but whatever
	std::vector<CLuaPlugin *> m_vpPlugins;
#endif

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

	// rcon commands
	void ListPlugins();

	void OnInit();
	void OnTick();
	void OnPlayerConnect();
	bool OnRconCommand(const char *pCommand, const char *pArguments);

	// If a lua plugin runs `Game:call_plugin("func")`
	// it will try to call "func" in all available plugins
	// the first plugin that provides this function will be called
	// and then it stops there.
	// Returns true if a plugin was found.
	//
	// TODO: also add `Game:call_plugins("func")` to call all not only the first
	bool CallPlugin(const char *pFunction, lua_State *pCaller);

private:
	bool LoadPlugin(const char *pName, const char *pFilename);
	void ReloadPlugins();
};

#endif
