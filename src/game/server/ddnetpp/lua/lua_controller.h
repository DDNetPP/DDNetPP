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
	bool OnRconCommand(int ClientId, const char *pCommand, const char *pArguments);

	// If a lua plugin runs `Game:call_plugin("func")`
	// it will try to call "func" in all available plugins
	// the first plugin that provides this function will be called
	// and then it stops there.
	// Returns true if a plugin was found.
	//
	// TODO: maybe there should be a way to call plugins by name as well
	//       like `Game:call_plugin("essential", "kick", 1)`
	//       which looks for a plugin called essential and in that plugin
	//       runs the function "kick" and passes 1 as argument
	//       an even more flexible approach would be adding some kind of "implements"
	//       api where plugins can say what kind of things they implement
	//       so a lib plugin has a function that looks like this
	//       function implements()
	//       	return {"rcon_argparser", "accounts"}
	//       end
	//       and then we have another plugin calling into that using
	//       `Game:call_imp("accounts", "logout", 1)`
	//       it sounds kinda cool on paper but idk if it has any actual use case
	//       it basically only namespaces so two plugins could provide
	//       a "logout" function under the same name and in one case
	//       it means logout from an chat account and in the other
	//       it means logout from rcon
	//       actually dont think thats worth it.
	//       Oh wait I found a use case.
	//       It allows plugins to be more sure about finding the kind of function
	//       that they are actually looking for.. hmm
	//       the other alternative to ensure that is long function names.
	//       So basically still just namespaces xd. Bro idk.
	//
	// TODO: also add `Game:call_plugins("func")` to call all not only the first
	bool CallPlugin(const char *pFunction, lua_State *pCaller);

	CLuaPlugin *FindPluginThatKnowsRconCommand(const char *pCommand);

private:
	bool LoadPlugin(const char *pName, const char *pFilename);
	void ReloadPlugins();
};

#endif
