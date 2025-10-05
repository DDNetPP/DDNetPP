#include "meteor.h"

#include <engine/shared/config.h>

#include <game/server/gamecontext.h>

CMeteor::CMeteor(CGameWorld *pGameWorld, int OwnerId, vec2 Pos) :
	CStableProjectile(pGameWorld, WEAPON_SHOTGUN, OwnerId, Pos)
{
	m_Vel = vec2(0.1f, 0.1f);
}

void CMeteor::Tick()
{
	float Friction = g_Config.m_SvMeteorFriction / 1000000.f;
	float MaxAccel = g_Config.m_SvMeteorMaxAccel / 1000.f;
	float AccelPreserve = g_Config.m_SvMeteorAccelPreserve / 1000.f;
	for(auto &Player : GameServer()->m_apPlayers)
	{
		if(Player && Player->GetCharacter())
		{
			vec2 CharPos = Player->GetCharacter()->m_Pos;
			m_Vel += normalize(CharPos - m_Pos) * (MaxAccel * AccelPreserve / (distance(CharPos, m_Pos) + AccelPreserve));
		}
	}
	m_Pos += m_Vel;
	m_Vel *= 1.f - Friction;
}
