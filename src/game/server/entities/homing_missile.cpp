/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "homing_missile.h"

#include <generated/protocol.h>

#include <game/server/gamecontext.h>
#include <game/server/teams.h>

CHomingMissile::CHomingMissile(CGameWorld *pGameWorld, int Lifetime, int Owner, float Force, vec2 Dir) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_PROJECTILE, true)
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
	m_MarkedForDestroy = true;
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

void CHomingMissile::TickDeferred()
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
	if(!GetId().has_value())
		return;

	CNetObj_Projectile Missile = {};
	if(!m_CalculatedVel)
		CalculateVel();

	Missile.m_X = (int)m_LastResetPos.x;
	Missile.m_Y = (int)m_LastResetPos.y;
	Missile.m_VelX = m_VelX;
	Missile.m_VelY = m_VelY;
	Missile.m_StartTick = m_LastResetTick;
	Missile.m_Type = WEAPON_GRENADE;
	Server()->SnapNewItem(GetId().value(), Missile);
}

CCharacter *CHomingMissile::CharacterNear()
{
	CCharacter *pOwner = GameServer()->GetPlayerChar(m_Owner);
	CCharacter *pTarget = GameWorld()->ClosestCharacter(m_Pos, 2000.f, pOwner ? pOwner : nullptr);

	if(pTarget)
		return pTarget;

	return nullptr;
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
}

bool CHomingMissile::Hit(CCharacter *pHitTarget)
{
	vec2 TargetPos = pHitTarget->m_Pos;

	if(distance(m_Pos, TargetPos) < 20.f)
	{
		pHitTarget->TakeDamage(m_Direction * std::max(0.001f, m_Force), 1, m_Owner, WEAPON_GRENADE);

		CCharacter *pOwner = GameServer()->GetPlayerChar(m_Owner);

		GameServer()->CreateExplosion(
			m_Pos,
			m_Owner,
			WEAPON_GRENADE,
			true,
			0,
			pOwner ? pOwner->TeamMask() : CClientMask().set());
		GameServer()->CreateSound(
			m_Pos,
			SOUND_GRENADE_EXPLODE,
			pOwner ? pOwner->TeamMask() : CClientMask().set());

		return true;
	}

	return false;
}
