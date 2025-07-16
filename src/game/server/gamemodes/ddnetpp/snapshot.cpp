#include <base/log.h>
#include <engine/shared/config.h>
#include <engine/shared/protocol.h>
#include <game/server/ddpp/enums.h>
#include <game/server/entities/character.h>
#include <game/server/entities/flag.h>
#include <game/server/gamemodes/DDRace.h>

#include "ddnetpp.h"

void CGameControllerDDNetPP::Snap(int SnappingClient)
{
	CGameControllerDDRace::Snap(SnappingClient);
	SnapFlags(SnappingClient);
	// FakeSnap(SnappingClient);
}

void CGameControllerDDNetPP::SnapFlags(int SnappingClient)
{
	int FlagCarrierRed = FLAG_MISSING;
	if(m_apFlags[TEAM_RED])
	{
		if(m_apFlags[TEAM_RED]->m_AtStand)
			FlagCarrierRed = FLAG_ATSTAND;
		else if(m_apFlags[TEAM_RED]->GetCarrier() && m_apFlags[TEAM_RED]->GetCarrier()->GetPlayer())
			FlagCarrierRed = m_apFlags[TEAM_RED]->GetCarrier()->GetPlayer()->GetCid();
		else
			FlagCarrierRed = FLAG_TAKEN;
	}

	int FlagCarrierBlue = FLAG_MISSING;
	if(m_apFlags[TEAM_BLUE])
	{
		if(m_apFlags[TEAM_BLUE]->m_AtStand)
			FlagCarrierBlue = FLAG_ATSTAND;
		else if(m_apFlags[TEAM_BLUE]->GetCarrier() && m_apFlags[TEAM_BLUE]->GetCarrier()->GetPlayer())
			FlagCarrierBlue = m_apFlags[TEAM_BLUE]->GetCarrier()->GetPlayer()->GetCid();
		else
			FlagCarrierBlue = FLAG_TAKEN;
	}

	if(Server()->IsSixup(SnappingClient))
	{
		protocol7::CNetObj_GameDataFlag *pGameDataObj = static_cast<protocol7::CNetObj_GameDataFlag *>(Server()->SnapNewItem(-protocol7::NETOBJTYPE_GAMEDATAFLAG, 0, sizeof(protocol7::CNetObj_GameDataFlag)));
		if(!pGameDataObj)
			return;

		pGameDataObj->m_FlagCarrierRed = FlagCarrierRed;
		pGameDataObj->m_FlagCarrierBlue = FlagCarrierBlue;
	}
	else
	{
		CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
		if(!pGameDataObj)
			return;

		pGameDataObj->m_FlagCarrierRed = FlagCarrierRed;
		pGameDataObj->m_FlagCarrierBlue = FlagCarrierBlue;
	}
}

// snap hacks to improve client prediction
void CGameControllerDDNetPP::FakeSnap(int SnappingClient)
{
	if(SnappingClient < 0 || SnappingClient >= MAX_CLIENTS)
		return;
	CPlayer *pPlayer = GameServer()->m_apPlayers[SnappingClient];
	if(!pPlayer)
		return;

	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
		return;

	// Smooth flag hooking by fokkonaut
	int Team = -1;
	if(pChr->Core()->HookedPlayer() == CLIENT_ID_FLAG_BLUE)
		Team = TEAM_BLUE;
	else if(pChr->Core()->HookedPlayer() == CLIENT_ID_FLAG_RED)
		Team = TEAM_RED;

	if(Team == -1)
		return;

	CFlag *pFlag = GameServer()->m_pController->m_apFlags[Team];
	if(!pFlag)
		return;

	// We dont send the NETOBJTYPE_PLAYERINFO object, because that would make the client render the tee. We could send it every 2nd snapshot, so the client doesnt
	// have the SNAP_PREV of it and wont render it aswell, but it seems to me that not having the object at all is also fine
	int FakeId = 60; // TODO: find a good value for this has to match the snapped m_HookedPlayer in character.cpp

	// log_info("ddnet++", "sending fake player to cid=%d", SnappingClient);

	// Send character at the position of the flag we are currently hooking
	CNetObj_Character *pCharacter = static_cast<CNetObj_Character *>(Server()->SnapNewItem(NETOBJTYPE_CHARACTER, FakeId, sizeof(CNetObj_Character)));
	if(!pCharacter)
		return;

	pCharacter->m_X = pFlag->GetPos().x;
	pCharacter->m_Y = pFlag->GetPos().y;

	// If the flag is getting hooked while close to us, the client predicts the invisible fake tee as if it would be colliding with us
	CNetObj_DDNetCharacter *pDDNetCharacter = static_cast<CNetObj_DDNetCharacter *>(Server()->SnapNewItem(NETOBJTYPE_DDNETCHARACTER, FakeId, sizeof(CNetObj_DDNetCharacter)));
	if(!pDDNetCharacter)
		return;
	pDDNetCharacter->m_Flags = CHARACTERFLAG_COLLISION_DISABLED;
}

// SnappingClient - Client Id of the player that will receive the snapshot
// pPlayer - CPlayer that is being snapped
// DDRaceScore - Current value of the score set by the ddnet code
int CGameControllerDDNetPP::SnapPlayerScore(int SnappingClient, CPlayer *pPlayer, int DDRaceScore)
{
	return pPlayer->GetScoreValue(SnappingClient);
}

int CGameControllerDDNetPP::SnapScoreLimit(int SnappingClient)
{
	if(SnappingClient < 0 || SnappingClient >= MAX_CLIENTS)
		return 0;

	CPlayer *pSnapReceiver = GameServer()->m_apPlayers[SnappingClient];
	if(!pSnapReceiver)
		return CGameControllerDDRace::SnapScoreLimit(SnappingClient);

	if(pSnapReceiver->IsInstagibMinigame())
	{
		if(pSnapReceiver->m_IsInstaMode_fng)
		{
			if(pSnapReceiver->m_IsInstaMode_idm)
				return g_Config.m_SvRifleScorelimit;
			else if(pSnapReceiver->m_IsInstaMode_gdm)
				return g_Config.m_SvGrenadeScorelimit;
		}
	}

	CMinigame *pMinigame = GameServer()->GetMinigame(SnappingClient);
	if(pMinigame)
		return pMinigame->ScoreLimit(pSnapReceiver);

	return CGameControllerDDRace::SnapScoreLimit(SnappingClient);
}

// SnappingClient - Client Id of the player that will receive the snapshot
// pPlayer - CPlayer that is being snapped
// pClientInfo - (in and output) info that is being snappend which is already pre filled by ddnet and can be altered.
// pPlayerInfo - (in and output) info that is being snappend which is already pre filled by ddnet and can be altered.
void CGameControllerDDNetPP::SnapPlayer6(int SnappingClient, CPlayer *pPlayer, CNetObj_ClientInfo *pClientInfo, CNetObj_PlayerInfo *pPlayerInfo)
{
	// hide players in the captcha room from the scoreboard
	// this is 0.6 only so 0.7 players see all players at all times (too lazy to fix)
	if(pPlayer->m_PendingCaptcha)
		pPlayerInfo->m_Team = TEAM_BLUE;
}

// SnappingClient - Client Id of the player that will receive the snapshot
// pPlayer - CPlayer that is being snapped
// PlayerFlags7 - the flags that were already set for that player by ddnet
int CGameControllerDDNetPP::SnapPlayerFlags7(int SnappingClient, CPlayer *pPlayer, int PlayerFlags7)
{
	return PlayerFlags7;
}
