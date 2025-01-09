#ifndef GAME_SERVER_ENTITIES_DUMMY_BLMAPV3_ARENA_H
#define GAME_SERVER_ENTITIES_DUMMY_BLMAPV3_ARENA_H

#include "dummybase.h"

#define V3_OFFSET_X -11 * 32
// ChillBlock5 v014 and earlier needs 0
// before that it was 277

#define V3_OFFSET_Y 0 * 32
// before that it was 48

class CDummyBlmapV3Arena : public CDummyBase
{
public:
	CDummyBlmapV3Arena(class CPlayer *pPlayer);
	virtual void OnTick() override;
	virtual ~CDummyBlmapV3Arena(){};
	const char *ModeStr() override { return "BlmapV3 area"; }
};

#endif
