#ifndef GAME_SERVER_DDNETPP_LUA_LUA_PLUGIN_H
#define GAME_SERVER_DDNETPP_LUA_LUA_PLUGIN_H

#include <generated/protocol.h>
#ifdef CONF_LUA

#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

#include <game/server/ddnetpp/lua/lua_game.h>
#include <game/server/ddnetpp/lua/lua_rcon_command.h>

#include <string>
#include <unordered_map>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

class IGameController;
class CGameContext;
class CPlayer;

class CLuaPlugin
{
	char m_aErrorMsg[512] = "";
	bool m_IsDisabled = false;

	class CTmpStorage
	{
	public:
		char m_aClSayMessage[2048] = "";
	};
	CTmpStorage m_TmpStorage;

public:
	// The key is the rcon command name
	std::unordered_map<std::string, CLuaRconCommand> m_RconCommands;

	// The key is the chat command name
	std::unordered_map<std::string, CLuaRconCommand> m_ChatCommands;

	CLuaPlugin(const char *pName, const char *pFullPath, CLuaGame *pGame);
	~CLuaPlugin();

	CLuaGame *m_pGame = nullptr;
	CLuaGame *Game() { return m_pGame; }
	const CLuaGame *Game() const { return m_pGame; }

private:
	// defines the player type for lua as a metatable
	void RegisterPlayerMetaTable();

	// defines the character type for lua as a metatable
	void RegisterCharacterMetaTable();

	// Defined the global "ddnetpp" table.
	// Which is the instance holding the entrypoint to the entire
	// plugin api.
	void RegisterGlobalDDNetPPInstance();

public:
	void RegisterGlobalState();
	bool LoadFile();

private:
	// returns true if the function was found
	bool CallLuaVoidNoArgs(const char *pFunction);
	// returns true if the function was found
	bool CallLuaVoidWithOneInt(const char *pFunction, int Num1);
	// returns true if the function was found
	bool CallLuaVoidWithTwoInts(const char *pFunction, int Num1, int Num2);

	// Calling C++ from lua
	static int CallbackSendChat(lua_State *L);
	static int CallbackSendVoteClearOptions(lua_State *L);
	static int CallbackSendVoteOptionAdd(lua_State *L);
	static int CallbackGetPlayer(lua_State *L);
	static int CallbackGetCharacter(lua_State *L);
	static int CallbackCallPlugin(lua_State *L);
	static int CallbackRegisterRcon(lua_State *L);
	static int CallbackRegisterChat(lua_State *L);
	static int CallbackPluginName(lua_State *L);

	// snap
	static int CallbackSnapNewId(lua_State *L);
	static int CallbackSnapFreeId(lua_State *L);
	static int CallbackSnapNewLaser(lua_State *L);

	// TODO: move these to some other scope? Because we have some
	//       for the game instance and some for the player
	static int CallbackPlayerId(lua_State *L);
	static int CallbackPlayerName(lua_State *L);

	// TODO: new scope?
	static int CallbackCharacterPos(lua_State *L);

public:
	// Calling lua from C++
	void OnInit();
	void OnTick();
	void OnSnap();
	bool OnChatMessage(int ClientId, CNetMsg_Cl_Say *pMsg, int &Team);
	void OnPlayerConnect(int ClientId);
	void OnPlayerDisconnect(int ClientId);
	bool OnRconCommand(int ClientId, const char *pCommand, const char *pArguments);
	bool OnChatCommand(int ClientId, const char *pCommand, const char *pArguments);
	void OnSetAuthed(int ClientId, int Level);
	bool OnServerMessage(int ClientId, const void *pData, int Size, int Flags);
	bool CallPlugin(const char *pFunction, lua_State *pCaller);

	// helpers
	bool IsRconCmdKnown(const char *pCommand);
	bool IsChatCmdKnown(const char *pCommand);

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

	// WARNING: you have to call this after calling lua_newuserdatauv
	//          I intentionally did not use a constructor because placement new is goofed syntax
	//          and just looking at this class one might think that it could be extended with an
	//          destructor if needed. Which is not being called unless we implement the lua
	//          garbage collector callback for it
	//          So i rather have this an init method to make it clear that this is not constructed
	//          and destructed like any other regular C++ object.
	void Init(uint32_t UniqueClientId, CLuaPlugin *pPlugin)
	{
		m_UniqueClientId = UniqueClientId;
		m_pPlugin = pPlugin;
	}
};

#endif

#endif
