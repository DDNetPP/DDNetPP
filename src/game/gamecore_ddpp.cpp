/* DDNet++ gamecore */

#include "gamecore.h"

#include <engine/shared/config.h>
#include <engine/server/server.h>

void CCharacterCore::DDPPWrite(CNetObj_CharacterCore *pObjCore)
{
	if(m_HookedPlayer == 98 || m_HookedPlayer == 99)
		pObjCore->m_HookedPlayer = -1;
	else
        pObjCore->m_HookedPlayer = m_HookedPlayer;
}

void CCharacterCore::DDPPRead(const CNetObj_CharacterCore *pObjCore)
{
	if (m_HookedPlayer == 98 || m_HookedPlayer == 99)
    {
        // pass
    }
	else
	    m_HookedPlayer = pObjCore->m_HookedPlayer;
}
