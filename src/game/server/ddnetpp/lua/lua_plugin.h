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
#include "lauxlib.h"
#include "lua.h"
}

class IGameController;
class CGameContext;

class CLuaRconCommand
{
	class CConstructorArgs
	{
	public:
		const char *m_pName = "";
		const char *m_pHelp = "";
		const char *m_pParams = "";
		int m_LuaCallbackRef = LUA_REFNIL;
	};

public:
	CLuaRconCommand(const char *pName, const char *pHelp, const char *pParams, int LuaCallbackRef)
	{
		str_copy(m_aName, pName);
		str_copy(m_aHelp, pHelp);
		str_copy(m_aParams, pParams);
		m_LuaCallbackRef = LuaCallbackRef;
	}
	CLuaRconCommand(CConstructorArgs Args) :
		CLuaRconCommand(Args.m_pName, Args.m_pHelp, Args.m_pParams, Args.m_LuaCallbackRef)
	{
	}

	// the name of the rcon command
	// that has to match the first word of the rcon line
	// that was executed
	char m_aName[128] = "";

	// helptext that will be shown to users in the console
	char m_aHelp[512] = "";

	// parameters described in the teeworlds console syntax
	// for example "sss" to take 3 unnamed non optional strings
	// or "s[name]?i[seconds]" to take the named non optional strinng argument "name"
	// followed by the optional integer argument "seconds"
	char m_aParams[512] = "";

	// The integer referencing the lua function to be called
	// when the rcon command is being executed
	// Stored in the LUA_REGISTRYINDEX
	//
	// You can push it onto the stack like this:
	//
	// ```C++
	// lua_rawgeti(LuaState(), LUA_REGISTRYINDEX, pCmd->m_LuaCallbackRef);
	// ```
	int m_LuaCallbackRef = LUA_REFNIL;
};

class CLuaPlugin
{
	char m_aErrorMsg[512] = "";
	bool m_IsDisabled = false;

	// The key is the rcon command name
	std::unordered_map<std::string, CLuaRconCommand> m_RconCommands;

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
	static int CallbackSendVoteClearOptions(lua_State *L);
	static int CallbackSendVoteOptionAdd(lua_State *L);
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
