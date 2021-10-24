/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#ifndef GAME_SERVER_ENTITIES_GUN_H
#define GAME_SERVER_ENTITIES_GUN_H

#include <game/gamecore.h>
#include <game/server/entity.h>

class CCharacter;

class CGun : public CEntity
{
	int m_EvalTick;

	vec2 m_Core;
	bool m_Freeze;
	bool m_Explosive;

	void Fire();
	int m_LastFire;

public:
	CGun(CGameWorld *pGameWorld, vec2 Pos, bool Freeze, bool Explosive, int Layer = 0, int Number = 0);

	virtual void Reset();
	virtual void Tick();
	virtual void Snap(int SnappingClient);
};

#endif // GAME_SERVER_ENTITIES_GUN_H
