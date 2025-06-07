#ifndef GAME_SERVER_ENTITIES_STABLE_PROJECTILE_H
#define GAME_SERVER_ENTITIES_STABLE_PROJECTILE_H

#include <game/server/entity.h>

class CStableProjectile : public CEntity
{
	int m_OwnerId;
	int m_Type;
	vec2 m_LastResetPos;
	int m_LastResetTick;
	bool m_CalculatedVel;
	int m_VelX;
	int m_VelY;

	void CalculateVel();

public:
	CStableProjectile(CGameWorld *pGameWorld, int OwnerId, int Type, vec2 Pos = vec2());

	virtual void Reset() override;
	virtual void TickDeferred() override;
	virtual void Snap(int SnappingClient) override;
};

#endif
