/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#ifndef GAME_SERVER_ENTITIES_FLAG_H
#define GAME_SERVER_ENTITIES_FLAG_H

#include <game/server/entity.h>

// TODO: use -2 and -3 instead and update all checks from upstream to use > -1 instead of != -1
// m_HookedPlayer != -1
// m_HookedPlayer > -1
// m_LastHookedPlayer as well and probably others
#define FLAG_RED 99
#define FLAG_BLUE 98

class CFlag : public CEntity
{
public:
	static constexpr float ms_PhysSize = 28.0f;
	;
	CCharacter *m_pCarryingCharacter;
	CCharacter *m_pLastCarryingCharacter;

	vec2 m_Vel;
	vec2 m_StandPos;

	int m_Team;
	int m_AtStand;
	int m_DropTick;
	int m_DropFreezeTick;
	int m_GrabTick;

	CFlag(CGameWorld *pGameWorld, int Team);

	virtual void Reset() override;
	virtual void TickPaused() override;
	virtual void Snap(int SnappingClient) override;

	CCharacter *GetCarrier() const { return m_pCarryingCharacter; }

	int m_TuneZone;
};

#endif
