#include <game/server/gamecontroller.h>
#ifdef CONF_LUA
#include "lua_plugin.h"

#include <base/dbg.h>
#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

#include <game/server/gamecontext.h>

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
		log_error("lua", "%s", pErrorMsg);
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

	// TODO: pass in table as arg and get nested table result as return val

	// TODO: return rust like result to lua
	//       {ERROR, nil}
	//       or
	//       {OK, {val}}
	pGame->Controller()->Lua()->CallPlugin(pFunction, L);

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
	if(lua_pcall(LuaState(), 0, 1, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "%s", pErrorMsg);
		SetError(pErrorMsg);
	}

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
		// TODO: this entire thing is a huge mess already and far from complete wtf
		//       should be moved to another method and properly tested
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
			lua_pushstring(pCaller, pValue);
			lua_settable(pCaller, -3);

			// pop value + copy of key, leaving original key
			lua_pop(LuaState(), 2);
			// stack now contains: -1 => key; -2 => table
		}
		// stack now contains: -1 => table (when lua_next returns 0 it pops the key
		// but does not push anything.)
		// Pop table
		lua_pop(LuaState(), 1);
		// Stack is now the same as it was on entry to this function
	}
	else
	{
		log_warn("lua", "plugin '%s' returned unsupported type from function %s()", Name(), pFunction);
		lua_pushnil(pCaller);
	}

	return true;
}

void CLuaPlugin::SetError(const char *pErrorMsg)
{
	dbg_assert(pErrorMsg, "lua plugin error is NULL");
	dbg_assert(pErrorMsg[0], "lua plugin error is empty");
	str_copy(m_aErrorMsg, pErrorMsg);
}

#endif
