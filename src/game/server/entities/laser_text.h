#ifndef GAME_SERVER_ENTITIES_LASER_TEXT_H
#define GAME_SERVER_ENTITIES_LASER_TEXT_H

/*
    Inspired by fngs lasertext / loltext
    https://github.com/Jupeyy/teeworlds-fng2-mod/blob/07ac6046c37eba552d5d0b59bcc7bf35904b3e4f/src/game/server/laser_text.cpp
    https://github.com/fstd/teeworlds/blob/edceb914f47f3fb6407a85f8cd01060bf79b847a/src/game/server/entities/loltext.cpp
*/

#include <game/server/entity.h>

class CLolPlasma : public CEntity
{
public:
	CLolPlasma(CGameWorld *pGameWorld, vec2 Pos) :
		CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER, Pos, 0)
	{
	}
	int GetIdWrap() { return GetId(); }
};

class CLaserText : public CEntity
{
public:
	CLaserText(CGameWorld *pGameWorld, vec2 Pos, int Owner, int pAliveTicks, const char *pText, int pTextLen);
	virtual ~CLaserText();

	virtual void Reset() override;
	virtual void Tick() override;
	virtual void TickPaused() override;
	virtual void Snap(int SnappingClient) override;

private:
	void CreateLetters();
	void CreateLetter(unsigned char Ascii, int Offset);

	int m_Owner;

	int m_AliveTicks;
	int m_CurTicks;
	int m_StartTick;

	char *m_Text;
	int m_TextLen;

	CLolPlasma **m_LolPlasmas;
	/*
		variable: m_MaxPlasmas

		Prepared expected plasmas
	*/
	int m_MaxPlasmas;
	/*
		variable: m_NumPlasma

		Actually created entitys should not exceed m_MaxPlasmas
	*/
	int m_NumPlasma;
};

#endif
