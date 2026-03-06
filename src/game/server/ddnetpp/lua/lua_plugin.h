#ifndef GAME_SERVER_DDNETPP_LUA_LUA_PLUGIN_H
#define GAME_SERVER_DDNETPP_LUA_LUA_PLUGIN_H

#ifdef CONF_LUA

#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

#include <game/server/ddnetpp/lua/lua_game.h>

#include <string>
#include <unordered_map>
#include <vector>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

class IGameController;
class CGameContext;
class CPlayer;

// returns true on success
// writes multiple teeworlds console statements (separated by semicolon)
// into the output array apStmts the amount of items is written to pNumStmts
//
// it strips the trailing semicolons
// and treats ;;; as an syntax error
bool SplitConsoleStatements(const char *apStmts[], size_t MaxStmts, size_t *pNumStmts, char *pLine, char *pError, size_t ErrorLen);

// returns true on success
// takes a mutable pInput string which it will also write to
// and writes the result to apArgs which is a user given array of size MaxArgs
// the amount of args it got it will write to pNumArgs
//
// on error it writes a reason to pError
bool SplitConsoleArgs(const char *apArgs[], size_t MaxArgs, size_t *pNumArgs, char *pInput, char *pError, size_t ErrorLen);

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
	class CParam
	{
	public:
		enum EType
		{
			INT,
			STRING,
			REST,
			INVALID
		};

		EType m_Type = EType::STRING;
		char m_aName[512] = "";
		bool m_Optional = false;

		void Reset()
		{
			m_Optional = false;
			m_aName[0] = '\0';
			m_Type = EType::INVALID;
		}
	};

	std::vector<CParam> m_vParsedParams;

	CLuaRconCommand(const char *pName, const char *pHelp, const char *pParams, int LuaCallbackRef)
	{
		str_copy(m_aName, pName);
		str_copy(m_aHelp, pHelp);
		str_copy(m_aParams, pParams);
		m_LuaCallbackRef = LuaCallbackRef;
		ParseParameters(m_vParsedParams, pParams, nullptr, 0);
	}
	CLuaRconCommand(CConstructorArgs Args) :
		CLuaRconCommand(Args.m_pName, Args.m_pHelp, Args.m_pParams, Args.m_LuaCallbackRef)
	{
	}

	// returns true on success and writes the parsed params to &vResult
	// takes the console params string as input in the pParameters argument
	// that should be in the format of the teeworlds console parameter description
	// like those: "ss", "sssi" or "s[name]?i[seconds]"
	//
	// On error it writes to the pError buffer
	static bool ParseParameters(std::vector<CParam> &vResult, const char *pParameters, char *pError, int ErrorLen);

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

	const char *Name() const { return m_aName; }
	const char *Help() const { return m_aHelp; }
	const char *Params() const { return m_aParams; }
};

// Same as SplitConsoleArgs
// but also takes vParams into consideration
// and splitting "r" params differently
bool SplitConsoleArgsWithParams(const char *apArgs[], size_t MaxArgs, size_t *pNumArgs, char *pInput, const std::vector<CLuaRconCommand::CParam> &vParams, char *pError, size_t ErrorLen);

class CLuaPlugin
{
	char m_aErrorMsg[512] = "";
	bool m_IsDisabled = false;

public:
	// The key is the rcon command name
	std::unordered_map<std::string, CLuaRconCommand> m_RconCommands;

	CLuaPlugin(const char *pName, const char *pFullPath, CLuaGame *pGame);
	~CLuaPlugin();

	CLuaGame *m_pGame = nullptr;
	CLuaGame *Game() { return m_pGame; }
	const CLuaGame *Game() const { return m_pGame; }

private:
	// defines the player type for lua as a metatable
	void RegisterPlayerMetaTable();

	// defined the game type for lua as metatable
	void RegisterGameMetaTable();

	// sets the global instance "Game" and leaves the stack clean
	void PushGameInstance();

public:
	void RegisterGlobalState();
	bool LoadFile();

private:
	// returns true if the function was found
	bool CallLuaVoidNoArgs(const char *pFunction);
	// returns true if the function was found
	bool CallLuaVoidWithTwoInts(const char *pFunction, int Num1, int Num2);

	// Calling C++ from lua
	static int CallbackSendChat(lua_State *L);
	static int CallbackSendVoteClearOptions(lua_State *L);
	static int CallbackSendVoteOptionAdd(lua_State *L);
	static int CallbackGetPlayer(lua_State *L);
	static int CallbackCallPlugin(lua_State *L);
	static int CallbackRegisterRcon(lua_State *L);
	static int CallbackPluginName(lua_State *L);

	// TODO: move these to some other scope? Because we have some
	//       for the game instance and some for the player
	static int CallbackPlayerId(lua_State *L);
	static int CallbackPlayerName(lua_State *L);

public:
	// Calling lua from C++
	void OnInit();
	void OnTick();
	void OnPlayerConnect();
	bool OnRconCommand(int ClientId, const char *pCommand, const char *pArguments);
	void OnSetAuthed(int ClientId, int Level);
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

// This is heap allocated for every player lookup from lua
// the memory for this is managed and garbage collected by lua
// That is why it is just a light handle holding the unique id
// which means that on calling instance methods we have to iterate all instances
// on the server side and find the player instance
//
// Using client id and indexing the players array would be much faster
// but then if players reconnect we could get the wrong client id
// there is a concept called generations to solve this issue
// eventually we can switch to that to improve performance
class CLuaPlayerHandle
{
public:
	uint32_t m_UniqueClientId = 0;

	// TODO: is this cursed?
	CLuaPlugin *m_pPlugin = nullptr;

	CLuaPlayerHandle(uint32_t UniqueClientId, CLuaPlugin *pPlugin) :
		m_UniqueClientId(UniqueClientId), m_pPlugin(pPlugin)
	{
	}
};

#endif

#endif
