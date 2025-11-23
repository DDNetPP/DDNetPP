#ifndef GAME_SERVER_DDPP_MINIGAME_PLAYER_STATE_H
#define GAME_SERVER_DDPP_MINIGAME_PLAYER_STATE_H

#include <generated/protocol.h>

// shared player state for all minigames
// keeps track of things such as score
// this state is supposed to be cleared on minigame join and leave
// and is there to not interfere with the main games state
class CMinigamePlayerState
{
public:
	void Reset();

	// Score that will also be displayed in the scoreboard
	// if the minigame is active

	// is separate from m_Score which is only used for ddnet
	// race times

	// The minigame score can/should be used for kills
	// in fng/dm/gctf/block/bomb and so on
	int m_Score = 0;

	// If set to TEAM_RED or TEAM_BLUE the minigame
	// should properly reflect that in the scoreboard
	int m_Team = TEAM_RED;

	int Score() const { return m_Score; }
};

#endif
