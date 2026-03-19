#ifndef GAME_SERVER_DDNETPP_LUA_LUA_PLUGIN_H
#define GAME_SERVER_DDNETPP_LUA_LUA_PLUGIN_H

#ifdef CONF_LUA

#include <base/log.h>
#include <base/str.h>
#include <base/types.h>
#include <base/vmath.h>

#include <generated/protocol.h>

#include <game/server/ddnetpp/lua/lua_game.h>
#include <game/server/ddnetpp/lua/lua_rcon_command.h>
#include <game/server/minigames/minigame_base.h>

#include <optional>
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
class CCharacter;

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

	// HOLY ANTI PATTERN WHAT THE FLIPFLOP IS GOING ON HERE XD
	// this is some cursed ah shit but it works and makes
	// the lua api simpler for the end user
	//
	// The thing is we need to get the snap receivers ddnet client version
	// when creating certain snap items that have ddnet extensions
	// lua knows the client id when it creates these items so it could
	// pass it back to C++ but i would like to hide that complexity
	// there should never be a need for a plugin to ignore the ddnet client version
	// maybe some super edge case hacky test cases, but in that case just use C++ duh.
	// So snap items can only be created within on_snap and during the execution of that function
	// we ensure that m_SnappingClient is set correctly.
	//
	// So in lua instead of this:
	//
	// function ddnetpp.on_snap(snapping_client)
	//   ddnetpp.snap.new_pickup({snapping_client = snapping_client, weapon = 2}
	// end
	//
	// Users only have to do this:
	//
	// function ddnetpp.on_snap(snapping_client)
	//   ddnetpp.snap.new_pickup({weapon = 2}
	// end
	//
	// TODO: Could also make it an std::optional and unset after on_snap
	//       and then properly lua error all calls to snap item creations
	//       instead of hitting the server assert
	int m_SnappingClient = -1;

public:
	// The key is the rcon command name
	std::unordered_map<std::string, CLuaRconCommand> m_RconCommands;

	// The key is the chat command name
	std::unordered_map<std::string, CLuaRconCommand> m_ChatCommands;

	// The snap item ids that were allocated by the plugin
	// using Server()->SnapNewId() we auto free them when the plugin is being reloaded
	std::vector<int> m_vSnapIds;

	CLuaPlugin(const char *pName, const char *pFullPath, CLuaGame *pGame);
	~CLuaPlugin();

	CLuaGame *m_pGame = nullptr;
	CLuaGame *Game() { return m_pGame; }
	const CLuaGame *Game() const { return m_pGame; }

private:
	void FreeSnapIds();

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
	// returns value if the function was found
	std::optional<int> CallLuaIntWithTwoInts(const char *pFunction, int Num1, int Num2);
	// returns true if the function was found
	bool CallLuaVoidWithPlayer(const char *pFunction, const CPlayer *pPlayer);

	// Returns false if there was no position table found at the index
	// and sets the plugin into error state
	// otherwise returns true and writes position to pOutPos
	[[nodiscard]] bool LuaGetPositionReturnValueOrError(const char *pFunction, int Index, vec2 *pOutPos);

	// set plugin to error state (but do NOT throw lua error) if the return value from an event was not a table
	// it does not throw a C++ exception so you need to look at the return value
	// if it is false there is an error
	[[nodiscard]] bool LuaReturnValueIsTableOrError(const char *pFunction, int Index, const char *pExpectedTableName);

	// Calling C++ from lua
	static int CallbackSendChat(lua_State *L);
	static int CallbackSendChatTarget(lua_State *L);
	static int CallbackSendBroadcast(lua_State *L);
	static int CallbackSendBroadcastTarget(lua_State *L);
	static int CallbackSendMotd(lua_State *L);
	static int CallbackSendMotdTarget(lua_State *L);
	static int CallbackLaserText(lua_State *L);
	static int CallbackRcon(lua_State *L);
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
	static int CallbackSnapNewPickup(lua_State *L);
	static int CallbackSnapNewCharacter(lua_State *L);
	static int CallbackSnapNewPlayerInfo(lua_State *L);
	static int CallbackSnapNewClientInfo(lua_State *L);

	// server
	static int CallbackServerTick(lua_State *L);
	static int CallbackServerTickSpeed(lua_State *L);

	// collision
	static int CallbackCollisionWidth(lua_State *L);
	static int CallbackCollisionHeight(lua_State *L);
	static int CallbackCollisionGetTileIndex(lua_State *L);
	static int CallbackCollisionGetTile(lua_State *L);
	static int CallbackCollisionMoveBox(lua_State *L);

	// TODO: move these to some other scope? Because we have some
	//       for the game instance and some for the player
	static int CallbackPlayerId(lua_State *L);
	static int CallbackPlayerName(lua_State *L);
	static int CallbackPlayerSetSkin(lua_State *L);
	static int CallbackPlayerUnsetSkin(lua_State *L);
	static int CallbackPlayerUnsetSkinColorBody(lua_State *L);

	// TODO: new scope?
	static int CallbackCharacterPos(lua_State *L);
	static int CallbackCharacterSetPosition(lua_State *L);
	static int CallbackCharacterId(lua_State *L);
	static int CallbackCharacterPlayer(lua_State *L);
	static int CallbackCharacterDie(lua_State *L);

public:
	// Calling lua from C++
	void OnInit();
	void OnTick();
	void OnPlayerTick(const CPlayer *pPlayer);
	bool OnCharacterTile(CCharacter *pChr, int GameIndex, int FrontIndex);
	bool OnCharacterGameTileChange(CCharacter *pChr, int GameIndex);
	bool OnSkipGameTile(CCharacter *pChr, int GameIndex);
	void OnSnap(int SnappingClient);
	int OnSnapGameInfoExFlags(int SnappingClient, int DDRaceFlags);
	int OnSnapGameInfoExFlags2(int SnappingClient, int DDRaceFlags);
	bool OnChatMessage(int ClientId, CNetMsg_Cl_Say *pMsg, int &Team);
	void OnPlayerConnect(int ClientId);
	void OnPlayerDisconnect(int ClientId);
	std::optional<vec2> OnPickSpawnPos(CPlayer *pPlayer);
	bool OnRconCommand(int ClientId, const char *pCommand, const char *pArguments);
	bool OnChatCommand(int ClientId, const char *pCommand, const char *pArguments);
	void OnSetAuthed(int ClientId, int Level);
	bool OnFireWeapon(int ClientId, int Weapon, vec2 Direction, vec2 MouseTarget, vec2 ProjStartPos);
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
