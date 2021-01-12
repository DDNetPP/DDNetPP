#ifndef GAME_SERVER_ENTITIES_DUMMYBASE_H
#define GAME_SERVER_ENTITIES_DUMMYBASE_H

#include <base/vmath.h>

class CCharacter;
class CPlayer;

class CDummyBase {
public:
	CDummyBase(class CCharacter *pChr, class CPlayer *pPlayer);

	void OnTick();

	vec2 GetPos();
	vec2 GetVel();
	void Die();
	void SetWeapon(int Weapon);
	void Fire();
	bool IsGrounded();
	int HookState();
	int Jumped();
	int Jumps();

	class IServer *Server();
	class CGameContext *GameServer();
	class CGameWorld *GameWorld();
	struct CNetObj_PlayerInput *Input();
	struct CNetObj_PlayerInput *LatestInput();


	CCharacter *m_pChr;
	CPlayer *m_pPlayer;
};

#endif
