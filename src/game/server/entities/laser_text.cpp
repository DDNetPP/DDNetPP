/*
    Inspired by fngs lasertext / loltext
    https://github.com/Jupeyy/teeworlds-fng2-mod/blob/07ac6046c37eba552d5d0b59bcc7bf35904b3e4f/src/game/server/laser_text.cpp
    https://github.com/fstd/teeworlds/blob/edceb914f47f3fb6407a85f8cd01060bf79b847a/src/game/server/entities/loltext.cpp
*/

#include "laser_text.h"

#include <base/system.h>

#include <game/server/gamecontext.h>

CLaserText::CLaserText(CGameWorld *pGameWorld, vec2 Pos, int AliveTicks, const char *pText) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER, Pos)
{
	m_Pos = Pos;
	GameWorld()->InsertEntity(this);

	m_CurTicks = Server()->Tick();
	m_StartTick = Server()->Tick();
	m_AliveTicks = AliveTicks;

	str_copy(m_aText, pText, sizeof(m_aText));
	m_TextLen = str_length(m_aText);

	m_NumPlasma = 0;
	m_MaxPlasmas = 0;
	for(int i = 0; i < m_TextLen; ++i)
	{
		unsigned char Letter = (unsigned char)m_aText[i];
		for(int x = 0; x < ASCII_CHAR_WIDTH; x++)
			for(int y = 0; y < ASCII_CHAR_HEIGHT; y++)
				if(gs_aaaAsciiTable[Letter][y][x])
					m_MaxPlasmas++;
	}
	// dbg_msg("laser_text", "New lasertext letters=%d plasmas=%d text=%s", m_TextLen, m_MaxPlasmas, m_Text);
	m_LolPlasmas = new CLolPlasma *[m_MaxPlasmas];

	CreateLetters();
}

CLaserText::~CLaserText()
{
	for(int i = 0; i < m_MaxPlasmas; ++i)
	{
		delete m_LolPlasmas[i];
	}
	delete[] m_LolPlasmas;
}

void CLaserText::CreateLetter(unsigned char Ascii, int Offset)
{
	// range ensured by datatype "unsigned char"
	// if(Ascii < 0 || Ascii > ASCII_TABLE_SIZE)
	// 	return;

	// dbg_msg("laser_text", "create letter %d at offset %d", Ascii, Offset);

	const int Spacing = 20;
	const int LetterWidth = (Spacing * ASCII_CHAR_WIDTH) + 5;
	vec2 Origin = vec2(GetPos().x + (Offset * LetterWidth), GetPos().y);
	for(int x = 0; x < ASCII_CHAR_WIDTH; x++)
	{
		for(int y = 0; y < ASCII_CHAR_HEIGHT; y++)
		{
			if(!gs_aaaAsciiTable[Ascii][y][x])
				continue;

			m_LolPlasmas[m_NumPlasma] = new CLolPlasma(
				GameWorld(),
				vec2(Origin.x + (x * Spacing), Origin.y + (y * Spacing)));
			m_NumPlasma++;
		}
	}
}

void CLaserText::CreateLetters()
{
	int Offset = 0;
	for(int i = 0; i < m_TextLen; ++i)
	{
		unsigned char Letter = (unsigned char)m_aText[i];
		CreateLetter(Letter, Offset++);
	}
	if(m_NumPlasma != m_MaxPlasmas)
	{
		dbg_msg("laser_text", "m_NumPlasma=%d does not match m_MaxPlasmas=%d", m_NumPlasma, m_MaxPlasmas);
		exit(1);
	}
}

void CLaserText::Reset()
{
	m_MarkedForDestroy = true;
}

void CLaserText::Tick()
{
	if(++m_CurTicks - m_StartTick > m_AliveTicks)
		Reset();
}

void CLaserText::TickPaused()
{
}

void CLaserText::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	for(int i = 0; i < m_MaxPlasmas; ++i)
	{
		CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, m_LolPlasmas[i]->GetIdWrap(), sizeof(CNetObj_Laser)));
		if(!pObj)
			return;

		pObj->m_X = m_LolPlasmas[i]->GetPos().x;
		pObj->m_Y = m_LolPlasmas[i]->GetPos().y;
		pObj->m_FromX = m_LolPlasmas[i]->GetPos().x;
		pObj->m_FromY = m_LolPlasmas[i]->GetPos().y;
		pObj->m_StartTick = Server()->Tick();
	}
}
