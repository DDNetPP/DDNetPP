/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_PLAYER_H
#define GAME_SERVER_PLAYER_H

#include <base/vmath.h>

#include <engine/shared/protocol.h>

#include "alloc.h"
#include "teeinfo.h"

// ddnet++ includes start
#include "captcha.h"
#include "ddpp/accounts.h"
#include "entities/character.h"
#include <game/version.h>
#include <memory>
#include <vector>
#define ACC_MAX_LEVEL 110 // WARNING!!! if you increase this value make sure to append needexp until max-1 in player.cpp:CalcExp()
#include "gamecontext.h"
// ddnet++ includes end
#include <memory>

class CCharacter;
class CGameContext;
class IServer;
struct CNetObj_PlayerInput;
struct CScorePlayerResult;

enum
{
	WEAPON_GAME = -3, // team switching etc
	WEAPON_SELF = -2, // console kill command
	WEAPON_WORLD = -1, // death tiles etc
};

// player object
class CPlayer
{
	MACRO_ALLOC_POOL_ID()
#define IN_CLASS_PLAYER
#include "player_ddpp.h"

public:
	CPlayer(CGameContext *pGameServer, uint32_t UniqueClientID, int ClientID, int Team, bool IsDummy = false);
	~CPlayer();

	void Reset();

	void TryRespawn();
	void Respawn(bool WeakHook = false); // with WeakHook == true the character will be spawned after all calls of Tick from other Players
	CCharacter *ForceSpawn(vec2 Pos); // required for loading savegames
	void SetTeam(int Team, bool DoChatMsg = true);
	int GetTeam() const { return m_Team; }
	int GetCID() const { return m_ClientID; }
	uint32_t GetUniqueCID() const { return m_UniqueClientID; }
	int GetClientVersion() const;
	bool SetTimerType(int TimerType);

	void Tick();
	void PostTick();

	// will be called after all Tick and PostTick calls from other players
	void PostPostTick();
	void Snap(int SnappingClient);
	void FakeSnap();

	void OnDirectInput(CNetObj_PlayerInput *pNewInput);
	void OnPredictedInput(CNetObj_PlayerInput *pNewInput);
	void OnPredictedEarlyInput(CNetObj_PlayerInput *pNewInput);
	void OnDisconnect();

	void KillCharacter(int Weapon = WEAPON_GAME);
	CCharacter *GetCharacter();

	void SpectatePlayerName(const char *pName);

	//---------------------------------------------------------
	// this is used for snapping so we know how we can clip the view for the player
	vec2 m_ViewPos;
	int m_TuneZone;
	int m_TuneZoneOld;

	// states if the client is chatting, accessing a menu etc.
	int m_PlayerFlags;

	// used for snapping to just update latency if the scoreboard is active
	int m_aCurLatency[MAX_CLIENTS];

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
	int m_aLastCommands[4];
	int m_LastCommandPos;
	int m_LastWhisperTo;
	int m_LastInvited;

	int m_SendVoteIndex;

	CTeeInfo m_TeeInfos;

	int m_DieTick;
	int m_PreviousDieTick;
	int m_Score;
	int m_JoinTick;
	bool m_ForceBalanced;
	int m_LastActionTick;
	int m_TeamChangeTick;
	bool m_SentSemicolonTip;

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
	const uint32_t m_UniqueClientID;
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

	int m_Paused;
	int64_t m_ForcePauseTime;
	int64_t m_LastPause;
	bool m_Afk;

	int m_DefEmote;
	int m_OverrideEmote;
	int m_OverrideEmoteReset;
	bool m_Halloween;

public:
	enum
	{
		PAUSE_NONE = 0,
		PAUSE_PAUSED,
		PAUSE_SPEC
	};

	enum
	{
		TIMERTYPE_DEFAULT = -1,
		TIMERTYPE_GAMETIMER = 0,
		TIMERTYPE_BROADCAST,
		TIMERTYPE_GAMETIMER_AND_BROADCAST,
		TIMERTYPE_SIXUP,
		TIMERTYPE_NONE,
	};

	bool m_DND;
	int64_t m_FirstVoteTick;
	char m_aTimeoutCode[64];

	void ProcessPause();
	int Pause(int State, bool Force);
	int ForcePause(int Time);
	int IsPaused();

	bool IsPlaying();
	int64_t m_Last_KickVote;
	int64_t m_Last_Team;
	int m_ShowOthers;
	bool m_ShowAll;
	vec2 m_ShowDistance;
	bool m_SpecTeam;
	bool m_NinjaJetpack;
	bool m_HasFinishScore;

	int m_ChatScore;

	bool m_Moderating;

	void UpdatePlaytime();
	void AfkTimer();
	void SetAfk(bool Afk);
	bool IsAfk() const { return m_Afk; }

	int64_t m_LastPlaytime;
	int64_t m_LastEyeEmote;
	int64_t m_LastBroadcast;
	int m_LastBroadcastImportance; // ddnet++ uses int for importance levels and ddnet bool for important or not

	CNetObj_PlayerInput *m_pLastTarget;
	bool m_LastTargetInit;

	bool m_EyeEmoteEnabled;
	int m_TimerType;

	int GetDefaultEmote() const;
	void OverrideDefaultEmote(int Emote, int Tick);
	bool CanOverrideDefaultEmote() const;

	bool m_FirstPacket;
	int64_t m_LastSQLQuery;
	void ProcessScoreResult(CScorePlayerResult &Result);
	std::shared_ptr<CScorePlayerResult> m_ScoreQueryResult;
	std::shared_ptr<CScorePlayerResult> m_ScoreFinishResult;
	bool m_NotEligibleForFinish;
	int64_t m_EligibleForFinishCheck;
	bool m_VotedForPractice;
	int m_SwapTargetsClientID; //Client ID of the swap target for the given player
	bool m_BirthdayAnnounced;
};

#endif
