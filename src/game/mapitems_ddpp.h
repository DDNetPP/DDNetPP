/* DDNet++ tiles */
#ifndef GAME_MAPITEMS_DDPP_H
#define GAME_MAPITEMS_DDPP_H

#include <engine/shared/protocol.h>

// layer types
enum
{
	ENTITY_WEAPON_GUN = 50,
	ENTITY_WEAPON_HAMMER,
	ENTITY_PLANT, // 243

	TILE_MONEY_PLUS = 114,
	TILE_MONEY_DOUBLE = 115,

	TILE_BANK_IN = 119,
	TILE_JAIL = 121,
	TILE_JAILRELEASE = 122,
	TILE_BALANCE_BATTLE_1 = 123,
	TILE_BALANCE_BATTLE_2 = 124,
	TILE_FNG_SCORE = 130,
	TILE_BLOCK_TOURNA_SPAWN = 131,
	TILE_PVP_ARENA_SPAWN = 132,

	TILE_BLOCKWAVE_HUMAN = 134,
	TILE_BLOCKWAVE_BOT = 135,

	TILE_RAINBOW = 145,
	TILE_ATOM = 146,
	TILE_TRAIL = 147,

	TILE_VANILLA_MODE = 152,
	TILE_DDRACE_MODE = 153,

	TILE_BLOODY = 154,

	TILE_MONEY = 160,
	TILE_SHOP = 161,
	TILE_ROOM = 162,
	TILE_DDPP_END = 163,
	TILE_MONEY_POLICE = 164,

	// block deathmath spawns
	TILE_BLOCK_DM_A1 = 165, //arena 1
	TILE_BLOCK_DM_A2 = 166, //arena 2
	TILE_BLOCK_DM_JOIN = 167, //join block dm mode

	TILE_SURVIVAL_LOBBY = 177,
	TILE_SURVIVAL_SPAWN = 178,
	TILE_SURVIVAL_DEATHMATCH = 179,

	TILE_SPREAD_GUN = 180,

	// botspawns planned for survival and singleplayer levels
	TILE_BOTSPAWN_1 = 181,
	TILE_BOTSPAWN_2 = 182,
	TILE_BOTSPAWN_3 = 183,
	TILE_BOTSPAWN_4 = 184,

	TILE_NO_HAMMER = 185,

	TILE_SHOP_SPAWN = 255, // entitiy index 64

	// NOT ACTUAL TILES - start
	CFG_TILE_OFF = 0,
	CFG_TILE_FREEZE,
	CFG_TILE_UNFREEZE,
	CFG_TILE_DEEP,
	CFG_TILE_UNDEEP,
	CFG_TILE_HOOK,
	CFG_TILE_UNHOOK,
	CFG_TILE_DEATH,
	CFG_TILE_GIVE_BLOODY,
	CFG_TILE_GIVE_RAINBOW,
	CFG_TILE_GIVE_SPREADGUN,
	// NOT ACTUAL TILES - end
	TILE_CONFIG_1 = 182,
	TILE_CONFIG_2 = 183,

	TILE_END_CUSTOM, // pack alle neuen tiles hier dr�ber! (all new tiles on top of this line pls)

	// F-DDrace
	NUM_INDICES = 256,
};

#endif