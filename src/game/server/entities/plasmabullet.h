// fokkonaut

#ifndef GAME_SERVER_ENTITIES_PLASMABULLET_H
#define GAME_SERVER_ENTITIES_PLASMABULLET_H

#include <game/server/entity.h>

class CGun;

class CPlasmaBullet : public CEntity
{
	vec2 m_Core;
	int m_EvalTick;
	int m_LifeTime;

	int m_ResponsibleTeam;
	int m_Freeze;
	int m_Unfreeze;
	int m_Bloody;
	int m_Ghost;

	int m_Owner;
	float m_Accel;

	int m_IsInsideWall;

	bool m_Explosive;
	bool HitCharacter();
	void Move();

public:
	CPlasmaBullet(CGameWorld *pGameWorld, int Owner, vec2 Pos, vec2 Dir, bool Freeze,
		bool Explosive, bool Unfreeze, bool Bloody, bool Ghost, int ResponsibleTeam, float Lifetime = 1.5, float Accel = 1.1f, float Speed = 1.0f);

	virtual void Reset();
	virtual void Tick();
	virtual void Snap(int SnappingClient);
};

#endif
