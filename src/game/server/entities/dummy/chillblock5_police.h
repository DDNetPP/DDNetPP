#ifndef GAME_SERVER_ENTITIES_DUMMY_CHILLBLOCK5_POLICE_H
#define GAME_SERVER_ENTITIES_DUMMY_CHILLBLOCK5_POLICE_H

#include "dummybase.h"

class CDummyChillBlock5Police : public CDummyBase
{
public:
	CDummyChillBlock5Police(class CCharacter *pChr, class CPlayer *pPlayer);
	virtual void OnTick();
	virtual ~CDummyChillBlock5Police(){};

private:
	bool m_Dummy_GotStuck;
	bool m_Dummy_GetSpeed;
	bool m_Dummy_SpawnAnimation;
	bool m_Dummy_ClosestPolice;
	int m_Dummy_SpawnAnimation_delay;
	int m_Dummy_dmm31;
	int m_Dummy_AttackMode;
};

#endif
