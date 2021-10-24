#include "meteor.h"
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>

CMeteor::CMeteor(CGameWorld *pGameWorld, vec2 Pos) :
	CStableProjectile(pGameWorld, WEAPON_SHOTGUN, Pos)
{
	m_Vel = vec2(0.1f, 0.1f);
}

void CMeteor::Tick()
{
	float Friction = g_Config.m_SvMeteorFriction / 1000000.f;
	float MaxAccel = g_Config.m_SvMeteorMaxAccel / 1000.f;
	float AccelPreserve = g_Config.m_SvMeteorAccelPreserve / 1000.f;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetCharacter())
		{
			vec2 CharPos = GameServer()->m_apPlayers[i]->GetCharacter()->m_Pos;
			m_Vel += normalize(CharPos - m_Pos) * (MaxAccel * AccelPreserve / (distance(CharPos, m_Pos) + AccelPreserve));
		}
	}
	m_Pos += m_Vel;
	m_Vel *= 1.f - Friction;
}
