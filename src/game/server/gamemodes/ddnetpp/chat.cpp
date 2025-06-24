#include <engine/shared/config.h>

#include "ddnetpp.h"

bool CGameControllerDDNetPP::OnChatMessage(const CNetMsg_Cl_Say *pMsg, int Length, int &Team, CPlayer *pPlayer)
{
	int ClientId = pPlayer->GetCid();
	if(pMsg->m_pMessage[0] == '/')
	{
		if(GameServer()->IsDDPPChatCommand(ClientId, pPlayer, pMsg->m_pMessage + 1))
			return true;
		return false;
	}

	if(GameServer()->IsChatMessageBlocked(ClientId, pPlayer, Team, pMsg->m_pMessage))
		return true;

	char aBuf[2048];
	if(g_Config.m_SvChatDiscordWebhook[0])
	{
		str_format(aBuf, sizeof(aBuf), "%s: %s", Server()->ClientName(ClientId), pMsg->m_pMessage);
		// dbg_msg("discord-chat", "sending to %s: %s", g_Config.m_SvChatDiscordWebhook, aBuf);
		GameServer()->SendDiscordWebhook(g_Config.m_SvChatDiscordWebhook, aBuf);
	}

	for(CPlayer *pPingedPlayer : GameServer()->m_apPlayers)
	{
		if(!pPingedPlayer)
			continue;
		if(!str_find_nocase(pMsg->m_pMessage, Server()->ClientName(pPingedPlayer->GetCid())))
			continue;

		pPingedPlayer->m_ReceivedChatPings++;
	}

	return false;
}
