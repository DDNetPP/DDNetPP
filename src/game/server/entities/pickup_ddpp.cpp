/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "pickup.h"
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>

#include <game/server/teams.h>

bool CPickup::DDPPIntersect(CCharacter *pChr, int *pRespawnTime)
{
	// player picked us up, is someone was hooking us, let them go
	switch(m_Type)
	{
	case POWERUP_HEALTH:
		if(pChr->GetPlayer()->m_IsVanillaWeapons)
		{
			if(pChr->IncreaseHealth(1))
			{
				GameServer()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH);
				*pRespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
			}
			return true;
		}
		return false;
		break;

	case POWERUP_ARMOR:
		if(pChr->GetPlayer()->m_SpookyGhostActive)
		{
			if(pChr->m_aSpookyGhostWeaponsBackupGot[2][1] || pChr->m_aSpookyGhostWeaponsBackupGot[3][1] || pChr->m_aSpookyGhostWeaponsBackupGot[4][1])
				GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR, pChr->Teams()->TeamMask(pChr->Team()));

			pChr->m_aSpookyGhostWeaponsBackup[2][1] = -1;
			pChr->m_aSpookyGhostWeaponsBackup[3][1] = -1;
			pChr->m_aSpookyGhostWeaponsBackup[4][1] = -1;

			pChr->m_aSpookyGhostWeaponsBackupGot[2][1] = false;
			pChr->m_aSpookyGhostWeaponsBackupGot[3][1] = false;
			pChr->m_aSpookyGhostWeaponsBackupGot[4][1] = false;

			pChr->GetPlayer()->m_SpawnShotgunActive = 0;
			pChr->GetPlayer()->m_SpawnGrenadeActive = 0;
			pChr->GetPlayer()->m_SpawnRifleActive = 0;
		}
		else if(pChr->GetPlayer()->m_IsVanillaWeapons)
		{
			if(pChr->IncreaseArmor(1))
			{
				GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR);
				*pRespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
			}
		}
		else
			return false;
		return true;
		break;

	case POWERUP_WEAPON:
		if(pChr->GetPlayer()->m_SpookyGhostActive)
		{
			//
		}
		else if(pChr->GetPlayer()->m_IsVanillaWeapons)
		{
			if(m_Subtype >= 0 && m_Subtype < NUM_WEAPONS && (!pChr->GetWeaponGot(m_Subtype) || (pChr->GetWeaponAmmo(m_Subtype) != -1 && !pChr->m_FreezeTime)))
			{
				if(pChr->GiveWeapon(m_Subtype, 10))
				{
					*pRespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;

					if(m_Subtype == WEAPON_GRENADE)
						GameServer()->CreateSound(m_Pos, SOUND_PICKUP_GRENADE, pChr->Teams()->TeamMask(pChr->Team()));
					else if(m_Subtype == WEAPON_SHOTGUN)
						GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, pChr->Teams()->TeamMask(pChr->Team()));
					else if(m_Subtype == WEAPON_LASER)
						GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, pChr->Teams()->TeamMask(pChr->Team()));
					else if(m_Subtype == WEAPON_GUN || m_Subtype == WEAPON_HAMMER)
						GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR, pChr->Teams()->TeamMask(pChr->Team()));

					if(pChr->GetPlayer())
						GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), m_Subtype);
				}
			}
		}
		else if(pChr->GetPlayer()->m_SpawnShotgunActive && m_Subtype == WEAPON_SHOTGUN)
		{
			pChr->GetPlayer()->m_SpawnShotgunActive = 0;
			if(pChr->GiveWeapon(m_Subtype))
			{
				GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, pChr->Teams()->TeamMask(pChr->Team()));

				if(pChr->GetPlayer())
					GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), m_Subtype);
			}
		}
		else if(pChr->GetPlayer()->m_SpawnGrenadeActive && m_Subtype == WEAPON_GRENADE)
		{
			pChr->GetPlayer()->m_SpawnGrenadeActive = 0;
			if(pChr->GiveWeapon(m_Subtype))
			{
				GameServer()->CreateSound(m_Pos, SOUND_PICKUP_GRENADE, pChr->Teams()->TeamMask(pChr->Team()));

				if(pChr->GetPlayer())
					GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), m_Subtype);
			}
		}
		else if(pChr->GetPlayer()->m_SpawnRifleActive && m_Subtype == WEAPON_LASER)
		{
			pChr->GetPlayer()->m_SpawnRifleActive = 0;
			if(pChr->GiveWeapon(m_Subtype))
			{
				GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, pChr->Teams()->TeamMask(pChr->Team()));

				if(pChr->GetPlayer())
					GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), m_Subtype);
			}
		}
		else
			return false;
		return true;
		break;

	case POWERUP_NINJA:
	{
		return pChr->GetPlayer()->m_SpookyGhostActive;
		break;
	}
	default:
		break;
	};
	return false;
}
