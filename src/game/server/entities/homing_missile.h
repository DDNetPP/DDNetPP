/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_HOMINGMISSILE_H
#define GAME_SERVER_ENTITIES_HOMINGMISSILE_H

class CHomingMissile : public CEntity
{
public:
	CHomingMissile(CGameWorld *pGameWorld, int Lifetime, int Owner, float Force, vec2 Dir);

	virtual void Reset();
	virtual void Tick();
	virtual void Snap(int SnappingClient);

	CCharacter *CharacterNear();

	void CalculateVel();
	virtual void TickDefered();
	void Move(CCharacter *pTarget);
	bool Hit(CCharacter *pHitCharacter);

private:
	vec2 m_LastResetPos;
	int m_LastResetTick;
	bool m_CalculatedVel;
	int m_VelX;
	int m_VelY;

	int m_StartTick;

	float m_Force;
	vec2 m_Direction;
	int m_Owner;
	int m_Lifetime;
};

#endif
