#ifndef GAME_SERVER_ENTITIES_DUMMY_CTF5_PVP_H
#define GAME_SERVER_ENTITIES_DUMMY_CTF5_PVP_H

#include "dummybase.h"

class CDummyCtf5Pvp : public CDummyBase
{
public:
	CDummyCtf5Pvp(class CCharacter *pChr, class CPlayer *pPlayer);
	virtual void OnTick();
	virtual ~CDummyCtf5Pvp(){};
};

#endif
