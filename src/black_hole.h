/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef BLACK_HOLE_H
#define BLACK_HOLE_H

#include <game/server/entity.h>

class CBlackHole : public CEntity
{
public:
	CBlackHole(CGameWorld *pGameWorld, vec2 Pos, int Owner);

	virtual void Attract();
	virtual void CreateHole();

	virtual void Reset() override;
	virtual void Tick() override;
	virtual void Snap(int SnappingClient) override;

private:
	int m_Owner;
	int m_Buffer;
	bool GoingToKill;
	int m_Timer;

	bool Attracting;
};

#endif
