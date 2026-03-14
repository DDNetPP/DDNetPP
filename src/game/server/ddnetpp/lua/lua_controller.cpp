#include "lua_controller.h"

#include <base/dbg.h>
#include <base/log.h>
#include <base/str.h>
#include <base/types.h>

#include <engine/server/server.h>
#include <engine/shared/protocol.h>

#include <generated/protocol.h>

#include <game/server/ddnetpp/lua/console_strings.h>
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
		State.m_CmdSender.AddRconCmd(pCmd);
	}
#endif
}

void CLuaController::OnAddChatCmd(const CLuaRconCommand *pCmd)
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
		State.m_CmdSender.AddChatCmd(pCmd);
	}
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

bool CLuaController::SendNextRemRcon(int ClientId)
{
#ifdef CONF_LUA
	CCmdSender *pSender = &m_aPlayers[ClientId].m_CmdSender;
	if(!pSender->m_RemoveRconIndex.has_value())
		return false;

	size_t NumSendMax = 3;
	size_t FromIdx = pSender->m_RemoveRconIndex.value();
	size_t ToIdx = std::min(pSender->m_vRemoveRcon.size() - 1, FromIdx + NumSendMax);
	pSender->m_RemoveRconIndex = ToIdx;

	// log_info(
	// 	"lua",
	// 	"sending removal commands from %" PRIzu " to %" PRIzu " to cid=%d",
	// 	FromIdx,
	// 	ToIdx,
	// 	ClientId);

	for(size_t i = FromIdx; i <= ToIdx; i++)
	{
		const char *pCmd = pSender->m_vRemoveRcon.at(i).c_str();
		m_Game.SendRconCmdRem(ClientId, pCmd);

		// log_info(
		// 	"lua",
		// 	" sending removal cmd='%s' %" PRIzu "/%" PRIzu " to cid=%d",
		// 	pCmd,
		// 	i,
		// 	pSender->m_vRemoveRcon.size(),
		// 	ClientId);
	}

	if(pSender->m_RemoveRconIndex.value() == (pSender->m_vRemoveRcon.size() - 1))
	{
		// log_info("lua", "done sending all removals to cid=%d", ClientId);
		pSender->m_RemoveRconIndex = std::nullopt;
		pSender->m_vRemoveRcon.clear();
	}

	return true;
#else
	return false;
#endif
}

bool CLuaController::SendNextAddRcon(int ClientId)
{
#ifdef CONF_LUA
	CCmdSender *pSender = &m_aPlayers[ClientId].m_CmdSender;
	if(pSender->m_SendRconIndex.has_value())
	{
		dbg_assert(
			pSender->m_SendRconIndex < pSender->m_vMissingRcon.size(),
			"rcon cmd send index=%" PRIzu " too big for cmds=%" PRIzu " cid=%d",
			pSender->m_SendRconIndex.value(),
			pSender->m_vMissingRcon.size(),
			ClientId);
	}

	if(!pSender->m_SendRconIndex.has_value())
	{
		if(pSender->m_vMissingRcon.empty())
		{
			// nothing to send
			if(pSender->m_vMissingRconNext.empty())
				return false;

			// some weird edge case where we have no send index but more commands??
			log_error("lua", "THIS IS WEIRD!");
			std::swap(pSender->m_vMissingRcon, pSender->m_vMissingRconNext);
			pSender->m_vMissingRconNext.clear();
		}

		// start new send
		pSender->m_SendRconIndex = 0;
		// log_info(
		// 	"lua",
		// 	"starting new rcon cmd send group of %" PRIzu " commands for cid=%d",
		// 	pSender->m_vMissingRcon.size(),
		// 	ClientId);

		m_Game.SendRconCmdGroupStart(ClientId, pSender->m_vMissingRcon.size());
	}

	size_t NumSendMax = 3;
	size_t FromIdx = pSender->m_SendRconIndex.value();
	size_t ToIdx = std::min(pSender->m_vMissingRcon.size() - 1, FromIdx + NumSendMax);
	pSender->m_SendRconIndex = ToIdx;

	// log_info(
	// 	"lua",
	// 	"sending commands from %" PRIzu " to %" PRIzu " to cid=%d",
	// 	FromIdx,
	// 	ToIdx,
	// 	ClientId);

	for(size_t i = FromIdx; i <= ToIdx; i++)
	{
		CLuaRconCommand *pCmd = &pSender->m_vMissingRcon.at(i);
		m_Game.SendRconCmdAdd(ClientId, pCmd);

		// log_info(
		// 	"lua",
		// 	" sending cmd='%s' %" PRIzu "/%" PRIzu " to cid=%d",
		// 	pCmd->Name(),
		// 	i,
		// 	pSender->m_vMissingRcon.size(),
		// 	ClientId);
	}

	// reached the end of the current group
	if(pSender->m_SendRconIndex.value() >= (pSender->m_vMissingRcon.size() - 1))
	{
		// log_info(
		// 	"lua",
		// 	"finished sending %" PRIzu " rcon commands to cid=%d",
		// 	pSender->m_vMissingRcon.size(),
		// 	ClientId);

		std::swap(pSender->m_vMissingRcon, pSender->m_vMissingRconNext);
		pSender->m_vMissingRconNext.clear();
		pSender->m_SendRconIndex = std::nullopt;

		// log_info(
		// 	"lua",
		// 	" there were %" PRIzu " new rcon commands queued while sending.",
		// 	pSender->m_vMissingRcon.size());

		m_Game.SendRconCmdGroupEnd(ClientId);

		// we do not instantly start the next group in this tick
		// that happens next tick
		return false;
	}
	return false;
#else
	return false;
#endif
}

bool CLuaController::SendNextRemChatCmd(int ClientId)
{
#ifdef CONF_LUA
	CCmdSender *pSender = &m_aPlayers[ClientId].m_CmdSender;
	if(!pSender->m_RemoveChatIndex.has_value())
		return false;

	size_t NumSendMax = 3;
	size_t FromIdx = pSender->m_RemoveChatIndex.value();
	size_t ToIdx = std::min(pSender->m_vRemoveChat.size() - 1, FromIdx + NumSendMax);
	pSender->m_RemoveChatIndex = ToIdx;

	// log_info(
	// 	"lua",
	// 	"sending removal commands from %" PRIzu " to %" PRIzu " to cid=%d",
	// 	FromIdx,
	// 	ToIdx,
	// 	ClientId);

	for(size_t i = FromIdx; i <= ToIdx; i++)
	{
		const char *pCmd = pSender->m_vRemoveChat.at(i).c_str();
		m_Game.SendChatCmdRem(ClientId, pCmd);

		// log_info(
		// 	"lua",
		// 	" sending removal cmd='%s' %" PRIzu "/%" PRIzu " to cid=%d",
		// 	pCmd,
		// 	i,
		// 	pSender->m_vRemoveChat.size(),
		// 	ClientId);
	}

	if(pSender->m_RemoveChatIndex.value() == (pSender->m_vRemoveChat.size() - 1))
	{
		// log_info("lua", "done sending all removals to cid=%d", ClientId);
		pSender->m_RemoveChatIndex = std::nullopt;
		pSender->m_vRemoveChat.clear();
	}

	return true;
#else
	return false;
#endif
}

bool CLuaController::SendNextAddChatCmd(int ClientId)
{
#ifdef CONF_LUA
	CCmdSender *pSender = &m_aPlayers[ClientId].m_CmdSender;
	if(pSender->m_SendChatIndex.has_value())
	{
		dbg_assert(
			pSender->m_SendChatIndex < pSender->m_vMissingChat.size(),
			"rcon cmd send index=%" PRIzu " too big for cmds=%" PRIzu " cid=%d",
			pSender->m_SendChatIndex.value(),
			pSender->m_vMissingChat.size(),
			ClientId);
	}

	if(!pSender->m_SendChatIndex.has_value())
	{
		if(pSender->m_vMissingChat.empty())
		{
			// nothing to send
			if(pSender->m_vMissingChatNext.empty())
				return false;

			// some weird edge case where we have no send index but more commands??
			log_error("lua", "THIS IS WEIRD!");
			std::swap(pSender->m_vMissingChat, pSender->m_vMissingChatNext);
			pSender->m_vMissingChatNext.clear();
		}

		// start new send
		pSender->m_SendChatIndex = 0;
		// log_info(
		// 	"lua",
		// 	"starting new rcon cmd send group of %" PRIzu " commands for cid=%d",
		// 	pSender->m_vMissingChat.size(),
		// 	ClientId);

		m_Game.SendChatCmdGroupStart(ClientId, pSender->m_vMissingChat.size());
	}

	size_t NumSendMax = 3;
	size_t FromIdx = pSender->m_SendChatIndex.value();
	size_t ToIdx = std::min(pSender->m_vMissingChat.size() - 1, FromIdx + NumSendMax);
	pSender->m_SendChatIndex = ToIdx;

	// log_info(
	// 	"lua",
	// 	"sending commands from %" PRIzu " to %" PRIzu " to cid=%d",
	// 	FromIdx,
	// 	ToIdx,
	// 	ClientId);

	for(size_t i = FromIdx; i <= ToIdx; i++)
	{
		CLuaRconCommand *pCmd = &pSender->m_vMissingChat.at(i);
		m_Game.SendChatCmdAdd(ClientId, pCmd);

		// log_info(
		// 	"lua",
		// 	" sending cmd='%s' %" PRIzu "/%" PRIzu " to cid=%d",
		// 	pCmd->Name(),
		// 	i,
		// 	pSender->m_vMissingChat.size(),
		// 	ClientId);
	}

	// reached the end of the current group
	if(pSender->m_SendChatIndex.value() >= (pSender->m_vMissingChat.size() - 1))
	{
		// log_info(
		// 	"lua",
		// 	"finished sending %" PRIzu " rcon commands to cid=%d",
		// 	pSender->m_vMissingChat.size(),
		// 	ClientId);

		std::swap(pSender->m_vMissingChat, pSender->m_vMissingChatNext);
		pSender->m_vMissingChatNext.clear();
		pSender->m_SendChatIndex = std::nullopt;

		// log_info(
		// 	"lua",
		// 	" there were %" PRIzu " new rcon commands queued while sending.",
		// 	pSender->m_vMissingChat.size());

		m_Game.SendChatCmdGroupEnd(ClientId);

		// we do not instantly start the next group in this tick
		// that happens next tick
		return false;
	}
	return false;
#else
	return false;
#endif
}

bool CLuaController::SendNextCmdInfo(int ClientId)
{
#ifdef CONF_LUA
	if(IsServerSendingNativeRconCmds(ClientId))
	{
		// log_info("lua", "waiting for server to finish sending rcon commands ...");
		return false;
	}

	// send pending removals before adds
	if(SendNextRemRcon(ClientId))
		return true;

	if(SendNextAddRcon(ClientId))
		return true;

	if(SendNextRemChatCmd(ClientId))
		return true;

	if(SendNextAddChatCmd(ClientId))
		return true;

	return false;
#else
	return false;
#endif
}

CLuaPlugin *CLuaController::RunChatCommand(int ClientId, const char *pFullCmd)
{
#ifdef CONF_LUA
	const char *pArguments = "";
	char aCommand[2048];
	int i;
	for(i = 0; pFullCmd[i]; i++)
	{
		if(pFullCmd[i] == ' ')
		{
			pArguments = str_skip_whitespaces_const(pFullCmd + i);
			break;
		}
		aCommand[i] = pFullCmd[i];
	}
	aCommand[i] = '\0';
	CLuaPlugin *pPlugin = FindPluginThatKnowsChatCommand(aCommand);
	if(!pPlugin)
		return nullptr;
	log_info(
		"server",
		"ClientId=%d lua plugin '%s' got chat command='%s' args='%s'",
		ClientId,
		pPlugin->Name(),
		aCommand,
		pArguments);
	pPlugin->OnChatCommand(ClientId, aCommand, pArguments);
	return pPlugin;
#else
	return nullptr;
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
	std::vector<std::string> vChatCommands;
	log_info("lua", "reloading %" PRIzu " plugins ..", m_vpPlugins.size());
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		// log_info("lua", "plugin='%s' has %" PRIzu " rcon commands", pPlugin->Name(), pPlugin->m_RconCommands.size());
		for(const auto &It : pPlugin->m_RconCommands)
			vRconCommands.emplace_back(It.second.Name());
		for(const auto &It : pPlugin->m_ChatCommands)
			vChatCommands.emplace_back(It.second.Name());

		delete pPlugin;
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
			State.m_CmdSender.m_vRemoveRcon.insert(
				State.m_CmdSender.m_vRemoveRcon.end(),
				vRconCommands.begin(),
				vRconCommands.end());
			State.m_CmdSender.m_RemoveRconIndex = 0;
		}
	}

	if(!vChatCommands.empty())
	{
		// first I thought we can only delete the commands that were removed
		// but then we don't properly diff the arguments and helptext
		// so we have to resend all

		// log_info("lua", "%" PRIzu " rcon commands scheduled for deletion ...", vChatCommands.size());
		for(CPlayer *pPlayer : m_pGameServer->m_apPlayers)
		{
			if(!pPlayer)
				continue;

			// TODO: if we reload too fast and another remove was already pending
			//       some rcon commands can be missed during removal

			CLuaPlayerState &State = m_aPlayers[pPlayer->GetCid()];
			State.m_CmdSender.m_vRemoveChat.insert(
				State.m_CmdSender.m_vRemoveChat.end(),
				vChatCommands.begin(),
				vChatCommands.end());
			State.m_CmdSender.m_RemoveChatIndex = 0;
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

		SendNextCmdInfo(pPlayer->GetCid());
	}
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		pPlugin->OnTick();
	}
#endif
}

void CLuaController::OnSnap()
{
#ifdef CONF_LUA
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		pPlugin->OnSnap();
	}
#endif
}

bool CLuaController::OnChatMessage(int ClientId, CNetMsg_Cl_Say *pMsg, int &Team)
{
#ifdef CONF_LUA
	// intentionally process the chat event first
	// it can alter the chat message
	// so that should also affect the chat command
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		if(pPlugin->OnChatMessage(ClientId, pMsg, Team))
			return true;
	}

	bool IsLuaChatCmd = false;
	char aLineWithoutLua[2048] = "";

	if(pMsg->m_pMessage[0] == '/' && pMsg->m_pMessage[1])
	{
		const char *pFullCmd = pMsg->m_pMessage + 1;
		if(!str_startswith(pFullCmd, "mc;"))
		{
			if(RunChatCommand(ClientId, pFullCmd))
			{
				IsLuaChatCmd = true;
			}
		}
		else
		{
			pFullCmd += str_length("mc;");

			const char *apStmts[1024] = {};
			char aError[512] = "";
			size_t NumStmts = 0;
			char aLine[2048];
			str_copy(aLine, pFullCmd);
			bool Ok = SplitConsoleStatements(apStmts, (sizeof(apStmts) / sizeof(apStmts[0])), &NumStmts, aLine, aError, sizeof(aError));
			if(!Ok)
			{
				log_error("lua", "statement splitter failed with error: %s", aError);
				return false;
			}

			str_copy(aLineWithoutLua, "/mc;");
			bool AddSemi = false;
			for(const char *pStmt : apStmts)
			{
				if(!pStmt)
					break;
				if(AddSemi)
				{
					str_append(aLineWithoutLua, ";");
					AddSemi = false;
				}

				if(RunChatCommand(ClientId, pStmt))
				{
					IsLuaChatCmd = true;
				}
				else
				{
					str_append(aLineWithoutLua, pStmt);
					AddSemi = true;
				}
			}
		}
	}

	if(IsLuaChatCmd)
	{
		// line was just a lua command so we drop it
		// otherwise the server says "no such command"
		// this happens if the chat message is something like "/luacommand"
		if(aLineWithoutLua[0] == '\0')
			return true;
		// line was just lua commands something like "/mc;luacmd1;luacmd2"
		if(!str_comp(aLineWithoutLua, "/mc;"))
			return true;

		dbg_assert(str_startswith(aLineWithoutLua, "/mc;"), "unexpected line without lua commands: '%s'", aLineWithoutLua);
		dbg_assert(aLineWithoutLua[str_length("/mc;")] != '\0', "unexpected line without lua commands: '%s'", aLineWithoutLua);

		// line was mixed with lua and non lua commands
		// in that case we edit the message to filter out the lua commands

		// oh boi memory management at its finest
		static char s_aMsgBuffer[2048];
		str_copy(s_aMsgBuffer, aLineWithoutLua);
		pMsg->m_pMessage = s_aMsgBuffer;
	}

	return false;
#else
	return false;
#endif
}

void CLuaController::OnPlayerConnect(int ClientId)
{
#ifdef CONF_LUA
	CLuaPlayerState &State = m_aPlayers[ClientId];
	State.m_CmdSender.m_SendChatIndex = std::nullopt;
	State.m_CmdSender.m_vMissingChat.clear();
	State.m_CmdSender.m_vMissingChatNext.clear();

	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		pPlugin->OnPlayerConnect(ClientId);

		for(const auto &It : pPlugin->m_ChatCommands)
			State.m_CmdSender.m_vMissingChat.emplace_back(It.second);
	}
#endif
}

void CLuaController::OnPlayerDisconnect(int ClientId)
{
#ifdef CONF_LUA
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		pPlugin->OnPlayerDisconnect(ClientId);
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
#else
	return false;
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
		State.m_CmdSender.m_SendRconIndex = std::nullopt;
		State.m_CmdSender.m_vMissingRcon.clear();
		State.m_CmdSender.m_vMissingRconNext.clear();

		for(CLuaPlugin *pPlugin : m_vpPlugins)
		{
			if(!pPlugin->IsActive())
				continue;

			for(const auto &It : pPlugin->m_RconCommands)
				State.m_CmdSender.m_vMissingRcon.emplace_back(It.second);
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

bool CLuaController::OnClientMessage(int ClientId, const void *pData, int Size, int Flags)
{
#ifdef CONF_LUA
	// TODO: implement
	return false;
#else
	return false;
#endif
}

bool CLuaController::OnServerMessage(int ClientId, const void *pData, int Size, int Flags)
{
#ifdef CONF_LUA
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		if(pPlugin->OnServerMessage(ClientId, pData, Size, Flags))
			return true;
	}
	return false;
#else
	return false;
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

CLuaPlugin *CLuaController::FindPluginThatKnowsChatCommand(const char *pCommand)
{
#ifdef CONF_LUA
	for(CLuaPlugin *pPlugin : m_vpPlugins)
	{
		if(!pPlugin->IsActive())
			continue;

		if(pPlugin->IsChatCmdKnown(pCommand))
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

ILuaController *CreateLuaController()
{
	return new CLuaController();
}
