#ifndef GAME_SERVER_ENTITIES_DUMMY_CHILLBLOCK5_RACE_H
#define GAME_SERVER_ENTITIES_DUMMY_CHILLBLOCK5_RACE_H

#include "dummybase.h"

#include <base/vmath.h>

class CDummyChillBlock5Race : public CDummyBase
{
public:
	CDummyChillBlock5Race(class CPlayer *pPlayer);
	void OnTick() override;
	void OnDeath() override;
	~CDummyChillBlock5Race() override = default;
	const char *ModeStr() override { return "ChillBlock5 Race"; }

private:
	int m_DummyHelpM8BeforeHfHook; //yep a bool int timer
	bool m_DummyHelpEmergency; //activate if boot falls of platform while helping
	bool m_DummyHelpNoEmergency; //this is just used to check if the bot planned to be in this situation. this bool is just used for not activating m_Dummy_help_emergency
	bool m_DummyHookMateAfterHammer; //after unfreezing a mate with hammer hold him with hook
	bool m_DummyHelpBeforeFly; //to help at the mate if he gets freeze before the hammerfly started
	bool m_Dummy2pPanicWhileHelping; //if the bot falls of the platform while helping at the 2p part
	bool m_DummyPanicBalance; //hammerhit part struggle -> balance
	bool m_DummyMateFailed; //a �var which toggles the dummy_2p_state value -2
	bool m_DummyHhHook; //check for hook in hammerhit at end
	bool m_DummyCollectedWeapons; //ob er nochmal zu den waffen hochfliegen muss
	bool m_DummyMateCollectedWeapons; //ob auch der race mate waffen hat
	bool m_DummyRjumped2; //ob der dummy grad den rj2 hinter sich hat
	bool m_DummyDdHelphook; //just a helphook bool ... used for start and stoop hooking while helping at the dummydrag part
	bool m_Dummy2pHook; //same as m_Dummy_dd_hook but in new sys
	int m_Dummy2pState; //Maybe cool stuff coming with it
	int m_DummyMode23; //yes dummymode23 has his own modes o.O
	int m_DummyNothingHappensCounter; // counts all the nonaction lol
	int m_DummyPanicWeapon; // if the bot has panic (nothing happens -> panic mate could get bored)  change the wepaon to this var value
	int m_DummySentChatMsg; // 0 == noMsgDisTick 1 == MsgDisTick              [to send a chat message just 1 time]
	int m_DummyMateHelpMode; //how the bot should help
	int m_DummyMovementMode23; //a movement mode for mode23
	bool m_DummyFreezed;
	int m_EmoteTickNext;
};

#endif
