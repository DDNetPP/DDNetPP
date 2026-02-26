#ifdef CONF_LUA
#include "lua_controller.h"

#include <base/dbg.h>
#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

#include <game/server/gamecontext.h>

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
	pPlugin->RegisterGlobalState(&m_Game);
	return pPlugin->LoadFile();
}

void CLuaController::ReloadPlugins()
{
	GameServer()->Storage()->ListDirectory(IStorage::TYPE_ALL, "plugins", FsListPluginCallback, this);
	OnInit();
}

#endif
