#ifndef GAME_SERVER_ENTITIES_METEOR_H
#define GAME_SERVER_ENTITIES_METEOR_H

#include <game/server/entities/stable_projectile.h>

class CMeteor : public CStableProjectile
{
	vec2 m_Vel;

public:
	CMeteor(CGameWorld *pGameWorld, vec2 Pos = vec2());

	virtual void Tick();
};

#endif
