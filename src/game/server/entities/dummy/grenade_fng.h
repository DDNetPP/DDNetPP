#ifndef GAME_SERVER_ENTITIES_DUMMY_GRENADE_FNG_H
#define GAME_SERVER_ENTITIES_DUMMY_GRENADE_FNG_H

#include "dummybase.h"

class CDummyGrenadeFng : public CDummyBase
{
public:
	CDummyGrenadeFng(class CPlayer *pPlayer);
	void OnTick() override;
	~CDummyGrenadeFng() override = default;
	const char *ModeStr() override { return "Grenade FNG"; }
};

#endif
