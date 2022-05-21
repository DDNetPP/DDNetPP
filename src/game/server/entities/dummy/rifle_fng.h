#ifndef GAME_SERVER_ENTITIES_DUMMY_RIFLE_FNG_H
#define GAME_SERVER_ENTITIES_DUMMY_RIFLE_FNG_H

#include "dummybase.h"

class CDummyRifleFng : public CDummyBase
{
public:
	CDummyRifleFng(class CCharacter *pChr, class CPlayer *pPlayer);
	virtual void OnTick();
	virtual ~CDummyRifleFng(){};
};

#endif
