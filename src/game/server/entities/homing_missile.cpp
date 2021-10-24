/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>

#include <game/server/teams.h>

#include "homing_missile.h"

CHomingMissile::CHomingMissile(CGameWorld *pGameWorld, int Lifetime, int Owner, float Force, vec2 Dir) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_PROJECTILE)
{
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
	GameServer()->m_World.DestroyEntity(this);
}

void CHomingMissile::Tick()
{
	m_Lifetime--;

	if(m_Lifetime < 0)
	{
		Reset();
		return;
	}

	if(!GameServer()->GetPlayerChar(m_Owner))
	{
		GameServer()->CreateExplosion(m_Pos, -1, WEAPON_GRENADE, true, 0, -1);
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE, -1);

		Reset();
		return;
	}

	if(GameServer()->Collision()->IsSolid(m_Pos.x, m_Pos.y))
	{
		GameServer()->CreateExplosion(m_Pos, m_Owner, WEAPON_GRENADE, true, 0, GameServer()->GetPlayerChar(m_Owner)->Teams()->TeamMask(0));
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE, GameServer()->GetPlayerChar(m_Owner)->Teams()->TeamMask(0));

		Reset();
		return;
	}

	CCharacter *pTarget = CharacterNear();
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
	if(Server()->Tick() % 4 == 1)
	{
		m_LastResetPos = m_Pos;
		m_LastResetTick = Server()->Tick();
	}
	m_CalculatedVel = false;
}

void CHomingMissile::CalculateVel()
{
	float Time = (Server()->Tick() - m_LastResetTick) / (float)Server()->TickSpeed();
	float Curvature = 0;
	float Speed = 0;

	Curvature = 0;
	Speed = 1000;

	m_VelX = ((m_Pos.x - m_LastResetPos.x) / Time / Speed) * 100;
	m_VelY = ((m_Pos.y - m_LastResetPos.y) / Time / Speed - Time * Speed * Curvature / 10000) * 100;

	m_CalculatedVel = true;
}

void CHomingMissile::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Projectile *pMissile = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, GetID(), sizeof(CNetObj_Projectile)));
	if(!pMissile)
		return;

	if(!m_CalculatedVel)
		CalculateVel();

	pMissile->m_X = (int)m_LastResetPos.x;
	pMissile->m_Y = (int)m_LastResetPos.y;
	pMissile->m_VelX = m_VelX;
	pMissile->m_VelY = m_VelY;
	pMissile->m_StartTick = m_LastResetTick;
	pMissile->m_Type = WEAPON_GRENADE;
}

CCharacter *CHomingMissile::CharacterNear()
{
	CCharacter *pOwner = GameServer()->GetPlayerChar(m_Owner);
	CCharacter *pTarget = GameWorld()->ClosestCharacter(m_Pos, 2000.f, pOwner ? pOwner : 0);

	if(pTarget)
		return pTarget;

	return 0x0;
}

void CHomingMissile::Move(CCharacter *pTarget)
{
	vec2 TargetPos = pTarget->m_Pos;

	//add pathfinding here

	if(m_Pos.x > TargetPos.x)
		m_Pos.x -= 5;
	if(m_Pos.x < TargetPos.x)
		m_Pos.x += 5;
	if(m_Pos.y > TargetPos.y)
		m_Pos.y -= 5;
	if(m_Pos.y < TargetPos.y)
		m_Pos.y += 5;

	return;
}

bool CHomingMissile::Hit(CCharacter *pHitTarget)
{
	vec2 TargetPos = pHitTarget->m_Pos;

	if(distance(m_Pos, TargetPos) < 20.f)
	{
		pHitTarget->TakeDamage(m_Direction * maximum(0.001f, m_Force), 1, m_Owner, WEAPON_GRENADE);

		CCharacter *pOwner = GameServer()->GetPlayerChar(m_Owner);

		GameServer()->CreateExplosion(m_Pos, m_Owner, WEAPON_GRENADE, true, 0, pOwner ? GameServer()->GetPlayerChar(m_Owner)->Teams()->TeamMask(0) : -1LL);
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE, pOwner ? GameServer()->GetPlayerChar(m_Owner)->Teams()->TeamMask(0) : -1LL);

		return true;
	}

	return false;
}