#include "ddnetpp.h"

bool CGameControllerDDNetPP::OnChatMessage(const CNetMsg_Cl_Say *pMsg, int Length, int &Team, CPlayer *pPlayer)
{
	int ClientId = pPlayer->GetCid();
	if(pMsg->m_pMessage[0] == '/')
	{
		if(GameServer()->IsDDPPChatCommand(ClientId, pPlayer, pMsg->m_pMessage + 1))
			return true;
	}
	return false;
}
