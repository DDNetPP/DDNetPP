#ifndef GAME_SERVER_ENTITIES_DUMMY_SHOPBOT_H
#define GAME_SERVER_ENTITIES_DUMMY_SHOPBOT_H

#include "dummybase.h"

class CDummyShopBot : public CDummyBase
{
public:
	CDummyShopBot(class CCharacter *pChr, class CPlayer *pPlayer);
	virtual void OnTick() override;
	virtual ~CDummyShopBot(){};
	const char *ModeStr() override { return "Shopbot"; }
};

#endif
