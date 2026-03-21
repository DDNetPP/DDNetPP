#ifndef GAME_SERVER_ENTITIES_DUMMY_SHOPBOT_H
#define GAME_SERVER_ENTITIES_DUMMY_SHOPBOT_H

#include "dummybase.h"

class CDummyShopBot : public CDummyBase
{
public:
	CDummyShopBot(class CPlayer *pPlayer);
	void OnTick() override;
	~CDummyShopBot() override = default;
	const char *ModeStr() override { return "Shopbot"; }
};

#endif
