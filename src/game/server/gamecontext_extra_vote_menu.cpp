// gamecontext scoped vote menu ddnet++ methods

#include "gamecontext.h"

#include <engine/shared/config.h>
#include <engine/shared/protocol.h>

// TODO: this should be a highly configurable system like the shop
//       where admins can remove and add entries they want to display
//       also there should probably be a way to turn the entire thing off
//       and if no entries are being displayed the refresh should do nothing

void CGameContext::RefreshExtraVoteMenu(int ClientId)
{
	// TODO: think about this longer than 0 seconds
	//       also refresh should be a flag and only happen once a tick max
	//       not happen directly

	if(ClientId < 0 || ClientId >= MAX_CLIENTS)
		return;

	CPlayer *pPlayer = m_apPlayers[ClientId];
	if(!pPlayer)
		return;

	// TODO: is it weird that the regular is 0 and the extra is -1?
	//       we use the regular as a guard for the extra so maybe the extra could
	//       start at 0 at all times
	pPlayer->m_SendVoteIndex = 0;
	pPlayer->m_SendExtraVoteMenuIndex = -1;

	CNetMsg_Sv_VoteClearOptions VoteClearOptionsMsg;
	Server()->SendPackMsg(&VoteClearOptionsMsg, MSGFLAG_VITAL, ClientId);
}

bool CGameContext::SendExtraVoteMenuEntry(int ClientId)
{
	if(ClientId < 0 || ClientId >= MAX_CLIENTS)
		return false;

	CPlayer *pPlayer = m_apPlayers[ClientId];
	if(!pPlayer)
		return false;

	if(pPlayer->m_SendVoteIndex == -1)
		return false; // we didn't start sending options yet

	if(pPlayer->m_SendExtraVoteMenuIndex >= 5) // Limit to 5 menu entries
		return false;

	pPlayer->m_SendExtraVoteMenuIndex++;

	char aBuf[512];
	CNetMsg_Sv_VoteOptionAdd AddMsg;

	// Menu entries based on index
	switch(pPlayer->m_SendExtraVoteMenuIndex)
	{
	case 0:
		// Money display
		if(pPlayer->IsLoggedIn())
		{
			str_format(aBuf, sizeof(aBuf), "💰 %s: %" PRId64, Loc("Money", ClientId), pPlayer->GetMoney());
			AddMsg.m_pDescription = aBuf;
		}
		else
		{
			AddMsg.m_pDescription = "📝 Write '/login' in chat to see your stats";
		}
		break;

	case 1:
		// VIP Status
		if(pPlayer->IsLoggedIn())
		{
			if(pPlayer->IsVip())
			{
				str_format(aBuf, sizeof(aBuf), "⭐ VIP Status: Active");
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "⭐ VIP Status: None");
			}
			AddMsg.m_pDescription = aBuf;
		}
		else
		{
			AddMsg.m_pDescription = "⭐ VIP: Login to see status";
		}
		break;

	case 2:
		// Level/XP
		if(pPlayer->IsLoggedIn())
		{
			str_format(aBuf, sizeof(aBuf), "📊 Level: %d | XP: %d", pPlayer->m_Account.m_Level, pPlayer->m_Account.m_Xp);
			AddMsg.m_pDescription = aBuf;
		}
		else
		{
			AddMsg.m_pDescription = "📊 Level: Login to see stats";
		}
		break;

	case 3:
		// Quick commands hint
		AddMsg.m_pDescription = "💡 Commands: /shop /profile /help";
		break;

	case 4:
		// Server info
		str_format(aBuf, sizeof(aBuf), "🌐 Players online: %d/%d", Server()->NumPlayers(), Server()->MaxClients());
		AddMsg.m_pDescription = aBuf;
		break;

	default:
		return false;
	}

	Server()->SendPackMsg(&AddMsg, MSGFLAG_VITAL, ClientId);
	return true;
}
