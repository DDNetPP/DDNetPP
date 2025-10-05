#ifndef GAME_SERVER_ENTITIES_DUMMY_CHILLBLOCK5_BLOCKER_TRYHARD_H
#define GAME_SERVER_ENTITIES_DUMMY_CHILLBLOCK5_BLOCKER_TRYHARD_H

#include "dummybase.h"

#include <base/vmath.h>

class CDummyChillBlock5BlockerTryHard : public CDummyBase
{
public:
	CDummyChillBlock5BlockerTryHard(class CPlayer *pPlayer);
	virtual void OnTick() override;
	virtual void OnDeath() override;
	virtual ~CDummyChillBlock5BlockerTryHard(){};
	const char *ModeStr() override { return "ChillBlock5 Blocker TryHard"; }

private:
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

	int m_EmoteTickNext;
};

#endif
