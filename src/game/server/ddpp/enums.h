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

enum EScore
{
	SCORE_TIME = 0,
	SCORE_LEVEL = 1,
	SCORE_BLOCK = 2,
	SCORE_CURRENT_SPREE = 3,
	SCORE_KING_OF_THE_HILL = 4,
};

enum class EDummyTest
{
	NONE,
	BLMAPCHILL_POLICE,
};

#endif
