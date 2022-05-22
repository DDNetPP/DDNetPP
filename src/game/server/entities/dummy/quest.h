#ifndef GAME_SERVER_ENTITIES_DUMMY_QUEST_H
#define GAME_SERVER_ENTITIES_DUMMY_QUEST_H

#include "dummybase.h"

class CDummyQuest : public CDummyBase
{
public:
	CDummyQuest(class CPlayer *pPlayer);
	virtual void OnTick() override;
	virtual ~CDummyQuest(){};
	const char *ModeStr() override { return "Quest"; }
};

#endif
