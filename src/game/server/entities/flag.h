/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#ifndef GAME_SERVER_ENTITIES_FLAG_H
#define GAME_SERVER_ENTITIES_FLAG_H

#include <engine/shared/protocol.h>
#include <game/server/entities/character.h>
#include <game/server/entity.h>

class CFlag : public CEntity
{
	CCharacter *m_pCarryingCharacter;
	CCharacter *m_pLastCarryingCharacter;

public:
	static constexpr float ms_PhysSize = 28.0f;

	vec2 m_Vel;
	vec2 m_StandPos;

	int m_Team;
	bool m_AtStand = true;
	int m_DropTick;
	int m_DropFreezeTick;
	int m_GrabTick;

	CFlag(CGameWorld *pGameWorld, int Team);

	void Reset() override;
	void TickPaused() override;
	void Snap(int SnappingClient) override;

	CCharacter *GetCarrier() const { return m_pCarryingCharacter; }
	CCharacter *GetLastCarrier() const { return m_pLastCarryingCharacter; }
	void SetCarrier(CCharacter *pChr) { m_pCarryingCharacter = pChr; }
	void SetLastCarrier(CCharacter *pChr) { m_pLastCarryingCharacter = pChr; }

	int m_TuneZone;

	// ddnet++
	bool m_IsGrounded;
};

#endif
