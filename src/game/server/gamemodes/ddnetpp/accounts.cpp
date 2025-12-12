#include <game/server/gamemodes/ddnetpp/ddnetpp.h>

void CGameControllerDDNetPP::LogoutAllAccounts()
{
	for(CPlayer *pPlayer : GameServer()->m_apPlayers)
	{
		if(!pPlayer)
			continue;
		if(!pPlayer->IsLoggedIn())
			continue;

		GameServer()->SendChatTarget(pPlayer->GetCid(), "[ACCOUNT] you have been logged out due to changes in the system");
		pPlayer->Logout();
	}
}
