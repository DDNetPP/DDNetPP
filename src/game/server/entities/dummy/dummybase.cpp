
#include "dummybase.h"

#include "../character.h"
#include <game/server/player.h>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>

CDummyBase::CDummyBase(class CCharacter *pChr, class CPlayer *pPlayer)
{
	m_pChr = pChr;
	m_pPlayer = pPlayer;
}

IServer *CDummyBase::Server()
{
	return m_pChr->Server();
}

CGameContext *CDummyBase::GameServer()
{
	return m_pChr->GameServer();
}

CGameWorld *CDummyBase::GameWorld()
{
	return m_pChr->GameWorld();
}

CNetObj_PlayerInput *CDummyBase::Input()
{
	return m_pChr->Input();
}

CNetObj_PlayerInput *CDummyBase::LatestInput()
{
	return m_pChr->LatestInput();
}

vec2 CDummyBase::GetPos()
{
	return m_pChr->GetCore().m_Pos;
}

vec2 CDummyBase::GetVel()
{
	return m_pChr->GetCore().m_Vel;
}

void CDummyBase::Die()
{
	m_pChr->Die(m_pChr->GetPlayer()->GetCID(), WEAPON_SELF);
}

void CDummyBase::SetWeapon(int Weapon)
{
	m_pChr->SetWeapon(Weapon);
}

void CDummyBase::Fire()
{
	m_pChr->Fire();
}

bool CDummyBase::IsGrounded()
{
	return m_pChr->IsGrounded();
}

int CDummyBase::HookState()
{
	return m_pChr->GetCore().m_HookState;
}

int CDummyBase::Jumped()
{
	return m_pChr->GetCore().m_Jumped;
}

int CDummyBase::Jumps()
{
	return m_pChr->GetCore().m_Jumps;
}
