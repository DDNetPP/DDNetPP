#include "lua_controller.h"

#include <base/dbg.h>
#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

#include <game/server/gamecontext.h>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

int CLuaController::FsListPluginCallback(const char *pFilename, int IsDir, int DirType, void *pUser)
{
#ifdef CONF_LUA
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
#else
	return 0;
#endif
}

void CLuaController::Init(IGameController *pController, CGameContext *pGameServer)
{
#ifdef CONF_LUA
	lua_State *pTmpState = luaL_newstate();
	log_info("lua", "got lua version %.0f, loading plugins ...", lua_version(pTmpState));
	lua_close(pTmpState);

	m_pController = pController;
	m_pGameServer = pGameServer;
	m_Game.Init(pController, pGameServer);

	ReloadPlugins();
#endif
}

CLuaController::~CLuaController()
{
#ifdef CONF_LUA
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		delete pPlugin;
	}
#endif
}

void CLuaController::ListPlugins()
{
#ifdef CONF_LUA
	size_t NumPlugins = m_vpPlugins.size();
	log_info("lua", "currently loaded %" PRIzu " plugins:", NumPlugins);

	size_t NumPrinted = 0;
	size_t MaxPrint = 16;
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(NumPrinted++ > (MaxPrint - 1))
		{
			log_info("lua", " and %" PRIzu " more ..", (NumPlugins - NumPrinted) + 1);
			break;
		}
		if(pPlugin->IsError())
		{
			log_info(
				"lua",
				" ! '%s' - failed: %s",
				pPlugin->Name(),
				pPlugin->ErrorMsg());
			continue;
		}
		if(!pPlugin->IsActive())
		{
			log_info(
				"lua",
				" ✘ '%s' (off)",
				pPlugin->Name());
			continue;
		}
		log_info(
			"lua",
			" ✔ '%s'",
			pPlugin->Name());
	}
#else
	log_error("lua", "lua plugin support is not enabled");
#endif
}

void CLuaController::OnInit()
{
#ifdef CONF_LUA
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		pPlugin->OnInit();
	}
#endif
}

void CLuaController::OnTick()
{
#ifdef CONF_LUA
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		pPlugin->OnTick();
	}
#endif
}

void CLuaController::OnPlayerConnect()
{
#ifdef CONF_LUA
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		pPlugin->OnPlayerConnect();
	}
#endif
}

bool CLuaController::CallPlugin(const char *pFunction, lua_State *pCaller)
{
#ifdef CONF_LUA
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		if(pPlugin->CallPlugin(pFunction, pCaller))
			return true;
	}
	return false;
#else
	return false;
#endif
}

bool CLuaController::LoadPlugin(const char *pName, const char *pFilename)
{
#ifdef CONF_LUA
	log_info("lua", "loading script %s ...", pFilename);
	CLuaPlugin *pPlugin = new CLuaPlugin(pName, pFilename, &m_Game);

	// also push plugins even if they crash on load
	// so we can show them to admins in the list plugins rcon
	// command together with a useful error message
	m_vpPlugins.emplace_back(pPlugin);
	pPlugin->RegisterGlobalState();
	return pPlugin->LoadFile();
#else
	return false;
#endif
}

void CLuaController::ReloadPlugins()
{
#ifdef CONF_LUA
	GameServer()->Storage()->ListDirectory(IStorage::TYPE_ALL, "plugins", FsListPluginCallback, this);
	OnInit();
#endif
}
