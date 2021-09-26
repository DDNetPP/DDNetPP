/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_PICKUP2_H
#define GAME_SERVER_ENTITIES_PICKUP2_H

#include <game/server/entity.h>

class CShield : public CEntity
{
public:
	CShield(CGameWorld *pGameWorld, vec2 Pos, int Owner, int Damage, int m_Type);
	
	virtual void Reset();
	virtual void Tick();
	virtual void Snap(int SnappingClient);
	
private:
	int m_Type;
	int m_Owner;

	vec2 m_Pos;
	int m_Damage;
	vec2 m_Vel;
};

#endif
