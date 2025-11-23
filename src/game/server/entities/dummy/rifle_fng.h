#ifndef GAME_SERVER_ENTITIES_DUMMY_RIFLE_FNG_H
#define GAME_SERVER_ENTITIES_DUMMY_RIFLE_FNG_H

#include "dummybase.h"

class CDummyRifleFng : public CDummyBase
{
public:
	CDummyRifleFng(class CPlayer *pPlayer);
	virtual void OnTick() override;
	virtual ~CDummyRifleFng() {}
	const char *ModeStr() override { return "Rifle FNG"; }
};

#endif
