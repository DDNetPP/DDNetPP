#ifndef GAME_SERVER_ENTITIES_PLANT_H
#define GAME_SERVER_ENTITIES_PLANT_H

#include <game/server/entity.h>

#define MAX_PLANT_PROJS 10

class CPlant : public CEntity
{
	vec2 m_LastResetPos;
	int m_LastResetTick;
	bool m_CalculatedVel;
	int m_VelX;
	int m_VelY;
	int m_GrowState;
	int m_GrowDelay;

	int IsCharacterNear();
	void Harvest();

	void CalculateVel();

	CStableProjectile *m_apPlantProj[MAX_PLANT_PROJS];
	CStableProjectile *pPlant;
	void ResetProjectiles();

public:
	CPlant(CGameWorld *pGameWorld, vec2 Pos = vec2());

	virtual void Tick();
	virtual void Reset();
	virtual void TickDefered();
	virtual void Snap(int SnappingClient);
};

#endif
