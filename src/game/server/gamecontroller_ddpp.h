#ifndef GAME_SERVER_GAMECONTROLLER_DDPP_H
// hack for headerguard linter
#endif

#ifndef IN_CLASS_IGAMECONTROLLER

#include <base/vmath.h>
#include <engine/map.h>
#include <engine/shared/protocol.h>
#include <game/server/teams.h>

#include <game/generated/protocol.h>
#include <game/generated/protocol7.h>

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

			it is recommended to prefix your code with this to get spam protection

			if(RateLimitPlayerVote(ClientId) || m_VoteCloseTime)
				return;

		Returns:
			return true to not run the rest of CGameContext::OnCallVoteNetMessage()
	*/
	virtual bool OnCallVoteNetMessage(const CNetMsg_Cl_CallVote *pMsg, int ClientId)
	{
		dbg_msg("fuck", "this");
		return false;
	}

private:
#ifndef IN_CLASS_IGAMECONTROLLER
};
#endif
