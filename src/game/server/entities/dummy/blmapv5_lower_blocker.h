#ifndef GAME_SERVER_ENTITIES_DUMMY_BLMAPV5_LOWER_BLOCKER_H
#define GAME_SERVER_ENTITIES_DUMMY_BLMAPV5_LOWER_BLOCKER_H

#include "dummybase.h"

class CDummyBlmapV5LowerBlocker : public CDummyBase
{
public:
	CDummyBlmapV5LowerBlocker(class CPlayer *pPlayer);
	virtual void OnTick() override;
	virtual void OnDeath() override;
	virtual ~CDummyBlmapV5LowerBlocker(){};
	const char *ModeStr() override { return "BlmapV5 lower"; }

private:
	bool m_rj_failed;
	bool m_panic_hook;
	int m_angry;
};

#endif
