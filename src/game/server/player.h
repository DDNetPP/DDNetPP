/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_PLAYER_H
#define GAME_SERVER_PLAYER_H

// this include should perhaps be removed
#include "entities/character.h"
#include "captcha.h"
#include "gamecontext.h"
#include <game/version.h>

#include "db_sqlite3.h" //ddpp ChillerDragon for threaded login

#include <vector>

#define ACC_MAX_LEVEL 110 // WARNING!!! if you increase this value make sure to append needexp until max-1 in player.cpp:CalcExp()

// player object
class CPlayer
{
	MACRO_ALLOC_POOL_ID()

	friend class CSaveTee;

public:
	CPlayer(CGameContext *pGameServer, int ClientID, int Team);
	~CPlayer();

	void Reset();

	void TryRespawn();
	void Respawn(bool WeakHook = false); // with WeakHook == true the character will be spawned after all calls of Tick from other Players
	CCharacter* ForceSpawn(vec2 Pos); // required for loading savegames
	void SetTeam(int Team, bool DoChatMsg=true);
	int GetTeam() const { return m_Team; };
	int GetCID() const { return m_ClientID; };

	void Tick();
	void PostTick();

	// will be called after all Tick and PostTick calls from other players
	void PostPostTick();
	void Snap(int SnappingClient);
	void FakeSnap();

	void OnDirectInput(CNetObj_PlayerInput *NewInput);
	void OnPredictedInput(CNetObj_PlayerInput *NewInput);
	void OnDisconnect(const char *pReason, bool silent = false);

	void ThreadKillCharacter(int Weapon = WEAPON_GAME);
	void KillCharacter(int Weapon = WEAPON_GAME);
	CCharacter *GetCharacter();

	void FindDuplicateSkins();

	//---------------------------------------------------------
	// this is used for snapping so we know how we can clip the view for the player
	vec2 m_ViewPos;
	int m_TuneZone;
	int m_TuneZoneOld;

	// states if the client is chatting, accessing a menu etc.
	int m_PlayerFlags;

	// used for snapping to just update latency if the scoreboard is active
	int m_aActLatency[MAX_CLIENTS];

	// used for spectator mode
	int m_SpectatorID;

	bool m_IsReady;

	//
	int m_Vote;
	int m_VotePos;
	//
	int m_LastVoteCall;
	int m_LastVoteTry;
	int m_LastChat;
	int m_LastSetTeam;
	int m_LastSetSpectatorMode;
	int m_LastChangeInfo;
	int m_LastEmote;
	int m_LastKill;
	int m_LastCommands[4];
	int m_LastCommandPos;
	int m_LastWhisperTo;

	int m_SendVoteIndex;





	// TODO: clean this up
	struct
	{
		char m_SkinName[64];
		int m_UseCustomColor;
		int m_ColorBody;
		int m_ColorFeet;
	} m_TeeInfos, m_LastToucherTeeInfos;

	int m_DieTick;
	int m_Score;
	int m_ScoreStartTick;
	int m_JoinTick;
	bool m_ForceBalanced;
	int m_LastActionTick;
	bool m_StolenSkin;
	int m_TeamChangeTick;
	bool m_SentSemicolonTip;
	struct
	{
		int m_TargetX;
		int m_TargetY;
	} m_LatestActivity;

	// network latency calculations
	struct
	{
		int m_Accum;
		int m_AccumMin;
		int m_AccumMax;
		int m_Avg;
		int m_Min;
		int m_Max;
	} m_Latency;

private:
	CCharacter *m_pCharacter;
	int m_NumInputs;
	CGameContext *m_pGameServer;

	CGameContext *GameServer() const { return m_pGameServer; }
	IServer *Server() const;

	//
	bool m_Spawning;
	bool m_WeakHookSpawn;
	int m_ClientID;
	int m_Team;


	// DDRace
public:
	enum
	{
		PAUSED_NONE=0,
		PAUSED_SPEC,
		PAUSED_PAUSED,
		PAUSED_FORCE
	};

	int m_Paused;
	bool m_DND;
	int64 m_FirstVoteTick;
	int64 m_NextPauseTick;
	char m_TimeoutCode[64];

	void ProcessPause();
	int m_ForcePauseTime;
	bool IsPlaying();
	int64 m_Last_KickVote;
	int64 m_Last_Team;
	int m_Authed;
	int m_ClientVersion;
	bool m_ShowOthers;
	bool m_ShowAll;
	bool m_SpecTeam;
	bool m_NinjaJetpack;
	bool m_Afk;
	int m_KillMe;

	int m_ChatScore;

	bool AfkTimer(int new_target_x, int new_target_y); //returns true if kicked
	void AfkVoteTimer(CNetObj_PlayerInput *NewTarget);
	int64 m_LastPlaytime;
	int64 m_LastEyeEmote;
	int m_LastTarget_x;
	int m_LastTarget_y;
	CNetObj_PlayerInput m_LastTarget;
	int m_Sent1stAfkWarning; // afk timer's 1st warning after 50% of sv_max_afk_time
	int m_Sent2ndAfkWarning; // afk timer's 2nd warning after 90% of sv_max_afk_time
	char m_pAfkMsg[160];
	bool m_EyeEmote;
	int m_TimerType;
	int m_DefEmote;
	int m_DefEmoteReset;
	bool m_Halloween;
	bool m_FirstPacket;


	/*

		DDNet++

	*/

	void ResetDDPP();
	void DDPPTick();
	void OnDisconnectDDPP();
	/*
		DDPPSnapChangeSkin

		returns true if changed skin
	*/
	bool DDPPSnapChangeSkin(CNetObj_ClientInfo *pClientInfo);
	void DDPPSnapChangePlayerInfo(int SnappingClient, CPlayer *pSnapping, CNetObj_PlayerInfo *pPlayerInfo);

	// usefull everywhere
	void MoneyTransaction(int Amount, const char *Description = "");
	bool IsInstagibMinigame();
	bool IsMaxLevel() { return m_level >= ACC_MAX_LEVEL; }
	bool IsLoggedIn() { return m_AccountID != 0; } // -1 filebased acc >0 sql id
	int GetAccID() { return m_AccountID; }
	void SetAccID(int ID);
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
	int64 GetXP() { return m_xp; }
	int64 GetNeededXP() { return m_neededxp; }
	int GetLevel() { return m_level; }
	void SetLevel(int level);
	int64 GetMoney() { return m_money; }
	/*
		SetMoney()

		WARNING you probably want to use MoneyTransaction(int Amount, char* Desc) instead!
		This is only used for specific cases! Dont touch it if you have no idea how it works xd
	*/
	void SetMoney(int money);
	bool m_IsVanillaModeByTile;
	bool m_IsVanillaDmg;
	bool m_IsVanillaWeapons; //also used for pickups
	bool m_IsVanillaCompetetive;
	// bool m_IsGodMode; //no damage (only usefull in vanilla or pvp based subgametypes)
	int m_LastBroadcast;
	int m_LastBroadcastImportance;

	bool m_MapSaveLoaded;

	// login and threads
	void ThreadLoginStart(const char * pUsername, const char * pPassword);
	static void ThreadLoginWorker(void * pArg);
	void ThreadLoginDone();

	struct CLoginData {
		// meta
		CGameContext *m_pGameContext;
		int m_LoginState;
		int m_ClientID;
		int m_AccountID;
		char m_aUsername[64];
		char m_aPassword[64];
		char m_aAccountRegDate[64];

		// acc
		bool m_IsModerator;
		bool m_IsSuperModerator;
		bool m_IsSupporter;
		bool m_IsAccFrozen;

		// city
		// TODO: make all those variables private and use protected getters and setters (issue #269)
		int64 m_level;
		int64 m_xp;
		int64 m_money;
		int m_shit;
		int m_GiftDelay;
	};
	CLoginData m_LoginData;
	enum {
		LOGIN_OFF,
		LOGIN_WAIT,
		LOGIN_DONE
	};

	//ascii animation frames
	char m_aAsciiFrame0[64];
	char m_aAsciiFrame1[64];
	char m_aAsciiFrame2[64];
	char m_aAsciiFrame3[64];
	char m_aAsciiFrame4[64];
	char m_aAsciiFrame5[64];
	char m_aAsciiFrame6[64];
	char m_aAsciiFrame7[64];
	char m_aAsciiFrame8[64];
	char m_aAsciiFrame9[64];
	char m_aAsciiFrame10[64];
	char m_aAsciiFrame11[64];
	char m_aAsciiFrame12[64];
	char m_aAsciiFrame13[64];
	char m_aAsciiFrame14[64];
	char m_aAsciiFrame15[64];

	char m_aAsciiPublishState[4]; // 4 digit int 0 = off 1 = on and each digit stands for different stuff.  1=visible at all 2=profile 3=not used yet 4=not used yet
	int m_AsciiAnimLen; //not used yet
	int m_AsciiAnimSpeed;
	int m_AsciiWatchFrame;
	int m_AsciiWatchTicker;
	int m_AsciiWatchingID;
	int m_AsciiViewsDefault; //direct with ascii view command
	int m_AsciiViewsProfile;

	//zCatch ChillerDragon
	int m_aCatchedID[64];

	bool m_IsDummy;

	// dummy 32 vars
	bool m_Dummy_32dummy;
	int m_Dummy_32look;

	//REAL DUMMY 32 VARS blmapchill police
	int m_DummyModeSpawn;

	//dummy 33 vars (Chillintelligenz)
	long m_ci_lowest_dest_dist; //max long len 2147483647
	long m_ci_latest_dest_dist;


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
	
	//bomb stats
	int m_BombGamesPlayed;
	int m_BombGamesWon;
	int m_BombBanTime;
	int m_BombTicksUnready;

	//block tourna

	bool m_IsBlockTourning;

	bool m_IsBlockDeathmatch;

	//blockwave

	bool m_IsBlockWaving;
	bool m_IsBlockWaveDead;
	bool m_IsBlockWaveWaiting;

	//chidraqul3 (minigame)
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
	int m_ProfileStyle; // 0=default 1=shit 2=social 3=show-off 4=pvp 5=bomber
	int m_ProfileViews;
	char m_ProfileStatus[50];
	char m_ProfileSkype[50];
	char m_ProfileYoutube[50];
	char m_ProfileEmail[50];
	char m_ProfileHomepage[50];
	char m_ProfileTwitter[50];
	char m_LastViewedProfile[32];
	bool m_IsProfileViewLoaded;


	//PvP-arena ChillerDragon
	int m_pvp_arena_tickets;
	//bool m_IsHammerfight; //moved to character and renamed to --> m_IsHammerarena
	int m_PVP_return_posX;
	int m_PVP_return_posY;
	int m_pvp_arena_last_kill_id;
	vec2 m_PVP_return_pos;

	int m_pvp_arena_games_played;
	int m_pvp_arena_kills;
	int m_pvp_arena_deaths;

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

	char m_aFngConfig[4]; //[0] = autojoin [1] = hammertune [2] = coming soon [3] = coming soon

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

	int m_SurvivalKills;
	int m_SurvivalDeaths;
	int m_SurvivalWins;

	//no name chat fix
	void FixForNoName(int ID);
	int m_FixNameID;
	bool m_ShowName;
	bool m_SetRealName;
	int64 m_SetRealNameTick;
	//ID == 1 // chat message
	int m_ChatTeam;
	char m_ChatText[256];
	//ID == 2 // kill message
	int m_MsgKiller;
	int m_MsgWeapon;
	int m_MsgModeSpecial;

	//zCatch ChillerDragon (instagib)
	int m_GrenadeKills;
	int m_GrenadeDeaths;
	int m_GrenadeSpree;
	int m_GrenadeShots;
	int m_GrenadeShotsNoRJ;
	int m_GrenadeWins;
	int m_RifleKills;
	int m_RifleDeaths;
	int m_RifleSpree;
	int m_RifleShots;
	int m_RifleWins;

	//ninjajetpack
	int m_NinjaJetpackBought;

	//spooky ghost
	int m_SpookyGhost;
	int m_SpookyGhostActive;

	int m_RealUseCustomColor;
	char m_RealSkinName[64];
	char m_RealName[64];
	char m_RealClan[64];
	int m_RealColorBody;
	int m_RealColorFeet;

	//spawn weapons
	int m_UseSpawnWeapons;
	int m_SpawnWeaponShotgun;
	int m_SpawnWeaponGrenade;
	int m_SpawnWeaponRifle;
	int m_SpawnShotgunActive;
	int m_SpawnGrenadeActive;
	int m_SpawnRifleActive;

	std::vector< std::vector<CWeapon*> > m_vWeaponLimit;

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
	int64 m_NoboSpawnStop;

	//Account stuff:
	bool m_IsFileAcc;

	//Moderator and SuperModerator
	bool m_IsModerator;
	bool m_IsSuperModerator;
	bool m_IsSuperModSpawn;
	bool m_IsSupporter;

	bool m_IsAccFrozen; //cant use the sql acc if true

	char m_LastLogoutIGN1[32]; 
	char m_LastLogoutIGN2[32];
	char m_LastLogoutIGN3[32];
	char m_LastLogoutIGN4[32];
	char m_LastLogoutIGN5[32];

	int m_iLastLogoutIGN1_usage;
	int m_iLastLogoutIGN2_usage;
	int m_iLastLogoutIGN3_usage;
	int m_iLastLogoutIGN4_usage;
	int m_iLastLogoutIGN5_usage;

	char m_aIP_1[32];
	char m_aIP_2[32];
	char m_aIP_3[32];

	char m_aClan1[32];
	char m_aClan2[32];
	char m_aClan3[32];

	char m_aAccSkin[32];

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
		5  be somewhat active: finish race/get block points/login to account/ TODO: send chat messages that recieve responses
		6  wait 20 sec
		7  solve the antibot captcha command
		8  played quest until finish map/make more than 10 block points
		9  wait 1 minute
	*/
	int m_PlayerHumanLevel;
	int m_PlayerHumanLevelState; // if the level has sublevels
	int64 m_HumanLevelTime;
	void PlayerHumanLevelTick();

	CCaptcha *m_pCaptcha;
	int m_homing_missiles_ammo;



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
	int m_TradeID;
	int64 m_TradeTick;
	int m_GangsterBagMoney;
	char m_aTradeOffer[256];

	int64 m_ShopBotAntiSpamTick;
	int m_ShopBotMesssagesRecieved;

	void JailPlayer(int seconds);
	int m_PoliceRank;
	bool m_PoliceHelper;
	int m_TaserLevel;
	int m_TaserPrice;
	bool m_TaserOn;
	bool m_IsJailed;
	int64 m_JailTime;
	int64 m_EscapeTime;
	char m_aEscapeReason[256];
	bool m_IsJailDoorOpen;
	//bool m_IsJailHammer;
	int m_JailHammer;
	//int m_JailHammerTime;
	int64 m_JailHammerDelay;
	//int m_CorruptID;
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
	char m_aShowHideConfig[16]; // [0]=blockpoints [1]=blockxp [2]=xp [3]=jail [4]=instafeed(1n1) [5]=questprogress [6]=questwarning [7]=instabroadcast


	// quests
	int m_QuestUnlocked; // maybe save this in sql and later people can choose all quests untill unlocked
	int m_QuestLevelUnlocked; // maybe save this in sql and later people can choose all levels untill unlocked
	int m_QuestState; // current quest 0 = not questing
	int m_QuestStateLevel; // current quest level (difficulty)
	int m_QuestLastQuestedPlayerID; // store here the id to make sure in level 3 quest 1 for example he doenst hammer 1 tee 5 times
	int m_QuestProgressValue; // saves the values of m_QuestLastQuestedPlayerID
	int m_QuestProgressValue2;
	bool m_QuestProgressBool;
	int m_QuestPlayerID; // the id of the player which is the quest
	char m_aQuestString[512]; // stores the quest information
	int m_aQuestProgress[2]; // stores the quest progress information
	bool m_QuestFailed;
	bool IsQuesting() { return m_QuestState != QUEST_OFF; }
	enum {
		QUEST_OFF		= 0,
		QUEST_HAMMER	= 1,
		QUEST_BLOCK		= 2,
		QUEST_RACE		= 3,
		QUEST_RIFLE		= 4,
		QUEST_FARM		= 5,
		QUEST_NUM_PLUS_ONE,
		QUEST_NUM		= QUEST_NUM_PLUS_ONE - 1,

		QUEST_NUM_LEVEL	= 9
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

	enum {
		SCORE_TIME,
		SCORE_LEVEL,
		SCORE_BLOCK
	};
	bool m_ScoreFixForDDNet;
	int m_AllowTimeScore;
	int m_DisplayScore;

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
	int m_shit;
	bool m_MoneyTilePlus;
	bool m_fake_admin;
	//int64 m_LastGift;
	int m_GiftDelay; //is still in sql as LastGift
	int64 m_LastFight;

	char m_aAccountLoginName[32];
	char m_aChangePassword[MAX_PW_LEN+1];
	char m_aAccountPassword[MAX_PW_LEN+1];
	char m_aAccountRegDate[32];

	char m_aSetPassword[MAX_PW_LEN+1]; //admin sql save string (used to resett passwords)
	char m_aSQLNameName[32]; //used to save account name admins interact with in the '/sql_name' command

	void ChangePassword();
	void Save(int SetLoggedIn);
	void SaveFileBased(int SetLoggedIn);
	void Logout(int SetLoggedIn = 0);
	void CheckLevel();
	void CalcExp();

	bool m_HammerRequest;

	int m_DummyMode;
	int m_dmm25; //change dummy modes in the mode 25  ( choose sub modes)
	float m_Dummy_nn_latest_Distance;
	float m_Dummy_nn_highest_Distance;
	float m_Dummy_nn_highest_Distance_touched;
	float m_Dummy_nn_latest_fitness;
	float m_Dummy_nn_highest_fitness;
	int m_Dummy_nn_time;

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

	int m_BlockSkill;
	int m_BlockPoints; //KILLS + other stuff like block tournaments won 
	int m_BlockPoints_Kills; //Block points (blocked others)
	int m_BlockPoints_Deaths; //Block -points (blocked by others)
	//bool m_BlockWasTouchedAndFreezed;  //This bool is used for: check if someone was touched and freezed and if we have this info we can set the touch id to -1 if this bool is true and he is unfreeze ---> if you get blocked and unfreezed agian and suicide you wont block die
	int m_SpawnBlocks;
	int m_BlockSpreeHighscore;

	/*
		m_LastToucherID

		Use UpdateLastToucher() to set this variable
		it tracks the id of the last person who touched it
		used for block kills
	*/
	int m_LastToucherID;
	int m_LastTouchTicks;
	char m_aLastToucherName[64];
	void UpdateLastToucher(int ID);

	int m_BlockBounty;

	//bool m_hammerfight;
	//bool m_isHeal;
	bool m_ninjasteam;
	bool m_disarm;
	//bool m_freezeShotgun;
	int m_RainbowColor;


	char m_aWrongLogin[256];

#if defined(CONF_SQL)
	int64 m_LastSQLQuery;
#endif

	int64 m_LastWarning;
	int m_ChilliWarnings;
	bool m_TROLL166;
	bool m_TROLL420;
	bool m_RconFreeze;

	private: // private ddnet+++
	int m_AccountID;
	int m_level;
	int64 m_xp;
	int64 m_neededxp;
	int64 m_money;
};

#endif
