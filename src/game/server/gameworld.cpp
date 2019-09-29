/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include "gameworld.h"
#include "entity.h"
#include "gamecontext.h"
#include <algorithm>
#include <utility>
#include <engine/shared/config.h>

//////////////////////////////////////////////////
// game world
//////////////////////////////////////////////////
CGameWorld::CGameWorld()
{
	m_pGameServer = 0x0;
	m_pServer = 0x0;

	m_Paused = false;
	m_ResetRequested = false;
	for(int i = 0; i < NUM_ENTTYPES; i++)
		m_apFirstEntityTypes[i] = 0;
}

CGameWorld::~CGameWorld()
{
	// delete all entities
	for(int i = 0; i < NUM_ENTTYPES; i++)
		while(m_apFirstEntityTypes[i])
			delete m_apFirstEntityTypes[i];
}

void CGameWorld::SetGameServer(CGameContext *pGameServer)
{
	m_pGameServer = pGameServer;
	m_pServer = m_pGameServer->Server();
}

CEntity *CGameWorld::FindFirst(int Type)
{
	return Type < 0 || Type >= NUM_ENTTYPES ? 0 : m_apFirstEntityTypes[Type];
}

int CGameWorld::FindEntities(vec2 Pos, float Radius, CEntity **ppEnts, int Max, int Type)
{
	if(Type < 0 || Type >= NUM_ENTTYPES)
		return 0;

	int Num = 0;
	for(CEntity *pEnt = m_apFirstEntityTypes[Type];	pEnt; pEnt = pEnt->m_pNextTypeEntity)
	{
		if(distance(pEnt->m_Pos, Pos) < Radius+pEnt->m_ProximityRadius)
		{
			if(ppEnts)
				ppEnts[Num] = pEnt;
			Num++;
			if(Num == Max)
				break;
		}
	}

	return Num;
}

void CGameWorld::InsertEntity(CEntity *pEnt)
{
#ifdef CONF_DEBUG
	for(CEntity *pCur = m_apFirstEntityTypes[pEnt->m_ObjType]; pCur; pCur = pCur->m_pNextTypeEntity)
		dbg_assert(pCur != pEnt, "err");
#endif

	// insert it
	if(m_apFirstEntityTypes[pEnt->m_ObjType])
		m_apFirstEntityTypes[pEnt->m_ObjType]->m_pPrevTypeEntity = pEnt;
	pEnt->m_pNextTypeEntity = m_apFirstEntityTypes[pEnt->m_ObjType];
	pEnt->m_pPrevTypeEntity = 0x0;
	m_apFirstEntityTypes[pEnt->m_ObjType] = pEnt;
}

void CGameWorld::DestroyEntity(CEntity *pEnt)
{
	pEnt->m_MarkedForDestroy = true;
}

void CGameWorld::RemoveEntity(CEntity *pEnt)
{
	// not in the list
	if(!pEnt->m_pNextTypeEntity && !pEnt->m_pPrevTypeEntity && m_apFirstEntityTypes[pEnt->m_ObjType] != pEnt)
		return;

	// remove
	if(pEnt->m_pPrevTypeEntity)
		pEnt->m_pPrevTypeEntity->m_pNextTypeEntity = pEnt->m_pNextTypeEntity;
	else
		m_apFirstEntityTypes[pEnt->m_ObjType] = pEnt->m_pNextTypeEntity;
	if(pEnt->m_pNextTypeEntity)
		pEnt->m_pNextTypeEntity->m_pPrevTypeEntity = pEnt->m_pPrevTypeEntity;

	// keep list traversing valid
	if(m_pNextTraverseEntity == pEnt)
		m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;

	pEnt->m_pNextTypeEntity = 0;
	pEnt->m_pPrevTypeEntity = 0;
}

//
void CGameWorld::Snap(int SnappingClient)
{
	for(int i = 0; i < NUM_ENTTYPES; i++)
		for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; )
		{
			m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
			pEnt->Snap(SnappingClient);
			pEnt = m_pNextTraverseEntity;
		}
}

void CGameWorld::Reset()
{
	// reset all entities
	for(int i = 0; i < NUM_ENTTYPES; i++)
		for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; )
		{
			m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
			pEnt->Reset();
			pEnt = m_pNextTraverseEntity;
		}
	RemoveEntities();

	GameServer()->m_pController->PostReset();
	RemoveEntities();

	m_ResetRequested = false;
}

void CGameWorld::RemoveEntities()
{
	// destroy objects marked for destruction
	for(int i = 0; i < NUM_ENTTYPES; i++)
		for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; )
		{
			m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
			if(pEnt->m_MarkedForDestroy)
			{
				RemoveEntity(pEnt);
				pEnt->Destroy();
			}
			pEnt = m_pNextTraverseEntity;
		}
}

bool distCompare(std::pair<float,int> a, std::pair<float,int> b)
{
	return (a.first < b.first);
}

void CGameWorld::UpdatePlayerMaps()
{
	if (Server()->Tick() % g_Config.m_SvMapUpdateRate != 0) return;

	std::pair<float,int> dist[MAX_CLIENTS];
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (!Server()->ClientIngame(i)) continue;
		int* map = Server()->GetIdMap(i);

		// compute distances
		for (int j = 0; j < MAX_CLIENTS; j++)
		{
			dist[j].second = j;
			if (!Server()->ClientIngame(j) || !GameServer()->m_apPlayers[j])
			{
				dist[j].first = 1e10;
				continue;
			}
			CCharacter* ch = GameServer()->m_apPlayers[j]->GetCharacter();
			if (!ch)
			{
				dist[j].first = 1e9;
				continue;
			}
			// copypasted chunk from character.cpp Snap() follows
			CCharacter* SnapChar = GameServer()->GetPlayerChar(i);
			if(SnapChar && !SnapChar->m_Super &&
				!GameServer()->m_apPlayers[i]->m_Paused && GameServer()->m_apPlayers[i]->GetTeam() != -1 &&
				!ch->CanCollide(i) &&
				(!GameServer()->m_apPlayers[i] ||
					GameServer()->m_apPlayers[i]->m_ClientVersion == VERSION_VANILLA ||
					(GameServer()->m_apPlayers[i]->m_ClientVersion >= VERSION_DDRACE &&
					!GameServer()->m_apPlayers[i]->m_ShowOthers
					)
				)
			)
				dist[j].first = 1e8;
			else
				dist[j].first = 0;

			dist[j].first += distance(GameServer()->m_apPlayers[i]->m_ViewPos, GameServer()->m_apPlayers[j]->GetCharacter()->m_Pos);
		}

		// always send the player himself
		dist[i].first = 0;

		// compute reverse map
		int rMap[MAX_CLIENTS];
		for (int j = 0; j < MAX_CLIENTS; j++)
		{
			rMap[j] = -1;
		}
		for (int j = 0; j < VANILLA_MAX_CLIENTS; j++)
		{
			if (map[j] == -1) continue;
			if (dist[map[j]].first > 5e9) map[j] = -1;
			else rMap[map[j]] = j;
		}

		std::nth_element(&dist[0], &dist[VANILLA_MAX_CLIENTS - 1], &dist[MAX_CLIENTS], distCompare);

		int mapc = 0;
		int demand = 0;
		for (int j = 0; j < VANILLA_MAX_CLIENTS - 1; j++)
		{
			int k = dist[j].second;
			if (rMap[k] != -1 || dist[j].first > 5e9) continue;
			while (mapc < VANILLA_MAX_CLIENTS && map[mapc] != -1) mapc++;
			if (mapc < VANILLA_MAX_CLIENTS - 1)
				map[mapc] = k;
			else
				demand++;
		}
		for (int j = MAX_CLIENTS - 1; j > VANILLA_MAX_CLIENTS - 2; j--)
		{
			int k = dist[j].second;
			if (rMap[k] != -1 && demand-- > 0)
				map[rMap[k]] = -1;
		}
		map[VANILLA_MAX_CLIENTS - 1] = -1; // player with empty name to say chat msgs
	}
}

void CGameWorld::Tick()
{
	if(m_ResetRequested)
		Reset();

	if(!m_Paused)
	{
		// update all objects
		for(int i = 0; i < NUM_ENTTYPES; i++)
			for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; )
			{
				m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
				pEnt->Tick();
				pEnt = m_pNextTraverseEntity;
			}

		for(int i = 0; i < NUM_ENTTYPES; i++)
			for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; )
			{
				m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
				pEnt->TickDefered();
				pEnt = m_pNextTraverseEntity;
			}
	}
	else
	{
		// update all objects
		for(int i = 0; i < NUM_ENTTYPES; i++)
			for(CEntity *pEnt = m_apFirstEntityTypes[i]; pEnt; )
			{
				m_pNextTraverseEntity = pEnt->m_pNextTypeEntity;
				pEnt->TickPaused();
				pEnt = m_pNextTraverseEntity;
			}
	}

	RemoveEntities();

	UpdatePlayerMaps();
}

// TODO: should be more general
//CCharacter *CGameWorld::IntersectCharacter(vec2 Pos0, vec2 Pos1, float Radius, vec2& NewPos, CEntity *pNotThis)
CCharacter *CGameWorld::IntersectCharacter(vec2 Pos0, vec2 Pos1, float Radius, vec2& NewPos, CCharacter *pNotThis, int CollideWith, class CCharacter *pThisOnly)
{
	// Find other players
	float ClosestLen = distance(Pos0, Pos1) * 100.0f;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)FindFirst(ENTTYPE_CHARACTER);
	for(; p; p = (CCharacter *)p->TypeNext())
	{
		if(p == pNotThis)
			continue;

		if(CollideWith != -1 && !p->CanCollide(CollideWith))
			continue;

		vec2 IntersectPos = closest_point_on_line(Pos0, Pos1, p->m_Pos);
		float Len = distance(p->m_Pos, IntersectPos);
		if(Len < p->m_ProximityRadius+Radius)
		{
			Len = distance(Pos0, IntersectPos);
			if(Len < ClosestLen)
			{
				NewPos = IntersectPos;
				ClosestLen = Len;
				pClosest = p;
			}
		}
	}

	return pClosest;
}


CCharacter *CGameWorld::ClosestCharacter(vec2 Pos, float Radius, CEntity *pNotThis)
{
	// Find other players
	float ClosestRange = Radius*2;
	CCharacter *pClosest = 0;

	CCharacter *p = (CCharacter *)GameServer()->m_World.FindFirst(ENTTYPE_CHARACTER);
	for(; p; p = (CCharacter *)p->TypeNext())
	{
		if(p == pNotThis)
			continue;

		float Len = distance(Pos, p->m_Pos);
		if(Len < p->m_ProximityRadius+Radius)
		{
			if(Len < ClosestRange)
			{
				ClosestRange = Len;
				pClosest = p;
			}
		}
	}

	return pClosest;
}

std::list<class CCharacter *> CGameWorld::IntersectedCharacters(vec2 Pos0, vec2 Pos1, float Radius, class CEntity *pNotThis)
{
	std::list< CCharacter * > listOfChars;

	CCharacter *pChr = (CCharacter *)FindFirst(CGameWorld::ENTTYPE_CHARACTER);
	for(; pChr; pChr = (CCharacter *)pChr->TypeNext())
	{
		if(pChr == pNotThis)
			continue;

		vec2 IntersectPos = closest_point_on_line(Pos0, Pos1, pChr->m_Pos);
		float Len = distance(pChr->m_Pos, IntersectPos);
		if(Len < pChr->m_ProximityRadius+Radius)
		{
			pChr->m_Intersection = IntersectPos;
			listOfChars.push_back(pChr);
		}
	}
	return listOfChars;
}

void CGameWorld::ReleaseHooked(int ClientID)
{
	CCharacter *pChr = (CCharacter *)CGameWorld::FindFirst(CGameWorld::ENTTYPE_CHARACTER);
		for(; pChr; pChr = (CCharacter *)pChr->TypeNext())
		{
			CCharacterCore* Core = pChr->Core();
			if(Core->m_HookedPlayer == ClientID && !pChr->m_Super)
			{
				Core->m_HookedPlayer = -1;
				Core->m_HookState = HOOK_RETRACTED;
				Core->m_TriggeredEvents |= COREEVENT_HOOK_RETRACT;
				Core->m_HookState = HOOK_RETRACTED;
			}
		}
}

CCharacter *CGameWorld::ClosestCharType(vec2 Pos, bool Human, CCharacter *pNotThis, bool SeeAll)
{
	// Find Humans
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

		float Len = distance(Pos, p->m_Pos);

		if (Len < ClosestRange || !ClosestRange)
		{
			ClosestRange = Len;
			pClosest = p;
		}
	}

	return pClosest;
}

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
