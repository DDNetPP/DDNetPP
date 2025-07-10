// This file can be included several times.

#ifndef IN_CLASS_PLAYER
#include "alloc.h"

// this include should perhaps be removed
#include "captcha.h"
#include "entities/character.h"
#include <game/generated/protocol.h>
#include <game/server/ddpp/enums.h>
#include <game/server/minigames/one_vs_one_block.h>
#include <game/version.h>

#include "ddpp/accounts.h"

#include <memory>
#include <vector>

#define ACC_MAX_LEVEL 110 // WARNING!!! if you increase this value make sure to append needexp until max-1 in player.cpp:CalcExp()
#include "gamecontext.h"
#include "score.h"
#include "teeinfo.h"

enum
{
	WEAPON_GAME = -3, // team switching etc
	WEAPON_SELF = -2, // console kill command
	WEAPON_WORLD = -1, // death tiles etc
};

// player object
class CPlayer
{
#endif

public:
	/*

		DDNet++

	*/

	void ConstructDDPP();
	void DestructDDPP();

	void DDPPProcessScoreResult(CAccountResult &Result);
	void DDPPProcessAdminCommandResult(CAdminCommandResult &Result);
	std::shared_ptr<CAccountResult> m_AccountQueryResult;
	std::shared_ptr<CAdminCommandResult> m_AdminCommandQueryResult;

	void ResetDDPP();
	void DDPPTick();
	void OnDisconnectDDPP();
	/*
		DDPPSnapChangeSkin

		returns true if changed skin
	*/
	bool DDPPSnapChangeSkin(CNetObj_ClientInfo *pClientInfo);

	int m_Language;
	int Language() { return m_Language; }
	void SetLanguage(int Lang) { m_Language = Lang; }
	void SetLanguage(const char *pLang);

	// usefull everywhere
	void MoneyTransaction(int Amount, const char *Description = "");
	bool IsInstagibMinigame() const;
	bool IsMaxLevel() { return GetLevel() >= ACC_MAX_LEVEL; }
	bool IsLoggedIn() { return GetAccId() != 0; } // -1 filebased acc >0 sql id
	int GetAccId() { return m_Account.m_Id; }
	void SetAccId(int Id);
	/*
		GiveXP(int value)

		Use this function to add value xp to a players stats
		It takes care of max level.
	*/
	void GiveXP(int value);
	/*
		GiveBlockPoints(int Points)

		Updates block points expects a positive integer.
		and also sends a chat message to the player
	*/
	void GiveBlockPoints(int Points);
	/*
		SetXP(int xp)

		WARNING you probably want to use GiveXp(int value); instead!
		SetXP() should only be used if it is really needed
	*/
	void SetXP(int xp);
	int64_t GetXP() { return m_Account.m_XP; }
	int64_t GetNeededXP() { return m_neededxp; }
	int GetLevel() { return m_Account.m_Level; }
	void SetLevel(int Level);
	int64_t GetMoney() { return m_Account.m_Money; }
	/*
		SetMoney()

		WARNING you probably want to use MoneyTransaction(int Amount, char* Desc) instead!
		This is only used for specific cases! Dont touch it if you have no idea how it works xd
	*/
	void SetMoney(int Money);
	bool m_IsVanillaModeByTile;
	bool m_IsVanillaDmg;
	bool m_IsVanillaWeapons; //also used for pickups
	bool m_IsVanillaCompetetive;
	// admin cheat to be invisble
	bool m_IsHiddenTee = false;
	// bool m_IsGodMode; //no damage (only usefull in vanilla or pvp based subgametypes)
	bool m_MapSaveLoaded;
	CTeeInfo m_LastToucherTeeInfos;
	int m_ScoreStartTick;

	/* defined in accounts.h and copied over on sql query result */
	CAccountData m_Account;

	int m_AsciiAnimLen; //not used yet
	int m_AsciiAnimSpeed;
	int m_AsciiWatchFrame;
	int m_AsciiWatchTicker;
	int m_AsciiWatchingId;
	int m_AsciiViewsDefault; //direct with ascii view command
	int m_AsciiViewsProfile;

	class CInputTracker
	{
		CNetObj_PlayerInput m_LastInput;

		unsigned int m_HookChanges = 0;
		unsigned int m_DirectionChanges = 0;
		unsigned int m_FireChanges = 0;
		unsigned int m_JumpChanges = 0;

		unsigned int m_FlagTicksPlaying = 0;
		unsigned int m_FlagTicksMenu = 0;
		unsigned int m_FlagTicksChatting = 0;
		unsigned int m_FlagTicksScoreboard = 0;
		unsigned int m_FlagTicksAim = 0;

	public:
		void OnTick(CNetObj_PlayerInput *pInput, int PlayerFlags);

		// true if the player did jump, fire, move and hook
		// at least once since he joined
		bool SentAllInputsAtLeastOnce() const;

		// amount of times the player changed direction
		// hooked, shot and jumped
		// since he joined
		unsigned int SumOfAllInputChanges() const;

		// number of gameticks that were spent with the chat open (playerflag chat sent)
		// it is never reset and a total sum of all time chatting for this player
		// since he connected to the server
		unsigned int TicksSpentChatting() const;
	};

	CInputTracker m_InputTracker;

	// the amount of times this player was pinged in the chat
	int m_ReceivedChatPings = 0;

	// DDNet++ specific vote extensions
	// works similar to CPlayer::m_SendVoteIndex
	// but is not for regular votes but for custom vote menus
	int m_SendExtraVoteMenuIndex;

	//zCatch ChillerDragon
	int m_aCatchedId[64];

	//###########
	//minigames
	//###########

	//bomb (moved character.h)
	//bool m_IsBomb;
	//bool m_IsBombing;
	//bool m_IsBombReady;
	//moved to gamecontext.cpp
	//int m_BombColor;
	//bool m_bwff; //black whithe flip flip

	// bomb
	int m_BombTicksUnready;

	// block tourna
	bool m_IsBlockTourning;
	bool m_IsBlockTourningDead;
	bool m_IsBlockTourningInArena;

	bool m_IsBlockDeathmatch;

	// block 1vs1
	// the player we sent a request too
	int m_BlockOneVsOneRequestedId = -1;
	bool m_IsBlockOneVsOneing = false;
	int64_t m_BlockOneVsOneInviteExpire = 0;
	bool m_BlockOneVsOneTeleported = false;
	COneVsOneBlock::CGameState *m_pBlockOneVsOneState = nullptr;

	// blockwave
	bool m_IsBlockWaving;
	bool m_IsBlockWaveDead;
	bool m_IsBlockWaveWaiting;

	// chidraqul3 (minigame)
	void chidraqul3_GameTick();
	bool JoinMultiplayer();
	bool m_C3_UpdateFrame;
	int m_GoldRespawnDelay;
	int m_GoldPos;
	bool m_GoldAlive;
	int m_HashGold;
	int m_Minigameworld_size_x;
	char m_HashSkin[12];
	int m_HashPos;
	int m_HashPosY;
	bool m_BoughtGame;
	int m_C3_GameState; //0=off 1=singleplayer 2=multiplayer

	//profiles
	char m_LastViewedProfile[32];
	bool m_IsProfileViewLoaded;

	//bool m_IsHammerfight; //moved to character and renamed to --> m_IsHammerarena
	int m_pvp_arena_last_kill_id;

	bool m_IsInstaArena_gdm;
	bool m_IsInstaArena_idm;
	bool m_IsInstaArena_fng; //depends on gdm or idm can be boomfng and fng
	bool m_IsInstaMode_gdm; // THESE MODES ARE USED FOR THE INSTAGIB
	bool m_IsInstaMode_idm; // SERVER AND ARE DIFFERENT FROM THE MINIGAME
	bool m_IsInstaMode_fng; // THEY USE THE SCOARBOARD AND A BIT DIFFERENT RESTRICTIONS
	int m_Insta1on1_id; //also used as Is1on1ing bool (id != -1 ---> is in 1on1)
	int m_Insta1on1_mode; //0 = gdm 1 = idm 2 = boomfng 3 = fng
	int m_Insta1on1_score;
	int m_InstaScore;
	bool m_HideInsta1on1_killmessages;
	vec2 m_InstaRoundEndPos;
	bool m_HasInstaRoundEndPos;
	int m_lastkilltime;
	int m_multi;
	int m_max_multi;

	int m_BalanceBattle_id;
	bool m_IsBalanceBatteling;
	bool m_IsBalanceBattlePlayer1;
	bool m_IsBalanceBattleDummy;

	// botspawn tiles
	int m_DummySpawnTile;

	//survival
	bool m_IsSurvivaling;
	bool m_IsSurvivalAlive;
	bool m_IsSurvivalLobby;
	bool m_IsSurvivalWinner;

	//no name chat fix
	void FixForNoName(int Id);
	int m_FixNameId;
	bool m_ShowName;
	bool m_SetRealName;
	int64_t m_SetRealNameTick;
	//Id == 1 // chat message
	int m_ChatTeam;
	char m_ChatText[256];
	//Id == 2 // kill message
	int m_MsgKiller;
	int m_MsgWeapon;
	int m_MsgModeSpecial;

	//gun
	int m_HeartGunActive;

	//spooky ghost
	int m_SpookyGhostActive;

	int m_RealUseCustomColor;
	char m_RealSkinName[64];
	char m_RealName[64];
	char m_RealClan[64];
	int m_RealColorBody;
	int m_RealColorFeet;

	//spawn weapons
	int m_SpawnShotgunActive;
	int m_SpawnGrenadeActive;
	int m_SpawnRifleActive;

	std::vector<CWeapon *> m_aWeaponLimit[NUM_WEAPONS];

	//city stuff
	//int m_broadcast_animation; //idk if this var will be used. plan: check for a running animation and animate it //try in gamecontext.cpp
	bool m_cheats_aimbot;
	bool m_dummy_member; //trusted by dummy

	//##########
	//city stuff
	//##########

	//to update scoreboard immediately
	bool m_ChangeTeamOnFlag;
	bool m_HadFlagOnDeath;

	bool m_IsNoboSpawn;
	int64_t m_NoboSpawnStop;

	//Account stuff:
	bool m_IsFileAcc;

	bool m_IsSuperModSpawn;

	/*
		PlayerHumanLevel

		The higher the value the less likley it is that this player is a robot.
		To unlock the next level a player has to master all previous ones.

		This human level then can be used to unlock features like registering an account or uing the chat
		or showing connection messages. In case those basic features are blocked due to spam attacks.

		0  freshly connected
		1  sent a few movement inputs
		2  wait 10 sec
		3  a few movement or chat inputs ( messages or commands )
		4  wait 10 sec
		5  be somewhat active: finish race/get block points/login to account/have chat open for 20 ticks
		6  wait 20 sec
		7  solve the antibot captcha command
		8  played quest until finish map/make more than 10 block points
		9  wait 1 minute
	*/
	int m_PlayerHumanLevel;
	int m_PlayerHumanLevelState; // if the level has sublevels
	int64_t m_HumanLevelTime;
	const int m_MaxPlayerHumanLevel = 9;
	void PlayerHumanLevelTick();

	CCaptcha *m_pCaptcha;

	// if set to true the "joined the game" chat message
	// will not be printed
	// this is used for server side dummies to connect silently
	// especially for minigames with frequent reconnects
	//
	// but it can also be used for anti flood reconnect spam silencing
	bool m_SilentJoinMessage = false;

	// if the anti flood protection is on
	// join messages in chat are delayed until the player is verified as legit
	// if the player leaves before verification the disconnect message is also not printed
	//
	// this is set to false as soon as the message got printed after verification
	bool m_PendingJoinMessage = true;

	// if this is true the player is not supposed to be able to interact
	// with anything that could affect other players
	//
	// this happens if the server is in lock down and has the sv_captcha_room (SvCaptchaRoom) active
	// the player is not supposed to show up in the scoreboard or chat until he touches the
	// TILE_CAPTCHA_VERIFY
	//
	// https://github.com/DDNetPP/DDNetPP/issues/400
	bool m_PendingCaptcha = false;

	// this is called when reaching the maximum human level
	// or touching the captcha verify tile
	void OnHumanVerify();

	//money and traiding

	int m_StockMarket_item_Cucumbers;
	int m_MoneyTilesMoney;

	char m_money_transaction0[512];
	char m_money_transaction1[512];
	char m_money_transaction2[512];
	char m_money_transaction3[512];
	char m_money_transaction4[512];
	char m_money_transaction5[512];
	char m_money_transaction6[512];
	char m_money_transaction7[512];
	char m_money_transaction8[512];
	char m_money_transaction9[512];

	int m_TradeMoney;
	int m_TradeItem;
	int m_TradeId;
	int64_t m_TradeTick;
	int m_GangsterBagMoney;
	char m_aTradeOffer[256];

	int64_t m_ShopBotAntiSpamTick;
	int m_ShopBotMesssagesRecieved;

	void JailPlayer(int seconds);
	float TaserFreezeTime();
	int m_TaserPrice;
	bool m_TaserOn;
	bool m_IsJailed;
	bool m_PoliceHelper;
	char m_aEscapeReason[256];
	bool m_IsJailDoorOpen;
	//bool m_IsJailHammer;
	int m_JailHammer;
	//int m_JailHammerTime;
	int64_t m_JailHammerDelay;
	//int m_CorruptId;
	//int m_CorruptMoney;
	short m_JailCode;

	// show/hide infos

	bool m_HideBlockXp;
	bool m_HideQuestProgress;
	bool m_HideQuestWarning;
	bool m_ShowBlockPoints;
	bool m_xpmsg;
	bool m_hidejailmsg;
	bool m_ShowInstaScoreBroadcast;

	// quests
	int m_QuestUnlocked; // maybe save this in sql and later people can choose all quests untill unlocked
	int m_QuestLevelUnlocked; // maybe save this in sql and later people can choose all levels untill unlocked
	int m_QuestState; // current quest 0 = not questing
	int m_QuestStateLevel; // current quest level (difficulty)
	int m_QuestLastQuestedPlayerId; // store here the id to make sure in level 3 quest 1 for example he doenst hammer 1 tee 5 times
	int m_QuestProgressValue; // saves the values of m_QuestLastQuestedPlayerId
	int m_QuestProgressValue2;
	bool m_QuestProgressBool;
	int m_QuestPlayerId; // the id of the player which is the quest
	char m_aQuestString[512]; // stores the quest information
	int m_aQuestProgress[2]; // stores the quest progress information
	bool m_QuestFailed;
	bool IsQuesting() { return m_QuestState != QUEST_OFF; }
	enum
	{
		QUEST_OFF = 0,
		QUEST_HAMMER = 1,
		QUEST_BLOCK = 2,
		QUEST_RACE = 3,
		QUEST_RIFLE = 4,
		QUEST_FARM = 5,
		QUEST_NUM_PLUS_ONE,
		QUEST_NUM = QUEST_NUM_PLUS_ONE - 1,

		QUEST_NUM_LEVEL = 9
	};
	// handled in gamecontext.cpp LoadQuest()
	//              QUEST         QUEST LEVEL
	//              0                                = Not Questing

	//              1             0                  = Hammer 1 tee [LEVEL 0]
	//              1             1                  = Hammer 2 tees [LEVEL 1]
	//              1             2                  = Hammer 3 tees [LEVEL 2]
	//              1             3                  = Hammer 5 tees [LEVEL 3]
	//              1             4                  = Hammer 10 freezed tees [LEVEL 4]
	//              1             5                  = Hammer <specific player> 20 times [LEVEL 5]
	//              1             6                  = Hammer freezed <specific player> 3 times [LEVEL 6]
	//              1             7                  = Hammer <specfifc player> 10 times and then block him [LEVEL 7]
	//              1             8                  = Hammer 2 tees in one hit [LEVEL 8]
	//              1             9                  = Hammer 10 freezed tees in a row while holding the flag [LEVEL 9]

	//              2             0                  = Block 1 tee [LEVEL 0]
	//              2             1                  = Block 2 tees [LEVEL 1]
	//              2             2                  = Block 3 tees [LEVEL 2]
	//              2             3                  = Block 5 tees [LEVEL 3]
	//              2             4                  = Block 10 tees without using a weapon [LEVEL 4]
	//              2             5                  = Block 5 tees and then block <specific player> [LEVEL 5]
	//              2             6                  = Block a tee which is on a 5 tees blocking spree [LEVEL 6]
	//              2             7                  = Block 11 tees without getting blocked [LEVEL 7]
	//              2             8                  = Block 3 tees without using hook [LEVEL 8]
	//              2             9                  = Block 11 tees whithout dieing while holding the flag [LEVEL 9]   //TODO: die = fail

	//              3             0                  = Finish race [LEVEL 0]
	//              3             1                  = Finish race under cfg_time1 [LEVEL 1]
	//              3             2                  = Finish race under cfg_time2 [LEVEL 2]
	//              3             3                  = Finish race backwards (hit start line after touching finish line) [LEVEL 3]
	//              3             4                  = Finish race under cfg_time3 [LEVEL 4]
	//              3             5                  = Finish race with the flag [LEVEL 5]
	//              3             6                  = Finish special race (touching event tile) [LEVEL 6]
	//              3             7                  = Finish special race under cfg_longracetime1 [LEVEL 7]
	//              3             8                  = Finish special race backwards [LEVEL 8]
	//              3             9                  = Finish race cfg_condition(No Hammer, No Gun (including jetpack), No Shotgun, No Grenade, No Rifle, No Ninja, without getting unfreeze, without getting freeze, without looking down,without looking up, no doublejump) [LEVEL 9]

	//              4             0                  = Rifle 1 tee [LEVEL 0]
	//              4             1                  = Rifle <specific player> 5 times [LEVEL 1]
	//              4             2                  = Rifle freezed <specific player> 5 times [LEVEL 2]
	//              4             3                  = Rilfe 10 tees and <specific player> [LEVEL 3]
	//              4             4                  = Rifle 10 freezed tees [LEVEL 4]
	//              4             5                  = Rifle yourself while being freezed [LEVEL 5]
	//              4             6                  = Rifle yourself while being freezed 10 times [LEVEL 6]
	//              4             7                  = Rifle <specific player> and then block him [LEVEL 7]
	//              4             8                  = Rifle 5 tees before blocking them [LEVEL 8]
	//              4             9                  = Rifle 20 freezed tees while having the flag [LEVEL 9]

	//              5             0                  = Farm 10 money on a moneytile [LEVEL 0]
	//              5             1                  = Farm 20 money on a moneytile [LEVEL 1]
	//              5             2                  = Farm 30 money on a moneytile [LEVEL 2]
	//              5             3                  = Farm 40 money on a moneytile [LEVEL 3]
	//              5             4                  = Farm 50 money on a moneytile [LEVEL 4]
	//              5             5                  = Farm 60 money on a moneytile [LEVEL 5]
	//              5             6                  = Farm 70 money on a moneytile [LEVEL 6]
	//              5             7                  = Farm 100 money on a police moneytile [LEVEL 7]
	//              5             8                  = Farm 100 money on a moneytile [LEVEL 8]
	//              5             9                  = Farm 200 xp with a flag [LEVEL 9]

	//#################
	// WARNING
	// update quest num
	// QUEST_NUM_LEVEL
	// if you add new quest level
	//#################

	// other

	EDisplayScore m_DisplayScore;
	int m_AllowTimeScore;
	/*
		m_MinigameScore

		Score that will also be displayed in the scoreboard
		if the minigame is active

		is seperate from m_Score which is only used for ddnet
		race times

		The minigame score can/should be used for kills
		in fng/dm/gctf/block/bomb and so on
	*/
	int m_MinigameScore = 0;

	bool m_CanClearFakeMotd;
	bool m_IsFakeMotd;
	bool m_IsTest;
	int m_failed_escapes;
	int m_escape_skill;
	bool m_escape_plan;
	bool m_escape_plan_b;
	bool m_escape_plan_c;
	bool m_BoughtRoom;
	int m_aliveplusxp;
	bool m_MoneyTilePlus;
	bool m_fake_admin;
	int64_t m_LastFight;

	char m_aSqlNameName[32]; //used to save account name admins interact with in the '/sql_name' command

	void Save(int SetLoggedIn);
	void SaveFileBased();
	void Logout(int SetLoggedIn = 0);
	void OnLogin();
	void CheckLevel();
	void CalcExp();

	bool m_HammerRequest;

	/*
		SetDummyMode

		returns false on error
		error is usually non existant dummy mode
	*/
	bool SetDummyMode(EDummyMode Mode);
	int DummyMode() { return m_DummyMode; }
	bool m_IsDummy;
	const char *DummyModeStr() { return m_aDummyMode; }
	char m_aDummyMode[128];
	CDummyBase *m_pDummyMode; // TODO: make private
	EDummyTest m_DummyTest = EDummyTest::NONE;

	int m_dmm25; //change dummy modes in the mode 25  ( choose sub modes)
	float m_Dummy_nn_latest_Distance;
	float m_Dummy_nn_highest_Distance;
	float m_Dummy_nn_highest_Distance_touched;
	float m_Dummy_nn_latest_fitness;
	float m_Dummy_nn_highest_fitness;
	int m_Dummy_nn_time;

	// dummy 32 vars
	bool m_Dummy_32dummy;
	int m_Dummy_32look;

	//REAL DUMMY 32 VARS blmapchill police
	int m_DummyModeSpawn;

	//dummy 33 vars (Chillintelligenz)
	long m_ci_lowest_dest_dist; //max long len 2147483647
	long m_ci_latest_dest_dist;

	CTwblPersistentState m_TwblPersistentState;

	//########
	//extras
	//########

	//miscelallaouos
	bool m_lasergun;

	//hook
	int m_HookPower; // 0=off 1=rainbow 2=bloody

	// infinite cosmetics
	bool m_InfRainbow;
	bool m_InfBloody;
	bool m_InfAtom;
	bool m_InfTrail;
	bool m_InfAutoSpreadGun;

	// cosmetic offers
	int m_rainbow_offer;
	int m_bloody_offer;
	int m_atom_offer;
	int m_trail_offer;
	int m_autospreadgun_offer;

	//dummy rainbow
	int m_DummyRainbowOfferAmount;

	// cosmetic backups (used to store cosmetics temprorary for example in competetive games)
	bool m_IsBackupBloody;
	bool m_IsBackupStrongBloody;
	bool m_IsBackupRainbow;
	bool m_IsBackupAtom;
	bool m_IsBackupTrail;
	bool m_IsBackupAutospreadgun;
	bool m_IsBackupWaveBloody;

	int m_KillStreak;
	//bool m_InBank; //moved character
	bool m_ExitBank;

	//BLOCK POINTS

	//bool m_BlockWasTouchedAndFreezed;  //This bool is used for: check if someone was touched and freezed and if we have this info we can set the touch id to -1 if this bool is true and he is unfreeze ---> if you get blocked and unfreezed again and suicide you wont block die
	int m_SpawnBlocks;
	int m_BlockSpreeHighscore;

	/*
		m_LastToucherId

		Use UpdateLastToucher() to set this variable
		it tracks the id of the last person who touched it
		used for block kills
	*/
	int m_LastToucherId;

	// ticks since last touch
	int m_LastTouchTicks;

	char m_aLastToucherName[64];
	void UpdateLastToucher(int Id);

	int m_BlockBounty;

	// gets incremented when touching the
	// king of the hill tile
	int m_KingOfTheHillScore = 0;

	//bool m_hammerfight;
	//bool m_isHeal;
	bool m_ninjasteam;
	bool m_disarm;
	//bool m_freezeShotgun;
	int m_RainbowColor;

	char m_aWrongLogin[256];

	int64_t m_LastWarning;
	int m_ChilliWarnings;
	bool m_TROLL166;
	bool m_TROLL420;
	bool m_RconFreeze;

private: // private ddnet+++
	int64_t m_neededxp;
	EDummyMode m_DummyMode;

#ifndef IN_CLASS_PLAYER
}
#endif
