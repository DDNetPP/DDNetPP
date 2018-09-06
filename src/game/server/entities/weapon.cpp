/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "weapon.h"

#include <game/server/teams.h>

CWeapon::CWeapon(CGameWorld *pGameWorld, int Weapon, int Lifetime, int Owner, int Direction, int ResponsibleTeam, int Bullets, bool Jetpack)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	m_Type = Weapon;
	m_Lifetime = Server()->TickSpeed() * Lifetime;
	m_ResponsibleTeam = ResponsibleTeam;
	m_Pos = GameServer()->GetPlayerChar(Owner)->m_Pos;
	m_Jetpack = Jetpack;
	m_Bullets = Bullets;
	m_Owner = Owner;

	m_Vel = vec2(5*Direction, -5);

	m_PickupDelay = Server()->TickSpeed() * 2;

	m_ID2 = Server()->SnapNewID();
	GameWorld()->InsertEntity(this);
}

void CWeapon::Reset()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	Server()->SnapFreeID(m_ID2);
	GameServer()->m_World.DestroyEntity(this);
}

int CWeapon::IsCharacterNear()
{
	CCharacter *apEnts[MAX_CLIENTS];
	int Num = GameWorld()->FindEntities(m_Pos, 20.0f, (CEntity**)apEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
	
	for (int i = 0; i < Num; ++i)
	{
		CCharacter * pChr = apEnts[i];

		if (pChr && pChr->IsAlive())
			return pChr->GetPlayer()->GetCID(); 
	}

	return -1;
}

void CWeapon::Pickup()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	
	int CharID = IsCharacterNear();
	if (CharID != -1)
	{
		CCharacter* pChar = GameServer()->GetPlayerChar(CharID);

		if (pChar->GetPlayer()->m_SpookyGhostActive && m_Type != WEAPON_GUN)
			return;

		if (pChar->GetWeaponGot(m_Type) && !m_Jetpack)
			return;

		if (m_Jetpack && pChar->m_Jetpack)
			return;

		if (m_Jetpack && !pChar->GetWeaponGot(WEAPON_GUN))
			return;


		pChar->GiveWeapon(m_Type, m_Bullets);

		pChar->SetActiveWeapon(m_Type);

		if ((m_Bullets != -1) && !pChar->GetPlayer()->m_IsSurvivaling)
			pChar->m_aDecreaseAmmo[m_Type] = true;

		if (m_Jetpack)
		{
			pChar->m_Jetpack = true;
			GameServer()->SendChatTarget(pChar->GetPlayer()->GetCID(), "You have a jetpack gun");
		}

		if (m_Type == WEAPON_SHOTGUN || m_Type == WEAPON_RIFLE)
			GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, pChar->Teams()->TeamMask(pChar->Team()));
		else if (m_Type == WEAPON_GRENADE)
			GameServer()->CreateSound(m_Pos, SOUND_PICKUP_GRENADE, pChar->Teams()->TeamMask(pChar->Team()));
		else if (m_Type == WEAPON_HAMMER || m_Type == WEAPON_GUN)
			GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR, pChar->Teams()->TeamMask(pChar->Team()));

		CPlayer* pOwner = GameServer()->GetPlayerChar(m_Owner)->GetPlayer();

		for(unsigned i = 0; i < pOwner->m_vWeaponLimit[m_Type].size(); i++)
		{
			if(pOwner->m_vWeaponLimit[m_Type][i] == this)
			{
				pOwner->m_vWeaponLimit[m_Type].erase(pOwner->m_vWeaponLimit[m_Type].begin() + i);
			}
		}

		Reset();
		return;
	}
}

void CWeapon::Tick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	// weapon hits death-tile or left the game layer, reset it
	if (GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y) == TILE_DEATH || GameLayerClipped(m_Pos))
	{
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", "weapon_return");
		Reset();
		return;
	}

	if (m_Lifetime == 0)
	{
		Reset();
		return;
	}

	if (m_Lifetime != -1)
		m_Lifetime--;


	if (m_PickupDelay > 0)
		m_PickupDelay--;

	if (m_PickupDelay <= 0 || IsCharacterNear() != m_Owner)
		Pickup();


	m_Vel.y += GameServer()->Tuning()->m_Gravity;


	//Friction
	bool Grounded = false;
	if (GameServer()->Collision()->CheckPoint(m_Pos.x + ms_PhysSize / 2, m_Pos.y + ms_PhysSize / 2 + 5))
		Grounded = true;
	if (GameServer()->Collision()->CheckPoint(m_Pos.x - ms_PhysSize / 2, m_Pos.y + ms_PhysSize / 2 + 5))
		Grounded = true;

	if (Grounded == true)
		m_Vel.x *= 0.75f;
	else
		m_Vel.x *= 0.98f;


	//Speedups
	if (GameServer()->Collision()->IsSpeedup(GameServer()->Collision()->GetMapIndex(m_Pos)))
	{
		int Force, MaxSpeed = 0;
		vec2 Direction, MaxVel, TempVel = m_Vel;
		float TeeAngle, SpeederAngle, DiffAngle, SpeedLeft, TeeSpeed;
		GameServer()->Collision()->GetSpeedup(GameServer()->Collision()->GetMapIndex(m_Pos), &Direction, &Force, &MaxSpeed);

		if (Force == 255 && MaxSpeed)
		{
			m_Vel = Direction * (MaxSpeed / 5);
		}

		else
		{
			if (MaxSpeed > 0 && MaxSpeed < 5) MaxSpeed = 5;
			//dbg_msg("speedup tile start", "Direction %f %f, Force %d, Max Speed %d", (Direction).x, (Direction).y, Force, MaxSpeed);
			if (MaxSpeed > 0)
			{
				if (Direction.x > 0.0000001f)
					SpeederAngle = -atan(Direction.y / Direction.x);
				else if (Direction.x < 0.0000001f)
					SpeederAngle = atan(Direction.y / Direction.x) + 2.0f * asin(1.0f);
				else if (Direction.y > 0.0000001f)
					SpeederAngle = asin(1.0f);
				else
					SpeederAngle = asin(-1.0f);

				if (SpeederAngle < 0)
					SpeederAngle = 4.0f * asin(1.0f) + SpeederAngle;

				if (TempVel.x > 0.0000001f)
					TeeAngle = -atan(TempVel.y / TempVel.x);
				else if (TempVel.x < 0.0000001f)
					TeeAngle = atan(TempVel.y / TempVel.x) + 2.0f * asin(1.0f);
				else if (TempVel.y > 0.0000001f)
					TeeAngle = asin(1.0f);
				else
					TeeAngle = asin(-1.0f);

				if (TeeAngle < 0)
					TeeAngle = 4.0f * asin(1.0f) + TeeAngle;

				TeeSpeed = sqrt(pow(TempVel.x, 2) + pow(TempVel.y, 2));

				DiffAngle = SpeederAngle - TeeAngle;
				SpeedLeft = MaxSpeed / 5.0f - cos(DiffAngle) * TeeSpeed;
				//dbg_msg("speedup tile debug", "MaxSpeed %i, TeeSpeed %f, SpeedLeft %f, SpeederAngle %f, TeeAngle %f", MaxSpeed, TeeSpeed, SpeedLeft, SpeederAngle, TeeAngle);
				if (abs(SpeedLeft) > Force && SpeedLeft > 0.0000001f)
					TempVel += Direction * Force;
				else if (abs(SpeedLeft) > Force)
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

void CWeapon::Snap(int SnappingClient)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = POWERUP_WEAPON;
	pP->m_Subtype = m_Type;

	if (!m_Jetpack)
		return;

	CNetObj_Projectile *pJetpackIndicator = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_ID2, sizeof(CNetObj_Projectile)));
	if (!pJetpackIndicator)
		return;

	pJetpackIndicator->m_X = pP->m_X;
	pJetpackIndicator->m_Y = pP->m_Y - 25;
	pJetpackIndicator->m_Type = WEAPON_SHOTGUN;
	pJetpackIndicator->m_StartTick = Server()->Tick();
}
