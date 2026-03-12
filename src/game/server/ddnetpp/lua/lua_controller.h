#ifndef GAME_SERVER_DDNETPP_LUA_LUA_CONTROLLER_H
#define GAME_SERVER_DDNETPP_LUA_LUA_CONTROLLER_H

#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

#include <engine/shared/ddnetpp/lua_controller.h>
#include <engine/shared/protocol.h>

#include <game/server/ddnetpp/lua/lua_game.h>
#include <game/server/ddnetpp/lua/lua_rcon_command.h>

#include <optional>
#include <vector>

class CLuaPlugin;
class IGameController;
class CGameContext;
struct lua_State;

class CLuaPlayerState
{
public:
	class CRconCmdSender
	{
	public:
		std::optional<size_t> m_SendIndex = std::nullopt;
		std::vector<CLuaRconCommand> m_vMissingCmds;
		std::vector<CLuaRconCommand> m_vMissingCmdsNext;
		void AddRconCmd(const CLuaRconCommand *pCmd);

		std::optional<size_t> m_RemoveIndex = std::nullopt;
		std::vector<std::string> m_vRemoveCmds;
	};

	CRconCmdSender m_RconSender;
};

class CLuaController : public ILuaController
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

	static int FsLoadDirPlugin(const char *pDirname, int DirType, void *pUser);
	static int FsListPluginCallback(const char *pFilename, int IsDir, int DirType, void *pUser);

	bool IsServerSendingNativeRconCmds(int ClientId);

public:
	// called when any plugin adds a new rcon command
	// it tracks them in a queue to send for all players
	void OnAddRconCmd(const CLuaRconCommand *pCmd);

private:
	// sends the next rcon commands if there are any new ones queued
	// this is called on tick
	//
	// returns true if it sent something
	// returns false if there is currently nothing to send
	bool SendNextRconCmd(int ClientId);

public:
	const IGameController *Controller() const { return m_pController; }
	IGameController *Controller() { return m_pController; }
	const CGameContext *GameServer() const { return m_pGameServer; }
	CGameContext *GameServer() { return m_pGameServer; }

	void Init(IGameController *pController, CGameContext *pGameServer);
	~CLuaController() override;

	CLuaPlayerState m_aPlayers[MAX_CLIENTS];

	// rcon commands
	void ListPlugins();
	void ReloadPlugins();

	void OnInit() override;
	void OnTick();
	void OnPlayerConnect(int ClientId);
	void OnPlayerDisconnect(int ClientId);
	bool OnRconCommand(int ClientId, const char *pCommand, const char *pArguments);
	void OnSetAuthed(int ClientId, int Level);
	bool OnClientMessage(int ClientId, const void *pData, int Size, int Flags) override;
	bool OnServerMessage(int ClientId, const void *pData, int Size, int Flags) override;

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
};

#endif
