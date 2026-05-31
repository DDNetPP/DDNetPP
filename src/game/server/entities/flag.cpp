/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include "flag.h"

#include <base/log.h>

#include <generated/protocol.h>

#include <game/server/gamecontext.h>

CFlag::CFlag(CGameWorld *pGameWorld, int Team) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_FLAG, false)
{
	log_info(
		"game",
		"%s flag constructed",
		Team == TEAM_RED ? "red" : "blue");

	m_Team = Team;
	m_pCarryingCharacter = nullptr;
	m_pLastCarryingCharacter = nullptr;
	m_GrabTick = 0;

	m_TuneZone = GameServer()->Collision()->IsTune(GameServer()->Collision()->GetMapIndex(m_Pos));

	Reset();
}

void CFlag::Reset()
{
	// to debug https://github.com/DDNetPP/DDNetPP/issues/327
	log_info(
		"ddnet++",
		"%s flag reset carrier=%s prev_carrier=%s",
		m_Team == TEAM_RED ? "red" : "blue",
		m_pCarryingCharacter ? Server()->ClientName(m_pCarryingCharacter->GetPlayer()->GetCid()) : "",
		m_pLastCarryingCharacter ? Server()->ClientName(m_pLastCarryingCharacter->GetPlayer()->GetCid()) : "");

	m_pCarryingCharacter = nullptr;
	m_pLastCarryingCharacter = nullptr;
	m_AtStand = true;
	m_Pos = m_StandPos;
	m_Vel = vec2(0, 0);
	m_GrabTick = 0;
	m_DropFreezeTick = 0;
	m_DropTick = 0;
}

void CFlag::TickPaused()
{
	++m_DropTick;
	if(m_GrabTick)
		++m_GrabTick;
}

void CFlag::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Flag Flag = {};
	Flag.m_X = (int)m_Pos.x;
	Flag.m_Y = (int)m_Pos.y;
	Flag.m_Team = m_Team;
	Server()->SnapNewItem(m_Team, Flag);
}
