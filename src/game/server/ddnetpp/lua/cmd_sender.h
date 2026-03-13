#ifndef GAME_SERVER_DDNETPP_LUA_CMD_SENDER_H
#define GAME_SERVER_DDNETPP_LUA_CMD_SENDER_H

#ifdef CONF_LUA

#include <game/server/ddnetpp/lua/lua_rcon_command.h>

#include <cstddef>
#include <optional>
#include <string>

class CCmdSender
{
public:
	std::optional<size_t> m_SendRconIndex = std::nullopt;
	std::vector<CLuaRconCommand> m_vMissingRcon;
	std::vector<CLuaRconCommand> m_vMissingRconNext;
	void AddRconCmd(const CLuaRconCommand *pCmd);

	std::optional<size_t> m_RemoveRconIndex = std::nullopt;
	std::vector<std::string> m_vRemoveRcon;

	std::optional<size_t> m_SendChatIndex = std::nullopt;
	std::vector<CLuaRconCommand> m_vMissingChat;
	std::vector<CLuaRconCommand> m_vMissingChatNext;
	void AddChatCmd(const CLuaRconCommand *pCmd);

	std::optional<size_t> m_RemoveChatIndex = std::nullopt;
	std::vector<std::string> m_vRemoveChat;
};

#endif // CONF_LUA

#endif // header guard
