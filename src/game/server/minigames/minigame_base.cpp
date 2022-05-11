// DDNet++ minigame base

#include <game/server/gamecontext.h>

#include "minigame_base.h"

CMinigame::CMinigame(CGameContext *pGameContext)
{
	m_pGameServer = pGameContext;
	m_State = 0;
	for(auto &SavePos : m_apSavedPositions)
		SavePos = nullptr;
	for(auto &RestorePos : m_aRestorePos)
		RestorePos = false;
}

CGameContext *CMinigame::GameServer()
{
	return m_pGameServer;
}

IServer *CMinigame::Server()
{
	return GameServer()->Server();
}

void CMinigame::SavePosition(CPlayer *pPlayer)
{
	if(!pPlayer)
		return;

	if(m_apSavedPositions[pPlayer->GetCID()])
		delete m_apSavedPositions[pPlayer->GetCID()];
	m_apSavedPositions[pPlayer->GetCID()] = nullptr;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	m_apSavedPositions[pPlayer->GetCID()] = new CSaveTee();
	m_apSavedPositions[pPlayer->GetCID()]->Save(pChr);
}

void CMinigame::LoadPosition(CCharacter *pChr)
{
	if(!pChr)
		return;
	CPlayer *pPlayer = pChr->GetPlayer();
	if(!pPlayer)
		return;
	if(!m_apSavedPositions[pPlayer->GetCID()])
		return;
	if(!m_aRestorePos[pPlayer->GetCID()])
		return;

	m_aRestorePos[pPlayer->GetCID()] = false;
	m_apSavedPositions[pPlayer->GetCID()]->Load(pChr, 0);
	delete m_apSavedPositions[pPlayer->GetCID()];
	m_apSavedPositions[pPlayer->GetCID()] = nullptr;
}

void CMinigame::SendChatAll(const char *pMessage)
{
	for(auto &Player : GameServer()->m_apPlayers)
		if(Player)
			if(IsActive(Player->GetCID()))
				GameServer()->SendChatTarget(Player->GetCID(), pMessage);
}

void CMinigame::SendBroadcastAll(const char *pMessage)
{
	for(auto &Player : GameServer()->m_apPlayers)
		if(Player)
			if(IsActive(Player->GetCID()))
				GameServer()->SendBroadcast(pMessage, Player->GetCID());
}
