#ifndef GAME_SERVER_DDPP_ENUMS_H
#define GAME_SERVER_DDPP_ENUMS_H

#include <engine/shared/protocol.h>

enum
{
	// magic value to represent absence of a flag
	FLAG_NONE = -1,

	// index in the IGameController::m_apFlags array
	FLAG_RED = 0,

	// index in the IGameController::m_apFlags array
	FLAG_BLUE = 1,

	// size of the IGameController::m_apFlags array
	NUM_FLAGS = 2
};

enum
{
	// fake client id used for hooking flags
	CLIENT_ID_FLAG_RED = MAX_CLIENTS,

	// fake client id used for hooking flags
	CLIENT_ID_FLAG_BLUE
};

enum
{
	// used to respawn players on round end/start
	// or minigame join/leave
	//
	// this is useful to avoid using WEAPON_GAME
	// because ddnet kicks you out of ddrace teams
	// if WEAPON_GAME is used
	WEAPON_MINIGAME = -4,
};

// enum
// {
// 	WEAPON_GAME = -3, // team switching etc
// 	WEAPON_SELF = -2, // console kill command
// 	WEAPON_WORLD = -1, // death tiles etc
// };

enum class EDisplayScore
{
	TIME,
	LEVEL,
	BLOCK,
	CURRENT_SPREE,
	KING_OF_THE_HILL,

	NUM_SCORES,
};

#define DISPLAY_SCORE_VALUES "time, level, block, current_spree, king_of_the_hill"

enum class EDummyTest
{
	NONE,
	BLMAPCHILL_POLICE,
};

// writes based on the input pInputText the output pDisplayScore
// returns true on match
// returns false on no match
bool str_to_display_score(const char *pInputText, EDisplayScore *pDisplayScore);

const char *display_score_to_str(EDisplayScore Score);

#endif
