#ifndef GAME_SERVER_ENTITIES_DUMMY_CHILLBLOCK5_RACE_H
#define GAME_SERVER_ENTITIES_DUMMY_CHILLBLOCK5_RACE_H

#include "dummybase.h"

#include <base/vmath.h>

class CDummyChillBlock5Race : public CDummyBase
{
public:
	CDummyChillBlock5Race(class CPlayer *pPlayer);
	virtual void OnTick() override;
	virtual void OnDeath() override;
	virtual ~CDummyChillBlock5Race() {}
	const char *ModeStr() override { return "ChillBlock5 Race"; }

private:
	int m_Dummy_help_m8_before_hf_hook; //yep a bool int timer
	bool m_Dummy_help_emergency; //activate if boot falls of platform while helping
	bool m_Dummy_help_no_emergency; //this is just used to check if the bot planned to be in this situation. this bool is just used for not activating m_Dummy_help_emergency
	bool m_Dummy_hook_mate_after_hammer; //after unfreezing a mate with hammer hold him with hook
	bool m_Dummy_help_before_fly; //to help at the mate if he gets freeze before the hammerfly started
	bool m_Dummy_2p_panic_while_helping; //if the bot falls of the platform while helping at the 2p part
	bool m_Dummy_panic_balance; //hammerhit part struggle -> balance
	bool m_Dummy_mate_failed; //a �var which toggles the dummy_2p_state value -2
	bool m_Dummy_hh_hook; //check for hook in hammerhit at end
	bool m_Dummy_collected_weapons; //ob er nochmal zu den waffen hochfliegen muss
	bool m_Dummy_mate_collected_weapons; //ob auch der race mate waffen hat
	bool m_Dummy_rjumped2; //ob der dummy grad den rj2 hinter sich hat
	bool m_Dummy_dd_helphook; //just a helphook bool ... used for start and stoop hooking while helping at the dummydrag part
	bool m_Dummy_2p_hook; //same as m_Dummy_dd_hook but in new sys
	int m_Dummy_2p_state; //Maybe cool stuff coming with it
	int m_Dummy_mode23; //yes dummymode23 has his own modes o.O
	int m_Dummy_nothing_happens_counter; // counts all the nonaction lol
	int m_Dummy_panic_weapon; // if the bot has panic (nothing happens -> panic mate could get bored)  change the wepaon to this var value
	int m_Dummy_sent_chat_msg; // 0 == noMsgDisTick 1 == MsgDisTick              [to send a chat message just 1 time]
	int m_Dummy_mate_help_mode; //how the bot should help
	int m_Dummy_movement_mode23; //a movement mode for mode23
	bool m_DummyFreezed;
	int m_EmoteTickNext;
};

#endif
