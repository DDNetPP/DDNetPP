/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_LASERBOX_H
#define GAME_SERVER_ENTITIES_LASERBOX_H

#include <game/server/entity.h>

class CBox : public CEntity
{
public:
	CBox(CGameWorld *pGameWorld, vec2 Pos, int Owner);
	
	virtual void Reset();
	virtual void Tick();
	virtual void BoxPhysics();
	virtual void Snap(int SnappingClient);
	vec2 m_Pos; //public because > gamecontroller.cpp

private:
	int m_Owner;
	int m_ID2;
	int m_ID3;
	int m_ID4;
	vec2 m_Vel;
};

#endif
