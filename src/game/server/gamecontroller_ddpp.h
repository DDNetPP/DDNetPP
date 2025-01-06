#ifndef GAME_SERVER_GAMECONTROLLER_DDPP_H
// hack for headerguard linter
#endif

#ifndef IN_CLASS_IGAMECONTROLLER

#include <base/vmath.h>
#include <engine/map.h>
#include <engine/shared/protocol.h>
#include <game/generated/protocol.h>
#include <game/generated/protocol7.h>
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

		Returns:
			return true to not run the rest of CGameContext::OnCallVoteNetMessage()
	*/
	virtual bool OnCallVoteNetMessage(const CNetMsg_Cl_CallVote *pMsg, int ClientId)
	{
		return false;
	}

	/*
		Function: OnChatMessage
			hooks into CGameContext::OnSayNetMessage()
			after unicode check and teehistorian already happend

		Returns:
			return true to not run the rest of CGameContext::OnSayNetMessage()
			which would print it to the chat or run it as a ddrace chat command
	*/
	virtual bool OnChatMessage(const CNetMsg_Cl_Say *pMsg, int Length, int &Team, CPlayer *pPlayer) { return false; };

	/*
		Function: SnapPlayerScore
			Warning its value could be overwritten by `CPlayer::DDPPSnapChangePlayerInfo()`

		Arguments:
			SnappingClient - Client Id of the player that will receive the snapshot
			pPlayer - CPlayer that is being snapped
			DDRaceScore - Current value of the score set by the ddnet code

		Returns:
			return the new score value that will be included in the snapshot
	*/
	virtual int SnapPlayerScore(int SnappingClient, CPlayer *pPlayer, int DDRaceScore);

	/*
		Function: PrintJoinMessage
			prints the chat message "entered and %s joined the game"
			this can be called on join
			or also at a later point in time (after human verification)
			if the anti flood is on
	*/
	virtual void PrintJoinMessage(CPlayer *pPlayer){};

	// flags only work if the ddnet++ gamecontroller is active
	// which should always be the case because not even sv_gametype can load pure ddnet
	virtual void DropFlag(int FlagId, int Dir = 1){};
	virtual void ChangeFlagOwner(int FlagId, int ClientId){};

	// returns true if the character had the flag and it got dropped
	// returns false if the character did not have the flag and nothing happend
	virtual bool CharacterDropFlag(CCharacter *pChr) { return false; }

	// returns -1 if the player has no flag
	// returns 0 of the player has the red flag
	// returns 1 if the player has the blue flag
	virtual int HasFlag(CCharacter *pChr) { return -1; };

private:
#ifndef IN_CLASS_IGAMECONTROLLER
};
#endif
