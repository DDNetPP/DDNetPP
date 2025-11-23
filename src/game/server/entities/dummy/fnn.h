#ifndef GAME_SERVER_ENTITIES_DUMMY_FNN_H
#define GAME_SERVER_ENTITIES_DUMMY_FNN_H

#include "dummybase.h"

#define FNN_MOVE_LEN 32768

class CDummyFNN : public CDummyBase
{
public:
	CDummyFNN(class CPlayer *pPlayer);
	virtual void OnTick() override;
	virtual void OnDeath() override;
	virtual void TakeDamage(vec2 Force, int Dmg, int From, int Weapon) override;
	virtual ~CDummyFNN() {}
	const char *ModeStr() override { return "FNN"; }

	int m_RandDirection = 0;

	bool m_Dummy_nn_ready;
	bool m_Dummy_nn_touched_by_humans;
	int m_aRecMove[FNN_MOVE_LEN];
	int m_FNN_ticks_loaded_run;
	int m_FNN_CurrentMoveIndex;

private:
	//dummymode 25 FNN vars
	bool m_Dummy_nn_stop;
	int m_Dummy_nn_ready_time;
	int m_FNN_start_servertick;
	int m_FNN_stop_servertick;
	vec2 m_StartPos;
};

#endif
