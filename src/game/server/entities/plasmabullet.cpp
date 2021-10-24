/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#include "plasmabullet.h"
#include <engine/config.h>
#include <engine/server.h>
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/teams.h>

CPlasmaBullet::CPlasmaBullet(CGameWorld *pGameWorld, int Owner, vec2 Pos, vec2 Dir, bool Freeze,
	bool Explosive, bool Unfreeze, bool Bloody, bool Ghost, int ResponsibleTeam, float Lifetime, float Accel, float Speed) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_Owner = Owner;
	m_Pos = Pos;
	m_Core = normalize(Dir) * Speed;
	m_Freeze = Freeze;
	m_Explosive = Explosive;
	m_Unfreeze = Unfreeze;
	m_Bloody = Bloody;
	m_Ghost = Ghost;
	m_EvalTick = Server()->Tick();
	m_LifeTime = Server()->TickSpeed() * Lifetime;
	m_ResponsibleTeam = ResponsibleTeam;
	m_Accel = Accel;
	GameWorld()->InsertEntity(this);
}

bool CPlasmaBullet::HitCharacter()
{
	vec2 To2;
	CCharacter *Hit = GameServer()->m_World.IntersectCharacter(m_Pos,
		m_Pos + m_Core, 0.0f, To2);
	if(!Hit)
		return false;
	if(Hit->Team() != m_ResponsibleTeam)
		return false;

	if(Hit->GetPlayer()->GetCID() == m_Owner) // dont hit yourself
	{
		return false;
	}
	else
	{
		Hit->SetEmote(3, Server()->Tick() + 2 * Server()->TickSpeed()); // eyeemote surprise
		GameServer()->SendEmoticon(Hit->GetPlayer()->GetCID(), 7); //emoticon ghost
	}

	if(m_Bloody)
		GameServer()->CreateDeath(m_Pos, Hit->GetPlayer()->GetCID());

	if(m_Freeze)
		Hit->Freeze();

	if(m_Unfreeze)
		Hit->UnFreeze();

	if(m_Explosive)
		GameServer()->CreateExplosion(m_Pos, m_Owner, WEAPON_GRENADE, true,
			m_ResponsibleTeam, Hit->Teams()->TeamMask(m_ResponsibleTeam));
	GameServer()->m_World.DestroyEntity(this);
	return true;
}

void CPlasmaBullet::Move()
{
	m_Pos += m_Core;
	m_Core *= m_Accel;
}

void CPlasmaBullet::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

void CPlasmaBullet::Tick()
{
	if(m_LifeTime == 0)
	{
		Reset();
		return;
	}
	m_LifeTime--;
	Move();
	HitCharacter();

	int Res = 0;
	Res = GameServer()->Collision()->IntersectNoLaser(m_Pos, m_Pos + m_Core, 0,
		0);
	if(Res)
	{
		if(m_Explosive)
			GameServer()->CreateExplosion(
				m_Pos,
				-1,
				WEAPON_GRENADE,
				true,
				m_ResponsibleTeam,
				((CGameControllerDDRace *)GameServer()->m_pController)->m_Teams.TeamMask(m_ResponsibleTeam));

		if(m_Bloody)
		{
			if(m_IsInsideWall == 1)
			{
				if(Server()->Tick() % 5 == 0)
				{
					GameServer()->CreateDeath(m_Pos, m_Owner);
				}
			}
			else
			{
				GameServer()->CreateDeath(m_Pos, m_Owner);
			}
		}

		if(m_Ghost && m_IsInsideWall == 0)
			m_IsInsideWall = 1; // enteres the wall, collides the first time

		if(m_IsInsideWall == 2 || !m_Ghost) // collides second time with a wall
			Reset();
	}
	else
	{
		if(m_Ghost && m_IsInsideWall == 1)
			m_IsInsideWall = 2; // leaves the wall
	}
}

void CPlasmaBullet::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;
	CCharacter *SnapChar = GameServer()->GetPlayerChar(SnappingClient);
	CPlayer *SnapPlayer = SnappingClient > -1 ? GameServer()->m_apPlayers[SnappingClient] : 0;
	int Tick = (Server()->Tick() % Server()->TickSpeed()) % 11;

	if(SnapChar && SnapChar->IsAlive() && (m_Layer == LAYER_SWITCH && !GameServer()->Collision()->m_pSwitchers[m_Number].m_Status[SnapChar->Team()]) && (!Tick))
		return;

	if(SnapPlayer && (SnapPlayer->GetTeam() == TEAM_SPECTATORS || SnapPlayer->IsPaused()) && SnapPlayer->m_SpectatorID != -1 && GameServer()->GetPlayerChar(SnapPlayer->m_SpectatorID) && GameServer()->GetPlayerChar(SnapPlayer->m_SpectatorID)->Team() != m_ResponsibleTeam && !SnapPlayer->m_ShowOthers)
		return;

	if(SnapPlayer && SnapPlayer->GetTeam() != TEAM_SPECTATORS && !SnapPlayer->IsPaused() && SnapChar && SnapChar && SnapChar->Team() != m_ResponsibleTeam && !SnapPlayer->m_ShowOthers)
		return;

	if(SnapPlayer && (SnapPlayer->GetTeam() == TEAM_SPECTATORS || SnapPlayer->IsPaused()) && SnapPlayer->m_SpectatorID == -1 && SnapChar && SnapChar->Team() != m_ResponsibleTeam && SnapPlayer->m_SpecTeam)
		return;

	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(
		NETOBJTYPE_LASER, GetID(), sizeof(CNetObj_Laser)));

	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_FromX = (int)m_Pos.x;
	pObj->m_FromY = (int)m_Pos.y;
	pObj->m_StartTick = m_EvalTick;
}
