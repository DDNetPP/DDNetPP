/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#ifndef PLASMA_TYPE
#define PLASMA_TYPE

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
	float m_Speed;

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
