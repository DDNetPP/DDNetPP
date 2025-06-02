// DDNet++ minigame base

#include <base/system.h>
#include <base/vmath.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/minigames/save_ddpp.h>

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

IGameController *CMinigame::Controller()
{
	return GameServer()->m_pController;
}

void CMinigame::SendChatTarget(int To, const char *pText, int VersionFlags)
{
	if(VersionFlags == -1)
		VersionFlags = CGameContext::FLAG_SIX | CGameContext::FLAG_SIXUP;
	GameServer()->SendChatTarget(To, pText, VersionFlags);
}

void CMinigame::CleanupMinigame()
{
	for(auto &SavePos : m_apSavedPositions)
		delete SavePos;
	for(auto &SavePos : m_apSavedPositionsDDPP)
		delete SavePos;
}

bool CMinigame::GetSavedPosition(CPlayer *pPlayer, vec2 *pPos)
{
	*pPos = vec2(-1, -1);
	if(!pPlayer)
		return false;
	if(!m_apSavedPositions[pPlayer->GetCid()])
		return false;

	*pPos = m_apSavedPositions[pPlayer->GetCid()]->GetPos();
	return true;
}

void CMinigame::ClearSavedPosition(CPlayer *pPlayer)
{
	if(!pPlayer)
		return;

	// wipe ddnet state
	delete m_apSavedPositions[pPlayer->GetCid()];
	m_apSavedPositions[pPlayer->GetCid()] = nullptr;

	// wipe ddnet++ state
	delete m_apSavedPositionsDDPP[pPlayer->GetCid()];
	m_apSavedPositionsDDPP[pPlayer->GetCid()] = nullptr;

	m_aRestorePos[pPlayer->GetCid()] = false;
}

void CMinigame::SavePosition(CPlayer *pPlayer)
{
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	ClearSavedPosition(pPlayer);

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

	m_apSavedPositions[pPlayer->GetCid()]->Load(pChr, 0);
	m_apSavedPositionsDDPP[pPlayer->GetCid()]->Load(pChr);
	ClearSavedPosition(pPlayer);
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
