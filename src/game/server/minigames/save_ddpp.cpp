// ddnet++ extension of ddnets CSaveTee
// store character data including ddnet++ state

#include "save_ddpp.h"

#include <game/server/entities/character.h>

void CSaveTeeDDPP::Save(CCharacter *pChr)
{
	for(int i = 0; i < NUM_WEAPONS; i++)
		for(int k = 0; k < 2; k++)
			m_aWeaponsBackup[i][k] = pChr->m_aWeaponsBackup[i][k];
	m_WeaponsBackupped = pChr->m_WeaponsBackupped;
	m_AliveSince = pChr->m_AliveSince;
	m_survivexpvalue = pChr->m_survivexpvalue;
	m_DDPP_Finished = pChr->m_DDPP_Finished;
	m_Rainbow = pChr->m_Rainbow;
	m_Bloody = pChr->m_Bloody;
	m_StrongBloody = pChr->m_StrongBloody;
	m_WaveBloody = pChr->m_WaveBloody;
	m_WaveBloodyGrow = pChr->m_WaveBloodyGrow;
	m_WaveBloodyStrength = pChr->m_WaveBloodyStrength;
	m_Atom = pChr->m_Atom;
	m_Trail = pChr->m_Trail;
	m_autospreadgun = pChr->m_autospreadgun;
	m_ninjasteam = pChr->m_ninjasteam;
	m_RandomCosmetics = pChr->m_RandomCosmetics;
	m_HomingMissile = pChr->m_HomingMissile;
}

void CSaveTeeDDPP::Load(CCharacter *pChr)
{
	for(int i = 0; i < NUM_WEAPONS; i++)
		for(int k = 0; k < 2; k++)
			pChr->m_aWeaponsBackup[i][k] = m_aWeaponsBackup[i][k];
	pChr->m_WeaponsBackupped = m_WeaponsBackupped;
	pChr->m_AliveSince = m_AliveSince;
	pChr->m_survivexpvalue = m_survivexpvalue;
	pChr->m_DDPP_Finished = m_DDPP_Finished;
	pChr->m_Rainbow = m_Rainbow;
	pChr->m_Bloody = m_Bloody;
	pChr->m_StrongBloody = m_StrongBloody;
	pChr->m_WaveBloody = m_WaveBloody;
	pChr->m_WaveBloodyGrow = m_WaveBloodyGrow;
	pChr->m_WaveBloodyStrength = m_WaveBloodyStrength;
	pChr->m_Atom = m_Atom;
	pChr->m_Trail = m_Trail;
	pChr->m_autospreadgun = m_autospreadgun;
	pChr->m_ninjasteam = m_ninjasteam;
	pChr->m_RandomCosmetics = m_RandomCosmetics;
	pChr->m_HomingMissile = m_HomingMissile;
}
