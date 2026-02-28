#ifndef GAME_SERVER_GAMECONTROLLER_DDPP_H
#define GAME_SERVER_GAMECONTROLLER_DDPP_H
#undef GAME_SERVER_GAMECONTROLLER_DDPP_H
// hack for headerguard linter
#endif

#ifndef IN_CLASS_IGAMECONTROLLER

#include <base/vmath.h>

#include <engine/map.h>
#include <engine/shared/protocol.h>

#include <generated/protocol.h>
#include <generated/protocol7.h>

#include <game/server/ddnetpp/lua/lua_controller.h>
#include <game/server/entities/character.h>
#include <game/server/teams.h>

struct CScoreLoadBestTimeResult;

class IGameController
{
public:
	virtual ~IGameController() = default;

private:
#endif // IN_CLASS_IGAMECONTROLLER

public:
	//
	// ddnet-insta inspired controller hook extensions
	//

	/*
		Function: OnCallVoteNetMessage
			hooks into CGameContext::OnCallVoteNetMessage()
			before any spam protection check

			This is being called when a player creates a new vote

			See also `OnVoteNetMessage()`

		Returns:
			return true to not run the rest of CGameContext::OnCallVoteNetMessage()
	*/
	virtual bool OnCallVoteNetMessage(const CNetMsg_Cl_CallVote *pMsg, int ClientId)
	{
		return false;
	}

	/*
		Function: OnClientPacket
			hooks early into CServer::ProcessClientPacket
			similar to CGameContext::OnMessage but converts both system and game messages
			and it can also drop the message before the server processes it

		Returns:
			return true to consume the message and drop it before it gets passed to the server code
			return false to let regular server code process the message
	*/
	virtual bool OnClientPacket(int ClientId, bool Sys, int MsgId, struct CNetChunk *pPacket, class CUnpacker *pUnpacker) { return false; }

	/*
		Function: LogoutAllAccounts
			Logs out all players on this server.
	*/
	virtual void LogoutAllAccounts() {}

	/*
		Function: IsAccountRconCmdRatelimited
			If the user is ratelimited and can not run any account related rcon
			commands that operate on the database.

			This blocks rcon commands such as "acc_set_password"
			if it returns true.

		Arguments:
			ClientId - id of the player to check
			pReason - buffer the reason for ratelimit will be written to (can be null) will be an empty string if not ratelimited
			ReasonSize - size of the pReason in bytes

		Returns:
			true - if all rcon account operations (not affecting own account only rcon commands)
			false - if all rcon account operations are allowed
	*/
	virtual bool IsAccountRconCmdRatelimited(int ClientId, char *pReason, int ReasonSize) { return false; }

	/*
		Function: OnVoteNetMessage
			hooks into CGameContext::OnVoteNetMessage()
			before any spam protection check

			This is being called when a player votes yes or no.

			See also `OnCallVoteNetMessage()`

		Returns:
			return true to not run the rest of CGameContext::OnVoteNetMessage()
	*/
	virtual bool OnVoteNetMessage(const CNetMsg_Cl_Vote *pMsg, int ClientId) { return false; }

	/*
		Function: OnChatMessage
			hooks into CGameContext::OnSayNetMessage()
			after unicode check and teehistorian already happened

		Returns:
			return true to not run the rest of CGameContext::OnSayNetMessage()
			which would print it to the chat or run it as a ddrace chat command
	*/
	virtual bool OnChatMessage(const CNetMsg_Cl_Say *pMsg, int Length, int &Team, CPlayer *pPlayer) { return false; }

	/*
		Function: SnapPlayerFlags7
			Set custom player flags for 0.7 connections.

		Arguments:
			SnappingClient - Client Id of the player that will receive the snapshot
			pPlayer - CPlayer that is being snapped
			PlayerFlags7 - the flags that were already set for that player by ddnet

		Returns:
			return the new flags value that should be snapped to the SnappingClient
	*/
	virtual int SnapPlayerFlags7(int SnappingClient, CPlayer *pPlayer, int PlayerFlags7) { return PlayerFlags7; }

	/*
		Function: SnapPlayer6
			Alter snap values for 0.6 snapshots.
			For 0.7 use `SnapPlayerFlags7()` and `SnapPlayerScore()`

			Be careful with setting `pPlayerInfo->m_Score` to not overwrite
			what `SnapPlayerScore()` tries to set.

		Arguments:
			SnappingClient - Client Id of the player that will receive the snapshot
			pPlayer - CPlayer that is being snapped
			pClientInfo - (in and output) info that is being snappend which is already pre filled by ddnet and can be altered.
			pPlayerInfo - (in and output) info that is being snappend which is already pre filled by ddnet and can be altered.
	*/
	virtual void SnapPlayer6(int SnappingClient, CPlayer *pPlayer, CNetObj_ClientInfo *pClientInfo, CNetObj_PlayerInfo *pPlayerInfo) {}
	virtual int SnapScoreLimit(int SnappingClient);

	virtual void SnapGameInfo(int SnappingClient, CNetObj_GameInfo *pGameInfo) {}
	virtual void SnapDDNetCharacter(int SnappingClient, CCharacter *pChr, CNetObj_DDNetCharacter *pDDNetCharacter) {}

	/*
		Function: ServerInfoClientScoreValue
			This method determines the value that should be used as score
			in the master server for a given player.
			See also `SnapPlayerScore()` for the value that will be displayed
			in the in game scoreboard.
			See also `ServerInfoClientScoreKind()` to determine the master server
			type (points/time).

		Arguments:
			pPlayer - CPlayer that is being sent to the master server

		Returns:
			return score to be shown in the master server info
	*/
	virtual int ServerInfoClientScoreValue(CPlayer *pPlayer);

	/*
		Function: ServerInfoClientScoreKind
			This method determines the value of the key
			"client_score_kind" in the master server info.
			Officially supported values are "points" and "time".
			See also `ServerInfoClientScoreValue` to find the matching value.

		Returns:
			return either "points" or "time" depending on the score type in the master
	*/
	virtual const char *ServerInfoClientScoreKind() { return "time"; }

	/*
		Function: PrintJoinMessage
			prints the chat message "entered and %s joined the game"
			this can be called on join
			or also at a later point in time (after human verification)
			if the anti flood is on
	*/
	virtual void PrintJoinMessage(CPlayer *pPlayer) {}

	// flags only work if the ddnet++ gamecontroller is active
	// which should always be the case because not even sv_gametype can load pure ddnet
	virtual void DropFlag(int FlagId, int Dir = 1) {}
	virtual void ChangeFlagOwner(int FlagId, int ClientId) {}

	// returns true if the character had the flag and it got dropped
	// returns false if the character did not have the flag and nothing happened
	virtual bool CharacterDropFlag(CCharacter *pChr) { return false; }

	// returns -1 if the player has no flag
	// returns 0 of the player has the red flag
	// returns 1 if the player has the blue flag
	virtual int HasFlag(CCharacter *pChr) { return -1; }

	/*
		Function: GetCarriedFlag
			Returns the type of flag the given player is currently carrying.
			Flag refers here to a CTF gametype flag which is either red, blue or none.

		Arguments:
			pPlayer - player to check

		Returns:
			FLAG_NONE -1
			FLAG_RED  0
			FLAG_BLUE 2
	*/
	virtual int GetCarriedFlag(CPlayer *pPlayer) { return FLAG_NONE; }

private:
	CLuaController m_LuaController;

public:
	CLuaController *Lua() { return &m_LuaController; }
	const CLuaController *Lua() const { return &m_LuaController; }

private:
#ifndef IN_CLASS_IGAMECONTROLLER
};
#endif
