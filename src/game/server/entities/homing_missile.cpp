/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "homing_missile.h"

#include <game/server/teams.h>

CHomingMissile::CHomingMissile(CGameWorld *pGameWorld, int Lifetime, int Owner, float Force, vec2 Dir)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PROJECTILE)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	m_Owner = Owner;
	m_StartTick = Server()->Tick();
	m_Lifetime = Server()->TickSpeed() * Lifetime;

	m_Direction = Dir;
	m_Force = Force;

	m_LastResetPos = GameServer()->GetPlayerChar(Owner)->m_Pos;
	m_LastResetTick = Server()->Tick();
	
	m_Pos = GameServer()->GetPlayerChar(Owner)->m_Pos;

	m_CalculatedVel = false;

	GameWorld()->InsertEntity(this);
}

void CHomingMissile::Reset()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	

	GameServer()->m_World.DestroyEntity(this);
}

void CHomingMissile::Tick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	m_Lifetime--;

	if (m_Lifetime < 0)
	{
		Reset();
		return;
	}

	if (!GameServer()->GetPlayerChar(m_Owner))
	{
		GameServer()->CreateExplosion(m_Pos, -1, WEAPON_GRENADE, true, 0, -1);
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE, -1);

		Reset();
		return;
	}

	if (GameServer()->Collision()->IsSolid(m_Pos.x, m_Pos.y))
	{
		GameServer()->CreateExplosion(m_Pos, m_Owner, WEAPON_GRENADE, true, 0, GameServer()->GetPlayerChar(m_Owner)->Teams()->TeamMask(0));
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE, GameServer()->GetPlayerChar(m_Owner)->Teams()->TeamMask(0));

		Reset();
		return;
	}


	CCharacter* pTarget = CharacterNear();
	if(!pTarget)
		return;

	Move(pTarget);
	
	if(Hit(pTarget))
	{
		Reset();
		return;
	}
}

void CHomingMissile::TickDefered()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Server()->Tick() % 4 == 1)
	{
		m_LastResetPos = m_Pos;
		m_LastResetTick = Server()->Tick();
	}
	m_CalculatedVel = false;
}

void CHomingMissile::CalculateVel()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	float Time = (Server()->Tick() - m_LastResetTick) / (float)Server()->TickSpeed();
	float Curvature = 0;
	float Speed = 0;

	Curvature = 0;
	Speed = 1000;

	m_VelX = ((m_Pos.x - m_LastResetPos.x) / Time / Speed) * 100;
	m_VelY = ((m_Pos.y - m_LastResetPos.y) / Time / Speed - Time * Speed*Curvature / 10000) * 100;

	m_CalculatedVel = true;
}

void CHomingMissile::Snap(int SnappingClient)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	if (NetworkClipped(SnappingClient))
		return;

	CNetObj_Projectile *pMissile = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_ID, sizeof(CNetObj_Projectile)));
	if (!pMissile)
		return;

	if (!m_CalculatedVel)
		CalculateVel();

	pMissile->m_X = (int)m_LastResetPos.x;
	pMissile->m_Y = (int)m_LastResetPos.y;
	pMissile->m_VelX = m_VelX;
	pMissile->m_VelY = m_VelY;
	pMissile->m_StartTick = m_LastResetTick;
	pMissile->m_Type = WEAPON_GRENADE;
}

CCharacter* CHomingMissile::CharacterNear()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	CCharacter* pOwner  = GameServer()->GetPlayerChar(m_Owner);
	CCharacter* pTarget = GameWorld()->ClosestCharacter(m_Pos, 2000.f, pOwner);

	if(pTarget)
		return pTarget;

	return 0x0;
}

void CHomingMissile::Move(CCharacter* pTarget)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	vec2 TargetPos = pTarget->m_Pos;

	//add pathfinding here

	if (m_Pos.x > TargetPos.x)
		m_Pos.x -= 10;
	if (m_Pos.x < TargetPos.x)
		m_Pos.x += 10;
	if (m_Pos.y > TargetPos.y)
		m_Pos.y -= 10;
	if (m_Pos.y < TargetPos.y)
		m_Pos.y += 10;

	return;
}

bool CHomingMissile::Hit(CCharacter* pHitTarget)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	
	vec2 TargetPos = pHitTarget->m_Pos;

	if(distance(m_Pos, TargetPos) < 20.f)
	{
		pHitTarget->TakeDamage(m_Direction * max(0.001f, m_Force), 1, m_Owner, WEAPON_GRENADE);

		GameServer()->CreateExplosion(m_Pos, m_Owner, WEAPON_GRENADE, true, 0, GameServer()->GetPlayerChar(m_Owner)->Teams()->TeamMask(0));
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE, GameServer()->GetPlayerChar(m_Owner)->Teams()->TeamMask(0));

		return true;
	}

	return false;
}