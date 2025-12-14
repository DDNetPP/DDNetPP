// This file can be included several times.

#ifndef IN_CLASS_GAMECONTEXT
#include "ddpp/dummymode.h"
#include "drop_pickup.h"
#include "minigames/balance.h"
#include "minigames/block_tournament.h"
#include "minigames/blockwave.h"
#include "minigames/bomb.h"
#include "minigames/instagib.h"
#include "minigames/minigame_base.h"
#include "minigames/one_vs_one_block.h"
#include "minigames/pvp_arena.h"
#include "minigames/survival.h"
#include "minigames/tdm_block.h"
#include "weapon.h"

#include <engine/antibot.h>
#include <engine/http.h>

#include <generated/protocol.h>
#include <generated/server_data.h>

#include <game/gamecore.h>
#include <game/server/ddpp/enums.h>
#include <game/server/ddpp/letters.h>
#include <game/server/entities/stable_projectile.h>
#include <game/server/entity.h>
#include <game/server/save.h>
#include <game/server/twbl/callback_ctx.h>

#include <server/ddnet_callback_ctx.h>

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
class CGameContext : public IGameServer
{
#endif

	class CAccounts *m_pAccounts;
	class CShop *m_pShop;
	class CLoc *m_pLoc;

	// See the setter and getter for documentation
	// `SetReconnectFlood()` and `ReconnectFlood()`
	bool m_IsReconnectFloodDetected = false;

public:
	const char *ServerInfoClientScoreKind() override;
	int ServerInfoClientScoreValue(int ClientId) override;

	// Sets the server in join/leave spam alert mode
	// this should be called when a high amount of reconnects is detected
	// and again if it stopped
	//
	// this is called from the ddnet++ controller
	void SetReconnectFlood(bool ActiveFlooding) { m_IsReconnectFloodDetected = ActiveFlooding; }

	// true if the server detected reconnect spam that is flooding the chat
	// It then tries to silence all spamming/automated reconnects
	// while still printing the join and leave messages of real players
	bool ReconnectFlood() { return m_IsReconnectFloodDetected; }

	// DDRace & DDnetPlusPlus (ddpp)
	//ChillerDragon

	void ConstructDDPP(int Resetting);

	virtual void LogoutAllPlayers() override;
	virtual void OnStartBlockTournament() override;
	//virtual void OnDDPPshutdown();

	class CAccounts *Accounts() { return m_pAccounts; }
	class CShop *Shop() { return m_pShop; }
	[[gnu::format_arg(2)]] const char *Loc(const char *pStr, int ClientId) const;

	// calls SendChatTarget under the hood
	// the pFormat string will be translated so make sure
	// it matches the text in server/ddpp/loc.cpp
	[[gnu::format(printf, 3, 4)]] void SendChatLoc(int ClientId, const char *pFormat, ...) const;

	// calls SendChatTarget under the hood
	// the pFormat string will be translated so make sure
	// it matches the text in server/ddpp/loc.cpp
	// ddnet++ often uses [systemname] prefixes for chat messages
	// to allow sharing translation between systems the system is separat
	// no square brackets needed for the system name
	//
	// Example:
	//
	//  SendChatLoc(ClientId, "ACCOUNTS", "You logged in");
	//
	// See also `SendChatLoc()` if you need no system prefix.
	[[gnu::format(printf, 4, 5)]] void SendChatLocSys(int ClientId, const char *pSystem, const char *pFormat, ...) const;

	// Uses `log_info("chatresp", "message..");` under the hood
	// use this to translate replies to commands
	// if ran from rcon it will reply in rcon and if ran in chat it will reply in chat
	// if ran in econ it will reply in econ.
	//
	// The `ClientId` is only used for translation not to chose the msg target
	// the message target is chosen by the logging system of ddnet
	//
	// Example:
	//
	//  ChatrespLocSys("ACCOUNTS", "You logged in");
	//
	// See also `SendChatLocSys()` if you want to send a targeted line instead of relying on the logger
	[[gnu::format(printf, 4, 5)]] void ChatrespLocSys(int ClientId, const char *pSystem, const char *pFormat, ...) const;

	[[gnu::format(printf, 3, 4)]] void ChatrespLoc(int ClientId, const char *pFormat, ...) const;

	// calls SendBroadcast under the hood
	// the pFormat string will be translated so make sure
	// it matches the text in server/ddpp/loc.cpp
	[[gnu::format(printf, 3, 4)]] void SendBroadcastLoc(int ClientId, const char *pFormat, ...);

	// calls SendBroadcast under the hood
	// the pFormat string will be translated so make sure
	// it matches the text in server/ddpp/loc.cpp
	[[gnu::format(printf, 5, 6)]] void SendBroadcastLocImportant(int ClientId, int Importance, bool IsSuperMod, const char *pFormat, ...);

	void ShowProfile(int ViewerId, int ViewedId);
	void ShowAdminWelcome(int ClientId);
	int PrintSpecialCharUsers(int ClientId);
	int TestSurvivalSpawns();

	/*
		LoadMapLive

		description:
			Loads a new map without resetting game state.
			Also do not resend map to in game players only to newly connected.
			This allows to update the map without interrupting gameplay

		bugs:
			default switcher state seems to be bugged
	*/
	void LoadMapLive(const char *pMapName);

	class CModifyTile
	{
	public:
		int m_Group;
		int m_Layer;
		int m_Index;
		int m_Flags;
		int m_X;
		int m_Y;

		CModifyTile(int Group, int Layer, int Index, int Flags, int X, int Y) :
			m_Group(Group),
			m_Layer(Layer),
			m_Index(Index),
			m_Flags(Flags),
			m_X(X),
			m_Y(Y)
		{
		}
	};
	std::vector<CModifyTile> m_vPendingModifyTiles;
	std::mutex m_PendingModifyTilesMutex;

	std::vector<CModifyTile> m_vFinishedModifyTiles;
	std::mutex m_FinishedModifyTilesMutex;

	// register tile to be set in the current map file
	// it will be picked up by the worker thread when it is free
	// the worker thread will then push it to m_FinishedModifyTiles
	// where the main thread can pick it up and load the newly generated map
	// and also inform the clients about the newly placed tiles
	void QueueTileForModify(int Group, int Layer, int Index, int Flags, int X, int Y);

	// should be run in a thread
	static void ModifyTileWorker(CGameContext *pGameServer);
	void ModifyTileWorkerResultTick();
	std::thread m_DDPPWorkerThread;
	std::atomic_bool m_StopDDPPWorkerThread = false;

	void StartDDPPWorkerThreads();
	void StopDDPPWorkerThreads();

	void SetSpawnweapons(bool Active, int ClientId);

	void ChatCommands();
	void DummyChat();
	void WriteWrongLoginJson(int ClientId, const char *pName, const char *pPassword);
	//Instagib Survival
	void WinInsta1on1(int WinnerId, int LooserId);
	bool CanJoinInstaArena(bool grenade, bool PrivateMatch);
	int m_insta_survival_gamestate; //0=prepearing 1=running 2=deathmatch
	int m_insta_survival_delay;
	int GetNextClientId();

	// dummy
	void CreateBasicDummys();
	int CreateNewDummy(EDummyMode Mode, bool Silent = false, int Tile = 0, EDummyTest TestMode = EDummyTest::NONE);

	// run commands using ./DDNetPP "defer say hi"
	// which will run "say hi" after the server fully initialized
	std::vector<std::string> m_vDeferQueue;

	// should be called after every component is fully initialized
	// will execute all commands registered with the "defer" command
	void RunDeferredCommands();

	// runs the command as rcon console command
	// after the server is fully initialized
	void DeferCommand(const char *pCommand);

	// gets decremented every tick
	// if it hits 0 all commands registered with "defer" will be executed
	int m_TicksUntilDefer = 10;

	//useful everywhere
	CPlayer *GetPlayerByAccountId(int AccountId);
	void AbuseMotd(const char *pMsg, int ClientId);

	// it is safe to pass in any ClientId
	// returned value might be null
	CPlayer *GetPlayerOrNullptr(int ClientId) const;

	// deprecated use IsMinigaming() instead
	int IsMinigame(int playerId);

	// also returns true for jail
	bool IsMinigaming(int ClientId);
	EDisplayScore MinigameScoreType(int ClientId);

	// set by the config sv_display_score
	EDisplayScore m_DisplayScore = EDisplayScore::TIME;

	// returns the currently active minigame or nullptr
	//
	// TODO: this could be cached instead of iterating minigames
	//       this could be stored in a CGameContext instance variable
	//       which is updated when the minigame is changed
	CMinigame *GetMinigame(int ClientId);
	bool IsDDPPgametype(const char *pGametype);
	int GetCidByName(const char *pName);
	int CountConnectedPlayers();
	int CountConnectedHumans();
	int CountIngameHumans();
	int CountConnectedBots();
	int CountTimeoutCodePlayers();
	bool IsServerEmpty() { return m_IsServerEmpty; }
	void CheckServerEmpty();
	bool IsAllowedCharSet(const char *pStr);
	int GetPlayerByTimeoutcode(const char *pTimeout);
	void GetSpreeType(int ClientId, char *pBuf, size_t BufSize, bool IsRecord = false);
	void LogoutAllPlayersMessage();
	// wrapper around ddnets always changing http api
	void HttpGetStable(const char *pUrl, const char *pContent);
	// wrapper around ddnets always changing http api
	void HttpPostStable(const char *pUrl, const char *pContent) const;
	void SendDiscordWebhook(const char *pWebhookUrl, const char *pContent) const;

	IHttp *m_pDdppHttp;

	// sends translated chat message to all players
	// announcing a spree milestone
	void SendSpreeMessage(int SpreeingId, int Spree);

	// sends translated chat message to all players announcing a spree was ended
	void SendEndSpreeMessage(int SpreeingId, int Spree, const char *aKiller);

	bool ShowJoinMessage(int ClientId);
	bool ShowLeaveMessage(int ClientId);
	bool ShowTeamSwitchMessage(int ClientId);

	enum
	{
		CON_SHOW_NONE,
		CON_SHOW_JOIN,
		CON_SHOW_JOIN_LEAVE,
		CON_SHOW_ALL,
	};

	int m_WrongRconAttempts;
	void ConnectAdventureBots();
	CLetters *m_pLetters;

	// sql
	void SQLaccount(int mode, int ClientId, const char *pUsername, const char *pPassword = "");
	void SQLcleanZombieAccounts(int ClientId);

	// is set to time_get() when sv_accounts was attempted to be set to 1
	// but it failed because sv_hostname or sv_port were not set yet
	// if sv_hostname or sv_port are set later with a few seconds delay
	// we will then activate sv_accounts
	//
	// this allows any kind of ordering in semicolon separated rcon commands
	// and especially any kind of order in autoexec config files
	//
	// but it will not turn on sv_accounts if some admin manually
	// sets sv_hostname or sv_port minutes after the sv_accounts attempt
	// because that would be weird
	int64_t m_LastAccountTurnOnAttempt = 0;

	// results of the sql worker thread
	// for rcon commands operating on accounts
	std::vector<std::shared_ptr<CAccountRconCmdResult>> m_vAccountRconCmdQueryResults;

	bool AdminChatPing(const char *pMsg);

	//shop
	int GetShopBot();
	bool m_CreateShopBot;
	bool m_ShopBotTileExists;

	// used for DO NOT CHANGE MAP votes
	int64_t m_VotingBlockedUntil = 0;

	//                                                                                                    \\ Escaping the escape seceqnze
	//char m_aAllowedCharSet[128] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789&!?*.:+@/\\-_ "; //warning also added space (needed for profile status)
	char m_aAllowedCharSet[128]; //assignment moved to constructor
	void SendBroadcastAll(const char *pText, int Importance = 1, bool Supermod = false);
	void KillAll();
	bool IsPosition(int PlayerId, int Pos);
	void StartAsciiAnimation(int ViewerId, int CreatorId, int Medium); //0='/ascii view' 1='/profile view'
	bool IsHooked(int HookedId, int Power);
	bool IsSameIp(int ClientId1, int ClientId2) const;
	void ShowInstaStats(int RequestingId, int RequestedId) const;
	void ShowSurvivalStats(int RequestingId, int RequestedId);
	void ShowDDPPStats(int RequestingId, int RequestedId);
	void LeaveInstagib(int Id);
	void SendChatInsta(const char *pMsg, int Weapon);
	void DoInstaScore(int score, int ClientId);
	void CheckInstaWin(int ClientId);
	void InstaGrenadeRoundEndTick(int ClientId);
	int m_InstaGrenadeRoundEndTickTicker;
	int m_InstaGrenadeRoundEndDelay; //never set this value directly it is only a storage variable
	int m_InstaGrenadeWinnerId;
	char m_aInstaGrenadeScoreboard[1024];
	void InstaRifleRoundEndTick(int ClientId);
	int m_InstaRifleRoundEndTickTicker;
	int m_InstaRifleRoundEndDelay; //never set this value directly it is only a storage variable
	int m_InstaRifleWinnerId;
	char m_aInstaRifleScoreboard[1024];
	bool ChillWriteToLine(char const *pFilename, unsigned LineNo, char const *pData);
	int ChillUpdateFileAcc(const char *pUsername, unsigned int Line, const char *pValue, int RequestingId) const;
	int m_LastVoteCallAll;
	void ConnectFngBots(int Amount, int Mode); //mode=0 rifle mode=1 grenade
	void SaveCosmetics(int ClientId);
	void LoadCosmetics(int ClientId);
	void DeleteCosmetics(int ClientId);
	void CheckDDPPshutdown();
	// if it returns false the message should be dropped
	bool DDPPOnMessage(int MsgId, void *pRawMsg, CUnpacker *pUnpacker, int ClientId);

	//chidraqul3 multiplayer
	int C3_GetFreeSlots();
	int C3_GetOnlinePlayers();
	void C3_MultiPlayer_GameTick(int ClientId);
	void C3_RenderFrame();

	//ShowHide load/save configs
	char BoolToChar(bool b);
	bool CharToBool(char c);
	void ShowHideConfigBoolToChar(int ClientId); // full side effect function which stores all showhide bools into the char array
	void ShowHideConfigCharToBool(int ClientId); // full side effect function which loads all the showhide bools from the char array

	//FNN
	void FNN_LoadRun(const char *pPath, int BotId);
	vec2 m_FinishTilePos;
	vec2 GetFinishTile();
	void TestPrintTiles(int BotId);
	bool m_IsDebug;

	// double moneytile announcement
	bool MoneyDoubleEnoughPlayers;

	//ddpp init
	float m_FNN_best_distance;
	float m_FNN_best_fitness;
	float m_FNN_best_distance_finish;
	void LoadFNNvalues();

	// currently using the default ddnet context shipped by twbl
	// CTwblCallbackCtx m_TwblCallbackCtx;
	TWBL::CDDNetCallbackCtx m_TwblCallbackCtx;

	void DDPP_Tick();
	void DDPP_SlowTick();
	void ChilliClanTick(int i);
	void AsciiTick(int i);

	// returns true if a net message was sent to ClientId
	// returns false if there are no more ddnet++ vote menu entries to send
	bool SendExtraVoteMenuEntry(int ClientId);

	// call this when data changed that is displayed in the
	// ddnet++ extra vote menu
	void RefreshExtraVoteMenu(int ClientId);

	// missions (singleplayer)
	int m_MissionUnlockedLevel;
	int m_MissionCurrentLevel;
	void LoadSinglePlayer();
	void SaveSinglePlayer() const;

	struct CBinaryStorage
	{
		int x, space1, space2; // wtf? xd
	};

	// TODO: make this a own class
	void SaveMapPlayerData();
	void LoadMapPlayerData();
	void ReadMapPlayerData(int ClientId = -1); // load debug only output do nothing
	int m_MapsavePlayers;
	int m_MapsaveLoadedPlayers;

	// implements https://github.com/DDNetPP/DDNetPP/issues/356
	// deactivate all police tiles if used by too many tees

	/*
		Variable: m_IsPoliceFarmActive

		updated in the same tick speed as moneytiles update
		AmountPoliceFarmPlayers() is greater or lower than g_Config.m_SvMaxPoliceFarmPlayers
	*/
	bool m_IsPoliceFarmActive;
	/*
		Function: AmountPoliceFarmPlayers()

		returns amount of tees that currently sit on a police money tile
	*/
	int AmountPoliceFarmPlayers();
	void CheckDeactivatePoliceFarm();

	//global chat
	void GlobalChatPrintMessage();
	void GlobalChatUpdateConfirms(const char *pStr);
	char m_aLastPrintedGlobalChatMessage[1024];

	CBlockTournament *m_pBlockTournament = nullptr;
	CBalance *m_pBalance = nullptr;
	CInstagib *m_pInstagib = nullptr;
	CBlockwave *m_pBlockwave = nullptr;
	COneVsOneBlock *m_pOneVsOneBlock = nullptr;
	CTdmBlock *m_pTdmBlock = nullptr;
	CPvpArena *m_pPvpArena = nullptr;
	CSurvival *m_pSurvival = nullptr;
	CBomb *m_pBomb = nullptr;

	std::vector<CMinigame *> m_vMinigames;

	//survival

	vec2 GetNextSurvivalSpawn(int ClientId);
	vec2 GetSurvivalLobbySpawn(int ClientId);
	void SurvivalLobbyTick();
	void SurvivalDeathmatchTick();
	void SurvivalStartGame();
	void SendChatSurvival(const char *pMsg);
	void SendBroadcastSurvival(const char *pMsg, int Importance = 1);
	/*
		SetPlayerSurvival()
		0	SURVIVAL_OFF
		1	SURVIVAL_LOBBY
		2	SURVIVAL_INGAME
		3	SURVIVAL_DIE
	*/
	void SetPlayerSurvival(int ClientId, int Mode);
	int CountSurvivalPlayers(bool Alive = false);
	/*
		SurvivalSetGameState()
		SURVIVAL_OFF
		SURVIVAL_LOBBY
		SURVIVAL_INGAME
		SURVIVAL_DM_COUNTDOWN
		SURVIVAL_DM
	*/
	void SurvivalSetGameState(int State);
	void SurvivalCheckWinnerAndDeathMatch();
	bool SurvivalPickWinner();
	int SurvivalGetRandomAliveId(int NotThis = -1);
	void SurvivalGetNextSpectator(int UpdateId, int KillerId);
	void SurvivalUpdateSpectators(int DiedId, int KillerId);
	/*
		m_survivalgamestate
		0	SURVIVAL_OFF
		1	SURVIVAL_LOBBY
		2	SURVIVAL_INGAME
		3	SURVIVAL_DM_COUNTDOWN
		4	SURVIVAL_DM
		should only be set by SurvivalSetGameState()
	*/
	int m_survivalgamestate;
	int m_survivallobbycountdown;
	int m_survival_dm_countdown;
	int m_survival_game_countdown;
	int m_survival_start_players;
	int m_survival_spawn_counter;
	char m_aLastSurvivalWinnerName[32];
	enum
	{
		SURVIVAL_OFF, // gamestate	playerstate
		SURVIVAL_LOBBY, // gamestate	playerstate
		SURVIVAL_INGAME, // gamestate	playerstate
		SURVIVAL_DM_COUNTDOWN, // gamestate
		SURVIVAL_DM, // gamestate

		SURVIVAL_DIE = 3 // playerstate
	};

	const char *GetBlockSkillGroup(int ClientId);
	int GetBlockSkillGroupInt(int ClientId);
	void UpdateBlockSkill(int Value, int ClientId);

	//blockwave

	void BlockWaveAddBots();
	void BlockWaveRemoveBots();
	void BlockWaveWonRound();
	void StartBlockWaveGame();
	void StopBlockWaveGame();
	void BlockWaveGameTick();
	void BlockWaveEndGame();
	void BlockWaveStartNewGame();
	void SendBroadcastBlockWave(const char *pMsg);
	void SendChatBlockWave(const char *pMsg);
	int CountBlockWavePlayers();
	int m_BlockWaveGameState; // 0=offline 1=preparing 2=round running
	int m_BlockWavePrepareDelay;
	int m_BlockWaveRound;

	//QUESTS

	void QuestReset(int ClientId);
	void QuestFailed(int ClientId);
	void QuestFailed2(int ClientId); //sets fail bool and doest restart
	bool QuestAddProgress(int ClientId, int GlobalMax, int LocalMax = -1);
	void QuestCompleted(int ClientId);
	int QuestReward(int ClientId);
	//void PickNextQuest(int playerId); //includeded in QuestComplete
	void StartQuest(int ClientId);
	int PickQuestPlayer(int ClientId);
	void CheckConnectQuestBot();

	//police
	void SendAllPolice(const char *pMessage);
	void AddEscapeReason(int ClientId, const char *pReason);

	//bank
	bool m_IsBankOpen;

	//balance battels
	void StopBalanceBattle();
	void StartBalanceBattle(int ClientId1, int ClientId2);
	void BalanceBattleTick();
	int m_BalanceBattleCountdown;
	int m_BalanceBattleState; // 0=offline 1=preparing 2=ingame
	int m_BalanceId1;
	int m_BalanceId2;
	int m_BalanceDummyId1;
	int m_BalanceDummyId2;

	//bomb
	void SendBroadcastBomb(const char *pMsg);
	void SendChatBomb(const char *pMsg);
	void EndBombGame(int WinnerId);
	void CheckStartBomb();
	void BombTick();
	int FindNextBomb();
	int CountBannedBombPlayers();
	int CountBombPlayers();
	int CountReadyBombPlayers();
	int m_BombGameState; //0=no bomb game created 1=bomb game created (lobby) 2=bomb game created (lobby)(locked) 3=bomb game created and running (ingame)
	int64_t m_BombMoney;
	int m_BombStartPlayers;
	int m_BombTick; //the ticking bomby ticker ticks until he flicks to zer0 then he kicks kaboooms!
	int m_BombStartCountDown;
	char m_BombMap[32]; //0 = Default 1 = NoArena

	int m_BombFinalColor;
	int m_BombColor;
	bool m_bwff; // black white flip flip

	// drop pickups
	std::vector<std::vector<CDropPickup *>> m_vDropLimit;

	/*****************
	 *     TRADE      *
	 ******************/
	//  --- selll
	int TradePrepareSell(const char *pToName, int FromId, const char *pItemName, int Price, bool IsPublic);
	int TradeSellCheckUser(const char *pToName, int FromId);
	//int TradeSellCheckItem(const char *pItemName, int FromId); //unused! kept just as backup if the new system isnt good
	//  --- buy
	int TradePrepareBuy(int BuyerId, const char *pSellerName, int ItemId);

	//  --- base
	int TradeItemToInt(const char *pItemName);
	const char *TradeItemToStr(int ItemId);
	int TradeHasItem(int ItemId, int Id);

	bool CheckAccounts(int AccountId);

	void GlobalChat(int ClientId, const char *pMsg);
	bool IsDDPPChatCommand(int ClientId, CPlayer *pPlayer, const char *pCommand);
	bool IsChatMessageBlocked(int ClientId, CPlayer *pPlayer, int Team, const char *pMessage);
	void VotedYes(CCharacter *pChr, CPlayer *pPlayer);
	void VotedNo(CCharacter *pChr);
	bool AbortTeamChange(int ClientId, CPlayer *pPlayer);
	bool AbortKill(int ClientId, CPlayer *pPlayer, CCharacter *pChr);
	/*
		CallVetoVote

		Veto votes only pass if nobody voted against it
		(vote yes doesnt count at all so if nobody votes yes or no the vote will pass)
	*/
	void CallVetoVote(int ClientId, const char *pDesc, const char *pCmd, const char *pReason, const char *pChatmsg, const char *pSixupDesc = 0);
	bool m_IsDDPPVetoVote = false;

	//Chiller
	//ChillerDragihn!
	//Chilli

	//int Friends_counter;

	//trading stuff (stock market)

	//char aTestMsg[1024];
	//int TestShareValue;
	int m_CucumberShareValue;

	char aBroadcastMSG[128];
	int m_iBroadcastDelay;

	struct CJail // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CJail> m_Jail;

	struct CJailrelease // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CJailrelease> m_Jailrelease;

	struct CBalanceBattleTile1 // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CBalanceBattleTile1> m_BalanceBattleTile1;

	struct CBalanceBattleTile2 // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CBalanceBattleTile2> m_BalanceBattleTile2;

	struct CSurvivalLobbyTile // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CSurvivalLobbyTile> m_SurvivalLobby;

	struct CSurvivalSpawnTile // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CSurvivalSpawnTile> m_SurvivalSpawn;

	struct CSurvivalDeathmatchTile // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CSurvivalDeathmatchTile> m_SurvivalDeathmatch;

	struct CBlockWaveBotTile // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CBlockWaveBotTile> m_BlockWaveBot;

	struct CBlockWaveHumanTile // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CBlockWaveHumanTile> m_BlockWaveHuman;

	struct CFngScore // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CFngScore> m_FngScore;

	struct CBlockTournaSpawn // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CBlockTournaSpawn> m_BlockTournaSpawn;

	struct CVanillaMode // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CVanillaMode> m_VanillaMode;

	struct CDDRaceMode // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CDDRaceMode> m_DDRaceMode;

	struct CBotSpawn1 // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CBotSpawn1> m_BotSpawn1;

	struct CBotSpawn2 // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CBotSpawn2> m_BotSpawn2;

	struct CBotSpawn3 // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CBotSpawn3> m_BotSpawn3;

	struct CBotSpawn4 // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CBotSpawn4> m_BotSpawn4;

	struct CNoHammer // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CNoHammer> m_NoHammer;

	struct CBlockDMA1 // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CBlockDMA1> m_BlockDMA1;

	struct CBlockDMA2 // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CBlockDMA2> m_BlockDMA2;

	void InitDDPPScore(int ClientId);
	void DestructDDPP();

	static const int LOGIN_BAN_DELAY = 60 * 60 * 12; // reset login attempts counter every day
	static const int REGISTER_BAN_DELAY = 60 * 60 * 12 * 7; // reset register attempts counter every week
	static const int NAMECHANGE_BAN_DELAY = 60 * 60 * 12; // reset namechange counter every day
	void RegisterBanCheck(int ClientId);
	void LoginBanCheck(int ClientId);
	void CheckDeleteLoginBanEntry(int ClientId);
	void CheckDeleteRegisterBanEntry(int ClientId);
	void CheckDeleteNamechangeMuteEntry(int ClientId);
	int64_t NameChangeMuteCheck(int ClientId);
	void SetIpJailed(int ClientId);
	bool CheckIpJailed(int ClientId);

	virtual void IncrementWrongRconAttempts() override;

private:
	bool InitTileDDPP(int Index, int x, int y);
	void OnClientEnterDDPP(int ClientId);
	void OnInitDDPP();

	bool m_IsServerEmpty;

	enum
	{
		MAX_REGISTER_BANS = 128,
		MAX_LOGIN_BANS = 128,
		MAX_NAME_CHANGE_MUTES = 32,
		MAX_JAILS = 16,
	};

	struct CGenericBan
	{
		NETADDR m_Addr;
		int m_Expire;
		int64_t m_LastAttempt;
		int m_NumAttempts;
	};
	CGenericBan m_aRegisterBans[MAX_REGISTER_BANS];
	CGenericBan m_aLoginBans[MAX_LOGIN_BANS];
	CGenericBan m_aNameChangeMutes[MAX_NAME_CHANGE_MUTES];
	NETADDR m_aJailIps[MAX_JAILS];
	int m_NumRegisterBans;
	int m_NumLoginBans;
	int m_NumNameChangeMutes;
	int m_NumJailIps;
	void RegisterBan(const NETADDR *pAddr, int Secs, const char *pDisplayName);
	void LoginBan(const NETADDR *pAddr, int Secs, const char *pDisplayName);
	void NameChangeMute(const NETADDR *pAddr, int Secs, const char *pDisplayName);
	int64_t NameChangeMuteTime(int ClientId);

	void ListSpamfilters();

	// reads the spamfilters.txt file
	// and updates the vector that is used to drop messages in chat
	void ReadSpamfilterList();

	// writes one line to the spamfilters.txt file
	// and also updates the internal vector
	void AddSpamfilter(const char *pFilter);

	// checks if one line of spamfilters.txt
	// is contained in message
	bool IsMessageSpamfiltered(const char *pMessage);

	// if one of these strings is contained in a chat message
	// it will be silently dropped
	std::vector<std::string> m_vSpamfilters;

	static void ConBuy(IConsole::IResult *pResult, void *pUserData);
	static void ConShop(IConsole::IResult *pResult, void *pUserData);

	static void ConRegisterBan(IConsole::IResult *pResult, void *pUserData);
	static void ConRegisterBanId(IConsole::IResult *pResult, void *pUserData);
	static void ConRegisterBanIp(IConsole::IResult *pResult, void *pUserData);
	static void ConUnRegisterBan(IConsole::IResult *pResult, void *pUserData);
	static void ConRegisterBans(IConsole::IResult *pResult, void *pUserData);

	static void ConLoginBan(IConsole::IResult *pResult, void *pUserData);
	static void ConLoginBanId(IConsole::IResult *pResult, void *pUserData);
	static void ConLoginBanIp(IConsole::IResult *pResult, void *pUserData);
	static void ConUnLoginBan(IConsole::IResult *pResult, void *pUserData);
	static void ConLoginBans(IConsole::IResult *pResult, void *pUserData);

	static void ConNameChangeMute(IConsole::IResult *pResult, void *pUserData);
	static void ConNameChangeMuteId(IConsole::IResult *pResult, void *pUserData);
	static void ConNameChangeMuteIp(IConsole::IResult *pResult, void *pUserData);
	static void ConNameChangeUnmute(IConsole::IResult *pResult, void *pUserData);
	static void ConNameChangeMutes(IConsole::IResult *pResult, void *pUserData);

	static void ConDummies(IConsole::IResult *pResult, void *pUserData);

	static void ConHammer(IConsole::IResult *pResult, void *pUserData);

	static void ConShow(IConsole::IResult *pResult, void *pUserData);
	static void ConHide(IConsole::IResult *pResult, void *pUserData);

	// cosmetics
	static void ConRainbow(IConsole::IResult *pResult, void *pUserData);
	static void ConOldRainbow(IConsole::IResult *pResult, void *pUserData);
	static void ConInfRainbow(IConsole::IResult *pResult, void *pUserData);
	static void ConBloody(IConsole::IResult *pResult, void *pUserData);
	static void ConOldBloody(IConsole::IResult *pResult, void *pUserData);
	static void ConInfBloody(IConsole::IResult *pResult, void *pUserData);
	static void ConAtom(IConsole::IResult *pResult, void *pUserData);
	static void ConOldAutoSpreadGun(IConsole::IResult *pResult, void *pUserData);
	static void ConInfAutoSpreadGun(IConsole::IResult *pResult, void *pUserData);
	static void ConAutoSpreadGun(IConsole::IResult *pResult, void *pUserData);
	static void ConOldAtom(IConsole::IResult *pResult, void *pUserData);
	static void ConInfAtom(IConsole::IResult *pResult, void *pUserData);
	static void ConTrail(IConsole::IResult *pResult, void *pUserData);
	static void ConOldTrail(IConsole::IResult *pResult, void *pUserData);
	static void ConInfTrail(IConsole::IResult *pResult, void *pUserData);
	static void ConDropHealth(IConsole::IResult *pResult, void *pUserData);
	static void ConDropArmor(IConsole::IResult *pResult, void *pUserData);

	static void ConHomingMissile(IConsole::IResult *pResult, void *pUserData);

	static void ConBlockVotes(IConsole::IResult *pResult, void *pUserData);
	static void ConUnblockVotes(IConsole::IResult *pResult, void *pUserData);

	//minigame (chidraqul)
	static void ConChidraqul(IConsole::IResult *pResult, void *pUserData);
	static void ConMinigames(IConsole::IResult *pResult, void *pUserData);
	static void ConToggleSpawn(IConsole::IResult *pResult, void *pUserData);

	//display score
	static void ConScore(IConsole::IResult *pResult, void *pUserData);

	//gun
	static void ConHeartGun(IConsole::IResult *pResult, void *pUserData);

	static void ConFreezeHammer(IConsole::IResult *pResult, void *pUserData);
	static void ConLaserGun(IConsole::IResult *pResult, void *pUserData);

	//spooky ghost
	static void ConSpookyGhostInfo(IConsole::IResult *pResult, void *pUserData);

	//spawn weapons
	static void ConSpawnWeapons(IConsole::IResult *pResult, void *pUserData);
	static void ConSpawnWeaponsInfo(IConsole::IResult *pResult, void *pUserData);

	//supermod
	static void ConSayServer(IConsole::IResult *pResult, void *pUserData);
	static void ConBroadcastServer(IConsole::IResult *pResult, void *pUserData);
	static void ConLaserText(IConsole::IResult *pResult, void *pUserData);

	//police
	static void ConPolicehelper(IConsole::IResult *pResult, void *pUserData);
	//static void ConTaserinfo(IConsole::IResult *pResult, void *pUserData);
	static void ConPoliceInfo(IConsole::IResult *pResult, void *pUserData);
	//static void ConPolicetaser(IConsole::IResult *pResult, void *pUserData);
	static void ConPoliceChat(IConsole::IResult *pResult, void *pUserData);
	static void ConJail(IConsole::IResult *pResult, void *pUserData);
	static void ConJailCode(IConsole::IResult *pResult, void *pUserData);
	static void ConReport(IConsole::IResult *pResult, void *pUserData);
	static void ConTaser(IConsole::IResult *pResult, void *pUserData);
	static void ConWanted(IConsole::IResult *pResult, void *pUserData);

	//money
	static void ConMoney(IConsole::IResult *pResult, void *pUserData);
	static void ConPay(IConsole::IResult *pResult, void *pUserData);
	static void ConBank(IConsole::IResult *pResult, void *pUserData);
	static void ConGangsterBag(IConsole::IResult *pResult, void *pUserData);
	static void ConGift(IConsole::IResult *pResult, void *pUserData);
	static void ConTrade(IConsole::IResult *pResult, void *pUserData);
	static void ConTr(IConsole::IResult *pResult, void *pUserData);

	static void ConBalance(IConsole::IResult *pResult, void *pUserData);
	static void ConInsta(IConsole::IResult *pResult, void *pUserData);
	static void ConJoin(IConsole::IResult *pResult, void *pUserData); //join the current event
	static void ConBlock(IConsole::IResult *pResult, void *pUserData);
	static void ConPvpArena(IConsole::IResult *pResult, void *pUserData);
	static void ConEvent(IConsole::IResult *pResult, void *pUserData);

	//info
	static void ConAccountInfo(IConsole::IResult *pResult, void *pUserData);
	//static void ConProfileInfo(IConsole::IResult *pResult, void *pUserData);
	static void ConOfferInfo(IConsole::IResult *pResult, void *pUserData);
	static void ConViewers(IConsole::IResult *pResult, void *pUserData);
	static void ConIp(IConsole::IResult *pResult, void *pUserData);

	static void ConChangelog(IConsole::IResult *pResult, void *pUserData);
	static void ConDDPPLogs(IConsole::IResult *pResult, void *pUserData);

	static void ConGive(IConsole::IResult *pResult, void *pUserData);

	static void ConStockMarket(IConsole::IResult *pResult, void *pUserData);
	static void ConCaptcha(IConsole::IResult *pResult, void *pUserData);
	static void ConHumanLevel(IConsole::IResult *pResult, void *pUserData);
	static void ConLang(IConsole::IResult *pResult, void *pUserData);

	static void ConPoop(IConsole::IResult *pResult, void *pUserData);

	static void ConBomb(IConsole::IResult *pResult, void *pUserData);
	static void ConSurvival(IConsole::IResult *pResult, void *pUserData);
	static void ConBlockWave(IConsole::IResult *pResult, void *pUserData);
	static void ConOneVsOneBlock(IConsole::IResult *pResult, void *pUserData);
	static void ConTdm(IConsole::IResult *pResult, void *pUserData);
	static void ConLeave(IConsole::IResult *pResult, void *pUserData);

	static void ConRoom(IConsole::IResult *pResult, void *pUserData);
	static void ConSpawn(IConsole::IResult *pResult, void *pUserData);
	static void ConHook(IConsole::IResult *pResult, void *pUserData);
	static void ConQuest(IConsole::IResult *pResult, void *pUserData);
	static void ConBounty(IConsole::IResult *pResult, void *pUserData);
	static void ConFng(IConsole::IResult *pResult, void *pUserData);

	// admin rcon
	static void ConFreezeLaser(IConsole::IResult *pResult, void *pUserData);
	static void ConGodmode(IConsole::IResult *pResult, void *pUserData);
	static void ConHidePlayer(IConsole::IResult *pResult, void *pUserData);
	static void ConVerifyPlayer(IConsole::IResult *pResult, void *pUserData);
	static void ConReloadSpamfilters(IConsole::IResult *pResult, void *pUserData);
	static void ConListSpamfilters(IConsole::IResult *pResult, void *pUserData);
	static void ConAddSpamfilter(IConsole::IResult *pResult, void *pUserData);
	static void Condisarm(IConsole::IResult *pResult, void *pUserData);
	static void Condummymode(IConsole::IResult *pResult, void *pUserData);
	static void ConDummyColor(IConsole::IResult *pResult, void *pUserData);
	static void ConDummySkin(IConsole::IResult *pResult, void *pUserData);
	static void ConForceColor(IConsole::IResult *pResult, void *pUserData);
	static void ConForceSkin(IConsole::IResult *pResult, void *pUserData);
	static void Conheal(IConsole::IResult *pResult, void *pUserData);
	static void Conninjasteam(IConsole::IResult *pResult, void *pUserData);
	static void ConForceUnJail(IConsole::IResult *pResult, void *pUserData);
	static void ConForceJail(IConsole::IResult *pResult, void *pUserData);
	static void ConHammerfightMode(IConsole::IResult *pResult, void *pUserData); //this is the hammerfightmode rcon command
	static void ConfreezeShotgun(IConsole::IResult *pResult, void *pUserData);
	static void ConDamage(IConsole::IResult *pResult, void *pUserData);

	// admin chat
	static void ConCC(IConsole::IResult *pResult, void *pUserData);
	static void ConDcDummy(IConsole::IResult *pResult, void *pUserData);
	static void ConTROLL166(IConsole::IResult *pResult, void *pUserData);
	static void ConTROLL420(IConsole::IResult *pResult, void *pUserData);
	static void ConTCMD3000(IConsole::IResult *pResult, void *pUserData);
	static void ConAntiFlood(IConsole::IResult *pResult, void *pUserData);
	static void ConAdmin(IConsole::IResult *pResult, void *pUserData);
	static void ConFNN(IConsole::IResult *pResult, void *pUserData);
	static void ConAdminChat(IConsole::IResult *pResult, void *pUserData);
	static void ConLive(IConsole::IResult *pResult, void *pUserData);
	static void ConRegex(IConsole::IResult *pResult, void *pUserData);
	static void ConMapsave(IConsole::IResult *pResult, void *pUserData);
	static void Conhammerfight(IConsole::IResult *pResult, void *pUserData);

	//SQL
	static void ConSql_ADD(IConsole::IResult *pResult, void *pUserData);

	static void ConRunTest(IConsole::IResult *pResult, void *pUserData);
	static void ConDeferCommand(IConsole::IResult *pResult, void *pUserData);

	//rcon api
	static void ConRconApiSayId(IConsole::IResult *pResult, void *pUserData);
	static void ConRconApiAlterTable(IConsole::IResult *pResult, void *pUserData);

	//account stuff
	static void ConChangePassword(IConsole::IResult *pResult, void *pUserData);
	static void ConAccLogout(IConsole::IResult *pResult, void *pUserData);
	static void ConLogin(IConsole::IResult *pResult, void *pUserData);
	static void ConRegister(IConsole::IResult *pResult, void *pUserData);
	static void ConLogin2(IConsole::IResult *pResult, void *pUserData);
	static void ConRegister2(IConsole::IResult *pResult, void *pUserData);
	static void ConACC2(IConsole::IResult *pResult, void *pUserData);
	static void ConSql(IConsole::IResult *pResult, void *pUserData);
	static void ConSqlName(IConsole::IResult *pResult, void *pUserData);
	static void ConSqlLogout(IConsole::IResult *pResult, void *pUserData);
	static void ConSqlLogoutAll(IConsole::IResult *pResult, void *pUserData);
	static void ConAcc_Info(IConsole::IResult *pResult, void *pUserData);
	static void ConStats(IConsole::IResult *pResult, void *pUserData);
	static void ConProfile(IConsole::IResult *pResult, void *pUserData);
	static void ConAscii(IConsole::IResult *pResult, void *pUserData);

	static void ConSetShopItemPrice(IConsole::IResult *pResult, void *pUserData);
	static void ConSetShopItemDescription(IConsole::IResult *pResult, void *pUserData);
	static void ConSetShopItemLevel(IConsole::IResult *pResult, void *pUserData);
	static void ConActivateShopItem(IConsole::IResult *pResult, void *pUserData);
	static void ConDeactivateShopItem(IConsole::IResult *pResult, void *pUserData);
	static void ConDeactivateAllShopItems(IConsole::IResult *pResult, void *pUserData);
	static void ConActivateAllShopItems(IConsole::IResult *pResult, void *pUserData);

	static void ConchainCaptchaRoom(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainDisplayScore(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainAccounts(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);

	// rcon_configs.cpp
	void RegisterDDNetPPCommands();

	bool DDPPCredits();
	bool DDPPInfo();
	bool DDPPPoints(IConsole::IResult *pResult, void *pUserData);

#ifndef IN_CLASS_GAMECONTEXT
}
#endif
