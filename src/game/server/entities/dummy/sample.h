#ifndef GAME_SERVER_ENTITIES_DUMMY_SAMPLE_H
#define GAME_SERVER_ENTITIES_DUMMY_SAMPLE_H

#include "dummybase.h"

class CDummySample : public CDummyBase
{
public:
	CDummySample(class CPlayer *pPlayer);
	virtual void OnTick() override;
	virtual ~CDummySample() {}
	const char *ModeStr() override { return "Sample"; }
};

#endif
