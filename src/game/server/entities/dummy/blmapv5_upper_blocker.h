#ifndef GAME_SERVER_ENTITIES_DUMMY_BLMAPV5_UPPER_BLOCKER_H
#define GAME_SERVER_ENTITIES_DUMMY_BLMAPV5_UPPER_BLOCKER_H

#include "dummybase.h"

class CDummyBlmapV5UpperBlocker : public CDummyBase
{
public:
	CDummyBlmapV5UpperBlocker(class CPlayer *pPlayer);
	virtual void OnTick() override;
	virtual void OnDeath() override;
	virtual ~CDummyBlmapV5UpperBlocker() {}
	const char *ModeStr() override { return "BlmapV5 upper"; }

private:
	bool m_move_left;
};

#endif
