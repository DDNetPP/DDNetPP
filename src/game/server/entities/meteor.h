#ifndef GAME_SERVER_ENTITIES_METEOR_H
#define GAME_SERVER_ENTITIES_METEOR_H

#include <game/server/entities/stable_projectile.h>

class CMeteor : public CStableProjectile
{
	vec2 m_Vel;
	int m_LifeSpan;

public:
	CMeteor(CGameWorld *pGameWorld, int OwnerId, vec2 Pos, int LifeSpan);

	void Tick() override;
};

#endif
