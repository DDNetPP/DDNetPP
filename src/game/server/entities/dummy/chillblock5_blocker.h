#ifndef GAME_SERVER_ENTITIES_DUMMY_CHILLBLOCK5_BLOCKER_H
#define GAME_SERVER_ENTITIES_DUMMY_CHILLBLOCK5_BLOCKER_H

#include "dummybase.h"

#include <base/vmath.h>

class CDummyChillBlock5Blocker : public CDummyBase
{
public:
	CDummyChillBlock5Blocker(class CPlayer *pPlayer);
	virtual void OnTick() override;
	virtual void OnDeath() override;
	virtual ~CDummyChillBlock5Blocker(){};
	const char *ModeStr() override { return "ChillBlock5 Blocker"; }

private:
	//dummymode 29 vars (ChillBlock5 blocker)
	int m_DummyFreezeBlockTrick;
	int m_Dummy_trick_panic_check_delay;
	bool m_Dummy_start_hook;
	bool m_Dummy_speedright; //used to go right from the left side of the freeze if there is enoigh speed
	bool m_Dummy_trick3_panic_check;
	bool m_Dummy_trick3_panic;
	bool m_Dummy_trick3_start_count;
	bool m_Dummy_trick3_panic_left;
	bool m_Dummy_trick4_hasstartpos;
	bool m_Dummy_lock_bored; //tricky way to keep the bored bool activatet
	bool m_Dummy_doBalance;
	bool m_Dummy_AttackedOnSpawn;
	bool m_Dummy_bored_cuz_nothing_happens;
	bool m_Dummy_movement_to_block_area_style_window; //yep dis is too long
	bool m_Dummy_planned_movment; // belongs to:   m_Dummy_movement_to_block_area_style_window

	//notstand vars fuer mode 18 (also used in 29)
	bool m_Dummy_jumped; //gesprungen wenn der notstand ausgetufen wird
	bool m_Dummy_hooked; //gehookt wenn der notstand ausgerufen wird
	bool m_Dummy_moved_left; //nach links gelaufen wenn der notstand ausgerufen wird
	bool m_Dummy_hook_delay; //hook delay wenn der notstand ausgerufen wurde
	bool m_Dummy_ruled; //ob der dummy in diesem leben schonmal am ruler spot war
	bool m_Dummy_pushing; //ob er jemand grad beim wayblocken aus seinem wb spot schiebt
	bool m_Dummy_emergency; // Notsand
	bool m_Dummy_wb_hooked; //ob er grad vom wayblockspot wen wayblockig hookt
	bool m_Dummy_left_freeze_full; //wenn jemand schon in die linke freeze wand geblockt wurde
	bool m_Dummy_happy; //wenn er sich auf seinem lieblings wb spot befindet
	bool m_Dummy_get_speed; //im tunnel anlauf holen wenn ausgebremst                     WARNING THIS VAR IS ALSO USED IN DUMMYMODE == 26
	bool m_Dummy_bored; //wenn dem bot langweilig wird wechselt er die wayblock taktik
	bool m_Dummy_special_defend; //dummy_mode18 mode bool
	bool m_Dummy_special_defend_attack; //sub var f�r m_Dummy_special_defend die abfr�gt ob der bot schon angreifen soll

	int m_Dummy_bored_counter; //z�hl hoch bis dem dummy lw wird

	int m_Dummy_mode18; //yes dummymode18 has his own modes o.O
	//bool mode18_main_init;              //yep one of the randomesteztes booleans in ze world
};

#endif
