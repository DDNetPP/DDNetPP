#ifndef GAME_SERVER_ENTITIES_DUMMY_BLOCKWAVE_H
#define GAME_SERVER_ENTITIES_DUMMY_BLOCKWAVE_H

#include "dummybase.h"

class CDummyBlockWave : public CDummyBase
{
public:
	CDummyBlockWave(class CPlayer *pPlayer);
	virtual void OnTick() override;
	virtual ~CDummyBlockWave() {}
	const char *ModeStr() override { return "BlockWave"; }
};

#endif
