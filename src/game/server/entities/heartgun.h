// Code taken from F-DDrace
// Modified by Filoqcus

#ifndef GAME_SERVER_ENTITIES_HEARTGUN_H
#define GAME_SERVER_ENTITIES_HEARTGUN_H

#include <game/server/entity.h>

enum
{
	HEART_NOT_COLLIDED = 0,
	HEART_COLLIDED_ONCE,
	HEART_COLLIDED_TWICE
};

class CHeartGun : public CEntity
{
public:
	CHeartGun(CGameWorld *pGameWorld, int Owner, vec2 Pos, vec2 Dir, bool Freeze,
		bool Explosive, bool Unfreeze, bool Bloody, bool Spooky, float Lifetime = 6.0f, float Accel = 1.0f, float Speed = 10.0f);
	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;

private:
	vec2 m_Core;
	vec2 m_PrevPos;
	vec2 m_Direction;

	int m_EvalTick;
	int m_LifeTime;

	CClientMask m_TeamMask;
	CCharacter *m_pOwner;
	int m_Owner;

	int m_Freeze;
	int m_Unfreeze;
	int m_Bloody;
	int m_Spooky;
	bool m_Explosive;

	float m_Accel;

	int m_CollisionState;

	void HitCharacter();
	void Move();
};

#endif
