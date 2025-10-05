#ifndef GAME_SERVER_ENTITIES_DROP_PICKUP_H
#define GAME_SERVER_ENTITIES_DROP_PICKUP_H

#include <game/server/entity.h>

class CDropPickup : public CEntity
{
public:
	CDropPickup(CGameWorld *pGameWorld, int Type, int Lifetime, int Owner, int Direction, float Force, int ResponsibleTeam);

	virtual void Reset() override;
	virtual void Tick() override;
	virtual void Snap(int SnappingClient) override;

	int IsCharacterNear();
	void Pickup();
	void Delete();

private:
	vec2 m_Vel;

	int m_Owner;
	int m_ResponsibleTeam;
	int m_Type;
	int m_Lifetime;

	int m_PickupDelay;
	bool m_EreasePickup;

	static int const ms_PhysSize = 14;
	int m_TuneZone;
};

#endif
