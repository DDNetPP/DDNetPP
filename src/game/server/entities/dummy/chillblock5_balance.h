#ifndef GAME_SERVER_ENTITIES_DUMMY_CHILLBLOCK5_BALANCE_H
#define GAME_SERVER_ENTITIES_DUMMY_CHILLBLOCK5_BALANCE_H

#include "dummybase.h"
#include <base/vmath.h>

class CDummyChillBlock5Balance : public CDummyBase
{
public:
	CDummyChillBlock5Balance(class CCharacter *pChr, class CPlayer *pPlayer);
	virtual void OnTick();
	virtual ~CDummyChillBlock5Balance(){};
};

#endif
