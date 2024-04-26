// DDNet++ minigame base

#include <game/server/gamecontext.h>

#include "save_ddpp.h"

#include "minigame_base.h"

CMinigame::CMinigame(CGameContext *pGameContext)
{
	m_pGameServer = pGameContext;
	m_State = 0;
	for(auto &SavePos : m_apSavedPositions)
		SavePos = nullptr;
	for(auto &SavePos : m_apSavedPositionsDDPP)
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

CCollision *CMinigame::Collision()
{
	return GameServer()->Collision();
}

void CMinigame::CleanupMinigame()
{
	for(auto &SavePos : m_apSavedPositions)
		delete SavePos;
	for(auto &SavePos : m_apSavedPositionsDDPP)
		delete SavePos;
}

void CMinigame::SavePosition(CPlayer *pPlayer)
{
	if(!pPlayer)
		return;

	if(m_apSavedPositions[pPlayer->GetCid()])
		delete m_apSavedPositions[pPlayer->GetCid()];
	m_apSavedPositions[pPlayer->GetCid()] = nullptr;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	// save ddnet state
	m_apSavedPositions[pPlayer->GetCid()] = new CSaveTee();
	m_apSavedPositions[pPlayer->GetCid()]->Save(pChr);

	// save ddnet++ state
	m_apSavedPositionsDDPP[pPlayer->GetCid()] = new CSaveTeeDDPP();
	m_apSavedPositionsDDPP[pPlayer->GetCid()]->Save(pChr);
}

void CMinigame::LoadPosition(CCharacter *pChr)
{
	if(!pChr)
		return;
	CPlayer *pPlayer = pChr->GetPlayer();
	if(!pPlayer)
		return;
	if(!m_apSavedPositions[pPlayer->GetCid()])
		return;
	if(!m_apSavedPositionsDDPP[pPlayer->GetCid()])
		return;
	if(!m_aRestorePos[pPlayer->GetCid()])
		return;

	m_aRestorePos[pPlayer->GetCid()] = false;

	// restore ddnet state
	m_apSavedPositions[pPlayer->GetCid()]->Load(pChr, 0);
	delete m_apSavedPositions[pPlayer->GetCid()];
	m_apSavedPositions[pPlayer->GetCid()] = nullptr;

	// restore ddnet++ state
	m_apSavedPositionsDDPP[pPlayer->GetCid()]->Load(pChr);
	delete m_apSavedPositionsDDPP[pPlayer->GetCid()];
	m_apSavedPositionsDDPP[pPlayer->GetCid()] = nullptr;
}

void CMinigame::SendChatAll(const char *pMessage)
{
	for(auto &Player : GameServer()->m_apPlayers)
		if(Player)
			if(IsActive(Player->GetCid()))
				GameServer()->SendChatTarget(Player->GetCid(), pMessage);
}

void CMinigame::SendBroadcastAll(const char *pMessage)
{
	for(auto &Player : GameServer()->m_apPlayers)
		if(Player)
			if(IsActive(Player->GetCid()))
				GameServer()->SendBroadcast(pMessage, Player->GetCid());
}
