/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_MUNIPACK_H
#define GAME_SERVER_ENTITIES_MUNIPACK_H

#include <game/server/entity.h>

class CMunipack : public CEntity
{
public:
	CMunipack(CGameWorld *pGameWorld, int Mode = 0);

	virtual void WeaponSwitch();
	virtual void FallDown();
	virtual void Tick();
	virtual void Snap(int SnappingClient);

private:
	int m_Type;
	int m_Subtype;
	int m_SpawnTick;
	int m_Buffer;
	int m_Mode;
	int Waiting;
	vec2 m_Vel;
};

#endif
