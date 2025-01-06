#ifndef GAME_SERVER_DDPP_ENUMS_H
#define GAME_SERVER_DDPP_ENUMS_H

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
