// ddnet++ extension of ddnets CSaveTee
// store character data including ddnet++ state

#ifndef GAME_SERVER_MINIGAMES_SAVE_DDPP_H
#define GAME_SERVER_MINIGAMES_SAVE_DDPP_H

#include <generated/protocol.h>

#include <cstdint>

class CCharacter;

class CSaveTeeDDPP
{
public:
	void Save(CCharacter *pChr);
	void Load(CCharacter *pChr);

private:
	int m_aWeaponsBackup[NUM_WEAPONS][2];
	bool m_WeaponsBackupped;
	int64_t m_AliveSince;
	int m_survivexpvalue;
	bool m_DDPP_Finished;
	bool m_Rainbow;
	bool m_Bloody;
	bool m_StrongBloody;
	bool m_WaveBloody;
	bool m_WaveBloodyGrow;
	int m_WaveBloodyStrength;
	bool m_Atom;
	bool m_Trail;
	bool m_autospreadgun;
	bool m_ninjasteam;
	bool m_RandomCosmetics;
	bool m_HomingMissile;
};

#endif
