// gamecontext scoped vote menu ddnet++ methods

#include "gamecontext.h"

#include <base/system.h>

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

	// bool InsertBeforeRegularVotes = true;
	// if(pPlayer->m_SendVoteIndex < m_NumVoteOptions) // not all regular votes have been sent yet
	// {
	// 	if(InsertBeforeRegularVotes)
	// 		return false;
	// }

	if(pPlayer->m_SendExtraVoteMenuIndex > -1)
		return false;
	pPlayer->m_SendExtraVoteMenuIndex++;

	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "%s: %" PRId64, Loc("Money", ClientId), pPlayer->GetMoney());

	CNetMsg_Sv_VoteOptionAdd AddMsg;
	AddMsg.m_pDescription = pPlayer->IsLoggedIn() ? aBuf : "Write '/login' in chat to see stats here";
	Server()->SendPackMsg(&AddMsg, MSGFLAG_VITAL, ClientId);

	return true;
}
