/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include "flag.h"

#include <base/log.h>

#include <generated/protocol.h>

#include <game/server/gamecontext.h>

CFlag::CFlag(CGameWorld *pGameWorld, int Team) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_FLAG, false)
{
	m_Team = Team;
	m_pCarryingCharacter = nullptr;
	m_pLastCarryingCharacter = nullptr;
	m_GrabTick = 0;

	m_TuneZone = GameServer()->Collision()->IsTune(GameServer()->Collision()->GetMapIndex(m_Pos));

	Reset();
}

void CFlag::Reset()
{
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
