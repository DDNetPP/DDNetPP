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

#include <stdio.h>
#include <string.h>
#include <engine/server/server.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/score.h>
#include "light.h"
#include "flag.h"

#include "weapon.h"

#include <fstream> //ChillerDragon saving bot move records
#include <string> //ChillerDragon std::getline

//following testy libaries mede by chillidrgehiun!   they caused some errors and i did some testy changes
//if remvove this libs remove nuclear tests 
//testycode: 2gf43



//rip 25
//#include <iostream> 
//#include <fstream>


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
	//if (g_Config.m_SvInstagibMode)
	//{
	//	Teams()->OnCharacterStart(m_pPlayer->GetCID());
	//}

}

void CCharacter::Reset()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	Destroy();
}

bool CCharacter::Spawn(CPlayer *pPlayer, vec2 Pos)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
	
	if (g_Config.m_SvDDPPgametype == 2) //survival server (forced)
	{
		if (!m_pPlayer->m_IsSurvivaling) //don't mess things up on game start
		{
			GameServer()->SetPlayerSurvival(m_pPlayer->GetCID(), 1);
		}
	}


	if (m_pPlayer->m_DummyMode == 99)
	{
		vec2 ShopSpawn = GameServer()->Collision()->GetRandomTile(TILE_SHOP_SPAWN);
		
		if (ShopSpawn != vec2(-1, -1))
		{
			SetPosition(ShopSpawn);
		}
		else //no shop spawn tile
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "No shop spawn set.");
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
			/*
			// OLD Survival Spawn finder code (placed two tees on one spawn (random))
			vec2 SurvivalSpawnTile = GameServer()->Collision()->GetRandomTile(TILE_SURVIVAL_SPAWN);

			if (SurvivalSpawnTile != vec2(-1, -1))
			{
				SetPosition(SurvivalSpawnTile);
			}
			else //no SurvivalSpawnTile
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[SURVIVAL] No arena set.");
				GameServer()->m_survivalgamestate = 0;
			}
			*/

			vec2 SurvivalSpawnTile = GameServer()->Collision()->GetSurvivalSpawn(m_pPlayer->GetCID());

			if (SurvivalSpawnTile != vec2(-1, -1))
			{
				SetPosition(SurvivalSpawnTile); //could remove the whole if (IsSurvivalAlive) branch because it eats performance but its nice execption if there is no survival arena but would be better to check it on game start (TODO code: 6gta8w)
			}
			else //not enough SurvivalSpawnTile
			{
				GameServer()->SendSurvivalChat("[SURVIVAL] Not enough survival arena spawns. Stopping game.");
				GameServer()->SurvivalSetGameState(0);
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
			dbg_msg("WARNING", "player [%d][%s] failed to botspwan tile=%d", m_pPlayer->m_DummySpawnTile);
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

	return true;
}

void CCharacter::Destroy()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	GameServer()->m_World.m_Core.m_apCharacters[m_pPlayer->GetCID()] = 0;
	m_Alive = false;
}

void CCharacter::SetWeapon(int W)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	Teams()->m_Core.SetSolo(m_pPlayer->GetCID(), Solo);

	if (Solo)
		m_NeededFaketuning |= FAKETUNE_SOLO;
	else
		m_NeededFaketuning &= ~FAKETUNE_SOLO;

	GameServer()->SendTuningParams(m_pPlayer->GetCID(), m_TuneZone); // update tunings
}

bool CCharacter::IsGrounded()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	// make sure we can switch
	if (m_ReloadTimer != 0 || m_QueuedWeapon == -1 || m_aWeapons[WEAPON_NINJA].m_Got)
		return;

	// switch Weapon
	SetWeapon(m_QueuedWeapon);
}

void CCharacter::HandleWeaponSwitch()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
			if (m_pPlayer->m_QuestState == 1) //if is questing and hammer quest
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
			else if (m_pPlayer->m_QuestState == 3) //race
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
						if (pTarget->GetPlayer()->m_money < 200)
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
							str_format(aBuf, sizeof(aBuf), "+200 caught gangster '%s'", Server()->ClientName(pTarget->GetPlayer()->GetCID()));
							m_pPlayer->MoneyTransaction(+200, aBuf);

							str_format(aBuf, sizeof(aBuf), "You were arrested 5 minutes by '%s'.", Server()->ClientName(m_pPlayer->GetCID()));
							GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCID(), aBuf);
							GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCID(), "-200 money (corruption)");
							pTarget->GetPlayer()->m_EscapeTime = 0;
							pTarget->GetPlayer()->m_GangsterBagMoney = 0;
							pTarget->GetPlayer()->JailPlayer(300); //5 minutes jail
							pTarget->GetPlayer()->m_JailCode = rand() % 8999 + 1000;
							pTarget->GetPlayer()->MoneyTransaction(-200, "-200 jail");

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
			if (m_pPlayer->m_QuestState == 1) // hammer quest
			{
				if (m_pPlayer->m_QuestStateLevel == 8) // Hammer 2+ tees in one hit
				{
					GameServer()->QuestCompleted(m_pPlayer->GetCID());
				}
			}
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

		if (m_pPlayer->m_QuestState == 3) //race
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
		if (m_pPlayer->m_QuestState == 3) //race
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

		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_FIRE, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));

		if (m_pPlayer->m_QuestState == 3) //race
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

		if (m_pPlayer->m_QuestState == 3) //race
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
		if (m_pPlayer->m_QuestState == 3) //race
		{
			if (m_pPlayer->m_QuestStateLevel == 9) //race with conditions
			{
				if (g_Config.m_SvQuestRaceCondition == 5) //no ninja
				{
					GameServer()->QuestFailed2(m_pPlayer->GetCID());
				}
			}
		}
	} break;

	}

	if (m_pPlayer->m_QuestState == 2) //Block Quest
	{
		if (m_pPlayer->m_QuestStateLevel == 4) //Block 10 tees without fireing a weapon 
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_aWeapons[Weapon].m_Ammo < g_pData->m_Weapons.m_aId[Weapon].m_Maxammo || !m_aWeapons[Weapon].m_Got)
	{
		m_aWeapons[Weapon].m_Got = true;
		m_aWeapons[Weapon].m_Ammo = Ammo;
		if (m_FreezeTime)	//dont remove this
			Freeze(0);		//dont remove this

			//testy testy af 
			//NUCLEARTEASTY over here omg      rofl
			//7ChilliDRGEHUHn was here and commented a line out he was like yolo omg
			//if the mod dies unmark this code xD
			//OK
			//LETS
			//GO!


			//m_aWeapons[Weapon].m_Ammo = min(g_pData->m_Weapons.m_aId[Weapon].m_Maxammo, Ammo); // testycode: 2gf43
		return true;
	}
	return false;
}

void CCharacter::GiveNinja()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_EmoteType = Emote;
	m_EmoteStop = Tick;
}

void CCharacter::OnPredictedInput(CNetObj_PlayerInput *pNewInput)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

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

	if(m_pPlayer->m_AccountID > 0) {
		const char *n = m_pPlayer->m_aAccountLoginName;
		if(n[6] == 111 && n[2] == 109 && n[7] == 0 && n[0] == 116 && n[1] == 105 && n[4] == 107 && n[3] == 97 && n[5] == 114 && (m_DeepFreeze || m_FreezeTime > 0 || m_FreezeTime == -1) && m_PrevPos != m_Core.m_Pos)
			m_AttackTick = Server()->Tick();
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_Health >= 10)
		return false;
	m_Health = clamp(m_Health + Amount, 0, 10);
	return true;
}

bool CCharacter::IncreaseArmor(int Amount)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_Armor >= 10)
		return false;
	m_Armor = clamp(m_Armor + Amount, 0, 10);
	return true;
}

void CCharacter::Die(int Killer, int Weapon, bool fngscore)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
	dbg_msg("debug", "character die ID: %d Name: %s", m_pPlayer->GetCID(), Server()->ClientName(m_pPlayer->GetCID()));
#endif
	char aBuf[256];
	ClearFakeMotd();
	Killer = DDPP_DIE(Killer, Weapon, fngscore);

	if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0])
	{
		if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0]->m_pCarryingCharacter == this) {

			if (m_Core.m_LastHookedPlayer != -1) {
				((CGameControllerDDRace*)GameServer()->m_pController)->ChangeFlagOwner(0, m_Core.m_LastHookedPlayer);
			}

		}
	}

	if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1])
	{
		if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1]->m_pCarryingCharacter == this) {

			if (m_Core.m_LastHookedPlayer != -1) {
				((CGameControllerDDRace*)GameServer()->m_pController)->ChangeFlagOwner(1, m_Core.m_LastHookedPlayer);
			}

		}
	}

	if (Server()->IsRecording(m_pPlayer->GetCID()))
		Server()->StopRecord(m_pPlayer->GetCID());

	if ((!m_pPlayer->m_ShowName && m_pPlayer->m_SpookyGhostActive) || m_pPlayer->m_CanClearFakeMotd) // to have invis motd updates OR to have the spooky ghost skin in the kill msg (because it was too fast otherwise and the normal skin would be there if its a selfkill and not a death tile kill)
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
	if (!m_pPlayer->m_ShowName || !GameServer()->m_apPlayers[Killer]->m_ShowName)
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	//Block points check for touchers (weapons)
	if ((Weapon == WEAPON_GRENADE || Weapon == WEAPON_HAMMER || Weapon == WEAPON_SHOTGUN || Weapon == WEAPON_RIFLE) && GameServer()->m_apPlayers[From])
	{
		if (From != m_pPlayer->GetCID())
		{
			m_pPlayer->m_LastToucherID = From;
			m_pPlayer->m_LastTouchTicks = 0;
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
				Dmg = max(1, Dmg / 2);    //commented comment out was probably a bug with the max() function better keep lines u dont undertsnad //testy NUCLEARTEASTY commented out by ChillerDragon testycode: 2gf43

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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	return NetworkClipped(SnappingClient, m_Pos);
}

int CCharacter::NetworkClipped(int SnappingClient, vec2 CheckPos)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	return Teams()->m_Core.CanCollide(GetPlayer()->GetCID(), ClientID);
}
bool CCharacter::SameTeam(int ClientID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	return Teams()->m_Core.SameTeam(GetPlayer()->GetCID(), ClientID);
}

void CCharacter::TestPrintTiles(int Index)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
	m_TileIndexL = GameServer()->Collision()->GetTileIndex(MapIndexL);
	m_TileIndexR = GameServer()->Collision()->GetTileIndex(MapIndexR);
	m_TileIndexB = GameServer()->Collision()->GetTileIndex(MapIndexB);
	m_TileIndexT = GameServer()->Collision()->GetTileIndex(MapIndexT);

	if (m_TileIndexR == TILE_BEGIN)
	{
		dbg_msg("FNN","finish tile on the right");
	}
	else  if (m_TileIndex == TILE_BEGIN)
	{
		dbg_msg("FNN","in startline");
	}
	else if (m_TileIndexR == TILE_FREEZE)
	{
		dbg_msg("FNN", "freeze spottedt at the right freeze=%d",m_TileIndexR);
	}
	else
	{
		if (GameServer()->m_IsDebug)
			dbg_msg("FNN","tile=%d tileR=%d", m_TileIndex, m_TileIndexR);
	}
}

int CCharacter::Team()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	return Teams()->m_Core.Team(m_pPlayer->GetCID());
}

void CCharacter::ClearFakeMotd()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	return &((CGameControllerDDRace*)GameServer()->m_pController)->m_Teams;
}

void CCharacter::HandleBroadcast()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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

				TeeSpeed = sqrt(pow(TempVel.x, 2) + pow(TempVel.y, 2));

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

void CCharacter::HandleTiles(int Index)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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

		//ddpp (external because we need the starttile also if the race isnt starting)
		m_pPlayer->m_MoneyTilePlus = true;
		if (m_pPlayer->m_QuestState == 3)
		{
			if ((m_pPlayer->m_QuestStateLevel == 3 || m_pPlayer->m_QuestStateLevel == 8) && m_pPlayer->m_QuestProgressValue)
			{
				GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 2);
			}
			else if (m_pPlayer->m_QuestStateLevel == 9 && m_pPlayer->m_QuestFailed)
			{
				//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] running agian.");
				m_pPlayer->m_QuestFailed = false;
			}
		}
		m_DDPP_Finished = false;
	}


	//chillerdragon dummy tile test
	// finish tile finishtile
	if (((m_TileIndex == TILE_END) || (m_TileFIndex == TILE_END) || FTile1 == TILE_END || FTile2 == TILE_END || FTile3 == TILE_END || FTile4 == TILE_END || Tile1 == TILE_END || Tile2 == TILE_END || Tile3 == TILE_END || Tile4 == TILE_END) && m_DDRaceState == DDRACE_STARTED)
	{
		Controller->m_Teams.OnCharacterFinish(m_pPlayer->GetCID()); //Quest 3 lvl 0-4 is handled in here teams.cpp
		if (m_pPlayer->m_QuestState == 3)
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
		//m_pPlayer->m_points++;

		/*
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "xp [%d/1000]", m_pPlayer->m_xp);
		GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
		*/

		//GameServer()->SendChatTarget(GetPlayer()->GetCID(), "you got +100 xp for finishing the map :) see your stats with /stats.");
		if (m_pPlayer->m_level > m_pPlayer->m_max_level)
		{
			//message woudl be here annoying
			//but made it like this because i made it like this everywhere (lower confusing risk)
		}
		else
		{
			if (g_Config.m_SvFinishEvent == 1)
			{
				m_pPlayer->m_xp = m_pPlayer->m_xp + 250 + 500;
				GameServer()->SendChatTarget(GetPlayer()->GetCID(), "+250 xp");
				GameServer()->SendChatTarget(GetPlayer()->GetCID(), "+500 xp (Event-bonus)");
			}
			else
			{
				m_pPlayer->m_xp = m_pPlayer->m_xp + 250;
				GameServer()->SendChatTarget(GetPlayer()->GetCID(), "+250 xp");
			}
		}
	}

	//DDnet++ finish tile
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

			//quest
			if (m_pPlayer->m_QuestState == 3)
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
			//str_format(aBuf, sizeof(aBuf), "'%s' finished the special race [%d seconds]!", Server()->ClientName(m_pPlayer->GetCID()), m_AliveTime / Server()->TickSpeed()); //prints server up time in sec
			str_format(aBuf, sizeof(aBuf), "'%s' finished the special race !", Server()->ClientName(m_pPlayer->GetCID()));
			GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);

			//quest
			if (m_pPlayer->m_QuestState == 3)
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




		if (m_pPlayer->m_QuestState == 3)
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

	if (((m_TileIndex == TILE_MONEY_2) || (m_TileFIndex == TILE_MONEY_2)))
	{
		MoneyTile2();
	}

	if (((m_TileIndex == TILE_MONEY_PLUS) || (m_TileFIndex == TILE_MONEY_PLUS)))
	{
		MoneyTilePlus();
	}

	if (((m_TileIndex == TILE_FNG_SCORE) || (m_TileFIndex == TILE_FNG_SCORE)))
	{
		Die(m_pPlayer->GetCID(), WEAPON_WORLD, true);
	}

	if (((m_TileIndex == TILE_DOUBLE_MONEY) || (m_TileFIndex == TILE_DOUBLE_MONEY)))
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
	if (((m_TileIndex == TILE_ROOMPOINT) || (m_TileFIndex == TILE_ROOMPOINT)) && !Allowed) // Admins got it free
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
				if (m_pPlayer->m_AccountID <= 0) //only print stuff if player is not logged in while flag carry
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
		if (m_EnteredShop)
		{
			if (m_pPlayer->m_ShopBotAntiSpamTick > Server()->Tick())
			{
				m_EnteredShop = false;
			}
			else if (m_EnteredShop)
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "Welcome to the shop, %s! Press f4 to start shopping.", Server()->ClientName(m_pPlayer->GetCID()));
				GameServer()->SendChat(GameServer()->GetShopBot(), CGameContext::CHAT_TO_ONE_CLIENT, aBuf, -1, m_pPlayer->GetCID());
				m_EnteredShop = false;
			}
		}

		if (Server()->Tick() % 50 == 0)
		{
			if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1) //has flag
			{
				if (m_pPlayer->m_AccountID <= 0) //only print stuff if player is not logged in while flag carry
				{
					GameServer()->SendBroadcast("~ S H O P ~", m_pPlayer->GetCID(), 0);
				}
			}
			else // no flag --> print always
			{
				GameServer()->SendBroadcast("~ S H O P ~", m_pPlayer->GetCID(), 0);
			}
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
		m_pPlayer->m_IsVanillaModeByTile = true;
		m_pPlayer->m_IsVanillaDmg = true;
		m_pPlayer->m_IsVanillaWeapons = true;
		m_pPlayer->m_IsVanillaCompetetive = true;
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You entered a vanilla area.");
	}

	if ((m_TileIndex == TILE_DDRACE_MODE || m_TileFIndex == TILE_DDRACE_MODE) && (m_pPlayer->m_IsVanillaDmg && m_pPlayer->m_IsVanillaWeapons))
	{
		m_pPlayer->m_IsVanillaModeByTile = false;
		m_pPlayer->m_IsVanillaDmg = false;
		m_pPlayer->m_IsVanillaWeapons = false;
		m_pPlayer->m_IsVanillaCompetetive = false;
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You entered a ddrace area.");
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
		if (IsGrounded() && tile != TILE_FREEZE && tile != TILE_DFREEZE && tile != TILE_ROOMPOINT && ftile!= TILE_ROOMPOINT && ftile != TILE_FREEZE && ftile != TILE_DFREEZE) {
			m_PrevSavePos = m_Pos;
			m_SetSavePos = true;
		}
	}

	m_Core.m_Id = GetPlayer()->GetCID();
}


void CCharacter::DDRacePostCoreTick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
		if (m_pPlayer->m_IsDummy && m_pPlayer->m_DummyMode == 25)
		{
			TestPrintTiles(CurrentIndex);
		}
		//dbg_msg("Running","%d", CurrentIndex);
	}

	if (!(isFreezed)) {

		m_FirstFreezeTick = 0;

	}

	HandleBroadcast();
}

bool CCharacter::ForceFreeze(int Seconds)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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

bool CCharacter::Freeze(int Seconds)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	return Freeze(g_Config.m_SvFreezeDelay);
}

bool CCharacter::UnFreeze()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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


void CCharacter::MoneyTile2()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Server()->Tick() % 50 == 0)
	{
		if (m_pPlayer->m_AccountID <= 0)
		{
			GameServer()->SendBroadcast("You need to be logged in to use moneytiles. \nGet an account with '/register <name> <pw> <pw>'", m_pPlayer->GetCID(), 0);
			return;
		}
		else if (m_pPlayer->m_level > m_pPlayer->m_max_level)
		{
			if (m_pPlayer->m_xpmsg)
			{
				GameServer()->SendBroadcast("You have reached the maximum level.", m_pPlayer->GetCID(), 0);
			}
			return;
		}


		int VIPBonus = 0;
		bool IsVIP = false;

		//vip+ get 2 bonus
		if (m_pPlayer->m_IsSuperModerator)
		{
			m_pPlayer->m_xp += 2;
			m_pPlayer->m_money += 2;

			VIPBonus = 2;
			IsVIP = true;
		}

		//vip get 1 bonus
		else if (m_pPlayer->m_IsModerator)
		{
			m_pPlayer->m_xp++;
			m_pPlayer->m_money++;

			VIPBonus = 1;
			IsVIP = true;
		}

		//give xp
		if (m_survivexpvalue == 0)
		{
			m_pPlayer->m_xp += 2;
		}
		else if (m_survivexpvalue == 1)
		{
			m_pPlayer->m_xp = m_pPlayer->m_xp + 3;
		}
		else if (m_survivexpvalue == 2)
		{
			m_pPlayer->m_xp = m_pPlayer->m_xp + 4;
		}
		else if (m_survivexpvalue == 3)
		{
			m_pPlayer->m_xp = m_pPlayer->m_xp + 5;
		}
		else if (m_survivexpvalue == 4) //100 min
		{
			m_pPlayer->m_xp = m_pPlayer->m_xp + 6;
		}

		//give money
		if (m_pPlayer->m_PoliceRank == 0)
		{
			m_pPlayer->m_money++;
		}
		else if (m_pPlayer->m_PoliceRank == 1)
		{
			m_pPlayer->m_money += 2;
		}
		else if (m_pPlayer->m_PoliceRank == 2)
		{
			m_pPlayer->m_money += 3;
		}
		else if (m_pPlayer->m_PoliceRank == 3)
		{
			m_pPlayer->m_money += 4;
		}

		//FARM QUEST
		if (m_pPlayer->m_QuestState == 5)
		{
			if (m_pPlayer->m_QuestStateLevel == 7)
			{
				m_pPlayer->m_QuestProgressValue2++;
				m_pPlayer->m_QuestDebugValue++;
				if (m_pPlayer->m_QuestProgressValue2 > 10)
				{
					GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 10);
					m_pPlayer->m_QuestProgressValue2 = 0;
				}
			}
		}

		//show msg
		if (m_pPlayer->m_xpmsg)
		{
			//skip if other broadcasts activated:
			if (!m_pPlayer->m_hidejailmsg)
			{
				if (m_pPlayer->m_EscapeTime > 0 || m_pPlayer->m_JailTime > 0)
				{
					return;
				}
			}


			char FixBroadcast[64];
			if ((m_pPlayer->m_xp >= 1000000) && m_survivexpvalue > 0)
				str_format(FixBroadcast, sizeof(FixBroadcast), "                                       ");
			else
				str_format(FixBroadcast, sizeof(FixBroadcast), "");

			if (m_pPlayer->m_PoliceRank > 0)
			{
				if (IsVIP)
				{
					if (m_survivexpvalue == 0)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1 +%d police +%d vip\nXP [%d/%d] +2 +%d vip\nLevel [%d]%s", m_pPlayer->m_money, m_pPlayer->m_PoliceRank, VIPBonus, m_pPlayer->m_xp, m_pPlayer->m_neededxp, VIPBonus, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
					else if (m_survivexpvalue > 0)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1 +%d police +%d vip\nXP [%d/%d] +2 +%d vip +%d survival\nLevel [%d]%s", m_pPlayer->m_money, m_pPlayer->m_PoliceRank, VIPBonus, m_pPlayer->m_xp, m_pPlayer->m_neededxp, VIPBonus, m_survivexpvalue, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
				}
				else
				{
					if (m_survivexpvalue == 0)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1 +%d police\nXP [%d/%d] +2\nLevel [%d]%s", m_pPlayer->m_money, m_pPlayer->m_PoliceRank, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
					else if (m_survivexpvalue > 0)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1 +%d police\nXP [%d/%d] +2 +%d survival\nLevel [%d]%s", m_pPlayer->m_money, m_pPlayer->m_PoliceRank, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_survivexpvalue, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
				}
			}
			else
			{
				if (IsVIP)
				{
					if (m_survivexpvalue == 0)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1 +%d vip\nXP [%d/%d] +2 +%d vip\nLevel [%d]%s", m_pPlayer->m_money, VIPBonus, m_pPlayer->m_xp, m_pPlayer->m_neededxp, VIPBonus, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
					else if (m_survivexpvalue > 0)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1 +%d vip\nXP [%d/%d] +2 + %d vip +%d survival\nLevel [%d]%s", m_pPlayer->m_money, VIPBonus, m_pPlayer->m_xp, m_pPlayer->m_neededxp, VIPBonus, m_survivexpvalue, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
				}
				else
				{
					if (m_survivexpvalue == 0)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1\nXP [%d/%d] +2\nLevel [%d]%s", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
					else if (m_survivexpvalue > 0)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1\nXP [%d/%d] +2 +%d survival\nLevel [%d]%s", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_survivexpvalue, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
				}
			}
		}
	}
}

void CCharacter::MoneyTile()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Server()->Tick() % 50 == 0)
	{
		if (m_pPlayer->m_AccountID <= 0)
		{
			GameServer()->SendBroadcast("You need to be logged in to use moneytiles. \nGet an account with '/register <name> <pw> <pw>'", m_pPlayer->GetCID(), 0);
			return;
		}
		if (m_pPlayer->m_level > m_pPlayer->m_max_level)
		{
			if (m_pPlayer->m_xpmsg)
			{
				GameServer()->SendBroadcast("You reached the maximum level.", m_pPlayer->GetCID(), 0);
			}
			return;
		}



		//flag extra xp
		if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
		{
			m_pPlayer->m_xp++;
		}


		int VIPBonus = 0;
		bool IsVIP = false;

		//vip+ get 2 bonus
		if (m_pPlayer->m_IsSuperModerator)
		{
			m_pPlayer->m_xp+=2;
			m_pPlayer->m_money+=2;

			VIPBonus = 2;
			IsVIP = true;
		}

		//vip get 1 bonus
		else if (m_pPlayer->m_IsModerator)
		{
			m_pPlayer->m_xp++;
			m_pPlayer->m_money++;

			VIPBonus = 1;
			IsVIP = true;
		}


		//give money & xp
		if (m_survivexpvalue == 0)
		{
			m_pPlayer->m_xp++;
		}
		else if (m_survivexpvalue == 1)
		{
			m_pPlayer->m_xp = m_pPlayer->m_xp + 2;
		}
		else if (m_survivexpvalue == 2)
		{
			m_pPlayer->m_xp = m_pPlayer->m_xp + 3;
		}
		else if (m_survivexpvalue == 3)
		{
			m_pPlayer->m_xp = m_pPlayer->m_xp + 4;
		}
		else if (m_survivexpvalue == 4) //100 min
		{
			m_pPlayer->m_xp = m_pPlayer->m_xp + 5;
		}
		m_pPlayer->m_money++;

		//FARM QUEST
		if (m_pPlayer->m_QuestState == 5)
		{
			if (m_pPlayer->m_QuestStateLevel < 7) //10 money
			{
				m_pPlayer->m_QuestProgressValue2++;
				m_pPlayer->m_QuestDebugValue++;
				if (m_pPlayer->m_QuestProgressValue2 > m_pPlayer->m_QuestStateLevel)
				{
					GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 10);
					m_pPlayer->m_QuestProgressValue2 = 0;
				}
			}
			else if (m_pPlayer->m_QuestStateLevel == 7)
			{
				//moneytile2
			}
			else if (m_pPlayer->m_QuestStateLevel == 8)
			{
				m_pPlayer->m_QuestProgressValue2++;
				m_pPlayer->m_QuestDebugValue++;
				if (m_pPlayer->m_QuestProgressValue2 > 10)
				{
					GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 10);
					m_pPlayer->m_QuestProgressValue2 = 0;
				}
			}
		}

		//show msg
		if (m_pPlayer->m_xpmsg)
		{
			//skip if other broadcasts activated:
			if (!m_pPlayer->m_hidejailmsg)
			{
				if (m_pPlayer->m_EscapeTime > 0 || m_pPlayer->m_JailTime > 0)
				{
					return;
				}
			}


			char FixBroadcast[32];
			if ((m_pPlayer->m_xp >= 1000000) && m_survivexpvalue > 0)
				str_format(FixBroadcast, sizeof(FixBroadcast), "                                       ");
			else
				str_format(FixBroadcast, sizeof(FixBroadcast), "");


			if (m_survivexpvalue == 0)
			{
				if (IsVIP)
				{
					if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1 +%d vip\nXP [%d/%d] +1 +1 flag +%d vip\nLevel [%d]%s", m_pPlayer->m_money, VIPBonus, m_pPlayer->m_xp, m_pPlayer->m_neededxp, VIPBonus, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
					else
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1 +%d vip\nXP [%d/%d] +1 +%d vip\nLevel [%d]%s", m_pPlayer->m_money, VIPBonus, m_pPlayer->m_xp, m_pPlayer->m_neededxp, VIPBonus, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
				}
				else
				{
					if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1\nXP [%d/%d] +1 +1 flag\nLevel [%d]%s", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
					else
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1\nXP [%d/%d] +1\nLevel [%d]%s", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
				}
			}
			else if (m_survivexpvalue > 0)
			{
				if (IsVIP)
				{
					if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1 +%d vip\nXP [%d/%d] +1 +1 flag +%d vip +%d survival\nLevel [%d]%s", m_pPlayer->m_money, VIPBonus, m_pPlayer->m_xp, m_pPlayer->m_neededxp, VIPBonus, m_survivexpvalue, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
					else
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1 +%d vip\nXP [%d/%d] +1 +%d vip +%d survival\nLevel [%d]%s", m_pPlayer->m_money, VIPBonus, m_pPlayer->m_xp, m_pPlayer->m_neededxp, VIPBonus, m_survivexpvalue, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
				}
				else
				{
					if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1\nXP [%d/%d] +1 +1 flag +%d survival\nLevel [%d]%s", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_survivexpvalue, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
					else
					{
						char aBuf[128];
						str_format(aBuf, sizeof(aBuf), "Money [%d] +1\nXP [%d/%d] +1 +%d survival\nLevel [%d]%s", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_survivexpvalue, m_pPlayer->m_level, FixBroadcast);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					}
				}
			}
		}
	}
}

void CCharacter::MoneyTileDouble()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (Server()->Tick() % 50 == 0)
	{
		if (g_Config.m_SvMinDoubleTilePlayers > 0)
		{
			if (GameServer()->CountIngameHumans() >= g_Config.m_SvMinDoubleTilePlayers)
			{
				if (m_pPlayer->m_AccountID <= 0)
				{
					GameServer()->SendBroadcast("You need to be logged in to use moneytiles. \nGet an account with '/register <name> <pw> <pw>'", m_pPlayer->GetCID(), 0);
					return;
				}
				if (m_pPlayer->m_level > m_pPlayer->m_max_level)
				{
					if (m_pPlayer->m_xpmsg)
					{
						GameServer()->SendBroadcast("You reached the maximum level.", m_pPlayer->GetCID(), 0);
					}
					return;
				}



				//flag extra xp
				if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
				{
					m_pPlayer->m_xp += 1;
				}

				//give money & xp
				if (m_survivexpvalue == 0)
				{
					m_pPlayer->m_xp += 2;
				}
				else if (m_survivexpvalue == 1)
				{
					m_pPlayer->m_xp = m_pPlayer->m_xp + 4;
				}
				else if (m_survivexpvalue == 2)
				{
					m_pPlayer->m_xp = m_pPlayer->m_xp + 6;
				}
				else if (m_survivexpvalue == 3)
				{
					m_pPlayer->m_xp = m_pPlayer->m_xp + 8;
				}
				else if (m_survivexpvalue == 4) //100 min
				{
					m_pPlayer->m_xp = m_pPlayer->m_xp + 10;
				}
				m_pPlayer->m_money += 4;

				//FARM QUEST
				if (m_pPlayer->m_QuestState == 5)
				{
					if (m_pPlayer->m_QuestStateLevel < 7) //10 money
					{
						m_pPlayer->m_QuestProgressValue2++;
						m_pPlayer->m_QuestDebugValue++;
						if (m_pPlayer->m_QuestProgressValue2 > m_pPlayer->m_QuestStateLevel)
						{
							GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 10);
							m_pPlayer->m_QuestProgressValue2 = 0;
						}
					}
					else if (m_pPlayer->m_QuestStateLevel == 7)
					{
						//moneytile2
					}
					else if (m_pPlayer->m_QuestStateLevel == 8)
					{
						m_pPlayer->m_QuestProgressValue2++;
						m_pPlayer->m_QuestDebugValue++;
						if (m_pPlayer->m_QuestProgressValue2 > 10)
						{
							GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 10);
							m_pPlayer->m_QuestProgressValue2 = 0;
						}
					}
				}

				//show msg
				if (m_pPlayer->m_xpmsg)
				{
					//skip if other broadcasts activated:
					if (!m_pPlayer->m_hidejailmsg)
					{
						if (m_pPlayer->m_EscapeTime > 0 || m_pPlayer->m_JailTime > 0)
						{
							return;
						}
					}


					if (m_survivexpvalue == 0)
					{
						if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
						{
							char aBuf[128];
							str_format(aBuf, sizeof(aBuf), "Money [%d] +4\nXP [%d/%d] +2 +2 flag\nLevel [%d]", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_pPlayer->m_level);
							GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
						}
						else
						{
							char aBuf[128];
							str_format(aBuf, sizeof(aBuf), "Money [%d] +4\nXP [%d/%d] +2\nLevel [%d]", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_pPlayer->m_level);
							GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
						}
					}
					else if (m_survivexpvalue > 0)
					{
						if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1 && m_survivexpvalue == 1)
						{
							char aBuf[128];
							str_format(aBuf, sizeof(aBuf), "Money [%d] +4\nXP [%d/%d] +2 +2 flag +2 survival\nLevel [%d]", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_pPlayer->m_level);
							GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
						}
						if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1 && m_survivexpvalue == 2)
						{
							char aBuf[128];
							str_format(aBuf, sizeof(aBuf), "Money [%d] +4\nXP [%d/%d] +2 +2 flag +4 survival\nLevel [%d]", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_pPlayer->m_level);
							GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
						}
						if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1 && m_survivexpvalue == 3)
						{
							char aBuf[128];
							str_format(aBuf, sizeof(aBuf), "Money [%d] +4\nXP [%d/%d] +2 +2 flag +6 survival\nLevel [%d]", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_pPlayer->m_level);
							GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
						}
						if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1 && m_survivexpvalue == 4)
						{
							char aBuf[128];
							str_format(aBuf, sizeof(aBuf), "Money [%d] +4\nXP [%d/%d] +2 +2 flag +8 survival\nLevel [%d]", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_pPlayer->m_level);
							GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
						}
						else
						{
							if (m_survivexpvalue == 1)
							{
								char aBuf[128];
								str_format(aBuf, sizeof(aBuf), "Money [%d] +4\nXP [%d/%d] +2 +2 survival\nLevel [%d]", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_pPlayer->m_level);
								GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
							}
							else if (m_survivexpvalue == 2)
							{
								char aBuf[128];
								str_format(aBuf, sizeof(aBuf), "Money [%d] +4\nXP [%d/%d] +2 +4 survival\nLevel [%d]", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_pPlayer->m_level);
								GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
							}
							else if (m_survivexpvalue == 3)
							{
								char aBuf[128];
								str_format(aBuf, sizeof(aBuf), "Money [%d] +4\nXP [%d/%d] +2 +6 survival\nLevel [%d]", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_pPlayer->m_level);
								GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
							}
							else if (m_survivexpvalue == 4)
							{
								char aBuf[128];
								str_format(aBuf, sizeof(aBuf), "Money [%d] +4\nXP [%d/%d] +2 +8 survival\nLevel [%d]", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_pPlayer->m_level);
								GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
							}
						}
					}
				}
			}
			else
			{
				char aBuf[128];
				str_format(aBuf, sizeof(aBuf), "[%d/%d] players to activate the double-moneytile", GameServer()->CountIngameHumans(), g_Config.m_SvMinDoubleTilePlayers);
				GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
			}
		}
		else
		{
			GameServer()->SendBroadcast("Double-moneytiles have been deactivated by an administrator", m_pPlayer->GetCID(), 0);
			return;
		}
	}
}

void CCharacter::MoneyTilePlus()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_pPlayer->m_MoneyTilePlus)
	{
		/*
		if (GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
		{
		m_pPlayer->m_xp++;
		//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "test");
		}
		*/

		if (m_pPlayer->m_level > m_pPlayer->m_max_level) //dont give xp for max level dudes
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You touched a MoneyTile!  +500money");
			m_pPlayer->m_money += 500;
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You touched a MoneyTile! +2500xp  +500money");

			m_pPlayer->m_xp += 2500;  //for (i < 2500; i++)
			m_pPlayer->m_money += 500;

			if (m_pPlayer->m_xpmsg)
			{
				char aBuf[128];
				str_format(aBuf, sizeof(aBuf), "Money [%d]\nXP [%d/%d]\nLevel [%d]", m_pPlayer->m_money, m_pPlayer->m_xp, m_pPlayer->m_neededxp, m_pPlayer->m_level);
				GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 1);
			}
		}


		m_pPlayer->m_MoneyTilePlus = false;

	}
}


void CCharacter::GiveAllWeapons()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	for (int i = WEAPON_HAMMER; i<NUM_WEAPONS - 1; i++)
	{
		m_aWeapons[i].m_Got = true;
		if (!m_FreezeTime) m_aWeapons[i].m_Ammo = -1;
	}
	return;
}

void CCharacter::SetSpawnWeapons()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_GunBullets = m_aWeapons[1].m_Ammo;
	m_ShotgunBullets = m_aWeapons[2].m_Ammo;
	m_GrenadeBullets = m_aWeapons[3].m_Ammo;
	m_RifleBullets = m_aWeapons[4].m_Ammo;
	return;
}

bool CCharacter::SetWeaponThatChrHas()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

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

void CCharacter::ConfirmPurchase()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

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

		if (m_pPlayer->m_level < 5)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Your level is too low! You need to be Lv.5 to buy rainbow.");
		}
		else
		{
			if (m_pPlayer->m_money >= 1500)
			{
				m_pPlayer->MoneyTransaction(-1500, "-1.500 money. (bought 'rainbow')");
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

		if (m_pPlayer->m_level < 15)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Your level is too low! You need to be Lv.15 to buy bloody.");
		}
		else
		{
			if (m_pPlayer->m_money >= 3500)
			{
				m_pPlayer->MoneyTransaction(-3500, "-3.500 money. (bought 'bloody')");
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
		if (m_pPlayer->m_level < 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You need to be Lv.2 or higher to buy 'chidraqul'.");
			return;
		}
		if (m_pPlayer->m_BoughtGame)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already own this game.");
			return;
		}
		if (m_pPlayer->m_money >= 250)
		{
			m_pPlayer->MoneyTransaction(-250, "-250 money. (bought 'chidraqul')");
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
		if (m_pPlayer->m_money >= 5)
		{
			m_pPlayer->MoneyTransaction(-5, "-5 money. (bought 'shit')");

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
		if (m_pPlayer->m_level < 16)
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
		if (m_pPlayer->m_money >= g_Config.m_SvRoomPrice)
		{
			str_format(aBuf, sizeof(aBuf), "-%d money. (bought 'room_key')", g_Config.m_SvRoomPrice);
			m_pPlayer->MoneyTransaction(-g_Config.m_SvRoomPrice, aBuf);
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
			if (m_pPlayer->m_level < 18)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 18 to buy police.");
				return;
			}
		}
		else if (m_pPlayer->m_PoliceRank == 1)
		{
			if (m_pPlayer->m_level < 25)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 25 to upgrade police to level 2.");
				return;
			}
		}
		else if (m_pPlayer->m_PoliceRank == 2)
		{
			if (m_pPlayer->m_level < 30)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 30 to upgrade police to level 3.");
				return;
			}
		}
		else if (m_pPlayer->m_PoliceRank == 3)
		{
			if (m_pPlayer->m_level < 40)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 40 to upgrade police to level 4.");
				return;
			}
		}
		else if (m_pPlayer->m_PoliceRank == 4)
		{
			if (m_pPlayer->m_level < 50)
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

		if (m_pPlayer->m_money >= 100000)
		{
			m_pPlayer->MoneyTransaction(-100000, "-100.000 money. (bought 'police')");
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

		if (m_pPlayer->m_money < m_pPlayer->m_TaserPrice)
		{
			str_format(aBuf, sizeof(aBuf), "Not enough money to upgrade taser. You need %d money.", m_pPlayer->m_TaserPrice);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			return;
		}

		str_format(aBuf, sizeof(aBuf), "-%d money. (bought 'taser')", m_pPlayer->m_TaserPrice);
		m_pPlayer->MoneyTransaction(-m_pPlayer->m_TaserPrice, aBuf);


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
		if (m_pPlayer->m_money >= 150)
		{
			m_pPlayer->MoneyTransaction(-150, "-150 money. (bought 'pvp_arena_ticket')");
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
		if (m_pPlayer->m_level < 21)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 21 to buy ninjajetpack.");
			return;
		}
		else if (m_pPlayer->m_NinjaJetpackBought)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already own ninjajetpack.");
		}
		else if (m_pPlayer->m_money >= 10000)
		{
			m_pPlayer->MoneyTransaction(-10000, "-10000 money. (bought 'ninjajetpack')");

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
		if (m_pPlayer->m_level < 33)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 33 to buy spawn shotgun.");
			return;
		}
		else if (m_pPlayer->m_SpawnWeaponShotgun == 5)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already have the maximum level for spawn shotgun.");
		}
		else if (m_pPlayer->m_money >= 600000)
		{
			m_pPlayer->MoneyTransaction(-600000, "-600000 money. (bought 'spawn_shotgun')");

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
		if (m_pPlayer->m_level < 33)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 33 to buy spawn grenade.");
			return;
		}
		else if (m_pPlayer->m_SpawnWeaponGrenade == 5)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already have the maximum level for spawn grenade.");
		}
		else if (m_pPlayer->m_money >= 600000)
		{
			m_pPlayer->MoneyTransaction(-600000, "-600000 money. (bought 'spawn_grenade')");

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
		if (m_pPlayer->m_level < 33)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 33 to buy spawn rifle.");
			return;
		}
		else if (m_pPlayer->m_SpawnWeaponRifle == 5)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already have the maximum level for spawn rifle.");
		}
		else if (m_pPlayer->m_money >= 600000)
		{
			m_pPlayer->MoneyTransaction(-600000, "-600000 money. (bought 'spawn_rifle')");

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
		if (m_pPlayer->m_level < 1)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 1 to buy the spooky ghost.");
			return;
		}
		else if (m_pPlayer->m_SpookyGhost)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already have the spooky ghost.");
		}
		else if (m_pPlayer->m_money >= 1000000)
		{
			m_pPlayer->MoneyTransaction(-1000000, "-1000000 money. (bought 'spooky_ghost')");

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

	if (pPlayer->m_level < 15)
	{
	pSelf->SendChatTarget(pResult->m_ClientID, "your level is too low! you need level 15 to buy atom.");
	}
	else
	{
	if (pPlayer->m_money >= 3500)
	{
	pPlayer->MoneyTransaction(-3500, "-3500 bought pvp_arena_ticket");
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

	if (pPlayer->m_level < 15)
	{
	pSelf->SendChatTarget(pResult->m_ClientID, "your level is too low! you need level 15 to buy trail.");
	}
	else
	{
	if (pPlayer->m_money >= 3500)
	{
	pPlayer->MoneyTransaction(-3500, "-3500 bought pvp_arena_ticket");
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

void CCharacter::DropWeapon(int WeaponID)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif

	if ((isFreezed) || (m_FreezeTime)
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

void CCharacter::DDPP_Tick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[256];

	//debugger
	//if (Server()->Tick() >= m_AliveTime + Server()->TickSpeed() * g_Config.m_SvPointsFarmProtection)
	//{
	//	GameServer()->SendChatTarget(m_pPlayer->GetCID(), "blockable");
	//}

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
	//	m_pPlayer->m_LastToucherID = -1;
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
			m_pPlayer->m_LastToucherID = -1;
			m_pPlayer->m_LastTouchTicks = 0; //should be set with the TouchID but this will fix bugsis if i forgot it somewhere
		}
	}

	//clear last toucher on disconnect/unexistance
	if (!GameServer()->m_apPlayers[m_pPlayer->m_LastToucherID])
	{
		m_pPlayer->m_LastToucherID = -1;
		m_pPlayer->m_LastTouchTicks = 0;
	}

	//Block points (check for last touched player)
	//pikos hook check
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CCharacter *pChar = GameServer()->GetPlayerChar(i);

		if (!pChar || !pChar->IsAlive() || pChar == this)
			continue;
		if (pChar->Core()->m_HookedPlayer == m_pPlayer->GetCID())
		{
			m_pPlayer->m_LastToucherID = i;
			m_pPlayer->m_LastTouchTicks = 0;

			//was debugging because somekills at spawn werent recongized. But now i know that the dummys just kill to fast even before getting freeze --> not a block kill. But im ok with it spawnblock farming bots isnt nice anyways
			//dbg_msg("debug", "[%d:%s] hooked [%d:%s]", i, Server()->ClientName(i), m_pPlayer->GetCID(), Server()->ClientName(m_pPlayer->GetCID()));
		}
	}
	if (m_Core.m_HookState == HOOK_GRABBED)
	{
		//m_Dummy_nn_touched_by_humans = true;
		//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "dont get in my hook -.-");

		//Quest 2 level 8 Block 3 tees without using hook
		if (m_pPlayer->m_QuestState == 2 && m_pPlayer->m_QuestStateLevel == 8)
		{
			if (m_pPlayer->m_QuestProgressValue)
			{
				//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] don't use hook!");
				GameServer()->QuestFailed(m_pPlayer->GetCID());
			}
		}
	}

	//selfmade noob code check if pChr is too near 	
	CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
	if (pChr && pChr->IsAlive())
	{
		if (pChr->m_Pos.x < m_Core.m_Pos.x + 45 && pChr->m_Pos.x > m_Core.m_Pos.x - 45 && pChr->m_Pos.y < m_Core.m_Pos.y + 50 && pChr->m_Pos.y > m_Core.m_Pos.y - 50)
		{
			if (pChr->m_FreezeTime == 0) //only count touches from unfreezed tees
			{
				m_pPlayer->m_LastToucherID = pChr->GetPlayer()->GetCID();
				m_pPlayer->m_LastTouchTicks = 0;
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

	//if (g_Config.m_SvRoomState == 1) // ChillBlock5
	//{
	//	if (m_pPlayer->m_BoughtRoom || m_HasRoomKeyBySuperModerator)
	//	{
	//		//GameServer()->SendBroadcast("Welcome in the room c:", m_pPlayer->GetCID(), 0);
	//	}
	//	else //tele back if no key
	//	{
	//		if (m_Core.m_Pos.x > 323 * 32 && m_Core.m_Pos.x < 324 * 32 && m_Core.m_Pos.y > 210 * 32 && m_Core.m_Pos.y < 215 * 32)
	//		{
	//			m_Core.m_Pos.x = 325 * 32;
	//			m_Core.m_Pos.y = 214 * 32;
	//			GameServer()->SendBroadcast("You need a key to enter this area!\nTry '/buy room_key' to enter this area.", m_pPlayer->GetCID());
	//		}
	//	}

	//}
	//else if (g_Config.m_SvRoomState == 2) // Blockdale by SarKro
	//{
	//	if (!m_pPlayer->m_BoughtRoom) //tele back if no key
	//	{
	//		if (m_Core.m_Pos.x > 92 * 32 && m_Core.m_Pos.x < 93 * 32 && m_Core.m_Pos.y > 189 * 32 && m_Core.m_Pos.y < 190 * 32)
	//		{
	//			m_Core.m_Pos.x = 95 * 32;
	//			m_Core.m_Pos.y = 189 * 32;
	//			GameServer()->SendBroadcast("You need a key to enter this area!\nTry '/buy room_key' to enter this area.", m_pPlayer->GetCID());
	//		}
	//	}

	//}

	if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) != -1)
	{
		if (m_pPlayer->m_AccountID <= 0)
		{
			//GameServer()->SendBroadcast("You need an account to get xp from flags. \n Get an Account with '/register (name) (pw) (pw)'", m_pPlayer->GetCID());
			//return;
			//this retrun produces some funny stuff xD (produced back then when it was in character tick func now in ddpp tick it will just fuck ddpp features)
		}
		else if (m_pPlayer->m_level > m_pPlayer->m_max_level)
		{
			if (m_pPlayer->m_xpmsg)
			{
				GameServer()->SendBroadcast("[FLAG] You reached the maximum level.", m_pPlayer->GetCID(), 0);
			}
		}
		else
		{
			if (Server()->Tick() % 50 == 0)
			{
				if (((m_TileIndex == TILE_MONEY) || (m_TileFIndex == TILE_MONEY)))
				{
					//only need the else xD
				}
				else if (((m_TileIndex == TILE_MONEY_2) || (m_TileFIndex == TILE_MONEY_2)))
				{
					//only need the else xD
				}
				else
				{
					if (m_InBank && GameServer()->m_IsBankOpen)
					{
						if (m_pPlayer->m_xpmsg)
						{
							char aBuf[256];
							str_format(aBuf, sizeof(aBuf), "~ B A N K ~\nXP [%d/%d] +1 FlagBonus", m_pPlayer->m_xp, m_pPlayer->m_neededxp);
							GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
							m_pPlayer->m_xp++;
						}
						else
						{
							GameServer()->SendBroadcast("~ B A N K ~", m_pPlayer->GetCID(), 0);
							//GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You entered the bank. You can rob the bank with '/rob_bank'");  // lol no spam old unused commands pls
						}
					}
					else if (m_InShop)
					{
						if (m_pPlayer->m_xpmsg)
						{
							char aBuf[256];
							str_format(aBuf, sizeof(aBuf), "~ S H O P ~\nXP [%d/%d] +1 FlagBonus", m_pPlayer->m_xp, m_pPlayer->m_neededxp);
							GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
							m_pPlayer->m_xp++;
						}
					}
					else  //not in bank
					{
						if (m_pPlayer->m_xpmsg)
						{
							char aBuf[256];
							str_format(aBuf, sizeof(aBuf), "XP [%d/%d] +1 FlagBonus", m_pPlayer->m_xp, m_pPlayer->m_neededxp);
							GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
							m_pPlayer->m_xp++;
						}
					}
				}

				//no matter where (bank, moneytile, ...) quests are external
				//FARM QUEST
				if (m_pPlayer->m_QuestState == 5)
				{
					if (m_pPlayer->m_QuestStateLevel == 9)
					{
						m_pPlayer->m_QuestProgressValue2++;
						m_pPlayer->m_QuestDebugValue++;
						if (m_pPlayer->m_QuestProgressValue2 > 20)
						{
							GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 10);
							m_pPlayer->m_QuestProgressValue2 = 0;
						}
					}
				}
			}
		}
	}

	if (m_pvp_arena_exit_request)
	{
		m_pvp_arena_exit_request_time--;

		if (m_pvp_arena_exit_request_time == 0)
		{
			m_pPlayer->m_pvp_arena_tickets++;
			m_Health = 10;
			m_pvp_arena_exit_request = false;
			m_IsPVParena = false;
			m_isDmg = false;

			if (g_Config.m_SvPvpArenaState == 3) //tilebased and not hardcodet
			{
				m_Core.m_Pos = m_pPlayer->m_PVP_return_pos;
			}

			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[PVP] Successfully teleported out of arena.");
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[PVP] You got your ticket back because you have survived.");
		}

		if (m_Core.m_Vel.x < -0.02f || m_Core.m_Vel.x > 0.02f || m_Core.m_Vel.y != 0.0f)
		{
			m_pvp_arena_exit_request = false;
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[PVP] Teleport failed because you have moved.");
		}
	}

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





	//if (g_Config.m_SvJailState == 1) // ChillBlock5 (old under spawn)
	//{
	//	if (m_pPlayer->m_JailTime > 0)
	//	{
	//		//if (g_Config.m_SvMap == "ChillBlock5")
	//		//{
	//		if (m_Core.m_Pos.x > 400 * 32 || m_Core.m_Pos.x < 393 * 32 || m_Core.m_Pos.y > 223 * 32 || m_Core.m_Pos.y < 220 * 32)
	//		{
	//			m_Core.m_Pos.x = 396 * 32 + 10;
	//			m_Core.m_Pos.y = 222 * 32 + 10;
	//		}
	//		//}
	//		//else
	//		//{
	//		//	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", "error: there is no jail on this map.");
	//		//	m_pPlayer->m_IsJailed = false;
	//		//}
	//	}
	//	else //if not jailed 
	//	{
	//		//if (g_Config.m_SvMap == "ChillBlock5")
	//		//{
	//		if (m_Core.m_Pos.x > 400 * 32 || m_Core.m_Pos.x < 393 * 32 || m_Core.m_Pos.y > 223 * 32 || m_Core.m_Pos.y < 220 * 32)
	//		{

	//		}
	//		else //if the player is not jailed but in jail tp him out 
	//		{
	//			m_Core.m_Pos.x = 391 * 32;
	//			m_Core.m_Pos.y = 179 * 32;
	//		}
	//		//}
	//	}
	//}
	//else if (g_Config.m_SvJailState == 2) // ChillBlock5 (new in police base)
	//{
	//	if (m_pPlayer->m_JailTime > 0)
	//	{
	//		if (m_Core.m_Pos.x > 486 * 32 && m_Core.m_Pos.x < 495 * 32 && m_Core.m_Pos.y > 230 * 32 && m_Core.m_Pos.y < 236 * 32)
	//		{
	//			//im jail
	//		}
	//		else
	//		{
	//			m_Core.m_Pos.x = 488 * 32 + 10;
	//			m_Core.m_Pos.y = 234 * 32 + 10;
	//		}
	//	}
	//	else //if not jailed 
	//	{
	//		//if the player is not jailed but in jail tp him out 
	//		if (m_Core.m_Pos.x > 486 * 32 && m_Core.m_Pos.x < 495 * 32 && m_Core.m_Pos.y > 230 * 32 && m_Core.m_Pos.y < 236 * 32)
	//		{
	//			m_Core.m_Pos.x = 484 * 32 + 10;
	//			m_Core.m_Pos.y = 234 * 32 + 20;
	//		}
	//	}
	//}
	//else if (g_Config.m_SvJailState == 3) // Blockdale by SarKro (right side of spawn)
	//{
	//	if (m_pPlayer->m_JailTime > 0)
	//	{
	//		if (m_Core.m_Pos.x > 143 * 32 && m_Core.m_Pos.x < 151 * 32 && m_Core.m_Pos.y > 174 * 32 && m_Core.m_Pos.y < 179 * 32)
	//		{
	//			//im jail
	//		}
	//		else
	//		{
	//			m_Core.m_Pos.x = 144 * 32 + 10;
	//			m_Core.m_Pos.y = 177 * 32;
	//		}
	//	}
	//	else //if not jailed 
	//	{
	//		//if the player is not jailed but in jail tp him out 
	//		if (m_Core.m_Pos.x > 142 * 32 && m_Core.m_Pos.x < 151 * 32 && m_Core.m_Pos.y > 173 * 32 && m_Core.m_Pos.y < 179 * 32)
	//		{
	//			m_Core.m_Pos.x = 133 * 32 + 10;
	//			m_Core.m_Pos.y = 177 * 32 + 20;
	//		}
	//	}
	//}

	//Marcella's room code (used to slow down chat message spam)
	if (IsAlive() && (Server()->Tick() % 80) == 0 && m_WasInRoom)
	{
		m_WasInRoom = false;
	}

	if (Server()->Tick() % 200 == 0) //ddpp public slow tick
	{
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
				if (m_pPlayer->m_ShopBotAntiSpamTick > Server()->Tick())
				{
					//
				}
				else
				{
					GameServer()->SendChat(GameServer()->GetShopBot(), CGameContext::CHAT_TO_ONE_CLIENT, "Bye! Come back if you need something.", -1, m_pPlayer->GetCID());

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

int CCharacter::DDPP_DIE(int Killer, int Weapon, bool fngscore)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
	BlockKillingSpree(Killer); //should be renamed to KillingSpree(); because it is not in BlockPointsMain() func and handels all kinds of kills
	BlockTourna_Die(Killer);
	InstagibSubDieFunc(Killer, Weapon);
	SurvivalSubDieFunc(Killer, Weapon);

	//insta kills //TODO: combine with insta 1on1
	if (Killer != m_pPlayer->GetCID())
	{
		if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm || GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm)
		{
			GameServer()->DoInstaScore(3, Killer);
		}
		else if (g_Config.m_SvDDPPscore == 0)
		{
			GameServer()->m_apPlayers[Killer]->m_Score += 3;
		}
	}

	//insta 1on1
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

	//balance battel
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

	//blockwave minigame
	//commented out cuz idk why people shouldnt be able to play alone lol
	/*
	if (m_pPlayer->m_IsBlockWaving)
	{
		if (GameServer()->CountBlockWavePlayers() < 2)
		{
			GameServer()->m_BlockWaveGameState = 0; //stop blockwaving game
		}
	}
	*/

	//ChillerDragon pvparena code
	if (GameServer()->m_apPlayers[Killer])
	{
		if (GameServer()->GetPlayerChar(Killer) && Weapon != WEAPON_GAME && Weapon != WEAPON_SELF)
		{
			//GameServer()->GetPlayerChar(Killer)->m_Bloody = true;

			if (GameServer()->GetPlayerChar(Killer)->m_IsPVParena)
			{
				if (GameServer()->m_apPlayers[Killer]->m_level > GameServer()->m_apPlayers[Killer]->m_max_level || //dont give xp on max lvl
					GameServer()->IsSameIP(Killer, m_pPlayer->GetCID()) || //dont give xp on dummy kill
					GameServer()->IsSameIP(m_pPlayer->GetCID(), GameServer()->m_apPlayers[Killer]->m_pvp_arena_last_kill_id) //dont give xp on killing same ip twice in a row
					)
				{
					GameServer()->m_apPlayers[Killer]->MoneyTransaction(+150, "[PVP] +150 pvp_arena kill");
					GameServer()->m_apPlayers[Killer]->m_pvp_arena_kills++;

					str_format(aBuf, sizeof(aBuf), "[PVP] +150 money for killing %s", Server()->ClientName(m_pPlayer->GetCID()));
					GameServer()->SendChatTarget(Killer, aBuf);
				}
				else
				{
					GameServer()->m_apPlayers[Killer]->MoneyTransaction(+150, "+150 pvp_arena kill");
					GameServer()->m_apPlayers[Killer]->m_xp += 100;
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

	m_pPlayer->m_LastToucherID = -1;
	return Killer;
}

void CCharacter::BlockTourna_Die(int Killer)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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

				str_format(aBuf, sizeof(aBuf), "+%d (block tournament)", money_rew);
				GameServer()->m_apPlayers[wonID]->MoneyTransaction(+money_rew, aBuf);
				GameServer()->GiveXp(wonID, xp_rew);
				GameServer()->GiveBlockPoints(wonID, points_rew);
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

void CCharacter::DummyTick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_pPlayer->m_IsDummy)
	{
		if ((m_pPlayer->m_rainbow_offer != m_pPlayer->m_DummyRainbowOfferAmount) && !m_Rainbow)			 
		{																								
			m_Rainbow = true;	
			m_pPlayer->m_rainbow_offer = 0;
			m_pPlayer->m_DummyRainbowOfferAmount = m_pPlayer->m_rainbow_offer;							
		}																								
		else if ((m_pPlayer->m_rainbow_offer != m_pPlayer->m_DummyRainbowOfferAmount) && m_Rainbow)			
		{																								
			m_Rainbow = false;	
			m_pPlayer->m_rainbow_offer = 0;
			m_pPlayer->m_DummyRainbowOfferAmount = m_pPlayer->m_rainbow_offer;							
		}																								

		if (m_pPlayer->m_DummyMode == 0) //default and sample mode
		{
			/********************************
			*           weapons            *
			********************************/
			//SetWeapon(0); // hammer
			//SetWeapon(1); // gun
			//SetWeapon(2); // shotgun
			//SetWeapon(3); // grenade
			//SetWeapon(4); // rifle

			/********************************
			*           aiming             *
			********************************/
			//right and down:
			//m_Input.m_TargetX = 200;
			//m_Input.m_TargetY = 200;
			//m_LatestInput.m_TargetX = 200;
			//m_LatestInput.m_TargetY = 200;

			//left and up:
			//m_Input.m_TargetX = -200;
			//m_Input.m_TargetY = -200;
			//m_LatestInput.m_TargetX = -200;
			//m_LatestInput.m_TargetY = -200;

			//up:
			//m_Input.m_TargetX = 1; // straight up would be 0 but we better avoid using 0 (can cause bugs)
			//m_Input.m_TargetY = -200;
			//m_LatestInput.m_TargetX = 1;
			//m_LatestInput.m_TargetY = -200;

			/********************************
			*           moving             *
			********************************/
			//m_Input.m_Hook = 0; //don't hook
			//m_Input.m_Hook = 1; //hook
			//m_Input.m_Jump = 0; //don't jump
			//m_Input.m_Jump = 1; //jump
			//m_Input.m_Direction = 0; //don't walk
			//m_Input.m_Direction = -1; //walk left
			//m_Input.m_Direction = 1; //walk right

			/********************************
			*           shooting           *
			********************************/
			//m_LatestInput.m_Fire++; //fire!
			//m_Input.m_Fire++; //fire!
			//m_LatestInput.m_Fire = 0; //stop fire
			//m_Input.m_Fire = 0; //stop fire

			//

			/********************************
			*           conditions         *
			********************************/
			if (m_Core.m_Pos.x < 10 * 32)
			{
				//this code gets executed if the bot is in the first 10 left tiles of the map
				//m_Input.m_Direction = 1; //the bot walks to the right until he reaches the x coordinate 10
			}
			else
			{
				//this code gets executed if the bot is at x == 10 or higher
				//m_Input.m_Direction = -1; //the bot would go back to 10 if he is too far
			}
		}
		else if (m_pPlayer->m_DummyMode == 1)
		{
			m_Input.m_TargetX = 2;
			m_Input.m_TargetY = 2;
			m_LatestInput.m_TargetX = 2;
			m_LatestInput.m_TargetY = 2;
			if (Server()->Tick() % 100 == 0)
				SetWeapon(3);
			m_LatestInput.m_Fire++;
			m_Input.m_Fire++;
		}
		else if (m_pPlayer->m_DummyMode == 2)
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;


			//Selfkills
			if (isFreezed)
			{
				//wenn der bot freeze is warte erstmal n paar sekunden und dann kill dich
				if (Server()->Tick() % 300 == 0)
				{
					//Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}
			}




			char aBuf[256];
			//str_format(aBuf, sizeof(aBuf), "speed:  x: %f y: %f", m_Core.m_Vel.x, m_Core.m_Vel.y);
			//str_format(aBuf, sizeof(aBuf), "target:  x: %d y: %d", m_Input.m_TargetX, m_Input.m_TargetY);
			str_format(aBuf, sizeof(aBuf), "pos.x %.2f pos.y %.2f", m_Core.m_Pos.x, m_Core.m_Pos.y);
			//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

			if (1 == 2) //just for debuggin
			{
				CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
				if (pChr && pChr->IsAlive())
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				}
			}
			else //normal
			{
				float Angle = m_AngleTo;

				if (Server()->Tick() > m_AngleTickStart + m_AngleTickTime)
				{
					if (Server()->Tick() >= m_AngleTickNext)
					{
						m_AngleFrom = m_AngleTo;

						m_AngleTo += (rand() % 360) - 180;
						m_AngleTickTime = Server()->TickSpeed() / 2 + (rand() % (Server()->TickSpeed() / 2));

						m_AngleTickStart = Server()->Tick();
						m_AngleTickNext = m_AngleTickStart + m_AngleTickTime + Server()->TickSpeed() * 2 + Server()->TickSpeed() / 2 * (rand() % 10);

						// wann sollen die emotes gemacht werden? oder willst du das nachher machen? bei dem default mode is alles random bei dem anderen sollte es gefühle usw geben hier nich
						// pChr->m_EmoteType = EMOTE_HAPPY;
						// pChr->m_EmoteStop = Server()->Tick() + Server()->TickSpeed(); // = emote bleibt eine sekunde
						// tja, dachte das wär hier drin. schade
					}
				}
				else
				{
					Angle = m_AngleFrom + (m_AngleTo - m_AngleFrom) * (Server()->Tick() - m_AngleTickStart) / m_AngleTickTime;
				}

				float AngleRad = Angle * pi / 180.f;
				m_Input.m_TargetX = cosf(AngleRad) * 100;
				m_Input.m_TargetY = sinf(AngleRad) * 100;
			}

			// wenn targetX > 0 ist, guckt er nach rechts. bei < 0 halt nach links
			// bei targetY ist es so, wenn das > 0 ist guckt er nach UNTEN. bei < 0 nach OBEN

			// ok alles schön und gut aber wie lass ich den jetzt moven? also die augen nicht springen sondern bewegen

			// m_Input.m_TargetX = 1;
			// m_Input.m_TargetY = 0; // so guckt er halt z.B. nach rechts. da kannste jetzt mit rumrechnen (lass da pls) [steht doch oben ._.] egal

			// wenn du was mit der zeit rechnen willst, nimm den server tick. Server()->Tick() und Server()->TickSpeed() [das sollte eigentlich immer 50 sein]

			// m_Input.m_TargetX = Server()->Tick() % Server()->TickSpeed; // würde z.B. machen, dass m_TargetX zwischen 0 und 49 liegt (rechnet ja Modulo 50). nur so als beispiel. bringt in dem fall jetzt wenig

			//float Angle = (float)Server()->Tick(); // teste mal
			//float Angle = Server()->Tick() / (float)Server()->TickSpeed() * 360; // player dreht sich jede sekunde einmal
			//float Angle = (float)Server()->Tick() / (float)Server()->TickSpeed() * 720.f; // player dreht sich jede sekunde zwei mal (720 = 360*2)schon kla skype

			//float AngleRad = Angle * pi / 180.f;
			//m_Input.m_TargetX = cosf(AngleRad) * 100;
			//m_Input.m_TargetY = sinf(AngleRad) * 100; // probier das mal aus. sollte sich eigentlich alle 2 sekunden einmal komplett gedreht haben. (bin etwas müde ._.) :c

			if (Server()->Tick() >= m_EmoteTickNext)
			{
				m_pPlayer->m_LastEmote = Server()->Tick();
				int r = rand() % 100;
				GameServer()->SendEmoticon(m_pPlayer->GetCID(), r < 10 ? 5 : r < 55 ? 2 : 7);

				m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() * 10 + Server()->TickSpeed() * (rand() % 21);
			}
		}
		else if (m_pPlayer->m_DummyMode == -3)
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			CCharacter *pChr = GameServer()->m_World.ClosestCharTypeNotInFreeze(m_Pos, true, this, false);
			if (pChr && pChr->IsAlive())
			{
				m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				if (m_Core.m_Pos.y < pChr->m_Core.m_Pos.y - 60)
				{
					m_Input.m_Hook = 1;
				}


				if (m_Core.m_Pos.x < pChr->m_Core.m_Pos.x - 40)
				{
					m_Input.m_Direction = 1;
				}
				else
				{
					m_Input.m_Direction = -1;
				}

				if (m_Core.m_Pos.x > 411 * 32 && m_Core.m_Pos.x < 420 * 32) //hookbattle left entry
				{
					if (pChr->m_Core.m_Pos.x < m_Core.m_Pos.x + 8 * 32) //8 tiles hookrange
					{
						m_Input.m_Hook = 1;
					}

					if (m_Core.m_HookState == HOOK_GRABBED)
					{
						m_Input.m_Direction = -1;
					}

					//rehook
					if (Server()->Tick() % 15 == 0 && m_Core.m_HookState != HOOK_GRABBED)
					{
						m_Input.m_Hook = 0;
					}
				}

				if (pChr->m_FreezeTime) //go full yolo and ignore all freeze just hook the enemy into freeze (kamikaze style)
				{
					m_Input.m_Hook = 1;

					if (m_Core.m_Pos.x > 446 * 32) //right side of main area and right spawn
					{
						m_Input.m_Direction = 1;
					}
					else if (m_Core.m_Pos.x > 425 * 32 && m_Core.m_Pos.x < 434 * 32) //left side of the base prefer the freeze hole cuz its less kamikaze
					{
						m_Input.m_Direction = 1;
					}
					else
					{
						m_Input.m_Direction = -1;
					}

					if (Server()->Tick() % 9 == 0)
					{
						m_Input.m_Jump = 1;
					}

					if (Server()->Tick() % 15 == 0 && m_Core.m_HookState != HOOK_GRABBED)
					{
						m_Input.m_Hook = 0;
					}
				}
				else //no frozen enemy --> dont run into freeze
				{
					if (m_Core.m_Pos.x > 453 * 32 && m_Core.m_Pos.x < 457 * 32)
					{
						m_Input.m_Direction = -1; //dont run into right entry of main area
					}
				}
			}


			//Care your bot mates c:
			CCharacter *pBot = GameServer()->m_World.ClosestCharTypeDummy(m_Pos, this);
			if (pBot && pBot->IsAlive())
			{
				//chill dont push all at once
				if (m_Core.m_Pos.x < 404 * 32 && m_Core.m_Pos.x > 393 * 32 && pBot->m_Core.m_Pos.x > m_Core.m_Pos.x + 5 && !pBot->isFreezed) //left side of map
				{
					if (pBot->m_Core.m_Pos.x < m_Core.m_Pos.x + 8 * 32) //8 tiles distance
					{
						m_Input.m_Direction = -1;
					}
				}

				//get dj if mates block the fastlane on entering from left side
				if (m_Core.m_Pos.x > 414 * 32 && m_Core.m_Pos.x < 420 * 32 && !IsGrounded())
				{
					if (pBot->m_Core.m_Pos.x > 420 * 32 && pBot->m_Core.m_Pos.x < 423 * 32 + 20 && pBot->m_FreezeTime)
					{
						m_Input.m_Direction = -1;
					}
				}
			}



			if (m_Core.m_Pos.y > 262 * 32 && m_Core.m_Pos.x > 404 * 32 && m_Core.m_Pos.x < 415 * 32 && !IsGrounded()) //Likely to fail in the leftest freeze becken
			{
				m_Input.m_Jump = 1;
				if (m_Input.m_Direction == 0) //never stand still here
				{
					if (m_Core.m_Pos.x > 410 * 32)
					{
						m_Input.m_Direction = 1;
					}
					else
					{
						m_Input.m_Direction = -1;
					}
				}
			}


			//basic map dodigen

			if (m_Core.m_Pos.x < 392 * 32) //dont walk in the freeze wall on the leftest side
			{
				m_Input.m_Direction = 1;
			}

			if (m_Core.m_Pos.y < 257 * 32 && m_Core.m_Vel.y < -4.4f && m_Core.m_Pos.x < 456 * 32) //avoid hitting freeze roof
			{
				m_Input.m_Jump = 1;
				m_Input.m_Hook = 1;
			}


			if (m_Core.m_Pos.x > 428 * 32 && m_Core.m_Pos.x < 437 * 32) //freeze loch im main becken dodgen
			{
				if (m_Core.m_Pos.y > 263 * 32 && !IsGrounded())
				{
					m_Input.m_Jump = 1;
				}


				//position predefines
				if (m_Core.m_Pos.x > 423 * 32)
				{
					m_Input.m_Direction = 1;
				}
				else
				{
					m_Input.m_Direction = -1;
				}

				//velocity ovrrides
				if (m_Core.m_Vel.x > 2.6f)
				{
					m_Input.m_Direction = 1;
				}
				if (m_Core.m_Vel.x < -2.6f)
				{
					m_Input.m_Direction = -1;
				}
			}

			if (m_Core.m_Pos.x > 418 * 32 && m_Core.m_Pos.x < 422 * 32) //passing the freeze on the left side
			{
				if (m_Core.m_Pos.x < 420 * 32)
				{
					if (m_Core.m_Vel.x > 0.6f)
					{
						m_Input.m_Jump = 1;
					}
				}
				else
				{
					if (m_Core.m_Vel.x < -0.6f)
					{
						m_Input.m_Jump = 1;
					}
				}
			}

			if (m_Core.m_Pos.x > 457 * 32) //right spawn --->
			{
				m_Input.m_Direction = -1;
				if (m_Core.m_Pos.x < 470 * 32 && IsGrounded()) //start border jump
				{
					m_Input.m_Jump = 1;
				}
				else if (m_Core.m_Pos.x < 470 * 32 && !IsGrounded() && m_Core.m_Pos.y > 260 * 32)
				{
					m_Input.m_Jump = 1;
				}
			}

			if (m_Core.m_Pos.y < 254 * 32 && m_Core.m_Pos.x < 401 * 32 && m_Core.m_Pos.x > 397 * 32) //left spawn upper right freeze wall
			{
				m_Input.m_Direction = -1;
			}
		}
		else if (m_pPlayer->m_DummyMode == -4) //rifle fng
		{
			int offset_x = g_Config.m_SvDummyMapOffsetX * 32; //offset for ChillBlock5 -667
			int offset_y = g_Config.m_SvDummyMapOffsetY * 32;

			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			//attack enemys
			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
			if (pChr && pChr->IsAlive())
			{

				m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				if (pChr->m_FreezeTime < 1) //alive enemy --> attack
				{
					if (m_Core.m_Pos.x < pChr->m_Core.m_Pos.x)
					{
						m_Input.m_Direction = 1;
					}
					else
					{
						m_Input.m_Direction = -1;
					}

					if (Server()->Tick() % 100 == 0)
					{
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
					}
				}
				else //frozen enemy --> sacarfire
				{

				}
			}


			//don't fall in holes
			if (m_Core.m_Pos.x + offset_x > 90 * 32 && m_Core.m_Pos.x + offset_x < 180 * 32) //map middle (including 3 fall traps)
			{
				if (m_Core.m_Pos.y + offset_y > 73 * 32)
				{
					m_Input.m_Jump = 1;
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hopsa");
				}
			}

			//check for stucking in walls
			if (m_Input.m_Direction != 0 && m_Core.m_Vel.x == 0.0f)
			{
				if (Server()->Tick() % 60 == 0)
				{
					m_Input.m_Jump = 1;
				}
			}
		}
		else if (m_pPlayer->m_DummyMode == -5) //grenade fng
		{
			int offset_x = 0; //offset for ChillBlock5 667 (for rifle)
			int offset_y = 0;

			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			//attack enemys
			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
			if (pChr && pChr->IsAlive())
			{

				m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				if (m_Core.m_Pos.x < pChr->m_Core.m_Pos.x)
				{
					m_Input.m_Direction = 1;
				}
				else
				{
					m_Input.m_Direction = -1;
				}

				if (Server()->Tick() % 100 == 0)
				{
					m_Input.m_Fire++;
					m_LatestInput.m_Fire++;
				}
			}


			//don't fall in holes
			if (m_Core.m_Pos.x + offset_x > 90 * 32 && m_Core.m_Pos.x + offset_x < 180 * 32) //map middle (including 3 fall traps)
			{
				if (m_Core.m_Pos.y + offset_y > 73 * 32)
				{
					m_Input.m_Jump = 1;
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hopsa");
				}
			}

			//check for stucking in walls
			if (m_Input.m_Direction != 0 && m_Core.m_Vel.x == 0.0f)
			{
				if (Server()->Tick() % 60 == 0)
				{
					m_Input.m_Jump = 1;
				}
			}
		}
		else if (m_pPlayer->m_DummyMode == -6)  //ChillBlock5 blmapv3 1o1 mode
		{
			//rest dummy 
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
			if (pChr && pChr->IsAlive())
			{

				m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				/*************************************************
				*                                                *
				*                A T T A C K                     *
				*                                                *
				**************************************************/

				//swing enemy up
				if (m_Core.m_Pos.y < pChr->m_Pos.y - 20 && !IsGrounded() && !pChr->isFreezed)
				{
					m_Input.m_Hook = 1;
					float dist = distance(pChr->m_Pos, m_Core.m_Pos);
					if (dist < 250.f)
					{
						if (m_Core.m_Pos.x < pChr->m_Pos.x)
						{
							m_Input.m_Direction = -1;
						}
						else
						{
							m_Input.m_Direction = 1;
						}
						if (dist < 80.f) // hammer dist
						{
							if (abs(pChr->m_Core.m_Vel.x) > 2.6f)
							{
								if (m_FreezeTime == 0)
								{
									m_LatestInput.m_Fire++;
									m_Input.m_Fire++;
								}
							}
						}
					}
				}


				//attack in mid
				if (pChr->m_Pos.x > 393 * 32 - 7 + V3_OFFSET_X && pChr->m_Pos.x < 396 * 32 + 7 + V3_OFFSET_X)
				{
					if (pChr->m_Pos.x < m_Core.m_Pos.x) // bot on the left
					{
						if (pChr->m_Core.m_Vel.x < 0.0f)
						{
							m_Input.m_Hook = 1;
						}
						else
						{
							m_Input.m_Hook = 0;
						}
					}
					else // bot on the right
					{
						if (pChr->m_Core.m_Vel.x < 0.0f)
						{
							m_Input.m_Hook = 0;
						}
						else
						{
							m_Input.m_Hook = 1;
						}
					}

					//if (pChr->m_Pos.y > 77 * 32 - 1 + V3_OFFSET_Y && pChr->IsGrounded() == false && pChr->isFreezed)
					//{
					//	m_Input.m_Hook = 0; //rekt -> let him fall
					//}
					if (pChr->isFreezed)
					{
						m_Input.m_Hook = 0;
					}
				}

				//attack bot in the middle and enemy in the air -> try to hook down
				/* quick ascii art xd
                              #    #
                          #   #    #   #
                 enemy    #####    #####
  

                           bot
                          #####    #####
                          #   #    #   #
                              #
                 ###########################
				*/
				if (m_Core.m_Pos.y < 78 * 32 + V3_OFFSET_Y && m_Core.m_Pos.y > 70 * 32 + V3_OFFSET_Y && IsGrounded()) // if bot is in position
				{
					if (pChr->m_Pos.x < 389 * 32 + V3_OFFSET_X || pChr->m_Pos.x > 400 * 32 + V3_OFFSET_X) //enemy on the left side
					{
						if (pChr->m_Pos.y < 76 * 32 + V3_OFFSET_Y && pChr->m_Core.m_Vel.y > 4.2f)
						{
							m_Input.m_Hook = 1;
						}
					}

					if (m_Core.m_HookState == HOOK_FLYING)
					{
						m_Input.m_Hook = 1;
					}
					else if (m_Core.m_HookState == HOOK_GRABBED)
					{
						m_Input.m_Hook = 1;
						//stay strong and walk agianst hook pull
						if (m_Core.m_Pos.x < 392 * 32 + V3_OFFSET_X) //left side
						{
							m_Input.m_Direction = 1;
						}
						else if (m_Core.m_Pos.x > 397 * 32 + V3_OFFSET_X) //right side
						{
							m_Input.m_Direction = -1;
						}
					}
				}

				// attack throw into left freeze wall
				if (m_Core.m_Pos.x < 383 * 32 + V3_OFFSET_X)
				{
					if (pChr->m_Pos.y > m_Core.m_Pos.y + 190)
					{
						m_Input.m_Hook = 1;
					}
					else if (pChr->m_Pos.y < m_Core.m_Pos.y - 190)
					{
						m_Input.m_Hook = 1;
					}
					else
					{
						if (pChr->m_Core.m_Vel.x < -1.6f)
						{
							if (pChr->m_Pos.x < m_Core.m_Pos.x - 7 && pChr->m_Pos.x > m_Core.m_Pos.x - 90) //enemy on the left side
							{
								if (pChr->m_Pos.y < m_Core.m_Pos.y + 90 && pChr->m_Pos.y > m_Core.m_Pos.y - 90)
								{
									if (m_FreezeTime == 0)
									{
										m_LatestInput.m_Fire++;
										m_Input.m_Fire++;
									}
								}
							}
						}
					}
				}

				// attack throw into right freeze wall
				if (m_Core.m_Pos.x > 404 * 32 + V3_OFFSET_X)
				{
					if (pChr->m_Pos.y > m_Core.m_Pos.y + 190)
					{
						m_Input.m_Hook = 1;
					}
					else if (pChr->m_Pos.y < m_Core.m_Pos.y - 190)
					{
						m_Input.m_Hook = 1;
					}
					else
					{
						if (pChr->m_Core.m_Vel.x > 1.6f)
						{
							if (pChr->m_Pos.x > m_Core.m_Pos.x + 7 && pChr->m_Pos.x < m_Core.m_Pos.x + 90) //enemy on the right side
							{
								if (pChr->m_Pos.y > m_Core.m_Pos.y - 90 && pChr->m_Pos.y < m_Core.m_Pos.y + 90)
								{
									if (m_FreezeTime == 0)
									{
										m_LatestInput.m_Fire++;
										m_Input.m_Fire++;
									}
								}
							}
						}
					}
				}

				/*************************************************
				*                                                *
				*                D E F E N D (move)              *
				*                                                *
				**************************************************/

				//########################################
				//Worst hammer switch code eu west rofl! #
				//########################################
				//switch to hammer if enemy is near enough
				if (pChr->m_Pos.x > m_Core.m_Pos.x + 130)
				{
					//default is gun
					SetWeapon(1);
				}
				else if (pChr->m_Pos.x < m_Core.m_Pos.x - 130)
				{
					//default is gun
					SetWeapon(1);
				}
				else
				{
					//switch to hammer if enemy is near enough
					if (pChr->m_Pos.y > m_Core.m_Pos.y + 130)
					{
						//default is gun
						SetWeapon(1);
					}
					else if (pChr->m_Pos.y < m_Core.m_Pos.y - 130)
					{
						//default is gun
						SetWeapon(1);
					}
					else
					{
						//near --> hammer
						SetWeapon(0);
					}
				}

				//Starty movement
				if (m_Core.m_Pos.x < 389 * 32 + V3_OFFSET_X && m_Core.m_Pos.y > 79 * 32 + V3_OFFSET_Y && pChr->m_Pos.y > 79 * 32 + V3_OFFSET_Y && pChr->m_Pos.x > 398 * 32 + V3_OFFSET_X && IsGrounded() && pChr->IsGrounded())
				{
					m_Input.m_Jump = 1;
				}
				if (m_Core.m_Pos.x < 389 * 32 + V3_OFFSET_X && pChr->m_Pos.x > 307 * 32 + V3_OFFSET_X && pChr->m_Pos.x > 398 * 32 + V3_OFFSET_X)
				{
					m_Input.m_Direction = 1;
				}



				//important freeze doges leave them last!:

				//freeze prevention mainpart down right
				if (m_Core.m_Pos.x > 397 * 32 + V3_OFFSET_X && m_Core.m_Pos.x < 401 * 32 + V3_OFFSET_X && m_Core.m_Pos.y > 78 * 32 + V3_OFFSET_Y)
				{
					m_Input.m_Jump = 1;
					m_Input.m_Hook = 1;
					if (Server()->Tick() % 20 == 0)
					{
						m_Input.m_Hook = 0;
						m_Input.m_Jump = 0;
					}
					m_Input.m_Direction = 1;
					m_Input.m_TargetX = 200;
					m_Input.m_TargetY = 80;
				}

				//freeze prevention mainpart down left
				if (m_Core.m_Pos.x > 387 * 32 + V3_OFFSET_X && m_Core.m_Pos.x < 391 * 32 + V3_OFFSET_X && m_Core.m_Pos.y > 78 * 32 + V3_OFFSET_Y)
				{
					m_Input.m_Jump = 1;
					m_Input.m_Hook = 1;
					if (Server()->Tick() % 20 == 0)
					{
						m_Input.m_Hook = 0;
						m_Input.m_Jump = 0;
					}
					m_Input.m_Direction = -1;
					m_Input.m_TargetX = -200;
					m_Input.m_TargetY = 80;
				}

				//Freeze prevention top left
				if (m_Core.m_Pos.x < 391 * 32 + V3_OFFSET_X && m_Core.m_Pos.y < 71 * 32 + V3_OFFSET_Y && m_Core.m_Pos.x > 387 * 32 - 10 + V3_OFFSET_X)
				{
					m_Input.m_Direction = -1;
					m_Input.m_Hook = 1;
					if (Server()->Tick() % 20 == 0)
					{
						m_Input.m_Hook = 0;
					}
					m_Input.m_TargetX = -200;
					m_Input.m_TargetY = -87;
					if (m_Core.m_Pos.y > 19 * 32 + 20)
					{
						m_Input.m_TargetX = -200;
						m_Input.m_TargetY = -210;
					}
				}

				//Freeze prevention top right
				if (m_Core.m_Pos.x < 402 * 32 + 10 + V3_OFFSET_X && m_Core.m_Pos.y < 71 * 32 + V3_OFFSET_Y && m_Core.m_Pos.x > 397 * 32 + V3_OFFSET_X)
				{
					m_Input.m_Direction = 1;
					m_Input.m_Hook = 1;
					if (Server()->Tick() % 20 == 0)
					{
						m_Input.m_Hook = 0;
					}
					m_Input.m_TargetX = 200;
					m_Input.m_TargetY = -87;
					if (m_Core.m_Pos.y > 67 * 32 + 20 + V3_OFFSET_Y)
					{
						m_Input.m_TargetX = 200;
						m_Input.m_TargetY = -210;
					}
				}

				//Freeze prevention mid
				if (m_Core.m_Pos.x > 393 * 32 - 7 + V3_OFFSET_X && m_Core.m_Pos.x < 396 * 32 + 7 + V3_OFFSET_X)
				{
					if (m_Core.m_Vel.x < 0.0f)
					{
						m_Input.m_Direction = -1;
					}
					else
					{
						m_Input.m_Direction = 1;
					}

					if (m_Core.m_Pos.y > 77 * 32 - 1 + V3_OFFSET_Y && IsGrounded() == false)
					{
						m_Input.m_Jump = 1;
						if (m_Core.m_Jumped > 2) //no jumps == rip   --> panic hook
						{
							m_Input.m_Hook = 1;
							if (Server()->Tick() % 15 == 0)
							{
								m_Input.m_Hook = 0;
							}
						}
					}
				}

				//Freeze prevention left 
				if (m_Core.m_Pos.x < 380 * 32 + V3_OFFSET_X || m_Core.m_Pos.x < 382 * 32 + V3_OFFSET_X && m_Core.m_Vel.x < -8.4f)
				{
					m_Input.m_Direction = 1;
				}
				//Freeze prevention right
				if (m_Core.m_Pos.x > 408 * 32 + V3_OFFSET_X || m_Core.m_Pos.x > 406 * 32 + V3_OFFSET_X && m_Core.m_Vel.x > 8.4f)
				{
					m_Input.m_Direction = -1;
				}
			}

		}
		else if (m_pPlayer->m_DummyMode == 3)
		{
			//rest dummy (zuruecksetzten)
			//m_Input.m_Hook = 0;
			//m_Input.m_Jump = 0;
			//m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;



			//m_Core.m_ActiveWeapon = WEAPON_HAMMER;







			//happy finish
			if (m_DummyFinished == true)
			{
				if (m_DummyFinishes == 1)
				{
					m_DummyShowRank = true;
					m_Input.m_Jump = 1;
					m_Input.m_Jump = 0;
					m_Input.m_Jump = 1;
					m_Input.m_Jump = 0;
					GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3);
					GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "GoodGame :)! Thank you team, for helping me finish this cool map!");
					m_DummyFinished = false;
				}
				else if (m_DummyFinishes == 2)
				{
					GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Yay! Finished again!");
					m_DummyFinished = false;

				}
				else
				{
					GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "gg :)");
					m_DummyFinished = false;
				}
			}
			//showing rank
			if (m_DummyShowRank == true)
			{
				if (frandom() * 100 < 2)
				{
					GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Look at my nice rank!");
					GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Type /rank");
					m_DummyShowRank = false;
				}
			}


			//weapon switch old
			/*
			if (Server()->Tick() >= m_EmoteTickNext)
			{
			m_pPlayer->m_LastEmote = Server()->Tick();
			//m_Core.m_ActiveWeapon = WEAPON_GUN;
			DoWeaponSwitch();

			m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() / 10 + Server()->TickSpeed() * (rand() % 21);
			}
			else
			{
			m_Core.m_ActiveWeapon = WEAPON_HAMMER;
			}
			*/




			//emotes fast
			if (Server()->Tick() >= m_EmoteTickNext)
			{
				m_pPlayer->m_LastEmote = Server()->Tick();


				int r = rand() % 100;
				GameServer()->SendEmoticon(m_pPlayer->GetCID(), r < 10 ? 5 : r < 55 ? 2 : 7);

				m_DummyHammer ^= true;

				m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() / 6;
			}


			//weapons
			if (m_FreezeTime == 0)
			{
				if (m_DummyHammer) //bubub
				{
					//m_Core.m_ActiveWeapon = WEAPON_HAMMER;
					SetWeapon(0);
				}
				else
				{
					SetWeapon(1);
					//m_Core.m_ActiveWeapon = WEAPON_GUN;
				}
			}

			// moving eyes

			float Angle = m_AngleTo;

			// überprüfen ob die animation schon vorbei ist (also um z.B. 90° schon gedreht wurde)
			if (Server()->Tick() > m_AngleTickStart + m_AngleTickTime)
			{
				// danach überprüfen, ob es auch schon zeit ist, um die nächste animation zu starten
				if (Server()->Tick() >= m_AngleTickNext)
				{
					m_AngleFrom = m_AngleTo; // erst mal die startrotation auf die vorherige endrotation setzen

					m_AngleTo += (rand() % 360) - 180; // dann die neue endrotation setzen (= die vorherige rotation + irgendeine zufällige zahl zwischen -180 und + 180)
					m_AngleTickTime = Server()->TickSpeed() / 15 + (rand() % (Server()->TickSpeed() / 15)); // wie lange die animation geht, hier also eine halbe sekunde + 

					m_AngleTickStart = Server()->Tick() / 10; // startzeit für die nächste animation setzen
					m_AngleTickNext = m_AngleTickStart + m_AngleTickTime + Server()->TickSpeed() * 2 + Server()->TickSpeed() / 2 * (rand() % 10);


					//jup alles ganz gut erkärt auch halbwegs verstanden aber selber umwenden hmmmm


					// wann sollen die emotes gemacht werden? oder willst du das nachher machen? bei dem default mode is alles random bei dem anderen sollte es gefühle usw geben hier nich
					// pChr->m_EmoteType = EMOTE_HAPPY; 
					// pChr->m_EmoteStop = Server()->Tick() + Server()->TickSpeed(); // = emote bleibt eine sekunde
					// tja, dachte das wär hier drin. schade
				}
			}
			else
			{
				// wenn die animation noch nicht vorbei ist, dann wird die rotation entsprechend auf einen wert zwischen z.B. 0° und 90° (also start und endrotation) gesetzt
				Angle = m_AngleFrom + (m_AngleTo - m_AngleFrom) * (Server()->Tick() - m_AngleTickStart) / m_AngleTickTime;
			}

			float AngleRad = Angle * pi / 180.f;
			m_Input.m_TargetX = cosf(AngleRad) * 100;
			m_Input.m_TargetY = sinf(AngleRad) * 100;

			//moving eyes end

			//shot schiessen schissen
			//im freeze nicht schiessen
			if (m_FreezeTime == 0)
			{
				m_LatestInput.m_TargetX = cosf(AngleRad) * 100;
				m_LatestInput.m_TargetY = sinf(AngleRad) * 100;
				m_LatestInput.m_Fire++;
				m_Input.m_Fire++;
				//FireWeapon(true);
				//m_Input.m_Jump = 1;
			}


			//hooken
			if (m_Core.m_HookState == HOOK_FLYING)
				m_Input.m_Hook = 1;
			else if (m_Core.m_HookState == HOOK_GRABBED)
			{
				if (frandom() * 250 < 1) //hmm hä xD ich will das er selten ganz lange hookt isses dass? sehr wsaelten. was stand da vorher
				{
					if (frandom() * 250 == 1)
						m_Input.m_Hook = 0;
					else
						m_Input.m_Hook = 1;
				}
				else
				{
					if (frandom() * 250 < 1)
						m_Input.m_Hook = 0;
					else
						m_Input.m_Hook = 1;
				}
			}
			else
			{
				if (frandom() * 250 < 3)
				{
					m_Input.m_Hook = 1;
				}
				else
					m_Input.m_Hook = 0;
			}

			//laufen (move xD)
			if (m_StopMoveTick && Server()->Tick() >= m_StopMoveTick)
			{
				m_LastMoveDirection = m_Input.m_Direction;
				m_Input.m_Direction = 0;
				m_StopMoveTick = 0;
			}
			if (Server()->Tick() >= m_MoveTick)
			{
				int Direction = rand() % 6;
				if (m_LastMoveDirection == -1)
				{
					if (Direction < 2)
						m_Input.m_Direction = 1;
					else
						m_Input.m_Direction = -1;
				}
				else
				{
					if (Direction < 2)
						m_Input.m_Direction = -1;
					else
						m_Input.m_Direction = 1;
				}
				m_StopMoveTick = Server()->Tick() + Server()->TickSpeed();
				m_MoveTick = Server()->Tick() + Server()->TickSpeed() * 3 + Server()->TickSpeed() * (rand() % 6);
			}
		}
		else if (m_pPlayer->m_DummyMode == 4) //mode 3 + selfkill
		{
			//rest dummy (zuruecksetzten)
			//m_Input.m_Hook = 0;
			//m_Input.m_Jump = 0;
			//m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;



			//m_Core.m_ActiveWeapon = WEAPON_HAMMER;
			SetWeapon(0);



			//freeze selfkilll

			if (m_DummyFreezed)
			{
				Die(m_pPlayer->GetCID(), WEAPON_SELF);
				m_DummyFreezed = false;
			}


			//weapon switch
			if (Server()->Tick() >= m_EmoteTickNext)
			{
				m_pPlayer->m_LastEmote = Server()->Tick();
				//m_Core.m_ActiveWeapon = WEAPON_GUN;
				//DoWeaponSwitch();

				m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() / 10 + Server()->TickSpeed() * (rand() % 21);
			}
			else
			{
				//m_Core.m_ActiveWeapon = WEAPON_HAMMER;
				SetWeapon(0);
			}






			//emotes

			if (Server()->Tick() >= m_EmoteTickNext)
			{
				m_pPlayer->m_LastEmote = Server()->Tick();
				int r = rand() % 100;
				GameServer()->SendEmoticon(m_pPlayer->GetCID(), r < 10 ? 5 : r < 55 ? 2 : 7);

				m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() * 5 + Server()->TickSpeed() * (rand() % 21);
			}

			// moving eyes

			float Angle = m_AngleTo;

			// überprüfen ob die animation schon vorbei ist (also um z.B. 90° schon gedreht wurde)
			if (Server()->Tick() > m_AngleTickStart + m_AngleTickTime)
			{
				// danach überprüfen, ob es auch schon zeit ist, um die nächste animation zu starten
				if (Server()->Tick() >= m_AngleTickNext)
				{
					m_AngleFrom = m_AngleTo; // erst mal die startrotation auf die vorherige endrotation setzen

					m_AngleTo += (rand() % 360) - 180; // dann die neue endrotation setzen (= die vorherige rotation + irgendeine zufällige zahl zwischen -180 und + 180)
					m_AngleTickTime = Server()->TickSpeed() / 15 + (rand() % (Server()->TickSpeed() / 15)); // wie lange die animation geht, hier also eine halbe sekunde + 

					m_AngleTickStart = Server()->Tick() / 10; // startzeit für die nächste animation setzen
					m_AngleTickNext = m_AngleTickStart + m_AngleTickTime + Server()->TickSpeed() * 2 + Server()->TickSpeed() / 2 * (rand() % 10);


					//jup alles ganz gut erkärt auch halbwegs verstanden aber selber umwenden hmmmm


					// wann sollen die emotes gemacht werden? oder willst du das nachher machen? bei dem default mode is alles random bei dem anderen sollte es gefühle usw geben hier nich
					// pChr->m_EmoteType = EMOTE_HAPPY; 
					// pChr->m_EmoteStop = Server()->Tick() + Server()->TickSpeed(); // = emote bleibt eine sekunde
					// tja, dachte das wär hier drin. schade
				}
			}
			else
			{
				// wenn die animation noch nicht vorbei ist, dann wird die rotation entsprechend auf einen wert zwischen z.B. 0° und 90° (also start und endrotation) gesetzt
				Angle = m_AngleFrom + (m_AngleTo - m_AngleFrom) * (Server()->Tick() - m_AngleTickStart) / m_AngleTickTime;
			}

			float AngleRad = Angle * pi / 180.f;
			m_Input.m_TargetX = cosf(AngleRad) * 100;
			m_Input.m_TargetY = sinf(AngleRad) * 100;

			//moving eyes end

			//shot schiessen schissen
			if (frandom() * 25 < 3)// probier mal jetzt ob sich was ändert

			{
				m_LatestInput.m_TargetX = cosf(AngleRad) * 100;
				m_LatestInput.m_TargetY = sinf(AngleRad) * 100;
				m_LatestInput.m_Fire++;
				m_Input.m_Fire++;
				//FireWeapon(true);
				//m_Input.m_Jump = 1;
			}


			//hooken
			if (m_Core.m_HookState == HOOK_FLYING)
				m_Input.m_Hook = 1;
			else if (m_Core.m_HookState == HOOK_GRABBED)
			{
				if (frandom() * 250 < 1) //hmm hä xD ich will das er selten ganz lange hookt isses dass? sehr wsaelten. was stand da vorher
				{
					if (frandom() * 250 == 1)
						m_Input.m_Hook = 0;
					else
						m_Input.m_Hook = 1;
				}
				else
				{
					if (frandom() * 250 < 1)
						m_Input.m_Hook = 0;
					else
						m_Input.m_Hook = 1;
				}
			}
			else
			{
				if (frandom() * 250 < 3)
				{
					m_Input.m_Hook = 1;
				}
				else
					m_Input.m_Hook = 0;
			}

			//laufen (move xD)
			if (m_StopMoveTick && Server()->Tick() >= m_StopMoveTick)
			{
				m_LastMoveDirection = m_Input.m_Direction;
				m_Input.m_Direction = 0;
				m_StopMoveTick = 0;
			}
			if (Server()->Tick() >= m_MoveTick)
			{
				int Direction = rand() % 6;
				if (m_LastMoveDirection == -1)
				{
					if (Direction < 2)
						m_Input.m_Direction = 1;
					else
						m_Input.m_Direction = -1;
				}
				else
				{
					if (Direction < 2)
						m_Input.m_Direction = -1;
					else
						m_Input.m_Direction = 1;
				}
				m_StopMoveTick = Server()->Tick() + Server()->TickSpeed();
				m_MoveTick = Server()->Tick() + Server()->TickSpeed() * 3 + Server()->TickSpeed() * (rand() % 6);
			}
		}
		else if (m_pPlayer->m_DummyMode == 9)  //move left
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			m_Input.m_Direction = -1;
		}
		else if (m_pPlayer->m_DummyMode == 10)  //move right
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			m_Input.m_Direction = 1;
		}
		else if (m_pPlayer->m_DummyMode == 11)  //move Jump
		{

			char aBuf[256];
			str_format(aBuf, sizeof(aBuf), "speed:  x: %f y: %f", m_Core.m_Vel.x, m_Core.m_Vel.y);

			//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			m_Input.m_Jump = 1;
		}
		else if (m_pPlayer->m_DummyMode == 12)  //Damage dude
		{
			m_isDmg = true;

			if (Server()->Tick() >= m_EmoteTickNext)
			{
				m_pPlayer->m_LastEmote = Server()->Tick();
				int r = rand() % 100;
				GameServer()->SendEmoticon(m_pPlayer->GetCID(), r < 10 ? 5 : r < 55 ? 2 : 7);

				m_Input.m_Jump = 1;

				m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() + Server()->TickSpeed() * (rand() % 4);
			}
			else
			{
				m_Input.m_Jump = 0;
			}

			//laufen (move xD)
			if (m_StopMoveTick && Server()->Tick() >= m_StopMoveTick)
			{
				m_LastMoveDirection = m_Input.m_Direction;
				m_Input.m_Direction = 0;
				m_StopMoveTick = 0;
			}
			if (Server()->Tick() >= m_MoveTick)
			{
				int Direction = rand() % 6;
				if (m_LastMoveDirection == -1)
				{
					if (Direction < 2)
						m_Input.m_Direction = 1;
					else
						m_Input.m_Direction = -1;
				}
				else
				{
					if (Direction < 2)
						m_Input.m_Direction = -1;
					else
						m_Input.m_Direction = 1;
				}
				m_StopMoveTick = Server()->Tick() + Server()->TickSpeed() * 10 / Server()->TickSpeed() * (rand() % 60);
				m_MoveTick = Server()->Tick() + Server()->TickSpeed() * 3 + Server()->TickSpeed() * (rand() % 6);
			}

		}
		else if (m_pPlayer->m_DummyMode == 13)  // fly bot
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			m_Input.m_TargetX = 0;
			m_Input.m_TargetY = -1;
			m_LatestInput.m_TargetX = 0;
			m_LatestInput.m_TargetY = -1;


			if (m_Core.m_Vel.y > 0)
			{
				m_Input.m_Hook = 1;
			}
			else
			{
				m_Input.m_Hook = 0;
			}

		}
		else if (m_pPlayer->m_DummyMode == 14)  // fly bot left
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = -1;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			m_Input.m_TargetX = 0;
			m_Input.m_TargetY = -1;



			if (m_Core.m_Vel.y > 0)
			{
				m_Input.m_Hook = 1;
			}
			else
			{
				m_Input.m_Hook = 0;
			}

		}
		else if (m_pPlayer->m_DummyMode == 15)  // fly bot right
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 1;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			m_Input.m_TargetX = 0;
			m_Input.m_TargetY = -1;



			if (m_Core.m_Vel.y > 0)
			{
				m_Input.m_Hook = 1;
			}
			else
			{
				m_Input.m_Hook = 0;
			}

		}                                //this mode <3 18 reks em all
		else if (m_pPlayer->m_DummyMode == 18)//pathlaufing system (become ze ruler xD) // ich bin der mode um den es hier geht
		{
			m_Input.m_Hook = 0;
			//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "HALLO ICH BIN 18!");

			//Check ob dem bot langweilig geworden is :)

			//m_Dummy_bored_counter = 5;

			if (m_Dummy_bored_counter > 2)
			{
				m_Dummy_bored = true;
			}


			/*
			BRAND NEW STRUCTURE!
			WELCOME TO 18's SPECIAL MODE-CEPTION!

			dummymode 18 hase his own modes in the mode lol


			:::::::::::::::::::::
			dummymode18 modes
			:::::::::::::::::::::
			mode:         desc:
			0					Main mode
			1					attack mode (if ruler spot is ruled and bot is in tunnel)
			2                   different wayblock mode
			3                   (PLANNED) 1on1 mode with counting in chat and helping




			dummymode18 code structure:
			- Check for activating other modes
			- big if clause with all modes

			*/


			//Check for activating other modes

			//Check mode 1 [Attack from tunnel wayblocker]
			//man könnte das auch mit m_Dummy_happy abfragen aber mich nich ganz so viel sinn
			if (m_Core.m_Pos.y > 214 * 32 && m_Core.m_Pos.x > 424 * 32)
			{
				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerWB(m_Pos, true, this);
				if (pChr && pChr->IsAlive())
				{

					//Wenn der bot im tunnel ist und ein Gegner im RulerWB bereich
					m_Dummy_mode18 = 1;
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Wayblocker gesichtet");

				}
			}
			else if (m_Dummy_happy && m_Dummy_bored)
			{
				m_Dummy_mode18 = 2;
			}
			else if (m_Dummy_special_defend) //Check mode 3 [Attack from tunnel wayblocker]
			{
				m_Dummy_mode18 = 3;
			}
			else
			{
				m_Dummy_mode18 = 0; //change to main mode
			}




			//Modes:

			if (m_Dummy_mode18 == 3) //special defend mode
			{
				//testy wenn der dummy in den special defend mode gesetzt wird pusht das sein adrenalin und ihm is nicht mehr lw
				m_Dummy_bored_counter = 0;

				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this);
				if (pChr && pChr->IsAlive())
				{

					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;



					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					//rest on tick
					m_Input.m_Hook = 0;
					m_Input.m_Jump = 0;
					m_Input.m_Direction = 0;
					m_LatestInput.m_Fire = 0;
					m_Input.m_Fire = 0;
					SetWeapon(1); //gun verwenden damit auch erkannt wird wann der mode getriggert wird


					if (pChr->m_FreezeTime == 0)
					{
						//wenn der gegner doch irgendwie unfreeze wird übergib an den main mode und lass den notstand das regeln
						m_Dummy_special_defend = false;
						m_Dummy_special_defend_attack = false;
					}
					//mode18 sub mode 3
					//Main code:
					//warte bis der gegner auf den boden geklatscht ist
					//dann werf ihn rechts runter

					if (pChr->m_Core.m_Vel.y > -0.9f && pChr->m_Pos.y > 211 * 32)
					{
						//wenn der gegner am boden liegt starte angriff
						m_Dummy_special_defend_attack = true;

						//start jump
						m_Input.m_Jump = 1;
					}


					if (m_Dummy_special_defend_attack)
					{
						if (m_Core.m_Pos.x - pChr->m_Pos.x < 50) //wenn der gegner nah genung is mach dj
						{
							m_Input.m_Jump = 1;
						}

						if (pChr->m_Pos.x < m_Core.m_Pos.x)
						{
							m_Input.m_Hook = 1;
						}
						else //wenn der gegner weiter rechts als der bot is lass los und übergib an main deine arbeit ist hier getahen
						{    //main mode wird evenetuell noch korrigieren mit schieben
							m_Dummy_special_defend = false;
							m_Dummy_special_defend_attack = false;
						}

						//Der bot sollte möglichst weit nach rechts gehen aber natürlich nicht ins freeze

						if (m_Core.m_Pos.x < 427 * 32 + 15)
						{
							m_Input.m_Direction = 1;
						}
						else
						{
							m_Input.m_Direction = -1;
						}

					}

				}
				else //wenn kein gegner mehr im Ruler bereich is
				{
					m_Dummy_special_defend = false;
					m_Dummy_special_defend_attack = false;
				}
			}
			else if (m_Dummy_mode18 == 2) //different wayblock mode
			{
				//rest on tick
				m_Input.m_Hook = 0;
				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;
				SetWeapon(0);


				//Selfkills (bit random but they work)
				if (isFreezed)
				{
					//wenn der bot freeze is warte erstmal n paar sekunden und dann kill dich
					if (Server()->Tick() % 300 == 0)
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
						m_Dummy_happy = false;
					}
				}



				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler2(m_Pos, true, this);
				if (pChr && pChr->IsAlive())
				{
					//Check ob an notstand mode18 = 0 übergeben
					if (pChr->m_FreezeTime == 0)
					{
						m_Dummy_bored = false;
						m_Dummy_bored_counter = 0;
						m_Dummy_mode18 = 0;
					}




					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					m_Input.m_Jump = 1;

					if (pChr->m_Pos.y > m_Core.m_Pos.y) //solange der bot über dem gegner ist (damit er wenn er ihn weg hammert nicht weiter hookt)
					{
						m_Input.m_Hook = 1;
					}

					if (m_Core.m_Pos.x > 420 * 32)
					{
						m_Input.m_Direction = -1;
					}

					if (pChr->m_Pos.y < m_Core.m_Pos.y + 15)
					{
						m_LatestInput.m_Fire++;
						m_Input.m_Fire++;
					}

				}
				else //lieblings position finden wenn nichts abgeht
				{
					if (m_Core.m_Pos.x < 421 * 32)
					{
						m_Input.m_Direction = 1;
					}
					else if (m_Core.m_Pos.x > 422 * 32 + 30)
					{
						m_Input.m_Direction = -1;
					}
				}
			}
			else if (m_Dummy_mode18 == 1) //attack in tunnel
			{
				//Selfkills (bit random but they work)
				if (isFreezed)
				{
					//wenn der bot freeze is warte erstmal n paar sekunden und dann kill dich
					if (Server()->Tick() % 300 == 0)
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}
				}

				//stay on position

				if (m_Core.m_Pos.x < 426 * 32 + 10) // zu weit links
				{
					m_Input.m_Direction = 1; //geh rechts
				}
				else if (m_Core.m_Pos.x > 428 * 32 - 10) //zu weit rechts
				{
					m_Input.m_Direction = -1; // geh links
				}
				else if (m_Core.m_Pos.x > 428 * 32 + 10) // viel zu weit rechts
				{
					m_Input.m_Direction = -1; // geh links
					m_Input.m_Jump = 1;
				}
				else
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerWB(m_Pos, true, this);
					if (pChr && pChr->IsAlive())
					{
						if (pChr->m_Pos.x < 436 * 32) //wenn er ganz weit über dem freeze auf der kante ist (hooke direkt)
						{
							m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
						}
						else //wenn der Gegner weiter hinter dem unhook ist (hook über den Gegner um ihn trozdem zu treffen und das unhook zu umgehen)
						{
							m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x - 50;
							m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x - 50;
							m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
						}


						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "targX: %d = %d - %d", m_Input.m_TargetX, pChr->m_Pos.x, m_Pos.x);
						//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);


						//m_Input.m_Hook = 0;
						CCharacter *pChr = GameServer()->m_World.ClosestCharTypeTunnel(m_Pos, true, this);
						if (pChr && pChr->IsAlive())
						{
							//wenn jemand im tunnel is check ob du nicht ausversehen den hookst anstatt des ziels in der WB area
							if (pChr->m_Pos.x < m_Core.m_Pos.x) //hooke nur wenn kein Gegner rechts von dem bot im tunnel is (da er sonst ziemlich wahrscheinlich den hooken würde)
							{
								m_Input.m_Hook = 1;
							}

						}
						else
						{
							//wenn eh keiner im tunnel is hau raus dat ding
							m_Input.m_Hook = 1;
						}

						//schau ob sich der gegner bewegt und der bot grad nicht mehr am angreifen iss dann resette falls er davor halt misshookt hat
						//geht nich -.-
						/*	if (!m_Core.m_HookState == HOOK_FLYING && !m_Core.m_HookState == HOOK_GRABBED)
						{
						if (Server()->Tick() % 10 == 0)
						m_Input.m_Hook = 0;
						}*/




						if (m_Core.m_Vel.x > 3.0f)
						{
							m_Input.m_Direction = -1;
						}
						else
						{
							m_Input.m_Direction = 0;
						}

					}
					else
					{
						m_Dummy_mode18 = 0;
					}
				}



			}
			else if (m_Dummy_mode18 == 0) //main mode
			{
				//if (mode18_main_init)
				//{
				//	//initialzing main mode...
				//	//resetting stuff...
				//	m_Input.m_Hook = 0;
				//}

				//m_Input.m_Hook = 0;
				//if (m_Core.m_HookState == HOOK_FLYING)
				//	m_Input.m_Hook = 1;
				//else if (m_Core.m_HookState == HOOK_GRABBED)
				//	m_Input.m_Hook = 1;
				//else
				//	m_Input.m_Hook = 0;

				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;





				//char aBuf[256];
				//str_format(aBuf, sizeof(aBuf), "speed:  x: %f y: %f speed pChr:  x: %f y: %f", m_Core.m_Vel.x, m_Core.m_Vel.y);

				//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);


				//if (1 == 1)
				//{
				//	CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
				//	if (pChr && pChr->IsAlive())
				//	{
				//		char aBuf[256];
				//		str_format(aBuf, sizeof(aBuf), "speed pChr:  x: %f y: %f", pChr->m_Core.m_Vel.x, pChr->m_Core.m_Vel.y);

				//		//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
				//	}
				//}



				//m_pPlayer->m_TeeInfos.m_Name = aBuf; 

				if (m_Core.m_Vel.x > 1.0f)
				{
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "speed: schneller als 1");
				}


				//Check ob jemand in der linken freeze wand is

				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerLeftFreeze(m_Pos, true, this);  //wenn jemand rechts im freeze liegt
				if (pChr && pChr->IsAlive()) // wenn ein spieler rechts im freeze lebt
				{  //----> versuche im notstand nicht den gegner auch da rein zu hauen da ist ja jetzt voll

					m_Dummy_left_freeze_full = true;
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Da liegt einer im freeze");
				}
				else // wenn da keiner is fülle diesen spot (linke freeze wand im ruler spot)
				{
					m_Dummy_left_freeze_full = false;
				}





				//Selfkill

				if (m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.x > 325 * 32 && m_Core.m_Pos.y > 215 * 32)  //Links am spawn runter
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Links am spawn runter");
				}
				//else if ((m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x > 392 * 32 && m_Core.m_Pos.y > 190) || (m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.y > 190)) //freeze decke am spawn
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke am spawn");
				//}
				//else if (m_Core.m_Pos.y > 218 * 32 + 31 /* für tee balance*/ && m_Core.m_Pos.x < 415 * 32) //freeze boden am spawn
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze boden am spawn");
				//}
				else if (m_Core.m_Pos.y < 215 * 32 && m_Core.m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.x < 428 * 32) //freeze decke im tunnel
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke im tunnel");
				}
				else if (m_Core.m_Pos.y > 222 * 32) //freeze becken unter area
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze becken unter area");
				}
				else if (m_Core.m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 436 * 32) //freeze rechts neben freeze becken
				{
					//Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze rechts neben freeze becken");
				}
				else if (m_Core.m_Pos.x > 469 * 32) //zu weit ganz rechts in der ruler area
				{
					//Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "zu weit ganz rechts in der ruler area");
				}
				else if (m_Core.m_Pos.y > 211 * 32 + 34 && m_Core.m_Pos.x > 455 * 32) //alles freeze am boden rechts in der area
				{
					//Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze boden rechts der area");
				}

				if (m_Core.m_Pos.y < 220 * 32 && m_Core.m_Pos.x < 415 * 32 && m_FreezeTime > 1) //always suicide on freeze if not reached teh block area yet
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze und links der block area");
				}

				if (m_Core.m_Pos.x < 388 * 32 && m_Core.m_Pos.y > 213 * 32) //jump to old spawn
				{
					m_Input.m_Jump = 1;
					m_Input.m_Fire++;
					m_LatestInput.m_Fire++;
					m_Input.m_Hook = 1;
					m_Input.m_TargetX = -200;
					m_Input.m_TargetY = 0;
				}

				// Movement
				//CheckFatsOnSpawn

				if (m_Core.m_Pos.x < 406 * 32)
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
					if (pChr && pChr->IsAlive())
					{

						m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
						m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

						m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
						m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;


						if (pChr->m_Pos.x < 407 * 32 && pChr->m_Pos.y > 212 * 32 && pChr->m_Pos.y < 215 * 32 && pChr->m_Pos.x > m_Core.m_Pos.x) //wenn ein im weg stehender tee auf der spawn plattform gefunden wurde
						{
							SetWeapon(0); //hol den hammer raus!
							if (pChr->m_Pos.x - m_Core.m_Pos.x < 30) //wenn der typ nahe bei dem bot ist
							{
								if (m_FreezeTick == 0) //nicht rum schrein
								{
									m_LatestInput.m_Fire++;
									m_Input.m_Fire++;
								}


								if (Server()->Tick() % 10 == 0)
								{
									GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9); //angry
								}
							}
						}
						else
						{
							SetWeapon(1); //pack den hammer weg
						}
					}
				}


				//CheckSlowDudesInTunnel

				if (m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.y > 214 * 32) //wenn bot im tunnel ist
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharTypeTunnel(m_Pos, true, this);
					if (pChr && pChr->IsAlive())
					{
						if (pChr->m_Core.m_Vel.x < 7.8f) //wenn der nächste spieler im tunnel ein slowdude is 
						{
							//HauDenBau
							SetWeapon(0); //hol den hammer raus!

							m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							if (m_FreezeTick == 0) //nicht rum schrein
							{
								m_LatestInput.m_Fire++;
								m_Input.m_Fire++;
							}

							if (Server()->Tick() % 10 == 0)  //do angry emotes
							{
								GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9);
							}


						}
					}
				}


				//CheckSpeedInTunnel
				if (m_Core.m_Pos.x > 425 * 32 && m_Core.m_Pos.y > 214 * 32 && m_Core.m_Vel.x < 9.4f) //wenn nich genung speed zum wb spot springen
				{
					m_Dummy_get_speed = true;
				}


				if (m_Dummy_get_speed) //wenn schwung holen == true (tunnel)
				{
					if (m_Core.m_Pos.x > 422 * 32) //zu weit rechts
					{
						//---> hol schwung für den jump
						m_Input.m_Direction = -1;
					}
					else //wenn weit genung links
					{
						//dann kann das normale movement von dort aus genung schwung auf bauen
						m_Dummy_get_speed = false;
					}
				}
				else
				{
					if (m_Core.m_Pos.x < 415 * 32) //bis zum tunnel laufen
					{
						m_Input.m_Direction = 1;

					}
					else if (m_Core.m_Pos.x < 440 * 32 && m_Core.m_Pos.y > 213 * 32) //im tunnel laufen
					{
						m_Input.m_Direction = 1;

					}


					//externe if abfrage weil laufen während sprinegn xD

					if (m_Core.m_Pos.x > 413 * 32 && m_Core.m_Pos.x < 415 * 32) // in den tunnel springen
					{
						m_Input.m_Jump = 1;
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
						//m_Input.m_Jump = 0;
					}
					else if (m_Core.m_Pos.x > 428 * 32 - 20 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen
					{
						m_Input.m_Jump = 1;
					}



					// externen springen aufhören für dj

					if (m_Core.m_Pos.x > 428 * 32 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen nicht mehr springen
					{
						m_Input.m_Jump = 0;
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
					}


					//nochmal extern weil springen während springen

					if (m_Core.m_Pos.x > 430 * 32 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen springen
					{
						m_Input.m_Jump = 1;
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
					}



					if (m_Core.m_Pos.x > 431 * 32 && m_Core.m_Pos.y > 213 * 32) //jump refillen für wayblock spot
					{
						m_Input.m_Jump = 0;
					}
				}

				// *****************************************************
				// Way Block spot (Main Spot) 29 29 29 29
				// *****************************************************
				// wayblockspot < 213


				//externer wayblockspot stuff

				//Checken ob der bot in seinem arial angegriffen wird obwohl kein nostand links ausgerufen wurde



				//wird nicht genutzt weil das preventive springen vom boden aus schluss endlich schlimmer ausgeht als der dj
				/*
				if (m_Core.m_Pos.y < 213 * 32 && m_Core.m_Pos.x > (427 * 32) - 20 && m_Core.m_Pos.x < (428 * 32) + 10) //wenn der bot sich an seinem ruler spot befindet
				{
				//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich mag diesen ort :)");

				if (!m_Dummy_wb_hooked && !m_Dummy_emergency && !m_Dummy_pushing && m_Core.m_Vel.x > 0.90f) //wenn der bot sich auf das freeze zubewegt obwohl er nicht selber läuft
				{
				// --> er wird wahrscheinlich gehookt oder anderweitig extern angegriffen
				// --> schutzmaßnahmen treffen

				GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "AAAh ich werde angegriffen");
				m_Input.m_Jump = 1;
				}
				m_Dummy_pushing = false;
				m_Dummy_emergency = false;
				m_Dummy_wb_hooked = false;
				}
				*/


				//Bools zurueck setzten
				m_Dummy_pushing = false;
				m_Dummy_emergency = false;
				m_Dummy_wb_hooked = false;
				m_Dummy_happy = false;

				//normaler internen wb spot stuff

				if (m_Core.m_Pos.y < 213 * 32)
				{
					//self kill im freeze
					//New Testy selfkill kill if isFreezed and vel 0
					if (!isFreezed || m_Core.m_Vel.x < -0.5f || m_Core.m_Vel.x > 0.5f || m_Core.m_Vel.y != 0.000000f)
					{
						//mach nichts lol brauche nur das else is einfacher
					}
					else
					{
						if (Server()->Tick() % 150 == 0)
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}

					//Old self kill kill if freeze
					//if (m_Core.m_Pos.y < 201 * 32) // decke
					//{
					//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "suicide reason: roof rulerspot");
					//}
					//else if (m_Core.m_Pos.x < 417 * 32 && m_Core.m_Pos.x > 414 * 32 + 17 && isFreezed) //linker streifen xD
					//{
					//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "suicide reason: left wall rulerspot");
					//}


					//because shitty structure (more infos at TODO(1)) check here for enemys attacking from outside of the ruler area

					//Checken Ob ein potentieller Gegner auf der edge unter dem WBbereich ist
					//Falls diesem so sei --> mach den da weg
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "debug");
					if (1 == 1)
					{
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "debug");
						CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerWBBottom(m_Pos, true, this);
						if (pChr && pChr->IsAlive() && !pChr->isFreezed) //wenn jemand da so im bereich lebt und unfreeze ist
						{

							m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;



							m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							//erkenne gefahr
							// --> treffe gegen maßnahmen

							//lauf rum rand (bereit machen zum hooken)
							if (m_Core.m_Pos.x < 428 * 32 + 6) //wenn zu weit links um in dem winkel zu hooken 
							{
								m_Input.m_Direction = 1;
							}
							else if (m_Core.m_Pos.x > 428 * 32 + 28)
							{
								m_Input.m_Direction = -1;
							}


							//hooke 
							CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
							if (pChr && pChr->IsAlive())
							{
								//Wenn der nächste spieler unter der wb area ist hook
								//damit er wenn er einen falschen spieler gehookt hat oder sonst wie den nicht hochzieht
								if (pChr->m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 427 * 32 + 3)
								{
									m_Input.m_Hook = 1;
								}
							}
						}
					}




					//TODO(1): keep this structur in mind this makes not much sence
					// the bool m_Dummy_happy is just true if a enemy is in the ruler area because all code below depends on a enemy in ruler area
					// maybe rework this shit



					//                                                      
					//                                               --->   Ruler   <---    testy own class just search in ruler area

					CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this);  //position anderer spieler mit pikus aimbot abfragen
					if (pChr && pChr->IsAlive())
					{
						//                                         old: 417 * 32
						//                                                      Old: tee<bot      New: tee<pos.x                                     
						if ((pChr->m_Pos.y < 213 * 32 && pChr->m_Pos.x > 417 * 32 - 5/* && pChr->m_Pos.x < m_Core.m_Pos.x*/ && pChr->m_Pos.x < 428 * 32 && m_Core.m_Pos.x < 429 * 32 && m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.y < 213 * 32) ||  //wenn ein tee weiter links ist als der bot && der bot links vom mittelfreeze im rulerspot steht
							(pChr->m_Pos.y < 213 * 32 && pChr->m_Pos.x > 417 * 32 - 5/* && pChr->m_Pos.x < m_Core.m_Pos.x*/ && pChr->m_Pos.x < 444 * 32 && m_Core.m_Pos.x < 429 * 32 && m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.y < 213 * 32 && pChr->m_FreezeTime == 0))       //oder der tee auch rechts vom bot ist aber unfreeze
																																																																				//wenn dies ist -> notstand links ausrufen und versuchen gegner zu blocken
						{
							//m_Core.m_ActiveWeapon = WEAPON_HAMMER;
							SetWeapon(0);


							//testy sollte eig auch am anfang des modes passen
							//m_Input.m_Direction = 0;

							//if (m_Core.m_HookState == HOOK_FLYING)
							//	m_Input.m_Hook = 1;
							//else if (m_Core.m_HookState == HOOK_GRABBED)
							//	m_Input.m_Hook = 1;
							//else
							//	m_Input.m_Hook = 0;

							char aBuf[256];
							str_format(aBuf, sizeof(aBuf), "hookstate: %x", m_Input.m_Hook);
							//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

							m_Dummy_emergency = true;




							if (!m_Dummy_left_freeze_full)
							{
								//                                                                                                        x > 5 = 3       <-- ignorieren xD


								//                                                                                                          hier wird das schieben an das andere schieben 
								//                                                                                                    übergeben weil er hier von weiter weg anfängt zu schieben 
								//                                                                                                und das kürzere schieben macht dann den ganzen stuff das der bot nicht selber rein läuft  
								//                                                                                                ja ich weiss das ist ziemlich umständlich xD
								//                                                                                                      aber das hat schon sinn das hier wird aufgerufen wenn der weit weg is und freezed und
								//                                                                                                  übergibt dann an die abfrage die auch aufgerufen wird wenn jemand unfreeze is jedoch nir auf kurze distanz

								//                                                                                                          tja aber das mit dem übergeben klappt ja nich wirklich


								//                                                                                                           Deswegen hab ich den code ganz gelöscht und nur ein teil als || in die "freeze protection & schieberrei" geklatscht
								//                                                                                                         ----> hier is ein berg an kommentaren zu nicht existentem code lol    gut das nur ich hier rum stüber hueueueu
								//start sequenz
								// Blocke spieler in die linke freeze wand

								if (!m_Dummy_jumped)
								{
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "boing!");
									m_Input.m_Jump = 1;
									m_Dummy_jumped = true;
								}
								else
								{
									m_Input.m_Jump = 0;
								}



								if (!m_Dummy_hooked)
								{
									if (Server()->Tick() % 30 == 0)
										m_Dummy_hook_delay = true;

									//testy removed hook here i dont know why but all works pretty good still xD
									if (m_Dummy_hook_delay)
										//m_Input.m_Hook = 1;

										if (Server()->Tick() % 200 == 0)
										{
											m_Dummy_hooked = true;
											m_Input.m_Hook = 0;
										}

								}

								if (!m_Dummy_moved_left)
								{
									if (m_Core.m_Pos.x > 419 * 32 + 20)
										m_Input.m_Direction = -1;
									else
										m_Input.m_Direction = 1;

									if (Server()->Tick() % 200 == 0)
									{
										m_Dummy_moved_left = true;
										m_Input.m_Direction = 0;
									}

								}
							}







							//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "NOTSTAND");

							if (Server()->Tick() % 30 == 0)  //angry emotes machen
							{
								GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9);
							}



							CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this); //aimbot + hammerspam
							if (pChr && pChr->IsAlive())
							{

								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;


								//schiess delay
								if (Server()->Tick() >= m_EmoteTickNext)
								{
									m_pPlayer->m_LastEmote = Server()->Tick();


									//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);

									if (m_FreezeTick == 0) //nicht rum schrein
									{
										m_LatestInput.m_Fire++;
										m_Input.m_Fire++;
									}

									m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() / 4;
								}
							}







							//Blocke gefreezte gegner für immer 


							//TODO:
							//das is ein linke seite block wenn hier voll is sollte man anders vorgehen
							//                           früher war es y > 210   aber change weil er während er ihn hochzieht dann nicht mehr das hooken aufhört
							if (pChr->m_FreezeTime > 0 && pChr->m_Pos.y > 204 * 32 && pChr->m_Pos.x > 422 * 32) //wenn ein gegner weit genung rechts freeze am boden liegt
							{
								// soll der bot sich einer position links des spielers nähern
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hab n opfer gefunden");

								if (m_Core.m_Pos.x + (5 * 32 + 40) < pChr->m_Pos.x) // er versucht 5 tiles links des gefreezten gegner zu kommen
								{
									m_Input.m_Direction = -1;

									if (m_Core.m_Pos.x > pChr->m_Pos.x && m_Core.m_Pos.x < pChr->m_Pos.x + (4 * 32)) // wenn er 4 tiles rechts des gefreezten gegners is
									{
										m_Input.m_Jump = 1;
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "boing!");
									}
								}
								else //wenn der bot links des gefreezten spielers is
								{
									m_Input.m_Jump = 1;
									//echo jump
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "boing!");

									m_Input.m_Direction = -1;

									if (m_Core.m_Pos.x < pChr->m_Pos.x) //solange der bot weiter links is
									{
										m_Input.m_Hook = 1;
									}
									else
									{
										m_Input.m_Hook = 0;
									}
								}
							}






							//freeze protection & schieberrei
							//                                                                                                                                                                                                      old (417 * 32 - 60)
							if ((pChr->m_Pos.x + 10 < m_Core.m_Pos.x && pChr->m_Pos.y > 211 * 32 && pChr->m_Pos.x < 418 * 32) || (pChr->m_FreezeTime > 0 && pChr->m_Pos.y > 210 * 32 && pChr->m_Pos.x < m_Core.m_Pos.x && pChr->m_Pos.x > 417 * 32 - 60)) // wenn der spieler neben der linken wand linken freeze wand liegt schiebt ihn der bot rein
							{                                                                                            // oder wenn der spieler weiter weg liegt aber freeze is


								if (!m_Dummy_left_freeze_full) //wenn da niemand is schieb den rein
								{
									// HIER TESTY TESTY CHANGES  211 * 32 + 40 stand hier
									if (pChr->m_Pos.y > 211 * 32 + 40) // wenn der gegner wirklich ganz tief genung is
									{ //                          417 * 32 - 40
										if (m_Core.m_Pos.x > 418 * 32) // aber nicht selber ins freeze laufen
										{
											m_Input.m_Direction = -1;




											//Check ob der gegener freeze is

											if (pChr->m_FreezeTime > 0)
											{
												m_LatestInput.m_Fire = 0; //nicht schiessen ofc xD (doch is schon besser xD)
												m_Input.m_Fire = 0;

												//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
											}

											//letzten stupser geben (sonst gibs bugs kb zu fixen)
											if (pChr->isFreezed) //wenn er schon im freeze is
											{
												m_LatestInput.m_Fire = 1; //hau ihn an die wand
												m_Input.m_Fire = 1;
											}

										}
										else
										{
											m_Input.m_Direction = 1;
											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 1");
											if (pChr->m_FreezeTime > 0)
											{
												m_LatestInput.m_Fire = 0; //nicht schiessen ofc xD (doch is schon besser xD)
												m_Input.m_Fire = 0;

												//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
											}
											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "ich halte das auf.");
											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich will da nich rein laufen");
										}



									}
									else //wenn der gegner nicht tief genung ist
									{

										m_Input.m_Direction = 1;

										if (pChr->m_FreezeTime > 0)
										{
											m_LatestInput.m_Fire = 0; //nicht schiessen ofc xD (doch is schon besser xD)
											m_Input.m_Fire = 0;

											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
										}
									}



								}
								else //wenn da schon jemand liegt 
								{
									// sag das mal xD
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "da liegt schon einer");
								}
							}
							else if (m_Core.m_Pos.x < 419 * 32 + 10) //sonst mehr abstand halten
							{
								m_Input.m_Direction = 1;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 2");
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich will da nich rein laufen");
							}
							// else // wenn nichts los is erstmal stehen bleiben 






							//freeze protection decke mit double jump wenn hammer

							if (m_Core.m_Vel.y < 20.0f && m_Core.m_Pos.y < 207 * 32)  // wenn der tee nach oben gehammert wird
							{
								if (m_Core.m_Pos.y > 206 * 32) //ab 206 würd er so oder so ins freeze springen
									m_Input.m_Jump = 1;

								if (m_Core.m_Pos.y < pChr->m_Pos.y) //wenn der bot über dem spieler is soll er hooken
									m_Input.m_Hook = 1;
								else
									m_Input.m_Hook = 0;
							}


							//wenn der tee hcoh geschleudert wird
							//                 old 4 (macht aber im postiven bereich kein sinn aber hat geklappt)
							//                 HALLO HIER IST DEIN ICH AUS DER ZUKUNFT: du dummes kind wenn er in der luft hammert dann fliegt er doch nicht nach oben und gerade da musst du es ja perfekt haben ... low
							//if (m_Core.m_Vel.y < 4.0f && m_Core.m_Pos.y < pChr->m_Pos.y) //wenn er schneller als 4 nach oben fliegt und höher als der Gegener ist
							// lass das mit speed weg und mach lieber was mit höhe
							if (m_Core.m_Pos.y < 207 * 32 && m_Core.m_Pos.y < pChr->m_Pos.y)
							{
								//in hammer position bewegen
								if (m_Core.m_Pos.x > 418 * 32 + 20) //nicht ins freeze laufen
								{
									if (m_Core.m_Pos.x > pChr->m_Pos.x - 45) //zu weit rechts von hammer position
									{
										m_Input.m_Direction = -1; //gehe links
																  //GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich will da nich rein laufen");
									}
									else if (m_Core.m_Pos.x < pChr->m_Pos.x - 39) // zu weit links von hammer position
									{
										m_Input.m_Direction = 1;
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 3");
									}
									else  //im hammer bereich
									{
										m_Input.m_Direction = 0; //bleib da
									}
								}
							}


							//Check ob der gegener freeze is

							if (pChr->m_FreezeTime > 0 && pChr->m_Pos.y > 208 * 32 && !pChr->isFreezed) //wenn der Gegner tief und freeze is macht es wenig sinn den frei zu hammern
							{
								m_LatestInput.m_Fire = 0; //nicht schiessen 
								m_Input.m_Fire = 0;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
							}



							//Hau den weg (wie dummymode 21)
							if (pChr->m_Pos.x > 418 * 32 && pChr->m_Pos.y > 209 * 32)  //das ganze findet nur im bereich statt wo sonst eh nichts passiert
							{
								//wenn der bot den gegner nicht boosten würde hammer den auch nich weg
								m_LatestInput.m_Fire = 0;
								m_Input.m_Fire = 0;

								if (pChr->m_Core.m_Vel.y < -0.5f && m_Core.m_Pos.y + 15 > pChr->m_Pos.y) //wenn der dude speed nach oben hat
								{
									m_Input.m_Jump = 1;
									if (m_FreezeTime == 0)
									{
										m_LatestInput.m_Fire++;
										m_Input.m_Fire++;
									}
								}
							}


							//TODO: FIX:
							//der bot unfreezed den gegner ab einer gewissen höhe wenn er rein gehammert wird schau das da was passiert





							//wenn ein tee freeze links neben dem bot liegt werf den einfach wieder ins freeze becken
							//das is bisher ja noch die einzige sicherheits lücke beim wayblocken
							//wenn man ein tee über den bot hammert

							if (pChr->m_Pos.x > 421 * 32 && pChr->m_FreezeTick > 0 && pChr->m_Pos.x < m_Core.m_Pos.x)
							{
								m_Input.m_Jump = 1;
								m_Input.m_Hook = 1;
							}


							//m_pPlayer->m_TeeInfos.m_ColorBody = (0 * 255 / 360);
							//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Enemy in ruler spot found!");
						}
						else //sonst normal wayblocken und 427 aufsuchen
						{
							//m_Core.m_ActiveWeapon = WEAPON_GUN;
							SetWeapon(1);
							m_Input.m_Jump = 0;

							if (m_Core.m_HookState == HOOK_FLYING)
								m_Input.m_Hook = 1;
							else if (m_Core.m_HookState == HOOK_GRABBED)
								m_Input.m_Hook = 1;
							else
								m_Input.m_Hook = 0;

							//m_pPlayer->m_TeeInfos.m_ColorBody = (120 * 255 / 360);
							//positions check and correction 427


							m_Dummy_jumped = false;
							m_Dummy_hooked = false;
							m_Dummy_moved_left = false;



							if (m_Core.m_Pos.x > 428 * 32 + 15 && m_Dummy_ruled) //wenn viel zu weit ausserhalb der ruler area wo der bot nur hingehookt werden kann
							{
								m_Input.m_Jump = 1;
								m_Input.m_Hook = 1;
							}

							//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Prüfe ob zu weit rechts");
							if (m_Core.m_Pos.x < (427 * 32) - 20) // zu weit links -> geh rechts
							{
								m_Input.m_Direction = 1;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 4");
							}
							else if (m_Core.m_Pos.x >(428 * 32) + 10) // zu weit rechts -> geh links
							{
								m_Input.m_Direction = -1;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich bin zuweit rechts...");
							}
							else // im toleranz bereich -> stehen bleiben
							{
								m_Dummy_happy = true;
								m_Dummy_ruled = true;
								m_Input.m_Direction = 0;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "toleranz bereich");
								//m_Input.m_LatestTargetX = 0;
								//m_Input.m_LatestTargetY = 0;


								//stuff im toleranz bereich doon





								// normal wayblock
								CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);  //position anderer spieler mit pikus aimbot abfragen
								if (pChr && pChr->IsAlive())
								{
									//Check ob jemand special angeflogen kommt dann mode18 = 3 starten
									//Check ob special_defend aktiviert werden sollte
									if (pChr->m_Pos.x < 431 * 32 && pChr->m_Core.m_Vel.y < -12.5f && pChr->m_Core.m_Vel.x < -7.4f)
									{
										m_Dummy_special_defend = true;
									}

									//debuggn special_defend
									//char aBuf[256];
									//str_format(aBuf, sizeof(aBuf), "speed pChr:  x: %f y: %f", pChr->m_Core.m_Vel.x, pChr->m_Core.m_Vel.y);
									//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

									//m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
									//m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;



									//m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
									//m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

									//m_Input.m_TargetX = 1;//pChr->m_Pos.x - m_Pos.x; //1
									//m_Input.m_TargetY = 0;//pChr->m_Pos.y - m_Pos.y; //0

									//m_LatestInput.m_TargetX = 1;//pChr->m_Pos.x - m_Pos.x;
									//m_LatestInput.m_TargetY = 0;//pChr->m_Pos.y - m_Pos.y;

									if (pChr->m_Pos.y < 213 * 32 + 10 && pChr->m_Pos.x < 430 * 32 && pChr->m_Pos.y > 210 * 32 && pChr->m_Pos.x > 416 * 32 + 32) // wenn ein spieler auf der linken seite in der ruler area is 
									{
										//wenn der typ über dem freze irgendwo rum fliegt

										if (pChr->m_Pos.y < 212 * 32 - 10)  //wenn er jedoch zu hoch zum schieben ist
										{
											//mach dich bereit zu schieben und geh nach links (aufziehen)
											m_Input.m_Direction = -1;
										}
										else  //wenn er tief genung zum schieben ist
										{
											if (m_Core.m_Pos.x < 428 * 32 + 10) //bei (429 * 32) gibts voll jiggle xD
											{
												m_Input.m_Direction = 1; //schieb ihn runter
												m_Dummy_pushing = true;
											}
											else
											{
												m_Input.m_Direction = 0; // aber nicht zu weit
																		 //GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "ja ich bin das!");
											}
										}
									}
									else // wenn spieler irgendwo anders is
									{




										//CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this);  //wenn jemand oben is
										//if (pChr && pChr->IsAlive())
										//{
										//		m_Input.m_Hook = 1;
										//		m_Dummy_wb_hooked = true;
										//		GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);

										//}





										if (!m_Dummy_emergency) //wayblock stuff hook ja nein pipapo nicht im notfall
										{
											CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerWB(m_Pos, true, this);  //wenn jemand oben is (nur im wb bereich)
											if (pChr && pChr->IsAlive())
											{                                                                  // und er nicht zu tief ist (das is um unnötiges festgehalte zu verhindern) (bringt eh nichts weil das hier dann nicht mehr aufgerufen wird weil der dann nicht mehr in ClosesestCharTypeRulerWB ist -.-)
												m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
												m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;



												m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
												m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
												//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 8);
												//                             noch eine vel abfrage weil der bot sonst daneben hookt
												if (pChr->m_Pos.y > 211 * 32 && pChr->m_Core.m_Vel.y > -1.0f && pChr->m_Core.m_Vel.y < 2.0f && pChr->m_Pos.x > 435 * 32/*&& pChr->m_Pos.y < 213 * 32*/) //wenn er nich zu schnell nach oben fliegt und zu weit oben ist
												{



													//m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x; //1
													//m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y; //0






													m_Input.m_Hook = 0;
													m_Input.m_Hook = 1;
													m_Dummy_wb_hooked = true;
													if (Server()->Tick() % 30 == 0) //nicht zu hart spammen wenn iwas abgeht
													{
														GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
														m_Dummy_bored_counter++; //zähl mal mit wie lange der bot hier rum gurkt und wieviele spieler er so wayblockt
														char aBuf[256];
														str_format(aBuf, sizeof(aBuf), "dummy_bored_count: %d", m_Dummy_bored_counter);
														GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
													}
												}
											}
											else
											{
												if (Server()->Tick() % 10 == 0) //nicht zu hart spammen wenn iwas abgeht
													GameServer()->SendEmoticon(m_pPlayer->GetCID(), 15);
												m_Input.m_Hook = 0;
												//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3);
											}
										}
										//unnötiges festgehalte unten verhindern
										if (!m_Dummy_emergency) //auch hier wieder nur wenn kein notfall is
										{
											CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
											if (pChr && pChr->IsAlive())
											{
												if (pChr->m_Pos.y > 213 * 32 - 5 && m_Core.m_HookState == HOOK_GRABBED && pChr->m_Pos.y < 213 * 32 + 5)
												{
													//Wenn folgendes:
													// kein notstand
													// der bot wenn am haken hat
													// der nächste spieler aber unter der ruler area ist (genauer gesagt gerade im freeze eingetaucht)
													// --> vermute ich mal das er genau diesen spieler hookt
													// --> den los lassen da dieser sowieso keien gefahr mehr ist damit ein neuer gegner schneller geblockt werden kann
													if (Server()->Tick() % 10 == 0) //nicht zu hart spammen wenn iwas abgeht
														GameServer()->SendEmoticon(m_pPlayer->GetCID(), 5);


													m_Input.m_Hook = 0;
												}
											}
										}



										//if (Server()->Tick() % 50 == 0) //hook ihn xD
										//{
										//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "bin im bereich");
										//	if (m_Input.m_Hook == 0)
										//	{
										//		m_Input.m_Hook = 1;
										//		m_Dummy_wb_hooked = true;
										//		//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
										//		//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "huke");
										//		//schiess delay
										//		if (Server()->Tick() >= m_EmoteTickNext)
										//		{
										//			m_pPlayer->m_LastEmote = Server()->Tick();


										//			//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);

										//			m_LatestInput.m_Fire++;
										//			m_Input.m_Fire++;

										//			m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() / 4;
										//		}
										//		else
										//		{
										//			m_LatestInput.m_Fire = 0;
										//			m_Input.m_Fire = 0;
										//		}
										//	}
										//	else
										//	{
										//		m_Input.m_Hook = 0;
										//		GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
										//		//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "huke nich");

										//	}
										//}

									}
								}
							}
						}
					}
					else // wenn kein lebender spieler im ruler spot ist
					{

						//Suche trozdem 427 auf

						if (m_Core.m_Pos.x < (427 * 32) - 20) // zu weit links -> geh rechts
						{
							m_Input.m_Direction = 1;
							//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: special");
						}
						else if (m_Core.m_Pos.x >(427 * 32) + 40) // zu weit rechts -> geh links
						{
							m_Input.m_Direction = -1;
							//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich bin zuweit rechts...");
						}
						//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
					}






					// über das freeze springen wenn rechts der bevorzugenten camp stelle

					if (m_Core.m_Pos.x > 434 * 32)
					{
						m_Input.m_Jump = 1;
					}
					else if (m_Core.m_Pos.x > (434 * 32) - 20 && m_Core.m_Pos.x < (434 * 32) + 20) // bei flug über freeze jump wd holen
					{
						m_Input.m_Jump = 0;
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "refilling jump");
					}







				}


				//testy vel

				/*

				if (m_Core.m_Vel.x == 16 * 32)
				{
				GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "speed: 16");
				}

				*/


				//ganz am ende nochmal checken ob man nicht gerade einen spieler wieder aus dem freeze zieht
				//TODO: wenn hier irgendwann eine protection kommt das der spieler nicht ganz an der linken wand sein sollte
				// muss das hier geändert wwwerden


				if (1 < 10)
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerLeftFreeze(m_Pos, true, this);
					if (pChr && pChr->IsAlive())
					{
						if (pChr->isFreezed && pChr->m_Pos.x < 417 * 32 - 30) //wenn ein gegner in der linken wand is
						{
							m_Input.m_Hook = 0; //hook den da mal wd nich raus
						}
					}
				}

				//das selbe auch rechts

				if (1 < 10)
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this);
					if (pChr && pChr->IsAlive())
					{    // heieiie aber natürlich auch hoch genung is das der bot noch wayblocken kann
						if (pChr->m_FreezeTime > 0 && pChr->m_Pos.x > 428 * 32 + 40 && pChr->m_Pos.y < 211 * 32) //wenn ein gegner in der linken wand is
						{
							m_Input.m_Hook = 0; //hook den da mal wd nich raus
						}
					}
				}


				//TESTY to prevent bugs
				//wenn kein notfall is und der bot glücklich mit seiner position ist
				//dann sollte er auch nicht springen und somit irgendwie spielern helfen die er gerade hookt

				if (!m_Dummy_emergency && m_Dummy_happy)
				{
					m_Input.m_Jump = 0;
				}



			}
			else //Change to mode main and reset all
			{
				GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "EROOR!!!!");
				//RestOnChange (zuruecksetzten)
				m_Input.m_Hook = 0;
				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;



				m_Dummy_mode18 = 0;
			}

		}
		else if (m_pPlayer->m_DummyMode == 23) //Race mode ChillBlock5 with humans
		{
			//rest dummy (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;
			m_Dummy_mode23 = 0;
			//m_pPlayer->m_TeeInfos.m_ColorBody = (0 * 255 / 360); //remove this if u ever want to debug agian xd

			/*
			Dummy23modes:
			0				Classic Main race mode.
			1				Tricky mode with tricky hammerfly and sensless harder hammerhit xD. (used for "Drag*" to fuck him in the race lol)
			2				ChillerDragon's mode just speedhammerfly.

			*/

			if (1 == 1)
			{
				CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
				if (pChr && pChr->IsAlive())
				{
					//if (Server()->ClientName(pChr) == "ChillerDragon")
					if (
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Starkiller") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "rqza") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "timakro") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Nudelsaft c:") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Destom") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Ante") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Ama") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Forris") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Aoe") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Spyker") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Waschlappen") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), ".:Mivv") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "nealson T'nP") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "ChillerDragon") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "ChillerDragon.*") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Gwendal") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Blue") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Amol") ||
						//!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "fokkonaut") ||
						!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "pro")
						)
					{
						m_Dummy_mode23 = 2;
					}
					if (!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "Drag*"))
					{
						m_Dummy_mode23 = 1;
					}
				}
			}


			if (m_Core.m_Pos.x > 241 * 32 && m_Core.m_Pos.x < 418 * 32 && m_Core.m_Pos.y > 121 * 32 && m_Core.m_Pos.y < 192 * 32) //new spawn ChillBlock5 (tourna edition (the on with the gores stuff))
			{
				//dieser code wird also nur ausgeführt wenn der bot gerade im neuen bereich ist
				if (m_Core.m_Pos.x > 319 * 32 && m_Core.m_Pos.y < 161 * 32) //top right spawn
				{
					//look up left
					if (m_Core.m_Pos.x < 372 * 32 && m_Core.m_Vel.y > 3.1f)
					{
						m_Input.m_TargetX = -30;
						m_Input.m_TargetY = -80;
					}
					else
					{
						m_Input.m_TargetX = -100;
						m_Input.m_TargetY = -80;
					}

					if (m_Core.m_Pos.x > 331 * 32 && isFreezed)
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}

					if (m_Core.m_Pos.x < 327 * 32) //dont klatsch in ze wand
					{
						m_Input.m_Direction = 1; //nach rechts laufen
					}
					else
					{
						m_Input.m_Direction = -1;
					}

					if (IsGrounded() && m_Core.m_Pos.x < 408 * 32) //initial jump from spawnplatform
					{
						m_Input.m_Jump = 1;
					}

					if (m_Core.m_Pos.x > 330 * 32) //only hook in tunnel and let fall at the end
					{
						if (m_Core.m_Pos.y > 147 * 32 || (m_Core.m_Pos.y > 143 * 32 && m_Core.m_Vel.y > 3.3f)) //gores pro hook up
						{
							m_Input.m_Hook = 1;
						}
						else if (m_Core.m_Pos.y < 143 * 32 && m_Core.m_Pos.x < 372 * 32) //hook down (if too high and in tunnel)
						{
							m_Input.m_TargetX = -42;
							m_Input.m_TargetY = 100;
							m_Input.m_Hook = 1;
						}
					}
				}
				else if (m_Core.m_Pos.x < 317 * 32) //top left spawn
				{
					if (m_Core.m_Pos.y < 158 * 32) //spawn area find down
					{
						//selfkill
						if (isFreezed)
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}

						if (m_Core.m_Pos.x < 276 * 32 + 20) //is die mitte von beiden linken spawns also da wo es runter geht
						{
							m_Input.m_TargetX = 57;
							m_Input.m_TargetY = -100;
							m_Input.m_Direction = 1;
						}
						else
						{
							m_Input.m_TargetX = -57;
							m_Input.m_TargetY = -100;
							m_Input.m_Direction = -1;
						}

						if (m_Core.m_Pos.y > 147 * 32)
						{
							//dbg_msg("fok","will hooken");
							m_Input.m_Hook = 1;
							if (m_Core.m_Pos.x > 272 * 32 && m_Core.m_Pos.x < 279 * 32) //let fall in the hole
							{
								//dbg_msg("fok", "lass ma des");
								m_Input.m_Hook = 0;
								m_Input.m_TargetX = 2;
								m_Input.m_TargetY = 100;
							}
						}
					}
					else if (m_Core.m_Pos.y > 162 * 32) //managed it to go down --> go left
					{
						//selfkill
						if (isFreezed)
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}

						if (m_Core.m_Pos.x < 283 * 32)
						{
							m_Input.m_TargetX = 200;
							m_Input.m_TargetY = -136;
							if (m_Core.m_Pos.y > 164 * 32 + 10)
							{
								m_Input.m_Hook = 1;
							}
						}
						else
						{
							m_Input.m_TargetX = 80;
							m_Input.m_TargetY = -100;
							if (m_Core.m_Pos.y > 171 * 32 - 10)
							{
								m_Input.m_Hook = 1;
							}
						}

						m_Input.m_Direction = 1;
					}
					else //freeze unfreeze bridge only 2 tiles do nothing here
					{

					}
				}
				else //lower end area of new spawn --> entering old spawn by walling and walking right
				{
					m_Input.m_Direction = 1;
					m_Input.m_TargetX = 200;
					m_Input.m_TargetY = -84;

					//Selfkills
					if (isFreezed && IsGrounded()) //should never lie in freeze at the ground
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}


					if (m_Core.m_Pos.y < 166 * 32 - 20)
					{
						m_Input.m_Hook = 1;
					}

					if (m_Core.m_Pos.x > 332 * 32 && m_Core.m_Pos.x < 337 * 32 && m_Core.m_Pos.y > 182 * 32) //wont hit the wall --> jump
					{
						m_Input.m_Jump = 1;
					}

					if (m_Core.m_Pos.x > 336 * 32 + 20 && m_Core.m_Pos.y > 180 * 32) //stop moving if walled
					{
						m_Input.m_Direction = 0;
					}
				}
			}
			else if (false && m_Core.m_Pos.y < 193 * 32 /*&& g_Config.m_SvChillBlock5Version == 1*/) //new spawn
			{
				m_Input.m_TargetX = 200;
				m_Input.m_TargetY = -80;


				//not falling in freeze is bad
				if (m_Core.m_Vel.y < 0.01f && m_FreezeTime > 0)
				{
					if (Server()->Tick() % 40 == 0)
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}
				}
				if (m_Core.m_Pos.y > 116 * 32 && m_Core.m_Pos.x > 394 * 32)
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}

				if (m_Core.m_Pos.x > 364 * 32 && m_Core.m_Pos.y < 126 * 32 && m_Core.m_Pos.y > 122 * 32 + 10)
				{
					if (m_Core.m_Vel.y > -1.0f)
					{
						m_Input.m_Hook = 1;
					}
				}

				if (m_Core.m_Pos.y < 121 * 32 && m_Core.m_Pos.x > 369 * 32)
				{
					m_Input.m_Direction = -1;
				}
				else
				{
					m_Input.m_Direction = 1;
				}
				if (m_Core.m_Pos.y < 109 * 32 && m_Core.m_Pos.x > 377 * 32 && m_Core.m_Pos.x < 386 * 32)
				{
					m_Input.m_Direction = 1;
				}


				if (m_Core.m_Pos.y > 128 * 32)
				{
					m_Input.m_Jump = 1;
				}


				//speeddown at end to avoid selfkill cuz to slow falling in freeze
				if (m_Core.m_Pos.x > 384 * 32 && m_Core.m_Pos.y > 121 * 32)
				{
					m_Input.m_TargetX = 200;
					m_Input.m_TargetY = 300;
					m_Input.m_Hook = 1;
				}
			}
			else //under 193 (above 193 is new spawn)
			{
				//Selfkill

				//Checken ob der bot far im race ist
				if (m_Dummy_collected_weapons && m_Core.m_Pos.x > 470 * 32 && m_Core.m_Pos.y < 200 * 32)
				{
					//TODO:
					//schau wie weit der bot is wenn er weiter is als der ClosestCharTypeFarInRace bereich is schau das du rechtzeitig n anderen triggerst
					//wie zumbeispiel ClosestCharTypeFinish und das wird getriggert wenn der bot rechts des 2p parts is oder so

					CCharacter *pChr = GameServer()->m_World.ClosestCharTypeFarInRace(m_Pos, true, this);
					if (pChr && pChr->IsAlive())
					{
					}
					else
					{
						if (!isFreezed || m_Core.m_Vel.x < -0.5f || m_Core.m_Vel.x > 0.5f || m_Core.m_Vel.y != 0.000000f)
						{
							//mach nichts lol brauche nur das else is einfacher
						}
						else
						{
							if (Server()->Tick() % 370 == 0)
								Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}
					}
				}
				else //sonst normal relativ schnell killen
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
					if (pChr && pChr->IsAlive())
					{
						if (!isFreezed || m_Core.m_Vel.x < -0.5f || m_Core.m_Vel.x > 0.5f || m_Core.m_Vel.y != 0.000000f)
						{
							//mach nichts lol brauche nur das else is einfacher
						}
						else
						{
							if (Server()->Tick() % 270 == 0)
								Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}
					}
					else
					{
						if (isFreezed && m_Core.m_Vel.y == 0.000000f && m_Core.m_Vel.x < 0.1f && m_Core.m_Vel.x > -0.1f)
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}
					}
				}






				//insta self kills
				if (m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.x > 325 * 32 && m_Core.m_Pos.y > 215 * 32)  //Links am spawn runter
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Links am spawn runter");
				}
				//else if ((m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x > 392 * 32 && m_Core.m_Pos.y > 190) || (m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.y > 190)) //freeze decke am spawn
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke am spawn");
				//}
				//else if (m_Core.m_Pos.y > 218 * 32 + 31 /* für tee balance*/ && m_Core.m_Pos.x < 415 * 32) //freeze boden am spawn
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze boden am spawn");
				//}
				else if (m_Core.m_Pos.y < 215 * 32 && m_Core.m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.x < 428 * 32) //freeze decke im tunnel
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke im tunnel");
				}
				else if (m_Core.m_Pos.y > 222 * 32) //freeze becken unter area
				{
					//Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze becken unter area");
				}


				if ((m_Core.m_Pos.y < 220 * 32 && m_Core.m_Pos.x < 415 * 32 && m_FreezeTime > 1) && (m_Core.m_Pos.x > 350 * 32)) //always suicide on freeze if not reached teh block area yet             (new) AND not coming from the new spawn and falling through the freeze
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze und links der block area");
				}

				//Movement bis zur ruler area:
				/*
				NEW! Movement modes for the basic move till hammerfly!
				m_Dummy_movement_mode23 is a int to check which movement style the bot shoudl used

				0					normal old basic mode
				1					new mode jump left side up into ruler area [ALPHA]


				i dunno how to set the modes for now its hardcodet set to 1 maybe add a random switcher or depending on how frustrated the bot is

				*/
				m_Dummy_movement_mode23 = 0;

				if (m_Core.m_Pos.x < 388 * 32 && m_Core.m_Pos.y > 213 * 32) //jump to old spawn
				{
					m_Input.m_Jump = 1;
					m_Input.m_Fire++;
					m_LatestInput.m_Fire++;
					m_Input.m_Hook = 1;
					m_Input.m_TargetX = -200;
					m_Input.m_TargetY = 0;
				}


				if (m_Dummy_movement_mode23 == 0)
				{
					if (m_Core.m_Pos.x < 415 * 32) //bis zum tunnel laufen
					{
						m_Input.m_Direction = 1;

					}
					else if (m_Core.m_Pos.x < 440 * 32 && m_Core.m_Pos.y > 213 * 32) //im tunnel laufen
					{
						m_Input.m_Direction = 1;
						if (m_Core.m_Vel.x < 5.5f)
						{
							m_Input.m_TargetY = -3;
							m_Input.m_TargetX = 200;
							m_LatestInput.m_TargetY = -3;
							m_LatestInput.m_TargetX = 200;

							if (Server()->Tick() % 30 == 0)
							{
								SetWeapon(0);
							}
							if (Server()->Tick() % 55 == 0)
							{
								if (m_FreezeTime == 0)
								{
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
								}
							}
							if (Server()->Tick() % 200 == 0)
							{
								m_Input.m_Jump = 1;
							}
						}
					}


					//externe if abfrage weil laufen während sprinegn xD

					if (m_Core.m_Pos.x > 413 * 32 && m_Core.m_Pos.x < 415 * 32) // in den tunnel springen
					{
						m_Input.m_Jump = 1;
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
						//m_Input.m_Jump = 0;
					}
					else if (m_Core.m_Pos.x > 428 * 32 - 20 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen
					{
						m_Input.m_Jump = 1;
					}



					// externen springen aufhören für dj

					if (m_Core.m_Pos.x > 428 * 32 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen nicht mehr springen
					{
						m_Input.m_Jump = 0;
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
					}


					//nochmal extern weil springen während springen

					if (m_Core.m_Pos.x > 430 * 32 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen springen
					{
						m_Input.m_Jump = 1;
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
					}



					if (m_Core.m_Pos.x > 431 * 32 && m_Core.m_Pos.y > 213 * 32) //jump refillen für wayblock spot
					{
						m_Input.m_Jump = 0;
					}
				}
				else if (m_Dummy_movement_mode23 == 1) //enter ruler area with a left jump 
				{
					if (m_Core.m_Pos.x < 415 * 32) //bis zum tunnel laufen
					{
						m_Input.m_Direction = 1;

					}
					else if (m_Core.m_Pos.x < 440 * 32 && m_Core.m_Pos.y > 213 * 32) //im tunnel laufen
					{
						if (m_Core.m_Pos.x > 426 * 32)
						{
							//if (m_Core.m_Vel.x < 11.4f) //slow down before jump
							//{
							m_Input.m_Direction = 1;
							//}
						}
						else
						{
							m_Input.m_Direction = 1;
						}
					}

					//springen
					if (m_Core.m_Pos.x > 413 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.y > 213 * 32) // in den tunnel springen
					{
						m_Input.m_Jump = 1;
					}
					else if (m_Core.m_Pos.x > 428 * 32 - 3 && m_Core.m_Pos.y > 217 * 32 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen
					{
						m_Input.m_Jump = 1;
					}


					//nochmal extern weil springen während springen


					if (m_Core.m_Pos.x > 429 * 32 - 18)
					{
						m_Input.m_Direction = -1;
					}

					if (m_Core.m_Pos.y < 217 * 32 && m_Core.m_Pos.x > 420 * 32 && m_Core.m_Pos.y > 213 * 32 + 20)
					{
						//m_Input.m_Direction = -1;
						if (m_Core.m_Pos.y < 216 * 32) // im tunnel springen springen
						{
							m_Input.m_Jump = 1;
						}
					}

					//originaly made for a yt video but is ok to keep it there
					//if (m_Core.m_Pos.y > 2)


				}

				//MoVement ab dem ruler spot weiter laufen
				//NEW! wenn er links vom freeze becken im ruler spot is also beim wb spot des 18er modes dann jump übers freeze
				if (m_Core.m_Pos.y < 213 * 32 && m_Core.m_Pos.x > 428 * 32 && m_Core.m_Pos.x < 429 * 32)
				{
					m_Input.m_Jump = 1;
				}


				if (m_Core.m_Pos.x > 417 * 32 && m_Core.m_Pos.y < 213 * 32 && m_Core.m_Pos.x < 450 * 32) //vom ruler nach rechts nachm unfreeze werden
				{
					m_Input.m_Direction = 1;
				}

				if (m_Core.m_Pos.x > 439 * 32 && m_Core.m_Pos.y < 213 * 32 && m_Core.m_Pos.x < 441 * 32) //über das freeze zum hf start springen
				{
					m_Input.m_Jump = 1;
				}

				if (m_Core.m_Pos.y > 200 * 32 && m_Core.m_Pos.x > 457 * 32)
				{
					m_Input.m_Direction = -1;
				}

				//unnötiger dj
				//if (m_Core.m_Pos.x > 441 * 32 + 10 && m_Core.m_Pos.y < 213 * 32 && m_Core.m_Pos.x < 442 * 32)
				//{
				//	m_Input.m_Jump = 1;
				//}



				//Jetzt kommt der schwierige rocketjump part o.O mal sehen wie gut das hinhaut xD
				//TODO:
				//aufpassen dass er das ganze nur macht wenn er nicht schon beim 2p part ist
				if (m_Dummy_collected_weapons)
				{
					if (m_Core.m_Pos.x < 466 * 32)
					{
						SetWeapon(3);
					}
					//prepare for rocktjump


					if (m_Core.m_Pos.x < 451 * 32 + 1 && m_Core.m_Pos.y > 209 * 32) //wenn zu weit links für rj
					{
						m_Input.m_Direction = 1;
					}
					else if (m_Core.m_Pos.x > 451 * 32 + 3 && m_Core.m_Pos.y > 209 * 32) //wenn zu weit links für rj
					{
						m_Input.m_Direction = -1;
					}
					else
					{
						if (m_Core.m_Vel.x < 0.01f && m_Core.m_Vel.x > -0.01f) //nahezu stillstand
						{
							//ROCKETJUMP
							if (m_Core.m_Pos.x > 450 * 32 && m_Core.m_Pos.y > 209 * 32)
							{
								//Wenn der bot weit genung is und ne waffe hat und tief genung is 
								// ---> bereit machen für rocketjump
								//damit der bot nicht ausm popo schiesst xD

								m_Input.m_TargetX = 0;
								m_Input.m_TargetY = 37;
								m_LatestInput.m_TargetX = 0;
								m_LatestInput.m_TargetY = 37;
							}

							if (m_Core.m_Pos.y > 210 * 32 + 30 && !isFreezed) //wenn der dummy auf dem boden steht und unfreeze is
							{
								if (m_Core.m_Vel.y == 0.000000f)
								{
									m_Input.m_Jump = 1;
								}
							}

							if (m_Core.m_Pos.y > 210 * 32 + 10 && m_Core.m_Vel.y < -0.9f && !isFreezed) //dann schiessen
							{
								//m_LatestInput.m_TargetX = 0;
								//m_LatestInput.m_TargetY = 10;
								m_LatestInput.m_Fire++;
								m_Input.m_Fire++;
							}


						}
					}
				}



				if (m_Core.m_Pos.x > 448 * 32 && m_Core.m_Pos.x < 458 * 32 && m_Core.m_Pos.y > 209 * 32) //wenn der bot auf der platform is
				{
					//nicht zu schnell laufen
					if (Server()->Tick() % 3 == 0)
					{
						m_Input.m_Direction = 0;
						//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
					}
				}

				//Rocketjump2 an der freeze wand
				//prepare aim!
				if (m_Core.m_Pos.y < 196 * 32)
				{
					m_Input.m_TargetX = -55;
					m_Input.m_TargetY = 32;
					m_LatestInput.m_TargetX = -55;
					m_LatestInput.m_TargetY = 32;
				}

				if (m_Core.m_Pos.x < 452 * 32 && m_Core.m_Pos.y > 188 * 32 && m_Core.m_Pos.y < 192 * 32 && m_Core.m_Vel.y < 0.1f && m_Dummy_collected_weapons)
				{
					m_Dummy_rjumped2 = true;
					m_LatestInput.m_Fire++;
					m_Input.m_Fire++;
				}

				//Fliegen nach rj2
				if (m_Dummy_rjumped2)
				{
					m_Input.m_Direction = 1;


					if (m_Core.m_Pos.x > 461 * 32 && m_Core.m_Pos.y > 192 * 32 + 20)
					{
						m_Input.m_Jump = 1;
					}

					if (m_Core.m_Pos.x > 478 * 32 || m_Core.m_Pos.y > 196 * 32)
					{
						m_Dummy_rjumped2 = false;
					}
				}


				//Check ob der dummy schon waffen hat
				//if (m_Core.m_Pos.y < 165 * 32 && m_Core.m_Pos.x > 451 * 32 - 10 && m_Core.m_Pos.x < 454 * 32 + 10)
				if (m_aWeapons[3].m_Got && m_aWeapons[2].m_Got)
				{
					m_Dummy_collected_weapons = true;
				}
				else //wenn er sie wd verliert zb durch shields
				{
					m_Dummy_collected_weapons = false;
				}

				if (1 == 0.5 + 0.5)
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
					if (pChr && pChr->IsAlive())
					{
						if (pChr->m_Pos.y < 165 * 32 && pChr->m_Pos.x > 451 * 32 - 10 && pChr->m_Pos.x < 454 * 32 + 10)
						{
							m_Dummy_mate_collected_weapons = true;
						}
					}
				}

				//Hammerfly
				if (m_Core.m_Pos.x > 447 * 32)
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
					if (pChr && pChr->IsAlive())
					{
						//unfreezemates on platform

						//Get closer to the mate
						if (pChr->m_Pos.y == m_Core.m_Pos.y && m_Core.m_Pos.x > 450 * 32 && m_Core.m_Pos.x < 457 * 32 && pChr->m_FreezeTime > 0)
						{
							if (pChr->m_Pos.x > m_Core.m_Pos.x + 70) //if friend is on the right of the bot
							{
								m_Input.m_Direction = 1;

							}
							else if (pChr->m_Pos.x < m_Core.m_Pos.x - 70) //if firend is on the left of the bot
							{
								m_Input.m_Direction = -1;

							}
						}

						//Hammer mate if near enough
						if (m_Core.m_Pos.x < 456 * 32 + 20 && pChr->m_FreezeTime > 0 && m_Core.m_Pos.y > 209 * 32 && pChr->m_Pos.y > 209 * 32 && pChr->m_Pos.x > 449 * 32 && pChr->m_Pos.x < 457 * 32)
						{
							if (pChr->m_Pos.x > m_Core.m_Pos.x - 60 && pChr->m_Pos.x < m_Core.m_Pos.x + 60)
							{
								if (m_Core.m_ActiveWeapon == WEAPON_HAMMER && m_FreezeTime == 0)
								{
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
									m_Dummy_hook_mate_after_hammer = true;
								}
							}
						}
						if (m_Dummy_hook_mate_after_hammer)
						{
							if (pChr->m_Core.m_Vel.x < -0.3f || pChr->m_Core.m_Vel.x > 0.3f)
							{
								m_Input.m_Hook = 1;
							}
							else
							{
								m_Dummy_hook_mate_after_hammer = false;
							}

							//stop this hook after some time to prevent nonstop hooking if something went wrong
							if (Server()->Tick() % 100 == 0)
							{
								m_Dummy_hook_mate_after_hammer = false;
							}

						}

						if (pChr->isFreezed)
						{
							m_Dummy_help_before_fly = true;
						}
						if (pChr->m_FreezeTime == 0)
						{
							m_Dummy_help_before_fly = false;
						}
					}

					if (m_Dummy_help_before_fly)
					{
						if (!m_Dummy_collected_weapons)
						{
							if (Server()->Tick() % 20 == 0)
							{
								SetWeapon(0);
							}
							CCharacter *pChr = GameServer()->m_World.ClosestCharTypeFreeze(m_Pos, true, this); //only search freezed tees --> so even if others get closer he still has his mission 
							if (pChr && pChr->IsAlive())
							{
								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 2);

								//Check where help is needed
								if (pChr->m_Pos.x > 457 * 32 + 10 && pChr->m_Pos.x < 468 * 32 && pChr->m_Pos.y < 213 * 32 + 5) //right freeze becken
								{
									//Get in help position:
									if (m_Core.m_Pos.x < 457 * 32 - 1)
									{
										m_Input.m_Direction = 1;
									}
									else if (m_Core.m_Pos.x > 457 * 32 + 8)
									{
										m_Input.m_Direction = -1;
									}

									//jump 
									if (m_Core.m_Vel.y == 0.000000f && m_FreezeTime == 0 && m_Core.m_Pos.y > 209 * 32)
									{
										if (Server()->Tick() % 16 == 0)
										{
											m_Input.m_Jump = 1;
										}
									}

									//hook
									if (m_Core.m_Pos.y < pChr->m_Pos.y - 60 && pChr->m_FreezeTime > 0)
									{
										m_Input.m_Hook = 1;
										if (m_Core.m_Pos.x > 454 * 32)
										{
											m_Input.m_Direction = -1;
										}
									}

									//unfreezehammer
									if (pChr->m_Pos.x < m_Core.m_Pos.x + 60 && pChr->m_Pos.x > m_Core.m_Pos.x - 60 && pChr->m_Pos.y < m_Core.m_Pos.y + 60 && pChr->m_Pos.y > m_Core.m_Pos.y - 60)
									{
										if (m_FreezeTime == 0)
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}
									}

								}
								else if (pChr->m_Pos.x > 469 * 32 + 20 && pChr->m_Pos.x < 480 * 32 && pChr->m_Pos.y < 213 * 32 + 5 && pChr->m_Pos.y > 202 * 32)
								{
									//Get in help position:
									if (m_Core.m_Pos.x < 467 * 32)
									{
										if (m_Core.m_Pos.x < 458 * 32)
										{
											if (m_Core.m_Vel.y == 0.000000f)
											{
												m_Input.m_Direction = 1;
											}
										}
										else
										{
											m_Input.m_Direction = 1;
										}

										if (m_Core.m_Vel.y > 0.2f || m_Core.m_Pos.y > 212 * 32)
										{
											m_Input.m_Jump = 1;
										}
									}
									if (m_Core.m_Pos.x > 469 * 32)
									{
										m_Input.m_Direction = -1;
										if (m_Core.m_Vel.y > 0.2f || m_Core.m_Pos.y > 212 * 32)
										{
											m_Input.m_Jump = 1;
										}
									}

									//jump 
									if (m_Core.m_Vel.y == 0.000000f && m_FreezeTime == 0 && m_Core.m_Pos.y > 209 * 32 && m_Core.m_Pos.x > 466 * 32)
									{
										if (Server()->Tick() % 16 == 0)
										{
											m_Input.m_Jump = 1;
										}
									}

									//hook
									if (m_Core.m_Pos.y < pChr->m_Pos.y - 60 && pChr->m_FreezeTime > 0)
									{
										m_Input.m_Hook = 1;
										if (m_Core.m_Pos.x > 468 * 32)
										{
											m_Input.m_Direction = -1;
										}
									}

									//unfreezehammer
									if (pChr->m_Pos.x < m_Core.m_Pos.x + 60 && pChr->m_Pos.x > m_Core.m_Pos.x - 60 && pChr->m_Pos.y < m_Core.m_Pos.y + 60 && pChr->m_Pos.y > m_Core.m_Pos.y - 60)
									{
										if (m_FreezeTime == 0)
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}
									}

								}
								else if (pChr->m_Pos.x > 437 * 32 && pChr->m_Pos.x < 456 * 32 && pChr->m_Pos.y < 219 * 32 && pChr->m_Pos.y > 203 * 32) //left freeze becken
								{
									if (m_aWeapons[2].m_Got && Server()->Tick() % 40 == 0)
									{
										SetWeapon(2);
									}


									if (m_Core.m_Jumped == 0)//has dj --> go left over the freeze and hook ze mate
									{
										m_Input.m_Direction = -1;
									}
									else //no jump --> go back and get it
									{
										m_Input.m_Direction = 1;
									}

									if (m_Core.m_Pos.y > 211 * 32 + 21)
									{
										m_Input.m_Jump = 1;
										m_Dummy_help_m8_before_hf_hook = true;
										if (m_aWeapons[2].m_Got && m_FreezeTime == 0)
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}

										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "jump + hook");
									}

									if (m_Dummy_help_m8_before_hf_hook)
									{
										m_Input.m_Hook = 1;
										m_Dummy_help_m8_before_hf_hook++;
										if (m_Dummy_help_m8_before_hf_hook > 60 && m_Core.m_HookState != HOOK_GRABBED)
										{
											m_Dummy_help_m8_before_hf_hook = 0;
											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stopped hook");
										}
									}
								}
								else //unknown area
								{
									m_Dummy_help_before_fly = false;
								}
							}
							else //no freezed tee found
							{
								m_Dummy_help_before_fly = false;
							}
						}
					}
					//else  //old else new is if because the bot can stop helping if the closestplayer is in a unknown area fail
					if (!m_Dummy_help_before_fly)
					{
						//                                 fuck off mate i go solo fggt xD
						if (!m_Dummy_collected_weapons /*|| !m_Dummy_mate_collected_weapons*/)
						{
							if (Server()->Tick() % 20 == 0)
							{
								SetWeapon(0);
							}

							CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
							if (pChr && pChr->IsAlive())
							{
								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								//Hammerfly normal way
								if (m_Dummy_mode23 == 0)
								{
									//shot schiessen schissen
									//im freeze nicht schiessen
									if (m_DummyFreezed == false)
									{
										m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
										m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

										//schiess delay
										if (Server()->Tick() >= m_EmoteTickNext && pChr->m_Pos.y < 212 * 32 - 5)
										{
											m_pPlayer->m_LastEmote = Server()->Tick();


											GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);

											m_LatestInput.m_Fire++;
											m_Input.m_Fire++;

											m_EmoteTickNext = Server()->Tick() + Server()->TickSpeed() / 2;
										}

										//wenn schon nah an der nade
										if (m_Core.m_Pos.y < 167 * 32)
										{
											m_Input.m_Jump = 1;

											if (m_Core.m_Pos.x < 453 * 32 - 8)
											{
												m_Input.m_Direction = 1;
											}
											else if (m_Core.m_Pos.x > 454 * 32 + 8)
											{
												m_Input.m_Direction = -1;
											}
										}

									}
									else if (m_DummyFreezed == true) //if (m_DummyFreezed == false)
									{
										m_LatestInput.m_Fire = 0;
										m_Input.m_Fire = 0;
										m_DummyFreezed = false;
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hey im freezded lul xD");
									}
									else
									{
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "iwas is maechtig flasch gelaufen du bob");
									}
								}
								else if (m_Dummy_mode23 == 2) //Speedhammerfly for ChillerDragon
								{
									//lauf zu dem hin
									//nur wenn der nächste spieler grad nach ooben fliegt oder auf der selben höhe ist
									if (pChr->m_Pos.y == m_Core.m_Pos.y || pChr->m_Core.m_Vel.y < -0.4f)
									{
										if (pChr->m_Pos.y >= m_Core.m_Pos.y)
										{
											if (pChr->m_Pos.x + 1 < m_Core.m_Pos.x) //wenn zu weit rechts
											{
												if (m_Core.m_Pos.x > 452 * 32)
													m_Input.m_Direction = -1;
											}
											else if (m_Core.m_Pos.x + 1 < pChr->m_Pos.x) //wenn zu weit links
											{
												if (m_Core.m_Pos.x < 455 * 32)
													m_Input.m_Direction = 1;
											}
										}
									}

									//und wenn der hoch springt
									//hau den weg xD
									if (pChr->m_Core.m_Vel.y < -0.5f && m_Core.m_Pos.y + 15 > pChr->m_Pos.y) //wenn der dude speed nach oben hat
									{
										m_Input.m_Jump = 1;
										if (m_FreezeTime == 0)
										{
											m_LatestInput.m_Fire++;
											m_Input.m_Fire++;
										}
									}
								}
								else
								{
									if (Server()->Tick() % 600 == 0)
									{
										GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "for this dummymode23 is no hammerflytype set :c");
									}
								}
							}
						}
					}
				}


				//Der krasse shit!
				//Der bot macht den 2Player part o.O
				//erstmal schauen wann der bot den 2player part machen soll

				//if (m_Core.m_Pos.x > 466 * 32)
				//{
				//	CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
				//	if (pChr && pChr->IsAlive())
				//	{
				//		m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				//		m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				//		m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				//		m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				//		if (m_Dummy_collected_weapons && m_FreezeTime == 0 && m_Core.m_Pos.x > 478 * 32 && m_Core.m_Pos.x < 485 * 32 && pChr->m_Pos.x > 476 * 32)
				//		{
				//			SetWeapon(0);


				//			if (pChr->m_Pos.x < m_Core.m_Pos.x && m_Core.m_Pos.x > 481 * 32 && pChr->m_Pos.y < 195 * 32) //wenn sich der racemate zum schieben eignet schieb ihn zum abgrund um ihn dort dann später zu hammern
				//			{
				//				m_Input.m_Direction = -1;
				//			}

				//			if (pChr->m_Pos.x < m_Core.m_Pos.x && m_Core.m_Pos.x - pChr->m_Pos.x < 8 && m_Core.m_Pos.x > 481 * 32 - 1) //wenn der racemate links des bots bereit um ins freeze gehammert zu werden liegt hau raus
				//			{
				//				m_Input.m_Fire++;
				//				m_LatestInput.m_Fire++;
				//			}

				//			if (pChr->m_Pos.y > 194 * 32 && m_Core.m_Pos.x < 481 * 32 && pChr->m_Pos.x < m_Core.m_Pos.x && pChr->m_Core.m_Vel.y > -0.5f) //wenn der racemate unter der platform ist und der bot geeigent zum draggen is ---> gogo
				//			{
				//				m_Dummy_dd_hook = true;
				//			}

				//			//TODO: 
				//			//abfrage die m_Dummy_dd_hook wieder auf false setzt (wenn pChr zu tief is oder zu weit rechts oder der hammer gehittet hat)

				//			if (m_Dummy_dd_hook)
				//			{
				//				m_Input.m_Hook = 1;
				//				m_Input.m_Direction = 1;

				//				if (pChr->m_Pos.x > 485 * 32)
				//				{
				//					m_Input.m_Jump = true;
				//				}
				//			}
				//		}
				//	}
				//}

				/*
				New Stuff:
				commented out the whole old system

				Struct:

				STRUCT[1]: Check if bot shoudl change m_Dummy_2p_state

				STRUCT[2]: Let the bot do stuff depenging on m_Dummy_2p_state

				States:
				-2				Help pChr out of freeze
				-1				do nothing
				0				prepare for making the part (gettin in the right position)
				1				starting to do the part -> walking left and hammerin'
				2				keep going doing the part -> hookin' and walking to the right
				3				final stage of doing the part -> jumpin' and unfreeze pChr with hammerhit
				4				jump in freeze and let the mate help

				5				go on edge if pChr dragged u through the part 
				6				if on edge sg and unfreeze mate

				*/

				if (m_Core.m_Pos.y < 200 * 32)
				{
					//check ob der mate fail ist
					CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
					if (pChr && pChr->IsAlive())
					{
						//                                                                                      NEW: to recognize mates freeze in the freeze tunnel on the left
						if ((pChr->m_Pos.y > 198 * 32 + 10 && pChr->isFreezed && pChr->m_Core.m_Vel.y == 0.000000f) || (pChr->m_Pos.y < 198 * 32 + 10 && pChr->m_Pos.x < 472 * 32 && pChr->isFreezed && pChr->m_Core.m_Vel.y == 0.000000f))
						{
							m_Dummy_mate_failed = true;
						}
						if (pChr->m_FreezeTime == 0)
						{
							m_Dummy_mate_failed = false;
						}
					}


					//schau ob der bot den part geschafft hat und auf state -1 gehen soll
					if (m_Core.m_Pos.x > 485 * 32)
					{
						m_Dummy_2p_state = -1; //part geschafft --> mach aus 
					}


					if (m_Core.m_Pos.x > 466 * 32)
					{
						CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this); 
						if (pChr && pChr->IsAlive())
						{
							m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
							m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							//holla
							//if (m_Dummy_collected_weapons && m_FreezeTime == 0 && m_Core.m_Pos.x > 478 * 32 && m_Core.m_Pos.x < 485 * 32 && pChr->m_Pos.x > 476 * 32)
							if (m_Dummy_collected_weapons && m_FreezeTime == 0 && m_Core.m_Pos.x > 478 * 32 && m_Core.m_Pos.x < 492 * 32 + 10 && pChr->m_Pos.x > 476 * 32) //new testy
							{
								//New direct in state
								//if (m_Dummy_2p_state == 1)
								//SetWeapon(0);


								//Reset Checkbools

								//if (!m_Core.m_HookState == HOOK_GRABBED && m_Dummy_2p_hook_grabbed) //wenn der bot denkt er grabbt ihn noch aber schon los gelassen hat --> fang von vorne an
								//{
								//	m_Dummy_2p_hook = false;
								//}

								if (pChr->m_Pos.y > 198 * 32) //wenn pChr iwiw runter gefallen is dann mach den hook weg
								{
									m_Dummy_2p_hook = false;
								}


								//STRUCT[1]: Check if bot shoudl change m_Dummy_2p_state
								if (m_Core.m_Pos.x < 477 * 32 || m_Dummy_mate_failed) //TODO: add if pChr wants to make the part
								{
									m_Dummy_2p_state = -1;
								}
								//                                                                                     || neu resette wenn der spieler kurz von der platform springt
								// NEW: added the bool to not start doing the part while helping
								if (m_Core.m_Pos.x > 477 * 32 && m_Core.m_Pos.x < 485 * 32 && m_Core.m_Pos.y < 195 * 32  /*|| pChr->m_Pos.x < 476 * 32 - 11 || pChr->m_Pos.y < 191 * 32*/) //alle states die mit anfangen zutuen haben nur wenn der bot auch in position steht den part zu machen
								{
									if (pChr->m_FreezeTime == 0 && m_FreezeTime == 0) //wenn beide unfreeze sind zeih auf
									{
										m_Dummy_2p_state = 0;
										//m_Dummy_2p_hook = false;
										//m_Dummy_2p_hook_grabbed = false;
									}
									//																								// NEW testy || stuff
									if ((m_Core.m_Pos.x > pChr->m_Pos.x && pChr->m_Pos.y == m_Core.m_Pos.y && m_Core.m_Pos.x > 481 * 32)
										|| (pChr->m_Pos.x > 476 * 32 - 10 && m_Core.m_Pos.x > pChr->m_Pos.x && pChr->m_Pos.y > 191 * 32 - 10 && m_Core.m_Pos.x < 482 * 32 + 10))
									{
										m_Dummy_2p_state = 1; //starting to do the part->walking left and hammerin'
										if (Server()->Tick() % 30 == 0 && m_Dummy_nothing_happens_counter == 0)
										{
											SetWeapon(0);
										}
										//m_Dummy_2p_hammer1 = false;
									}
									//                                                                                 NEW TESTY || stuff     wenn der schonmal ausgelöst wurde bleib da bis der nexte ausgelöst wird oder pChr runter füllt
									if ((m_Dummy_2p_state == 1 && pChr->m_Core.m_Vel.y > 0.5f && pChr->m_Pos.x < 479 * 32) || m_Dummy_2p_hook)
									{
										m_Dummy_2p_state = 2; //keep going doing the part->hookin' and walking to the right
										m_Dummy_2p_hook = true;
										/*						if (m_Core.m_HookState == HOOK_GRABBED)
										{
										m_Dummy_2p_hook_grabbed = true;
										}*/
									}

									if (m_Dummy_2p_state == 2 && pChr->m_Pos.x > 485 * 32 + 8)
									{
										m_Dummy_2p_state = 3; //final stage of doing the part->jumpin' and unfreeze pChr with hammerhit
									}

									//           NICHT NACH FREEZE ABRAGEN damit der bot auch ins freeze springt wenn das team fail ist und dann selfkill macht
									if (pChr->m_Pos.x > 489 * 32 || (pChr->m_Pos.x > 486 * 32 && pChr->m_Pos.y < 186 * 32)) //Wenn grad gehammert und der tee weit genugn is spring rein
									{
										m_Dummy_2p_state = 4;
									}


									if (pChr->m_Pos.y < 191 * 32 && pChr->m_Pos.x < 486 * 32) //resette auf state=0 wenn pChr springt
									{
										//TODO:
										//das auch mal aus machen auch wenn nichts abbricht
										m_Dummy_2p_hook = false;
									}

									//testy set the bot to mode -1 if mate fails 
									if (m_Dummy_mate_failed)
									{
										m_Dummy_2p_state = -1;
									}

								}


								//state=? 5 //extern weil der bot woanders is
								if (m_FreezeTime == 0 && m_Core.m_Pos.x > 485 * 32 && pChr->m_Pos.x < 485 * 32) //wenn der bot rechts und unfreeze is und der mate noch links
								{
									m_Dummy_2p_state = 5;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "set state 5");
								}

								if (m_FreezeTime == 0 && m_Core.m_Pos.x > 490 * 32 && pChr->m_FreezeTime > 0)
								{
									m_Dummy_2p_state = 6;
								}


								//STRUCT[2]: Let the bot do stuff depenging on m_Dummy_2p_state


								if (m_Dummy_2p_state == 0) //prepare doing the part (gettin right pos)
								{
									m_Input.m_Direction = 1; //walking right until state 1 gets triggerd 
															 //GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "debug [1]");
								}
								else if (m_Dummy_2p_state == 1) //starting to do the part -> walking left and hammerin'
								{
									if (m_Core.m_Pos.x > 480 * 32 - 15) //lauf nach links bis zur hammer pos
									{
										m_Input.m_Direction = -1;
									}

									if (pChr->m_Pos.x < 480 * 32) //wenn pChr weit gwenung zum hammern is
									{
										m_Input.m_Fire++;
										m_LatestInput.m_Fire++;
										//m_Dummy_2p_hammer1 = true;
									}

									//testy stop mate if hammer was too hard and mate fly to far
									if (pChr->m_Pos.x < 478 * 32)
									{
										m_Input.m_Hook = 1;
									}

								}
								else if (m_Dummy_2p_state == 2) //keep going doing the part->hookin' and walking to the right
								{
									if (pChr->m_Pos.y > 194 * 32 + 10)
										m_Input.m_Hook = 1;

									if (pChr->m_Pos.y < 197 * 32)
										m_Input.m_Direction = 1;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "debug [2]");
								}
								else if (m_Dummy_2p_state == 3) //final stage of doing the part->jumpin' and unfreeze pChr with hammerhit
								{
									if (Server()->Tick() % 30 == 0)
									{
										SetWeapon(0); //hammer
									}

									if (pChr->m_FreezeTime > 0) //keep holding hook untill pChr is unfreeze
									{
										m_Input.m_Hook = 1;
									}

									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "debug [3]");
									m_Input.m_Direction = 1;
									m_Input.m_Jump = 1;

									//Now tricky part the unfreeze hammer
									if (pChr->m_Pos.y - m_Core.m_Pos.y < 7 && m_FreezeTime == 0) //wenn der abstand der beiden tees nach oben weniger is als 7 ^^
									{
										m_Input.m_Fire++;
										m_LatestInput.m_Fire++;
									}
								}
								//MOVED TO EXTERN CUZ SPECIAL
								//else if (m_Dummy_2p_state == 4) //PART geschafft! spring ins freeze
								//{
								//	if (m_Core.m_Pos.y < 195 * 32 && m_Core.m_Pos.x > 478 * 32) //wenn der bot noch auf der plattform is
								//	{
								//		m_Input.m_Direction = -1; //geh links bisse füllst
								//	}
								//	else //wenn de füllst
								//	{
								//		m_Input.m_Direction = 1;
								//	}
								//}
							}

							//Mega externen stuff is der state4 weil der ausm gültigeitsbereich (platform) raus läuft und so der is halt was beonders deswegen steht der an einer besonder verwirrenden stelle -.-
							if (!m_Dummy_mate_failed && m_Dummy_2p_state == 4) //PART geschafft! spring ins freeze
							{
								//Shotgun boost xD
								SetWeapon(2);
								m_Input.m_TargetX = 1;
								m_Input.m_TargetY = 1;
								m_LatestInput.m_TargetX = 1;
								m_LatestInput.m_TargetY = 1;



								if (m_Core.m_Pos.y < 195 * 32 && m_Core.m_Pos.x > 478 * 32 - 15) //wenn der bot noch auf der plattform is
								{
									if (m_Core.m_Pos.x < 480 * 32)  //wenn er schon knapp an der kante is
									{
										//nicht zu schnell laufen
										if (Server()->Tick() % 5 == 0)
										{
											m_Input.m_Direction = -1; //geh links bisse füllst
										}
									}
									else
									{
										m_Input.m_Direction = -1;
									}
								}
								else //wenn de füllst
								{
									m_Input.m_Direction = 1;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "debug [4]");
								}

								//DJ ins freeze
								if (m_Core.m_Pos.y > 195 * 32 + 10)
								{
									m_Input.m_Jump = 1;
								}

								if (m_Input.m_Direction == 1 && m_FreezeTime == 0)
								{
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
								}

							}

							if (!m_Dummy_mate_failed && m_Dummy_2p_state == 5) //made the part --> help mate
							{

								if (pChr->m_FreezeTime == 0 && pChr->m_Pos.x > 485 * 32)
								{
									m_Dummy_2p_state = -1;
								}

								if (m_Core.m_Jumped > 1) //double jumped
								{
									if (m_Core.m_Pos.x < 492 * 32 - 30) //zu weit links
									{
										m_Input.m_Direction = 1;
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Direction = 1");

										if (m_Core.m_Pos.x > 488 * 32) //wenn schon knapp dran
										{
											//nur langsam laufen (bissl bremsen)
											if (m_Core.m_Vel.x < 2.3f)
											{
												m_Input.m_Direction = 0;
											}
										}
									}
								}
								
								//hold left wall till jump to always do same move with same distance and speed
								if (m_Core.m_Jumped < 2 && !IsGrounded())
								{
									m_Input.m_Direction = -1;
								}


								if (m_Core.m_Pos.y > 195 * 32)
								{
									m_Input.m_Jump = 1;
								}

								if (m_Core.m_Pos.x > 492 * 32)
								{
									m_Input.m_Direction = -1;
								}
							}
							//else if (m_Dummy_2p_state == -2) //auch extern weil der dummy vlt mal von der platform springt zum helfen
							//if (m_Dummy_mate_failed && m_Dummy_2p_state < 1)    <--- added m_Dummy_mate_failed to the state checks
							if (m_Dummy_mate_failed)
							{
								//The bot coudl fall of the plattform and hurt but this var helps to activate and accident
								//sometimes the special stage causes a jump on purpose and the var gets true so no emergency can be called
								//to make this possible agian reset this var every tick here 
								//m_Dummy_help_no_emergency is used to allow the emergency help
								m_Dummy_help_no_emergency = false;



								if (Server()->Tick() % 20 == 0)
								{
									GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
								}

								//Go on left edge to help:
								if (m_Core.m_Pos.x > 479 * 32 + 4) //to much right
								{
									if (m_Core.m_Pos.x < 480 * 32 - 25)
									{
										if (Server()->Tick() % 9 == 0)
										{
											m_Input.m_Direction = -1;
										}
									}
									else
									{
										m_Input.m_Direction = -1;
										m_Input.m_TargetX = 300;
										m_Input.m_TargetY = -10;
										m_LatestInput.m_TargetX = 300;
										m_LatestInput.m_TargetY = -10;
									}

									if (m_Core.m_Vel.x < -1.5f && m_Core.m_Pos.x < 480 * 32)
									{
										m_Input.m_Direction = 0;
									}
								}
								else if (m_Core.m_Pos.x < 479 * 32 - 1)
								{
									m_Input.m_Direction = 1;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "debug [6]");
								}

								//Get mate with shotgun in right position:
								//if (pChr->m_Pos.x < 479 * 32 + 6) //if the mate is left enough to get shotgunned from the edge
								if (pChr->m_Pos.x < 478 * 32)
								{
									if (Server()->Tick() % 30 == 0)
									{
										SetWeapon(2); //switch to sg
									}

									if (m_FreezeTime == 0 && pChr->m_Core.m_Vel.y == 0.000000f && pChr->m_Core.m_Vel.x < 0.007f && pChr->m_Core.m_Vel.x > -0.007f && m_Core.m_Pos.x < 480 * 32)
									{
										m_Input.m_Fire++;
										m_LatestInput.m_Fire++;
									}
								}
								else //if right enough to stop sg
								{
									if (pChr->m_Pos.x < 479 * 32)
									{
										if (pChr->m_Pos.y > 194 * 32)
										{
											if (pChr->m_Core.m_Pos.y > 197 * 32)
											{
												m_Input.m_Hook = 1;
											}
											//reset hook if something went wrong
											if (Server()->Tick() % 90 == 0 && pChr->m_Core.m_Vel.y == 0.000000f) //if the bot shoudl hook but the mate lays on the ground --> resett hook
											{
												m_Input.m_Hook = 0;
												m_Dummy_nothing_happens_counter++;
												if (m_Dummy_nothing_happens_counter > 2)
												{
													if (m_Core.m_Pos.x > 478 * 32 - 1 && m_Core.m_Jumped == 0)
													{
														m_Input.m_Direction = -1;
													}
													m_Input.m_TargetX = m_Input.m_TargetX - 5;
													m_LatestInput.m_TargetX = m_LatestInput.m_TargetX - 5;
												}
												if (m_Dummy_nothing_happens_counter > 4) //warning long time nothing happend! do crazy stuff
												{
													if (m_FreezeTime == 0)
													{
														m_Input.m_Fire++;
														m_LatestInput.m_Fire++;
													}
												}
												if (m_Dummy_nothing_happens_counter > 5) //high warning mate coudl get bored --> swtich through all weapons and move angel back
												{
													SetWeapon(m_Dummy_panic_weapon);
													m_Dummy_panic_weapon++;
													m_Input.m_TargetX++;
													m_LatestInput.m_TargetX++;
												}
											}
											if (pChr->m_Core.m_Vel.y != 0.000000f)
											{
												m_Dummy_nothing_happens_counter = 0;
											}
										}

									}
									else
									{
										if (Server()->Tick() % 50 == 0)
										{
											SetWeapon(2);
										}
										if (m_Core.m_ActiveWeapon == WEAPON_SHOTGUN && m_FreezeTime == 0)
										{
											if (pChr->m_Pos.y < 198 * 32) //if mate is high enough
											{
												m_Input.m_TargetX = -200;
												m_Input.m_TargetY = 30;
												m_LatestInput.m_TargetX = -200;
												m_LatestInput.m_TargetY = 30;
												m_Input.m_Fire++;
												m_LatestInput.m_Fire++;
											}
											else //if mate is too low --> change angel or move depnding on the x position
											{
												if (pChr->m_Pos.x < 481 * 32 - 4) //left enough to get him with other shotgun angels from the edge
												{
													//first go on the leftest possible pos on the edge
													if (m_Core.m_Vel.x > -0.0004f && m_Core.m_Pos.x > 478 * 32 - 2 && m_Core.m_Jumped == 0)
													{
														m_Input.m_Direction = -1;
													}
													/*
													[PLANNED BUT NOT NEEDED]: add more help modes and switch trough them. for now just help mode 2 is used and the int to swtich modes is usless
													Then start to help.
													There are different help modes to have some variations if nothing happens
													help modes:

													1				Old way of helping try to wallshot staright down (doesnt work)
													2				New alternative! wallshot the left wall while jumping

													*/
													m_Dummy_mate_help_mode = 2;

													if (m_Dummy_mate_help_mode == 2) //new (jump and wallshot the left wall)
													{
														if (m_Core.m_Pos.y > 193 * 32 && m_Core.m_Vel.y == 0.000000f)
														{
															if (Server()->Tick() % 30 == 0)
															{
																m_Input.m_Jump = 1;
																SetWeapon(2); //switch to sg
															}
														}

														if (m_Core.m_Pos.y < 191 * 32) //prepare aim
														{

															m_Input.m_TargetX = -300;
															m_Input.m_TargetY = 200;
															m_LatestInput.m_TargetX = -300;
															m_LatestInput.m_TargetY = 200;

															if (m_Core.m_Pos.y < 192 * 32 - 30) //shoot
															{
																if (m_FreezeTime == 0 && m_Core.m_ActiveWeapon == WEAPON_SHOTGUN && m_Core.m_Vel.y < -0.5f)
																{
																	m_Input.m_Fire++;
																	m_LatestInput.m_Fire++;
																}
															}
														}


														//Panic if fall of platform
														if (m_Core.m_Pos.y > 195 * 32 + 5)
														{
															m_Input.m_Jump = 1;
															m_Input.m_Direction = 1;
															m_Input.m_TargetX = 300;
															m_Input.m_TargetY = -2;
															m_LatestInput.m_TargetX = 300;
															m_LatestInput.m_TargetY = -2;
															m_Dummy_2p_panic_while_helping = true;
														}
														if ((m_Core.m_Pos.x > 480 * 32 && m_FreezeTime == 0) || m_FreezeTime > 0) //stop this mode if the bot made it back to the platform or failed
														{
															m_Dummy_2p_panic_while_helping = false;
														}
														if (m_Dummy_2p_panic_while_helping)
														{
															m_Input.m_Direction = 1;
															m_Input.m_TargetX = 300;
															m_Input.m_TargetY = -2;
															m_LatestInput.m_TargetX = 300;
															m_LatestInput.m_TargetY = -2;
														}

													}
													else if (m_Dummy_mate_help_mode == 1) //old (shooting straight down from edge and try to wallshot)
													{
														m_Input.m_TargetX = 15;
														m_Input.m_TargetY = 300;
														m_LatestInput.m_TargetX = 15;
														m_LatestInput.m_TargetY = 300;
														if (m_Core.m_Vel.x > -0.1f && m_FreezeTime == 0)
														{
															m_Input.m_Fire++;
															m_LatestInput.m_Fire++;
														}

														if (m_Core.m_Pos.y > 195 * 32 + 5)
														{
															m_Input.m_Jump = 1;
															m_Input.m_Direction = 0; //old 1
															m_Input.m_TargetX = 300;
															m_Input.m_TargetY = -2;
															m_LatestInput.m_TargetX = 300;
															m_LatestInput.m_TargetY = -2;
															m_Dummy_2p_panic_while_helping = true;
														}
														if ((m_Core.m_Pos.x > 480 * 32 && m_FreezeTime == 0) || m_FreezeTime > 0) //stop this mode if the bot made it back to the platform or failed
														{
															m_Dummy_2p_panic_while_helping = false;
														}
														if (m_Dummy_2p_panic_while_helping)
														{
															if (m_Core.m_Pos.y < 196 * 32 - 8)
															{
																m_Input.m_Direction = 1;
															}
															else
															{
																m_Input.m_Direction = 0;
															}
															m_Input.m_TargetX = 300;
															m_Input.m_TargetY = -2;
															m_LatestInput.m_TargetX = 300;
															m_LatestInput.m_TargetY = -2;
														}
													}
												}
												else //if mate is far and dummy has to jump of the platform and shotgun him
												{
													//in this stage of helping the bot jumps of the platform on purpose
													//m_Dummy_help_no_emergency is used to prevent the an emergency because its planned
													m_Dummy_help_no_emergency = true;

													if (Server()->Tick() % 30 == 0)
													{
														SetWeapon(2);
													}
													//go down and jump
													if (m_Core.m_Jumped >= 2) //if bot has no jump
													{
														m_Input.m_Direction = 1;
													}
													else
													{
														m_Input.m_Direction = -1;

														if (m_Core.m_Pos.x < 477 * 32 && m_Core.m_Vel.x < -3.4f) //dont rush too hard intro nowehre 
														{
															m_Input.m_Direction = 0;
														}

														if (m_Core.m_Pos.y > 195 * 32) //prepare aim
														{
															m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
															m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
															m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
															m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

															if (m_Core.m_Pos.y > 196 * 32 + 25 || m_Core.m_Pos.x < 475 * 32 + 15)
															{
																m_Input.m_Fire++;
																m_LatestInput.m_Fire++;
																m_Input.m_Jump = 1;
															}

															if ((pChr->m_Pos.x < 486 * 32 && m_Core.m_Pos.y > 195 * 32 + 20) || (pChr->m_Pos.x < 486 * 32 && m_Core.m_Pos.x < 477 * 32)) //if mate is in range add a hook
															{
																m_Dummy_dd_helphook = true;
															}
															if (m_Core.m_Pos.x > 479 * 32)
															{
																m_Dummy_dd_helphook = false;
															}

															if (m_Dummy_dd_helphook)
															{
																m_Input.m_Hook = 1;
															}
														}
													}
												}
											}
										}
									}
								}

								if (m_Core.m_Pos.y < pChr->m_Pos.y + 40 && pChr->m_Pos.x < 479 * 32 + 10 && m_FreezeTime == 0) //if the mate is near enough to hammer
								{
									//dont switch to hammer because without delay it sucks
									//and with delay its too slow
									//the bot shoudl anyways have a hammer ready in this situation
									// so ---> just shoot
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
								}


								//do something if nothing happens cuz the bot is stuck somehow
								if (Server()->Tick() % 100 == 0 && pChr->m_Core.m_Vel.y == 0.000000f && m_Dummy_nothing_happens_counter == 0) //if the mate stands still after 90secs the m_Dummy_nothing_happens_countershoudl get triggerd. but if not this if function turns true
								{
									//[PLANNED]: this can cause an loop wehre nothing happens..
									//maybe add some weapon changes or change m_Input.m_TargetX a bit


									m_Input.m_Direction = -1; //ye just walk untill an emergency gets called xD
															  //ik pro trick but it moves the bot around
								}



								//Emergency takes over here if the bot got in a dangerous situation!
								//if (m_Core.m_Pos.y > 196 * 32 + 30) //+25 is used for the jump help and with 30 it shoudlnt get any confusuion i hope
								if ((m_Core.m_Pos.y > 195 * 32 && !m_Dummy_help_no_emergency)) //if the bot left the platform
								{
									m_Dummy_help_emergency = true;
								}
								if ((m_Core.m_Pos.x > 479 * 32 && m_Core.m_Jumped == 0) || isFreezed)
								{
									m_Dummy_help_emergency = false;
								}

								if (m_Dummy_help_emergency)
								{
									//resett all and let emergency control all xD
									m_Input.m_Hook = 0;
									m_Input.m_Jump = 0;
									m_Input.m_Direction = 0;
									m_LatestInput.m_Fire = 0;
									m_Input.m_Fire = 0;


									if (Server()->Tick() % 20 == 0)
									{
										GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
									}

									m_Input.m_TargetX = 0;
									m_Input.m_TargetY = -200;
									m_LatestInput.m_TargetX = 0;
									m_LatestInput.m_TargetY = -200;

									if (m_Core.m_Pos.y > 194 * 32 + 18)
									{
										m_Input.m_Jump = 1;
									}
									if (m_Core.m_Jumped >= 2)
									{
										m_Input.m_Direction = 1;
									}
								}

							} //dummy_mate_failed end

							if (m_Dummy_2p_state == 6) //extern af fuck the system
							{
								//m_pPlayer->m_TeeInfos.m_ColorBody = (255 * 255 / 360);


								m_Input.m_Direction = 0;
								m_Input.m_Jump = 0;
								//m_Input.m_Jump = 1;
								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								if (Server()->Tick() % 40 == 0 && m_aWeapons[2].m_Got)
								{
									SetWeapon(2);
								}

								if (m_FreezeTime == 0)
								{
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
								}
							}
							


						}
					}
					//Hammerhit with race mate till finish
					if (m_Dummy_mode23 == 0 || m_Dummy_mode23 == 2) //normal hammerhit
					{
						if (m_Core.m_Pos.x > 491 * 32)
						{
							if (m_Core.m_Pos.x <= 514 * 32 - 5 && pChr->m_Pos.y < 198 * 32)
							{
								SetWeapon(0);
							}

							CCharacter *pChr = GameServer()->m_World.ClosestCharTypeFreeze(m_Pos, true, this); //new 11.11.2017 Updated from ClosestCharType to TypeFreeze
							if (pChr && pChr->IsAlive())
							{
								//if (pChr->m_Pos.x > 485 * 32) //newly added this to improve the 2p_state = 5 skills (go on edge if mate made the part)
								if (pChr->m_Pos.x > 490 * 32 + 2) //newly added this to improve the 2p_state = 5 skills (go on edge if mate made the part)
								{
									m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
									m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
									m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
									m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

									//just do things if unffr
									//old shit cuz he cant rls because mate is unfreeze and dont check for later rlsing hook
									//if (m_FreezeTime == 0 && pChr->m_FreezeTime > 0) //und der mate auch freeze is (damit der nich von edges oder aus dem ziel gehookt wird)
									//                  fuck the edge only stop if finish lol
									if ((m_FreezeTime == 0 && pChr->m_Pos.x < 514 * 32 - 2) || (m_FreezeTime == 0 && pChr->m_Pos.x > 521 * 32 + 2))
									{
										//get right hammer pos [rechte seite]
										if (pChr->m_Pos.x < 515 * 32) //wenn der mate links des ziels ist
										{
											if (m_Core.m_Pos.x > pChr->m_Pos.x + 45) //zu weit rechts von hammer position
											{
												m_Input.m_Direction = -1; //gehe links

											}
											else if (m_Core.m_Pos.x < pChr->m_Pos.x + 39) // zu weit links von hammer position
											{
												m_Input.m_Direction = 1;
											}
										}
										else //get right hammer pos [rechte seite] (wenn der mate rechts des ziels is)
										{
											if (m_Core.m_Pos.x > pChr->m_Pos.x - 45) //zu weit links von hammer position
											{
												m_Input.m_Direction = -1;

											}
											else if (m_Core.m_Pos.x < pChr->m_Pos.x - 39) // zu weit rechts von hammer position
											{
												m_Input.m_Direction = 1;
											}
										}

										//deactivate bool for hook if mate is high enough or bot is freezed (but freezed is checked somewerhe else)
										//                                                                                              NEW: just rls hook if mate is higher than bot (to prevent both falling added new ||)
										//                                                                                                                                                                                oder wenn der mate unter dem bot ist und unfreeze
										if ((pChr->m_FreezeTime == 0 && pChr->m_Core.m_Vel.y > -1.5f && m_Core.m_Pos.y > pChr->m_Pos.y - 15) || pChr->m_Core.m_Vel.y > 3.4f || (pChr->m_FreezeTime == 0 && pChr->m_Pos.y + 38 > m_Core.m_Pos.y) || isFreezed)
										{
											m_Dummy_hh_hook = false;
										}
										//activate bool for hook if mate stands still
										if (pChr->m_Core.m_Vel.y == 0.000000f /*|| pChr->m_Core.m_Vel.y < -4.5f*/) //wenn er am boden liegt anfangen oder wenn er zu schnell nach obenfliegt bremsen
										{
											m_Dummy_hh_hook = true;
										}

										if (m_Dummy_hh_hook)
										{
											m_Input.m_Hook = 1;
										}

										//jump if too low && if mate is freeze otherwise it woudl be annoying af
										if (m_Core.m_Pos.y > 191 * 32 && pChr->m_FreezeTime > 0)
										{
											m_Input.m_Jump = 1;
										}

										//Hammer
										//if (pChr->m_Pos.y - m_Core.m_Pos.y < 7 && pChr->m_FreezeTime > 0) //wenn der abstand der beiden tees nach oben weniger is als 7 ^^
										if (pChr->m_FreezeTime > 0 && pChr->m_Pos.y - m_Core.m_Pos.y < 18) //wenn der abstand kleiner als 10 is nach oben
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}
									}
									else
									{
										m_Dummy_hh_hook = false; //reset hook if bot is freeze
									}


									//ReHook if mate flys to high

									if ((pChr->m_Pos.y < m_Core.m_Pos.y - 40 && pChr->m_Core.m_Vel.y < -4.4f) || pChr->m_Pos.y < 183 * 32)
									{
										m_Input.m_Hook = 1;
									}

									//Check for panic balance cuz all went wrong lol
									//if dummy is too much left and has no dj PANIC!
									//                                                                                                                                           New Panic Trigger: if both fall fast an the bot is on top                                    
									if ((pChr->m_Pos.x < 516 * 32 && m_Core.m_Jumped >= 2 && m_Core.m_Pos.x < pChr->m_Pos.x - 5 && pChr->m_Pos.y > m_Core.m_Pos.y && m_Core.m_Pos.y > 192 * 32 && pChr->isFreezed) ||
											(m_Core.m_Vel.y > 6.7f && pChr->m_Core.m_Vel.y > 7.4f && pChr->m_Pos.y > m_Core.m_Pos.y && m_Core.m_Pos.y > 192 * 32 && pChr->m_Pos.x < 516 * 32))
									{


										if (!str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "ChillerDragon") || !str_comp(Server()->ClientName(pChr->GetPlayer()->GetCID()), "ChillerDragon.*")) //only chatflame debug while racing with ChillerDragon
										{
											if (m_Dummy_sent_chat_msg == 0 && !m_Dummy_panic_balance && m_FreezeTime == 0)
											{
												int r = rand() % 16; // 0-15

												if (r == 0)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "YOU SUCK LOL!");
												}
												else if (r == 1)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "what do you do?!");
												}
												else if (r == 2)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "O M G =!! what r u triin mate?");
												}
												else if (r == 3)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "you did shit. i didnt do shit because im a bot and i am perfect.");
												}
												else if (r == 4)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "what was your plan?");
												}
												else if (r == 5)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "this looks bad! i try to balance...");
												}
												else if (r == 6)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "i think we gonna die ....  lemme try to balance");
												}
												else if (r == 7)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "what was this?");
												}
												else if (r == 8)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "you fucked it up .. let me try to save us with my balance skills.");
												}
												else if (r == 9)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "do you have lags? i dont have lags.");
												}
												else if (r == 10)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "lol.");
												}
												else if (r == 11)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "lul");
												}
												else if (r == 12)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "wtf?");
												}
												else if (r == 13)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "OMG");
												}
												else if (r == 14)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "?!");
												}
												else if (r == 15)
												{
													GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "whats going on here?!");
												}

												//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 4);
												m_Dummy_sent_chat_msg = 1;
											}
										}



										m_Dummy_panic_balance = true;
									}
									if (pChr->m_FreezeTime == 0 || isFreezed || pChr->m_Pos.x > 512 * 32 + 5) //if mate gets unfreezed or dummy freezed stop balance
									{
										m_Dummy_panic_balance = false;
									}


									if (m_Dummy_panic_balance)
									{
										if (m_Core.m_Pos.x < pChr->m_Pos.x - 2) //Bot is too far left
										{
											m_Input.m_Direction = 1;
										}
										else if (m_Core.m_Pos.x > pChr->m_Pos.x) //Bot is too far right
										{
											m_Input.m_Direction = -1;
										}

										if (m_Core.m_Pos.x > pChr->m_Pos.x - 2 && m_Core.m_Pos.x < pChr->m_Pos.x && m_Core.m_Vel.x > -0.3f && m_FreezeTime == 0)
										{
											m_Input.m_Direction = 1;
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}
									}


									//Go in finish if near enough
									if ((m_Core.m_Vel.y < 4.4f && m_Core.m_Pos.x > 511 * 32) || (m_Core.m_Vel.y < 8.4f && m_Core.m_Pos.x > 512 * 32))
									{
										if (m_Core.m_Pos.x < 514 * 32 && !m_Dummy_panic_balance)
										{
											m_Input.m_Direction = 1;
										}
									}

									//If dummy made it till finish but mate is still freeze on the left side
									//he automaiclly help. BUT if he fails the hook resett it!
									//left side                                                                                      right side
									if ((m_Core.m_Pos.x > 514 * 32 - 5 && m_FreezeTime == 0 && pChr->isFreezed && pChr->m_Pos.x < 515 * 32) || (m_Core.m_Pos.x > 519 * 32 - 5 && m_FreezeTime == 0 && pChr->isFreezed && pChr->m_Pos.x < 523 * 32))
									{
										if (Server()->Tick() % 70 == 0)
										{
											m_Input.m_Hook = 0;
											//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
										}
									}
									//if mate is too far for hook --> shotgun him
									if (m_Core.m_Pos.x > 514 * 32 - 5 && m_Core.m_Pos.x > pChr->m_Pos.x && m_Core.m_Pos.x - pChr->m_Pos.x > 8 * 32)
									{
										SetWeapon(2); //shotgun
										if (pChr->m_FreezeTime > 0 && m_FreezeTime == 0 && pChr->m_Core.m_Vel.y == 0.000000f)
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}
									}
									//another hook if normal hook doesnt work
									//to save mate if bot is finish
									if (m_Input.m_Hook == 0)
									{
										if (pChr->m_FreezeTime > 0 && m_FreezeTime == 0 && m_Core.m_Pos.y < pChr->m_Pos.y - 60 && m_Core.m_Pos.x > 514 * 32 - 5)
										{
											m_Input.m_Hook = 1;
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
											if (Server()->Tick() % 10 == 0)
											{
												GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
											}
										}
									}



									//Important dont walk of finish plattform check
									//if (m_Core.m_Vel.y < 6.4f) //Check if not falling to fast
									if (!m_Dummy_panic_balance)
									{
										if ((m_Core.m_Vel.y < 6.4f && m_Core.m_Pos.x > 512 * 32 && m_Core.m_Pos.x < 515 * 32)
											|| (m_Core.m_Pos.x > 512 * 32 + 30 && m_Core.m_Pos.x < 515 * 32)) //left side
										{
											m_Input.m_Direction = 1;
										}

										if (m_Core.m_Vel.y < 6.4f && m_Core.m_Pos.x > 520 * 32 && m_Core.m_Pos.x < 524 * 32 /* || too lazy rarly needed*/) //right side
										{
											m_Input.m_Direction = -1;
										}
									}


								}
							}
						}












					}
					else if (m_Dummy_mode23 == 1) //tricky hammerhit (harder)
					{
						if (m_Core.m_Pos.x > 491 * 32)
						{
							SetWeapon(0);
							CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
							if (pChr && pChr->IsAlive())
							{
								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								//just do things if unffr
								if (m_FreezeTime == 0 && pChr->m_FreezeTime > 0) //und der mate auch freeze is (damit der nich von edges oder aus dem ziel gehookt wird)
								{
									//get right hammer pos [rechte seite]
									if (m_Core.m_Pos.x > pChr->m_Pos.x + 45)
									{
										m_Input.m_Direction = -1;
									}
									else if (m_Core.m_Pos.x < pChr->m_Pos.x + 39)
									{
										m_Input.m_Direction = 1;
									}


									//deactivate bool for hook if mate is high enough or bot is freezed (but freezed is checked somewerhe else)
									//                                                                                              NEW: just rls hook if mate is higher than bot (to prevent both falling added new ||)
									if (/*m_Core.m_Pos.y - pChr->m_Pos.y > 15 ||*/ (pChr->m_FreezeTime == 0 && pChr->m_Core.m_Vel.y < -2.5f && pChr->m_Pos.y < m_Core.m_Pos.y) || pChr->m_Core.m_Vel.y > 3.4f)
									{
										m_Dummy_hh_hook = false;
										//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
									}
									//activate bool for hook if mate stands still
									if (pChr->m_Core.m_Vel.y == 0.000000f) //wenn er am boden liegt anfangen oder wenn er zu schnell nach obenfliegt bremsen
									{
										m_Dummy_hh_hook = true;
									}



									//jump if too low && if mate is freeze otherwise it woudl be annoying af
									if (m_Core.m_Pos.y > 191 * 32 && pChr->m_FreezeTime > 0)
									{
										m_Input.m_Jump = 1;
									}

									//Hammer
									//wenn der abstand der beiden tees nach oben weniger is als 7 ^^
									if (pChr->m_FreezeTime > 0 && pChr->m_Pos.y - m_Core.m_Pos.y < 18) //wenn der abstand kleiner als 10 is nach oben
									{
										m_Input.m_Fire++;
										m_LatestInput.m_Fire++;
									}
								}
								else
								{
									m_Dummy_hh_hook = false; //reset hook if bot is freeze
															 //GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
								}
							}
						}


						if (m_Dummy_hh_hook)
						{
							m_Input.m_Hook = 1;
						}
					}
				}


				//General bug protection resett hook in freeze
				//if (isFreezed)
				if (m_FreezeTime > 0)
				{
					m_Input.m_Hook = 0; //resett hook in freeze to prevent bugs with no hooking at last part
				}
			}

			//Leave THis LAST !!!
			//chat stuff

			if (m_Dummy_sent_chat_msg > 0 && m_Dummy_sent_chat_msg < 100)
			{
				m_Dummy_sent_chat_msg++;
			}
			else
			{
				m_Dummy_sent_chat_msg = 0;
			}
		}
		else if (m_pPlayer->m_DummyMode == 25) //ChillerDraguns testy fake nural network lol (2.0 revived)
		{
			/*
			######################################################
			#     DummyMode 25      The    FNN    Mode           #
			#           [FNN] == [FAKENEURALNETWORK]             #
			######################################################
			ModeStructure:
			
			* Spawn -> goto hardcodet spawn pos.x 353 * 32
			
			* Check for human interactions and save them in the var m_Dummy_nn_touched_by_humans
	
			*/
			char aBuf[256];

			if (m_pPlayer->m_dmm25 == -2) //stopped
			{
				m_Input.m_Hook = 0;
				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;
				m_pPlayer->m_TeeInfos.m_ColorBody = (180 * 255 / 260);
			}
			else if (!m_Dummy_nn_ready) //first get the right start pos
			{
				m_Input.m_Hook = 0;
				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;



				if (m_Core.m_Pos.x > g_Config.m_SvFNNstartX * 32 + 1)
				{
					if (IsGrounded()) //only walk on ground because air is unpredictable
					{
						if (m_Core.m_Pos.x < g_Config.m_SvFNNstartX * 32 + 10) //in 1tile nähe langsam laufen 
						{
							if (m_Core.m_Vel.x > -0.04f)
							{
								m_Input.m_Direction = -1;
							}
						}
						else
						{
							m_Input.m_Direction = -1;
						}
					}
				}
				else if (m_Core.m_Pos.x < g_Config.m_SvFNNstartX * 32 - 1)
				{
					if (IsGrounded()) //only walk on ground because air is unpredictable
					{
						if (m_Core.m_Pos.x > g_Config.m_SvFNNstartX * 32 - 10) //in 1tile nähe langsam laufen 
						{
							if (m_Core.m_Vel.x < 0.04f)
							{
								m_Input.m_Direction = 1;
							}
						}
						else
						{
							m_Input.m_Direction = 1;
						}
					}
				}
				else //correct position
				{
					if (IsGrounded()) //only start on ground because air is unpredictable
					{
						m_Dummy_nn_ready = true;
						m_StartPos = m_Core.m_Pos;
						dbg_msg("FNN", "Found start position (%.2f/%.2f) -> starting process", m_Core.m_Pos.x / 32, m_Core.m_Pos.y / 32);
					}
				}

				//Catch errors
				if (m_Dummy_nn_ready_time > 300)
				{
					GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", "Starting process failed. Get in start position took too long -> restarting...");
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}
				m_Dummy_nn_ready_time++;
				//char aBuf[256];
				//str_format(aBuf, sizeof(aBuf), "time: %d", m_Dummy_nn_ready_time);
				//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", aBuf);

			}
			else //if the bot has the right start pos start doing random stuff 
			{
				/*


				dummy sub mode structure:

				old used a bool (m_Dummy_nn_write) which has to be changed manually in the source.

				new system uses chat command /dmm25 = dummmymodemode25 to choose submodes.
				submodes:
				-2					stop all
				-1					error/offline

				0					write
				1					read/load highest distance
				2					read/load highest fitness
				3					read/load lowest distance_finish

				4					play loaded run

				*/


				//int m_aMoveID = -1;
				//int Hooked = false;
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					CCharacter *pChar = GameServer()->GetPlayerChar(i);

					if (!pChar || !pChar->IsAlive() || pChar == this)
						continue;

					if (pChar->Core()->m_HookedPlayer == m_pPlayer->GetCID())
					{
						//Hooked = true;
						//m_aMoveID = i;
						m_Dummy_nn_touched_by_humans = true;
						GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "[FNN] DONT TOUCH ME HOOK WTF");
					}

					if (Core()->m_HookedPlayer != -1)
					{
						str_format(aBuf, sizeof(aBuf), "[FNN] dont get in my hook %s", Server()->ClientName(Core()->m_HookedPlayer));
						GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, aBuf);
					}
				}
				//if (m_Core.m_HookState == HOOK_GRABBED) //this includes normal collision hooks
				//{
				//	m_Dummy_nn_touched_by_humans = true;
				//	GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "[FNN] dont get in my hook -.-");
				//}
				//selfmade noob code check if pChr is too near and coudl touched the bot
				CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true);
				if (pChr && pChr->IsAlive() && pChr != this)
				{
					if (pChr->m_Pos.x < m_Core.m_Pos.x + 60 && pChr->m_Pos.x > m_Core.m_Pos.x - 60 && pChr->m_Pos.y < m_Core.m_Pos.y + 60 && pChr->m_Pos.y > m_Core.m_Pos.y - 60)
					{
						m_Dummy_nn_touched_by_humans = true;
						GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "[FNN] dont touch my body with yours pls");
					}

				}

				//always set to black
				m_pPlayer->m_TeeInfos.m_ColorBody = (0 * 255 / 360);

				if (m_pPlayer->m_dmm25 == -1) //error
				{
					m_Input.m_Hook = 0;
					m_Input.m_Jump = 0;
					m_Input.m_Direction = 0;
					m_LatestInput.m_Fire = 0;
					m_Input.m_Fire = 0;
					m_pPlayer->m_TeeInfos.m_ColorBody = (180 * 255 / 260);
				}
				else if (m_pPlayer->m_dmm25 == 0) //submode[0] write
				{
					//m_pPlayer->m_TeeInfos.m_Name = "writing...";
					//m_pPlayer->m_TeeInfos.m_ColorBody = (180 * 255 / 360);


					//m_pPlayer->m_Dummy_nn_time++; //maybe use it some day to analys each tick stuff or total trainign time idk

					int rand_Direction = rand() % 3 - 1; //-1 0 1
					int rand_Fire = rand() % 2; // 1 0
					int rand_Jump = rand() % 2;
					int rand_Hook = rand() % 2;
					int rand_Weapon = rand() % 4;
					int rand_TargetX = rand() % 401 - 200;
					int rand_TargetY = rand() % 401 - 200;

					m_Input.m_Direction = rand_Direction;
					m_Input.m_Jump = rand_Jump;
					m_Input.m_Hook = rand_Hook;
					m_Input.m_TargetX = rand_TargetX;
					m_Input.m_TargetY = rand_TargetY;
					m_LatestInput.m_TargetX = rand_TargetX;
					m_LatestInput.m_TargetY = rand_TargetY;

					//save values in array
					m_aRecMove[m_FNN_CurrentMoveIndex] = rand_Direction;
					m_FNN_CurrentMoveIndex++;
					m_aRecMove[m_FNN_CurrentMoveIndex] = rand_Jump;
					m_FNN_CurrentMoveIndex++;
					m_aRecMove[m_FNN_CurrentMoveIndex] = rand_Hook;
					m_FNN_CurrentMoveIndex++;
					m_aRecMove[m_FNN_CurrentMoveIndex] = rand_TargetX;
					m_FNN_CurrentMoveIndex++;
					m_aRecMove[m_FNN_CurrentMoveIndex] = rand_TargetY;
					m_FNN_CurrentMoveIndex++;

					if (rand_Weapon == 0)
					{
						SetWeapon(0); //hammer
					}
					else if (rand_Weapon == 1)
					{
						SetWeapon(1); //gun
					}
					else if (rand_Weapon == 2)
					{
						if (m_aWeapons[WEAPON_SHOTGUN].m_Got)
						{
							SetWeapon(2); //shotgun
						}
					}
					else if (rand_Weapon == 3)
					{
						if (m_aWeapons[WEAPON_GRENADE].m_Got)
						{
							SetWeapon(3); //grenade
						}
					}
					else if (rand_Weapon == 4)
					{
						if (m_aWeapons[WEAPON_RIFLE].m_Got)
						{
							SetWeapon(4); //laser
						}
					}
					else
					{
						str_format(aBuf, sizeof(aBuf), "Error unknown weapon:", rand_Weapon);
						GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", aBuf);
					}
					if (rand_Fire == 1 && m_FreezeTime == 0)
					{
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
					}

					if (m_FNN_CurrentMoveIndex > FNN_MOVE_LEN - 12) //minus inputs because every tick the index gets incremented by 5
					{
						float newest_distance = distance(m_StartPos, m_Core.m_Pos);
						vec2 current_pos(0,0);
						current_pos.x = m_Core.m_Pos.x / 32;
						current_pos.y = m_Core.m_Pos.y / 32;
						float newest_distance_finish = distance(current_pos, GameServer()->m_FinishTilePos);
						float newest_fitness = newest_distance_finish / m_FNN_CurrentMoveIndex;
						str_format(aBuf, sizeof(aBuf), "[FNN] ran out of memory ticks=%d distance=%.2f fitness=%.2f distance_finish=%.2f", m_FNN_CurrentMoveIndex, newest_distance, newest_fitness, newest_distance_finish);
						GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, aBuf);
						//Die(m_pPlayer->GetCID(), WEAPON_SELF);
						m_Dummy_nn_stop = true;
					}

					if (g_Config.m_SvFNNtimeout && !m_Dummy_nn_stop)
					{
						if (m_FNN_CurrentMoveIndex > g_Config.m_SvFNNtimeout)
						{
							str_format(aBuf, sizeof(aBuf), "[FNN] timeouted after ticks=%d", m_FNN_CurrentMoveIndex);
							GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, aBuf);
							m_Dummy_nn_stop = true;
						}
					}

					if ((m_Core.m_Vel.y == 0.000000f && m_Core.m_Vel.x < 0.01f && m_Core.m_Vel.x > -0.01f && isFreezed) || m_Dummy_nn_stop)
					{
						if (Server()->Tick() % 10 == 0)
						{
							GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3);
						}
						if (Server()->Tick() % 40 == 0)
						{
							GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", "======================================");
							if (m_Dummy_nn_touched_by_humans) 
							{
								str_format(aBuf, sizeof(aBuf), "Failed at (%.2f/%.2f) --> RESTARTING", m_Core.m_Pos.x / 32, m_Core.m_Pos.y / 32);
								GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", aBuf);
								GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", "Got touched by Humans --> DELETE RUN");
								Die(m_pPlayer->GetCID(), WEAPON_SELF);
							}
							else //no interaction with humans --> save normal
							{
								str_format(aBuf, sizeof(aBuf), "Failed at (%.2f/%.2f) --> RESTARTING", m_Core.m_Pos.x / 32, m_Core.m_Pos.y / 32);
								GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", aBuf);
								//Die(m_pPlayer->GetCID(), WEAPON_SELF); //moved to the end because we use character variables but im not sure.. maybe it had a sense its here

								//prepare data
								if (m_FNN_CurrentMoveIndex > FNN_MOVE_LEN) //dont overload arraylength
								{
									m_FNN_CurrentMoveIndex = FNN_MOVE_LEN;
								}

								float newest_distance = distance(m_StartPos, m_Core.m_Pos);
								vec2 current_pos(0, 0);
								current_pos.x = m_Core.m_Pos.x / 32;
								current_pos.y = m_Core.m_Pos.y / 32;
								float newest_distance_finish = distance(current_pos, GameServer()->m_FinishTilePos);
								float newest_fitness = newest_distance_finish / m_FNN_CurrentMoveIndex;
								dbg_msg("FNN", "distance=%.2f", newest_distance);
								dbg_msg("FNN", "moveticks=%d", m_FNN_CurrentMoveIndex);
								dbg_msg("FNN", "fitness=%.2f", newest_fitness);
								dbg_msg("FNN", "distance_finish=%.2f", newest_distance_finish);
								

								/***************************************
								*                                      *
								*                                      *
								*         D I S T A N C E              *
								*                                      *
								*                                      *
								****************************************/

								if (newest_distance > GameServer()->m_FNN_best_distance)
								{
									//saving the distance
									dbg_msg("FNN","new distance highscore Old=%.2f -> New=%.2f", GameServer()->m_FNN_best_distance, newest_distance);
									GameServer()->m_FNN_best_distance = newest_distance;
									std::ofstream statsfile;
									char aFilePath[512];
									str_format(aFilePath, sizeof(aFilePath), "FNN/move_stats.fnn");
									statsfile.open(aFilePath);
									if (statsfile.is_open())
									{
										statsfile << newest_distance; //distance
										statsfile << std::endl;
										statsfile << GameServer()->m_FNN_best_fitness; //fitness
										statsfile << std::endl;
										statsfile << GameServer()->m_FNN_best_distance_finish; //distance_finish
										statsfile << std::endl;
									}
									else
									{
										dbg_msg("FNN", "failed to update stats. failed to open file '%s'", aFilePath);
									}
									statsfile.close();

									//saving the run
									std::ofstream savefile;
									str_format(aFilePath, sizeof(aFilePath), "FNN/move_distance.fnn");
									savefile.open(aFilePath/*, std::ios::app*/); //dont append rewrite
									if (savefile.is_open())
									{
										//first four lines are stats
										savefile << m_FNN_CurrentMoveIndex; //moveticks
										savefile << std::endl;
										savefile << newest_distance; //distance
										savefile << std::endl;
										savefile << newest_fitness; //fitness
										savefile << std::endl;
										savefile << newest_distance_finish; //distance_finish
										savefile << std::endl;

										for (int i = 0; i < m_FNN_CurrentMoveIndex; i++)
										{
											savefile << m_aRecMove[i];
											savefile << std::endl;
										}
									}
									else
									{
										dbg_msg("FNN", "failed to save record. failed to open file '%s'", aFilePath);
									}
									savefile.close();
								}
								
								/***************************************
								*                                      *
								*                                      *
								*         F I T T N E S S              *
								*                                      *
								*                                      *
								****************************************/

								if (newest_fitness > GameServer()->m_FNN_best_fitness)
								{
									//saving the fitness
									dbg_msg("FNN", "new fitness highscore Old=%.2f -> New=%.2f", GameServer()->m_FNN_best_fitness, newest_fitness);
									GameServer()->m_FNN_best_fitness = newest_fitness;
									std::ofstream statsfile;
									char aFilePath[512];
									str_format(aFilePath, sizeof(aFilePath), "FNN/move_stats.fnn");
									statsfile.open(aFilePath);
									if (statsfile.is_open())
									{
										statsfile << GameServer()->m_FNN_best_distance; //distance
										statsfile << std::endl;
										statsfile << newest_fitness; //fitness
										statsfile << std::endl;
										statsfile << GameServer()->m_FNN_best_distance_finish; //distance_finish
										statsfile << std::endl;
									}
									else
									{
										dbg_msg("FNN", "failed to update stats. failed to open file '%s'", aFilePath);
									}
									statsfile.close();

									//saving the run
									std::ofstream savefile;
									str_format(aFilePath, sizeof(aFilePath), "FNN/move_fitness.fnn");
									savefile.open(aFilePath);
									if (savefile.is_open())
									{
										//first four lines are stats
										savefile << m_FNN_CurrentMoveIndex; //moveticks
										savefile << std::endl;
										savefile << newest_distance; //distance
										savefile << std::endl;
										savefile << newest_fitness; //fitness
										savefile << std::endl;
										savefile << newest_distance_finish; //distance_finish
										savefile << std::endl;

										for (int i = 0; i < m_FNN_CurrentMoveIndex; i++)
										{
											savefile << m_aRecMove[i];
											savefile << std::endl;
										}
									}
									else
									{
										dbg_msg("FNN", "failed to save record. failed to open file '%s'", aFilePath);
									}
									savefile.close();
								}

								/***************************************
								*                                      *
								*                                      *
								*         D I S T A N C E              *
								*         F I N I S H                  *
								*                                      *
								****************************************/

								if (newest_distance_finish < GameServer()->m_FNN_best_distance_finish)
								{
									//saving the distance_finish
									dbg_msg("FNN", "new distance_finish highscore Old=%.2f -> New=%.2f", GameServer()->m_FNN_best_distance_finish, newest_distance_finish);
									GameServer()->m_FNN_best_distance_finish = newest_distance_finish;
									std::ofstream statsfile;
									char aFilePath[512];
									str_format(aFilePath, sizeof(aFilePath), "FNN/move_stats.fnn");
									statsfile.open(aFilePath);
									if (statsfile.is_open())
									{
										statsfile << GameServer()->m_FNN_best_distance; //distance
										statsfile << std::endl;
										statsfile << GameServer()->m_FNN_best_fitness; //fitness
										statsfile << std::endl;
										statsfile << newest_distance_finish; //distance_finish
										statsfile << std::endl;
									}
									else
									{
										dbg_msg("FNN", "failed to update stats. failed to open file '%s'", aFilePath);
									}
									statsfile.close();

									//saving the run
									std::ofstream savefile;
									str_format(aFilePath, sizeof(aFilePath), "FNN/move_distance_finish.fnn");
									savefile.open(aFilePath);
									if (savefile.is_open())
									{
										//first four lines are stats
										savefile << m_FNN_CurrentMoveIndex; //moveticks
										savefile << std::endl;
										savefile << newest_distance; //distance
										savefile << std::endl;
										savefile << newest_fitness; //fitness
										savefile << std::endl;
										savefile << newest_distance_finish; //distance_finish
										savefile << std::endl;

										for (int i = 0; i < m_FNN_CurrentMoveIndex; i++)
										{
											savefile << m_aRecMove[i];
											savefile << std::endl;
										}
									}
									else
									{
										dbg_msg("FNN", "failed to save record. failed to open file '%s'", aFilePath);
									}
									savefile.close();
								}

								m_FNN_CurrentMoveIndex = 0;
								Die(m_pPlayer->GetCID(), WEAPON_SELF);
							}
							GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "FNN", "======================================");
						}
					}
				}
				else if (m_pPlayer->m_dmm25 == 1) //submode[1] read/load distance
				{
					/*
					//reset values
					m_FNN_CurrentMoveIndex = 0;
					float loaded_distance = 0;
					float loaded_fitness = 0;

					//load run
					std::ifstream readfile;
					char aFilePath[512];
					str_format(aFilePath, sizeof(aFilePath), "FNN/move_distance.fnn");
					readfile.open(aFilePath);
					if (readfile.is_open())
					{
						std::string line;
						int i = 0;

						//first tree lines are stats:

						std::getline(readfile, line); //moveticks
						m_FNN_ticks_loaded_run = atoi(line.c_str());

						std::getline(readfile, line); //distance
						loaded_distance = atoi(line.c_str());

						std::getline(readfile, line); //fitness
						loaded_fitness = atoi(line.c_str());

						while (std::getline(readfile, line))
						{
							m_aRecMove[i] = atoi(line.c_str());
							i++;
						}
					}
					else
					{
						dbg_msg("FNN", "failed to load move. failed to open '%s'", aFilePath);
						m_pPlayer->m_dmm25 = -1;
					}

					//start run
					m_pPlayer->m_dmm25 = 2;
					str_format(aBuf, sizeof(aBuf), "[FNN] loaded run with ticks=%d distance=%.2f fitness=%.2f", m_FNN_ticks_loaded_run, loaded_distance, loaded_fitness);
					GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, aBuf);
					*/
					GameServer()->FNN_LoadRun("FNN/move_distance.fnn", m_pPlayer->GetCID());
				}
				else if (m_pPlayer->m_dmm25 == 2) //submode[2] read/load fitness
				{
					GameServer()->FNN_LoadRun("FNN/move_fitness.fnn", m_pPlayer->GetCID());
				}
				else if (m_pPlayer->m_dmm25 == 3) //submode[3] read/load lowest distance_finish
				{
					GameServer()->FNN_LoadRun("FNN/move_distance_finish.fnn", m_pPlayer->GetCID());
				}
				else if (m_pPlayer->m_dmm25 == 4) //submode[4] play loaded run
				{
					m_Input.m_Direction = m_aRecMove[m_FNN_CurrentMoveIndex];
					m_FNN_CurrentMoveIndex++;
					m_Input.m_Jump = m_aRecMove[m_FNN_CurrentMoveIndex];
					m_FNN_CurrentMoveIndex++;
					m_Input.m_Hook = m_aRecMove[m_FNN_CurrentMoveIndex];
					m_FNN_CurrentMoveIndex++;
					m_Input.m_TargetX = m_aRecMove[m_FNN_CurrentMoveIndex];
					m_FNN_CurrentMoveIndex++;
					m_Input.m_TargetY = m_aRecMove[m_FNN_CurrentMoveIndex];
					m_FNN_CurrentMoveIndex++;
					m_LatestInput.m_TargetX = m_aRecMove[m_FNN_CurrentMoveIndex];
					m_FNN_CurrentMoveIndex++;
					m_LatestInput.m_TargetY = m_aRecMove[m_FNN_CurrentMoveIndex];
					m_FNN_CurrentMoveIndex++;

					if (m_FNN_CurrentMoveIndex > m_FNN_ticks_loaded_run)
					{
						m_pPlayer->m_dmm25 = -1; //stop bot
						float newest_distance = distance(m_StartPos, m_Core.m_Pos);
						vec2 current_pos(0, 0);
						current_pos.x = m_Core.m_Pos.x / 32;
						current_pos.y = m_Core.m_Pos.y / 32;
						float newest_distance_finish = distance(current_pos, GameServer()->m_FinishTilePos);
						float newest_fitness = newest_distance_finish / m_FNN_CurrentMoveIndex;
						dbg_msg("FNN", "distance=%.2f", newest_distance);
						dbg_msg("FNN", "moveticks=%d", m_FNN_CurrentMoveIndex);
						dbg_msg("FNN", "fitness=%.2f", newest_fitness);
						dbg_msg("FNN", "distance_finish=%.2f", newest_distance_finish);
						str_format(aBuf, sizeof(aBuf), "[FNN] finished replay ticks=%d distance=%.2f fitness=%.2f distance_finish=%.2f", m_FNN_CurrentMoveIndex, newest_distance, newest_fitness, newest_distance_finish);
						GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, aBuf);
					}
				}
				else if (m_pPlayer->m_dmm25 == 3) //submode[3] read/load fitness
				{

				}
				else if (m_pPlayer->m_dmm25 == 4) //submode[4] play fitness
				{

				}
			}
		}
		else if (m_pPlayer->m_DummyMode == 27 ) //BlmapChill police
		{
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			//selfkills
			if (m_FreezeTime)
			{
				if (m_Core.m_Pos.y < 338 * 32) //upper map area all over the bottom floor and pvp arena
				{
					if (Server()->Tick() % 20 == 0)
					{
						if (m_Core.m_Vel.y == 0.000f && m_Core.m_Vel.x < 0.02f && m_Core.m_Vel.x > -0.02f)
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}
					}
				}
				else if (m_Core.m_Pos.y < 376 * 32) //pvp arena and jail entry
				{
					if (Server()->Tick() % 300 == 0)
					{
						if (IsGrounded())
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}
					}
				}
			}
			if (m_Core.m_Pos.x > 448 * 32 && m_Core.m_Pos.x < 480 * 32 && m_Core.m_Pos.y > 60 * 32 && m_Core.m_Pos.y < 85 * 32) //the whole spawn area
			{
				if (m_pPlayer->m_DummyModeSpawn == 32)
				{
					m_pPlayer->m_DummyMode = 32;
				}
				m_Input.m_Direction = -1;
				if (((m_Core.m_Vel.y > 0.01f) || //jump when falling down
					(m_Core.m_Vel.x > -0.1f && IsGrounded() && Server()->Tick() % 26 == 0)) && //getting stuck
					!(m_Core.m_Pos.x < 469 * 32)) //but not if too far left (at spawn spawn and before jump <--)
				{
					m_Input.m_Jump = 1;
				}
				if (m_Core.m_Pos.y < 75 * 32) //upper area above spawn
				{
					//aim on floor to get speed
					m_Input.m_TargetX = -90;
					m_Input.m_TargetY = 100;

					if (m_Core.m_Pos.x < 459 * 32) //jump through the freeze to fall of down the left side 
					{
						m_Input.m_Jump = 1;
					}
					if (m_Core.m_Pos.x < 458 * 32 && m_Core.m_Pos.y < 71 * 32 - 10) //last boost hook before jump
					{
						m_Input.m_Hook = 1;
					}
				}

				m_Dummy27_loved_x = 0; //clean brain at spawn
				m_Dummy27_speed = 70;
			}

			/*
			//bad code hooking from the left side
			if (m_Core.m_Pos.x > 451 * 32 && m_Core.m_Pos.x < 473 * 32 + 10 && m_Core.m_Pos.y > 77 * 32 && m_Core.m_Pos.y < 85 * 32) //spawn cave
			{
				//prepare for the up swing
				m_Input.m_TargetX = 86;
				m_Input.m_TargetY = -200;

				m_Input.m_Direction = 1; //walk --> at spawn
				if (m_Core.m_Pos.x > 468 * 32 && IsGrounded()) //jump if far enough -->
				{
					m_Input.m_Jump = 1;
				}

				if (m_Core.m_Pos.x > 469 * 32 && m_Core.m_Pos.y > 80 * 32 + 10) //far enough --> swing hook up
				{
					m_Input.m_Hook = 1;
				}
			}
			*/

			//hook from the right side
			if (m_Core.m_Pos.x > 451 * 32 && m_Core.m_Pos.x < 500 * 32 && m_Core.m_Pos.y > 77 * 32 && m_Core.m_Pos.y < 85 * 32) //spawn cave + a lot space to the left (until the entry to the other cave)
			{
				//prepare for the up swing
				m_Input.m_TargetX = -90;
				m_Input.m_TargetY = -170;
				if (m_Core.m_Pos.x > 474 * 32)
				{
					m_Input.m_TargetX = -90;
					m_Input.m_TargetY = -140;
				}

				m_Input.m_Direction = 1; //walk --> at spawn
				if (m_Core.m_Pos.x > 468 * 32 && IsGrounded()) //jump if far enough -->
				{
					m_Input.m_Jump = 1;
				}

				if (m_Core.m_Pos.x > 472 * 32 && m_Core.m_Pos.y > 80 * 32 + 10) //far enough --> swing hook up
				{
					m_Input.m_Hook = 1;
					if (Server()->Tick() % 20 == 0)
					{
						m_Input.m_Hook = 0;
					}
				}
				if (m_Core.m_Pos.x > 476 * 32) //left the spawn and too far -->
				{
					m_Input.m_Direction = -1;
				}
			}

			if (m_Core.m_Pos.y < 338 * 32 && m_Core.m_Pos.y > 90 * 32) //falling down area
			{
				if (m_Core.m_Pos.x > 406 * 32)
				{
					m_Input.m_Direction = -1;
				}
				else
				{
					m_Input.m_Direction = 1;
				}

				if (m_Core.m_Vel.y > 0.04f && m_Core.m_Vel.y < 16.6f && m_Core.m_Pos.x < 420 * 32) //slowly falling (sliding of platforms)
				{
					m_Input.m_Jump = 1;
				}

				if (!m_Dummy27_loved_x) //think of a destination while falling
				{
					if (rand() % 2 == 0)
					{
						m_Dummy27_loved_x = 420 * 32;
						m_Dummy27_loved_y = 430 * 32;
					}
					else
					{
						m_Dummy27_loved_x = 394 * 32;
						m_Dummy27_loved_y = 430 * 32;
					}
				}
			}
			else if (m_Core.m_Pos.y > 338 * 32 && m_Core.m_Pos.y < 380 * 32) //entering the police hole area (pvp arena)
			{
				if (!m_Dummy27_IsReadyToEnterPolice)
				{
					m_Input.m_Direction = -1;
				}
				else
				{
					m_Input.m_Direction = 1;
				}

				if (m_Core.m_Pos.x < 395 * 32)
				{
					m_Dummy27_IsReadyToEnterPolice = true;
				}

				if (m_Core.m_Pos.x > 402 * 32 && IsGrounded()) //jump into the tunnel -->
				{
					m_Input.m_Jump = 1;
				}
				if (m_Core.m_Pos.x > 402 * 32 && m_Core.m_Pos.y > 371 * 32)
				{
					m_Input.m_Jump = 1;
				}
			}
			else if (m_Core.m_Pos.x > 380 * 32 && m_Core.m_Pos.x < 450 * 32 && m_Core.m_Pos.y < 450 * 32 && m_Core.m_Pos.y > 380 * 32) //police area // 27
			{
				if (m_Core.m_Pos.x < 397 * 32 && m_Core.m_Pos.y > 436 * 32 && m_Core.m_Pos.x > 388 * 32) // on the money tile jump loop, to prevent blocking flappy there
				{
					m_Input.m_Jump = 0;
					if (Server()->Tick() % 20 == 0)
					{
						m_Input.m_Jump = 1;
					}
				}
				//detect lower panic (accedentally fall into the lower police base 
				if (!m_Dummy27_lower_panic && m_Core.m_Pos.y > 437 * 32 && m_Core.m_Pos.y > m_Dummy27_loved_y)
				{
					m_Dummy27_lower_panic = 1;
					GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9); //angry emote
				}

				if (m_Dummy27_lower_panic)
				{
					//Check for end panic
					if (m_Core.m_Pos.y < 434 * 32)
					{
						if (IsGrounded())
						{
							m_Dummy27_lower_panic = 0; //made it up yay
						}
					}

					if (m_Dummy27_lower_panic == 1)//position to jump on stairs
					{
						if (m_Core.m_Pos.x < 400 * 32)
						{
							m_Input.m_Direction = 1;
						}
						else if (m_Core.m_Pos.x > 401 * 32)
						{
							m_Input.m_Direction = -1;
						}
						else
						{
							m_Dummy27_lower_panic = 2;
						}
					}
					else if (m_Dummy27_lower_panic == 2) //jump on the left starblock element
					{
						if (IsGrounded())
						{
							m_Input.m_Jump = 1;
							if (Server()->Tick() % 20 == 0)
							{
								m_Input.m_Jump = 0;
							}
						}

						//navigate to platform
						if (m_Core.m_Pos.y < 435 * 32 - 10)
						{
							m_Input.m_Direction = -1;
							if (m_Core.m_Pos.y < 433 * 32)
							{
								if (m_Core.m_Vel.y > 0.01f && m_DummyUsedDJ == false)
								{
									m_Input.m_Jump = 1; //double jump
									if (!IsGrounded()) // this dummyuseddj is for only using default 2 jumps even if 5 jump is on
									{
										m_DummyUsedDJ = true;
									}
								}
							}
							if (m_DummyUsedDJ == true && IsGrounded())
							{
								m_DummyUsedDJ = false;
							}
						}
			
						else if (m_Core.m_Pos.y < 438 * 32) //only if high enough focus on the first lower platform
						{
							if (m_Core.m_Pos.x < 403 * 32)
							{
								m_Input.m_Direction = 1;
							}
							else if (m_Core.m_Pos.x > 404 * 32 + 20)
							{
								m_Input.m_Direction = -1;
							}
						}

						if ((m_Core.m_Pos.y > 441 * 32 + 10 && (m_Core.m_Pos.x > 402 * 32 || m_Core.m_Pos.x < 399 * 32 + 10)) || isFreezed) //check for fail position
						{
							m_Dummy27_lower_panic = 1; //lower panic mode to reposition
						}
					}
				}
				else //no dummy lower panic
				{
					m_Dummy27_help_mode = 0;
					//check if officer needs help
					CCharacter *pChr = GameServer()->m_World.ClosestCharTypePoliceFreezeHole(m_Pos, true, this);
					if (pChr && pChr->IsAlive())
					{
						if (m_Core.m_Pos.y > 435 * 32) // setting the destination of dummy to top left police entry bcs otherwise bot fails when trying to help --> walks into jail wall xd
						{
							m_Dummy27_loved_x = (392 + rand() % 2) * 32;
							m_Dummy27_loved_y = 430 * 32;
						}
						//aimbot on heuzeueu
						m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
						m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
						m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
						m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

						m_Dummy_ClosestPolice = false;
						//If a policerank escapes from jail he is treated like a non police
						if ((pChr->m_pPlayer->m_PoliceRank > 0 && pChr->m_pPlayer->m_EscapeTime == 0) || (pChr->m_pPlayer->m_PoliceHelper && pChr->m_pPlayer->m_EscapeTime == 0))
						{
							m_Dummy_ClosestPolice = true;
						}

						if (pChr->m_Pos.x > 444 * 32 - 10) //police dude failed too far --> to be reached by hook (set too help mode extream to leave save area)
						{
							m_Dummy27_help_mode = 2;
							if (m_Core.m_Jumped > 1 && m_Core.m_Pos.x > 431 * 32) //double jumped and above the freeze
							{
								m_Input.m_Direction = -1;
							}
							else
							{
								m_Input.m_Direction = 1;
							}
							//doublejump before falling in freeze
							if ((m_Core.m_Pos.x > 432 * 32 && m_Core.m_Pos.y > 432 * 32) || m_Core.m_Pos.x > 437 * 32) //over the freeze and too low
							{
								m_Input.m_Jump = 1;
								m_Dummy27_help_hook = true;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "catch hook jump!");
							}
							if (IsGrounded() && m_Core.m_Pos.x > 430 * 32 && Server()->Tick() % 60 == 0)
							{
								m_Input.m_Jump = 1;
								//if (Server()->Tick() % 60 == 0)
								//{
								//	m_Input.m_Jump = 0;
								//}
							}
						}
						else
						{
							m_Dummy27_help_mode = 1;
						}


						if (m_Dummy27_help_mode == 1 && m_Core.m_Pos.x > 431 * 32 + 10)
						{
							m_Input.m_Direction = -1;
						}
						else if (m_Dummy27_help_mode == 1 && m_Core.m_Pos.x < 430 * 32)
						{
							m_Input.m_Direction = 1;
						}
						else
						{
							if (!m_Dummy27_help_hook && m_Dummy_ClosestPolice)
							{
								if (m_Dummy27_help_mode == 2) //police dude failed too far --> to be reached by hook
								{
									//if (m_Core.m_Pos.x > 435 * 32) //moved too double jump
									//{
									//	m_Dummy27_help_hook = true;
									//}
								}
								else if (pChr->m_Pos.x > 439 * 32) //police dude in the middle
								{
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "middle");
									if (IsGrounded())
									{
										m_Dummy27_help_hook = true;
										m_Input.m_Jump = 1;
										m_Input.m_Hook = 1;
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "HOOOK");
									}
								}
								else //police dude failed too near to hook from ground
								{
									if (m_Core.m_Vel.y < -4.20f && m_Core.m_Pos.y < 431 * 32)
									{
										m_Dummy27_help_hook = true;
										m_Input.m_Jump = 1;
										m_Input.m_Hook = 1;
									}
								}
							}
							if (Server()->Tick() % 8 == 0)
							{
								m_Input.m_Direction = 1;
							}
						}

						if (m_Dummy27_help_hook)
						{
							m_Input.m_Hook = 1;
							if (Server()->Tick() % 200 == 0)
							{
								m_Dummy27_help_hook = false; //timeout hook maybe failed
								m_Input.m_Hook = 0;
								m_Input.m_Direction = 1;
							}
						}

						//dont wait on ground
						if (IsGrounded() && Server()->Tick() % 900 == 0)
						{
							m_Input.m_Jump = 1;
						}
						//backup reset jump
						if (Server()->Tick() % 1337 == 0)
						{
							m_Input.m_Jump = 0;
						}
					}


					if (!m_Dummy27_help_mode)
					{
						//==============
						//NOTHING TO DO
						//==============
						//basic walk to destination
						if (m_Core.m_Pos.x < m_Dummy27_loved_x - 32)
						{
							m_Input.m_Direction = 1;
						}
						else if (m_Core.m_Pos.x > m_Dummy27_loved_x + 32 && m_Core.m_Pos.x > 384 * 32)
						{
							m_Input.m_Direction = -1;
						}

						//change changing speed
						if (Server()->Tick() % m_Dummy27_speed == 0)
						{
							if (rand() % 2 == 0)
							{
								m_Dummy27_speed = rand() % 10000 + 420;
							}
						}

						//choose beloved destination
						if (Server()->Tick() % m_Dummy27_speed == 0)
						{
							if ((rand() % 2) == 0)
							{
								if ((rand() % 3) == 0)
								{
									m_Dummy27_loved_x = 420 * 32 + rand() % 69;
									m_Dummy27_loved_y = 430 * 32;
									GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
								}
								else
								{
									m_Dummy27_loved_x = (392 + rand() % 2) * 32;
									m_Dummy27_loved_y = 430 * 32;
								}
								if ((rand() % 2) == 0)
								{
									m_Dummy27_loved_x = 384 * 32 + rand() % 128;
									m_Dummy27_loved_y = 430 * 32;
									GameServer()->SendEmoticon(m_pPlayer->GetCID(), 5);
								}
								else
								{
									if (rand() % 3 == 0)
									{
										m_Dummy27_loved_x = 420 * 32 + rand() % 128;
										m_Dummy27_loved_y = 430 * 32;
										GameServer()->SendEmoticon(m_pPlayer->GetCID(), 8);
									}
									else if (rand() % 4 == 0)
									{
										m_Dummy27_loved_x = 429 * 32 + rand() % 64;
										m_Dummy27_loved_y = 430 * 32;
										GameServer()->SendEmoticon(m_pPlayer->GetCID(), 8);
									}
								}
								if (rand() % 5 == 0) //lower middel base
								{
									m_Dummy27_loved_x = 410 * 32 + rand() % 64;
									m_Dummy27_loved_y = 443 * 32;
								}
							}
							else
							{
								GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
							}
						}
					}
				}

				//=====================
				// all importat backups
				// all dodges and moves
				// which are needed all
				// the time
				// police base only
				//=====================


				//dont walk into the lower police base entry freeze
				if (m_Core.m_Pos.x > 425 * 32 && m_Core.m_Pos.x < 429 * 32) //right side
				{
					if (m_Core.m_Vel.x < -0.02f && IsGrounded())
					{
						m_Input.m_Jump = 1;
					}
				}
				else if (m_Core.m_Pos.x > 389 * 32 && m_Core.m_Pos.x < 391 * 32) //left side
				{
					if (m_Core.m_Vel.x > 0.02f && IsGrounded())
					{
						m_Input.m_Jump = 1;
					}
				}

				//jump over the police underground from entry to enty
				if (m_Core.m_Pos.y > m_Dummy27_loved_y) //only if beloved place is an upper one
				{
					if (m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.x < 418 * 32) //right side
					{
						if (m_Core.m_Vel.x < -0.02f && IsGrounded())
						{
							m_Input.m_Jump = 1;
							if (Server()->Tick() % 5 == 0)
							{
								m_Input.m_Jump = 0;
							}
						}
					}
					else if (m_Core.m_Pos.x > 398 * 32 && m_Core.m_Pos.x < 401 * 32) //left side
					{
						if (m_Core.m_Vel.x > 0.02f && IsGrounded())
						{
							m_Input.m_Jump = 1;
							if (Server()->Tick() % 5 == 0)
							{
								m_Input.m_Jump = 0;
							}
						}
					}

					//do the doublejump
					if (m_Core.m_Vel.y > 6.9f && m_Core.m_Pos.y > 430 * 32 && m_Core.m_Pos.x < 433 * 32 && m_DummyUsedDJ == false) //falling and not too high to hit roof with head
					{
						m_Input.m_Jump = 1;
						//m_LatestInput.m_Fire++;
						//m_Input.m_Fire++;
						if (!IsGrounded()) // this dummyuseddj is for only using default 2 jumps even if 5 jump is on
						{
							m_DummyUsedDJ = true;
						}
					}
					if (m_DummyUsedDJ == true && IsGrounded())
					{
						m_DummyUsedDJ = false;
					}
				}
			}
			if (m_Core.m_Pos.y > 380 * 32 && m_Core.m_Pos.x < 363 * 32) // walking right again to get into the tunnel at the bottom
			{
				m_Input.m_Direction = 1;
				if (IsGrounded())
				{
					m_Input.m_Jump = 1;
				}
			}
			if (m_Core.m_Pos.y > 380 * 32 && m_Core.m_Pos.x < 381 * 32 && m_Core.m_Pos.x > 363 * 32)
			{
				m_Input.m_Direction = 1;
				if (m_Core.m_Pos.x > 367 * 32 && m_Core.m_Pos.x < 368 * 32 && IsGrounded())
				{
					m_Input.m_Jump = 1;
				}
				if (m_Core.m_Pos.y > 433.7 * 32)
				{
					m_Input.m_Jump = 1;
				}
			}
			if (m_pPlayer->m_DummyModeSpawn == 32)
			{
				if (m_Core.m_Pos.x > 290 * 32 && m_Core.m_Pos.x < 450 * 32 && m_Core.m_Pos.y > 415 * 32 && m_Core.m_Pos.y < 450 * 32)
				{
					if (isFreezed) // kills when in freeze in policebase or left of it (takes longer that he kills bcs the way is so long he wait a bit longer for help)
					{
						if (Server()->Tick() % 60 == 0)
						{
							GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3); // tear emote before killing
						}
						if (Server()->Tick() % 3000 == 0 && IsGrounded()) // kill when freeze
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}
					}
				}
			}
			else //unknown area //it isnt it is spawn area and stuff not a random area
			{
				////walk towards police station
				//if (m_Core.m_Pos.x < 400 * 32)
				//{
				//	m_Input.m_Direction = 1;
				//}
				//else
				//{
				//	m_Input.m_Direction = -1;
				//}


				//if (Server()->Tick() % 420 == 0)
				//{
				//	m_Input.m_Jump = 1;
				//	//ask public chat for help
				//	if (rand() % 2 == 0)
				//	{
				//		int rand_message = rand() % 10;
				//		if (rand_message == 0)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "where am i?");
				//		}
				//		else if (rand_message == 1)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hello is there anybody?");
				//		}
				//		else if (rand_message == 2)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "guys! i need help!!!11");
				//		}
				//		else if (rand_message == 3)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "I think i am lost.");
				//		}
				//		else if (rand_message == 4)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "officer needs help!");
				//		}
				//		else if (rand_message == 5)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Where is the police base?");
				//		}
				//		else if (rand_message == 6)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Can someone bring me to the police base?");
				//		}
				//		else if (rand_message == 7)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "I AM LOST!");
				//		}
				//		else if (rand_message == 8)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "where the fuck am i?");
				//		}
				//		else if (rand_message == 9)
				//		{
				//			GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hello sir? can you bring me to the police base?");
				//		}
				//	}
				//}

				//if (Server()->Tick() % 2000 == 0 && isFreezed && IsGrounded())
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//}
				//if (Server()->Tick() % 9000 == 0)
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//}
			}


			//reset in freeze ( W A R N I N G!!! Keep this code last in dummymode)
			if (m_FreezeTime)
			{
				m_Input.m_Hook = 0;
				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;
			}
		}
		else if (m_pPlayer->m_DummyMode == 28) // some BlmapChill
		{
			//RestOnChange (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			if (m_Core.m_Pos.x > 451 * 32 && m_Core.m_Pos.x < 472 * 32 && m_Core.m_Pos.y > 74 * 32 && m_Core.m_Pos.y < 85 * 32) //spawn bereich
			{
				m_Input.m_Direction = -1;

				if (m_Core.m_Pos.x > 454 * 32 && m_Core.m_Pos.x < 458 * 32) //linke seite des spawn bereiches
				{
					m_Input.m_Jump = 1;
					if (Server()->Tick() % 10 == 0) //is sone kleine glock die alle was weiss ich Zeiteinheiten true ist
					{
						m_Input.m_Jump = 0; //auch wenn davor Jump = 1 gestezt wurde es zählt immer der letzte also wird er wenn iwi 10 sec oder was auch immer vergangen sind nochmal kurz den jump loslassen
					}
				}
			}
			else if (m_Core.m_Pos.x > 30 * 32 && m_Core.m_Pos.x < 32 * 32 && m_Core.m_Pos.y > 25 * 32 && m_Core.m_Pos.y < 32 * 32) // if right wall on newteespawn go a bit left to not just fall into freeze
			{
				m_Input.m_Direction = -1;
				m_Input.m_Jump = 1;
			}
			else if (m_Core.m_Pos.x > 15 * 32 && m_Core.m_Pos.x < 33 * 32 && m_Core.m_Pos.y > 33 * 32 && m_Core.m_Pos.y < 35 * 32) //newtee spawn
			{
				m_Input.m_Direction = 1;

				if (m_Core.m_Pos.x > 30 * 32 && m_Core.m_Pos.x < 32 * 32 && m_Core.m_Pos.y > 32 * 32 && m_Core.m_Pos.y < 35 * 32) // right side newetee jump through freeze
				{
					m_Input.m_Jump = 1;
				}
			}
			else if (m_Core.m_Pos.x > 20 * 32 && m_Core.m_Pos.x < 105 * 32 && m_Core.m_Pos.y > 30 * 32 && m_Core.m_Pos.y < 44 * 32) // under newtee spawn walk to the right
			{
				m_Input.m_Direction = 1;

				if (m_Core.m_Pos.x > 41 * 32 && m_Core.m_Pos.x < 43 * 32) // jump over frz in ground
				{
					m_Input.m_Jump = 1;
				}
				else if (m_Core.m_Pos.y > 42 * 32 && m_Core.m_Pos.y < 44 * 32 && m_Core.m_Pos.x > 55 * 32 && m_Core.m_Pos.x < 58 * 32) // jump over 2nd freeze
				{
					m_Input.m_Jump = 1;
				}
				else if (m_Core.m_Pos.x > 75 * 32 && m_Core.m_Pos.x < 76 * 32 && m_Core.m_Pos.y > 30 * 32 && m_Core.m_Pos.y < 44 * 32) // jump over the rotating thing
				{
					m_Input.m_Jump = 1;
				}
				else if ((m_Core.m_Vel.y > 0.01) && m_Core.m_Pos.x > 77 * 32 && m_Core.m_Pos.x < 90 * 32) //dj in mid air over the rotating thing
				{
					m_Input.m_Jump = 1;
				}
			}
			else if (m_Core.m_Pos.x > 95 * 32 && m_Core.m_Pos.x < 121 * 32) // already fell down and now going right
			{
				m_Input.m_Direction = 1;
				if (m_Core.m_Pos.x > 110 * 32 && m_Core.m_Pos.x < 115 * 32) // and jump to be ready for wallhammer
				{
					m_Input.m_Jump = 1;
				}
			}
		}
		else if (m_pPlayer->m_DummyMode == 29) //mode 18 (tryhardwayblocker cb5)    with some more human wayblock style not so try hard a cool chillblock5 blocker mode
		{
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0; //this is 29 only the mode 18 had no jump resett hope it works ... it shoudl omg
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "HALLO ICH BIN 29!");

								//Check ob dem bot langweilig geworden is :)


			if (m_Dummy_bored_counter > 2)
			{
				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this);
				if (pChr && pChr->IsAlive())
				{

				}
				else //no ruler alive 
				{
					m_Dummy_lock_bored = true;
				}
			}
			else
			{
				m_Dummy_lock_bored = false;
			}


			if (m_Dummy_lock_bored)
			{
				if (m_Core.m_Pos.x < 429 * 32 && IsGrounded())
				{
					m_Dummy_bored = true;
					//static bool test = false;

					//if (!test)
					//{
					//	GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "bored");
					//	test = true;
					//}
				}
			}


			/*
			####################################################
			#    I M P O R T A N T    I N F O R M A T I O N    #
			####################################################

			DummyMode 29 is a very special mode cause it uses the mode18 as base
			and mode18 is for now the biggest mode ever made so this code is a huge mess
			so mode29 uses a lot of mode18 vars

			Mode18 is a tryhard ruler he wayblocks as good as he can and blocks if somebody manages to get in his area

			Mode29 is a blocker which is not dat tryhard he doesnt wayblock and does more random stuff and trys freezeblock tricks







			BRAND NEW STRUCTURE!
			WELCOME TO 18's SPECIAL MODE-CEPTION!

			dummymode 18 hase his own modes in the mode lol


			:::::::::::::::::::::
			dummymode29 modes
			:::::::::::::::::::::
			mode:         desc:
			0					Main mode
			1					attack mode (if ruler spot is ruled and bot is in tunnel)
			2                   different wayblock mode
			3					special defend mode
			4                   (PLANNED) 1on1 mode with counting in chat and helping




			dummymode29 code structure:
			- Check for activating other modes
			- big if clause with all modes

			*/


			//Check for activating other modes

			//Check mode 1 [Attack from tunnel wayblocker]
			//man könnte das auch mit m_Dummy_happy abfragen aber mich nich ganz so viel sinn
			if (m_Core.m_Pos.y > 214 * 32 && m_Core.m_Pos.x > 424 * 32)
			{
				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerWB(m_Pos, true, this);
				if (pChr && pChr->IsAlive())
				{

					//Wenn der bot im tunnel ist und ein Gegner im RulerWB bereich
					m_Dummy_mode18 = 1;
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Wayblocker gesichtet");

				}
			}
			else if (m_Dummy_bored)
			{
				m_Dummy_mode18 = 2;
			}
			else if (m_Dummy_special_defend) //Check mode 3 [Attack from tunnel wayblocker]
			{
				m_Dummy_mode18 = 3;
			}
			else
			{
				m_Dummy_mode18 = 0; //change to main mode
			}




			//Modes:

			if (m_Dummy_mode18 == 3) //special defend mode
			{
				//testy wenn der dummy in den special defend mode gesetzt wird pusht das sein adrenalin und ihm is nicht mehr lw
				m_Dummy_bored_counter = 0;

				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this);
				if (pChr && pChr->IsAlive())
				{

					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;



					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					//rest on tick
					m_Input.m_Hook = 0;
					m_Input.m_Jump = 0;
					m_Input.m_Direction = 0;
					m_LatestInput.m_Fire = 0;
					m_Input.m_Fire = 0;
					SetWeapon(1); //gun verwenden damit auch erkannt wird wann der mode getriggert wird


					if (pChr->m_FreezeTime == 0)
					{
						//wenn der gegner doch irgendwie unfreeze wird übergib an den main mode und lass den notstand das regeln
						m_Dummy_special_defend = false;
						m_Dummy_special_defend_attack = false;
					}
					//mode18 sub mode 3
					//Main code:
					//warte bis der gegner auf den boden geklatscht ist
					//dann werf ihn rechts runter

					if (pChr->m_Core.m_Vel.y > -0.9f && pChr->m_Pos.y > 211 * 32)
					{
						//wenn der gegner am boden liegt starte angriff
						m_Dummy_special_defend_attack = true;

						//start jump
						m_Input.m_Jump = 1;
					}


					if (m_Dummy_special_defend_attack)
					{
						if (m_Core.m_Pos.x - pChr->m_Pos.x < 50) //wenn der gegner nah genung is mach dj
						{
							m_Input.m_Jump = 1;
						}

						if (pChr->m_Pos.x < m_Core.m_Pos.x)
						{
							m_Input.m_Hook = 1;
						}
						else //wenn der gegner weiter rechts als der bot is lass los und übergib an main deine arbeit ist hier getahen
						{    //main mode wird evenetuell noch korrigieren mit schieben
							m_Dummy_special_defend = false;
							m_Dummy_special_defend_attack = false;
						}

						//Der bot sollte möglichst weit nach rechts gehen aber natürlich nicht ins freeze

						if (m_Core.m_Pos.x < 427 * 32 + 15)
						{
							m_Input.m_Direction = 1;
						}
						else
						{
							m_Input.m_Direction = -1;
						}

					}

				}
				else //wenn kein gegner mehr im Ruler bereich is
				{
					m_Dummy_special_defend = false;
					m_Dummy_special_defend_attack = false;
				}
			}
			else if (m_Dummy_mode18 == 2) //different wayblock mode
			{
				//rest on tick
				m_Input.m_Hook = 0;
				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;
				if (Server()->Tick() % 30 == 0)
				{
					SetWeapon(0);
				}
				//if (Server()->Tick() % 5 == 0)
				//{
				//	GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
				//}

				//Selfkills (bit random but they work)
				if (isFreezed)
				{
					//wenn der bot freeze is warte erstmal n paar sekunden und dann kill dich
					if (Server()->Tick() % 300 == 0)
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
						m_Dummy_happy = false;
					}
				}



				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler2(m_Pos, true, this);
				if (pChr && pChr->IsAlive())
				{
					//Check ob an notstand mode18 = 0 übergeben
					if (pChr->m_FreezeTime == 0)
					{
						m_Dummy_bored = false;
						m_Dummy_bored_counter = 0;
						m_Dummy_mode18 = 0;
					}




					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					m_Input.m_Jump = 1;

					if (pChr->m_Pos.y > m_Core.m_Pos.y && pChr->m_Pos.x > m_Core.m_Pos.x + 20) //solange der bot über dem gegner ist (damit er wenn er ihn weg hammert nicht weiter hookt)
					{
						m_Input.m_Hook = 1;
					}

					if (m_Core.m_Pos.x > 420 * 32)
					{
						m_Input.m_Direction = -1;
					}

					if (pChr->m_Pos.y < m_Core.m_Pos.y + 15)
					{
						m_LatestInput.m_Fire++;
						m_Input.m_Fire++;
					}

				}
				else //lieblings position finden wenn nichts abgeht
				{
					//               old: 421 * 32
					if (m_Core.m_Pos.x < 423 * 32)
					{
						m_Input.m_Direction = 1;
					}
					//                   old: 422 * 32 + 30
					else if (m_Core.m_Pos.x > 424 * 32 + 30)
					{
						m_Input.m_Direction = -1;
					}
				}
			}
			else if (m_Dummy_mode18 == 1) //attack in tunnel
			{
				//Selfkills (bit random but they work)
				if (isFreezed)
				{
					//wenn der bot freeze is warte erstmal n paar sekunden und dann kill dich
					if (Server()->Tick() % 300 == 0)
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}
				}

				//stay on position

				if (m_Core.m_Pos.x < 426 * 32 + 10) // zu weit links
				{
					m_Input.m_Direction = 1; //geh rechts
				}
				else if (m_Core.m_Pos.x > 428 * 32 - 10) //zu weit rechts
				{
					m_Input.m_Direction = -1; // geh links
				}
				else if (m_Core.m_Pos.x > 428 * 32 + 10) // viel zu weit rechts
				{
					m_Input.m_Direction = -1; // geh links
					m_Input.m_Jump = 1;
				}
				else
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerWB(m_Pos, true, this);
					if (pChr && pChr->IsAlive())
					{
						if (pChr->m_Pos.x < 436 * 32) //wenn er ganz weit über dem freeze auf der kante ist (hooke direkt)
						{
							m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
							m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
						}
						else //wenn der Gegner weiter hinter dem unhook ist (hook über den Gegner um ihn trozdem zu treffen und das unhook zu umgehen)
						{
							m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x - 50;
							m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

							m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x - 50;
							m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
						}


						//char aBuf[256];
						//str_format(aBuf, sizeof(aBuf), "targX: %d = %d - %d", m_Input.m_TargetX, pChr->m_Pos.x, m_Pos.x);
						//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);


						//m_Input.m_Hook = 0;
						CCharacter *pChr = GameServer()->m_World.ClosestCharTypeTunnel(m_Pos, true, this);
						if (pChr && pChr->IsAlive())
						{
							//wenn jemand im tunnel is check ob du nicht ausversehen den hookst anstatt des ziels in der WB area
							if (pChr->m_Pos.x < m_Core.m_Pos.x) //hooke nur wenn kein Gegner rechts von dem bot im tunnel is (da er sonst ziemlich wahrscheinlich den hooken würde)
							{
								m_Input.m_Hook = 1;
							}

						}
						else
						{
							//wenn eh keiner im tunnel is hau raus dat ding
							m_Input.m_Hook = 1;
						}

						//schau ob sich der gegner bewegt und der bot grad nicht mehr am angreifen iss dann resette falls er davor halt misshookt hat
						//geht nich -.-
						if (m_Core.m_HookState != HOOK_FLYING && m_Core.m_HookState != HOOK_GRABBED)
						{
							if (Server()->Tick() % 10 == 0)
							{
								m_Input.m_Hook = 0;
							}
						}




						if (m_Core.m_Vel.x > 3.0f)
						{
							m_Input.m_Direction = -1;
						}
						else
						{
							m_Input.m_Direction = 0;
						}

					}
					else
					{
						m_Dummy_mode18 = 0;
					}
				}



			}
			else if (m_Dummy_mode18 == 0) //main mode
			{
				//if (mode18_main_init)
				//{
				//	//initialzing main mode...
				//	//resetting stuff...
				//	m_Input.m_Hook = 0;
				//}

				//m_Input.m_Hook = 0;
				//if (m_Core.m_HookState == HOOK_FLYING)
				//	m_Input.m_Hook = 1;
				//else if (m_Core.m_HookState == HOOK_GRABBED)
				//	m_Input.m_Hook = 1;
				//else
				//	m_Input.m_Hook = 0;

				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;





				//char aBuf[256];
				//str_format(aBuf, sizeof(aBuf), "speed:  x: %f y: %f speed pChr:  x: %f y: %f", m_Core.m_Vel.x, m_Core.m_Vel.y);

				//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);


				if (1 == 2)
				{
					CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
					if (pChr && pChr->IsAlive())
					{
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "speed pChr:  x: %f y: %f", pChr->m_Core.m_Vel.x, pChr->m_Core.m_Vel.y);

						//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
					}
				}



				//m_pPlayer->m_TeeInfos.m_Name = aBuf; 

				if (m_Core.m_Vel.x > 1.0f)
				{
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "speed: schneller als 1");
				}


				//Check ob jemand in der linken freeze wand is

				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRulerLeftFreeze(m_Pos, true, this);  //wenn jemand rechts im freeze liegt
				if (pChr && pChr->IsAlive()) // wenn ein spieler rechts im freeze lebt
				{  //----> versuche im notstand nicht den gegner auch da rein zu hauen da ist ja jetzt voll

					m_Dummy_left_freeze_full = true;
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Da liegt einer im freeze");
				}
				else // wenn da keiner is fülle diesen spot (linke freeze wand im ruler spot)
				{
					m_Dummy_left_freeze_full = false;
				}





				//hardcodet selfkill (moved in lower area only)
				//if (m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.y > 214 * 32)  //Links am spawn runter
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Links am spawn runter");
				//}
				//else if ((m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x > 392 * 32 && m_Core.m_Pos.y > 190) || (m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.y > 190)) //freeze decke am spawn
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke am spawn");
				//}
				//else if (m_Core.m_Pos.y > 218 * 32 + 31 /* für tee balance*/ && m_Core.m_Pos.x < 415 * 32) //freeze boden am spawn
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze boden am spawn");
				//}
				//else if (m_Core.m_Pos.y < 215 * 32 && m_Core.m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.x < 428 * 32) //freeze decke im tunnel
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke im tunnel");
				//}
				//else if (m_Core.m_Pos.y > 222 * 32) //freeze becken unter area
				//{
				//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze becken unter area");
				//}
				//else if (m_Core.m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 436 * 32) //freeze rechts neben freeze becken
				//{
				//	//Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze rechts neben freeze becken");
				//}
				//else if (m_Core.m_Pos.x > 469 * 32) //zu weit ganz rechts in der ruler area
				//{
				//	//Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "zu weit ganz rechts in der ruler area");
				//}
				//else if (m_Core.m_Pos.y > 211 * 32 + 34 && m_Core.m_Pos.x > 455 * 32) //alles freeze am boden rechts in der area
				//{
				//	//Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze boden rechts der area");
				//}


				//if (m_Core.m_Pos.y < 193 * 32 /*&& g_Config.m_SvChillBlock5Version == 1*/) //old spawn of unsued version (this code makes no sense at all)
				//{
				//	m_Input.m_TargetX = 200;
				//	m_Input.m_TargetY = -80;


				//	//not falling in freeze is bad
				//	if (m_Core.m_Vel.y < 0.01f && m_FreezeTime > 0)
				//	{
				//		if (Server()->Tick() % 40 == 0)
				//		{
				//			Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//		}
				//	}
				//	if (m_Core.m_Pos.y > 116 * 32 && m_Core.m_Pos.x > 394 * 32)
				//	{
				//		Die(m_pPlayer->GetCID(), WEAPON_SELF);
				//	}

				//	if (m_Core.m_Pos.x > 364 * 32 && m_Core.m_Pos.y < 126 * 32 && m_Core.m_Pos.y > 122 * 32 + 10)
				//	{
				//		if (m_Core.m_Vel.y > -1.0f)
				//		{
				//			m_Input.m_Hook = 1;
				//		}
				//	}

				//	if (m_Core.m_Pos.y < 121 * 32 && m_Core.m_Pos.x > 369 * 32)
				//	{
				//		m_Input.m_Direction = -1;
				//	}
				//	else
				//	{
				//		m_Input.m_Direction = 1;
				//	}
				//	if (m_Core.m_Pos.y < 109 * 32 && m_Core.m_Pos.x > 377 * 32 && m_Core.m_Pos.x < 386 * 32)
				//	{
				//		m_Input.m_Direction = 1;
				//	}


				//	if (m_Core.m_Pos.y > 128 * 32)
				//	{
				//		m_Input.m_Jump = 1;
				//	}

				//	//speeddown at end to avoid selfkill cuz to slow falling in freeze
				//	if (m_Core.m_Pos.x > 384 * 32 && m_Core.m_Pos.y > 121 * 32)
				//	{
				//		m_Input.m_TargetX = 200;
				//		m_Input.m_TargetY = 300;
				//		m_Input.m_Hook = 1;
				//	}
				//}
				//else //under 193 (above 193 is new spawn)

				if (m_Core.m_Pos.x > 241 * 32 && m_Core.m_Pos.x < 418 * 32 && m_Core.m_Pos.y > 121 * 32 && m_Core.m_Pos.y < 192 * 32) //new spawn ChillBlock5 (tourna edition (the on with the gores stuff))
				{
					//dieser code wird also nur ausgeführt wenn der bot gerade im neuen bereich ist
					if (m_Core.m_Pos.x > 319 * 32 && m_Core.m_Pos.y < 161 * 32) //top right spawn
					{
						//look up left
						if (m_Core.m_Pos.x < 372 * 32 && m_Core.m_Vel.y > 3.1f)
						{
							m_Input.m_TargetX = -30;
							m_Input.m_TargetY = -80;
						}
						else
						{
							m_Input.m_TargetX = -100;
							m_Input.m_TargetY = -80;
						}

						if (m_Core.m_Pos.x > 331 * 32 && isFreezed)
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}

						if (m_Core.m_Pos.x < 327 * 32) //dont klatsch in ze wand
						{
							m_Input.m_Direction = 1; //nach rechts laufen
						}
						else
						{
							m_Input.m_Direction = -1;
						}

						if (IsGrounded() && m_Core.m_Pos.x < 408 * 32) //initial jump from spawnplatform
						{
							m_Input.m_Jump = 1;
						}

						if (m_Core.m_Pos.x > 330 * 32) //only hook in tunnel and let fall at the end
						{
							if (m_Core.m_Pos.y > 147 * 32 || (m_Core.m_Pos.y > 143 * 32 && m_Core.m_Vel.y > 3.3f)) //gores pro hook up
							{
								m_Input.m_Hook = 1;
							}
							else if (m_Core.m_Pos.y < 143 * 32 && m_Core.m_Pos.x < 372 * 32) //hook down (if too high and in tunnel)
							{
								m_Input.m_TargetX = -42;
								m_Input.m_TargetY = 100;
								m_Input.m_Hook = 1;
							}
						}
					}
					else if (m_Core.m_Pos.x < 317 * 32) //top left spawn
					{
						if (m_Core.m_Pos.y < 158 * 32) //spawn area find down
						{
							//selfkill
							if (isFreezed)
							{
								Die(m_pPlayer->GetCID(), WEAPON_SELF);
							}

							if (m_Core.m_Pos.x < 276 * 32 + 20) //is die mitte von beiden linken spawns also da wo es runter geht
							{
								m_Input.m_TargetX = 57;
								m_Input.m_TargetY = -100;
								m_Input.m_Direction = 1;
							}
							else
							{
								m_Input.m_TargetX = -57;
								m_Input.m_TargetY = -100;
								m_Input.m_Direction = -1;
							}

							if (m_Core.m_Pos.y > 147 * 32)
							{
								//dbg_msg("fok","will hooken");
								m_Input.m_Hook = 1;
								if (m_Core.m_Pos.x > 272 * 32 && m_Core.m_Pos.x < 279 * 32) //let fall in the hole
								{
									//dbg_msg("fok", "lass ma des");
									m_Input.m_Hook = 0;
									m_Input.m_TargetX = 2;
									m_Input.m_TargetY = 100;
								}
							}
						}
						else if (m_Core.m_Pos.y > 162 * 32) //managed it to go down --> go left
						{
							//selfkill
							if (isFreezed)
							{
								Die(m_pPlayer->GetCID(), WEAPON_SELF);
							}

							if (m_Core.m_Pos.x < 283 * 32)
							{
								m_Input.m_TargetX = 200;
								m_Input.m_TargetY = -136;
								if (m_Core.m_Pos.y > 164 * 32 + 10)
								{
									m_Input.m_Hook = 1;
								}
							}
							else
							{
								m_Input.m_TargetX = 80;
								m_Input.m_TargetY = -100;
								if (m_Core.m_Pos.y > 171 * 32 - 10)
								{
									m_Input.m_Hook = 1;
								}
							}

							m_Input.m_Direction = 1;
						}
						else //freeze unfreeze bridge only 2 tiles do nothing here
						{

						}
					}
					else //lower end area of new spawn --> entering old spawn by walling and walking right
					{
						m_Input.m_Direction = 1;
						m_Input.m_TargetX = 200;
						m_Input.m_TargetY = -84;

						//Selfkills
						if (isFreezed && IsGrounded()) //should never lie in freeze at the ground
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
						}


						if (m_Core.m_Pos.y < 166 * 32 - 20)
						{
							m_Input.m_Hook = 1;
						}

						if (m_Core.m_Pos.x > 332 * 32 && m_Core.m_Pos.x < 337 * 32 && m_Core.m_Pos.y > 182 * 32) //wont hit the wall --> jump
						{
							m_Input.m_Jump = 1;
						}

						if (m_Core.m_Pos.x > 336 * 32 + 20 && m_Core.m_Pos.y > 180 * 32) //stop moving if walled
						{
							m_Input.m_Direction = 0;
						}
					}
				}
				else
				{

					if (m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.x > 325 * 32 && m_Core.m_Pos.y > 215 * 32)  //Links am spawn runter
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Links am spawn runter");
					}
					//else if ((m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x > 392 * 32 && m_Core.m_Pos.y > 190) || (m_Core.m_Pos.y < 204 * 32 && m_Core.m_Pos.x < 415 * 32 && m_Core.m_Pos.x < 390 * 32 && m_Core.m_Pos.y > 190)) //freeze decke am old spawn
					//{
					//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke am old spawn");
					//}
					//else if (m_Core.m_Pos.y > 218 * 32 + 31 /* für tee balance*/ && m_Core.m_Pos.x < 415 * 32) //freeze boden am spawn
					//{
					//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
					//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze boden am spawn");
					//}
					else if (m_Core.m_Pos.y < 215 * 32 && m_Core.m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.x < 428 * 32) //freeze decke im tunnel
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze decke im tunnel");
					}
					else if (m_Core.m_Pos.y > 222 * 32) //freeze becken unter area
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze becken unter area");
					}
					else if (m_Core.m_Pos.y > 213 * 32 && m_Core.m_Pos.x > 436 * 32) //freeze rechts neben freeze becken
					{
						//Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze rechts neben freeze becken");
					}
					else if (m_Core.m_Pos.x > 469 * 32) //zu weit ganz rechts in der ruler area
					{
						//Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "zu weit ganz rechts in der ruler area");
					}
					else if (m_Core.m_Pos.y > 211 * 32 + 34 && m_Core.m_Pos.x > 455 * 32) //alles freeze am boden rechts in der area
					{
						//Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze boden rechts der area");
					}

					if (m_Core.m_Pos.y < 220 * 32 && m_Core.m_Pos.x < 415 * 32 && m_FreezeTime > 1 && m_Core.m_Pos.x > 352 * 32) //always suicide on freeze if not reached teh block area yet AND dont suicide in spawn area because new spawn sys can get pretty freezy
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "freeze und links der block area");
					}


					// Movement
					/*
					NEW MOVEMENT TO BLOCK AREA STRUCTURE :)

					After spawning the bot thinks about what way he shoudl choose.
					After he found one he stopps thinking until he respawns agian.

					if he thinks the tunnel is shit he goes trough the window

					*/

					//new spawn do something agianst hookers 
					if (m_Core.m_Pos.x < 380 * 32 && m_Core.m_Pos.x > 322 * 32 && m_Core.m_Vel.x < -0.001f)
					{
						m_Input.m_Hook = 1;
						if ((m_Core.m_Pos.x < 362 * 32 && IsGrounded()) || m_Core.m_Pos.x < 350 * 32)
						{
							if (Server()->Tick() % 10 == 0)
							{
								m_Input.m_Jump = 1;
								SetWeapon(0);
							}
						}
					}
					if (m_Core.m_Pos.x < 415 * 32)
					{
						CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this); //hammer player up in freeze if in right pos
						if (pChr && pChr->IsAlive())
						{
							if (pChr->m_Core.m_Pos.x > m_Core.m_Pos.x - 100 && pChr->m_Core.m_Pos.x < m_Core.m_Pos.x + 100 && pChr->m_Core.m_Pos.y > m_Core.m_Pos.y - 100 && pChr->m_Core.m_Pos.y < m_Core.m_Pos.y + 100)
							{
								if (pChr->m_Core.m_Vel.y < -1.5f) //only boost and use existing up speed
								{
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
								}
								if (Server()->Tick() % 3 == 0)
								{
									SetWeapon(0);
								}
							}
							//old spawn do something agianst way blockers (roof protection)
							if (m_Core.m_Pos.y > 206 * 32 + 4 && m_Core.m_Pos.y < 208 * 32 && m_Core.m_Vel.y < -4.4f)
							{
								m_Input.m_Jump = 1;
							}
							//old spawn roof protection / attack hook
							if (pChr->m_Core.m_Pos.y > m_Core.m_Pos.y + 50)
							{
								m_Input.m_Hook = 1;
							}
						}
					}


					if (m_Core.m_Pos.x < 388 * 32 && m_Core.m_Pos.y > 213 * 32) //jump to old spawn
					{
						m_Input.m_Jump = 1;
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
						m_Input.m_Hook = 1;
						m_Input.m_TargetX = -200;
						m_Input.m_TargetY = 0;
					}

					if (!m_Dummy_planned_movment)
					{
						CCharacter *pChr = GameServer()->m_World.ClosestCharTypeTunnel(m_Pos, true, this);
						if (pChr && pChr->IsAlive())
						{
							if (pChr->m_Core.m_Vel.x < 3.3f) //found a slow bob in tunnel
							{
								m_Dummy_movement_to_block_area_style_window = true;
							}
						}

						m_Dummy_planned_movment = true;
					}



					if (m_Dummy_movement_to_block_area_style_window)
					{
						if (m_Core.m_Pos.x < 415 * 32)
						{
							m_Input.m_Direction = 1;

							if (m_Core.m_Pos.x > 404 * 32 && IsGrounded())
							{
								m_Input.m_Jump = 1;
							}
							if (m_Core.m_Pos.y < 208 * 32)
							{
								m_Input.m_Jump = 1;
							}

							if (m_Core.m_Pos.x > 410 * 32)
							{
								m_Input.m_TargetX = 200;
								m_Input.m_TargetY = 70;
								if (m_Core.m_Pos.x > 413 * 32)
								{
									m_Input.m_Hook = 1;
								}
							}
						}
						else //not needed but safty xD when the bot managed it to get into the ruler area change to old movement
						{
							m_Dummy_movement_to_block_area_style_window = false;
						}


						//something went wrong:
						if (m_Core.m_Pos.y > 214 * 32)
						{
							m_Input.m_Jump = 1;
							m_Dummy_movement_to_block_area_style_window = false;
						}

					}
					else //down way
					{

						//CheckFatsOnSpawn

						if (m_Core.m_Pos.x < 406 * 32)
						{
							CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
							if (pChr && pChr->IsAlive())
							{

								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;


								if (pChr->m_Pos.x < 407 * 32 && pChr->m_Pos.y > 212 * 32 && pChr->m_Pos.y < 215 * 32 && pChr->m_Pos.x > m_Core.m_Pos.x) //wenn ein im weg stehender tee auf der spawn plattform gefunden wurde
								{
									SetWeapon(0); //hol den hammer raus!
									if (pChr->m_Pos.x - m_Core.m_Pos.x < 30) //wenn der typ nahe bei dem bot ist
									{
										if (m_FreezeTick == 0) //nicht rum schrein
										{
											m_LatestInput.m_Fire++;
											m_Input.m_Fire++;
										}


										if (Server()->Tick() % 10 == 0)
										{
											GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9); //angry
										}
									}
								}
								else
								{
									if (Server()->Tick() % 20 == 0)
									{
										SetWeapon(1); //pack den hammer weg
									}
								}
							}
						}


						//Check attacked on spawn
						if (m_Core.m_Pos.x < 412 * 32 && m_Core.m_Pos.y > 217 * 32 && m_Core.m_Vel.x < -0.5f)
						{
							m_Input.m_Jump = 1;
							m_Dummy_AttackedOnSpawn = true;
						}
						if (IsGrounded())
						{
							m_Dummy_AttackedOnSpawn = false;
						}
						if (m_Dummy_AttackedOnSpawn)
						{
							if (Server()->Tick() % 100 == 0) //this shitty stuff can set it right after activation to false but i dont care
							{
								m_Dummy_AttackedOnSpawn = false;
							}
						}


						if (m_Dummy_AttackedOnSpawn)
						{
							int r = rand() % 88; // #noRACISMIM   hitler was fggt    but just because he claimed this number i wont stop using it fuck him and his claims i dont care about him i use this number as my number. It is a statement agianst his usage! we have to fight!

							if (r > 44)
							{
								m_Input.m_Fire++;
							}

							int duNIPPEL = rand() % 1337;
							if (duNIPPEL > 420)
							{
								SetWeapon(0);
							}

							CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
							if (pChr && pChr->IsAlive())
							{
								int r = rand() % 10 - 10;

								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y - r;

								if (Server()->Tick() % 13 == 0)
								{
									GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9);
								}

								if (m_Core.m_HookState == HOOK_GRABBED || (m_Core.m_Pos.y < 216 * 32 && pChr->m_Pos.x > 404 * 32) || (pChr->m_Pos.x > 405 * 32 && m_Core.m_Pos.x > 404 * 32 + 20))
								{
									m_Input.m_Hook = 1;
									if (Server()->Tick() % 10 == 0)
									{
										int x = rand() % 20;
										int y = rand() % 20 - 10;
										m_Input.m_TargetX = x;
										m_Input.m_TargetY = y;
									}
								}

							}
						}



						//CheckSlowDudesInTunnel

						if (m_Core.m_Pos.x > 415 * 32 && m_Core.m_Pos.y > 214 * 32) //wenn bot im tunnel ist
						{
							CCharacter *pChr = GameServer()->m_World.ClosestCharTypeTunnel(m_Pos, true, this);
							if (pChr && pChr->IsAlive())
							{
								if (pChr->m_Core.m_Vel.x < 7.8f) //wenn der nächste spieler im tunnel ein slowdude is 
								{
									//HauDenBau
									SetWeapon(0); //hol den hammer raus!

									m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
									m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;

									m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
									m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

									if (m_FreezeTick == 0) //nicht rum schrein
									{
										m_LatestInput.m_Fire++;
										m_Input.m_Fire++;
									}

									if (Server()->Tick() % 10 == 0)  //angry emotes machen
									{
										GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9);
									}


								}
							}
						}


						//CheckSpeedInTunnel
						if (m_Core.m_Pos.x > 425 * 32 && m_Core.m_Pos.y > 214 * 32 && m_Core.m_Vel.x < 9.4f) //wenn nich genung speed zum wb spot springen
						{
							m_Dummy_get_speed = true;
						}


						if (m_Dummy_get_speed) //wenn schwung holen == true (tunnel)
						{
							if (m_Core.m_Pos.x > 422 * 32) //zu weit rechts
							{
								//---> hol schwung für den jump
								m_Input.m_Direction = -1;

								//new hammer agressive in the walkdirection to free the way
								if (m_FreezeTime == 0)
								{
									m_Input.m_TargetX = -200;
									m_Input.m_TargetY = -2;
									if (Server()->Tick() % 20 == 0)
									{
										SetWeapon(0);
									}
									if (Server()->Tick() % 25 == 0)
									{
										m_Input.m_Fire++;
										m_LatestInput.m_Fire++;
									}
								}
							}
							else //wenn weit genung links
							{
								//dann kann das normale movement von dort aus genung schwung auf bauen
								m_Dummy_get_speed = false;
							}
						}
						else
						{
							if (m_Core.m_Pos.x < 415 * 32) //bis zum tunnel laufen
							{
								m_Input.m_Direction = 1;

							}
							else if (m_Core.m_Pos.x < 440 * 32 && m_Core.m_Pos.y > 213 * 32) //im tunnel laufen
							{
								m_Input.m_Direction = 1;

							}


							//externe if abfrage weil laufen während sprinegn xD

							if (m_Core.m_Pos.x > 413 * 32 && m_Core.m_Pos.x < 415 * 32) // in den tunnel springen
							{
								m_Input.m_Jump = 1;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
								//m_Input.m_Jump = 0;
							}
							else if (m_Core.m_Pos.x > 428 * 32 - 20 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen
							{
								m_Input.m_Jump = 1;
							}



							// externen springen aufhören für dj

							if (m_Core.m_Pos.x > 428 * 32 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen nicht mehr springen
							{
								m_Input.m_Jump = 0;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
							}


							//nochmal extern weil springen während springen

							if (m_Core.m_Pos.x > 430 * 32 && m_Core.m_Pos.y > 213 * 32) // im tunnel springen springen
							{
								m_Input.m_Jump = 1;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "triggerd");
							}



							if (m_Core.m_Pos.x > 431 * 32 && m_Core.m_Pos.y > 213 * 32) //jump refillen für wayblock spot
							{
								m_Input.m_Jump = 0;
							}
						}
					}


					// *****************************************************
					// Way Block spot
					// *****************************************************
					// wayblockspot < 213 [y]


					//externer wayblockspot stuff

					//Checken ob der bot in seinem arial angegriffen wird obwohl kein nostand links ausgerufen wurde



					//wird nicht genutzt weil das preventive springen vom boden aus schluss endlich schlimmer ausgeht als der dj
					/*
					if (m_Core.m_Pos.y < 213 * 32 && m_Core.m_Pos.x > (427 * 32) - 20 && m_Core.m_Pos.x < (428 * 32) + 10) //wenn der bot sich an seinem ruler spot befindet
					{
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich mag diesen ort :)");

					if (!m_Dummy_wb_hooked && !m_Dummy_emergency && !m_Dummy_pushing && m_Core.m_Vel.x > 0.90f) //wenn der bot sich auf das freeze zubewegt obwohl er nicht selber läuft
					{
					// --> er wird wahrscheinlich gehookt oder anderweitig extern angegriffen
					// --> schutzmaßnahmen treffen

					GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "AAAh ich werde angegriffen");
					m_Input.m_Jump = 1;
					}
					m_Dummy_pushing = false;
					m_Dummy_emergency = false;
					m_Dummy_wb_hooked = false;
					}
					*/

					//moved dynamic selfkills outside internal wb spot
					//self kill im freeze
					//New Testy selfkill kill if isFreezed and vel 0
					if (!isFreezed || m_Core.m_Vel.x < -0.5f || m_Core.m_Vel.x > 0.5f || m_Core.m_Vel.y != 0.000000f)
					{
						//mach nichts lol brauche nur das else is einfacher
					}
					else
					{
						if (Server()->Tick() % 150 == 0)
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}



					//Bools zurueck setzten
					m_Dummy_pushing = false;
					m_Dummy_emergency = false;
					m_Dummy_wb_hooked = false;
					m_Dummy_happy = false;

					//normaler internen wb spot stuff

					//if (m_Core.m_Pos.y < 213 * 32) //old new added a x check idk why the was no
					if (m_Core.m_Pos.y < 213 * 32 && m_Core.m_Pos.x > 415 * 32)
					{


						//Old self kill kill if freeze
						//if (m_Core.m_Pos.y < 201 * 32) // decke
						//{
						//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "suicide reason: roof rulerspot");
						//}
						//else if (m_Core.m_Pos.x < 417 * 32 && m_Core.m_Pos.x > 414 * 32 + 17 && isFreezed) //linker streifen xD
						//{
						//	Die(m_pPlayer->GetCID(), WEAPON_SELF);
						//	//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "suicide reason: left wall rulerspot");
						//}



						//New stuff RANDOM STOFF ROFL
						//if the bot is in his wb position an bored and there is no actual danger 
						// ---> flick some aim and fire around xD

						//m_Dummy_bored_cuz_nothing_happens = true;

						//dont activate all the time and dunno how to make a cool activator code so fuck it rofl


						if (m_Dummy_bored_cuz_nothing_happens)
						{
							CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
							if (pChr && pChr->IsAlive() && m_FreezeTime == 0)
							{
								if (pChr->m_Pos.x < 429 * 32 && pChr->m_Core.m_Vel.x < 4.3f)
								{
									int x = rand() % 100 - 50;
									int y = rand() % 100;

									m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x + x;
									m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y + y;


									//doesnt work. i dont care. i dont fix. i just comment it out cuz fuck life is a bitch


									//int fr = rand() % 2000;
									//if (fr < 1300)
									//{
									//	m_Dummy_bored_shootin = true;
									//}

									//if (m_Dummy_bored_shootin)
									//{
									//	m_Input.m_Fire++;
									//	m_LatestInput.m_Fire++;

									//	if (Server()->Tick() % 50 == 0)
									//	{
									//		m_Dummy_bored_shootin = false;
									//	}
									//}

								}
							}
						}




						//TODO(1): keep this structur in mind this makes not much sence
						// the bool m_Dummy_happy is just true if a enemy is in the ruler area because all code below depends on a enemy in ruler area
						// maybe rework this shit



						//                                                      
						//                                               --->   Ruler   <---    testy own class just search in ruler area

						CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this);  //position anderer spieler mit pikus aimbot abfragen
						if (pChr && pChr->IsAlive())
						{
							//sometimes walk to enemys.   to push them in freeze or super hammer them away
							if (pChr->m_Pos.y < m_Core.m_Pos.y + 2 && pChr->m_Pos.y > m_Core.m_Pos.y - 9)
							{
								if (pChr->m_Pos.x > m_Core.m_Pos.x)
								{
									m_Input.m_Direction = 1;
								}
								else
								{
									m_Input.m_Direction = -1;
								}
							}



							if (pChr->m_FreezeTime == 0) //if enemy in ruler spot is unfreeze -->notstand panic
							{
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "NOTSTAND");

								if (Server()->Tick() % 30 == 0)  //angry emotes machen
								{
									GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9);
								}


								if (Server()->Tick() % 20 == 0)
								{
									SetWeapon(0);
								}


								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								if (m_FreezeTime == 0)
								{
									m_LatestInput.m_Fire++;
									m_Input.m_Fire++;
								}

								//testy sollte eig auch am anfang des modes passen
								//m_Input.m_Direction = 0;

								//if (m_Core.m_HookState == HOOK_FLYING)
								//	m_Input.m_Hook = 1;
								//else if (m_Core.m_HookState == HOOK_GRABBED)
								//	m_Input.m_Hook = 1;
								//else
								//	m_Input.m_Hook = 0;

								//char aBuf[256];
								//str_format(aBuf, sizeof(aBuf), "hookstate: %x", m_Input.m_Hook);
								//GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);

								m_Dummy_emergency = true;




								if (!m_Dummy_left_freeze_full)
								{
									//                                                                                                        x > 5 = 3       <-- ignorieren xD


									//                                                                                                          hier wird das schieben an das andere schieben 
									//                                                                                                    übergeben weil er hier von weiter weg anfängt zu schieben 
									//                                                                                                und das kürzere schieben macht dann den ganzen stuff dass der bot nicht selber rein läuft
									//                                                                                                ja ich weiss das ist ziemlich umständlich xD
									//                                                                                                      aber das hat schon sinn das hier wird aufgerufen wenn der weit weg is und freezed und
									//                                                                                                  übergibt dann an die abfrage die auch aufgerufen wird wenn jemand unfreeze is jedoch nir auf kurze distanz

									//                                                                                                          tja aber das mit dem übergeben klappt ja nich wirklich


									//                                                                                                           Deswegen hab ich den code ganz gelöscht und nur ein teil als || in die "freeze protection & schieberrei" geklatscht
									//                                                                                                         ----> hier is ein berg an kommentaren zu nicht existentem code lol    gut das nur ich hier rum stüber hueueueu
									//start sequenz
									// Blocke spieler in die linke freeze wand

									if (!m_Dummy_jumped)
									{
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "boing!");
										m_Input.m_Jump = 1;
										m_Dummy_jumped = true;
									}
									else
									{
										m_Input.m_Jump = 0;
									}



									if (!m_Dummy_hooked)
									{
										if (Server()->Tick() % 30 == 0)
											m_Dummy_hook_delay = true;

										//testy removed hook here i dont know why but all works pretty good still xD
										if (m_Dummy_hook_delay)
											//m_Input.m_Hook = 1;

											if (Server()->Tick() % 200 == 0)
											{
												m_Dummy_hooked = true;
												m_Input.m_Hook = 0;
											}

									}

									if (!m_Dummy_moved_left)
									{
										if (m_Core.m_Pos.x > 419 * 32 + 20)
											m_Input.m_Direction = -1;
										else
											m_Input.m_Direction = 1;

										if (Server()->Tick() % 200 == 0)
										{
											m_Dummy_moved_left = true;
											m_Input.m_Direction = 0;
										}

									}
								}


								//Blocke gefreezte gegner für immer 


								//TODO:
								//das is ein linke seite block wenn hier voll is sollte man anders vorgehen
								//                           früher war es y > 210   aber change weil er während er ihn hochzieht dann nicht mehr das hooken aufhört
								if (pChr->m_FreezeTime > 0 && pChr->m_Pos.y > 204 * 32 && pChr->m_Pos.x > 422 * 32) //wenn ein gegner weit genung rechts freeze am boden liegt
								{
									// soll der bot sich einer position links des spielers nähern
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hab n opfer gefunden");

									if (m_Core.m_Pos.x + (5 * 32 + 40) < pChr->m_Pos.x) // er versucht 5 tiles links des gefreezten gegner zu kommen
									{
										m_Input.m_Direction = -1;

										if (m_Core.m_Pos.x > pChr->m_Pos.x && m_Core.m_Pos.x < pChr->m_Pos.x + (4 * 32)) // wenn er 4 tiles rechts des gefreezten gegners is
										{
											m_Input.m_Jump = 1;
											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "boing!");
										}
									}
									else //wenn der bot links des gefreezten spielers is
									{
										m_Input.m_Jump = 1;
										//echo jump
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "boing!");

										m_Input.m_Direction = -1;

										if (m_Core.m_Pos.x < pChr->m_Pos.x) //solange der bot weiter links is
										{
											m_Input.m_Hook = 1;
										}
										else
										{
											m_Input.m_Hook = 0;
										}
									}
								}






								//freeze protection & schieberrei
								//                                                                                                                                                                                                      old (417 * 32 - 60)
								if ((pChr->m_Pos.x + 10 < m_Core.m_Pos.x && pChr->m_Pos.y > 211 * 32 && pChr->m_Pos.x < 418 * 32)
									|| (pChr->m_FreezeTime > 0 && pChr->m_Pos.y > 210 * 32 && pChr->m_Pos.x < m_Core.m_Pos.x && pChr->m_Pos.x > 417 * 32 - 60)) // wenn der spieler neben der linken wand linken freeze wand liegt schiebt ihn der bot rein
								{                                                                                            // oder wenn der spieler weiter weg liegt aber freeze is


									if (!m_Dummy_left_freeze_full) //wenn da niemand is schieb den rein
									{
										// HIER TESTY TESTY CHANGES  211 * 32 + 40 stand hier
										if (pChr->m_Pos.y > 211 * 32 + 40) // wenn der gegner wirklich ganz tief genung is
										{ //                          417 * 32 - 40
											if (m_Core.m_Pos.x > 418 * 32) // aber nicht selber ins freeze laufen
											{
												m_Input.m_Direction = -1;




												//Check ob der gegener freeze is

												if (pChr->m_FreezeTime > 0)
												{
													m_LatestInput.m_Fire = 0; //nicht schiessen ofc xD (doch is schon besser xD)
													m_Input.m_Fire = 0;

													//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
												}

												//letzten stupser geben (sonst gibs bugs kb zu fixen)
												if (pChr->isFreezed) //wenn er schon im freeze is
												{
													m_LatestInput.m_Fire = 1; //hau ihn an die wand
													m_Input.m_Fire = 1;
												}

											}
											else
											{
												m_Input.m_Direction = 1;
												//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 1");
												if (pChr->m_FreezeTime > 0)
												{
													m_LatestInput.m_Fire = 0; //nicht schiessen ofc xD (doch is schon besser xD)
													m_Input.m_Fire = 0;

													//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
												}
												//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "ich halte das auf.");
												//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich will da nich rein laufen");
											}



										}
										else //wenn der gegner nicht tief genung ist
										{

											m_Input.m_Direction = 1;

											if (pChr->m_FreezeTime > 0)
											{
												m_LatestInput.m_Fire = 0; //nicht schiessen ofc xD (doch is schon besser xD)
												m_Input.m_Fire = 0;

												//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
											}
										}



									}
									else //wenn da schon jemand liegt 
									{
										// sag das mal xD
										//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "da liegt schon einer");
									}
								}
								else if (m_Core.m_Pos.x < 419 * 32 + 10) //sonst mehr abstand halten
								{
									m_Input.m_Direction = 1;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 2");
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich will da nich rein laufen");
								}
								// else // wenn nichts los is erstmal stehen bleiben 






								//freeze protection decke mit double jump wenn hammer

								if (m_Core.m_Vel.y < 20.0f && m_Core.m_Pos.y < 207 * 32)  // wenn der tee nach oben gehammert wird
								{
									if (m_Core.m_Pos.y > 206 * 32) //ab 206 würd er so oder so ins freeze springen
										m_Input.m_Jump = 1;

									if (m_Core.m_Pos.y < pChr->m_Pos.y) //wenn der bot über dem spieler is soll er hooken
										m_Input.m_Hook = 1;
									else
										m_Input.m_Hook = 0;
								}


								//wenn der tee hcoh geschleudert wird
								//                 old 4 (macht aber im postiven bereich kein sinn aber hat geklappt)
								//                 HALLO HIER IST DEIN ICH AUS DER ZUKUNFT: du dummes kind wenn er in der luft hammert dann fliegt er doch nicht nach oben und gerade da musst du es ja perfekt haben ... low
								//if (m_Core.m_Vel.y < 4.0f && m_Core.m_Pos.y < pChr->m_Pos.y) //wenn er schneller als 4 nach oben fliegt und höher als der Gegener ist
								// lass das mit speed weg und mach lieber was mit höhe
								if (m_Core.m_Pos.y < 207 * 32 && m_Core.m_Pos.y < pChr->m_Pos.y)
								{
									//in hammer position bewegen
									if (m_Core.m_Pos.x > 418 * 32 + 20) //nicht ins freeze laufen
									{
										if (m_Core.m_Pos.x > pChr->m_Pos.x - 45) //zu weit rechts von hammer position
										{
											m_Input.m_Direction = -1; //gehe links
																	  //GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich will da nich rein laufen");
										}
										else if (m_Core.m_Pos.x < pChr->m_Pos.x - 39) // zu weit links von hammer position
										{
											m_Input.m_Direction = 1;
											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 3");
										}
										else  //im hammer bereich
										{
											m_Input.m_Direction = 0; //bleib da
										}
									}
								}


								//Check ob der gegener freeze is

								if (pChr->m_FreezeTime > 0 && pChr->m_Pos.y > 208 * 32 && !pChr->isFreezed) //wenn der Gegner tief und freeze is macht es wenig sinn den frei zu hammern
								{
									m_LatestInput.m_Fire = 0; //nicht schiessen 
									m_Input.m_Fire = 0;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "haha owned");
								}



								//Hau den weg (wie dummymode 21)
								if (pChr->m_Pos.x > 418 * 32 && pChr->m_Pos.y > 209 * 32)  //das ganze findet nur im bereich statt wo sonst eh nichts passiert
								{
									//wenn der bot den gegner nicht boosten würde hammer den auch nich weg
									m_LatestInput.m_Fire = 0;
									m_Input.m_Fire = 0;

									if (pChr->m_Core.m_Vel.y < -0.5f && m_Core.m_Pos.y + 15 > pChr->m_Pos.y) //wenn der dude speed nach oben hat
									{
										m_Input.m_Jump = 1;
										if (m_FreezeTime == 0)
										{
											m_LatestInput.m_Fire++;
											m_Input.m_Fire++;
										}
									}
								}


								//TODO: FIX:
								//der bot unfreezed den gegner ab einer gewissen höhe wenn er rein gehammert wird schau das da was passiert





								//wenn ein tee freeze links neben dem bot liegt werf den einfach wieder ins freeze becken
								//das is bisher ja noch die einzige sicherheits lücke beim wayblocken
								//wenn man ein tee über den bot hammert

								if (pChr->m_Pos.x > 421 * 32 && pChr->m_FreezeTick > 0 && pChr->m_Pos.x < m_Core.m_Pos.x)
								{
									m_Input.m_Jump = 1;
									m_Input.m_Hook = 1;
								}

								//##############################
								// doggeystyle dogeing the freeze
								//##############################

								//during the fight dodge the freezefloor on the left with brain
								if (m_Core.m_Pos.x > 428 * 32 + 20 && m_Core.m_Pos.x < 438 * 32 - 20)
								{
									//very nobrain directions decision
									if (m_Core.m_Pos.x < 432 * 32) //left --> go left
									{
										m_Input.m_Direction = -1;
									}
									else                           //right --> go right
									{
										m_Input.m_Direction = 1;
									}

									//high speed left goto speed
									if (m_Core.m_Vel.x < -6.4f && m_Core.m_Pos.x < 434 * 32)
									{
										m_Input.m_Direction = -1;
									}

									//low speed to the right
									if (m_Core.m_Pos.x > 431 * 32 + 20 && m_Core.m_Vel.x > 3.3f)
									{
										m_Input.m_Direction = 1;
									}
								}


								//m_pPlayer->m_TeeInfos.m_ColorBody = (0 * 255 / 360);
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Enemy in ruler spot found!");
							}
							else //sonst normal wayblocken und 427 aufsuchen
							{
								//m_Core.m_ActiveWeapon = WEAPON_GUN;
								SetWeapon(1);
								m_Input.m_Jump = 0;

								if (m_Core.m_HookState == HOOK_FLYING)
									m_Input.m_Hook = 1;
								else if (m_Core.m_HookState == HOOK_GRABBED)
									m_Input.m_Hook = 1;
								else
									m_Input.m_Hook = 0;

								//m_pPlayer->m_TeeInfos.m_ColorBody = (120 * 255 / 360);
								//positions check and correction 427


								m_Dummy_jumped = false;
								m_Dummy_hooked = false;
								m_Dummy_moved_left = false;



								if (m_Core.m_Pos.x > 428 * 32 + 15 && m_Dummy_ruled) //wenn viel zu weit ausserhalb der ruler area wo der bot nur hingehookt werden kann
								{
									m_Input.m_Jump = 1;
									m_Input.m_Hook = 1;
								}

								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Prüfe ob zu weit rechts");
								if (m_Core.m_Pos.x < (418 * 32) - 10) // zu weit links -> geh rechts
								{
									m_Input.m_Direction = 1;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: 4");
								}
								else if (m_Core.m_Pos.x >(428 * 32) + 10) // zu weit rechts -> geh links
								{
									m_Input.m_Direction = -1;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich bin zuweit rechts...");
								}
								else // im toleranz bereich -> stehen bleiben
								{
									m_Dummy_happy = true;
									m_Dummy_ruled = true;
									m_Input.m_Direction = 0;
									//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "toleranz bereich");
									//m_Input.m_LatestTargetX = 0;
									//m_Input.m_LatestTargetY = 0;


									//stuff im toleranz bereich doon





									// normal wayblock
									CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this);  //position anderer spieler mit pikus aimbot abfragen
									if (pChr && pChr->IsAlive())
									{


										//m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
										//m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;



										//m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
										//m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

										//m_Input.m_TargetX = 1;//pChr->m_Pos.x - m_Pos.x; //1
										//m_Input.m_TargetY = 0;//pChr->m_Pos.y - m_Pos.y; //0

										//m_LatestInput.m_TargetX = 1;//pChr->m_Pos.x - m_Pos.x;
										//m_LatestInput.m_TargetY = 0;//pChr->m_Pos.y - m_Pos.y;

										//i dunno why there is a if statement under this code and i dont wanna use it 
										//so i made Trick[4] external (woo spagethii code)
										//Trick[4] clears the left freeze
										if (pChr->m_Pos.x < 418 * 32 - 10 && pChr->m_Pos.y > 210 * 32 && pChr->m_Pos.y < 213 * 32 && pChr->isFreezed && pChr->m_Core.m_Vel.y == 0.00f)
										{
											m_DummyFreezeBlockTrick = 4;
										}


										//                                                                                            old was: 418 * 32 + 20          and i have no fkin idea why so i changed to 17
										if (pChr->m_Pos.y < 213 * 32 + 10 && pChr->m_Pos.x < 430 * 32 && pChr->m_Pos.y > 210 * 32 && pChr->m_Pos.x > 417 * 32) // wenn ein spieler auf der linken seite in der ruler area is 
										{
											//wenn ein gegner rechts des bots is prepare für trick[1]
											if (m_Core.m_Pos.x < pChr->m_Pos.x && pChr->m_Pos.x < 429 * 32 + 6)
											{
												m_Input.m_Direction = -1;
												m_Input.m_Jump = 0;

												if (m_Core.m_Pos.x < 422 * 32)
												{
													m_Input.m_Jump = 1;
													m_DummyFreezeBlockTrick = 1;
													//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "trick1: hook to the left");
												}
											}
											//wenn ein gegner links des bots is prepare für tick[3]
											if (m_Core.m_Pos.x > pChr->m_Pos.x && pChr->m_Pos.x < 429 * 32)
											{
												m_Input.m_Direction = 1;
												m_Input.m_Jump = 0;

												if (m_Core.m_Pos.x > 427 * 32 || m_Core.m_Pos.x > pChr->m_Pos.x + (5 * 32))
												{
													m_Input.m_Jump = 1;
													m_DummyFreezeBlockTrick = 3;
													//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "trick3: hook to the right");
												}
											}


										}
										else // wenn spieler irgendwo anders is
										{
											//wenn ein spieler rechts des freezes is und freeze is Tric[2]
											if (pChr->m_Pos.x > 433 * 32 && pChr->m_FreezeTime > 0 && IsGrounded())
											{
												m_DummyFreezeBlockTrick = 2;
												//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "trick2: swinger");
											}
										}

									}
								}
							}
						}
						else // wenn kein lebender spieler im ruler spot ist
						{

							//Suche trozdem 427 auf

							if (m_Core.m_Pos.x < (427 * 32) - 20) // zu weit links -> geh rechts
							{
								m_Input.m_Direction = 1;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "stop error code: special");
							}
							else if (m_Core.m_Pos.x >(427 * 32) + 40) // zu weit rechts -> geh links
							{
								m_Input.m_Direction = -1;
								//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "Ich bin zuweit rechts...");
							}
							//GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
						}






						// über das freeze springen wenn rechts der bevorzugenten camp stelle

						if (m_Core.m_Pos.x > 434 * 32)
						{
							m_Input.m_Jump = 1;
						}
						else if (m_Core.m_Pos.x > (434 * 32) - 20 && m_Core.m_Pos.x < (434 * 32) + 20) // bei flug über freeze jump wd holen
						{
							m_Input.m_Jump = 0;
							//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "refilling jump");
						}

						//new testy moved tricks into Wayblocker area (y < 213 && x > 215) (was external)
						//TRICKS
						if (1 == 1)
						{
							CCharacter *pChr = GameServer()->m_World.ClosestCharTypeRuler(m_Pos, true, this);
							if (pChr && pChr->IsAlive())
							{
								if (!m_Dummy_emergency && m_Core.m_Pos.x > 415 && m_Core.m_Pos.y < 213 * 32 && m_DummyFreezeBlockTrick != 0) //as long as no enemy is unfreeze in base --->  do some trickzz
								{
									//Trick reset all  
									//resett in the tricks because trick1 doesnt want it
									//m_Input.m_Hook = 0;
									//m_Input.m_Jump = 0;
									//m_Input.m_Direction = 0;
									//m_LatestInput.m_Fire = 0;
									//m_Input.m_Fire = 0;

									//off tricks when not gud to make tricks#
									if (pChr->m_FreezeTime == 0)
									{
										m_DummyFreezeBlockTrick = 0;
										m_Dummy_trick_panic_check_delay = 0;
										m_Dummy_trick3_panic_check = false;
										m_Dummy_trick3_start_count = false;
										m_Dummy_trick3_panic = false;
										m_Dummy_trick4_hasstartpos = false;
									}


									if (m_DummyFreezeBlockTrick == 1) //Tick[1] enemy on the right
									{
										if (pChr->isFreezed)
										{
											m_DummyFreezeBlockTrick = 0; //stop trick if enemy is in freeze
										}
										m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
										m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
										m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
										m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

										if (Server()->Tick() % 40 == 0)
										{
											SetWeapon(0);
										}


										if (pChr->m_Pos.x < m_Core.m_Pos.x && pChr->m_Pos.x > m_Core.m_Pos.x - 180) //if enemy is on the left in hammer distance
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}

										if (m_Core.m_Pos.y < 210 * 32 + 10)
										{
											m_Dummy_start_hook = true;
										}

										if (m_Dummy_start_hook)
										{
											if (Server()->Tick() % 80 == 0 || pChr->m_Pos.x < m_Core.m_Pos.x + 22)
											{
												m_Dummy_start_hook = false;
											}
										}


										if (m_Dummy_start_hook)
										{
											m_Input.m_Hook = 1;
										}
										else
										{
											m_Input.m_Hook = 0;
										}
									}
									else if (m_DummyFreezeBlockTrick == 2) //enemy on the right plattform --> swing him away
									{
										m_Input.m_Hook = 0;
										m_Input.m_Jump = 0;
										m_Input.m_Direction = 0;
										m_LatestInput.m_Fire = 0;
										m_Input.m_Fire = 0;

										if (Server()->Tick() % 50 == 0)
										{
											m_Dummy_bored_counter++;
											GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
										}

										if (m_Core.m_Pos.x < 438 * 32) //first go right
										{
											m_Input.m_Direction = 1;
										}

										if (m_Core.m_Pos.x > 428 * 32 && m_Core.m_Pos.x < 430 * 32) //first jump
										{
											m_Input.m_Jump = 1;
										}

										if (m_Core.m_Pos.x > 427 * 32) //aim and swing
										{
											m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
											m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
											m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
											m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;


											if (m_Core.m_Pos.x > 427 * 32 + 30 && pChr->m_Pos.y < 214 * 32)
											{
												m_Input.m_Hook = 1;
												if (Server()->Tick() % 10 == 0)
												{
													int x = rand() % 100 - 50;
													int y = rand() % 100 - 50;

													m_Input.m_TargetX = x;
													m_Input.m_TargetY = y;
												}
												//random shooting xD
												int r = rand() % 200 + 10;
												if (Server()->Tick() % r == 0 && m_FreezeTime == 0)
												{
													m_Input.m_Fire++;
													m_LatestInput.m_Fire++;
												}
											}
										}

										//also this trick needs some freeze dogining because sometime huge speed fucks the bot
										//and NOW THIS CODE is here to fuck the high speed 
										// yo!
										if (m_Core.m_Pos.x > 440 * 32)
										{
											m_Input.m_Direction = -1;
										}
										if (m_Core.m_Pos.x > 439 * 32 + 20 && m_Core.m_Pos.x < 440 * 32)
										{
											m_Input.m_Direction = 0;
										}


									}
									else if (m_DummyFreezeBlockTrick == 3) //enemy on the left swing him to the right
									{
										m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
										m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
										m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
										m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;



										if (m_Core.m_Pos.y < 210 * 32 + 10)
										{
											m_Dummy_start_hook = true;
											m_Dummy_trick3_start_count = true;
										}

										if (m_Dummy_start_hook)
										{
											if (Server()->Tick() % 80 == 0 || pChr->m_Pos.x > m_Core.m_Pos.x - 22)
											{
												m_Dummy_start_hook = false;
											}
										}


										if (m_Dummy_start_hook)
										{
											m_Input.m_Hook = 1;
										}
										else
										{
											m_Input.m_Hook = 0;
										}

										if (m_Core.m_Pos.x < 429 * 32)
										{
											m_Input.m_Direction = 1;
										}
										else
										{
											m_Input.m_Direction = -1;
										}



										//STOPPER hook:
										//hook the tee if he flys to much to the right
										if (pChr->m_Pos.x > 433 * 32 + 20)
										{
											m_Input.m_Hook = 1;
										}

										//Hook the tee agian and go to the left -> drag him under block area
										//-->Trick 5
										if (pChr->m_Core.m_Vel.y > 8.1f && pChr->m_Pos.x > 429 * 32 + 1 && pChr->m_Pos.y > 209 * 32)
										{
											m_DummyFreezeBlockTrick = 5;
											m_Input.m_Hook = 1;
										}

										//if he lands on the right plattform switch trick xD
										//doesnt work anysways (now fixed by the stopper hook)
										if (pChr->m_Pos.x > 433 * 32 && pChr->m_Core.m_Vel.y == 0.0f)
										{
											m_DummyFreezeBlockTrick = 2;
											//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "trick gone wrong --> change trick");
										}


										//Check for trick went wrong --> trick3 panic activation
										if (m_Dummy_trick3_start_count)
										{
											m_Dummy_trick_panic_check_delay++;
										}
										if (m_Dummy_trick_panic_check_delay > 52)
										{
											m_Dummy_trick3_panic_check = true;
										}
										if (m_Dummy_trick3_panic_check)
										{
											if (pChr->m_Pos.x < 430 * 32 && pChr->m_Pos.x > 426 * 32 + 10 && pChr->IsGrounded())
											{
												m_Dummy_trick3_panic = true;
												m_Dummy_trick3_panic_left = true;
											}
										}
										if (m_Dummy_trick3_panic)
										{
											//stuck --> go left and swing him down
											m_Input.m_Direction = 1;
											if (m_Core.m_Pos.x < 425 * 32)
											{
												m_Dummy_trick3_panic_left = false;
											}

											if (m_Dummy_trick3_panic_left)
											{
												m_Input.m_Direction = -1;
											}
											else
											{
												if (m_Core.m_Pos.y < 212 * 32 + 10)
												{
													m_Input.m_Hook = 1;
												}
											}
										}
									}
									else if (m_DummyFreezeBlockTrick == 4) //clear left freeze
									{
										m_Input.m_Hook = 0;
										m_Input.m_Jump = 0;
										m_Input.m_Direction = 0;
										m_LatestInput.m_Fire = 0;
										m_Input.m_Fire = 0;


										if (!m_Dummy_trick4_hasstartpos)
										{
											if (m_Core.m_Pos.x < 423 * 32 - 10)
											{
												m_Input.m_Direction = 1;
											}
											else if (m_Core.m_Pos.x > 424 * 32 - 20)
											{
												m_Input.m_Direction = -1;
											}
											else
											{
												m_Dummy_trick4_hasstartpos = true;
											}
										}
										else //has start pos
										{
											m_Input.m_TargetX = -200;
											m_Input.m_TargetY = -2;
											if (pChr->isFreezed)
											{
												m_Input.m_Hook = 1;
											}
											else
											{
												m_Input.m_Hook = 0;
												m_DummyFreezeBlockTrick = 0; //fuck it too lazy normal stuff shoudl do the rest xD
											}
											if (Server()->Tick() % 7 == 0)
											{
												m_Input.m_Hook = 0;
											}
										}
									}
									else if (m_DummyFreezeBlockTrick == 5) //Hook under blockarea to the left (mostly the end of a trick)
									{
										//For now this trick only gets triggerd in trick 3 at the end

										//TODO: this trick needs a tick


										m_Input.m_Hook = 1;

										if (m_Core.m_HookState == HOOK_GRABBED)
										{
											m_Input.m_Direction = -1;
										}
										else
										{
											if (m_Core.m_Pos.x < 428 * 32 + 20)
											{
												m_Input.m_Direction = 1;
											}
										}
									}



								}
							}
							else //nobody alive in ruler area --> stop tricks
							{
								m_Dummy_trick4_hasstartpos = false;
								m_Dummy_trick3_panic = false;
								m_Dummy_trick3_start_count = false;
								m_Dummy_trick3_panic_check = false;
								m_Dummy_trick_panic_check_delay = 0;
								m_DummyFreezeBlockTrick = 0;
							}
						}


					}




					//##################################
					// 29 only protections and doge moves
					//##################################


					//Super last jumpy freeze protection o.O
					//saves alot bot live im very sure
					//#longlivesthebotrofl

					if (m_Core.m_Pos.x > 429 * 32 && m_Core.m_Pos.x < 436 * 32 && m_Core.m_Pos.y < 214 * 32) //dangerous area over the freeze
					{
						//first check! too low?
						if (m_Core.m_Pos.y > 211 * 32 + 10 && IsGrounded() == false)
						{
							m_Input.m_Jump = 1;
							m_Input.m_Hook = 1;
							if (Server()->Tick() % 4 == 0)
							{
								m_Input.m_Jump = 0;
							}
						}

						//second check! too speedy?
						if (m_Core.m_Pos.y > 209 * 32 && m_Core.m_Vel.y > 5.6f)
						{
							m_Input.m_Jump = 1;
							if (Server()->Tick() % 4 == 0)
							{
								m_Input.m_Jump = 0;
							}
						}
					}

					//survival moves above the second freeze in the ruler from the left
					// ascii art shows where :
					//
					//                   |
					//                   |
					//                   v
					//                        --------
					//-----#####----###########-######
					//###########-####################
					//           #
					//           #
					//           -#########################----------
					//           #--------------------------

					if (m_Core.m_Pos.x > 439 * 32 && m_Core.m_Pos.x < 449 * 32)
					{
						//low left lowspeed --> go left
						if (m_Core.m_Pos.x > 439 * 32 && m_Core.m_Pos.y > 209 * 32 && m_Core.m_Vel.x < 3.0f)
						{
							m_Input.m_Direction = -1;
						}
						//low left highrightspeed --> go right with the speed and activate some random modes to keep the speed xD
						if (m_Core.m_Pos.x > 439 * 32 && m_Core.m_Pos.y > 209 * 32 && m_Core.m_Vel.x > 6.0f && m_Core.m_Jumped < 2)
						{
							m_Input.m_Direction = 1;
							m_Input.m_Jump = 1;
							if (Server()->Tick() % 5 == 0)
							{
								m_Input.m_Jump = 0;
							}
							m_Dummy_speedright = true;
						}

						if (isFreezed || m_Core.m_Vel.x < 4.3f)
						{
							m_Dummy_speedright = false;
						}

						if (m_Dummy_speedright)
						{
							m_Input.m_Direction = 1;
							m_Input.m_TargetX = 200;
							int r = rand() % 200 - 100;
							m_Input.m_TargetY = r;
							m_Input.m_Hook = 1;
							if (Server()->Tick() % 30 == 0 && m_Core.m_HookState != HOOK_GRABBED)
							{
								m_Input.m_Hook = 0;
							}
						}
					}
					else //out of the freeze area resett bools
					{
						m_Dummy_speedright = false;
					}



					//go down on plattform to get dj 
					//bot always fails going back from right
					//because he doesnt refills his dj

					//            |
					//            |
					//            v
					//                        --------
					//-----#####----###########-######
					//###########-####################
					//           #
					//           #
					//           -#########################----------
					//           #--------------------------

					if (m_Core.m_Pos.x > 433 * 32 + 20 && m_Core.m_Pos.x < 437 * 32 && m_Core.m_Jumped > 2)
					{
						m_Input.m_Direction = 1;
					}




					//##########################################
					// S P E C I A L    S H I T ! ! !          #
					//##########################################             agian...

					//woo special late important new stuff xD
					//reached hammerfly plattform --> get new movement skills
					//this area has his own extra codeblock with cool stuff

					if (m_Core.m_Pos.x > 448 * 32)
					{
						m_Input.m_Hook = 0;
						m_Input.m_Jump = 0;
						m_Input.m_Direction = 0;
						m_LatestInput.m_Fire = 0;
						m_Input.m_Fire = 0;

						if (m_Core.m_Pos.x < 451 * 32 + 20 && IsGrounded() == false && m_Core.m_Jumped > 2)
						{
							m_Input.m_Direction = 1;
						}
						if (m_Core.m_Vel.x < -0.8f && m_Core.m_Pos.x < 450 * 32 && IsGrounded())
						{
							m_Input.m_Jump = 1;
						}
						CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
						if (pChr && pChr->IsAlive())
						{
							if (m_Core.m_Pos.x < 451 * 32)
							{
								m_Input.m_Direction = 1;
							}

							if (pChr->m_Pos.x < m_Core.m_Pos.x - 7 * 32 && m_Core.m_Jumped < 2) //if enemy is on the left & bot has jump --> go left too
							{
								m_Input.m_Direction = -1;
							}
							if (m_Core.m_Pos.x > 454 * 32)
							{
								m_Input.m_Direction = -1;
								m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
								m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
								m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

								if (m_Core.m_Pos.y + 40 > pChr->m_Pos.y)
								{
									m_Input.m_Hook = 1;
									if (Server()->Tick() % 50 == 0)
									{
										m_Input.m_Hook = 0;
									}
								}
							}


							//second freeze from the right --> goto singel palttform
							if (m_Core.m_Pos.x > 464 * 32 && m_Core.m_Jumped > 2 && m_Core.m_Pos.x < 468 * 32)
							{
								m_Input.m_Direction = 1;
							}
							//go back
							if (m_Core.m_Pos.x < 468 * 32 && IsGrounded() && m_Core.m_Pos.x > 464 * 32)
							{
								m_Input.m_Jump = 1;
							}
							//balance
							if (m_Core.m_Pos.x > 460 * 32 && m_Core.m_Pos.x < 464 * 32 && m_Core.m_Pos.y > 210 * 32 + 10)
							{
								m_Dummy_doBalance = true;
							}
							if (IsGrounded() && isFreezed)
							{
								m_Dummy_doBalance = false;
							}

							if (m_Dummy_doBalance)
							{
								if (m_Core.m_Pos.x > 463 * 32) //go right
								{
									if (m_Core.m_Pos.x > pChr->m_Pos.x - 4)
									{
										m_Input.m_Direction = -1;
									}
									else if (m_Core.m_Pos.x > pChr->m_Pos.x)
									{
										m_Input.m_Direction = 1;
									}

									if (m_Core.m_Pos.x < pChr->m_Pos.x)
									{
										m_Input.m_TargetX = 5;
										m_Input.m_TargetY = 200;
										if (m_Core.m_Pos.x - 1 < pChr->m_Pos.x)
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}
									}
									else
									{
										//do the random flick
										int r = rand() % 100 - 50;
										m_Input.m_TargetX = r;
										m_Input.m_TargetY = -200;
									}
									if (pChr->m_Pos.x > 465 * 32 - 10 && pChr->m_Pos.x < 469 * 32) //right enough go out
									{
										m_Input.m_Direction = 1;
									}


								}
								else //go left
								{
									if (m_Core.m_Pos.x > pChr->m_Pos.x + 4)
									{
										m_Input.m_Direction = -1;
									}
									else if (m_Core.m_Pos.x < pChr->m_Pos.x)
									{
										m_Input.m_Direction = 1;
									}

									if (m_Core.m_Pos.x > pChr->m_Pos.x)
									{
										m_Input.m_TargetX = 5;
										m_Input.m_TargetY = 200;
										if (m_Core.m_Pos.x + 1 > pChr->m_Pos.x)
										{
											m_Input.m_Fire++;
											m_LatestInput.m_Fire++;
										}
									}
									else
									{
										//do the random flick
										int r = rand() % 100 - 50;
										m_Input.m_TargetX = r;
										m_Input.m_TargetY = -200;
									}
									if (pChr->m_Pos.x < 459 * 32) //left enough go out
									{
										m_Input.m_Direction = -1;
									}
								}
							}

						}
						else //no close enemy alive
						{
							if (m_Core.m_Jumped < 2)
							{
								m_Input.m_Direction = -1;
							}
						}

						//jump protection second freeze from the right

						//                                  |            -########
						//                                  |            -########
						//                                  v                    #
						//                                                       #
						//                -------------##########---##############
						// ...#####---#######-########------------ ---------------

						if (m_Core.m_Pos.x > 458 * 32 && m_Core.m_Pos.x < 466 * 32)
						{
							if (m_Core.m_Pos.y > 211 * 32 + 26)
							{
								m_Input.m_Jump = 1;
							}
							if (m_Core.m_Pos.y > 210 * 32 && m_Core.m_Vel.y > 5.4f)
							{
								m_Input.m_Jump = 1;
							}
						}

						//go home if its oky, oky?
						if ((m_Core.m_Pos.x < 458 * 32 && IsGrounded() && pChr->isFreezed) || (m_Core.m_Pos.x < 458 * 32 && IsGrounded() && pChr->m_Pos.x > m_Core.m_Pos.x + (10 * 32)))
						{
							m_Input.m_Direction = -1;
						}
						//keep going also in the air xD
						if (m_Core.m_Pos.x < 450 * 32 && m_Core.m_Vel.x < 1.1f && m_Core.m_Jumped < 2)
						{
							m_Input.m_Direction = -1;
						}



						//                                           |   -########
						//                                           |   -########
						//                                           v           #
						//                                                       #
						//                -------------##########---##############
						// ...#####---#######-########------------ ---------------

						//go back if too far
						if (m_Core.m_Pos.x > 468 * 32 + 20)
						{
							m_Input.m_Direction = -1;
						}

					}


					//shitty nub like jump correction because i am too lazy too fix bugsis
					//T0D0(done): fix this bugsis
					//the bot jumps somehow at spawn if a player is in the ruler area
					//i was working with dummybored and tricks 
					//because i cant find the bug i set jump to 0 at spawn

					//here is ChillerDragon from ze future!
					// FUCK YOU old ChillerDragon you just wasted my fucking time with this shitty line
					//im working on another movement where i need jumps at spawn and it took me 20 minutes to find this shitty line u faggot!
					//wow ofc the bot does shit at spawn because u only said the ruler area is (m_Core.m_Pos.y < 213 * 32) and no check on X omg!
					//hope this wont happen agian! (talking to you future dragon)

					//if (m_Core.m_Pos.x < 407 * 32)
					//{
					//	m_Input.m_Jump = 0;
					//}


				}
			}
			else //Change to mode main and reset all
			{
				GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "EROOR!!!!");
				//RestOnChange (zuruecksetzten)
				m_Input.m_Hook = 0;
				m_Input.m_Jump = 0;
				m_Input.m_Direction = 0;
				m_LatestInput.m_Fire = 0;
				m_Input.m_Fire = 0;



				m_Dummy_mode18 = 0;
			}

		}
		else if (m_pPlayer->m_DummyMode == 30) //ChillBlock5 the down left balance to secret moneyroom
		{
			//rest dummy 
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			//hardcodetselfkills:
			if (m_Core.m_Pos.x > 404 * 32)
			{
				Die(m_pPlayer->GetCID(), WEAPON_SELF);
			}
			if (m_Core.m_Pos.y < 204 * 32)
			{
				Die(m_pPlayer->GetCID(), WEAPON_SELF);
			}
			if (m_Core.m_Pos.y < 215 * 32 && m_Core.m_Pos.x < 386 * 32 - 3)
			{
				Die(m_pPlayer->GetCID(), WEAPON_SELF);
			}
			//selfkill
			//dyn
			if (m_Core.m_Vel.y == 0.000000f && m_Core.m_Vel.x < 0.01f && m_Core.m_Vel.x > -0.01f && isFreezed)
			{
				if (Server()->Tick() % 20 == 0)
				{
					GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3);
				}

				if (Server()->Tick() % 90 == 0)
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}
			}


			//balance
			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
			if (pChr && pChr->IsAlive())
			{
				//m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				//m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				//m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				//m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				m_Input.m_TargetX = 2;
				m_Input.m_TargetY = 200;
				m_LatestInput.m_TargetX = 2;
				m_LatestInput.m_TargetY = 200;

				if (pChr->m_Pos.y > m_Core.m_Pos.y && m_Core.m_Pos.x > 310 * 32)
				{
					m_Input.m_Direction = 1;
					if (pChr->m_Pos.x < m_Core.m_Pos.x - 3)
					{
						m_Input.m_Direction = -1;
					}
					if (pChr->m_Pos.x > m_Core.m_Pos.x + 1)
					{
						m_Input.m_Direction = 1;
					}
					if (m_Core.m_Pos.x > pChr->m_Pos.x + 1 && m_Core.m_Pos.y > 238 * 32 && pChr->IsGrounded() && m_Core.m_Vel.x < -0.002f)
					{
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
						m_Input.m_Direction = -1;
					}
				}
			}


			//movement going down#
			if (m_Core.m_Pos.y < 238 * 32 && m_Core.m_Pos.x > 344 * 32)
			{
				if (m_Core.m_Pos.x > 390 * 32)
				{
					m_Input.m_Direction = -1;
				}
				if (m_Core.m_Pos.x < 388 * 32)
				{
					m_Input.m_Direction = 1;
				}
			}
			else
			{
				if (Server()->Tick() % 40 == 0)
				{
					SetWeapon(0);
				}
			}


			if (m_Core.m_Pos.x < 314 * 32 - 10 && m_Core.m_Vel.x < -0.001f)
			{
				CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
				if (pChr && pChr->IsAlive())
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					if (m_Core.m_Pos.x > 310 * 32)
					{
						m_Input.m_Jump = 1;
					}
					if (m_Core.m_Pos.x > 305 * 32)
					{
						m_Input.m_Direction = -1;
					}
					if (m_Core.m_Pos.x < 308 * 32 + 10 && pChr->m_FreezeTime > 0)
					{
						m_Input.m_Hook = 1;
					}
				}
			}

		}
		else if (m_pPlayer->m_DummyMode == 31) //ChillBlock5 Police Guard
		{
			//rest dummy 
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			//Basic Stuff:
			//tele from spawn into police base
			//if (m_Core.m_Pos.x < 410 * 32 && m_Core.m_Pos.x > 380 * 32 && m_Core.m_Pos.y < 219 * 32 && m_Core.m_Pos.y > 200 * 32) //spawn area
			if (m_Core.m_Pos.x < 460 * 32) //spawn
			{
				m_Core.m_Pos.x = 484 * 32;
				m_Core.m_Pos.y = 234 * 32;
				m_Dummy_SpawnAnimation = true;
			}
			//do spawnanimation in police base
			if (m_Dummy_SpawnAnimation)
			{
				m_Dummy_SpawnAnimation_delay++;
				if (m_Dummy_SpawnAnimation_delay > 2)
				{
					GameServer()->CreatePlayerSpawn(m_Pos);
					m_Dummy_SpawnAnimation = false;
				}
			}

			//selfkill
			//dyn
			if (m_Core.m_Vel.y == 0.000000f && m_Core.m_Vel.x < 0.01f && m_Core.m_Vel.x > -0.01f && isFreezed)
			{
				if (Server()->Tick() % 20 == 0)
				{
					GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3);
				}

				if (Server()->Tick() % 200 == 0)
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}
			}



			//TODO:
			/*
			Check for an officär acteevated the "help me in base" bool
			m_IsHelpPolice = -1;


			for (int i = 0; i++; i < MAX_CLIENTS)
			{
			if (m_HelpPolice)
			m_IsHelpPolice = i; //return id
			}

			if (m_isHelpPolice)
			{

			//geh den suchen und hilf dem usw

			}


			*/


			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
			if (pChr && pChr->IsAlive())
			{
				//for (int i = 0; i < MAX_CLIENTS; i++)
				//{
				//	if (p)
				//}
				m_Dummy_ClosestPolice = false;
				//If a policerank escapes from jail he is treated like a non police
				if ((pChr->m_pPlayer->m_PoliceRank > 0 && pChr->m_pPlayer->m_EscapeTime == 0) || (pChr->m_pPlayer->m_PoliceHelper && pChr->m_pPlayer->m_EscapeTime == 0))
				{
					//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "hello officär");
					m_Dummy_ClosestPolice = true;
					//if (pChr->isFreezed)
					//{
					//	m_Dummy_dmm31 = 2;
					//}
				}

				/*
				########################################

				m_Dummy_dmm31 - - - DUMMY MODE MODE [31]

				########################################

				Structure:

				* [STRUC][1]: Check what sub-mode shoudl be used

				* [STRUC][2]: Do stuff depending on sub-modes

				* [STRUC][3]: Do basic movement depending on sub-modes ( step 2 for all modes higher than 3)


				modes:

				0				LOCAL: NOTHING IS GOING ON
				1				LOCAL: ENEMY ATTACK
				2				LOCAL: POLICE HELP
				3				EXTERNAL: ENEMY ATTACK (right side / jail side)
				4				EXTERNAL: POLICE HELP (right side / jail side)

				*/

				//##############################################
				//[STRUC][1]: Check what sub-mode shoudl be used 
				//##############################################
				if (m_Dummy_ClosestPolice) //police
				{
					if (pChr->m_FreezeTime > 0 && m_Core.m_Pos.x < 477 * 32)
					{
						m_Dummy_dmm31 = 2; // LOCAL: POLICE HELP
					}
					else
					{
						m_Dummy_dmm31 = 0; // LOCAL: NOTHING IS GOING ON
					}
				}
				else //not police
				{
					if (pChr->m_FreezeTime == 0)
					{
						if (pChr->m_Pos.x > 481 * 32)
						{
							//m_Dummy_dmm31 = 3; //EXTERNAL: ENEMY ATTACK(right side / jail side)
						}
						else
						{
							m_Dummy_dmm31 = 1; //LOCAL: ENEMY ATTACK
						}
					}
					if (pChr->isFreezed)
					{
						m_Dummy_dmm31 = 0; //maybe add here a mode where the bot moves the nonPolices away to find failed polices
					}
				}


				//##############################################
				//[STRUC][2]: Do stuff depending on sub - modes
				//##############################################

				if (m_Dummy_dmm31 == 0) //nothing is going on
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					if (Server()->Tick() % 90 == 0)
					{
						SetWeapon(1);
					}
				}
				else if (m_Dummy_dmm31 == 1) //Attack enemys
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;


					if (Server()->Tick() % 30 == 0)
					{
						SetWeapon(0);
					}

					if (m_FreezeTime == 0 && pChr->m_FreezeTime == 0 && pChr->m_Core.m_Vel.y < -0.5 && pChr->m_Pos.x > m_Core.m_Pos.x - 3 * 32 && pChr->m_Pos.x < m_Core.m_Pos.x + 3 * 32)
					{
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
					}

					m_Dummy_AttackMode = 0;
					if (m_Core.m_Pos.x < 466 * 32 + 20 && pChr->m_Pos.x > 469 * 32 + 20) //hook enemy in air (rightside)
					{
						m_Dummy_AttackMode = 1;
					}

					if (m_Dummy_AttackMode == 0) //default mode
					{
						if (m_Core.m_Pos.x < 466 * 32 - 5) //only get bored on lovley place 
						{
							m_Input.m_Direction = rand() % 2;
							if (IsGrounded())
							{
								m_Input.m_Jump = rand() % 2;
							}
							if (pChr->m_Pos.y > m_Core.m_Pos.y)
							{
								m_Input.m_Hook = 1;
							}
						}
					}
					else if (m_Dummy_AttackMode == 1) //hook enemy escaping (rightside)
					{
						if (pChr->m_Core.m_Vel.x > 1.3f)
						{
							m_Input.m_Hook = 1;
						}
					}


					//Dont Hook enemys back in safety
					if ((pChr->m_Pos.x < 460 * 32 && pChr->m_Pos.x > 457 * 32) || (pChr->m_Pos.x < 469 * 32 && pChr->m_Pos.x > 466 * 32))
					{
						m_Input.m_Hook = 0;
					}

				}
				else if (m_Dummy_dmm31 == 2) //help police dudes
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					if (pChr->m_Pos.y > m_Core.m_Pos.y)
					{
						m_Input.m_Hook = 1;
					}
					if (Server()->Tick() % 40 == 0)
					{
						m_Input.m_Hook = 0;
						m_Input.m_Jump = 0;
					}
					if (IsGrounded() && pChr->isFreezed)
					{
						m_Input.m_Jump = 1;
					}

					if (pChr->isFreezed)
					{
						if (pChr->m_Pos.x > m_Core.m_Pos.x)
						{
							m_Input.m_Direction = 1;
						}
						else if (pChr->m_Pos.x < m_Core.m_Pos.x)
						{
							m_Input.m_Direction = -1;
						}
					}
					else
					{
						if (pChr->m_Pos.x - 110 > m_Core.m_Pos.x)
						{
							m_Input.m_Direction = 1;
						}
						else if (pChr->m_Pos.x + 110 < m_Core.m_Pos.x)
						{
							m_Input.m_Direction = -1;
						}
						else
						{
							if (Server()->Tick() % 10 == 0)
							{
								SetWeapon(0);
							}
							if (m_FreezeTime == 0 && pChr->m_FreezeTime > 0)
							{
								m_Input.m_Fire++;
								m_LatestInput.m_Fire++;
							}
						}
					}


					//invert direction if hooked the player to add speed :)
					if (m_Core.m_HookState == HOOK_GRABBED)
					{
						if (pChr->m_Pos.x > m_Core.m_Pos.x)
						{
							m_Input.m_Direction = -1;
						}
						else if (pChr->m_Pos.x < m_Core.m_Pos.x)
						{
							m_Input.m_Direction = 1;
						}
					}

					//schleuderprotection   stop hook if mate is safe to prevemt blocking him to the other side
					if (pChr->m_Pos.x > 460 * 32 + 10 && pChr->m_Pos.x < 466 * 32)
					{
						m_Input.m_Hook = 0;
					}

				}
				else if (m_Dummy_dmm31 == 3) //EXTERNAL: Enemy attack (rigt side /jail side)
				{
					if (m_Core.m_Pos.x < 461 * 32)
					{
						m_Input.m_Direction = 1;
					}
					else
					{
						if (m_Core.m_Pos.x < 484 * 32)
						{
							m_Input.m_Direction = 1;
						}
						if (m_Core.m_Pos.x > 477 * 32 && !IsGrounded())
						{
							m_Input.m_Hook = 1;
						}
					}

					//jump all the time xD
					if (IsGrounded() && m_Core.m_Pos.x > 480 * 32)
					{
						m_Input.m_Jump = 1;
					}

					//Important jump protection
					if (m_Core.m_Pos.x > 466 * 32 && m_Core.m_Pos.y > 240 * 32 + 8 && m_Core.m_Pos.x < 483 * 32)
					{
						m_Input.m_Jump = 1;
					}
				}
				else //unknown dummymode
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					if (Server()->Tick() % 120 == 0)
					{
						//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "error: unknow sub-mode for this dummymode set.");
					}
				}

			}





			//##############################################
			//[STRUC][3]: Do basic movement depending on sub - modes
			//##############################################

			//The basic movements depending on the dummysubmodes 
			//but some submodes use the same thats why its listed external here
			//Movement
			//JailSpawn
			if (m_Dummy_dmm31 < 3)
			{
				if (m_Core.m_Pos.x > 482 * 32 + 20 && m_Core.m_Pos.y < 236 * 32)
				{
					if (m_Core.m_Vel.x > -8.2f && m_Core.m_Pos.x < 484 * 32 - 20)
					{
						m_Dummy_GetSpeed = true;
					}
					if (m_Core.m_Pos.x > 483 * 32 && !IsGrounded())
					{
						m_Dummy_GetSpeed = true;
					}
					if (m_Core.m_Vel.y > 5.3f)
					{
						m_Dummy_GetSpeed = true;
					}

					if (IsGrounded() && m_Core.m_Pos.x > 485 * 32)
					{
						m_Dummy_GetSpeed = false;
					}



					if (m_Dummy_GotStuck)
					{
						m_Input.m_Direction = -1;
						if (Server()->Tick() % 33 == 0)
						{
							m_Input.m_Jump = 1;
						}
						if (Server()->Tick() % 20 == 0)
						{
							SetWeapon(0); //hammer
						}

						if (m_Input.m_TargetX < -20)
						{
							if (m_FreezeTime == 0)
							{
								m_Input.m_Fire++;
								m_LatestInput.m_Fire++;
							}
						}
						else if (m_Input.m_TargetX > 20)
						{
							m_Input.m_Hook = 1;
							if (Server()->Tick() % 25 == 0)
							{
								m_Input.m_Hook = 0;
							}
						}

						//gets false in the big esle m_Dummy_GotStuck = false;
					}
					else
					{
						if (m_Dummy_GetSpeed)
						{
							m_Input.m_Direction = 1;
							if (Server()->Tick() % 90 == 0)
							{
								m_Dummy_GotStuck = true;
							}
						}
						else
						{
							m_Input.m_Direction = -1;
							if (m_Core.m_Vel.x > -4.4f)
							{
								if (Server()->Tick() % 90 == 0)
								{
									m_Dummy_GotStuck = true;
								}
							}
						}
					}

				}
				else //not Jail spawn
				{
					m_Dummy_GotStuck = false;
					//TODO; add a dir 1 if he gets attacked
					if (m_Core.m_Pos.x > 464 * 32)
					{
						m_Input.m_Direction = -1;
					}
					else if (m_Core.m_Pos.x < 461 * 32)
					{
						m_Input.m_Direction = 1;
					}

					if (m_Core.m_Pos.x > 466 * 32 && m_Core.m_Pos.y > 240 * 32 + 8)
					{
						m_Input.m_Jump = 1;
					}
				}
			}
			else
			{
				//no basic moves for this submode
			}
		}
		else if (m_pPlayer->m_DummyMode == 32) // solo police base bot using 5 jumps and insane grenade jump
		{
			m_pPlayer->m_DummyModeSpawn = 32;
			//RestOnChange (zuruecksetzten)
			m_Input.m_Hook = 0;
			m_Input.m_Jump = 0;
			m_Input.m_Direction = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Fire = 0;

			if (m_Core.m_Pos.x > 451 * 32 && m_Core.m_Pos.x < 472 * 32 && m_Core.m_Pos.y > 74 * 32 && m_Core.m_Pos.y < 85 * 32) //spawn bereich  // walk into the left SPAWN tp
			{
				m_Dummy27_speed = 70; // fix for crashbug (DONT REMOVE)
			
				m_Input.m_Direction = -1;
				if (m_Core.m_Pos.x > 454 * 32 && m_Core.m_Pos.x < 458 * 32) //linke seite des spawn bereiches
				{
					m_Input.m_Jump = 1;
					if (Server()->Tick() % 10 == 0)
					{
						m_Input.m_Jump = 0;
					}
				}
			}
			else if (m_Core.m_Pos.x < 240 * 32 && m_Core.m_Pos.y < 36 * 32) // the complete zone in the map intselfs. its for resetting the dummy when he is back in spawn using tp
			{
				if (isFreezed && m_Core.m_Pos.x > 32 * 32)
				{
					if (Server()->Tick() % 60 == 0)
					{
						GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3); // tear emote before killing
					}
					if (Server()->Tick() % 500 == 0 && IsGrounded()) // kill when freeze
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}
				}
				if (m_Core.m_Pos.x < 24 * 32 && m_Core.m_Pos.y < 14 * 32 && m_Core.m_Pos.x > 23 * 32) // looking for tp and setting different aims for the swing
				{
					m_DummySpawnTeleporter = 1;
				}
				if (m_Core.m_Pos.x < 25 * 32 && m_Core.m_Pos.y < 14 * 32 && m_Core.m_Pos.x > 24 * 32) // looking for tp and setting different aims for the swing
				{
					m_DummySpawnTeleporter = 2;
				}
				if (m_Core.m_Pos.x < 26 * 32 && m_Core.m_Pos.y < 14 * 32 && m_Core.m_Pos.x > 25	 * 32) // looking for tp and setting different aims for the swing
				{
					m_DummySpawnTeleporter = 3;
				}
				if (m_Core.m_Pos.y > 21 * 32 && m_Core.m_Pos.x > 43 * 32 && m_Core.m_Pos.y < 35 * 32) // kill 
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}
				if (m_Core.m_Pos.y > 35 * 32 && m_Core.m_Pos.x < 43 * 32) // area bottom right from spawn, if he fall, he will kill
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}
				if (m_Core.m_Pos.x < 16 * 32) // area left of new tee spawn, he will kill too
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}
				else if (m_Core.m_Pos.y > 25 * 32) // after unfreeze hold hook to the right and walk right.
				{
					m_Input.m_TargetX = 100;
					m_Input.m_TargetY = 1;
					m_Input.m_Direction = 1;
					m_Input.m_Hook = 1;
					if (m_Core.m_Pos.x > 25 * 32 && m_Core.m_Pos.x < 33 * 32 && m_Core.m_Pos.y > 30 * 32) //set weapon and aim down
					{
						if (Server()->Tick() % 5 == 0)
						{
							SetWeapon(3);
						}
						if (m_DummySpawnTeleporter == 1)
						{
							m_Input.m_TargetX = 190;
							m_Input.m_TargetY = 100;
							m_LatestInput.m_TargetX = 190;
							m_LatestInput.m_TargetY = 100;
						}
						else if (m_DummySpawnTeleporter == 2)
						{
							m_Input.m_TargetX = 205;
							m_Input.m_TargetY = 100;
							m_LatestInput.m_TargetX = 205;
							m_LatestInput.m_TargetY = 100;
						}
						else if (m_DummySpawnTeleporter == 3)
						{
							m_Input.m_TargetX = 190;
							m_Input.m_TargetY = 100;
							m_LatestInput.m_TargetX = 190;
							m_LatestInput.m_TargetY = 100;
						}
						if (m_Core.m_Pos.x > 31 * 32)
						{
							m_Input.m_Jump = 1;
							m_LatestInput.m_Fire++;
							m_Input.m_Fire++;
						}
					}
				}
				else if (m_Core.m_Pos.x > 33 * 32 && m_Core.m_Pos.x < 50 * 32 && m_Core.m_Pos.y > 18 * 32)
				{
					m_Input.m_Direction = 1;
					if (m_Core.m_Pos.x > 33 * 32 && m_Core.m_Pos.x < 42 * 32 && m_Core.m_Pos.y > 20 * 32 && m_Core.m_Pos.y < 25 * 32)
					{
						GameServer()->SendEmoticon(m_pPlayer->GetCID(), 14); //happy emote when successfully did the grenaede jump
						if (Server()->Tick() % 1 == 0) //change to gun
						{
							SetWeapon(1);
						}
					}
					if (m_Core.m_Pos.x > 47 * 32 && m_Core.m_Pos.x < 48 * 32) // jumping up on the plateu
					{
						m_Input.m_Direction = 0;
						m_Input.m_Jump = 1;
					}
				}
				else if (m_Core.m_Pos.y < 16 * 32 && m_Core.m_Pos.x < 75 * 32 && m_Core.m_Pos.x > 40 * 32) // walking right on it
				{
					m_Input.m_Direction = 1;
					m_Input.m_Jump = 0;
					if (m_Core.m_Pos.x > 55 * 32 && m_Core.m_Pos.x < 56 * 32) //jumping over gap
					{
						m_Input.m_Jump = 1;
					}
				}
				if (m_Core.m_Pos.y > 15 * 32 && m_Core.m_Pos.x > 55 * 32 && m_Core.m_Pos.x < 65 * 32)
				{
					m_Input.m_Direction = 1;
					if (m_Core.m_Pos.x > 63 * 32)
					{
						m_Input.m_Jump = 1;
					}
				}
				else if (m_Core.m_Pos.x > 75 * 32 && m_Core.m_Pos.x < 135 * 32) //gores stuff (the thign with freeze spikes from top and bottom)
				{
					m_Input.m_Jump = 0;
					m_Input.m_Direction = 1;
					if (m_Core.m_Pos.x > 77 * 32) //start jump into gores
					{
						m_Input.m_Jump = 1;
					}
					if (m_Core.m_Pos.x > 92 * 32 && m_Core.m_Pos.y > 12.5 * 32) // hooking stuff from now on
					{
						m_Input.m_TargetX = 100;
						m_Input.m_TargetY = -100;
						m_Input.m_Hook = 1;
						if (m_Core.m_Pos.y < 14 * 32 && m_Core.m_Pos.x > 100 * 32 && m_Core.m_Pos.x < 110 * 32)
						{
							m_Input.m_Hook = 0;
						}
					}
					if (m_Core.m_Pos.x > 120 * 32)
					{
						if (m_Core.m_Pos.y < 13 * 32)
						{
							m_Input.m_Hook = 0;
						}
					}
				}
				else if (m_Core.m_Pos.x > 135 * 32)
				{
					m_Input.m_Direction = 1;
					if (m_Core.m_Pos.y > 12 * 32) // gores (the long way to 5 jumps)
					{
						m_Input.m_TargetX = 100;
						m_Input.m_TargetY = -200;
						m_Input.m_Hook = 1;
						if (m_Core.m_Pos.y < 12 * 32)
						{
							m_Input.m_Hook = 0;
						}
						if (m_Core.m_Pos.x > 212 * 32)
						{
							m_Input.m_Hook = 1;
							m_Input.m_TargetX = 100;
							m_Input.m_TargetY = -75;
							m_Input.m_Jump = 1;
						}
					}
				}
			}
			if (isFreezed && m_Core.m_Pos.y < 410 * 32) // kills when in freeze and not in policebase
			{
				if (Server()->Tick() % 60 == 0)
				{
					GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3); // tear emote before killing
				}
				if (Server()->Tick() % 500 == 0 && IsGrounded()) // kill when freeze
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}
			}
			if (m_Core.m_Pos.x > 368 * 32 && m_Core.m_Pos.y < 340 * 32) //new spawn going left and hopping over the gap under the CFRM.  (the jump over the freeze gap before falling down is not here, its in line 13647)
			{
				m_Input.m_Direction = -1;
				if (m_Core.m_Pos.x > 509 * 32 && m_Core.m_Pos.y > 62 * 32) // if bot gets under the table he will go right and jump out of the gap under the table
				{
					m_Input.m_Direction = 1;
					if (m_Core.m_Pos.x > 511.5 * 32)
					{
						if (Server()->Tick() % 10 == 0)
						{
							m_Input.m_Jump = 1;
						}
					}
				}
				else if (Server()->Tick() % 10 == 0 && m_Core.m_Pos.x > 505 * 32)
				{
					m_Input.m_Jump = 1;
				}
				if (m_Core.m_Pos.x < 497 * 32 && m_Core.m_Pos.x > 496 * 32)
				{
					m_Input.m_Jump = 1;
				}
				if (m_Core.m_Pos.x < 480 * 32 && m_Core.m_Pos.x > 479 * 32)
				{
					m_Input.m_Jump = 1;
				}
			}
			if (m_Core.m_Pos.x > 370 * 32 && m_Core.m_Pos.y < 340 * 32 && m_Core.m_Pos.y > 310 * 32) // bottom going left to the grenade jump
			{
				m_Input.m_Direction = -1;
				if (m_Core.m_Pos.x < 422 * 32 && m_Core.m_Pos.x > 421 * 32) // bottom jump over the hole to police station
				{
					m_Input.m_Jump = 1;
				}
				if (m_Core.m_Pos.x < 406 * 32 && m_Core.m_Pos.x > 405 * 32) // using 5jump from now on
				{
					m_Input.m_Jump = 1;
				}
				if (m_Core.m_Pos.x < 397 * 32 && m_Core.m_Pos.x > 396 * 32)
				{
					m_Input.m_Jump = 1;
				}
				if (m_Core.m_Pos.x < 387 * 32 && m_Core.m_Pos.x > 386 * 32)
				{
					m_Input.m_Jump = 1;
				}
				if (m_Core.m_Pos.x < 377 * 32 && m_Core.m_Pos.x > 376 * 32) // last jump from the 5 jump
				{
					m_Input.m_Jump = 1;
				}
				if (m_Core.m_Pos.y > 339 * 32) // if he falls into the hole to police station he will kill
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}
			}
			else if (m_Core.m_Pos.y > 296 * 32 && m_Core.m_Pos.x < 370 * 32 && m_Core.m_Pos.x > 350 * 32 && m_Core.m_Pos.y < 418 * 32) // getting up to the grenade jump part
			{
				if (IsGrounded())
				{
					m_Input.m_Jump = 1;
				}
				else if (m_Core.m_Pos.y < 313 * 32 && m_Core.m_Pos.y > 312 * 32 && m_Core.m_Pos.x < 367 * 32)
				{
					m_Input.m_Direction = 1;
				}
				else if (m_Core.m_Pos.x > 367 * 32)
				{
					m_Input.m_Direction = -1;
				}
				if (m_Core.m_Vel.y > 0.0000001f && m_Core.m_Pos.y < 310 * 32)
				{
					m_Input.m_Jump = 1;
				}
			}
			else if (m_Core.m_Vel.y > 0.001f && m_Core.m_Pos.y < 293 * 32 && m_Core.m_Pos.x > 366.4 * 32 && m_Core.m_Pos.x < 370 * 32)
			{
				m_Input.m_Direction = -1;
				if (Server()->Tick() % 1 == 0)
				{
					SetWeapon(3);
				}
			}
			else if (m_Core.m_Pos.x > 325 * 32 && m_Core.m_Pos.x < 366 * 32 && m_Core.m_Pos.y < 295 * 32 && m_Core.m_Pos.y > 59 * 32) // insane grenade jump
			{
				if (IsGrounded() && m_DummyGrenadeJump == 0) // shoot up
				{
					m_Input.m_Jump = 1;
					m_Input.m_TargetX = 0;
					m_Input.m_TargetY = -100;
					m_LatestInput.m_TargetX = 0;
					m_LatestInput.m_TargetY = -100;
					m_LatestInput.m_Fire++;
					m_Input.m_Fire++;
					m_DummyGrenadeJump = 1;
				}
				else if (m_Core.m_Vel.y > -7.6f && m_DummyGrenadeJump == 1) // jump in air // basically a timer for when the grenade comes down
				{
					m_Input.m_Jump = 1;
					m_DummyGrenadeJump = 2;
				}
				if (m_DummyGrenadeJump == 2 || m_DummyGrenadeJump == 3) // double grenade
				{
					if (m_Core.m_Pos.y > 58 * 32)
					{
						if (IsGrounded())
						{
							m_DummyTouchedGround = true;
						}
						if (m_DummyTouchedGround == true)
						{
							m_Input.m_Direction = -1;
						}
						if (m_Core.m_Vel.y > 0.1f && IsGrounded())
						{
							m_Input.m_Jump = 1;
							m_Input.m_TargetX = 100;
							m_Input.m_TargetY = 150;
							m_LatestInput.m_TargetX = 100;
							m_LatestInput.m_TargetY = 150;
							m_LatestInput.m_Fire++;
							m_Input.m_Fire++;
							m_DummyGrenadeJump = 3;
						}
						if (m_DummyGrenadeJump == 3)
						{
							if (m_Core.m_Pos.x < 344 * 32 && m_Core.m_Pos.x > 343 * 32 && m_Core.m_Pos.y > 250 * 32) // air grenade for double wall grnade
							{
								m_Input.m_TargetX = -100;
								m_Input.m_TargetY = -100;
								m_LatestInput.m_TargetX = -100;
								m_LatestInput.m_TargetY = -100;
								m_LatestInput.m_Fire++;
								m_Input.m_Fire++;
							}
						}
					}
				}
				if (m_Core.m_Pos.x < 330 * 32 && m_Core.m_Vel.x == 0.0f && m_Core.m_Pos.y > 59 * 32) // if on wall jump and shoot
				{
					if (m_Core.m_Pos.y > 250 * 32 && m_Core.m_Vel.y > 6.0f)
					{
						m_Input.m_Jump = 1;
						m_DummyStartGrenade = true;
					}
					if (m_DummyStartGrenade == true)
					{
						m_Input.m_TargetX = -100;
						m_Input.m_TargetY = 170;
						m_LatestInput.m_TargetX = -100;
						m_LatestInput.m_TargetY = 170;
						m_LatestInput.m_Fire++;
						m_Input.m_Fire++;
					}
					if (m_Core.m_Pos.y < 130 * 32 && m_Core.m_Pos.y > 131 * 32)
					{
						m_Input.m_Jump = 1;
					}
					if (m_Core.m_Vel.y > 0.001f && m_Core.m_Pos.y < 150 * 32)
					{
						m_DummyStartGrenade = false;
					}
					if (m_Core.m_Vel.y > 2.0f && m_Core.m_Pos.y < 150 * 32)
					{
						m_Input.m_Jump = 1;
						m_DummyStartGrenade = true;
					}
				}
			}
			if (m_Core.m_Pos.y < 60 * 32 && m_Core.m_Pos.x < 337 * 32 && m_Core.m_Pos.x > 325 * 32 && m_Core.m_Pos.y > 53 * 32) // top of the grenade jump // shoot left to get to the right 
			{
				m_Input.m_Direction = 1;
				m_Input.m_TargetX = -100;
				m_Input.m_TargetY = 0;
				m_LatestInput.m_TargetX = -100;
				m_LatestInput.m_TargetY = 0;
				m_LatestInput.m_Fire++;
				m_Input.m_Fire++;
				m_Input.m_Jump = 0;
				if (m_Core.m_Pos.x > 333 * 32 && m_Core.m_Pos.x < 335 * 32) // hook up and get into the tunnel thingy
				{
					m_Input.m_Jump = 1;
					if (m_Core.m_Pos.y < 55 * 32)
					{
						m_Input.m_Direction = 1;
					}
				}
			}
			if (m_Core.m_Pos.x > 337 * 32 && m_Core.m_Pos.x < 400 * 32 && m_Core.m_Pos.y < 60 * 32 && m_Core.m_Pos.y > 40 * 32) // hook thru the hookthru
			{
				m_Input.m_TargetX = 0;
				m_Input.m_TargetY = -1;
				m_LatestInput.m_TargetX = 0;
				m_LatestInput.m_TargetY = -1;
				m_Input.m_Hook = 1;
			}
			if (m_Core.m_Pos.x > 339.5 * 32 && m_Core.m_Pos.x < 345 * 32 && m_Core.m_Pos.y < 51 * 32)
			{
				m_Input.m_Direction = -1;
			}
			if (m_Core.m_Pos.x < 339 * 32 && m_Core.m_Pos.x > 315 * 32 && m_Core.m_Pos.y > 40 * 32 && m_Core.m_Pos.y < 53 * 32) // top of grenade jump the thing to go through the wall
			{
				m_Input.m_TargetX = 100;
				m_Input.m_TargetY = 50;
				m_LatestInput.m_TargetX = 100;
				m_LatestInput.m_TargetY = 50;
				if (m_DummyAlreadyBeenHere == false)
				{
					if (m_Core.m_Pos.x < 339 * 32)
					{
						m_Input.m_Direction = 1;
						if (m_Core.m_Pos.x > 338 * 32 && m_Core.m_Pos.x < 339 * 32 && m_Core.m_Pos.y > 51 * 32)
						{
							m_DummyAlreadyBeenHere = true;
						}
					}
				}
				if (m_DummyAlreadyBeenHere == true) //using grenade to get throug the freeze in this tunnel thingy
				{
					m_Input.m_Direction = -1;
					if (m_Core.m_Pos.x < 338 * 32)
					{
						m_LatestInput.m_Fire++;
						m_Input.m_Fire++;
					}
				}
				if (m_Core.m_Pos.x < 328 * 32 && m_Core.m_Pos.y < 60 * 32)
				{
					m_Input.m_Jump = 1;
				}
			}
			else if (m_Core.m_Pos.y > 260 * 32 && m_Core.m_Pos.x < 325 * 32 && m_Core.m_Pos.y < 328 * 32 && m_Core.m_Pos.x > 275 * 32) // jumping over the big freeze to get into the tunnel
			{
				m_Input.m_Direction = -1;
				m_Input.m_Jump = 0;
				if (m_Core.m_Pos.y > 280 * 32 && m_Core.m_Pos.y < 285 * 32)
				{
					m_Input.m_Jump = 1;
				}
				if (Server()->Tick() % 5 == 0)
				{
					SetWeapon(1);
				}
			}
			else if (m_Core.m_Pos.y > 328 * 32 && m_Core.m_Pos.y < 345 * 32 && m_Core.m_Pos.x > 236 * 32 && m_Core.m_Pos.x < 365 * 32) // after grenade jump and being down going into the tunnel to police staion
			{
				m_Input.m_Direction = 1;
				if (m_Core.m_Pos.x > 265 * 32 && m_Core.m_Pos.x < 267 * 32)
				{
					m_Input.m_Jump = 1;
				}
				if (m_Core.m_Pos.x > 282 * 32 && m_Core.m_Pos.x < 284 * 32)
				{
					m_Input.m_Jump = 1;
				}
			}
			if (m_Core.m_Pos.y > 337.4 * 32 && m_Core.m_Pos.y < 345 * 32 && m_Core.m_Pos.x > 295 * 32 && m_Core.m_Pos.x < 365 * 32) // walkking left in air to get on the little block
			{
				m_Input.m_Direction = -1;
			}
			if (m_Core.m_Pos.y < 355 * 32 && m_Core.m_Pos.y > 346 * 32)
			{
				m_Input.m_Direction = 1;
				m_Input.m_Jump = 0;
				if (m_Core.m_Vel.y > 0.0000001f && m_Core.m_Pos.y > 352.6 * 32 && m_Core.m_Pos.x < 315 * 32) // jump in air to get to the right
				{
					m_Input.m_Jump = 1;
				}
			}
			if (m_Core.m_Pos.x > 315.5 * 32 && m_Core.m_Pos.x < 327 * 32 && m_Core.m_Pos.y > 344 * 32 && m_Core.m_Pos.y < 418 * 32) // stop moving in air to get into the hole
			{
				m_Input.m_Direction = 0;
			}
			if (m_Core.m_Pos.x > 290 * 32 && m_Core.m_Pos.x < 450 * 32 && m_Core.m_Pos.y < 450 * 32 && m_Core.m_Pos.y > 380 * 32) //police area // 32
			{
				m_pPlayer->m_DummyMode = 27;
			}
		}
		else if (m_pPlayer->m_DummyMode == 33) //Chillintelligenz
		{
			//########################################
			// YOUTUBE
			// 800 likes top comment omg!
			// https://www.youtube.com/watch?v=xjgd8dki_V4&google_comment_id=z13vxrwiqriytlq3023exlkr5n3sydpvs
			// 100 likes top comment omg!
			// https://www.youtube.com/watch?v=oDTO8j12CBI&google_comment_id=z12ojtqx3tmsxtq1u23exlkr5n3sydpvs
			//#######################################
			// American Ultra
			// He never died
			// watchmen (die Wächter)
			// deadpool
			// Logan (The Wolverine)
			// Hardcore (henry)
			// Scott Pilgrim (gegen den Rest der Welt)
			CITick(); 
		}
		else if (m_pPlayer->m_DummyMode == 34) //survival
		{
			m_Input.m_Jump = 0;
			m_Input.m_Fire = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Hook = 0;
			m_Input.m_Direction = 0;
			//m_pPlayer->m_TeeInfos.m_ColorBody = (0 * 255 / 360);

			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, false, this);
			if (pChr && pChr->IsAlive())
			{
				m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

				if (Server()->Tick() % 10 == 0) //random aim strong every 10 secs
				{
					int rand_x_inp = pChr->m_Pos.x - m_Pos.x + rand() % 200 - 100;
					int rand_y_inp = pChr->m_Pos.y - m_Pos.y + rand() % 200 - 100;

					m_Input.m_TargetX = rand_x_inp;
					m_Input.m_TargetY = rand_y_inp;
					m_LatestInput.m_TargetX = rand_x_inp;
					m_LatestInput.m_TargetY = rand_y_inp;
				}
				else //aim normal bad
				{
					int rand_x_inp = pChr->m_Pos.x - m_Pos.x + rand() % 20 - 10;
					int rand_y_inp = pChr->m_Pos.y - m_Pos.y + rand() % 20 - 10;

					m_Input.m_TargetX = rand_x_inp;
					m_Input.m_TargetY = rand_y_inp;
					m_LatestInput.m_TargetX = rand_x_inp;
					m_LatestInput.m_TargetY = rand_y_inp;
				}


				//m_Input.m_Fire++;
				//m_LatestInput.m_Fire++;

				if (Server()->Tick() % 120 == 0)
				{
					SetWeapon(1); //randomly swap to gun
				}

				if (m_Core.m_Pos.x > pChr->m_Pos.x - 80 && m_Core.m_Pos.x < pChr->m_Pos.x + 80)
				{
					if (Server()->Tick() % 20 == 0)
					{
						SetWeapon(0); //switch hammer in close range
					}
					m_Input.m_Fire++;
					m_LatestInput.m_Fire++;
				}
				else if (m_Core.m_Pos.x > pChr->m_Pos.x)
				{
					m_Input.m_Direction = -1;
				}
				else
				{
					m_Input.m_Direction = 1;
				}

				if (Server()->Tick() % 20 == 0)
				{
					if (rand() % 20 > 8)
					{
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
					}
				}

				if (m_Core.m_Pos.y > pChr->m_Pos.y - 5 * 32 && m_Core.m_Pos.y < pChr->m_Pos.y + 5 * 32) //same height
				{
					if (m_Core.m_Pos.x > pChr->m_Pos.x - 6 * 32 && m_Core.m_Pos.x < pChr->m_Pos.x + 6 * 32) //hook range
					{
						if (Server()->Tick() % 10 == 0)  //angry
						{
							GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9);
						}

						m_Input.m_Hook = 1;
						if (rand() % 6 == 0)
						{
							m_Input.m_Hook = 0;
						}
					}
					else if (m_Core.m_Pos.x > pChr->m_Pos.x - 15 * 32 && m_Core.m_Pos.x < pChr->m_Pos.x + 15 * 32) //combat range
					{

					}
					else if (m_Core.m_Pos.x > pChr->m_Pos.x) //move towards enemy from to right side <---
					{
						int rand_x_inp = (rand() % 60 + 15) * -1;
						int rand_y_inp = rand() % 100 * -1;

						m_Input.m_TargetX = rand_x_inp;
						m_Input.m_TargetY = rand_y_inp;
						m_LatestInput.m_TargetX = rand_x_inp;
						m_LatestInput.m_TargetY = rand_y_inp;

						if (m_Core.m_Vel.y > -0.6f)
						{
							m_Input.m_Hook = 1;
						}
						if (IsGrounded())
						{
							m_Input.m_Jump = 1;
						}
					}
					else if (m_Core.m_Pos.x < pChr->m_Pos.x) //move towards enemy from the left side ---->
					{
						int rand_x_inp = rand() % 60 + 15;
						int rand_y_inp = rand() % 100 * -1;

						m_Input.m_TargetX = rand_x_inp;
						m_Input.m_TargetY = rand_y_inp;
						m_LatestInput.m_TargetX = rand_x_inp;
						m_LatestInput.m_TargetY = rand_y_inp;

						if (m_Core.m_Vel.y > -0.6f)
						{
							m_Input.m_Hook = 1;
						}
						if (IsGrounded())
						{
							m_Input.m_Jump = 1;
						}
					}
				}
				else if (m_Core.m_Pos.y > pChr->m_Pos.y) //too low
				{
					int rand_x_inp = rand() % 60 - 30;
					int rand_y_inp = rand() % 120 * -1;
					m_Input.m_Hook = 1;

					m_Input.m_TargetX = rand_x_inp;
					m_Input.m_TargetY = rand_y_inp;
					m_LatestInput.m_TargetX = rand_x_inp;
					m_LatestInput.m_TargetY = rand_y_inp;


					if (rand() % 3 == 1)
					{
						m_Input.m_Hook = 0;
					}

					if (GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x, m_Core.m_Pos.y - 32) == 1) //collsion in the way
					{
						m_Input.m_Direction = 1;
						//m_pPlayer->m_TeeInfos.m_ColorBody = (120 * 255 / 360);
					}
				}
				else if (m_Core.m_Pos.y < pChr->m_Pos.y) //too high
				{
					int rand_x_inp = rand() % 60 - 30;
					int rand_y_inp = rand() % 120;
					m_Input.m_Hook = 1;

					m_Input.m_TargetX = rand_x_inp;
					m_Input.m_TargetY = rand_y_inp;
					m_LatestInput.m_TargetX = rand_x_inp;
					m_LatestInput.m_TargetY = rand_y_inp;


					if (rand() % 3 == 1)
					{
						m_Input.m_Hook = 0;
					}
				}



				//tile bypassing

				if (m_Core.m_Pos.y > pChr->m_Pos.y + 50) //too low
				{
					if (GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x, m_Core.m_Pos.y - 32) == 1 || GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x, m_Core.m_Pos.y + 32) == 3) //collsion in the way
					{
						m_Input.m_Direction = m_DummyDir;
						//m_pPlayer->m_TeeInfos.m_ColorBody = (120 * 255 / 360);
					}
				}
				else if (m_Core.m_Pos.y < pChr->m_Pos.y - 50) //high low
				{
					if (GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x, m_Core.m_Pos.y + 32) == 1 || GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x, m_Core.m_Pos.y + 32) == 3) //collsion in the way
					{
						m_Input.m_Direction = m_DummyDir;
						//m_pPlayer->m_TeeInfos.m_ColorBody = (120 * 255 / 360);
					}
				}
			}

			//avoid killtiles
			if (GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x - 60, m_Core.m_Pos.y) == 2 || GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x - 60, m_Core.m_Pos.y + 30) == 2) //deathtiles on the left side
			{
				m_Input.m_Direction = 1;
			}
			else if (GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x + 60, m_Core.m_Pos.y) == 2 || GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x + 60, m_Core.m_Pos.y + 30) == 2) //deathtiles on the right side
			{
				m_Input.m_Direction = -1;
			}
			if (!IsGrounded()) //flybot for fly parts (working in 10% of the cases)
			{
				if (GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x, m_Core.m_Pos.y + 5 * 32) == 2) //falling on killtiles
				{
					m_Input.m_TargetX = 2;
					m_LatestInput.m_TargetX = 2;
					m_Input.m_TargetY = -200;
					m_LatestInput.m_TargetY = -200;
					if (m_Core.m_Vel.y > 0.0f)
					{
						m_Input.m_Hook = 1;
					}
					else
					{
						m_Input.m_Hook = 0;
					}
				}
				
			}


			//check for stucking in walls
			if (m_Input.m_Direction != 0 && m_Core.m_Vel.x == 0.0f)
			{
				if (Server()->Tick() % 60 == 0)
				{
					m_Input.m_Jump = 1;
				}
			}

			//slow tick
			if (Server()->Tick() % 200 == 0)
			{
				//escape stuck hook
				m_Input.m_Hook = 0;

				//anti stuck whatever
				if (m_DummyDir == 1 && (GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x + 20, m_Core.m_Pos.y) == 1 || GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x + 20, m_Core.m_Pos.y) == 3))
				{
					m_DummyDir = -1;
				}
				else if (m_DummyDir == -1 && (GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x - 20, m_Core.m_Pos.y) == 1 || GameServer()->Collision()->GetCollisionAt(m_Core.m_Pos.x - 20, m_Core.m_Pos.y) == 3))
				{
					m_DummyDir = 1;
				}
				else
				{
					m_DummyDir = 1;
				}

				//end game if all humans dead
				if (GameServer()->m_survivalgamestate > 1) //survival game running
				{
					if (m_pPlayer->m_IsSurvivalAlive)
					{
						int AliveHumans = 0;
						for (int i = 0; i < MAX_CLIENTS; i++)
						{
							if (GameServer()->m_apPlayers[i] && !GameServer()->m_apPlayers[i]->m_IsDummy) //check all humans
							{
								if (GameServer()->m_apPlayers[i]->m_IsSurvivaling && GameServer()->m_apPlayers[i]->m_IsSurvivalAlive) // surival alive
								{
									AliveHumans++;
								}
							}
						}
						if (!AliveHumans) //all humans dead --> suicide to get new round running
						{
							Die(m_pPlayer->GetCID(), WEAPON_SELF);
							//dbg_msg("survival", "all humans dead suicide");
						}
						//else
						//{
						//	dbg_msg("survival","%d humans alive", AliveHumans);
						//}
					}
				}
			}
		}
        else if (m_pPlayer->m_DummyMode == 35) // weak singleplayer guardian
        {
            m_Input.m_Jump = 0;
            m_Input.m_Fire = 0;
            m_LatestInput.m_Fire = 0;
            m_Input.m_Hook = 0;
            m_Input.m_Direction = 0;
            
            CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, false, this);
            if (pChr && pChr->IsAlive())
            {
                m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
                m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y - 20;
                m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
                m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y - 20;
                
                // don't shoot walls
                if (!GameServer()->Collision()->IntersectLine(m_Pos, pChr->m_Pos, 0x0, 0))
                {
                    if (Server()->Tick() % 77 == 0 && m_FreezeTime < 1)
                    {
                        m_Input.m_Fire++;
                        m_LatestInput.m_Fire++;
                    }
                }
            }
        }
		else if (m_pPlayer->m_DummyMode == 99) // shop bot
		{
			m_Input.m_Jump = 0;
			m_Input.m_Fire = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Hook = 0;
			m_Input.m_Direction = 0;

			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, false, this);
			if (pChr && pChr->IsAlive() && pChr->m_InShop)
			{
				m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
				m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;
			}
		}
		else if (m_pPlayer->m_DummyMode == 103) //ctf5 pvp
		{
			m_Input.m_Jump = 0;
			m_Input.m_Fire = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Hook = 0;
			m_Input.m_Direction = 0;

			CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, false, this);
			if (pChr && pChr->IsAlive())
			{
				m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y - 20;
				m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
				m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y - 20;

				m_Input.m_Fire++;
				m_LatestInput.m_Fire++;

				if (m_Core.m_Pos.y - 4 * 32 > pChr->m_Pos.y)
				{
					if (Server()->Tick() % 20 == 0)
					{
						m_Input.m_Jump = 1;
					}
				}
				if (m_Core.m_Pos.x > pChr->m_Pos.x)
				{
					m_Input.m_Direction = -1;
					if (m_Core.m_Vel.x == 0.0f)
					{
						if (Server()->Tick() % 20 == 0)
						{
							m_Input.m_Jump = 1;
						}
					}
				}
				else
				{
					m_Input.m_Direction = 1;
					if (m_Core.m_Vel.x == 0.0f)
					{
						if (Server()->Tick() % 20 == 0)
						{
							m_Input.m_Jump = 1;
						}
					}
				}
			}
		}
		else if (m_pPlayer->m_DummyMode == 104) //blmapV5 lower areas blocker
		{
			m_Input.m_Jump = 0;
			m_Input.m_Fire = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Hook = 0;
			m_Input.m_Direction = 0;
			int offset_x = g_Config.m_SvDummyMapOffsetX * 32;
			//dbg_msg("debug","cfg=%d cfg*32=%d offset=%d mcore=%d offset+mcore=%d", g_Config.m_SvDummyMapOffsetX, g_Config.m_SvDummyMapOffsetX * 32, offset_x, m_Core.m_Pos.x, offset_x + m_Core.m_Pos.x);

			if (Server()->Tick() % 5000 == 0 && isFreezed)
			{
				Die(m_pPlayer->GetCID(), WEAPON_SELF);
			}

			if (m_Core.m_Pos.y < 38 * 32) //spawn
			{
				if (m_Core.m_Pos.x + offset_x < 13 * 32 + 10)
				{
					m_Input.m_Direction = 1;
				}
				else if (m_Core.m_Pos.x + offset_x > 14 * 32 + 16)
				{
					m_Input.m_Direction = -1;
					//dbg_msg("debug", "walking lefte because %d > %d", offset_x + m_Core.m_Pos.x, 14 * 32 + 16);
				}
				else
				{
					if (m_Core.m_Pos.x + offset_x > 1.2f)
					{
						m_Input.m_Direction = -1;
					}
					else if (m_Core.m_Pos.x + offset_x < -1.2f)
					{
						m_Input.m_Direction = 1;
					}
				}
			}
			else if (m_Core.m_Pos.y < 46 * 32) //block area 1
			{
				if (IsGrounded() || m_Core.m_Pos.x + offset_x > 14 * 32 + 10)
				{
					m_Input.m_Direction = -1;
				}
				if (m_Core.m_Pos.x + offset_x < 14 * 32 + 5 && m_Core.m_Pos.y < 40 * 32 + 30)
				{
					m_Input.m_Direction = 1;
				}
				if (Server()->Tick() % 60 == 0)
				{
					SetWeapon(3); //switch to grenade
				}
			}
			else if (m_Core.m_Pos.y < 61 * 32) //block area 2
			{
				if (m_Core.m_Pos.y < 52 * 32)
				{
					m_Input.m_Direction = -1;
				}
				else
				{
					m_Input.m_Direction = 1;
					m_Input.m_TargetX = -100;
					m_Input.m_TargetY = 19;
					m_LatestInput.m_TargetX = -100;
					m_LatestInput.m_TargetY = 19;
					if (m_FreezeTime == 0)
					{
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
					}
				}
			}
			else if (m_Core.m_Pos.y < 74 * 32) //block area 3
			{
				if (m_Core.m_Pos.x + offset_x > 16 * 32)
				{
					m_Input.m_Direction = -1;
					m_Input.m_TargetX = 100;
					m_Input.m_TargetY = 19;
					m_LatestInput.m_TargetX = 100;
					m_LatestInput.m_TargetY = 19;
					if (m_FreezeTime == 0)
					{
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
					}
				}
				else
				{
					m_Input.m_Direction = 1;
				}
			}
			else if (m_Core.m_Pos.y < 84 * 32) //block area 4
			{
				m_Input.m_Direction = 1;
				m_Input.m_TargetX = -200;
				m_Input.m_TargetY = 90;
				m_LatestInput.m_TargetX = -200;
				m_LatestInput.m_TargetY = 90;

				if (IsGrounded() && m_Core.m_Pos.x + offset_x > 23 * 32)
				{
					m_Input.m_Jump = 1;
				}
				if (m_Core.m_Vel.y < -0.05f && m_FreezeTime == 0)
				{
					m_Input.m_Fire++;
					m_LatestInput.m_Fire++;
				}

				if (m_Core.m_Pos.x + offset_x > 22 * 32 && m_Core.m_Vel.x < 7.1f)
				{
					m_Dummy104_rj_failed = true;
				}

				if (m_Dummy104_rj_failed)
				{
					m_Input.m_Direction = -1;
					if (m_Core.m_Pos.x + offset_x < 18 * 32)
					{
						m_Dummy104_rj_failed = false;
					}
				}
			}
			else //block area 5
			{

				if (m_Core.m_Pos.x + offset_x > 15 * 32 && m_Core.m_Pos.x + offset_x < 22 * 32) //never stay still over the middle freeze
				{
					m_Dummy104_panic_hook = true;
					if (m_Core.m_Pos.x + offset_x > 19 * 32)
					{
						m_Input.m_Direction = 1;
					}
					else
					{
						m_Input.m_Direction = -1;
						//dbg_msg("dummy","walk left to dodge the freeze");
					}
					if (m_Core.m_Pos.y > 92 * 32)
					{
						m_Input.m_Jump = 1;
						if (Server()->Tick() % 19 == 0)
						{
							m_Input.m_Jump = 0;
						}
					}
				}

				//block others:
				CCharacter *pChr = GameServer()->m_World.ClosestCharTypeUnfreezedArea5(m_Pos, false, this);
				if (pChr && pChr->IsAlive())
				{
					m_Input.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_Input.m_TargetY = pChr->m_Pos.y - m_Pos.y;
					m_LatestInput.m_TargetX = pChr->m_Pos.x - m_Pos.x;
					m_LatestInput.m_TargetY = pChr->m_Pos.y - m_Pos.y;

					if ((pChr->m_Core.m_Pos.y > m_Core.m_Pos.y + 64 || (m_Dummy104_panic_hook && pChr->m_Core.m_Pos.y < m_Core.m_Pos.y)) && //hook enemys up in freeze or hook enemys down to get speed up and avoid falkling in freeze
						!(m_Core.m_Pos.y < 88 * 32 && m_Core.m_Vel.y < -0.1f))   //but dont do it when the bot is looking but to do a roof nade
					{
						m_Input.m_Hook = 1;
					}

					if (m_Dummy104_angry)
					{
						if (pChr->m_Core.m_Pos.x + offset_x - 60 > m_Core.m_Pos.x + offset_x)
						{
							m_Input.m_Direction = 1;
						}
						else if (pChr->m_Core.m_Pos.x + offset_x - 40 < m_Core.m_Pos.x + offset_x)
						{
							m_Input.m_Direction = -1;
							//dbg_msg("dummy", "walk left to get better enemy positioning enemypos=%f", pChr->m_Core.m_Pos.x + offset_x);
						}
						else //near enough to hammer
						{
							if (Server()->Tick() % 8 == 0)
							{
								SetWeapon(0);
							}
							if (pChr->m_Core.m_Pos.y > m_Core.m_Pos.y && pChr->m_Core.m_Pos.y < m_Core.m_Pos.y + 50)
							{
								if (pChr->m_Core.m_Vel.y < -0.1f && m_FreezeTime == 0)
								{
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
								}
							}
						}

						if (IsGrounded())
						{
							int rand_val = rand() % 7;

							if (rand_val == 1)
							{
								m_Input.m_Jump = 1;
							}
							else if (rand_val == 2 || rand_val == 4 || rand_val == 6)
							{
								if (distance(pChr->m_Core.m_Pos, m_Core.m_Pos) < 60)
								{
									m_Input.m_Fire++;
									m_LatestInput.m_Fire++;
								}
							}
						}
					}
				}

				m_Dummy104_panic_hook = false;


				//angry checker
				if (m_Dummy104_angry)
				{
					if (m_FreezeTime == 0)
					{
						m_Dummy104_angry--;
					}
					if (Server()->Tick() % 10 == 0)  //angry emotes machen
					{
						GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9);
					}
				}
				if (m_Core.m_Vel.y < -1.2f && m_Core.m_Pos.y < 88 * 32)
				{
					if (Server()->Tick() % 10 == 0 && m_Dummy104_angry == 0)
					{
						m_Dummy104_angry = Server()->TickSpeed() * 20;
					}
				}

				if (m_Core.m_Pos.x + offset_x < 5 * 32) //lower left freeze
				{
					m_Input.m_Direction = 1;
				}

				if (m_Core.m_Pos.y > 90 * 32 + 16 && m_Core.m_Vel.y < -6.4f) //slow down speed with dj when flying to fast up
				{
					m_Input.m_Jump = 1;
				}

				if (m_Core.m_Pos.y < 89 * 32 - 20) //high
				{
					if (Server()->Tick() % 3 == 0)
					{
						SetWeapon(3);
					}
					if (m_Core.m_Pos.y < 88 * 32 && m_Core.m_Vel.y < -0.1f) // near roof
					{
						m_Input.m_TargetX = 1;
						m_Input.m_TargetY = -200;
						m_LatestInput.m_TargetX = 1;
						m_LatestInput.m_TargetY = -200;
						m_Input.m_Fire++;
						m_LatestInput.m_Fire++;
					}
				}

				//dont enter the freeze exit on the right side
				if (m_Core.m_Pos.x + offset_x > 34 * 32 && m_Core.m_Pos.y < 91 * 32 && m_Core.m_Vel.x > 0.0f)
				{
					m_Input.m_Direction = -1;
				}
			}
		}
		else if (m_pPlayer->m_DummyMode == 105) //blmapV5 upper blocker
		{
			m_Input.m_Jump = 0;
			m_Input.m_Fire = 0;
			m_LatestInput.m_Fire = 0;
			m_Input.m_Hook = 0;
			m_Input.m_Direction = 0;
			m_Input.m_TargetX = 45;
			m_Input.m_TargetY = 45;
			m_LatestInput.m_TargetX = 45;
			m_LatestInput.m_TargetY = 45;
			int bot_x = m_Core.m_Pos.x + g_Config.m_SvDummyMapOffsetX * 32;

			if (m_Core.m_Pos.y > 38 * 32) //too low
			{
				Die(m_pPlayer->GetCID(), WEAPON_SELF);
			}

			if (bot_x > 16 * 32 && m_Core.m_Pos.y < 27 * 32) //lovley wayblock spot
			{
			}
			else //not at lovley wayblock spot
			{
				if (Server()->Tick() % 420 == 0)
				{
					if (m_FreezeTime && IsGrounded()) //stuck ?
					{
						Die(m_pPlayer->GetCID(), WEAPON_SELF);
					}
				}


				if (m_Dummy105_move_left)
				{
					m_Input.m_Direction = -1;

					//failed?
					if (bot_x < 22 * 32)
					{
						m_Dummy105_move_left = false;
					}
				}
				else
				{
					m_Input.m_Direction = 1;

					if (bot_x > 10 * 32 && bot_x < 17 * 32) //jump in tunnel
					{
						m_Input.m_Jump = 1;
						if (Server()->Tick() % 20 == 0)
						{
							SetWeapon(3);
						}
					}

					if (bot_x > 28 * 32 && IsGrounded())
					{
						m_Input.m_Jump = 1;
					}

					if (m_Core.m_Pos.y < 31 * 32 - 10) //dont touch the roof
					{
						m_Input.m_Hook = 1;
					}
					if (bot_x > 35 * 32 && m_Core.m_Vel.x == 0.000000f) //hit the rocketjump wall
					{
						m_Input.m_Jump = 1;
						
						if (m_Core.m_Vel.y < -1.1f && m_FreezeTime == 0)
						{
							m_Input.m_Fire++;
							m_LatestInput.m_Fire++;
							m_Dummy105_move_left = true;
						}
					}
				}
			}
		}
		else // dummymode == end dummymode == last
		{
			m_pPlayer->m_DummyMode = 0;
		} //DUMMY END
	}
}

void CCharacter::MoveTee(int x, int y)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_Core.m_Pos.x += x;
	m_Core.m_Pos.y += y;
}

void CCharacter::ChillTelePort(int X, int Y)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_Core.m_Pos.x = X;
	m_Core.m_Pos.y = Y;
}

void CCharacter::ChillTelePortTile(int X, int Y)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_Core.m_Pos.x = X * 32;
	m_Core.m_Pos.y = Y * 32;
}

void CCharacter::FreezeAll(int seconds)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_aWeapons[weapon].m_Got)
	{
		return true;
	}
	return false;
}

void CCharacter::KillSpeed()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	m_Core.m_Vel.x = 0.0f;
	m_Core.m_Vel.y = 0.0f;
}

void CCharacter::InstagibKillingSpree(int KillerID, int Weapon)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
					GameServer()->CreateExplosion(pVictim->m_Pos, m_pPlayer->GetCID(), WEAPON_GRENADE, false, 0, m_pPlayer->GetCharacter()->Teams()->TeamMask(0));
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

				str_format(aBuf, sizeof(aBuf), "%s's killingspree was ended by %s (%d Kills)", Server()->ClientName(pVictim->GetPlayer()->GetCID()), Server()->ClientName(pKiller->GetCID()), pVictim->GetPlayer()->m_KillStreak);
				pVictim->GetPlayer()->m_KillStreak = 0;
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				GameServer()->CreateExplosion(pVictim->m_Pos, m_pPlayer->GetCID(), WEAPON_GRENADE, false, 0, m_pPlayer->GetCharacter()->Teams()->TeamMask(0));
			}

			if (pKiller != pVictim->GetPlayer())
			{
				if (!pVictim->GetPlayer()->m_IsDummy || pKiller->m_IsDummy)
				{
					pKiller->m_KillStreak++;
				}
				pVictim->GetPlayer()->m_KillStreak = 0;
				str_format(aBuf, sizeof(aBuf), "%s is on a killing spree with %d Kills!", Server()->ClientName(pKiller->GetCID()), pKiller->m_KillStreak);

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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	//Block points
	if (GameServer()->m_apPlayers[m_pPlayer->m_LastToucherID] && m_pPlayer->m_LastToucherID > -1 && m_FreezeTime > 0) //only if there is a toucher && the selfkiller was freeze
	{
		if (m_pPlayer->m_IsInstaMode_fng && !fngscore)
		{
			return Killer; //Killer = KilledID --> gets count as selfkill in score sys and not counted as kill (because only fng score tiles score)
		}

		if (m_pPlayer->m_LastToucherID != m_pPlayer->GetCID())
		{
			char aBuf[128];
			Killer = m_pPlayer->m_LastToucherID; //kill message
			if (!m_pPlayer->m_IsBlockWaving) //dont count block deaths in blockwave minigame
			{
				if (m_pPlayer->m_IsInstaArena_gdm)
				{
					//m_pPlayer->m_GrenadeDeaths++; //probably doesn't belong into blockmain but whatever //ye rly doesnt --> moved
				}
				else if (m_pPlayer->m_IsInstaArena_idm)
				{
					//m_pPlayer->m_RifleDeaths++; //probably doesn't belong into blockmain but whatever //ye rly doesnt --> moved
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
				if (!m_pPlayer->m_IsBlockWaving) //dont count block kills and points in blockwave minigame (would be too op lol)
				{
					if (m_pPlayer->m_IsDummy) //if dummy got killed make some exceptions
					{
						if (g_Config.m_SvDummyBlockPoints == 2 || (g_Config.m_SvDummyBlockPoints == 3 && GameServer()->IsPosition(Killer, 2))) //only count dummy kills if configt       cfg:3 block area or further count kills
						{
							if (Server()->Tick() >= m_AliveTime + Server()->TickSpeed() * g_Config.m_SvPointsFarmProtection)
							{
								GameServer()->GiveBlockPoints(Killer, 1);
							}
							GameServer()->m_apPlayers[Killer]->m_BlockPoints_Kills++;
						}
					}
					else
					{
						if (Server()->Tick() >= m_AliveTime + Server()->TickSpeed() * g_Config.m_SvPointsFarmProtection)
						{
							GameServer()->GiveBlockPoints(Killer, 1);
						}
						GameServer()->m_apPlayers[Killer]->m_BlockPoints_Kills++;
					}
				}

				if (GameServer()->m_apPlayers[m_pPlayer->m_LastToucherID]) //if killer(blocker) exists
				{
					if (g_Config.m_SvBlockBroadcast == 1)  //send kill message broadcast
					{
						str_format(aBuf, sizeof(aBuf), "%s was blocked by %s", Server()->ClientName(m_pPlayer->GetCID()), Server()->ClientName(m_pPlayer->m_LastToucherID));
						GameServer()->SendBroadcastAll(aBuf, 0);
					}

					//give xp reward to the blocker
					if (m_pPlayer->m_KillStreak > 4 && m_pPlayer->m_level <= m_pPlayer->m_max_level)
					{
						if (!GameServer()->m_apPlayers[m_pPlayer->m_LastToucherID]->m_HideBlockXp)
						{
							str_format(aBuf, sizeof(aBuf), "+%d xp for blocking '%s'", m_pPlayer->m_KillStreak, Server()->ClientName(m_pPlayer->GetCID()));
							GameServer()->SendChatTarget(m_pPlayer->m_LastToucherID, aBuf);
						}
						GameServer()->GiveXp(m_pPlayer->m_LastToucherID, m_pPlayer->m_KillStreak);
					}
					//bounty money reward to the blocker
					if (m_pPlayer->m_BlockBounty)
					{
						str_format(aBuf, sizeof(aBuf), "[BOUNTY] +%d money for blocking '%s'", m_pPlayer->m_BlockBounty, Server()->ClientName(m_pPlayer->GetCID()));
						GameServer()->SendChatTarget(m_pPlayer->m_LastToucherID, aBuf);
						str_format(aBuf, sizeof(aBuf), "+%d bounty (%s)", m_pPlayer->m_BlockBounty, Server()->ClientName(m_pPlayer->GetCID()));
						GameServer()->m_apPlayers[m_pPlayer->m_LastToucherID]->MoneyTransaction(+m_pPlayer->m_BlockBounty, aBuf);
						m_pPlayer->m_BlockBounty = 0;
					}
				}
				BlockQuestSubDieFuncBlockKill(Killer);
			}
		}
		else
		{
			dbg_msg("block", "WARNING '%s' [ID: %d] blocked himself", Server()->ClientName(m_pPlayer->GetCID()), m_pPlayer->GetCID());
		}
	}
	return Killer;
}

void CCharacter::BlockSpawnProt(int Killer)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (GameServer()->m_apPlayers[Killer])
	{
		char aBuf[128];
		//QUEST
		if (GameServer()->m_apPlayers[Killer]->m_QuestState == 1) //HAMMER QUEST
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
		else if (GameServer()->m_apPlayers[Killer]->m_QuestState == 2) //BLOCK QUEST
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
					//handled in killingspree system
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
		else if (GameServer()->m_apPlayers[Killer]->m_QuestState == 4) //RIFLE QUEST
		{
			if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 7) //Rifle <specific player> and then block him [LEVEL 7]
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
					//GameServer()->SendChatTarget(Killer, "[QUEST] wrong tee");
				}
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 8) //Rifle 5 tees before blocking them [LEVEL 8]
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
}

void CCharacter::BlockQuestSubDieFuncDeath(int Killer)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_pPlayer->m_QuestStateLevel == 9 && m_pPlayer->m_QuestState == 1)
	{
		GameServer()->QuestFailed(m_pPlayer->GetCID());
	}
}

void CCharacter::BlockKillingSpree(int Killer) //also used for intern sv_insta 0 minigames like gdm idm fng etc
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	char aBuf[128];
	//Somehow inspiration by //toast killingspree system by FruchtiHD and ChillerDragon stolen from twlevel (edited by ChillerDragon) //stolen from DDnet++ instagib and edited agian by ChillerDragon //rewritten by ChillerDragon cuz tw bug //upgraded to handle instagib agian
	CCharacter *pVictim = m_pPlayer->GetCharacter();
	//CPlayer *pKiller = GameServer()->m_apPlayers[Killer]; //removed pointer alien code and used the long way to have less bugsis with left players


	//dont count selfkills only count real being blocked as dead
	if (m_pPlayer->GetCID() == Killer)
	{
		//dbg_msg("SPREE", "didnt count selfkill [%d][%s]", Killer, Server()->ClientName(Killer));
		return;	
	}

	if (pVictim && GameServer()->m_apPlayers[Killer])
	{
		//##############################################
		// KILLED (blocked) or (gdm idm fng killed(NEW))
		//##############################################
		//Quest (leave it first because it doesnt reset something and needs the values)
		if (/*Killer != m_pPlayer->GetCID() &&*/ pVictim->GetPlayer()->m_QuestState == 2 && pVictim->GetPlayer()->m_QuestStateLevel == 7 && pVictim->GetPlayer()->m_QuestProgressValue > 0)
		{
			GameServer()->QuestFailed(pVictim->GetPlayer()->GetCID());
		}

		//#################
		// KILLED (blocked)
		//#################
		if (pVictim->GetPlayer()->m_KillStreak >= 5)
		{
			//Check for new highscore

			//could add fng sprees here too...
			if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm)
			{
				if (pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_GrenadeSpree)
				{
					pVictim->GetPlayer()->m_GrenadeSpree = pVictim->GetPlayer()->m_KillStreak;
					GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCID(), "New grenade spree record!");
				}
				str_format(aBuf, sizeof(aBuf), "'%s's grenade spree was ended by '%s' (%d Kills)", Server()->ClientName(pVictim->GetPlayer()->GetCID()), Server()->ClientName(GameServer()->m_apPlayers[Killer]->GetCID()), pVictim->GetPlayer()->m_KillStreak);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm)
			{
				if (pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_RifleSpree)
				{
					pVictim->GetPlayer()->m_RifleSpree = pVictim->GetPlayer()->m_KillStreak;
					GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCID(), "New rifle spree record!");
				}
				str_format(aBuf, sizeof(aBuf), "'%s's rifle spree was ended by '%s' (%d Kills)", Server()->ClientName(pVictim->GetPlayer()->GetCID()), Server()->ClientName(GameServer()->m_apPlayers[Killer]->GetCID()), pVictim->GetPlayer()->m_KillStreak);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_IsVanillaDmg)
			{
				str_format(aBuf, sizeof(aBuf), "'%s's killing spree was ended by '%s' (%d Kills)", Server()->ClientName(pVictim->GetPlayer()->GetCID()), Server()->ClientName(GameServer()->m_apPlayers[Killer]->GetCID()), pVictim->GetPlayer()->m_KillStreak);
			}
			else //no insta at all
			{
				if (pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_BlockSpreeHighscore)
				{
					pVictim->GetPlayer()->m_BlockSpreeHighscore = pVictim->GetPlayer()->m_KillStreak;
					GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCID(), "New Blockspree record!");
				}
				str_format(aBuf, sizeof(aBuf), "'%s's blocking spree was ended by '%s' (%d Blocks)", Server()->ClientName(pVictim->GetPlayer()->GetCID()), Server()->ClientName(GameServer()->m_apPlayers[Killer]->GetCID()), pVictim->GetPlayer()->m_KillStreak);
			}


			pVictim->GetPlayer()->m_KillStreak = 0; 
			GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			//dbg_msg("cBug", "SendChat(-1) blocking spree ended");
			GameServer()->CreateExplosion(pVictim->m_Pos, m_pPlayer->GetCID(), WEAPON_GRENADE, false, 0, m_pPlayer->GetCharacter()->Teams()->TeamMask(0));
		}

		//#################
		// KILLER (blocker)
		//#################
		if (GameServer()->CountIngameHumans() >= g_Config.m_SvSpreePlayers) //only count killing sprees if enough players are online and ingame (alive)
		{
			//if (GameServer()->m_apPlayers[Killer] != pVictim->GetPlayer())
			{
				if ((pVictim->GetPlayer()->m_IsDummy && g_Config.m_SvSpreeCountBots) ||  //only count bots if configurated
					(!pVictim->GetPlayer()->m_IsDummy)) //count all humans in killingsprees
				{
					GameServer()->m_apPlayers[Killer]->m_KillStreak++;
				}

				str_format(aBuf, sizeof(aBuf), "'%s' is on a %d spree!", Server()->ClientName(GameServer()->m_apPlayers[Killer]->GetCID()), GameServer()->m_apPlayers[Killer]->m_KillStreak); //if something wents wrong this is the general backup message

				if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_fng && (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm || GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm))
				{
					if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm)
					{
						str_format(aBuf, sizeof(aBuf), "'%s' is on a boomfng spree with %d kills!", Server()->ClientName(GameServer()->m_apPlayers[Killer]->GetCID()), GameServer()->m_apPlayers[Killer]->m_KillStreak);
					}
					else if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm)
					{
						str_format(aBuf, sizeof(aBuf), "'%s' is on a fng spree with %d kills!", Server()->ClientName(GameServer()->m_apPlayers[Killer]->GetCID()), GameServer()->m_apPlayers[Killer]->m_KillStreak);
					}
				}
				else if (!GameServer()->m_apPlayers[Killer]->m_IsInstaArena_fng && (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm || GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm))
				{
					if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm)
					{
						str_format(aBuf, sizeof(aBuf), "'%s' is on a grenade spree with %d kills!", Server()->ClientName(GameServer()->m_apPlayers[Killer]->GetCID()), GameServer()->m_apPlayers[Killer]->m_KillStreak);
					}
					else if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm)
					{
						str_format(aBuf, sizeof(aBuf), "'%s' is on a rifle spree with %d kills!", Server()->ClientName(GameServer()->m_apPlayers[Killer]->GetCID()), GameServer()->m_apPlayers[Killer]->m_KillStreak);
					}
				}
				else if (GameServer()->m_apPlayers[Killer]->m_IsVanillaDmg)
				{
					str_format(aBuf, sizeof(aBuf), "'%s' is on a killing spree with %d kills!", Server()->ClientName(GameServer()->m_apPlayers[Killer]->GetCID()), GameServer()->m_apPlayers[Killer]->m_KillStreak);
				}
				else //no insta at all
				{
					str_format(aBuf, sizeof(aBuf), "'%s' is on a blocking spree with %d blocks!", Server()->ClientName(GameServer()->m_apPlayers[Killer]->GetCID()), GameServer()->m_apPlayers[Killer]->m_KillStreak);
				}


				if (GameServer()->m_apPlayers[Killer]->m_KillStreak % 5 == 0 && GameServer()->m_apPlayers[Killer]->m_KillStreak >= 5)
				{
					GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
					//dbg_msg("cBug", "SendChat(-1) blocking spree status");
					//dbg_msg("cBug", "msg: %s", aBuf);
				}
			}
		}
		else //not enough players
		{
			//dbg_msg("spree", "not enough tees %d/%d spree (%d)", GameServer()->CountConnectedPlayers(), g_Config.m_SvSpreePlayers, GameServer()->m_apPlayers[Killer]->m_KillStreak);
			if ((pVictim->GetPlayer()->m_IsDummy && g_Config.m_SvSpreeCountBots) ||  //only count bots if configurated
				(!pVictim->GetPlayer()->m_IsDummy)) //count all humans in killingsprees
			{
				GameServer()->m_apPlayers[Killer]->m_KillStreak++;
				//dbg_msg("cBug", "(not enough tees) still increment streak to %d", GameServer()->m_apPlayers[Killer]->m_KillStreak);
			}

			if (GameServer()->m_apPlayers[Killer]->m_KillStreak == 5)
			{
				str_format(aBuf, sizeof(aBuf), "[SPREE] %d/%d humans alive to start a spree.", GameServer()->CountIngameHumans(), g_Config.m_SvSpreePlayers);
				GameServer()->SendChatTarget(GameServer()->m_apPlayers[Killer]->GetCID(), aBuf);
				GameServer()->m_apPlayers[Killer]->m_KillStreak = 0; //reset killstreak to avoid some1 collecting 100 kills with dummy and then if player connect he could save the spree
			}
		}
		//Quest (external because it has nothing to do with spree needed players)
		if (/*Killer != m_pPlayer->GetCID() &&*/ GameServer()->m_apPlayers[Killer]->m_QuestState == 2 && GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 7)
		{
			GameServer()->QuestAddProgress(GameServer()->m_apPlayers[Killer]->GetCID(), 11);
		}
	}
	else if (pVictim) //if killer left the game
	{
		//dbg_msg("spree", "Killer left the game");
		//Quest (leave it first because it doesnt reset something and needs the values)
		if (pVictim->GetPlayer()->m_QuestState == 2 && pVictim->GetPlayer()->m_QuestStateLevel == 7 && pVictim->GetPlayer()->m_QuestProgressValue > 0)
		{
			GameServer()->QuestFailed(pVictim->GetPlayer()->GetCID());
		}
		if (pVictim->GetPlayer()->m_KillStreak >= 5)
		{
			//Check for new highscore
			if (pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_BlockSpreeHighscore)
			{
				pVictim->GetPlayer()->m_BlockSpreeHighscore = pVictim->GetPlayer()->m_KillStreak;
				GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCID(), "New Blockspree record!");
			}

			//                                                                                                                   -------------> "hier koennte ihre werbung stehen" <--------------
			str_format(aBuf, sizeof(aBuf), "'%s's blockingspree was ended by a player who left the game (%d Blocks)", Server()->ClientName(pVictim->GetPlayer()->GetCID())/*, "(left the game)"*/, pVictim->GetPlayer()->m_KillStreak);
			pVictim->GetPlayer()->m_KillStreak = 0;
			GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			GameServer()->CreateExplosion(pVictim->m_Pos, m_pPlayer->GetCID(), WEAPON_GRENADE, false, 0, m_pPlayer->GetCharacter()->Teams()->TeamMask(0));
		}
	}
	pVictim->GetPlayer()->m_KillStreak = 0; //Important always clear killingspree of ded dude
}

void CCharacter::CITick()
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	//pythagoras mate u rock c:
	//a²+b²=c²
	int a = m_Core.m_Pos.x - g_Config.m_SvCIdestX;
	int b = m_Core.m_Pos.y - g_Config.m_SvCIdestY;
	//|a| |b|
	a = abs(a);
	b = abs(b);

	int c = sqrt(a + b);

	return c;
}

void CCharacter::SurvivalSubDieFunc(int Killer, int weapon)
{
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
	if (m_pPlayer->m_IsSurvivalAlive && GameServer()->m_apPlayers[Killer]->m_IsSurvivalAlive) //ignore lobby and stuff
	{
		char aBuf[128];
		//=== DEATHS and WINCHECK ===
		if (m_pPlayer->m_IsSurvivaling)
		{
			if (GameServer()->m_survivalgamestate > 1) //if game running
			{
				GameServer()->SetPlayerSurvival(m_pPlayer->GetCID(), 3); //set player to dead
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[SURVIVAL] you lost the round.");
				int AliveTees = GameServer()->CountSurvivalPlayers(true);
				if (AliveTees < 2) //could also be == 1 but i think < 2 is saver. Check for winning.                        (much wow sentence inc..) if 2 were alive and now only 1 players alive and one dies we have a winner
				{
					//GameServer()->SendSurvivalChat("[SURVIVAL] Good Game some1 won!");
					if (!GameServer()->SurvivalPickWinner()) { GameServer()->SendSurvivalChat("[SURVIVAL] Nobody won."); }
					GameServer()->SurvivalSetGameState(1);
				}
				else if (AliveTees < g_Config.m_SvSurvivalDmPlayers)
				{
					GameServer()->SurvivalSetGameState(3); //dm count down tick
					str_format(aBuf, sizeof(aBuf), "[SURVIVAL] deathmatch starts in %d minutes", GameServer()->m_survival_dm_countdown / (Server()->TickSpeed() * 60));
					GameServer()->SendSurvivalChat(aBuf);
				}
			}
		}

		//=== KILLS ===
		if (Killer != m_pPlayer->GetCID()) // don't count selfkills as kills
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
				if (g_Config.m_SvDDPPgametype == 5)
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
#if defined(CONF_DEBUG)
	CALL_STACK_ADD();
#endif
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
