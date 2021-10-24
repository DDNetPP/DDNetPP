/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "black_hole.h"
#include <engine/config.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>

/* Cyser!xXx's BlackHole Code 23.01.2013*/
//////////////////////////////////////////////////
// CBlackHole
//////////////////////////////////////////////////

const int m_Longitude = 150;
const int m_Strength = 4;

CBlackHole::CBlackHole(CGameWorld *pGameWorld, vec2 Pos, int Owner) :
	CEntity(pGameWorld, NULL /*CGameWorld::ENTTYPE_LASER*/)
{
	m_Pos = Pos;
	m_Owner = Owner;
	GoingToKill = false;

	GameWorld()->InsertEntity(this);
}

void CBlackHole::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

void CBlackHole::Attract()
{
	CCharacter *pOwnerChar = GameServer()->GetPlayerChar(m_Owner);
	CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos, m_Longitude, 0x0);

	/*For Automatic BlackHole: */
	if(pOwnerChar && pChr && pOwnerChar->GetPlayer()->GetTeam() == pChr->GetPlayer()->GetTeam())
		return;

	/*Don't fire if the tee is behind the wall*/
	if(pChr && GameServer()->Collision()->IntersectLine(m_Pos, pChr->m_Pos, 0x0, 0))
		return;

	if(!pChr)
		return;

	if(length(m_Pos - pChr->m_Pos) > /*28*/ 35 && length(m_Pos - pChr->m_Pos) < m_Longitude)
	{
		vec2 Temp = pChr->m_Core.m_Vel + (normalize(m_Pos - pChr->m_Pos) * m_Strength);
		pChr->m_Core.m_Vel = Temp;
		Attracting = true;
		GoingToKill = true;
	}
	else if(length(m_Pos - pChr->m_Pos) > m_Longitude)
	{
		Attracting = false;
		GoingToKill = false;
	}

	// Hit Character:

	if(GoingToKill == true)
		m_Timer++;
	else
		m_Timer = 1;

	if(GoingToKill == true)
	{
		if(m_Timer == 2)
			GameServer()->SendBroadcast("3", pChr->GetPlayer()->GetCID());
		if(m_Timer == 50)
			GameServer()->SendBroadcast("2", pChr->GetPlayer()->GetCID());
		if(m_Timer == 100)
			GameServer()->SendBroadcast("1", pChr->GetPlayer()->GetCID());

		if(m_Timer == 149)
		{
			GameServer()->CreateExplosion(m_Pos, m_Owner, WEAPON_HAMMER, false);
			GameServer()->CreateExplosion(m_Pos, m_Owner, WEAPON_HAMMER, false);
			m_Timer = 1;
		}
	}
	//
}

void CBlackHole::CreateHole()
{
	m_Buffer++;

	if(m_Buffer > 5)
	{
		GameServer()->CreateDeath(m_Pos, m_Owner);
		m_Buffer = 1;
	}
}

void CBlackHole::Tick()
{
	CCharacter *pOwnerChar = GameServer()->GetPlayerChar(m_Owner);

	if(m_Owner != -1 && !pOwnerChar)
		Reset();

	Attract();
	CreateHole();
}

void CBlackHole::Snap(int SnappingClient)
{
	/*CCharacter *pOwnerChar = GameServer()->GetPlayerChar(m_Owner);

	CNetObj_Laser *pObj = static_cast<CNetObj_Laser*>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID, sizeof(CNetObj_Laser))); 
	
	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;	
	if(pOwnerChar && Attracting == true)
	{
		pObj->m_FromX = (int)pOwnerChar->m_Pos.x;
		pObj->m_FromY = (int)pOwnerChar->m_Pos.y;
	}
	else
	{
		pObj->m_FromX = (int)m_Pos.x;
		pObj->m_FromY = (int)m_Pos.y;
	}
	pObj->m_StartTick = Server()->Tick();*/
}