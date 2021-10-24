#include <game/server/gamecontext.h>

#include "plant.h"

CPlant::CPlant(CGameWorld *pGameWorld, vec2 Pos) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_PROJECTILE)
{
	m_Pos = Pos;
	m_Pos.y += 15;
	m_GrowDelay = rand() % 100 + 50;
	m_LastResetTick = Server()->Tick();
	m_CalculatedVel = false;

	GameWorld()->InsertEntity(this);
}

void CPlant::Reset()
{
	GameServer()->m_World.DestroyEntity(this);
}

void CPlant::TickDefered()
{
	if(Server()->Tick() % 4 == 1)
	{
		m_LastResetPos = m_Pos;
		m_LastResetTick = Server()->Tick();
	}
	m_CalculatedVel = false;
}

void CPlant::CalculateVel()
{
	float Time = (Server()->Tick() - m_LastResetTick) / (float)Server()->TickSpeed();
	float Curvature = 0;
	float Speed = 0;

	Curvature = GameServer()->Tuning()->m_ShotgunCurvature;
	Speed = GameServer()->Tuning()->m_ShotgunSpeed;

	m_VelX = ((m_Pos.x - m_LastResetPos.x) / Time / Speed) * 100;
	m_VelY = ((m_Pos.y - m_LastResetPos.y) / Time / Speed - Time * Speed * Curvature / 10000) * 100;

	m_CalculatedVel = true;
}

int CPlant::IsCharacterNear()
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

void CPlant::ResetProjectiles()
{
	for(unsigned i = 0; i < MAX_PLANT_PROJS; i++)
	{
		if(m_apPlantProj[i])
		{
			m_apPlantProj[i]->Reset();
			m_apPlantProj[i] = NULL;
			// delete m_apPlantProj[i];
		}
	}
}

void CPlant::Harvest()
{
#if defined(CONF_DEBUG)
#endif
	int CharID = IsCharacterNear();
	if(CharID != -1)
	{
		CCharacter *pChr = GameServer()->GetPlayerChar(CharID);
		pChr->m_CanHarvestPlant = true;

		if(!pChr->m_HarvestPlant)
			return;

		//pChr->GiveMoney oder GiveSeeds oder son dreck xD

		pChr->m_HarvestPlant = false;
		pChr->m_CanHarvestPlant = false;
		ResetProjectiles();
		m_GrowState = 0;
		return;
	}
}

void CPlant::Tick()
{
#if defined(CONF_DEBUG)
#endif
	if(m_GrowState != 0 && IsCharacterNear() != -1)
		Harvest();

	if(Server()->Tick() % m_GrowDelay == 0)
	{
		m_GrowDelay = rand() % 100 + 50;
		m_GrowState++;

		if(m_GrowState == 1)
		{
			m_apPlantProj[0] = new CStableProjectile(&GameServer()->m_World, WEAPON_SHOTGUN, vec2(m_Pos.x + (rand() % 4) - 8, m_Pos.y - 10));
		}
		else if(m_GrowState == 2)
		{
			m_apPlantProj[1] = new CStableProjectile(&GameServer()->m_World, WEAPON_SHOTGUN, vec2(m_Pos.x + (rand() % 4) - 8, m_Pos.y - 28));
		}
		else if(m_GrowState == 3)
		{
			m_apPlantProj[2] = new CStableProjectile(&GameServer()->m_World, WEAPON_SHOTGUN, vec2(m_Pos.x + (rand() % 4) - 8, m_Pos.y - 45));
		}
		else if(m_GrowState == 4)
		{
			m_apPlantProj[3] = new CStableProjectile(&GameServer()->m_World, WEAPON_SHOTGUN, vec2(m_Pos.x + (rand() % 4) - 8, m_Pos.y - 56));
		}
		else if(m_GrowState == 5)
		{
			m_apPlantProj[4] = new CStableProjectile(&GameServer()->m_World, WEAPON_SHOTGUN, vec2(m_Pos.x, m_Pos.y - 63));
			m_apPlantProj[5] = new CStableProjectile(&GameServer()->m_World, WEAPON_SHOTGUN, vec2(m_Pos.x - 5, m_Pos.y - 65));
			m_apPlantProj[6] = new CStableProjectile(&GameServer()->m_World, WEAPON_SHOTGUN, vec2(m_Pos.x + 8, m_Pos.y - 65));
		}
		else if(m_GrowState == 6)
		{
			m_apPlantProj[7] = new CStableProjectile(&GameServer()->m_World, WEAPON_SHOTGUN, vec2(m_Pos.x, m_Pos.y - 68));
			m_apPlantProj[8] = new CStableProjectile(&GameServer()->m_World, WEAPON_SHOTGUN, vec2(m_Pos.x - 8, m_Pos.y - 72));
			m_apPlantProj[9] = new CStableProjectile(&GameServer()->m_World, WEAPON_SHOTGUN, vec2(m_Pos.x + 10, m_Pos.y - 73));
		}
	}
}

void CPlant::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Projectile *pProj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_ID, sizeof(CNetObj_Projectile)));
	if(!pProj)
		return;

	if(!m_CalculatedVel)
		CalculateVel();

	pProj->m_X = (int)m_LastResetPos.x;
	pProj->m_Y = (int)m_LastResetPos.y;
	pProj->m_VelX = m_VelX;
	pProj->m_VelY = m_VelY;
	pProj->m_StartTick = m_LastResetTick;
	pProj->m_Type = WEAPON_SHOTGUN;
}
