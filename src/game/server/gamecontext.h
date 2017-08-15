/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMECONTEXT_H
#define GAME_SERVER_GAMECONTEXT_H

#include <engine/server.h>
#include <engine/console.h>
#include <engine/shared/memheap.h>

#include <game/layers.h>
#include <game/voting.h>

#include "eventhandler.h"
#include "gamecontroller.h"
#include "gameworld.h"
#include "player.h"

#include "db_sqlite3.h"

#include "score.h"
#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif
/*
	Tick
		Game Context (CGameContext::tick)
			Game World (GAMEWORLD::tick)
				Reset world if requested (GAMEWORLD::reset)
				All entities in the world (ENTITY::tick)
				All entities in the world (ENTITY::tick_defered)
				Remove entities marked for deletion (GAMEWORLD::remove_entities)
			Game Controller (GAMECONTROLLER::tick)
			All players (CPlayer::tick)


	Snap
		Game Context (CGameContext::snap)
			Game World (GAMEWORLD::snap)
				All entities in the world (ENTITY::snap)
			Game Controller (GAMECONTROLLER::snap)
			Events handler (EVENT_HANDLER::snap)
			All players (CPlayer::snap)

*/

enum
{
	NUM_TUNINGZONES = 256
};

class CGameContext : public IGameServer
{
	IServer *m_pServer;
	class IConsole *m_pConsole;
	CLayers m_Layers;
	CCollision m_Collision;
	CNetObjHandler m_NetObjHandler;
	CTuningParams m_Tuning;
	CTuningParams m_TuningList[NUM_TUNINGZONES];

	static void ConTuneParam(IConsole::IResult *pResult, void *pUserData);
	static void ConTuneReset(IConsole::IResult *pResult, void *pUserData);
	static void ConTuneDump(IConsole::IResult *pResult, void *pUserData);
	static void ConTuneZone(IConsole::IResult *pResult, void *pUserData);
	static void ConTuneDumpZone(IConsole::IResult *pResult, void *pUserData);
	static void ConTuneResetZone(IConsole::IResult *pResult, void *pUserData);
	static void ConTuneSetZoneMsgEnter(IConsole::IResult *pResult, void *pUserData);
	static void ConTuneSetZoneMsgLeave(IConsole::IResult *pResult, void *pUserData);
	static void ConSwitchOpen(IConsole::IResult *pResult, void *pUserData);
	static void ConPause(IConsole::IResult *pResult, void *pUserData);
	static void ConChangeMap(IConsole::IResult *pResult, void *pUserData);
	static void ConRandomMap(IConsole::IResult *pResult, void *pUserData);
	static void ConRandomUnfinishedMap(IConsole::IResult *pResult, void *pUserData);
	static void ConSaveTeam(IConsole::IResult *pResult, void *pUserData);
	static void ConLoadTeam(IConsole::IResult *pResult, void *pUserData);
	static void ConRestart(IConsole::IResult *pResult, void *pUserData);
	static void ConBroadcast(IConsole::IResult *pResult, void *pUserData);
	static void ConSay(IConsole::IResult *pResult, void *pUserData);
	static void ConSetTeam(IConsole::IResult *pResult, void *pUserData);
	static void ConSetTeamAll(IConsole::IResult *pResult, void *pUserData);
	//static void ConSwapTeams(IConsole::IResult *pResult, void *pUserData);
	//static void ConShuffleTeams(IConsole::IResult *pResult, void *pUserData);
	//static void ConLockTeams(IConsole::IResult *pResult, void *pUserData);
	static void ConAddVote(IConsole::IResult *pResult, void *pUserData);
	static void ConRemoveVote(IConsole::IResult *pResult, void *pUserData);
	static void ConForceVote(IConsole::IResult *pResult, void *pUserData);
	static void ConClearVotes(IConsole::IResult *pResult, void *pUserData);
	static void ConVote(IConsole::IResult *pResult, void *pUserData);
	static void ConchainSpecialMotdupdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);

	CGameContext(int Resetting);
	void Construct(int Resetting);

	bool m_Resetting;
public:
	IServer *Server() const { return m_pServer; }
	class IConsole *Console() { return m_pConsole; }
	CCollision *Collision() { return &m_Collision; }
	CTuningParams *Tuning() { return &m_Tuning; }
	CTuningParams *TuningList() { return &m_TuningList[0]; }

	CGameContext();
	~CGameContext();

	void Clear();

	CEventHandler m_Events;
	CPlayer *m_apPlayers[MAX_CLIENTS];

	IGameController *m_pController;
	CGameWorld m_World;

	// helper functions
	class CCharacter *GetPlayerChar(int ClientID);
	
	//int m_LockTeams;

	// voting
	void StartVote(const char *pDesc, const char *pCommand, const char *pReason);
	void EndVote();
	void SendVoteSet(int ClientID);
	void SendVoteStatus(int ClientID, int Total, int Yes, int No);
	void AbortVoteKickOnDisconnect(int ClientID);

	int m_VoteCreator;
	int64 m_VoteCloseTime;
	bool m_VoteUpdate;
	int m_VotePos;
	char m_aVoteDescription[VOTE_DESC_LENGTH];
	char m_aVoteCommand[VOTE_CMD_LENGTH];
	char m_aVoteReason[VOTE_REASON_LENGTH];
	int m_NumVoteOptions;
	int m_VoteEnforce;
	char m_ZoneEnterMsg[NUM_TUNINGZONES][256]; // 0 is used for switching from or to area without tunings
	char m_ZoneLeaveMsg[NUM_TUNINGZONES][256];

	char m_aDeleteTempfile[128];
	void DeleteTempfile();

	enum
	{
		VOTE_ENFORCE_UNKNOWN=0,
		VOTE_ENFORCE_NO,
		VOTE_ENFORCE_YES,
	};
	CHeap *m_pVoteOptionHeap;
	CVoteOptionServer *m_pVoteOptionFirst;
	CVoteOptionServer *m_pVoteOptionLast;

	// helper functions
	void CreateDamageInd(vec2 Pos, float AngleMod, int Amount, int64_t Mask=-1);
	void CreateExplosion(vec2 Pos, int Owner, int Weapon, bool NoDamage, int ActivatedTeam, int64_t Mask);
	void CreateHammerHit(vec2 Pos, int64_t Mask=-1);
	void CreatePlayerSpawn(vec2 Pos, int64_t Mask=-1);
	void CreateDeath(vec2 Pos, int Who, int64_t Mask=-1);
	void CreateSound(vec2 Pos, int Sound, int64_t Mask=-1);
	void CreateSoundGlobal(int Sound, int Target=-1);


	enum
	{
		CHAT_ALL=-2,
		CHAT_SPEC=-1,
		CHAT_RED=0,
		CHAT_BLUE=1,
		CHAT_WHISPER_SEND=2,
		CHAT_WHISPER_RECV=3
	};

	// network
	void CallVote(int ClientID, const char *aDesc, const char *aCmd, const char *pReason, const char *aChatmsg);
	void SendChatTarget(int To, const char *pText);
	void SendChatTeam(int Team, const char *pText);
	void SendChat(int ClientID, int Team, const char *pText, int SpamProtectionClientID = -1);
	void SendEmoticon(int ClientID, int Emoticon);
	void SendWeaponPickup(int ClientID, int Weapon);
	void SendBroadcast(const char *pText, int ClientID);

	void List(int ClientID, const char* filter);

	//
	void CheckPureTuning();
	void SendTuningParams(int ClientID, int Zone = 0);

	struct CVoteOptionServer *GetVoteOption(int Index);
	void ProgressVoteOptions(int ClientID);

	//
	//void SwapTeams();

	void LoadMapSettings();

	// engine events
	virtual void OnInit();
	virtual void OnConsoleInit();
	virtual void OnMapChange(char *pNewMapName, int MapNameSize);
	virtual void OnShutdown();

	virtual void OnTick();
	virtual void OnPreSnap();
	virtual void OnSnap(int ClientID);
	virtual void OnPostSnap();

	virtual void OnMessage(int MsgID, CUnpacker *pUnpacker, int ClientID);

	virtual void OnClientConnected(int ClientID);
	virtual void OnClientEnter(int ClientID, bool silent = false);
	virtual void OnClientDrop(int ClientID, const char *pReason, bool silent = false);
	virtual void OnClientDirectInput(int ClientID, void *pInput);
	virtual void OnClientPredictedInput(int ClientID, void *pInput);

	virtual bool IsClientReady(int ClientID);
	virtual bool IsClientPlayer(int ClientID);

	virtual const char *GameType();
	virtual const char *Version();
	virtual const char *NetVersion();

	// DDRace & DDnetPlusPlus
	//ChillerDragon

	void ShowProfile(int ViewerID, int ViewedID);

	void ChatCommands();
	void DummyChat();
	//Instagib Survival
	void WinInsta1on1(int WinnerID);
	bool CanJoinInstaArena(bool grenade, bool PrivateMatch);
	void SurvivalTick();
	int m_survival_gamestate; //0=prepearing 1=running 2=deathmatch
	int m_survival_delay;

	//dummy
	void CreateBasicDummys();
	int CreateNewDummy(int dummymode, bool silent = false);
	int GetNextClientID();

	//usefull everywhere
	int IsMinigame(int playerID);
	int GetCIDByName(const char *pName);
	int CountConnectedPlayers();
	int CountIngameHumans();
	void SendBroadcastAll(const char *pText);
	void KillAll();
	bool IsPosition(int playerID, int pos);
	void StartAsciiAnimation(int viewerID, int creatorID, int medium); //0='/ascii view' 1='/profile view'
	bool IsHooked(int hookedID, int power);
	bool IsSameIP(int ID_1, int ID_2);

	void DDPP_Tick();
	void ChilliClanTick(int i);
	void AsciiTick(int i);

	//survival

	void SurvivalLobbyTick();
	void SurvivalStartGame();
	void SendSurvivalChat(const char *pMsg);
	void SendSurvivalBroadcast(const char *pMsg);
	void SetPlayerSurvival(int id,int mode); //0=off 1=lobby 2=ingame 3=die
	int CountSurvivalPlayers(bool Alive = false);
	void SurvivalSetGameState(int state);
	bool SurvivalPickWinner();
	int m_survivalgamestate; //0=offline 1=lobby 2=ingame 3=deathmatch
	int m_survivallobbycountdown;

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

	//police
	void SendAllPolice(const char *pMessage);

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
	void EndBombGame(int WinnerID);
	void CheckStartBomb();
	void BombTick();
	int FindNextBomb();
	int CountBannedBombPlayers();
	int CountBombPlayers();
	int CountReadyBombPlayers();
	int m_BombGameState; //0=no bomb game created 1=bomb game created (lobby) 2=bomb game created (lobby)(locked) 3=bomb game created and running (ingame)
	int m_BombMoney;
	int m_BombStartPlayers;
	int m_BombTick; //the ticking bomby ticker ticks until he flicks to zer0 then he kicks kaboooms!
	int m_BombStartCountDown;
	char m_BombMap[32]; //0 = Default 1 = NoArena

	int m_BombFinalColor;
	int m_BombColor;
	bool m_bwff; //black whithe flip flip




	int ProcessSpamProtection(int ClientID);
	int GetDDRaceTeam(int ClientID);
	int64 m_LastMapVote;

	CSql *m_Database;
	bool CheckAccounts(int AccountID);

	//Chiller
	//ChillerDragihn!
	//Chilli

	//int Friends_counter;

	//trading stuff (stock market)

	//char aTestMsg[1024];
	//int TestShareValue;
	int m_CucumberShareValue;
	char aBroadcastMSG[128];


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

private:

	bool m_VoteWillPass;
	class IScore *m_pScore;

	//DDRace Console Commands

	//static void ConMute(IConsole::IResult *pResult, void *pUserData);
	//static void ConUnmute(IConsole::IResult *pResult, void *pUserData);
	static void ConKillPlayer(IConsole::IResult *pResult, void *pUserData);

	static void ConNinja(IConsole::IResult *pResult, void *pUserData);
	static void ConUnSolo(IConsole::IResult *pResult, void *pUserData);
	static void ConUnDeep(IConsole::IResult *pResult, void *pUserData);
	static void Condisarm(IConsole::IResult *pResult, void *pUserData);
	static void Condummymode(IConsole::IResult *pResult, void *pUserData);
	static void ConDummyColor(IConsole::IResult *pResult, void *pUserData);
	static void ConDummySkin(IConsole::IResult *pResult, void *pUserData);
	static void Conheal(IConsole::IResult *pResult, void *pUserData);
	static void Conninjasteam(IConsole::IResult *pResult, void *pUserData);
	static void ConForceUnJail(IConsole::IResult *pResult, void *pUserData);
	static void ConForceJail(IConsole::IResult *pResult, void *pUserData);
	static void ConHammerfightMode(IConsole::IResult *pResult, void *pUserData); //this is the hammerfightmode rcon command
	static void Conhammerfight(IConsole::IResult *pResult, void *pUserData);
	static void ConUnSuper(IConsole::IResult *pResult, void *pUserData);
	static void ConSuper(IConsole::IResult *pResult, void *pUserData);
	static void ConfreezeShotgun(IConsole::IResult *pResult, void *pUserData);
	static void ConDamage(IConsole::IResult *pResult, void *pUserData);
	static void ConShotgun(IConsole::IResult *pResult, void *pUserData);
	static void ConGrenade(IConsole::IResult *pResult, void *pUserData);
	static void ConRifle(IConsole::IResult *pResult, void *pUserData);
	static void ConHammer(IConsole::IResult *pResult, void *pUserData);
	static void ConWeapons(IConsole::IResult *pResult, void *pUserData);
	static void ConUnShotgun(IConsole::IResult *pResult, void *pUserData);
	static void ConUnGrenade(IConsole::IResult *pResult, void *pUserData);
	static void ConUnRifle(IConsole::IResult *pResult, void *pUserData);
	static void ConUnWeapons(IConsole::IResult *pResult, void *pUserData);
	static void ConAddWeapon(IConsole::IResult *pResult, void *pUserData);
	static void ConRemoveWeapon(IConsole::IResult *pResult, void *pUserData);

	static void ConFreeze(IConsole::IResult *pResult, void *pUserData);
	static void ConUnFreeze(IConsole::IResult *pResult, void *pUserData);
	static void ConPullhammer(IConsole::IResult *pResult, void *pUserData);

	// cosmetics
	static void ConRainbow(IConsole::IResult *pResult, void *pUserData);
	static void ConOldRainbow(IConsole::IResult *pResult, void *pUserData);
	static void ConInfRainbow(IConsole::IResult *pResult, void *pUserData);
	static void ConBloody(IConsole::IResult *pResult, void *pUserData);
	static void ConOldBloody(IConsole::IResult *pResult, void *pUserData);
	static void ConInfBloody(IConsole::IResult *pResult, void *pUserData);
	static void ConAtom(IConsole::IResult *pResult, void *pUserData);
	static void ConOldAtom(IConsole::IResult *pResult, void *pUserData);
	static void ConInfAtom(IConsole::IResult *pResult, void *pUserData);
	static void ConTrail(IConsole::IResult *pResult, void *pUserData);
	static void ConOldTrail(IConsole::IResult *pResult, void *pUserData);
	static void ConInfTrail(IConsole::IResult *pResult, void *pUserData);

	//HACK ChillerDragon
	//static void ConHack(IConsole::IResult *pResult, void *pUserData);

	void ModifyWeapons(IConsole::IResult *pResult, void *pUserData, int Weapon, bool Remove);
	void MoveCharacter(int ClientID, int X, int Y, bool Raw = false);
	static void ConGoLeft(IConsole::IResult *pResult, void *pUserData);
	static void ConGoRight(IConsole::IResult *pResult, void *pUserData);
	static void ConGoUp(IConsole::IResult *pResult, void *pUserData);
	static void ConGoDown(IConsole::IResult *pResult, void *pUserData);
	static void ConMove(IConsole::IResult *pResult, void *pUserData);
	static void ConMoveRaw(IConsole::IResult *pResult, void *pUserData);

	static void ConToTeleporter(IConsole::IResult *pResult, void *pUserData);
	static void ConToCheckTeleporter(IConsole::IResult *pResult, void *pUserData);
	static void ConTeleport(IConsole::IResult *pResult, void *pUserData);

	static void ConCredits(IConsole::IResult *pResult, void *pUserData);
	static void ConInfo(IConsole::IResult *pResult, void *pUserData);
	static void ConCC(IConsole::IResult *pResult, void *pUserData);
	static void ConMinigameinfo(IConsole::IResult *pResult, void *pUserData);
	static void ConShop(IConsole::IResult *pResult, void *pUserData);
	static void ConHelp(IConsole::IResult *pResult, void *pUserData);
	static void ConSettings(IConsole::IResult *pResult, void *pUserData);
	static void ConRules(IConsole::IResult *pResult, void *pUserData);
	static void ConKill(IConsole::IResult *pResult, void *pUserData);
	static void ConAdminChat(IConsole::IResult *pResult, void *pUserData);
	static void ConShow(IConsole::IResult *pResult, void *pUserData);
	static void ConHide(IConsole::IResult *pResult, void *pUserData);
	static void ConTogglejailmsg(IConsole::IResult *pResult, void *pUserData);
	static void ConTogglePause(IConsole::IResult *pResult, void *pUserData);
	static void ConToggleSpec(IConsole::IResult *pResult, void *pUserData);
	static void ConForcePause(IConsole::IResult *pResult, void *pUserData);
	static void ConTeamTop5(IConsole::IResult *pResult, void *pUserData);
	static void ConTop5(IConsole::IResult *pResult, void *pUserData);
	static void ConPoints(IConsole::IResult *pResult, void *pUserData);
	#if defined(CONF_SQL)
	static void ConTimes(IConsole::IResult *pResult, void *pUserData);
	//static void ConPoints(IConsole::IResult *pResult, void *pUserData);
	static void ConTopPoints(IConsole::IResult *pResult, void *pUserData);
	#endif

	static void ConUTF8(IConsole::IResult *pResult, void *pUserData);
	static void ConDND(IConsole::IResult *pResult, void *pUserData);
	static void ConMapInfo(IConsole::IResult *pResult, void *pUserData);
	static void ConTimeout(IConsole::IResult *pResult, void *pUserData);
	static void ConSave(IConsole::IResult *pResult, void *pUserData);
	static void ConLoad(IConsole::IResult *pResult, void *pUserData);
	static void ConMap(IConsole::IResult *pResult, void *pUserData);
	static void ConTeamRank(IConsole::IResult *pResult, void *pUserData);
	static void ConRank(IConsole::IResult *pResult, void *pUserData);
	static void ConBroadTime(IConsole::IResult *pResult, void *pUserData);
	static void ConJoinTeam(IConsole::IResult *pResult, void *pUserData);
	static void ConLockTeam(IConsole::IResult *pResult, void *pUserData);
	static void ConMe(IConsole::IResult *pResult, void *pUserData);
	//static void ConPlayerinfo(IConsole::IResult *pResult, void *pUserData);
	static void ConWhisper(IConsole::IResult *pResult, void *pUserData);
	static void ConConverse(IConsole::IResult *pResult, void *pUserData);
	static void ConSetEyeEmote(IConsole::IResult *pResult, void *pUserData);
	static void ConToggleBroadcast(IConsole::IResult *pResult, void *pUserData);
	static void ConEyeEmote(IConsole::IResult *pResult, void *pUserData);
	static void ConShowOthers(IConsole::IResult *pResult, void *pUserData);
	static void ConShowAll(IConsole::IResult *pResult, void *pUserData);
	static void ConSpecTeam(IConsole::IResult *pResult, void *pUserData);
	static void ConNinjaJetpack(IConsole::IResult *pResult, void *pUserData);
	static void ConSayTime(IConsole::IResult *pResult, void *pUserData);
	static void ConSayTimeAll(IConsole::IResult *pResult, void *pUserData);
	static void ConTime(IConsole::IResult *pResult, void *pUserData);
	static void ConSetTimerType(IConsole::IResult *pResult, void *pUserData);
	static void ConRescue(IConsole::IResult *pResult, void *pUserData);
	static void ConProtectedKill(IConsole::IResult *pResult, void *pUserData);

	static void ConBuy(IConsole::IResult *pResult, void *pUserData);

	static void ConMute(IConsole::IResult *pResult, void *pUserData);
	static void ConMuteID(IConsole::IResult *pResult, void *pUserData);
	static void ConMuteIP(IConsole::IResult *pResult, void *pUserData);
	static void ConUnmute(IConsole::IResult *pResult, void *pUserData);
	static void ConMutes(IConsole::IResult *pResult, void *pUserData);

	static void ConList(IConsole::IResult *pResult, void *pUserData);
	static void ConDestroyLaser(IConsole::IResult *pResult, void *pUserData);
	static void ConFreezeLaser(IConsole::IResult *pResult, void *pUserData);
	static void ConFreezeHammer(IConsole::IResult *pResult, void *pUserData);
	static void ConUnFreezeHammer(IConsole::IResult *pResult, void *pUserData);

	//ChillerDragon (ddpp)

	//account stuff
	static void ConChangePassword(IConsole::IResult *pResult, void *pUserData);
	static void ConAccLogout(IConsole::IResult *pResult, void *pUserData);
	static void ConLogin(IConsole::IResult *pResult, void *pUserData);
	static void ConRegister(IConsole::IResult *pResult, void *pUserData);
	static void ConSQL(IConsole::IResult *pResult, void *pUserData);
	static void ConSQLName(IConsole::IResult *pResult, void *pUserData);
	static void ConSQL_ADD(IConsole::IResult *pResult, void *pUserData);
	static void ConAcc_Info(IConsole::IResult *pResult, void *pUserData);
	static void ConStats(IConsole::IResult *pResult, void *pUserData);
	static void ConProfile(IConsole::IResult *pResult, void *pUserData);
	static void ConAscii(IConsole::IResult *pResult, void *pUserData);

	//minigame (chidraqul)
	static void ConChidraqul(IConsole::IResult *pResult, void *pUserData);
	static void ConMinigameLeft(IConsole::IResult *pResult, void *pUserData);
	static void ConMinigameRight(IConsole::IResult *pResult, void *pUserData);
	static void ConMinigameUp(IConsole::IResult *pResult, void *pUserData);
	static void ConMinigameDown(IConsole::IResult *pResult, void *pUserData);

	static void ConMinigames(IConsole::IResult *pResult, void *pUserData);

	static void ConToggleXpMsg(IConsole::IResult *pResult, void *pUserData);
	static void ConToggleSpawn(IConsole::IResult *pResult, void *pUserData);
	static void ConSayServer(IConsole::IResult *pResult, void *pUserData);

	//police
	static void ConPolicehelper(IConsole::IResult *pResult, void *pUserData);
	//static void ConTaserinfo(IConsole::IResult *pResult, void *pUserData);
	static void ConPoliceInfo(IConsole::IResult *pResult, void *pUserData);
	//static void ConPolicetaser(IConsole::IResult *pResult, void *pUserData);
	static void ConPoliceChat(IConsole::IResult *pResult, void *pUserData);
	static void ConJail(IConsole::IResult *pResult, void *pUserData);
	static void ConReport(IConsole::IResult *pResult, void *pUserData);
	static void ConTaser(IConsole::IResult *pResult, void *pUserData);

	//money 
	static void ConMoney(IConsole::IResult *pResult, void *pUserData);
	static void ConPay(IConsole::IResult *pResult, void *pUserData);
	static void ConBank(IConsole::IResult *pResult, void *pUserData);
	static void ConGangsterBag(IConsole::IResult *pResult, void *pUserData);
	static void ConGift(IConsole::IResult *pResult, void *pUserData);
	static void ConTrade(IConsole::IResult *pResult, void *pUserData);

	static void ConBalance(IConsole::IResult *pResult, void *pUserData);
	static void ConInsta(IConsole::IResult *pResult, void *pUserData);
	static void ConPvpArena(IConsole::IResult *pResult, void *pUserData);
	static void ConEvent(IConsole::IResult *pResult, void *pUserData);

	//info
	static void ConAccountInfo(IConsole::IResult *pResult, void *pUserData);
	//static void ConProfileInfo(IConsole::IResult *pResult, void *pUserData);
	static void ConOfferInfo(IConsole::IResult *pResult, void *pUserData);

	static void ConChangelog(IConsole::IResult *pResult, void *pUserData);

	static void ConGive(IConsole::IResult *pResult, void *pUserData);

	static void ConTCMD3000(IConsole::IResult *pResult, void *pUserData);

	static void ConStockMarket(IConsole::IResult *pResult, void *pUserData);

	static void ConPoop(IConsole::IResult *pResult, void *pUserData);

	static void ConBomb(IConsole::IResult *pResult, void *pUserData);
	static void ConSurvival(IConsole::IResult *pResult, void *pUserData);

	static void ConRoom(IConsole::IResult *pResult, void *pUserData);
	static void ConGodmode(IConsole::IResult *pResult, void *pUserData);
	static void ConHook(IConsole::IResult *pResult, void *pUserData);
	static void ConQuest(IConsole::IResult *pResult, void *pUserData);
	static void ConBounty(IConsole::IResult *pResult, void *pUserData);
	static void ConTROLL166(IConsole::IResult *pResult, void *pUserData);
	static void ConTROLL420(IConsole::IResult *pResult, void *pUserData);


	//static void ConAfk(IConsole::IResult *pResult, void *pUserData);
	//static void ConAddPolicehelper(IConsole::IResult *pResult, void *pUserData);
	//static void ConRemovePolicehelper(IConsole::IResult *pResult, void *pUserData);

	enum
	{
		MAX_MUTES=32,
	};
	struct CMute
	{
		NETADDR m_Addr;
		int m_Expire;
	};

	CMute m_aMutes[MAX_MUTES];
	int m_NumMutes;
	void Mute(IConsole::IResult *pResult, NETADDR *Addr, int Secs, const char *pDisplayName);
	void Whisper(int ClientID, char *pStr);
	void WhisperID(int ClientID, int VictimID, char *pMessage);
	void Converse(int ClientID, char *pStr);

public:
	CLayers *Layers() { return &m_Layers; }
	class IScore *Score() { return m_pScore; }
	bool m_VoteKick;
	bool m_VoteSpec;
	enum
	{
		VOTE_ENFORCE_NO_ADMIN = VOTE_ENFORCE_YES + 1,
		VOTE_ENFORCE_YES_ADMIN
	};
	int m_VoteEnforcer;
	void SendRecord(int ClientID);
	static void SendChatResponse(const char *pLine, void *pUser, bool Highlighted = false);
	static void SendChatResponseAll(const char *pLine, void *pUser);
	virtual void OnSetAuthed(int ClientID,int Level);
	virtual bool PlayerCollision();
	virtual bool PlayerHooking();
	virtual float PlayerJetpack();

	void ResetTuning();

	int m_ChatResponseTargetID;
	int m_ChatPrintCBIndex;
};

class CQueryPlayer : public CQuery
{
public:
	int m_ClientID;
	CGameContext *m_pGameServer;
};

class CQueryChangePassword : public CQueryPlayer //ChillerDragon's testy test (no idea what im doing here xd)
{
	void OnData();
public:
};

class CQuerySetPassword : public CQueryPlayer //ChillerDragon's testy test (no idea what im doing here xd)
{
	void OnData();
public:
};

class CQueryRegister : public CQueryPlayer
{
	void OnData();
public:
	std::string m_Name;
	std::string m_Password;
};

class CQueryLogin : public CQueryPlayer
{
	void OnData();
public:
};

inline int64_t CmaskAll() { return -1LL; }
inline int64_t CmaskOne(int ClientID) { return 1LL<<ClientID; }
inline int64_t CmaskAllExceptOne(int ClientID) { return CmaskAll()^CmaskOne(ClientID); }
inline bool CmaskIsSet(int64_t Mask, int ClientID) { return (Mask&CmaskOne(ClientID)) != 0; }
#endif
