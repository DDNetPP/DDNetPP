#ifndef GAME_SERVER_ENTITIES_DUMMY_ADVENTURE_H
#define GAME_SERVER_ENTITIES_DUMMY_ADVENTURE_H

#include "dummybase.h"

class CDummyAdventure : public CDummyBase
{
public:
	CDummyAdventure(class CPlayer *pPlayer);
	void OnTick() override;
	~CDummyAdventure() override = default;
	const char *ModeStr() override { return "Adventure"; }

	bool m_IsAimbot = false;
};

#endif
