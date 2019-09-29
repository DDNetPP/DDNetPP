/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include <engine/shared/config.h>
#include <game/server/gamemodes/DDRace.h>

#include "door.h"

CDoor::CDoor(CGameWorld *pGameWorld, vec2 Pos, float Rotation, int Length,
		int Number) :
		CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER)
{
	m_Number = Number;
	m_Pos = Pos;
	m_Length = Length;
	m_Direction = vec2(sin(Rotation), cos(Rotation));
	vec2 To = Pos + normalize(m_Direction) * m_Length;

	GameServer()->Collision()->IntersectNoLaser(Pos, To, &this->m_To, 0);
	ResetCollision();
	GameWorld()->InsertEntity(this);
}

void CDoor::Open(int Tick, bool ActivatedTeam[])
{
	m_EvalTick = Server()->Tick();
}

void CDoor::ResetCollision()
{
	for (int i = 0; i < m_Length - 1; i++)
	{
		vec2 CurrentPos(m_Pos.x + (m_Direction.x * i),
				m_Pos.y + (m_Direction.y * i));
		if (GameServer()->Collision()->CheckPoint(CurrentPos)
				|| GameServer()->Collision()->GetTile(m_Pos.x, m_Pos.y)
				|| GameServer()->Collision()->GetFTile(m_Pos.x, m_Pos.y))
			break;
		else
			GameServer()->Collision()->SetDCollisionAt(
					m_Pos.x + (m_Direction.x * i),
					m_Pos.y + (m_Direction.y * i), TILE_STOPA, 0/*Flags*/,
					m_Number);
	}
}

void CDoor::Open(int Team)
{
}

void CDoor::Close(int Team)
{
}

void CDoor::Reset()
{
}

void CDoor::Tick()
{
}

void CDoor::Snap(int SnappingClient)
{
	if (NetworkClipped(SnappingClient, m_Pos)
			&& NetworkClipped(SnappingClient, m_To))
		return;

	CNetObj_Laser *pObj = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(
			NETOBJTYPE_LASER, m_ID, sizeof(CNetObj_Laser)));

	if (!pObj)
		return;

	pObj->m_X = (int) m_Pos.x;
	pObj->m_Y = (int) m_Pos.y;

	CCharacter * Char = GameServer()->GetPlayerChar(SnappingClient);
	int Tick = (Server()->Tick() % Server()->TickSpeed()) % 11;

	if(SnappingClient > -1 && (GameServer()->m_apPlayers[SnappingClient]->GetTeam() == -1
				|| GameServer()->m_apPlayers[SnappingClient]->m_Paused)
			&& GameServer()->m_apPlayers[SnappingClient]->m_SpectatorID != SPEC_FREEVIEW)
		Char = GameServer()->GetPlayerChar(GameServer()->m_apPlayers[SnappingClient]->m_SpectatorID);

	if (Char == 0)
		return;

	if (Char->IsAlive() && GameServer()->Collision()->m_NumSwitchers > 0
			&& !GameServer()->Collision()->m_pSwitchers[m_Number].m_Status[Char->Team()]
			&& (!Tick))
		return;

	if (Char->Team() == TEAM_SUPER)
	{
		pObj->m_FromX = (int) m_Pos.x;
		pObj->m_FromY = (int) m_Pos.y;
	}
	else if (Char->IsAlive() && GameServer()->Collision()->m_NumSwitchers > 0
		&& GameServer()->Collision()->m_pSwitchers[m_Number].m_Status[Char->Team()])
	{
		pObj->m_FromX = (int) m_To.x;
		pObj->m_FromY = (int) m_To.y;
	}
	else
	{
		pObj->m_FromX = (int) m_Pos.x;
		pObj->m_FromY = (int) m_Pos.y;
	}
	pObj->m_StartTick = Server()->Tick();
}
