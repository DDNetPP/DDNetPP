#ifndef GAME_SERVER_GAMEMODES_DDNETPP_DDNETPP_H
#define GAME_SERVER_GAMEMODES_DDNETPP_DDNETPP_H

#include "../DDRace.h"

class CGameControllerDDNetPP : public CGameControllerDDRace
{
public:
	CGameControllerDDNetPP(class CGameContext *pGameServer);
	~CGameControllerDDNetPP() override;

	// convience accessors to copy code from gamecontext
	class IServer *Server() const { return GameServer()->Server(); }
	class CConfig *Config() { return GameServer()->Config(); }
	class IConsole *Console() { return GameServer()->Console(); }
	class IStorage *Storage() { return GameServer()->Storage(); }

	void SetArmorProgress(CCharacter *pCharacer, int Progress) override;

	void Tick() override;
	bool CanJoinTeam(int Team, int NotThisId, char *pErrorReason, int ErrorReasonSize) override;
	void PrintJoinMessage(CPlayer *pPlayer) override;
	void OnPlayerConnect(class CPlayer *pPlayer) override;
	void OnPlayerDisconnect(class CPlayer *pPlayer, const char *pReason, bool Silent = false) override;
	void DoTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg = true) override;
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) override;

	const char *CommandByVoteMsg(const CNetMsg_Cl_CallVote *pMsg);

	bool OnCallVoteNetMessage(const CNetMsg_Cl_CallVote *pMsg, int ClientId) override;
	bool OnChatMessage(const CNetMsg_Cl_Say *pMsg, int Length, int &Team, CPlayer *pPlayer) override;

	int64_t m_NextMinuteReset = 0;
	int64_t m_Next10MinutesReset = 0;
	int64_t m_LastConnectionSpamThresholdHit = 0;

	int m_NumConnectionsInTheLastMinute = 0;
	int m_NumConnectionsInTheLast10Minutes = 0;

	// automatic flood detection
	void DetectReconnectFlood();

	// flags.cpp
	void FlagTick();
	void DropFlag(int FlagId, int Dir = 1) override;
	void ChangeFlagOwner(int FlagId, int ClientId) override;
	int HasFlag(CCharacter *pChr) override;
};

#endif
