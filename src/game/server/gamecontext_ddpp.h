#ifndef IN_CLASS_GAMECONTEXT
#include <deque>
#include <engine/antibot.h>
#include <game/generated/protocol.h>
#include <game/generated/server_data.h>
#include <game/server/entities/stable_projectile.h>
#include <game/server/entity.h>
#include <game/server/save.h>

#include <game/gamecore.h>

#include <vector>

#include "minigames/minigame_base.h"

#include "minigames/balance.h"
#include "minigames/block_tournament.h"

#include "ddpp/dummymode.h"

#include "drop_pickup.h"
#include "weapon.h"
class CGameContext : public IGameServer
{
#endif

	class CAccounts *m_pAccounts;
	class CShop *m_pShop;
	class CLoc *m_pLoc;

public:
	// DDRace & DDnetPlusPlus (ddpp)
	//ChillerDragon

	void ConstructDDPP();

	virtual void LogoutAllPlayers() override;
	virtual void OnStartBlockTournament() override;
	//virtual void OnDDPPshutdown();

	class CAccounts *Accounts() { return m_pAccounts; }
	class CShop *Shop() { return m_pShop; }
	const char *Loc(const char *pStr, int ClientID);

	void ShowProfile(int ViewerID, int ViewedID);
	void ShowAdminWelcome(int ID);
	int PrintSpecialCharUsers(int ID);
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
	void SetSpawnweapons(bool Active, int ClientID);

	void ChatCommands();
	void DummyChat();
	void SaveWrongLogin(const char *pLogin);
	//Instagib Survival
	void WinInsta1on1(int WinnerID, int LooserID);
	bool CanJoinInstaArena(bool grenade, bool PrivateMatch);
	int m_insta_survival_gamestate; //0=prepearing 1=running 2=deathmatch
	int m_insta_survival_delay;
	int GetNextClientID();

	// dummy
	void CreateBasicDummys();
	int CreateNewDummy(EDummyMode Mode, bool Silent = false, int Tile = 0);

	//usefull everywhere
	void AbuseMotd(const char *pMsg, int ClientID);
	int IsMinigame(int playerID);
	EScore MinigameScoreType(int ClientID);
	bool IsDDPPgametype(const char *pGametype);
	int GetCIDByName(const char *pName);
	int CountConnectedPlayers();
	int CountConnectedHumans();
	int CountIngameHumans();
	int CountConnectedBots();
	int CountTimeoutCodePlayers();
	bool IsServerEmpty() { return m_IsServerEmpty; }
	void CheckServerEmpty();
	bool IsAllowedCharSet(const char *pStr);
	int GetPlayerByTimeoutcode(const char *pTimeout);
	void GetSpreeType(int ClientID, char *pBuf, size_t BufSize, bool IsRecord = false);
	void LogoutAllPlayersMessage();

	bool ShowJoinMessage(int ClientID);
	bool ShowLeaveMessage(int ClientID);
	bool ShowTeamSwitchMessage(int ClientID);

	enum
	{
		CON_SHOW_NONE,
		CON_SHOW_JOIN,
		CON_SHOW_JOIN_LEAVE,
		CON_SHOW_ALL,
	};
	enum
	{
		// CHAT_ALL = -2,
		// CHAT_SPEC = -1,
		// CHAT_RED = 0,
		// CHAT_BLUE = 1,
		// CHAT_WHISPER_SEND = 2,
		// CHAT_WHISPER_RECV = 3,
		CHAT_TO_ONE_CLIENT = 4,

		// CHAT_SIX = 1 << 0,
		// CHAT_SIXUP = 1 << 1,
	};

	int m_WrongRconAttempts;
	void ConnectAdventureBots();
	CLetters *m_pLetters;

	// sql
	void SQLaccount(int mode, int ClientID, const char *pUsername, const char *pPassword = "");
	void SQLcleanZombieAccounts(int ClientID);

	bool m_ClientLeftServer[MAX_CLIENTS];
	bool AdminChatPing(const char *pMsg);

	/*
		m_LastAccountMode
		keeps track of changes of the sv_account_stuff config
		it is used to logout all players
	*/
	int m_LastAccountMode;

	//shop
	int GetShopBot();
	bool m_CreateShopBot;
	bool m_ShopBotTileExists;

	//                                                                                                    \\ Escaping the escape seceqnze
	//char m_aAllowedCharSet[128] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789&!?*.:+@/\\-_ "; //warning also added space (needed for profile status)
	char m_aAllowedCharSet[128]; //assignment moved to constructor
	void SendBroadcastAll(const char *pText, int importance = 1, bool supermod = false);
	void KillAll();
	bool IsPosition(int playerID, int pos);
	void StartAsciiAnimation(int viewerID, int creatorID, int medium); //0='/ascii view' 1='/profile view'
	bool IsHooked(int hookedID, int power);
	bool IsSameIP(int ID_1, int ID_2);
	void JoinInstagib(int weapon, bool fng, int ID);
	void ShowInstaStats(int requestID, int requestedID);
	void ShowSurvivalStats(int requestID, int requestedID);
	void ShowDDPPStats(int requestID, int requestedID);
	void LeaveInstagib(int ID);
	void SendChatInsta(const char *pMsg, int Weapon);
	void DoInstaScore(int score, int id);
	void CheckInstaWin(int ID);
	void InstaGrenadeRoundEndTick(int ID);
	int m_InstaGrenadeRoundEndTickTicker;
	int m_InstaGrenadeRoundEndDelay; //never set this value directly it is only a storage variable
	int m_InstaGrenadeWinnerID;
	char m_aInstaGrenadeScoreboard[1024];
	void InstaRifleRoundEndTick(int ID);
	int m_InstaRifleRoundEndTickTicker;
	int m_InstaRifleRoundEndDelay; //never set this value directly it is only a storage variable
	int m_InstaRifleWinnerID;
	char m_aInstaRifleScoreboard[1024];
	bool ChillWriteToLine(char const *filename, unsigned lineNo, char const *data);
	int ChillUpdateFileAcc(const char *account, unsigned int line, const char *value, int requestingID);
	int m_LastVoteCallAll;
	void ConnectFngBots(int amount, int mode); //mode=0 rifle mode=1 grenade
	void SaveCosmetics(int id);
	void LoadCosmetics(int id);
	void DeleteCosmetics(int id);
	void CheckDDPPshutdown();

	//chidraqul3 multiplayer
	int C3_GetFreeSlots();
	int C3_GetOnlinePlayers();
	void C3_MultiPlayer_GameTick(int id);
	void C3_RenderFrame();

	//ShowHide load/save configs
	char BoolToChar(bool b);
	bool CharToBool(char c);
	void ShowHideConfigBoolToChar(int id); // full side effect function which stores all showhide bools into the char array
	void ShowHideConfigCharToBool(int id); // full side effect function which loads all the showhide bools from the char array

	//FNN
	void FNN_LoadRun(const char *path, int botID);
	vec2 m_FinishTilePos;
	vec2 GetFinishTile();
	void TestPrintTiles(int botID);
	bool m_IsDebug;

	// double moneytile announcement
	bool MoneyDoubleEnoughPlayers;

	//ddpp init
	float m_FNN_best_distance;
	float m_FNN_best_fitness;
	float m_FNN_best_distance_finish;
	void LoadFNNvalues();

	void DDPP_Tick();
	void DDPP_SlowTick();
	void ChilliClanTick(int i);
	void AsciiTick(int i);

	// missions (singleplayer)
	int m_MissionUnlockedLevel;
	int m_MissionCurrentLevel;
	void LoadSinglePlayer();
	void SaveSinglePlayer();

	struct CBinaryStorage
	{
		int x, space1, space2; // wtf? xd
	};

	// TODO: make this a own class
	void SaveMapPlayerData();
	void LoadMapPlayerData();
	void ReadMapPlayerData(int ClientID = -1); // load debug only output do nothing
	int m_MapsavePlayers;
	int m_MapsaveLoadedPlayers;

	//global chat
	void GlobalChatPrintMessage();
	void GlobalChatUpdateConfirms(const char *pStr);
	char m_aLastPrintedGlobalChatMessage[1024];

	CBlockTournament *m_pBlockTournament;
	CBalance *m_pBalance;

	std::vector<CMinigame *> m_vMinigames;

	//survival

	vec2 GetNextSurvivalSpawn(int ClientID);
	vec2 GetSurvivalLobbySpawn(int ClientID);
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
	void SetPlayerSurvival(int id, int mode);
	int CountSurvivalPlayers(bool Alive = false);
	/*
		SurvivalSetGameState()
		SURVIVAL_OFF
		SURVIVAL_LOBBY
		SURVIVAL_INGAME
		SURVIVAL_DM_COUNTDOWN
		SURVIVAL_DM
	*/
	void SurvivalSetGameState(int state);
	void SurvivalCheckWinnerAndDeathMatch();
	bool SurvivalPickWinner();
	int SurvivalGetRandomAliveID(int NotThis = -1);
	void SurvivalGetNextSpectator(int UpdateID, int KillerID);
	void SurvivalUpdateSpectators(int DiedID, int KillerID);
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

	const char *GetBlockSkillGroup(int id);
	int GetBlockSkillGroupInt(int id);
	void UpdateBlockSkill(int value, int id);

	//blockwave

	void BlockWaveAddBots();
	void BlockWaveWonRound();
	void StartBlockWaveGame();
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

	void QuestReset(int playerID);
	void QuestFailed(int playerID);
	void QuestFailed2(int playerID); //sets fail bool and doest restart
	bool QuestAddProgress(int playerID, int globalMAX, int localMAX = -1);
	void QuestCompleted(int playerID);
	int QuestReward(int playerID);
	//void PickNextQuest(int playerID); //includeded in QuestComplete
	void StartQuest(int playerID);
	int PickQuestPlayer(int playerID);
	void CheckConnectQuestBot();

	//police
	void SendAllPolice(const char *pMessage);
	void AddEscapeReason(int ID, const char *pReason);

	//bank
	bool m_IsBankOpen;

	//balance battels
	void StopBalanceBattle();
	void StartBalanceBattle(int ID1, int ID2);
	void BalanceBattleTick();
	int m_BalanceBattleCountdown;
	int m_BalanceBattleState; // 0=offline 1=preparing 2=ingame
	int m_BalanceID1;
	int m_BalanceID2;
	int m_BalanceDummyID1;
	int m_BalanceDummyID2;

	//bomb
	void SendBroadcastBomb(const char *pMsg);
	void SendChatBomb(const char *pMsg);
	void EndBombGame(int WinnerID);
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
	bool m_bwff; //black whithe flip flip

	// drop pickups
	std::vector<std::vector<CDropPickup *>> m_vDropLimit;

	/*****************
	*     TRADE      *
	******************/
	//  --- selll
	int TradePrepareSell(const char *pToName, int FromID, const char *pItemName, int Price, bool IsPublic);
	int TradeSellCheckUser(const char *pToName, int FromID);
	//int TradeSellCheckItem(const char *pItemName, int FromID); //unused! keept just as backup if the new system isnt good
	//  --- buy
	int TradePrepareBuy(int BuyerID, const char *pSellerName, int ItemID);

	//  --- base
	int TradeItemToInt(const char *pItemName);
	const char *TradeItemToStr(int ItemID);
	int TradeHasItem(int ItemID, int ID);

	bool CheckAccounts(int AccountID);

	void GlobalChat(int ClientID, const char *pMsg);
	bool IsDDPPChatCommand(int ClientID, CPlayer *pPlayer, const char *pCommand);
	bool IsChatMessageBlocked(int ClientID, CPlayer *pPlayer, int Team, const char *pMesage);
	void VotedYes(CCharacter *pChr, CPlayer *pPlayer);
	void VotedNo(CCharacter *pChr);
	bool AbortTeamChange(int ClientID, CPlayer *pPlayer);
	bool AbortKill(int ClientID, CPlayer *pPlayer, CCharacter *pChr);
	/*
		CallVetoVote

		Veto votes only pass if nobody voted agianst it
		(vote yes doesnt count at all so if nobody votes yes or no the vote will pass)
	*/
	void CallVetoVote(int ClientID, const char *pDesc, const char *pCmd, const char *pReason, const char *pChatmsg, const char *pSixupDesc = 0);
	bool m_IsDDPPVetoVote;

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

	struct CPVPArenaSpawn // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CPVPArenaSpawn> m_PVPArenaSpawn;

	struct CVanillaMode // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CVanillaMode> m_VanillaMode;

	struct CDDraceMode // probably doesn't belong here, but whatever
	{
		int m_NumContestants;
		vec2 m_Center;
	};
	std::vector<CDDraceMode> m_DDraceMode;

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

	void InitDDPPScore(int ClientID);
	void DestructDDPP();

	static const int LOGIN_BAN_DELAY = 60 * 60 * 12; // reset login attempts counter every day
	static const int REGISTER_BAN_DELAY = 60 * 60 * 12 * 7; // reset register attempts counter every week
	static const int NAMECHANGE_BAN_DELAY = 60 * 60 * 12; // reset namechange counter every day
	void RegisterBanCheck(int ClientID);
	void LoginBanCheck(int ClientID);
	void CheckDeleteLoginBanEntry(int ClientID);
	void CheckDeleteRegisterBanEntry(int ClientID);
	void CheckDeleteNamechangeMuteEntry(int ClientID);
	int64_t NameChangeMuteCheck(int ClientID);
	void SetIpJailed(int ClientID);
	bool CheckIpJailed(int ClientID);

	virtual void IncrementWrongRconAttempts() override;

private:
	bool InitTileDDPP(int Index, int x, int y);
	void OnClientEnterDDPP(int ClientID);
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
	NETADDR m_aJailIPs[MAX_JAILS];
	int m_NumRegisterBans;
	int m_NumLoginBans;
	int m_NumNameChangeMutes;
	int m_NumJailIPs;
	void RegisterBan(NETADDR *Addr, int Secs, const char *pDisplayName);
	void LoginBan(NETADDR *Addr, int Secs, const char *pDisplayName);
	void NameChangeMute(NETADDR *Addr, int Secs, const char *pDisplayName);
	int64_t NameChangeMuteTime(int ClientID);

	static void ConFreezeLaser(IConsole::IResult *pResult, void *pUserData);
	static void ConDestroyLaser(IConsole::IResult *pResult, void *pUserData);
	static void ConBuy(IConsole::IResult *pResult, void *pUserData);
	static void ConShop(IConsole::IResult *pResult, void *pUserData);

	static void ConRegisterBan(IConsole::IResult *pResult, void *pUserData);
	static void ConRegisterBanID(IConsole::IResult *pResult, void *pUserData);
	static void ConRegisterBanIP(IConsole::IResult *pResult, void *pUserData);
	static void ConUnRegisterBan(IConsole::IResult *pResult, void *pUserData);
	static void ConRegisterBans(IConsole::IResult *pResult, void *pUserData);

	static void ConLoginBan(IConsole::IResult *pResult, void *pUserData);
	static void ConLoginBanID(IConsole::IResult *pResult, void *pUserData);
	static void ConLoginBanIP(IConsole::IResult *pResult, void *pUserData);
	static void ConUnLoginBan(IConsole::IResult *pResult, void *pUserData);
	static void ConLoginBans(IConsole::IResult *pResult, void *pUserData);

	static void ConNameChangeMute(IConsole::IResult *pResult, void *pUserData);
	static void ConNameChangeMuteID(IConsole::IResult *pResult, void *pUserData);
	static void ConNameChangeMuteIP(IConsole::IResult *pResult, void *pUserData);
	static void ConNameChangeUnmute(IConsole::IResult *pResult, void *pUserData);
	static void ConNameChangeMutes(IConsole::IResult *pResult, void *pUserData);

	static void ConDummies(IConsole::IResult *pResult, void *pUserData);

	static void ConHammer(IConsole::IResult *pResult, void *pUserData);
	static void ConFreeze(IConsole::IResult *pResult, void *pUserData);
	static void ConUnFreeze(IConsole::IResult *pResult, void *pUserData);

	static void ConCC(IConsole::IResult *pResult, void *pUserData);
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
	static void Conhammerfight(IConsole::IResult *pResult, void *pUserData);
	static void ConfreezeShotgun(IConsole::IResult *pResult, void *pUserData);
	static void ConDamage(IConsole::IResult *pResult, void *pUserData);
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

	//SQL
	static void ConSQL_ADD(IConsole::IResult *pResult, void *pUserData);

	//rcon api
	static void ConRconApiSayID(IConsole::IResult *pResult, void *pUserData);
	static void ConRconApiAlterTable(IConsole::IResult *pResult, void *pUserData);

	//account stuff
	static void ConChangePassword(IConsole::IResult *pResult, void *pUserData);
	static void ConAccLogout(IConsole::IResult *pResult, void *pUserData);
	static void ConLogin(IConsole::IResult *pResult, void *pUserData);
	static void ConRegister(IConsole::IResult *pResult, void *pUserData);
	static void ConLogin2(IConsole::IResult *pResult, void *pUserData);
	static void ConRegister2(IConsole::IResult *pResult, void *pUserData);
	static void ConACC2(IConsole::IResult *pResult, void *pUserData);
	static void ConSQL(IConsole::IResult *pResult, void *pUserData);
	static void ConSQLName(IConsole::IResult *pResult, void *pUserData);
	static void ConSQLLogout(IConsole::IResult *pResult, void *pUserData);
	static void ConSQLLogoutAll(IConsole::IResult *pResult, void *pUserData);
	static void ConAcc_Info(IConsole::IResult *pResult, void *pUserData);
	static void ConStats(IConsole::IResult *pResult, void *pUserData);
	static void ConProfile(IConsole::IResult *pResult, void *pUserData);
	static void ConAscii(IConsole::IResult *pResult, void *pUserData);

	static void ConActivateShopItem(IConsole::IResult *pResult, void *pUserData);
	static void ConDeactivateShopItem(IConsole::IResult *pResult, void *pUserData);

	//minigame (chidraqul)
	static void ConChidraqul(IConsole::IResult *pResult, void *pUserData);

	static void ConMinigames(IConsole::IResult *pResult, void *pUserData);

	static void ConToggleSpawn(IConsole::IResult *pResult, void *pUserData);

	//display score
	static void ConScore(IConsole::IResult *pResult, void *pUserData);

	//spooky ghost
	static void ConSpookyGhostInfo(IConsole::IResult *pResult, void *pUserData);

	//spawn weapons
	static void ConSpawnWeapons(IConsole::IResult *pResult, void *pUserData);
	static void ConSpawnWeaponsInfo(IConsole::IResult *pResult, void *pUserData);

	//supermod
	static void ConSayServer(IConsole::IResult *pResult, void *pUserData);
	static void ConBroadcastServer(IConsole::IResult *pResult, void *pUserData);

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

	static void ConRoom(IConsole::IResult *pResult, void *pUserData);
	static void ConSpawn(IConsole::IResult *pResult, void *pUserData);
	static void ConGodmode(IConsole::IResult *pResult, void *pUserData);
	static void ConHook(IConsole::IResult *pResult, void *pUserData);
	static void ConQuest(IConsole::IResult *pResult, void *pUserData);
	static void ConBounty(IConsole::IResult *pResult, void *pUserData);
	static void ConFng(IConsole::IResult *pResult, void *pUserData);

	//admin
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

	bool DDPPCredits();
	bool DDPPInfo();
	bool DDPPPoints(IConsole::IResult *pResult, void *pUserData);

#ifndef IN_CLASS_GAMECONTEXT
}
#endif
