#ifndef GAME_SERVER_GAMEMODES_DDNETPP_DDNETPP_H
#define GAME_SERVER_GAMEMODES_DDNETPP_DDNETPP_H

#include "../ddnet.h"
#include "game/race_state.h"

class CGameControllerDDNetPP : public CGameControllerDDNet
{
public:
	CGameControllerDDNetPP(class CGameContext *pGameServer);
	~CGameControllerDDNetPP() override;

	// convenience accessors to copy code from gamecontext
	class IServer *Server() const { return GameServer()->Server(); }
	class CConfig *Config() { return GameServer()->Config(); }
	class IConsole *Console() { return GameServer()->Console(); }
	class IStorage *Storage() { return GameServer()->Storage(); }

	void SetArmorProgress(CCharacter *pCharacter, int Progress) override;

	void Tick() override;
	bool OnEntity(int Index, int x, int y, int Layer, int Flags, bool Initial, int Number) override;
	bool CanJoinTeam(int Team, int NotThisId, char *pErrorReason, int ErrorReasonSize) override;
	int GetAutoTeam(int NotThisId) override;
	void PrintJoinMessage(CPlayer *pPlayer) override;
	void OnPlayerConnect(class CPlayer *pPlayer) override;
	void OnPlayerDisconnect(class CPlayer *pPlayer, const char *pReason, bool Silent = false) override;

	// contains the base logic needed to make ddnet++ work
	// you almost never want to not run this code
	// TODO: remove Silent arg like in ddnet-insta
	virtual void DDNetPPDisconnect(CPlayer *pPlayer, const char *pReason, bool Silent);

	// logs the disconnect to console
	// and prints it to the chat
	// you can override this to silence the message or change it
	// TODO: remove silent arg
	virtual void PrintDisconnect(CPlayer *pPlayer, const char *pReason, bool Silent);

	// prints the join message into public chat
	// you can override this to silence the message or change it
	virtual void PrintConnect(CPlayer *pPlayer, const char *pName);

	// prints the mod welcome and version message
	// called on join
	virtual void PrintModWelcome(CPlayer *pPlayer);

	void DoTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg = true) override;
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) override;
	int ServerInfoClientScoreValue(CPlayer *pPlayer) override;
	const char *ServerInfoClientScoreKind() override;

	const char *CommandByVoteMsg(const CNetMsg_Cl_CallVote *pMsg);

	bool OnCallVoteNetMessage(const CNetMsg_Cl_CallVote *pMsg, int ClientId) override;
	bool OnVoteNetMessage(const CNetMsg_Cl_Vote *pMsg, int ClientId) override;
	bool OnChatMessage(const CNetMsg_Cl_Say *pMsg, int Length, int &Team, CPlayer *pPlayer) override;
	CPlayer *GetPlayerByUniqueId(uint32_t UniqueId);

	int64_t m_NextMinuteReset = 0;
	int64_t m_Next10MinutesReset = 0;
	int64_t m_LastConnectionSpamThresholdHit = 0;

	// count the amount of quick reconnects
	// if there are too many we start anti flood mode
	// and show join messages delayed
	int m_NumShortConnectionsInTheLastMinute = 0;
	int m_NumShortConnectionsInTheLast10Minutes = 0;

	// automatic flood detection
	void DetectReconnectFlood();

	// accounts
	void LogoutAllAccounts() override;
	bool IsAccountRconCmdRatelimited(int ClientId, char *pReason, int ReasonSize) override;
	void ProcessAccountRconCmdResult(CAccountRconCmdResult &Result);

	// spawn.cpp
	bool CanSpawn(int Team, vec2 *pOutPos, int ClientId) override;

	// flags.cpp
	void FlagTick();
	void DropFlag(int FlagId, int Dir = 1) override;
	void ChangeFlagOwner(int FlagId, int ClientId) override;
	bool CharacterDropFlag(CCharacter *pChr) override;
	// DEPRECATED! use GetCarriedFlag instead
	int HasFlag(CCharacter *pChr) override;
	int GetCarriedFlag(CPlayer *pPlayer) override;

	// snapshot.cpp
	void Snap(int SnappingClient) override;
	void SnapFlags(int SnappingClient);
	void FakeSnap(int SnappingClient);
	int SnapPlayerScore(int SnappingClient, CPlayer *pPlayer) override;
	void SnapPlayer6(int SnappingClient, CPlayer *pPlayer, CNetObj_ClientInfo *pClientInfo, CNetObj_PlayerInfo *pPlayerInfo) override;
	int SnapPlayerFlags7(int SnappingClient, CPlayer *pPlayer, int PlayerFlags7) override;
	int SnapScoreLimit(int SnappingClient) override;
	void SnapGameInfo(int SnappingClient, CNetObj_GameInfo *pGameInfo) override;

	// tiles.cpp
	void HandleCharacterTiles(class CCharacter *pChr, int MapIndex) override;
	void HandleCharacterTilesDDPP(
		class CCharacter *pChr,
		int TileIndex,
		int TileFIndex,
		int Tile1,
		int Tile2,
		int Tile3,
		int Tile4,
		int FTile1,
		int FTile2,
		int FTile3,
		int FTile4,
		ERaceState PlayerDDRaceState);
	void HandleCosmeticTiles(class CCharacter *pChr);

	// returns true if it killed
	bool HandleTilesThatCanKill(class CCharacter *pChr);
};

#endif
