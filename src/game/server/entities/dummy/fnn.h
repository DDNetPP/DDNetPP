#ifndef GAME_SERVER_ENTITIES_DUMMY_FNN_H
#define GAME_SERVER_ENTITIES_DUMMY_FNN_H

#include "dummybase.h"

#define FNN_MOVE_LEN 32768

class CDummyFNN : public CDummyBase
{
public:
	CDummyFNN(class CPlayer *pPlayer);
	void OnTick() override;
	void OnDeath() override;
	void TakeDamage(vec2 Force, int Dmg, int From, int Weapon) override;
	~CDummyFNN() override = default;
	const char *ModeStr() override { return "FNN"; }

	int m_RandDirection = 0;

	bool m_DummyNnReady;
	bool m_DummyNnTouchedByHumans;
	int m_aRecMove[FNN_MOVE_LEN];
	int m_FnnTicksLoadedRun;
	int m_FnnCurrentMoveIndex;

private:
	//dummymode 25 FNN vars
	bool m_DummyNnStop;
	int m_DummyNnReadyTime;
	int m_FnnStartServertick;
	int m_FnnStopServertick;
	vec2 m_StartPos;
};

#endif
