#ifndef GAME_SERVER_ENTITIES_WEAPON_H
#define GAME_SERVER_ENTITIES_WEAPON_H

#include <base/vmath.h>
#include <game/server/entity.h>
#include <game/server/gameworld.h>

class CWeapon : public CEntity
{
public:
	CWeapon(CGameWorld *pGameWorld, int Weapon, int Lifetime, int Owner, int DDRaceTeam, int Direction, int Bullets, bool Jetpack = false, bool SpreadGun = false);
	~CWeapon() override;

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;

	int IsCharacterNear();

	void IsShieldNear();

	void Pickup();

private:
	vec2 m_Vel;

	int m_DDRaceTeam;
	int m_Owner;
	int m_Type;
	int m_Lifetime;
	int m_Bullets;

	int m_PickupDelay;

	bool m_Jetpack;
	bool m_SpreadGun;

	static int const ms_PhysSize = 14;

	// have to define a new ID variable for the bullet
	int m_Id2;
	int m_Id3;
	int m_Id4;
	int m_Id5;

	int m_TuneZone;
};

#endif
