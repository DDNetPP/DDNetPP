/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "controlled-laser.h"

/////////////////////////////////////
//                                 //
//     Remote control laser        //
//     01.01.2013 by Cyser!xXx     //
//                                 //
/////////////////////////////////////

CRControl::CRControl(CGameWorld *pGameWorld, vec2 Pos, int Owner, int Damage, int LifeTime)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PROJECTILE)
{
	m_Pos = Pos;
	m_Owner = Owner;
	m_Damage = Damage;
	m_LifeTime = LifeTime;

	GameWorld()->InsertEntity(this);
}

void CRControl::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

void CRControl::Tick()
{
	CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos, 20.0f, 0);
	CCharacter *pOwnerChar = GameServer()->GetPlayerChar(m_Owner);

	if(GameServer()->m_World.m_Paused == true)
		GameWorld()->DestroyEntity(this);

	m_LifeTime--;

	if(m_LifeTime <= 0)
		Reset();

	if(m_Owner != -1 && !pOwnerChar)
		m_Owner = -1;
	
	if(m_Owner != -1) /*If there isn't an owner*/
	{
		if(pOwnerChar)
		{
			vec2 m_FlyPos =	vec2(pOwnerChar->m_LatestInput.m_TargetX, pOwnerChar->m_LatestInput.m_TargetY)+pOwnerChar->m_Pos;
			m_Pos = m_FlyPos;
			m_Pos.x += cosf(Server()->Tick()*0.1f - 60)*80.0f;
			m_Pos.y += sinf(Server()->Tick()*0.1f - 60)*80.0f;
		}

		if(pChr && pChr->GetPlayer()->GetCID() != m_Owner)
		{
			pChr->TakeDamage(m_Pos, m_Damage, m_Owner, WEAPON_GRENADE);
			GameServer()->CreateExplosion(m_Pos, m_Owner, WEAPON_RIFLE, false);
			Reset();
		}
	}
	
	int Collide = GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y)&CCollision::COLFLAG_SOLID;
	if(Collide)
	{
		if(pOwnerChar)
			GameServer()->CreateExplosion(m_Pos, m_Owner, WEAPON_GRENADE, false);
		Reset();
	}
}

void CRControl::Snap(int SnappingClient)
{
	CNetObj_Laser *pObj = static_cast<CNetObj_Laser*>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_ID, sizeof(CNetObj_Laser))); 
	
	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
}

