#ifdef CONF_LUA
#include "lua_plugins.h"

#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

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

int CLuaController::FsListPluginCallback(const char *pFilename, int IsDir, int DirType, void *pUser)
{
	CLuaController *pSelf = (CLuaController *)pUser;
	if(IsDir || !str_comp(".", pFilename) || !str_comp("..", pFilename))
		return 0;
	if(!str_endswith(pFilename, ".lua"))
		return 0;

	char aBasePath[IO_MAX_PATH_LENGTH];
	pSelf->GameServer()->Storage()->GetCompletePath(DirType, "plugins", aBasePath, sizeof(aBasePath));
	char aFullpath[IO_MAX_PATH_LENGTH];
	str_format(aFullpath, sizeof(aFullpath), "%s/%s", aBasePath, pFilename);
	pSelf->LoadPlugin(aFullpath);

	return 0;
}

void CLuaController::Init(IGameController *pController, CGameContext *pGameServer)
{
	log_info("lua", "init bridge..");
	m_pController = pController;
	m_pGameServer = pGameServer;
	m_Game.Init(pController, pGameServer);

	m_pLuaState = luaL_newstate();
	lua_State *L = m_pLuaState;
	luaL_openlibs(L);

	RegisterLuaBridgeTable(L);
	PushGameToLua(L, &m_Game);

	ReloadPlugins();
}

CLuaController::~CLuaController()
{
	if(m_pLuaState)
	{
		log_info("lua", "cleaning up lua state...");
		lua_close(LuaState());
	}
}

bool CLuaController::LoadPlugin(const char *pFilename)
{
	log_info("lua", "loading script %s ...", pFilename);

	// TODO: can we share state between multiple plugins?
	if(luaL_dofile(LuaState(), pFilename) != LUA_OK)
	{
		const char *pLuaError = lua_tostring(LuaState(), -1);
		log_error("lua", "%s: %s", pFilename, pLuaError);
		lua_pop(LuaState(), 1);
		return false;
	}
	return true;
}

void CLuaController::ReloadPlugins()
{
	GameServer()->Storage()->ListDirectory(IStorage::TYPE_ALL, "plugins", FsListPluginCallback, this);
}

#endif
