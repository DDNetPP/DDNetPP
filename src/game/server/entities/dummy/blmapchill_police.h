// made by fokkonaut and ChillerDragon

#ifndef GAME_SERVER_ENTITIES_DUMMY_BLMAPCHILL_POLICE_H
#define GAME_SERVER_ENTITIES_DUMMY_BLMAPCHILL_POLICE_H

#include "dummybase.h"
#include <base/vmath.h>

class CDummyBlmapChillPolice : public CDummyBase
{
public:
	CDummyBlmapChillPolice(class CCharacter *pChr, class CPlayer *pPlayer);
	virtual void OnTick();
	virtual ~CDummyBlmapChillPolice(){};

private:
	bool CheckStuck();
	void OldPoliceMoves();
	void NewPoliceMoves();

	void WalkPoliceDir(int Direction);
	void WalkPoliceLeft() { WalkPoliceDir(DIRECTION_LEFT); }
	void WalkPoliceRight() { WalkPoliceDir(DIRECTION_RIGHT); }
	bool HelpOfficerRight();
	bool HelpOfficerLeft();

	int m_LovedX;
	int m_LovedY;
	int m_LowerPanic;
	int m_Speed;
	int m_HelpMode;
	int m_GrenadeJump;
	int m_SpawnTeleporter;
	int m_FailedAttempts;
	int m_Confused;
	int m_Sad;

	bool m_IsHelpHook;
	bool m_IsClosestPolice;
	bool m_DidRocketjump;
	bool m_HasTouchedGround;
	bool m_HasAlreadyBeenHere;
	bool m_HasStartGrenade;
	bool m_IsDJUsed;
	bool m_HasReachedCinemaEntrance;
	bool m_GetSpeed;

	vec2 m_LastStuckCheckPos;
};

#endif
