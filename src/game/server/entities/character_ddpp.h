// This file can be included several times.

#ifndef IN_CLASS_CHARACTER
#include <deque>
#include <engine/antibot.h>
#include <game/generated/protocol.h>
#include <game/generated/server_data.h>
#include <game/server/ddpp/teleportation_request.h>
#include <game/server/entities/stable_projectile.h>
#include <game/server/entity.h>
#include <game/server/save.h>

#include <game/gamecore.h>

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

#include <vector>

#include "drop_pickup.h"
#include "weapon.h"
class CCharacter : public CEntity
{
#endif

public:
	~CCharacter();

private:
	// hooks
	void ConstructDDPP();
	void DestructDDPP();
	void SnapCharacterDDPP();
	bool HandleTilesDDPP(int Index);
	void DDPPDDRacePostCoreTick();
	/*
		FireWeaponDDPP

		abort ddnet fire when returning true
	*/
	bool FireWeaponDDPP(bool &FullAuto);
	void PostFireWeapon();

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
	//int m_AimbotSwitchTick;
	bool m_AimbotMode; // wenn true, dann ist aimbot aktiviert, bei false halt nicht wozu? damit man weiss ob der bot den aimbot an hat oder halt random durch die gegend guckt? ne brauchts eig nich oder? lass erstmal ohne
	bool m_MoveMode;
	bool m_LeftMM;

	bool m_DummyShowRank;
	bool m_DummyFreezed;
	bool m_DummyHammer;

	int m_FreezeKillNext;
	int m_MoveTick;
	int m_LastMoveDirection;
	int m_StopMoveTick;
	void ClearFakeMotd();
	void SendShopMessage(const char *pMsg);

public:
	bool m_IsFreeShopBot;
	bool m_DummyFinished;
	int m_DummyFinishes;
	int m_LastIndexTile;
	int m_LastIndexFrontTile;
	vec2 MousePos() { return vec2(m_Core.m_Input.m_TargetX + m_Pos.x, m_Core.m_Input.m_TargetY + m_Pos.y); };
	vec2 GetPosition() { return m_Core.m_Pos; } //proudly mede by ChillerDragon dupe of CEntitiy::GetPos() ??
	vec2 GetVel() { return m_Core.m_Vel; }
	void SetHealth(int Health) { m_Health = Health; }
	void TakeHammerHit(CCharacter *pFrom); //ddpp implemented from fng2
	bool m_OnFire;
	void DDPP_Tick();
	void DDPP_FlagTick();
	void CosmeticTick();
	void PreSpawnDDPP();
	void PostSpawnDDPP();
	void DDPPPostCoreTick();
	bool DDPPTakeDamage(vec2 Force, int Dmg, int From, int Weapon);
	int DDPP_DIE(int Killer, int Weapon, bool FngScore = false);
	void TwblTick();
	void DummyTick();

	// useful everywhere
	void DDPP_TakeDamageInstagib(int Dmg, int From, int Weapon);
	void MoveTee(int x, int y);
	void ChillTelePort(float X, float Y);
	void ChillTelePortTile(int X, int Y);
	void FreezeAll(int Seconds);
	bool HasWeapon(int Weapon);
	void KillSpeed();
	int GetAimDir() const;
	void UnDeep() { m_Core.m_DeepFrozen = false; }
	// returns false if in team 0 and true otherwise
	bool IsInDDraceTeam();

	//Chillintelligenz
	void CITick();
	void CIRestart();
	int CIGetDestDist();

	int m_ci_freezetime;

	//Block
	int BlockPointsMain(int Killer, bool FngScore = false);
	void XpOnKill(int Killer);
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
	void PoliceHammerHit(CCharacter *pTarget);
	void DDPPHammerHit(CCharacter *pTarget);
	bool IsHammerBlocked();
	void DDPPGunFire(vec2 Direction);
	bool SpecialGunProjectile(vec2 Direction, vec2 ProjStartPos, int Lifetime);
	bool FreezeShotgun(vec2 Direction, vec2 ProjStartPos);
	bool m_FreezeHammer;

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
	void InstagibKillingSpree(int KillerId, int Weapon);
	bool m_UpdateInstaScoreBoard;
	bool m_hammerfight; //used for the rcon command has nothing todo with arenas yet
	//bool m_IsHammerarena; //used for chillerdragons hammerfight arena '/hammerfight'
	//bool m_Hammerarena_exit_request;
	//int m_Hammerarena_exit_request_time;
	//
	bool m_IsPvpArenaing = false;
	bool m_isHeal;
	bool m_freezeShotgun;
	bool m_FreezeLaser;
	bool m_isDmg;
	int64_t m_FirstFreezeTick;
	bool m_fake_super;
	bool m_Godmode;
	bool m_Fire;
	/*
		ForceFreeze

		mede by ChillerDragon too freeze no matter what used for freezing while freezed (for example for tournaments to have all same freeze time even if some wer freezed at tourna start)
		WARNING FORCE FREEZE ISNT ABLE TO OVERWRITE FREEZE AS IT SHOULD!!!
		it can still be used to bypass super but thats all i guess
	*/
	bool ForceFreeze(int Seconds);

	CTeleportationRequest m_TeleRequest;
	// Choses random tile if Offset is -1
	CTeleportationRequest &RequestTeleToTile(int Tile, int Offset = -1);
	CTeleportationRequest &RequestTeleToPos(vec2 Pos);

	//trading stuff (stock market)
	//int m_StockMarket_item_Cucumbers; //player.h

	//weapons in kill messages

	int m_LastHitWeapon;
	int m_OldLastHookedPlayer;
	bool m_GotTasered;
	int64_t m_LastTaserUse;
	/*
		FreezeFloat

		ddnet++ overload of ddnets Freeze(int)
		for more precise freeze times
		used for taser
	*/
	bool FreezeFloat(float Seconds);

	// drop pickups
	void DropHealth(int amount = 1);
	void DropArmor(int amount = 1);
	void DropWeapon(int WeaponId);
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
	bool m_EnteredShop; // TODO: move to CShop ?

	//bank
	bool m_InBank;

	//bomb
	bool m_IsBombing;
	bool m_IsBomb;
	bool m_IsBombReady;
	int m_BombPosX;
	int m_BombPosY;

	enum
	{
		MONEYTILE_NONE,
		MONEYTILE_NORMAL,
		MONEYTILE_POLICE,
		MONEYTILE_DOUBLE,
	};

	int m_OnMoneytile;

	//battlegores
	void KillFreeze(bool unfreeze);

	bool HandleConfigTile(int Type);

	// atom vars (not to be confused with atom wars) <--- made chillidreghuhn giggle xd
	std::vector<CStableProjectile *> m_AtomProjs;
	int m_AtomPosition;

	// trail vars
	std::vector<CStableProjectile *> m_TrailProjs;
	struct HistoryPoint
	{
		vec2 m_Pos;
		float m_Dist;

		HistoryPoint(vec2 Pos, float Dist) :
			m_Pos(Pos), m_Dist(Dist) {}
	};
	std::deque<HistoryPoint> m_TrailHistory;
	float m_TrailHistoryLength;

	CNetObj_PlayerInput *Input() { return &m_Input; };
	void Fire(bool Fire = true);
	int GetReloadTimer() { return m_ReloadTimer; }
	void MineTeeBreakBlock();

	/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	 *	All these variables are stored in CSaveTeeDDPP                       *
	 *                                                                       *
	 *	Add variables that have to be saved here and also in save_ddpp.h     *
	 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	int m_aWeaponsBackup[NUM_WEAPONS][2];
	bool m_WeaponsBackupped;
	int64_t m_AliveSince;
	int m_survivexpvalue;
	bool m_DDPP_Finished;

	bool HasBloody();

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

	int64_t m_NextCantEnterRoomMessage = 0;
	int64_t m_NextCantEnterVipRoomMessage = 0;
	bool CanEnterRoom();

	// character_tiles.cpp

	bool IsOnTile(int Tile) const;

	void OnTileShop();
	void OnTileBank();
	void OnTileRoom();
	void OnTileVipPlusOnly();
	void OnTileMoney();
	void OnTileMoneyPolice();
	void OnTileMoneyPlus();
	void OnTileMoneyDouble();
	void OnTileStart();
	void OnTileFinish();
	void OnTileSpecialFinish();

private:
#ifndef IN_CLASS_CHARACTER
}
#endif
