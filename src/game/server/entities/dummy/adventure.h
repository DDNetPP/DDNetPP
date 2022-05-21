#ifndef GAME_SERVER_ENTITIES_DUMMY_ADVENTURE_H
#define GAME_SERVER_ENTITIES_DUMMY_ADVENTURE_H

#include "dummybase.h"

class CDummyAdventure : public CDummyBase
{
public:
	CDummyAdventure(class CCharacter *pChr, class CPlayer *pPlayer);
	virtual void OnTick();
	virtual ~CDummyAdventure(){};
};

#endif
