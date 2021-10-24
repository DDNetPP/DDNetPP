
#include "dummybase.h"

#include "../character.h"
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>

CDummyBase::CDummyBase(class CCharacter *pChr, class CPlayer *pPlayer)
{
	m_pCharacter = pChr;
	m_pPlayer = pPlayer;
	m_DebugColor = -1;
	m_WantedWeapon = -1;
	m_RtfGetSpeed = 0;
	m_LtfGetSpeed = 0;
	m_GoSlow = false;
	m_AsBackwards = false;
	m_AsTopFree = false;
	m_AsBottomFree = false;
}

IServer *CDummyBase::Server()
{
	return m_pCharacter->Server();
}

CGameContext *CDummyBase::GameServer()
{
	return m_pCharacter->GameServer();
}

CGameWorld *CDummyBase::GameWorld()
{
	return m_pCharacter->GameWorld();
}

CNetObj_PlayerInput *CDummyBase::Input()
{
	return m_pCharacter->Input();
}

CNetObj_PlayerInput *CDummyBase::LatestInput()
{
	return m_pCharacter->LatestInput();
}

bool CDummyBase::TicksPassed(int Ticks) { return Server()->Tick() % Ticks == 0; }
vec2 CDummyBase::GetPos() { return m_pCharacter->Core()->m_Pos; }
vec2 CDummyBase::GetVel() { return m_pCharacter->Core()->m_Vel; }
int CDummyBase::HookState() { return m_pCharacter->Core()->m_HookState; }
int CDummyBase::Jumped() { return m_pCharacter->Core()->m_Jumped; }
int CDummyBase::JumpedTotal() { return m_pCharacter->Core()->m_JumpedTotal; }
int CDummyBase::Jumps() { return m_pCharacter->Core()->m_Jumps; }
bool CDummyBase::IsGrounded() { return m_pCharacter->IsGrounded(); }
bool CDummyBase::IsFrozen() { return m_pCharacter->isFreezed; }
int CDummyBase::GetTargetX() { return m_pCharacter->Input()->m_TargetX; }
int CDummyBase::GetTargetY() { return m_pCharacter->Input()->m_TargetY; }
int CDummyBase::GetDirection() { return m_pCharacter->Input()->m_Direction; }

void CDummyBase::SetWeapon(int Weapon)
{
	m_pCharacter->SetWeapon(Weapon);
	m_WantedWeapon = -1;
}
void CDummyBase::Die() { m_pCharacter->Die(m_pCharacter->GetPlayer()->GetCID(), WEAPON_SELF); }
void CDummyBase::Left() { m_pCharacter->Input()->m_Direction = DIRECTION_LEFT; }
void CDummyBase::Right() { m_pCharacter->Input()->m_Direction = DIRECTION_RIGHT; }
void CDummyBase::StopMoving() { m_pCharacter->Input()->m_Direction = DIRECTION_NONE; }
void CDummyBase::SetDirection(int Direction) { m_pCharacter->Input()->m_Direction = Direction; }
void CDummyBase::Hook(bool Stroke) { m_pCharacter->Input()->m_Hook = Stroke; }
void CDummyBase::Jump(bool Stroke) { m_pCharacter->Input()->m_Jump = Stroke; }
void CDummyBase::Aim(int TargetX, int TargetY)
{
	AimX(TargetX);
	AimY(TargetY);
}
void CDummyBase::AimX(int TargetX)
{
	m_pCharacter->LatestInput()->m_TargetX = TargetX;
	m_pCharacter->Input()->m_TargetX = TargetX;
}
void CDummyBase::AimY(int TargetY)
{
	m_pCharacter->LatestInput()->m_TargetY = TargetY;
	m_pCharacter->Input()->m_TargetY = TargetY;
}
void CDummyBase::AimPos(vec2 Pos) { Aim(Pos.x - GetPos().x, Pos.y - GetPos().y); }
void CDummyBase::Fire(bool Stroke)
{
	if(Stroke)
	{
		m_pCharacter->LatestInput()->m_Fire++;
		m_pCharacter->Input()->m_Fire++;
	}
	else
	{
		m_pCharacter->LatestInput()->m_Fire = 0;
		m_pCharacter->Input()->m_Fire = 0;
	}
}

void CDummyBase::Tick()
{
	if(!m_pCharacter->IsAlive() || !m_pPlayer->m_IsDummy)
		return;

	// Prepare input
	m_pCharacter->ResetInput();
	Hook(0);

	// Then start controlling
	OnTick();

	if(m_WantedWeapon != -1)
		SetWeapon(m_WantedWeapon);
}

bool CDummyBase::IsPolice(CCharacter *pChr)
{
	if(!pChr)
		return false;
	return pChr->GetPlayer()->m_PoliceRank || pChr->GetPlayer()->m_PoliceHelper;
}

int CDummyBase::GetTile(int PosX, int PosY)
{
	return GameServer()->Collision()->GetCustTile(PosX, PosY);
}

int CDummyBase::GetFTile(int PosX, int PosY)
{
	return GameServer()->Collision()->GetCustFTile(PosX, PosY);
}

bool CDummyBase::IsFreezeTile(int PosX, int PosY)
{
	int Tile = GetTile(PosX, PosY);
	int FTile = GetFTile(PosX, PosY);
	return Tile == TILE_FREEZE || FTile == TILE_FREEZE || Tile == TILE_DFREEZE || FTile == TILE_DFREEZE;
}

void CDummyBase::AvoidTile(int Tile)
{
#define IS_TILE(x, y) (GetTile(x * 32, y * 32) == Tile || GetFTile(x * 32, y * 32) == Tile)
#define AIR(x, y) !IS_TILE(x, y)
#define SOLID(x, y) GameServer()->Collision()->IsSolid(x * 32, y * 32)
	int X = GetPos().x / 32;
	int Y = GetPos().y / 32;

	// sides
	if(IS_TILE(X + 1, Y))
		Left();
	if(IS_TILE(X - 1, Y))
		Right();

	// corners
	if(AIR(X - 1, Y) && IS_TILE(X + 1, Y - 1))
		Left();
	if(AIR(X + 1, Y) && IS_TILE(X - 1, Y - 1))
		Right();

	// small edges
	if(AIR(X - 1, Y) && IS_TILE(X - 1, Y + 1))
		Right();

	if(AIR(X + 1, Y) && IS_TILE(X + 1, Y + 1))
		Left();

	// big edges
	if(AIR(X - 1, Y) && AIR(X - 2, Y) && AIR(X - 2, Y + 1) && IS_TILE(X - 2, Y + 1))
		Right();
	if(AIR(X + 1, Y) && AIR(X + 2, Y) && AIR(X + 2, Y + 1) && IS_TILE(X + 2, Y + 1))
		Left();

	// while falling
	if(IS_TILE(X, Y + GetVel().y))
	{
		if(SOLID(X - GetVel().y, Y + GetVel().y))
			Right();
		if(SOLID(X + GetVel().y, Y + GetVel().y))
			Left();
	}

#undef IS_TILE
#undef AIR
#undef SOLID
}

void CDummyBase::AvoidFreeze()
{
	AvoidTile(TILE_FREEZE);
	AvoidTile(TILE_DFREEZE);
}

void CDummyBase::AvoidDeath()
{
	AvoidTile(TILE_DEATH);
	if((GetPos().x / 32) + 5 >= GameServer()->Collision()->GetWidth() + 200)
		Left();
	else if((GetPos().x / 32) - 5 <= -200)
		Right();
}

void CDummyBase::AvoidFreezeWeapons()
{
	// avoid hitting freeze roof
	if(GetVel().y < -0.05)
	{
		int distY = GetPos().y + GetVel().y * 4 - 40;
		if(!GameServer()->Collision()->IntersectLine(GetPos(), vec2(GetPos().x, distY), 0, 0) &&
			IsFreezeTile(GetPos().x, distY))
		{
			Aim(GetVel().x, -200);
			Fire();
			if(TicksPassed(10))
				SetWeapon(WEAPON_GRENADE);
			m_WantedWeapon = WEAPON_GRENADE;
		}
	}
	// avoid hitting freeze floor
	else if(GetVel().y > 0.05)
	{
		int distY = GetPos().y + GetVel().y * 3 + 20;
		if(!GameServer()->Collision()->IntersectLine(GetPos(), vec2(GetPos().x, distY), 0, 0) &&
			IsFreezeTile(GetPos().x, distY))
		{
			Aim(GetVel().x, 200);
			if(JumpedTotal() == Jumps() - 1)
			{
				Fire();
				// TODO: priotize weapons the bot actually has
				int PanicWeapon = (rand() % 2 == 0) ? WEAPON_GRENADE : WEAPON_LASER;
				if(TicksPassed(10))
					SetWeapon(PanicWeapon);
				m_WantedWeapon = PanicWeapon;
			}
			Jump();
		}
	}
	// jump over freeze when flying into it from the right side
	if(GetVel().x < -2.2f && (IsGrounded() || GetVel().y > 2.2f))
	{
		if(IsFreezeTile(GetPos().x - 32, GetPos().y) ||
			(GetVel().y > 2.2f && IsFreezeTile(GetPos().x - 32, GetPos().y + 30)))
			Jump();
	}
	// jump over freeze when flying into it from the left side
	if(GetVel().x > 2.2f && (IsGrounded() || GetVel().y > 2.2f))
	{
		if(IsFreezeTile(GetPos().x + 32, GetPos().y) ||
			(GetVel().y < -2.2f && IsFreezeTile(GetPos().x + 32, GetPos().y + 30)))
			Jump();
	}
}

void CDummyBase::RightAntiStuck()
{
	AntiStuckDir(DIRECTION_RIGHT);
}

void CDummyBase::LeftAntiStuck()
{
	AntiStuckDir(DIRECTION_LEFT);
}

void CDummyBase::AntiStuckDir(int Direction)
{
	SetDirection(Direction);
	if(m_GoSlow)
	{
		StopMoving();
		if(TicksPassed(3))
			SetDirection(Direction);
		if(TicksPassed(200))
			m_GoSlow = false;
		if(m_AsTopFree)
			Jump(rand() % 5 == 0);
		return;
	}
	if(m_AsBackwards)
	{
		SetDirection(-Direction);
		m_AsTopFree = !GameServer()->Collision()->IsSolid(GetPos().x + 20 * Direction, GetPos().y - 20) &&
			      !GameServer()->Collision()->IsSolid(GetPos().x + 20 * Direction, GetPos().y - 40) &&
			      !GameServer()->Collision()->IsSolid(GetPos().x + 20 * Direction, GetPos().y - 70);
		m_AsBottomFree = !GameServer()->Collision()->IsSolid(GetPos().x + 20 * Direction, GetPos().y + 20) &&
				 !GameServer()->Collision()->IsSolid(GetPos().x + 20 * Direction, GetPos().y + 40) &&
				 !GameServer()->Collision()->IsSolid(GetPos().x + 20 * Direction, GetPos().y + 70);
		if(m_AsTopFree || m_AsBottomFree)
		{
			Jump(rand() % 5 == 0);
			m_AsBackwards = false;
			if(m_AsTopFree && !GameServer()->Collision()->IsSolid(GetPos().x - 20 * Direction, GetPos().y - 20))
			{
				// when there is space go fast
			}
			else
			{
				m_GoSlow = true;
			}
		}
		return;
	}
	if(GameServer()->Collision()->IsSolid(GetPos().x + 60 * Direction, GetPos().y) ||
		GameServer()->Collision()->IsSolid(GetPos().x + 30 * Direction, GetPos().y) ||
		GameServer()->Collision()->IsSolid(GetPos().x + 10 * Direction, GetPos().y))
	{
		Jump(rand() % 5 == 0);
		// too slow? Check if in a dead end
		if(((Direction == DIRECTION_LEFT && GetVel().x > -1.1f) || (Direction == DIRECTION_RIGHT && GetVel().x < 1.1f)) && IsGrounded())
		{
			if(
				/* top blocked */
				(
					GameServer()->Collision()->IsSolid(GetPos().x + 10 * Direction, GetPos().y - 20) ||
					GameServer()->Collision()->IsSolid(GetPos().x + 10 * Direction, GetPos().y - 40) ||
					GameServer()->Collision()->IsSolid(GetPos().x + 10 * Direction, GetPos().y - 70)) &&
				/* bottom blocked */
				(
					GameServer()->Collision()->IsSolid(GetPos().x + 10 * Direction, GetPos().y + 20) ||
					GameServer()->Collision()->IsSolid(GetPos().x + 10 * Direction, GetPos().y + 40) ||
					GameServer()->Collision()->IsSolid(GetPos().x + 10 * Direction, GetPos().y + 70)) &&
				/* left blocked */
				(
					GameServer()->Collision()->IsSolid(GetPos().x + 20 * Direction, GetPos().y - 30) ||
					GameServer()->Collision()->IsSolid(GetPos().x + 20 * Direction, GetPos().y + 30)))
			{
				m_AsBackwards = true;
			}
		}
	}
}

void CDummyBase::RightThroughFreeze()
{
	if(m_pCharacter->m_FreezeTime)
	{
		m_RtfGetSpeed = 0;
		return;
	}
	Right();
	if(m_RtfGetSpeed)
	{
		Left();
		if(IsFreezeTile(GetPos().x + m_RtfGetSpeed, GetPos().y) || IsFreezeTile(GetPos().x + m_RtfGetSpeed, GetPos().y + 16))
			return;
		m_RtfGetSpeed = 0;
	}
	// jump through freeze if one is close or go back if no vel
	for(int i = 5; i < 160; i += 5)
	{
		// ignore freeze behind collision
		if(GameServer()->Collision()->IsSolid(GetPos().x + i, GetPos().y))
			break;

		if(IsFreezeTile(GetPos().x + i, GetPos().y) || IsFreezeTile(GetPos().x + i, GetPos().y + 16))
		{
			if(GetVel().y > 1.1f)
			{
				if(!IsFreezeTile(GetPos().x - 32, GetPos().y) && !IsFreezeTile(GetPos().x - 32, GetPos().y + 16))
					Left();
			}
			if(IsGrounded() && GetVel().x > 8.8f)
				Jump(TicksPassed(2));
			if(i < 22 && GetVel().x < 5.5f)
			{
				int k;
				for(k = 5; k < 160; k += 5)
				{
					if(IsFreezeTile(GetPos().x - k, GetPos().y) || IsFreezeTile(GetPos().x - k, GetPos().y + 16))
					{
						break;
					}
				}
				m_RtfGetSpeed = k < 80 ? 20 : 40;
				Left();
			}
			break;
		}
	}
}

void CDummyBase::LeftThroughFreeze()
{
	if(m_pCharacter->m_FreezeTime)
	{
		m_LtfGetSpeed = 0;
		return;
	}
	Left();
	if(m_LtfGetSpeed)
	{
		Right();
		if(IsFreezeTile(GetPos().x - m_LtfGetSpeed, GetPos().y) || IsFreezeTile(GetPos().x - m_LtfGetSpeed, GetPos().y + 16))
			return;
		m_LtfGetSpeed = 0;
	}
	// jump through freeze if one is close or go back if no vel
	for(int i = 5; i < 160; i += 5)
	{
		// ignore freeze behind collision
		if(GameServer()->Collision()->IsSolid(GetPos().x - i, GetPos().y))
			break;

		if(IsFreezeTile(GetPos().x - i, GetPos().y) || IsFreezeTile(GetPos().x - i, GetPos().y + 16))
		{
			if(GetVel().y > 1.1f)
			{
				if(!IsFreezeTile(GetPos().x + 32, GetPos().y) && !IsFreezeTile(GetPos().x + 32, GetPos().y + 16))
					Right();
			}
			if(IsGrounded() && GetVel().x < -8.8f)
				Jump(TicksPassed(2));
			if(i < 22 && GetVel().x > -5.5f)
			{
				int k;
				for(k = 5; k < 160; k += 5)
				{
					if(IsFreezeTile(GetPos().x + k, GetPos().y) || IsFreezeTile(GetPos().x + k, GetPos().y + 16))
					{
						break;
					}
				}
				m_LtfGetSpeed = k < 80 ? 20 : 40;
				Right();
			}
			break;
		}
	}
}

void CDummyBase::DebugColor(int DebugColor)
{
	if(DebugColor == m_DebugColor)
		return;
	m_DebugColor = DebugColor;

	if(DebugColor == -1)
	{
		// m_pPlayer->ResetSkin();
		return;
	}

	int BaseColor = (DebugColor * 30) * 0x010000;
	int Color = 0xff32;
	if(DebugColor == COLOR_BLACK)
		Color = BaseColor = 0;
	else if(DebugColor == COLOR_WHITE)
		Color = BaseColor = 255;

	m_pPlayer->m_TeeInfos.m_ColorBody = Color;
}
