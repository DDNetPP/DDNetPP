#ifdef CONF_LUA
#include "lua_plugin.h"

#include <base/dbg.h>
#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>

#include <lua.h>

CLuaPlugin::CLuaPlugin(const char *pName, const char *pFullPath)
{
	log_info("lua", "initializing plugin %s ...", pName);
	m_pLuaState = luaL_newstate();
	luaL_openlibs(LuaState());
	str_copy(m_aName, pName);
	str_copy(m_aFullPath, pFullPath);
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

	// Set __index = method_table
	lua_settable(LuaState(), -3);

	lua_pop(LuaState(), 1); // Pop metatable
}

void CLuaPlugin::RegisterGameInstance(CLuaGame *pGame)
{
	lua_pushlightuserdata(LuaState(), pGame);
	luaL_getmetatable(LuaState(), "Game");
	lua_setmetatable(LuaState(), -2);
	lua_setglobal(LuaState(), "Game");
}

void CLuaPlugin::RegisterGlobalState(CLuaGame *pGame)
{
	RegisterGameTable();
	RegisterGameInstance(pGame);
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
	CLuaGame *pGame = (CLuaGame *)lua_touserdata(L, 1);
	const char *pMessage = lua_tostring(L, 2);
	pGame->SendChat(pMessage);
	return 0;
}

int CLuaPlugin::CallbackCallPlugin(lua_State *L)
{
	CLuaGame *pGame = (CLuaGame *)lua_touserdata(L, 1);
	const char *pFunction = lua_tostring(L, 2);

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

bool CLuaPlugin::CopyReturnedTable(const char *pFunction, lua_State *pCaller, int Depth)
{
	// not sure how safe it is to keep this assert here xd
	// but it for sure helps debugging
	dbg_assert(lua_istable(LuaState(), -1), "copy table got not table");

	// TODO: this entire thing is a huge mess already and far from complete wtf
	//       also it looks horribly slow
	//       there has to be a better way to do this

	size_t TableLen = lua_rawlen(LuaState(), -1);
	// log_info("lua", "got table with %zu keys", TableLen);

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
		// TODO: check key type and error if its unsupported
		// TODO: support other values than string and error if its not supported
		const char *pKey = lua_tostring(LuaState(), -1);
		const char *pValue = lua_tostring(LuaState(), -2);
		// log_info("lua", "copy table %s => %s", pKey, pValue);

		lua_pushstring(pCaller, pKey);

		if(lua_istable(LuaState(), -2))
		{
			log_info("lua", "got nested table");

			lua_newtable(pCaller);
			lua_pushstring(pCaller, "key");
			lua_pushstring(pCaller, "val");
			lua_settable(pCaller, -3);

			lua_settable(pCaller, -3);

			// NO WAY THIS WORKS=???
			// CopyReturnedTable(pFunction, pCaller);
		}
		else
		{
			lua_pushstring(pCaller, pValue);

			// TODO: this could probably be in the scope below
			//       depending on what happens with the table copy above
			lua_settable(pCaller, -3);
		}

		// pop value + copy of key, leaving original key
		lua_pop(LuaState(), 2);
		// stack now contains: -1 => key; -2 => table
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
