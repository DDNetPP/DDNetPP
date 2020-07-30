/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <new>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <game/mapitems.h>

#include "character.h"
#include "laser.h"
#include "plasmabullet.h"
#include "projectile.h"
#include "meteor.h"
#include "homing_missile.h"

#include <stdio.h>
#include <string.h>
#include <engine/server/server.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/score.h>
#include "light.h"
#include "flag.h"

#include "weapon.h"


MACRO_ALLOC_POOL_ID_IMPL(CCharacter, MAX_CLIENTS)

// Character, "physical" player's part
CCharacter::CCharacter(CGameWorld *pWorld)
	: CEntity(pWorld, CGameWorld::ENTTYPE_CHARACTER)
{
	m_ProximityRadius = ms_PhysSize;
	m_Health = 0;
	m_Armor = 0;

	// variable initializations constructor
	m_ci_freezetime = 0;
	m_DummyDriveDuration = 0;
	m_pvp_arena_tele_request_time = 0;
	//if (g_Config.m_SvInstagibMode)
	//{
	//	Teams()->OnCharacterStart(m_pPlayer->GetCID());
	//}

}

void CCharacter::Reset()
{
	Destroy();
}

bool CCharacter::Spawn(CPlayer *pPlayer, vec2 Pos)
{
	m_EmoteStop = -1;
	m_LastAction = -1;
	m_LastNoAmmoSound = -1;
	m_LastWeapon = WEAPON_HAMMER;
	m_QueuedWeapon = -1;
	m_LastRefillJumps = false;
	m_LastPenalty = false;
	m_LastBonus = false;

	m_pPlayer = pPlayer;
	m_Pos = Pos;

	m_IsSpecHF = false;
	

	m_Core.Reset();
	m_Core.Init(&GameServer()->m_World.m_Core, GameServer()->Collision(), &((CGameControllerDDRace*)GameServer()->m_pController)->m_Teams.m_Core, &((CGameControllerDDRace*)GameServer()->m_pController)->m_TeleOuts);
	//zCatch ChillerDragon
	if (g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2 || m_pPlayer->m_IsInstaMode_gdm) //gdm & zCatch grenade
	{
		m_Core.m_ActiveWeapon = WEAPON_GRENADE;
	}
	else if (g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4 || m_pPlayer->m_IsInstaMode_idm) //idm & zCatch rifle
	{
		m_Core.m_ActiveWeapon = WEAPON_RIFLE;
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
			GameServer()->SetPlayerSurvival(m_pPlayer->GetCID(), 1);
		}
	}
	*/


	if (m_pPlayer->m_DummyMode == 99)
	{
		vec2 ShopSpawn = GameServer()->Collision()->GetRandomTile(TILE_SHOP_SPAWN);
		
		if (ShopSpawn != vec2(-1, -1))
		{
			SetPosition(ShopSpawn);
		}
		else // no shop spawn tile -> fallback to shop tile
		{
			vec2 ShopTile = GameServer()->Collision()->GetRandomTile(TILE_SHOP);

			if (ShopTile != vec2(-1, -1))
			{
				SetPosition(ShopTile);
				m_IsFreeShopBot = true;
			}
			else // no shop tile
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "No shop spawn set.");
			}
		}
	}
	else if (m_pPlayer->m_JailTime)
	{
		vec2 JailPlayerSpawn = GameServer()->Collision()->GetRandomTile(TILE_JAIL);

		if (JailPlayerSpawn != vec2(-1, -1))
		{
			SetPosition(JailPlayerSpawn);
		}
		else //no jailplayer
		{
			//GetCharacter()->SetPosition(DefaultSpawn); //crashbug for mod stealer
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "No jail set.");
		}
	}
	else if (m_pPlayer->m_IsBlockWaving && !m_pPlayer->m_IsBlockWaveWaiting)
	{
		if (m_pPlayer->m_IsDummy)
		{
			vec2 BlockWaveSpawnTile = GameServer()->Collision()->GetRandomTile(TILE_BLOCKWAVE_BOT);

			if (BlockWaveSpawnTile != vec2(-1, -1))
			{
				SetPosition(BlockWaveSpawnTile);
			}
			else //no BlockWaveSpawnTile
			{
				//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[BlockWave] No arena set.");
				GameServer()->m_BlockWaveGameState = 0;
			}
		}
		else
		{
			vec2 BlockWaveSpawnTile = GameServer()->Collision()->GetRandomTile(TILE_BLOCKWAVE_HUMAN);

			if (BlockWaveSpawnTile != vec2(-1, -1))
			{
				SetPosition(BlockWaveSpawnTile);
			}
			else //no BlockWaveSpawnTile
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[BlockWave] No arena set.");
				GameServer()->m_BlockWaveGameState = 0;
			}
		}
	}
	else if (m_pPlayer->m_IsSurvivaling)
	{
		if (m_pPlayer->m_IsSurvivalAlive)
		{
			// OLD Survival Spawn finder code (placed two tees on one spawn (random))
			// vec2 SurvivalSpawnTile = GameServer()->Collision()->GetRandomTile(TILE_SURVIVAL_SPAWN);
			// vec2 SurvivalSpawnTile = GameServer()->Collision()->GetSurvivalSpawn(m_pPlayer->GetCID());
			vec2 SurvivalSpawnTile = GameServer()->Collision()->GetSurvivalSpawn(GameServer()->m_survival_spawn_counter++);

			if (SurvivalSpawnTile != vec2(-1, -1))
			{
				SetPosition(SurvivalSpawnTile);
			}
			else //no SurvivalSpawnTile
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[SURVIVAL] No arena set.");
				GameServer()->SurvivalSetGameState(CGameContext::SURVIVAL_OFF);
			}
		}
		else
		{
			vec2 SurvivalLobbyTile = GameServer()->Collision()->GetRandomTile(TILE_SURVIVAL_LOBBY);

			if (SurvivalLobbyTile != vec2(-1, -1))
			{
				SetPosition(SurvivalLobbyTile);
			}
			else //no SurvivalLobbyTile
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[SURVIVAL] No lobby set.");
				GameServer()->SurvivalSetGameState(0);
			}
		}
	}
	else if (m_pPlayer->m_DummySpawnTile)
	{
		vec2 SpawnTile;
		if (m_pPlayer->m_DummySpawnTile == 1)
			SpawnTile = GameServer()->Collision()->GetRandomTile(TILE_BOTSPAWN_1);
		else if (m_pPlayer->m_DummySpawnTile == 2)
			SpawnTile = GameServer()->Collision()->GetRandomTile(TILE_BOTSPAWN_2);
		else if (m_pPlayer->m_DummySpawnTile == 3)
			SpawnTile = GameServer()->Collision()->GetRandomTile(TILE_BOTSPAWN_3);
		else if (m_pPlayer->m_DummySpawnTile == 4)
			SpawnTile = GameServer()->Collision()->GetRandomTile(TILE_BOTSPAWN_4);

		if (SpawnTile != vec2(-1, -1))
		{
			SetPosition(SpawnTile);
		}
		else //no botspawn tile
		{
			dbg_msg("WARNING", "player [%d][%s] failed to botspwan tile=%d",
				m_pPlayer->GetCID(), Server()->ClientName(m_pPlayer->GetCID()), m_pPlayer->m_DummySpawnTile);
			m_pPlayer->m_DummySpawnTile = 0;
		}
	}
	else if (m_pPlayer->m_IsBlockDeathmatch)
	{
		if (g_Config.m_SvBlockDMarena == 1)
		{
			vec2 BlockDMSpawn = GameServer()->Collision()->GetRandomTile(TILE_BLOCK_DM_A1);
			if (BlockDMSpawn != vec2(-1, -1))
			{
				SetPosition(BlockDMSpawn);
			}
			else
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[BLOCK] no deathmatch arena 1 found.");
				m_pPlayer->m_IsBlockDeathmatch = false;
				m_Core.m_Pos = m_Pos;
			}
		}
		else if (g_Config.m_SvBlockDMarena == 2)
		{
			vec2 BlockDMSpawn = GameServer()->Collision()->GetRandomTile(TILE_BLOCK_DM_A2);
			if (BlockDMSpawn != vec2(-1, -1))
			{
				SetPosition(BlockDMSpawn);
			}
			else
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[BLOCK] no deathmatch arena 2 found.");
				m_pPlayer->m_IsBlockDeathmatch = false;
				m_Core.m_Pos = m_Pos;
			}
		}
		else
		{
			dbg_msg("WARNING", "Invalid block deathmatch arena");
		}
	}
	else if (m_pPlayer->m_IsBalanceBatteling || m_pPlayer->m_IsBalanceBattleDummy)
	{
		if (m_pPlayer->m_IsBalanceBattlePlayer1)
		{
			vec2 BalanceBattleSpawn = GameServer()->Collision()->GetRandomTile(TILE_BALANCE_BATTLE_1);

			if (BalanceBattleSpawn != vec2(-1, -1))
			{
				SetPosition(BalanceBattleSpawn);
			}
			else //no balance battle spawn tile
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[balance] no battle arena found.");
				m_pPlayer->m_IsBalanceBatteling = false;
				m_Core.m_Pos = m_Pos;
			}
		}
		else
		{
			vec2 BalanceBattleSpawn = GameServer()->Collision()->GetRandomTile(TILE_BALANCE_BATTLE_2);

			if (BalanceBattleSpawn != vec2(-1, -1))
			{
				SetPosition(BalanceBattleSpawn);
			}
			else //no balance battle spawn tile
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[balance] no battle arena found.");
				m_pPlayer->m_IsBalanceBatteling = false;
				m_Core.m_Pos = m_Pos;
			}
		}
	}
	else if (m_pPlayer->m_IsSuperModSpawn && !m_pPlayer->IsInstagibMinigame())
	{
		m_Core.m_Pos.x = g_Config.m_SvSuperSpawnX * 32;
		m_Core.m_Pos.y = g_Config.m_SvSuperSpawnY * 32;
	}
	else if (m_pPlayer->m_IsNoboSpawn)
	{
		char aBuf[128];
		if (pPlayer->m_NoboSpawnStop > Server()->Tick())
		{
			str_format(aBuf, sizeof(aBuf), "[NoboSpawn] Time until real spawn is unlocked: %d sec", (pPlayer->m_NoboSpawnStop - Server()->Tick()) / Server()->TickSpeed());
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			m_Core.m_Pos.x = g_Config.m_SvNoboSpawnX * 32;
			m_Core.m_Pos.y = g_Config.m_SvNoboSpawnY * 32;
		}
		else
		{
			m_Core.m_Pos = m_Pos;
			m_pPlayer->m_IsNoboSpawn = false;
			str_format(aBuf, sizeof(aBuf), "[NoboSpawn] Welcome to the real spawn!");
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
		}
	}
	else
	{
		m_Core.m_Pos = m_Pos;
	}

	GameServer()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = &m_Core;

	m_ReckoningTick = 0;
	mem_zero(&m_SendCore, sizeof(m_SendCore));
	mem_zero(&m_ReckoningCore, sizeof(m_ReckoningCore));

	GameServer()->m_World.InsertEntity(this);
	m_Alive = true;

	GameServer()->m_pController->OnCharacterSpawn(this);

	Teams()->OnCharacterSpawn(GetPlayer()->GetCID());

	DDRaceInit();

	m_TuneZone = GameServer()->Collision()->IsTune(GameServer()->Collision()->GetMapIndex(Pos));
	m_TuneZoneOld = -1; // no zone leave msg on spawn
	m_NeededFaketuning = 0; // reset fake tunings on respawn and send the client
	SendZoneMsgs(); // we want a entermessage also on spawn
	GameServer()->SendTuningParams(m_pPlayer->GetCID(), m_TuneZone);

	Server()->StartRecord(m_pPlayer->GetCID());

	m_AliveTime = Server()->Tick();


	if (g_Config.m_SvInstagibMode)
	{
		Teams()->OnCharacterStart(m_pPlayer->GetCID());
		m_CpActive = -2;
	}

	m_aWeapons[0].m_Ammo = -1; //this line is added by ChillerDragon to prevent hammer in vanilla mode to run out of ammo. Im sure this solution is a bit hacky ... to who ever who is reading this comment: feel free to fix the core of the problem.

	if (!m_pPlayer->m_IsSurvivaling && !m_pPlayer->m_IsVanillaWeapons)
	{
		m_aWeapons[1].m_Ammo = -1; // added by fokkonaut to have -1 (infinite) bullets of gun at spawn and not 10. after freeze you would have -1 anyways so why not when spawning
	}

	if (m_pPlayer->m_IsSurvivaling && !g_Config.m_SvSurvivalGunAmmo)
	{
		m_aWeapons[1].m_Got = false;
		m_Core.m_ActiveWeapon = WEAPON_HAMMER;
	}

	if (GetPlayer()->m_IsSurvivaling && GetPlayer()->m_IsSurvivalAlive == false)
	{
		GameServer()->LoadCosmetics(GetPlayer()->GetCID());
	}

	m_LastHitWeapon = -1;

	m_pPlayer->m_SpawnShotgunActive = 0;
	m_pPlayer->m_SpawnGrenadeActive = 0;
	m_pPlayer->m_SpawnRifleActive = 0;

	if (g_Config.m_SvAllowSpawnWeapons)
	{
		SetSpawnWeapons();
	}

	SaveRealInfos();

	UnsetSpookyGhost();

	if (m_pPlayer->m_HadFlagOnDeath)
	{
		m_pPlayer->m_ChangeTeamOnFlag = true;
		m_pPlayer->m_HadFlagOnDeath = false;
	}

	return true;
}

void CCharacter::Destroy()
{
	GameServer()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = 0;
	m_Alive = false;
}

void CCharacter::SetWeapon(int W)
{
	if (W == m_Core.m_ActiveWeapon)
		return;

	m_LastWeapon = m_Core.m_ActiveWeapon;
	m_QueuedWeapon = -1;
	m_Core.m_ActiveWeapon = W;
	GameServer()->CreateSound(m_Pos, SOUND_WEAPON_SWITCH, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));

	if (m_Core.m_ActiveWeapon < 0 || m_Core.m_ActiveWeapon >= NUM_WEAPONS)
		m_Core.m_ActiveWeapon = 0;
}

void CCharacter::SetSolo(bool Solo)
{
	Teams()->m_Core.SetSolo(m_pPlayer->GetCID(), Solo);

	if (Solo)
		m_NeededFaketuning |= FAKETUNE_SOLO;
	else
		m_NeededFaketuning &= ~FAKETUNE_SOLO;

	GameServer()->SendTuningParams(m_pPlayer->GetCID(), m_TuneZone); // update tunings
}

bool CCharacter::IsGrounded()
{
	if (GameServer()->Collision()->CheckPoint(m_Pos.x + m_ProximityRadius / 2, m_Pos.y + m_ProximityRadius / 2 + 5))
		return true;
	if (GameServer()->Collision()->CheckPoint(m_Pos.x - m_ProximityRadius / 2, m_Pos.y + m_ProximityRadius / 2 + 5))
		return true;

	int index = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x, m_Pos.y + m_ProximityRadius / 2 + 4));
	int tile = GameServer()->Collision()->GetTileIndex(index);
	int flags = GameServer()->Collision()->GetTileFlags(index);
	if (tile == TILE_STOPA || (tile == TILE_STOP && flags == ROTATION_0) || (tile == TILE_STOPS && (flags == ROTATION_0 || flags == ROTATION_180)))
		return true;
	tile = GameServer()->Collision()->GetFTileIndex(index);
	flags = GameServer()->Collision()->GetFTileFlags(index);
	if (tile == TILE_STOPA || (tile == TILE_STOP && flags == ROTATION_0) || (tile == TILE_STOPS && (flags == ROTATION_0 || flags == ROTATION_180)))
		return true;

	return false;
}

void CCharacter::HandleJetpack()
{
	if (isFreezed || m_FreezeTime)
		return;

	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));

	bool FullAuto = false;
	if (m_Core.m_ActiveWeapon == WEAPON_GRENADE || m_Core.m_ActiveWeapon == WEAPON_SHOTGUN || m_Core.m_ActiveWeapon == WEAPON_RIFLE)
		FullAuto = true;
	if (m_Jetpack && m_Core.m_ActiveWeapon == WEAPON_GUN)
		FullAuto = true;

	// check if we gonna fire
	bool WillFire = false;
	if (CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses)
		WillFire = true;

	if (FullAuto && (m_LatestInput.m_Fire & 1) && m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo)
		WillFire = true;

	if (!WillFire)
		return;

	// check for ammo
	if (!m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo)
	{
		return;
	}

	switch (m_Core.m_ActiveWeapon)
	{
	case WEAPON_GUN:
	{
		if (m_Jetpack)
		{
			float Strength;
			if (!m_TuneZone)
				Strength = GameServer()->Tuning()->m_JetpackStrength;
			else
				Strength = GameServer()->TuningList()[m_TuneZone].m_JetpackStrength;
			TakeDamage(Direction * -1.0f * (Strength / 100.0f / 6.11f), g_pData->m_Weapons.m_Hammer.m_pBase->m_Damage, m_pPlayer->GetCID(), m_Core.m_ActiveWeapon);
		}
	}
	}
}

void CCharacter::HandleNinja()
{
	if (m_Core.m_ActiveWeapon != WEAPON_NINJA)
		return;

	if ((Server()->Tick() - m_Ninja.m_ActivationTick) > (g_pData->m_Weapons.m_Ninja.m_Duration * Server()->TickSpeed() / 1000))
	{
		// time's up, return
		m_Ninja.m_CurrentMoveTime = 0;
		m_aWeapons[WEAPON_NINJA].m_Got = false;
		m_Core.m_ActiveWeapon = m_LastWeapon;

		SetWeapon(m_Core.m_ActiveWeapon);
		return;
	}

	int NinjaTime = m_Ninja.m_ActivationTick + (g_pData->m_Weapons.m_Ninja.m_Duration * Server()->TickSpeed() / 1000) - Server()->Tick();

	if (NinjaTime % Server()->TickSpeed() == 0 && NinjaTime / Server()->TickSpeed() <= 5)
	{
		GameServer()->CreateDamageInd(m_Pos, 0, NinjaTime / Server()->TickSpeed(), Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
	}

	if (!m_pPlayer->m_IsVanillaDmg)
	{
		m_Armor = 10 - (NinjaTime / 15);
	}

	// force ninja Weapon
	SetWeapon(WEAPON_NINJA);

	m_Ninja.m_CurrentMoveTime--;

	if (m_Ninja.m_CurrentMoveTime == 0)
	{
		// reset velocity
		m_Core.m_Vel = m_Ninja.m_ActivationDir*m_Ninja.m_OldVelAmount;
	}

	if (m_Ninja.m_CurrentMoveTime > 0)
	{
		// Set velocity
		m_Core.m_Vel = m_Ninja.m_ActivationDir * g_pData->m_Weapons.m_Ninja.m_Velocity;
		vec2 OldPos = m_Pos;
		GameServer()->Collision()->MoveBox(&m_Core.m_Pos, &m_Core.m_Vel, vec2(m_ProximityRadius, m_ProximityRadius), 0.f);

		// reset velocity so the client doesn't predict stuff
		m_Core.m_Vel = vec2(0.f, 0.f);

		// check if we Hit anything along the way
		{
			CCharacter *aEnts[MAX_CLIENTS];
			vec2 Dir = m_Pos - OldPos;
			float Radius = m_ProximityRadius * 2.0f;
			vec2 Center = OldPos + Dir * 0.5f;
			int Num = GameServer()->m_World.FindEntities(Center, Radius, (CEntity**)aEnts, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

			// check that we're not in solo part
			if (Teams()->m_Core.GetSolo(m_pPlayer->GetCID()))
				return;

			for (int i = 0; i < Num; ++i)
			{
				if (aEnts[i] == this)
					continue;

				// Don't hit players in other teams
				if (Team() != aEnts[i]->Team())
					continue;

				// Don't hit players in solo parts
				if (Teams()->m_Core.GetSolo(aEnts[i]->m_pPlayer->GetCID()))
					return;

				// make sure we haven't Hit this object before
				bool bAlreadyHit = false;
				for (int j = 0; j < m_NumObjectsHit; j++)
				{
					if (m_apHitObjects[j] == aEnts[i])
						bAlreadyHit = true;
				}
				if (bAlreadyHit)
					continue;

				// check so we are sufficiently close
				if (distance(aEnts[i]->m_Pos, m_Pos) >(m_ProximityRadius * 2.0f))
					continue;

				// Hit a player, give him damage and stuffs...
				GameServer()->CreateSound(aEnts[i]->m_Pos, SOUND_NINJA_HIT, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
				// set his velocity to fast upward (for now)
				if (m_NumObjectsHit < 10)
					m_apHitObjects[m_NumObjectsHit++] = aEnts[i];

				aEnts[i]->TakeDamage(vec2(0, -10.0f), g_pData->m_Weapons.m_Ninja.m_pBase->m_Damage, m_pPlayer->GetCID(), WEAPON_NINJA);
			}
		}

		return;
	}

	return;
}


void CCharacter::DoWeaponSwitch()
{
	// make sure we can switch
	if (m_ReloadTimer != 0 || m_QueuedWeapon == -1 || m_aWeapons[WEAPON_NINJA].m_Got)
		return;

	// switch Weapon
	SetWeapon(m_QueuedWeapon);
}

void CCharacter::HandleWeaponSwitch()
{
	int WantedWeapon = m_Core.m_ActiveWeapon;
	if (m_QueuedWeapon != -1)
		WantedWeapon = m_QueuedWeapon;

	bool Anything = false;
	for (int i = 0; i < NUM_WEAPONS - 1; ++i)
		if (m_aWeapons[i].m_Got)
			Anything = true;
	if (!Anything)
		return;
	// select Weapon
	int Next = CountInput(m_LatestPrevInput.m_NextWeapon, m_LatestInput.m_NextWeapon).m_Presses;
	int Prev = CountInput(m_LatestPrevInput.m_PrevWeapon, m_LatestInput.m_PrevWeapon).m_Presses;

	if (Next < 128) // make sure we only try sane stuff
	{
		while (Next) // Next Weapon selection
		{
			WantedWeapon = (WantedWeapon + 1) % NUM_WEAPONS;
			if (m_aWeapons[WantedWeapon].m_Got)
				Next--;
		}
	}

	if (Prev < 128) // make sure we only try sane stuff
	{
		while (Prev) // Prev Weapon selection
		{
			WantedWeapon = (WantedWeapon - 1)<0 ? NUM_WEAPONS - 1 : WantedWeapon - 1;
			if (m_aWeapons[WantedWeapon].m_Got)
				Prev--;
		}
	}

	// Direct Weapon selection
	if (m_LatestInput.m_WantedWeapon)
		WantedWeapon = m_Input.m_WantedWeapon - 1;

	// check for insane values
	if (WantedWeapon >= 0 && WantedWeapon < NUM_WEAPONS && WantedWeapon != m_Core.m_ActiveWeapon && m_aWeapons[WantedWeapon].m_Got)
		m_QueuedWeapon = WantedWeapon;

	DoWeaponSwitch();
}

void CCharacter::FireWeapon(bool Bot)
{
	if (m_ReloadTimer != 0)
		return;

	DoWeaponSwitch();
	vec2 Direction = normalize(vec2(m_LatestInput.m_TargetX, m_LatestInput.m_TargetY));

	bool FullAuto = false;
	if (m_Core.m_ActiveWeapon == WEAPON_GRENADE || m_Core.m_ActiveWeapon == WEAPON_SHOTGUN || m_Core.m_ActiveWeapon == WEAPON_RIFLE || m_Pullhammer)
		FullAuto = true;
	if (m_Jetpack && m_Core.m_ActiveWeapon == WEAPON_GUN)
		FullAuto = true;
	if (m_autospreadgun && m_Core.m_ActiveWeapon == WEAPON_GUN)
		FullAuto = true;
	if (m_pPlayer->m_InfAutoSpreadGun && m_Core.m_ActiveWeapon == WEAPON_GUN)
		FullAuto = true;

	// check if we gonna fire
	bool WillFire = false;
	if (CountInput(m_LatestPrevInput.m_Fire, m_LatestInput.m_Fire).m_Presses)
	{
		WillFire = true;
		if (m_pPlayer->m_PlayerFlags&PLAYERFLAG_SCOREBOARD && m_pPlayer->m_SpookyGhost && m_Core.m_ActiveWeapon == WEAPON_GUN)
		{
			m_CountSpookyGhostInputs = true;
		}
		
		if ((m_ShopWindowPage != -1) && (m_PurchaseState == 1))
		{
			m_ChangeShopPage = true;
		}
	}

	if (FullAuto && (m_LatestInput.m_Fire & 1) && m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo)
		WillFire = true;

	if (!WillFire && !m_Fire)
	{
		if (m_Pullhammer)
			m_PullingID = -1;
		return;
	}

	/*
	// Used to debug the ddnet client dummy bug
	// could be usefull in the future maybe
	if (m_LatestInput.m_Fire != m_LatestPrevInput.m_Fire + 1 || m_LatestInput.m_Fire != m_Input.m_Fire + 1)
	{
	dbg_msg("ddpp-inp", "latestinp: %d latestprevinp: %d inp: %d name: '%s'     (BROKEN INPUT)", m_LatestInput.m_Fire, m_LatestPrevInput.m_Fire, m_Input.m_Fire, Server()->ClientName(m_pPlayer->GetCID()));
	}
	else
	{
	dbg_msg("ddpp-inp", "latestinp: %d latestprevinp: %d inp: %d name: '%s'", m_LatestInput.m_Fire, m_LatestPrevInput.m_Fire, m_Input.m_Fire, Server()->ClientName(m_pPlayer->GetCID()));
	}
	*/

	if (GetPlayer()->GetCharacter() && m_Pullhammer && m_Core.m_ActiveWeapon == WEAPON_HAMMER)
	{
		if (m_PullingID == -1 || !GameServer()->GetPlayerChar(m_PullingID)) //no one gets pulled, so search for one!
		{
			CCharacter * pTarget = GameWorld()->ClosestCharacter(MousePos(), 20.f, GetPlayer()->GetCharacter()); // Don't allow the user to use it on their self, Alot of people seem to be abusing and bugging themselves into walls... -.-
			if (pTarget)
				m_PullingID = pTarget->GetPlayer()->GetCID();
		}
		else
		{
			//crash prevention
			CPlayer * pTargetPlayer = GameServer()->m_apPlayers[m_PullingID];

			if (pTargetPlayer)
			{
				CCharacter *pTarget = GameServer()->m_apPlayers[m_PullingID]->GetCharacter();

				if (pTarget->GetPlayer()->GetCharacter() && pTarget)
				{
					pTarget->Core()->m_Pos = MousePos();
					pTarget->Core()->m_Vel.y = 0;
				}
				else
				{
					m_PullingID = -1;
					return;
				}
			}
			else
			{
				m_PullingID = -1;
				return;
			}
		}
		return;
	}

	// check for ammo
	if (!m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo || m_FreezeTime)
	{
		if (m_pPlayer->m_IsVanillaWeapons)
		{
			// 125ms is a magical limit of how fast a human can click
			m_ReloadTimer = 125 * Server()->TickSpeed() / 1000;
			GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO);
		}
		else
		{
			// Timer stuff to avoid shrieking orchestra caused by unfreeze-plasma
			if (m_PainSoundTimer <= 0)
			{
				m_PainSoundTimer = 1 * Server()->TickSpeed();
				GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_LONG, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
			}
		}
		return;
	}

	vec2 ProjStartPos = m_Pos + Direction*m_ProximityRadius*0.75f;

	switch (m_Core.m_ActiveWeapon)
	{
	case WEAPON_HAMMER:
	{
		//hammer delay on super jail hammer
		if (m_pPlayer->m_JailHammer > 1 && m_pPlayer->m_JailHammerDelay)
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "You have to wait %d minutes to use your super jail hammer agian.", (m_pPlayer->m_JailHammerDelay / Server()->TickSpeed()) / 60);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			return;
		}



		// reset objects Hit
		m_NumObjectsHit = 0;
		GameServer()->CreateSound(m_Pos, SOUND_HAMMER_FIRE, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));

		if (m_Hit&DISABLE_HIT_HAMMER) break;

		CCharacter *apEnts[MAX_CLIENTS];
		int Hits = 0;
		int Num = GameServer()->m_World.FindEntities(ProjStartPos, m_ProximityRadius*0.5f, (CEntity**)apEnts,
			MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

		for (int i = 0; i < Num; ++i)
		{
			CCharacter *pTarget = apEnts[i];

			//if ((pTarget == this) || GameServer()->Collision()->IntersectLine(ProjStartPos, pTarget->m_Pos, NULL, NULL))
			if ((pTarget == this || (pTarget->IsAlive() && !CanCollide(pTarget->GetPlayer()->GetCID()))))
				continue;

			// set his velocity to fast upward (for now)
			if (length(pTarget->m_Pos - ProjStartPos) > 0.0f)
				GameServer()->CreateHammerHit(pTarget->m_Pos - normalize(pTarget->m_Pos - ProjStartPos)*m_ProximityRadius*0.5f, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
			else
				GameServer()->CreateHammerHit(ProjStartPos, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));


			vec2 Dir = vec2(0.f, 0.f);
			if (m_pPlayer->m_IsInstaMode_fng && m_pPlayer->m_aFngConfig[1] == '1')
			{
				pTarget->TakeHammerHit(this);
			}
			else
			{
				if (length(pTarget->m_Pos - m_Pos) > 0.0f)
					Dir = normalize(pTarget->m_Pos - m_Pos);
				else
					Dir = vec2(0.f, -1.f);
			}

			/*pTarget->TakeDamage(vec2(0.f, -1.f) + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f, g_pData->m_Weapons.m_Hammer.m_pBase->m_Damage,
			m_pPlayer->GetCID(), m_Core.m_ActiveWeapon);*/

			// shop bot
			if (pTarget->m_pPlayer->m_IsDummy)
			{
				if (pTarget->m_pPlayer->m_DummyMode == 99)
				{
					StartShop();
				}
			}

			//Bomb (put it dat early cuz the unfreeze stuff)
			if (m_IsBombing && pTarget->m_IsBombing)
			{
				if (m_IsBomb) //if bomb hits others --> they get bomb
				{
					if (!pTarget->isFreezed && !pTarget->m_FreezeTime) //you cant bomb freezed players
					{
						m_IsBomb = false;
						pTarget->m_IsBomb = true;

						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "%s bombed %s", Server()->ClientName(m_pPlayer->GetCID()), Server()->ClientName(pTarget->GetPlayer()->GetCID()));
						GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "bomb", aBuf);
					}
				}
			}

			//Quests  (before police so no confusion i hope)
			if (m_pPlayer->m_QuestState == CPlayer::QUEST_HAMMER) //if is questing and hammer quest
			{
				if (GameServer()->IsSameIP(m_pPlayer->GetCID(), pTarget->GetPlayer()->GetCID()))
				{
					if ((m_pPlayer->m_QuestStateLevel == 4 && pTarget->m_FreezeTime == 0) || // freezed quest
						m_pPlayer->m_QuestStateLevel == 5 || // <specific player> quest
						m_pPlayer->m_QuestStateLevel == 6 || // <specific player> quest
						m_pPlayer->m_QuestStateLevel == 7) // <specific player> quest
					{
						//dont send message here
					}
					else
					{
						if (!m_pPlayer->m_HideQuestWarning)
						{
							GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] your dummy doesnt count.");
						}
					}
				}
				else
				{
					if (m_pPlayer->m_QuestStateLevel == 0)
					{
						GameServer()->QuestCompleted(m_pPlayer->GetCID());
					}
					else if (m_pPlayer->m_QuestStateLevel == 1)
					{
						if (m_pPlayer->m_QuestLastQuestedPlayerID == pTarget->GetPlayer()->GetCID())
						{
							if (!m_pPlayer->m_HideQuestWarning)
							{
								GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] hammer a different tee.");
							}
						}
						else
						{
							GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 2);
							m_pPlayer->m_QuestLastQuestedPlayerID = pTarget->GetPlayer()->GetCID();
						}
					}
					else if (m_pPlayer->m_QuestStateLevel == 2)
					{
						if (m_pPlayer->m_QuestLastQuestedPlayerID == pTarget->GetPlayer()->GetCID())
						{
							if (!m_pPlayer->m_HideQuestWarning)
							{
								GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] hammer a different tee.");
							}
						}
						else
						{
							GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 3);
							m_pPlayer->m_QuestLastQuestedPlayerID = pTarget->GetPlayer()->GetCID();
						}
					}
					else if (m_pPlayer->m_QuestStateLevel == 3)
					{
						if (m_pPlayer->m_QuestLastQuestedPlayerID == pTarget->GetPlayer()->GetCID())
						{
							if (!m_pPlayer->m_HideQuestWarning)
							{
								GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] hammer a different tee.");
							}
						}
						else
						{
							GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 5);
							m_pPlayer->m_QuestLastQuestedPlayerID = pTarget->GetPlayer()->GetCID();
						}
					}
					else if (m_pPlayer->m_QuestStateLevel == 4)
					{
						if (pTarget->m_FreezeTime == 0)
						{
							//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] this tee is not freezed.");
						}
						else if (m_pPlayer->m_QuestLastQuestedPlayerID == pTarget->GetPlayer()->GetCID())
						{
							if (!m_pPlayer->m_HideQuestWarning)
							{
								GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] hammer a different tee.");
							}
						}
						else
						{
							GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 10);
							m_pPlayer->m_QuestLastQuestedPlayerID = pTarget->GetPlayer()->GetCID();
						}
					}
					else if (m_pPlayer->m_QuestStateLevel == 5)
					{
						if (m_pPlayer->m_QuestPlayerID != pTarget->GetPlayer()->GetCID())
						{
							//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] you hammered the wrong tee.");
						}
						else
						{
							GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 20);
							m_pPlayer->m_QuestLastQuestedPlayerID = pTarget->GetPlayer()->GetCID();
						}
					}
					else if (m_pPlayer->m_QuestStateLevel == 6)
					{
						if (m_pPlayer->m_QuestPlayerID != pTarget->GetPlayer()->GetCID())
						{
							//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] you hammered the wrong tee.");
						}
						else if (pTarget->m_FreezeTime == 0)
						{
							//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] this tee is not freezed.");
						}
						else
						{
							GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 3);
							m_pPlayer->m_QuestLastQuestedPlayerID = pTarget->GetPlayer()->GetCID();
						}
					}
					else if (m_pPlayer->m_QuestStateLevel == 7)
					{
						if (m_pPlayer->m_QuestPlayerID != pTarget->GetPlayer()->GetCID())
						{
							//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] you hammered the wrong tee.");
						}
						else
						{
							GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 11, 10);
							m_pPlayer->m_QuestLastQuestedPlayerID = pTarget->GetPlayer()->GetCID();
						}
					}
					else if (m_pPlayer->m_QuestStateLevel == 9)
					{
						if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1) //has flag
						{
							if (m_pPlayer->m_QuestLastQuestedPlayerID == pTarget->GetPlayer()->GetCID())
							{
								if (!m_pPlayer->m_HideQuestWarning)
								{
									GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] hammer a different tee.");
								}
							}
							else if (pTarget->m_FreezeTime == 0)
							{
								//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] this tee is not freezed.");
							}
							else
							{
								GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 10);
								m_pPlayer->m_QuestLastQuestedPlayerID = pTarget->GetPlayer()->GetCID();
							}
						}
						else
						{
							//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] you have to carry the flag");
						}
					}
					else
					{
						dbg_msg("QUEST", "WARNING! character.cpp undefined quest level %d", m_pPlayer->m_QuestStateLevel);
					}
				}
			}
			else if (m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
			{
				if (m_pPlayer->m_QuestStateLevel == 9) //race with conditions
				{
					if (g_Config.m_SvQuestRaceCondition == 0) //no hammer
					{
						GameServer()->QuestFailed2(m_pPlayer->GetCID());
					}
				}
			}


			//Police catch gangstazz
			if (m_pPlayer->m_PoliceRank && pTarget->m_FreezeTime > 1 && m_pPlayer->m_JailHammer)
			{
				char aBuf[256];

				if (!GameServer()->IsMinigame(pTarget->GetPlayer()->GetCID()))
				{
					if (pTarget->GetPlayer()->m_EscapeTime) //always prefer normal hammer
					{
						if (pTarget->GetPlayer()->GetMoney() < 200)
						{
							str_format(aBuf, sizeof(aBuf), "You caught the gangster '%s' (5 minutes arrest).", Server()->ClientName(pTarget->GetPlayer()->GetCID()));
							GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
							GameServer()->SendChatTarget(m_pPlayer->GetCID(), "+5 minutes extra arrest: He had no money to corrupt you!");

							str_format(aBuf, sizeof(aBuf), "You were arrested for 5 minutes by '%s'.", Server()->ClientName(m_pPlayer->GetCID()));
							GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCID(), aBuf);
							GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCID(), "+5 minutes extra: You couldn't corrupt the police!");
							pTarget->GetPlayer()->m_EscapeTime = 0;
							pTarget->GetPlayer()->m_GangsterBagMoney = 0;
							pTarget->GetPlayer()->JailPlayer(600); //10 minutes jail
							pTarget->GetPlayer()->m_JailCode = rand() % 8999 + 1000;
						}
						else
						{
							str_format(aBuf, sizeof(aBuf), "You caught the gangster '%s' (5 minutes arrest).", Server()->ClientName(pTarget->GetPlayer()->GetCID()));
							GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
							GameServer()->SendChatTarget(m_pPlayer->GetCID(), "+200 money (corruption)");
							str_format(aBuf, sizeof(aBuf), "caught gangster '%s'", Server()->ClientName(pTarget->GetPlayer()->GetCID()));
							m_pPlayer->MoneyTransaction(+200, aBuf);

							str_format(aBuf, sizeof(aBuf), "You were arrested 5 minutes by '%s'.", Server()->ClientName(m_pPlayer->GetCID()));
							GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCID(), aBuf);
							GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCID(), "-200 money (corruption)");
							pTarget->GetPlayer()->m_EscapeTime = 0;
							pTarget->GetPlayer()->m_GangsterBagMoney = 0;
							pTarget->GetPlayer()->JailPlayer(300); //5 minutes jail
							pTarget->GetPlayer()->m_JailCode = rand() % 8999 + 1000;
							pTarget->GetPlayer()->MoneyTransaction(-200, "jail");

						}
					}
					else //super jail hammer
					{
						if (m_pPlayer->m_JailHammer > 1)
						{
							str_format(aBuf, sizeof(aBuf), "You jailed '%s' (%d seconds arrested).", Server()->ClientName(pTarget->GetPlayer()->GetCID()), m_pPlayer->m_JailHammer);
							GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
							m_pPlayer->m_JailHammerDelay = Server()->TickSpeed() * 1200; // can only use every 20 minutes super hammer

							str_format(aBuf, sizeof(aBuf), "You were arrested by '%s' for %d seconds.", m_pPlayer->m_JailHammer, Server()->ClientName(m_pPlayer->GetCID()));
							GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCID(), aBuf);
							pTarget->GetPlayer()->m_EscapeTime = 0;
							pTarget->GetPlayer()->m_GangsterBagMoney = 0;
							pTarget->GetPlayer()->JailPlayer(Server()->TickSpeed() * m_pPlayer->m_JailHammer);
							pTarget->GetPlayer()->m_JailCode = rand() % 8999 + 1000;
						}
					}
				}
			}


			float Strength; //hammer bounce code (marked by ChillerDragon)
			if (!m_TuneZone)
				Strength = GameServer()->Tuning()->m_HammerStrength;
			else
				Strength = GameServer()->TuningList()[m_TuneZone].m_HammerStrength;

			vec2 Temp = pTarget->m_Core.m_Vel + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f;
			if (Temp.x > 0 && ((pTarget->m_TileIndex == TILE_STOP && pTarget->m_TileFlags == ROTATION_270) || (pTarget->m_TileIndexL == TILE_STOP && pTarget->m_TileFlagsL == ROTATION_270) || (pTarget->m_TileIndexL == TILE_STOPS && (pTarget->m_TileFlagsL == ROTATION_90 || pTarget->m_TileFlagsL == ROTATION_270)) || (pTarget->m_TileIndexL == TILE_STOPA) || (pTarget->m_TileFIndex == TILE_STOP && pTarget->m_TileFFlags == ROTATION_270) || (pTarget->m_TileFIndexL == TILE_STOP && pTarget->m_TileFFlagsL == ROTATION_270) || (pTarget->m_TileFIndexL == TILE_STOPS && (pTarget->m_TileFFlagsL == ROTATION_90 || pTarget->m_TileFFlagsL == ROTATION_270)) || (pTarget->m_TileFIndexL == TILE_STOPA) || (pTarget->m_TileSIndex == TILE_STOP && pTarget->m_TileSFlags == ROTATION_270) || (pTarget->m_TileSIndexL == TILE_STOP && pTarget->m_TileSFlagsL == ROTATION_270) || (pTarget->m_TileSIndexL == TILE_STOPS && (pTarget->m_TileSFlagsL == ROTATION_90 || pTarget->m_TileSFlagsL == ROTATION_270)) || (pTarget->m_TileSIndexL == TILE_STOPA)))
				Temp.x = 0;
			if (Temp.x < 0 && ((pTarget->m_TileIndex == TILE_STOP && pTarget->m_TileFlags == ROTATION_90) || (pTarget->m_TileIndexR == TILE_STOP && pTarget->m_TileFlagsR == ROTATION_90) || (pTarget->m_TileIndexR == TILE_STOPS && (pTarget->m_TileFlagsR == ROTATION_90 || pTarget->m_TileFlagsR == ROTATION_270)) || (pTarget->m_TileIndexR == TILE_STOPA) || (pTarget->m_TileFIndex == TILE_STOP && pTarget->m_TileFFlags == ROTATION_90) || (pTarget->m_TileFIndexR == TILE_STOP && pTarget->m_TileFFlagsR == ROTATION_90) || (pTarget->m_TileFIndexR == TILE_STOPS && (pTarget->m_TileFFlagsR == ROTATION_90 || pTarget->m_TileFFlagsR == ROTATION_270)) || (pTarget->m_TileFIndexR == TILE_STOPA) || (pTarget->m_TileSIndex == TILE_STOP && pTarget->m_TileSFlags == ROTATION_90) || (pTarget->m_TileSIndexR == TILE_STOP && pTarget->m_TileSFlagsR == ROTATION_90) || (pTarget->m_TileSIndexR == TILE_STOPS && (pTarget->m_TileSFlagsR == ROTATION_90 || pTarget->m_TileSFlagsR == ROTATION_270)) || (pTarget->m_TileSIndexR == TILE_STOPA)))
				Temp.x = 0;
			if (Temp.y < 0 && ((pTarget->m_TileIndex == TILE_STOP && pTarget->m_TileFlags == ROTATION_180) || (pTarget->m_TileIndexB == TILE_STOP && pTarget->m_TileFlagsB == ROTATION_180) || (pTarget->m_TileIndexB == TILE_STOPS && (pTarget->m_TileFlagsB == ROTATION_0 || pTarget->m_TileFlagsB == ROTATION_180)) || (pTarget->m_TileIndexB == TILE_STOPA) || (pTarget->m_TileFIndex == TILE_STOP && pTarget->m_TileFFlags == ROTATION_180) || (pTarget->m_TileFIndexB == TILE_STOP && pTarget->m_TileFFlagsB == ROTATION_180) || (pTarget->m_TileFIndexB == TILE_STOPS && (pTarget->m_TileFFlagsB == ROTATION_0 || pTarget->m_TileFFlagsB == ROTATION_180)) || (pTarget->m_TileFIndexB == TILE_STOPA) || (pTarget->m_TileSIndex == TILE_STOP && pTarget->m_TileSFlags == ROTATION_180) || (pTarget->m_TileSIndexB == TILE_STOP && pTarget->m_TileSFlagsB == ROTATION_180) || (pTarget->m_TileSIndexB == TILE_STOPS && (pTarget->m_TileSFlagsB == ROTATION_0 || pTarget->m_TileSFlagsB == ROTATION_180)) || (pTarget->m_TileSIndexB == TILE_STOPA)))
				Temp.y = 0;
			if (Temp.y > 0 && ((pTarget->m_TileIndex == TILE_STOP && pTarget->m_TileFlags == ROTATION_0) || (pTarget->m_TileIndexT == TILE_STOP && pTarget->m_TileFlagsT == ROTATION_0) || (pTarget->m_TileIndexT == TILE_STOPS && (pTarget->m_TileFlagsT == ROTATION_0 || pTarget->m_TileFlagsT == ROTATION_180)) || (pTarget->m_TileIndexT == TILE_STOPA) || (pTarget->m_TileFIndex == TILE_STOP && pTarget->m_TileFFlags == ROTATION_0) || (pTarget->m_TileFIndexT == TILE_STOP && pTarget->m_TileFFlagsT == ROTATION_0) || (pTarget->m_TileFIndexT == TILE_STOPS && (pTarget->m_TileFFlagsT == ROTATION_0 || pTarget->m_TileFFlagsT == ROTATION_180)) || (pTarget->m_TileFIndexT == TILE_STOPA) || (pTarget->m_TileSIndex == TILE_STOP && pTarget->m_TileSFlags == ROTATION_0) || (pTarget->m_TileSIndexT == TILE_STOP && pTarget->m_TileSFlagsT == ROTATION_0) || (pTarget->m_TileSIndexT == TILE_STOPS && (pTarget->m_TileSFlagsT == ROTATION_0 || pTarget->m_TileSFlagsT == ROTATION_180)) || (pTarget->m_TileSIndexT == TILE_STOPA)))
				Temp.y = 0;
			Temp -= pTarget->m_Core.m_Vel;

			if (m_pPlayer->m_IsInstaMode_fng) //dont damage with hammer in fng
			{
				pTarget->TakeDamage((vec2(0.f, -1.0f) + Temp) * Strength, 0,
					m_pPlayer->GetCID(), m_Core.m_ActiveWeapon);
			}
			else
			{
				pTarget->TakeDamage((vec2(0.f, -1.0f) + Temp) * Strength, g_pData->m_Weapons.m_Hammer.m_pBase->m_Damage,
					m_pPlayer->GetCID(), m_Core.m_ActiveWeapon);
			}

			if (!pTarget->m_pPlayer->m_RconFreeze && !m_pPlayer->m_IsInstaMode_fng)
				pTarget->UnFreeze();

			if (m_FreezeHammer)
				pTarget->Freeze();


			Hits++;
		}

		// if we Hit anything, we have to wait for the reload
		if (Hits)
			m_ReloadTimer = Server()->TickSpeed() / 3;

		if (Hits > 1)
		{
			if (m_pPlayer->m_QuestState == CPlayer::QUEST_HAMMER)
			{
				if (m_pPlayer->m_QuestStateLevel == 8) // Hammer 2+ tees in one hit
				{
					GameServer()->QuestCompleted(m_pPlayer->GetCID());
				}
			}
		}

		if (m_CanHarvestPlant)
		{
			m_HarvestPlant = true;
		}

	} break;

	case WEAPON_GUN:
	{
		if (!m_Jetpack || !m_pPlayer->m_NinjaJetpack)
		{
			int Lifetime;
			if (!m_TuneZone)
				Lifetime = (int)(Server()->TickSpeed()*GameServer()->Tuning()->m_GunLifetime);
			else
				Lifetime = (int)(Server()->TickSpeed()*GameServer()->TuningList()[m_TuneZone].m_GunLifetime);

			if (m_pPlayer->m_SpookyGhostActive && (m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun))
			{
				float a = GetAngle(Direction);
				a += (0.070f) * 2;

				new CPlasmaBullet
				(
					GameWorld(),
					m_pPlayer->GetCID(),	//owner
					ProjStartPos,			//pos
					Direction,				//dir
					0,						//freeze
					0,						//explosive
					0,						//unfreeze
					1,						//bloody
					1,						//ghost
					Team(),					//responibleteam
					6,						//lifetime
					1.0f,					//accel
					10.0f					//speed
				);
					
				new CPlasmaBullet
				(
					GameWorld(),
					m_pPlayer->GetCID(),						//owner
					ProjStartPos,								//pos
					vec2(cosf(a - 0.200f), sinf(a - 0.200f)),	//dir
					0,											//freeze
					0,											//explosive
					0,											//unfreeze
					1,											//bloody
					1,											//ghost
					Team(),										//responibleteam
					6,											//lifetime
					1.0f,										//accel
					10.0f										//speed
					);

				new CPlasmaBullet
				(
					GameWorld(),
					m_pPlayer->GetCID(),						//owner
					ProjStartPos,								//pos
					vec2(cosf(a - 0.040f), sinf(a - 0.040f)),	//dir
					0,											//freeze
					0,											//explosive
					0,											//unfreeze
					1,											//bloody
					1,											//ghost
					Team(),										//responibleteam
					6,											//lifetime
					1.0f,										//accel
					10.0f										//speed
				);

				GameServer()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
			}
			else if (m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun)
			{
				//idk if this is the right place to set some shooting speed but yolo
				//just copied the general code for all weapons and put it here
				if (!m_ReloadTimer)
				{
					float FireDelay;
					if (!m_TuneZone)
						GameServer()->Tuning()->Get(38 + m_Core.m_ActiveWeapon, &FireDelay);
					else
						GameServer()->TuningList()[m_TuneZone].Get(38 + m_Core.m_ActiveWeapon, &FireDelay);
					m_ReloadTimer = FireDelay * Server()->TickSpeed() / 5000;
				}

				//----- ChillerDragon tried to create 2nd projectile -----
				//Just copy and pasted the whole code agian
				float a = GetAngle(Direction);
				a += (0.070f) * 2;

				new CProjectile
					(
						GameWorld(),
						WEAPON_GUN,//Type
						m_pPlayer->GetCID(),//Owner
						ProjStartPos,//Pos
						vec2(cosf(a), sinf(a)),//Dir
						Lifetime,//Span
						0,//Freeze
						0,//Explosive
						0,//Force
						-1,//SoundImpact
						WEAPON_GUN//Weapon
						);

				new CProjectile
					(
						GameWorld(),
						WEAPON_GUN,//Type
						m_pPlayer->GetCID(),//Owner
						ProjStartPos,//Pos
						vec2(cosf(a - 0.070f), sinf(a - 0.070f)),//Dir
						Lifetime,//Span
						0,//Freeze
						0,//Explosive
						0,//Force
						-1,//SoundImpact
						WEAPON_GUN//Weapon
						);

				new CProjectile
					(
						GameWorld(),
						WEAPON_GUN,//Type
						m_pPlayer->GetCID(),//Owner
						ProjStartPos,//Pos
						vec2(cosf(a - 0.170f), sinf(a - 0.170f)),//Dir
						Lifetime,//Span
						0,//Freeze
						0,//Explosive
						0,//Force
						-1,//SoundImpact
						WEAPON_GUN//Weapon
						);

				CProjectile *pProj = new CProjectile
					(
						GameWorld(),
						WEAPON_GUN,//Type
						m_pPlayer->GetCID(),//Owner
						ProjStartPos,//Pos
						Direction,//Dir
						Lifetime,//Span
						0,//Freeze
						0,//Explosive
						0,//Force
						-1,//SoundImpact
						WEAPON_GUN//Weapon
						);

				// pack the Projectile and send it to the client Directly
				CNetObj_Projectile p;
				pProj->FillInfo(&p);

				CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
				Msg.AddInt(1);
				for (unsigned i = 0; i < sizeof(CNetObj_Projectile) / sizeof(int); i++)
					Msg.AddInt(((int *)&p)[i]);

				Server()->SendMsg(&Msg, 0, m_pPlayer->GetCID());
				GameServer()->CreateSound(m_Pos, SOUND_GUN_FIRE, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
			}
			else if (m_pPlayer->m_SpookyGhostActive)
			{
				new CPlasmaBullet
				(
					GameWorld(), 
					m_pPlayer->GetCID(),	//owner
					ProjStartPos,			//pos
					Direction,				//dir
					0,						//freeze
					0,						//explosive
					0,						//unfreeze
					1,						//bloody
					1,						//ghost
					Team(),					//responibleteam
					6,						//lifetime
					1.0f,					//accel
					10.0f					//speed
				);
				GameServer()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
			}
			else if (m_pPlayer->m_lasergun)
			{
				int RifleSpread = 1;
				float Spreading[] = { -0.070f, 0, 0.070f };
				for (int i = -RifleSpread; i <= RifleSpread; ++i)
				{
					float a = GetAngle(Direction);
					a += Spreading[i + 1];
					new CLaser(GameWorld(), m_Pos, vec2(cosf(a), sinf(a)), GameServer()->Tuning()->m_LaserReach, m_pPlayer->GetCID(), 0);
				}


				// summon meteor
				//CMeteor *pMeteor = new CMeteor(GameWorld(), ProjStartPos);
			}
			else
			{
				CProjectile *pProj = new CProjectile
				(
					GameWorld(),
					WEAPON_GUN,//Type
					m_pPlayer->GetCID(),//Owner
					ProjStartPos,//Pos
					Direction,//Dir
					Lifetime,//Span
					0,//Freeze
					0,//Explosive
					0,//Force
					-1,//SoundImpact
					WEAPON_GUN//Weapon
				);

				// pack the Projectile and send it to the client Directly
				CNetObj_Projectile p;
				pProj->FillInfo(&p);

				CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
				Msg.AddInt(1);
				for (unsigned i = 0; i < sizeof(CNetObj_Projectile) / sizeof(int); i++)
					Msg.AddInt(((int *)&p)[i]);

				Server()->SendMsg(&Msg, 0, m_pPlayer->GetCID());
				GameServer()->CreateSound(m_Pos, SOUND_GUN_FIRE, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
			}


		}

		if (m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
		{
			if (m_pPlayer->m_QuestStateLevel == 9) //race with conditions
			{
				if (g_Config.m_SvQuestRaceCondition == 1) //no gun (also jetpack)
				{
					GameServer()->QuestFailed2(m_pPlayer->GetCID());
				}
			}
		}

		//spooky ghost
		if (m_pPlayer->m_PlayerFlags&PLAYERFLAG_SCOREBOARD && m_pPlayer->m_SpookyGhost && m_Core.m_ActiveWeapon == WEAPON_GUN && m_CountSpookyGhostInputs)
		{
			m_TimesShot++;
			if ((m_TimesShot == 2) && !m_pPlayer->m_SpookyGhostActive)
			{
				SetSpookyGhost();
				m_TimesShot = 0;
			}
			else if ((m_TimesShot == 2) && m_pPlayer->m_SpookyGhostActive)
			{
				UnsetSpookyGhost();
				m_TimesShot = 0;
			}
			m_CountSpookyGhostInputs = false;
		}

	} break;

	case WEAPON_SHOTGUN:
	{

		if (m_freezeShotgun || m_pPlayer->m_IsVanillaWeapons) //freezeshotgun
		{
			int ShotSpread = 2;

			CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			Msg.AddInt(ShotSpread * 2 + 1);

			for (int i = -ShotSpread; i <= ShotSpread; ++i)
			{
				float Spreading[] = { -0.185f, -0.070f, 0, 0.070f, 0.185f };
				float a = GetAngle(Direction);
				a += Spreading[i + 2];
				float v = 1 - (absolute(i) / (float)ShotSpread);
				float Speed = mix((float)GameServer()->Tuning()->m_ShotgunSpeeddiff, 1.0f, v);
				CProjectile *pProj = new CProjectile(GameWorld(), WEAPON_SHOTGUN,
					m_pPlayer->GetCID(),
					ProjStartPos,
					vec2(cosf(a), sinf(a))*Speed,
					(int)(Server()->TickSpeed()*GameServer()->Tuning()->m_ShotgunLifetime),
					1, 0, 0, -1, WEAPON_SHOTGUN);

				// pack the Projectile and send it to the client Directly
				CNetObj_Projectile p;
				pProj->FillInfo(&p);

				for (unsigned i = 0; i < sizeof(CNetObj_Projectile) / sizeof(int); i++)
					Msg.AddInt(((int *)&p)[i]);
			}

			Server()->SendMsg(&Msg, 0, m_pPlayer->GetCID());

			GameServer()->CreateSound(m_Pos, SOUND_SHOTGUN_FIRE);
		}
		else //normal ddnet (no freezshotgun etc)
		{
			float LaserReach;
			if (!m_TuneZone)
				LaserReach = GameServer()->Tuning()->m_LaserReach;
			else
				LaserReach = GameServer()->TuningList()[m_TuneZone].m_LaserReach;

			new CLaser(&GameServer()->m_World, m_Pos, Direction, LaserReach, m_pPlayer->GetCID(), WEAPON_SHOTGUN);
			GameServer()->CreateSound(m_Pos, SOUND_SHOTGUN_FIRE, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
		}


		//race quest (shotgun)
		if (m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
		{
			if (m_pPlayer->m_QuestStateLevel == 9) //race with conditions
			{
				if (g_Config.m_SvQuestRaceCondition == 2) //no shotgun
				{
					GameServer()->QuestFailed2(m_pPlayer->GetCID());
				}
			}
		}
	} break;

	case WEAPON_GRENADE:
	{
		if (g_Config.m_SvInstagibMode || m_pPlayer->m_IsInstaMode_gdm)
		{
			m_pPlayer->m_GrenadeShots++;
			m_pPlayer->m_GrenadeShotsNoRJ++;
		}


		int Lifetime;
		if (!m_TuneZone)
			Lifetime = (int)(Server()->TickSpeed()*GameServer()->Tuning()->m_GrenadeLifetime);
		else
			Lifetime = (int)(Server()->TickSpeed()*GameServer()->TuningList()[m_TuneZone].m_GrenadeLifetime);

		if (m_HomingMissile)
		{
			/* CHomingMissile *pMissile = */ new CHomingMissile(GameWorld(), 100, m_pPlayer->GetCID(), 0, Direction);
		}
		else
		{
			CProjectile *pProj = new CProjectile
			(
				GameWorld(),
				WEAPON_GRENADE,//Type
				m_pPlayer->GetCID(),//Owner
				ProjStartPos,//Pos
				Direction,//Dir
				Lifetime,//Span
				0,//Freeze
				true,//Explosive
				0,//Force
				SOUND_GRENADE_EXPLODE,//SoundImpact
				WEAPON_GRENADE//Weapon
			);//SoundImpact

			  // pack the Projectile and send it to the client Directly
			CNetObj_Projectile p;
			pProj->FillInfo(&p);

			CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
			Msg.AddInt(1);
			for (unsigned i = 0; i < sizeof(CNetObj_Projectile) / sizeof(int); i++)
				Msg.AddInt(((int *)&p)[i]);
			Server()->SendMsg(&Msg, 0, m_pPlayer->GetCID());
		}

		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_FIRE, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));

		if (m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
		{
			if (m_pPlayer->m_QuestStateLevel == 9) //race with conditions
			{
				if (g_Config.m_SvQuestRaceCondition == 3) //no grenade
				{
					GameServer()->QuestFailed2(m_pPlayer->GetCID());
				}
			}
		}
	} break;
	case WEAPON_RIFLE:
	{
		if (g_Config.m_SvInstagibMode)
		{
			m_pPlayer->m_RifleShots++;
		}

		float LaserReach;
		if (!m_TuneZone)
			LaserReach = GameServer()->Tuning()->m_LaserReach;
		else
			LaserReach = GameServer()->TuningList()[m_TuneZone].m_LaserReach;

		new CLaser(GameWorld(), m_Pos, Direction, LaserReach, m_pPlayer->GetCID(), WEAPON_RIFLE);
		GameServer()->CreateSound(m_Pos, SOUND_RIFLE_FIRE, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));

		if (m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
		{
			if (m_pPlayer->m_QuestStateLevel == 9) //race with conditions
			{
				if (g_Config.m_SvQuestRaceCondition == 4) //no rifle
				{
					GameServer()->QuestFailed2(m_pPlayer->GetCID());
				}
			}
		}
	} break;

	case WEAPON_NINJA:
	{
		// reset Hit objects
		m_NumObjectsHit = 0;

		m_Ninja.m_ActivationDir = Direction;
		m_Ninja.m_CurrentMoveTime = g_pData->m_Weapons.m_Ninja.m_Movetime * Server()->TickSpeed() / 1000;
		m_Ninja.m_OldVelAmount = length(m_Core.m_Vel);

		GameServer()->CreateSound(m_Pos, SOUND_NINJA_FIRE, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
		if (m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
		{
			if (m_pPlayer->m_QuestStateLevel == 9) // race with conditions
			{
				if (g_Config.m_SvQuestRaceCondition == 5) // no ninja
				{
					GameServer()->QuestFailed2(m_pPlayer->GetCID());
				}
			}
		}
	} break;

	}

	if (m_pPlayer->m_QuestState == CPlayer::QUEST_BLOCK)
	{
		if (m_pPlayer->m_QuestStateLevel == 4) // Block 10 tees without fireing a weapon 
		{
			GameServer()->QuestFailed(m_pPlayer->GetCID());
		}
	}

	m_AttackTick = Server()->Tick();

	if (m_pPlayer->m_IsVanillaWeapons)
	{
		if (m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo > 0) // -1 == unlimited
			m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;
	}

	if (m_aDecreaseAmmo[m_Core.m_ActiveWeapon]) // picked up a dropped weapon without infinite bullets (-1)
	{
		m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;

		if (m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo == 0)
		{
			m_aDecreaseAmmo[m_Core.m_ActiveWeapon] = false;
			m_aWeapons[m_Core.m_ActiveWeapon].m_Got = false;
			SetWeaponThatChrHas();
		}
	}

	// shop window
	if ((m_ChangeShopPage) && (m_ShopWindowPage != -1) && (m_PurchaseState == 1))
	{
		ShopWindow(GetAimDir());
		m_ChangeShopPage = false;
	}

	//spawn weapons

	if (m_pPlayer->m_SpawnShotgunActive && m_Core.m_ActiveWeapon == WEAPON_SHOTGUN) 
	{
		m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;
		if (m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo == 0)
		{
			m_pPlayer->m_SpawnShotgunActive = 0;
			SetWeaponGot(WEAPON_SHOTGUN, false);
			SetWeaponThatChrHas();
		}
	}

	if (m_pPlayer->m_SpawnGrenadeActive && m_Core.m_ActiveWeapon == WEAPON_GRENADE)
	{
		m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;
		if (m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo == 0)
		{
			m_pPlayer->m_SpawnGrenadeActive = 0;
			SetWeaponGot(WEAPON_GRENADE, false);
			SetWeaponThatChrHas();
		}
	}

	if (m_pPlayer->m_SpawnRifleActive && m_Core.m_ActiveWeapon == WEAPON_RIFLE)
	{
		m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;
		if (m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo == 0)
		{
			m_pPlayer->m_SpawnRifleActive = 0;
			SetWeaponGot(WEAPON_RIFLE, false);
			SetWeaponThatChrHas();
		}
	}

	if (!m_ReloadTimer)
	{
		float FireDelay;
		if (!m_TuneZone)
			GameServer()->Tuning()->Get(38 + m_Core.m_ActiveWeapon, &FireDelay);
		else
			GameServer()->TuningList()[m_TuneZone].Get(38 + m_Core.m_ActiveWeapon, &FireDelay);
		m_ReloadTimer = FireDelay * Server()->TickSpeed() / 1000;
		if (m_OnFire)
		{
			m_OnFire = false;
			m_ReloadTimer = 200 * Server()->TickSpeed() / 1000;
		}
		//if (m_pPlayer->m_autospreadgun) //ddpp faster shooting
		//{
		//	m_ReloadTimer = FireDelay * Server()->TickSpeed() / 5000;
		//}
		//else
		//{
		//	m_ReloadTimer = FireDelay * Server()->TickSpeed() / 1000;
		//}
	}
}

void CCharacter::HandleWeapons()
{
	//ninja
	HandleNinja();
	HandleJetpack();

	if (m_PainSoundTimer > 0)
		m_PainSoundTimer--;

	// check reload timer
	if (m_ReloadTimer)
	{
		m_ReloadTimer--;
		return;
	}

	// fire Weapon, if wanted
	FireWeapon();

	if (m_pPlayer->m_IsVanillaWeapons && !m_FreezeTime)
	{
		// ammo regen
		int AmmoRegenTime = g_pData->m_Weapons.m_aId[m_Core.m_ActiveWeapon].m_Ammoregentime;
		if (AmmoRegenTime)
		{
			// If equipped and not active, regen ammo?
			if (m_ReloadTimer <= 0)
			{
				if (m_aWeapons[m_Core.m_ActiveWeapon].m_AmmoRegenStart < 0)
					m_aWeapons[m_Core.m_ActiveWeapon].m_AmmoRegenStart = Server()->Tick();

				if ((Server()->Tick() - m_aWeapons[m_Core.m_ActiveWeapon].m_AmmoRegenStart) >= AmmoRegenTime * Server()->TickSpeed() / 1000)
				{
					// Add some ammo
					m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo = min(m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo + 1, 10);
					m_aWeapons[m_Core.m_ActiveWeapon].m_AmmoRegenStart = -1;
				}
			}
			else
			{
				m_aWeapons[m_Core.m_ActiveWeapon].m_AmmoRegenStart = -1;
			}
		}
	}

	return;
}

bool CCharacter::GiveWeapon(int Weapon, int Ammo)
{
	if (m_aWeapons[Weapon].m_Ammo < g_pData->m_Weapons.m_aId[Weapon].m_Maxammo || !m_aWeapons[Weapon].m_Got)
	{
		m_aWeapons[Weapon].m_Got = true;
		m_aWeapons[Weapon].m_Ammo = Ammo;
		if (m_FreezeTime)	//dont remove this
			Freeze(0);		//dont remove this
			// m_aWeapons[Weapon].m_Ammo = min(g_pData->m_Weapons.m_aId[Weapon].m_Maxammo, Ammo); // commented out by chiller
		return true;
	}
	return false;
}

void CCharacter::GiveNinja()
{
	m_Ninja.m_ActivationTick = Server()->Tick();
	m_aWeapons[WEAPON_NINJA].m_Got = true;
	if (!m_FreezeTime)
		m_aWeapons[WEAPON_NINJA].m_Ammo = -1;
	if (m_Core.m_ActiveWeapon != WEAPON_NINJA)
		m_LastWeapon = m_Core.m_ActiveWeapon;
	m_Core.m_ActiveWeapon = WEAPON_NINJA;

	if (!m_aWeapons[WEAPON_NINJA].m_Got)
		GameServer()->CreateSound(m_Pos, SOUND_PICKUP_NINJA, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
}

void CCharacter::SetEmote(int Emote, int Tick)
{
	m_EmoteType = Emote;
	m_EmoteStop = Tick;
}

void CCharacter::OnPredictedInput(CNetObj_PlayerInput *pNewInput)
{
	// check for changes
	if (mem_comp(&m_Input, pNewInput, sizeof(CNetObj_PlayerInput)) != 0)
		m_LastAction = Server()->Tick();

	// copy new input
	mem_copy(&m_Input, pNewInput, sizeof(m_Input));
	m_NumInputs++;

	// it is not allowed to aim in the center
	if (m_Input.m_TargetX == 0 && m_Input.m_TargetY == 0)
		m_Input.m_TargetY = -1;
}

void CCharacter::OnDirectInput(CNetObj_PlayerInput *pNewInput)
{
	mem_copy(&m_LatestPrevInput, &m_LatestInput, sizeof(m_LatestInput));
	mem_copy(&m_LatestInput, pNewInput, sizeof(m_LatestInput));

	// it is not allowed to aim in the center
	if (m_LatestInput.m_TargetX == 0 && m_LatestInput.m_TargetY == 0)
		m_LatestInput.m_TargetY = -1;

	if (m_NumInputs > 2 && m_pPlayer->GetTeam() != TEAM_SPECTATORS)
	{
		HandleWeaponSwitch();
		FireWeapon();
	}

	mem_copy(&m_LatestPrevInput, &m_LatestInput, sizeof(m_LatestInput));
}

void CCharacter::ResetInput()
{
	m_Input.m_Direction = 0;
	//m_Input.m_Hook = 0;
	// simulate releasing the fire button
	if ((m_Input.m_Fire & 1) != 0)
		m_Input.m_Fire++;
	m_Input.m_Fire &= INPUT_STATE_MASK;
	m_Input.m_Jump = 0;
	m_LatestPrevInput = m_LatestInput = m_Input;
}

void CCharacter::Tick()
{

	/*if(m_pPlayer->m_ForceBalanced)
	{
	char Buf[128];
	str_format(Buf, sizeof(Buf), "You were moved to %s due to team balancing", GameServer()->m_pController->GetTeamName(m_pPlayer->GetTeam()));
	GameServer()->SendBroadcast(Buf, m_pPlayer->GetCID());

	m_pPlayer->m_ForceBalanced = false;
	}*/

	if (m_Paused)
		return;

	if (m_Atom || m_pPlayer->m_InfAtom)
	{
		if (m_AtomProjs.empty())
		{
			for (int i = 0; i<NUM_ATOMS; i++)
			{
				m_AtomProjs.push_back(new CStableProjectile(GameWorld(), i % 2 ? WEAPON_GRENADE : WEAPON_SHOTGUN));
			}
			m_AtomPosition = 0;
		}
		if (++m_AtomPosition >= 60)
		{
			m_AtomPosition = 0;
		}
		vec2 AtomPos;
		AtomPos.x = m_Pos.x + 200 * cos(m_AtomPosition*M_PI * 2 / 60);
		AtomPos.y = m_Pos.y + 80 * sin(m_AtomPosition*M_PI * 2 / 60);
		for (int i = 0; i<NUM_ATOMS; i++)
		{
			m_AtomProjs[i]->m_Pos = rotate_around_point(AtomPos, m_Pos, i*M_PI * 2 / NUM_ATOMS);
		}
	}
	else if (!m_AtomProjs.empty())
	{
		for (std::vector<CStableProjectile *>::iterator it = m_AtomProjs.begin(); it != m_AtomProjs.end(); ++it)
		{
			GameServer()->m_World.DestroyEntity(*it);
		}
		m_AtomProjs.clear();
	}

	if (m_Trail || m_pPlayer->m_InfTrail)
	{
		if (m_TrailProjs.empty())
		{
			for (int i = 0; i<NUM_TRAILS; i++)
			{
				m_TrailProjs.push_back(new CStableProjectile(GameWorld(), WEAPON_SHOTGUN));
			}
			m_TrailHistory.clear();
			m_TrailHistory.push_front(HistoryPoint(m_Pos, 0.0f));
			m_TrailHistory.push_front(HistoryPoint(m_Pos, NUM_TRAILS*TRAIL_DIST));
			m_TrailHistoryLength = NUM_TRAILS*TRAIL_DIST;
		}
		vec2 FrontPos = m_TrailHistory.front().m_Pos;
		if (FrontPos != m_Pos)
		{
			float FrontLength = distance(m_Pos, FrontPos);
			m_TrailHistory.push_front(HistoryPoint(m_Pos, FrontLength));
			m_TrailHistoryLength += FrontLength;
		}

		while (1)
		{
			float LastDist = m_TrailHistory.back().m_Dist;
			if (m_TrailHistoryLength - LastDist >= NUM_TRAILS*TRAIL_DIST)
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
		for (int i = 0; i<NUM_TRAILS; i++)
		{
			float Length = (i + 1)*TRAIL_DIST;
			float NextDist = 0.0f;
			while (1)
			{
				// in case floating point arithmetic errors should fuck us up
				// don't crash and recalculate total history length
				if ((unsigned int)HistoryPos >= m_TrailHistory.size())
				{
					m_TrailHistoryLength = 0.0f;
					for (std::deque<HistoryPoint>::iterator it = m_TrailHistory.begin(); it != m_TrailHistory.end(); ++it)
					{
						m_TrailHistoryLength += it->m_Dist;
					}
					break;
				}
				NextDist = m_TrailHistory[HistoryPos].m_Dist;

				if (Length <= HistoryPosLength + NextDist)
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
			m_TrailProjs[i]->m_Pos += (m_TrailHistory[HistoryPos + 1].m_Pos - m_TrailProjs[i]->m_Pos)*(AdditionalLength / NextDist);
		}
	}
	else if (!m_TrailProjs.empty())
	{
		for (std::vector<CStableProjectile *>::iterator it = m_TrailProjs.begin(); it != m_TrailProjs.end(); ++it)
		{
			GameServer()->m_World.DestroyEntity(*it);
		}
		m_TrailProjs.clear();
	}

	DummyTick();
	DDPP_Tick();
	DDRaceTick();

	m_Core.m_Input = m_Input;

	int carry1 = 1; int carry2 = 1;
	if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0])
	{
		if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0]->m_pCarryingCharacter == NULL)
		{
			carry1 = 0;
		}

		m_Core.setFlagPos(0, ((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0]->m_Pos, ((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0]->m_AtStand, ((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0]->m_Vel, carry1);
	}
	if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1])
	{
		if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1]->m_pCarryingCharacter == NULL)
		{
			carry2 = 0;
		}

		m_Core.setFlagPos(1, ((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1]->m_Pos, ((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1]->m_AtStand, ((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1]->m_Vel, carry2);
	}
	//if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0] != NULL) { if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0]->m_pCarryingCharacter == NULL) { carry1 = 0; } }
	//if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1] != NULL) { if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1]->m_pCarryingCharacter == NULL) { carry2 = 0; } }

	//m_Core.setFlagPos(((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0]->m_Pos, ((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1]->m_Pos, ((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0]->m_AtStand, ((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1]->m_AtStand, ((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0]->m_Vel, ((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1]->m_Vel, carry1, carry2);

	m_Core.Tick(true, false);
	if (m_Core.m_updateFlagVel == 98) {
		((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0]->m_Vel = m_Core.m_UFlagVel;
	}
	else if (m_Core.m_updateFlagVel == 99) {
		((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1]->m_Vel = m_Core.m_UFlagVel;
	}

	/*// handle death-tiles and leaving gamelayer
	if(GameServer()->Collision()->GetCollisionAt(m_Pos.x+m_ProximityRadius/3.f, m_Pos.y-m_ProximityRadius/3.f)&CCollision::COLFLAG_DEATH ||
	GameServer()->Collision()->GetCollisionAt(m_Pos.x+m_ProximityRadius/3.f, m_Pos.y+m_ProximityRadius/3.f)&CCollision::COLFLAG_DEATH ||
	GameServer()->Collision()->GetCollisionAt(m_Pos.x-m_ProximityRadius/3.f, m_Pos.y-m_ProximityRadius/3.f)&CCollision::COLFLAG_DEATH ||
	GameServer()->Collision()->GetCollisionAt(m_Pos.x-m_ProximityRadius/3.f, m_Pos.y+m_ProximityRadius/3.f)&CCollision::COLFLAG_DEATH ||
	GameLayerClipped(m_Pos))
	{
	Die(m_pPlayer->GetCID(), WEAPON_WORLD);
	}*/

	// handle Weapons
	HandleWeapons();

	DDRacePostCoreTick();

	// Previnput
	m_PrevInput = m_Input;

	m_PrevPos = m_Core.m_Pos;

	if (m_Core.m_LastHookedPlayer != m_OldLastHookedPlayer)
	{
		m_LastHitWeapon = -1;
	}
	m_OldLastHookedPlayer = m_Core.m_LastHookedPlayer;


	if (m_ShopMotdTick < Server()->Tick())
	{
		m_ShopWindowPage = -1;
		m_PurchaseState = 0;
	}	


	return;
}

void CCharacter::TickDefered()
{
	// advance the dummy
	{
		CWorldCore TempWorld;
		m_ReckoningCore.Init(&TempWorld, GameServer()->Collision(), &((CGameControllerDDRace*)GameServer()->m_pController)->m_Teams.m_Core, &((CGameControllerDDRace*)GameServer()->m_pController)->m_TeleOuts);
		m_ReckoningCore.m_Id = m_pPlayer->GetCID();
		m_ReckoningCore.Tick(false, false);
		m_ReckoningCore.Move();
		m_ReckoningCore.Quantize();
	}

	//lastsentcore
	vec2 StartPos = m_Core.m_Pos;
	vec2 StartVel = m_Core.m_Vel;
	bool StuckBefore = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));

	m_Core.m_Id = m_pPlayer->GetCID();
	m_Core.Move();
	bool StuckAfterMove = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));
	m_Core.Quantize();
	bool StuckAfterQuant = GameServer()->Collision()->TestBox(m_Core.m_Pos, vec2(28.0f, 28.0f));
	m_Pos = m_Core.m_Pos;

	if (!StuckBefore && (StuckAfterMove || StuckAfterQuant))
	{
		// Hackish solution to get rid of strict-aliasing warning
		union
		{
			float f;
			unsigned u;
		}StartPosX, StartPosY, StartVelX, StartVelY;

		StartPosX.f = StartPos.x;
		StartPosY.f = StartPos.y;
		StartVelX.f = StartVel.x;
		StartVelY.f = StartVel.y;

		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "STUCK!!! %d %d %d %f %f %f %f %x %x %x %x",
			StuckBefore,
			StuckAfterMove,
			StuckAfterQuant,
			StartPos.x, StartPos.y,
			StartVel.x, StartVel.y,
			StartPosX.u, StartPosY.u,
			StartVelX.u, StartVelY.u);
		GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
	}

	int Events = m_Core.m_TriggeredEvents;
	//int Mask = CmaskAllExceptOne(m_pPlayer->GetCID());

	if (Events&COREEVENT_GROUND_JUMP) GameServer()->CreateSound(m_Pos, SOUND_PLAYER_JUMP, Teams()->TeamMask(Team(), m_pPlayer->GetCID()));

	if (Events&COREEVENT_HOOK_ATTACH_PLAYER) GameServer()->CreateSound(m_Pos, SOUND_HOOK_ATTACH_PLAYER, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
	if (Events&COREEVENT_HOOK_ATTACH_GROUND) GameServer()->CreateSound(m_Pos, SOUND_HOOK_ATTACH_GROUND, Teams()->TeamMask(Team(), m_pPlayer->GetCID(), m_pPlayer->GetCID()));
	if (Events&COREEVENT_HOOK_HIT_NOHOOK) GameServer()->CreateSound(m_Pos, SOUND_HOOK_NOATTACH, Teams()->TeamMask(Team(), m_pPlayer->GetCID(), m_pPlayer->GetCID()));


	if (m_pPlayer->GetTeam() == TEAM_SPECTATORS)
	{
		m_Pos.x = m_Input.m_TargetX;
		m_Pos.y = m_Input.m_TargetY;
	}

	// update the m_SendCore if needed
	{
		CNetObj_Character Predicted;
		CNetObj_Character Current;
		mem_zero(&Predicted, sizeof(Predicted));
		mem_zero(&Current, sizeof(Current));
		m_ReckoningCore.Write(&Predicted);
		m_Core.Write(&Current);

		// only allow dead reackoning for a top of 3 seconds
		if (m_Core.m_pReset || m_ReckoningTick + Server()->TickSpeed() * 3 < Server()->Tick() || mem_comp(&Predicted, &Current, sizeof(CNetObj_Character)) != 0)
		{
			m_ReckoningTick = Server()->Tick();
			m_SendCore = m_Core;
			m_ReckoningCore = m_Core;
			m_Core.m_pReset = false;
		}
	}
}

void CCharacter::TickPaused()
{
	++m_AttackTick;
	++m_DamageTakenTick;
	++m_Ninja.m_ActivationTick;
	++m_ReckoningTick;
	if (m_LastAction != -1)
		++m_LastAction;
	if (m_aWeapons[m_Core.m_ActiveWeapon].m_AmmoRegenStart > -1)
		++m_aWeapons[m_Core.m_ActiveWeapon].m_AmmoRegenStart;
	if (m_EmoteStop > -1)
		++m_EmoteStop;
}

bool CCharacter::IncreaseHealth(int Amount)
{
	if (m_Health >= 10)
		return false;
	m_Health = clamp(m_Health + Amount, 0, 10);
	return true;
}

bool CCharacter::IncreaseArmor(int Amount)
{
	if (m_Armor >= 10)
		return false;
	m_Armor = clamp(m_Armor + Amount, 0, 10);
	return true;
}

void CCharacter::Die(int Killer, int Weapon, bool fngscore)
{
#if defined(CONF_DEBUG)
	dbg_msg("debug", "character die ID: %d Name: %s", m_pPlayer->GetCID(), Server()->ClientName(m_pPlayer->GetCID()));
#endif
	char aBuf[256];
	ClearFakeMotd();
	Killer = DDPP_DIE(Killer, Weapon, fngscore);

	if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0])
	{
		if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0]->m_pCarryingCharacter == this) {

			m_pPlayer->m_HadFlagOnDeath = true;

			if (m_Core.m_LastHookedPlayer != -1) {
				((CGameControllerDDRace*)GameServer()->m_pController)->ChangeFlagOwner(0, Killer);
			}

		}
	}

	if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1])
	{
		if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1]->m_pCarryingCharacter == this) {

			m_pPlayer->m_HadFlagOnDeath = true;

			if (m_Core.m_LastHookedPlayer != -1) {
				((CGameControllerDDRace*)GameServer()->m_pController)->ChangeFlagOwner(1, Killer);
			}

		}
	}

	if (Server()->IsRecording(m_pPlayer->GetCID()))
		Server()->StopRecord(m_pPlayer->GetCID());

	// to have invis motd updates OR to have the spooky ghost skin in the kill msg 
	// (because it was too fast otherwise and the normal skin would be there if its a selfkill and not a death tile kill)
	if ((!m_pPlayer->m_ShowName && m_pPlayer->m_SpookyGhostActive) || m_pPlayer->m_CanClearFakeMotd)
	{
		m_pPlayer->m_RespawnTick = Server()->Tick() + Server()->TickSpeed() / 10;

		m_pPlayer->m_CanClearFakeMotd = false;
	}

	//m_pPlayer->m_RespawnTick = Server()->Tick();

	int ModeSpecial = GameServer()->m_pController->OnCharacterDeath(this, GameServer()->m_apPlayers[Killer], Weapon);

	str_format(aBuf, sizeof(aBuf), "kill killer='%d:%s' victim='%d:%s' weapon=%d special=%d",
		Killer, Server()->ClientName(Killer),
		m_pPlayer->GetCID(), Server()->ClientName(m_pPlayer->GetCID()), Weapon, ModeSpecial);
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

	if (Killer < 0 || Killer == m_pPlayer->GetCID())
	{
		m_LastHitWeapon = -1;
		Weapon = -1;
	}

	// send the kill message
	if (GameServer()->m_apPlayers[Killer] &&
		(!m_pPlayer->m_ShowName || !GameServer()->m_apPlayers[Killer]->m_ShowName))
	{
		if (!GameServer()->m_apPlayers[Killer]->m_ShowName)
			GameServer()->m_apPlayers[Killer]->FixForNoName(0);	// just for the name to appear because otherwise there would be no name in the kill msg

		m_pPlayer->m_MsgKiller = Killer;
		if (!m_pPlayer->m_IsSurvivaling && !m_pPlayer->IsInstagibMinigame())
			m_pPlayer->m_MsgWeapon = m_LastHitWeapon;
		else
			m_pPlayer->m_MsgWeapon = Weapon;
		m_pPlayer->m_MsgModeSpecial = ModeSpecial;
		m_pPlayer->FixForNoName(2);
	}
	else
	{
		CNetMsg_Sv_KillMsg Msg;
		Msg.m_Killer = Killer;
		Msg.m_Victim = m_pPlayer->GetCID();
		if (!m_pPlayer->m_IsSurvivaling && !m_pPlayer->IsInstagibMinigame())
			Msg.m_Weapon = m_LastHitWeapon;
		else
			Msg.m_Weapon = Weapon;
		Msg.m_ModeSpecial = ModeSpecial;
		Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, -1);
	}

	// a nice sound
	GameServer()->CreateSound(m_Pos, SOUND_PLAYER_DIE, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));

	// this is for auto respawn after 3 secs
	m_pPlayer->m_DieTick = Server()->Tick();

	m_Alive = false;
	GameServer()->m_World.RemoveEntity(this);
	GameServer()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = 0;
	GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCID(), Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
	Teams()->OnCharacterDeath(GetPlayer()->GetCID(), Weapon);
}

bool CCharacter::TakeDamage(vec2 Force, int Dmg, int From, int Weapon)
{
	//Block points check for touchers (weapons)
	if ((Weapon == WEAPON_GRENADE || Weapon == WEAPON_HAMMER || Weapon == WEAPON_SHOTGUN || Weapon == WEAPON_RIFLE) && GameServer()->m_apPlayers[From])
	{
		if (From != m_pPlayer->GetCID())
		{
			m_pPlayer->UpdateLastToucher(From);
			m_LastHitWeapon = Weapon;
		}
	}

	////dragon test [FNN] isTouched check
	if (m_pPlayer->m_IsDummy && m_pPlayer->m_DummyMode == 25 && m_Dummy_nn_ready && From != m_pPlayer->GetCID())
	{
		if ((Weapon == WEAPON_GRENADE || Weapon == WEAPON_HAMMER || Weapon == WEAPON_SHOTGUN || Weapon == WEAPON_RIFLE) && GameServer()->m_apPlayers[From])
		{
			m_Dummy_nn_touched_by_humans = true;
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "[FNN] please stop shooting me %s", Server()->ClientName(From));
			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, aBuf);
		}

		//return false; //removes hammer knockback
	}


	//zCatch ChillerDragon
	if (g_Config.m_SvInstagibMode || (m_pPlayer->m_IsInstaMode_gdm && GameServer()->m_apPlayers[From]->m_IsInstaMode_gdm) || (m_pPlayer->m_IsInstaMode_idm && GameServer()->m_apPlayers[From]->m_IsInstaMode_idm)) //in (all instagib modes) or (both players in gdm/idm mode) --->  1hit
	{
		DDPP_TakeDamageInstagib(Dmg, From, Weapon);
	}
	else
	{
		if ((m_isDmg || m_pPlayer->m_IsVanillaDmg) /*&& !m_pPlayer->m_IsSurvivalLobby*/)
		{
			//m_Core.m_Vel += Force;

			//  dragon      if(GameServer()->m_pController->IsFriendlyFire(m_pPlayer->GetCID(), From) && !g_Config.m_SvTeamdamage)
			//	dragon      return false;

			// m_pPlayer only inflicts half damage on self

			if (From == m_pPlayer->GetCID())
			{
				Dmg = max(1, Dmg / 2);

				if (m_pPlayer->m_IsVanillaCompetetive && Weapon == WEAPON_RIFLE)
				{
					return false; //no rifle self damage in competetive vanilla games (for example survival)
				}
			}

			m_DamageTaken++;

			// create healthmod indicator
			if (Server()->Tick() < m_DamageTakenTick + 25)
			{
				// make sure that the damage indicators doesn't group together
				GameServer()->CreateDamageInd(m_Pos, m_DamageTaken*0.25f, Dmg);
			}
			else
			{
				m_DamageTaken = 0;
				GameServer()->CreateDamageInd(m_Pos, 0, Dmg);
			}

			if (Dmg)
			{
				if (m_Armor)
				{
					if (Dmg > 1)
					{
						m_Health--;
						Dmg--;
					}

					if (Dmg > m_Armor)
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
			if (From >= 0 && From != m_pPlayer->GetCID() && GameServer()->m_apPlayers[From])
			{
				int64_t Mask = CmaskOne(From);
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS && GameServer()->m_apPlayers[i]->m_SpectatorID == From)
						Mask |= CmaskOne(i);
				}
				GameServer()->CreateSound(GameServer()->m_apPlayers[From]->m_ViewPos, SOUND_HIT, Mask);
			}

			// check for death
			if (m_Health <= 0)
			{
				Die(From, Weapon);

				// set attacker's face to happy (taunt!)
				if (From >= 0 && From != m_pPlayer->GetCID() && GameServer()->m_apPlayers[From])
				{
					CCharacter *pChr = GameServer()->m_apPlayers[From]->GetCharacter();
					if (pChr)
					{
						pChr->m_EmoteType = EMOTE_HAPPY;
						pChr->m_EmoteStop = Server()->Tick() + Server()->TickSpeed();
					}
				}

				return false;
			}

			if (Dmg > 2)
				GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_LONG);
			else
				GameServer()->CreateSound(m_Pos, SOUND_PLAYER_PAIN_SHORT);

			if (!m_Jetpack)
			{
				m_EmoteType = EMOTE_PAIN;
				m_EmoteStop = Server()->Tick() + 500 * Server()->TickSpeed() / 1000;
			}

		}
		else if ((From != -1) && GameServer()->m_apPlayers[From] && GameServer()->m_apPlayers[From]->m_SpookyGhostActive)
		{
			// dont do emote pain if the shooter has spooky ghost and shoot plasma projectile
		}
		else //normal ddnet code (else to IsDmg)
		{


			if (!m_Jetpack || m_Core.m_ActiveWeapon != WEAPON_GUN)
			{
				m_EmoteType = EMOTE_PAIN;
				m_EmoteStop = Server()->Tick() + 500 * Server()->TickSpeed() / 1000;
			}
		}
	}

	//force external af because we always want some nice moving tees after taking damage
	vec2 Temp = m_Core.m_Vel + Force;
	if (Temp.x > 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_270) || (m_TileIndexL == TILE_STOP && m_TileFlagsL == ROTATION_270) || (m_TileIndexL == TILE_STOPS && (m_TileFlagsL == ROTATION_90 || m_TileFlagsL == ROTATION_270)) || (m_TileIndexL == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_270) || (m_TileFIndexL == TILE_STOP && m_TileFFlagsL == ROTATION_270) || (m_TileFIndexL == TILE_STOPS && (m_TileFFlagsL == ROTATION_90 || m_TileFFlagsL == ROTATION_270)) || (m_TileFIndexL == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_270) || (m_TileSIndexL == TILE_STOP && m_TileSFlagsL == ROTATION_270) || (m_TileSIndexL == TILE_STOPS && (m_TileSFlagsL == ROTATION_90 || m_TileSFlagsL == ROTATION_270)) || (m_TileSIndexL == TILE_STOPA)))
		Temp.x = 0;
	if (Temp.x < 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_90) || (m_TileIndexR == TILE_STOP && m_TileFlagsR == ROTATION_90) || (m_TileIndexR == TILE_STOPS && (m_TileFlagsR == ROTATION_90 || m_TileFlagsR == ROTATION_270)) || (m_TileIndexR == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_90) || (m_TileFIndexR == TILE_STOP && m_TileFFlagsR == ROTATION_90) || (m_TileFIndexR == TILE_STOPS && (m_TileFFlagsR == ROTATION_90 || m_TileFFlagsR == ROTATION_270)) || (m_TileFIndexR == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_90) || (m_TileSIndexR == TILE_STOP && m_TileSFlagsR == ROTATION_90) || (m_TileSIndexR == TILE_STOPS && (m_TileSFlagsR == ROTATION_90 || m_TileSFlagsR == ROTATION_270)) || (m_TileSIndexR == TILE_STOPA)))
		Temp.x = 0;
	if (Temp.y < 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_180) || (m_TileIndexB == TILE_STOP && m_TileFlagsB == ROTATION_180) || (m_TileIndexB == TILE_STOPS && (m_TileFlagsB == ROTATION_0 || m_TileFlagsB == ROTATION_180)) || (m_TileIndexB == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_180) || (m_TileFIndexB == TILE_STOP && m_TileFFlagsB == ROTATION_180) || (m_TileFIndexB == TILE_STOPS && (m_TileFFlagsB == ROTATION_0 || m_TileFFlagsB == ROTATION_180)) || (m_TileFIndexB == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_180) || (m_TileSIndexB == TILE_STOP && m_TileSFlagsB == ROTATION_180) || (m_TileSIndexB == TILE_STOPS && (m_TileSFlagsB == ROTATION_0 || m_TileSFlagsB == ROTATION_180)) || (m_TileSIndexB == TILE_STOPA)))
		Temp.y = 0;
	if (Temp.y > 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_0) || (m_TileIndexT == TILE_STOP && m_TileFlagsT == ROTATION_0) || (m_TileIndexT == TILE_STOPS && (m_TileFlagsT == ROTATION_0 || m_TileFlagsT == ROTATION_180)) || (m_TileIndexT == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_0) || (m_TileFIndexT == TILE_STOP && m_TileFFlagsT == ROTATION_0) || (m_TileFIndexT == TILE_STOPS && (m_TileFFlagsT == ROTATION_0 || m_TileFFlagsT == ROTATION_180)) || (m_TileFIndexT == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_0) || (m_TileSIndexT == TILE_STOP && m_TileSFlagsT == ROTATION_0) || (m_TileSIndexT == TILE_STOPS && (m_TileSFlagsT == ROTATION_0 || m_TileSFlagsT == ROTATION_180)) || (m_TileSIndexT == TILE_STOPA)))
		Temp.y = 0;
	m_Core.m_Vel = Temp;

	return true;
}

void CCharacter::DDPP_TakeDamageInstagib(int Dmg, int From, int Weapon)
{
	if (m_Godmode || (m_pPlayer->m_IsInstaArena_gdm && GameServer()->m_InstaGrenadeRoundEndTickTicker) || (m_pPlayer->m_IsInstaArena_idm && GameServer()->m_InstaRifleRoundEndTickTicker))
	{
		//CHEATER!!
	}
	else
	{
		if (From == m_pPlayer->GetCID())
		{
			m_pPlayer->m_GrenadeShotsNoRJ--; //warning also reduce NoRJ shots on close kills
		}

		if (From != m_pPlayer->GetCID() && Dmg >= g_Config.m_SvNeededDamage2NadeKill)
		{
			if (m_pPlayer->m_IsInstaMode_fng || GameServer()->m_apPlayers[From]->m_IsInstaMode_fng)
			{
				if (!m_FreezeTime)
				{
					//char aBuf[256];
					//str_format(aBuf, sizeof(aBuf), "freezetime %d", m_FreezeTime);
					//GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
					Freeze(10);
					// on fire mode
					if (g_Config.m_SvOnFireMode == 1)
					{
						if (GameServer()->m_apPlayers[From] && GameServer()->m_apPlayers[From]->GetCharacter())
						{
							GameServer()->m_apPlayers[From]->GetCharacter()->m_ReloadTimer = 200 * Server()->TickSpeed() / 1000;
						}
					}
				}
				else
				{
					//GameServer()->SendChat(-1, CGameContext::CHAT_ALL, "returned cuz freeze time");
					//return false; //dont count freezed tee shots (no score or sound or happy emote)
					//dont return because we loose hammer vel then
					return; //we can return agian because the instagib stuff has his own func and got moved out of TakeDamage();
				}
			}
			else
			{
				Die(From, Weapon);
			}


			//do scoring (by ChillerDragon)
			if (g_Config.m_SvInstagibMode || g_Config.m_SvDDPPscore == 0)
			{
				GameServer()->m_apPlayers[From]->m_Score++;
			}
			GameServer()->DoInstaScore(1, From);


			//save the kill
			//if (!m_pPlayer->m_IsInstaArena_fng) //damage is only a hit not a kill in insta ---> well move it complety al to kill makes more performance sense
			//{
			//	if (g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2 || GameServer()->m_apPlayers[From]->m_IsInstaArena_gdm) //gdm & zCatch grenade
			//	{
			//		GameServer()->m_apPlayers[From]->m_GrenadeKills++;
			//	}
			//	else if (g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4 || GameServer()->m_apPlayers[From]->m_IsInstaArena_idm) // idm & zCatch rifle
			//	{
			//		GameServer()->m_apPlayers[From]->m_RifleKills++;
			//	}
			//}


			//killingspree system by toast stolen from twf (shit af xd(has crashbug too if a killingspreeeer gets killed))
			//GameServer()->m_apPlayers[From]->m_KillStreak++;
			//char aBuf[256];
			//str_format(aBuf, sizeof(aBuf), "%s's Killingspree was ended by %s (%d Kills)", Server()->ClientName(m_pPlayer->GetCID()), Server()->ClientName(GameServer()->m_apPlayers[From]->GetCID()), m_pPlayer->m_KillStreak);
			//if (m_pPlayer->m_KillStreak >= 5)
			//{
			//	GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			//	GameServer()->CreateExplosion(m_pPlayer->GetCharacter()->m_Pos, m_pPlayer->GetCID(), WEAPON_GRENADE, false, 0, m_pPlayer->GetCharacter()->Teams()->TeamMask(0));
			//}
			//m_pPlayer->m_KillStreak = 0;
			//char m_SpreeMsg[10][100] = { "on a killing spree", "on a rampage", "dominating", "unstoppable", "godlike", "prolike", "cheating", "the master","the best","imba" };
			//int iBuf = ((GameServer()->m_apPlayers[From]->m_KillStreak / 5) - 1) % 10;
			//str_format(aBuf, sizeof(aBuf), "%s is %s with %d Kills!", Server()->ClientName(GameServer()->m_apPlayers[From]->GetCID()), m_SpreeMsg[iBuf], GameServer()->m_apPlayers[From]->m_KillStreak);
			//if (m_pPlayer->m_KillStreak % 5 == 0 && GameServer()->m_apPlayers[From]->m_KillStreak >= 5)
			//	GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);

			// set attacker's face to happy (taunt!)
			if (From >= 0 && From != m_pPlayer->GetCID() && GameServer()->m_apPlayers[From])
			{
				CCharacter *pChr = GameServer()->m_apPlayers[From]->GetCharacter();
				if (pChr)
				{
					pChr->m_EmoteType = EMOTE_HAPPY;
					pChr->m_EmoteStop = Server()->Tick() + Server()->TickSpeed();
				}
			}


			// do damage Hit sound
			if (From >= 0 && From != m_pPlayer->GetCID() && GameServer()->m_apPlayers[From])
			{
				int64_t Mask = CmaskOne(From);
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS && GameServer()->m_apPlayers[i]->m_SpectatorID == From)
						Mask |= CmaskOne(i);
				}
				GameServer()->CreateSound(GameServer()->m_apPlayers[From]->m_ViewPos, SOUND_HIT, Mask);
			}

			//if zCatch mode --> move to spec
			if (g_Config.m_SvInstagibMode == 2 || g_Config.m_SvInstagibMode == 4) //grenade and rifle zCatch
			{
				if (From != m_pPlayer->GetCID())
				{
					m_pPlayer->SetTeam(-1, 0);
				}

				//Save The Player in catch array
				GameServer()->m_apPlayers[From]->m_aCatchedID[m_pPlayer->GetCID()] = 1;
			}
		}
	}
}

void CCharacter::Snap(int SnappingClient)
{

	int id = m_pPlayer->GetCID();

	if (SnappingClient > -1 && !Server()->Translate(id, SnappingClient))
		return;

	if (NetworkClipped(SnappingClient))
		return;

	if (SnappingClient > -1)
	{
		CCharacter* SnapChar = GameServer()->GetPlayerChar(SnappingClient);
		CPlayer* SnapPlayer = GameServer()->m_apPlayers[SnappingClient];

		if ((SnapPlayer->GetTeam() == TEAM_SPECTATORS || SnapPlayer->m_Paused) && SnapPlayer->m_SpectatorID != -1
			&& !CanCollide(SnapPlayer->m_SpectatorID) && !SnapPlayer->m_ShowOthers)
			return;

		if (SnapPlayer->GetTeam() != TEAM_SPECTATORS && !SnapPlayer->m_Paused && SnapChar && !SnapChar->m_Super
			&& !CanCollide(SnappingClient) && !SnapPlayer->m_ShowOthers)
			return;

		if ((SnapPlayer->GetTeam() == TEAM_SPECTATORS || SnapPlayer->m_Paused) && SnapPlayer->m_SpectatorID == -1
			&& !CanCollide(SnappingClient) && SnapPlayer->m_SpecTeam)
			return;
	}

	if (m_Paused)
		return;

	CNetObj_Character *pCharacter = static_cast<CNetObj_Character *>(Server()->SnapNewItem(NETOBJTYPE_CHARACTER, id, sizeof(CNetObj_Character)));
	if (!pCharacter)
		return;

	// da oben sind ja die ganzen abfragen, ob der spieler sichtbar ist, ob er richtig erstellt werden konnte, 
	// ob das game nicht pausiert ist und so.
	// wenn du das jetzt oben hinschreibst dann passiert das vor den abfragen
	// kann evtl. zu einem crash oder ähnlichem führen
	if (m_WaveBloody)
	{
		if (m_WaveBloodyStrength < 1 || Server()->Tick() % m_WaveBloodyStrength == 0)
		{
			GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCID());
			if (m_WaveBloodyStrength < -5)
			{
				for (int i = 0; i < 3; i++) //strong bloody
				{
					GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCID()); //hier wird der effekt erstellt.
				}
			}
		}

		if (Server()->Tick() % 11 == 0) // wave speed
		{
			if (m_WaveBloodyGrow)
			{
				m_WaveBloodyStrength++;
			}
			else
			{
				m_WaveBloodyStrength--;
			}
		}

		if (m_WaveBloodyStrength > 12)
		{
			m_WaveBloodyGrow = false;
		}
		else if (m_WaveBloodyStrength < -10)
		{
			m_WaveBloodyGrow = true;
		}
	}

	if (m_Bloody || GameServer()->IsHooked(m_pPlayer->GetCID(), 2) ||m_pPlayer->m_InfBloody) //wenn bloody aktiviert ist
	{
		if (Server()->Tick() % 3 == 0) //low bloody
		{
			GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCID()); //hier wird der effekt erstellt.
		}
	}

	if (m_StrongBloody) // wenn strong bloody aktiviert ist
	{
		for (int i = 0; i < 3; i++) //strong bloody
		{
			GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCID()); //hier wird der effekt erstellt.
		}
	}

	if (m_pPlayer->m_ninjasteam || m_ninjasteam)
	{
		for (int i = 0; i < 3; i++) //hier wird eine schleife erstellt, damit sich der effekt wiederholt
		{
			GameServer()->CreatePlayerSpawn(m_Pos); //hier wird der spawn effekt erstellt
		}
	}

	if (m_isHeal)
	{
		GameServer()->CreateDeath(m_Pos, m_pPlayer->GetCID()); //hier wird der tee zerplatzt xD effekt erstellt.
		m_isHeal = false;
	}


	// write down the m_Core
	if (!m_ReckoningTick || GameServer()->m_World.m_Paused)
	{
		// no dead reckoning when paused because the client doesn't know
		// how far to perform the reckoning
		pCharacter->m_Tick = 0;
		m_Core.Write(pCharacter);
	}
	else
	{
		pCharacter->m_Tick = m_ReckoningTick;
		m_SendCore.Write(pCharacter);
	}

	// set emote
	if (m_EmoteStop < Server()->Tick())
	{
		m_EmoteType = m_pPlayer->m_DefEmote;
		m_EmoteStop = -1;
	}
	pCharacter->m_Emote = m_EmoteType;

	if (pCharacter->m_HookedPlayer != -1)
	{
		if (!Server()->Translate(pCharacter->m_HookedPlayer, SnappingClient))
			pCharacter->m_HookedPlayer = -1;
	}

	pCharacter->m_AttackTick = m_AttackTick;
	pCharacter->m_Direction = m_Input.m_Direction;
	pCharacter->m_Weapon = m_Core.m_ActiveWeapon;
	pCharacter->m_AmmoCount = 0;
	pCharacter->m_Health = 0;
	pCharacter->m_Armor = 0;

	// change eyes and use ninja graphic if player is freeze
	if (m_DeepFreeze)
	{
		if (pCharacter->m_Emote == EMOTE_NORMAL)
			pCharacter->m_Emote = EMOTE_PAIN;
		pCharacter->m_Weapon = WEAPON_NINJA;
	}
	else if (m_FreezeTime > 0 || m_FreezeTime == -1)
	{
		if (pCharacter->m_Emote == EMOTE_NORMAL)
			pCharacter->m_Emote = EMOTE_BLINK;
		pCharacter->m_Weapon = WEAPON_NINJA;
	}

	// jetpack and ninjajetpack prediction
	if (m_pPlayer->GetCID() == SnappingClient)
	{
		if (m_Jetpack && pCharacter->m_Weapon != WEAPON_NINJA)
		{
			if (!(m_NeededFaketuning & FAKETUNE_JETPACK))
			{
				m_NeededFaketuning |= FAKETUNE_JETPACK;
				GameServer()->SendTuningParams(m_pPlayer->GetCID(), m_TuneZone);
			}
		}
		else
		{
			if (m_NeededFaketuning & FAKETUNE_JETPACK)
			{
				m_NeededFaketuning &= ~FAKETUNE_JETPACK;
				GameServer()->SendTuningParams(m_pPlayer->GetCID(), m_TuneZone);
			}
		}
	}

	// change eyes, use ninja graphic and set ammo count if player has ninjajetpack
	if (m_pPlayer->m_NinjaJetpack && m_Jetpack && m_Core.m_ActiveWeapon == WEAPON_GUN && !m_DeepFreeze && !(m_FreezeTime > 0 || m_FreezeTime == -1))
	{
		if (pCharacter->m_Emote == EMOTE_NORMAL)
			pCharacter->m_Emote = EMOTE_HAPPY;
		pCharacter->m_Weapon = WEAPON_NINJA;
		pCharacter->m_AmmoCount = 10;
	}

	if (m_pPlayer->GetCID() == SnappingClient || SnappingClient == -1 ||
		(!g_Config.m_SvStrictSpectateMode && m_pPlayer->GetCID() == GameServer()->m_apPlayers[SnappingClient]->m_SpectatorID))
	{
		pCharacter->m_Health = m_Health;
		pCharacter->m_Armor = m_Armor;
		if (m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo > 0)
		{
			if (m_pPlayer->m_IsVanillaWeapons)
			{
				pCharacter->m_AmmoCount = m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo;
			}
			else
			{
				pCharacter->m_AmmoCount = (!m_FreezeTime) ? m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo : 0;
			}
		}
	}

	if (GetPlayer()->m_Afk || GetPlayer()->m_Paused)
		pCharacter->m_Emote = EMOTE_BLINK;

	if (pCharacter->m_Emote == EMOTE_NORMAL)
	{
		if (250 - ((Server()->Tick() - m_LastAction) % (250)) < 5)
			pCharacter->m_Emote = EMOTE_BLINK;
	}

	if (m_pPlayer->m_Halloween)
	{
		if (1200 - ((Server()->Tick() - m_LastAction) % (1200)) < 5)
		{
			GameServer()->SendEmoticon(m_pPlayer->GetCID(), EMOTICON_GHOST);
		}
	}

	pCharacter->m_PlayerFlags = GetPlayer()->m_PlayerFlags;
}

int CCharacter::NetworkClipped(int SnappingClient)
{
	return NetworkClipped(SnappingClient, m_Pos);
}

int CCharacter::NetworkClipped(int SnappingClient, vec2 CheckPos)
{
	if (SnappingClient == -1 || GameServer()->m_apPlayers[SnappingClient]->m_ShowAll)
		return 0;

	float dx = GameServer()->m_apPlayers[SnappingClient]->m_ViewPos.x - CheckPos.x;
	float dy = GameServer()->m_apPlayers[SnappingClient]->m_ViewPos.y - CheckPos.y;

	if (absolute(dx) > 1000.0f || absolute(dy) > 800.0f)
		return 1;

	if (distance(GameServer()->m_apPlayers[SnappingClient]->m_ViewPos, CheckPos) > 4000.0f)
		return 1;
	return 0;
}

// DDRace

bool CCharacter::CanCollide(int ClientID)
{
	return Teams()->m_Core.CanCollide(GetPlayer()->GetCID(), ClientID);
}
bool CCharacter::SameTeam(int ClientID)
{
	return Teams()->m_Core.SameTeam(GetPlayer()->GetCID(), ClientID);
}

// void CCharacter::TestPrintTiles(int Index)
// {
// #if defined(CONF_DEBUG)
// #endif
// 	int MapIndex = Index;
// 	//int PureMapIndex = GameServer()->Collision()->GetPureMapIndex(m_Pos);
// 	float Offset = 4.0f;
// 	int MapIndexL = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x + (m_ProximityRadius / 2) + Offset, m_Pos.y));
// 	int MapIndexR = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x - (m_ProximityRadius / 2) - Offset, m_Pos.y));
// 	int MapIndexT = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x, m_Pos.y + (m_ProximityRadius / 2) + Offset));
// 	int MapIndexB = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x, m_Pos.y - (m_ProximityRadius / 2) - Offset));
// 	//dbg_msg("","N%d L%d R%d B%d T%d",MapIndex,MapIndexL,MapIndexR,MapIndexB,MapIndexT);
// 	m_TileIndex = GameServer()->Collision()->GetTileIndex(MapIndex);
// 	m_TileIndexL = GameServer()->Collision()->GetTileIndex(MapIndexL);
// 	m_TileIndexR = GameServer()->Collision()->GetTileIndex(MapIndexR);
// 	m_TileIndexB = GameServer()->Collision()->GetTileIndex(MapIndexB);
// 	m_TileIndexT = GameServer()->Collision()->GetTileIndex(MapIndexT);

// 	if (m_TileIndexR == TILE_BEGIN)
// 	{
// 		dbg_msg("FNN","finish tile on the right");
// 	}
// 	else  if (m_TileIndex == TILE_BEGIN)
// 	{
// 		dbg_msg("FNN","in startline");
// 	}
// 	else if (m_TileIndexR == TILE_FREEZE)
// 	{
// 		dbg_msg("FNN", "freeze spottedt at the right freeze=%d",m_TileIndexR);
// 	}
// 	else
// 	{
// 		if (GameServer()->m_IsDebug)
// 			dbg_msg("FNN","tile=%d tileR=%d", m_TileIndex, m_TileIndexR);
// 	}
// }

int CCharacter::Team()
{
	return Teams()->m_Core.Team(m_pPlayer->GetCID());
}

void CCharacter::ClearFakeMotd()
{
	if (m_pPlayer->m_IsFakeMotd)
	{
		GameServer()->AbuseMotd(g_Config.m_SvMotd, m_pPlayer->GetCID());
		//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "clear fake motd");
		m_pPlayer->m_CanClearFakeMotd = true;
		m_pPlayer->m_IsFakeMotd = false;
	}
}

CGameTeams* CCharacter::Teams()
{
	return &((CGameControllerDDRace*)GameServer()->m_pController)->m_Teams;
}

void CCharacter::HandleBroadcast()
{
	CPlayerData *pData = GameServer()->Score()->PlayerData(m_pPlayer->GetCID());

	if (m_DDRaceState == DDRACE_STARTED && m_CpLastBroadcast != m_CpActive &&
		m_CpActive > -1 && m_CpTick > Server()->Tick() && m_pPlayer->m_ClientVersion == VERSION_VANILLA &&
		pData->m_BestTime && pData->m_aBestCpTime[m_CpActive] != 0)
	{
		char aBroadcast[128];
		float Diff = m_CpCurrent[m_CpActive] - pData->m_aBestCpTime[m_CpActive];
		str_format(aBroadcast, sizeof(aBroadcast), "Checkpoint | Diff : %+5.2f", Diff);
		GameServer()->SendBroadcast(aBroadcast, m_pPlayer->GetCID());
		m_CpLastBroadcast = m_CpActive;
		m_LastBroadcast = Server()->Tick();
	}
	else if ((m_pPlayer->m_TimerType == 1 || m_pPlayer->m_TimerType == 2) && m_DDRaceState == DDRACE_STARTED && m_LastBroadcast + Server()->TickSpeed() * g_Config.m_SvTimeInBroadcastInterval <= Server()->Tick())
	{
		char aBuftime[64];
		int IntTime = (int)((float)(Server()->Tick() - m_StartTime) / ((float)Server()->TickSpeed()));
		str_format(aBuftime, sizeof(aBuftime), "%s%d:%s%d", ((IntTime / 60) > 9) ? "" : "0", IntTime / 60, ((IntTime % 60) > 9) ? "" : "0", IntTime % 60);
		GameServer()->SendBroadcast(aBuftime, m_pPlayer->GetCID());
		m_CpLastBroadcast = m_CpActive;
		m_LastBroadcast = Server()->Tick();
	}
}

void CCharacter::HandleSkippableTiles(int Index)
{
	// handle death-tiles and leaving gamelayer
	if((GameServer()->Collision()->GetCollisionAt(m_Pos.x+m_ProximityRadius/3.f, m_Pos.y-m_ProximityRadius/3.f) == TILE_DEATH ||
			GameServer()->Collision()->GetCollisionAt(m_Pos.x+m_ProximityRadius/3.f, m_Pos.y+m_ProximityRadius/3.f) == TILE_DEATH ||
			GameServer()->Collision()->GetCollisionAt(m_Pos.x-m_ProximityRadius/3.f, m_Pos.y-m_ProximityRadius/3.f) == TILE_DEATH ||
			GameServer()->Collision()->GetFCollisionAt(m_Pos.x+m_ProximityRadius/3.f, m_Pos.y-m_ProximityRadius/3.f) == TILE_DEATH||
			GameServer()->Collision()->GetFCollisionAt(m_Pos.x+m_ProximityRadius/3.f, m_Pos.y+m_ProximityRadius/3.f) == TILE_DEATH ||
			GameServer()->Collision()->GetFCollisionAt(m_Pos.x-m_ProximityRadius/3.f, m_Pos.y-m_ProximityRadius/3.f) == TILE_DEATH ||
			GameServer()->Collision()->GetCollisionAt(m_Pos.x-m_ProximityRadius/3.f, m_Pos.y+m_ProximityRadius/3.f) == TILE_DEATH) &&
			!m_Super && !(Team() && Teams()->TeeFinished(m_pPlayer->GetCID())))
	{
		Die(m_pPlayer->GetCID(), WEAPON_WORLD);
		return;
	}

	//// handle fng score tiles (mede by ChillerDragon hehe)
	//if ((GameServer()->Collision()->GetCollisionAt(m_Pos.x + m_ProximityRadius / 3.f, m_Pos.y - m_ProximityRadius / 3.f) == TILE_FNG_SCORE ||
	//	GameServer()->Collision()->GetCollisionAt(m_Pos.x + m_ProximityRadius / 3.f, m_Pos.y + m_ProximityRadius / 3.f) == TILE_FNG_SCORE ||
	//	GameServer()->Collision()->GetCollisionAt(m_Pos.x - m_ProximityRadius / 3.f, m_Pos.y - m_ProximityRadius / 3.f) == TILE_FNG_SCORE ||
	//	GameServer()->Collision()->GetFCollisionAt(m_Pos.x + m_ProximityRadius / 3.f, m_Pos.y - m_ProximityRadius / 3.f) == TILE_FNG_SCORE ||
	//	GameServer()->Collision()->GetFCollisionAt(m_Pos.x + m_ProximityRadius / 3.f, m_Pos.y + m_ProximityRadius / 3.f) == TILE_FNG_SCORE ||
	//	GameServer()->Collision()->GetFCollisionAt(m_Pos.x - m_ProximityRadius / 3.f, m_Pos.y - m_ProximityRadius / 3.f) == TILE_FNG_SCORE ||
	//	GameServer()->Collision()->GetCollisionAt(m_Pos.x - m_ProximityRadius / 3.f, m_Pos.y + m_ProximityRadius / 3.f) == TILE_FNG_SCORE) &&
	//	!m_Super && !(Team() && Teams()->TeeFinished(m_pPlayer->GetCID()))) //yolo leave super cheats also activated in fng cuz whatever
	//{
	//	dbg_msg("fok","fak"); //doesnt get triggerd
	//	Die(m_pPlayer->GetCID(), WEAPON_WORLD, true);
	//	return;
	//}

	if (GameLayerClipped(m_Pos))
	{
		Die(m_pPlayer->GetCID(), WEAPON_WORLD);
		return;
	}

	if (Index < 0)
		return;

	// handle speedup tiles
	if (GameServer()->Collision()->IsSpeedup(Index))
	{
		vec2 Direction, MaxVel, TempVel = m_Core.m_Vel;
		int Force, MaxSpeed = 0;
		float TeeAngle, SpeederAngle, DiffAngle, SpeedLeft, TeeSpeed;
		GameServer()->Collision()->GetSpeedup(Index, &Direction, &Force, &MaxSpeed);
		if (Force == 255 && MaxSpeed)
		{
			m_Core.m_Vel = Direction * (MaxSpeed / 5);
		}
		else
		{
			if (MaxSpeed > 0 && MaxSpeed < 5) MaxSpeed = 5;
			//dbg_msg("speedup tile start","Direction %f %f, Force %d, Max Speed %d", (Direction).x,(Direction).y, Force, MaxSpeed);
			if (MaxSpeed > 0)
			{
				if (Direction.x > 0.0000001f)
					SpeederAngle = -atan(Direction.y / Direction.x);
				else if (Direction.x < 0.0000001f)
					SpeederAngle = atan(Direction.y / Direction.x) + 2.0f * asin(1.0f);
				else if (Direction.y > 0.0000001f)
					SpeederAngle = asin(1.0f);
				else
					SpeederAngle = asin(-1.0f);

				if (SpeederAngle < 0)
					SpeederAngle = 4.0f * asin(1.0f) + SpeederAngle;

				if (TempVel.x > 0.0000001f)
					TeeAngle = -atan(TempVel.y / TempVel.x);
				else if (TempVel.x < 0.0000001f)
					TeeAngle = atan(TempVel.y / TempVel.x) + 2.0f * asin(1.0f);
				else if (TempVel.y > 0.0000001f)
					TeeAngle = asin(1.0f);
				else
					TeeAngle = asin(-1.0f);

				if (TeeAngle < 0)
					TeeAngle = 4.0f * asin(1.0f) + TeeAngle;

				TeeSpeed = sqrt((double)(pow(TempVel.x, 2) + pow(TempVel.y, 2)));

				DiffAngle = SpeederAngle - TeeAngle;
				SpeedLeft = MaxSpeed / 5.0f - cos(DiffAngle) * TeeSpeed;
				//dbg_msg("speedup tile debug","MaxSpeed %i, TeeSpeed %f, SpeedLeft %f, SpeederAngle %f, TeeAngle %f", MaxSpeed, TeeSpeed, SpeedLeft, SpeederAngle, TeeAngle);
				if (abs((int)SpeedLeft) > Force && SpeedLeft > 0.0000001f)
					TempVel += Direction * Force;
				else if (abs((int)SpeedLeft) > Force)
					TempVel += Direction * -Force;
				else
					TempVel += Direction * SpeedLeft;
			}
			else
				TempVel += Direction * Force;

			if (TempVel.x > 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_270) || (m_TileIndexL == TILE_STOP && m_TileFlagsL == ROTATION_270) || (m_TileIndexL == TILE_STOPS && (m_TileFlagsL == ROTATION_90 || m_TileFlagsL == ROTATION_270)) || (m_TileIndexL == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_270) || (m_TileFIndexL == TILE_STOP && m_TileFFlagsL == ROTATION_270) || (m_TileFIndexL == TILE_STOPS && (m_TileFFlagsL == ROTATION_90 || m_TileFFlagsL == ROTATION_270)) || (m_TileFIndexL == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_270) || (m_TileSIndexL == TILE_STOP && m_TileSFlagsL == ROTATION_270) || (m_TileSIndexL == TILE_STOPS && (m_TileSFlagsL == ROTATION_90 || m_TileSFlagsL == ROTATION_270)) || (m_TileSIndexL == TILE_STOPA)))
				TempVel.x = 0;
			if (TempVel.x < 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_90) || (m_TileIndexR == TILE_STOP && m_TileFlagsR == ROTATION_90) || (m_TileIndexR == TILE_STOPS && (m_TileFlagsR == ROTATION_90 || m_TileFlagsR == ROTATION_270)) || (m_TileIndexR == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_90) || (m_TileFIndexR == TILE_STOP && m_TileFFlagsR == ROTATION_90) || (m_TileFIndexR == TILE_STOPS && (m_TileFFlagsR == ROTATION_90 || m_TileFFlagsR == ROTATION_270)) || (m_TileFIndexR == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_90) || (m_TileSIndexR == TILE_STOP && m_TileSFlagsR == ROTATION_90) || (m_TileSIndexR == TILE_STOPS && (m_TileSFlagsR == ROTATION_90 || m_TileSFlagsR == ROTATION_270)) || (m_TileSIndexR == TILE_STOPA)))
				TempVel.x = 0;
			if (TempVel.y < 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_180) || (m_TileIndexB == TILE_STOP && m_TileFlagsB == ROTATION_180) || (m_TileIndexB == TILE_STOPS && (m_TileFlagsB == ROTATION_0 || m_TileFlagsB == ROTATION_180)) || (m_TileIndexB == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_180) || (m_TileFIndexB == TILE_STOP && m_TileFFlagsB == ROTATION_180) || (m_TileFIndexB == TILE_STOPS && (m_TileFFlagsB == ROTATION_0 || m_TileFFlagsB == ROTATION_180)) || (m_TileFIndexB == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_180) || (m_TileSIndexB == TILE_STOP && m_TileSFlagsB == ROTATION_180) || (m_TileSIndexB == TILE_STOPS && (m_TileSFlagsB == ROTATION_0 || m_TileSFlagsB == ROTATION_180)) || (m_TileSIndexB == TILE_STOPA)))
				TempVel.y = 0;
			if (TempVel.y > 0 && ((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_0) || (m_TileIndexT == TILE_STOP && m_TileFlagsT == ROTATION_0) || (m_TileIndexT == TILE_STOPS && (m_TileFlagsT == ROTATION_0 || m_TileFlagsT == ROTATION_180)) || (m_TileIndexT == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_0) || (m_TileFIndexT == TILE_STOP && m_TileFFlagsT == ROTATION_0) || (m_TileFIndexT == TILE_STOPS && (m_TileFFlagsT == ROTATION_0 || m_TileFFlagsT == ROTATION_180)) || (m_TileFIndexT == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_0) || (m_TileSIndexT == TILE_STOP && m_TileSFlagsT == ROTATION_0) || (m_TileSIndexT == TILE_STOPS && (m_TileSFlagsT == ROTATION_0 || m_TileSFlagsT == ROTATION_180)) || (m_TileSIndexT == TILE_STOPA)))
				TempVel.y = 0;
			m_Core.m_Vel = TempVel;
			//dbg_msg("speedup tile end","(Direction*Force) %f %f   m_Core.m_Vel%f %f",(Direction*Force).x,(Direction*Force).y,m_Core.m_Vel.x,m_Core.m_Vel.y);
			//dbg_msg("speedup tile end","Direction %f %f, Force %d, Max Speed %d", (Direction).x,(Direction).y, Force, MaxSpeed);
		}
	}
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
		m_DeepFreeze = true;
	else if(Type == CFG_TILE_UNDEEP)
		m_DeepFreeze = false;
	else if(Type == CFG_TILE_DEATH)
	{
		Die(m_pPlayer->GetCID(), WEAPON_WORLD); // TODO: probably should be in places where TILE_DEATH is and not here
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

void CCharacter::HandleTiles(int Index)
{
	CGameControllerDDRace* Controller = (CGameControllerDDRace*)GameServer()->m_pController;
	int MapIndex = Index;
	//int PureMapIndex = GameServer()->Collision()->GetPureMapIndex(m_Pos);
	float Offset = 4.0f;
	int MapIndexL = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x + (m_ProximityRadius / 2) + Offset, m_Pos.y));
	int MapIndexR = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x - (m_ProximityRadius / 2) - Offset, m_Pos.y));
	int MapIndexT = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x, m_Pos.y + (m_ProximityRadius / 2) + Offset));
	int MapIndexB = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x, m_Pos.y - (m_ProximityRadius / 2) - Offset));
	//dbg_msg("","N%d L%d R%d B%d T%d",MapIndex,MapIndexL,MapIndexR,MapIndexB,MapIndexT);
	m_TileIndex = GameServer()->Collision()->GetTileIndex(MapIndex);
	m_TileFlags = GameServer()->Collision()->GetTileFlags(MapIndex);
	m_TileIndexL = GameServer()->Collision()->GetTileIndex(MapIndexL);
	m_TileFlagsL = GameServer()->Collision()->GetTileFlags(MapIndexL);
	m_TileIndexR = GameServer()->Collision()->GetTileIndex(MapIndexR);
	m_TileFlagsR = GameServer()->Collision()->GetTileFlags(MapIndexR);
	m_TileIndexB = GameServer()->Collision()->GetTileIndex(MapIndexB);
	m_TileFlagsB = GameServer()->Collision()->GetTileFlags(MapIndexB);
	m_TileIndexT = GameServer()->Collision()->GetTileIndex(MapIndexT);
	m_TileFlagsT = GameServer()->Collision()->GetTileFlags(MapIndexT);
	m_TileFIndex = GameServer()->Collision()->GetFTileIndex(MapIndex);
	m_TileFFlags = GameServer()->Collision()->GetFTileFlags(MapIndex);
	m_TileFIndexL = GameServer()->Collision()->GetFTileIndex(MapIndexL);
	m_TileFFlagsL = GameServer()->Collision()->GetFTileFlags(MapIndexL);
	m_TileFIndexR = GameServer()->Collision()->GetFTileIndex(MapIndexR);
	m_TileFFlagsR = GameServer()->Collision()->GetFTileFlags(MapIndexR);
	m_TileFIndexB = GameServer()->Collision()->GetFTileIndex(MapIndexB);
	m_TileFFlagsB = GameServer()->Collision()->GetFTileFlags(MapIndexB);
	m_TileFIndexT = GameServer()->Collision()->GetFTileIndex(MapIndexT);
	m_TileFFlagsT = GameServer()->Collision()->GetFTileFlags(MapIndexT);//
	m_TileSIndex = (GameServer()->Collision()->m_pSwitchers && GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetDTileNumber(MapIndex)].m_Status[Team()]) ? (Team() != TEAM_SUPER) ? GameServer()->Collision()->GetDTileIndex(MapIndex) : 0 : 0;
	m_TileSFlags = (GameServer()->Collision()->m_pSwitchers && GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetDTileNumber(MapIndex)].m_Status[Team()]) ? (Team() != TEAM_SUPER) ? GameServer()->Collision()->GetDTileFlags(MapIndex) : 0 : 0;
	m_TileSIndexL = (GameServer()->Collision()->m_pSwitchers && GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetDTileNumber(MapIndexL)].m_Status[Team()]) ? (Team() != TEAM_SUPER) ? GameServer()->Collision()->GetDTileIndex(MapIndexL) : 0 : 0;
	m_TileSFlagsL = (GameServer()->Collision()->m_pSwitchers && GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetDTileNumber(MapIndexL)].m_Status[Team()]) ? (Team() != TEAM_SUPER) ? GameServer()->Collision()->GetDTileFlags(MapIndexL) : 0 : 0;
	m_TileSIndexR = (GameServer()->Collision()->m_pSwitchers && GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetDTileNumber(MapIndexR)].m_Status[Team()]) ? (Team() != TEAM_SUPER) ? GameServer()->Collision()->GetDTileIndex(MapIndexR) : 0 : 0;
	m_TileSFlagsR = (GameServer()->Collision()->m_pSwitchers && GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetDTileNumber(MapIndexR)].m_Status[Team()]) ? (Team() != TEAM_SUPER) ? GameServer()->Collision()->GetDTileFlags(MapIndexR) : 0 : 0;
	m_TileSIndexB = (GameServer()->Collision()->m_pSwitchers && GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetDTileNumber(MapIndexB)].m_Status[Team()]) ? (Team() != TEAM_SUPER) ? GameServer()->Collision()->GetDTileIndex(MapIndexB) : 0 : 0;
	m_TileSFlagsB = (GameServer()->Collision()->m_pSwitchers && GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetDTileNumber(MapIndexB)].m_Status[Team()]) ? (Team() != TEAM_SUPER) ? GameServer()->Collision()->GetDTileFlags(MapIndexB) : 0 : 0;
	m_TileSIndexT = (GameServer()->Collision()->m_pSwitchers && GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetDTileNumber(MapIndexT)].m_Status[Team()]) ? (Team() != TEAM_SUPER) ? GameServer()->Collision()->GetDTileIndex(MapIndexT) : 0 : 0;
	m_TileSFlagsT = (GameServer()->Collision()->m_pSwitchers && GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetDTileNumber(MapIndexT)].m_Status[Team()]) ? (Team() != TEAM_SUPER) ? GameServer()->Collision()->GetDTileFlags(MapIndexT) : 0 : 0;
	//dbg_msg("Tiles","%d, %d, %d, %d, %d", m_TileSIndex, m_TileSIndexL, m_TileSIndexR, m_TileSIndexB, m_TileSIndexT);
	//Sensitivity
	int S1 = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x + m_ProximityRadius / 3.f, m_Pos.y - m_ProximityRadius / 3.f));
	int S2 = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x + m_ProximityRadius / 3.f, m_Pos.y + m_ProximityRadius / 3.f));
	int S3 = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x - m_ProximityRadius / 3.f, m_Pos.y - m_ProximityRadius / 3.f));
	int S4 = GameServer()->Collision()->GetPureMapIndex(vec2(m_Pos.x - m_ProximityRadius / 3.f, m_Pos.y + m_ProximityRadius / 3.f));
	int Tile1 = GameServer()->Collision()->GetTileIndex(S1);
	int Tile2 = GameServer()->Collision()->GetTileIndex(S2);
	int Tile3 = GameServer()->Collision()->GetTileIndex(S3);
	int Tile4 = GameServer()->Collision()->GetTileIndex(S4);
	int FTile1 = GameServer()->Collision()->GetFTileIndex(S1);
	int FTile2 = GameServer()->Collision()->GetFTileIndex(S2);
	int FTile3 = GameServer()->Collision()->GetFTileIndex(S3);
	int FTile4 = GameServer()->Collision()->GetFTileIndex(S4);
	//dbg_msg("","N%d L%d R%d B%d T%d",m_TileIndex,m_TileIndexL,m_TileIndexR,m_TileIndexB,m_TileIndexT);
	//dbg_msg("","N%d L%d R%d B%d T%d",m_TileFIndex,m_TileFIndexL,m_TileFIndexR,m_TileFIndexB,m_TileFIndexT);
	if (Index < 0)
	{
		m_LastRefillJumps = false;
		m_LastPenalty = false;
		m_LastBonus = false;
		return;
	}
	int cp = GameServer()->Collision()->IsCheckpoint(MapIndex);
	if (cp != -1 && m_DDRaceState == DDRACE_STARTED && cp > m_CpActive)
	{
		m_CpActive = cp;
		m_CpCurrent[cp] = m_Time;
		m_CpTick = Server()->Tick() + Server()->TickSpeed() * 2;
		if (m_pPlayer->m_ClientVersion >= VERSION_DDRACE) {
			CPlayerData *pData = GameServer()->Score()->PlayerData(m_pPlayer->GetCID());
			CNetMsg_Sv_DDRaceTime Msg;
			Msg.m_Time = (int)m_Time;
			Msg.m_Check = 0;
			Msg.m_Finish = 0;

			if (m_CpActive != -1 && m_CpTick > Server()->Tick())
			{
				if (pData->m_BestTime && pData->m_aBestCpTime[m_CpActive] != 0)
				{
					float Diff = (m_CpCurrent[m_CpActive] - pData->m_aBestCpTime[m_CpActive]) * 100;
					Msg.m_Check = (int)Diff;
				}
			}

			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, m_pPlayer->GetCID());
		}
	}
	int cpf = GameServer()->Collision()->IsFCheckpoint(MapIndex);
	if (cpf != -1 && m_DDRaceState == DDRACE_STARTED && cpf > m_CpActive)
	{
		m_CpActive = cpf;
		m_CpCurrent[cpf] = m_Time;
		m_CpTick = Server()->Tick() + Server()->TickSpeed() * 2;
		if (m_pPlayer->m_ClientVersion >= VERSION_DDRACE) {
			CPlayerData *pData = GameServer()->Score()->PlayerData(m_pPlayer->GetCID());
			CNetMsg_Sv_DDRaceTime Msg;
			Msg.m_Time = (int)m_Time;
			Msg.m_Check = 0;
			Msg.m_Finish = 0;

			if (m_CpActive != -1 && m_CpTick > Server()->Tick())
			{
				if (pData->m_BestTime && pData->m_aBestCpTime[m_CpActive] != 0)
				{
					float Diff = (m_CpCurrent[m_CpActive] - pData->m_aBestCpTime[m_CpActive]) * 100;
					Msg.m_Check = (int)Diff;
				}
			}

			Server()->SendPackMsg(&Msg, MSGFLAG_VITAL, m_pPlayer->GetCID());
		}
	}
	int tcp = GameServer()->Collision()->IsTCheckpoint(MapIndex);
	if (tcp)
		m_TeleCheckpoint = tcp;

	// start                                                                                                                                                                                                                                                                                                                                   
	if (((m_TileIndex == TILE_BEGIN) || (m_TileFIndex == TILE_BEGIN) || FTile1 == TILE_BEGIN || FTile2 == TILE_BEGIN || FTile3 == TILE_BEGIN || FTile4 == TILE_BEGIN || Tile1 == TILE_BEGIN || Tile2 == TILE_BEGIN || Tile3 == TILE_BEGIN || Tile4 == TILE_BEGIN))
	{
		if (m_DDRaceState == DDRACE_NONE || m_DDRaceState == DDRACE_FINISHED || (m_DDRaceState == DDRACE_STARTED && !Team()))
		{
			bool CanBegin = true;
			if (g_Config.m_SvResetPickups)
			{
				for (int i = WEAPON_SHOTGUN; i < NUM_WEAPONS; ++i)
				{
					m_aWeapons[i].m_Got = false;
					if (m_Core.m_ActiveWeapon == i)
						m_Core.m_ActiveWeapon = WEAPON_GUN;
				}
			}
			if (g_Config.m_SvTeam == 2 && (Team() == TEAM_FLOCK || Teams()->Count(Team()) <= 1))
			{
				if (m_LastStartWarning < Server()->Tick() - 3 * Server()->TickSpeed())
				{
					GameServer()->SendChatTarget(GetPlayer()->GetCID(), "Server admin requires you to be in a team and with other tees to start");
					m_LastStartWarning = Server()->Tick();
				}
				Die(GetPlayer()->GetCID(), WEAPON_WORLD);
				CanBegin = false;
			}
			if (CanBegin)
			{
				Teams()->OnCharacterStart(m_pPlayer->GetCID());
				m_CpActive = -2;
			}
			else {
				
			}
		}

		// ddpp (external because we need the starttile also if the race isnt starting)
		m_pPlayer->m_MoneyTilePlus = true;
		if (m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
		{
			if ((m_pPlayer->m_QuestStateLevel == 3 || m_pPlayer->m_QuestStateLevel == 8) && m_pPlayer->m_QuestProgressValue)
			{
				GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 2);
			}
			else if (m_pPlayer->m_QuestStateLevel == 9 && m_pPlayer->m_QuestFailed)
			{
				// GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] running agian.");
				m_pPlayer->m_QuestFailed = false;
			}
		}
		m_DDPP_Finished = false;
	}


	// chillerdragon dummy tile test
	// finish tile finishtile
	if (((m_TileIndex == TILE_END) || (m_TileFIndex == TILE_END) || FTile1 == TILE_END || FTile2 == TILE_END || FTile3 == TILE_END || FTile4 == TILE_END || Tile1 == TILE_END || Tile2 == TILE_END || Tile3 == TILE_END || Tile4 == TILE_END) && m_DDRaceState == DDRACE_STARTED)
	{
		Controller->m_Teams.OnCharacterFinish(m_pPlayer->GetCID()); // Quest 3 lvl 0-4 is handled in here teams.cpp
		if (m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
		{
			if (m_pPlayer->m_QuestStateLevel == 5)
			{
				if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1) //has flag
				{
					GameServer()->QuestCompleted(m_pPlayer->GetCID());
				}
				else
				{
					GameServer()->QuestFailed(m_pPlayer->GetCID());
				}
			}
			else if (m_pPlayer->m_QuestStateLevel == 9)
			{
				if (!m_pPlayer->m_QuestFailed)
				{
					GameServer()->QuestCompleted(m_pPlayer->GetCID());
				}
			}
		}

		m_DummyFinished = true;
		m_DummyFinishes++;

		/*
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "xp [%d/1000]", m_pPlayer->GetXP());
		GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
		*/
	}

	// DDNet++ finish tile
	if (((m_TileIndex == TILE_DDPP_END) || (m_TileFIndex == TILE_DDPP_END)) && !m_DDPP_Finished)
	{
		char aBuf[256];
		if (m_DDRaceState == DDRACE_STARTED)
		{
			float time = (float)(Server()->Tick() - Controller->m_Teams.GetStartTime(m_pPlayer))
				/ ((float)Server()->TickSpeed());
			if (time < 0.000001f)
				return;
			str_format(aBuf, sizeof(aBuf), "'%s' finished the special race [%d:%5.2f]!", Server()->ClientName(m_pPlayer->GetCID()), (int)time / 60, time - ((int)time / 60 * 60));
			GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);

			// quest
			if (m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
			{
				if (m_pPlayer->m_QuestStateLevel == 7)
				{
					if ((int)time > g_Config.m_SvQuestSpecialRaceTime)
					{
						GameServer()->QuestFailed(m_pPlayer->GetCID());
					}
					else
					{
						GameServer()->QuestCompleted(m_pPlayer->GetCID());
					}
				}
			}
		}
		else
		{
			// str_format(aBuf, sizeof(aBuf), "'%s' finished the special race [%d seconds]!", Server()->ClientName(m_pPlayer->GetCID()), m_AliveTime / Server()->TickSpeed()); //prints server up time in sec
			str_format(aBuf, sizeof(aBuf), "'%s' finished the special race !", Server()->ClientName(m_pPlayer->GetCID()));
			GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);

			// quest
			if (m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
			{
				if (m_pPlayer->m_QuestStateLevel == 7)
				{
					if (Server()->Tick() > m_AliveTime + Server()->TickSpeed() * g_Config.m_SvQuestSpecialRaceTime)
					{
						GameServer()->QuestFailed(m_pPlayer->GetCID());
					}
					else
					{
						GameServer()->QuestCompleted(m_pPlayer->GetCID());
					}
				}
			}
		}




		if (m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
		{
			if (m_pPlayer->m_QuestStateLevel == 6)
			{
				GameServer()->QuestCompleted(m_pPlayer->GetCID());
			}
			else if (m_pPlayer->m_QuestStateLevel == 8) //backwards
			{
				GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 2, 1);
			}
		}

		m_DDPP_Finished = true;
	}

	// DDNet++ config tiles
	if ((m_TileIndex == TILE_CONFIG_1) || (m_TileFIndex == TILE_CONFIG_1))
	{
		if(HandleConfigTile(g_Config.m_SvCfgTile1))
			return;
	}
	else if ((m_TileIndex == TILE_CONFIG_2) || (m_TileFIndex == TILE_CONFIG_2))
	{
		if(HandleConfigTile(g_Config.m_SvCfgTile2))
			return;
	}

	// freeze
	if (((m_TileIndex == TILE_FREEZE) || (m_TileFIndex == TILE_FREEZE)) && !m_Super && !m_DeepFreeze)
	{
		//Chiller Special
		if (m_Core.m_Pos.y > 223 * 32 && m_Core.m_Pos.y < 225 * 32 && m_Core.m_Pos.x < 438 * 32 && m_Core.m_Pos.x > 427 * 32 && m_fake_super)
		{
			UnFreeze();
		}
		else //normale freeze
		{
			Freeze();
			m_DummyFreezed = true;
			if ((m_pPlayer->GetCID() == GameServer()->m_BalanceID1 || m_pPlayer->GetCID() == GameServer()->m_BalanceID2) && GameServer()->m_BalanceBattleState == 2)
			{
				Die(m_pPlayer->GetCID(), WEAPON_SELF);
				return;
			}
		}
	}
	else if (((m_TileIndex == TILE_UNFREEZE) || (m_TileFIndex == TILE_UNFREEZE)) && !m_DeepFreeze)
	{
		UnFreeze();
		//MoneyTile();
	}

	// deep freeze
	if (((m_TileIndex == TILE_DFREEZE) || (m_TileFIndex == TILE_DFREEZE)) && !m_Super && !m_DeepFreeze)
		m_DeepFreeze = true;
	else if (((m_TileIndex == TILE_DUNFREEZE) || (m_TileFIndex == TILE_DUNFREEZE)) && !m_Super && m_DeepFreeze)
		m_DeepFreeze = false;

	// endless hook
	if (((m_TileIndex == TILE_EHOOK_START) || (m_TileFIndex == TILE_EHOOK_START)) && !m_EndlessHook)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "Endless hook has been activated");
		m_EndlessHook = true;
	}
	else if (((m_TileIndex == TILE_EHOOK_END) || (m_TileFIndex == TILE_EHOOK_END)) && m_EndlessHook)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "Endless hook has been deactivated");
		m_EndlessHook = false;
	}

	// hit others
	if (((m_TileIndex == TILE_HIT_END) || (m_TileFIndex == TILE_HIT_END)) && m_Hit != (DISABLE_HIT_GRENADE | DISABLE_HIT_HAMMER | DISABLE_HIT_RIFLE | DISABLE_HIT_SHOTGUN))
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You can't hit others");
		m_Hit = DISABLE_HIT_GRENADE | DISABLE_HIT_HAMMER | DISABLE_HIT_RIFLE | DISABLE_HIT_SHOTGUN;
	}
	else if (((m_TileIndex == TILE_HIT_START) || (m_TileFIndex == TILE_HIT_START)) && m_Hit != HIT_ALL)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You can hit others");
		m_Hit = HIT_ALL;
	}

	// collide with others
	if (((m_TileIndex == TILE_NPC_END) || (m_TileFIndex == TILE_NPC_END)) && m_Core.m_Collision)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You can't collide with others");
		m_Core.m_Collision = false;
		m_NeededFaketuning |= FAKETUNE_NOCOLL;
		GameServer()->SendTuningParams(m_pPlayer->GetCID(), m_TuneZone); // update tunings
	}
	else if (((m_TileIndex == TILE_NPC_START) || (m_TileFIndex == TILE_NPC_START)) && !m_Core.m_Collision)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You can collide with others");
		m_Core.m_Collision = true;
		m_NeededFaketuning &= ~FAKETUNE_NOCOLL;
		GameServer()->SendTuningParams(m_pPlayer->GetCID(), m_TuneZone); // update tunings
	}

	// hook others
	if (((m_TileIndex == TILE_NPH_END) || (m_TileFIndex == TILE_NPH_END)) && m_Core.m_Hook)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You can't hook others");
		m_Core.m_Hook = false;
		m_NeededFaketuning |= FAKETUNE_NOHOOK;
		GameServer()->SendTuningParams(m_pPlayer->GetCID(), m_TuneZone); // update tunings
	}
	else if (((m_TileIndex == TILE_NPH_START) || (m_TileFIndex == TILE_NPH_START)) && !m_Core.m_Hook)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You can hook others");
		m_Core.m_Hook = true;
		m_NeededFaketuning &= ~FAKETUNE_NOHOOK;
		GameServer()->SendTuningParams(m_pPlayer->GetCID(), m_TuneZone); // update tunings
	}

	// unlimited air jumps
	if (((m_TileIndex == TILE_SUPER_START) || (m_TileFIndex == TILE_SUPER_START)) && !m_SuperJump)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You have unlimited air jumps");
		m_SuperJump = true;
		if (m_Core.m_Jumps == 0)
		{
			m_NeededFaketuning &= ~FAKETUNE_NOJUMP;
			GameServer()->SendTuningParams(m_pPlayer->GetCID(), m_TuneZone); // update tunings
		}
	}
	else if (((m_TileIndex == TILE_SUPER_END) || (m_TileFIndex == TILE_SUPER_END)) && m_SuperJump)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You don't have unlimited air jumps");
		m_SuperJump = false;
		if (m_Core.m_Jumps == 0)
		{
			m_NeededFaketuning |= FAKETUNE_NOJUMP;
			GameServer()->SendTuningParams(m_pPlayer->GetCID(), m_TuneZone); // update tunings
		}
	}

	// walljump
	if ((m_TileIndex == TILE_WALLJUMP) || (m_TileFIndex == TILE_WALLJUMP))
	{
		if (m_Core.m_Vel.y > 0 && m_Core.m_Colliding && m_Core.m_LeftWall)
		{
			m_Core.m_LeftWall = false;
			m_Core.m_JumpedTotal = m_Core.m_Jumps - 1;
			m_Core.m_Jumped = 1;
		}
	}

	// FreezeShotgun
	/* TILE TEST
	if (((m_TileIndex == TILE_FSHOTGUN_START) || (m_TileFIndex == TILE_FSHOTGUN_START)) && !m_fShotgun)
	{
	GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You have a freeze shotgun!");
	m_fShotgun = true;
	}
	else if (((m_TileIndex == TILE_JETPACK_END) || (m_TileFIndex == TILE_JETPACK_END)) && m_fShotgun)
	{
	GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You lost your freeze shotgun!");
	m_fShotgun = false;
	}
	*/

	if (((m_TileIndex == 66) || (m_TileFIndex == 66)) && m_Core.m_Vel.x < 0) {

		if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1]->m_pCarryingCharacter == this || ((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0]->m_pCarryingCharacter == this) {
		}
		else {
			/*
			if (!(m_LastIndexTile == 66 || m_LastIndexFrontTile == 66) ){
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "You need a Flag to enter this Area!");
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			}*/

			if ((int)GameServer()->Collision()->GetPos(MapIndexL).x)
				if ((int)GameServer()->Collision()->GetPos(MapIndexL).x < (int)m_Core.m_Pos.x)
					m_Core.m_Pos = m_PrevPos;
			m_Core.m_Vel.x = 0;
		}

	}


	if (((m_TileIndex == 67) || (m_TileFIndex == 67)) && m_Core.m_Vel.x > 0) {

		if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1]->m_pCarryingCharacter == this || ((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0]->m_pCarryingCharacter == this) {
		}
		else {
			/*
			if (!(m_LastIndexTile == 67 || m_LastIndexFrontTile == 67) ){
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "You need a Flag to enter this Area!");
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			}*/

			if ((int)GameServer()->Collision()->GetPos(MapIndexL).x)
				if ((int)GameServer()->Collision()->GetPos(MapIndexL).x < (int)m_Core.m_Pos.x)
					m_Core.m_Pos = m_PrevPos;
			m_Core.m_Vel.x = 0;
		}

	}

	// cosmetic tiles
	//rainbow
	if (((m_TileIndex == TILE_RAINBOW) || (m_TileFIndex == TILE_RAINBOW)))
	{
		if (((m_LastIndexTile == TILE_RAINBOW) || (m_LastIndexFrontTile == TILE_RAINBOW)))
			return;

		if ((m_Rainbow) || (m_pPlayer->m_InfRainbow))
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You lost rainbow!");
			m_Rainbow = false;
			m_pPlayer->m_InfRainbow = false;
		}
		else
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You got rainbow!");
			m_Rainbow = true;
		}
	}

	//bloody
	if (((m_TileIndex == TILE_BLOODY) || (m_TileFIndex == TILE_BLOODY)))
	{
		if (((m_LastIndexTile == TILE_BLOODY) || (m_LastIndexFrontTile == TILE_BLOODY)))
			return;

		if ((m_Bloody) || (m_StrongBloody) || (m_pPlayer->m_InfBloody))
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You lost bloody!");
			m_Bloody = false;
			m_StrongBloody = false;
			m_pPlayer->m_InfBloody = false;
		}
		else
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You got bloody!");
			m_Bloody = true;
		}
	}

	// atom
	if (((m_TileIndex == TILE_ATOM) || (m_TileFIndex == TILE_ATOM)))
	{
		if (((m_LastIndexTile == TILE_ATOM) || (m_LastIndexFrontTile == TILE_ATOM)))
			return;

		if ((m_Atom) || (m_pPlayer->m_InfAtom))
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You lost atom!");
			m_Atom = false;
			m_pPlayer->m_InfAtom = false;
		}
		else
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You got atom!");
			m_Atom = true;
		}
	}

	// trail
	if (((m_TileIndex == TILE_TRAIL) || (m_TileFIndex == TILE_TRAIL)))
	{
		if (((m_LastIndexTile == TILE_TRAIL) || (m_LastIndexFrontTile == TILE_TRAIL)))
			return;

		if ((m_Trail) || (m_pPlayer->m_InfTrail))
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You lost trail!");
			m_Trail = false;
			m_pPlayer->m_InfTrail = false;
		}
		else
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You got trail!");
			m_Trail = true;
		}
	}

	// spread gun
	if (((m_TileIndex == TILE_SPREAD_GUN) || (m_TileFIndex == TILE_SPREAD_GUN)))
	{
		if (((m_LastIndexTile == TILE_SPREAD_GUN) || (m_LastIndexFrontTile == TILE_SPREAD_GUN)))
			return;

		if ((m_autospreadgun) || (m_pPlayer->m_InfAutoSpreadGun))
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You lost spread gun!");
			m_autospreadgun = false;
			m_pPlayer->m_InfAutoSpreadGun = false;
		}
		else
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You got spread gun!");
			m_autospreadgun = true;
		}
	}

	m_LastIndexTile = m_TileIndex; // do not remove
	m_LastIndexFrontTile = m_TileFIndex; // do not remove


	//hammer tiles
	if (((m_TileIndex == TILE_NO_HAMMER) || (m_TileFIndex == TILE_NO_HAMMER)))
	{
		m_aWeapons[WEAPON_HAMMER].m_Got = false;
		if (!SetWeaponThatChrHas()) // Cheat gun if hammer was last weapon
		{
			m_aWeapons[WEAPON_GUN].m_Got = true;
		}
		SetWeaponThatChrHas();
	}


	//Money Tiles
	if (((m_TileIndex == TILE_MONEY) || (m_TileFIndex == TILE_MONEY)))
	{
		MoneyTile();
	}

	if (((m_TileIndex == TILE_MONEY_POLICE) || (m_TileFIndex == TILE_MONEY_POLICE)))
	{
		MoneyTilePolice();
	}

	if (((m_TileIndex == TILE_MONEY_PLUS) || (m_TileFIndex == TILE_MONEY_PLUS)))
	{
		MoneyTilePlus();
	}

	if (((m_TileIndex == TILE_FNG_SCORE) || (m_TileFIndex == TILE_FNG_SCORE)))
	{
		Die(m_pPlayer->GetCID(), WEAPON_WORLD, true);
	}

	if (((m_TileIndex == TILE_MONEY_DOUBLE) || (m_TileFIndex == TILE_MONEY_DOUBLE)))
	{
		MoneyTileDouble();
	}

	// jetpack gun
	if (((m_TileIndex == TILE_JETPACK_START) || (m_TileFIndex == TILE_JETPACK_START)) && !m_Jetpack && m_aWeapons[WEAPON_GUN].m_Got)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You have a jetpack gun");
		m_Jetpack = true;
	}
	else if (((m_TileIndex == TILE_JETPACK_END) || (m_TileFIndex == TILE_JETPACK_END)) && m_Jetpack)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You lost your jetpack gun");
		m_Jetpack = false;
	}

	// ROOM POINT
	bool Allowed = false;
	if (g_Config.m_SvRoomState == 0) //all
	{
		Allowed = true;
	}
	else if (g_Config.m_SvRoomState == 1) //buy
	{
		Allowed = (m_pPlayer->m_BoughtRoom) ? true : false;
	}
	else if (g_Config.m_SvRoomState == 2) //buy invite
	{
		Allowed = (m_pPlayer->m_BoughtRoom || m_HasRoomKeyBySuperModerator) ? true : false;
	}
	else if (g_Config.m_SvRoomState == 3) //buy admin
	{
		Allowed = (m_pPlayer->m_BoughtRoom || Server()->IsAuthed(m_pPlayer->GetCID())) ? true : false;
	}
	else if (g_Config.m_SvRoomState == 4) //buy admin invite
	{
		Allowed = (m_pPlayer->m_BoughtRoom || Server()->IsAuthed(m_pPlayer->GetCID()) || m_HasRoomKeyBySuperModerator) ? true : false;
	}

	//ROOMTILE
	if (((m_TileIndex == TILE_ROOM) || (m_TileFIndex == TILE_ROOM)) && !Allowed) // Admins got it free
	{
		//ChillerDragon upgrade to not cheat the map or stuff and tele too far

		//if (distance(m_Core.m_Pos, m_PrevSavePos) > 10 * 32)
		//{
			//dbg_msg("debug","Player's last pos too nearby distance: INT %d FLOAT %2.f", distance(m_Core.m_Pos, m_PrevSavePos), distance(m_Core.m_Pos, m_PrevSavePos));
			if (m_Core.m_Vel.x > 0)
			{
				m_Core.m_Pos = vec2(m_Core.m_Pos.x - 1 * 32, m_Core.m_Pos.y);
				m_Pos = vec2(m_Pos.x - 1 * 32,m_Pos.y);
				m_Core.m_Vel = vec2(-3, 0);
			}
			else
			{
				m_Core.m_Pos = vec2(m_Core.m_Pos.x + 1 * 32, m_Core.m_Pos.y);
				m_Pos = vec2(m_Pos.x + 1 * 32, m_Pos.y);
				m_Core.m_Vel = vec2(+3, 0);
			}
		//}
		//else
		//{
		//	dbg_msg("debug","distance ok loading PrevSavePos distance: INT %d FLOAT %2.f", distance(m_Core.m_Pos, m_PrevSavePos), distance(m_Core.m_Pos, m_PrevSavePos));

		//	m_Core.m_Pos = m_PrevSavePos;
		//	m_Pos = m_PrevSavePos;
		//	m_PrevPos = m_PrevSavePos;
		//	m_Core.m_Vel = vec2(0, 0);
		//	m_Core.m_HookedPlayer = -1;
		//	m_Core.m_HookState = HOOK_RETRACTED;
		//	m_Core.m_TriggeredEvents |= COREEVENT_HOOK_RETRACT;
		//	GameWorld()->ReleaseHooked(GetPlayer()->GetCID());
		//	m_Core.m_HookPos = m_Core.m_Pos;
		//}

		if (!m_WasInRoom)
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You need a key to enter this area!\nTry '/buy room_key' to enter this area.");
		}

		m_WasInRoom=true;
	}

	if (m_TileIndex == TILE_BLOCK_DM_JOIN || m_TileFIndex == TILE_BLOCK_DM_JOIN)
	{
		if (!m_pPlayer->m_IsBlockDeathmatch)
		{
			m_pPlayer->m_IsBlockDeathmatch = true;
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[BLOCK] you joined the deathmatch arena!");
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[BLOCK] type /leave to leave");
		}
	}

	if(m_TileIndex == TILE_BANK_IN || m_TileFIndex == TILE_BANK_IN) //BANKTILES
	{
		if (Server()->Tick() % 30 == 0 && GameServer()->m_IsBankOpen)
		{
			if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1) //has flag
			{
				if (!m_pPlayer->IsLoggedIn()) // only print stuff if player is not logged in while flag carry
				{
					GameServer()->SendBroadcast("~ B A N K ~", m_pPlayer->GetCID(), 0);
				}
			}
			else // no flag --> print always
			{
				GameServer()->SendBroadcast("~ B A N K ~", m_pPlayer->GetCID(), 0);
			}
		}
		m_InBank = true;
	}

	if (m_TileIndex == TILE_SHOP || m_TileFIndex == TILE_SHOP) // SHOP
	{
		if (!m_InShop)
		{
			m_EnteredShop = true;
			m_InShop = true;
		}
		if (Server()->Tick() % 450 == 0 || m_EnteredShop)
		{
			if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1) //has flag
			{
				if (!m_pPlayer->IsLoggedIn()) // only print stuff if player is not logged in while flag carry
				{
					GameServer()->SendBroadcast("~ S H O P ~", m_pPlayer->GetCID(), 0);
				}
			}
			else // no flag --> print always
			{
				GameServer()->SendBroadcast("~ S H O P ~", m_pPlayer->GetCID(), 0);
			}
		}
		if (m_EnteredShop)
		{
			if (m_pPlayer->m_ShopBotAntiSpamTick <= Server()->Tick())
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "Welcome to the shop, %s! Press f4 to start shopping.", Server()->ClientName(m_pPlayer->GetCID()));
				SendShopMessage(aBuf);
			}
			m_EnteredShop = false;
		}
	}


	if (m_TileIndex == TILE_JAIL || m_TileFIndex == TILE_JAIL)
	{
		//GameServer()->SendBroadcast("You were arrested by the police!", m_pPlayer->GetCID(), 0); //dont spam people in jail this is just an tele tile
	}
	else if (m_TileIndex == TILE_JAILRELEASE || m_TileFIndex == TILE_JAILRELEASE)
	{
		//GameServer()->SendBroadcast("Your life as a gangster is over, don't get caught again!", m_pPlayer->GetCID(), 0); //dont send the message here wtf this is just an to tele tile
		m_InJailOpenArea = true;
		//dbg_msg("ddpp-tiles", "in jail release area");
	}

	if ((m_TileIndex == TILE_VANILLA_MODE || m_TileFIndex == TILE_VANILLA_MODE) && !(m_pPlayer->m_IsVanillaDmg && m_pPlayer->m_IsVanillaWeapons))
	{
		if (m_pPlayer->m_DummyMode != DUMMYMODE_ADVENTURE)
		{
			m_pPlayer->m_IsVanillaModeByTile = true;
			m_pPlayer->m_IsVanillaDmg = true;
			m_pPlayer->m_IsVanillaWeapons = true;
			m_pPlayer->m_IsVanillaCompetetive = true;
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You entered a vanilla area.");
		}
	}

	if ((m_TileIndex == TILE_DDRACE_MODE || m_TileFIndex == TILE_DDRACE_MODE) && (m_pPlayer->m_IsVanillaDmg && m_pPlayer->m_IsVanillaWeapons))
	{
		if (m_pPlayer->m_DummyMode == DUMMYMODE_ADVENTURE)
		{
			Die(m_pPlayer->GetCID(), WEAPON_SELF);
		}
		else
		{
			m_pPlayer->m_IsVanillaModeByTile = false;
			m_pPlayer->m_IsVanillaDmg = false;
			m_pPlayer->m_IsVanillaWeapons = false;
			m_pPlayer->m_IsVanillaCompetetive = false;
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You entered a ddrace area.");
		}
	}

	// solo part
	if (((m_TileIndex == TILE_SOLO_START) || (m_TileFIndex == TILE_SOLO_START)) && !Teams()->m_Core.GetSolo(m_pPlayer->GetCID()))
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You are now in a solo part.");
		SetSolo(true);
	}
	else if (((m_TileIndex == TILE_SOLO_END) || (m_TileFIndex == TILE_SOLO_END)) && Teams()->m_Core.GetSolo(m_pPlayer->GetCID()))
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You are now out of the solo part.");
		SetSolo(false);
	}

	// refill jumps
	if (((m_TileIndex == TILE_REFILL_JUMPS) || (m_TileFIndex == TILE_REFILL_JUMPS)) && !m_LastRefillJumps)
	{
		m_Core.m_JumpedTotal = 0;
		m_Core.m_Jumped = 0;
		m_LastRefillJumps = true;
	}
	if ((m_TileIndex != TILE_REFILL_JUMPS) && (m_TileFIndex != TILE_REFILL_JUMPS))
	{
		m_LastRefillJumps = false;
	}

	// stopper
	if (((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_270) || (m_TileIndexL == TILE_STOP && m_TileFlagsL == ROTATION_270) || (m_TileIndexL == TILE_STOPS && (m_TileFlagsL == ROTATION_90 || m_TileFlagsL == ROTATION_270)) || (m_TileIndexL == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_270) || (m_TileFIndexL == TILE_STOP && m_TileFFlagsL == ROTATION_270) || (m_TileFIndexL == TILE_STOPS && (m_TileFFlagsL == ROTATION_90 || m_TileFFlagsL == ROTATION_270)) || (m_TileFIndexL == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_270) || (m_TileSIndexL == TILE_STOP && m_TileSFlagsL == ROTATION_270) || (m_TileSIndexL == TILE_STOPS && (m_TileSFlagsL == ROTATION_90 || m_TileSFlagsL == ROTATION_270)) || (m_TileSIndexL == TILE_STOPA)) && m_Core.m_Vel.x > 0)
	{
		if ((int)GameServer()->Collision()->GetPos(MapIndexL).x)
			if ((int)GameServer()->Collision()->GetPos(MapIndexL).x < (int)m_Core.m_Pos.x)
				m_Core.m_Pos = m_PrevPos;
		m_Core.m_Vel.x = 0;
	}
	if (((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_90) || (m_TileIndexR == TILE_STOP && m_TileFlagsR == ROTATION_90) || (m_TileIndexR == TILE_STOPS && (m_TileFlagsR == ROTATION_90 || m_TileFlagsR == ROTATION_270)) || (m_TileIndexR == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_90) || (m_TileFIndexR == TILE_STOP && m_TileFFlagsR == ROTATION_90) || (m_TileFIndexR == TILE_STOPS && (m_TileFFlagsR == ROTATION_90 || m_TileFFlagsR == ROTATION_270)) || (m_TileFIndexR == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_90) || (m_TileSIndexR == TILE_STOP && m_TileSFlagsR == ROTATION_90) || (m_TileSIndexR == TILE_STOPS && (m_TileSFlagsR == ROTATION_90 || m_TileSFlagsR == ROTATION_270)) || (m_TileSIndexR == TILE_STOPA)) && m_Core.m_Vel.x < 0)
	{
		if ((int)GameServer()->Collision()->GetPos(MapIndexR).x)
			if ((int)GameServer()->Collision()->GetPos(MapIndexR).x >(int)m_Core.m_Pos.x)
				m_Core.m_Pos = m_PrevPos;
		m_Core.m_Vel.x = 0;
	}
	if (((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_180) || (m_TileIndexB == TILE_STOP && m_TileFlagsB == ROTATION_180) || (m_TileIndexB == TILE_STOPS && (m_TileFlagsB == ROTATION_0 || m_TileFlagsB == ROTATION_180)) || (m_TileIndexB == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_180) || (m_TileFIndexB == TILE_STOP && m_TileFFlagsB == ROTATION_180) || (m_TileFIndexB == TILE_STOPS && (m_TileFFlagsB == ROTATION_0 || m_TileFFlagsB == ROTATION_180)) || (m_TileFIndexB == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_180) || (m_TileSIndexB == TILE_STOP && m_TileSFlagsB == ROTATION_180) || (m_TileSIndexB == TILE_STOPS && (m_TileSFlagsB == ROTATION_0 || m_TileSFlagsB == ROTATION_180)) || (m_TileSIndexB == TILE_STOPA)) && m_Core.m_Vel.y < 0)
	{
		if ((int)GameServer()->Collision()->GetPos(MapIndexB).y)
			if ((int)GameServer()->Collision()->GetPos(MapIndexB).y >(int)m_Core.m_Pos.y)
				m_Core.m_Pos = m_PrevPos;
		m_Core.m_Vel.y = 0;
	}
	if (((m_TileIndex == TILE_STOP && m_TileFlags == ROTATION_0) || (m_TileIndexT == TILE_STOP && m_TileFlagsT == ROTATION_0) || (m_TileIndexT == TILE_STOPS && (m_TileFlagsT == ROTATION_0 || m_TileFlagsT == ROTATION_180)) || (m_TileIndexT == TILE_STOPA) || (m_TileFIndex == TILE_STOP && m_TileFFlags == ROTATION_0) || (m_TileFIndexT == TILE_STOP && m_TileFFlagsT == ROTATION_0) || (m_TileFIndexT == TILE_STOPS && (m_TileFFlagsT == ROTATION_0 || m_TileFFlagsT == ROTATION_180)) || (m_TileFIndexT == TILE_STOPA) || (m_TileSIndex == TILE_STOP && m_TileSFlags == ROTATION_0) || (m_TileSIndexT == TILE_STOP && m_TileSFlagsT == ROTATION_0) || (m_TileSIndexT == TILE_STOPS && (m_TileSFlagsT == ROTATION_0 || m_TileSFlagsT == ROTATION_180)) || (m_TileSIndexT == TILE_STOPA)) && m_Core.m_Vel.y > 0)
	{
		//dbg_msg("","%f %f",GameServer()->Collision()->GetPos(MapIndex).y,m_Core.m_Pos.y);
		if ((int)GameServer()->Collision()->GetPos(MapIndexT).y)
			if ((int)GameServer()->Collision()->GetPos(MapIndexT).y < (int)m_Core.m_Pos.y)
				m_Core.m_Pos = m_PrevPos;
		m_Core.m_Vel.y = 0;
		m_Core.m_Jumped = 0;
		m_Core.m_JumpedTotal = 0;
	}

	// handle switch tiles
	if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_SWITCHOPEN && Team() != TEAM_SUPER)
	{
		GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetSwitchNumber(MapIndex)].m_Status[Team()] = true;
		GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetSwitchNumber(MapIndex)].m_EndTick[Team()] = 0;
		GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetSwitchNumber(MapIndex)].m_Type[Team()] = TILE_SWITCHOPEN;
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_SWITCHTIMEDOPEN && Team() != TEAM_SUPER)
	{
		GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetSwitchNumber(MapIndex)].m_Status[Team()] = true;
		GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetSwitchNumber(MapIndex)].m_EndTick[Team()] = Server()->Tick() + 1 + GameServer()->Collision()->GetSwitchDelay(MapIndex)*Server()->TickSpeed();
		GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetSwitchNumber(MapIndex)].m_Type[Team()] = TILE_SWITCHTIMEDOPEN;
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_SWITCHTIMEDCLOSE && Team() != TEAM_SUPER)
	{
		GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetSwitchNumber(MapIndex)].m_Status[Team()] = false;
		GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetSwitchNumber(MapIndex)].m_EndTick[Team()] = Server()->Tick() + 1 + GameServer()->Collision()->GetSwitchDelay(MapIndex)*Server()->TickSpeed();
		GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetSwitchNumber(MapIndex)].m_Type[Team()] = TILE_SWITCHTIMEDCLOSE;
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_SWITCHCLOSE && Team() != TEAM_SUPER)
	{
		GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetSwitchNumber(MapIndex)].m_Status[Team()] = false;
		GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetSwitchNumber(MapIndex)].m_EndTick[Team()] = 0;
		GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetSwitchNumber(MapIndex)].m_Type[Team()] = TILE_SWITCHCLOSE;
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_FREEZE && Team() != TEAM_SUPER)
	{
		if (GameServer()->Collision()->GetSwitchNumber(MapIndex) == 0 || GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetSwitchNumber(MapIndex)].m_Status[Team()])
			Freeze(GameServer()->Collision()->GetSwitchDelay(MapIndex));
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_DFREEZE && Team() != TEAM_SUPER && GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetSwitchNumber(MapIndex)].m_Status[Team()])
	{
		m_DeepFreeze = true;
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_DUNFREEZE && Team() != TEAM_SUPER && GameServer()->Collision()->m_pSwitchers[GameServer()->Collision()->GetSwitchNumber(MapIndex)].m_Status[Team()])
	{
		m_DeepFreeze = false;
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_HIT_START && m_Hit&DISABLE_HIT_HAMMER && GameServer()->Collision()->GetSwitchDelay(MapIndex) == WEAPON_HAMMER)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You can hammer hit others");
		m_Hit &= ~DISABLE_HIT_HAMMER;
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_HIT_END && !(m_Hit&DISABLE_HIT_HAMMER) && GameServer()->Collision()->GetSwitchDelay(MapIndex) == WEAPON_HAMMER)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You can't hammer hit others");
		m_Hit |= DISABLE_HIT_HAMMER;
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_HIT_START && m_Hit&DISABLE_HIT_SHOTGUN && GameServer()->Collision()->GetSwitchDelay(MapIndex) == WEAPON_SHOTGUN)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You can shoot others with shotgun");
		m_Hit &= ~DISABLE_HIT_SHOTGUN;
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_HIT_END && !(m_Hit&DISABLE_HIT_SHOTGUN) && GameServer()->Collision()->GetSwitchDelay(MapIndex) == WEAPON_SHOTGUN)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You can't shoot others with shotgun");
		m_Hit |= DISABLE_HIT_SHOTGUN;
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_HIT_START && m_Hit&DISABLE_HIT_GRENADE && GameServer()->Collision()->GetSwitchDelay(MapIndex) == WEAPON_GRENADE)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You can shoot others with grenade");
		m_Hit &= ~DISABLE_HIT_GRENADE;
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_HIT_END && !(m_Hit&DISABLE_HIT_GRENADE) && GameServer()->Collision()->GetSwitchDelay(MapIndex) == WEAPON_GRENADE)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You can't shoot others with grenade");
		m_Hit |= DISABLE_HIT_GRENADE;
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_HIT_START && m_Hit&DISABLE_HIT_RIFLE && GameServer()->Collision()->GetSwitchDelay(MapIndex) == WEAPON_RIFLE)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You can shoot others with rifle");
		m_Hit &= ~DISABLE_HIT_RIFLE;
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_HIT_END && !(m_Hit&DISABLE_HIT_RIFLE) && GameServer()->Collision()->GetSwitchDelay(MapIndex) == WEAPON_RIFLE)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You can't shoot others with rifle");
		m_Hit |= DISABLE_HIT_RIFLE;
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_JUMP)
	{
		int newJumps = GameServer()->Collision()->GetSwitchDelay(MapIndex);

		if (newJumps != m_Core.m_Jumps)
		{
			char aBuf[256];
			if (newJumps == 1)
				str_format(aBuf, sizeof(aBuf), "You can jump %d time", newJumps);
			else
				str_format(aBuf, sizeof(aBuf), "You can jump %d times", newJumps);
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), aBuf);

			if (newJumps == 0 && !m_SuperJump)
			{
				m_NeededFaketuning |= FAKETUNE_NOJUMP;
				GameServer()->SendTuningParams(m_pPlayer->GetCID(), m_TuneZone); // update tunings
			}
			else if (m_Core.m_Jumps == 0)
			{
				m_NeededFaketuning &= ~FAKETUNE_NOJUMP;
				GameServer()->SendTuningParams(m_pPlayer->GetCID(), m_TuneZone); // update tunings
			}

			m_Core.m_Jumps = newJumps;
		}
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_PENALTY && !m_LastPenalty)
	{
		int min = GameServer()->Collision()->GetSwitchDelay(MapIndex);
		int sec = GameServer()->Collision()->GetSwitchNumber(MapIndex);
		int Team = Teams()->m_Core.Team(m_Core.m_Id);

		m_StartTime -= (min * 60 + sec) * Server()->TickSpeed();

		if (Team != TEAM_FLOCK && Team != TEAM_SUPER)
		{
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (Teams()->m_Core.Team(i) == Team && i != m_Core.m_Id && GameServer()->m_apPlayers[i])
				{
					CCharacter* pChar = GameServer()->m_apPlayers[i]->GetCharacter();

					if (pChar)
						pChar->m_StartTime = m_StartTime;
				}
			}
		}

		m_LastPenalty = true;
	}
	else if (GameServer()->Collision()->IsSwitch(MapIndex) == TILE_BONUS && !m_LastBonus)
	{
		int min = GameServer()->Collision()->GetSwitchDelay(MapIndex);
		int sec = GameServer()->Collision()->GetSwitchNumber(MapIndex);
		int Team = Teams()->m_Core.Team(m_Core.m_Id);

		m_StartTime += (min * 60 + sec) * Server()->TickSpeed();
		if (m_StartTime > Server()->Tick())
			m_StartTime = Server()->Tick();

		if (Team != TEAM_FLOCK && Team != TEAM_SUPER)
		{
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (Teams()->m_Core.Team(i) == Team && i != m_Core.m_Id && GameServer()->m_apPlayers[i])
				{
					CCharacter* pChar = GameServer()->m_apPlayers[i]->GetCharacter();

					if (pChar)
						pChar->m_StartTime = m_StartTime;
				}
			}
		}

		m_LastBonus = true;
	}

	if (GameServer()->Collision()->IsSwitch(MapIndex) != TILE_PENALTY)
	{
		m_LastPenalty = false;
	}

	if (GameServer()->Collision()->IsSwitch(MapIndex) != TILE_BONUS)
	{
		m_LastBonus = false;
	}

	int z = GameServer()->Collision()->IsTeleport(MapIndex);
	if (!g_Config.m_SvOldTeleportHook && !g_Config.m_SvOldTeleportWeapons && z && Controller->m_TeleOuts[z - 1].size())
	{
		if (m_Super)
			return;
		int Num = Controller->m_TeleOuts[z - 1].size();
		m_Core.m_Pos = Controller->m_TeleOuts[z - 1][(!Num) ? Num : rand() % Num];
		if (!g_Config.m_SvTeleportHoldHook)
		{
			m_Core.m_HookedPlayer = -1;
			m_Core.m_HookState = HOOK_RETRACTED;
			m_Core.m_TriggeredEvents |= COREEVENT_HOOK_RETRACT;
			m_Core.m_HookPos = m_Core.m_Pos;
		}
		if (g_Config.m_SvTeleportLoseWeapons)
		{
			for (int i = WEAPON_SHOTGUN; i<NUM_WEAPONS - 1; i++)
				m_aWeapons[i].m_Got = false;
		}
		return;
	}
	int evilz = GameServer()->Collision()->IsEvilTeleport(MapIndex);
	if (evilz && Controller->m_TeleOuts[evilz - 1].size())
	{
		if (m_Super)
			return;
		int Num = Controller->m_TeleOuts[evilz - 1].size();
		m_Core.m_Pos = Controller->m_TeleOuts[evilz - 1][(!Num) ? Num : rand() % Num];
		if (!g_Config.m_SvOldTeleportHook && !g_Config.m_SvOldTeleportWeapons)
		{
			m_Core.m_Vel = vec2(0, 0);

			if (!g_Config.m_SvTeleportHoldHook)
			{
				m_Core.m_HookedPlayer = -1;
				m_Core.m_HookState = HOOK_RETRACTED;
				m_Core.m_TriggeredEvents |= COREEVENT_HOOK_RETRACT;
				GameWorld()->ReleaseHooked(GetPlayer()->GetCID());
				m_Core.m_HookPos = m_Core.m_Pos;
			}
			if (g_Config.m_SvTeleportLoseWeapons)
			{
				for (int i = WEAPON_SHOTGUN; i<NUM_WEAPONS - 1; i++)
					m_aWeapons[i].m_Got = false;
			}
		}
		return;
	}
	if (GameServer()->Collision()->IsCheckEvilTeleport(MapIndex))
	{
		if (m_Super)
			return;
		// first check if there is a TeleCheckOut for the current recorded checkpoint, if not check previous checkpoints
		for (int k = m_TeleCheckpoint - 1; k >= 0; k--)
		{
			if (Controller->m_TeleCheckOuts[k].size())
			{
				m_Core.m_HookedPlayer = -1;
				m_Core.m_HookState = HOOK_RETRACTED;
				m_Core.m_TriggeredEvents |= COREEVENT_HOOK_RETRACT;
				int Num = Controller->m_TeleCheckOuts[k].size();
				m_Core.m_Pos = Controller->m_TeleCheckOuts[k][(!Num) ? Num : rand() % Num];
				GameWorld()->ReleaseHooked(GetPlayer()->GetCID());
				m_Core.m_Vel = vec2(0, 0);
				m_Core.m_HookPos = m_Core.m_Pos;
				return;
			}
		}
		// if no checkpointout have been found (or if there no recorded checkpoint), teleport to start
		vec2 SpawnPos;
		if (GameServer()->m_pController->CanSpawn(m_pPlayer->GetTeam(), &SpawnPos, m_pPlayer))
		{
			m_Core.m_HookedPlayer = -1;
			m_Core.m_HookState = HOOK_RETRACTED;
			m_Core.m_TriggeredEvents |= COREEVENT_HOOK_RETRACT;
			m_Core.m_Pos = SpawnPos;
			GameWorld()->ReleaseHooked(GetPlayer()->GetCID());
			m_Core.m_Vel = vec2(0, 0);
			m_Core.m_HookPos = m_Core.m_Pos;
		}
		return;
	}
	if (GameServer()->Collision()->IsCheckTeleport(MapIndex))
	{
		if (m_Super)
			return;
		// first check if there is a TeleCheckOut for the current recorded checkpoint, if not check previous checkpoints
		for (int k = m_TeleCheckpoint - 1; k >= 0; k--)
		{
			if (Controller->m_TeleCheckOuts[k].size())
			{
				m_Core.m_HookedPlayer = -1;
				m_Core.m_HookState = HOOK_RETRACTED;
				m_Core.m_TriggeredEvents |= COREEVENT_HOOK_RETRACT;
				int Num = Controller->m_TeleCheckOuts[k].size();
				m_Core.m_Pos = Controller->m_TeleCheckOuts[k][(!Num) ? Num : rand() % Num];
				m_Core.m_HookPos = m_Core.m_Pos;
				return;
			}
		}
		// if no checkpointout have been found (or if there no recorded checkpoint), teleport to start
		vec2 SpawnPos;
		if (GameServer()->m_pController->CanSpawn(m_pPlayer->GetTeam(), &SpawnPos, m_pPlayer))
		{
			m_Core.m_HookedPlayer = -1;
			m_Core.m_HookState = HOOK_RETRACTED;
			m_Core.m_TriggeredEvents |= COREEVENT_HOOK_RETRACT;
			m_Core.m_Pos = SpawnPos;
			m_Core.m_HookPos = m_Core.m_Pos;
		}
		return;
	}
}

void CCharacter::HandleTuneLayer()
{

	m_TuneZoneOld = m_TuneZone;
	int CurrentIndex = GameServer()->Collision()->GetMapIndex(m_Pos);
	m_TuneZone = GameServer()->Collision()->IsTune(CurrentIndex);

	if (m_TuneZone)
		m_Core.m_pWorld->m_Tuning[g_Config.m_ClDummy] = GameServer()->TuningList()[m_TuneZone]; // throw tunings from specific zone into gamecore
	else
		m_Core.m_pWorld->m_Tuning[g_Config.m_ClDummy] = *GameServer()->Tuning();

	if (m_TuneZone != m_TuneZoneOld) // dont send tunigs all the time
	{
		// send zone msgs
		SendZoneMsgs();
	}
}

void CCharacter::SendZoneMsgs()
{
	// send zone leave msg
	if (m_TuneZoneOld >= 0 && GameServer()->m_ZoneLeaveMsg[m_TuneZoneOld]) // m_TuneZoneOld >= 0: avoid zone leave msgs on spawn
	{
		const char* cur = GameServer()->m_ZoneLeaveMsg[m_TuneZoneOld];
		const char* pos;
		while ((pos = str_find(cur, "\\n")))
		{
			char aBuf[256];
			str_copy(aBuf, cur, pos - cur + 1);
			aBuf[pos - cur + 1] = '\0';
			cur = pos + 2;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
		}
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), cur);
	}
	// send zone enter msg
	if (GameServer()->m_ZoneEnterMsg[m_TuneZone])
	{
		const char* cur = GameServer()->m_ZoneEnterMsg[m_TuneZone];
		const char* pos;
		while ((pos = str_find(cur, "\\n")))
		{
			char aBuf[256];
			str_copy(aBuf, cur, pos - cur + 1);
			aBuf[pos - cur + 1] = '\0';
			cur = pos + 2;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
		}
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), cur);
	}
}

void CCharacter::DDRaceTick()
{
	if (!m_pPlayer->m_IsVanillaDmg)
	{
		m_Armor = (m_FreezeTime >= 0) ? 10 - (m_FreezeTime / 15) : 0;
	}
	if (m_Input.m_Direction != 0 || m_Input.m_Jump != 0)
		m_LastMove = Server()->Tick();

	if (m_FreezeTime > 0 || m_FreezeTime == -1)
	{
		if (m_FreezeTime % Server()->TickSpeed() == Server()->TickSpeed() - 1 || m_FreezeTime == -1)
		{
			GameServer()->CreateDamageInd(m_Pos, 0, (m_FreezeTime + 1) / Server()->TickSpeed(), Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
		}
		if (m_FreezeTime > 0)
			m_FreezeTime--;
		else
			m_Ninja.m_ActivationTick = Server()->Tick();
		m_Input.m_Direction = 0;
		m_Input.m_Jump = 0;
		m_Input.m_Hook = 0;
		//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "drr sagt 0");

		if (m_FreezeTime == 1)
			UnFreeze();
	}

	HandleTuneLayer(); // need this before coretick

	{
		int index = GameServer()->Collision()->GetPureMapIndex(m_Pos);
		int tile = GameServer()->Collision()->GetTileIndex(index);
		int ftile = GameServer()->Collision()->GetFTileIndex(index);
		if (IsGrounded() && tile != TILE_FREEZE && tile != TILE_DFREEZE && tile != TILE_ROOM && ftile!= TILE_ROOM && ftile != TILE_FREEZE && ftile != TILE_DFREEZE) {
			m_PrevSavePos = m_Pos;
			m_SetSavePos = true;
		}
	}

	m_Core.m_Id = GetPlayer()->GetCID();
}


void CCharacter::DDRacePostCoreTick()
{
	isFreezed = false;
	m_Time = (float)(Server()->Tick() - m_StartTime) / ((float)Server()->TickSpeed());

	if (m_pPlayer->m_DefEmoteReset >= 0 && m_pPlayer->m_DefEmoteReset <= Server()->Tick())
	{
		m_pPlayer->m_DefEmoteReset = -1;
		m_EmoteType = m_pPlayer->m_DefEmote = EMOTE_NORMAL;
		m_EmoteStop = -1;
	}

	if (m_EndlessHook || (m_Super && g_Config.m_SvEndlessSuperHook))
	{
		m_Core.m_HookTick = 0;
		//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hook = 0 endless");
	}

	if (m_DeepFreeze && !m_Super)
		Freeze();

	if (m_Core.m_Jumps == 0 && !m_Super)
		m_Core.m_Jumped = 3;
	else if (m_Core.m_Jumps == 1 && m_Core.m_Jumped > 0)
		m_Core.m_Jumped = 3;
	else if (m_Core.m_JumpedTotal < m_Core.m_Jumps - 1 && m_Core.m_Jumped > 1)
		m_Core.m_Jumped = 1;

	if ((m_Super || m_SuperJump) && m_Core.m_Jumped > 1)
		m_Core.m_Jumped = 1;

	int CurrentIndex = GameServer()->Collision()->GetMapIndex(m_Pos);
	HandleSkippableTiles(CurrentIndex);

	// handle Anti-Skip tiles
	std::list < int > Indices = GameServer()->Collision()->GetMapIndices(m_PrevPos, m_Pos);
	if (!Indices.empty())
		for (std::list < int >::iterator i = Indices.begin(); i != Indices.end(); i++)
		{
			HandleTiles(*i);
			//dbg_msg("Running","%d", *i);
		}
	else
	{
		HandleTiles(CurrentIndex);
		m_LastIndexTile = 0;
		m_LastIndexFrontTile = 0;
		// if (m_pPlayer->m_IsDummy && m_pPlayer->m_DummyMode == 25)
		// {
		// 	TestPrintTiles(CurrentIndex);
		// }
		//dbg_msg("Running","%d", CurrentIndex);
	}

	if (!(isFreezed)) {

		m_FirstFreezeTick = 0;

	}

	HandleBroadcast();
}

bool CCharacter::ForceFreeze(int Seconds)
{
	isFreezed = true;
	if (Seconds <= 0 || m_FreezeTime == -1 )
		return false;
	if (m_FreezeTick < Server()->Tick() - Server()->TickSpeed() || Seconds == -1)
	{
		if (!m_WeaponsBackupped) //only save once
		{
			for (int i = 0; i < NUM_WEAPONS; i++)
			{
				if (m_aWeapons[i].m_Got)
				{
					m_aWeaponsBackup[i][1] = m_aWeapons[i].m_Ammo; //save all ammo sats for m_IsVanillaWeapons to load em on unfreeze
																   //dbg_msg("vanilla", "'%s' saved weapon[%d] ammo[%d]", Server()->ClientName(m_pPlayer->GetCID()),i, m_aWeaponsBackup[i][1]);
					m_aWeapons[i].m_Ammo = 0; //dont set this to 0 in freeze to allow shoting in freeze (can be used for events)
				}
			}
			m_WeaponsBackupped = true;
		}

		if (!m_pPlayer->m_IsVanillaWeapons)
		{
			m_Armor = 0;
		}

		if (m_FreezeTick == 0 || m_FirstFreezeTick == 0) {
			m_FirstFreezeTick = Server()->Tick();
		}

		m_FreezeTime = Seconds == -1 ? Seconds : Seconds * Server()->TickSpeed();
		m_FreezeTick = Server()->Tick();
		return true;
	}
	return false;
}

bool CCharacter::Freeze(float Seconds)
{
	KillFreeze(false);
	isFreezed = true;
	if ((Seconds <= 0 || m_Super || m_FreezeTime == -1 || m_FreezeTime > Seconds * Server()->TickSpeed()) && Seconds != -1)
		return false;
	if (m_FreezeTick < Server()->Tick() - Server()->TickSpeed() || Seconds == -1)
	{
		if (!m_WeaponsBackupped) //only save once
		{
			for (int i = 0; i < NUM_WEAPONS; i++)
			{
				if (m_aWeapons[i].m_Got)
				{
					m_aWeaponsBackup[i][1] = m_aWeapons[i].m_Ammo; //save all ammo sats for m_IsVanillaWeapons to load em on unfreeze
					//dbg_msg("vanilla", "'%s' saved weapon[%d] ammo[%d]", Server()->ClientName(m_pPlayer->GetCID()),i, m_aWeaponsBackup[i][1]);
					m_aWeapons[i].m_Ammo = 0; //dont set this to 0 in freeze to allow shoting in freeze (can be used for events)
				}
			}
			m_WeaponsBackupped = true;
		}

		if (!m_pPlayer->m_IsVanillaWeapons)
		{
			m_Armor = 0;
		}

		if (m_FreezeTick == 0 || m_FirstFreezeTick == 0) {
			m_FirstFreezeTick = Server()->Tick();
		}

		m_FreezeTime = Seconds == -1 ? Seconds : Seconds * Server()->TickSpeed();
		m_FreezeTick = Server()->Tick();
		return true;
	}
	return false;
}

bool CCharacter::Freeze()
{
	return Freeze(g_Config.m_SvFreezeDelay);
}

bool CCharacter::UnFreeze()
{
	KillFreeze(true);
	if (m_FreezeTime > 0)
	{
		//BlockWave
		BlockWaveFreezeTicks = 0;
		m_pPlayer->m_IsBlockWaveDead = false;

		if (!m_pPlayer->m_IsVanillaDmg)
		{
			m_Armor = 10;
		}
		if (m_WeaponsBackupped) //only load once
		{
			for (int i = 0; i < NUM_WEAPONS; i++)
			{
				if (m_aWeapons[i].m_Got)
				{
					if (m_pPlayer->m_IsVanillaWeapons || m_aDecreaseAmmo[i] || (m_pPlayer->m_SpawnShotgunActive && i == WEAPON_SHOTGUN) || (m_pPlayer->m_SpawnGrenadeActive && i == WEAPON_GRENADE) || (m_pPlayer->m_SpawnRifleActive && i == WEAPON_RIFLE))
					{
						m_aWeapons[i].m_Ammo = m_aWeaponsBackup[i][1];
						//dbg_msg("vanilla", "'%s' loaded weapon[%d] ammo[%d]", Server()->ClientName(m_pPlayer->GetCID()), i, m_aWeapons[i].m_Ammo);
					}
					else
					{
						m_aWeapons[i].m_Ammo = -1;
						//dbg_msg("not vanilla", "'%s' loaded weapon[%d] ammo[%d]", Server()->ClientName(m_pPlayer->GetCID()), i, m_aWeapons[i].m_Ammo);
					}
				}
			}
			m_WeaponsBackupped = false;
		}

		if (!m_GotTasered)
			m_LastHitWeapon = -1;
		else
			m_GotTasered = false;

		if (!m_aWeapons[m_Core.m_ActiveWeapon].m_Got)
			m_Core.m_ActiveWeapon = WEAPON_GUN;
		m_FreezeTime = 0;
		m_FreezeTick = 0;
		m_FirstFreezeTick = 0;
		if (m_Core.m_ActiveWeapon == WEAPON_HAMMER) m_ReloadTimer = 0;
		return true;
	}
	return false;
}

void CCharacter::MoneyTile()
{
	if (Server()->Tick() % 50)
		return;
	if (!m_pPlayer->IsLoggedIn())
	{
		GameServer()->SendBroadcast("You need to be logged in to use moneytiles. \nGet an account with '/register <name> <pw> <pw>'", m_pPlayer->GetCID(), 0);
		return;
	}
	if (m_pPlayer->m_QuestState == CPlayer::QUEST_FARM)
	{
		if (m_pPlayer->m_QuestStateLevel < 7) // 10 money
		{
			m_pPlayer->m_QuestProgressValue2++;
			if (m_pPlayer->m_QuestProgressValue2 > m_pPlayer->m_QuestStateLevel)
			{
				GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 10);
				m_pPlayer->m_QuestProgressValue2 = 0;
			}
		}
		else if (m_pPlayer->m_QuestStateLevel == 7)
		{
			// moneytile police
		}
		else if (m_pPlayer->m_QuestStateLevel == 8)
		{
			m_pPlayer->m_QuestProgressValue2++;
			if (m_pPlayer->m_QuestProgressValue2 > 10)
			{
				GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 10);
				m_pPlayer->m_QuestProgressValue2 = 0;
			}
		}
	}
	if (m_pPlayer->IsMaxLevel())
	{
		if (m_pPlayer->m_xpmsg)
		{
			GameServer()->SendBroadcast("You reached the maximum level.", m_pPlayer->GetCID(), 0);
		}
		return;
	}

	int XP = 0;
	int Money = 0;
	int VIPBonus = 0;

	// flag extra xp
	if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
	{
		XP += 1;
	}

	// vip+ get 2 bonus
	if (m_pPlayer->m_IsSuperModerator)
	{
		XP += 2;
		Money += 2;
		VIPBonus = 2; // only for broadcast not used in calculation
	}
	// vip get 1 bonus
	else if (m_pPlayer->m_IsModerator)
	{
		XP += 1;
		Money += 1;
		VIPBonus = 1; // only for broadcast not used in calculation
	}

	// tile gain and survival bonus
	XP += 1 + m_survivexpvalue;
	Money += 1;

	// give money & xp
	m_pPlayer->GiveXP(XP);
	m_pPlayer->MoneyTransaction(Money);
	m_pPlayer->m_MoneyTilesMoney += Money;

	// show msg
	if (m_pPlayer->m_xpmsg)
	{
		// skip if other broadcasts activated:
		if (!m_pPlayer->m_hidejailmsg)
		{
			if (m_pPlayer->m_EscapeTime > 0 || m_pPlayer->m_JailTime > 0)
			{
				return;
			}
		}

		char FixBroadcast[32];
		if ((m_pPlayer->GetXP() >= 1000000) && m_survivexpvalue > 0)
			str_format(FixBroadcast, sizeof(FixBroadcast), "                                       ");
		else
			str_format(FixBroadcast, sizeof(FixBroadcast), "");

		char aBuf[128];
		if (m_survivexpvalue == 0)
		{
			if (VIPBonus)
			{
				if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1 +%d vip\nXP [%llu/%llu] +1 +1 flag +%d vip\nLevel [%d]", m_pPlayer->GetMoney(), VIPBonus, m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_pPlayer->GetLevel());
				else
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1 +%d vip\nXP [%llu/%llu] +1 +%d vip\nLevel [%d]", m_pPlayer->GetMoney(), VIPBonus, m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_pPlayer->GetLevel());
			}
			else
			{
				if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1\nXP [%llu/%llu] +1 +1 flag\nLevel [%d]", m_pPlayer->GetMoney(), m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_pPlayer->GetLevel());
				else
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1\nXP [%llu/%llu] +1\nLevel [%d]", m_pPlayer->GetMoney(), m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_pPlayer->GetLevel());
			}
		}
		else if (m_survivexpvalue > 0)
		{
			if (VIPBonus)
			{
				if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1 +%d vip\nXP [%llu/%llu] +1 +1 flag +%d vip +%d survival\nLevel [%d]", m_pPlayer->GetMoney(), VIPBonus, m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_survivexpvalue, m_pPlayer->GetLevel());
				else
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1 +%d vip\nXP [%llu/%llu] +1 +%d vip +%d survival\nLevel [%d]", m_pPlayer->GetMoney(), VIPBonus, m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_survivexpvalue, m_pPlayer->GetLevel());
			}
			else
			{
				if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1\nXP [%llu/%llu] +1 +1 flag +%d survival\nLevel [%d]", m_pPlayer->GetMoney(), m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_survivexpvalue, m_pPlayer->GetLevel());
				else
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1\nXP [%llu/%llu] +1 +%d survival\nLevel [%d]", m_pPlayer->GetMoney(), m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_survivexpvalue, m_pPlayer->GetLevel());
			}
		}
		str_append(aBuf, FixBroadcast, sizeof(aBuf));
		GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
	}
}

void CCharacter::MoneyTilePolice()
{
	if (Server()->Tick() % 50)
		return;
	if (!m_pPlayer->IsLoggedIn())
	{
		GameServer()->SendBroadcast("You need to be logged in to use moneytiles. \nGet an account with '/register <name> <pw> <pw>'", m_pPlayer->GetCID(), 0);
		return;
	}
	if (m_pPlayer->m_QuestState == CPlayer::QUEST_FARM)
	{
		if (m_pPlayer->m_QuestStateLevel == 7)
		{
			m_pPlayer->m_QuestProgressValue2++;
			if (m_pPlayer->m_QuestProgressValue2 > 10)
			{
				GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 10);
				m_pPlayer->m_QuestProgressValue2 = 0;
			}
		}
	}
	if (m_pPlayer->IsMaxLevel())
	{
		if (m_pPlayer->m_xpmsg)
		{
			GameServer()->SendBroadcast("You have reached the maximum level.", m_pPlayer->GetCID(), 0);
		}
		return;
	}

	int XP = 0;
	int Money = 0;
	int VIPBonus = 0;

	// vip+ get 2 bonus
	if (m_pPlayer->m_IsSuperModerator)
	{
		XP += 2;
		Money += 2;
		VIPBonus = 2; // only for broadcast not used in calculation
	}
	// vip get 1 bonus
	else if (m_pPlayer->m_IsModerator)
	{
		XP += 1;
		Money += 1;
		VIPBonus = 1; // only for broadcast not used in calculation
	}

	// tile gain and survival bonus
	XP += 2 + m_survivexpvalue;
	Money += 1 + m_pPlayer->m_PoliceRank;

	// give money & xp
	m_pPlayer->GiveXP(XP);
	m_pPlayer->MoneyTransaction(Money);
	m_pPlayer->m_MoneyTilesMoney += Money;

	// show msg
	if (m_pPlayer->m_xpmsg)
	{
		// skip if other broadcasts activated:
		if (!m_pPlayer->m_hidejailmsg)
		{
			if (m_pPlayer->m_EscapeTime > 0 || m_pPlayer->m_JailTime > 0)
			{
				return;
			}
		}

		char FixBroadcast[64];
		if ((m_pPlayer->GetXP() >= 1000000) && m_survivexpvalue > 0)
			str_format(FixBroadcast, sizeof(FixBroadcast), "                                       ");
		else
			str_format(FixBroadcast, sizeof(FixBroadcast), "");

		char aBuf[128];
		if (m_pPlayer->m_PoliceRank > 0)
		{
			if (VIPBonus)
			{
				if (m_survivexpvalue == 0)
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1 +%d police +%d vip\nXP [%llu/%llu] +2 +%d vip\nLevel [%d]", m_pPlayer->GetMoney(), m_pPlayer->m_PoliceRank, VIPBonus, m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_pPlayer->GetLevel());
				else if (m_survivexpvalue > 0)
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1 +%d police +%d vip\nXP [%llu/%llu] +2 +%d vip +%d survival\nLevel [%d]", m_pPlayer->GetMoney(), m_pPlayer->m_PoliceRank, VIPBonus, m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_survivexpvalue, m_pPlayer->GetLevel());
			}
			else
			{
				if (m_survivexpvalue == 0)
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1 +%d police\nXP [%llu/%llu] +2\nLevel [%d]", m_pPlayer->GetMoney(), m_pPlayer->m_PoliceRank, m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_pPlayer->GetLevel());
				else if (m_survivexpvalue > 0)
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1 +%d police\nXP [%llu/%llu] +2 +%d survival\nLevel [%d]", m_pPlayer->GetMoney(), m_pPlayer->m_PoliceRank, m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_survivexpvalue, m_pPlayer->GetLevel());
			}
		}
		else
		{
			if (VIPBonus)
			{
				if (m_survivexpvalue == 0)
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1 +%d vip\nXP [%llu/%llu] +2 +%d vip\nLevel [%d]", m_pPlayer->GetMoney(), VIPBonus, m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_pPlayer->GetLevel());
				else if (m_survivexpvalue > 0)
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1 +%d vip\nXP [%llu/%llu] +2 +%d vip +%d survival\nLevel [%d]", m_pPlayer->GetMoney(), VIPBonus, m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_survivexpvalue, m_pPlayer->GetLevel());
			}
			else
			{
				if (m_survivexpvalue == 0)
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1\nXP [%llu/%llu] +2\nLevel [%d]", m_pPlayer->GetMoney(), m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_pPlayer->GetLevel());
				else if (m_survivexpvalue > 0)
					str_format(aBuf, sizeof(aBuf), "Money [%llu] +1\nXP [%llu/%llu] +2 +%d survival\nLevel [%d]", m_pPlayer->GetMoney(), m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_survivexpvalue, m_pPlayer->GetLevel());
			}
		}
		str_append(aBuf, FixBroadcast, sizeof(aBuf));
		GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
	}
}

void CCharacter::MoneyTileDouble()
{
	if (Server()->Tick() % 50)
		return;
	if (g_Config.m_SvMinDoubleTilePlayers == 0)
	{
		GameServer()->SendBroadcast("double moneytiles have been deactivated by an administrator", m_pPlayer->GetCID(), 0);
		return;
	}
	if (GameServer()->CountIngameHumans() < g_Config.m_SvMinDoubleTilePlayers)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "[%llu/%llu] players to activate the double moneytile", GameServer()->CountIngameHumans(), g_Config.m_SvMinDoubleTilePlayers);
		GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
		return;
	}
	if (!m_pPlayer->IsLoggedIn())
	{
		GameServer()->SendBroadcast("You need to be logged in to use moneytiles. \nGet an account with '/register <name> <pw> <pw>'", m_pPlayer->GetCID(), 0);
		return;
	}
	if (m_pPlayer->m_QuestState == CPlayer::QUEST_FARM)
	{
		if (m_pPlayer->m_QuestStateLevel < 7) // 10 money
		{
			m_pPlayer->m_QuestProgressValue2++;
			if (m_pPlayer->m_QuestProgressValue2 > m_pPlayer->m_QuestStateLevel)
			{
				GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 10);
				m_pPlayer->m_QuestProgressValue2 = 0;
			}
		}
		else if (m_pPlayer->m_QuestStateLevel == 7)
		{
			// moneytile police
		}
		else if (m_pPlayer->m_QuestStateLevel == 8)
		{
			m_pPlayer->m_QuestProgressValue2++;
			if (m_pPlayer->m_QuestProgressValue2 > 10)
			{
				GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 10);
				m_pPlayer->m_QuestProgressValue2 = 0;
			}
		}
	}
	if (m_pPlayer->IsMaxLevel())
	{
		if (m_pPlayer->m_xpmsg)
		{
			GameServer()->SendBroadcast("You reached the maximum level.", m_pPlayer->GetCID(), 0);
		}
		return;
	}

	int XP = 0;
	int Money = 0;

	// flag extra xp
	if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
	{
		XP += 2;
	}

	// tile gain and survival bonus
	int Survival = (m_survivexpvalue + 1);
	XP += 2 * Survival;
	Money += 4;

	// give money & xp
	m_pPlayer->GiveXP(XP);
	m_pPlayer->MoneyTransaction(Money);
	m_pPlayer->m_MoneyTilesMoney += Money;

	// show msg
	if (m_pPlayer->m_xpmsg)
	{
		// skip if other broadcasts activated:
		if (!m_pPlayer->m_hidejailmsg)
		{
			if (m_pPlayer->m_EscapeTime > 0 || m_pPlayer->m_JailTime > 0)
			{
				return;
			}
		}

		char aBuf[128];
		if (m_survivexpvalue == 0)
		{
			if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
				str_format(aBuf, sizeof(aBuf), "Money [%llu] +4\nXP [%llu/%llu] +2 +2 flag\nLevel [%d]", m_pPlayer->GetMoney(), m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_pPlayer->GetLevel());
			else
				str_format(aBuf, sizeof(aBuf), "Money [%llu] +4\nXP [%llu/%llu] +2\nLevel [%d]", m_pPlayer->GetMoney(), m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_pPlayer->GetLevel());
		}
		else if (m_survivexpvalue > 0)
		{
			if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
				str_format(aBuf, sizeof(aBuf), "Money [%llu] +4\nXP [%llu/%llu] +2 +2 flag +%d survival\nLevel [%d]", m_pPlayer->GetMoney(), m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), Survival, m_pPlayer->GetLevel());
			else
				str_format(aBuf, sizeof(aBuf), "Money [%llu] +4\nXP [%llu/%llu] +2 +%d survival\nLevel [%d]", m_pPlayer->GetMoney(), m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), Survival, m_pPlayer->GetLevel());
		}
		GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
	}
}

void CCharacter::MoneyTilePlus()
{
	if (!m_pPlayer->m_MoneyTilePlus)
		return;		
	m_pPlayer->m_MoneyTilePlus = false;

	if (m_pPlayer->IsMaxLevel())
	{
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You touched a MoneyTile Plus!  +500money");
	}
	else
	{
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You touched a MoneyTile Plus! +2500xp  +500money");
		m_pPlayer->GiveXP(2500);
	}
	if (m_pPlayer->m_xpmsg && m_pPlayer->IsLoggedIn())
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "Money [%llu]\nXP [%llu/%llu]\nLevel [%d]", m_pPlayer->GetMoney(), m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_pPlayer->GetLevel());
		GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 1);
	}
	m_pPlayer->MoneyTransaction(+500, "moneytile plus");
}

void CCharacter::GiveAllWeapons()
{
	for (int i = WEAPON_HAMMER; i<NUM_WEAPONS - 1; i++)
	{
		m_aWeapons[i].m_Got = true;
		if (!m_FreezeTime) m_aWeapons[i].m_Ammo = -1;
	}
	return;
}

void CCharacter::SetSpawnWeapons()
{
	if (m_pPlayer->m_UseSpawnWeapons && !m_pPlayer->IsInstagibMinigame() && !m_pPlayer->m_IsSurvivaling)
	{
		if (m_pPlayer->m_SpawnWeaponShotgun)
		{
			m_aWeapons[2].m_Got = true;
			m_aWeapons[2].m_Ammo = m_pPlayer->m_SpawnWeaponShotgun;
			m_pPlayer->m_SpawnShotgunActive = 1;
		}

		if (m_pPlayer->m_SpawnWeaponGrenade)
		{
			m_aWeapons[3].m_Got = true;
			m_aWeapons[3].m_Ammo = m_pPlayer->m_SpawnWeaponGrenade;
			m_pPlayer->m_SpawnGrenadeActive = 1;
		}

		if (m_pPlayer->m_SpawnWeaponRifle)
		{
			m_aWeapons[4].m_Got = true;
			m_aWeapons[4].m_Ammo = m_pPlayer->m_SpawnWeaponRifle;
			m_pPlayer->m_SpawnRifleActive = 1;
		}
	}

	return;
}

void CCharacter::SetSpookyGhost()
{
	if (m_pPlayer->m_IsBlockTourning || (m_pPlayer->m_IsSurvivaling && m_pPlayer->m_IsSurvivalLobby == false)) // no ghost in competetive minigames
		return;

	if (!m_SpookyGhostWeaponsBackupped)
	{

		for (int i = 0; i < NUM_WEAPONS; i++)
		{
			m_aSpookyGhostWeaponsBackup[i][1] = m_aWeapons[i].m_Ammo;
			m_aSpookyGhostWeaponsBackupGot[i][1] = m_aWeapons[i].m_Got;
			m_aWeapons[i].m_Ammo = 0;
			m_aWeapons[i].m_Got = false;
		}
		m_SpookyGhostWeaponsBackupped = true;
		m_aWeapons[1].m_Got = 1;
		m_aWeapons[1].m_Ammo = -1;
	}

	str_copy(m_pPlayer->m_TeeInfos.m_SkinName, "ghost", sizeof(m_pPlayer->m_TeeInfos.m_SkinName));
	m_pPlayer->m_TeeInfos.m_UseCustomColor = 0;

	m_pPlayer->m_SpookyGhostActive = 1;
}

void CCharacter::UnsetSpookyGhost()
{
	if (m_SpookyGhostWeaponsBackupped)
	{
		for (int i = 0; i < NUM_WEAPONS; i++)
		{
			m_aWeapons[i].m_Got = m_aSpookyGhostWeaponsBackupGot[i][1];
			if (m_pPlayer->m_IsVanillaWeapons || m_pPlayer->m_SpawnShotgunActive || m_pPlayer->m_SpawnGrenadeActive || m_pPlayer->m_SpawnRifleActive)
			{
				m_aWeapons[i].m_Ammo = m_aSpookyGhostWeaponsBackup[i][1];
			}
			else
			{
				m_aWeapons[i].m_Ammo = -1;
			}
		}
		m_SpookyGhostWeaponsBackupped = false;
	}

	str_copy(m_pPlayer->m_TeeInfos.m_SkinName, m_pPlayer->m_RealSkinName, sizeof(m_pPlayer->m_TeeInfos.m_SkinName));
	m_pPlayer->m_TeeInfos.m_UseCustomColor = m_pPlayer->m_RealUseCustomColor;

	m_pPlayer->m_SpookyGhostActive = 0;

	return;
}

void CCharacter::SaveRealInfos()
{
	if (!m_pPlayer->m_SpookyGhostActive)
	{
		str_copy(m_pPlayer->m_RealSkinName, m_pPlayer->m_TeeInfos.m_SkinName, sizeof(m_pPlayer->m_RealSkinName));
		m_pPlayer->m_RealUseCustomColor = m_pPlayer->m_TeeInfos.m_UseCustomColor;
		str_copy(m_pPlayer->m_RealClan, Server()->ClientClan(m_pPlayer->GetCID()), sizeof(m_pPlayer->m_RealClan));
		str_copy(m_pPlayer->m_RealName, Server()->ClientName(m_pPlayer->GetCID()), sizeof(m_pPlayer->m_RealName));
	}

	return;
}

void CCharacter::BulletAmounts()
{
	m_GunBullets = m_aWeapons[1].m_Ammo;
	m_ShotgunBullets = m_aWeapons[2].m_Ammo;
	m_GrenadeBullets = m_aWeapons[3].m_Ammo;
	m_RifleBullets = m_aWeapons[4].m_Ammo;
	return;
}

bool CCharacter::SetWeaponThatChrHas()
{
	if (m_aWeapons[WEAPON_GUN].m_Got)
		SetWeapon(WEAPON_GUN);
	else if (m_aWeapons[WEAPON_HAMMER].m_Got)
		SetWeapon(WEAPON_HAMMER);
	else if (m_aWeapons[WEAPON_GRENADE].m_Got)
		SetWeapon(WEAPON_GRENADE);
	else if (m_aWeapons[WEAPON_SHOTGUN].m_Got)
		SetWeapon(WEAPON_SHOTGUN);
	else if (m_aWeapons[WEAPON_RIFLE].m_Got)
		SetWeapon(WEAPON_RIFLE);
	else
		return false;

	return true;
}

void CCharacter::ShopWindow(int Dir)
{

	m_ShopMotdTick = 0;

	// if you add something to the shop make sure to also add pages here and extend conshop in ddracechat.cpp.

	int m_MaxShopPage = 13; // UPDATE THIS WITH EVERY PAGE YOU ADD!!!!!

	if (Dir == 0)
	{
		m_ShopWindowPage = 0;
	}
	else if (Dir == 1)
	{
		m_ShopWindowPage++;
		if (m_ShopWindowPage > m_MaxShopPage)
		{
			m_ShopWindowPage = 0;
		}
	}
	else if (Dir == -1)
	{
		m_ShopWindowPage--;
		if (m_ShopWindowPage < 0)
		{
			m_ShopWindowPage = m_MaxShopPage;
		}
	}

	
	char aItem[256];
	char aLevelTmp[128];
	char aPriceTmp[16];
	char aTimeTmp[256];
	char aInfo[1028];

	if (m_ShopWindowPage == 0)
	{
		str_format(aItem, sizeof(aItem), "Welcome to the shop! If you need help, use '/shop help'.\n\n"
			"By shooting to the right you go one site forward,\n"
			"and by shooting left you go one site backwards.\n\n"
			"If you need more help, visit '/shop help'.");
	}
	else if (m_ShopWindowPage == 1)
	{
		str_format(aItem, sizeof(aItem), "        ~  R A I N B O W  ~      ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "5");
		str_format(aPriceTmp, sizeof(aPriceTmp), "1.500");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item until you're dead.");
		str_format(aInfo, sizeof(aInfo), "Rainbow will make your tee change the color very fast.");
	}
	else if (m_ShopWindowPage == 2)
	{
		str_format(aItem, sizeof(aItem), "        ~  B L O O D Y  ~      ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "15");
		str_format(aPriceTmp, sizeof(aPriceTmp), "3.500");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item until you're dead.");
		str_format(aInfo, sizeof(aInfo), "Bloody will give your tee a permanent kill effect.");
	}
	else if (m_ShopWindowPage == 3)
	{
		str_format(aItem, sizeof(aItem), "        ~  C H I D R A Q U L  ~      ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "2");
		str_format(aPriceTmp, sizeof(aPriceTmp), "250");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item until\n"
			"you disconnect.");
		str_format(aInfo, sizeof(aInfo), "Chidraqul is a minigame by ChillerDragon.\n"
			"More information about this game coming soon.");
	}
	else if (m_ShopWindowPage == 4)
	{
		str_format(aItem, sizeof(aItem), "        ~  S H I T  ~      ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "0");
		str_format(aPriceTmp, sizeof(aPriceTmp), "5");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "Shit is a fun item. You can use to '/poop' on other players.\n"
			"You can also see your shit amount in your '/profile'.");
	}
	else if (m_ShopWindowPage == 5)
	{
		str_format(aItem, sizeof(aItem), "        ~  R O O M K E Y  ~      ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "16");
		str_format(aPriceTmp, sizeof(aPriceTmp), "%d", g_Config.m_SvRoomPrice);
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item until\n"
			"you disconnect.");
		str_format(aInfo, sizeof(aInfo), "If you have the room key you can enter the bank room.\n"
			"It's under the spawn and there is a money tile.");
	}
	else if (m_ShopWindowPage == 6)
	{
		str_format(aItem, sizeof(aItem), "        ~  P O L I C E  ~      ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "18");
		str_format(aPriceTmp, sizeof(aPriceTmp), "100.000");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "Police officers get help from the police bot.\n"
			"For more information about the specific police ranks\n"
			"please visit '/policeinfo'.");
	}
	else if (m_ShopWindowPage == 7)
	{
		str_format(aItem, sizeof(aItem), "        ~  T A S E R  ~      ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "30");
		str_format(aPriceTmp, sizeof(aPriceTmp), "50.000");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "Taser replaces your unfreeze rifle with a rifle that freezes\n"
			"other tees. You can toggle it using '/taser <on/off>'.\n"
			"For more information about the taser and your taser stats,\n"
			"plase visit '/taser info'.");
	}
	else if (m_ShopWindowPage == 8)
	{
		str_format(aItem, sizeof(aItem), "    ~  P V P A R E N A T I C K E T  ~  ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "0");
		str_format(aPriceTmp, sizeof(aPriceTmp), "150");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "You can join the pvp arena using '/pvp_arena join' if you have a ticket.");
	}
	else if (m_ShopWindowPage == 9)
	{
		str_format(aItem, sizeof(aItem), "       ~  N I N J A J E T P A C K  ~     ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "21");
		str_format(aPriceTmp, sizeof(aPriceTmp), "10.000");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "It will make your jetpack gun be a ninja.\n"
			"Toggle it using '/ninjajetpack'.");
	}
	else if (m_ShopWindowPage == 10)
	{
		str_format(aItem, sizeof(aItem), "     ~  S P A W N S H O T G U N  ~   ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "33");
		str_format(aPriceTmp, sizeof(aPriceTmp), "600.000");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "You will have shotgun if you respawn.\n"
			"For more information about spawn weapons,\n"
			"please visit '/spawnweaponsinfo'.");
	}
	else if (m_ShopWindowPage == 11)
	{
		str_format(aItem, sizeof(aItem), "      ~  S P A W N G R E N A D E  ~    ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "33");
		str_format(aPriceTmp, sizeof(aPriceTmp), "600.000");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "You will have grenade if you respawn.\n"
			"For more information about spawn weapons,\n"
			"please visit '/spawnweaponsinfo'.");
	}
	else if (m_ShopWindowPage == 12)
	{
		str_format(aItem, sizeof(aItem), "       ~  S P A W N R I F L E  ~       ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "33");
		str_format(aPriceTmp, sizeof(aPriceTmp), "600.000");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "You will have rifle if you respawn.\n"
			"For more information about spawn weapons,\n"
			"please visit '/spawnweaponsinfo'.");
	}
	else if (m_ShopWindowPage == 13)
	{
		str_format(aItem, sizeof(aItem), "       ~  S P O O K Y G H O S T  ~     ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "1");
		str_format(aPriceTmp, sizeof(aPriceTmp), "1.000.000");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "Using this item you can hide from other players behind bushes.\n"
			"If your ghost is activated you will be able to shoot plasma\n"
			"projectiles. For more information please visit '/spookyghostinfo'.");
	}
	else
	{
		str_format(aItem, sizeof(aItem), "");
	}
	//////////////////// UPDATE m_MaxShopPage ON TOP OF THIS FUNCTION!!! /////////////////////////


	char aLevel[128];
	str_format(aLevel, sizeof(aLevel), "Needed level: %s", aLevelTmp);
	char aPrice[16];
	str_format(aPrice, sizeof(aPrice), "Price: %s", aPriceTmp);
	char aTime[256];
	str_format(aTime, sizeof(aTime), "Time: %s", aTimeTmp);


	char aBase[512];
	if (m_ShopWindowPage > 0)
	{
		str_format(aBase, sizeof(aBase),
			"***************************\n"
			"        ~  S H O P  ~      \n"
			"***************************\n\n"
			"%s\n\n"
			"%s\n"
			"%s\n"
			"%s\n\n"
			"%s\n\n"
			"***************************\n"
			"If you want to buy an item press f3.\n\n\n"
			"              ~ %d ~              ", aItem, aLevel, aPrice, aTime, aInfo, m_ShopWindowPage);
	}
	else
	{
		str_format(aBase, sizeof(aBase),
			"***************************\n"
			"        ~  S H O P  ~      \n"
			"***************************\n\n"
			"%s\n\n"
			"***************************\n"
			"If you want to buy an item press f3.", aItem);
	}

	GameServer()->AbuseMotd(aBase, GetPlayer()->GetCID());

	m_ShopMotdTick = Server()->Tick() + Server()->TickSpeed() * 10; // motd is there for 10 sec

	return;
}

void CCharacter::StartShop()
{
	if (!m_InShop)
		return;
	if (m_PurchaseState == 2) // already in buy confirmation state
		return;
	if (m_ShopWindowPage != -1)
		return;

	ShopWindow(0);
	m_PurchaseState = 1;
}

void CCharacter::ConfirmPurchase()
{
	if ((m_ShopWindowPage == -1) || (m_ShopWindowPage == 0))
		return;

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf),
		"***************************\n"
		"        ~  S H O P  ~      \n"
		"***************************\n\n"
		"Are you sure you want to buy this item?\n\n"
		"f3 - yes\n"
		"f4 - no\n\n"
		"***************************\n");

	GameServer()->AbuseMotd(aBuf, GetPlayer()->GetCID());

	m_PurchaseState = 2;

	return;
}

void CCharacter::PurchaseEnd(bool canceled)
{
	if (m_PurchaseState != 2) // nothing to end here
		return;

	char aResult[256];
	if (canceled)
	{
		char aBuf[256];
		str_format(aResult, sizeof(aResult), "You canceled the purchase.");
		str_format(aBuf, sizeof(aBuf),
			"***************************\n"
			"        ~  S H O P  ~      \n"
			"***************************\n\n"
			"%s\n\n"
			"***************************\n", aResult);

		GameServer()->AbuseMotd(aBuf, GetPlayer()->GetCID());
	}
	else
	{
		BuyItem(m_ShopWindowPage);
		ShopWindow(0);
	}

	m_PurchaseState = 1;

	return;
}

void CCharacter::BuyItem(int ItemID)
{

	if ((g_Config.m_SvShopState == 1) && !m_InShop)
	{
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You have to be in the shop to buy some items.");
		return;
	}

	char aBuf[256];

	if (ItemID == 1)
	{
		if (m_Rainbow)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already own rainbow.");
			return;
		}

		if (m_pPlayer->GetLevel() < 5)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Your level is too low! You need to be Lv.5 to buy rainbow.");
		}
		else
		{
			if (m_pPlayer->GetMoney() >= 1500)
			{
				m_pPlayer->MoneyTransaction(-1500, "bought 'rainbow'");
				m_Rainbow = true;
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought rainbow until death.");
			}
			else
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money! You need 1.500 money.");
			}
		}
	}
	else if (ItemID == 2)
	{
		if (m_Bloody)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already own bloody.");
			return;
		}

		if (m_pPlayer->GetLevel() < 15)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Your level is too low! You need to be Lv.15 to buy bloody.");
		}
		else
		{
			if (m_pPlayer->GetMoney() >= 3500)
			{
				m_pPlayer->MoneyTransaction(-3500, "bought 'bloody'");
				m_Bloody = true;
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought bloody until death.");
			}
			else
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money! You need 3 500.");
			}
		}
	}
	else if (ItemID == 3)
	{
		if (m_pPlayer->GetLevel() < 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You need to be Lv.2 or higher to buy 'chidraqul'.");
			return;
		}
		if (m_pPlayer->m_BoughtGame)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already own this game.");
			return;
		}
		if (m_pPlayer->GetMoney() >= 250)
		{
			m_pPlayer->MoneyTransaction(-250, "bought 'chidraqul'");
			m_pPlayer->m_BoughtGame = true;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought 'chidraqul' until you disconnect. Check '/chidraqul info' for more information.");
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money! You need 250 money.");
		}
	}
	else if (ItemID == 4)
	{
		if (m_pPlayer->GetMoney() >= 5)
		{
			m_pPlayer->MoneyTransaction(-5, "bought 'shit'");

			m_pPlayer->m_shit++;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought shit.");
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money!");
		}
	}
	else if (ItemID == 5)
	{
		if (m_pPlayer->GetLevel() < 16)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You need to be Lv.16 or higher to buy a key.");
			return;
		}
		if (m_pPlayer->m_BoughtRoom)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already own this item.");
			return;
		}
		if (g_Config.m_SvRoomState == 0)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Room has been turned off by admin.");
			return;
		}
		if (m_pPlayer->GetMoney() >= g_Config.m_SvRoomPrice)
		{
			m_pPlayer->MoneyTransaction(-g_Config.m_SvRoomPrice, "bought 'room_key'");
			m_pPlayer->m_BoughtRoom = true;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought a key. You can now enter the bankroom until you disconnect.");
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money! You need 5.000 money.");
		}
	}
	else if (ItemID == 6)
	{
		if (m_pPlayer->m_PoliceRank == 0)
		{
			if (m_pPlayer->GetLevel() < 18)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 18 to buy police.");
				return;
			}
		}
		else if (m_pPlayer->m_PoliceRank == 1)
		{
			if (m_pPlayer->GetLevel() < 25)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 25 to upgrade police to level 2.");
				return;
			}
		}
		else if (m_pPlayer->m_PoliceRank == 2)
		{
			if (m_pPlayer->GetLevel() < 30)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 30 to upgrade police to level 3.");
				return;
			}
		}
		else if (m_pPlayer->m_PoliceRank == 3)
		{
			if (m_pPlayer->GetLevel() < 40)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 40 to upgrade police to level 4.");
				return;
			}
		}
		else if (m_pPlayer->m_PoliceRank == 4)
		{
			if (m_pPlayer->GetLevel() < 50)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 50 to upgrade police to level 5.");
				return;
			}
		}


		if (m_pPlayer->m_PoliceRank > 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already bought maximum police lvl.");
			return;
		}

		if (m_pPlayer->GetMoney() >= 100000)
		{
			m_pPlayer->MoneyTransaction(-100000, "bought 'police'");
			m_pPlayer->m_PoliceRank++;
			str_format(aBuf, sizeof(aBuf), "You bought PoliceRank[%d]!", m_pPlayer->m_PoliceRank);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Not enough money! You need 100.000 money.");
		}
	}
	else if (ItemID == 7)
	{
		if (m_pPlayer->m_PoliceRank < 3)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't own a weapon license.");
			return;
		}

		if (m_pPlayer->m_TaserLevel == 0)
		{
			m_pPlayer->m_TaserPrice = 50000;
		}
		else if (m_pPlayer->m_TaserLevel == 1)
		{
			m_pPlayer->m_TaserPrice = 75000;
		}
		else if (m_pPlayer->m_TaserLevel == 2)
		{
			m_pPlayer->m_TaserPrice = 100000;
		}
		else if (m_pPlayer->m_TaserLevel == 3)
		{
			m_pPlayer->m_TaserPrice = 150000;
		}
		else if (m_pPlayer->m_TaserLevel == 4)
		{
			m_pPlayer->m_TaserPrice = 200000;
		}
		else if (m_pPlayer->m_TaserLevel == 5)
		{
			m_pPlayer->m_TaserPrice = 200000;
		}
		else if (m_pPlayer->m_TaserLevel == 6)
		{
			m_pPlayer->m_TaserPrice = 200000;
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Taser already max level.");
			return;
		}

		if (m_pPlayer->GetMoney() < m_pPlayer->m_TaserPrice)
		{
			str_format(aBuf, sizeof(aBuf), "Not enough money to upgrade taser. You need %d money.", m_pPlayer->m_TaserPrice);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			return;
		}

		m_pPlayer->MoneyTransaction(-m_pPlayer->m_TaserPrice, "bought 'taser'");

		m_pPlayer->m_TaserLevel++;
		if (m_pPlayer->m_TaserLevel == 1)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought a taser. (Type '/taser help' for all cmds)");
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Taser has been upgraded.");
		}
	}
	else if (ItemID == 8)
	{
		if (m_pPlayer->GetMoney() >= 150)
		{
			m_pPlayer->MoneyTransaction(-150, "bought 'pvp_arena_ticket'");
			m_pPlayer->m_pvp_arena_tickets++;

			str_format(aBuf, sizeof(aBuf), "You bought a pvp_arena_ticket. You have %d tickets.", m_pPlayer->m_pvp_arena_tickets);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money! You need 150 money.");
		}
	}
	else if (ItemID == 9)
	{
		if (m_pPlayer->GetLevel() < 21)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 21 to buy ninjajetpack.");
			return;
		}
		else if (m_pPlayer->m_NinjaJetpackBought)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already own ninjajetpack.");
		}
		else if (m_pPlayer->GetMoney() >= 10000)
		{
			m_pPlayer->MoneyTransaction(-10000, "bought 'ninjajetpack'");

			m_pPlayer->m_NinjaJetpackBought = 1;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought ninjajetpack. Turn it on using '/ninjajetpack'.");
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money!");
		}
	}
	else if (ItemID == 10)
	{
		if (m_pPlayer->GetLevel() < 33)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 33 to buy spawn shotgun.");
			return;
		}
		else if (m_pPlayer->m_SpawnWeaponShotgun == 5)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already have the maximum level for spawn shotgun.");
		}
		else if (m_pPlayer->GetMoney() >= 600000)
		{
			m_pPlayer->MoneyTransaction(-600000, "bought 'spawn_shotgun'");

			m_pPlayer->m_SpawnWeaponShotgun++;
			if (m_pPlayer->m_SpawnWeaponShotgun == 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought spawn shotgun. For more infos check '/spawnweaponsinfo'.");
			}
			else if (m_pPlayer->m_SpawnWeaponShotgun > 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Spawn shotgun upgraded.");
			}
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money!");
		}
	}
	else if (ItemID == 11)
	{
		if (m_pPlayer->GetLevel() < 33)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 33 to buy spawn grenade.");
			return;
		}
		else if (m_pPlayer->m_SpawnWeaponGrenade == 5)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already have the maximum level for spawn grenade.");
		}
		else if (m_pPlayer->GetMoney() >= 600000)
		{
			m_pPlayer->MoneyTransaction(-600000, "bought 'spawn_grenade'");

			m_pPlayer->m_SpawnWeaponGrenade++;
			if (m_pPlayer->m_SpawnWeaponGrenade == 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought spawn grenade. For more infos check '/spawnweaponsinfo'.");
			}
			else if (m_pPlayer->m_SpawnWeaponGrenade > 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Spawn grenade upgraded.");
			}
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money!");
		}
	}
	else if (ItemID == 12)
	{
		if (m_pPlayer->GetLevel() < 33)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 33 to buy spawn rifle.");
			return;
		}
		else if (m_pPlayer->m_SpawnWeaponRifle == 5)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already have the maximum level for spawn rifle.");
		}
		else if (m_pPlayer->GetMoney() >= 600000)
		{
			m_pPlayer->MoneyTransaction(-600000, "bought 'spawn_rifle'");

			m_pPlayer->m_SpawnWeaponRifle++;
			if (m_pPlayer->m_SpawnWeaponRifle == 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought spawn rifle. For more infos check '/spawnweaponsinfo'.");
			}
			else if (m_pPlayer->m_SpawnWeaponRifle > 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Spawn rifle upgraded.");
			}
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money!");
		}
	}
	else if (ItemID == 13)
	{
		if (m_pPlayer->GetLevel() < 1)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 1 to buy the spooky ghost.");
			return;
		}
		else if (m_pPlayer->m_SpookyGhost)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already have the spooky ghost.");
		}
		else if (m_pPlayer->GetMoney() >= 1000000)
		{
			m_pPlayer->MoneyTransaction(-1000000, "bought 'spooky_ghost'");

			m_pPlayer->m_SpookyGhost = 1;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought the spooky ghost. For more infos check '/spookyghostinfo'.");
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money!");
		}
	}
	else
	{
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Invalid shop item. Choose another one.");
	}
	
	/*else if (!str_comp_nocase(aItem, "atom"))
	{
	if (pPlayer->GetCharacter()->m_Atom)
	{
	pSelf->SendChatTarget(pResult->m_ClientID, "You already own atom");
	return;
	}

	if (pPlayer->GetLevel() < 15)
	{
	pSelf->SendChatTarget(pResult->m_ClientID, "your level is too low! you need level 15 to buy atom.");
	}
	else
	{
	if (pPlayer->GetMoney() >= 3500)
	{
	pPlayer->MoneyTransaction(-3500, "bought pvp_arena_ticket");
	pPlayer->GetCharacter()->m_Atom = true;
	pSelf->SendChatTarget(pResult->m_ClientID, "you bought atom until death.");
	}
	else
	{
	pSelf->SendChatTarget(pResult->m_ClientID, "you don't have enough money! You need 3 500.");
	}
	}
	}
	else if (!str_comp_nocase(aItem, "trail"))
	{
	if (pPlayer->GetCharacter()->m_Trail)
	{
	pSelf->SendChatTarget(pResult->m_ClientID, "you already own trail");
	return;
	}

	if (pPlayer->GetLevel() < 15)
	{
	pSelf->SendChatTarget(pResult->m_ClientID, "your level is too low! you need level 15 to buy trail.");
	}
	else
	{
	if (pPlayer->GetMoney() >= 3500)
	{
	pPlayer->MoneyTransaction(-3500, "bought pvp_arena_ticket");
	pPlayer->GetCharacter()->m_Trail = true;
	pSelf->SendChatTarget(pResult->m_ClientID, "you bought trail until death.");
	}
	else
	{
	pSelf->SendChatTarget(pResult->m_ClientID, "you don't have enough money! You need 3 500.");
	}
	}
	}*/

	return;
}

void CCharacter::DropLoot()
{
	if (m_pPlayer->m_IsSurvivaling && !m_pPlayer->m_IsSurvivalLobby)
	{
		// survival weapon, health and weapon drops
		DropArmor(rand() % 6);
		DropHealth(rand() % 6);
		DropWeapon(WEAPON_GUN);
		DropWeapon(WEAPON_SHOTGUN);
		DropWeapon(WEAPON_GRENADE);
		DropWeapon(WEAPON_RIFLE);
	}
	else if (!GameServer()->IsMinigame(m_pPlayer->GetCID()))
	{
		int SpecialGun = 0;
		if (m_Jetpack || m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun)
			SpecialGun = 1;
		// block drop 0-2 weapons
		DropWeapon(rand() % (NUM_WEAPONS - (3+SpecialGun)) + (2-SpecialGun)); // no hammer or ninja and gun only if special gun
		DropWeapon(rand() % (NUM_WEAPONS - (3+SpecialGun)) + (2-SpecialGun));
	}
}

void CCharacter::DropHealth(int amount)
{
	if (amount > 64) { amount = 64; }
	for (int i = 0; i < amount; i++)
	{
		while (GameServer()->m_vDropLimit[POWERUP_HEALTH].size() > (long unsigned int)g_Config.m_SvMaxDrops)
		{
			GameServer()->m_vDropLimit[POWERUP_HEALTH][0]->Reset();
			GameServer()->m_vDropLimit[POWERUP_HEALTH].erase(GameServer()->m_vDropLimit[POWERUP_HEALTH].begin());
		}
		CDropPickup *p = new CDropPickup(
			&GameServer()->m_World,
			POWERUP_HEALTH,
			300, // lifetime
			m_pPlayer->GetCID(),
			rand() % 3 - 1, // direction
			(float)(amount / 5), // force
			Team()
		);
		GameServer()->m_vDropLimit[POWERUP_HEALTH].push_back(p);
	}
}

void CCharacter::DropArmor(int amount)
{
	if (amount > 64) { amount = 64; }
	for (int i = 0; i < amount; i++)
	{
		while (GameServer()->m_vDropLimit[POWERUP_ARMOR].size() > (long unsigned int)g_Config.m_SvMaxDrops)
		{
			GameServer()->m_vDropLimit[POWERUP_ARMOR][0]->Reset();
			GameServer()->m_vDropLimit[POWERUP_ARMOR].erase(GameServer()->m_vDropLimit[POWERUP_ARMOR].begin());
		}
		CDropPickup *p = new CDropPickup(
			&GameServer()->m_World,
			POWERUP_ARMOR,
			300, // lifetime
			m_pPlayer->GetCID(),
			rand() % 3 - 1, // direction
			(float)(amount / 5), // force
			Team()
		);
		GameServer()->m_vDropLimit[POWERUP_ARMOR].push_back(p);
	}
}

void CCharacter::DropWeapon(int WeaponID)
{

	if ((isFreezed) || (m_FreezeTime) || (!m_aWeapons[WeaponID].m_Got)
		|| (m_pPlayer->IsInstagibMinigame())
		|| (m_pPlayer->m_SpookyGhostActive && WeaponID != WEAPON_GUN)
		|| (WeaponID == WEAPON_NINJA)
		|| (WeaponID == WEAPON_HAMMER && !m_pPlayer->m_IsSurvivaling && g_Config.m_SvAllowDroppingWeapons != 1 && g_Config.m_SvAllowDroppingWeapons != 2)
		|| (WeaponID == WEAPON_GUN && !m_Jetpack && !m_autospreadgun && !m_pPlayer->m_InfAutoSpreadGun && !m_pPlayer->m_IsSurvivaling && g_Config.m_SvAllowDroppingWeapons != 1 && g_Config.m_SvAllowDroppingWeapons != 2)
		|| (WeaponID == WEAPON_RIFLE && (m_pPlayer->m_SpawnRifleActive || m_aDecreaseAmmo[WEAPON_RIFLE]) && g_Config.m_SvAllowDroppingWeapons != 1 && g_Config.m_SvAllowDroppingWeapons != 3)
		|| (WeaponID == WEAPON_SHOTGUN && (m_pPlayer->m_SpawnShotgunActive || m_aDecreaseAmmo[WEAPON_SHOTGUN]) && g_Config.m_SvAllowDroppingWeapons != 1 && g_Config.m_SvAllowDroppingWeapons != 3)
		|| (WeaponID == WEAPON_GRENADE && (m_pPlayer->m_SpawnGrenadeActive || m_aDecreaseAmmo[WEAPON_GRENADE]) && g_Config.m_SvAllowDroppingWeapons != 1 && g_Config.m_SvAllowDroppingWeapons != 3)
		)
	{
		return;
	}

	if (m_pPlayer->m_vWeaponLimit[WeaponID].size() == 5)
	{
		m_pPlayer->m_vWeaponLimit[WeaponID][0]->Reset();
		m_pPlayer->m_vWeaponLimit[WeaponID].erase(m_pPlayer->m_vWeaponLimit[WeaponID].begin());
	}

	int m_CountWeapons = 0;

	for (int i = 5; i > -1; i--)
	{
		if (m_aWeapons[i].m_Got)
			m_CountWeapons++;
	}

	if (WeaponID == WEAPON_GUN && (m_Jetpack || m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun))
	{
		if (m_Jetpack && (m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun))
		{
			m_Jetpack = false;
			m_autospreadgun = false;
			m_pPlayer->m_InfAutoSpreadGun = false;
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You lost your jetpack gun");
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You lost your spread gun");
			GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));

			CWeapon *Weapon = new CWeapon(&GameServer()->m_World, WeaponID, 300, m_pPlayer->GetCID(), GetAimDir(), Team(), m_aWeapons[WeaponID].m_Ammo, true, true);
			m_pPlayer->m_vWeaponLimit[WEAPON_GUN].push_back(Weapon);
		}
		else if (m_Jetpack)
		{
			m_Jetpack = false;
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You lost your jetpack gun");
			GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));

			CWeapon *Weapon = new CWeapon(&GameServer()->m_World, WeaponID, 300, m_pPlayer->GetCID(), GetAimDir(), Team(), m_aWeapons[WeaponID].m_Ammo, true);
			m_pPlayer->m_vWeaponLimit[WEAPON_GUN].push_back(Weapon);
		}
		else if (m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun)
		{
			m_autospreadgun = false;
			m_pPlayer->m_InfAutoSpreadGun = false;
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You lost your spread gun");
			GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));

			CWeapon *Weapon = new CWeapon(&GameServer()->m_World, WeaponID, 300, m_pPlayer->GetCID(), GetAimDir(), Team(), m_aWeapons[WeaponID].m_Ammo, false, true);
			m_pPlayer->m_vWeaponLimit[WEAPON_GUN].push_back(Weapon);
		}
	}
	else if (m_CountWeapons > 1)
	{
		m_aWeapons[WeaponID].m_Got = false;
		GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));

		CWeapon *Weapon = new CWeapon(&GameServer()->m_World, WeaponID, 300, m_pPlayer->GetCID(), GetAimDir(), Team(), m_aWeapons[WeaponID].m_Ammo);
		m_pPlayer->m_vWeaponLimit[WeaponID].push_back(Weapon);
	}

	SetWeaponThatChrHas();
	return;
}

void CCharacter::PvPArenaTick()
{
	if (m_pvp_arena_tele_request_time < 0)
		return;
	m_pvp_arena_tele_request_time--;

	if (m_pvp_arena_tele_request_time == 1)
	{
		if (m_pvp_arena_exit_request)
		{
			m_pPlayer->m_pvp_arena_tickets++;
			m_Health = 10;
			m_IsPVParena = false;
			m_isDmg = false;

			if (g_Config.m_SvPvpArenaState == 3) //tilebased and not hardcodet
			{
				m_Core.m_Pos = m_pPlayer->m_PVP_return_pos;
			}

			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[PVP] Successfully teleported out of arena.");
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[PVP] You got your ticket back because you have survived.");
		}
		else // join request
		{
			m_pPlayer->m_pvp_arena_tickets--;
			m_pPlayer->m_pvp_arena_games_played++;
			m_IsPVParena = true;
			m_isDmg = true;
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[PVP] Teleporting to arena... good luck and have fun!");
		}
	}

	if (m_Core.m_Vel.x < -0.02f || m_Core.m_Vel.x > 0.02f || m_Core.m_Vel.y != 0.0f)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[PVP] Teleport failed because you have moved.");
		m_pvp_arena_tele_request_time = -1;
	}
}

void CCharacter::DDPP_Tick()
{
	char aBuf[256];

	PvPArenaTick();

	if (m_RandomCosmetics)
	{
		if (Server()->Tick() % 22 == 0)
		{
			int r = rand() % 10;
			if (r == 0)
			{
				m_Rainbow ^= true;
			}
			else if (r == 1)
			{
				//m_StrongBloody ^= true;
			}
			else if (r == 2)
			{
				m_Bloody ^= true;
			}
			else if (r == 3)
			{
				m_Atom ^= true;
			}
			else if (r == 4)
			{
				m_Trail ^= true;
			}
			else if (r == 5)
			{
				m_autospreadgun ^= true;
			}
			else if (r > 8)
			{
				m_ninjasteam = true;
			}


			if (Server()->Tick() % 5 == 0 && m_ninjasteam)
			{
				m_ninjasteam = false;
			}
		}
	}

	if (GameServer()->m_BlockWaveGameState)
	{
		if (m_pPlayer->m_IsBlockWaving)
		{
			if (m_FreezeTime > 0 && !m_pPlayer->m_IsBlockWaveDead)
			{
				BlockWaveFreezeTicks++; //gets set to zer0 in Unfreeze() func
				if (BlockWaveFreezeTicks > Server()->TickSpeed() * 4)
				{
					str_format(aBuf, sizeof(aBuf), "[BlockWave] '%s' died.", Server()->ClientName(m_pPlayer->GetCID()));
					GameServer()->SendBlockWaveSay(aBuf);
					m_pPlayer->m_IsBlockWaveDead = true; //also gets set to zer0 on Unfreezefunc
				}
			}
		}
	}

	if (m_pPlayer->m_IsBlockTourning)
	{
		if (GameServer()->m_BlockTournaState == 2) //only do it ingame
		{
			if (m_FreezeTime)
			{
				m_BlockTournaDeadTicks++;
				if (m_BlockTournaDeadTicks > 15 * Server()->TickSpeed())
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}
			}
			else
			{
				m_BlockTournaDeadTicks = 0;
			}
		}
	}

	//spawnblock reducer
	if (Server()->Tick() % 1200 == 0 && m_pPlayer->m_SpawnBlocks > 0)
	{
		m_pPlayer->m_SpawnBlocks--;
	}

	//Block points (clear touchid on freeze and unfreeze agian)
	//if (m_pPlayer->m_LastToucherID != -1 && isFreezed) //didn't use m_FreezeTime because we want a freeze tile here not an freezelaser or something (idk about freeze canons)
	//{
	//	m_pPlayer->m_BlockWasTouchedAndFreezed = true;
	//}
	//if (m_pPlayer->m_BlockWasTouchedAndFreezed && m_FreezeTime == 0) //player got touched and freezed and unfreezed agian --> reset toucher because it isnt his kill anymore
	//{
	//	m_pPlayer->UpdateLastToucher(-1);
	//}
	//Better system: Remove LastToucherID after some unfreeze time this has less bugs and works also good in other situations like: your racing with your mate and then you rush away solo and fail and suicide (this situation wont count as kill). 
	if (m_pPlayer->m_LastToucherID != -1 && m_FreezeTime == 0)
	{
		//char aBuf[64];
		//str_format(aBuf, sizeof(aBuf), "ID: %d is not -1", m_pPlayer->m_LastToucherID); //ghost debug
		//dbg_msg("block", aBuf);

		m_pPlayer->m_LastTouchTicks++;
		if (m_pPlayer->m_LastTouchTicks > Server()->TickSpeed() * 3) //3 seconds unfreeze --> wont die block death on freeze suicide
		{
			//char aBuf[64];
			//str_format(aBuf, sizeof(aBuf), "'%s' [ID: %d] touch removed", Server()->ClientName(m_pPlayer->m_LastToucherID), m_pPlayer->m_LastToucherID);
			//GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			m_pPlayer->UpdateLastToucher(-1);
		}
	}

	/*
	// wtf why did i code that? xd
	// wait until blocker disconnects to not count as blocked ?!?
	//clear last toucher on disconnect/unexistance
	if (!GameServer()->m_apPlayers[m_pPlayer->m_LastToucherID])
	{
		m_pPlayer->UpdateLastToucher(-1);
	}
	*/

	//Block points (check for last touched player)
	//pikos hook check
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CCharacter *pChar = GameServer()->GetPlayerChar(i);

		if (!pChar || !pChar->IsAlive() || pChar == this)
			continue;
		if (pChar->Core()->m_HookedPlayer == m_pPlayer->GetCID())
		{
			m_pPlayer->UpdateLastToucher(i);

			//was debugging because somekills at spawn werent recongized. But now i know that the dummys just kill to fast even before getting freeze --> not a block kill. But im ok with it spawnblock farming bots isnt nice anyways
			//dbg_msg("debug", "[%d:%s] hooked [%d:%s]", i, Server()->ClientName(i), m_pPlayer->GetCID(), Server()->ClientName(m_pPlayer->GetCID()));
		}
	}
	if (m_Core.m_HookState == HOOK_GRABBED)
	{
		//m_Dummy_nn_touched_by_humans = true;
		//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "dont get in my hook -.-");

		//Quest 2 level 8 Block 3 tees without using hook
		if (m_pPlayer->m_QuestState == CPlayer::QUEST_BLOCK && m_pPlayer->m_QuestStateLevel == 8)
		{
			if (m_pPlayer->m_QuestProgressValue)
			{
				//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] don't use hook!");
				GameServer()->QuestFailed(m_pPlayer->GetCID());
			}
		}
	}

	// selfmade nobo code check if pChr is too near
	CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
	if (pChr && pChr->IsAlive() && pChr->m_Input.m_Direction) // no afk killers pls
	{
		// only count touches from unfreezed & grounded tees
		if (pChr->m_FreezeTime == 0 && pChr->IsGrounded())
		{
			if (pChr->m_Pos.x < m_Core.m_Pos.x + 45 && pChr->m_Pos.x > m_Core.m_Pos.x - 45 && pChr->m_Pos.y < m_Core.m_Pos.y + 50 && pChr->m_Pos.y > m_Core.m_Pos.y - 50)
			{
				m_pPlayer->UpdateLastToucher(pChr->GetPlayer()->GetCID());
			}
		}
	}

	//hook extras
	//if (m_pPlayer->m_IsHookRainbow)
	//{

	//}


	//dbg_msg("", "koordinaten: x=%d y=%d", (int)(m_Pos.x / 32.f), (int)(m_Pos.y / 32.f));
	//survivexp stuff
	if (m_AliveTime)
	{
		if (Server()->Tick() >= m_AliveTime + Server()->TickSpeed() * 6000)  //100min
		{
			m_survivexpvalue = 4;
		}
		else if (Server()->Tick() >= m_AliveTime + Server()->TickSpeed() * 3600)  //60min
		{
			m_survivexpvalue = 3;
		}
		else if (Server()->Tick() >= m_AliveTime + Server()->TickSpeed() * 1200)  //20min
		{
			m_survivexpvalue = 2;
		}
		else if (Server()->Tick() >= m_AliveTime + Server()->TickSpeed() * 300) //5min
		{
			m_survivexpvalue = 1;
		}
	}

	DDPP_FlagTick();

	if (m_pPlayer->m_GiftDelay > 0)
	{
		m_pPlayer->m_GiftDelay--;
		if (m_pPlayer->m_GiftDelay == 1)
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[GIFT] delay expired.");
		}
	}

	if (m_pPlayer->m_JailTime > 0)
	{
		m_pPlayer->m_EscapeTime = 0;
		m_pPlayer->m_JailTime--;
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Your are arrested for %d seconds. \nType '/hide jail' to hide this info.", m_pPlayer->m_JailTime / Server()->TickSpeed());
		if (Server()->Tick() % 40 == 0)
		{
			if (!m_pPlayer->m_hidejailmsg)
			{
				GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
			}
		}
		if (m_pPlayer->m_JailTime == 1)
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[JAIL] You were released from jail.");
			vec2 JailReleaseSpawn = GameServer()->Collision()->GetRandomTile(TILE_JAILRELEASE);
			//vec2 DefaultSpawn = GameServer()->Collision()->GetRandomTile(ENTITY_SPAWN);

			if (JailReleaseSpawn != vec2(-1, -1))
			{
				SetPosition(JailReleaseSpawn);
			}
			else //no jailrelease
			{
				//SetPosition(DefaultSpawn); //crashbug for mod stealer
				GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[JAIL] no jail release spot found.");
			}
		}
	}

	if (m_pPlayer->m_EscapeTime > 0)
	{
		m_pPlayer->m_EscapeTime--;
		char aBuf[256];
		if (m_isDmg)
		{
			str_format(aBuf, sizeof(aBuf), "Avoid policehammers for the next %d seconds. \n!WARNING! DAMAGE IS ACTIVATED ON YOU!\nType '/hide jail' to hide this info.", m_pPlayer->m_EscapeTime / Server()->TickSpeed());
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "Avoid policehammers for the next %d seconds. \nType '/hide jail' to hide this info.", m_pPlayer->m_EscapeTime / Server()->TickSpeed());
		}

		if (Server()->Tick() % Server()->TickSpeed() * 60 == 0)
		{
			if (!m_pPlayer->m_hidejailmsg)
			{
				GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
			}
		}
		if (m_pPlayer->m_EscapeTime == 1)
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "Your life as a gangster is over. You are free now.");
			GameServer()->AddEscapeReason(GetPlayer()->GetCID(), "unknown");
			m_isDmg = false;
			m_Health = 10;
		}
	}

	if (g_Config.m_SvPvpArenaState == 1 || g_Config.m_SvPvpArenaState == 2) //the two old hardcodet maps ChillBlock5 and BlmapChill (new system uses tiles)
	{
		if (g_Config.m_SvPvpArenaState == 1) // ChillBlock5 pvp
		{
			if (m_IsPVParena)
			{
				if (m_Core.m_Pos.x > 414 * 32 && m_Core.m_Pos.x < 447 * 32 && m_Core.m_Pos.y < 175 * 32 && m_Core.m_Pos.y > 160 * 32) //in arena
				{

				}
				else //not in arena
				{

					m_pPlayer->m_PVP_return_posX = m_Core.m_Pos.x;
					m_pPlayer->m_PVP_return_posY = m_Core.m_Pos.y;


					//if not in arena tele to random arena spawn:

					int r = rand() % 3; // 0 1 2
					if (r == 0)
					{
						m_Core.m_Pos.x = 420 * 32;
						m_Core.m_Pos.y = 166 * 32 - 5;
					}
					else if (r == 1)
					{
						m_Core.m_Pos.x = 430 * 32;
						m_Core.m_Pos.y = 170 * 32;
					}
					else if (r == 2)
					{
						m_Core.m_Pos.x = 440 * 32;
						m_Core.m_Pos.y = 166 * 32 - 5;
					}

				}
			}
			else //not in pvp mode
			{
				if (m_Core.m_Pos.x > 414 * 32 && m_Core.m_Pos.x < 447 * 32 && m_Core.m_Pos.y < 175 * 32 && m_Core.m_Pos.y > 160 * 32) //in arena
				{
					m_Core.m_Pos.x = m_pPlayer->m_PVP_return_posX;
					m_Core.m_Pos.y = m_pPlayer->m_PVP_return_posY;
				}
				else //not in arena
				{

				}
			}
		}
		else if (g_Config.m_SvPvpArenaState == 2) // BlmapChill pvp
		{
			if (m_IsPVParena)
			{
				if (m_Core.m_Pos.x > 357 * 32 && m_Core.m_Pos.x < 369 * 32 && m_Core.m_Pos.y < 380 * 32 && m_Core.m_Pos.y > 364 * 32) //in arena
				{

				}
				else //not in arena
				{

					m_pPlayer->m_PVP_return_posX = m_Core.m_Pos.x;
					m_pPlayer->m_PVP_return_posY = m_Core.m_Pos.y;


					//if not in arena tele to random arena spawn:

					int r = rand() % 3; // 0 1 2
					if (r == 0)
					{
						m_Core.m_Pos.x = 360 * 32 + 44;
						m_Core.m_Pos.y = 379 * 32;
					}
					else if (r == 1)
					{
						m_Core.m_Pos.x = 366 * 32 + 53;
						m_Core.m_Pos.y = 379 * 32;
					}
					else if (r == 2)
					{
						m_Core.m_Pos.x = 363 * 32 + 53;
						m_Core.m_Pos.y = 373 * 32;
					}

				}
			}
			else //not in pvp mode
			{
				if (m_Core.m_Pos.x > 357 * 32 && m_Core.m_Pos.x < 369 * 32 && m_Core.m_Pos.y < 380 * 32 && m_Core.m_Pos.y > 364 * 32) //in arena
				{
					m_Core.m_Pos.x = m_pPlayer->m_PVP_return_posX;
					m_Core.m_Pos.y = m_pPlayer->m_PVP_return_posY;
				}
				else //not in arena
				{

				}
			}
		}
	}

	//Marcella's room code (used to slow down chat message spam)
	if (IsAlive() && (Server()->Tick() % 80) == 0 && m_WasInRoom)
	{
		m_WasInRoom = false;
	}

	if (Server()->Tick() % 200 == 0) //ddpp public slow tick
	{
		if (m_pPlayer->m_ShowInstaScoreBroadcast)
			m_UpdateInstaScoreBoard = true;
	}

	if (m_UpdateInstaScoreBoard) //gets printed on update or every 200 % whatever modulo ticks
	{
		if (m_pPlayer->m_IsInstaArena_gdm)
		{
			str_format(aBuf, sizeof(aBuf), "score: %04d/%04d                                                                                                                 0", m_pPlayer->m_InstaScore, g_Config.m_SvGrenadeScorelimit);
			GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
		}
		if (m_pPlayer->m_IsInstaArena_idm)
		{
			str_format(aBuf, sizeof(aBuf), "score: %04d/%04d                                                                                                                 0", m_pPlayer->m_InstaScore, g_Config.m_SvRifleScorelimit);
			GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
		}
	}
	m_UpdateInstaScoreBoard = false;

	//General var resetter by ChillerDragon [ I M P O R T A N T] leave var resetter last --> so it wont influence ddpp tick stuff
	if (Server()->Tick() % 20 == 0)
	{
		if (m_InBank)
		{
			if (m_TileIndex != TILE_BANK_IN && m_TileFIndex != TILE_BANK_IN)
			{
				GameServer()->SendBroadcast(" ", m_pPlayer->GetCID(), 0);
				m_InBank = false; // DDracePostCoreTick() (which handels tiles) is after DDPP_Tick() so while being in bank it will never be false because tiles are always stronger than DDPP tick        <---- this comment was made before the tile checker if clause but can be interesting for further resettings
			}
		}
		if (m_InShop)
		{
			if (m_TileIndex != TILE_SHOP && m_TileFIndex != TILE_SHOP)
			{
				if (m_pPlayer->m_ShopBotAntiSpamTick <= Server()->Tick())
				{
					SendShopMessage("Bye! Come back if you need something.");
					m_pPlayer->m_ShopBotAntiSpamTick = Server()->Tick() + Server()->TickSpeed() * 5;
				}

				if (m_ShopWindowPage != -1)
				{
					GameServer()->AbuseMotd("", GetPlayer()->GetCID());
				}

				GameServer()->SendBroadcast("", m_pPlayer->GetCID(), 0);

				m_PurchaseState = 0;
				m_ShopWindowPage = -1;

				m_InShop = false;
			}
		}
	}
	//fast resets
	m_InJailOpenArea = false;

}

void CCharacter::DDPP_FlagTick()
{
	if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) == -1)
		return;

	if (!m_pPlayer->IsLoggedIn())
		return; // GameServer()->SendBroadcast("You need an account to get xp from flags. \n Get an Account with '/register (name) (pw) (pw)'", m_pPlayer->GetCID());

	if (Server()->Tick() % 50 == 0)
	{
		if (((m_TileIndex == TILE_MONEY) || (m_TileFIndex == TILE_MONEY)))
			return;
		if (((m_TileIndex == TILE_MONEY_POLICE) || (m_TileFIndex == TILE_MONEY_POLICE)))
			return;
		if (((m_TileIndex == TILE_MONEY_DOUBLE) || (m_TileFIndex == TILE_MONEY_DOUBLE)))
			return;

		// no matter where (bank, moneytile, ...) quests are independent
		if (m_pPlayer->m_QuestState == CPlayer::QUEST_FARM)
		{
			if (m_pPlayer->m_QuestStateLevel == 9)
			{
				m_pPlayer->m_QuestProgressValue2++;
				if (m_pPlayer->m_QuestProgressValue2 > 20)
				{
					GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 10);
					m_pPlayer->m_QuestProgressValue2 = 0;
				}
			}
		}

		if (m_pPlayer->IsMaxLevel())
		{
			if (m_pPlayer->m_xpmsg)
			{
				GameServer()->SendBroadcast("[FLAG] You reached the maximum level.", m_pPlayer->GetCID(), 0);
			}
			return;
		}

		int VIPBonus = 0;

		// vip+ get 2 bonus
		if (m_pPlayer->m_IsSuperModerator)
		{
			m_pPlayer->GiveXP(2);
			m_pPlayer->MoneyTransaction(+2);

			VIPBonus = 2;
		}

		// vip get 1 bonus
		else if (m_pPlayer->m_IsModerator)
		{
			m_pPlayer->GiveXP(1);
			m_pPlayer->MoneyTransaction(+1);

			VIPBonus = 1;
		}

		if (m_InBank && GameServer()->m_IsBankOpen)
		{
			if (VIPBonus)
			{
				if (!m_pPlayer->m_xpmsg)
				{
					GameServer()->SendBroadcast("~ B A N K ~", m_pPlayer->GetCID(), 0);
					// GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You entered the bank. You can rob the bank with '/rob_bank'");  // lol no spam old unused commands pls
				}
				else if (m_survivexpvalue == 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ B A N K ~\nXP [%llu/%llu] +1 flag +%d vip", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus);
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					m_pPlayer->GiveXP(1);
				}
				else if (m_survivexpvalue > 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ B A N K ~\nXP [%llu/%llu] +1 flag +%d vip + %d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_survivexpvalue);
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					m_pPlayer->GiveXP(1); //flag
					m_pPlayer->GiveXP(m_survivexpvalue); // survival
				}
			}
			else
			{
				if (!m_pPlayer->m_xpmsg)
				{
					GameServer()->SendBroadcast("~ B A N K ~", m_pPlayer->GetCID(), 0);
				}
				else if (m_survivexpvalue == 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ B A N K ~\nXP [%llu/%llu] +1 flag", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP());
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					m_pPlayer->GiveXP(1);
				}
				else if (m_survivexpvalue > 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ B A N K ~\nXP [%llu/%llu] +1 flag +%d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_survivexpvalue);
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					m_pPlayer->GiveXP(1); //flag
					m_pPlayer->GiveXP(m_survivexpvalue); // survival
				}
			}
		}
		else if (m_InShop)
		{
			if (!m_pPlayer->m_xpmsg)
			{
				GameServer()->SendBroadcast("~ S H O P ~", m_pPlayer->GetCID(), 0);
			}
			else if (m_survivexpvalue == 0)
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "~ S H O P ~\nXP [%llu/%llu] +1 flag +%d vip", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus);
				GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
				m_pPlayer->GiveXP(1);
			}
			else if (m_survivexpvalue > 0)
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "~ S H O P ~\nXP [%llu/%llu] +1 flag +%d vip + %d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_survivexpvalue);
				GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
				m_pPlayer->GiveXP(1); //flag
				m_pPlayer->GiveXP(m_survivexpvalue); // survival
			}
			else
			{
				if (!m_pPlayer->m_xpmsg)
				{
					GameServer()->SendBroadcast("~ S H O P ~", m_pPlayer->GetCID(), 0);
				}
				else if (m_survivexpvalue == 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ S H O P ~\nXP [%llu/%llu] +1 flag", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP());
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					m_pPlayer->GiveXP(1);
				}
				else if (m_survivexpvalue > 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ S H O P ~\nXP [%llu/%llu] +1 flag +%d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_survivexpvalue);
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					m_pPlayer->GiveXP(1); //flag
					m_pPlayer->GiveXP(m_survivexpvalue); // survival
				}
			}
		}
		else  //not in bank
		{
			if (VIPBonus)
			{
				if (m_pPlayer->m_xpmsg)
				{
					if (m_survivexpvalue == 0)
					{
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "XP [%llu/%llu] +1 flag +%d vip", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
						m_pPlayer->GiveXP(1);
					}
					else if (m_survivexpvalue > 0)
					{
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "XP [%llu/%llu] +1 flag +%d vip +%d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_survivexpvalue);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
						m_pPlayer->GiveXP(1); //flag
						m_pPlayer->GiveXP(m_survivexpvalue); // survival
					}
				}
			}
			else
			{
				if (m_pPlayer->m_xpmsg)
				{
					if (m_survivexpvalue == 0)
					{
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "XP [%llu/%llu] +1 flag", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP());
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
						m_pPlayer->GiveXP(1);
					}
					else if (m_survivexpvalue > 0)
					{
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "XP [%llu/%llu] +1 flag +%d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_survivexpvalue);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
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

	if(!GameServer()->m_pController->CanSpawn(m_pPlayer->GetTeam(), &SpawnPos, m_pPlayer))
		return false;

	if (Server()->IsRecording(m_pPlayer->GetCID()))
		Server()->StopRecord(m_pPlayer->GetCID());

	SetPosition(SpawnPos);
	return true;
}

int CCharacter::DDPP_DIE(int Killer, int Weapon, bool fngscore)
{
	char aBuf[256];

	if (m_pPlayer->m_IsVanillaModeByTile) //reset vanilla mode but never go out of vanilla mode in survival
	{
		m_pPlayer->m_IsVanillaDmg = false;
		m_pPlayer->m_IsVanillaWeapons = false;
	}
	if (m_pPlayer->m_IsSurvivaling)
	{
		m_pPlayer->m_IsVanillaDmg = true;
		m_pPlayer->m_IsVanillaWeapons = true;
		m_pPlayer->m_IsVanillaCompetetive = true;
	}

	if (m_pPlayer->m_IsDummy && m_pPlayer->m_DummyMode == 33) //chillintelligenz
	{
		CIRestart();
	}

	// remove atom projectiles on death
	if (!m_AtomProjs.empty())
	{
		for (std::vector<CStableProjectile *>::iterator it = m_AtomProjs.begin(); it != m_AtomProjs.end(); ++it)
		{
			GameServer()->m_World.DestroyEntity(*it);
		}
		m_AtomProjs.clear();
	}

	// remove trail projectiles on death
	if (!m_TrailProjs.empty())
	{
		for (std::vector<CStableProjectile *>::iterator it = m_TrailProjs.begin(); it != m_TrailProjs.end(); ++it)
		{
			GameServer()->m_World.DestroyEntity(*it);
		}
		m_TrailProjs.clear();
	}

	Killer = BlockPointsMain(Killer, fngscore);
	BlockSpawnProt(Killer); //idk if this should be included in BlockPointsMain() but spawnkills no matter what kind are evil i guess but then we should rename it to SpawnKillProt() imo
	//BlockQuestSubDieFuncBlockKill(Killer); //leave this before killing sprees to also have information about killingspree values from dead tees (needed for quest2 lvl6) //included in BlockPointsMain because it handels block kills
	BlockQuestSubDieFuncDeath(Killer); //only handling quest failed (using external func because the other player is needed and its good to extract it in antoher func and because im funcy now c:) //new reason the first func is blockkill and this one is all kinds of death
	KillingSpree(Killer); // previously called BlockKillingSpree()
	BlockTourna_Die(Killer);
	DropLoot(); // has to be called before survival because it only droops loot if survival alive
	InstagibSubDieFunc(Killer, Weapon);
	SurvivalSubDieFunc(Killer, Weapon);

	if (GameServer()->IsDDPPgametype("battlegores"))
		if (GameServer()->m_apPlayers[Killer] && Killer != m_pPlayer->GetCID())
			GameServer()->m_apPlayers[Killer]->m_Score++;

	// TODO: combine with insta 1on1
	// insta kills
	if (Killer != m_pPlayer->GetCID() && GameServer()->m_apPlayers[Killer])
	{
		if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm || GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm)
		{
			GameServer()->DoInstaScore(3, Killer);
		}
		else if (GameServer()->IsDDPPgametype("fng"))
		{
			GameServer()->m_apPlayers[Killer]->m_Score += 3;
		}
	}

	// TODO: refactor this code and put it in own function
	// insta 1on1
	if (GameServer()->m_apPlayers[Killer])
	{
		if (GameServer()->m_apPlayers[Killer]->m_Insta1on1_id != -1 && Killer != m_pPlayer->GetCID() && (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm || GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm)) //is in 1on1
		{
			GameServer()->m_apPlayers[Killer]->m_Insta1on1_score++;
			str_format(aBuf, sizeof(aBuf), "%s:%d killed %s:%d", Server()->ClientName(Killer), GameServer()->m_apPlayers[Killer]->m_Insta1on1_score, Server()->ClientName(m_pPlayer->GetCID()), m_pPlayer->m_Insta1on1_score);
			if (!GameServer()->m_apPlayers[Killer]->m_HideInsta1on1_killmessages)
			{
				GameServer()->SendChatTarget(Killer, aBuf);
			}
			if (!m_pPlayer->m_HideInsta1on1_killmessages)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			}
			if (GameServer()->m_apPlayers[Killer]->m_Insta1on1_score >= 5)
			{
				GameServer()->WinInsta1on1(Killer, m_pPlayer->GetCID());
			}
		}
	}

	// TODO: refactor this code and put it in own function
	// balance battle
	if (m_pPlayer->m_IsBalanceBatteling && GameServer()->m_BalanceBattleState == 2) //ingame in a balance battle
	{
		if (GameServer()->m_BalanceID1 == m_pPlayer->GetCID())
		{
			if (GameServer()->m_apPlayers[GameServer()->m_BalanceID2])
			{
				GameServer()->SendChatTarget(GameServer()->m_BalanceID2, "[balance] you won!");
				GameServer()->SendChatTarget(GameServer()->m_BalanceID1, "[balance] you lost!");
				GameServer()->m_apPlayers[GameServer()->m_BalanceID1]->m_IsBalanceBatteling = false;
				GameServer()->m_apPlayers[GameServer()->m_BalanceID2]->m_IsBalanceBatteling = false;
				GameServer()->m_BalanceBattleState = 0;
				if (GameServer()->GetPlayerChar(GameServer()->m_BalanceID2))
				{
					GameServer()->GetPlayerChar(GameServer()->m_BalanceID2)->Die(GameServer()->m_BalanceID2, WEAPON_SELF);
				}
				//dbg_msg("balance", "%s:%d lost and %s:%d got killed too", Server()->ClientName(GameServer()->m_BalanceID1), GameServer()->m_BalanceID1, Server()->ClientName(GameServer()->m_BalanceID2), GameServer()->m_BalanceID2);
				GameServer()->StopBalanceBattle();
			}
		}
		else if (GameServer()->m_BalanceID2 == m_pPlayer->GetCID())
		{
			if (GameServer()->m_apPlayers[GameServer()->m_BalanceID1])
			{
				GameServer()->SendChatTarget(GameServer()->m_BalanceID1, "[balance] you won!");
				GameServer()->SendChatTarget(GameServer()->m_BalanceID2, "[balance] you lost!");
				GameServer()->m_apPlayers[GameServer()->m_BalanceID1]->m_IsBalanceBatteling = false;
				GameServer()->m_apPlayers[GameServer()->m_BalanceID2]->m_IsBalanceBatteling = false;
				GameServer()->m_BalanceBattleState = 0;
				if (GameServer()->GetPlayerChar(GameServer()->m_BalanceID1))
				{
					GameServer()->GetPlayerChar(GameServer()->m_BalanceID1)->Die(GameServer()->m_BalanceID1, WEAPON_SELF);
				}
				//dbg_msg("balance", "%s:%d lost and %s:%d got killed too", Server()->ClientName(GameServer()->m_BalanceID2), GameServer()->m_BalanceID2, Server()->ClientName(GameServer()->m_BalanceID1), GameServer()->m_BalanceID1);
				GameServer()->StopBalanceBattle();
			}
		}
	}

	// TODO: refactor this code and put it in own function
	// ChillerDragon pvparena code
	if (GameServer()->m_apPlayers[Killer])
	{
		if (GameServer()->GetPlayerChar(Killer) && Weapon != WEAPON_GAME && Weapon != WEAPON_SELF)
		{
			//GameServer()->GetPlayerChar(Killer)->m_Bloody = true;

			if (GameServer()->GetPlayerChar(Killer)->m_IsPVParena)
			{
				if (GameServer()->m_apPlayers[Killer]->IsMaxLevel() ||
					GameServer()->IsSameIP(Killer, m_pPlayer->GetCID()) || // dont give xp on dummy kill
					GameServer()->IsSameIP(m_pPlayer->GetCID(), GameServer()->m_apPlayers[Killer]->m_pvp_arena_last_kill_id) // dont give xp on killing same ip twice in a row
					)
				{
					GameServer()->m_apPlayers[Killer]->MoneyTransaction(+150, "pvp_arena kill");
					GameServer()->m_apPlayers[Killer]->m_pvp_arena_kills++;

					str_format(aBuf, sizeof(aBuf), "[PVP] +150 money for killing %s", Server()->ClientName(m_pPlayer->GetCID()));
					GameServer()->SendChatTarget(Killer, aBuf);
				}
				else
				{
					GameServer()->m_apPlayers[Killer]->MoneyTransaction(+150, "pvp_arena kill");
					GameServer()->m_apPlayers[Killer]->GiveXP(100);
					GameServer()->m_apPlayers[Killer]->m_pvp_arena_kills++;

					str_format(aBuf, sizeof(aBuf), "[PVP] +100 xp +150 money for killing %s", Server()->ClientName(m_pPlayer->GetCID()));
					GameServer()->SendChatTarget(Killer, aBuf);
				}

				int r = rand() % 100;
				if (r > 92)
				{
					GameServer()->m_apPlayers[Killer]->m_pvp_arena_tickets++;
					GameServer()->SendChatTarget(Killer, "[PVP] +1 pvp_arena_ticket        (special random drop for kill)");
				}
				GameServer()->m_apPlayers[Killer]->m_pvp_arena_last_kill_id = m_pPlayer->GetCID();
			}
		}
	}
	if (m_pPlayer) //victim
	{
		//m_pPlayer->m_InfRainbow = true;
		if (m_IsPVParena)
		{
			m_pPlayer->m_pvp_arena_deaths++;

			str_format(aBuf, sizeof(aBuf), "[PVP] You lost the arena-fight because you were killed by %s.", Server()->ClientName(Killer));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
		}
	}

	//bomb
	if (m_IsBombing)
	{
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[BOMB] you lost bomb because you died.");
	}

	m_pPlayer->UpdateLastToucher(-1);
	return Killer;
}

void CCharacter::BlockTourna_Die(int Killer)
{
	char aBuf[128];

	//Block tourna
	if (GameServer()->m_BlockTournaState == 2) //ingame
	{
		if (m_pPlayer->m_IsBlockTourning)
		{
			//update skill levels
			if (m_pPlayer->GetCID() == Killer) //selfkill
			{
				GameServer()->UpdateBlockSkill(-40, Killer);
			}
			else
			{
				int deadskill = m_pPlayer->m_BlockSkill;
				int killskill = GameServer()->m_apPlayers[Killer]->m_BlockSkill;
				int skilldiff = abs(deadskill - killskill);
				if (skilldiff < 1500) //pretty same skill lvl
				{
					if (deadskill < killskill) //the killer is better
					{
						GameServer()->UpdateBlockSkill(-29, m_pPlayer->GetCID()); //killed
						GameServer()->UpdateBlockSkill(+30, Killer); //killer
					}
					else //the killer is worse
					{
						GameServer()->UpdateBlockSkill(-40, m_pPlayer->GetCID()); //killed
						GameServer()->UpdateBlockSkill(+40, Killer); //killer
					}
				}
				else //unbalanced skill lvl --> punish harder and reward nicer
				{
					if (deadskill < killskill) //the killer is better
					{
						GameServer()->UpdateBlockSkill(-19, m_pPlayer->GetCID()); //killed
						GameServer()->UpdateBlockSkill(+20, Killer); //killer
					}
					else //the killer is worse
					{
						GameServer()->UpdateBlockSkill(-60, m_pPlayer->GetCID()); //killed
						GameServer()->UpdateBlockSkill(+60, Killer); //killer
					}
				}
			}

			//let him die and check for tourna win
			m_pPlayer->m_IsBlockTourning = false;
			int wonID = GameServer()->CountBlockTournaAlive();

			if (wonID == -404)
			{
				str_format(aBuf, sizeof(aBuf), "[BLOCK] error %d", wonID);
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				GameServer()->m_BlockTournaState = 0;
			}
			else if (wonID < 0 || wonID == -420)
			{
				if (wonID == -420)
				{
					wonID = 0;
				}
				wonID *= -1;
				str_format(aBuf, sizeof(aBuf), "[BLOCK] '%s' won the tournament (%d players).", Server()->ClientName(wonID), GameServer()->m_BlockTournaStartPlayers);
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				GameServer()->m_BlockTournaState = 3; //set end state


													  //give price to the winner
				int xp_rew;
				int points_rew;
				int money_rew;
				int skill_rew;
				if (GameServer()->m_BlockTournaStartPlayers <= 5) //depending on how many tees participated
				{
					xp_rew = 100;
					points_rew = 3;
					money_rew = 50;
					skill_rew = 10;
				}
				else if (GameServer()->m_BlockTournaStartPlayers <= 10)
				{
					xp_rew = 150;
					points_rew = 5;
					money_rew = 100;
					skill_rew = 20;
				}
				else if (GameServer()->m_BlockTournaStartPlayers <= 15)
				{
					xp_rew = 300;
					points_rew = 10;
					money_rew = 200;
					skill_rew = 30;
				}
				else if (GameServer()->m_BlockTournaStartPlayers <= 32)
				{
					xp_rew = 700;
					points_rew = 25;
					money_rew = 500;
					skill_rew = 120;
				}
				else if (GameServer()->m_BlockTournaStartPlayers <= 44)
				{
					xp_rew = 1200;
					points_rew = 30;
					money_rew = 1000;
					skill_rew = 400;
				}
				else
				{
					xp_rew = 25000;
					points_rew = 100;
					money_rew = 15000;
					skill_rew = 900;
				}

				str_format(aBuf, sizeof(aBuf), "[BLOCK] +%d xp", xp_rew);
				GameServer()->SendChatTarget(wonID, aBuf);
				str_format(aBuf, sizeof(aBuf), "[BLOCK] +%d money", money_rew);
				GameServer()->SendChatTarget(wonID, aBuf);
				str_format(aBuf, sizeof(aBuf), "[BLOCK] +%d points", points_rew);
				GameServer()->SendChatTarget(wonID, aBuf);

				GameServer()->m_apPlayers[wonID]->MoneyTransaction(+money_rew, "block tournament");
				GameServer()->m_apPlayers[wonID]->GiveXP(xp_rew);
				GameServer()->m_apPlayers[wonID]->GiveBlockPoints(points_rew);
				GameServer()->UpdateBlockSkill(+skill_rew, wonID);
			}
			else if (wonID == 0)
			{
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, "[BLOCK] nobody won the tournament");
				GameServer()->m_BlockTournaState = 0;
			}
			else if (wonID > 1)
			{
				str_format(aBuf, sizeof(aBuf), "[BLOCK] you died and placed as rank %d in the tournament", wonID + 1);
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "[BLOCK] error %d", wonID);
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				GameServer()->m_BlockTournaState = 0;
			}
		}
	}
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

void CCharacter::FreezeAll(int seconds)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetCharacter())
		{
			GameServer()->m_apPlayers[i]->GetCharacter()->Freeze(seconds);
		}
	}
}

bool CCharacter::HasWeapon(int weapon)
{
	if (m_aWeapons[weapon].m_Got)
	{
		return true;
	}
	return false;
}

void CCharacter::KillSpeed()
{
	m_Core.m_Vel.x = 0.0f;
	m_Core.m_Vel.y = 0.0f;
}

void CCharacter::InstagibKillingSpree(int KillerID, int Weapon)
{
	char aBuf[128];

	//killingspree system by FruchtiHD and ChillerDragon stolen from twlevel (edited by ChillerDragon)
	CCharacter *pVictim = m_pPlayer->GetCharacter();
	CPlayer *pKiller = GameServer()->m_apPlayers[KillerID];
	if (GameServer()->CountConnectedPlayers() >= g_Config.m_SvSpreePlayers) //only count killing sprees if enough players are online (also counting spectators)
	{
		if (pVictim && pKiller)
		{

			if (Weapon == WEAPON_GAME)
			{
				if (pVictim->GetPlayer()->m_KillStreak >= 5)
				{
					//Check for new highscore
					if (g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2) //gdm & zCatch grenade
					{
						//dbg_msg("insta", "checking for highscore grenade");
						if (pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_GrenadeSpree)
						{
							pVictim->GetPlayer()->m_GrenadeSpree = pVictim->GetPlayer()->m_KillStreak;
							GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCID(), "New grenade Killingspree record!");
						}
						//str_format(aBuf, sizeof(aBuf), "last: %d top: %d", pVictim->GetPlayer()->m_KillStreak, pVictim->GetPlayer()->m_GrenadeSpree);
						//dbg_msg("insta", aBuf);
					}
					else if (g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4) // idm & zCatch rifle
					{
						//dbg_msg("insta", "checking for highscore rifle");
						if (pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_RifleSpree)
						{
							pVictim->GetPlayer()->m_RifleSpree = pVictim->GetPlayer()->m_KillStreak;
							GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCID(), "New rifle Killingspree record!");
						}
						//str_format(aBuf, sizeof(aBuf), "last: %d top: %d", pVictim->GetPlayer()->m_KillStreak, pVictim->GetPlayer()->m_GrenadeSpree);
						//dbg_msg("insta", aBuf);
					}

					str_format(aBuf, sizeof(aBuf), "%s's killingspree was ended by %s (%d Kills)", Server()->ClientName(pVictim->GetPlayer()->GetCID()), Server()->ClientName(pVictim->GetPlayer()->GetCID()), pVictim->GetPlayer()->m_KillStreak);
					pVictim->GetPlayer()->m_KillStreak = 0;
					GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
					GameServer()->CreateExplosion(pVictim->m_Pos, m_pPlayer->GetCID(), WEAPON_GRENADE, true, 0, m_pPlayer->GetCharacter()->Teams()->TeamMask(0));
				}
			}

			if (pVictim->GetPlayer()->m_KillStreak >= 5)
			{
				//Check for new highscore
				if (g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2) //gdm & zCatch grenade
				{
					//dbg_msg("insta", "checking for highscore grenade");
					if (pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_GrenadeSpree)
					{
						pVictim->GetPlayer()->m_GrenadeSpree = pVictim->GetPlayer()->m_KillStreak;
						GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCID(), "New grenade Killingspree record!");
					}
					//str_format(aBuf, sizeof(aBuf), "last: %d top: %d", pVictim->GetPlayer()->m_KillStreak, pVictim->GetPlayer()->m_GrenadeSpree);
					//dbg_msg("insta", aBuf);
				}
				else if (g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4) // idm & zCatch rifle
				{
					//dbg_msg("insta", "checking for highscore rifle");
					if (pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_RifleSpree)
					{
						pVictim->GetPlayer()->m_RifleSpree = pVictim->GetPlayer()->m_KillStreak;
						GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCID(), "New rifle Killingspree record!");
					}
					//str_format(aBuf, sizeof(aBuf), "last: %d top: %d", pVictim->GetPlayer()->m_KillStreak, pVictim->GetPlayer()->m_GrenadeSpree);
					//dbg_msg("insta", aBuf);
				}

				str_format(aBuf, sizeof(aBuf), "'%s's killingspree was ended by %s (%d Kills)", Server()->ClientName(pVictim->GetPlayer()->GetCID()), Server()->ClientName(pKiller->GetCID()), pVictim->GetPlayer()->m_KillStreak);
				pVictim->GetPlayer()->m_KillStreak = 0;
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				GameServer()->CreateExplosion(pVictim->m_Pos, m_pPlayer->GetCID(), WEAPON_GRENADE, true, 0, m_pPlayer->GetCharacter()->Teams()->TeamMask(0));
			}

			if (pKiller != pVictim->GetPlayer())
			{
				if (!pVictim->GetPlayer()->m_IsDummy || pKiller->m_IsDummy)
				{
					pKiller->m_KillStreak++;
				}
				pVictim->GetPlayer()->m_KillStreak = 0;
				str_format(aBuf, sizeof(aBuf), "'%s' is on a killing spree with %d Kills!", Server()->ClientName(pKiller->GetCID()), pKiller->m_KillStreak);

				if (pKiller->m_KillStreak % 5 == 0 && pKiller->m_KillStreak >= 5)
					GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);

				//Finish time if cfg val reached
				if (pKiller->m_KillStreak == g_Config.m_SvKillsToFinish && g_Config.m_SvInstagibMode) //only finish if sv_insta is on... needed for the future if we actiavte this killsys in ddrace mode (sv_insta 0) to dont fuck up race scores
				{
					CGameControllerDDRace* Controller = (CGameControllerDDRace*)GameServer()->m_pController;
					Controller->m_Teams.OnCharacterFinish(pKiller->GetCID());
				}
			}
		}
	}
	else //not enough players
	{
		//str_format(aBuf, sizeof(aBuf), "not enough tees %d/%d spree (%d)", GameServer()->CountConnectedPlayers(), g_Config.m_SvSpreePlayers, pKiller->m_KillStreak);
		//dbg_msg("insta", aBuf);
		if (pKiller != pVictim->GetPlayer())
		{
			if (!pVictim->GetPlayer()->m_IsDummy || pKiller->m_IsDummy)
			{
				pKiller->m_KillStreak++;
			}

			pVictim->GetPlayer()->m_KillStreak = 0;
			if (pKiller->m_KillStreak == 5)
			{
				str_format(aBuf, sizeof(aBuf), "[SPREE] %d players needed to start a spree.", g_Config.m_SvSpreePlayers);
				GameServer()->SendChatTarget(pKiller->GetCID(), aBuf);
				pKiller->m_KillStreak = 0; //reset killstreak to avoid some1 collecting 100 kills with dummy and then if player connect he could save the spree
			}
		}
	}
	pVictim->GetPlayer()->m_KillStreak = 0; //Important always clear killingspree of ded dude
}

int CCharacter::BlockPointsMain(int Killer, bool fngscore)
{
	if (m_FreezeTime <= 0)
		return Killer;
	if (m_pPlayer->m_LastToucherID == -1)
		return Killer;
	if (m_pPlayer->m_IsInstaMode_fng && !fngscore)
		return Killer; // Killer = KilledID --> gets count as selfkill in score sys and not counted as kill (because only fng score tiles score)

	if (m_pPlayer->m_LastToucherID == m_pPlayer->GetCID())
	{
		dbg_msg("block", "WARNING '%s' [ID: %d] blocked himself", Server()->ClientName(m_pPlayer->GetCID()), m_pPlayer->GetCID());
		return Killer;
	}

	char aBuf[128];
	Killer = m_pPlayer->m_LastToucherID; // kill message

	if (g_Config.m_SvBlockBroadcast == 1)  // send kill message broadcast
	{
		str_format(aBuf, sizeof(aBuf), "'%s' was blocked by '%s'", Server()->ClientName(m_pPlayer->GetCID()), Server()->ClientName(Killer));
		GameServer()->SendBroadcastAll(aBuf, 0);
	}

	BlockQuestSubDieFuncBlockKill(Killer);

	// track deaths of blocked
	if (!m_pPlayer->m_IsBlockWaving) // dont count block deaths in blockwave minigame
	{
		if (m_pPlayer->m_IsInstaArena_gdm)
		{
			//m_pPlayer->m_GrenadeDeaths++; // probably doesn't belong into blockmain but whatever //ye rly doesnt --> moved
		}
		else if (m_pPlayer->m_IsInstaArena_idm)
		{
			//m_pPlayer->m_RifleDeaths++; // probably doesn't belong into blockmain but whatever //ye rly doesnt --> moved
		}
		else
		{
			if (m_pPlayer->m_IsDummy)
			{
				if (g_Config.m_SvDummyBlockPoints)
				{
					m_pPlayer->m_BlockPoints_Deaths++;
				}
			}
			else
			{
				m_pPlayer->m_BlockPoints_Deaths++;
			}
		}
	}

	if (GameServer()->m_apPlayers[Killer])
	{
		// give kills and points to blocker
		if (!m_pPlayer->m_IsBlockWaving) // dont count block kills and points in blockwave minigame (would be too op lol)
		{
			if (m_pPlayer->m_IsDummy) // if dummy got killed make some exceptions
			{
				if (g_Config.m_SvDummyBlockPoints == 2 || (g_Config.m_SvDummyBlockPoints == 3 && GameServer()->IsPosition(Killer, 2))) //only count dummy kills if configt       cfg:3 block area or further count kills
				{
					if (Server()->Tick() >= m_AliveTime + Server()->TickSpeed() * g_Config.m_SvPointsFarmProtection)
					{
						GameServer()->m_apPlayers[Killer]->GiveBlockPoints(1);
					}
					GameServer()->m_apPlayers[Killer]->m_BlockPoints_Kills++;
				}
			}
			else
			{
				if (Server()->Tick() >= m_AliveTime + Server()->TickSpeed() * g_Config.m_SvPointsFarmProtection)
				{
					GameServer()->m_apPlayers[Killer]->GiveBlockPoints(1);
				}
				GameServer()->m_apPlayers[Killer]->m_BlockPoints_Kills++;
			}
		}
		// give xp reward to the blocker
		if (m_pPlayer->m_KillStreak > 4 && m_pPlayer->IsMaxLevel())
		{
			if (!GameServer()->m_apPlayers[Killer]->m_HideBlockXp)
			{
				str_format(aBuf, sizeof(aBuf), "+%d xp for blocking '%s'", m_pPlayer->m_KillStreak, Server()->ClientName(m_pPlayer->GetCID()));
				GameServer()->SendChatTarget(Killer, aBuf);
			}
			GameServer()->m_apPlayers[Killer]->GiveXP( m_pPlayer->m_KillStreak);
		}
		// bounty money reward to the blocker
		if (m_pPlayer->m_BlockBounty)
		{
			str_format(aBuf, sizeof(aBuf), "[BOUNTY] +%d money for blocking '%s'", m_pPlayer->m_BlockBounty, Server()->ClientName(m_pPlayer->GetCID()));
			GameServer()->SendChatTarget(Killer, aBuf);
			str_format(aBuf, sizeof(aBuf), "bounty '%s'", m_pPlayer->m_BlockBounty, Server()->ClientName(m_pPlayer->GetCID()));
			GameServer()->m_apPlayers[Killer]->MoneyTransaction(+m_pPlayer->m_BlockBounty, aBuf);
			m_pPlayer->m_BlockBounty = 0;
		}
	}
	return Killer;
}

void CCharacter::BlockSpawnProt(int Killer)
{
	char aBuf[128];
	if (GameServer()->m_apPlayers[Killer] && GameServer()->m_apPlayers[Killer]->GetCharacter() && m_pPlayer->GetCID() != Killer)
	{
		//punish spawn blockers
		if (GameServer()->IsPosition(Killer, 3)) //if killer is in spawn area
		{
			GameServer()->m_apPlayers[Killer]->m_SpawnBlocks++;
			if (g_Config.m_SvSpawnBlockProtection == 1 || g_Config.m_SvSpawnBlockProtection == 2)
			{
				GameServer()->SendChatTarget(Killer, "[WARNING] spawnblocking is illegal.");
				//str_format(aBuf, sizeof(aBuf), "[debug] spawnblocks: %d", GameServer()->m_apPlayers[Killer]->m_SpawnBlocks);
				//GameServer()->SendChatTarget(Killer, aBuf);

				if (GameServer()->m_apPlayers[Killer]->m_SpawnBlocks > 2)
				{
					str_format(aBuf, sizeof(aBuf), "'%s' is spawnblocking. catch him!", Server()->ClientName(Killer));
					GameServer()->SendAllPolice(aBuf);
					GameServer()->SendChatTarget(Killer, "Police is searching you because of spawnblocking.");
					GameServer()->m_apPlayers[Killer]->m_EscapeTime += Server()->TickSpeed() * 120; // + 2 minutes escape time
					GameServer()->AddEscapeReason(Killer, "spawnblock");
				}
			}
		}
	}
}

void CCharacter::BlockQuestSubDieFuncBlockKill(int Killer)
{
	if (!GameServer()->m_apPlayers[Killer])
		return;

	char aBuf[128];
	//QUEST
	if (GameServer()->m_apPlayers[Killer]->m_QuestState == CPlayer::QUEST_HAMMER)
	{
		if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 7)
		{
			if (GameServer()->m_apPlayers[Killer]->m_QuestProgressValue < 10)
			{
				//GameServer()->SendChatTarget(Killer, "[QUEST] hammer the tee 10 times before blocking him.");
			}
			else
			{
				GameServer()->QuestAddProgress(Killer, 11);
			}
		}
	}
	else if (GameServer()->m_apPlayers[Killer]->m_QuestState == CPlayer::QUEST_BLOCK)
	{
		if (GameServer()->IsSameIP(Killer, m_pPlayer->GetCID()))
		{
			if (!m_pPlayer->m_HideQuestWarning)
			{
				GameServer()->SendChatTarget(Killer, "[QUEST] your dummy doesn't count.");
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] your dummy doesn't count."); //send it both so that he recives the message. i know this can be weird on lanpartys but fuck it xd
			}
		}
		else
		{
			if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 0)
			{
				GameServer()->QuestCompleted(Killer);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 1)
			{
				GameServer()->QuestAddProgress(Killer, 2);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 2)
			{
				GameServer()->QuestAddProgress(Killer, 3);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 3)
			{
				GameServer()->QuestAddProgress(Killer, 5);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 4)
			{
				GameServer()->QuestAddProgress(Killer, 10);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 5)
			{
				if (GameServer()->m_apPlayers[Killer]->m_QuestProgressValue < 5)
				{
					GameServer()->QuestAddProgress(Killer, 6, 5);
				}
				else
				{
					if (m_pPlayer->GetCID() != GameServer()->m_apPlayers[Killer]->m_QuestPlayerID)
					{
						str_format(aBuf, sizeof(aBuf), "[QUEST] You have to block '%s' to complete the quest.", Server()->ClientName(GameServer()->m_apPlayers[Killer]->m_QuestPlayerID));
						if (!m_pPlayer->m_HideQuestWarning)
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
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 6)
			{
				if (m_pPlayer->m_KillStreak < 5)
				{
					str_format(aBuf, sizeof(aBuf), "[QUEST] '%s' is only on a %d tee blockingspree", Server()->ClientName(m_pPlayer->GetCID()), m_pPlayer->m_KillStreak);
					if (!m_pPlayer->m_HideQuestWarning)
					{
						GameServer()->SendChatTarget(Killer, aBuf);
					}
				}
				else
				{
					GameServer()->QuestCompleted(Killer);
				}
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 7)
			{
				GameServer()->QuestAddProgress(Killer, 11);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 8)
			{
				GameServer()->QuestAddProgress(Killer, 3);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 9) //TODO: TEST THIS QUEST (should be working now)
			{
				//success (blocking player)
				if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(GameServer()->m_apPlayers[Killer]->GetCharacter()) != -1)
				{
					GameServer()->QuestAddProgress(Killer, 11);
				}
				else
				{
					if (!m_pPlayer->m_HideQuestWarning)
					{
						GameServer()->SendChatTarget(Killer, "[QUEST] You need the flag.");
					}
				}
			}
		}
	}
	else if (GameServer()->m_apPlayers[Killer]->m_QuestState == CPlayer::QUEST_RIFLE)
	{
		if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 7) // Rifle <specific player> and then block him [LEVEL 7]
		{
			if (GameServer()->m_apPlayers[Killer]->m_QuestPlayerID == m_pPlayer->GetCID())
			{
				if (GameServer()->m_apPlayers[Killer]->m_QuestProgressValue)
				{
					GameServer()->QuestAddProgress(Killer, 2);
				}
			}
			else
			{
				// GameServer()->SendChatTarget(Killer, "[QUEST] wrong tee");
			}
		}
		else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 8) // Rifle 5 tees before blocking them [LEVEL 8]
		{
			if (GameServer()->m_apPlayers[Killer]->m_QuestProgressBool)
			{
				if (GameServer()->m_apPlayers[Killer]->m_QuestLastQuestedPlayerID == m_pPlayer->GetCID())
				{
					GameServer()->QuestAddProgress(Killer, 5);
					GameServer()->m_apPlayers[Killer]->m_QuestProgressBool = false;
					GameServer()->m_apPlayers[Killer]->m_QuestLastQuestedPlayerID = -1;
				}
				else
				{
					if (!m_pPlayer->m_HideQuestWarning)
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
	if (Killer != m_pPlayer->GetCID() && m_pPlayer->m_QuestState == CPlayer::QUEST_BLOCK && m_pPlayer->m_QuestStateLevel == 7 && m_pPlayer->m_QuestProgressValue > 0)
	{
		GameServer()->QuestFailed(m_pPlayer->GetCID());
	}
	if (m_pPlayer->m_QuestStateLevel == 9 && m_pPlayer->m_QuestState == CPlayer::QUEST_HAMMER)
	{
		GameServer()->QuestFailed(m_pPlayer->GetCID());
	}
}

void CCharacter::KillingSpree(int Killer) // handles all ddnet++ gametype sprees (not other server types as fng or instagib only servers)
{
	char aBuf[128];
	// Somehow inspiration by //toast killingspree
	// system by FruchtiHD and ChillerDragon stolen from twlevel (edited by ChillerDragon)
	// stolen from DDnet++ instagib and edited agian by ChillerDragon
	// rewritten by ChillerDragon cuz tw bug
	// upgraded to handle instagib agian
	// rewritten by ChillerDragon in 2019 cuz old system was fucked in the head

	CPlayer *pKiller = GameServer()->m_apPlayers[Killer]; //removed pointer alien code and used the long way to have less bugsis with left players

	// dont count selfkills only count real being blocked as dead
	if (m_pPlayer->GetCID() == Killer)
	{
		//dbg_msg("SPREE", "didnt count selfkill [%d][%s]", Killer, Server()->ClientName(Killer));
		return;	
	}

	char aKillerName[32];
	char aSpreeType[16];

	if (GameServer()->m_apPlayers[Killer])
		str_format(aKillerName, sizeof(aKillerName), "'%s'", Server()->ClientName(Killer));
	else
		str_format(aKillerName, sizeof(aKillerName), "'%s'", m_pPlayer->m_aLastToucherName);
		// str_copy(aKillerName, "a player who left the game", sizeof(aKillerName));

	if (m_pPlayer->m_KillStreak >= 5)
	{
		GameServer()->GetSpreeType(m_pPlayer->GetCID(), aSpreeType, sizeof(aSpreeType), true);
		str_format(aBuf, sizeof(aBuf), "'%s's %s spree was ended by %s (%d Kills)", Server()->ClientName(m_pPlayer->GetCID()), aSpreeType, aKillerName, m_pPlayer->m_KillStreak);
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		GameServer()->CreateExplosion(m_Pos, m_pPlayer->GetCID(), WEAPON_GRENADE, true, 0, m_pPlayer->GetCharacter()->Teams()->TeamMask(0));
	}

	if (pKiller)
	{
		//#################
		// KILLER (blocker)
		//#################
		if ((m_pPlayer->m_IsDummy && g_Config.m_SvSpreeCountBots) ||  //only count bots if configurated
			(!m_pPlayer->m_IsDummy)) //count all humans in killingsprees
		{
			GameServer()->m_apPlayers[Killer]->m_KillStreak++;
		}
		// only count killing sprees if enough players are online and ingame (alive)
		if (GameServer()->CountIngameHumans() < g_Config.m_SvSpreePlayers)
		{
			//dbg_msg("spree", "not enough tees %d/%d spree (%d)", GameServer()->CountConnectedPlayers(), g_Config.m_SvSpreePlayers, GameServer()->m_apPlayers[Killer]->m_KillStreak);
			if (GameServer()->m_apPlayers[Killer]->m_KillStreak == 5) // TODO: what if one has 6 kills and then all players leave then he can farm dummys?
			{
				str_format(aBuf, sizeof(aBuf), "[SPREE] %d/%d humans alive to start a spree.", GameServer()->CountIngameHumans(), g_Config.m_SvSpreePlayers);
				GameServer()->SendChatTarget(Killer, aBuf);
				GameServer()->m_apPlayers[Killer]->m_KillStreak = 0; // reset killstreak to avoid some1 collecting 100 kills with dummy and then if player connect he could save the spree
			}
		}
		else // enough players
		{
			if (GameServer()->m_apPlayers[Killer]->m_KillStreak % 5 == 0 && GameServer()->m_apPlayers[Killer]->m_KillStreak >= 5)
			{
				GameServer()->GetSpreeType(Killer, aSpreeType, sizeof(aSpreeType), false);
				str_format(aBuf, sizeof(aBuf), "%s is on a %s spree with %d kills!", aKillerName, aSpreeType, pKiller->m_KillStreak);
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
		}
	}
	m_pPlayer->m_KillStreak = 0; //Important always clear killingspree of ded dude
}

void CCharacter::CITick()
{
	//Check for stuck --> restart
	if (isFreezed)
	{
		m_ci_freezetime++;
	}
	else
	{
		m_ci_freezetime = 0;
	}
	if (m_ci_freezetime > g_Config.m_SvCIfreezetime * Server()->TickSpeed())
	{
		Die(m_pPlayer->GetCID(), WEAPON_SELF); //call CIRestart() there
	}
}

void CCharacter::CIRestart()
{
	char aBuf[128];

	//str_format(aBuf, sizeof(aBuf), "%x", GameServer()->Score()->LoadCIData()); //linux compile error (doesnt work on win anyways)
	//if (!str_comp(aBuf, "error"))
	//{
	//	dbg_msg("CI", "error loading data...");
	//}
	//else
	//{
	//	dbg_msg("CI", "loaded DIST [%x]", GameServer()->Score()->LoadCIData());
	//}

	m_pPlayer->m_ci_latest_dest_dist = CIGetDestDist();
	str_format(aBuf, sizeof(aBuf), "Dist: %d", m_pPlayer->m_ci_latest_dest_dist);
	dbg_msg("CI", aBuf);

	if (m_pPlayer->m_ci_latest_dest_dist < m_pPlayer->m_ci_lowest_dest_dist)
	{
		str_format(aBuf, sizeof(aBuf), "NEW [%d] OLD [%d]", m_pPlayer->m_ci_latest_dest_dist, m_pPlayer->m_ci_lowest_dest_dist);
		dbg_msg("CI", aBuf);
		m_pPlayer->m_ci_lowest_dest_dist = m_pPlayer->m_ci_latest_dest_dist;
	}

	str_format(aBuf, sizeof(aBuf), "%d", m_pPlayer->m_ci_lowest_dest_dist);

	GameServer()->Score()->SaveCIData(aBuf);
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

void CCharacter::SurvivalSubDieFunc(int Killer, int weapon)
{
	bool selfkill = Killer == m_pPlayer->GetCID();
	if (m_pPlayer->m_IsSurvivalAlive && GameServer()->m_apPlayers[Killer]->m_IsSurvivalAlive) //ignore lobby and stuff
	{
		//=== DEATHS and WINCHECK ===
		if (m_pPlayer->m_IsSurvivaling)
		{
			if (GameServer()->m_survivalgamestate > 1) //if game running
			{
				GameServer()->SetPlayerSurvival(m_pPlayer->GetCID(), GameServer()->SURVIVAL_DIE);
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[SURVIVAL] you lost the round.");
				GameServer()->SurvivalCheckWinnerAndDeathMatch();
				GameServer()->SurvivalGetNextSpectator(m_pPlayer->GetCID(), Killer);
				GameServer()->SurvivalUpdateSpectators(m_pPlayer->GetCID(), Killer);
			}
		}

		//=== KILLS ===
		if (!selfkill)
		{
			if (GameServer()->m_apPlayers[Killer] && GameServer()->m_apPlayers[Killer]->m_IsSurvivaling)
			{
				GameServer()->m_apPlayers[Killer]->m_SurvivalKills++;
			}
		}
	}
}

void CCharacter::InstagibSubDieFunc(int Killer, int Weapon)
{
	//=== DEATHS ===
	if (g_Config.m_SvInstagibMode)
	{
		if (g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2) //gdm & zCatch grenade
		{
			m_pPlayer->m_GrenadeDeaths++;
		}
		else if (g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4) // idm & zCatch rifle
		{
			m_pPlayer->m_RifleDeaths++;
		}

		InstagibKillingSpree(Killer, Weapon);
	}
	else if (m_pPlayer->m_IsInstaArena_gdm)
	{
		m_pPlayer->m_GrenadeDeaths++;
	}
	else if (m_pPlayer->m_IsInstaArena_idm)
	{
		m_pPlayer->m_RifleDeaths++;
	}

	//=== KILLS ===
	if (GameServer()->m_apPlayers[Killer])
	{
		if (g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2 || GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm) //gdm & zCatch grenade
		{
			GameServer()->m_apPlayers[Killer]->m_GrenadeKills++;
		}
		else if (g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4 || GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm) // idm & zCatch rifle
		{
			GameServer()->m_apPlayers[Killer]->m_RifleKills++;
		}

		//=== MULTIS (credit to noby) ===
		if (Killer != m_pPlayer->GetCID()) // don't count selkills as multi
		{
			time_t ttmp = time(NULL);
			if ((ttmp - GameServer()->m_apPlayers[Killer]->m_lastkilltime) <= 5)
			{
				GameServer()->m_apPlayers[Killer]->m_multi++;
				if (GameServer()->m_apPlayers[Killer]->m_max_multi < GameServer()->m_apPlayers[Killer]->m_multi)
				{
					GameServer()->m_apPlayers[Killer]->m_max_multi = GameServer()->m_apPlayers[Killer]->m_multi;
				}
				char aBuf[128];
				if (GameServer()->IsDDPPgametype("fng"))
				{
					str_format(aBuf, sizeof(aBuf), "'%s' multi x%d!", Server()->ClientName(Killer), GameServer()->m_apPlayers[Killer]->m_multi);
					GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				}
				if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_fng)
				{
					if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm) // grenade
					{
						str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' multi x%d!", Server()->ClientName(Killer), GameServer()->m_apPlayers[Killer]->m_multi);
						GameServer()->SayInsta(aBuf, 4);
					}
					else if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm) // rifle
					{
						str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' multi x%d!", Server()->ClientName(Killer), GameServer()->m_apPlayers[Killer]->m_multi);
						GameServer()->SayInsta(aBuf, 5);
					}
				}
			}
			else
			{
				GameServer()->m_apPlayers[Killer]->m_multi = 1;
			}
			GameServer()->m_apPlayers[Killer]->m_lastkilltime = ttmp;
		}
	}
}

void CCharacter::Pause(bool Pause)
{
	m_Paused = Pause;
	if (Pause)
	{
		GameServer()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = 0;
		GameServer()->m_World.RemoveEntity(this);

		if (m_Core.m_HookedPlayer != -1) // Keeping hook would allow cheats
		{
			m_Core.m_HookedPlayer = -1;
			m_Core.m_HookState = HOOK_RETRACTED;
			m_Core.m_TriggeredEvents |= COREEVENT_HOOK_RETRACT;
		}
	}
	else
	{
		m_Core.m_Vel = vec2(0, 0);
		GameServer()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = &m_Core;
		GameServer()->m_World.InsertEntity(this);
	}
}

void CCharacter::DDRaceInit()
{
	m_Paused = false;
	m_DDRaceState = DDRACE_NONE;
	m_PrevPos = m_Pos;
	m_SetSavePos = false;
	m_LastBroadcast = 0;
	m_TeamBeforeSuper = 0;
	m_Core.m_Id = GetPlayer()->GetCID();
	if (g_Config.m_SvTeam == 2)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "Please join a team before you start");
		m_LastStartWarning = Server()->Tick();
	}
	m_TeleCheckpoint = 0;
	m_EndlessHook = g_Config.m_SvEndlessDrag;
	m_Hit = g_Config.m_SvHit ? HIT_ALL : DISABLE_HIT_GRENADE | DISABLE_HIT_HAMMER | DISABLE_HIT_RIFLE | DISABLE_HIT_SHOTGUN;
	m_SuperJump = false;
	m_Jetpack = false;
	m_freezeShotgun = false;
	m_isDmg = false;
	m_Core.m_Jumps = 2;
	m_FreezeHammer = false;
	//Testy testy Chilliwashere
	m_Dummy_mode18 = 0;
	m_Dummy_panic_weapon = 0;

	//m_Dummy_nn_Tick = 0;
	/*m_Dummy_nn_time = 0;
	m_Dummy_nn_latest_fitness = 0.0f;
	m_Dummy_nn_highest_fitness = 0.0f;
	m_Dummy_nn_latest_Distance = 0.0f;
	m_Dummy_nn_highest_Distance = 0.0f;*/

	//str_format(m_Dummy_friend, sizeof(m_Dummy_friend), "nobody");
	//m_Dummy_friend = "nobody";
	//m_Dummy_FriendID = -1;

	// disable finite cosmetics by default
	m_Rainbow = false;
	m_Bloody = false;
	m_Atom = false;
	m_Trail = false;

	int Team = Teams()->m_Core.Team(m_Core.m_Id);

	if (Teams()->TeamLocked(Team))
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (Teams()->m_Core.Team(i) == Team && i != m_Core.m_Id && GameServer()->m_apPlayers[i])
			{
				CCharacter* pChar = GameServer()->m_apPlayers[i]->GetCharacter();

				if (pChar)
				{
					m_DDRaceState = pChar->m_DDRaceState;
					m_StartTime = pChar->m_StartTime;
				}
			}
		}
	}
}

void CCharacter::Rescue()
{
	if (m_SetSavePos && !m_Super && !m_DeepFreeze && IsGrounded() && m_Pos == m_PrevPos) {
		if (m_LastRescue + g_Config.m_SvRescueDelay * Server()->TickSpeed() > Server()->Tick())
		{
			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "You have to wait %d seconds until you can rescue yourself", (m_LastRescue + g_Config.m_SvRescueDelay * Server()->TickSpeed() - Server()->Tick()) / Server()->TickSpeed());
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), aBuf);
			return;
		}

		int index = GameServer()->Collision()->GetPureMapIndex(m_Pos);
		if (GameServer()->Collision()->GetTileIndex(index) == TILE_FREEZE || GameServer()->Collision()->GetFTileIndex(index) == TILE_FREEZE) {
			m_LastRescue = Server()->Tick();
			m_Core.m_Pos = m_PrevSavePos;
			m_Pos = m_PrevSavePos;
			m_PrevPos = m_PrevSavePos;
			m_Core.m_Vel = vec2(0, 0);
			m_Core.m_HookedPlayer = -1;
			m_Core.m_HookState = HOOK_RETRACTED;
			m_Core.m_TriggeredEvents |= COREEVENT_HOOK_RETRACT;
			GameWorld()->ReleaseHooked(GetPlayer()->GetCID());
			m_Core.m_HookPos = m_Core.m_Pos;
			UnFreeze();
		}
	}
}

//======================
//                     =
//  DDNet++            =
//     (ddpp funcs)    =
//                     =
//======================

void CCharacter::SendShopMessage(const char *pMsg)
{
	int recv = m_pPlayer->m_ShopBotMesssagesRecieved / 2; // 2 messages = enter + leave
	if (g_Config.m_SvMaxShopMessages != -1 && g_Config.m_SvMaxShopMessages <= recv)
		return;

	GameServer()->SendChat(GameServer()->GetShopBot(), CGameContext::CHAT_TO_ONE_CLIENT, pMsg, -1, m_pPlayer->GetCID());
	m_pPlayer->m_ShopBotMesssagesRecieved++;
}

bool CCharacter::InputActive()
{
	static bool IsWalk = false;
	// static bool IsFire = false;
	static bool IsJump = false;
	static bool IsHook = false;
	if (m_Input.m_Direction)
		IsWalk = true;
	// if (m_Input.m_Fire)
		// IsFire = true;
	if (m_Input.m_Jump)
		IsJump = true;
	if (m_Input.m_Hook)
		IsHook = true;
	if (IsWalk && IsJump && IsHook)
	{
		IsWalk = false;
		IsJump = false;
		IsHook = false;
		return true;
	}
	return false;
}

int CCharacter::GetAimDir()
{
	if (m_Input.m_TargetX < 0)
		return -1;
	else
		return 1;
	return 0;
}

void CCharacter::TakeHammerHit(CCharacter* pFrom)
{
	vec2 Dir;
	if (length(m_Pos - pFrom->m_Pos) > 0.0f)
		Dir = normalize(m_Pos - pFrom->m_Pos);
	else
		Dir = vec2(0.f, -1.f);

	vec2 Push = vec2(0.f, -1.f) + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f;
	//if (GameServer()->m_pController->IsTeamplay() && pFrom->GetPlayer() && m_pPlayer->GetTeam() == pFrom->GetPlayer()->GetTeam() && IsFreezed()) 
	if (false)
	{
		Push.x *= g_Config.m_SvMeltHammerScaleX*0.01f;
		Push.y *= g_Config.m_SvMeltHammerScaleY*0.01f;
	}
	else 
	{
		Push.x *= g_Config.m_SvHammerScaleX*0.01f;
		Push.y *= g_Config.m_SvHammerScaleY*0.01f;
	}

	m_Core.m_Vel += Push;
}

void CCharacter::KillFreeze(bool unfreeze)
{
	if (!g_Config.m_SvFreezeKillDelay)
		return;
	if (unfreeze) // stop counting
	{
		m_FirstFreezeTick = 0;
		return;
	}
	if (!m_FirstFreezeTick) // start counting
	{
		m_FirstFreezeTick = Server()->Tick();
		return;
	}
	if (Server()->Tick() - m_FirstFreezeTick > (Server()->TickSpeed() / 10) * g_Config.m_SvFreezeKillDelay)
	{
		Die(m_pPlayer->GetCID(), WEAPON_SELF);
		m_FirstFreezeTick = 0;
	}
}
