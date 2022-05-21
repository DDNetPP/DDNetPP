#ifndef GAME_SERVER_ENTITIES_DUMMY_SAMPLE_H
#define GAME_SERVER_ENTITIES_DUMMY_SAMPLE_H

#include "dummybase.h"

class CDummySample : public CDummyBase
{
public:
	CDummySample(class CCharacter *pChr, class CPlayer *pPlayer);
	virtual void OnTick();
	virtual ~CDummySample(){};
};

#endif
