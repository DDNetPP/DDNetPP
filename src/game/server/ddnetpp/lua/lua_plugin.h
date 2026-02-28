#ifndef GAME_SERVER_DDNETPP_LUA_LUA_PLUGIN_H
#define GAME_SERVER_DDNETPP_LUA_LUA_PLUGIN_H

#ifdef CONF_LUA

#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

#include <game/server/ddnetpp/lua/lua_game.h>

#include <string>
#include <unordered_map>
extern "C" {
#include "lua.h"
}

class IGameController;
class CGameContext;

class CLuaPlugin
{
	char m_aErrorMsg[512] = "";
	bool m_IsDisabled = false;

	// The key is the rcon command
	// The value is the lua refrence to the callback
	std::unordered_map<std::string, int> m_RconCommands;

public:
	CLuaPlugin(const char *pName, const char *pFullPath, CLuaGame *pGame);
	~CLuaPlugin();

	CLuaGame *m_pGame = nullptr;
	CLuaGame *Game() { return m_pGame; }
	const CLuaGame *Game() const { return m_pGame; }

private:
	void RegisterGameTable();
	void RegisterGameInstance();

public:
	void RegisterGlobalState();
	bool LoadFile();

private:
	// returns true if the function was found
	bool CallLuaVoidNoArgs(const char *pFunction);

	// Calling C++ from lua
	static int CallbackSendChat(lua_State *L);
	static int CallbackCallPlugin(lua_State *L);
	static int CallbackRegisterRcon(lua_State *L);
	static int CallbackPluginName(lua_State *L);

public:
	// Calling lua from C++
	void OnInit();
	void OnTick();
	void OnPlayerConnect();
	bool OnRconCommand(int ClientId, const char *pCommand, const char *pArguments);
	bool CallPlugin(const char *pFunction, lua_State *pCaller);

	// helpers
	bool IsRconCmdKnown(const char *pCommand);

private:
	// returns true on success
	// and false if the table contains unsupported fields
	//
	// Copies the returned table from a lua plugin
	// to another lua state when being called from another plugin
	bool CopyReturnedTable(const char *pFunction, lua_State *pCaller, int Depth);

	// passes on the arguments when one plugin calls a function from another plugin
	// the callee is *this* and its arguments will be copied to the callers lua state
	//
	// returns the number of found and supported arguments that were pushed on to the
	// lua stack of the caller
	//
	// the stack offset is the lua stack offset of the callee where we start to look
	// for arguments
	int PassOnArgs(const char *pFunction, lua_State *pCaller, int StackOffset);

public:
	lua_State *m_pLuaState = nullptr;
	char m_aName[IO_MAX_PATH_LENGTH] = "";
	char m_aFullPath[IO_MAX_PATH_LENGTH] = "";
	lua_State *LuaState() { return m_pLuaState; }
	const char *Name() const { return m_aName; }
	const char *FullPath() const { return m_aFullPath; }

	void SetError(const char *pErrorMsg);
	bool IsError() const { return m_aErrorMsg[0] != '\0'; }
	const char *ErrorMsg() const { return m_aErrorMsg; }
	bool IsActive() const { return !IsError() && !m_IsDisabled; }
};

#endif

#endif
