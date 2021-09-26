/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_WALL_H
#define GAME_SERVER_ENTITIES_WALL_H

#include <game/server/entity.h>

class CWall : public CEntity
{
public:
	CWall(CGameWorld *pGameWorld, vec2 From, vec2 To, int Owner);
	
	virtual void Reset();
	virtual void Tick();
	virtual void Snap(int SnappingClient);
	
	vec2 m_From;
	vec2 m_To;

protected:
	bool HitCharacter();
	
private:
	int m_Owner;
	int m_ID2;
	int m_StartTick; 
};

#endif
