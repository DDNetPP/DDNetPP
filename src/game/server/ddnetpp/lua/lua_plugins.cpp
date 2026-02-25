#ifdef CONF_LUA
#include "lua_plugins.h"

#include <base/log.h>

#include <game/server/gamecontext.h>

#include <lua.hpp>

static int LuaCallbackSendChat(lua_State *L)
{
	CLuaGame *pGame = (CLuaGame *)lua_touserdata(L, 1);
	const char *pMessage = lua_tostring(L, 2);
	pGame->SendChat(pMessage);
	lua_pushinteger(L, 666);
	return 1;
}

void CLuaGame::Init(IGameController *pController, CGameContext *pGameServer)
{
	m_pController = pController;
	m_pGameServer = pGameServer;
}

void CLuaGame::SendChat(const char *pMessage)
{
	GameServer()->SendChat(-1, TEAM_ALL, pMessage);
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

void CLuaController::Init(IGameController *pController, CGameContext *pGameServer)
{
	log_info("lua", "init bridge..");
	m_Game.Init(pController, pGameServer);

	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	RegisterLuaBridgeTable(L);
	PushGameToLua(L, &m_Game);

	// TODO: can we share state between multiple plugins?

	if(luaL_dofile(L, "plugin.lua") != LUA_OK)
	{
		const char *pLuaError = lua_tostring(L, -1);
		log_error("lua", "%s", pLuaError);
		lua_pop(L, 1);
	}
	lua_close(L);
}

#endif
