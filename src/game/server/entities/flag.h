/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#ifndef GAME_SERVER_ENTITIES_FLAG_H
#define GAME_SERVER_ENTITIES_FLAG_H

#include <base/log.h>

#include <engine/shared/protocol.h>

#include <generated/protocol.h>

#include <game/server/entities/character.h>
#include <game/server/entity.h>

class CFlag : public CEntity
{
	CCharacter *m_pCarryingCharacter = nullptr;
	CCharacter *m_pLastCarryingCharacter = nullptr;

public:
	static constexpr float ms_PhysSize = 28.0f;

	vec2 m_Vel = vec2(0, 0);
	vec2 m_StandPos = vec2(0, 0);

	int m_Team = TEAM_RED;
	bool m_AtStand = true;
	int m_DropTick = 0;
	int m_DropFreezeTick = 0;
	int m_GrabTick = 0;

	CFlag(CGameWorld *pGameWorld, int Team);

	void Reset() override;
	void TickPaused() override;
	void Snap(int SnappingClient) override;

	CCharacter *GetCarrier() const { return m_pCarryingCharacter; }
	CCharacter *GetLastCarrier() const { return m_pLastCarryingCharacter; }
	void SetCarrier(CCharacter *pChr)
	{
		m_pCarryingCharacter = pChr;
		log_info("game", "set %s flag carrier to %p", m_Team == TEAM_RED ? "red" : "blue", pChr);
	}
	void SetLastCarrier(CCharacter *pChr) { m_pLastCarryingCharacter = pChr; }

	int m_TuneZone = 0;

	// ddnet++
	bool m_IsGrounded = false;
};

#endif
