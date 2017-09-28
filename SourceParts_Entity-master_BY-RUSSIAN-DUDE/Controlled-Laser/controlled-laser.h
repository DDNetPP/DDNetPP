/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_RCONTROL_H
#define GAME_SERVER_ENTITIES_RCONTROL_H

#include <game/server/entity.h>

class CRControl: public CEntity
{
public:
	CRControl(CGameWorld *pGameWorld, vec2 Pos, int Owner, int Damage, int Lifetime);
	
	virtual void Reset();
	virtual void Tick();
	virtual void Snap(int SnappingClient);

	vec2 m_FlyPos;
private:
	int m_Owner;
	int m_Damage;
	int m_LifeTime;

	vec2 m_Pos;
	vec2 m_Vel;
};

#endif
