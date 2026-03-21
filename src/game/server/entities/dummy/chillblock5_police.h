#ifndef GAME_SERVER_ENTITIES_DUMMY_CHILLBLOCK5_POLICE_H
#define GAME_SERVER_ENTITIES_DUMMY_CHILLBLOCK5_POLICE_H

#include "dummybase.h"

class CDummyChillBlock5Police : public CDummyBase
{
public:
	CDummyChillBlock5Police(class CPlayer *pPlayer);
	void OnTick() override;
	void OnDeath() override;
	~CDummyChillBlock5Police() override = default;
	const char *ModeStr() override { return "ChillBlock5 Police"; }

private:
	bool m_DummyGotStuck;
	bool m_DummyGetSpeed;
	bool m_DummySpawnAnimation;
	bool m_DummyClosestPolice;
	int m_DummySpawnAnimationDelay;
	int m_DummyDmm31;
	int m_DummyAttackMode;
};

#endif
