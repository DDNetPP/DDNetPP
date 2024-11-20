#ifndef GAME_SERVER_ENTITIES_DUMMY_QUEST_H
#define GAME_SERVER_ENTITIES_DUMMY_QUEST_H

#include "dummybase.h"

class CDummyQuest : public CDummyBase
{
	bool m_IsAimbot = false;

public:
	CDummyQuest(class CPlayer *pPlayer);
	void OnTick() override;
	~CDummyQuest() override = default;
	const char *ModeStr() override { return "Quest"; }
};

#endif
