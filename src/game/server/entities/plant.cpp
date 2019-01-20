/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "plant.h"

#include <game/server/teams.h>

CPlant::CPlant(CGameWorld *pGameWorld)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PROJECTILE)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_ProximityRadius = PickupPhysSize;
	Reset();

	GameWorld()->InsertEntity(this);
}

void CPlant::Reset()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	if (g_pData->m_aPickups[m_Type].m_Spawndelay > 0)
		m_SpawnTick = Server()->Tick() + Server()->TickSpeed() * g_pData->m_aPickups[m_Type].m_Spawndelay;
	else
		m_SpawnTick = -1;
}

void CPlant::Tick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	Move();
	//--start uncommented for m_IsVanillaWeapons --
	// wait for respawn
	if(m_SpawnTick > 0)
	{
		if(Server()->Tick() > m_SpawnTick)
		{
			// respawn
			m_SpawnTick = -1;

			if(m_Type == POWERUP_WEAPON)
				GameServer()->CreateSound(m_Pos, SOUND_WEAPON_SPAWN);
		}
		else
			return;
	}
	//--end uncommented for m_IsVanillaWeapons --

	// Check if a player intersected us
	CCharacter *apEnts[MAX_CLIENTS];
	int Num = GameWorld()->FindEntities(m_Pos, 20.0f, (CEntity**)apEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
	for(int i = 0; i < Num; ++i) {
		CCharacter * pChr = apEnts[i];
		if(pChr && pChr->IsAlive())
		{
			if(m_Layer == LAYER_SWITCH && !GameServer()->Collision()->m_pSwitchers[m_Number].m_Status[pChr->Team()]) continue;
			bool sound = false;
			// player picked us up, is someone was hooking us, let them go
			int RespawnTime = -1; //needed for m_IsVanillaWeapons
			switch (m_Type)
			{
				case POWERUP_HEALTH:
					if (pChr->GetPlayer()->m_IsVanillaWeapons)
					{
						if (pChr->IncreaseHealth(1))
						{
							GameServer()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH);
							RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
						}
					}
					else
					{
						if (pChr->Freeze())
						{
							GameServer()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH, pChr->Teams()->TeamMask(pChr->Team()));
						}
					}
					break;

				case POWERUP_ARMOR:
					if (pChr->GetPlayer()->m_SpookyGhostActive)
					{
						if (pChr->m_aSpookyGhostWeaponsBackupGot[2][1] || pChr->m_aSpookyGhostWeaponsBackupGot[3][1] || pChr->m_aSpookyGhostWeaponsBackupGot[4][1])
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
					else if (pChr->GetPlayer()->m_IsVanillaWeapons)
					{
						if (pChr->IncreaseArmor(1))
						{
							GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR);
							RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;
						}
					}
					else
					{
						if (pChr->Team() == TEAM_SUPER) continue;
						for (int i = WEAPON_SHOTGUN; i < NUM_WEAPONS; i++)
						{
							if (pChr->GetWeaponGot(i))
							{
								if (!(pChr->m_FreezeTime && i == WEAPON_NINJA))
								{
									pChr->SetWeaponGot(i, false);
									pChr->SetWeaponAmmo(i, 0);
									sound = true;
								}
							}
						}
						pChr->GetPlayer()->m_SpawnShotgunActive = 0;
						pChr->GetPlayer()->m_SpawnGrenadeActive = 0;
						pChr->GetPlayer()->m_SpawnRifleActive = 0;
						pChr->SetNinjaActivationDir(vec2(0, 0));
						pChr->SetNinjaActivationTick(-500);
						pChr->SetNinjaCurrentMoveTime(0);
						if (sound)
						{
							pChr->SetLastWeapon(WEAPON_GUN);
							GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR, pChr->Teams()->TeamMask(pChr->Team()));
						}
						if (!pChr->m_FreezeTime && pChr->GetActiveWeapon() >= WEAPON_SHOTGUN)
							pChr->SetActiveWeapon(WEAPON_HAMMER);
					}
					break;

				case POWERUP_WEAPON:
					if (pChr->GetPlayer()->m_SpookyGhostActive)
					{
						//
					}
					else if (pChr->GetPlayer()->m_IsVanillaWeapons)
					{
						if (m_Subtype >= 0 && m_Subtype < NUM_WEAPONS && (!pChr->GetWeaponGot(m_Subtype) || (pChr->GetWeaponAmmo(m_Subtype) != -1 && !pChr->m_FreezeTime)))
						{
							if (pChr->GiveWeapon(m_Subtype, 10))
							{
								RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;

								if (m_Subtype == WEAPON_GRENADE)
									GameServer()->CreateSound(m_Pos, SOUND_PICKUP_GRENADE, pChr->Teams()->TeamMask(pChr->Team()));
								else if (m_Subtype == WEAPON_SHOTGUN)
									GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, pChr->Teams()->TeamMask(pChr->Team()));
								else if (m_Subtype == WEAPON_RIFLE)
									GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, pChr->Teams()->TeamMask(pChr->Team()));
								else if (m_Subtype == WEAPON_GUN || m_Subtype == WEAPON_HAMMER)
									GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR, pChr->Teams()->TeamMask(pChr->Team()));

								if (pChr->GetPlayer())
									GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), m_Subtype);
							}
						}
					}
					else if (pChr->GetPlayer()->m_SpawnShotgunActive && m_Subtype == WEAPON_SHOTGUN)
					{
						pChr->GetPlayer()->m_SpawnShotgunActive = 0;
						if (pChr->GiveWeapon(m_Subtype, -1))
						{
							GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, pChr->Teams()->TeamMask(pChr->Team()));
		
							if (pChr->GetPlayer())
								GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), m_Subtype);
						}
					}
					else if (pChr->GetPlayer()->m_SpawnGrenadeActive && m_Subtype == WEAPON_GRENADE)
					{
						pChr->GetPlayer()->m_SpawnGrenadeActive = 0;
						if (pChr->GiveWeapon(m_Subtype, -1))
						{
							GameServer()->CreateSound(m_Pos, SOUND_PICKUP_GRENADE, pChr->Teams()->TeamMask(pChr->Team()));

							if (pChr->GetPlayer())
								GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), m_Subtype);
						}
					}
					else if (pChr->GetPlayer()->m_SpawnRifleActive && m_Subtype == WEAPON_RIFLE)
					{
						pChr->GetPlayer()->m_SpawnRifleActive = 0;
						if (pChr->GiveWeapon(m_Subtype, -1))
						{
							GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, pChr->Teams()->TeamMask(pChr->Team()));

							if (pChr->GetPlayer())
								GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), m_Subtype);
						}
					}
					else
					{
						if (pChr->m_aDecreaseAmmo[m_Subtype])
							pChr->m_aDecreaseAmmo[m_Subtype] = false;

						if (m_Subtype >= 0 && m_Subtype < NUM_WEAPONS && (!pChr->GetWeaponGot(m_Subtype) || (pChr->GetWeaponAmmo(m_Subtype) != -1 && !pChr->m_FreezeTime)))
						{
							if (pChr->GiveWeapon(m_Subtype, -1))
							{
								//RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;

								if (m_Subtype == WEAPON_GRENADE)
									GameServer()->CreateSound(m_Pos, SOUND_PICKUP_GRENADE, pChr->Teams()->TeamMask(pChr->Team()));
								else if (m_Subtype == WEAPON_SHOTGUN)
									GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, pChr->Teams()->TeamMask(pChr->Team()));
								else if (m_Subtype == WEAPON_RIFLE)
									GameServer()->CreateSound(m_Pos, SOUND_PICKUP_SHOTGUN, pChr->Teams()->TeamMask(pChr->Team()));
								else if (m_Subtype == WEAPON_GUN || m_Subtype == WEAPON_HAMMER)
									GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR, pChr->Teams()->TeamMask(pChr->Team()));

								if (pChr->GetPlayer())
									GameServer()->SendWeaponPickup(pChr->GetPlayer()->GetCID(), m_Subtype);
							}
						}
					}
					break;

			case POWERUP_NINJA:
				{
					if (pChr->GetPlayer()->m_SpookyGhostActive)
					{
						//
					}
					else
					{
						// activate ninja on target player
						pChr->GiveNinja();
						//RespawnTime = g_pData->m_aPickups[m_Type].m_Respawntime;

						/*// loop through all players, setting their emotes
						CCharacter *pC = static_cast<CCharacter *>(GameServer()->m_World.FindFirst(CGameWorld::ENTTYPE_CHARACTER));
						for(; pC; pC = (CCharacter *)pC->TypeNext())
						{
							if (pC != pChr)
								pC->SetEmote(EMOTE_SURPRISE, Server()->Tick() + Server()->TickSpeed());
						}*/
					}
					break;
				}
				default:
					break;
			};

			//--start2 uncommented for m_IsVanillaWeapons --
			if (RespawnTime >= 0)
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "pickup player='%d:%s' item=%d/%d",
					pChr->GetPlayer()->GetCID(), Server()->ClientName(pChr->GetPlayer()->GetCID()), m_Type, m_Subtype);
				GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
				m_SpawnTick = Server()->Tick() + Server()->TickSpeed() * RespawnTime;
			}
			//--end2 uncommented for m_IsVanillaWeapons --
		}
	}
}

void CPlant::TickPaused()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	//--start3 uncommented for m_IsVanillaWeapons --
	if(m_SpawnTick != -1)
		++m_SpawnTick;
	//--end2 uncommented for m_IsVanillaWeapons --
}

void CPlant::Snap(int SnappingClient)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	//--start4 uncommented for m_IsVanillaWeapons --
	if(m_SpawnTick != -1 || NetworkClipped(SnappingClient))
		return;
	//--end4 uncommented for m_IsVanillaWeapons --

	CCharacter *Char = GameServer()->GetPlayerChar(SnappingClient);

	if(SnappingClient > -1 && (GameServer()->m_apPlayers[SnappingClient]->GetTeam() == -1
				|| GameServer()->m_apPlayers[SnappingClient]->m_Paused)
			&& GameServer()->m_apPlayers[SnappingClient]->m_SpectatorID != SPEC_FREEVIEW)
		Char = GameServer()->GetPlayerChar(GameServer()->m_apPlayers[SnappingClient]->m_SpectatorID);

	int Tick = (Server()->Tick()%Server()->TickSpeed())%11;
	if (Char && Char->IsAlive() &&
			(m_Layer == LAYER_SWITCH &&
					!GameServer()->Collision()->m_pSwitchers[m_Number].m_Status[Char->Team()])
					&& (!Tick))
		return;

	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = m_Type;
	pP->m_Subtype = m_Subtype;
}

void CPlant::Move()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Server()->Tick()%int(Server()->TickSpeed() * 0.15f) == 0)
	{
		int Flags;
		int index = GameServer()->Collision()->IsMover(m_Pos.x,m_Pos.y, &Flags);
		if (index)
		{
			m_Core=GameServer()->Collision()->CpSpeed(index, Flags);
		}
		m_Pos += m_Core;
	}
}
