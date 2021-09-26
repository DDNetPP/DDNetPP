/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "wall.h"

////////////////////////////////////
//                                //
//       CWall by Cyser!xXx       //
//                                //
////////////////////////////////////

CWall::CWall(CGameWorld *pGameWorld, vec2 From, vec2 To, int Owner)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_From = From;
	m_To = To;
	m_Owner = Owner;

	GameWorld()->InsertEntity(this);
	
	this->m_ID2 = Server()->SnapNewID();
}

bool CWall::HitCharacter()
{
	vec2 At;
	CCharacter *OwnerChar = GameServer()->GetPlayerChar(m_Owner);
	CCharacter *Hit = GameServer()->m_World.IntersectCharacter(m_From, m_To, 0.f, At, 0x0);

	if(!Hit)
		return false;

	if(OwnerChar && Hit->GetPlayer()->GetCID() == m_Owner &&  Hit->m_ActiveWeapon == WEAPON_GUN)
	{
		Reset();
		OwnerChar->GetPlayer()->m_ActiveWalls = 0;
		return false;
	}
	
	Hit->TakeDamage(vec2(0.f, 0.f), 1, m_Owner, WEAPON_HAMMER);
	
	if(OwnerChar)
		OwnerChar->GetPlayer()->m_ActiveWalls--;
	
	return true;
}
	
void CWall::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

void CWall::Tick()
{
	CCharacter *OwnerChar = GameServer()->GetPlayerChar(m_Owner);

	if(!OwnerChar)
	{
		Reset();
	}
	HitCharacter();
}

void CWall::Snap(int SnappingClient)
{
	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID, sizeof(CNetObj_Laser)));

	pObj->m_X = (int)m_From.x;
	pObj->m_Y = (int)m_From.y;
	pObj->m_FromX = (int)m_To.x;
	pObj->m_FromY = (int)m_To.y;
	pObj->m_StartTick = Server()->Tick();
   
	CNetObj_Laser *pObj2 = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID2, sizeof(CNetObj_Laser)));

	pObj2->m_X = (int)m_To.x;
	pObj2->m_Y = (int)m_To.y;
	pObj2->m_FromX = (int)m_To.x;
	pObj2->m_FromY = (int)m_To.y;
	pObj2->m_StartTick = Server()->Tick();
}
