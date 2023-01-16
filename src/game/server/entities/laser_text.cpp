/*
    Inspired by fngs lasertext / loltext
    https://github.com/Jupeyy/teeworlds-fng2-mod/blob/07ac6046c37eba552d5d0b59bcc7bf35904b3e4f/src/game/server/laser_text.cpp
    https://github.com/fstd/teeworlds/blob/edceb914f47f3fb6407a85f8cd01060bf79b847a/src/game/server/entities/loltext.cpp
*/

#include <game/server/gamecontext.h>

#include "laser_text.h"

CLaserText::CLaserText(CGameWorld *pGameWorld, vec2 Pos, int Owner, int pAliveTicks, char *pText, int pTextLen) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER, Pos)
{
	m_Pos = Pos;
	m_Owner = Owner;
	GameWorld()->InsertEntity(this);

	m_CurTicks = Server()->Tick();
	m_StartTick = Server()->Tick();
	m_AliveTicks = pAliveTicks;
}

void CLaserText::Reset()
{
	m_MarkedForDestroy = true;
}

void CLaserText::Tick()
{
	if(++m_CurTicks - m_StartTick > m_AliveTicks)
		m_MarkedForDestroy = true;
}

void CLaserText::TickPaused()
{
}

void CLaserText::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, GetID(), sizeof(CNetObj_Laser)));
	if(!pObj)
		return;

	pObj->m_X = GetPos().x;
	pObj->m_Y = GetPos().y;
	pObj->m_FromX = GetPos().x;
	pObj->m_FromY = GetPos().y;
	pObj->m_StartTick = Server()->Tick();
}