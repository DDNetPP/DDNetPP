#include "lua_controller.h"

#include <base/dbg.h>
#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

#include <engine/server/server.h>
#include <engine/shared/protocol.h>

#include <generated/protocol.h>

#include <game/server/ddnetpp/lua/lua_plugin.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>

#include <utility>
#include <vector>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

int CLuaController::FsLoadDirPlugin(const char *pDirname, int DirType, void *pUser)
{
#ifdef CONF_LUA
	CLuaController *pSelf = (CLuaController *)pUser;
	// log_info("lua", "got dir '%s'", pDirname);

	char aEntry[IO_MAX_PATH_LENGTH];
	str_format(aEntry, sizeof(aEntry), "plugins/%s/src/main.lua", pDirname);
	if(!pSelf->GameServer()->Storage()->FileExists(aEntry, DirType))
	{
		// log_warn("lua", "missing file '%s'", aEntry);
		return 0;
	}

	char aFullpath[IO_MAX_PATH_LENGTH];
	pSelf->GameServer()->Storage()->GetCompletePath(DirType, aEntry, aFullpath, sizeof(aFullpath));
	// log_info("lua", " found entry '%s'", aFullpath);
	pSelf->LoadPlugin(pDirname, aFullpath);
	return 0;
#else
	return 0;
#endif
}

int CLuaController::FsListPluginCallback(const char *pFilename, int IsDir, int DirType, void *pUser)
{
#ifdef CONF_LUA
	CLuaController *pSelf = (CLuaController *)pUser;
	if(!str_comp(".", pFilename) || !str_comp("..", pFilename))
		return 0;
	if(IsDir)
		return FsLoadDirPlugin(pFilename, DirType, pUser);
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

void CLuaController::OnAddRconCmd(const CLuaRconCommand *pCmd)
{
#ifdef CONF_LUA
	for(const CPlayer *pPlayer : m_pGameServer->m_apPlayers)
	{
		// we do not push into 128 vectors for every single lua command
		// that is being added
		//
		// we just copy all lua commands at once if a new player auths
		// this happens in OnSetAuthed
		// only players already authed when a plugin adds new commands get live updates
		//
		// We could also do it the same for offline players and remove this
		// check. It would be simpler but I feel like performance wise
		// its a bit of a waste. Altho I do not like performance becoming worse
		// the more players are connected :/ ideally performance is guaranteed
		// and player count unrelated. So testing on an empty server is easier
		// we do not want to find bottlenecks in production once the server gets full
		// but whatever :/ it is was it is for now
		if(!pPlayer)
			continue;

		CLuaPlayerState &State = m_aPlayers[pPlayer->GetCid()];
		State.m_RconSender.AddRconCmd(pCmd);
	}
#endif
}

void CLuaPlayerState::CRconCmdSender::AddRconCmd(const CLuaRconCommand *pCmd)
{
#ifdef CONF_LUA
	if(!m_SendIndex.has_value())
	{
		m_SendIndex = 0;
	}

	// we already started a send group
	// so queue the cmd for the next group
	if(m_SendIndex.value() > 0)
	{
		m_vMissingCmdsNext.emplace_back(*pCmd);
		return;
	}

	m_vMissingCmds.emplace_back(*pCmd);
#endif
}

bool CLuaController::IsServerSendingNativeRconCmds(int ClientId)
{
#ifdef CONF_LUA
	if(!m_Game.Server()->ClientIngame(ClientId))
		return false;
	CServer *pServer = static_cast<CServer *>(m_Game.Server());
	CServer::CClient &Client = pServer->m_aClients[ClientId];

	// ddnet never sends to unauthed clients
	if(!pServer->IsRconAuthed(ClientId))
		return false;

	// currently sending rcon commands
	if(Client.m_pRconCmdToSend != nullptr)
		return true;

	// TODO: does ddnet really never send maps to 0.7 players?
	if(Client.m_Sixup)
		return false;

	if(Client.m_MaplistEntryToSend == CServer::CClient::MAPLIST_DISABLED ||
		Client.m_MaplistEntryToSend == CServer::CClient::MAPLIST_DONE)
		return false;

	// currently sending maps
	return true;
#else
	return false;
#endif
}

bool CLuaController::SendNextRconCmd(int ClientId)
{
#ifdef CONF_LUA
	if(IsServerSendingNativeRconCmds(ClientId))
	{
		// log_info("lua", "waiting for server to finish sending rcon commands ...");
		return false;
	}

	CLuaPlayerState::CRconCmdSender *pSender = &m_aPlayers[ClientId].m_RconSender;

	// send pending removals before adds
	if(pSender->m_RemoveIndex.has_value())
	{
		size_t NumSendMax = 3;
		size_t FromIdx = pSender->m_RemoveIndex.value();
		size_t ToIdx = std::min(pSender->m_vRemoveCmds.size() - 1, FromIdx + NumSendMax);
		pSender->m_RemoveIndex = ToIdx;

		// log_info(
		// 	"lua",
		// 	"sending removal commands from %" PRIzu " to %" PRIzu " to cid=%d",
		// 	FromIdx,
		// 	ToIdx,
		// 	ClientId);

		for(size_t i = FromIdx; i <= ToIdx; i++)
		{
			const char *pCmd = pSender->m_vRemoveCmds.at(i).c_str();
			m_Game.SendRconCmdRem(ClientId, pCmd);

			// log_info(
			// 	"lua",
			// 	" sending removal cmd='%s' %" PRIzu "/%" PRIzu " to cid=%d",
			// 	pCmd,
			// 	i,
			// 	pSender->m_vRemoveCmds.size(),
			// 	ClientId);
		}

		if(pSender->m_RemoveIndex.value() == (pSender->m_vRemoveCmds.size() - 1))
		{
			// log_info("lua", "done sending all removals to cid=%d", ClientId);
			pSender->m_RemoveIndex = std::nullopt;
			pSender->m_vRemoveCmds.clear();
		}

		return true;
	}

	if(pSender->m_SendIndex.has_value())
	{
		dbg_assert(
			pSender->m_SendIndex < pSender->m_vMissingCmds.size(),
			"rcon cmd send index=%" PRIzu " too big for cmds=%" PRIzu " cid=%d",
			pSender->m_SendIndex.value(),
			pSender->m_vMissingCmds.size(),
			ClientId);
	}

	if(!pSender->m_SendIndex.has_value())
	{
		if(pSender->m_vMissingCmds.empty())
		{
			// nothing to send
			if(pSender->m_vMissingCmdsNext.empty())
				return false;

			// some weird edge case where we have no send index but more commands??
			log_error("lua", "THIS IS WEIRD!");
			std::swap(pSender->m_vMissingCmds, pSender->m_vMissingCmdsNext);
			pSender->m_vMissingCmdsNext.clear();
		}

		// start new send
		pSender->m_SendIndex = 0;
		// log_info(
		// 	"lua",
		// 	"starting new rcon cmd send group of %" PRIzu " commands for cid=%d",
		// 	pSender->m_vMissingCmds.size(),
		// 	ClientId);

		m_Game.SendRconCmdGroupStart(ClientId, pSender->m_vMissingCmds.size());
	}

	size_t NumSendMax = 3;
	size_t FromIdx = pSender->m_SendIndex.value();
	size_t ToIdx = std::min(pSender->m_vMissingCmds.size() - 1, FromIdx + NumSendMax);
	pSender->m_SendIndex = ToIdx;

	// log_info(
	// 	"lua",
	// 	"sending commands from %" PRIzu " to %" PRIzu " to cid=%d",
	// 	FromIdx,
	// 	ToIdx,
	// 	ClientId);

	for(size_t i = FromIdx; i <= ToIdx; i++)
	{
		CLuaRconCommand *pCmd = &pSender->m_vMissingCmds.at(i);
		m_Game.SendRconCmdAdd(ClientId, pCmd);

		// log_info(
		// 	"lua",
		// 	" sending cmd='%s' %" PRIzu "/%" PRIzu " to cid=%d",
		// 	pCmd->Name(),
		// 	i,
		// 	pSender->m_vMissingCmds.size(),
		// 	ClientId);
	}

	// reached the end of the current group
	if(pSender->m_SendIndex.value() >= (pSender->m_vMissingCmds.size() - 1))
	{
		// log_info(
		// 	"lua",
		// 	"finished sending %" PRIzu " rcon commands to cid=%d",
		// 	pSender->m_vMissingCmds.size(),
		// 	ClientId);

		std::swap(pSender->m_vMissingCmds, pSender->m_vMissingCmdsNext);
		pSender->m_vMissingCmdsNext.clear();
		pSender->m_SendIndex = std::nullopt;

		// log_info(
		// 	"lua",
		// 	" there were %" PRIzu " new rcon commands queued while sending.",
		// 	pSender->m_vMissingCmds.size());

		m_Game.SendRconCmdGroupEnd(ClientId);

		// we do not instantly start the next group in this tick
		// that happens next tick
		return false;
	}

	return true;
#else
	return false;
#endif
}

void CLuaController::Init(IGameController *pController, CGameContext *pGameServer)
{
#ifdef CONF_LUA
	const char *pLuaVersion = LUA_VERSION_MAJOR "." LUA_VERSION_MINOR "." LUA_VERSION_RELEASE;
	log_info("lua", "got lua version %s, loading plugins ...", pLuaVersion);

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

void CLuaController::ReloadPlugins()
{
#ifdef CONF_LUA
	std::vector<std::string> vRconCommands;
	log_info("lua", "reloading %" PRIzu " plugins ..", m_vpPlugins.size());
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		// log_info("lua", "plugin='%s' has %" PRIzu " rcon commands", pPlugin->Name(), pPlugin->m_RconCommands.size());
		for(const auto &It : pPlugin->m_RconCommands)
			vRconCommands.emplace_back(It.second.Name());
	}

	// TODO: call some before and after reload hooks here for the plugins
	//       so they can cleanup and or persist state across reloads
	m_vpPlugins.clear();

	GameServer()->Storage()->ListDirectory(IStorage::TYPE_ALL, "plugins", FsListPluginCallback, this);
	OnInit();

	if(!vRconCommands.empty())
	{
		// first I thought we can only delete the commands that were removed
		// but then we don't properly diff the arguments and helptext
		// so we have to resend all

		// log_info("lua", "%" PRIzu " rcon commands scheduled for deletion ...", vRconCommands.size());
		for(CPlayer *pPlayer : m_pGameServer->m_apPlayers)
		{
			if(!pPlayer)
				continue;

			// TODO: if we reload too fast and another remove was already pending
			//       some rcon commands can be missed during removal

			CLuaPlayerState &State = m_aPlayers[pPlayer->GetCid()];
			State.m_RconSender.m_vRemoveCmds.insert(
				State.m_RconSender.m_vRemoveCmds.end(),
				vRconCommands.begin(),
				vRconCommands.end());
			State.m_RconSender.m_RemoveIndex = 0;
		}
	}
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
	for(const CPlayer *pPlayer : m_pGameServer->m_apPlayers)
	{
		if(!pPlayer)
			continue;

		SendNextRconCmd(pPlayer->GetCid());
	}
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

bool CLuaController::OnRconCommand(int ClientId, const char *pCommand, const char *pArguments)
{
#ifdef CONF_LUA

	CLuaPlugin *pPlugin = FindPluginThatKnowsRconCommand(pCommand);
	if(!pPlugin)
		return false;

	log_info(
		"server",
		"ClientId=%d key='%s' lua plugin '%s' got rcon command='%s' args='%s'",
		ClientId,
		GameServer()->Server()->GetAuthName(ClientId),
		pPlugin->Name(),
		pCommand,
		pArguments);

	pPlugin->OnRconCommand(ClientId, pCommand, pArguments);
	return true;
#endif
}

void CLuaController::OnSetAuthed(int ClientId, int Level)
{
#ifdef CONF_LUA
	// ignore Level because the server api is more stable
	if(m_Game.Server()->IsRconAuthed(ClientId))
	{
		CLuaPlayerState &State = m_aPlayers[ClientId];

		// TODO: does the client clear the command completions on logout?
		//       what if someone reloggs do we mess up state here?
		State.m_RconSender.m_SendIndex = std::nullopt;
		State.m_RconSender.m_vMissingCmds.clear();
		State.m_RconSender.m_vMissingCmdsNext.clear();

		for(CLuaPlugin *pPlugin : m_vpPlugins)
		{
			if(!pPlugin->IsActive())
				continue;

			for(const auto &It : pPlugin->m_RconCommands)
				State.m_RconSender.m_vMissingCmds.emplace_back(It.second);
		}
	}

	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		pPlugin->OnSetAuthed(ClientId, Level);
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

CLuaPlugin *CLuaController::FindPluginThatKnowsRconCommand(const char *pCommand)
{
#ifdef CONF_LUA
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		if(pPlugin->IsRconCmdKnown(pCommand))
			return pPlugin;
	}
	return nullptr;
#else
	return nullptr;
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
