// gamecontext scoped votes ddnet++ methods

#include "gamecontext.h"
#include "gamemodes/ddnet.h"

#include <engine/shared/config.h>

#include <game/server/ddpp/shop.h>
#include <game/server/entities/flag.h>

void CGameContext::VotedYes(CCharacter *pChr, CPlayer *pPlayer)
{
	// SendChatTarget(ClientId, "you pressed f3");
	if(!pChr)
		return;
	if(Shop()->VoteYes(pChr->GetPlayer()->GetCid()))
		return;

	IGameController *Controller = pPlayer->GetCharacter()->GameServer()->m_pController;
	if(Controller->CharacterDropFlag(pChr))
	{
		// SendChatTarget(ClientId, "you dropped the flag");
	}
}

void CGameContext::VotedNo(CCharacter *pChr)
{
	//SendChatTarget(ClientId, "you pressed f4");
	if(!pChr)
		return;
	if(Shop()->VoteNo(pChr->GetPlayer()->GetCid()))
		return;
	if(g_Config.m_SvAllowDroppingWeapons)
		pChr->DropWeapon(pChr->GetActiveWeapon()); // drop the weapon your holding.
}

void CGameContext::SendVoteStart(int ClientId, int VoteCreatorId, int Timeout, int Type, const char *pDescription, const char *pReason)
{
	::CNetMsg_Sv_VoteSet Msg6;
	protocol7::CNetMsg_Sv_VoteSet Msg7;

	Msg7.m_ClientId = VoteCreatorId;
	Msg6.m_Timeout = Msg7.m_Timeout = Timeout;
	Msg6.m_pDescription = Msg7.m_pDescription = pDescription;
	Msg6.m_pReason = Msg7.m_pReason = pReason;

	if(Type == VOTE_TYPE_OPTION)
		Msg7.m_Type = protocol7::VOTE_START_OP;
	else if(Type == VOTE_TYPE_SPECTATE)
		Msg7.m_Type = protocol7::VOTE_START_SPEC;
	else if(Type == VOTE_TYPE_KICK)
		Msg7.m_Type = protocol7::VOTE_START_SPEC;
	else
		dbg_assert_failed("Invalid vote Type: %d", Type);

	if(ClientId == -1)
	{
		for(int i = 0; i < Server()->MaxClients(); i++)
		{
			if(!m_apPlayers[i])
				continue;
			if(!Server()->IsSixup(i))
				Server()->SendPackMsg(&Msg6, MSGFLAG_VITAL, i);
			else
				Server()->SendPackMsg(&Msg7, MSGFLAG_VITAL, i);
		}
	}
	else
	{
		if(!Server()->IsSixup(ClientId))
			Server()->SendPackMsg(&Msg6, MSGFLAG_VITAL, ClientId);
		else
			Server()->SendPackMsg(&Msg7, MSGFLAG_VITAL, ClientId);
	}
}

void CGameContext::SendVoteEnd(int ClientId, int VoteCreatorId, int Enforce)
{
	::CNetMsg_Sv_VoteSet Msg6;
	protocol7::CNetMsg_Sv_VoteSet Msg7;

	Msg7.m_ClientId = VoteCreatorId;
	Msg6.m_Timeout = Msg7.m_Timeout = 0;
	Msg6.m_pDescription = Msg7.m_pDescription = "";
	Msg6.m_pReason = Msg7.m_pReason = "";

	if(Enforce == VOTE_ENFORCE_NO || Enforce == VOTE_ENFORCE_NO_ADMIN)
		Msg7.m_Type = protocol7::VOTE_END_FAIL;
	else if(Enforce == VOTE_ENFORCE_YES || Enforce == VOTE_ENFORCE_YES_ADMIN)
		Msg7.m_Type = protocol7::VOTE_END_PASS;
	else if(Enforce == VOTE_ENFORCE_ABORT || Enforce == VOTE_ENFORCE_CANCEL)
		Msg7.m_Type = protocol7::VOTE_END_ABORT;
	else
		Msg7.m_Type = protocol7::VOTE_UNKNOWN;

	if(Enforce == VOTE_ENFORCE_NO_ADMIN || Enforce == VOTE_ENFORCE_YES_ADMIN)
		Msg7.m_ClientId = -1;

	if(ClientId == -1)
	{
		for(int i = 0; i < Server()->MaxClients(); i++)
		{
			if(!m_apPlayers[i])
				continue;
			if(!Server()->IsSixup(i))
				Server()->SendPackMsg(&Msg6, MSGFLAG_VITAL, i);
			else
				Server()->SendPackMsg(&Msg7, MSGFLAG_VITAL, i);
		}
	}
	else
	{
		if(!Server()->IsSixup(ClientId))
			Server()->SendPackMsg(&Msg6, MSGFLAG_VITAL, ClientId);
		else
			Server()->SendPackMsg(&Msg7, MSGFLAG_VITAL, ClientId);
	}
}
