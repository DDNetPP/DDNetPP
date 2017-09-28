/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "munipack.h"

////////////////////////////////////
//              (Cys)             //
// CMunipack created by Cyser!xXx //
//           30.12.2012           //
////////////////////////////////////

/* m_Mode = 0 -> spawn with Map-Entity
   m_Mode = 1 -> spawn with Hammer */

CMunipack::CMunipack(CGameWorld *pGameWorld, int Mode)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP)
{
	m_Mode = Mode;
	m_SpawnTick = -1;

	GameWorld()->InsertEntity(this);
}

void CMunipack::WeaponSwitch()
{
	m_Buffer++;

	if(m_Buffer >= 1 && m_Buffer < 50)
	{
		m_Type = POWERUP_WEAPON;
		m_Subtype = WEAPON_SHOTGUN;
	}
	if(m_Buffer == 50)
	{
		m_Type = POWERUP_WEAPON;
		m_Subtype = WEAPON_GRENADE;
	}
	if(m_Buffer == 100)
	{
		m_Type = POWERUP_WEAPON;
		m_Subtype = WEAPON_RIFLE;
	}
	if(m_Buffer == 150)
	{
		m_Type = POWERUP_WEAPON;
		m_Subtype = WEAPON_GUN;
	}
	if(m_Buffer == 200)
	{
		m_Type = POWERUP_WEAPON;
		m_Subtype = WEAPON_HAMMER;
	}
	if(m_Buffer == 250)
		m_Buffer = 1;
}

void CMunipack::FallDown()
{
	if(m_Mode == 1)
	{
		m_Vel.y += GameWorld()->m_Core.m_Tuning.m_Gravity;
		m_Vel.x*=0.97f;
		
		if(m_Vel.x < 0.01f && m_Vel.x > -0.01f)
			m_Vel.x = 0;

		if(m_Pos.x/32-20 > GameServer()->Collision()->GetWidth() || m_Pos.y/32-20 > GameServer()->Collision()->GetHeight() || m_Pos.x/32+20 < 0 || m_Pos.y/32+20 < 0 || GameServer()->Collision()->GetCollisionAt(m_Pos.x, m_Pos.y)&CCollision::COLFLAG_DEATH)
				GameWorld()->DestroyEntity(this);

		GameServer()->Collision()->MoveBox(&m_Pos,&m_Vel, vec2(28.0f, 28.0f), 0.5f);
	}
}

void CMunipack::Tick()
{
	// wait for respawn
	if(m_SpawnTick > 0)
	{
		if(Server()->Tick() > m_SpawnTick && 6 /*= config_respawn*/ > -1)
		{
			// respawn
			m_SpawnTick = -1;

			if(m_Type == POWERUP_WEAPON)
				GameServer()->CreateSound(m_Pos, SOUND_WEAPON_SPAWN);
		}
		else if(m_Mode != 1)
			return;	
	}
	
	WeaponSwitch();
	FallDown();
	
	// Check if a player intersected us
	CCharacter *pChr = GameServer()->m_World.ClosestCharacter(m_Pos, 20.0f, 0);
	if(pChr && pChr->IsAlive())
	{	
		if(m_Mode == 1) /*Waiting = self take protection for ~1 second*/
			Waiting++;	
		
		if((Waiting > 45 && m_Mode == 1) || m_Mode == 0)
		for(int i=1;i<NUM_WEAPONS-1;i++) // give all weapons to the player
		{
			if(pChr->m_aWeapons[i].m_Ammo < g_pData->m_Weapons.m_aId[i].m_Maxammo || !pChr->m_aWeapons[i].m_Got) 
			{
				pChr->m_aWeapons[i].m_Got = true;
				pChr->m_aWeapons[i].m_Ammo = min(g_pData->m_Weapons.m_aId[i].m_Maxammo, 10);
			
				if(m_Mode == 1)
				{
					GameServer()->SendChatTarget(pChr->GetPlayer()->GetCID(),"You got an ammunation-packet.");
					GameWorld()->DestroyEntity(this); /*= m_SpawnTick = 0; - destroy item*/
					Waiting = 2;
				}
				else if(m_Mode == 0)
					m_SpawnTick = Server()->Tick() + Server()->TickSpeed() * 6 /*= config_respawn*/;
				
				GameServer()->CreateSound(m_Pos, SOUND_WEAPON_SWITCH);
				pChr->SetEmote(EMOTE_HAPPY, Server()->Tick() + 1200 * Server()->TickSpeed() / 1000);
			}
		}
	}
}

void CMunipack::Snap(int SnappingClient)
{
	if(m_SpawnTick != -1 || NetworkClipped(SnappingClient))
		return;

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = m_Type;
	pP->m_Subtype = m_Subtype;
}
