#ifndef GAME_SERVER_ENTITIES_DUMMY_BLMAPV5_LOWER_BLOCKER_H
#define GAME_SERVER_ENTITIES_DUMMY_BLMAPV5_LOWER_BLOCKER_H

#include "dummybase.h"

class CDummyBlmapV5LowerBlocker : public CDummyBase
{
public:
	CDummyBlmapV5LowerBlocker(class CPlayer *pPlayer);
	void OnTick() override;
	void OnDeath() override;
	~CDummyBlmapV5LowerBlocker() override = default;
	const char *ModeStr() override { return "BlmapV5 lower"; }

private:
	int m_Angry = 0;
	bool m_RjFailed = false;
	bool m_PanicHook = false;
};

#endif
