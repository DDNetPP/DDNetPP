/* ddnet++ 2021 */

#include "gameworld.h"
#include "entity.h"
#include "gamecontext.h"
#include <algorithm>
#include <utility>
#include <engine/shared/config.h>

//////////////////////////////////////////////////
// game world ddnet++
//////////////////////////////////////////////////

CCharacter *CGameWorld::ClosestCharTypeRuler(vec2 Pos, bool Human, CCharacter *pNotThis)  // Chilidreghuns function stolen from piku 
{   
	// Find Humans
	float ClosestRange = 0.f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)GameServer()->m_World.FindFirst(ENTTYPE_CHARACTER);
	for (; p; p = (CCharacter *)p->TypeNext())
	{
		if (p == pNotThis)
			continue;

		if (!g_Config.m_SvDummySeeDummy)
		{
			if (Human && p->GetPlayer()->m_IsDummy)
				continue;
			else if (!Human && !p->GetPlayer()->m_IsDummy)
				continue;
		}

		if (p->m_Pos.y > 213 * 32 || p->m_Pos.x < 416 * 32 || p->m_Pos.x > 446 * 32 || p->m_Pos.y < 198 * 32) // wenn der spieler nicht in der ruler area gefunden wurde such weiter (hoff ich mal xD)
			continue;


		float Len = distance(Pos, p->m_Pos);

		if (Len < ClosestRange || !ClosestRange)
		{
			ClosestRange = Len;
			pClosest = p;
		}
	}

	return pClosest;
}


CCharacter *CGameWorld::ClosestCharTypeRuler2(vec2 Pos, bool Human, CCharacter *pNotThis)  // Ruler2 is nur im ruler bereich ohne linke freeze wand  (gemacht f�r mode18 == 2)
{
	// Find Humans
	float ClosestRange = 0.f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)GameServer()->m_World.FindFirst(ENTTYPE_CHARACTER);
	for (; p; p = (CCharacter *)p->TypeNext())
	{
		if (p == pNotThis)
			continue;

		if (!g_Config.m_SvDummySeeDummy)
		{
			if (Human && p->GetPlayer()->m_IsDummy)
				continue;
			else if (!Human && !p->GetPlayer()->m_IsDummy)
				continue;
		}

		if (p->m_Pos.y > 213 * 32 || p->m_Pos.x < 417 * 32 || p->m_Pos.x > 444 * 32 || p->m_Pos.y < 198 * 32) // wenn der gegner im unfreeze bereich der ruler area ist
			continue;


		float Len = distance(Pos, p->m_Pos);

		if (Len < ClosestRange || !ClosestRange)
		{
			ClosestRange = Len;
			pClosest = p;
		}
	}

	return pClosest;
}


CCharacter *CGameWorld::ClosestCharTypeRulerLeftFreeze(vec2 Pos, bool Human, CCharacter *pNotThis) 
{
	// Find Humans
	float ClosestRange = 0.f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)GameServer()->m_World.FindFirst(ENTTYPE_CHARACTER);
	for (; p; p = (CCharacter *)p->TypeNext())
	{
		if (p == pNotThis)
			continue;

		if (!g_Config.m_SvDummySeeDummy)
		{
			if (Human && p->GetPlayer()->m_IsDummy)
				continue;
			else if (!Human && !p->GetPlayer()->m_IsDummy)
				continue;
		}

		//if (p->m_Pos.y > 213 * 32 || p->m_Pos.x < 415 * 32 || p->m_Pos.x > 416 * 32)
		if (p->m_Pos.y > 213 * 32 || p->m_Pos.x < 416 * 32 || p->m_Pos.x > 417 * 32 - 10 || p->m_Pos.y < 198 * 32) // wenn der spieler nicht in der ruler area (im linken freeze) gefunden wurde
			continue;


		float Len = distance(Pos, p->m_Pos);

		if (Len < ClosestRange || !ClosestRange)
		{
			ClosestRange = Len;
			pClosest = p;
		}
	}

	return pClosest;
}

CCharacter *CGameWorld::ClosestCharTypeRulerWB(vec2 Pos, bool Human, CCharacter *pNotThis) 
{
	// Find Humans
	float ClosestRange = 0.f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)GameServer()->m_World.FindFirst(ENTTYPE_CHARACTER);
	for (; p; p = (CCharacter *)p->TypeNext())
	{
		if (p == pNotThis)
			continue;

		if (!g_Config.m_SvDummySeeDummy)
		{
			if (Human && p->GetPlayer()->m_IsDummy)
				continue;
			else if (!Human && !p->GetPlayer()->m_IsDummy)
				continue;
		}

		if (p->m_Pos.y > 213 * 32 || p->m_Pos.x < 434 * 32 || p->m_Pos.x > 441 * 32 || p->m_Pos.y < 198 * 32) // nur der wb bereich [neu der Wbbereich h�rt bei x: 441 auf]
			continue;


		float Len = distance(Pos, p->m_Pos);

		if (Len < ClosestRange || !ClosestRange)
		{
			ClosestRange = Len;
			pClosest = p;
		}
	}

	return pClosest;
}

CCharacter *CGameWorld::ClosestCharTypeTunnel(vec2 Pos, bool Human, CCharacter *pNotThis)
{
	// Find Humans
	float ClosestRange = 0.f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)GameServer()->m_World.FindFirst(ENTTYPE_CHARACTER);
	for (; p; p = (CCharacter *)p->TypeNext())
	{
		if (p == pNotThis)
			continue;

		if (!g_Config.m_SvDummySeeDummy)
		{
			if (Human && p->GetPlayer()->m_IsDummy)
				continue;
			else if (!Human && !p->GetPlayer()->m_IsDummy)
				continue;
		}

		if (p->m_Pos.y < 213 * 32 || p->m_Pos.x > 429 * 32 || p->m_Pos.x < 419 * 32 || p->m_Pos.y > 218 * 32 + 60) // nur der tunnel
			continue;


		float Len = distance(Pos, p->m_Pos);

		if (Len < ClosestRange || !ClosestRange)
		{
			ClosestRange = Len;
			pClosest = p;
		}
	}

	return pClosest;
}

/*
	Lower wayblock to check if its a potential attacker
	who wants to hook the bot from below
*/
CCharacter *CGameWorld::ClosestCharTypeRulerWBBottom(vec2 Pos, bool Human, CCharacter *pNotThis)
{
	// Find Humans
	float ClosestRange = 0.f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)GameServer()->m_World.FindFirst(ENTTYPE_CHARACTER);
	for (; p; p = (CCharacter *)p->TypeNext())
	{
		if (p == pNotThis)
			continue;

		if (!g_Config.m_SvDummySeeDummy)
		{
			if (Human && p->GetPlayer()->m_IsDummy)
				continue;
			else if (!Human && !p->GetPlayer()->m_IsDummy)
				continue;
		}

		if (p->m_Pos.y > 218 * 32 || p->m_Pos.y < 215 * 32 || p->m_Pos.x < 435 * 32 || p->m_Pos.x > 439 * 32) // lower wayblock area
			continue;


		float Len = distance(Pos, p->m_Pos);

		if (Len < ClosestRange || !ClosestRange)
		{
			ClosestRange = Len;
			pClosest = p;
		}
	}

	return pClosest;
}


CCharacter *CGameWorld::ClosestCharTypeDummy(vec2 Pos, CCharacter *pNotThis)  // find closest dummy
{
	// Find Dummys
	float ClosestRange = 0.f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)GameServer()->m_World.FindFirst(ENTTYPE_CHARACTER);
	for (; p; p = (CCharacter *)p->TypeNext())
	{
		if (p == pNotThis)
			continue;

		//if (Human && p->GetPlayer()->m_IsDummy)
		//	continue;
		//else if (!Human && !p->GetPlayer()->m_IsDummy)
		//	continue;

		if (!p->GetPlayer()->m_IsDummy)
			continue;

		float Len = distance(Pos, p->m_Pos);

		if (Len < ClosestRange || !ClosestRange)
		{
			ClosestRange = Len;
			pClosest = p;
		}
	}

	return pClosest;
}

CCharacter *CGameWorld::ClosestCharTypeFarInRace(vec2 Pos, bool Human, CCharacter *pNotThis)  // find closest dummy far in race
{
	// Find Dummys
	float ClosestRange = 0.f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)GameServer()->m_World.FindFirst(ENTTYPE_CHARACTER);
	for (; p; p = (CCharacter *)p->TypeNext())
	{
		if (p == pNotThis)
			continue;

		if (!g_Config.m_SvDummySeeDummy)
		{
			if (Human && p->GetPlayer()->m_IsDummy)
				continue;
			else if (!Human && !p->GetPlayer()->m_IsDummy)
				continue;
		}

		//if (p->m_Pos.y > 200 * 32 || p->m_Pos.x < 466 * 32 || p->m_Pos.x > 498 * 32) // around the 2player part
		//	continue;

		if (p->m_Pos.y > 200 * 32 || p->m_Pos.x < 466 * 32) //ab weit
			continue;


		float Len = distance(Pos, p->m_Pos);

		if (Len < ClosestRange || !ClosestRange)
		{
			ClosestRange = Len;
			pClosest = p;
		}
	}

	return pClosest;
}

CCharacter *CGameWorld::ClosestCharTypePoliceFreezeHole(vec2 Pos, bool Human, CCharacter *pNotThis)  // BlmapChill right police freeze
{
	// Find Humans
	float ClosestRange = 0.f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)GameServer()->m_World.FindFirst(ENTTYPE_CHARACTER);
	for (; p; p = (CCharacter *)p->TypeNext())
	{
		if (p == pNotThis)
			continue;

		if (!g_Config.m_SvDummySeeDummy)
		{
			if (Human && p->GetPlayer()->m_IsDummy)
				continue;
			else if (!Human && !p->GetPlayer()->m_IsDummy)
				continue;
		}

		if (!p->GetPlayer()->m_PoliceRank && !p->GetPlayer()->m_PoliceHelper)
			continue;
		if (p->m_FreezeTime == 0 || p->m_Pos.y > 438 * 32 || p->m_Pos.x < 430 * 32 || p->m_Pos.x > 445 * 32 || p->m_Pos.y < 423 * 32) // only freezed tees in the hole coming from short entry into blmapchill police base
			continue;

		float Len = distance(Pos, p->m_Pos);

		if (Len < ClosestRange || !ClosestRange)
		{
			ClosestRange = Len;
			pClosest = p;
		}
	}

	return pClosest;
}

CCharacter *CGameWorld::ClosestCharTypePoliceFreezePitLeft(vec2 Pos, bool Human, CCharacter *pNotThis)  // BlmapChill left freeze pit
{
	// Find Humans
	float ClosestRange = 0.f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)GameServer()->m_World.FindFirst(ENTTYPE_CHARACTER);
	for (; p; p = (CCharacter *)p->TypeNext())
	{
		if (p == pNotThis)
			continue;

		if (!g_Config.m_SvDummySeeDummy)
		{
			if (Human && p->GetPlayer()->m_IsDummy)
				continue;
			else if (!Human && !p->GetPlayer()->m_IsDummy)
				continue;
		}

		if (!p->GetPlayer()->m_PoliceRank && !p->GetPlayer()->m_PoliceHelper)
			continue;
		if (p->m_FreezeTime == 0 || p->m_Pos.y > 436 * 32 || p->m_Pos.x < 363 * 32 || p->m_Pos.x > 381 * 32 || p->m_Pos.y < 420 * 32)
			continue;

		float Len = distance(Pos, p->m_Pos);

		if (Len < ClosestRange || !ClosestRange)
		{
			ClosestRange = Len;
			pClosest = p;
		}
	}

	return pClosest;
}

CCharacter *CGameWorld::ClosestCharTypeFreeze(vec2 Pos, bool Human, CCharacter *pNotThis, bool SeeAll)
{
	float ClosestRange = 0.f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)GameServer()->m_World.FindFirst(ENTTYPE_CHARACTER);
	for (; p; p = (CCharacter *)p->TypeNext())
	{
		if (p == pNotThis)
			continue;

		if (!SeeAll)
		{
			if (Human && p->GetPlayer()->m_IsDummy)
				continue;
			else if (!Human && !p->GetPlayer()->m_IsDummy)
				continue;
		}


		if (p->m_FreezeTime == 0) //freezed -> continue
			continue;


		float Len = distance(Pos, p->m_Pos);

		if (Len < ClosestRange || !ClosestRange)
		{
			ClosestRange = Len;
			pClosest = p;
		}
	}

	return pClosest;
}

CCharacter *CGameWorld::ClosestCharTypeNotInFreeze(vec2 Pos, bool Human, CCharacter *pNotThis, bool SeeAll)
{
	float ClosestRange = 0.f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)GameServer()->m_World.FindFirst(ENTTYPE_CHARACTER);
	for (; p; p = (CCharacter *)p->TypeNext())
	{
		if (p == pNotThis)
			continue;

		if (!SeeAll)
		{
			if (Human && p->GetPlayer()->m_IsDummy)
				continue;
			else if (!Human && !p->GetPlayer()->m_IsDummy)
				continue;
		}

		if (p->isFreezed)
			continue;

		float Len = distance(Pos, p->m_Pos);

		if (Len < ClosestRange || !ClosestRange)
		{
			ClosestRange = Len;
			pClosest = p;
		}
	}

	return pClosest;
}

CCharacter *CGameWorld::ClosestCharTypeUnfreezedArea5(vec2 Pos, bool Human, CCharacter *pNotThis, bool SeeAll)  // blmapV5 potential enemys in area5
{
	float ClosestRange = 0.f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)GameServer()->m_World.FindFirst(ENTTYPE_CHARACTER);
	for (; p; p = (CCharacter *)p->TypeNext())
	{
		if (p == pNotThis)
			continue;

		if (p->m_Pos.y < 84 * 32 || p->m_Pos.y > 104 * 32 || p->m_Pos.x < 32 || p->m_Pos.x > 36 * 32)
			continue;

		if (!SeeAll)
		{
			if (Human && p->GetPlayer()->m_IsDummy)
				continue;
			else if (!Human && !p->GetPlayer()->m_IsDummy)
				continue;
		}


		if (p->isFreezed) //freezed -> continue
			continue;


		float Len = distance(Pos, p->m_Pos);

		if (Len < ClosestRange || !ClosestRange)
		{
			ClosestRange = Len;
			pClosest = p;
		}
	}

	return pClosest;
}