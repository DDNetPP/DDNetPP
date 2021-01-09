// made by fokkonaut and ChillerDragon

#ifndef GAME_SERVER_ENTITIES_DUMMY_BLMAPCHILL_POLICE_H
#define GAME_SERVER_ENTITIES_DUMMY_BLMAPCHILL_POLICE_H

#include <base/vmath.h>
#include "dummybase.h"

class CDummyBlmapChillPolice : public CDummyBase {
public:
    CDummyBlmapChillPolice(class CCharacter *pChr, class CPlayer *pPlayer);

    void OnTick();

    int m_LovedX;
    int m_LovedY;
    int m_LowerPanic;
    int m_Speed;
    int m_HelpMode;
    int m_GrenadeJump;
    int m_SpawnTeleporter;

    bool m_IsHelpHook;
    bool m_IsClosestPolice;
    bool m_DidRocketjump;
    bool m_HasTouchedGround;
    bool m_HasAlreadyBeenHere;
    bool m_HasStartGrenade;
    bool m_IsDJUsed;
    bool m_HashReachedCinemaEntrance;
    bool m_GetSpeed;
};

#endif
