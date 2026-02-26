#ifdef CONF_LUA
#include "lua_plugin.h"

#include <base/dbg.h>
#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

#include <game/server/gamecontext.h>

#include <lua.hpp>

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

void CLuaPlugin::CallLuaVoidNoArgs(const char *pFunction)
{
	// TODO: don't we need to pop the global of the stack again?
	//       i tried `lua_pop(LuaState(), 1);` and it segfaulted
	//       not popping it works but i feel like its wrong
	lua_getglobal(LuaState(), pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// log_error("lua", "%s is nil", pFunction);
		return;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// log_error("lua", "%s is not a function", pFunction);
		return;
	}
	if(lua_pcall(LuaState(), 0, 0, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "%s", pErrorMsg);
		SetError(pErrorMsg);
	}
}

int CLuaPlugin::CallbackSendChat(lua_State *L)
{
	CLuaGame *pGame = (CLuaGame *)lua_touserdata(L, 1);
	const char *pMessage = lua_tostring(L, 2);
	pGame->SendChat(pMessage);
	return 0;
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

void CLuaPlugin::SetError(const char *pErrorMsg)
{
	dbg_assert(pErrorMsg, "lua plugin error is NULL");
	dbg_assert(pErrorMsg[0], "lua plugin error is empty");
	str_copy(m_aErrorMsg, pErrorMsg);
}

#endif
