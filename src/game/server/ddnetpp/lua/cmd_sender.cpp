#include <game/server/ddnetpp/lua/cmd_sender.h>
#include <game/server/ddnetpp/lua/lua_rcon_command.h>

#ifdef CONF_LUA

void CCmdSender::AddRconCmd(const CLuaRconCommand *pCmd)
{
	if(!m_SendRconIndex.has_value())
	{
		m_SendRconIndex = 0;
	}

	// we already started a send group
	// so queue the cmd for the next group
	if(m_SendRconIndex.value() > 0)
	{
		m_vMissingRconNext.emplace_back(*pCmd);
		return;
	}

	m_vMissingRcon.emplace_back(*pCmd);
}

void CCmdSender::AddChatCmd(const CLuaRconCommand *pCmd)
{
	if(!m_SendChatIndex.has_value())
	{
		m_SendChatIndex = 0;
	}

	// we already started a send group
	// so queue the cmd for the next group
	if(m_SendChatIndex.value() > 0)
	{
		m_vMissingChatNext.emplace_back(*pCmd);
		return;
	}

	m_vMissingChat.emplace_back(*pCmd);
}

#endif
