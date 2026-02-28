#ifdef CONF_LUA
#include "lua_plugin.h"

#include <base/dbg.h>
#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

#include <game/server/ddnetpp/lua/lua_game.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>

#include <string>
extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

CLuaPlugin::CLuaPlugin(const char *pName, const char *pFullPath, CLuaGame *pGame)
{
	log_info("lua", "initializing plugin %s ...", pName);
	m_pLuaState = luaL_newstate();
	luaL_openlibs(LuaState());
	str_copy(m_aName, pName);
	str_copy(m_aFullPath, pFullPath);
	m_pGame = pGame;
}

CLuaPlugin::~CLuaPlugin()
{
	if(LuaState())
	{
		log_info("lua", "cleaning up plugin %s ...", Name());
		lua_close(LuaState());
	}
}

void CLuaPlugin::RegisterGameTable()
{
	luaL_newmetatable(LuaState(), "Game");

	// --- Define __index (methods) ---
	lua_pushstring(LuaState(), "__index");
	lua_newtable(LuaState()); // Create method table

	// Add methods
	lua_pushstring(LuaState(), "send_chat");
	lua_pushcfunction(LuaState(), CallbackSendChat);
	lua_settable(LuaState(), -3);

	lua_pushstring(LuaState(), "call_plugin");
	lua_pushcfunction(LuaState(), CallbackCallPlugin);
	lua_settable(LuaState(), -3);

	lua_pushstring(LuaState(), "register_rcon");
	lua_pushcfunction(LuaState(), CallbackRegisterRcon);
	lua_settable(LuaState(), -3);

	lua_pushstring(LuaState(), "plugin_name");
	lua_pushcfunction(LuaState(), CallbackPluginName);
	lua_settable(LuaState(), -3);

	// Set __index = method_table
	lua_settable(LuaState(), -3);

	lua_pop(LuaState(), 1); // Pop metatable
}

void CLuaPlugin::RegisterGameInstance()
{
	lua_pushlightuserdata(LuaState(), this);
	luaL_getmetatable(LuaState(), "Game");
	lua_setmetatable(LuaState(), -2);
	lua_setglobal(LuaState(), "Game");
}

void CLuaPlugin::RegisterGlobalState()
{
	RegisterGameTable();
	RegisterGameInstance();
}

bool CLuaPlugin::LoadFile()
{
	if(luaL_dofile(LuaState(), FullPath()) != LUA_OK)
	{
		const char *pError = lua_tostring(LuaState(), -1);
		log_error("lua", "%s: %s", FullPath(), pError);
		lua_pop(LuaState(), 1);
		SetError(pError);
		return false;
	}
	return true;
}

bool CLuaPlugin::CallLuaVoidNoArgs(const char *pFunction)
{
	lua_getglobal(LuaState(), pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal because we dont run pcall
		lua_pop(LuaState(), 1);
		// log_error("lua", "%s is nil", pFunction);
		return false;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal because we dont run pcall
		lua_pop(LuaState(), 1);
		// log_error("lua", "%s is not a function", pFunction);
		return false;
	}
	if(lua_pcall(LuaState(), 0, 0, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to call %s() with error: %s", Name(), pFunction, pErrorMsg);
		SetError(pErrorMsg);
	}
	return true;
}

int CLuaPlugin::CallbackSendChat(lua_State *L)
{
	CLuaPlugin *pSelf = ((CLuaPlugin *)lua_touserdata(L, 1));
	CLuaGame *pGame = pSelf->Game();
	const char *pMessage = lua_tostring(L, 2);
	pGame->SendChat(pMessage);
	return 0;
}

int CLuaPlugin::CallbackCallPlugin(lua_State *L)
{
	CLuaPlugin *pSelf = ((CLuaPlugin *)lua_touserdata(L, 1));
	CLuaGame *pGame = pSelf->Game();
	const char *pFunction = lua_tostring(L, 2);

	// TODO: now that we also have pSelf available it would be nicer to pass
	//       that as argument to CallPlugin instead of L
	//       so we can fully operate on the caller plugin if needed

	if(!pGame->Controller()->Lua()->CallPlugin(pFunction, L))
	{
		// luaL_error(L, "no plugin implements %s()", pFunction);

		// ok = false
		lua_pushboolean(L, false);

		// data = nil
		lua_pushnil(L);
		return 2;
	}

	// ok and data have to be set by CallPlugin
	return 2;
}

static const char *LuaCheckStringStrict(lua_State *L, int Index)
{
	int Type = lua_type(L, Index);
	if(Type == LUA_TSTRING)
		return luaL_checkstring(L, Index);

	char aError[512];
	str_format(aError, sizeof(aError), "string expected, got %s", luaL_typename(L, Index));
	luaL_argerror(L, Index, aError);

	// unreachable
	return "";
}

int CLuaPlugin::CallbackRegisterRcon(lua_State *L)
{
	CLuaPlugin *pSelf = ((CLuaPlugin *)lua_touserdata(L, 1));

	const char *pName = LuaCheckStringStrict(L, 2);
	luaL_checktype(L, 3, LUA_TFUNCTION);

	if(pSelf->m_RconCommands.contains(std::string(pName)))
	{
		// silently override previous registrations
		// not sure what is the most user friendly here
		// could also throw an error so users dont get confused
		// but that takes away the flexibility to override library code
		// could also register both callbacks and run both
		// but thats also odd i think
		//
		// maybe the best would be to error on duplication by default
		// and provider another method like `Game:register_rcon_override()`
		// to intentionally override

		// log_warn("lua", "%s was already registered in rcon", pName);
		int OldFunc = pSelf->m_RconCommands.at(std::string(pName));
		luaL_unref(L, LUA_REGISTRYINDEX, OldFunc);
	}

	// we need to move stack 3 to 0
	// because luaL_ref pops first element from the stack
	// but it is our thirf argument

	// so we just pop the first two args

	// push copy of the third argument (the lua callback)
	// onto the top of the stack so luaL_ref can find it
	lua_pushvalue(L, 3);
	int FuncRef = luaL_ref(L, LUA_REGISTRYINDEX);
	pSelf->m_RconCommands[std::string(pName)] = FuncRef;
	// pop our pushed value
	lua_pop(L, 1);

	lua_pushboolean(L, true);
	return 1;
}

int CLuaPlugin::CallbackPluginName(lua_State *L)
{
	CLuaPlugin *pSelf = ((CLuaPlugin *)lua_touserdata(L, 1));
	lua_pushstring(L, pSelf->Name());
	return 1;
}

void CLuaPlugin::OnInit()
{
	dbg_assert(IsActive(), "called inactive plugin");
	CallLuaVoidNoArgs("on_init");
}

void CLuaPlugin::OnTick()
{
	dbg_assert(IsActive(), "called inactive plugin");
	CallLuaVoidNoArgs("on_tick");
}

void CLuaPlugin::OnPlayerConnect()
{
	dbg_assert(IsActive(), "called inactive plugin");
	CallLuaVoidNoArgs("on_player_connect");
}

bool CLuaPlugin::OnRconCommand(int ClientId, const char *pCommand, const char *pArguments)
{
	dbg_assert(IsActive(), "called inactive plugin");

	if(!m_RconCommands.contains(pCommand))
	{
		// log_info("lua", "plugin '%s' does not know rcon command '%s'", Name(), pCommand);
		return false;
	}
	// log_info("lua", "plugin '%s' does know rcon command '%s'", Name(), pCommand);

	int FuncRef = m_RconCommands.at(pCommand);
	if(FuncRef == LUA_REFNIL)
	{
		char aError[512];
		str_format(aError, sizeof(aError), "invalid lua callback for rcon command '%s'", pCommand);
		log_error("lua", "%s", aError);
		SetError(aError);
		return false;
	}

	lua_rawgeti(LuaState(), LUA_REGISTRYINDEX, FuncRef);

	lua_pushinteger(LuaState(), ClientId);
	lua_pushstring(LuaState(), pArguments);
	if(lua_pcall(LuaState(), 2, 0, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to run callback for rcon command '%s' with error: %s", Name(), pCommand, pErrorMsg);
		SetError(pErrorMsg);
	}
	return true;
}

bool CLuaPlugin::CallPlugin(const char *pFunction, lua_State *pCaller)
{
	dbg_assert(IsActive(), "called inactive plugin");

	lua_getglobal(LuaState(), pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal because we dont run pcall
		lua_pop(LuaState(), 1);
		// log_error("lua", "%s is nil", pFunction);
		return false;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal because we dont run pcall
		lua_pop(LuaState(), 1);
		// log_error("lua", "%s is not a function", pFunction);
		return false;
	}

	int NumArgs = PassOnArgs(pFunction, pCaller, 3);
	if(lua_pcall(LuaState(), NumArgs, 1, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "%s", pErrorMsg);
		SetError(pErrorMsg);
	}

	// return true as "ok" or "found" as first value
	// to signal the caller that a plugin got this function
	// and the second argument will be its return value
	lua_pushboolean(pCaller, true);

	if(lua_isinteger(LuaState(), -1))
	{
		lua_pushinteger(pCaller, lua_tointeger(LuaState(), -1));
	}
	else if(lua_isnumber(LuaState(), -1))
	{
		lua_pushnumber(pCaller, lua_tonumber(LuaState(), -1));
	}
	else if(lua_isboolean(LuaState(), -1))
	{
		lua_pushboolean(pCaller, lua_toboolean(LuaState(), -1));
	}
	else if(lua_isnil(LuaState(), -1))
	{
		lua_pushnil(pCaller);
	}
	else if(lua_isstring(LuaState(), -1))
	{
		lua_pushstring(pCaller, lua_tostring(LuaState(), -1));
	}
	else if(lua_istable(LuaState(), -1))
	{
		CopyReturnedTable(pFunction, pCaller, 0);
	}
	else
	{
		log_warn("lua", "plugin '%s' returned unsupported type from function %s()", Name(), pFunction);
		lua_pushnil(pCaller);
	}

	return true;
}

bool CLuaPlugin::IsRconCmdKnown(const char *pCommand)
{
	dbg_assert(IsActive(), "called inactive plugin");

	if(!m_RconCommands.contains(pCommand))
	{
		// log_info("lua", "plugin '%s' does not know rcon command '%s'", Name(), pCommand);
		return false;
	}
	// log_info("lua", "plugin '%s' does know rcon command '%s'", Name(), pCommand);

	int FuncRef = m_RconCommands.at(pCommand);
	if(FuncRef == LUA_REFNIL)
		return false;
	return true;
}

bool CLuaPlugin::CopyReturnedTable(const char *pFunction, lua_State *pCaller, int Depth)
{
	// not sure how safe it is to keep this assert here xd
	// but it for sure helps debugging
	dbg_assert(lua_istable(LuaState(), -1), "copy table got not table");

	// TODO: this entire thing is a huge mess already and far from complete wtf
	//       also it looks horribly slow
	//       there has to be a better way to do this

	size_t TableLen = lua_rawlen(LuaState(), -1);
	// log_info("lua", "%*sgot table with %zu keys (depth=%d)", Depth, "", TableLen, Depth);

	// random af idk what im doing
	lua_checkstack(pCaller, TableLen * 4);

	// TODO: use the faster table creator because we know the size
	lua_newtable(pCaller);

	// Push another reference to the table on top of the stack (so we know
	// where it is, and this function can work for negative, positive and
	// pseudo indices
	lua_pushvalue(LuaState(), -1);
	// stack now contains: -1 => table
	lua_pushnil(LuaState());
	// stack now contains: -1 => nil; -2 => table
	while(lua_next(LuaState(), -2))
	{
		// stack now contains: -1 => value; -2 => key; -3 => table
		// copy the key so that lua_tostring does not modify the original
		lua_pushvalue(LuaState(), -2);
		// stack now contains: -1 => key; -2 => value; -3 => key; -4 => table

		// key scope for organization
		{
			if(lua_isinteger(LuaState(), -1))
			{
				lua_pushinteger(pCaller, lua_tointeger(LuaState(), -1));
			}
			else if(lua_isstring(LuaState(), -1))
			{
				lua_pushstring(pCaller, lua_tostring(LuaState(), -1));
			}
			else
			{
				// crashing the server if the plugin is doing weird stuff is bad
				// lua supports weird values like functions as keys

				// TODO: try to hit this assert with a crazy table return
				//       and then test if there is a way to not crash the server with an assert
				//       and properly disable the plugin without breaking any lua state

				dbg_assert_failed("plugin '%s' returned unsupported table key from function %s()", Name(), pFunction);
			}

			// pop key so value is on the top of the stack for table
			// deep copy recursion
			lua_pop(LuaState(), 1);
		}

		// i feel like this is not the smartest way of doing a deep copy
		// we are listing all types twice
		// i feel like CopyReturnedTable() could be refactored
		// into CopyAnyValue()
		if(lua_isinteger(LuaState(), -1))
		{
			lua_pushinteger(pCaller, lua_tointeger(LuaState(), -1));
			lua_settable(pCaller, -3);
		}
		else if(lua_isnumber(LuaState(), -1))
		{
			lua_pushnumber(pCaller, lua_tonumber(LuaState(), -1));
			lua_settable(pCaller, -3);
		}
		else if(lua_istable(LuaState(), -1))
		{
			// log_info("lua", "%*sgot nested table", Depth, "");
			CopyReturnedTable(pFunction, pCaller, ++Depth);
			lua_settable(pCaller, -3);

			// pop table
			lua_pop(LuaState(), 1);
		}
		else if(lua_isstring(LuaState(), -1))
		{
			lua_pushstring(pCaller, lua_tostring(LuaState(), -1));
			lua_settable(pCaller, -3);
		}
		else
		{
			SetError("unsupported table value type returned");
			log_error("lua", "plugin '%s' returned unsupported table value from function %s()", Name(), pFunction);

			// fallback nil value for caller to leave the callers lua stack in good state
			lua_pushnil(pCaller);
			lua_settable(pCaller, -3);
		}

		// pop something, nobody knows what
		lua_pop(LuaState(), 1);

		// // pop value + copy of key, leaving original key
		// lua_pop(LuaState(), 2);
		// // stack now contains: -1 => key; -2 => table
	}
	// stack now contains: -1 => table (when lua_next returns 0 it pops the key
	// but does not push anything.)
	// Pop table
	if(Depth == 0)
		lua_pop(LuaState(), 1);
	// Stack is now the same as it was on entry to this function

	return true;
}

int CLuaPlugin::PassOnArgs(const char *pFunction, lua_State *pCaller, int StackOffset)
{
	int NumArgs = 0;
	for(int ArgStack = StackOffset; !lua_isnone(pCaller, ArgStack); ArgStack++)
	{
		if(lua_isinteger(pCaller, ArgStack))
		{
			NumArgs++;
			lua_pushinteger(LuaState(), lua_tointeger(pCaller, ArgStack));
		}
		else if(lua_isstring(pCaller, ArgStack))
		{
			NumArgs++;
			lua_pushstring(LuaState(), lua_tostring(pCaller, ArgStack));
		}
		else
		{
			log_warn("lua", "plugin '%s' in function %s() was called with unsupported arg", Name(), pFunction);
			break;
		}
	}
	return NumArgs;
}

void CLuaPlugin::SetError(const char *pErrorMsg)
{
	dbg_assert(pErrorMsg, "lua plugin error is NULL");
	dbg_assert(pErrorMsg[0], "lua plugin error is empty");
	str_copy(m_aErrorMsg, pErrorMsg);
}

#endif
