#include <base/log.h>
#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/player.h>
#include <game/server/teams.h>
#include <generated/protocol.h>

#include <cmath>

#include "pickup.h"
#include "weapon.h"

CWeapon::CWeapon(CGameWorld *pGameWorld, int Weapon, int Lifetime, int Owner, int DDRaceTeam, int Direction, int Bullets, bool Jetpack, bool SpreadGun) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_WEAPON)
{
	m_Type = Weapon;
	m_Lifetime = Server()->TickSpeed() * Lifetime;
	m_Pos = GameServer()->GetPlayerChar(Owner)->m_Pos;
	m_Jetpack = Jetpack;
	m_SpreadGun = SpreadGun;
	m_Bullets = Bullets;
	m_Owner = Owner;
	m_DDRaceTeam = DDRaceTeam;

	m_Vel = vec2(5 * Direction, -5);

	m_PickupDelay = Server()->TickSpeed() * 2;

	m_Id2 = Server()->SnapNewId();
	m_Id3 = Server()->SnapNewId();
	m_Id4 = Server()->SnapNewId();
	m_Id5 = Server()->SnapNewId();

	m_TuneZone = GameServer()->Collision()->IsTune(GameServer()->Collision()->GetMapIndex(m_Pos));

	GameWorld()->InsertEntity(this);
}

CWeapon::~CWeapon()
{
	Server()->SnapFreeId(m_Id2);
	Server()->SnapFreeId(m_Id3);
	Server()->SnapFreeId(m_Id4);
	Server()->SnapFreeId(m_Id5);
}

void CWeapon::Reset()
{
	if(m_MarkedForDestroy)
		return;

	if(m_Owner >= 0)
	{
		CPlayer *pOwner = GameServer()->m_apPlayers[m_Owner];
		if(pOwner)
		{
			std::vector<CWeapon *> &DroppedWeapons = pOwner->m_aWeaponLimit[m_Type];
			for(unsigned i = 0; i < DroppedWeapons.size(); i++)
			{
				if(DroppedWeapons[i] == this)
				{
					DroppedWeapons.erase(DroppedWeapons.begin() + i);
					break;
				}
			}
		}
	}

	if(IsCharacterNear() == -1)
		GameServer()->CreateDeath(m_Pos, -1);

	m_MarkedForDestroy = true;
}

void CWeapon::IsShieldNear()
{
	CPickup *apEnts[9];
	mem_zero(apEnts, sizeof(apEnts));
	int Num = GameWorld()->FindEntities(m_Pos, 20.0f, (CEntity **)apEnts, 9, CGameWorld::ENTTYPE_PICKUP);

	for(int i = 0; i < Num; i++)
	{
		CPickup *pShield = apEnts[i];

		if(pShield->Type() == POWERUP_ARMOR)
		{
			GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR);
			Reset();
			break;
		}
	}
}

int CWeapon::IsCharacterNear()
{
	CCharacter *apEnts[MAX_CLIENTS];
	int Num = GameWorld()->FindEntities(m_Pos, 20.0f, (CEntity **)apEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

	for(int i = 0; i < Num; ++i)
	{
		CCharacter *pChr = apEnts[i];
		if(pChr && pChr->IsAlive() && pChr->Team() == m_DDRaceTeam)
			return pChr->GetPlayer()->GetCid();
	}

	return -1;
}

void CWeapon::Pickup()
{
	int CharId = IsCharacterNear();
	if(CharId != -1)
	{
		CCharacter *pChar = GameServer()->GetPlayerChar(CharId);
		if(!pChar)
			return;
		if(pChar->GetPlayer()->m_SpookyGhostActive && m_Type != WEAPON_GUN)
			return;

		if(
			(pChar->GetPlayer()->m_SpawnRifleActive && m_Type == WEAPON_LASER) || (pChar->GetPlayer()->m_SpawnShotgunActive && m_Type == WEAPON_SHOTGUN) || (pChar->GetPlayer()->m_SpawnGrenadeActive && m_Type == WEAPON_GRENADE))
		{
			//
		}
		else if(pChar->GetWeaponGot(m_Type) && !m_Jetpack && !m_SpreadGun)
			return;

		if((m_Jetpack || m_SpreadGun) && !pChar->GetWeaponGot(WEAPON_GUN))
			return;

		if(m_SpreadGun && m_Jetpack && pChar->Core()->m_Jetpack && (pChar->m_autospreadgun || pChar->GetPlayer()->m_InfAutoSpreadGun))
			return;

		if(!m_SpreadGun && m_Jetpack && pChar->Core()->m_Jetpack)
			return;

		if(!m_Jetpack && m_SpreadGun && (pChar->m_autospreadgun || pChar->GetPlayer()->m_InfAutoSpreadGun))
			return;

		if(m_Type == WEAPON_LASER)
			pChar->GetPlayer()->m_SpawnRifleActive = 0;
		else if(m_Type == WEAPON_SHOTGUN)
			pChar->GetPlayer()->m_SpawnShotgunActive = 0;
		else if(m_Type == WEAPON_GRENADE)
			pChar->GetPlayer()->m_SpawnGrenadeActive = 0;

		pChar->GiveWeapon(m_Type, false, m_Bullets);

		pChar->SetActiveWeapon(m_Type);

		if((m_Bullets != -1) && !pChar->GetPlayer()->m_IsSurvivaling)
			pChar->m_aDecreaseAmmo[m_Type] = true;

		if(m_SpreadGun && (!pChar->m_autospreadgun && !pChar->GetPlayer()->m_InfAutoSpreadGun))
		{
			pChar->m_autospreadgun = true;
			GameServer()->SendChatTarget(pChar->GetPlayer()->GetCid(), "You have a spread gun");
		}
		if(m_Jetpack && !pChar->Core()->m_Jetpack)
		{
			pChar->Core()->m_Jetpack = true;
			GameServer()->SendChatTarget(pChar->GetPlayer()->GetCid(), "You have a jetpack gun");
		}

		if(m_Type == WEAPON_SHOTGUN || m_Type == WEAPON_LASER)
			GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, pChar->Teams()->TeamMask(pChar->Team()));
		else if(m_Type == WEAPON_GRENADE)
			GameServer()->CreateSound(m_Pos, SOUND_PICKUP_GRENADE, pChar->Teams()->TeamMask(pChar->Team()));
		else if(m_Type == WEAPON_HAMMER || m_Type == WEAPON_GUN)
			GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR, pChar->Teams()->TeamMask(pChar->Team()));

		Reset();
		return;
	}
}

void CWeapon::Tick()
{
	if(m_Owner != -1 && !GameServer()->m_apPlayers[m_Owner])
	{
		m_Owner = -1;
	}

	// weapon hits death-tile or left the game layer, reset it
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y) == TILE_DEATH || GameLayerClipped(m_Pos))
	{
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", "weapon_return");

		Reset();
		return;
	}

	if(m_Lifetime == 0)
	{
		Reset();
		return;
	}

	if(m_Lifetime != -1)
		m_Lifetime--;

	if(m_PickupDelay > 0)
		m_PickupDelay--;

	if(m_PickupDelay <= 0 || IsCharacterNear() != m_Owner)
		Pickup();

	IsShieldNear();

	m_Vel.y += GameServer()->Tuning()->m_Gravity;

	//Friction
	bool Grounded = false;
	if(GameServer()->Collision()->CheckPoint(m_Pos.x + ms_PhysSize / 2, m_Pos.y + ms_PhysSize / 2 + 5))
		Grounded = true;
	if(GameServer()->Collision()->CheckPoint(m_Pos.x - ms_PhysSize / 2, m_Pos.y + ms_PhysSize / 2 + 5))
		Grounded = true;

	if(Grounded)
		m_Vel.x *= 0.75f;
	else
		m_Vel.x *= 0.98f;

	//Speedups
	if(GameServer()->Collision()->IsSpeedup(GameServer()->Collision()->GetMapIndex(m_Pos)))
	{
		int Force, Type, MaxSpeed = 0;
		vec2 Direction, TempVel = m_Vel;
		float TeeAngle, SpeederAngle, DiffAngle, SpeedLeft, TeeSpeed;
		GameServer()->Collision()->GetSpeedup(GameServer()->Collision()->GetMapIndex(m_Pos), &Direction, &Force, &MaxSpeed, &Type);

		if(Type == TILE_SPEED_BOOST)
		{
			// missing changes from this commit
			// https://github.com/ddnet/ddnet/commit/09eb4e62010af13a9b68189b56e882de24c164e6
			log_error("ddnet++", "warning new speedup not supported yet.");
		}

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
					SpeederAngle = atan(Direction.y / Direction.x) + 2.0f * std::asin(1.0f);
				else if(Direction.y > 0.0000001f)
					SpeederAngle = std::asin(1.0f);
				else
					SpeederAngle = std::asin(-1.0f);

				if(SpeederAngle < 0)
					SpeederAngle = 4.0f * std::asin(1.0f) + SpeederAngle;

				if(TempVel.x > 0.0000001f)
					TeeAngle = -atan(TempVel.y / TempVel.x);
				else if(TempVel.x < 0.0000001f)
					TeeAngle = atan(TempVel.y / TempVel.x) + 2.0f * std::asin(1.0f);
				else if(TempVel.y > 0.0000001f)
					TeeAngle = std::asin(1.0f);
				else
					TeeAngle = std::asin(-1.0f);

				if(TeeAngle < 0)
					TeeAngle = 4.0f * std::asin(1.0f) + TeeAngle;

				TeeSpeed = sqrt(pow(TempVel.x, 2) + pow(TempVel.y, 2));

				DiffAngle = SpeederAngle - TeeAngle;
				SpeedLeft = MaxSpeed / 5.0f - std::cos(DiffAngle) * TeeSpeed;
				//dbg_msg("speedup tile debug", "MaxSpeed %i, TeeSpeed %f, SpeedLeft %f, SpeederAngle %f, TeeAngle %f", MaxSpeed, TeeSpeed, SpeedLeft, SpeederAngle, TeeAngle);
				if(std::abs(SpeedLeft) > Force && SpeedLeft > 0.0000001f)
					TempVel += Direction * Force;
				else if(std::abs(SpeedLeft) > Force)
					TempVel += Direction * -Force;
				else
					TempVel += Direction * SpeedLeft;
			}
			else
				TempVel += Direction * Force;
		}
		m_Vel = TempVel;
	}

	GameServer()->Collision()->MoveBox(
		&m_Pos,
		&m_Vel,
		vec2(ms_PhysSize, ms_PhysSize),
		vec2(GameServer()->TuningList()[m_TuneZone].m_GroundElasticityX, GameServer()->TuningList()[m_TuneZone].m_GroundElasticityY));
}

void CWeapon::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CCharacter *pSnappingChar = GameServer()->GetPlayerChar(SnappingClient);
	if(pSnappingChar && pSnappingChar->Team() != m_DDRaceTeam)
		return;

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, GetId(), sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = POWERUP_WEAPON;
	pP->m_Subtype = m_Type;

	int JetpackIndicatorHeight;

	if(m_Jetpack && !m_SpreadGun)
	{
		JetpackIndicatorHeight = 25;
	}
	else
	{
		JetpackIndicatorHeight = 45;
	}

	if(m_Jetpack)
	{
		CNetObj_Projectile *pJetpackIndicator = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_Id2, sizeof(CNetObj_Projectile)));
		if(pJetpackIndicator)
		{
			pJetpackIndicator->m_X = pP->m_X;
			pJetpackIndicator->m_Y = pP->m_Y - JetpackIndicatorHeight;
			pJetpackIndicator->m_Type = WEAPON_SHOTGUN;
			pJetpackIndicator->m_StartTick = Server()->Tick();
		}
	}

	if(m_SpreadGun)
	{
		CNetObj_Projectile *pSpreadGunIndicator1 = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_Id3, sizeof(CNetObj_Projectile)));
		if(pSpreadGunIndicator1)
		{
			pSpreadGunIndicator1->m_X = pP->m_X;
			pSpreadGunIndicator1->m_Y = pP->m_Y - 25;
			pSpreadGunIndicator1->m_Type = WEAPON_SHOTGUN;
			pSpreadGunIndicator1->m_StartTick = Server()->Tick();
		}

		CNetObj_Projectile *pSpreadGunIndicator2 = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_Id4, sizeof(CNetObj_Projectile)));
		if(pSpreadGunIndicator2)
		{
			pSpreadGunIndicator2->m_X = pP->m_X - 20;
			pSpreadGunIndicator2->m_Y = pP->m_Y - 25;
			pSpreadGunIndicator2->m_Type = WEAPON_SHOTGUN;
			pSpreadGunIndicator2->m_StartTick = Server()->Tick();
		}

		CNetObj_Projectile *pSpreadGunIndicator3 = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_Id5, sizeof(CNetObj_Projectile)));
		if(pSpreadGunIndicator3)
		{
			pSpreadGunIndicator3->m_X = pP->m_X + 20;
			pSpreadGunIndicator3->m_Y = pP->m_Y - 25;
			pSpreadGunIndicator3->m_Type = WEAPON_SHOTGUN;
			pSpreadGunIndicator3->m_StartTick = Server()->Tick();
		}
	}
}
