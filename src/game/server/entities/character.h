/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_CHARACTER_H
#define GAME_SERVER_ENTITIES_CHARACTER_H

#include <deque>
#include <game/server/entity.h>
#include <game/server/entities/stable_projectile.h>
#include <game/generated/server_data.h>
#include <game/generated/protocol.h>

#include <game/gamecore.h>

#include <vector>
#include "dummy/blmapchill_police.h"

#include "weapon.h"
#include "drop_pickup.h"

#define NUM_ATOMS 6
#define NUM_TRAILS 20
#define TRAIL_DIST 20

#define FNN_MOVE_LEN 32768

#define V3_OFFSET_X 0 * 32 //was 277
#define V3_OFFSET_Y 0 * 32 //was 48

class CGameTeams;

enum
{
	WEAPON_GAME = -3, // team switching etc
	WEAPON_SELF = -2, // console kill command
	WEAPON_WORLD = -1, // death tiles etc
};

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
	MACRO_ALLOC_POOL_ID()

	friend class CSaveTee; // need to use core

public:
	//character's size
	static const int ms_PhysSize = 28;

	CCharacter(CGameWorld *pWorld);
	~CCharacter();

	virtual void Reset();
	virtual void Destroy();
	virtual void Tick();
	virtual void TickDefered();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);
	virtual int NetworkClipped(int SnappingClient);
	virtual int NetworkClipped(int SnappingClient, vec2 CheckPos);

	bool IsGrounded();

	void SetWeapon(int W);
	void SetSolo(bool Solo);
	void HandleWeaponSwitch();
	void DoWeaponSwitch();

	void HandleWeapons();
	void HandleNinja();
	void HandleJetpack();

	void OnPredictedInput(CNetObj_PlayerInput *pNewInput);
	void OnDirectInput(CNetObj_PlayerInput *pNewInput);
	void ResetInput();
	void FireWeapon(bool Bot = false);

	void Die(int Killer, int Weapon, bool fngscore = false);
	bool TakeDamage(vec2 Force, int Dmg, int From, int Weapon);

	bool Spawn(class CPlayer *pPlayer, vec2 Pos);
	bool Remove();

	bool IncreaseHealth(int Amount);
	bool IncreaseArmor(int Amount);

	bool GiveWeapon(int Weapon, bool Remove = false, int Ammo = -1);
	void GiveNinja();
	void RemoveNinja();

	void SetEmote(int Emote, int Tick);

	void Rescue();

	int NeededFaketuning() {return m_NeededFaketuning;}
	bool IsAlive() const { return m_Alive; } 
	bool IsPaused() const { return m_Paused; }
	class CPlayer *GetPlayer() { return m_pPlayer; }

	bool m_IsSpecHF;
	void SetPosition(vec2 Pos) { m_Core.m_Pos = Pos; }
	vec2 GetPosition() { return m_Core.m_Pos;  } //proudly mede by ChillerDragon
	void TakeHammerHit(CCharacter* pFrom); //ddpp implemented from fng2

	bool isFreezed;
	bool m_OnFire;

	int m_NeededFaketuning; // sowwy ChillerDragon made it public

private:
	// player controlling this character
	class CPlayer *m_pPlayer;

	bool m_Alive;
	bool m_Paused;

	// weapon info
	CEntity *m_apHitObjects[10];
	int m_NumObjectsHit;

	struct WeaponStat
	{
		int m_AmmoRegenStart;
		int m_Ammo;
		int m_Ammocost;
		bool m_Got;

	} m_aWeapons[NUM_WEAPONS];

	int m_aWeaponsBackup[NUM_WEAPONS][2];
	bool m_WeaponsBackupped;

	int m_LastWeapon;
	int m_QueuedWeapon;

	int m_ReloadTimer; // moved to public for onfire mode
	int m_AttackTick;

	int m_DamageTaken;
	//bool isFreezed; //ChillerDragon moved this public

	int m_EmoteType;
	int m_EmoteStop;

	// last tick that the player took any action ie some input
	int m_LastAction;
	int m_LastNoAmmoSound;

	// these are non-heldback inputs
	CNetObj_PlayerInput m_LatestPrevInput;
	CNetObj_PlayerInput m_LatestInput;

	// input
	CNetObj_PlayerInput m_PrevInput;
	CNetObj_PlayerInput m_Input;
	CNetObj_PlayerInput m_SavedInput;
	int m_NumInputs;
	int m_Jumped;

	int m_DamageTakenTick;

	int m_Health;
	int m_Armor;

	// ninja
	struct
	{
		vec2 m_ActivationDir;
		int m_ActivationTick;
		int m_CurrentMoveTime;
		int m_OldVelAmount;
	} m_Ninja;

	// the player core for the physics
	CCharacterCore m_Core;

	// info for dead reckoning
	int m_ReckoningTick; // tick that we are performing dead reckoning From
	CCharacterCore m_SendCore; // core that we should send
	CCharacterCore m_ReckoningCore; // the dead reckoning core

	// DDRace


	void HandleTiles(int Index);
	float m_Time;
	int m_LastBroadcast;
	void DDRaceInit();
	void HandleSkippableTiles(int Index);
	void DDRaceTick();
	void DDRacePostCoreTick();
	void HandleBroadcast();
	void HandleTuneLayer();
	void SendZoneMsgs();

	bool m_SetSavePos;
	vec2 m_PrevSavePos;

	/*

		DDNet++

	*/

	void HandleTilesDDPP(int Index, int Tile1, int Tile2, int Tile3, int Tile4, int FTile1, int FTile2, int FTile3, int FTile4, int MapIndexL, int MapIndexR, int MapIndexT, int MapIndexB);
	void DDPPDDRacePostCoreTick();

	// also: es gibt eine start- und endposition f�r die augen
	// ebenso wie eine startzeit und eine endzeit (bzw. eigentlich nur wie lange die animation geht)

	// z.B. startzeit waere jetzt, die zeit wie lange die animation geht w�re 1 sekunde
	// dann waere m_AngleTickStart = Server()->Tick() // = die jetzige zeit
	// und m_AngleTickTime = Server()->TickSpeed() // eine sekunde

	// m_AngleFrom ist die startposition, also z.B. 0(grad) (dann guckt er glaub ich nach rechts)
	// m_AngleTo dann z.B. 90(grad) (guckt nach unten)
	// d.h. er dreht sich von jetzt bis in einer sekunde um 90(grad)
	float m_AngleFrom; //
	float m_AngleTo;
	int m_AngleTickStart;
	int m_AngleTickTime;
	int m_AngleTickNext; // und das ist die zeit, wann die n�chste animation beginnen soll

	int m_EmoteTickNext;
	int m_EmoteTickNextFast;
	int m_HookTickNext;
	//int m_AimbotSwitchTick;
	bool m_AimbotMode; // wenn true, dann ist aimbot aktiviert, bei false halt nicht wozu? damit man weiss ob der bot den aimbot an hat oder halt random durch die gegend guckt? ne brauchts eig nich oder? lass erstmal ohne
	bool m_MoveMode;
	bool m_LeftMM;
	
	int m_DummyFinishes;


	bool m_DummyShowRank;
	bool m_DummyFinished;
	bool m_DummyFreezed;
	bool m_DummyHammer;

	bool m_getxp;

	int m_FreezeKillNext;
	int m_MoveTick;
	int m_LastMoveDirection;
	int m_StopMoveTick;
	bool m_IsFreeShopBot;
	void ClearFakeMotd();
	void SendShopMessage(const char *pMsg);

public:
	CGameTeams* Teams();

	bool m_taxi;
	bool m_WasInRoom;

	void DDPP_Tick();
	void DDPP_FlagTick();
	int DDPP_DIE(int Killer, int Weapon, bool fngscore = false);
	void BlockTourna_Die(int Killer);
	void DummyTick();
	void PvPArenaTick();

	//usefull everywhere
	void DDPP_TakeDamageInstagib(int Dmg, int From, int Weapon);
	void MoveTee(int x, int y);
	void ChillTelePort(float X, float Y);
	void ChillTelePortTile(int X, int Y);
	void FreezeAll(int seconds);
	bool HasWeapon(int weapon);
	void KillSpeed();
	int GetAimDir();
	bool InputActive();

	//Chillintelligenz
	void CITick();
	void CIRestart();
	int CIGetDestDist();

	int m_ci_freezetime;

	//Block
	int BlockPointsMain(int Killer, bool fngscore = false);
	void BlockSpawnProt(int Killer);
	void BlockQuestSubDieFuncBlockKill(int Killer);
	void BlockQuestSubDieFuncDeath(int Killer);

	/*
		QuestHammerHit

		gets called on hammer hit
	*/
	void QuestHammerHit(CCharacter *pTarget);
	void QuestShotgun();
	void QuestGrenade();
	void QuestRifle();
	void QuestNinja();
	void QuestFireWeapon();
	void DDPPFireWeapon();
	void PoliceHammerHit(CCharacter *pTarget);
	void DDPPHammerHit(CCharacter *pTarget);
	bool IsHammerBlocked();
	void DDPPGunFire(vec2 Direction);
	bool SpecialGunProjectile(vec2 Direction, vec2 ProjStartPos, int Lifetime);
	bool FreezeShotgun(vec2 Direction, vec2 ProjStartPos);

	/*
		KillingSpree

		was called BlockKillingSpree once
		but now handles all ddnet++ gametype sprees.
		So block and minigames (fng/vanilla).
		But not other gametypes as instagib or fng (sv_gametype or other server cfgs).
	*/
	void KillingSpree(int Killer);

	//block tourna

	int m_BlockTournaDeadTicks;

	//blockwave
	int BlockWaveFreezeTicks;

	//survival
	void SurvivalSubDieFunc(int Killer, int weapon);

	//instagib 
	int m_SpreeTimerState; //0 = ready 1 = running (i know could be bool for now but maybe ill add different modes like count from spawn or count from first kill)
	void InstagibSubDieFunc(int Killer, int Weapon);
	void InstagibKillingSpree(int KillerID, int Weapon);
	bool m_UpdateInstaScoreBoard;

	void Pause(bool Pause);
	bool ForceFreeze(int Seconds); //mede by ChillerDragon too freeze no matter what used for freezing while freezed (for example for tournaments to have all same freeze time even if some wer freezed at tourna start)
	//WARNING FORCE FREEZE ISNT ABLE TO OVERWRITE FREEZE AS IT SHOULD!!!
	//it can still be used to bypass super but thats all i guess
	bool Freeze(float Time);
	bool Freeze(); 
	bool UnFreeze();
	void MoneyTile();
	void MoneyTilePolice();
	void MoneyTilePlus();
	void MoneyTileDouble();
	void GiveAllWeapons();
	int m_DDRaceState;
	int Team();
	bool CanCollide(int ClientID);
	bool SameTeam(int ClientID);
	bool m_Super;
	bool m_SuperJump;
	int m_survivexpvalue;
	bool m_hammerfight; //used for the rcon command has nothing todo with arenas yet
	//bool m_IsHammerarena; //used for chillerdragons hammerfight arena '/hammerfight'
	//bool m_Hammerarena_exit_request;
	//int m_Hammerarena_exit_request_time;
	bool m_IsPVParena;
	bool m_pvp_arena_exit_request;
	int m_pvp_arena_tele_request_time;
	bool m_isHeal;
	bool m_Jetpack;
	bool m_freezeShotgun;
	bool m_FreezeLaser;
	bool m_DestroyLaser;
	bool m_isDmg;
	bool m_NinjaJetpack;
	int m_TeamBeforeSuper;
	int m_FreezeTime;
	int m_FreezeTick;
	int64 m_FirstFreezeTick;
	bool m_DeepFreeze;
	bool m_EndlessHook;
	bool m_FreezeHammer;
	bool m_fake_super;
	bool m_Godmode;
	bool m_Fire;
	bool m_DDPP_Finished;

	//trading stuff (stock market)
	//int m_StockMarket_item_Cucumbers; //player.h

	//weapons in kill messages

	int m_LastHitWeapon;
	int m_OldLastHookedPlayer;
	bool m_GotTasered;

	// drop pickups
	void DropHealth(int amount = 1);
	void DropArmor(int amount = 1);
	void DropWeapon(int WeaponID);
	bool m_aDecreaseAmmo[NUM_WEAPONS];

	void DropLoot();

	bool SetWeaponThatChrHas();


	//spawn weapons
	void SetSpawnWeapons();


	void BulletAmounts();
	int m_GunBullets;
	int m_ShotgunBullets;
	int m_GrenadeBullets;
	int m_RifleBullets;

	//spooky ghost
	void SetSpookyGhost();
	void UnsetSpookyGhost();
	void SaveRealInfos();
	bool m_CountSpookyGhostInputs;

	int m_TimesShot;

	int m_aSpookyGhostWeaponsBackup[NUM_WEAPONS][2];
	bool m_SpookyGhostWeaponsBackupped;
	int m_aSpookyGhostWeaponsBackupGot[NUM_WEAPONS][2];

	//room by supermoderator invited
	bool m_HasRoomKeyBySuperModerator;
	bool DDPP_Respawn();


	//harvest plant
	bool m_CanHarvestPlant;
	bool m_HarvestPlant;

	// shop
	bool m_InShop;
	bool m_EnteredShop;
	bool m_LeftShop;

	void ShopWindow(int Dir);
	int m_ShopWindowPage;

	int64 m_ShopMotdTick;

	void BuyItem(int ItemID);
	void ConfirmPurchase();
	void StartShop();

	int m_PurchaseState;

	void PurchaseEnd(bool canceled);

	bool m_ChangeShopPage;


	//bank
	bool m_InBank;

	//jail
	bool m_InJailOpenArea;

	//bomb
	bool m_IsBombing;
	bool m_IsBomb;
	bool m_IsBombReady;
	int m_BombPosX;
	int m_BombPosY;

	//battlegores
	void KillFreeze(bool unfreeze);

	bool HandleConfigTile(int Type);

	// finite cosmetics
	bool m_Rainbow;
	bool m_Bloody;
	bool m_StrongBloody;
	bool m_WaveBloody;
	bool m_WaveBloodyGrow;
	int m_WaveBloodyStrength;
	bool m_Atom;
	bool m_Trail;
	bool m_autospreadgun;
	bool m_ninjasteam;
	bool m_RandomCosmetics; // admin only cosmetic doesn't have to be backupped or anything. Because it won't check if u have these cosmetics unlocked.

	bool m_HomingMissile;

	// atom vars (not to be confused with atom wars) <--- made chillidreghuhn giggle xd
	std::vector<CStableProjectile *> m_AtomProjs;
	int m_AtomPosition;

	// trail vars
	std::vector<CStableProjectile *> m_TrailProjs;
	struct HistoryPoint
	{
		vec2 m_Pos;
		float m_Dist;

		HistoryPoint(vec2 Pos, float Dist): m_Pos(Pos), m_Dist(Dist) {}
	};
	std::deque<HistoryPoint> m_TrailHistory;
	float m_TrailHistoryLength;

	enum {
		DUMMYMODE_DEFAULT=0,
		DUMMYMODE_ADVENTURE=-7,
		DUMMYMODE_QUEST=36,
	};

	//dummymode public vars (used by survival 34)
	int m_DummyDir;

	//dummymode 25 FNN vars
	bool m_Dummy_nn_ready;
	bool m_Dummy_nn_touched_by_humans;
	bool m_Dummy_nn_stop;
	int m_Dummy_nn_ready_time;
	int m_FNN_CurrentMoveIndex;
	int m_aRecMove[FNN_MOVE_LEN];
	int m_FNN_ticks_loaded_run;
	int m_FNN_start_servertick;
	int m_FNN_stop_servertick;
	vec2 m_StartPos;
	// void TestPrintTiles(int Index);

	//dummymode 105 vars
	bool m_Dummy105_move_left;

	//dummymode 104 vars
	bool m_Dummy104_panic_hook;
	int m_Dummy104_angry;
	bool m_Dummy104_rj_failed;

	//dummymode 27 vars (BlmapChill police guard)
	bool m_Dummy27_IsReadyToEnterPolice;
	int m_Dummy27_loved_x;
	int m_Dummy27_loved_y;
	int m_Dummy27_lower_panic;
	int m_Dummy27_speed;
	int m_Dummy27_help_mode; //0=off 1=right side 2=right_side_extreme
	bool m_Dummy27_help_hook;

	//dummymode 31 vars (ChillBlock5 police guard)
	bool m_Dummy_SpawnAnimation;
	int m_Dummy_SpawnAnimation_delay;
	bool m_Dummy_GetSpeed;
	bool m_Dummy_GotStuck;
	bool m_Dummy_ClosestPolice;
	int m_Dummy_dmm31;
	int m_Dummy_AttackMode;

	//dummymode 32 (Blampchill police racer)
	int m_DummyGrenadeJump;
	bool m_DummyTouchedGround;
	bool m_DummyAlreadyBeenHere;
	bool m_DummyStartGrenade;
	bool m_DummyUsedDJ;
	int m_DummySpawnTeleporter; // 1 = left, 2 = middle, 3 = right (the totele 9 at spawn)s

	//dummy 29 vars !!!!! also use 18 vars in 29 xD

	CDummyBlmapChillPolice *m_pDummyBlmapChillPolice;

	CNetObj_PlayerInput *Input() { return &m_Input; };
	CNetObj_PlayerInput *LatestInput() { return &m_LatestInput; };
	void Fire(bool Fire = true);
	int GetReloadTimer() { return m_ReloadTimer; }

	//dummymode 29 vars (ChillBlock5 blocker)
	int m_DummyFreezeBlockTrick;
	int m_Dummy_trick_panic_check_delay;
	bool m_Dummy_start_hook;
	bool m_Dummy_speedright;  //used to go right from the left side of the freeze if there is enoigh speed
	bool m_Dummy_trick3_panic_check;
	bool m_Dummy_trick3_panic;
	bool m_Dummy_trick3_start_count;
	bool m_Dummy_trick3_panic_left;
	bool m_Dummy_trick4_hasstartpos;
	bool m_Dummy_lock_bored;             //tricky way to keep the bored bool activatet
	bool m_Dummy_doBalance;
	bool m_Dummy_AttackedOnSpawn;
	bool m_Dummy_bored_shootin;
	bool m_Dummy_bored_cuz_nothing_happens;
	bool m_Dummy_movement_to_block_area_style_window;  //yep dis is too long
	bool m_Dummy_planned_movment; // belongs to:   m_Dummy_movement_to_block_area_style_window


	//dummy 19 vars
	int m_DummyDriveDuration;


	//dummymode 23 vars

	int m_Dummy_help_m8_before_hf_hook; //yep a bool int timer
	bool m_Dummy_help_emergency;		//activate if boot falls of platform while helping
	bool m_Dummy_help_no_emergency;		//this is just used to check if the bot planned to be in this situation. this bool is just used for not activating m_Dummy_help_emergency
	bool m_Dummy_hook_mate_after_hammer;//after unfreezing a mate with hammer hold him with hook
	bool m_Dummy_help_before_fly;		//to help at the mate if he gets freeze before the hammerfly started
	bool m_Dummy_2p_panic_while_helping;//if the bot falls of the platform while helping at the 2p part
	bool m_Dummy_panic_balance;			//hammerhit part struggle -> balance
	bool m_Dummy_mate_failed;			//a �var which toggles the dummy_2p_state value -2
	bool m_Dummy_hh_hook;				//check for hook in hammerhit at end
	bool m_Dummy_collected_weapons;		//ob er nochmal zu den waffen hochfliegen muss
	bool m_Dummy_mate_collected_weapons;//ob auch der race mate waffen hat
	bool m_Dummy_rjumped2;				//ob der dummy grad den rj2 hinter sich hat
	bool m_Dummy_dd_hook;               //hier hookt er im 2p part                          nvm i renamed in 2p not dd      ignore -> [CARE USED OLD VAR FROM OLD SYSTEM FOR NEW SYSTEM CHECK THIS STUFF IF USE OLD SYSTEM AGIAN!!!!]
	bool m_Dummy_dd_helphook;			//just a helphook bool ... used for start and stoop hooking while helping at the dummydrag part
	bool m_Dummy_2p_hook;				//same as m_Dummy_dd_hook but in new sys 
	bool m_Dummy_2p_hook_grabbed;		// for better resetting if part failed
	int m_Dummy_2p_state;				//Maybe cool stuff comign with it
	int m_Dummy_mode23;                 //yes dummymode23 has his own modes o.O
	int m_Dummy_nothing_happens_counter;// counts all the nonaction lol
	int m_Dummy_panic_weapon;			// if the bot has panic (nothing happens -> panic mate coudl get bored)  change the wepaon to this var value
	int m_Dummy_sent_chat_msg;			// 0 == noMsgDisTick 1 == MsgDisTick              [to send a chat message just 1 time]
	int m_Dummy_mate_help_mode;			//how the bot shoudl help
	int m_Dummy_movement_mode23;		//a movement mode for mode23
	//int m_Dummy_rj_fails_counter;		//the hammerfly and weapon check sometimes causes fails and the dummy tries to rocketjump but is not abel to do it. this counter is used to detect this problem and kill the bot is detected

	//bool m_Dummy_2p_hammer1;			//Check if he did the first hammer

	//bool m_Dummy_rj_ready;				//check if the bot has the perfect position to make the rocketjump

	//notstand vars fuer mode 18 (also used in 29)
	bool m_Dummy_jumped;				//gesprungen wenn der notstand ausgetufen wird
	bool m_Dummy_hooked;				//gehookt wenn der notstand ausgerufen wird
	bool m_Dummy_moved_left;			//nach links gelaufen wenn der notstand ausgerufen wird
	bool m_Dummy_hook_delay;			//hook delay wenn der notstand ausgerufen wurde
	bool m_Dummy_ruled;					//ob der dummy in diesem leben schonmal am ruler spot war
	bool m_Dummy_pushing;				//ob er jemand grad beim wayblocken aus seinem wb spot schiebt
	bool m_Dummy_emergency;				// Notsand
	bool m_Dummy_wb_hooked;				//ob er grad vom wayblockspot wen wayblockig hookt
	bool m_Dummy_left_freeze_full;		//wenn jemand schon in die linke freeze wand geblockt wurde
	bool m_Dummy_happy;                 //wenn er sich auf seinem lieblings wb spot befindet
	bool m_Dummy_get_speed;             //im tunnel anlauf holen wenn ausgebremst                     WARNING THIS VAR IS ALSO USED IN DUMMYMODE == 26
	bool m_Dummy_bored;					//wenn dem bot langweilig wird wechselt er die wayblock taktik
	bool m_Dummy_special_defend;        //dummy_mode18 mode bool
	bool m_Dummy_special_defend_attack; //sub var f�r m_Dummy_special_defend die abfr�gt ob der bot schon angreifen soll

	int m_Dummy_bored_counter;          //z�hl hoch bis dem dummy lw wird

	int m_Dummy_mode18;                 //yes dummymode18 has his own modes o.O
	//bool mode18_main_init;              //yep one of the randomesteztes booleans in ze world

	enum
	{
		HIT_ALL=0,
		DISABLE_HIT_HAMMER=1,
		DISABLE_HIT_SHOTGUN=2,
		DISABLE_HIT_GRENADE=4,
		DISABLE_HIT_RIFLE=8
	};
	int m_Hit;
	int m_TuneZone;
	int m_TuneZoneOld;
	int m_PainSoundTimer;
	int m_LastMove;
	int m_StartTime;
	vec2 m_PrevPos;
	int m_TeleCheckpoint;
	int m_CpTick;
	int m_CpActive;
	int m_CpLastBroadcast;
	float m_CpCurrent[25];
	int m_TileIndex;
	int m_TileFlags;
	int m_TileFIndex;
	int m_TileFFlags;
	int m_TileSIndex;
	int m_TileSFlags;
	int m_TileIndexL;
	int m_TileFlagsL;
	int m_TileFIndexL;
	int m_TileFFlagsL;
	int m_TileSIndexL;
	int m_TileSFlagsL;
	int m_TileIndexR;
	int m_TileFlagsR;
	int m_TileFIndexR;
	int m_TileFFlagsR;
	int m_TileSIndexR;
	int m_TileSFlagsR;
	int m_TileIndexT;
	int m_TileFlagsT;
	int m_TileFIndexT;
	int m_TileFFlagsT;
	int m_TileSIndexT;
	int m_TileSFlagsT;
	int m_TileIndexB;
	int m_TileFlagsB;
	int m_TileFIndexB;
	int m_TileFFlagsB;
	int m_TileSIndexB;
	int m_TileSFlagsB;
	vec2 m_Intersection;
	int64 m_LastStartWarning;
	int64 m_LastRescue;
	bool m_LastRefillJumps;
	bool m_LastPenalty;
	bool m_LastBonus;

	int64 m_AliveTime;

	int m_LastIndexTile;
	int m_LastIndexFrontTile;

	// Setters/Getters because i don't want to modify vanilla vars access modifiers
	int GetLastWeapon() { return m_LastWeapon; };
	void SetLastWeapon(int LastWeap) {m_LastWeapon = LastWeap; };
	int GetActiveWeapon() { return m_Core.m_ActiveWeapon; };
	void SetActiveWeapon(int ActiveWeap) {m_Core.m_ActiveWeapon = ActiveWeap; };
	void SetLastAction(int LastAction) {m_LastAction = LastAction; };
	int GetArmor() { return m_Armor; };
	void SetArmor(int Armor) {m_Armor = Armor; };
	CCharacterCore GetCore() { return m_Core; };
	void SetCore(CCharacterCore Core) {m_Core = Core; };
	CCharacterCore* Core() { return &m_Core; };
	bool GetWeaponGot(int Type) { return m_aWeapons[Type].m_Got; };
	void SetWeaponGot(int Type, bool Value) { m_aWeapons[Type].m_Got = Value; };
	int GetWeaponAmmo(int Type) { return m_aWeapons[Type].m_Ammo; };
	void SetWeaponAmmo(int Type, int Value) { m_aWeapons[Type].m_Ammo = Value; };
	/*inline*/ bool IsAlive() { return m_Alive; }; //testy inline by ChillerDragon
	void SetEmoteType(int EmoteType) { m_EmoteType = EmoteType; };
	void SetEmoteStop(int EmoteStop) { m_EmoteStop = EmoteStop; };
	void SetNinjaActivationDir(vec2 ActivationDir) { m_Ninja.m_ActivationDir = ActivationDir; };
	void SetNinjaActivationTick(int ActivationTick) { m_Ninja.m_ActivationTick = ActivationTick; };
	void SetNinjaCurrentMoveTime(int CurrentMoveTime) { m_Ninja.m_CurrentMoveTime = CurrentMoveTime; };
	vec2 MousePos() { return vec2(m_Core.m_Input.m_TargetX + m_Pos.x, m_Core.m_Input.m_TargetY + m_Pos.y); };
};

enum
{
	DDRACE_NONE = 0,
	DDRACE_STARTED,
	DDRACE_CHEAT, // no time and won't start again unless ordered by a mod or death
	DDRACE_FINISHED
};

#endif
