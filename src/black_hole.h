/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_BLACKHOLE_H
#define GAME_SERVER_ENTITIES_BLACKHOLE_H

#include <game/server/entity.h>

class CBlackHole : public CEntity
{
public:
	CBlackHole(CGameWorld *pGameWorld, vec2 Pos, int Owner);

	virtual void Reset();
	virtual void Attract();
	virtual void CreateHole();
	virtual void Tick();
	virtual void Snap(int SnappingClient);

private:
	int m_Owner;
	int m_Buffer;
	bool GoingToKill;
	int m_Timer;

	bool Attracting;

	vec2 m_Pos;
};

#endif