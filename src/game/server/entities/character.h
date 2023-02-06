/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_CHARACTER_H
#define GAME_SERVER_ENTITIES_CHARACTER_H

#include <game/server/entity.h>
#include <game/server/save.h>

#include <game/gamecore.h>

// ddnet++
#include "drop_pickup.h"
#include "dummy/adventure.h"
#include "dummy/blmapchill_police.h"
#include "dummy/blmapv3_arena.h"
#include "dummy/blmapv5_lower_blocker.h"
#include "dummy/blmapv5_upper_blocker.h"
#include "dummy/blockwave.h"
#include "dummy/chillblock5_balance.h"
#include "dummy/chillblock5_blocker.h"
#include "dummy/chillblock5_blocker_tryhard.h"
#include "dummy/chillblock5_police.h"
#include "dummy/chillblock5_race.h"
#include "dummy/ctf5_pvp.h"
#include "dummy/fnn.h"
#include "dummy/grenade_fng.h"
#include "dummy/quest.h"
#include "dummy/rifle_fng.h"
#include "dummy/sample.h"
#include "dummy/shopbot.h"
#include "dummy/survival.h"
#include "weapon.h"
#include <deque>
#include <game/generated/server_data.h>
#include <game/server/entities/stable_projectile.h>
#include <vector>

#define NUM_ATOMS 6
#define NUM_TRAILS 20
#define TRAIL_DIST 20

class CAntibot;
class CGameTeams;
class CGameWorld;
class IAntibot;
struct CAntibotCharacterData;

enum
{
	FAKETUNE_FREEZE = 1,
	FAKETUNE_SOLO = 2,
	FAKETUNE_NOJUMP = 4,
	FAKETUNE_NOCOLL = 8,
	FAKETUNE_NOHOOK = 16,
	FAKETUNE_JETPACK = 32,
	FAKETUNE_NOHAMMER = 64,
};

class CCharacter : public CEntity
{
#define IN_CLASS_CHARACTER
#include "character_ddpp.h"
	MACRO_ALLOC_POOL_ID()

	friend class CSaveTee; // need to use core

public:
	CCharacter(CGameWorld *pWorld, CNetObj_PlayerInput LastInput);

	void Reset() override;
	void Destroy() override;
	void PreTick();
	void Tick() override;
	void TickDeferred() override;
	void TickPaused() override;
	void Snap(int SnappingClient) override;
	void SwapClients(int Client1, int Client2) override;

	bool CanSnapCharacter(int SnappingClient);

	bool IsGrounded();

	void SetWeapon(int W);
	void SetJetpack(bool Active);
	void SetSolo(bool Solo);
	void SetSuper(bool Super);
	void SetLiveFrozen(bool Active);
	void SetDeepFrozen(bool Active);
	void HandleWeaponSwitch();
	void DoWeaponSwitch();

	void HandleWeapons();
	void HandleNinja();
	void HandleJetpack();
	void HandlePullHammer();
	void HandleKickHammer();

	void OnPredictedInput(CNetObj_PlayerInput *pNewInput);
	void OnDirectInput(CNetObj_PlayerInput *pNewInput);
	void ResetHook();
	void ResetInput();
	void FireWeapon();

	void Die(int Killer, int Weapon, bool fngscore = false);
	bool TakeDamage(vec2 Force, int Dmg, int From, int Weapon);

	bool Spawn(class CPlayer *pPlayer, vec2 Pos);
	bool Remove();

	bool IncreaseHealth(int Amount);
	bool IncreaseArmor(int Amount);

	bool GiveWeapon(int Weapon, bool Remove = false, int Ammo = -1);
	void GiveNinja();
	void RemoveNinja();
	void SetEndlessHook(bool Enable);

	void SetEmote(int Emote, int Tick);

	void Rescue();

	int NeededFaketuning() { return m_NeededFaketuning; }
	bool IsAlive() const { return m_Alive; }
	bool IsPaused() const { return m_Paused; }
	class CPlayer *GetPlayer() { return m_pPlayer; }
	int64_t TeamMask();

	void SetPosition(vec2 Pos) { m_Core.m_Pos = Pos; }

	bool isFreezed;

	int m_NeededFaketuning; // sowwy ChillerDragon made it public

private:
	// player controlling this character
	class CPlayer *m_pPlayer;

	bool m_Alive;
	bool m_Paused;

	// weapon info
	CEntity *m_apHitObjects[10];
	int m_NumObjectsHit;

	int m_LastWeapon;
	int m_QueuedWeapon;

	int m_ReloadTimer;
	int m_AttackTick;

	int m_DamageTaken;
	//bool isFreezed; //ChillerDragon moved this public

	int m_EmoteType;
	int m_EmoteStop;

	// last tick that the player took any action ie some input
	int m_LastAction;
	int m_LastNoAmmoSound;

	// these are non-heldback inputs
	CNetObj_PlayerInput m_LatestPrevPrevInput;
	CNetObj_PlayerInput m_LatestPrevInput;
	CNetObj_PlayerInput m_LatestInput;

	// input
	CNetObj_PlayerInput m_PrevInput;
	CNetObj_PlayerInput m_Input;
	CNetObj_PlayerInput m_SavedInput;
	int m_NumInputs;

	int m_DamageTakenTick;

	int m_Health;
	int m_Armor;

	// the player core for the physics
	CCharacterCore m_Core;
	CGameTeams *m_pTeams = nullptr;

	std::map<int, std::vector<vec2>> *m_pTeleOuts = nullptr;
	std::map<int, std::vector<vec2>> *m_pTeleCheckOuts = nullptr;

	// info for dead reckoning
	int m_ReckoningTick; // tick that we are performing dead reckoning From
	CCharacterCore m_SendCore; // core that we should send
	CCharacterCore m_ReckoningCore; // the dead reckoning core

	// DDRace

	void SnapCharacter(int SnappingClient, int ID);
	static bool IsSwitchActiveCb(int Number, void *pUser);
	void SetTimeCheckpoint(int TimeCheckpoint);
	void HandleTiles(int Index);
	float m_Time;
	int m_LastBroadcast;
	void DDRaceInit();
	void HandleSkippableTiles(int Index);
	void SetRescue();
	void DDRaceTick();
	void DDRacePostCoreTick();
	void HandleBroadcast();
	void HandleTuneLayer();
	void SendZoneMsgs();
	IAntibot *Antibot();

	bool m_SetSavePos;
	CSaveTee m_RescueTee;

public:
	CGameTeams *Teams() { return m_pTeams; }
	void SetTeams(CGameTeams *pTeams);
	void SetTeleports(std::map<int, std::vector<vec2>> *pTeleOuts, std::map<int, std::vector<vec2>> *pTeleCheckOuts);

	void FillAntibot(CAntibotCharacterData *pData);
	void Pause(bool Pause);
	bool Freeze(int Seconds);
	bool Freeze();
	bool UnFreeze();
	void GiveAllWeapons();
	void ResetPickups();
	int m_DDRaceState;
	int Team();
	bool CanCollide(int ClientID);
	bool SameTeam(int ClientID);
	bool m_NinjaJetpack;
	int m_TeamBeforeSuper;
	int m_FreezeTime;
	bool m_FrozenLastTick;
	bool m_FreezeHammer;
	bool m_PullHammer;
	bool m_KickHammer;
	bool m_Invisible;
	int m_PullingID;
	bool m_IsFiring;
	int m_TuneZone;
	int m_TuneZoneOld;
	int m_PainSoundTimer;
	int m_LastMove;
	int m_StartTime;
	vec2 m_PrevPos;
	int m_TeleCheckpoint;

	int m_TimeCpBroadcastEndTick;
	int m_LastTimeCp;
	int m_LastTimeCpBroadcasted;
	float m_aCurrentTimeCp[MAX_CHECKPOINTS];

	int m_TileIndex;
	int m_TileFIndex;

	int m_MoveRestrictions;

	vec2 m_Intersection;
	int64_t m_LastStartWarning;
	int64_t m_LastRescue;
	bool m_LastRefillJumps;
	bool m_LastPenalty;
	bool m_LastBonus;
	vec2 m_TeleGunPos;
	bool m_TeleGunTeleport;
	bool m_IsBlueTeleGunTeleport;
	int m_StrongWeakID;

	int m_SpawnTick;
	int m_WeaponChangeTick;

	// Setters/Getters because i don't want to modify vanilla vars access modifiers
	int GetLastWeapon() { return m_LastWeapon; }
	void SetLastWeapon(int LastWeap) { m_LastWeapon = LastWeap; }
	int GetActiveWeapon() { return m_Core.m_ActiveWeapon; }
	void SetActiveWeapon(int ActiveWeap) { m_Core.m_ActiveWeapon = ActiveWeap; }
	void SetLastAction(int LastAction) { m_LastAction = LastAction; }
	int GetArmor() { return m_Armor; }
	void SetArmor(int Armor) { m_Armor = Armor; }
	CCharacterCore GetCore() { return m_Core; }
	void SetCore(CCharacterCore Core) { m_Core = Core; }
	CCharacterCore *Core() { return &m_Core; }
	bool GetWeaponGot(int Type) { return m_Core.m_aWeapons[Type].m_Got; }
	void SetWeaponGot(int Type, bool Value) { m_Core.m_aWeapons[Type].m_Got = Value; }
	int GetWeaponAmmo(int Type) { return m_Core.m_aWeapons[Type].m_Ammo; }
	void SetWeaponAmmo(int Type, int Value) { m_Core.m_aWeapons[Type].m_Ammo = Value; }
	void SetNinjaActivationDir(vec2 ActivationDir) { m_Core.m_Ninja.m_ActivationDir = ActivationDir; }
	void SetNinjaActivationTick(int ActivationTick) { m_Core.m_Ninja.m_ActivationTick = ActivationTick; }
	void SetNinjaCurrentMoveTime(int CurrentMoveTime) { m_Core.m_Ninja.m_CurrentMoveTime = CurrentMoveTime; }

	int GetLastAction() const { return m_LastAction; }

	bool HasTelegunGun() { return m_Core.m_HasTelegunGun; }
	bool HasTelegunGrenade() { return m_Core.m_HasTelegunGrenade; }
	bool HasTelegunLaser() { return m_Core.m_HasTelegunLaser; }

	bool HammerHitDisabled() { return m_Core.m_HammerHitDisabled; }
	bool ShotgunHitDisabled() { return m_Core.m_ShotgunHitDisabled; }
	bool LaserHitDisabled() { return m_Core.m_LaserHitDisabled; }
	bool GrenadeHitDisabled() { return m_Core.m_GrenadeHitDisabled; }

	bool IsSuper() { return m_Core.m_Super; }

	CSaveTee &GetRescueTeeRef() { return m_RescueTee; }
};

enum
{
	DDRACE_NONE = 0,
	DDRACE_STARTED,
	DDRACE_CHEAT, // no time and won't start again unless ordered by a mod or death
	DDRACE_FINISHED
};

#endif
