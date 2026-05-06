#include "stable_projectile.h"

#include <game/server/gamecontext.h>

CStableProjectile::CStableProjectile(CGameWorld *pGameWorld, int OwnerId, int Type, vec2 Pos) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_PROJECTILE, true)
{
	m_OwnerId = OwnerId;
	m_Type = Type;

	m_Pos = Pos;
	m_LastResetPos = Pos;

	m_LastResetTick = Server()->Tick();
	m_CalculatedVel = false;

	GameWorld()->InsertEntity(this);
}

void CStableProjectile::Reset()
{
	m_MarkedForDestroy = true;
}

void CStableProjectile::TickDeferred()
{
	if(Server()->Tick() % 4 == 1)
	{
		m_LastResetPos = m_Pos;
		m_LastResetTick = Server()->Tick();
	}
	m_CalculatedVel = false;
}

void CStableProjectile::CalculateVel()
{
	float Time = (Server()->Tick() - m_LastResetTick) / (float)Server()->TickSpeed();
	float Curvature = 0;
	float Speed = 0;

	int TuneZone = GameServer()->Collision()->IsTune(GameServer()->Collision()->GetMapIndex(m_Pos));
	switch(m_Type)
	{
	case WEAPON_GRENADE:
		if(!TuneZone)
		{
			Curvature = GameServer()->GlobalTuning()->m_GrenadeCurvature;
			Speed = GameServer()->GlobalTuning()->m_GrenadeSpeed;
		}
		else
		{
			Curvature = GameServer()->TuningList()[TuneZone].m_GrenadeCurvature;
			Speed = GameServer()->TuningList()[TuneZone].m_GrenadeSpeed;
		}

		break;

	case WEAPON_SHOTGUN:
		if(!TuneZone)
		{
			Curvature = GameServer()->GlobalTuning()->m_ShotgunCurvature;
			Speed = GameServer()->GlobalTuning()->m_ShotgunSpeed;
		}
		else
		{
			Curvature = GameServer()->TuningList()[TuneZone].m_ShotgunCurvature;
			Speed = GameServer()->TuningList()[TuneZone].m_ShotgunSpeed;
		}

		break;

	case WEAPON_GUN:
		if(!TuneZone)
		{
			Curvature = GameServer()->GlobalTuning()->m_GunCurvature;
			Speed = GameServer()->GlobalTuning()->m_GunSpeed;
		}
		else
		{
			Curvature = GameServer()->TuningList()[TuneZone].m_GunCurvature;
			Speed = GameServer()->TuningList()[TuneZone].m_GunSpeed;
		}
		break;
	}

	m_VelX = ((m_Pos.x - m_LastResetPos.x) / Time / Speed) * 100;
	m_VelY = ((m_Pos.y - m_LastResetPos.y) / Time / Speed - Time * Speed * Curvature / 10000) * 100;

	m_CalculatedVel = true;
}

void CStableProjectile::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;
	if(!GetId().has_value())
		return;

	CCharacter *pOwnerChar = nullptr;
	if(m_OwnerId >= 0)
		pOwnerChar = GameServer()->GetPlayerChar(m_OwnerId);

	CClientMask TeamMask = CClientMask().set();
	if(pOwnerChar && pOwnerChar->IsAlive())
		TeamMask = pOwnerChar->TeamMask();

	if(SnappingClient != SERVER_DEMO_CLIENT && !TeamMask.test(SnappingClient))
		return;

	if(!m_CalculatedVel)
		CalculateVel();

	CNetObj_Projectile Proj = {};
	Proj.m_X = (int)m_LastResetPos.x;
	Proj.m_Y = (int)m_LastResetPos.y;
	Proj.m_VelX = m_VelX;
	Proj.m_VelY = m_VelY;
	Proj.m_StartTick = m_LastResetTick;
	Proj.m_Type = m_Type;
	Server()->SnapNewItem(GetId().value(), Proj);
}
