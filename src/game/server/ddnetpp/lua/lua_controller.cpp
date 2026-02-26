#ifdef CONF_LUA
#include "lua_controller.h"

#include <base/dbg.h>
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

	char aName[IO_MAX_PATH_LENGTH];
	str_copy(aName, pFilename);
	aName[str_length(aName) - str_length(".lua")] = '\0';
	pSelf->LoadPlugin(aName, aFullpath);

	return 0;
}

void CLuaController::Init(IGameController *pController, CGameContext *pGameServer)
{
	log_info("lua", "init bridge..");
	m_pController = pController;
	m_pGameServer = pGameServer;
	m_Game.Init(pController, pGameServer);

	ReloadPlugins();
}

CLuaController::~CLuaController()
{
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		delete pPlugin;
	}
}

void CLuaController::OnTick()
{
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		pPlugin->OnTick();
	}
}

void CLuaController::OnInit()
{
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		pPlugin->OnInit();
	}
}

bool CLuaController::LoadPlugin(const char *pName, const char *pFilename)
{
	log_info("lua", "loading script %s ...", pFilename);

	CLuaPlugin *pPlugin = new CLuaPlugin(pName, pFilename);

	// also push plugins even if they crash on load
	// so we can show them to admins in the list plugins rcon
	// command together with a useful error message
	m_vpPlugins.emplace_back(pPlugin);

	// TODO: make this less ugly. Where does it belong?
	RegisterLuaBridgeTable(pPlugin->LuaState());
	PushGameToLua(pPlugin->LuaState(), &m_Game);

	if(luaL_dofile(pPlugin->LuaState(), pFilename) != LUA_OK)
	{
		const char *pLuaError = lua_tostring(pPlugin->LuaState(), -1);
		log_error("lua", "%s: %s", pFilename, pLuaError);
		lua_pop(pPlugin->LuaState(), 1);
		pPlugin->SetError(pLuaError);
		return false;
	}
	return true;
}

void CLuaController::ReloadPlugins()
{
	GameServer()->Storage()->ListDirectory(IStorage::TYPE_ALL, "plugins", FsListPluginCallback, this);
	OnInit();
}

#endif
