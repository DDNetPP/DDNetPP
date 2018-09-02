/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_WEAPON_H
#define GAME_SERVER_ENTITIES_WEAPON_H

class CWeapon : public CEntity
{
public:
	CWeapon(CGameWorld *pGameWorld, int Weapon, int Lifetime, int Owner, int Direction, int ResponsibleTeam, bool Jetpack);

	virtual void Reset();
	virtual void Tick();
	virtual void Snap(int SnappingClient);

	int IsCharacterNear();

private:
	vec2 m_Vel;

	int m_Owner;
	int m_ResponsibleTeam;
	int m_Type;
	int m_Lifetime;

	int m_PickupDelay;

	bool m_Jetpack;

	static int const ms_PhysSize = 14;

	// have to define a new ID variable for the bullet
	int m_ID2;
};

#endif
