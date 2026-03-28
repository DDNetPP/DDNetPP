#ifdef CONF_LUA
#include "lua_plugin.h"

#include <base/dbg.h>
#include <base/log.h>
#include <base/secure.h>
#include <base/str.h>
#include <base/types.h>
#include <base/vmath.h>

#include <engine/shared/protocol.h>

#include <generated/protocol.h>

#include <game/gamecore.h>
#include <game/mapitems.h>
#include <game/server/ddnetpp/lua/console_strings.h>
#include <game/server/ddnetpp/lua/custom_lua_types.h>
#include <game/server/ddnetpp/lua/lua_game.h>
#include <game/server/ddnetpp/lua/position.h>
#include <game/server/ddnetpp/lua/stack_checker.h>
#include <game/server/ddnetpp/lua/table_unpacker.h>
#include <game/server/entities/character.h>
#include <game/server/entities/laser_text.h>
#include <game/server/gamecontext.h>
#include <game/server/gamecontroller.h>
#include <game/server/player.h>
#include <game/teamscore.h>

#include <insta/server/skin_info_manager.h>

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

static CLuaPlayerHandle *LuaCheckCharacterHandle(lua_State *L, int Index)
{
	auto *pPlayerHandle = static_cast<CLuaPlayerHandle *>(luaL_checkudata(L, Index, "Character"));
	if(!pPlayerHandle)
		return nullptr;
	CPlayer *pPlayer = pPlayerHandle->m_pPlugin->Game()->GameServer()->GetPlayerByUniqueId(pPlayerHandle->m_UniqueClientId);
	if(!pPlayer)
	{
		// Is it weird to say "invalid Player"
		// when calling a character instance method?
		// Its technically the correct error but weird to expose that to the user.
		// Hmm...
		luaL_error(L, "invalid Player");
		return nullptr;
	}
	CCharacter *pChr = pPlayer->GetCharacter();
	if(!pChr)
	{
		luaL_error(L, "invalid Character");
		return nullptr;
	}
	return pPlayerHandle;
}

CLuaPlugin::CLuaPlugin(const char *pName, const char *pFullPath, CLuaGame *pGame)
{
	log_info("lua", "initializing plugin %s ...", pName);
	m_pLuaState = luaL_newstate();
	luaL_openlibs(LuaState());
	str_copy(m_aName, pName);
	str_copy(m_aFullPath, pFullPath);
	m_pGame = pGame;
}

CLuaPlugin::~CLuaPlugin()
{
	if(LuaState())
	{
		// WARNING: logging here segfaults on rcon command "plugins reload"
		//          https://github.com/DDNetPP/DDNetPP/issues/524#issuecomment-4066439351
		// log_info("lua", "cleaning up plugin '%s' ...", Name());
		FreeSnapIds();
		lua_close(LuaState());
	}
}

void CLuaPlugin::FreeSnapIds()
{
	for(int SnapId : m_vSnapIds)
		Game()->Server()->SnapFreeId(SnapId);
	m_vSnapIds.clear();
}

template<typename T>
T *CLuaPlugin::SnapNewItem(int Id)
{
	int ItemKey = (T::ms_MsgId << 16) | Id;
	if(Game()->Server()->SnapBuilderGetItemData(ItemKey))
	{
		luaL_error(LuaState(), "snap item with type_id=%d and id=%d already exists", T::ms_MsgId, Id);
		return nullptr;
	}

	return Game()->Server()->SnapNewItem<T>(Id);
}

void *CLuaPlugin::SnapNewItem(int Type, int Id, int Size)
{
	int ItemKey = (Type << 16) | Id;
	if(Game()->Server()->SnapBuilderGetItemData(ItemKey))
	{
		luaL_error(LuaState(), "snap item with type_id=%d and id=%d already exists", Type, Id);
		return nullptr;
	}

	return Game()->Server()->SnapNewItem(Type, Id, Size);
}

void CLuaPlugin::RegisterPlayerMetaTable()
{
	LUA_CHECK_STACK(LuaState());

	if(luaL_newmetatable(LuaState(), "Player") == 0)
		dbg_assert_failed("lua metatable Player already exists");

	// --- Define __index (methods) ---
	lua_pushstring(LuaState(), "__index");
	lua_newtable(LuaState()); // Create method table

	// Add methods
	lua_pushstring(LuaState(), "id");
	lua_pushcfunction(LuaState(), CallbackPlayerId);
	lua_settable(LuaState(), -3);

	lua_pushstring(LuaState(), "name");
	lua_pushcfunction(LuaState(), CallbackPlayerName);
	lua_settable(LuaState(), -3);

	lua_pushstring(LuaState(), "set_skin");
	lua_pushcfunction(LuaState(), CallbackPlayerSetSkin);
	lua_settable(LuaState(), -3);

	lua_pushstring(LuaState(), "unset_skin");
	lua_pushcfunction(LuaState(), CallbackPlayerUnsetSkin);
	lua_settable(LuaState(), -3);

	lua_pushstring(LuaState(), "unset_skin_color_body");
	lua_pushcfunction(LuaState(), CallbackPlayerUnsetSkinColorBody);
	lua_settable(LuaState(), -3);

	// Set __index = method_table
	lua_settable(LuaState(), -3);

	lua_pop(LuaState(), 1); // Pop metatable
}

void CLuaPlugin::RegisterCharacterMetaTable()
{
	LUA_CHECK_STACK(LuaState());

	if(luaL_newmetatable(LuaState(), "Character") == 0)
		dbg_assert_failed("lua metatable Character already exists");

	// --- Define __index (methods) ---
	lua_pushstring(LuaState(), "__index");
	lua_newtable(LuaState()); // Create method table

	// Add methods
	lua_pushstring(LuaState(), "pos");
	lua_pushcfunction(LuaState(), CallbackCharacterPos);
	lua_settable(LuaState(), -3);

	lua_pushstring(LuaState(), "set_position");
	lua_pushcfunction(LuaState(), CallbackCharacterSetPosition);
	lua_settable(LuaState(), -3);

	lua_pushstring(LuaState(), "id");
	lua_pushcfunction(LuaState(), CallbackCharacterId);
	lua_settable(LuaState(), -3);

	lua_pushstring(LuaState(), "player");
	lua_pushcfunction(LuaState(), CallbackCharacterPlayer);
	lua_settable(LuaState(), -3);

	lua_pushstring(LuaState(), "die");
	lua_pushcfunction(LuaState(), CallbackCharacterDie);
	lua_settable(LuaState(), -3);

	lua_pushstring(LuaState(), "give_weapon");
	lua_pushcfunction(LuaState(), CallbackCharacterGiveWeapon);
	lua_settable(LuaState(), -3);

	lua_pushstring(LuaState(), "remove_weapon");
	lua_pushcfunction(LuaState(), CallbackCharacterRemoveWeapon);
	lua_settable(LuaState(), -3);

	// Set __index = method_table
	lua_settable(LuaState(), -3);

	lua_pop(LuaState(), 1); // Pop metatable
}

void CLuaPlugin::RegisterGlobalDDNetPPInstance()
{
	LUA_CHECK_STACK(LuaState());

	lua_newtable(LuaState());

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackLogInfo, 1);
	lua_setfield(LuaState(), -2, "log_info");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackLogWarn, 1);
	lua_setfield(LuaState(), -2, "log_warn");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackLogError, 1);
	lua_setfield(LuaState(), -2, "log_error");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackSendChat, 1);
	lua_setfield(LuaState(), -2, "send_chat");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackSendChatTarget, 1);
	lua_setfield(LuaState(), -2, "send_chat_target");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackSendBroadcast, 1);
	lua_setfield(LuaState(), -2, "send_broadcast");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackSendBroadcastTarget, 1);
	lua_setfield(LuaState(), -2, "send_broadcast_target");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackSendMotd, 1);
	lua_setfield(LuaState(), -2, "send_motd");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackSendMotdTarget, 1);
	lua_setfield(LuaState(), -2, "send_motd_target");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackLaserText, 1);
	lua_setfield(LuaState(), -2, "laser_text");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackCreateDamageIndicator, 1);
	lua_setfield(LuaState(), -2, "create_damage_indicator");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackCreateHammerHit, 1);
	lua_setfield(LuaState(), -2, "create_hammer_hit");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackCreateExplosion, 1);
	lua_setfield(LuaState(), -2, "create_explosion");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackCreatePlayerSpawn, 1);
	lua_setfield(LuaState(), -2, "create_player_spawn");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackCreateDeath, 1);
	lua_setfield(LuaState(), -2, "create_death");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackCreateSound, 1);
	lua_setfield(LuaState(), -2, "create_sound");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackCreateSoundGlobal, 1);
	lua_setfield(LuaState(), -2, "create_sound_global");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackRcon, 1);
	lua_setfield(LuaState(), -2, "rcon");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackSendVoteClearOptions, 1);
	lua_setfield(LuaState(), -2, "send_vote_clear_options");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackSendVoteOptionAdd, 1);
	lua_setfield(LuaState(), -2, "send_vote_option_add");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackGetPlayer, 1);
	lua_setfield(LuaState(), -2, "get_player");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackCallPlugin, 1);
	lua_setfield(LuaState(), -2, "call_plugin");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackGetCharacter, 1);
	lua_setfield(LuaState(), -2, "get_character");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackRegisterRcon, 1);
	lua_setfield(LuaState(), -2, "register_rcon");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackRegisterChat, 1);
	lua_setfield(LuaState(), -2, "register_chat");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackSecureRandBelow, 1);
	lua_setfield(LuaState(), -2, "secure_rand_below");

	lua_pushlightuserdata(LuaState(), this);
	lua_pushcclosure(LuaState(), CallbackPluginName, 1);
	lua_setfield(LuaState(), -2, "plugin_name");

	// ddnetpp.snap sub table
	{
		lua_newtable(LuaState());

		lua_pushlightuserdata(LuaState(), this);
		lua_pushcclosure(LuaState(), CallbackSnapNewId, 1);
		lua_setfield(LuaState(), -2, "new_id");

		lua_pushlightuserdata(LuaState(), this);
		lua_pushcclosure(LuaState(), CallbackSnapFreeId, 1);
		lua_setfield(LuaState(), -2, "free_id");

		lua_pushlightuserdata(LuaState(), this);
		lua_pushcclosure(LuaState(), CallbackSnapNewLaser, 1);
		lua_setfield(LuaState(), -2, "new_laser");

		lua_pushlightuserdata(LuaState(), this);
		lua_pushcclosure(LuaState(), CallbackSnapNewPickup, 1);
		lua_setfield(LuaState(), -2, "new_pickup");

		lua_pushlightuserdata(LuaState(), this);
		lua_pushcclosure(LuaState(), CallbackSnapNewCharacter, 1);
		lua_setfield(LuaState(), -2, "new_character");

		lua_pushlightuserdata(LuaState(), this);
		lua_pushcclosure(LuaState(), CallbackSnapNewPlayerInfo, 1);
		lua_setfield(LuaState(), -2, "new_player_info");

		lua_pushlightuserdata(LuaState(), this);
		lua_pushcclosure(LuaState(), CallbackSnapNewClientInfo, 1);
		lua_setfield(LuaState(), -2, "new_client_info");

		lua_pushlightuserdata(LuaState(), this);
		lua_pushcclosure(LuaState(), CallbackSnapNewProjectile, 1);
		lua_setfield(LuaState(), -2, "new_projectile");
	}
	lua_setfield(LuaState(), -2, "snap");

	// ddnetpp.server sub table
	{
		lua_newtable(LuaState());

		lua_pushlightuserdata(LuaState(), this);
		lua_pushcclosure(LuaState(), CallbackServerTick, 1);
		lua_setfield(LuaState(), -2, "tick");

		lua_pushlightuserdata(LuaState(), this);
		lua_pushcclosure(LuaState(), CallbackServerTickSpeed, 1);
		lua_setfield(LuaState(), -2, "tick_speed");
	}
	lua_setfield(LuaState(), -2, "server");

	// ddnetpp.collision sub table
	{
		lua_newtable(LuaState());

		lua_pushlightuserdata(LuaState(), this);
		lua_pushcclosure(LuaState(), CallbackCollisionWidth, 1);
		lua_setfield(LuaState(), -2, "width");

		lua_pushlightuserdata(LuaState(), this);
		lua_pushcclosure(LuaState(), CallbackCollisionHeight, 1);
		lua_setfield(LuaState(), -2, "height");

		lua_pushlightuserdata(LuaState(), this);
		lua_pushcclosure(LuaState(), CallbackCollisionGetTileIndex, 1);
		lua_setfield(LuaState(), -2, "get_tile_index");

		lua_pushlightuserdata(LuaState(), this);
		lua_pushcclosure(LuaState(), CallbackCollisionGetTile, 1);
		lua_setfield(LuaState(), -2, "get_tile");

		lua_pushlightuserdata(LuaState(), this);
		lua_pushcclosure(LuaState(), CallbackCollisionMoveBox, 1);
		lua_setfield(LuaState(), -2, "move_box");
	}
	lua_setfield(LuaState(), -2, "collision");

	// ddnetpp.protocol sub table
	{
		lua_newtable(LuaState());

		lua_pushstring(LuaState(), "POWERUP_HEALTH");
		lua_pushinteger(LuaState(), POWERUP_HEALTH);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "POWERUP_ARMOR");
		lua_pushinteger(LuaState(), POWERUP_ARMOR);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "POWERUP_WEAPON");
		lua_pushinteger(LuaState(), POWERUP_WEAPON);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "POWERUP_NINJA");
		lua_pushinteger(LuaState(), POWERUP_NINJA);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "POWERUP_ARMOR_SHOTGUN");
		lua_pushinteger(LuaState(), POWERUP_ARMOR_SHOTGUN);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "POWERUP_ARMOR_GRENADE");
		lua_pushinteger(LuaState(), POWERUP_ARMOR_GRENADE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "POWERUP_ARMOR_NINJA");
		lua_pushinteger(LuaState(), POWERUP_ARMOR_NINJA);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "POWERUP_ARMOR_LASER");
		lua_pushinteger(LuaState(), POWERUP_ARMOR_LASER);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "NUM_POWERUPS");
		lua_pushinteger(LuaState(), NUM_POWERUPS);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "WEAPON_HAMMER");
		lua_pushinteger(LuaState(), WEAPON_HAMMER);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "WEAPON_GUN");
		lua_pushinteger(LuaState(), WEAPON_GUN);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "WEAPON_SHOTGUN");
		lua_pushinteger(LuaState(), WEAPON_SHOTGUN);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "WEAPON_GRENADE");
		lua_pushinteger(LuaState(), WEAPON_GRENADE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "WEAPON_LASER");
		lua_pushinteger(LuaState(), WEAPON_LASER);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "WEAPON_NINJA");
		lua_pushinteger(LuaState(), WEAPON_NINJA);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "NUM_WEAPONS");
		lua_pushinteger(LuaState(), NUM_WEAPONS);
		lua_settable(LuaState(), -3);
	}
	lua_setfield(LuaState(), -2, "protocol");

	// ddnetpp.skin_priority sub table
	{
		lua_newtable(LuaState());

		lua_pushstring(LuaState(), "USER");
		lua_pushinteger(LuaState(), (int)ESkinPrio::USER);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "LOW");
		lua_pushinteger(LuaState(), (int)ESkinPrio::LOW);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "RAINBOW");
		lua_pushinteger(LuaState(), (int)ESkinPrio::RAINBOW);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "HIGH");
		lua_pushinteger(LuaState(), (int)ESkinPrio::HIGH);
		lua_settable(LuaState(), -3);
	}
	lua_setfield(LuaState(), -2, "skin_priority");

	// ddnetpp.tile sub table
	{
		lua_newtable(LuaState());

		lua_pushstring(LuaState(), "AIR");
		lua_pushinteger(LuaState(), TILE_AIR);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "SOLID");
		lua_pushinteger(LuaState(), TILE_SOLID);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "DEATH");
		lua_pushinteger(LuaState(), TILE_DEATH);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "NOHOOK");
		lua_pushinteger(LuaState(), TILE_NOHOOK);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "FREEZE");
		lua_pushinteger(LuaState(), TILE_FREEZE);
		lua_settable(LuaState(), -3);
	}
	lua_setfield(LuaState(), -2, "tile");

	// ddnetpp.weapon sub table
	{
		lua_newtable(LuaState());

		lua_pushstring(LuaState(), "GAME");
		lua_pushinteger(LuaState(), WEAPON_GAME);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "SELF");
		lua_pushinteger(LuaState(), WEAPON_SELF);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "WORLD");
		lua_pushinteger(LuaState(), WEAPON_WORLD);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "NONE");
		lua_pushinteger(LuaState(), -1);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "HAMMER");
		lua_pushinteger(LuaState(), WEAPON_HAMMER);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "GUN");
		lua_pushinteger(LuaState(), WEAPON_GUN);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "SHOTGUN");
		lua_pushinteger(LuaState(), WEAPON_SHOTGUN);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "GRENADE");
		lua_pushinteger(LuaState(), WEAPON_GRENADE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "LASER");
		lua_pushinteger(LuaState(), WEAPON_LASER);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "NINJA");
		lua_pushinteger(LuaState(), WEAPON_NINJA);
		lua_settable(LuaState(), -3);
	}
	lua_setfield(LuaState(), -2, "weapon");

	// ddnetpp.hook sub table
	{
		lua_newtable(LuaState());

		lua_pushstring(LuaState(), "RETRACTED");
		lua_pushinteger(LuaState(), HOOK_RETRACTED);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "IDLE");
		lua_pushinteger(LuaState(), HOOK_IDLE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "RETRACT_START");
		lua_pushinteger(LuaState(), HOOK_RETRACT_START);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "RETRACT_END");
		lua_pushinteger(LuaState(), HOOK_RETRACT_END);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "FLYING");
		lua_pushinteger(LuaState(), HOOK_FLYING);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "GRABBED");
		lua_pushinteger(LuaState(), HOOK_GRABBED);
		lua_settable(LuaState(), -3);
	}
	lua_setfield(LuaState(), -2, "hook");

	// ddnetpp.eye_emote sub table
	{
		lua_newtable(LuaState());

		lua_pushstring(LuaState(), "NORMAL");
		lua_pushinteger(LuaState(), EMOTE_NORMAL);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "PAIN");
		lua_pushinteger(LuaState(), EMOTE_PAIN);
		lua_settable(LuaState(), -3);
	}
	lua_setfield(LuaState(), -2, "eye_emote");

	// ddnetpp.team sub table
	{
		lua_newtable(LuaState());

		lua_pushstring(LuaState(), "ALL");
		lua_pushinteger(LuaState(), TEAM_ALL);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "SPECTATORS");
		lua_pushinteger(LuaState(), TEAM_SPECTATORS);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "RED");
		lua_pushinteger(LuaState(), TEAM_RED);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "BLUE");
		lua_pushinteger(LuaState(), TEAM_BLUE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "GAME");
		lua_pushinteger(LuaState(), TEAM_GAME);
		lua_settable(LuaState(), -3);
	}
	lua_setfield(LuaState(), -2, "team");

	// ddnetpp.sound sub table
	{
		lua_newtable(LuaState());

		lua_pushstring(LuaState(), "GUN_FIRE");
		lua_pushinteger(LuaState(), SOUND_GUN_FIRE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "SHOTGUN_FIRE");
		lua_pushinteger(LuaState(), SOUND_SHOTGUN_FIRE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "GRENADE_FIRE");
		lua_pushinteger(LuaState(), SOUND_GRENADE_FIRE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "HAMMER_FIRE");
		lua_pushinteger(LuaState(), SOUND_HAMMER_FIRE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "HAMMER_HIT");
		lua_pushinteger(LuaState(), SOUND_HAMMER_HIT);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "NINJA_FIRE");
		lua_pushinteger(LuaState(), SOUND_NINJA_FIRE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "GRENADE_EXPLODE");
		lua_pushinteger(LuaState(), SOUND_GRENADE_EXPLODE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "NINJA_HIT");
		lua_pushinteger(LuaState(), SOUND_NINJA_HIT);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "LASER_FIRE");
		lua_pushinteger(LuaState(), SOUND_LASER_FIRE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "LASER_BOUNCE");
		lua_pushinteger(LuaState(), SOUND_LASER_BOUNCE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "WEAPON_SWITCH");
		lua_pushinteger(LuaState(), SOUND_WEAPON_SWITCH);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "PLAYER_PAIN_SHORT");
		lua_pushinteger(LuaState(), SOUND_PLAYER_PAIN_SHORT);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "PLAYER_PAIN_LONG");
		lua_pushinteger(LuaState(), SOUND_PLAYER_PAIN_LONG);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "BODY_LAND");
		lua_pushinteger(LuaState(), SOUND_BODY_LAND);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "PLAYER_AIRJUMP");
		lua_pushinteger(LuaState(), SOUND_PLAYER_AIRJUMP);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "PLAYER_JUMP");
		lua_pushinteger(LuaState(), SOUND_PLAYER_JUMP);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "PLAYER_DIE");
		lua_pushinteger(LuaState(), SOUND_PLAYER_DIE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "PLAYER_SPAWN");
		lua_pushinteger(LuaState(), SOUND_PLAYER_SPAWN);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "PLAYER_SKID");
		lua_pushinteger(LuaState(), SOUND_PLAYER_SKID);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "TEE_CRY");
		lua_pushinteger(LuaState(), SOUND_TEE_CRY);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "HOOK_LOOP");
		lua_pushinteger(LuaState(), SOUND_HOOK_LOOP);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "HOOK_ATTACH_GROUND");
		lua_pushinteger(LuaState(), SOUND_HOOK_ATTACH_GROUND);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "HOOK_ATTACH_PLAYER");
		lua_pushinteger(LuaState(), SOUND_HOOK_ATTACH_PLAYER);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "HOOK_NOATTACH");
		lua_pushinteger(LuaState(), SOUND_HOOK_NOATTACH);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "PICKUP_HEALTH");
		lua_pushinteger(LuaState(), SOUND_PICKUP_HEALTH);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "PICKUP_ARMOR");
		lua_pushinteger(LuaState(), SOUND_PICKUP_ARMOR);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "PICKUP_GRENADE");
		lua_pushinteger(LuaState(), SOUND_PICKUP_GRENADE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "PICKUP_SHOTGUN");
		lua_pushinteger(LuaState(), SOUND_PICKUP_SHOTGUN);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "PICKUP_NINJA");
		lua_pushinteger(LuaState(), SOUND_PICKUP_NINJA);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "WEAPON_SPAWN");
		lua_pushinteger(LuaState(), SOUND_WEAPON_SPAWN);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "WEAPON_NOAMMO");
		lua_pushinteger(LuaState(), SOUND_WEAPON_NOAMMO);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "HIT");
		lua_pushinteger(LuaState(), SOUND_HIT);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "CHAT_SERVER");
		lua_pushinteger(LuaState(), SOUND_CHAT_SERVER);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "CHAT_CLIENT");
		lua_pushinteger(LuaState(), SOUND_CHAT_CLIENT);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "CHAT_HIGHLIGHT");
		lua_pushinteger(LuaState(), SOUND_CHAT_HIGHLIGHT);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "CTF_DROP");
		lua_pushinteger(LuaState(), SOUND_CTF_DROP);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "CTF_RETURN");
		lua_pushinteger(LuaState(), SOUND_CTF_RETURN);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "CTF_GRAB_PL");
		lua_pushinteger(LuaState(), SOUND_CTF_GRAB_PL);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "CTF_GRAB_EN");
		lua_pushinteger(LuaState(), SOUND_CTF_GRAB_EN);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "CTF_CAPTURE");
		lua_pushinteger(LuaState(), SOUND_CTF_CAPTURE);
		lua_settable(LuaState(), -3);

		lua_pushstring(LuaState(), "MENU");
		lua_pushinteger(LuaState(), SOUND_MENU);
		lua_settable(LuaState(), -3);
	}
	lua_setfield(LuaState(), -2, "sound");

	// ddnetpp.flags sub table
	{
		lua_newtable(LuaState());

		// ddnetpp.flags.gameinfo sub sub table
		{
			lua_newtable(LuaState());

			lua_pushstring(LuaState(), "TIMESCORE");
			lua_pushinteger(LuaState(), GAMEINFOFLAG_TIMESCORE);
			lua_settable(LuaState(), -3);

			lua_pushstring(LuaState(), "GAMETYPE_RACE");
			lua_pushinteger(LuaState(), GAMEINFOFLAG_GAMETYPE_RACE);
			lua_settable(LuaState(), -3);

			lua_pushstring(LuaState(), "GAMETYPE_FASTCAP");
			lua_pushinteger(LuaState(), GAMEINFOFLAG_GAMETYPE_FASTCAP);
			lua_settable(LuaState(), -3);

			lua_pushstring(LuaState(), "GAMETYPE_FNG");
			lua_pushinteger(LuaState(), GAMEINFOFLAG_GAMETYPE_FNG);
			lua_settable(LuaState(), -3);

			lua_pushstring(LuaState(), "GAMETYPE_DDRACE");
			lua_pushinteger(LuaState(), GAMEINFOFLAG_GAMETYPE_DDRACE);
			lua_settable(LuaState(), -3);
		}
		lua_setfield(LuaState(), -2, "gameinfo");
	}
	lua_setfield(LuaState(), -2, "flags");

	lua_setglobal(LuaState(), "ddnetpp");
}

void CLuaPlugin::RegisterGlobalState()
{
	LUA_CHECK_STACK(LuaState());

	RegisterGlobalDDNetPPInstance();
	RegisterPlayerMetaTable();
	RegisterCharacterMetaTable();
}

bool CLuaPlugin::LoadFile()
{
	if(luaL_dofile(LuaState(), FullPath()) != LUA_OK)
	{
		const char *pError = lua_tostring(LuaState(), -1);
		log_error("lua", "%s: %s", FullPath(), pError);
		SetError(pError);
		lua_pop(LuaState(), 1);
		return false;
	}
	return true;
}

bool CLuaPlugin::CallLuaVoidNoArgs(const char *pFunction)
{
	LUA_CHECK_STACK_DETAIL(LuaState(), pFunction);
	lua_getglobal(LuaState(), "ddnetpp");
	lua_getfield(LuaState(), -1, pFunction);

	// lua_getglobal(LuaState(), pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal and getfield because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is nil", pFunction);
		return false;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal and getfield because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is not a function", pFunction);
		return false;
	}
	if(lua_pcall(LuaState(), 0, 0, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to call %s() with error: %s", Name(), pFunction, pErrorMsg);
		SetError(pErrorMsg);
		lua_pop(LuaState(), 1);
	}
	// pop the global "ddnetpp"
	lua_pop(LuaState(), 1);
	return true;
}

bool CLuaPlugin::CallLuaVoidWithOneInt(const char *pFunction, int Num1)
{
	LUA_CHECK_STACK_DETAIL(LuaState(), pFunction);
	lua_getglobal(LuaState(), "ddnetpp");
	lua_getfield(LuaState(), -1, pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is nil", pFunction);
		return false;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is not a function", pFunction);
		return false;
	}
	lua_pushinteger(LuaState(), Num1);
	if(lua_pcall(LuaState(), 1, 0, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to call %s() with error: %s", Name(), pFunction, pErrorMsg);
		SetError(pErrorMsg);
		lua_pop(LuaState(), 1);
	}
	// pop global "ddnetpp"
	lua_pop(LuaState(), 1);
	return true;
}

bool CLuaPlugin::CallLuaVoidWithTwoInts(const char *pFunction, int Num1, int Num2)
{
	LUA_CHECK_STACK_DETAIL(LuaState(), pFunction);
	lua_getglobal(LuaState(), "ddnetpp");
	lua_getfield(LuaState(), -1, pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is nil", pFunction);
		return false;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is not a function", pFunction);
		return false;
	}
	lua_pushinteger(LuaState(), Num1);
	lua_pushinteger(LuaState(), Num2);
	if(lua_pcall(LuaState(), 2, 0, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to call %s() with error: %s", Name(), pFunction, pErrorMsg);
		SetError(pErrorMsg);
		lua_pop(LuaState(), 1);
	}
	// pop global "ddnetpp"
	lua_pop(LuaState(), 1);
	return true;
}

std::optional<int> CLuaPlugin::CallLuaIntWithTwoInts(const char *pFunction, int Num1, int Num2)
{
	LUA_CHECK_STACK_DETAIL(LuaState(), pFunction);
	lua_getglobal(LuaState(), "ddnetpp");
	lua_getfield(LuaState(), -1, pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is nil", pFunction);
		return std::nullopt;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is not a function", pFunction);
		return std::nullopt;
	}
	lua_pushinteger(LuaState(), Num1);
	lua_pushinteger(LuaState(), Num2);
	if(lua_pcall(LuaState(), 2, 1, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to call %s() with error: %s", Name(), pFunction, pErrorMsg);
		SetError(pErrorMsg);
		lua_pop(LuaState(), 2);
		return std::nullopt;
	}

	if(!lua_isinteger(LuaState(), -1))
	{
		lua_Debug LuaInfo;
		lua_getglobal(LuaState(), pFunction);
		lua_getinfo(LuaState(), ">Sl", &LuaInfo);

		char aError[512];
		str_format(
			aError,
			sizeof(aError),
			"%s:%d return value for '%s' should be integer, got %s",
			LuaInfo.short_src,
			LuaInfo.linedefined,
			pFunction,
			luaL_typename(LuaState(), -1));
		log_error("lua", "%s", aError);
		SetError(aError);
		// pop error and global "ddnetpp"
		lua_pop(LuaState(), 2);
		return std::nullopt;
	}

	int Value = lua_tointeger(LuaState(), -1);
	// pop value and global "ddnetpp"
	lua_pop(LuaState(), 2);
	return Value;
}

bool CLuaPlugin::CallLuaVoidWithPlayer(const char *pFunction, const CPlayer *pPlayer)
{
	LUA_CHECK_STACK_DETAIL(LuaState(), pFunction);
	lua_getglobal(LuaState(), "ddnetpp");
	lua_getfield(LuaState(), -1, pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is nil", pFunction);
		return false;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is not a function", pFunction);
		return false;
	}
	auto *pPlayerHandle = static_cast<CLuaPlayerHandle *>(lua_newuserdatauv(LuaState(), sizeof(CLuaPlayerHandle), 0));
	pPlayerHandle->Init(pPlayer->GetUniqueCid(), this);
	luaL_setmetatable(LuaState(), "Player");
	if(lua_pcall(LuaState(), 1, 0, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to call %s() with error: %s", Name(), pFunction, pErrorMsg);
		SetError(pErrorMsg);
		lua_pop(LuaState(), 1);
	}
	// pop global "ddnetpp"
	lua_pop(LuaState(), 1);
	return true;
}

int CLuaPlugin::CallbackLogInfo(lua_State *L)
{
	int NumArgs = lua_gettop(L);
	CLuaPlugin *pPlugin = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));
	if(NumArgs == 1)
	{
		const char *pMessage = luaL_checkstring(L, 1);
		log_info(pPlugin->Name(), "%s", pMessage);
	}
	else if(NumArgs == 2)
	{
		const char *pSystem = luaL_checkstring(L, 1);
		const char *pMessage = luaL_checkstring(L, 2);
		log_info(pSystem, "%s", pMessage);
	}
	else
	{
		luaL_error(L, "too many arguments for logger");
	}
	return 0;
}

int CLuaPlugin::CallbackLogWarn(lua_State *L)
{
	int NumArgs = lua_gettop(L);
	CLuaPlugin *pPlugin = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));
	if(NumArgs == 1)
	{
		const char *pMessage = luaL_checkstring(L, 1);
		log_warn(pPlugin->Name(), "%s", pMessage);
	}
	else if(NumArgs == 2)
	{
		const char *pSystem = luaL_checkstring(L, 1);
		const char *pMessage = luaL_checkstring(L, 2);
		log_warn(pSystem, "%s", pMessage);
	}
	else
	{
		luaL_error(L, "too many arguments for logger");
	}
	return 0;
}

int CLuaPlugin::CallbackLogError(lua_State *L)
{
	int NumArgs = lua_gettop(L);
	CLuaPlugin *pPlugin = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));
	if(NumArgs == 1)
	{
		const char *pMessage = luaL_checkstring(L, 1);
		log_error(pPlugin->Name(), "%s", pMessage);
	}
	else if(NumArgs == 2)
	{
		const char *pSystem = luaL_checkstring(L, 1);
		const char *pMessage = luaL_checkstring(L, 2);
		log_error(pSystem, "%s", pMessage);
	}
	else
	{
		luaL_error(L, "too many arguments for logger");
	}
	return 0;
}

int CLuaPlugin::CallbackSendChat(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();
	const char *pMessage = luaL_checkstring(L, 1);
	pGame->SendChat(pMessage);
	return 0;
}

int CLuaPlugin::CallbackSendChatTarget(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();
	int ClientId = LuaCheckClientId(L, 1);
	const char *pMessage = luaL_checkstring(L, 2);
	pGame->GameServer()->SendChatTarget(ClientId, pMessage);
	return 0;
}

int CLuaPlugin::CallbackSendBroadcast(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();
	const char *pMessage = luaL_checkstring(L, 1);
	pGame->GameServer()->SendBroadcastAll(pMessage);
	return 0;
}

int CLuaPlugin::CallbackSendBroadcastTarget(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();
	int ClientId = LuaCheckClientId(L, 1);
	const char *pMessage = luaL_checkstring(L, 2);
	pGame->GameServer()->SendBroadcast(pMessage, ClientId);
	return 0;
}

int CLuaPlugin::CallbackSendMotd(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();
	const char *pMessage = luaL_checkstring(L, 1);
	pGame->GameServer()->AbuseMotd(pMessage, -1);
	return 0;
}

int CLuaPlugin::CallbackSendMotdTarget(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();
	int ClientId = LuaCheckClientId(L, 1);
	const char *pMessage = luaL_checkstring(L, 2);
	pGame->GameServer()->AbuseMotd(pMessage, ClientId);
	return 0;
}

int CLuaPlugin::CallbackLaserText(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();

	vec2 Pos = LuaCheckArgPosition(L, 1);
	const char *pMessage = luaL_checkstring(L, 2);
	int AliveTicks = pGame->Server()->TickSpeed() * 3;
	if(lua_isinteger(L, 3))
	{
		AliveTicks = lua_tointeger(L, 3);
	}

	new CLaserText(
		&pGame->GameServer()->m_World,
		Pos,
		AliveTicks,
		pMessage);
	return 0;
}

int CLuaPlugin::CallbackCreateDamageIndicator(lua_State *L)
{
	int NumArgs = lua_gettop(L);
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();

	vec2 Pos = LuaCheckArgPosition(L, 1);
	float Angle = 0.0f;
	int Amount = 1;
	CClientMask Mask;
	Mask.set();

	if(NumArgs >= 2)
		Angle = luaL_checknumber(L, 2);
	if(NumArgs >= 3)
		Amount = luaL_checkinteger(L, 3);
	if(NumArgs >= 4)
		Mask = LuaCheckArgClientMask(L, 4);

	pGame->GameServer()->CreateDamageInd(Pos, Angle, Amount, Mask);

	return 0;
}

int CLuaPlugin::CallbackCreateHammerHit(lua_State *L)
{
	int NumArgs = lua_gettop(L);
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();

	vec2 Pos = LuaCheckArgPosition(L, 1);
	CClientMask Mask;
	Mask.set();
	if(NumArgs >= 2)
		Mask = LuaCheckArgClientMask(L, 2);

	pGame->GameServer()->CreateHammerHit(Pos);

	return 0;
}

int CLuaPlugin::CallbackCreateExplosion(lua_State *L)
{
	int NumArgs = lua_gettop(L);
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();

	vec2 Pos = LuaCheckArgPosition(L, 1);
	int OwnerId = -1;
	int Weapon = WEAPON_GRENADE;
	bool NoDamage = false;
	int ActivatedTeam = TEAM_FLOCK;
	CClientMask Mask;
	Mask.set();

	if(NumArgs >= 2)
		OwnerId = LuaCheckClientId(L, 2);
	if(NumArgs >= 3)
		Weapon = luaL_checkinteger(L, 3);
	if(NumArgs >= 4)
		NoDamage = lua_toboolean(L, 4); // TODO: where luaL_checkboolean?
	if(NumArgs >= 5)
		ActivatedTeam = luaL_checkinteger(L, 5);
	if(NumArgs >= 6)
		Mask = LuaCheckArgClientMask(L, 6);

	pGame->GameServer()->CreateExplosion(Pos, OwnerId, Weapon, NoDamage, ActivatedTeam, Mask);
	return 0;
}

int CLuaPlugin::CallbackCreatePlayerSpawn(lua_State *L)
{
	int NumArgs = lua_gettop(L);
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();

	vec2 Pos = LuaCheckArgPosition(L, 1);
	CClientMask Mask;
	Mask.set();
	if(NumArgs >= 2)
		Mask = LuaCheckArgClientMask(L, 2);

	pGame->GameServer()->CreatePlayerSpawn(Pos, Mask);

	return 0;
}

int CLuaPlugin::CallbackCreateDeath(lua_State *L)
{
	int NumArgs = lua_gettop(L);
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();

	vec2 Pos = LuaCheckArgPosition(L, 1);
	int ClientId = 0;
	CClientMask Mask;
	Mask.set();
	if(NumArgs >= 2)
		ClientId = luaL_checkinteger(L, 2);
	if(NumArgs >= 3)
		Mask = LuaCheckArgClientMask(L, 3);

	pGame->GameServer()->CreateDeath(Pos, ClientId, Mask);

	return 0;
}

int CLuaPlugin::CallbackCreateSound(lua_State *L)
{
	int NumArgs = lua_gettop(L);
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();

	vec2 Pos = LuaCheckArgPosition(L, 1);
	int SoundId = SOUND_HAMMER_FIRE;
	CClientMask Mask;
	Mask.set();
	if(NumArgs >= 2)
		SoundId = luaL_checkinteger(L, 2);
	if(NumArgs >= 3)
		Mask = LuaCheckArgClientMask(L, 3);

	pGame->GameServer()->CreateSound(Pos, SoundId, Mask);

	return 0;
}

int CLuaPlugin::CallbackCreateSoundGlobal(lua_State *L)
{
	int NumArgs = lua_gettop(L);
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();

	int SoundId = SOUND_HAMMER_FIRE;
	int TargetId = -1;
	if(NumArgs >= 1)
		SoundId = luaL_checkinteger(L, 1);
	if(NumArgs >= 2)
		TargetId = luaL_checkinteger(L, 2);

	pGame->GameServer()->CreateSoundGlobal(SoundId, TargetId);

	return 0;
}

int CLuaPlugin::CallbackRcon(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();
	const char *pCommand = luaL_checkstring(L, 1);
	// a safer but weaker api would be turning this off by default
	// and having another "rcon_dangerous()" method xd
	bool InterpretSemicolons = true;
	pGame->GameServer()->Console()->ExecuteLine(pCommand, IConsole::CLIENT_ID_UNSPECIFIED, InterpretSemicolons);
	return 0;
}

int CLuaPlugin::CallbackSendVoteClearOptions(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();
	int ClientId = luaL_checkinteger(L, 1);
	pGame->SendVoteClearOptions(ClientId);
	return 0;
}

int CLuaPlugin::CallbackSendVoteOptionAdd(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();
	int ClientId = luaL_checkinteger(L, 1);
	const char *pDescription = LuaCheckArgStringStrict(L, 2);
	pGame->SendVoteOptionAdd(ClientId, pDescription);
	return 0;
}

int CLuaPlugin::CallbackGetPlayer(lua_State *L)
{
	CLuaPlugin *pSelf = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));
	CLuaGame *pGame = pSelf->Game();
	int ClientId = luaL_checkinteger(L, 1);

	CPlayer *pPlayer = pGame->GameServer()->GetPlayerOrNullptr(ClientId);
	if(!pPlayer)
	{
		lua_pushnil(L);
		return 1;
	}

	auto *pPlayerHandle = static_cast<CLuaPlayerHandle *>(lua_newuserdatauv(L, sizeof(CLuaPlayerHandle), 0));
	pPlayerHandle->Init(pPlayer->GetUniqueCid(), pSelf);
	luaL_setmetatable(L, "Player");
	return 1;
}

int CLuaPlugin::CallbackGetCharacter(lua_State *L)
{
	CLuaPlugin *pSelf = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));
	CLuaGame *pGame = pSelf->Game();
	int ClientId = luaL_checkinteger(L, 1);

	CPlayer *pPlayer = pGame->GameServer()->GetPlayerOrNullptr(ClientId);
	if(!pPlayer)
	{
		lua_pushnil(L);
		return 1;
	}
	CCharacter *pChr = pPlayer->GetCharacter();
	// plugins should be powerful but simple
	// we do not want plugins to have to do a nil and a alive check
	// also CPlayer::GetCharacter() does return nullptr for dead characters
	// there should never be valuable information in dead character instances anyways
	// if there really is a use case for dead characters
	// there should be a new method for it like Game:get_dead_character() which makes it clear
	// and keeps the main api Game:get_character() clean and simple to use
	if(!pChr || !pChr->IsAlive())
	{
		lua_pushnil(L);
		return 1;
	}

	// it is a bit cursed to store a player handle for characters
	// but it works
	// when we lookup the character we just go through the player
	// the server has to do a lookup anyways and the client ids and unique client ids are the same
	// its just a different lua metatable
	auto *pPlayerHandle = static_cast<CLuaPlayerHandle *>(lua_newuserdatauv(L, sizeof(CLuaPlayerHandle), 0));
	pPlayerHandle->Init(pPlayer->GetUniqueCid(), pSelf);
	luaL_setmetatable(L, "Character");
	return 1;
}

int CLuaPlugin::CallbackCallPlugin(lua_State *L)
{
	CLuaPlugin *pSelf = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));
	CLuaGame *pGame = pSelf->Game();
	const char *pFunction = lua_tostring(L, 1);

	// TODO: now that we also have pSelf available it would be nicer to pass
	//       that as argument to CallPlugin instead of L
	//       so we can fully operate on the caller plugin if needed

	if(!pGame->Controller()->Lua()->CallPlugin(pFunction, L))
	{
		// luaL_error(L, "no plugin implements %s()", pFunction);

		// ok = false
		lua_pushboolean(L, false);

		// data = nil
		lua_pushnil(L);
		return 2;
	}

	// ok and data have to be set by CallPlugin
	return 2;
}

int CLuaPlugin::CallbackRegisterRcon(lua_State *L)
{
	CLuaPlugin *pSelf = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));

	const char *pName = LuaCheckArgStringStrict(L, 1);
	const char *pParams = LuaCheckArgStringStrict(L, 2);
	const char *pHelp = LuaCheckArgStringStrict(L, 3);
	luaL_checktype(L, 4, LUA_TFUNCTION);

	std::vector<CLuaRconCommand::CParam> vParams;
	char aError[512] = "";
	if(!CLuaRconCommand::ParseParameters(vParams, pParams, aError, sizeof(aError)))
	{
		luaL_error(L, "rcon cmd '%s' invalid params: %s", pName, aError);
		return 0;
	}

	if(pSelf->m_RconCommands.contains(std::string(pName)))
	{
		// silently override previous registrations
		// not sure what is the most user friendly here
		// could also throw an error so users dont get confused
		// but that takes away the flexibility to override library code
		// could also register both callbacks and run both
		// but thats also odd i think
		//
		// maybe the best would be to error on duplication by default
		// and provider another method like `Game:register_rcon_override()`
		// to intentionally override

		// log_warn("lua", "%s was already registered in rcon", pName);
		int OldFunc = pSelf->m_RconCommands.at(std::string(pName)).m_LuaCallbackRef;
		luaL_unref(L, LUA_REGISTRYINDEX, OldFunc);
	}

	// we need to move stack 3 to 0
	// because luaL_ref pops first element from the stack
	// but it is our third argument

	// so we just pop the first two args

	// push copy of the third argument (the lua callback)
	// onto the top of the stack so luaL_ref can find it
	lua_pushvalue(L, 4);
	int FuncRef = luaL_ref(L, LUA_REGISTRYINDEX);

	// TODO: according to https://en.cppreference.com/w/cpp/container/unordered_map/emplace
	//       if the same rcon command is registered twice it should do nothing
	//       because emplace ignores duplicated keys on the second insert
	//       but my testing showed that the rcon command defined lower in the lua script
	//       will actually be ran
	//       so it happens what i want to happen but as far as i understand it shouldnt
	//       so that is scary
	pSelf->m_RconCommands.emplace(
		std::string(pName),
		CLuaRconCommand({
			.m_pName = pName,
			.m_pHelp = pHelp,
			.m_pParams = pParams,
			.m_LuaCallbackRef = FuncRef,
		}));

	const CLuaRconCommand *pCmd = &pSelf->m_RconCommands.at(std::string(pName));
	pSelf->Game()->Controller()->Lua()->OnAddRconCmd(pCmd);

	// pop our pushed value
	lua_pop(L, 1);

	lua_pushboolean(L, true);
	return 1;
}

int CLuaPlugin::CallbackRegisterChat(lua_State *L)
{
	CLuaPlugin *pSelf = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));

	const char *pName = LuaCheckArgStringStrict(L, 1);
	const char *pParams = LuaCheckArgStringStrict(L, 2);
	const char *pHelp = LuaCheckArgStringStrict(L, 3);
	luaL_checktype(L, 4, LUA_TFUNCTION);

	std::vector<CLuaRconCommand::CParam> vParams;
	char aError[512] = "";
	if(!CLuaRconCommand::ParseParameters(vParams, pParams, aError, sizeof(aError)))
	{
		luaL_error(L, "chat cmd '%s' invalid params: %s", pName, aError);
		return 0;
	}

	if(pSelf->m_ChatCommands.contains(std::string(pName)))
	{
		// silently override previous registrations
		// not sure what is the most user friendly here
		// could also throw an error so users dont get confused
		// but that takes away the flexibility to override library code
		// could also register both callbacks and run both
		// but thats also odd i think
		//
		// maybe the best would be to error on duplication by default
		// and provider another method like `Game:register_rcon_override()`
		// to intentionally override

		// log_warn("lua", "%s was already registered as chat cmd", pName);
		int OldFunc = pSelf->m_ChatCommands.at(std::string(pName)).m_LuaCallbackRef;
		luaL_unref(L, LUA_REGISTRYINDEX, OldFunc);
	}

	// we need to move stack 3 to 0
	// because luaL_ref pops first element from the stack
	// but it is our third argument

	// so we just pop the first two args

	// push copy of the third argument (the lua callback)
	// onto the top of the stack so luaL_ref can find it
	lua_pushvalue(L, 4);
	int FuncRef = luaL_ref(L, LUA_REGISTRYINDEX);

	// TODO: according to https://en.cppreference.com/w/cpp/container/unordered_map/emplace
	//       if the same rcon command is registered twice it should do nothing
	//       because emplace ignores duplicated keys on the second insert
	//       but my testing showed that the rcon command defined lower in the lua script
	//       will actually be ran
	//       so it happens what i want to happen but as far as i understand it shouldnt
	//       so that is scary
	pSelf->m_ChatCommands.emplace(
		std::string(pName),
		CLuaRconCommand({
			.m_pName = pName,
			.m_pHelp = pHelp,
			.m_pParams = pParams,
			.m_LuaCallbackRef = FuncRef,
		}));

	const CLuaRconCommand *pCmd = &pSelf->m_ChatCommands.at(std::string(pName));
	pSelf->Game()->Controller()->Lua()->OnAddChatCmd(pCmd);

	// pop our pushed value
	lua_pop(L, 1);

	lua_pushboolean(L, true);
	return 1;
}

int CLuaPlugin::CallbackSecureRandBelow(lua_State *L)
{
	int Below = luaL_checkinteger(L, 1);
	lua_pushinteger(L, secure_rand_below(Below));
	return 1;
}

int CLuaPlugin::CallbackPluginName(lua_State *L)
{
	CLuaPlugin *pSelf = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));
	lua_pushstring(L, pSelf->Name());
	return 1;
}

int CLuaPlugin::CallbackSnapNewId(lua_State *L)
{
	CLuaPlugin *pSelf = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));
	CLuaGame *pGame = pSelf->Game();
	int SnapId = pGame->Server()->SnapNewId();
	lua_pushinteger(L, SnapId);
	pSelf->m_vSnapIds.emplace_back(SnapId);
	return 1;
}

int CLuaPlugin::CallbackSnapFreeId(lua_State *L)
{
	CLuaPlugin *pSelf = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));
	CLuaGame *pGame = pSelf->Game();
	int SnapId = luaL_checkinteger(L, 1);
	pGame->Server()->SnapFreeId(SnapId);
	pSelf->m_vSnapIds.erase(
		std::remove(pSelf->m_vSnapIds.begin(), pSelf->m_vSnapIds.end(), SnapId),
		pSelf->m_vSnapIds.end());
	return 0;
}

int CLuaPlugin::CallbackSnapNewLaser(lua_State *L)
{
	CLuaPlugin *pSelf = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));
	LUA_CHECK_STACK(L);

	CTableUnpacker Unpacker(L, -1, "laser", __FILE__, __LINE__);
	int SnapId = Unpacker.GetInt("id");
	vec2 Pos = Unpacker.GetPosition("pos");

	vec2 FromPos = Pos;
	// error the plugin if the key exist and is in wrong shape
	// but use defaults if it does not exist
	if(Unpacker.IsNestedTable("from_pos"))
		FromPos = Unpacker.GetPosition("from_pos");
	int StartTick = Unpacker.GetIntOrDefault("start_tick", 0);

	CNetObj_Laser *pObj = pSelf->SnapNewItem<CNetObj_Laser>(SnapId);
	if(!pObj)
		return false;

	// log_info("lua", "snapping laser with id=%d ...", SnapId);

	pObj->m_X = (int)Pos.x;
	pObj->m_Y = (int)Pos.y;
	pObj->m_FromX = (int)FromPos.x;
	pObj->m_FromY = (int)FromPos.y;
	pObj->m_StartTick = StartTick;
	return 0;
}

int CLuaPlugin::CallbackSnapNewPickup(lua_State *L)
{
	CLuaPlugin *pSelf = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));
	CLuaGame *pGame = pSelf->Game();
	LUA_CHECK_STACK(L);

	CTableUnpacker Unpacker(L, -1, "pickup", __FILE__, __LINE__);
	int SnapId = Unpacker.GetInt("id");
	vec2 Pos = Unpacker.GetPosition("pos");
	int Type = Unpacker.GetIntOrDefault("type", POWERUP_WEAPON);
	int SubType = Unpacker.GetIntOrDefault("sub_type", WEAPON_HAMMER);
	int SwitchNumber = Unpacker.GetIntOrDefault("switch_number", 0);
	int Flags = Unpacker.GetIntOrDefault("flags", 0);

	CSnapContext Context = CSnapContext(
		pGame->GameServer()->GetClientVersion(pSelf->m_SnappingClient),
		pGame->Server()->IsSixup(pSelf->m_SnappingClient),
		pSelf->m_SnappingClient);
	pGame->GameServer()->SnapPickup(
		Context,
		SnapId,
		Pos,
		Type,
		SubType,
		SwitchNumber,
		Flags);
	return 0;
}

int CLuaPlugin::CallbackSnapNewCharacter(lua_State *L)
{
	CLuaPlugin *pSelf = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));
	CLuaGame *pGame = pSelf->Game();

	int SnappingClient = pSelf->m_SnappingClient;
	if(pGame->Server()->IsSixup(SnappingClient))
	{
		// TODO: implement
		// TODO: we can even allow passing the 0.7 only m_TriggeredEvents
		//       as an optional argument from lua
		return 0;
	}

	CTableUnpacker Unpacker(L, 1, "character");
	int SnapId = Unpacker.GetInt("id");

	CNetObj_Character *pCharacter = pSelf->SnapNewItem<CNetObj_Character>(SnapId);
	if(!pCharacter)
		return 0;

	pCharacter->m_Tick = Unpacker.GetIntOrDefault("tick", 0);
	vec2 Pos = Unpacker.GetPosition("pos");
	pCharacter->m_X = Pos.x;
	pCharacter->m_Y = Pos.y;
	// TODO: do we need to scale velocity similar to how we do it with coordinates and positions?
	pCharacter->m_VelX = Unpacker.GetIntOrDefault("vel_x", 0);
	pCharacter->m_VelY = Unpacker.GetIntOrDefault("vel_y", 0);
	pCharacter->m_Angle = Unpacker.GetIntOrDefault("angle", 0);
	pCharacter->m_Direction = Unpacker.GetIntOrDefault("direction", 0);
	pCharacter->m_Jumped = Unpacker.GetIntOrDefault("jumped", 0);
	pCharacter->m_HookedPlayer = Unpacker.GetIntOrDefault("hooked_player", -1);
	pCharacter->m_HookState = Unpacker.GetIntOrDefault("hook_state", HOOK_IDLE);
	pCharacter->m_HookTick = Unpacker.GetIntOrDefault("hook_tick", 0);
	pCharacter->m_HookX = Unpacker.GetCoordinateOptional("hook_x").value_or(Pos.x / 32.0f);
	pCharacter->m_HookY = Unpacker.GetCoordinateOptional("hook_y").value_or(Pos.y / 32.0f);
	pCharacter->m_HookDx = Unpacker.GetIntOrDefault("hook_dx", 0);
	pCharacter->m_HookDy = Unpacker.GetIntOrDefault("hook_dy", 0);
	pCharacter->m_PlayerFlags = Unpacker.GetIntOrDefault("player_flags", 0);
	pCharacter->m_Health = Unpacker.GetIntOrDefault("health", 10);
	pCharacter->m_Armor = Unpacker.GetIntOrDefault("armor", 0);
	pCharacter->m_AmmoCount = Unpacker.GetIntOrDefault("ammo_count", -1);
	pCharacter->m_Weapon = Unpacker.GetIntOrDefault("weapon", WEAPON_GUN);
	pCharacter->m_Emote = Unpacker.GetIntOrDefault("eye_emote", EMOTE_NORMAL);
	pCharacter->m_AttackTick = Unpacker.GetIntOrDefault("attack_tick", 0);
	return 0;
}

int CLuaPlugin::CallbackSnapNewPlayerInfo(lua_State *L)
{
	CLuaPlugin *pSelf = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));
	CLuaGame *pGame = pSelf->Game();

	int SnappingClient = pSelf->m_SnappingClient;
	if(pGame->Server()->IsSixup(SnappingClient))
	{
		// TODO: implement
		return 0;
	}

	CTableUnpacker Unpacker(L, 1, "player_info");
	int SnapId = Unpacker.GetInt("id");

	CNetObj_PlayerInfo *pItem = pSelf->SnapNewItem<CNetObj_PlayerInfo>(SnapId);
	if(!pItem)
		return 0;

	pItem->m_Local = Unpacker.GetBooleanOptional("is_local").value_or(false);
	pItem->m_ClientId = Unpacker.GetInt("client_id");
	pItem->m_Team = Unpacker.GetIntOrDefault("team", TEAM_GAME);
	pItem->m_Score = Unpacker.GetIntOrDefault("score", 0);
	pItem->m_Latency = Unpacker.GetIntOrDefault("latency", 0);
	return 0;
}

int CLuaPlugin::CallbackSnapNewClientInfo(lua_State *L)
{
	CLuaPlugin *pSelf = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));
	CLuaGame *pGame = pSelf->Game();

	int SnappingClient = pSelf->m_SnappingClient;
	if(pGame->Server()->IsSixup(SnappingClient))
	{
		// TODO: implement
		return 0;
	}

	CTableUnpacker Unpacker(L, 1, "client_info");
	int SnapId = Unpacker.GetInt("id");

	CNetObj_ClientInfo *pItem = pSelf->SnapNewItem<CNetObj_ClientInfo>(SnapId);
	if(!pItem)
		return 0;

	char aName[MAX_NAME_LENGTH] = "";
	bool GotName = Unpacker.GetStringOrFalse("name", aName, sizeof(aName));

	char aClan[MAX_CLAN_LENGTH] = "";
	bool GotClan = Unpacker.GetStringOrFalse("clan", aClan, sizeof(aClan));

	StrToInts(pItem->m_aName, std::size(pItem->m_aName), GotName ? aName : "lua");
	StrToInts(pItem->m_aClan, std::size(pItem->m_aClan), GotClan ? aClan : "");

	pItem->m_Country = Unpacker.GetIntOrDefault("country", -1);

	char aSkin[MAX_SKIN_LENGTH] = "";
	bool GotSkin = Unpacker.GetStringOrFalse("skin", aSkin, sizeof(aSkin));
	StrToInts(pItem->m_aSkin, std::size(pItem->m_aSkin), GotSkin ? aSkin : "default");

	pItem->m_UseCustomColor = Unpacker.GetBooleanOptional("use_custom_color").value_or(false);
	pItem->m_ColorBody = Unpacker.GetIntOrDefault("color_body", 2);
	pItem->m_ColorFeet = Unpacker.GetIntOrDefault("color_feet", 2);
	return 0;
}

int CLuaPlugin::CallbackSnapNewProjectile(lua_State *L)
{
	CLuaPlugin *pSelf = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)));
	CLuaGame *pGame = pSelf->Game();

	LUA_CHECK_STACK(L);

	CTableUnpacker Unpacker(L, -1, "projectile", __FILE__, __LINE__);
	int SnapId = Unpacker.GetInt("id");
	vec2 Pos = Unpacker.GetPosition("pos");
	int VelX = Unpacker.GetFloatOptional("vel_x").value_or(0.0f);
	int VelY = Unpacker.GetCoordinateOptional("vel_y").value_or(0.0f);
	int Type = Unpacker.GetIntOrDefault("type", WEAPON_GRENADE);
	int StartTick = Unpacker.GetIntOrDefault("start_tick", pGame->Server()->Tick() - 3);

	int Owner = Unpacker.GetIntOrDefault("owner", -1);
	int SwitchNumber = Unpacker.GetIntOrDefault("switch_number", -1);
	int TuneZone = Unpacker.GetIntOrDefault("tune_zone", -1);

	int Flags = 0;
	std::optional<int> OptFlags = Unpacker.GetIntOptional("flags");
	if(OptFlags.has_value())
	{
		Flags = OptFlags.value();
	}
	else
	{
		lua_getfield(L, -1, "flags");
		if(!lua_isnil(L, -1))
		{
			CTableUnpacker FlagsUnpacker(L, -1, "projectile.flags", __FILE__, __LINE__);

			if(FlagsUnpacker.GetBooleanOptional("bounce_horizontal").value_or(false))
				Flags |= PROJECTILEFLAG_BOUNCE_HORIZONTAL;
			if(FlagsUnpacker.GetBooleanOptional("bounce_vertical").value_or(false))
				Flags |= PROJECTILEFLAG_BOUNCE_VERTICAL;
			if(FlagsUnpacker.GetBooleanOptional("explosive").value_or(false))
				Flags |= PROJECTILEFLAG_EXPLOSIVE;
			if(FlagsUnpacker.GetBooleanOptional("freeze").value_or(false))
				Flags |= PROJECTILEFLAG_FREEZE;
			if(FlagsUnpacker.GetBooleanOptional("normalize_vel").value_or(false))
				Flags |= PROJECTILEFLAG_NORMALIZE_VEL;
		}
		lua_pop(L, 1);
	}

	int SnappingClientVersion = pGame->GameServer()->GetClientVersion(pSelf->m_SnappingClient);

	if(SnappingClientVersion >= VERSION_DDNET_ENTITY_NETOBJS)
	{
		CNetObj_DDNetProjectile *pObj = static_cast<CNetObj_DDNetProjectile *>(
			pSelf->SnapNewItem(NETOBJTYPE_DDNETPROJECTILE,
				SnapId,
				sizeof(CNetObj_DDNetProjectile)));
		if(!pObj)
			return 0;

		pObj->m_X = round_to_int(Pos.x * 100.0f);
		pObj->m_Y = round_to_int(Pos.y * 100.0f);
		pObj->m_VelX = VelX;
		pObj->m_VelY = VelY;
		pObj->m_Type = Type;
		pObj->m_StartTick = StartTick;

		// ddnet extensions
		pObj->m_Owner = Owner;
		pObj->m_SwitchNumber = SwitchNumber;
		pObj->m_TuneZone = TuneZone;
		pObj->m_Flags = Flags;
	}
	else
	{
		CNetObj_Projectile *pObj = pSelf->SnapNewItem<CNetObj_Projectile>(SnapId);
		if(!pObj)
			return 0;

		pObj->m_X = (int)Pos.x;
		pObj->m_Y = (int)Pos.y;
		pObj->m_VelX = VelX;
		pObj->m_VelY = VelY;
		pObj->m_Type = Type;
		pObj->m_StartTick = StartTick;
	}

	return 0;
}

int CLuaPlugin::CallbackServerTick(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();
	lua_pushinteger(L, pGame->Server()->Tick());
	return 1;
}

int CLuaPlugin::CallbackServerTickSpeed(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();
	lua_pushinteger(L, pGame->Server()->TickSpeed());
	return 1;
}

int CLuaPlugin::CallbackCollisionWidth(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();
	lua_pushinteger(L, pGame->Collision()->GetWidth());
	return 1;
}

int CLuaPlugin::CallbackCollisionHeight(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();
	lua_pushinteger(L, pGame->Collision()->GetHeight());
	return 1;
}

int CLuaPlugin::CallbackCollisionGetTileIndex(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();

	// omega user friendly api
	// lua can call get_tile_index(0, 0) to get the origin tile top left
	// and          get_tile_index(1, 1) to get the one diagonal one further right and one further down
	// but can also do get_tile_index(1.2, 1.9) and get the exact same tile. Which is nice for passing character:pos() fields in there
	//
	// and we also overload it with a table so one can do get_tile_index({x=1,y=1}) or pass in the full character:pos()

	ivec2 Pos;
	LuaCheckPosOrXandY(L, 1, Pos);
	int TileIndex = pGame->Collision()->GetTile(Pos.x, Pos.y);
	lua_pushinteger(L, TileIndex);
	return 1;
}

int CLuaPlugin::CallbackCollisionGetTile(lua_State *L)
{
	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();
	ivec2 LuaPos;
	CTableUnpacker Unpacker(L, 1, "pos");
	LuaPos.x = Unpacker.GetIntOrFloat("x");
	LuaPos.y = Unpacker.GetIntOrFloat("y");
	ivec2 RealPos = LuaPos * 32;

	int TileIndex = pGame->Collision()->GetTile(RealPos.x, RealPos.y);
	int TileFlags = pGame->Collision()->GetTileFlags(
		pGame->Collision()->GetIndex(RealPos.x, RealPos.y));

	lua_newtable(L);

	lua_pushstring(L, "x");
	lua_pushinteger(L, LuaPos.x);
	lua_settable(L, -3);

	lua_pushstring(L, "y");
	lua_pushinteger(L, LuaPos.y);
	lua_settable(L, -3);

	lua_pushstring(L, "index");
	lua_pushinteger(L, TileIndex);
	lua_settable(L, -3);

	lua_pushstring(L, "flags");
	lua_pushinteger(L, TileFlags);
	lua_settable(L, -3);

	return 1;
}

int CLuaPlugin::CallbackCollisionMoveBox(lua_State *L)
{
	int NumArgs = lua_gettop(L);

	CLuaGame *pGame = static_cast<CLuaPlugin *>(lua_touserdata(L, lua_upvalueindex(1)))->Game();
	vec2 Pos = LuaCheckArgPosition(L, 1);

	// TODO: is it weird that we translate coordinates but not velocity? As in multiply by 32
	vec2 Vel = LuaCheckArgVec2(L, 2, "vel");

	vec2 Size = CCharacterCore::PhysicalSizeVec2();
	if(NumArgs >= 3)
	{
		// TODO: same question for size. If lua coordinates use 1 unit for one tile instead of 32
		//       does it then make sense to measure a box size in a 32 scaled factor?
		Size = LuaCheckArgVec2(L, 3, "size");
	}

	vec2 Elast = vec2(pGame->GameServer()->GlobalTuning()->m_GroundElasticityX,
		pGame->GameServer()->GlobalTuning()->m_GroundElasticityY);
	if(NumArgs >= 4)
	{
		Elast = LuaCheckArgVec2(L, 4, "elasticity");
	}

	// log_info("lua", "move box got pos %.2f %.2f", Pos.x, Pos.y);
	// log_info("lua", " move box got vel %.2f %.2f", Vel.x, Vel.y);
	// log_info("lua", " move box got size %.2f %.2f", Size.x, Size.y);
	// log_info("lua", " move box got elast %.2f %.2f", Elast.x, Elast.y);

	bool IsGrounded = false;
	pGame->Collision()->MoveBox(
		&Pos,
		&Vel,
		Size,
		Elast,
		&IsGrounded);

	// log_info("lua", "  after movebox pos -> %.2f %.2f", Pos.x, Pos.y);
	// log_info("lua", "  after movebox pos friendly coord -> %.2f %.2f", Pos.x / 32, Pos.y / 32);
	// log_info("lua", "  after movebox vel -> %.2f %.2f", Vel.x, Vel.y);

	LuaPushPosition(L, Pos);
	LuaPushVec2(L, Vel);
	lua_pushboolean(L, IsGrounded);
	return 3;
}

int CLuaPlugin::CallbackPlayerId(lua_State *L)
{
	CPlayer *pPlayer = LuaCheckPlayer(L, 1);
	lua_pushinteger(L, pPlayer->GetCid());
	return 1;
}

int CLuaPlugin::CallbackPlayerName(lua_State *L)
{
	CPlayer *pPlayer = LuaCheckPlayer(L, 1);
	lua_pushstring(L, pPlayer->Name());
	return 1;
}

int CLuaPlugin::CallbackPlayerSetSkin(lua_State *L)
{
	int NumArgs = lua_gettop(L);
	CPlayer *pPlayer = LuaCheckPlayer(L, 1);

	CTableUnpacker Unpacker(L, 2, "skin_info");
	std::optional<int> ColorBody = Unpacker.GetIntOptional("color_body");
	std::optional<int> ColorFeet = Unpacker.GetIntOptional("color_feet");
	std::optional<bool> UseCustomColor = Unpacker.GetBooleanOptional("use_custom_color");
	char aSkinName[512] = "";
	bool GotSkinName = Unpacker.GetStringOrFalse("name", aSkinName, sizeof(aSkinName));

	ESkinPrio Priority = ESkinPrio::HIGH;
	if(NumArgs >= 3 && lua_isinteger(L, 3))
	{
		int LuaPrio = lua_tointeger(L, 3);
		LuaPrio = std::clamp(LuaPrio, (int)ESkinPrio::USER, (int)ESkinPrio::NUM_SKINPRIOS - 1);
		Priority = (ESkinPrio)LuaPrio;
	}

	if(ColorBody.has_value())
		pPlayer->m_SkinInfoManager.SetColorBody(Priority, ColorBody.value());
	if(ColorFeet.has_value())
		pPlayer->m_SkinInfoManager.SetColorFeet(Priority, ColorFeet.value());
	if(UseCustomColor.has_value())
		pPlayer->m_SkinInfoManager.SetUseCustomColor(Priority, UseCustomColor.value());
	if(GotSkinName)
		pPlayer->m_SkinInfoManager.SetSkinName(Priority, aSkinName);

	return 0;
}

int CLuaPlugin::CallbackPlayerUnsetSkin(lua_State *L)
{
	int NumArgs = lua_gettop(L);
	CPlayer *pPlayer = LuaCheckPlayer(L, 1);

	ESkinPrio Priority = ESkinPrio::HIGH;
	if(NumArgs >= 2 && lua_isinteger(L, 2))
	{
		int LuaPrio = lua_tointeger(L, 2);
		LuaPrio = std::clamp(LuaPrio, (int)ESkinPrio::USER, (int)ESkinPrio::NUM_SKINPRIOS - 1);
		Priority = (ESkinPrio)LuaPrio;
	}

	pPlayer->m_SkinInfoManager.UnsetAll(Priority);
	return 0;
}

int CLuaPlugin::CallbackPlayerUnsetSkinColorBody(lua_State *L)
{
	int NumArgs = lua_gettop(L);
	CPlayer *pPlayer = LuaCheckPlayer(L, 1);

	ESkinPrio Priority = ESkinPrio::HIGH;
	if(NumArgs >= 2 && lua_isinteger(L, 2))
	{
		int LuaPrio = lua_tointeger(L, 2);
		LuaPrio = std::clamp(LuaPrio, (int)ESkinPrio::USER, (int)ESkinPrio::NUM_SKINPRIOS - 1);
		Priority = (ESkinPrio)LuaPrio;
	}

	pPlayer->m_SkinInfoManager.UnsetColorBody(Priority);
	return 0;
}

int CLuaPlugin::CallbackCharacterPos(lua_State *L)
{
	CCharacter *pChr = LuaCheckCharacter(L, 1);

	// technically the position is a float
	// but it does not use the floating point precision
	// https://github.com/ddnet/ddnet/issues/11890
	//
	// I decided to expose the 32 divided position as float
	// because it is the most user friendly value
	// which is also shown in the debug hud in the client
	// if you walk 10 and a half tiles from the origin of the map
	// to the right your position will be 10.5 in lua which is nice
	lua_newtable(L);
	lua_pushstring(L, "x");
	lua_pushnumber(L, pChr->GetPos().x / 32.0f);
	lua_settable(L, -3);
	lua_pushstring(L, "y");
	lua_pushnumber(L, pChr->GetPos().y / 32.0f);
	lua_settable(L, -3);
	return 1;
}

int CLuaPlugin::CallbackCharacterSetPosition(lua_State *L)
{
	CCharacter *pChr = LuaCheckCharacter(L, 1);
	ivec2 Pos;
	LuaCheckPosOrXandY(L, 2, Pos);
	vec2 FloatPos;
	FloatPos.x = Pos.x;
	FloatPos.y = Pos.y;
	pChr->SetPosition(FloatPos);
	return 0;
}

int CLuaPlugin::CallbackCharacterId(lua_State *L)
{
	CCharacter *pChr = LuaCheckCharacter(L, 1);
	lua_pushinteger(L, pChr->GetPlayer()->GetCid());
	return 1;
}

int CLuaPlugin::CallbackCharacterPlayer(lua_State *L)
{
	CLuaPlayerHandle *pCharacterHandle = LuaCheckCharacterHandle(L, 1);

	auto *pPlayerHandle = static_cast<CLuaPlayerHandle *>(lua_newuserdatauv(L, sizeof(CLuaPlayerHandle), 0));
	pPlayerHandle->Init(pCharacterHandle->m_UniqueClientId, pCharacterHandle->m_pPlugin);
	luaL_setmetatable(L, "Player");
	return 1;
}

int CLuaPlugin::CallbackCharacterDie(lua_State *L)
{
	int NumArgs = lua_gettop(L);
	CCharacter *pChr = LuaCheckCharacter(L, 1);

	// TODO: cursed ddnet++ kill code ignores the weapon
	//       and does not show it in kill feed
	//       https://github.com/DDNetPP/DDNetPP/issues/529

	int KillerId = pChr->GetPlayer()->GetCid();
	int Weapon = WEAPON_GAME;

	if(NumArgs > 1)
		KillerId = LuaCheckClientId(L, 2);
	if(NumArgs > 2)
		Weapon = luaL_checkinteger(L, 3);

	pChr->Die(KillerId, Weapon);
	return 0;
}

int CLuaPlugin::CallbackCharacterGiveWeapon(lua_State *L)
{
	int NumArgs = lua_gettop(L);
	CCharacter *pChr = LuaCheckCharacter(L, 1);

	int Weapon = luaL_checkinteger(L, 2);

	int Ammo = -1;
	if(NumArgs >= 3)
		Ammo = luaL_checkinteger(L, 3);

	pChr->GiveWeapon(Weapon, false, Ammo);
	return 0;
}

int CLuaPlugin::CallbackCharacterRemoveWeapon(lua_State *L)
{
	CCharacter *pChr = LuaCheckCharacter(L, 1);
	int Weapon = luaL_checkinteger(L, 2);
	pChr->GiveWeapon(Weapon, true);
	return 0;
}

void CLuaPlugin::OnInit()
{
	dbg_assert(IsActive(), "called inactive plugin");
	CallLuaVoidNoArgs("on_init");
}

void CLuaPlugin::OnTick()
{
	dbg_assert(IsActive(), "called inactive plugin");
	CallLuaVoidNoArgs("on_tick");
}

void CLuaPlugin::OnPlayerTick(const CPlayer *pPlayer)
{
	dbg_assert(IsActive(), "called inactive plugin");
	CallLuaVoidWithPlayer("on_player_tick", pPlayer);
}

bool CLuaPlugin::OnCharacterTile(CCharacter *pChr, int GameIndex, int FrontIndex)
{
	dbg_assert(IsActive(), "called inactive plugin");
	if(!pChr->IsAlive())
		return true;
	int ClientId = pChr->GetPlayer()->GetCid();

	const char *pFunction = "on_character_tile";
	LUA_CHECK_STACK_DETAIL(LuaState(), pFunction);
	lua_getglobal(LuaState(), "ddnetpp");
	lua_getfield(LuaState(), -1, pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is nil", pFunction);
		return false;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is not a function", pFunction);
		return false;
	}
	auto *pPlayerHandle = static_cast<CLuaPlayerHandle *>(lua_newuserdatauv(LuaState(), sizeof(CLuaPlayerHandle), 0));
	pPlayerHandle->Init(pChr->GetPlayer()->GetUniqueCid(), this);
	luaL_setmetatable(LuaState(), "Character");
	lua_pushinteger(LuaState(), GameIndex);
	lua_pushinteger(LuaState(), FrontIndex);
	if(lua_pcall(LuaState(), 3, 0, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to call %s() with error: %s", Name(), pFunction, pErrorMsg);
		SetError(pErrorMsg);
		lua_pop(LuaState(), 1);
	}
	// pop global "ddnetpp"
	lua_pop(LuaState(), 1);

	// If the plugin kills or kicks the character or player
	// we need to return true
	// to skip the other tile handle code that would follow
	// otherwise it will segfault
	if(!Game()->GameServer()->GetPlayerChar(ClientId))
		return true;
	return false;
}

bool CLuaPlugin::OnCharacterGameTileChange(CCharacter *pChr, int GameIndex)
{
	dbg_assert(IsActive(), "called inactive plugin");
	if(!pChr->IsAlive())
		return true;
	int ClientId = pChr->GetPlayer()->GetCid();

	const char *pFunction = "on_character_game_tile_change";
	LUA_CHECK_STACK_DETAIL(LuaState(), pFunction);
	lua_getglobal(LuaState(), "ddnetpp");
	lua_getfield(LuaState(), -1, pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is nil", pFunction);
		return false;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is not a function", pFunction);
		return false;
	}
	auto *pPlayerHandle = static_cast<CLuaPlayerHandle *>(lua_newuserdatauv(LuaState(), sizeof(CLuaPlayerHandle), 0));
	pPlayerHandle->Init(pChr->GetPlayer()->GetUniqueCid(), this);
	luaL_setmetatable(LuaState(), "Character");
	lua_pushinteger(LuaState(), GameIndex);
	if(lua_pcall(LuaState(), 2, 0, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to call %s() with error: %s", Name(), pFunction, pErrorMsg);
		SetError(pErrorMsg);
		lua_pop(LuaState(), 1);
	}
	// pop global "ddnetpp"
	lua_pop(LuaState(), 1);

	// If the plugin kills or kicks the character or player
	// we need to return true
	// to skip the other tile handle code that would follow
	// otherwise it will segfault
	if(!Game()->GameServer()->GetPlayerChar(ClientId))
		return true;
	return false;
}

bool CLuaPlugin::OnSkipGameTile(CCharacter *pChr, int GameIndex)
{
	dbg_assert(IsActive(), "called inactive plugin");
	if(!pChr->IsAlive())
		return false;
	int ClientId = pChr->GetPlayer()->GetCid();

	const char *pFunction = "on_skip_game_tile";
	LUA_CHECK_STACK_DETAIL(LuaState(), pFunction);
	lua_getglobal(LuaState(), "ddnetpp");
	lua_getfield(LuaState(), -1, pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is nil", pFunction);
		return false;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is not a function", pFunction);
		return false;
	}
	auto *pPlayerHandle = static_cast<CLuaPlayerHandle *>(lua_newuserdatauv(LuaState(), sizeof(CLuaPlayerHandle), 0));
	pPlayerHandle->Init(pChr->GetPlayer()->GetUniqueCid(), this);
	luaL_setmetatable(LuaState(), "Character");
	lua_pushinteger(LuaState(), GameIndex);
	if(lua_pcall(LuaState(), 2, 1, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to call %s() with error: %s", Name(), pFunction, pErrorMsg);
		SetError(pErrorMsg);
		lua_pop(LuaState(), 1);
	}

	// If the plugin kills or kicks the character or player
	// we need to return true
	// to skip the other tile handle code that would follow
	// otherwise it will segfault
	if(!Game()->GameServer()->GetPlayerChar(ClientId))
	{
		// pop global "ddnetpp"
		lua_pop(LuaState(), 1);
		return true;
	}

	// TODO: move this into a helper
	int Type = lua_type(LuaState(), -1);
	if(Type != LUA_TBOOLEAN)
	{
		lua_Debug LuaInfo;
		lua_getglobal(LuaState(), pFunction);
		lua_getinfo(LuaState(), ">Sl", &LuaInfo);

		char aError[512];
		str_format(
			aError,
			sizeof(aError),
			"%s:%d return value for '%s' should be boolean, got %s",
			LuaInfo.short_src,
			LuaInfo.linedefined,
			pFunction,
			luaL_typename(LuaState(), -1));
		log_error("lua", "%s", aError);
		SetError(aError);
		// pop error and global "ddnetpp"
		lua_pop(LuaState(), 2);
		return false;
	}

	bool Skip = lua_toboolean(LuaState(), -1);
	// pop skip and global "ddnetpp"
	lua_pop(LuaState(), 2);
	return Skip;
}

void CLuaPlugin::OnSnap(int SnappingClient)
{
	dbg_assert(IsActive(), "called inactive plugin");
	m_SnappingClient = SnappingClient;
	CallLuaVoidWithOneInt("on_snap", SnappingClient);
}

int CLuaPlugin::OnSnapGameInfoExFlags(int SnappingClient, int DDRaceFlags)
{
	dbg_assert(IsActive(), "called inactive plugin");
	return CallLuaIntWithTwoInts("on_snap_gameinfo_flags", SnappingClient, DDRaceFlags).value_or(DDRaceFlags);
}

int CLuaPlugin::OnSnapGameInfoExFlags2(int SnappingClient, int DDRaceFlags)
{
	dbg_assert(IsActive(), "called inactive plugin");
	return CallLuaIntWithTwoInts("on_snap_gameinfo_flags2", SnappingClient, DDRaceFlags).value_or(DDRaceFlags);
}

static void LuaCharacterObjToTable(lua_State *L, const CNetObj_Character *pObj)
{
	lua_newtable(L);

	lua_pushstring(L, "tick");
	lua_pushinteger(L, pObj->m_Tick);
	lua_settable(L, -3);

	lua_pushstring(L, "x");
	lua_pushnumber(L, pObj->m_X / 32.0f);
	lua_settable(L, -3);

	lua_pushstring(L, "y");
	lua_pushnumber(L, pObj->m_Y / 32.0f);
	lua_settable(L, -3);

	lua_pushstring(L, "vel_x");
	lua_pushinteger(L, pObj->m_VelX);
	lua_settable(L, -3);

	lua_pushstring(L, "vel_y");
	lua_pushinteger(L, pObj->m_VelY);
	lua_settable(L, -3);

	lua_pushstring(L, "angle");
	lua_pushinteger(L, pObj->m_Angle);
	lua_settable(L, -3);

	lua_pushstring(L, "direction");
	lua_pushinteger(L, pObj->m_Direction);
	lua_settable(L, -3);

	lua_pushstring(L, "jumped");
	lua_pushinteger(L, pObj->m_Jumped);
	lua_settable(L, -3);

	lua_pushstring(L, "hooked_player");
	lua_pushinteger(L, pObj->m_HookedPlayer);
	lua_settable(L, -3);

	lua_pushstring(L, "hook_state");
	lua_pushinteger(L, pObj->m_HookState);
	lua_settable(L, -3);

	lua_pushstring(L, "hook_tick");
	lua_pushinteger(L, pObj->m_HookTick);
	lua_settable(L, -3);

	lua_pushstring(L, "hook_x");
	lua_pushinteger(L, pObj->m_HookX);
	lua_settable(L, -3);

	lua_pushstring(L, "hook_y");
	lua_pushinteger(L, pObj->m_HookY);
	lua_settable(L, -3);

	lua_pushstring(L, "hook_dx");
	lua_pushinteger(L, pObj->m_HookDx);
	lua_settable(L, -3);

	lua_pushstring(L, "hook_dy");
	lua_pushinteger(L, pObj->m_HookDy);
	lua_settable(L, -3);

	lua_pushstring(L, "player_flags");
	lua_pushinteger(L, pObj->m_PlayerFlags);
	lua_settable(L, -3);

	lua_pushstring(L, "health");
	lua_pushinteger(L, pObj->m_Health);
	lua_settable(L, -3);

	lua_pushstring(L, "armor");
	lua_pushinteger(L, pObj->m_Armor);
	lua_settable(L, -3);

	lua_pushstring(L, "ammo_count");
	lua_pushinteger(L, pObj->m_AmmoCount);
	lua_settable(L, -3);

	lua_pushstring(L, "weapon");
	lua_pushinteger(L, pObj->m_Weapon);
	lua_settable(L, -3);

	lua_pushstring(L, "emote");
	lua_pushinteger(L, pObj->m_Emote);
	lua_settable(L, -3);

	lua_pushstring(L, "attack_tick");
	lua_pushinteger(L, pObj->m_AttackTick);
	lua_settable(L, -3);
}

// TODO: also implement a 0.7 version and call the same lua callback
//       so lua always implements ddnetpp.on_snap_character() and gets passed either 0.6 or 0.7 data
//       if lua then wants to return 0.7 or 0.6 specific values it can just check is_sixup()
void CLuaPlugin::OnSnapCharacter6(int SnappingClient, CCharacter *pChr, CNetObj_Character *pObj)
{
	dbg_assert(IsActive(), "called inactive plugin");
	if(!pChr->IsAlive())
		return;

	const char *pFunction = "on_snap_character";
	LUA_CHECK_STACK_DETAIL(LuaState(), pFunction);
	lua_getglobal(LuaState(), "ddnetpp");
	lua_getfield(LuaState(), -1, pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is nil", pFunction);
		return;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is not a function", pFunction);
		return;
	}

	lua_pushinteger(LuaState(), SnappingClient);

	auto *pPlayerHandle = static_cast<CLuaPlayerHandle *>(lua_newuserdatauv(LuaState(), sizeof(CLuaPlayerHandle), 0));
	pPlayerHandle->Init(pChr->GetPlayer()->GetUniqueCid(), this);
	luaL_setmetatable(LuaState(), "Character");

	LuaCharacterObjToTable(LuaState(), pObj);

	if(lua_pcall(LuaState(), 3, 1, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to call %s() with error: %s", Name(), pFunction, pErrorMsg);
		SetError(pErrorMsg);
		lua_pop(LuaState(), 2);
		return;
	}

	if(!LuaSnapCharacterReturnValueOrError(pFunction, -1, pObj))
	{
		// pop return val and global "ddnetpp"
		lua_pop(LuaState(), 2);
	}

	// pop return val and global "ddnetpp"
	lua_pop(LuaState(), 2);
}

bool CLuaPlugin::OnChatMessage(int ClientId, CNetMsg_Cl_Say *pMsg, int &Team)
{
	dbg_assert(IsActive(), "called inactive plugin");

	const char *pFunction = "on_chat";
	LUA_CHECK_STACK_DETAIL(LuaState(), pFunction);
	lua_getglobal(LuaState(), "ddnetpp");
	lua_getfield(LuaState(), -1, pFunction);

	// lua_getglobal(LuaState(), pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal and getfield because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is nil", pFunction);
		return false;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal and getfield because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is not a function", pFunction);
		return false;
	}

	lua_pushinteger(LuaState(), ClientId);

	lua_newtable(LuaState());
	lua_pushstring(LuaState(), "message");
	lua_pushstring(LuaState(), pMsg->m_pMessage);
	lua_settable(LuaState(), -3);
	lua_pushstring(LuaState(), "team");
	lua_pushinteger(LuaState(), Team);
	lua_settable(LuaState(), -3);

	// // I was thinking about passing the original team the client sent
	// // as an argument too
	// // so the plugin gets msg.team and raw_team args
	// // but that seems confusing
	// // so lets only send the ddnet processed team
	// // and use it as read and write
	// lua_pushinteger(LuaState(), pMsg->m_Team);

	if(lua_pcall(LuaState(), 2, 1, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to call %s() with error: %s", Name(), pFunction, pErrorMsg);
		SetError(pErrorMsg);
		lua_pop(LuaState(), 1);
	}

	int Type = lua_type(LuaState(), -1);
	if(Type != LUA_TTABLE)
	{
		lua_Debug LuaInfo;
		lua_getglobal(LuaState(), pFunction);
		lua_getinfo(LuaState(), ">Sl", &LuaInfo);

		char aError[512];
		str_format(
			aError,
			sizeof(aError),
			"%s:%d return value for '%s' should be a table, got %s",
			LuaInfo.short_src,
			LuaInfo.linedefined,
			pFunction,
			luaL_typename(LuaState(), -1));
		log_error("lua", "%s", aError);
		SetError(aError);
		// pop dbg info and global "ddnetpp"
		lua_pop(LuaState(), 2);
		return false;
	}

	lua_getfield(LuaState(), -1, "message");
	if(!lua_isstring(LuaState(), -1))
	{
		lua_Debug LuaInfo;
		lua_getglobal(LuaState(), pFunction);
		lua_getinfo(LuaState(), ">Sl", &LuaInfo);

		char aError[512];
		str_format(
			aError,
			sizeof(aError),
			"%s:%d returned table in '%s' is missing key '%s'",
			LuaInfo.short_src,
			LuaInfo.linedefined,
			pFunction,
			"message");
		log_error("lua", "%s", aError);
		SetError(aError);
		// pop dbg info and global "ddnetpp" and field
		lua_pop(LuaState(), 3);
		return false;
	}
	const char *pMessage = lua_tostring(LuaState(), -1);
	str_copy(m_TmpStorage.m_aClSayMessage, pMessage);
	pMsg->m_pMessage = m_TmpStorage.m_aClSayMessage;
	lua_pop(LuaState(), 1); // message

	lua_getfield(LuaState(), -1, "team");
	if(!lua_isinteger(LuaState(), -1))
	{
		lua_Debug LuaInfo;
		lua_getglobal(LuaState(), pFunction);
		lua_getinfo(LuaState(), ">Sl", &LuaInfo);

		char aError[512];
		str_format(
			aError,
			sizeof(aError),
			"%s:%d returned table in '%s' is missing key '%s'",
			LuaInfo.short_src,
			LuaInfo.linedefined,
			pFunction,
			"team");
		log_error("lua", "%s", aError);
		SetError(aError);
		// pop dbg info and global "ddnetpp" and field
		lua_pop(LuaState(), 3);
		return false;
	}
	Team = lua_tointeger(LuaState(), -1);
	lua_pop(LuaState(), 1); // team

	// pop global "ddnetpp" and the returned table???
	lua_pop(LuaState(), 2);
	return false;
}

void CLuaPlugin::OnPlayerConnect(int ClientId)
{
	dbg_assert(IsActive(), "called inactive plugin");
	CallLuaVoidWithOneInt("on_player_connect", ClientId);
}

void CLuaPlugin::OnPlayerDisconnect(int ClientId)
{
	dbg_assert(IsActive(), "called inactive plugin");
	CallLuaVoidWithOneInt("on_player_disconnect", ClientId);
}

std::optional<vec2> CLuaPlugin::OnPickSpawnPos(CPlayer *pPlayer)
{
	dbg_assert(IsActive(), "called inactive plugin");
	const char *pFunction = "on_pick_spawn_pos";
	LUA_CHECK_STACK_DETAIL(LuaState(), pFunction);
	lua_getglobal(LuaState(), "ddnetpp");
	lua_getfield(LuaState(), -1, pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is nil", pFunction);
		return std::nullopt;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is not a function", pFunction);
		return std::nullopt;
	}
	auto *pPlayerHandle = static_cast<CLuaPlayerHandle *>(lua_newuserdatauv(LuaState(), sizeof(CLuaPlayerHandle), 0));
	pPlayerHandle->Init(pPlayer->GetUniqueCid(), this);
	luaL_setmetatable(LuaState(), "Player");
	if(lua_pcall(LuaState(), 1, 1, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to call %s() with error: %s", Name(), pFunction, pErrorMsg);
		SetError(pErrorMsg);
		// pop error and global "ddnetpp"
		lua_pop(LuaState(), 2);
		return std::nullopt;
	}

	// the return value can be "nil" or a table
	// any other type will crash the plugin to avoid confusion
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop pos return value and global "ddnetpp"
		lua_pop(LuaState(), 2);
		return std::nullopt;
	}

	vec2 Pos;
	if(!LuaGetPositionReturnValueOrError(pFunction, -1, &Pos))
	{
		// pop pos return value and global "ddnetpp"
		lua_pop(LuaState(), 2);
		return std::nullopt;
	}

	// pop pos return value and global "ddnetpp"
	lua_pop(LuaState(), 2);
	return Pos;
}

bool CLuaPlugin::LuaGetPositionReturnValueOrError(const char *pFunction, int Index, vec2 *pOutPos)
{
	// table unpacker below also throws an error
	// but this error message is nicer because it mentions
	// the function name
	if(!LuaReturnValueIsTableOrError(pFunction, -1, "pos"))
		return false;

	CTableUnpacker Unpacker(LuaState(), -1, "pos");
	const char *apKeys[] = {"x", "y"};
	// TODO: this is some weird code, i tried to be smart and write dry code but it became
	//       kinda complicated and slow
	for(const char *pKey : apKeys)
	{
		std::optional<int> Coord = Unpacker.GetCoordinateOptional(pKey);
		if(!Coord.has_value())
		{
			char aError[512] = "";
			str_format(
				aError,
				sizeof(aError),
				"returned table from '%s()' is missing the key '%s'",
				pFunction,
				pKey);
			log_error("lua", "plugin '%s' error: %s", Name(), aError);
			SetError(aError);
			return false;
		}
		if(pOutPos)
		{
			if(!str_comp(pKey, "x"))
				pOutPos->x = Coord.value();
			if(!str_comp(pKey, "y"))
				pOutPos->y = Coord.value();
		}
	}
	return true;
}

std::optional<int> CLuaPlugin::LuaGetReturnValueIntFieldOrError(const char *pFunction, CTableUnpacker *pUnpacker, const char *pKey)
{
	auto Value = pUnpacker->GetIntOptional(pKey);
	if(!Value.has_value())
	{
		char aError[512] = "";
		str_format(
			aError,
			sizeof(aError),
			"returned table '%s' from '%s()' is missing the key '%s'",
			pUnpacker->TableName(),
			pFunction,
			pKey);
		log_error("lua", "plugin '%s' error: %s", Name(), aError);
		SetError(aError);
		return std::nullopt;
	}
	return Value;
}

bool CLuaPlugin::LuaSnapCharacterReturnValueOrError(const char *pFunction, int Index, CNetObj_Character *pObj)
{
	// table unpacker below also throws an error
	// but this error message is nicer because it mentions
	// the function name
	if(!LuaReturnValueIsTableOrError(pFunction, -1, "snap_item"))
		return false;
	CTableUnpacker Unpacker(LuaState(), -1, "snap_item");

	// // this is how a required field could look like but I decided to make all optional lol
	// if(!(Value = LuaGetReturnValueIntFieldOrError(pFunction, &Unpacker, "tick")).has_value())
	// 	return false;
	// pObj->m_Tick = Value.value();

	// intentionally do not allow to change the snap item id from lua
	// its tricky to do on the C++ side and it should never be a use case
	// would be super messy

	pObj->m_Tick = Unpacker.GetIntOrDefault("tick", pObj->m_Tick);
	pObj->m_X = Unpacker.GetCoordinateOptional("x").value_or(pObj->m_X);
	pObj->m_Y = Unpacker.GetCoordinateOptional("y").value_or(pObj->m_Y);
	pObj->m_VelX = Unpacker.GetIntOrDefault("vel_x", pObj->m_VelX);
	pObj->m_VelY = Unpacker.GetIntOrDefault("vel_y", pObj->m_VelY);
	pObj->m_Angle = Unpacker.GetIntOrDefault("angle", pObj->m_Angle);
	pObj->m_Direction = Unpacker.GetIntOrDefault("direction", pObj->m_Direction);
	pObj->m_Jumped = Unpacker.GetIntOrDefault("jumped", pObj->m_Jumped);
	pObj->m_HookedPlayer = Unpacker.GetIntOrDefault("hooked_player", pObj->m_HookedPlayer);
	pObj->m_HookState = Unpacker.GetIntOrDefault("hook_state", pObj->m_HookState);
	pObj->m_HookTick = Unpacker.GetIntOrDefault("hook_tick", pObj->m_HookTick);
	pObj->m_HookX = Unpacker.GetIntOrDefault("hook_x", pObj->m_HookX);
	pObj->m_HookY = Unpacker.GetIntOrDefault("hook_y", pObj->m_HookY);
	pObj->m_HookDx = Unpacker.GetIntOrDefault("hook_dx", pObj->m_HookDx);
	pObj->m_HookDy = Unpacker.GetIntOrDefault("hook_dy", pObj->m_HookDy);
	pObj->m_PlayerFlags = Unpacker.GetIntOrDefault("player_flags", pObj->m_PlayerFlags);
	pObj->m_Health = Unpacker.GetIntOrDefault("health", pObj->m_Health);
	pObj->m_Armor = Unpacker.GetIntOrDefault("armor", pObj->m_Armor);
	pObj->m_AmmoCount = Unpacker.GetIntOrDefault("ammo_count", pObj->m_AmmoCount);
	pObj->m_Weapon = Unpacker.GetIntOrDefault("weapon", pObj->m_Weapon);
	pObj->m_Emote = Unpacker.GetIntOrDefault("emote", pObj->m_Emote);
	pObj->m_AttackTick = Unpacker.GetIntOrDefault("attack_tick", pObj->m_AttackTick);

	return true;
}

bool CLuaPlugin::LuaReturnValueIsTableOrError(const char *pFunction, int Index, const char *pExpectedTableName)
{
	LUA_CHECK_STACK_DETAIL(LuaState(), pFunction);
	if(lua_type(LuaState(), Index) == LUA_TTABLE)
		return true;

	lua_Debug LuaInfo;
	lua_getglobal(LuaState(), pFunction);
	lua_getinfo(LuaState(), ">Sl", &LuaInfo);

	char aError[512];
	str_format(
		aError,
		sizeof(aError),
		"%s:%d return value for '%s' should be the '%s' table, got %s",
		LuaInfo.short_src,
		LuaInfo.linedefined,
		pFunction,
		pExpectedTableName,
		luaL_typename(LuaState(), Index));
	log_error("lua", "%s", aError);
	SetError(aError);

	// TODO: the stack checker doesnt like it
	//       but i dont know why
	//       getglobal pushes onto the stack for sure
	//       and getinfo shouldnt pop it as far as i understand

	// // pop error
	// lua_pop(LuaState(), 1);
	return false;
}

// https://github.com/DDNetPP/DDNetPP/issues/512
static bool PushRconArgs(lua_State *L, const CLuaRconCommand *pCmd, const char *pArguments, char *pError, int ErrorLen)
{
	if(pError && ErrorLen)
		pError[0] = '\0';
	if(pCmd->m_aParams[0] == '\0')
	{
		lua_pushstring(L, pArguments);
		return true;
	}

	const char *apArgs[512] = {};
	char aError[512] = "";
	size_t NumArgs = 0;
	char aArgBuf[2048];
	str_copy(aArgBuf, pArguments);
	bool WordSplitOk = SplitConsoleArgsWithParams(
		apArgs,
		(sizeof(apArgs) / sizeof(apArgs[0])),
		&NumArgs,
		aArgBuf,
		pCmd->m_vParsedParams,
		aError,
		sizeof(aError));
	if(!WordSplitOk)
	{
		if(pError)
			str_format(pError, ErrorLen, "failed to parse arguments: %s", aError);
		return false;
	}

	if(NumArgs > pCmd->m_vParsedParams.size())
	{
		if(pError)
		{
			str_format(
				pError,
				ErrorLen,
				"rcon command '%s' too many arguments %" PRIzu " out of %" PRIzu " (expected: %s)",
				pCmd->m_aName,
				NumArgs,
				pCmd->m_vParsedParams.size(),
				pCmd->m_aParams);
		}
		return false;
	}
	if(NumArgs < pCmd->m_vParsedParams.size())
	{
		const CLuaRconCommand::CParam *pParam = &pCmd->m_vParsedParams[NumArgs];
		if(!pParam->m_Optional)
		{
			if(pError)
			{
				str_format(
					pError,
					ErrorLen,
					"rcon command '%s' missing argument"
					"%s%s%s"
					"at position %" PRIzu " (expected: %s)",
					pCmd->m_aName,
					pParam->m_aName[0] ? " '" : " ",
					pParam->m_aName[0] ? pParam->m_aName : "",
					pParam->m_aName[0] ? "' " : "",
					NumArgs + 1,
					pCmd->m_aParams);
			}
			return false;
		}
	}

	lua_newtable(L);
	size_t NumParams = 0;
	for(const CLuaRconCommand::CParam &Param : pCmd->m_vParsedParams)
	{
		if(NumParams >= NumArgs)
			break;

		// key
		if(Param.m_aName[0])
		{
			lua_pushstring(L, Param.m_aName);
		}
		else
		{
			// lua ah 1 based array index
			// for unnamed arguments
			lua_pushinteger(L, NumParams + 1);
		}
		// log_info("lua", "got param %" PRIzu " '%s' with value '%s'", NumParams, Param.m_aName, apArgs[NumParams]);

		int Value;

		// value
		switch(Param.m_Type)
		{
		case CLuaRconCommand::CParam::EType::INT:
			if(!str_toint(apArgs[NumParams], &Value))
			{
				if(pError)
				{
					str_format(
						pError,
						ErrorLen,
						"argument '%s' is not a valid number",
						apArgs[NumParams]);
				}

				// pop the table and the key
				// because we abort early
				lua_pop(L, 2);
				return false;
			}
			lua_pushinteger(L, Value);
			break;
		case CLuaRconCommand::CParam::EType::STRING:
			lua_pushstring(L, apArgs[NumParams]);
			break;
		case CLuaRconCommand::CParam::EType::REST:
			lua_pushstring(L, apArgs[NumParams]);
			break;
		case CLuaRconCommand::CParam::EType::INVALID:
			dbg_assert_failed("got invalid param");
			break;
		}
		lua_settable(L, -3);
		NumParams++;
	}

	return true;
}

bool CLuaPlugin::OnRconCommand(int ClientId, const char *pCommand, const char *pArguments)
{
	dbg_assert(IsActive(), "called inactive plugin");
	LUA_CHECK_STACK(LuaState());

	if(!m_RconCommands.contains(pCommand))
	{
		// log_info("lua", "plugin '%s' does not know rcon command '%s'", Name(), pCommand);
		return false;
	}
	// log_info("lua", "plugin '%s' does know rcon command '%s'", Name(), pCommand);

	const CLuaRconCommand *pCmd = &m_RconCommands.at(pCommand);

	char aError[512] = "";
	if(pCmd->m_LuaCallbackRef == LUA_REFNIL)
	{
		str_format(aError, sizeof(aError), "invalid lua callback for rcon command '%s'", pCommand);
		log_error("lua", "%s", aError);
		SetError(aError);
		return false;
	}

	lua_rawgeti(LuaState(), LUA_REGISTRYINDEX, pCmd->m_LuaCallbackRef);

	// first callback arg the integer "client_id"
	lua_pushinteger(LuaState(), ClientId);

	// second callback arg the table or string "args"
	// depends if the user provided params or not
	if(!PushRconArgs(LuaState(), pCmd, pArguments, aError, sizeof(aError)))
	{
		// pop func and client id because we do not call the function
		lua_pop(LuaState(), 2);

		// this does get shown to the user actually but is not ideal
		// "chatresp" might be better or sending rcon line to the client id directly
		// otherwise this does not work with econ, chat or threads
		log_error("lua", "%s", aError);
		return true;
	}

	if(lua_pcall(LuaState(), 2, 0, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to run callback for rcon command '%s' with error: %s", Name(), pCommand, pErrorMsg);
		SetError(pErrorMsg);
		// pop error
		lua_pop(LuaState(), 1);
	}
	return true;
}

bool CLuaPlugin::OnChatCommand(int ClientId, const char *pCommand, const char *pArguments)
{
	dbg_assert(IsActive(), "called inactive plugin");
	LUA_CHECK_STACK(LuaState());

	if(!m_ChatCommands.contains(pCommand))
	{
		// log_info("lua", "plugin '%s' does not know chat command '%s'", Name(), pCommand);
		return false;
	}
	// log_info("lua", "plugin '%s' does know chat command '%s'", Name(), pCommand);

	const CLuaRconCommand *pCmd = &m_ChatCommands.at(pCommand);

	char aError[512] = "";
	if(pCmd->m_LuaCallbackRef == LUA_REFNIL)
	{
		str_format(aError, sizeof(aError), "invalid lua callback for rcon command '%s'", pCommand);
		log_error("lua", "%s", aError);
		SetError(aError);
		return false;
	}

	lua_rawgeti(LuaState(), LUA_REGISTRYINDEX, pCmd->m_LuaCallbackRef);

	// first callback arg the integer "client_id"
	lua_pushinteger(LuaState(), ClientId);

	// second callback arg the table or string "args"
	// depends if the user provided params or not
	if(!PushRconArgs(LuaState(), pCmd, pArguments, aError, sizeof(aError)))
	{
		// pop func and client id because we do not call the function
		lua_pop(LuaState(), 2);

		// this does get shown to the user actually but is not ideal
		// "chatresp" might be better or sending rcon line to the client id directly
		// otherwise this does not work with econ, chat or threads
		//
		// UPDATE: i just checked with "chatresp" and it did not work for chat commands
		//         not sure what broke the log scope here we should still be on the main thread
		//         directly in the chat command scope.
		//         but somehow it gets logged to server scope
		//         anyways will just to sendchattarget
		log_error("lua", "%s", aError);
		Game()->GameServer()->SendChatTarget(ClientId, aError);
		return true;
	}

	if(lua_pcall(LuaState(), 2, 0, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to run callback for chat command '%s' with error: %s", Name(), pCommand, pErrorMsg);
		SetError(pErrorMsg);
		// pop error
		lua_pop(LuaState(), 1);
	}
	return true;
}

void CLuaPlugin::OnSetAuthed(int ClientId, int Level)
{
	dbg_assert(IsActive(), "called inactive plugin");

	// TODO: rethink this api, actually I do not want to expose level to lua
	//       it should already implement the planned ddnet role api
	//       and have callbacks like "on_rcon_authed(clientid, rolename)"
	//                          and  "on_rcon_logout(clientid)"

	CallLuaVoidWithTwoInts("on_rcon_authed", ClientId, Level);
}

bool CLuaPlugin::OnFireWeapon(int ClientId, int Weapon, vec2 Direction, vec2 MouseTarget, vec2 ProjStartPos)
{
	dbg_assert(IsActive(), "called inactive plugin");
	LUA_CHECK_STACK(LuaState());

	const char *pFunction = "on_fire_weapon";
	lua_getglobal(LuaState(), "ddnetpp");

	lua_getfield(LuaState(), -1, pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is nil", pFunction);
		return false;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal and function because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is not a function", pFunction);
		return false;
	}
	lua_pushinteger(LuaState(), ClientId);
	lua_pushinteger(LuaState(), Weapon);
	LuaPushVec2(LuaState(), Direction);
	LuaPushVec2(LuaState(), MouseTarget);
	LuaPushVec2(LuaState(), ServerPosToLua(ProjStartPos));
	if(lua_pcall(LuaState(), 5, 1, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "plugin '%s' failed to call %s() with error: %s", Name(), pFunction, pErrorMsg);
		SetError(pErrorMsg);
		lua_pop(LuaState(), 1);
	}

	// optional return of false to drop block the fire
	if(lua_isboolean(LuaState(), -1))
	{
		bool DoFire = lua_toboolean(LuaState(), -1);
		if(DoFire == false)
		{
			// global "ddnetpp" and return value
			lua_pop(LuaState(), 2);
			return true;
		}
	}

	// global "ddnetpp" and return value
	lua_pop(LuaState(), 2);
	return false;
}

// WARNING: if this method uses a logger the rcon command "reload plugins"
//          will receive a rcon line message which calls this method again
//          so there is a high risk of recursion
//          https://github.com/DDNetPP/DDNetPP/issues/524
bool CLuaPlugin::OnServerMessage(int ClientId, const void *pData, int Size, int Flags)
{
	dbg_assert(IsActive(), "called inactive plugin");
	LUA_CHECK_STACK(LuaState());

	const char *pFunction = "on_server_message";
	lua_getglobal(LuaState(), "ddnetpp");
	lua_getfield(LuaState(), -1, pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal and field because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is nil", pFunction);
		return false;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal and field because we dont run pcall
		lua_pop(LuaState(), 2);
		// log_error("lua", "%s is not a function", pFunction);
		return false;
	}

	lua_pushinteger(LuaState(), ClientId);
	lua_pushlstring(LuaState(), (const char *)pData, Size);
	lua_pushinteger(LuaState(), Flags);
	if(lua_pcall(LuaState(), 3, 1, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		// WARNING: we can not use any kind of logger here it would cause recursion
		// log_error("lua", "plugin '%s' failed to call %s() with error: %s", Name(), pFunction, pErrorMsg);
		SetError(pErrorMsg);
		// pop error and global "ddnetpp"
		lua_pop(LuaState(), 2);
		return false;
	}

	// TODO: move this into a helper
	int Type = lua_type(LuaState(), -1);
	if(Type != LUA_TBOOLEAN)
	{
		lua_Debug LuaInfo;
		lua_getglobal(LuaState(), pFunction);
		lua_getinfo(LuaState(), ">Sl", &LuaInfo);

		char aError[512];
		str_format(
			aError,
			sizeof(aError),
			"%s:%d return value for '%s' should be boolean, got %s",
			LuaInfo.short_src,
			LuaInfo.linedefined,
			pFunction,
			luaL_typename(LuaState(), -1));
		// WARNING: we can not use any kind of logger here it would cause recursion
		// log_error("lua", "%s", aError);
		SetError(aError);
		// pop error and global "ddnetpp"
		lua_pop(LuaState(), 2);
		return false;
	}

	bool Res = lua_toboolean(LuaState(), -1);
	// pop result and global "ddnetpp"
	lua_pop(LuaState(), 2);
	return Res;
}

bool CLuaPlugin::CallPlugin(const char *pFunction, lua_State *pCaller)
{
	dbg_assert(IsActive(), "called inactive plugin");
	LUA_CHECK_STACK(LuaState());

	lua_getglobal(LuaState(), pFunction);
	if(lua_isnoneornil(LuaState(), -1))
	{
		// pop getglobal because we dont run pcall
		lua_pop(LuaState(), 1);
		// log_error("lua", "%s is nil", pFunction);
		return false;
	}
	if(!lua_isfunction(LuaState(), -1))
	{
		// pop getglobal because we dont run pcall
		lua_pop(LuaState(), 1);
		// log_error("lua", "%s is not a function", pFunction);
		return false;
	}

	int NumArgs = PassOnArgs(pFunction, pCaller, 3);
	if(lua_pcall(LuaState(), NumArgs, 1, 0) != LUA_OK)
	{
		const char *pErrorMsg = lua_tostring(LuaState(), -1);
		log_error("lua", "%s", pErrorMsg);
		SetError(pErrorMsg);
		// pop error
		lua_pop(LuaState(), 1);
	}

	// return true as "ok" or "found" as first value
	// to signal the caller that a plugin got this function
	// and the second argument will be its return value
	lua_pushboolean(pCaller, true);

	if(lua_isinteger(LuaState(), -1))
	{
		lua_pushinteger(pCaller, lua_tointeger(LuaState(), -1));
	}
	else if(lua_isnumber(LuaState(), -1))
	{
		lua_pushnumber(pCaller, lua_tonumber(LuaState(), -1));
	}
	else if(lua_isboolean(LuaState(), -1))
	{
		lua_pushboolean(pCaller, lua_toboolean(LuaState(), -1));
	}
	else if(lua_isnil(LuaState(), -1))
	{
		lua_pushnil(pCaller);
	}
	else if(lua_isstring(LuaState(), -1))
	{
		lua_pushstring(pCaller, lua_tostring(LuaState(), -1));
	}
	else if(lua_istable(LuaState(), -1))
	{
		CopyReturnedTable(pFunction, pCaller, 0);
	}
	else
	{
		log_warn("lua", "plugin '%s' returned unsupported type from function %s()", Name(), pFunction);
		lua_pushnil(pCaller);
	}

	return true;
}

bool CLuaPlugin::IsRconCmdKnown(const char *pCommand)
{
	dbg_assert(IsActive(), "called inactive plugin");

	if(!m_RconCommands.contains(pCommand))
	{
		// log_info("lua", "plugin '%s' does not know rcon command '%s'", Name(), pCommand);
		return false;
	}
	// log_info("lua", "plugin '%s' does know rcon command '%s'", Name(), pCommand);

	int FuncRef = m_RconCommands.at(pCommand).m_LuaCallbackRef;
	if(FuncRef == LUA_REFNIL)
		return false;
	return true;
}

bool CLuaPlugin::IsChatCmdKnown(const char *pCommand)
{
	dbg_assert(IsActive(), "called inactive plugin");

	if(!m_ChatCommands.contains(pCommand))
	{
		// log_info("lua", "plugin '%s' does not know chat command '%s'", Name(), pCommand);
		return false;
	}
	// log_info("lua", "plugin '%s' does know chat command '%s'", Name(), pCommand);

	int FuncRef = m_ChatCommands.at(pCommand).m_LuaCallbackRef;
	if(FuncRef == LUA_REFNIL)
		return false;
	return true;
}

bool CLuaPlugin::CopyReturnedTable(const char *pFunction, lua_State *pCaller, int Depth)
{
	// not sure how safe it is to keep this assert here xd
	// but it for sure helps debugging
	dbg_assert(lua_istable(LuaState(), -1), "copy table got not table");

	// TODO: this entire thing is a huge mess already and far from complete wtf
	//       also it looks horribly slow
	//       there has to be a better way to do this

	size_t TableLen = lua_rawlen(LuaState(), -1);
	// log_info("lua", "%*sgot table with %zu keys (depth=%d)", Depth, "", TableLen, Depth);

	// random af idk what im doing
	lua_checkstack(pCaller, TableLen * 4);

	// TODO: use the faster table creator because we know the size
	lua_newtable(pCaller);

	// Push another reference to the table on top of the stack (so we know
	// where it is, and this function can work for negative, positive and
	// pseudo indices
	lua_pushvalue(LuaState(), -1);
	// stack now contains: -1 => table
	lua_pushnil(LuaState());
	// stack now contains: -1 => nil; -2 => table
	while(lua_next(LuaState(), -2))
	{
		// stack now contains: -1 => value; -2 => key; -3 => table
		// copy the key so that lua_tostring does not modify the original
		lua_pushvalue(LuaState(), -2);
		// stack now contains: -1 => key; -2 => value; -3 => key; -4 => table

		// key scope for organization
		{
			if(lua_isinteger(LuaState(), -1))
			{
				lua_pushinteger(pCaller, lua_tointeger(LuaState(), -1));
			}
			else if(lua_isstring(LuaState(), -1))
			{
				lua_pushstring(pCaller, lua_tostring(LuaState(), -1));
			}
			else
			{
				// crashing the server if the plugin is doing weird stuff is bad
				// lua supports weird values like functions as keys

				// TODO: try to hit this assert with a crazy table return
				//       and then test if there is a way to not crash the server with an assert
				//       and properly disable the plugin without breaking any lua state

				dbg_assert_failed("plugin '%s' returned unsupported table key from function %s()", Name(), pFunction);
			}

			// pop key so value is on the top of the stack for table
			// deep copy recursion
			lua_pop(LuaState(), 1);
		}

		// i feel like this is not the smartest way of doing a deep copy
		// we are listing all types twice
		// i feel like CopyReturnedTable() could be refactored
		// into CopyAnyValue()
		if(lua_isinteger(LuaState(), -1))
		{
			lua_pushinteger(pCaller, lua_tointeger(LuaState(), -1));
			lua_settable(pCaller, -3);
		}
		else if(lua_isnumber(LuaState(), -1))
		{
			lua_pushnumber(pCaller, lua_tonumber(LuaState(), -1));
			lua_settable(pCaller, -3);
		}
		else if(lua_istable(LuaState(), -1))
		{
			// log_info("lua", "%*sgot nested table", Depth, "");
			CopyReturnedTable(pFunction, pCaller, ++Depth);
			lua_settable(pCaller, -3);

			// pop table
			lua_pop(LuaState(), 1);
		}
		else if(lua_isstring(LuaState(), -1))
		{
			lua_pushstring(pCaller, lua_tostring(LuaState(), -1));
			lua_settable(pCaller, -3);
		}
		else
		{
			SetError("unsupported table value type returned");
			log_error("lua", "plugin '%s' returned unsupported table value from function %s()", Name(), pFunction);

			// fallback nil value for caller to leave the callers lua stack in good state
			lua_pushnil(pCaller);
			lua_settable(pCaller, -3);
		}

		// pop something, nobody knows what
		lua_pop(LuaState(), 1);

		// // pop value + copy of key, leaving original key
		// lua_pop(LuaState(), 2);
		// // stack now contains: -1 => key; -2 => table
	}
	// stack now contains: -1 => table (when lua_next returns 0 it pops the key
	// but does not push anything.)
	// Pop table
	if(Depth == 0)
		lua_pop(LuaState(), 1);
	// Stack is now the same as it was on entry to this function

	return true;
}

int CLuaPlugin::PassOnArgs(const char *pFunction, lua_State *pCaller, int StackOffset)
{
	int NumArgs = 0;
	for(int ArgStack = StackOffset; !lua_isnone(pCaller, ArgStack); ArgStack++)
	{
		if(lua_isinteger(pCaller, ArgStack))
		{
			NumArgs++;
			lua_pushinteger(LuaState(), lua_tointeger(pCaller, ArgStack));
		}
		else if(lua_isstring(pCaller, ArgStack))
		{
			NumArgs++;
			lua_pushstring(LuaState(), lua_tostring(pCaller, ArgStack));
		}
		else
		{
			log_warn("lua", "plugin '%s' in function %s() was called with unsupported arg", Name(), pFunction);
			break;
		}
	}
	return NumArgs;
}

void CLuaPlugin::SetError(const char *pErrorMsg)
{
	dbg_assert(pErrorMsg, "lua plugin error is NULL");
	dbg_assert(pErrorMsg[0], "lua plugin error is empty");
	str_copy(m_aErrorMsg, pErrorMsg);
}

#endif
