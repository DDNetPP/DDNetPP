// ddnet++ generic character stuff

#include <base/ddpp_logs.h>
#include <base/math_ddpp.h>
#include <base/system.h>
#include <engine/server/server.h>
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/mapitems.h>
#include <game/mapitems_ddpp.h>
#include <game/server/ddpp/shop.h>
#include <game/server/ddpp/teleportation_request.h>
#include <game/server/entities/laser_text.h>
#include <game/server/gamecontext.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/player.h>

#include "flag.h"
#include "homing_missile.h"
#include "laser.h"
#include "plasmabullet.h"
#include "projectile.h"

#include <cinttypes>

#include "character.h"

CCharacter::~CCharacter()
{
	DestructDDPP();
}

void CCharacter::ConstructDDPP()
{
	// variable initializations constructor
	m_ci_freezetime = 0;
	m_DDPP_Finished = false;
	//if (g_Config.m_SvInstagibMode)
	//{
	//	Teams()->OnCharacterStart(m_pPlayer->GetCid());
	//}
	m_OnMoneytile = MONEYTILE_NONE;
}

void CCharacter::DestructDDPP()
{
}

void CCharacter::PreSpawnDDPP()
{
	// called before the entitiy is inserted into world
	// and before we are alive
}

void CCharacter::PostSpawnDDPP()
{
	m_freezeShotgun = false;
	m_isDmg = false;

	// disable finite cosmetics by default
	m_Rainbow = false;
	m_Bloody = false;
	m_Atom = false;
	m_Trail = false;

	m_AliveSince = time_get();
	if(g_Config.m_SvInstagibMode)
	{
		Teams()->OnCharacterStart(m_pPlayer->GetCid());
		m_LastTimeCp = -2;
	}

	m_Core.m_aWeapons[0].m_Ammo = -1; //this line is added by ChillerDragon to prevent hammer in vanilla mode to run out of ammo. Im sure this solution is a bit hacky ... to who ever who is reading this comment: feel free to fix the core of the problem.

	m_LastTaserUse = Server()->Tick();
	//zCatch ChillerDragon
	if(g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2 || m_pPlayer->m_IsInstaMode_gdm) //gdm & zCatch grenade
	{
		m_Core.m_ActiveWeapon = WEAPON_GRENADE;
	}
	else if(g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4 || m_pPlayer->m_IsInstaMode_idm) //idm & zCatch rifle
	{
		m_Core.m_ActiveWeapon = WEAPON_LASER;
	}
	else
	{
		m_Core.m_ActiveWeapon = WEAPON_GUN;
	}

	/*
	if ("ddpp gametype survival forced") //survival server (forced)
	{
		if (!m_pPlayer->m_IsSurvivaling) //don't mess things up on game start
		{
			GameServer()->SetPlayerSurvival(m_pPlayer->GetCid(), 1);
		}
	}
	*/

	if(!m_pPlayer->m_IsSurvivaling && !m_pPlayer->m_IsVanillaWeapons)
	{
		m_Core.m_aWeapons[1].m_Ammo = -1; // added by fokkonaut to have -1 (infinite) bullets of gun at spawn and not 10. after freeze you would have -1 anyways so why not when spawning
	}

	if(m_pPlayer->m_IsSurvivaling && !g_Config.m_SvSurvivalGunAmmo)
	{
		m_Core.m_aWeapons[1].m_Got = false;
		m_Core.m_ActiveWeapon = WEAPON_HAMMER;
	}

	for(auto &Minigame : GameServer()->m_vMinigames)
		Minigame->PostSpawn(this);
	for(auto &Minigame : GameServer()->m_vMinigames)
		Minigame->LoadPosition(this);

	if(GetPlayer()->m_IsSurvivaling && !GetPlayer()->m_IsSurvivalAlive)
	{
		GameServer()->LoadCosmetics(GetPlayer()->GetCid());
	}

	m_LastHitWeapon = -1;

	m_pPlayer->m_SpawnShotgunActive = 0;
	m_pPlayer->m_SpawnGrenadeActive = 0;
	m_pPlayer->m_SpawnRifleActive = 0;

	if(g_Config.m_SvAllowSpawnWeapons && !GameServer()->IsMinigame(m_pPlayer->GetCid()))
	{
		SetSpawnWeapons();
	}

	SaveRealInfos();

	UnsetSpookyGhost();

	if(m_pPlayer->m_HadFlagOnDeath)
	{
		m_pPlayer->m_ChangeTeamOnFlag = true;
		m_pPlayer->m_HadFlagOnDeath = false;
	}

	Core()->m_DDNetPP.m_RestrictionData.m_CanEnterVipPlusOnly = GetPlayer()->m_Account.m_IsSuperModerator;
}

void CCharacter::DDPPDDRacePostCoreTick()
{
	// has to be post core to make sure
	// teleporting over freeze does not freeze
	m_TeleRequest.Tick();

	if(!IsAlive())
		return;

	if(!isFreezed)
		m_FirstFreezeTick = 0;
}

void CCharacter::ClearFakeMotd()
{
	if(m_pPlayer->m_IsFakeMotd)
	{
		GameServer()->AbuseMotd(g_Config.m_SvMotd, m_pPlayer->GetCid());
		//GameServer()->SendChatTarget(m_pPlayer->GetCid(), "clear fake motd");
		m_pPlayer->m_CanClearFakeMotd = true;
		m_pPlayer->m_IsFakeMotd = false;
	}
}

void CCharacter::BulletAmounts()
{
	m_GunBullets = m_Core.m_aWeapons[1].m_Ammo;
	m_ShotgunBullets = m_Core.m_aWeapons[2].m_Ammo;
	m_GrenadeBullets = m_Core.m_aWeapons[3].m_Ammo;
	m_RifleBullets = m_Core.m_aWeapons[4].m_Ammo;
}

bool CCharacter::HasBloody()
{
	return m_Bloody || m_StrongBloody || GetPlayer()->m_InfBloody;
}

bool CCharacter::CanEnterRoom()
{
	// ROOM POINT
	bool Allowed = false;
	if(g_Config.m_SvRoomState == 0) //all
	{
		Allowed = true;
	}
	else if(g_Config.m_SvRoomState == 1) //buy
	{
		Allowed = m_pPlayer->m_BoughtRoom;
	}
	else if(g_Config.m_SvRoomState == 2) //buy invite
	{
		Allowed = m_pPlayer->m_BoughtRoom || m_HasRoomKeyBySuperModerator;
	}
	else if(g_Config.m_SvRoomState == 3) //buy admin
	{
		Allowed = m_pPlayer->m_BoughtRoom || Server()->GetAuthedState(GetPlayer()->GetCid());
	}
	else if(g_Config.m_SvRoomState == 4) //buy admin invite
	{
		Allowed = m_pPlayer->m_BoughtRoom || Server()->GetAuthedState(GetPlayer()->GetCid()) || m_HasRoomKeyBySuperModerator;
	}
	return Allowed;
}

bool CCharacter::HandleConfigTile(int Type)
{
	if(Type == CFG_TILE_OFF)
		return false;

	if(Type == CFG_TILE_FREEZE)
		Freeze();
	else if(Type == CFG_TILE_UNFREEZE)
		UnFreeze();
	else if(Type == CFG_TILE_DEEP)
		m_Core.m_DeepFrozen = true;
	else if(Type == CFG_TILE_UNDEEP)
		m_Core.m_DeepFrozen = false;
	else if(Type == CFG_TILE_DEATH)
	{
		Die(m_pPlayer->GetCid(), WEAPON_WORLD); // TODO: probably should be in places where TILE_DEATH is and not here
		return true;
	}
	else if(Type == CFG_TILE_GIVE_BLOODY)
		m_Bloody = true;
	else if(Type == CFG_TILE_GIVE_RAINBOW)
		m_Rainbow = true;
	else if(Type == CFG_TILE_GIVE_SPREADGUN)
		m_autospreadgun = true;
	return false;
}

void CCharacter::SnapCharacterDDPP()
{
	// da oben sind ja die ganzen abfragen, ob der spieler sichtbar ist, ob er richtig erstellt werden konnte,
	// ob das game nicht pausiert ist und so.
	// wenn du das jetzt oben hinschreibst dann passiert das vor den abfragen
	// kann evtl. zu einem crash oder ähnlichem führen
	if(m_WaveBloody)
	{
		if(m_WaveBloodyStrength < 1 || Server()->Tick() % m_WaveBloodyStrength == 0)
		{
			GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCid());
			if(m_WaveBloodyStrength < -5)
			{
				for(int i = 0; i < 3; i++) //strong bloody
				{
					GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCid()); //hier wird der effekt erstellt.
				}
			}
		}

		if(Server()->Tick() % 11 == 0) // wave speed
		{
			if(m_WaveBloodyGrow)
			{
				m_WaveBloodyStrength++;
			}
			else
			{
				m_WaveBloodyStrength--;
			}
		}

		if(m_WaveBloodyStrength > 12)
		{
			m_WaveBloodyGrow = false;
		}
		else if(m_WaveBloodyStrength < -10)
		{
			m_WaveBloodyGrow = true;
		}
	}

	if(m_Bloody || GameServer()->IsHooked(m_pPlayer->GetCid(), 2) || m_pPlayer->m_InfBloody) //wenn bloody aktiviert ist
	{
		if(Server()->Tick() % 3 == 0) //low bloody
		{
			GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCid()); //hier wird der effekt erstellt.
		}
	}

	if(m_StrongBloody) // wenn strong bloody aktiviert ist
	{
		for(int i = 0; i < 3; i++) //strong bloody
		{
			GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCid()); //hier wird der effekt erstellt.
		}
	}

	if(m_pPlayer->m_ninjasteam || m_ninjasteam)
	{
		for(int i = 0; i < 3; i++) //hier wird eine schleife erstellt, damit sich der effekt wiederholt
		{
			GameServer()->CreatePlayerSpawn(m_Pos); //hier wird der spawn effekt erstellt
		}
	}

	if(m_isHeal)
	{
		GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCid()); //hier wird der tee zerplatzt xD effekt erstellt.
		m_isHeal = false;
	}
}

void CCharacter::SetSpookyGhost()
{
	if(m_pPlayer->m_IsBlockTourning || (m_pPlayer->m_IsSurvivaling && !m_pPlayer->m_IsSurvivalLobby)) // no ghost in competetive minigames
		return;

	if(!m_SpookyGhostWeaponsBackupped)
	{
		for(int i = 0; i < NUM_WEAPONS; i++)
		{
			m_aSpookyGhostWeaponsBackup[i][1] = m_Core.m_aWeapons[i].m_Ammo;
			m_aSpookyGhostWeaponsBackupGot[i][1] = m_Core.m_aWeapons[i].m_Got;
			m_Core.m_aWeapons[i].m_Ammo = 0;
			m_Core.m_aWeapons[i].m_Got = false;
		}
		m_SpookyGhostWeaponsBackupped = true;
		m_Core.m_aWeapons[1].m_Got = true;
		m_Core.m_aWeapons[1].m_Ammo = -1;
	}

	str_copy(m_pPlayer->m_TeeInfos.m_aSkinName, "ghost", sizeof(m_pPlayer->m_TeeInfos.m_aSkinName));
	m_pPlayer->m_TeeInfos.m_UseCustomColor = 0;

	m_pPlayer->m_SpookyGhostActive = 1;
}

void CCharacter::UnsetSpookyGhost()
{
	if(m_SpookyGhostWeaponsBackupped)
	{
		for(int i = 0; i < NUM_WEAPONS; i++)
		{
			m_Core.m_aWeapons[i].m_Got = m_aSpookyGhostWeaponsBackupGot[i][1];
			if(m_pPlayer->m_IsVanillaWeapons || m_pPlayer->m_SpawnShotgunActive || m_pPlayer->m_SpawnGrenadeActive || m_pPlayer->m_SpawnRifleActive)
			{
				m_Core.m_aWeapons[i].m_Ammo = m_aSpookyGhostWeaponsBackup[i][1];
			}
			else
			{
				m_Core.m_aWeapons[i].m_Ammo = -1;
			}
		}
		m_SpookyGhostWeaponsBackupped = false;
	}

	str_copy(m_pPlayer->m_TeeInfos.m_aSkinName, m_pPlayer->m_RealSkinName, sizeof(m_pPlayer->m_TeeInfos.m_aSkinName));
	m_pPlayer->m_TeeInfos.m_UseCustomColor = m_pPlayer->m_RealUseCustomColor;

	m_pPlayer->m_SpookyGhostActive = 0;
}

void CCharacter::SaveRealInfos()
{
	if(!m_pPlayer->m_SpookyGhostActive)
	{
		str_copy(m_pPlayer->m_RealSkinName, m_pPlayer->m_TeeInfos.m_aSkinName, sizeof(m_pPlayer->m_RealSkinName));
		m_pPlayer->m_RealUseCustomColor = m_pPlayer->m_TeeInfos.m_UseCustomColor;
		str_copy(m_pPlayer->m_RealClan, Server()->ClientClan(m_pPlayer->GetCid()), sizeof(m_pPlayer->m_RealClan));
		str_copy(m_pPlayer->m_RealName, Server()->ClientName(m_pPlayer->GetCid()), sizeof(m_pPlayer->m_RealName));
	}
}

bool CCharacter::SetWeaponThatChrHas()
{
	if(m_Core.m_aWeapons[WEAPON_GUN].m_Got)
		SetWeapon(WEAPON_GUN);
	else if(m_Core.m_aWeapons[WEAPON_HAMMER].m_Got)
		SetWeapon(WEAPON_HAMMER);
	else if(m_Core.m_aWeapons[WEAPON_GRENADE].m_Got)
		SetWeapon(WEAPON_GRENADE);
	else if(m_Core.m_aWeapons[WEAPON_SHOTGUN].m_Got)
		SetWeapon(WEAPON_SHOTGUN);
	else if(m_Core.m_aWeapons[WEAPON_LASER].m_Got)
		SetWeapon(WEAPON_LASER);
	else
		return false;

	return true;
}

void CCharacter::DropLoot()
{
#ifdef CONF_DEBUG
	// https://github.com/DDNetPP/DDNetPP/issues/325
	// dbg_msg("drop", "DropLoot() ClientId=%d", m_pPlayer->GetCid());
#endif
	if(m_pPlayer->m_IsSurvivaling && !m_pPlayer->m_IsSurvivalLobby)
	{
		// survival weapon, health and weapon drops
		DropArmor(rand() % 6);
		DropHealth(rand() % 6);
		DropWeapon(WEAPON_GUN);
		DropWeapon(WEAPON_SHOTGUN);
		DropWeapon(WEAPON_GRENADE);
		DropWeapon(WEAPON_LASER);
	}
	else if(!GameServer()->IsMinigame(m_pPlayer->GetCid()))
	{
		int SpecialGun = 0;
		if(m_Core.m_Jetpack || m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun)
			SpecialGun = 1;
		// block drop 0-2 weapons
		DropWeapon((rand() % (NUM_WEAPONS - (3 + SpecialGun))) + (2 - SpecialGun)); // no hammer or ninja and gun only if special gun
		DropWeapon((rand() % (NUM_WEAPONS - (3 + SpecialGun))) + (2 - SpecialGun));
	}
}

void CCharacter::DropHealth(int amount)
{
	if(amount > 64)
	{
		amount = 64;
	}
	for(int i = 0; i < amount; i++)
	{
		while(GameServer()->m_vDropLimit[POWERUP_HEALTH].size() > (long unsigned int)g_Config.m_SvMaxDrops)
		{
			GameServer()->m_vDropLimit[POWERUP_HEALTH][0]->Reset();
			GameServer()->m_vDropLimit[POWERUP_HEALTH].erase(GameServer()->m_vDropLimit[POWERUP_HEALTH].begin());
		}
		CDropPickup *p = new CDropPickup(
			&GameServer()->m_World,
			POWERUP_HEALTH,
			300, // lifetime
			m_pPlayer->GetCid(),
			(rand() % 3) - 1, // direction
			(float)(amount / 5), // force
			Team());
		GameServer()->m_vDropLimit[POWERUP_HEALTH].push_back(p);
	}
}

void CCharacter::DropArmor(int amount)
{
	if(amount > 64)
	{
		amount = 64;
	}
	for(int i = 0; i < amount; i++)
	{
		while(GameServer()->m_vDropLimit[POWERUP_ARMOR].size() > (long unsigned int)g_Config.m_SvMaxDrops)
		{
			GameServer()->m_vDropLimit[POWERUP_ARMOR][0]->Reset();
			GameServer()->m_vDropLimit[POWERUP_ARMOR].erase(GameServer()->m_vDropLimit[POWERUP_ARMOR].begin());
		}
		CDropPickup *p = new CDropPickup(
			&GameServer()->m_World,
			POWERUP_ARMOR,
			300, // lifetime
			m_pPlayer->GetCid(),
			(rand() % 3) - 1, // direction
			(float)(amount / 5), // force
			Team());
		GameServer()->m_vDropLimit[POWERUP_ARMOR].push_back(p);
	}
}

void CCharacter::DropWeapon(int WeaponId)
{
	{
		char aAssert[512];
		str_format(aAssert, sizeof(aAssert), "DropWeapon(WeaponId=%d) weapon out of range 0-%d", WeaponId, NUM_WEAPONS);
		dbg_assert(WeaponId >= 0 && WeaponId < NUM_WEAPONS, aAssert);
	}
	if(!g_Config.m_SvAllowDroppingWeapons)
		return;
	if(isFreezed || m_FreezeTime)
		return;
	if(!m_Core.m_aWeapons[WeaponId].m_Got)
		return;
	if(m_pPlayer->IsInstagibMinigame())
		return;
	if(m_pPlayer->m_SpookyGhostActive && WeaponId != WEAPON_GUN)
		return;

	bool HasJetPack = m_Core.m_Jetpack;
	bool HasSpreadGun = m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun;
	bool DropAll = g_Config.m_SvAllowDroppingWeapons == 1; // all (+spawn weapons)
	bool DropNotSpawn = g_Config.m_SvAllowDroppingWeapons == 2; // normal weapons+hammer+gun
	bool DropNotHammerPistol = g_Config.m_SvAllowDroppingWeapons == 3; // normal weapons+spawn weapons
	bool DropSpawnAllowed = DropAll || DropNotHammerPistol;

	if(WeaponId == WEAPON_NINJA)
		return;
	if(WeaponId == WEAPON_HAMMER && !m_pPlayer->m_IsSurvivaling && !DropAll && !DropNotSpawn)
		return;
	if(WeaponId == WEAPON_GUN && !HasJetPack && !HasSpreadGun && !m_pPlayer->m_IsSurvivaling && !DropAll && !DropNotSpawn)
		return;
	if(WeaponId == WEAPON_LASER && (m_pPlayer->m_SpawnRifleActive || m_aDecreaseAmmo[WEAPON_LASER]) && !DropSpawnAllowed)
		return;
	if(WeaponId == WEAPON_SHOTGUN && (m_pPlayer->m_SpawnShotgunActive || m_aDecreaseAmmo[WEAPON_SHOTGUN]) && !DropSpawnAllowed)
		return;
	if(WeaponId == WEAPON_GRENADE && (m_pPlayer->m_SpawnGrenadeActive || m_aDecreaseAmmo[WEAPON_GRENADE]) && !DropSpawnAllowed)
		return;

	std::vector<CWeapon *> &DroppedWeapons = m_pPlayer->m_aWeaponLimit[WeaponId];
	if(DroppedWeapons.size() >= 5)
	{
		CWeapon *pWeapon = DroppedWeapons[0];
		if(pWeapon)
			pWeapon->Reset();
	}

	int CountWeapons = 0;
	for(auto &Weapon : m_Core.m_aWeapons)
	{
		if(Weapon.m_Got)
			CountWeapons++;
	}

	if(CountWeapons > 1)
	{
		if(HasJetPack)
		{
			m_Core.m_Jetpack = false;
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You lost your jetpack gun");
		}
		if(HasSpreadGun)
		{
			m_autospreadgun = false;
			m_pPlayer->m_InfAutoSpreadGun = false;
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You lost your spread gun");
		}

		m_Core.m_aWeapons[WeaponId].m_Got = false;
		GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCid()));

		CWeapon *Weapon = new CWeapon(&GameServer()->m_World, WeaponId, 300, m_pPlayer->GetCid(),
			GetAimDir(), Team(), m_Core.m_aWeapons[WeaponId].m_Ammo, HasJetPack, HasSpreadGun);
		DroppedWeapons.push_back(Weapon);
	}

	SetWeaponThatChrHas();
}

void CCharacter::CosmeticTick()
{
	if(m_Atom || m_pPlayer->m_InfAtom)
	{
		if(m_AtomProjs.empty())
		{
			for(int i = 0; i < NUM_ATOMS; i++)
				m_AtomProjs.push_back(new CStableProjectile(GameWorld(), i % 2 ? WEAPON_GRENADE : WEAPON_SHOTGUN));
			m_AtomPosition = 0;
		}
		if(++m_AtomPosition >= 60)
			m_AtomPosition = 0;
		vec2 AtomPos;
		AtomPos.x = m_Pos.x + 200 * cos(m_AtomPosition * M_PI * 2 / 60);
		AtomPos.y = m_Pos.y + 80 * sin(m_AtomPosition * M_PI * 2 / 60);
		for(int i = 0; i < NUM_ATOMS; i++)
			m_AtomProjs[i]->m_Pos = rotate_around_point(AtomPos, m_Pos, i * M_PI * 2 / NUM_ATOMS);
	}
	else if(!m_AtomProjs.empty())
	{
		for(auto &AtomProj : m_AtomProjs)
			AtomProj->m_MarkedForDestroy = true;
		m_AtomProjs.clear();
	}

	if(m_Trail || m_pPlayer->m_InfTrail)
	{
		if(m_TrailProjs.empty())
		{
			for(int i = 0; i < NUM_TRAILS; i++)
				m_TrailProjs.push_back(new CStableProjectile(GameWorld(), WEAPON_SHOTGUN));
			m_TrailHistory.clear();
			m_TrailHistory.emplace_front(m_Pos, 0.0f);
			m_TrailHistory.emplace_front(m_Pos, NUM_TRAILS * TRAIL_DIST);
			m_TrailHistoryLength = NUM_TRAILS * TRAIL_DIST;
		}
		vec2 FrontPos = m_TrailHistory.front().m_Pos;
		if(FrontPos != m_Pos)
		{
			float FrontLength = distance(m_Pos, FrontPos);
			m_TrailHistory.emplace_front(m_Pos, FrontLength);
			m_TrailHistoryLength += FrontLength;
		}

		while(true)
		{
			float LastDist = m_TrailHistory.back().m_Dist;
			if(m_TrailHistoryLength - LastDist >= NUM_TRAILS * TRAIL_DIST)
			{
				m_TrailHistory.pop_back();
				m_TrailHistoryLength -= LastDist;
			}
			else
			{
				break;
			}
		}

		int HistoryPos = 0;
		float HistoryPosLength = 0.0f;
		float AdditionalLength = 0.0f;
		for(int i = 0; i < NUM_TRAILS; i++)
		{
			float Length = (i + 1) * TRAIL_DIST;
			float NextDist = 0.0f;
			while(true)
			{
				// in case floating point arithmetic errors should fuck us up
				// don't crash and recalculate total history length
				if((unsigned int)HistoryPos >= m_TrailHistory.size())
				{
					m_TrailHistoryLength = 0.0f;
					for(auto &TrailHistoryEntry : m_TrailHistory)
						m_TrailHistoryLength += TrailHistoryEntry.m_Dist;
					break;
				}
				NextDist = m_TrailHistory[HistoryPos].m_Dist;

				if(Length <= HistoryPosLength + NextDist)
				{
					AdditionalLength = Length - HistoryPosLength;
					break;
				}
				else
				{
					HistoryPos += 1;
					HistoryPosLength += NextDist;
					AdditionalLength = 0;
				}
			}
			m_TrailProjs[i]->m_Pos = m_TrailHistory[HistoryPos].m_Pos;
			m_TrailProjs[i]->m_Pos += (m_TrailHistory[HistoryPos + 1].m_Pos - m_TrailProjs[i]->m_Pos) * (AdditionalLength / NextDist);
		}
	}
	else if(!m_TrailProjs.empty())
	{
		for(auto &TrailProj : m_TrailProjs)
			TrailProj->m_MarkedForDestroy = true;
		m_TrailProjs.clear();
	}
}

void CCharacter::DDPPPostCoreTick()
{
	if(!m_Alive)
		return;

	if(m_Core.m_updateFlagVel == CLIENT_ID_FLAG_RED)
	{
		GameServer()->m_pController->m_apFlags[FLAG_RED]->m_Vel = m_Core.m_UFlagVel;
	}
	else if(m_Core.m_updateFlagVel == CLIENT_ID_FLAG_BLUE)
	{
		GameServer()->m_pController->m_apFlags[FLAG_BLUE]->m_Vel = m_Core.m_UFlagVel;
	}
	if(m_Core.m_DDNetPP.m_LastHookedPlayer != m_OldLastHookedPlayer)
	{
		m_LastHitWeapon = -1;
	}
	m_OldLastHookedPlayer = m_Core.m_DDNetPP.m_LastHookedPlayer;

	GameServer()->Shop()->MotdTick(GetPlayer()->GetCid());
}

CTeleportationRequest &CCharacter::RequestTeleToTile(int Tile, int Offset)
{
	m_TeleRequest.TeleportToTile(this, Tile, Offset);
	return m_TeleRequest;
}

CTeleportationRequest &CCharacter::RequestTeleToPos(vec2 Pos)
{
	m_TeleRequest.TeleportToPos(this, Pos);
	return m_TeleRequest;
}

void CCharacter::DDPP_Tick()
{
	if(g_Config.m_SvOffDDPP)
		return;

	// has to stay on the top
	// is used to count how many players are on a moneytile
	// this tick
	m_OnMoneytile = MONEYTILE_NONE;

	char aBuf[256];
	DummyTick();
	CosmeticTick();

	m_pPlayer->m_InputTracker.OnTick(&m_Input, m_pPlayer->m_PlayerFlags);

	for(int i = 0; i < 2; i++)
	{
		CFlag *Flag = ((CGameControllerDDRace *)GameServer()->m_pController)->m_apFlags[i];
		if(!Flag)
			continue;
		int CarryId = -1;
		if(Flag->GetCarrier())
			CarryId = Flag->GetCarrier()->GetPlayer()->GetCid();
		m_Core.m_DDNetPP.SetFlagPos(i, Flag->m_Pos, Flag->m_AtStand, Flag->m_Vel, CarryId);
	}
	// do we really have to compute that every tick?
	m_Core.m_DDNetPP.m_RestrictionData.m_CanEnterRoom = CanEnterRoom();

	if(m_RandomCosmetics)
	{
		if(Server()->Tick() % 22 == 0)
		{
			int r = rand() % 10;
			if(r == 0)
			{
				m_Rainbow ^= true;
			}
			else if(r == 1)
			{
				//m_StrongBloody ^= true;
			}
			else if(r == 2)
			{
				m_Bloody ^= true;
			}
			else if(r == 3)
			{
				m_Atom ^= true;
			}
			else if(r == 4)
			{
				m_Trail ^= true;
			}
			else if(r == 5)
			{
				m_autospreadgun ^= true;
			}
			else if(r > 8)
			{
				m_ninjasteam = true;
			}

			if(Server()->Tick() % 5 == 0 && m_ninjasteam)
			{
				m_ninjasteam = false;
			}
		}
	}

	if(GameServer()->m_BlockWaveGameState)
	{
		if(m_pPlayer->m_IsBlockWaving)
		{
			if(m_FreezeTime > 0 && !m_pPlayer->m_IsBlockWaveDead)
			{
				BlockWaveFreezeTicks++; //gets set to zer0 in Unfreeze() func
				if(BlockWaveFreezeTicks > Server()->TickSpeed() * 4)
				{
					str_format(aBuf, sizeof(aBuf), "[BlockWave] '%s' died.", Server()->ClientName(m_pPlayer->GetCid()));
					GameServer()->SendChatBlockWave(aBuf);
					m_pPlayer->m_IsBlockWaveDead = true; //also gets set to zer0 on Unfreezefunc
				}
			}
		}
	}

	for(auto &Minigame : GameServer()->m_vMinigames)
		Minigame->CharacterTick(this);

	//spawnblock reducer
	if(Server()->Tick() % 1200 == 0 && m_pPlayer->m_SpawnBlocks > 0)
	{
		m_pPlayer->m_SpawnBlocks--;
	}

	//Block points (clear touchid on freeze and unfreeze again)
	//if (m_pPlayer->m_LastToucherId != -1 && isFreezed) //didn't use m_FreezeTime because we want a freeze tile here not an freezelaser or something (idk about freeze canons)
	//{
	//	m_pPlayer->m_BlockWasTouchedAndFreezed = true;
	//}
	//if (m_pPlayer->m_BlockWasTouchedAndFreezed && m_FreezeTime == 0) //player got touched and freezed and unfreezed again --> reset toucher because it isnt his kill anymore
	//{
	//	m_pPlayer->UpdateLastToucher(-1);
	//}
	//Better system: Remove LastToucherId after some unfreeze time this has less bugs and works also good in other situations like: your racing with your mate and then you rush away solo and fail and suicide (this situation wont count as kill).
	if(m_pPlayer->m_LastToucherId != -1 && m_FreezeTime == 0)
	{
		//char aBuf[64];
		//str_format(aBuf, sizeof(aBuf), "Id: %d is not -1", m_pPlayer->m_LastToucherId); //ghost debug
		//dbg_msg("block", aBuf);

		m_pPlayer->m_LastTouchTicks++;
		if(m_pPlayer->m_LastTouchTicks > Server()->TickSpeed() * 3) //3 seconds unfreeze --> wont die block death on freeze suicide
		{
			//char aBuf[64];
			//str_format(aBuf, sizeof(aBuf), "'%s' [Id: %d] touch removed", Server()->ClientName(m_pPlayer->m_LastToucherId), m_pPlayer->m_LastToucherId);
			//GameServer()->SendChatTarget(m_pPlayer->GetCid(), aBuf);
			m_pPlayer->UpdateLastToucher(-1);
		}
	}

	/*
	// wtf why did i code that? xd
	// wait until blocker disconnects to not count as blocked ?!?
	//clear last toucher on disconnect/unexistance
	if (!GameServer()->m_apPlayers[m_pPlayer->m_LastToucherId])
	{
		m_pPlayer->UpdateLastToucher(-1);
	}
	*/

	//Block points (check for last touched player)
	//pikos hook check
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		CCharacter *pChar = GameServer()->GetPlayerChar(i);

		if(!pChar || !pChar->IsAlive() || pChar == this)
			continue;
		if(pChar->Core()->HookedPlayer() == m_pPlayer->GetCid())
		{
			m_pPlayer->UpdateLastToucher(i);

			//was debugging because somekills at spawn werent recongized. But now i know that the dummys just kill to fast even before getting freeze --> not a block kill. But im ok with it spawnblock farming bots isnt nice anyways
			//dbg_msg("debug", "[%d:%s] hooked [%d:%s]", i, Server()->ClientName(i), m_pPlayer->GetCid(), Server()->ClientName(m_pPlayer->GetCid()));
		}
	}
	if(m_Core.m_HookState == HOOK_GRABBED)
	{
		//m_Dummy_nn_touched_by_humans = true;
		//GameServer()->SendChat(m_pPlayer->GetCid(), TEAM_ALL, "dont get in my hook -.-");

		//Quest 2 level 8 Block 3 tees without using hook
		if(m_pPlayer->m_QuestState == CPlayer::QUEST_BLOCK && m_pPlayer->m_QuestStateLevel == 8)
		{
			if(m_pPlayer->m_QuestProgressValue)
			{
				//GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[QUEST] don't use hook!");
				GameServer()->QuestFailed(m_pPlayer->GetCid());
			}
		}
	}

	// selfmade nobo code check if pChr is too near
	CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
	if(pChr && pChr->IsAlive() && pChr->m_Input.m_Direction) // no afk killers pls
	{
		// only count touches from unfreezed & grounded tees
		if(pChr->m_FreezeTime == 0 && pChr->IsGrounded())
		{
			if(pChr->m_Pos.x < m_Core.m_Pos.x + 45 && pChr->m_Pos.x > m_Core.m_Pos.x - 45 && pChr->m_Pos.y < m_Core.m_Pos.y + 50 && pChr->m_Pos.y > m_Core.m_Pos.y - 50)
			{
				m_pPlayer->UpdateLastToucher(pChr->GetPlayer()->GetCid());
			}
		}
	}

	//hook extras
	//if (m_pPlayer->m_IsHookRainbow)
	//{

	//}

	//dbg_msg("", "koordinaten: x=%d y=%d", (int)(m_Pos.x / 32.f), (int)(m_Pos.y / 32.f));
	//survivexp stuff
	if(m_SpawnTick)
	{
		if(Server()->Tick() >= m_SpawnTick + Server()->TickSpeed() * 6000) //100min
		{
			m_survivexpvalue = 4;
		}
		else if(Server()->Tick() >= m_SpawnTick + Server()->TickSpeed() * 3600) //60min
		{
			m_survivexpvalue = 3;
		}
		else if(Server()->Tick() >= m_SpawnTick + Server()->TickSpeed() * 1200) //20min
		{
			m_survivexpvalue = 2;
		}
		else if(Server()->Tick() >= m_SpawnTick + Server()->TickSpeed() * 300) //5min
		{
			m_survivexpvalue = 1;
		}
	}

	DDPP_FlagTick();

	if(m_pPlayer->m_Account.m_GiftDelay > 0)
	{
		m_pPlayer->m_Account.m_GiftDelay--;
		if(m_pPlayer->m_Account.m_GiftDelay == 1)
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "[GIFT] delay expired.");
		}
	}

	if(m_pPlayer->m_Account.m_JailTime > 0)
	{
		m_pPlayer->m_Account.m_EscapeTime = 0;
		m_pPlayer->m_Account.m_JailTime--;
		str_format(aBuf, sizeof(aBuf), "Your are arrested for %" PRId64 " seconds. \nType '/hide jail' to hide this info.", m_pPlayer->m_Account.m_JailTime / Server()->TickSpeed());
		if(Server()->Tick() % 40 == 0)
		{
			if(!m_pPlayer->m_hidejailmsg)
			{
				GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
			}
		}
		if(m_pPlayer->m_Account.m_JailTime == 1)
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "[JAIL] You were released from jail.");
			vec2 JailReleaseSpawn = GameServer()->Collision()->GetRandomTile(TILE_JAILRELEASE);
			//vec2 DefaultSpawn = GameServer()->Collision()->GetRandomTile(ENTITY_SPAWN);

			if(JailReleaseSpawn != vec2(-1, -1))
			{
				SetPosition(JailReleaseSpawn);
			}
			else //no jailrelease
			{
				//SetPosition(DefaultSpawn); //crashbug for mod stealer
				GameServer()->SendChatTarget(GetPlayer()->GetCid(), "[JAIL] no jail release spot found.");
			}
		}
	}

	if(m_pPlayer->m_Account.m_EscapeTime > 0)
	{
		m_pPlayer->m_Account.m_EscapeTime--;
		if(m_isDmg)
		{
			str_format(aBuf, sizeof(aBuf), "Avoid policehammers for the next %" PRId64 " seconds. \n!WARNING! DAMAGE IS ACTIVATED ON YOU!\nType '/hide jail' to hide this info.", m_pPlayer->m_Account.m_EscapeTime / Server()->TickSpeed());
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "Avoid policehammers for the next %" PRId64 " seconds. \nType '/hide jail' to hide this info.", m_pPlayer->m_Account.m_EscapeTime / Server()->TickSpeed());
		}

		if(Server()->Tick() % Server()->TickSpeed() * 60 == 0)
		{
			if(!m_pPlayer->m_hidejailmsg)
			{
				GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
			}
		}
		if(m_pPlayer->m_Account.m_EscapeTime == 1)
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "Your life as a gangster is over. You are free now.");
			GameServer()->AddEscapeReason(GetPlayer()->GetCid(), "unknown");
			m_isDmg = false;
			m_Health = 10;
		}
	}

	if(Server()->Tick() % 200 == 0) //ddpp public slow tick
	{
		if(m_pPlayer->m_ShowInstaScoreBroadcast)
			m_UpdateInstaScoreBoard = true;
	}

	if(m_UpdateInstaScoreBoard) //gets printed on update or every 200 % whatever modulo ticks
	{
		if(m_pPlayer->m_IsInstaArena_gdm)
		{
			str_format(aBuf, sizeof(aBuf), "score: %04d/%04d                                                                                                                 0", m_pPlayer->m_InstaScore, g_Config.m_SvGrenadeScorelimit);
			GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
		}
		if(m_pPlayer->m_IsInstaArena_idm)
		{
			str_format(aBuf, sizeof(aBuf), "score: %04d/%04d                                                                                                                 0", m_pPlayer->m_InstaScore, g_Config.m_SvRifleScorelimit);
			GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
		}
	}
	m_UpdateInstaScoreBoard = false;

	//General var resetter by ChillerDragon [ I M P O R T A N T] leave var resetter last --> so it wont influence ddpp tick stuff
	if(Server()->Tick() % 20 == 0)
	{
		if(m_InBank)
		{
			if(!IsOnTile(TILE_BANK))
			{
				GameServer()->SendBroadcast(" ", m_pPlayer->GetCid(), 0);
				m_InBank = false; // DDracePostCoreTick() (which handels tiles) is after DDPP_Tick() so while being in bank it will never be false because tiles are always stronger than DDPP tick        <---- this comment was made before the tile checker if clause but can be interesting for further resettings
			}
		}
		if(GameServer()->Shop()->IsInShop(GetPlayer()->GetCid()))
		{
			if(m_TileIndex != TILE_SHOP && m_TileFIndex != TILE_SHOP)
			{
				if(m_pPlayer->m_ShopBotAntiSpamTick <= Server()->Tick())
				{
					SendShopMessage("Bye! Come back if you need something.");
					m_pPlayer->m_ShopBotAntiSpamTick = Server()->Tick() + Server()->TickSpeed() * 5;
				}
				GameServer()->SendBroadcast("", m_pPlayer->GetCid(), 0);
				GameServer()->Shop()->LeaveShop(GetPlayer()->GetCid());
			}
		}
	}
}

void CCharacter::DDPP_FlagTick()
{
	if(GameServer()->m_pController->HasFlag(this) == -1)
		return;

	if(!m_pPlayer->IsLoggedIn())
		return; // GameServer()->SendBroadcast("You need an account to get xp from flags. \n Get an Account with '/register (name) (pw) (pw)'", m_pPlayer->GetCid());

	if(Server()->Tick() % 50 == 0)
	{
		if(((m_TileIndex == TILE_MONEY) || (m_TileFIndex == TILE_MONEY)))
			return;
		if(((m_TileIndex == TILE_MONEY_POLICE) || (m_TileFIndex == TILE_MONEY_POLICE)))
			return;
		if(((m_TileIndex == TILE_MONEY_DOUBLE) || (m_TileFIndex == TILE_MONEY_DOUBLE)))
			return;

		// no matter where (bank, moneytile, ...) quests are independent
		if(m_pPlayer->m_QuestState == CPlayer::QUEST_FARM)
		{
			if(m_pPlayer->m_QuestStateLevel == 9)
			{
				m_pPlayer->m_QuestProgressValue2++;
				if(m_pPlayer->m_QuestProgressValue2 > 20)
				{
					GameServer()->QuestAddProgress(m_pPlayer->GetCid(), 10);
					m_pPlayer->m_QuestProgressValue2 = 0;
				}
			}
		}

		if(m_pPlayer->IsMaxLevel())
		{
			if(m_pPlayer->m_xpmsg)
			{
				GameServer()->SendBroadcast("[FLAG] You reached the maximum level.", m_pPlayer->GetCid(), 0);
			}
			return;
		}

		int VIPBonus = 0;

		// vip+ get 2 bonus
		if(m_pPlayer->m_Account.m_IsSuperModerator)
		{
			m_pPlayer->GiveXP(2);
			m_pPlayer->MoneyTransaction(+2);

			VIPBonus = 2;
		}

		// vip get 1 bonus
		else if(m_pPlayer->m_Account.m_IsModerator)
		{
			m_pPlayer->GiveXP(1);
			m_pPlayer->MoneyTransaction(+1);

			VIPBonus = 1;
		}

		if(m_InBank && GameServer()->m_IsBankOpen)
		{
			if(VIPBonus)
			{
				if(!m_pPlayer->m_xpmsg)
				{
					GameServer()->SendBroadcast("~ B A N K ~", m_pPlayer->GetCid(), 0);
					// GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You entered the bank. You can rob the bank with '/rob_bank'");  // lol no spam old unused commands pls
				}
				else if(m_survivexpvalue == 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ B A N K ~\nXP [%" PRId64 "/%" PRId64 "] +1 flag +%d vip", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus);
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
					m_pPlayer->GiveXP(1);
				}
				else if(m_survivexpvalue > 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ B A N K ~\nXP [%" PRId64 "/%" PRId64 "] +1 flag +%d vip + %d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_survivexpvalue);
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
					m_pPlayer->GiveXP(1); //flag
					m_pPlayer->GiveXP(m_survivexpvalue); // survival
				}
			}
			else
			{
				if(!m_pPlayer->m_xpmsg)
				{
					GameServer()->SendBroadcast("~ B A N K ~", m_pPlayer->GetCid(), 0);
				}
				else if(m_survivexpvalue == 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ B A N K ~\nXP [%" PRId64 "/%" PRId64 "] +1 flag", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP());
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
					m_pPlayer->GiveXP(1);
				}
				else if(m_survivexpvalue > 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ B A N K ~\nXP [%" PRId64 "/%" PRId64 "] +1 flag +%d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_survivexpvalue);
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
					m_pPlayer->GiveXP(1); //flag
					m_pPlayer->GiveXP(m_survivexpvalue); // survival
				}
			}
		}
		else if(GameServer()->Shop()->IsInShop(GetPlayer()->GetCid()))
		{
			if(!m_pPlayer->m_xpmsg)
			{
				GameServer()->SendBroadcast("~ S H O P ~", m_pPlayer->GetCid(), 0);
			}
			else if(m_survivexpvalue == 0)
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "~ S H O P ~\nXP [%" PRId64 "/%" PRId64 "] +1 flag +%d vip", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus);
				GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
				m_pPlayer->GiveXP(1);
			}
			else if(m_survivexpvalue > 0)
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "~ S H O P ~\nXP [%" PRId64 "/%" PRId64 "] +1 flag +%d vip + %d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_survivexpvalue);
				GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
				m_pPlayer->GiveXP(1); //flag
				m_pPlayer->GiveXP(m_survivexpvalue); // survival
			}
			else
			{
				if(!m_pPlayer->m_xpmsg)
				{
					GameServer()->SendBroadcast("~ S H O P ~", m_pPlayer->GetCid(), 0);
				}
				else if(m_survivexpvalue == 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ S H O P ~\nXP [%" PRId64 "/%" PRId64 "] +1 flag", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP());
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
					m_pPlayer->GiveXP(1);
				}
				else if(m_survivexpvalue > 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ S H O P ~\nXP [%" PRId64 "/%" PRId64 "] +1 flag +%d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_survivexpvalue);
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
					m_pPlayer->GiveXP(1); //flag
					m_pPlayer->GiveXP(m_survivexpvalue); // survival
				}
			}
		}
		else //not in bank
		{
			if(VIPBonus)
			{
				if(m_pPlayer->m_xpmsg)
				{
					if(m_survivexpvalue == 0)
					{
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "XP [%" PRId64 "/%" PRId64 "] +1 flag +%d vip", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
						m_pPlayer->GiveXP(1);
					}
					else if(m_survivexpvalue > 0)
					{
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "XP [%" PRId64 "/%" PRId64 "] +1 flag +%d vip +%d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_survivexpvalue);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
						m_pPlayer->GiveXP(1); // flag
						m_pPlayer->GiveXP(m_survivexpvalue); // survival
					}
				}
			}
			else
			{
				if(m_pPlayer->m_xpmsg)
				{
					if(m_survivexpvalue == 0)
					{
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "XP [%" PRId64 "/%" PRId64 "] +1 flag", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP());
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
						m_pPlayer->GiveXP(1);
					}
					else if(m_survivexpvalue > 0)
					{
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "XP [%" PRId64 "/%" PRId64 "] +1 flag +%d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_survivexpvalue);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
						m_pPlayer->GiveXP(1); //flag
						m_pPlayer->GiveXP(m_survivexpvalue); // survival
					}
				}
			}
		}
	}
}

bool CCharacter::DDPP_Respawn()
{
	vec2 SpawnPos;

	if(!GameServer()->m_pController->CanSpawn(m_pPlayer->GetTeam(), &SpawnPos, m_pPlayer, GameServer()->GetDDRaceTeam(GetPlayer()->GetCid())))
		return false;

	if(Server()->IsRecording(m_pPlayer->GetCid()))
		Server()->StopRecord(m_pPlayer->GetCid());

	SetPosition(SpawnPos);
	return true;
}

int CCharacter::DDPP_DIE(int Killer, int Weapon, bool FngScore)
{
#if defined(CONF_DEBUG)
	dbg_msg("debug", "character die Id: %d Name: %s", m_pPlayer->GetCid(), Server()->ClientName(m_pPlayer->GetCid()));
#endif
	ClearFakeMotd();
	char aBuf[256];

	if(m_pPlayer->m_IsVanillaModeByTile) //reset vanilla mode but never go out of vanilla mode in survival
	{
		m_pPlayer->m_IsVanillaDmg = false;
		m_pPlayer->m_IsVanillaWeapons = false;
	}
	if(m_pPlayer->m_IsSurvivaling)
	{
		m_pPlayer->m_IsVanillaDmg = true;
		m_pPlayer->m_IsVanillaWeapons = true;
		m_pPlayer->m_IsVanillaCompetetive = true;
	}

	if(m_pPlayer->m_IsDummy && m_pPlayer->DummyMode() == DUMMYMODE_CHILLINTELLIGENCE) //chillintelligenz
	{
		CIRestart();
	}

	// remove atom projectiles on death
	if(!m_AtomProjs.empty())
	{
		for(auto &AtomProj : m_AtomProjs)
			AtomProj->m_MarkedForDestroy = true;
		m_AtomProjs.clear();
	}

	// remove trail projectiles on death
	if(!m_TrailProjs.empty())
	{
		for(auto &TrailProj : m_TrailProjs)
			TrailProj->m_MarkedForDestroy = true;
		m_TrailProjs.clear();
	}

	Killer = BlockPointsMain(Killer, FngScore);
	XpOnKill(Killer);
	BlockSpawnProt(Killer); //idk if this should be included in BlockPointsMain() but spawnkills no matter what kind are evil i guess but then we should rename it to SpawnKillProt() imo
	//BlockQuestSubDieFuncBlockKill(Killer); //leave this before killing sprees to also have information about killingspree values from dead tees (needed for quest2 lvl6) //included in BlockPointsMain because it handels block kills
	BlockQuestSubDieFuncDeath(Killer); //only handling quest failed (using external func because the other player is needed and its good to extract it in antoher func and because im funcy now c:) //new reason the first func is blockkill and this one is all kinds of death
	KillingSpree(Killer); // previously called BlockKillingSpree()
	DropLoot(); // has to be called before survival because it only droops loot if survival alive
	for(auto &Minigame : GameServer()->m_vMinigames)
		Minigame->OnDeath(this, Killer, Weapon);
	if(m_TeleRequest.IsActive())
		m_TeleRequest.OnDeath();
	InstagibSubDieFunc(Killer, Weapon);
	SurvivalSubDieFunc(Killer, Weapon);

	if(GetPlayer()->m_IsDummy && GetPlayer()->m_pDummyMode)
		GetPlayer()->m_pDummyMode->OnDeath();

	if(g_Config.m_SvDDPPscore == 0)
		if(GameServer()->m_apPlayers[Killer] && Killer != m_pPlayer->GetCid())
			GameServer()->m_apPlayers[Killer]->m_MinigameScore++;

	// TODO: combine with insta 1on1
	// insta kills
	if(Killer != m_pPlayer->GetCid() && GameServer()->m_apPlayers[Killer])
	{
		if(GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm || GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm)
		{
			GameServer()->DoInstaScore(3, Killer);
		}
		else if(GameServer()->IsDDPPgametype("fng"))
		{
			GameServer()->m_apPlayers[Killer]->m_MinigameScore += 3;
		}
	}

	// TODO: refactor this code and put it in own function
	// insta 1on1
	if(GameServer()->m_apPlayers[Killer])
	{
		if(GameServer()->m_apPlayers[Killer]->m_Insta1on1_id != -1 && Killer != m_pPlayer->GetCid() && (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm || GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm)) //is in 1on1
		{
			GameServer()->m_apPlayers[Killer]->m_Insta1on1_score++;
			str_format(aBuf, sizeof(aBuf), "%s:%d killed %s:%d", Server()->ClientName(Killer), GameServer()->m_apPlayers[Killer]->m_Insta1on1_score, Server()->ClientName(m_pPlayer->GetCid()), m_pPlayer->m_Insta1on1_score);
			if(!GameServer()->m_apPlayers[Killer]->m_HideInsta1on1_killmessages)
			{
				GameServer()->SendChatTarget(Killer, aBuf);
			}
			if(!m_pPlayer->m_HideInsta1on1_killmessages)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCid(), aBuf);
			}
			if(GameServer()->m_apPlayers[Killer]->m_Insta1on1_score >= 5)
			{
				GameServer()->WinInsta1on1(Killer, m_pPlayer->GetCid());
			}
		}
	}

	// TODO: refactor this code and put it in own function
	// balance battle
	if(m_pPlayer->m_IsBalanceBatteling && GameServer()->m_BalanceBattleState == 2) //ingame in a balance battle
	{
		if(GameServer()->m_BalanceId1 == m_pPlayer->GetCid())
		{
			if(GameServer()->m_apPlayers[GameServer()->m_BalanceId2])
			{
				GameServer()->SendChatTarget(GameServer()->m_BalanceId2, "[balance] you won!");
				GameServer()->SendChatTarget(GameServer()->m_BalanceId1, "[balance] you lost!");
				GameServer()->m_apPlayers[GameServer()->m_BalanceId1]->m_IsBalanceBatteling = false;
				GameServer()->m_apPlayers[GameServer()->m_BalanceId2]->m_IsBalanceBatteling = false;
				GameServer()->m_BalanceBattleState = 0;
				if(GameServer()->GetPlayerChar(GameServer()->m_BalanceId2))
				{
					GameServer()->GetPlayerChar(GameServer()->m_BalanceId2)->Die(GameServer()->m_BalanceId2, WEAPON_SELF);
				}
				//dbg_msg("balance", "%s:%d lost and %s:%d got killed too", Server()->ClientName(GameServer()->m_BalanceId1), GameServer()->m_BalanceId1, Server()->ClientName(GameServer()->m_BalanceId2), GameServer()->m_BalanceId2);
				GameServer()->StopBalanceBattle();
			}
		}
		else if(GameServer()->m_BalanceId2 == m_pPlayer->GetCid())
		{
			if(GameServer()->m_apPlayers[GameServer()->m_BalanceId1])
			{
				GameServer()->SendChatTarget(GameServer()->m_BalanceId1, "[balance] you won!");
				GameServer()->SendChatTarget(GameServer()->m_BalanceId2, "[balance] you lost!");
				GameServer()->m_apPlayers[GameServer()->m_BalanceId1]->m_IsBalanceBatteling = false;
				GameServer()->m_apPlayers[GameServer()->m_BalanceId2]->m_IsBalanceBatteling = false;
				GameServer()->m_BalanceBattleState = 0;
				if(GameServer()->GetPlayerChar(GameServer()->m_BalanceId1))
				{
					GameServer()->GetPlayerChar(GameServer()->m_BalanceId1)->Die(GameServer()->m_BalanceId1, WEAPON_SELF);
				}
				//dbg_msg("balance", "%s:%d lost and %s:%d got killed too", Server()->ClientName(GameServer()->m_BalanceId2), GameServer()->m_BalanceId2, Server()->ClientName(GameServer()->m_BalanceId1), GameServer()->m_BalanceId1);
				GameServer()->StopBalanceBattle();
			}
		}
	}

	//bomb
	if(m_IsBombing)
	{
		GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[BOMB] you lost bomb because you died.");
	}

	m_pPlayer->UpdateLastToucher(-1);
	if(GameServer()->m_pController->m_apFlags[0])
	{
		if(GameServer()->m_pController->m_apFlags[0]->GetCarrier() == this)
		{
			m_pPlayer->m_HadFlagOnDeath = true;

			if(m_Core.m_DDNetPP.m_LastHookedPlayer != -1)
			{
				GameServer()->m_pController->ChangeFlagOwner(0, Killer);
			}
		}
	}

	if(GameServer()->m_pController->m_apFlags[1])
	{
		if(GameServer()->m_pController->m_apFlags[1]->GetCarrier() == this)
		{
			m_pPlayer->m_HadFlagOnDeath = true;

			if(m_Core.m_DDNetPP.m_LastHookedPlayer != -1)
			{
				GameServer()->m_pController->ChangeFlagOwner(1, Killer);
			}
		}
	}

	// to have invis motd updates OR to have the spooky ghost skin in the kill msg
	// (because it was too fast otherwise and the normal skin would be there if its a selfkill and not a death tile kill)
	if((!m_pPlayer->m_ShowName && m_pPlayer->m_SpookyGhostActive) || m_pPlayer->m_CanClearFakeMotd)
	{
		// TODO: fix this merge with upstream removed CPlayer::m_RespawnTick
		// m_pPlayer->m_RespawnTick = Server()->Tick() + Server()->TickSpeed() / 10;

		m_pPlayer->m_CanClearFakeMotd = false;
	}

	//m_pPlayer->m_RespawnTick = Server()->Tick();

	return Killer;
}

bool CCharacter::DDPPTakeDamage(vec2 Force, int Dmg, int From, int Weapon)
{
	// m_pPlayer only inflicts half damage on self
	if(From == m_pPlayer->GetCid())
		Dmg = maximum(1, Dmg / 2);
	//Block points check for touchers (weapons)
	if(From >= 0)
	{
		if((Weapon == WEAPON_GRENADE || Weapon == WEAPON_HAMMER || Weapon == WEAPON_SHOTGUN || Weapon == WEAPON_LASER) && GameServer()->m_apPlayers[From])
		{
			if(From != m_pPlayer->GetCid())
			{
				m_pPlayer->UpdateLastToucher(From);
				m_LastHitWeapon = Weapon;
			}
		}
	}

	if(GetPlayer()->m_IsDummy && GetPlayer()->m_pDummyMode)
		GetPlayer()->m_pDummyMode->TakeDamage(Force, Dmg, From, Weapon);

	//zCatch ChillerDragon
	if(From >= 0)
	{
		if(g_Config.m_SvInstagibMode || (m_pPlayer->m_IsInstaMode_gdm && GameServer()->m_apPlayers[From]->m_IsInstaMode_gdm) || (m_pPlayer->m_IsInstaMode_idm && GameServer()->m_apPlayers[From]->m_IsInstaMode_idm)) //in (all instagib modes) or (both players in gdm/idm mode) --->  1hit
		{
			DDPP_TakeDamageInstagib(Dmg, From, Weapon);
			return true;
		}
	}
	if((m_isDmg || m_pPlayer->m_IsVanillaDmg) /*&& !m_pPlayer->m_IsSurvivalLobby*/)
	{
		//m_Core.m_Vel += Force;

		//  dragon      if(GameServer()->m_pController->IsFriendlyFire(m_pPlayer->GetCid(), From) && !g_Config.m_SvTeamdamage)
		//	dragon      return false;

		// m_pPlayer only inflicts half damage on self

		if(From == m_pPlayer->GetCid())
		{
			Dmg = maximum(1, Dmg / 2);

			if(m_pPlayer->m_IsVanillaCompetetive && Weapon == WEAPON_LASER)
			{
				// used to be "false" as in no damage but now its true as in ddnet++ damage used
				return true; //no rifle self damage in competetive vanilla games (for example survival)
			}
		}

		m_DamageTaken++;

		// create healthmod indicator
		if(Server()->Tick() < m_DamageTakenTick + 25)
		{
			// make sure that the damage indicators doesn't group together
			GameServer()->CreateDamageInd(m_Pos, m_DamageTaken * 0.25f, Dmg);
		}
		else
		{
			m_DamageTaken = 0;
			GameServer()->CreateDamageInd(m_Pos, 0, Dmg);
		}

		if(Dmg)
		{
			if(m_Armor)
			{
				if(Dmg > 1)
				{
					m_Health--;
					Dmg--;
				}

				if(Dmg > m_Armor)
				{
					Dmg -= m_Armor;
					m_Armor = 0;
				}
				else
				{
					m_Armor -= Dmg;
					Dmg = 0;
				}
			}

			m_Health -= Dmg;
		}

		m_DamageTakenTick = Server()->Tick();

		// do damage Hit sound
		if(From >= 0 && From != m_pPlayer->GetCid() && GameServer()->m_apPlayers[From])
		{
			// int64_t Mask = CmaskOne(From);
			// for(int i = 0; i < MAX_CLIENTS; i++)
			// {
			// 	if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS && GameServer()->m_apPlayers[i]->m_SpectatorId == From)
			// 		Mask |= CmaskOne(i);
			// }
			// GameServer()->CreateSound(GameServer()->m_apPlayers[From]->m_ViewPos, SOUND_HIT, Mask);
			// TODO: this was done to fix build after merge and is untested
			GameServer()->CreateSound(GameServer()->m_apPlayers[From]->m_ViewPos, SOUND_HIT, TeamMask());
		}

		// check for death
		if(m_Health <= 0)
		{
			Die(From, Weapon);

			// set attacker's face to happy (taunt!)
			if(From >= 0 && From != m_pPlayer->GetCid() && GameServer()->m_apPlayers[From])
			{
				CCharacter *pChr = GameServer()->m_apPlayers[From]->GetCharacter();
				if(pChr)
				{
					pChr->m_EmoteType = EMOTE_HAPPY;
					pChr->m_EmoteStop = Server()->Tick() + Server()->TickSpeed();
				}
			}

			// used to be false as in no damage but now its true as in ddnet++ damage used
			return true;
		}

		if(Dmg > 2)
			GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_LONG);
		else
			GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_SHORT);

		if(!m_Core.m_Jetpack)
		{
			m_EmoteType = EMOTE_PAIN;
			m_EmoteStop = Server()->Tick() + 500 * Server()->TickSpeed() / 1000;
		}
	}
	else if((From != -1) && GameServer()->m_apPlayers[From] && GameServer()->m_apPlayers[From]->m_SpookyGhostActive)
	{
		// dont do emote pain if the shooter has spooky ghost and shoot plasma projectile
	}
	else //normal ddnet code (else to IsDmg)
		return false;
	return true;
}

void CCharacter::MoveTee(int x, int y)
{
	m_Core.m_Pos.x += x;
	m_Core.m_Pos.y += y;
}

void CCharacter::ChillTelePort(float X, float Y)
{
	m_Core.m_Pos.x = X;
	m_Core.m_Pos.y = Y;
}

void CCharacter::ChillTelePortTile(int X, int Y)
{
	m_Core.m_Pos.x = X * 32;
	m_Core.m_Pos.y = Y * 32;
}

void CCharacter::FreezeAll(int Seconds)
{
	for(auto &Player : GameServer()->m_apPlayers)
		if(Player && Player->GetCharacter())
			Player->GetCharacter()->Freeze(Seconds);
}

bool CCharacter::HasWeapon(int Weapon)
{
	return m_Core.m_aWeapons[Weapon].m_Got;
}

void CCharacter::KillSpeed()
{
	m_Core.m_Vel.x = 0.0f;
	m_Core.m_Vel.y = 0.0f;
}

int CCharacter::BlockPointsMain(int Killer, bool FngScore)
{
	if(m_FreezeTime <= 0)
		return Killer;
	if(m_pPlayer->m_LastToucherId == -1)
		return Killer;
	if(m_pPlayer->m_IsInstaMode_fng && !FngScore)
		return Killer; // Killer = KilledId --> gets count as selfkill in score sys and not counted as kill (because only fng score tiles score)

	if(m_pPlayer->m_LastToucherId == m_pPlayer->GetCid())
	{
		dbg_msg("block", "WARNING '%s' [Id: %d] blocked himself", Server()->ClientName(m_pPlayer->GetCid()), m_pPlayer->GetCid());
		return Killer;
	}

	char aBuf[128];
	Killer = m_pPlayer->m_LastToucherId; // kill message

	if(g_Config.m_SvBlockBroadcast == 1) // send kill message broadcast
	{
		str_format(aBuf, sizeof(aBuf), "'%s' was blocked by '%s'", Server()->ClientName(m_pPlayer->GetCid()), Server()->ClientName(Killer));
		GameServer()->SendBroadcastAll(aBuf, 0);
	}

	BlockQuestSubDieFuncBlockKill(Killer);

	// track deaths of blocked
	if(!m_pPlayer->m_IsBlockWaving) // dont count block deaths in blockwave minigame
	{
		if(m_pPlayer->m_IsInstaArena_gdm)
		{
			//m_pPlayer->m_Account.m_GrenadeDeaths++; // probably doesn't belong into blockmain but whatever //ye rly doesnt --> moved
		}
		else if(m_pPlayer->m_IsInstaArena_idm)
		{
			//m_pPlayer->m_Account.m_RifleDeaths++; // probably doesn't belong into blockmain but whatever //ye rly doesnt --> moved
		}
		else
		{
			if(m_pPlayer->m_IsDummy)
			{
				if(g_Config.m_SvDummyBlockPoints)
				{
					m_pPlayer->m_Account.m_BlockPoints_Deaths++;
				}
			}
			else
			{
				m_pPlayer->m_Account.m_BlockPoints_Deaths++;
			}
		}
	}

	if(GameServer()->m_apPlayers[Killer])
	{
		// give kills and points to blocker
		if(!m_pPlayer->m_IsBlockWaving) // dont count block kills and points in blockwave minigame (would be too op lol)
		{
			if(m_pPlayer->m_IsDummy) // if dummy got killed make some exceptions
			{
				if(g_Config.m_SvDummyBlockPoints == 2 || (g_Config.m_SvDummyBlockPoints == 3 && GameServer()->IsPosition(Killer, 2))) //only count dummy kills if configt       cfg:3 block area or further count kills
				{
					if(Server()->Tick() >= m_SpawnTick + Server()->TickSpeed() * g_Config.m_SvPointsFarmProtection)
					{
						GameServer()->m_apPlayers[Killer]->GiveBlockPoints(1);
					}
					GameServer()->m_apPlayers[Killer]->m_Account.m_BlockPoints_Kills++;
				}
			}
			else
			{
				if(Server()->Tick() >= m_SpawnTick + Server()->TickSpeed() * g_Config.m_SvPointsFarmProtection)
				{
					GameServer()->m_apPlayers[Killer]->GiveBlockPoints(1);
				}
				GameServer()->m_apPlayers[Killer]->m_Account.m_BlockPoints_Kills++;
			}
		}
		// bounty money reward to the blocker
		if(m_pPlayer->m_BlockBounty)
		{
			str_format(aBuf, sizeof(aBuf), "[BOUNTY] +%d money for blocking '%s'", m_pPlayer->m_BlockBounty, Server()->ClientName(m_pPlayer->GetCid()));
			GameServer()->SendChatTarget(Killer, aBuf);
			str_format(aBuf, sizeof(aBuf), "bounty '%s'", Server()->ClientName(m_pPlayer->GetCid()));
			GameServer()->m_apPlayers[Killer]->MoneyTransaction(+m_pPlayer->m_BlockBounty, aBuf);
			m_pPlayer->m_BlockBounty = 0;
		}
	}
	return Killer;
}

void CCharacter::XpOnKill(int Killer)
{
	CPlayer *pKiller = GameServer()->m_apPlayers[Killer];
	if(!pKiller)
		return;
	if(!pKiller->GetCharacter())
		return;
	if(Killer == m_pPlayer->GetCid())
		return;
	if(GameServer()->IsSameIp(Killer, m_pPlayer->GetCid()))
		return;
	if(!pKiller->IsLoggedIn())
		return;
	if(pKiller->IsMaxLevel())
		return;
	if(Server()->Tick() < pKiller->GetCharacter()->m_SpawnTick + Server()->TickSpeed() * 15)
		return;
	if(Server()->Tick() < m_SpawnTick + Server()->TickSpeed() * 15)
		return;
	// no xp for players that only moved shortly after spawn or spawned afk
	if(m_pPlayer->m_LastPlaytime < m_AliveSince + time_freq() * 3)
		return;

	int TotalXP = 100;

	if(m_SpawnTick)
	{
		// victim
		if(Server()->Tick() >= m_SpawnTick + Server()->TickSpeed() * 30) // 30 secs
			TotalXP += 100;
		if(Server()->Tick() >= m_SpawnTick + Server()->TickSpeed() * 60) // 60 secs
			TotalXP += 150;
		if(Server()->Tick() >= m_SpawnTick + Server()->TickSpeed() * 600) // 10 mins
			TotalXP += 500;

		// killer
		if(Server()->Tick() >= pKiller->GetCharacter()->m_SpawnTick + Server()->TickSpeed() * 600) // 10 mins
			TotalXP += 20;
	}

	// TODO:

	// +1xp per weapon the killed has.
	// +1xp per jump the killed has.
	// +2xp if the killed has jetpack (not too op in fights and can be dropped to farm jetpack kills).
	// +50xp if the killed has endless jumps or endless hook.

	if(pKiller->m_KillStreak >= 10)
		TotalXP += 30;
	if(pKiller->m_KillStreak >= 20)
		TotalXP += 60;
	if(pKiller->m_KillStreak >= 25)
		TotalXP += 80;

	if(m_pPlayer->m_KillStreak >= 10)
		TotalXP += 1000;
	if(m_pPlayer->m_KillStreak >= 20)
		TotalXP += 2000;
	if(m_pPlayer->m_KillStreak >= 25)
		TotalXP += 4000;

	if(!pKiller->m_HideBlockXp)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "+%d xp for blocking '%s'", TotalXP, Server()->ClientName(m_pPlayer->GetCid()));
		GameServer()->SendChatTarget(Killer, aBuf);
	}
	pKiller->GiveXP(TotalXP);
}

void CCharacter::BlockSpawnProt(int Killer)
{
	char aBuf[128];
	if(GameServer()->m_apPlayers[Killer] && GameServer()->m_apPlayers[Killer]->GetCharacter() && m_pPlayer->GetCid() != Killer)
	{
		//punish spawn blockers
		if(GameServer()->IsPosition(Killer, 3)) //if killer is in spawn area
		{
			GameServer()->m_apPlayers[Killer]->m_SpawnBlocks++;
			if(g_Config.m_SvSpawnBlockProtection == 1 || g_Config.m_SvSpawnBlockProtection == 2)
			{
				GameServer()->SendChatTarget(Killer, "[WARNING] spawnblocking is illegal.");
				//str_format(aBuf, sizeof(aBuf), "[debug] spawnblocks: %d", GameServer()->m_apPlayers[Killer]->m_SpawnBlocks);
				//GameServer()->SendChatTarget(Killer, aBuf);

				if(GameServer()->m_apPlayers[Killer]->m_SpawnBlocks > 2)
				{
					str_format(aBuf, sizeof(aBuf), "'%s' is spawnblocking. catch him!", Server()->ClientName(Killer));
					GameServer()->SendAllPolice(aBuf);
					GameServer()->SendChatTarget(Killer, "Police is searching you because of spawnblocking.");
					GameServer()->m_apPlayers[Killer]->m_Account.m_EscapeTime += Server()->TickSpeed() * 120; // + 2 minutes escape time
					GameServer()->AddEscapeReason(Killer, "spawnblock");
				}
			}
		}
	}
}

void CCharacter::BlockQuestSubDieFuncBlockKill(int Killer)
{
	if(!GameServer()->m_apPlayers[Killer])
		return;

	char aBuf[128];
	//QUEST
	if(GameServer()->m_apPlayers[Killer]->m_QuestState == CPlayer::QUEST_HAMMER)
	{
		if(GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 7)
		{
			if(GameServer()->m_apPlayers[Killer]->m_QuestProgressValue < 10)
			{
				//GameServer()->SendChatTarget(Killer, "[QUEST] hammer the tee 10 times before blocking him.");
			}
			else
			{
				GameServer()->QuestAddProgress(Killer, 11);
			}
		}
	}
	else if(GameServer()->m_apPlayers[Killer]->m_QuestState == CPlayer::QUEST_BLOCK)
	{
		if(GameServer()->IsSameIp(Killer, m_pPlayer->GetCid()))
		{
			if(!m_pPlayer->m_HideQuestWarning)
			{
				GameServer()->SendChatTarget(Killer, "[QUEST] your dummy doesn't count.");
				GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[QUEST] your dummy doesn't count."); //send it both so that he recives the message. i know this can be weird on lanpartys but fuck it xd
			}
		}
		else
		{
			if(GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 0)
			{
				GameServer()->QuestCompleted(Killer);
			}
			else if(GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 1)
			{
				GameServer()->QuestAddProgress(Killer, 2);
			}
			else if(GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 2)
			{
				GameServer()->QuestAddProgress(Killer, 3);
			}
			else if(GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 3)
			{
				GameServer()->QuestAddProgress(Killer, 5);
			}
			else if(GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 4)
			{
				GameServer()->QuestAddProgress(Killer, 10);
			}
			else if(GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 5)
			{
				if(GameServer()->m_apPlayers[Killer]->m_QuestProgressValue < 5)
				{
					GameServer()->QuestAddProgress(Killer, 6, 5);
				}
				else
				{
					if(m_pPlayer->GetCid() != GameServer()->m_apPlayers[Killer]->m_QuestPlayerId)
					{
						str_format(aBuf, sizeof(aBuf), "[QUEST] You have to block '%s' to complete the quest.", Server()->ClientName(GameServer()->m_apPlayers[Killer]->m_QuestPlayerId));
						if(!m_pPlayer->m_HideQuestWarning)
						{
							GameServer()->SendChatTarget(Killer, aBuf);
						}
					}
					else
					{
						GameServer()->QuestAddProgress(Killer, 6);
					}
				}
			}
			else if(GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 6)
			{
				if(m_pPlayer->m_KillStreak < 5)
				{
					str_format(aBuf, sizeof(aBuf), "[QUEST] '%s' is only on a %d tee blockingspree", Server()->ClientName(m_pPlayer->GetCid()), m_pPlayer->m_KillStreak);
					if(!m_pPlayer->m_HideQuestWarning)
					{
						GameServer()->SendChatTarget(Killer, aBuf);
					}
				}
				else
				{
					GameServer()->QuestCompleted(Killer);
				}
			}
			else if(GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 7)
			{
				GameServer()->QuestAddProgress(Killer, 11);
			}
			else if(GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 8)
			{
				GameServer()->QuestAddProgress(Killer, 3);
			}
			else if(GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 9) //TODO: TEST THIS QUEST (should be working now)
			{
				//success (blocking player)
				if(GameServer()->m_pController->HasFlag(GameServer()->m_apPlayers[Killer]->GetCharacter()) != -1)
				{
					GameServer()->QuestAddProgress(Killer, 11);
				}
				else
				{
					if(!m_pPlayer->m_HideQuestWarning)
					{
						GameServer()->SendChatTarget(Killer, "[QUEST] You need the flag.");
					}
				}
			}
		}
	}
	else if(GameServer()->m_apPlayers[Killer]->m_QuestState == CPlayer::QUEST_RIFLE)
	{
		if(GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 7) // Rifle <specific player> and then block him [LEVEL 7]
		{
			if(GameServer()->m_apPlayers[Killer]->m_QuestPlayerId == m_pPlayer->GetCid())
			{
				if(GameServer()->m_apPlayers[Killer]->m_QuestProgressValue)
				{
					GameServer()->QuestAddProgress(Killer, 2);
				}
			}
			else
			{
				// GameServer()->SendChatTarget(Killer, "[QUEST] wrong tee");
			}
		}
		else if(GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 8) // Rifle 5 tees before blocking them [LEVEL 8]
		{
			if(GameServer()->m_apPlayers[Killer]->m_QuestProgressBool)
			{
				if(GameServer()->m_apPlayers[Killer]->m_QuestLastQuestedPlayerId == m_pPlayer->GetCid())
				{
					GameServer()->QuestAddProgress(Killer, 5);
					GameServer()->m_apPlayers[Killer]->m_QuestProgressBool = false;
					GameServer()->m_apPlayers[Killer]->m_QuestLastQuestedPlayerId = -1;
				}
				else
				{
					if(!m_pPlayer->m_HideQuestWarning)
					{
						GameServer()->SendChatTarget(Killer, "[QUEST] wrong tee");
					}
				}
			}
		}
	}
}

void CCharacter::BlockQuestSubDieFuncDeath(int Killer)
{
	if(Killer != m_pPlayer->GetCid() && m_pPlayer->m_QuestState == CPlayer::QUEST_BLOCK && m_pPlayer->m_QuestStateLevel == 7 && m_pPlayer->m_QuestProgressValue > 0)
	{
		GameServer()->QuestFailed(m_pPlayer->GetCid());
	}
	if(m_pPlayer->m_QuestStateLevel == 9 && m_pPlayer->m_QuestState == CPlayer::QUEST_HAMMER)
	{
		GameServer()->QuestFailed(m_pPlayer->GetCid());
	}
}

void CCharacter::KillingSpree(int Killer) // handles all ddnet++ gametype sprees (not other server types as fng or instagib only servers)
{
	char aBuf[128];
	// Somehow inspiration by //toast killingspree
	// system by FruchtiHD and ChillerDragon stolen from twlevel (edited by ChillerDragon)
	// stolen from DDnet++ instagib and edited again by ChillerDragon
	// rewritten by ChillerDragon cuz tw bug
	// upgraded to handle instagib again
	// rewritten by ChillerDragon in 2019 cuz old system was fucked in the head

	CPlayer *pKiller = GameServer()->m_apPlayers[Killer]; //removed pointer alien code and used the long way to have less bugsis with left players

	// dont count selfkills only count real being blocked as dead
	if(m_pPlayer->GetCid() == Killer)
	{
		//dbg_msg("SPREE", "didnt count selfkill [%d][%s]", Killer, Server()->ClientName(Killer));
		return;
	}

	char aKillerName[32];
	if(GameServer()->m_apPlayers[Killer])
		str_format(aKillerName, sizeof(aKillerName), "%s", Server()->ClientName(Killer));
	else
		str_format(aKillerName, sizeof(aKillerName), "%s", m_pPlayer->m_aLastToucherName);
	// str_copy(aKillerName, "a player who left the game", sizeof(aKillerName));

	if(m_pPlayer->m_KillStreak >= 5)
	{
		GameServer()->SendEndSpreeMessage(m_pPlayer->GetCid(), m_pPlayer->m_KillStreak, aKillerName);
		GameServer()->CreateExplosion(m_Pos, m_pPlayer->GetCid(), WEAPON_GRENADE, true, 0, m_pPlayer->GetCharacter()->Teams()->TeamMask(0));
	}

	if(pKiller)
	{
		//#################
		// KILLER (blocker)
		//#################
		if((m_pPlayer->m_IsDummy && g_Config.m_SvSpreeCountBots) || //only count bots if configurated
			(!m_pPlayer->m_IsDummy)) //count all humans in killingsprees
		{
			if(!GameServer()->GetDDRaceTeam(Killer)) // only allow increasing spree from team 0
			{
				GameServer()->m_apPlayers[Killer]->m_KillStreak++;
			}
		}
		// only count killing sprees if enough players are online and ingame (alive)
		if(GameServer()->CountIngameHumans() < g_Config.m_SvSpreePlayers)
		{
			//dbg_msg("spree", "not enough tees %d/%d spree (%d)", GameServer()->CountConnectedPlayers(), g_Config.m_SvSpreePlayers, GameServer()->m_apPlayers[Killer]->m_KillStreak);
			if(GameServer()->m_apPlayers[Killer]->m_KillStreak == 5) // TODO: what if one has 6 kills and then all players leave then he can farm dummys?
			{
				str_format(aBuf, sizeof(aBuf), GameServer()->Loc("%d players needed to start a spree.", Killer), g_Config.m_SvSpreePlayers);
				GameServer()->SendChatTarget(Killer, aBuf);
				GameServer()->m_apPlayers[Killer]->m_KillStreak = 0; // reset killstreak to avoid some1 collecting 100 kills with dummy and then if player connect he could save the spree
			}
		}
		else // enough players
		{
			if(GameServer()->m_apPlayers[Killer]->m_KillStreak % 5 == 0 && GameServer()->m_apPlayers[Killer]->m_KillStreak >= 5)
			{
				GameServer()->SendSpreeMessage(Killer, pKiller->m_KillStreak);
			}
		}
	}
	m_pPlayer->m_KillStreak = 0; //Important always clear killingspree of ded dude
}

void CCharacter::CITick()
{
	//Check for stuck --> restart
	if(isFreezed)
	{
		m_ci_freezetime++;
	}
	else
	{
		m_ci_freezetime = 0;
	}
	if(m_ci_freezetime > g_Config.m_SvCIfreezetime * Server()->TickSpeed())
	{
		Die(m_pPlayer->GetCid(), WEAPON_SELF); //call CIRestart() there
	}
}

void CCharacter::CIRestart()
{
	char aBuf[128];

	//str_format(aBuf, sizeof(aBuf), "%x", GameServer()->Score()->LoadCidata()); //linux compile error (doesnt work on win anyways)
	//if (!str_comp(aBuf, "error"))
	//{
	//	dbg_msg("CI", "error loading data...");
	//}
	//else
	//{
	//	dbg_msg("CI", "loaded DIST [%x]", GameServer()->Score()->LoadCidata());
	//}

	m_pPlayer->m_ci_latest_dest_dist = CIGetDestDist();
	str_format(aBuf, sizeof(aBuf), "Dist: %ld", m_pPlayer->m_ci_latest_dest_dist);
	dbg_msg("CI", "%s", aBuf);

	if(m_pPlayer->m_ci_latest_dest_dist < m_pPlayer->m_ci_lowest_dest_dist)
	{
		str_format(aBuf, sizeof(aBuf), "NEW [%ld] OLD [%ld]", m_pPlayer->m_ci_latest_dest_dist, m_pPlayer->m_ci_lowest_dest_dist);
		dbg_msg("CI", "%s", aBuf);
		m_pPlayer->m_ci_lowest_dest_dist = m_pPlayer->m_ci_latest_dest_dist;
	}

	str_format(aBuf, sizeof(aBuf), "%ld", m_pPlayer->m_ci_lowest_dest_dist);

	// TODO: got lost when file score was removed by upstream
	// GameServer()->Score()->SaveCidata(aBuf);
}

int CCharacter::CIGetDestDist()
{
	//pythagoras mate u rock c:
	//a²+b²=c²
	int a = m_Core.m_Pos.x - g_Config.m_SvCIdestX;
	int b = m_Core.m_Pos.y - g_Config.m_SvCIdestY;
	//|a| |b|
	a = abs(a);
	b = abs(b);

	int c = sqrt((double)(a + b));

	return c;
}

void CCharacter::SurvivalSubDieFunc(int Killer, int Weapon)
{
	bool Selfkill = Killer == m_pPlayer->GetCid();
	if(m_pPlayer->m_IsSurvivalAlive && GameServer()->m_apPlayers[Killer]->m_IsSurvivalAlive) //ignore lobby and stuff
	{
		//=== DEATHS and WINCHECK ===
		if(m_pPlayer->m_IsSurvivaling)
		{
			if(GameServer()->m_survivalgamestate > 1) //if game running
			{
				GameServer()->SetPlayerSurvival(m_pPlayer->GetCid(), CGameContext::SURVIVAL_DIE);
				GameServer()->SendChatTarget(m_pPlayer->GetCid(), "[SURVIVAL] you lost the round.");
				GameServer()->SurvivalCheckWinnerAndDeathMatch();
				GameServer()->SurvivalGetNextSpectator(m_pPlayer->GetCid(), Killer);
				GameServer()->SurvivalUpdateSpectators(m_pPlayer->GetCid(), Killer);
			}
		}

		//=== KILLS ===
		if(!Selfkill)
		{
			if(GameServer()->m_apPlayers[Killer] && GameServer()->m_apPlayers[Killer]->m_IsSurvivaling)
			{
				GameServer()->m_apPlayers[Killer]->m_Account.m_SurvivalKills++;
			}
		}
	}
}

bool CCharacter::IsHammerBlocked()
{
	//hammer delay on super jail hammer
	if(m_pPlayer->m_JailHammer > 1 && m_pPlayer->m_JailHammerDelay)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "You have to wait %" PRId64 " minutes to use your super jail hammer again.", (m_pPlayer->m_JailHammerDelay / Server()->TickSpeed()) / 60);
		GameServer()->SendChatTarget(m_pPlayer->GetCid(), aBuf);
		return true;
	}
	return false;
}

void CCharacter::DDPPHammerHit(CCharacter *pTarget)
{
	/*pTarget->TakeDamage(vec2(0.f, -1.f) + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f, g_pData->m_Weapons.m_Hammer.m_pBase->m_Damage,
    m_pPlayer->GetCid(), m_Core.m_ActiveWeapon);*/

	// shop bot
	if(pTarget->m_pPlayer->m_IsDummy)
	{
		if(pTarget->m_pPlayer->DummyMode() == DUMMYMODE_SHOPBOT)
		{
			GameServer()->Shop()->StartShop(GetPlayer()->GetCid());
		}
	}

	//Bomb (put it dat early cuz the unfreeze stuff)
	if(m_IsBombing && pTarget->m_IsBombing)
	{
		if(m_IsBomb) //if bomb hits others --> they get bomb
		{
			if(!pTarget->isFreezed && !pTarget->m_FreezeTime) //you cant bomb freezed players
			{
				m_IsBomb = false;
				pTarget->m_IsBomb = true;

				char aBuf[128];
				str_format(aBuf, sizeof(aBuf), "%s bombed %s", Server()->ClientName(m_pPlayer->GetCid()), Server()->ClientName(pTarget->GetPlayer()->GetCid()));
				GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "bomb", aBuf);
			}
		}
	}
	QuestHammerHit(pTarget);
	PoliceHammerHit(pTarget);
}

void CCharacter::PoliceHammerHit(CCharacter *pTarget)
{
	//Police catch gangstazz
	if(m_pPlayer->m_Account.m_PoliceRank && pTarget->m_FreezeTime > 1 && m_pPlayer->m_JailHammer)
	{
		char aBuf[256];

		if(!GameServer()->IsMinigame(pTarget->GetPlayer()->GetCid()))
		{
			if(pTarget->GetPlayer()->m_Account.m_EscapeTime) //always prefer normal hammer
			{
				if(pTarget->GetPlayer()->GetMoney() < 200)
				{
					str_format(aBuf, sizeof(aBuf), "You caught the gangster '%s' (5 minutes arrest).", Server()->ClientName(pTarget->GetPlayer()->GetCid()));
					GameServer()->SendChatTarget(m_pPlayer->GetCid(), aBuf);
					GameServer()->SendChatTarget(m_pPlayer->GetCid(), "+5 minutes extra arrest: He had no money to corrupt you!");

					str_format(aBuf, sizeof(aBuf), "You were arrested for 5 minutes by '%s'.", Server()->ClientName(m_pPlayer->GetCid()));
					GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCid(), aBuf);
					GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCid(), "+5 minutes extra: You couldn't corrupt the police!");
					pTarget->GetPlayer()->m_Account.m_EscapeTime = 0;
					pTarget->GetPlayer()->m_GangsterBagMoney = 0;
					pTarget->GetPlayer()->JailPlayer(600); //10 minutes jail
					pTarget->GetPlayer()->m_JailCode = rand() % 8999 + 1000;
				}
				else
				{
					str_format(aBuf, sizeof(aBuf), "You caught the gangster '%s' (5 minutes arrest).", Server()->ClientName(pTarget->GetPlayer()->GetCid()));
					GameServer()->SendChatTarget(m_pPlayer->GetCid(), aBuf);
					GameServer()->SendChatTarget(m_pPlayer->GetCid(), "+200 money (corruption)");
					str_format(aBuf, sizeof(aBuf), "caught gangster '%s'", Server()->ClientName(pTarget->GetPlayer()->GetCid()));
					m_pPlayer->MoneyTransaction(+200, aBuf);

					str_format(aBuf, sizeof(aBuf), "You were arrested 5 minutes by '%s'.", Server()->ClientName(m_pPlayer->GetCid()));
					GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCid(), aBuf);
					GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCid(), "-200 money (corruption)");
					pTarget->GetPlayer()->m_Account.m_EscapeTime = 0;
					pTarget->GetPlayer()->m_GangsterBagMoney = 0;
					pTarget->GetPlayer()->JailPlayer(300); //5 minutes jail
					pTarget->GetPlayer()->m_JailCode = rand() % 8999 + 1000;
					pTarget->GetPlayer()->MoneyTransaction(-200, "jail");
				}
			}
			else //super jail hammer
			{
				if(m_pPlayer->m_JailHammer > 1)
				{
					str_format(aBuf, sizeof(aBuf), "You jailed '%s' (%d seconds arrested).", Server()->ClientName(pTarget->GetPlayer()->GetCid()), m_pPlayer->m_JailHammer);
					GameServer()->SendChatTarget(m_pPlayer->GetCid(), aBuf);
					m_pPlayer->m_JailHammerDelay = Server()->TickSpeed() * 1200; // can only use every 20 minutes super hammer

					str_format(aBuf, sizeof(aBuf), "You were arrested by '%s' for %d seconds.", Server()->ClientName(m_pPlayer->GetCid()), m_pPlayer->m_JailHammer);
					GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCid(), aBuf);
					pTarget->GetPlayer()->m_Account.m_EscapeTime = 0;
					pTarget->GetPlayer()->m_GangsterBagMoney = 0;
					pTarget->GetPlayer()->JailPlayer(Server()->TickSpeed() * m_pPlayer->m_JailHammer);
					pTarget->GetPlayer()->m_JailCode = rand() % 8999 + 1000;
				}
			}
		}
	}
}

void CCharacter::DDPPGunFire(vec2 Direction)
{
	if(m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
	{
		if(m_pPlayer->m_QuestStateLevel == 9) //race with conditions
		{
			if(g_Config.m_SvQuestRaceCondition == 1) //no gun (also jetpack)
			{
				GameServer()->QuestFailed2(m_pPlayer->GetCid());
			}
		}
	}

	//spooky ghost
	if(m_pPlayer->m_PlayerFlags & PLAYERFLAG_SCOREBOARD && m_pPlayer->m_Account.m_SpookyGhost && m_Core.m_ActiveWeapon == WEAPON_GUN && m_CountSpookyGhostInputs)
	{
		m_TimesShot++;
		if((m_TimesShot == 2) && !m_pPlayer->m_SpookyGhostActive)
		{
			SetSpookyGhost();
			m_TimesShot = 0;
		}
		else if((m_TimesShot == 2) && m_pPlayer->m_SpookyGhostActive)
		{
			UnsetSpookyGhost();
			m_TimesShot = 0;
		}
		m_CountSpookyGhostInputs = false;
	}
}

bool CCharacter::SpecialGunProjectile(vec2 Direction, vec2 ProjStartPos, int Lifetime)
{
	if(m_pPlayer->m_SpookyGhostActive && (m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun))
	{
		float a = angle(Direction);
		a += (0.070f) * 2;

		new CPlasmaBullet(
			GameWorld(),
			m_pPlayer->GetCid(), // owner
			ProjStartPos, // pos
			Direction, // dir
			false, // freeze
			false, // explosive
			false, // unfreeze
			true, // bloody
			true, // ghost
			Team(), // responibleteam
			6, // lifetime
			1.0f, // accel
			10.0f // speed
		);

		new CPlasmaBullet(
			GameWorld(),
			m_pPlayer->GetCid(), // owner
			ProjStartPos, // pos
			vec2(cosf(a - 0.200f), sinf(a - 0.200f)), // dir
			false, // freeze
			false, // explosive
			false, // unfreeze
			true, // bloody
			true, // ghost
			Team(), // responibleteam
			6, // lifetime
			1.0f, // accel
			10.0f // speed
		);

		new CPlasmaBullet(
			GameWorld(),
			m_pPlayer->GetCid(), // owner
			ProjStartPos, // pos
			vec2(cosf(a - 0.040f), sinf(a - 0.040f)), // dir
			false, // freeze
			false, // explosive
			false, // unfreeze
			true, // bloody
			true, // ghost
			Team(), // responibleteam
			6, // lifetime
			1.0f, // accel
			10.0f // speed
		);

		GameServer()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCid()));
	}
	else if(m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun)
	{
		//idk if this is the right place to set some shooting speed but yolo
		//just copied the general code for all weapons and put it here
		if(!m_ReloadTimer)
		{
			float FireDelay;
			if(!m_TuneZone)
				GameServer()->Tuning()->Get(38 + m_Core.m_ActiveWeapon, &FireDelay);
			else
				GameServer()->TuningList()[m_TuneZone].Get(38 + m_Core.m_ActiveWeapon, &FireDelay);
			m_ReloadTimer = FireDelay * Server()->TickSpeed() / 5000;
		}

		//----- ChillerDragon tried to create 2nd projectile -----
		// Just copy and pasted the whole code again
		float a = angle(Direction);
		a += (0.070f) * 2;

		new CProjectile(
			GameWorld(),
			WEAPON_GUN, // Type
			m_pPlayer->GetCid(), // Owner
			ProjStartPos, // Pos
			vec2(cosf(a), sinf(a)), // Dir
			Lifetime, // Span
			false, // Freeze
			false, // Explosive
			-1, // SoundImpact
			Direction // InitDir
		);

		new CProjectile(
			GameWorld(),
			WEAPON_GUN, // Type
			m_pPlayer->GetCid(), // Owner
			ProjStartPos, // Pos
			vec2(cosf(a - 0.070f), sinf(a - 0.070f)), // Dir
			Lifetime, // Span
			false, // Freeze
			false, // Explosive
			-1, // SoundImpact
			Direction // InitDir
		);

		new CProjectile(
			GameWorld(),
			WEAPON_GUN, // Type
			m_pPlayer->GetCid(), // Owner
			ProjStartPos, // Pos
			vec2(cosf(a - 0.170f), sinf(a - 0.170f)), // Dir
			Lifetime, // Span
			false, // Freeze
			false, // Explosive
			-1, // SoundImpact
			Direction // InitDir
		);

		CProjectile *pProj = new CProjectile(
			GameWorld(),
			WEAPON_GUN, // Type
			m_pPlayer->GetCid(), // Owner
			ProjStartPos, // Pos
			Direction, // Dir
			Lifetime, // Span
			false, // Freeze
			false, // Explosive
			-1, // SoundImpact
			Direction // InitDir
		);

		// pack the Projectile and send it to the client Directly
		CNetObj_Projectile p;
		pProj->FillInfo(&p);

		GameServer()->CreateSound(m_Pos, SOUND_GUN_FIRE, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCid()));
	}
	else if(m_pPlayer->m_SpookyGhostActive)
	{
		new CPlasmaBullet(
			GameWorld(),
			m_pPlayer->GetCid(), // owner
			ProjStartPos, // pos
			Direction, // dir
			false, // freeze
			false, // explosive
			false, // unfreeze
			true, // bloody
			true, // ghost
			Team(), // responibleteam
			6, // lifetime
			1.0f, // accel
			10.0f // speed
		);
		GameServer()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCid()));
	}
	else if(m_pPlayer->m_lasergun)
	{
		int RifleSpread = 1;
		float Spreading[] = {-0.070f, 0, 0.070f};
		for(int i = -RifleSpread; i <= RifleSpread; ++i)
		{
			float a = angle(Direction);
			a += Spreading[i + 1];
			new CLaser(GameWorld(), m_Pos, vec2(cosf(a), sinf(a)), GameServer()->Tuning()->m_LaserReach, m_pPlayer->GetCid(), 0);
		}

		// summon meteor
		//CMeteor *pMeteor = new CMeteor(GameWorld(), ProjStartPos);
	}
	else
		return false;
	return true;
}

bool CCharacter::FreezeShotgun(vec2 Direction, vec2 ProjStartPos)
{
	if(m_freezeShotgun || m_pPlayer->m_IsVanillaWeapons) //freezeshotgun
	{
		int ShotSpread = 2;

		for(int i = -ShotSpread; i <= ShotSpread; ++i)
		{
			float Spreading[] = {-0.185f, -0.070f, 0, 0.070f, 0.185f};
			float a = angle(Direction);
			a += Spreading[i + 2];
			float v = 1 - (absolute(i) / (float)ShotSpread);
			float Speed = mix((float)GameServer()->Tuning()->m_ShotgunSpeeddiff, 1.0f, v);
			CProjectile *pProj = new CProjectile(
				GameWorld(),
				WEAPON_SHOTGUN, // Type
				m_pPlayer->GetCid(), // Owner
				ProjStartPos, // Pos
				vec2(cosf(a), sinf(a)) * Speed, // Dir
				(int)(Server()->TickSpeed() * GameServer()->Tuning()->m_ShotgunLifetime), // Span
				true, // Freeze
				false, //Explosive
				-1, // SoundImpact,
				ProjStartPos, // InitDir
				WEAPON_SHOTGUN);

			// pack the Projectile and send it to the client Directly
			CNetObj_Projectile p;
			pProj->FillInfo(&p);
		}

		GameServer()->CreateSound(m_Pos, SOUND_SHOTGUN_FIRE);
		return true;
	}
	return false;
}

void CCharacter::MineTeeBreakBlock()
{
	if(!g_Config.m_SvMineTeeHammer)
		return;

	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));
	int x = round_to_int(m_Core.m_Pos.x + (Direction.x * 32)) / 32;
	int y = round_to_int(m_Core.m_Pos.y + (Direction.y * 32)) / 32;
	// dbg_msg("minetee", "x=%d y=%d pos(%.2f/%.2f) dir(%.2f/%.2f", x, y, m_Core.m_Pos.x, m_Core.m_Pos.y, Direction.x, Direction.y);

	int Group, Layer;
	while(Collision()->FirstNonEmpty(x, y, &Group, &Layer))
	{
		CNetMsg_Sv_ModifyTile Msg;
		Msg.m_X = x;
		Msg.m_Y = y;
		Msg.m_Group = Group;
		Msg.m_Layer = Layer;
		Msg.m_Index = 0;
		Msg.m_Flags = 0;
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);

		// update collsion server side
		Collision()->ModifyTile(Msg.m_X, Msg.m_Y, Msg.m_Group, Msg.m_Layer, Msg.m_Index, Msg.m_Flags);
	}
}

bool CCharacter::FireWeaponDDPP(bool &FullAuto)
{
	if(m_ReloadTimer != 0)
		return false;

	DoWeaponSwitch();
	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));

	if(m_autospreadgun && m_Core.m_ActiveWeapon == WEAPON_GUN)
		FullAuto = true;
	if(m_pPlayer->m_InfAutoSpreadGun && m_Core.m_ActiveWeapon == WEAPON_GUN)
		FullAuto = true;

	// don't fire hammer when player is deep and sv_deepfly is disabled
	if(!g_Config.m_SvDeepfly && m_Core.m_ActiveWeapon == WEAPON_HAMMER && m_Core.m_DeepFrozen)
		return false;

	// check if we gonna fire
	bool WillFire = false;
	if(CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses)
	{
		WillFire = true;
		if(m_pPlayer->m_PlayerFlags & PLAYERFLAG_SCOREBOARD && m_pPlayer->m_Account.m_SpookyGhost && m_Core.m_ActiveWeapon == WEAPON_GUN)
		{
			m_CountSpookyGhostInputs = true;
		}

		GameServer()->Shop()->WillFireWeapon(GetPlayer()->GetCid());
	}

	if(FullAuto && (m_LatestInput.m_Fire & 1) && m_Core.m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo)
		WillFire = true;

	if(!WillFire && !m_Fire)
		return false;

	if(m_FreezeTime)
		return false;

	// check for ammo
	if(!m_Core.m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo || m_FreezeTime)
	{
		if(m_pPlayer->m_IsVanillaWeapons)
		{
			// 125ms is a magical limit of how fast a human can click
			m_ReloadTimer = 125 * Server()->TickSpeed() / 1000;
			GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO);
		}
		return true;
	}

	bool IsDDNetPPHit = false;
	vec2 ProjStartPos = m_Pos + Direction * GetProximityRadius() * 0.75f;

	switch(m_Core.m_ActiveWeapon)
	{
	case WEAPON_HAMMER:
	{
		if(IsHammerBlocked())
			return true;

		// reset objects Hit
		m_NumObjectsHit = 0;
		GameServer()->CreateSound(m_Pos, SOUND_HAMMER_FIRE, TeamMask());

		Antibot()->OnHammerFire(m_pPlayer->GetCid());

		if(m_Core.m_HammerHitDisabled)
			break;

		MineTeeBreakBlock();

		CCharacter *apEnts[MAX_CLIENTS];
		int Hits = 0;
		int Num = GameServer()->m_World.FindEntities(ProjStartPos, GetProximityRadius() * 0.5f, (CEntity **)apEnts,
			MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

		for(int i = 0; i < Num; ++i)
		{
			CCharacter *pTarget = apEnts[i];

			//if ((pTarget == this) || GameServer()->Collision()->IntersectLine(ProjStartPos, pTarget->m_Pos, NULL, NULL))
			if((pTarget == this || (pTarget->IsAlive() && !CanCollide(pTarget->GetPlayer()->GetCid()))))
				continue;

			if(m_pPlayer->m_IsInstaMode_fng)
			{
				// set his velocity to fast upward (for now)
				if(length(pTarget->m_Pos - ProjStartPos) > 0.0f)
					GameServer()->CreateHammerHit(pTarget->m_Pos - normalize(pTarget->m_Pos - ProjStartPos) * GetProximityRadius() * 0.5f, TeamMask());
				else
					GameServer()->CreateHammerHit(ProjStartPos, TeamMask());
			}

			vec2 Dir = vec2(0.f, 0.f);
			if(m_pPlayer->m_IsInstaMode_fng && m_pPlayer->m_Account.m_aFngConfig[1] == '1')
			{
				pTarget->TakeHammerHit(this);
				IsDDNetPPHit = true;
			}
			else
			{
				if(length(pTarget->m_Pos - m_Pos) > 0.0f)
					Dir = normalize(pTarget->m_Pos - m_Pos);
				else
					Dir = vec2(0.f, -1.f);
			}

			DDPPHammerHit(pTarget);

			float Strength;
			if(!m_TuneZone)
				Strength = GameServer()->Tuning()->m_HammerStrength;
			else
				Strength = GameServer()->TuningList()[m_TuneZone].m_HammerStrength;

			if(m_pPlayer->m_IsInstaMode_fng) // don't damage with hammer in fng
			{
				vec2 Temp = pTarget->m_Core.m_Vel + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f;
				Temp = ClampVel(pTarget->m_MoveRestrictions, Temp);
				Temp -= pTarget->m_Core.m_Vel;
				pTarget->TakeDamage((vec2(0.f, -1.0f) + Temp) * Strength, 0,
					m_pPlayer->GetCid(), m_Core.m_ActiveWeapon);
				IsDDNetPPHit = true;
			}

			// ddnet removed freeze hammer
			// if(m_FreezeHammer)
			// 	pTarget->Freeze();

			Hits++;
		}

		if(Hits > 1)
		{
			if(m_pPlayer->m_QuestState == CPlayer::QUEST_HAMMER)
			{
				if(m_pPlayer->m_QuestStateLevel == 8) // Hammer 2+ tees in one hit
				{
					GameServer()->QuestCompleted(m_pPlayer->GetCid());
				}
			}
		}

		if(m_CanHarvestPlant)
		{
			m_HarvestPlant = true;
		}

		// if we Hit anything, we have to wait for the reload
		if(Hits)
		{
			float FireDelay;
			if(!m_TuneZone)
				FireDelay = GameServer()->Tuning()->m_HammerHitFireDelay;
			else
				FireDelay = GameServer()->TuningList()[m_TuneZone].m_HammerHitFireDelay;
			m_ReloadTimer = FireDelay * Server()->TickSpeed() / 1000;
		}
	}
	break;

	case WEAPON_GUN:
	{
		if(!m_Core.m_Jetpack || !m_pPlayer->m_NinjaJetpack || m_Core.m_HasTelegunGun)
		{
			int Lifetime;
			if(!m_TuneZone)
				Lifetime = (int)(Server()->TickSpeed() * GameServer()->Tuning()->m_GunLifetime);
			else
				Lifetime = (int)(Server()->TickSpeed() * GameServer()->TuningList()[m_TuneZone].m_GunLifetime);

			if(SpecialGunProjectile(Direction, ProjStartPos, Lifetime))
				IsDDNetPPHit = true;
		}

		DDPPGunFire(Direction);
	}
	break;

	case WEAPON_SHOTGUN:
	{
		if(FreezeShotgun(Direction, ProjStartPos))
			IsDDNetPPHit = true;

		QuestShotgun();
	}
	break;

	case WEAPON_GRENADE:
	{
		if(g_Config.m_SvInstagibMode || m_pPlayer->m_IsInstaMode_gdm)
		{
			m_pPlayer->m_Account.m_GrenadeShots++;
			m_pPlayer->m_Account.m_GrenadeShotsNoRJ++;
		}

		if(m_HomingMissile)
		{
			/* CHomingMissile *pMissile = */ new CHomingMissile(GameWorld(), 100, m_pPlayer->GetCid(), 0, Direction);
			IsDDNetPPHit = true;
		}
		QuestGrenade();
	}
	break;
	case WEAPON_LASER:
	{
		if(g_Config.m_SvInstagibMode)
			m_pPlayer->m_Account.m_RifleShots++;
		QuestRifle();
	}
	break;

	case WEAPON_NINJA:
	{
		QuestNinja();
	}
	break;
	}

	m_AttackTick = Server()->Tick();

	/*if(m_Core.m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo > 0) // -1 == unlimited
		m_Core.m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;*/

	if(!m_ReloadTimer && !IsDDNetPPHit)
	{
		float FireDelay;
		if(!m_TuneZone)
			GameServer()->Tuning()->Get(38 + m_Core.m_ActiveWeapon, &FireDelay);
		else
			GameServer()->TuningList()[m_TuneZone].Get(38 + m_Core.m_ActiveWeapon, &FireDelay);
		m_ReloadTimer = FireDelay * Server()->TickSpeed() / 1000;
		if(m_OnFire)
		{
			m_OnFire = false;
			m_ReloadTimer = 200 * Server()->TickSpeed() / 1000;
		}
	}

	return IsDDNetPPHit;
}

void CCharacter::PostFireWeapon()
{
	QuestFireWeapon();
	m_AttackTick = Server()->Tick();

	if(m_pPlayer->m_IsVanillaWeapons)
	{
		if(m_Core.m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo > 0) // -1 == unlimited
			m_Core.m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;
	}

	if(m_aDecreaseAmmo[m_Core.m_ActiveWeapon]) // picked up a dropped weapon without infinite bullets (-1)
	{
		m_Core.m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;

		if(m_Core.m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo == 0)
		{
			m_aDecreaseAmmo[m_Core.m_ActiveWeapon] = false;
			m_Core.m_aWeapons[m_Core.m_ActiveWeapon].m_Got = false;
			SetWeaponThatChrHas();
		}
	}

	GameServer()->Shop()->FireWeapon(GetAimDir(), GetPlayer()->GetCid());

	//spawn weapons

	if(m_pPlayer->m_SpawnShotgunActive && m_Core.m_ActiveWeapon == WEAPON_SHOTGUN)
	{
		m_Core.m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;
		if(m_Core.m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo == 0)
		{
			m_pPlayer->m_SpawnShotgunActive = 0;
			SetWeaponGot(WEAPON_SHOTGUN, false);
			SetWeaponThatChrHas();
		}
	}

	if(m_pPlayer->m_SpawnGrenadeActive && m_Core.m_ActiveWeapon == WEAPON_GRENADE)
	{
		m_Core.m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;
		if(m_Core.m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo == 0)
		{
			m_pPlayer->m_SpawnGrenadeActive = 0;
			SetWeaponGot(WEAPON_GRENADE, false);
			SetWeaponThatChrHas();
		}
	}

	if(m_Core.m_ActiveWeapon == WEAPON_LASER)
	{
		if(m_pPlayer->m_TaserOn)
			m_LastTaserUse = Server()->Tick();
		if(m_pPlayer->m_SpawnRifleActive)
		{
			m_Core.m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;
			if(m_Core.m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo == 0)
			{
				m_pPlayer->m_SpawnRifleActive = 0;
				SetWeaponGot(WEAPON_LASER, false);
				SetWeaponThatChrHas();
			}
		}
	}
}

void CCharacter::SendShopMessage(const char *pMsg)
{
	int Recv = m_pPlayer->m_ShopBotMesssagesRecieved / 2; // 2 messages = enter + leave
	if(g_Config.m_SvMaxShopMessages != -1 && g_Config.m_SvMaxShopMessages <= Recv)
		return;

	GameServer()->SendChat(GameServer()->GetShopBot(), TEAM_ALL, pMsg, -1, CGameContext::FLAG_SIX | CGameContext::FLAG_SIXUP, m_pPlayer->GetCid());
	m_pPlayer->m_ShopBotMesssagesRecieved++;
}

bool CCharacter::IsInDDraceTeam()
{
	int DDraceTeam = GameServer()->GetDDRaceTeam(GetPlayer()->GetCid());
	return DDraceTeam != 0;
}

int CCharacter::GetAimDir() const
{
	if(m_Input.m_TargetX < 0)
		return -1;
	else
		return 1;
	return 0;
}

void CCharacter::TakeHammerHit(CCharacter *pFrom)
{
	vec2 Dir;
	if(length(m_Pos - pFrom->m_Pos) > 0.0f)
		Dir = normalize(m_Pos - pFrom->m_Pos);
	else
		Dir = vec2(0.f, -1.f);

	vec2 Push = vec2(0.f, -1.f) + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f;
	//if (GameServer()->m_pController->IsTeamplay() && pFrom->GetPlayer() && m_pPlayer->GetTeam() == pFrom->GetPlayer()->GetTeam() && IsFreezed())
	// {
	// 	Push.x *= g_Config.m_SvMeltHammerScaleX * 0.01f;
	// 	Push.y *= g_Config.m_SvMeltHammerScaleY * 0.01f;
	// }
	// else
	{
		Push.x *= g_Config.m_SvHammerScaleX * 0.01f;
		Push.y *= g_Config.m_SvHammerScaleY * 0.01f;
	}

	m_Core.m_Vel += Push;
}

void CCharacter::KillFreeze(bool Unfreeze)
{
	if(!g_Config.m_SvFreezeKillDelay)
		return;
	if(Unfreeze) // stop counting
	{
		m_FirstFreezeTick = 0;
		return;
	}
	if(!m_FirstFreezeTick) // start counting
	{
		m_FirstFreezeTick = Server()->Tick();
		return;
	}
	if(Server()->Tick() - m_FirstFreezeTick > (Server()->TickSpeed() / 10) * g_Config.m_SvFreezeKillDelay)
	{
		Die(m_pPlayer->GetCid(), WEAPON_SELF);
		m_FirstFreezeTick = 0;
	}
}

bool CCharacter::ForceFreeze(int Seconds)
{
	isFreezed = true;
	if(Seconds <= 0 || m_FreezeTime == -1)
		return false;
	if(m_Core.m_FreezeStart < Server()->Tick() - Server()->TickSpeed() || Seconds == -1)
	{
		if(!m_WeaponsBackupped) //only save once
		{
			for(int i = 0; i < NUM_WEAPONS; i++)
			{
				if(m_Core.m_aWeapons[i].m_Got)
				{
					m_aWeaponsBackup[i][1] = m_Core.m_aWeapons[i].m_Ammo; //save all ammo sats for m_IsVanillaWeapons to load em on unfreeze
						//dbg_msg("vanilla", "'%s' saved weapon[%d] ammo[%d]", Server()->ClientName(m_pPlayer->GetCid()),i, m_aWeaponsBackup[i][1]);
					m_Core.m_aWeapons[i].m_Ammo = 0; //dont set this to 0 in freeze to allow shoting in freeze (can be used for events)
				}
			}
			m_WeaponsBackupped = true;
		}

		if(!m_pPlayer->m_IsVanillaWeapons)
		{
			m_Armor = 0;
		}

		if(m_Core.m_FreezeStart == 0 || m_FirstFreezeTick == 0)
		{
			m_FirstFreezeTick = Server()->Tick();
		}

		m_FreezeTime = Seconds == -1 ? Seconds : Seconds * Server()->TickSpeed();
		m_Core.m_FreezeStart = Server()->Tick();
		return true;
	}
	return false;
}

bool CCharacter::FreezeFloat(float Seconds)
{
	KillFreeze(false);
	isFreezed = true;
	if((Seconds <= 0 || m_Core.m_Super || m_FreezeTime == -1 || m_FreezeTime > Seconds * Server()->TickSpeed()) && Seconds != -1)
		return false;
	if(m_Core.m_FreezeStart < Server()->Tick() - Server()->TickSpeed() || Seconds == -1)
	{
		if(!m_WeaponsBackupped) //only save once
		{
			for(int i = 0; i < NUM_WEAPONS; i++)
			{
				if(m_Core.m_aWeapons[i].m_Got)
				{
					m_aWeaponsBackup[i][1] = m_Core.m_aWeapons[i].m_Ammo; //save all ammo sats for m_IsVanillaWeapons to load em on unfreeze
					//dbg_msg("vanilla", "'%s' saved weapon[%d] ammo[%d]", Server()->ClientName(m_pPlayer->GetCid()),i, m_aWeaponsBackup[i][1]);
					m_Core.m_aWeapons[i].m_Ammo = 0; //dont set this to 0 in freeze to allow shoting in freeze (can be used for events)
				}
			}
			m_WeaponsBackupped = true;
		}

		if(!m_pPlayer->m_IsVanillaWeapons)
		{
			m_Armor = 0;
		}

		if(m_Core.m_FreezeStart == 0 || m_FirstFreezeTick == 0)
		{
			m_FirstFreezeTick = Server()->Tick();
		}

		m_FreezeTime = Seconds == -1 ? Seconds : Seconds * Server()->TickSpeed();
		m_Core.m_FreezeStart = Server()->Tick();
		return true;
	}
	return false;
}

void CCharacter::SetSpawnWeapons()
{
	if(m_pPlayer->m_Account.m_UseSpawnWeapons && !m_pPlayer->IsInstagibMinigame() && !m_pPlayer->m_IsSurvivaling)
	{
		if(m_pPlayer->m_Account.m_SpawnWeaponShotgun)
		{
			m_Core.m_aWeapons[2].m_Got = true;
			m_Core.m_aWeapons[2].m_Ammo = m_pPlayer->m_Account.m_SpawnWeaponShotgun;
			m_pPlayer->m_SpawnShotgunActive = 1;
		}

		if(m_pPlayer->m_Account.m_SpawnWeaponGrenade)
		{
			m_Core.m_aWeapons[3].m_Got = true;
			m_Core.m_aWeapons[3].m_Ammo = m_pPlayer->m_Account.m_SpawnWeaponGrenade;
			m_pPlayer->m_SpawnGrenadeActive = 1;
		}

		if(m_pPlayer->m_Account.m_SpawnWeaponRifle)
		{
			m_Core.m_aWeapons[4].m_Got = true;
			m_Core.m_aWeapons[4].m_Ammo = m_pPlayer->m_Account.m_SpawnWeaponRifle;
			m_pPlayer->m_SpawnRifleActive = 1;
		}
	}
}
