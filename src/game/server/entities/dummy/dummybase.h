#ifndef GAME_SERVER_ENTITIES_DUMMYBASE_H
#define GAME_SERVER_ENTITIES_DUMMYBASE_H

#include <base/vmath.h>

class CCharacter;
class CPlayer;

class CDummyBase
{
public:
	CDummyBase(class CCharacter *pChr, class CPlayer *pPlayer);
	void Tick();

private:
	virtual void OnTick() {}

	int m_RtfGetSpeed;
	int m_LtfGetSpeed;
	bool m_GoSlow;
	bool m_AsBackwards;
	bool m_AsTopFree;
	bool m_AsBottomFree;

protected:
	class IServer *Server();
	class CGameContext *GameServer();
	class CGameWorld *GameWorld();
	struct CNetObj_PlayerInput *Input();
	struct CNetObj_PlayerInput *LatestInput();

	CCharacter *m_pCharacter;
	CPlayer *m_pPlayer;

	bool TicksPassed(int Ticks);

	// Setters
	void Left();
	void Right();
	void SetDirection(int Direction);
	void StopMoving();
	void Hook(bool Stroke = true);
	void Jump(bool Stroke = true);
	void Fire(bool Stroke = true);
	void Aim(int TargetX, int TargetY);
	void AimPos(vec2 Pos);
	void AimX(int TargetX);
	void AimY(int TargetY);

	void SetWeapon(int Weapon);
	void Die();

	// Getters
	vec2 GetPos();
	vec2 GetVel();

	int HookState();
	int Jumped();
	int JumpedTotal();
	int Jumps();
	bool IsGrounded();
	bool IsFrozen();

	int GetTargetX();
	int GetTargetY();
	int GetDirection();

	enum Directions
	{
		DIRECTION_LEFT = -1,
		DIRECTION_NONE = 0,
		DIRECTION_RIGHT = 1
	};

	int m_WantedWeapon;

	bool IsPolice(CCharacter *pChr);

	//
	int GetTile(int PosX, int PosY);
	int GetFTile(int PosX, int PosY);
	bool IsFreezeTile(int PosX, int PosY);

	void AvoidTile(int Tile);
	void AvoidFreeze();
	void AvoidDeath();
	void AntiStuckDir(int Direction);

	/*
		Function: AvoidFreezeWeapons
		Avoid freeze floor and roof using jumps and grenade
	*/
	void AvoidFreezeWeapons();
	/*
		Function: RightAntiStuck
		Walks right and trys to avoid getting stuck in simple obstacles
		can jump over collision in the way and go back if there is collision on top
	*/
	void RightAntiStuck();
	/*
		Function: LeftAntiStuck
		see RightAntiStuck
	*/
	void LeftAntiStuck();
	/*
		Function: RightThroughFreeze
		Walks right and jumps through simple freeze walls
	*/
	void RightThroughFreeze();
	/*
		Function: LeftThroughFreeze
		Walks left and jumps through simple freeze walls
	*/
	void LeftThroughFreeze();

	// Debug
	enum SkinColor
	{
		COLOR_RED,
		COLOR_ORANGE,
		COLOR_YELLOW,
		COLOR_GREEN,
		COLOR_CYAN,
		COLOR_CYAN_BLUE,
		COLOR_BLUE,
		COLOR_BLUE_MAGENTA,
		COLOR_MAGENTA,

		COLOR_BLACK,
		COLOR_WHITE,
	};

	int m_DebugColor;
	void DebugColor(int Color);
};

#endif
