/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "shield.h"

/* Cyser!xXx's transformed Shield Code 18.01.2013*/
//////////////////////////////////////////////////
// CShield
//////////////////////////////////////////////////
CShield::CShield(CGameWorld *pGameWorld, vec2 Pos, int Owner, int Damage, int Type)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP)
{
	m_Pos = Pos;
	m_Owner = Owner;
	m_Damage = Damage;
	m_Type = Type;

	GameWorld()->InsertEntity(this);
}

void CShield::Reset()
{
	GameServer()->CreateDeath(m_Pos, m_Owner);
	GameWorld()->DestroyEntity(this);
}

void CShield::Tick()
{
	CCharacter *m_Chr = GameServer()->m_World.ClosestCharacter(m_Pos, 20.0f, 0);
	CCharacter *m_Ownerchar = GameServer()->GetPlayerChar(m_Owner);
	
	if(GameServer()->m_World.m_Paused == true)
		Reset();
	
	if(m_Owner != -1 && !m_Ownerchar)
		Reset();
	
	if(m_Owner != -1)
	{
		if(m_Ownerchar)
		{
			m_Vel = m_Ownerchar->m_Core.m_Vel;
			m_Pos = m_Ownerchar->m_Pos;

			m_Pos.x += cosf(Server()->Tick()*0.1f - m_Type)*100.0f;
			m_Pos.y += sinf(Server()->Tick()*0.1f - m_Type)*100.0f;
		}

		if(m_Chr && m_Chr->GetPlayer()->GetCID() != m_Owner)
		{
			m_Chr->TakeDamage(vec2(0,0), m_Damage, m_Owner, WEAPON_NINJA);
			Reset();
		}
	}	
}

void CShield::Snap(int SnappingClient)
{
	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = POWERUP_ARMOR;
	pP->m_Subtype = 0;
}
