#ifndef GAME_SERVER_ENTITIES_DUMMY_BLMAPV5_UPPER_BLOCKER_H
#define GAME_SERVER_ENTITIES_DUMMY_BLMAPV5_UPPER_BLOCKER_H

#include "dummybase.h"

class CDummyBlmapV5UpperBlocker : public CDummyBase
{
public:
	CDummyBlmapV5UpperBlocker(class CPlayer *pPlayer);
	void OnTick() override;
	void OnDeath() override;
	~CDummyBlmapV5UpperBlocker() override = default;
	const char *ModeStr() override { return "BlmapV5 upper"; }

private:
	bool m_MoveLeft;
};

#endif
