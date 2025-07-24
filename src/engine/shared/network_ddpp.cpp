/* network.h scoped ddnet++ methods */
#include <base/hash_ctxt.h>
#include <base/system.h>

#include <engine/console.h>

#include "network.h"
#include <engine/message.h>
#include <engine/shared/protocol.h>
#include <game/generated/protocol.h>

void CNetServer::BotInit(int BotId)
{
	m_aSlots[BotId].m_Connection.BotConnect();
}

void CNetServer::BotDelete(int BotId)
{
	m_aSlots[BotId].m_Connection.BotDrop();
}

void CNetConnection::BotConnect()
{
	m_State = EState::BOT;
}

void CNetConnection::BotDrop()
{
	m_State = EState::OFFLINE;
}
