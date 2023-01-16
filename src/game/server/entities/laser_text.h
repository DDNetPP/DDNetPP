/*
    Inspired by fngs lasertext / loltext
    https://github.com/Jupeyy/teeworlds-fng2-mod/blob/07ac6046c37eba552d5d0b59bcc7bf35904b3e4f/src/game/server/laser_text.cpp
    https://github.com/fstd/teeworlds/blob/edceb914f47f3fb6407a85f8cd01060bf79b847a/src/game/server/entities/loltext.cpp
*/

#ifndef GAME_SERVER_ENTITIES_LASER_H
#define GAME_SERVER_ENTITIES_LASER_H

#include <game/server/entity.h>

class CLaserChar : public CEntity
{
public:
	CLaserChar(CGameWorld *pGameWorld, vec2 Pos) :
		CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER, Pos, 0)
	{
	}
};

class CLaserText : public CEntity
{
public:
	CLaserText(CGameWorld *pGameWorld, vec2 Pos, int Owner, int pAliveTicks, char *pText, int pTextLen);

	virtual void Reset() override;
	virtual void Tick() override;
	virtual void TickPaused() override;
	virtual void Snap(int SnappingClient) override;

private:
	int m_Owner;

	int m_AliveTicks;
	int m_CurTicks;
	int m_StartTick;

	CLaserChar **m_Chars;
	int m_CharNum;
};

#endif