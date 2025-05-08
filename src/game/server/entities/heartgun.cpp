// Code taken from F-DDrace
// Modified by Filoqcus

#include "heartgun.h"
#include "character.h"
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/generated/server_data.h>
#include <game/server/gamecontext.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/teams.h>

CHeartGun::CHeartGun(CGameWorld *pGameWorld, int Owner, vec2 Pos, vec2 Dir, bool Freeze,
	bool Explosive, bool Unfreeze, bool Bloody, bool Spooky, float Lifetime, float Accel, float Speed) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_HEART_GUN, Pos)
{
	m_Owner = Owner;
	m_Pos = Pos;
	m_Core = normalize(Dir) * Speed;
	m_Freeze = Freeze;
	m_Explosive = Explosive;
	m_Unfreeze = Unfreeze;
	m_Bloody = Bloody;
	m_Spooky = Spooky;
	m_Direction = Dir;
	m_EvalTick = Server()->Tick();
	m_LifeTime = Server()->TickSpeed() * Lifetime;
	m_Accel = Accel;

	m_PrevPos = m_Pos;

	GameWorld()->InsertEntity(this);
}

void CHeartGun::Reset()
{
	m_MarkedForDestroy = true;
}

void CHeartGun::Tick()
{
	m_pOwner = nullptr;
	if(GameServer()->GetPlayerChar(m_Owner))
		m_pOwner = GameServer()->GetPlayerChar(m_Owner);

	if(m_Owner >= 0 && !m_pOwner && Config()->m_SvDestroyBulletsOnDeath)
	{
		Reset();
		return;
	}

	m_TeamMask = m_pOwner ? m_pOwner->TeamMask() : MAX_CLIENTS;

	m_LifeTime--;
	if(m_LifeTime <= 0)
	{
		Reset();
		return;
	}

	Move();
	HitCharacter();

	if(GameServer()->Collision()->IsSolid(m_Pos.x, m_Pos.y))
	{
		if(m_Explosive)
		{
			GameServer()->CreateExplosion(m_Pos, m_Owner, WEAPON_GRENADE, m_Owner == -1, m_pOwner ? m_pOwner->Team() : -1, m_TeamMask);
			GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE, m_TeamMask);
		}

		if(m_Bloody)
			GameServer()->CreateDeath(m_PrevPos, m_Owner, m_TeamMask);

		if(m_CollisionState == HEART_NOT_COLLIDED)
			m_CollisionState = HEART_COLLIDED_ONCE;

		if(m_CollisionState == HEART_COLLIDED_TWICE || !m_Spooky)
		{
			Reset();
			return;
		}
	}
	else if(m_CollisionState == HEART_COLLIDED_ONCE)
		m_CollisionState = HEART_COLLIDED_TWICE;

	m_PrevPos = m_Pos;
}

void CHeartGun::Move()
{
	m_Pos += m_Core;
	m_Core *= m_Accel;
}

void CHeartGun::HitCharacter()
{
	vec2 NewPos = m_Pos + m_Core;
	CCharacter *pHit = GameWorld()->IntersectCharacter(m_PrevPos, NewPos, 6.0f, NewPos, m_pOwner, m_Owner);
	if(!pHit)
		return;

	if(m_Bloody)
		GameServer()->CreateDeath(pHit->GetPos(), pHit->GetPlayer()->GetCid(), m_TeamMask);

	if(m_Freeze)
		pHit->Freeze();
	else if(m_Unfreeze)
		pHit->UnFreeze();

	if(m_Explosive)
	{
		GameServer()->CreateExplosion(m_Pos, m_Owner, WEAPON_GRENADE, m_Owner == -1, pHit->Team(), m_TeamMask);
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE, m_TeamMask);
	}

	if(m_Spooky)
	{
		pHit->SetEmote(EMOTE_SURPRISE, Server()->Tick() + 2 * Server()->TickSpeed());
		GameServer()->SendEmoticon(pHit->GetPlayer()->GetCid(), 7, -1);
	}

	pHit->SetEmote(EMOTE_HAPPY, Server()->Tick() + 2 * Server()->TickSpeed());
	GameServer()->SendEmoticon(pHit->GetPlayer()->GetCid(), 2, -1);

	Reset();
}

void CHeartGun::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	int SnappingClientVersion = GameServer()->GetClientVersion(SnappingClient);
	bool Sixup = Server()->IsSixup(SnappingClient);
	GameServer()->SnapPickup(CSnapContext(SnappingClientVersion, Sixup), GetId(), m_Pos, POWERUP_HEALTH, 0, 0);
}
