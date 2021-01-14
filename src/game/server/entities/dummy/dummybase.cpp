
#include "dummybase.h"

#include "../character.h"
#include <game/server/player.h>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>

CDummyBase::CDummyBase(class CCharacter *pChr, class CPlayer *pPlayer)
{
	m_pCharacter = pChr;
	m_pPlayer = pPlayer;
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

vec2 CDummyBase::GetPos()
{
	return m_pCharacter->GetCore().m_Pos;
}

vec2 CDummyBase::GetVel()
{
	return m_pCharacter->GetCore().m_Vel;
}

void CDummyBase::Die()
{
	m_pCharacter->Die(m_pCharacter->GetPlayer()->GetCID(), WEAPON_SELF);
}

void CDummyBase::SetWeapon(int Weapon)
{
	m_pCharacter->SetWeapon(Weapon);
}

void CDummyBase::Fire()
{
	m_pCharacter->Fire();
}

bool CDummyBase::IsGrounded()
{
	return m_pCharacter->IsGrounded();
}

int CDummyBase::HookState()
{
	return m_pCharacter->GetCore().m_HookState;
}

int CDummyBase::Jumped()
{
	return m_pCharacter->GetCore().m_Jumped;
}

int CDummyBase::Jumps()
{
	return m_pCharacter->GetCore().m_Jumps;
}
