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

static int LuaCallbackSendChat(lua_State *L)
{
	CLuaGame *pGame = (CLuaGame *)lua_touserdata(L, 1);
	const char *pMessage = lua_tostring(L, 2);
	pGame->SendChat(pMessage);
	lua_pushinteger(L, 666);
	return 1;
}

static void RegisterLuaBridgeTable(lua_State *L)
{
	luaL_newmetatable(L, "Game");

	// --- Define __index (methods) ---
	lua_pushstring(L, "__index");
	lua_newtable(L); // Create method table

	// Add methods
	lua_pushstring(L, "send_chat");
	lua_pushcfunction(L, LuaCallbackSendChat);
	lua_settable(L, -3);

	// Set __index = method_table
	lua_settable(L, -3);

	lua_pop(L, 1); // Pop metatable
}

static void PushGameToLua(lua_State *L, CLuaGame *pGame)
{
	lua_pushlightuserdata(L, pGame);
	luaL_getmetatable(L, "Game");
	lua_setmetatable(L, -2);
	lua_setglobal(L, "Game");
}

void CLuaPlugin::RegisterGlobalState(CLuaGame *pGame)
{
	// TODO: make this less ugly. Where does it belong?
	RegisterLuaBridgeTable(LuaState());
	PushGameToLua(LuaState(), pGame);
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

void CLuaPlugin::SetError(const char *pErrorMsg)
{
	dbg_assert(pErrorMsg, "lua plugin error is NULL");
	dbg_assert(pErrorMsg[0], "lua plugin error is empty");
	str_copy(m_aErrorMsg, pErrorMsg);
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

#endif
