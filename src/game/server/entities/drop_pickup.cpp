#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>

#include <game/server/teams.h>

#include "drop_pickup.h"
#include "pickup.h"

CDropPickup::CDropPickup(CGameWorld *pGameWorld, int Type, int Lifetime, int Owner, int Direction, float Force, int ResponsibleTeam) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP)
{
	if(GameServer()->GetPlayerChar(Owner))
	{
		m_Type = Type;
		m_Lifetime = Server()->TickSpeed() * Lifetime;
		m_ResponsibleTeam = ResponsibleTeam;
		m_Pos = GameServer()->GetPlayerChar(Owner)->m_Pos;
		m_Owner = Owner;

		m_Vel = vec2(5 * Direction, -5);
		m_Vel.x += (rand() % 10 - 5) * Force;
		m_Vel.y += (rand() % 10 - 5) * Force;

		m_PickupDelay = Server()->TickSpeed() * 2;

		GameWorld()->InsertEntity(this);
	}
	else // invalid owner
	{
		// https://github.com/DDNetPP/DDNetPP/issues/296
		dbg_msg("drop_pickup", "[WARNING] playerchar=%d is invalid", Owner);
		m_Type = Type;
		m_Lifetime = 1;
		m_ResponsibleTeam = ResponsibleTeam;
		m_Pos = vec2(0, 0);
		m_Owner = Owner;

		m_Vel = vec2(5 * Direction, -5);
		m_Vel.x += (rand() % 10 - 5) * Force;
		m_Vel.y += (rand() % 10 - 5) * Force;

		m_PickupDelay = 1;

		GameWorld()->InsertEntity(this);
		Delete();
	}
}

void CDropPickup::Delete()
{
	m_EreasePickup = true;
	Reset();
}

void CDropPickup::Reset()
{
	if(m_EreasePickup)
	{
		for(unsigned i = 0; i < GameServer()->m_vDropLimit[m_Type].size(); i++)
		{
			if(GameServer()->m_vDropLimit[m_Type][i] == this)
			{
				GameServer()->m_vDropLimit[m_Type].erase(GameServer()->m_vDropLimit[m_Type].begin() + i);
			}
		}
	}

	if(IsCharacterNear() == -1)
		GameServer()->CreateDeath(m_Pos, -1);

	GameServer()->m_World.DestroyEntity(this);
}

int CDropPickup::IsCharacterNear()
{
	CCharacter *apEnts[MAX_CLIENTS];
	int Num = GameWorld()->FindEntities(m_Pos, 20.0f, (CEntity **)apEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

	for(int i = 0; i < Num; ++i)
	{
		CCharacter *pChr = apEnts[i];
		if(pChr && pChr->IsAlive())
			return pChr->GetPlayer()->GetCID();
	}

	return -1;
}

void CDropPickup::Pickup()
{
	int CharID = IsCharacterNear();
	if(CharID != -1)
	{
		CCharacter *pChar = GameServer()->GetPlayerChar(CharID);
		if(!pChar)
			return;

		if(pChar->GetPlayer()->m_IsVanillaDmg)
		{
			if(m_Type == POWERUP_HEALTH)
				pChar->IncreaseHealth(1);
			else if(m_Type == POWERUP_ARMOR)
				pChar->IncreaseArmor(1);
		}

		if(m_Type == POWERUP_HEALTH)
			GameServer()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH, pChar->Teams()->TeamMask(pChar->Team()));
		else if(m_Type == POWERUP_ARMOR)
			GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR, pChar->Teams()->TeamMask(pChar->Team()));

		Delete();
		return;
	}
}

void CDropPickup::Tick()
{
	if(m_Owner != -1 && GameServer()->m_ClientLeftServer[m_Owner])
	{
		m_Owner = -1;
	}

	// weapon hits death-tile or left the game layer, reset it
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y) == TILE_DEATH || GameLayerClipped(m_Pos))
	{
		Delete();
		return;
	}

	if(m_Lifetime == 0)
	{
		Delete();
		return;
	}

	if(m_Lifetime != -1)
		m_Lifetime--;

	if(m_PickupDelay > 0)
		m_PickupDelay--;

	if(m_PickupDelay <= 0 || IsCharacterNear() != m_Owner)
		Pickup();

	m_Vel.y += GameServer()->Tuning()->m_Gravity;

	//Friction
	bool Grounded = false;
	if(GameServer()->Collision()->CheckPoint(m_Pos.x + ms_PhysSize / 2, m_Pos.y + ms_PhysSize / 2 + 5))
		Grounded = true;
	if(GameServer()->Collision()->CheckPoint(m_Pos.x - ms_PhysSize / 2, m_Pos.y + ms_PhysSize / 2 + 5))
		Grounded = true;

	if(Grounded == true)
		m_Vel.x *= 0.75f;
	else
		m_Vel.x *= 0.98f;

	//Speedups
	if(GameServer()->Collision()->IsSpeedup(GameServer()->Collision()->GetMapIndex(m_Pos)))
	{
		int Force, MaxSpeed = 0;
		vec2 Direction, MaxVel, TempVel = m_Vel;
		float TeeAngle, SpeederAngle, DiffAngle, SpeedLeft, TeeSpeed;
		GameServer()->Collision()->GetSpeedup(GameServer()->Collision()->GetMapIndex(m_Pos), &Direction, &Force, &MaxSpeed);

		if(Force == 255 && MaxSpeed)
		{
			m_Vel = Direction * (MaxSpeed / 5);
		}

		else
		{
			if(MaxSpeed > 0 && MaxSpeed < 5)
				MaxSpeed = 5;
			//dbg_msg("speedup tile start", "Direction %f %f, Force %d, Max Speed %d", (Direction).x, (Direction).y, Force, MaxSpeed);
			if(MaxSpeed > 0)
			{
				if(Direction.x > 0.0000001f)
					SpeederAngle = -atan(Direction.y / Direction.x);
				else if(Direction.x < 0.0000001f)
					SpeederAngle = atan(Direction.y / Direction.x) + 2.0f * asin(1.0f);
				else if(Direction.y > 0.0000001f)
					SpeederAngle = asin(1.0f);
				else
					SpeederAngle = asin(-1.0f);

				if(SpeederAngle < 0)
					SpeederAngle = 4.0f * asin(1.0f) + SpeederAngle;

				if(TempVel.x > 0.0000001f)
					TeeAngle = -atan(TempVel.y / TempVel.x);
				else if(TempVel.x < 0.0000001f)
					TeeAngle = atan(TempVel.y / TempVel.x) + 2.0f * asin(1.0f);
				else if(TempVel.y > 0.0000001f)
					TeeAngle = asin(1.0f);
				else
					TeeAngle = asin(-1.0f);

				if(TeeAngle < 0)
					TeeAngle = 4.0f * asin(1.0f) + TeeAngle;

				TeeSpeed = sqrt(pow(TempVel.x, 2) + pow(TempVel.y, 2));

				DiffAngle = SpeederAngle - TeeAngle;
				SpeedLeft = MaxSpeed / 5.0f - cos(DiffAngle) * TeeSpeed;
				//dbg_msg("speedup tile debug", "MaxSpeed %i, TeeSpeed %f, SpeedLeft %f, SpeederAngle %f, TeeAngle %f", MaxSpeed, TeeSpeed, SpeedLeft, SpeederAngle, TeeAngle);
				if(abs(SpeedLeft) > Force && SpeedLeft > 0.0000001f)
					TempVel += Direction * Force;
				else if(abs(SpeedLeft) > Force)
					TempVel += Direction * -Force;
				else
					TempVel += Direction * SpeedLeft;
			}
			else
				TempVel += Direction * Force;
		}
		m_Vel = TempVel;
	}

	GameServer()->Collision()->MoveBox(&m_Pos, &m_Vel, vec2(ms_PhysSize, ms_PhysSize), 0.5f);
}

void CDropPickup::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = m_Type;
}
