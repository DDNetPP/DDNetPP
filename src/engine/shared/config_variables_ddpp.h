/*************************************
*                                    *
*             DDNet++                *
*                                    *
**************************************/

#ifndef ENGINE_SHARED_CONFIG_VARIABLES_DDPP_H
#define ENGINE_SHARED_CONFIG_VARIABLES_DDPP_H
#undef ENGINE_SHARED_CONFIG_VARIABLES_DDPP_H // this file will be included several times

// refine macros only to please my IDE should be defined already
#ifndef MACRO_CONFIG_INT
#define MACRO_CONFIG_INT(Name,ScriptName,Def,Min,Max,Save,Desc) int m_##Name;
#endif
#ifndef MACRO_CONFIG_STR
#define MACRO_CONFIG_STR(Name,ScriptName,Len,Def,Save,Desc) char m_##Name[Len]; // Flawfinder: ignore
#endif

MACRO_CONFIG_INT(SvVoteDelayAll, sv_vote_delay_all, 600, 0, 9999, CFGFLAG_SERVER, "The time in seconds between any vote (ddpp total afecting all players)")
MACRO_CONFIG_STR(SvClientSuggestionSupported, sv_client_suggestion_supported, 128, "Update to 11.4.3 or higher for full support on DDNet++ servers", CFGFLAG_SERVER, "Broadcast to display to players that their client isnt fully supported")

MACRO_CONFIG_INT(SvJailState, sv_jailstate, 0, 0, 2, CFGFLAG_SERVER, "use 1 to actiavte jail")
MACRO_CONFIG_INT(SvBankState, sv_bankstate, 0, 0, 1, CFGFLAG_SERVER, "use 1 to actiavte bank")

MACRO_CONFIG_INT(SvAllowMinigame, sv_allow_minigame, 1, 0, 2, CFGFLAG_SERVER, "allow users to play minigames")
MACRO_CONFIG_STR(SvMinigameDefaultSkin, sv_minigame_default_skin, 12, "#", CFGFLAG_SERVER, "Change default skin in minigames")

MACRO_CONFIG_INT(SvMaxShopMessages, sv_max_shop_messages, 3, -1, 1000, CFGFLAG_SERVER | CFGFLAG_GAME, "How many times the shopbot greets per player. (-1=infinite 0=off)")

MACRO_CONFIG_INT(SvDummyMapOffsetX, sv_dummy_map_offset_x, 0, -99999, 99999, CFGFLAG_SERVER | CFGFLAG_GAME, "move the dummyhardcoded moves x tiles (only for supported modes)")
MACRO_CONFIG_INT(SvDummyMapOffsetY, sv_dummy_map_offset_y, 0, -99999, 99999, CFGFLAG_SERVER | CFGFLAG_GAME, "move the dummyhardcoded moves y tiles (only for supported modes)")
MACRO_CONFIG_INT(SvBasicDummys, sv_autoconnect_bots, 0, 0, 1, CFGFLAG_SERVER | CFGFLAG_GAME, "0=off 1=ChillBlock5")
MACRO_CONFIG_INT(SvSpawntilesMode, sv_spawntiles_mode, 1, 0, 1, CFGFLAG_SERVER | CFGFLAG_GAME, "0=all 1=blue(insta)def(ddr)red(none)")
MACRO_CONFIG_INT(SvMaxDrops, sv_max_drops, 600, 5, 800, CFGFLAG_SERVER, "Maximum amount of dropped healt and ammo (used for survival and admin commands)")

MACRO_CONFIG_INT(SvMinAdminPing, sv_min_admin_ping, 0, 0, 1024, CFGFLAG_SERVER, "remove admin ping from messages that are shorter than x (0=allow all admin pings)")

MACRO_CONFIG_INT(SvAutoFixBrokenAccs, sv_auto_fix_broken_accs, 0, 0, 1, CFGFLAG_SERVER, "search and fix accounts falsely set to logged in on this port (breaks on 99 999 999+ accs)")

// DDNet++ shutdown if not veto at config time if less than x players online
MACRO_CONFIG_INT(SvDDPPshutdown, sv_ddpp_shutdown, 0, 0, 1, CFGFLAG_SERVER | CFGFLAG_GAME, "shutdown srv if players<sv_ddpp_shutdown_players and hour=sv_ddpp_shutdown_hour")
MACRO_CONFIG_INT(SvDDPPshutdownHour, sv_ddpp_shutdown_hour, 4, 0, 24, CFGFLAG_SERVER | CFGFLAG_GAME, "at this hour the server will restart if sv_ddpp_restart is 1")
MACRO_CONFIG_INT(SvDDPPshutdownPlayers, sv_ddpp_shutdown_players, 3, 0, MAX_CLIENTS, CFGFLAG_SERVER | CFGFLAG_GAME, "less than x players needed to successfully pass a sv_ddpp_shutdown")

//minimum players for double money tile
MACRO_CONFIG_INT(SvMinDoubleTilePlayers, sv_min_double_tile_players, 0, 0, 64, CFGFLAG_SERVER | CFGFLAG_GAME, "minimum players for the double money tile to activate (0 = off)")

//MACRO_CONFIG_INT(SvChillBlock5Version, sv_chillblock5_version, 0, 0, 1, CFGFLAG_SERVER, "0=old 1=upper spawn") //not needed yet because upper and lower spawn are dynamic anyways

MACRO_CONFIG_INT(SvKickChilliClan, sv_kick_chilli_clan, 1, 0, 2, CFGFLAG_SERVER, "Punish 'Chilli.*' members with wrong skin 0=off 1=freeze 2=kick")
//STATES
MACRO_CONFIG_INT(SvShopState, sv_shop_state, 0, 0, 1, CFGFLAG_SERVER | CFGFLAG_GAME, "0=can buy from everywhere, 1=need to be in shop to buy smth")

MACRO_CONFIG_INT(SvRoomState, sv_roomstate, 1, 0, 4, CFGFLAG_SERVER|CFGFLAG_GAME, "0=off 1=buy 2=buy/invite 3=buy/admin 4=buy/admin/invite")
MACRO_CONFIG_INT(SvPvpArenaState, sv_pvp_arena_state, 0, 0, 3, CFGFLAG_SERVER | CFGFLAG_GAME, "0=off 1=ChillBlock5 2=BlmapChill 3=tilebased")

MACRO_CONFIG_INT(SvAllowChidraqul, sv_allow_chidraqul, 3, 0, 3, CFGFLAG_SERVER, "allow users to play chidraqul")
MACRO_CONFIG_INT(SvChidraqulWorldX, sv_chidraqul_world_x, 10, 1, 64, CFGFLAG_SERVER, "chidraqul world size (x)")
MACRO_CONFIG_INT(SvChidraqulSlots, sv_chidraqul_slots, 3, 0, 10, CFGFLAG_SERVER, "how many slots the chidraqul multiplayer allows")
MACRO_CONFIG_STR(SvChidraqulDefaultSkin, sv_chidraqul_default_skin, 12, "#", CFGFLAG_SERVER, "Change default skin in chidraqul")

MACRO_CONFIG_INT(SvBlockBroadcast, sv_block_broadcast, 0, 0, 1, CFGFLAG_SERVER, "shows all blocks in broadcast")
MACRO_CONFIG_INT(SvFakeSuper, sv_fake_super, 0, 0, 1, CFGFLAG_SERVER, "0 to deactivate the /fake_super command")
MACRO_CONFIG_INT(SvFlagSounds, sv_flag_sounds, 0, 0, 1, CFGFLAG_SERVER, "blockflags make annoying sounds this command toggle the publicsounds")

MACRO_CONFIG_INT(SvAllowSpawnWeapons, sv_allow_spawn_weapons, 1, 0, 1, CFGFLAG_SERVER, "0 to deactivate spawnweapons")
MACRO_CONFIG_INT(SvAllowDroppingWeapons, sv_allow_dropping_weapons, 4, 0, 4, CFGFLAG_SERVER, "0=off, 1=all (+spawnweaps), 2=normal weaps+hammer+gun, 3=normal weaps+spawnweaps, 4=normal weaps")

MACRO_CONFIG_INT(SvShowBotsInScoreboard, sv_show_bots_in_scoreboard, 2, 0, 2, CFGFLAG_SERVER, "0=hide, 1=hide minigame bots only, 2=show all bots (it takes some seconds to update)")

MACRO_CONFIG_INT(SvFinishEvent, sv_finish_event, 0, 0, 1, CFGFLAG_SERVER, "xp event gives more xp for finishing a map")
MACRO_CONFIG_INT(SvRoomPrice, sv_room_price, 5000, 250, 500000, CFGFLAG_SERVER, "changes the price for the '/shop' item 'room_key'")
MACRO_CONFIG_STR(SvAdString, sv_ad_string, 128, "chillerdragon.tk\ntest", CFGFLAG_SERVER, "advertisement shown at adv places xd")

MACRO_CONFIG_STR(SvDatabasePath, sv_database_path, 512, "accounts.db", CFGFLAG_SERVER, "path/to/sqlite3_database.db used to save DDNet++ data like accounts")
MACRO_CONFIG_INT(SvAccountStuff, sv_account_stuff, 0, 0, 2, CFGFLAG_SERVER, "0=off 1=sqlite 2=filebased (changes logout all)"   /*"0=off 1=blockcity 2=instagib(coming soon)"*/)
MACRO_CONFIG_INT(SvSuperSpawnX, sv_super_spawn_x, 393, 0, 1000, CFGFLAG_SERVER | CFGFLAG_GAME, "x coord for the supermod spawn")
MACRO_CONFIG_INT(SvSuperSpawnY, sv_super_spawn_y, 212, 0, 1000, CFGFLAG_SERVER | CFGFLAG_GAME, "y coord for the supermod spawn")
MACRO_CONFIG_INT(SvNoboSpawnX, sv_nobo_spawn_x, 328, 0, 1000, CFGFLAG_SERVER | CFGFLAG_GAME, "x coord for the nobo spawn")
MACRO_CONFIG_INT(SvNoboSpawnY, sv_nobo_spawn_y, 160, 0, 1000, CFGFLAG_SERVER | CFGFLAG_GAME, "y coord for the nobo spawn")
MACRO_CONFIG_INT(SvNoboSpawnTime, sv_nobo_spawn_time, 0, 0, 1000, CFGFLAG_SERVER | CFGFLAG_GAME, "how many minutes the nobo spawn lasts 0=off")
MACRO_CONFIG_INT(SvSuperSpawnDDraceStart, sv_super_spawn_ddrace_start, 0, 0, 1, CFGFLAG_SERVER | CFGFLAG_GAME, "start ddrace time on supermod spawn")

MACRO_CONFIG_INT(SvPoopMSG, sv_poop_msg, 1, 0, 2, CFGFLAG_SERVER, "0=off 1=on 2=extreme(coudl fuck server but idk)")

MACRO_CONFIG_INT(SvAllowBomb, sv_allow_bomb, 0, 0, 1, CFGFLAG_SERVER | CFGFLAG_GAME, "0=off 1=on")
MACRO_CONFIG_INT(SvAllowBombSelfkill, sv_allow_bomb_selfkill, 0, 0, 1, CFGFLAG_SERVER | CFGFLAG_GAME, "0=off 1=on suicide in '/bomb' games")
MACRO_CONFIG_INT(SvBombTicks, sv_bomb_ticks, 1500, 10, 255000, CFGFLAG_SERVER, "after how many ticks the bomb explodes")
MACRO_CONFIG_INT(SvBombStartDelay, sv_bomb_start_delay, 5, 1, 20, CFGFLAG_SERVER, "after how many % 40 == 0 the game starts")
MACRO_CONFIG_INT(SvBombSpawnX, sv_bomb_spawn_x, 101, 0, 5000, CFGFLAG_SERVER | CFGFLAG_GAME, "x pos of the bomb arena spawn (players spawn at x+CID*2)")
MACRO_CONFIG_INT(SvBombSpawnY, sv_bomb_spawn_y, 459, 0, 5000, CFGFLAG_SERVER | CFGFLAG_GAME, "y pos of the bomb arena spawn")
MACRO_CONFIG_INT(SvBombLockable, sv_bomb_lockable, 0, 0, 2, CFGFLAG_SERVER, "0=No 1=Mods 2=all")
MACRO_CONFIG_INT(SvBombUnreadyKickDelay, sv_bomb_unready_kick_delay, 800, 520, 50000000, CFGFLAG_SERVER, "after how many ticks an unready player gets kicked out of lobby")

MACRO_CONFIG_INT(SvCIdestX, sv_ci_dest_x, 10, -1000, 1000, CFGFLAG_SERVER, "Chillintelligenz X tile destination")
MACRO_CONFIG_INT(SvCIdestY, sv_ci_dest_y, 10, -1000, 1000, CFGFLAG_SERVER, "Chillintelligenz Y tile destination")
MACRO_CONFIG_INT(SvCIfreezetime, sv_ci_freezetime, 2, 0, 1000, CFGFLAG_SERVER, "Chillintelligenz kill delay in secs")

MACRO_CONFIG_INT(SvAllowBalance, sv_allow_balance, 0, 0, 1, CFGFLAG_SERVER, "Allow balance battles or not 0=no 1=allow")
MACRO_CONFIG_INT(SvAllowTrade, sv_allow_trade, 0, 0, 1, CFGFLAG_SERVER, "Allow trade command (can be cheaty in races) 0=no 1=allow")

//MACRO_CONFIG_INT(SvTestValA, sv_test_val_a, 255, 0, 255000, CFGFLAG_SERVER, "chiller's secret test cfgs atm used for color exploration")
//MACRO_CONFIG_INT(SvTestValB, sv_test_val_b, 255, 0, 255000, CFGFLAG_SERVER, "chiller's secret test cfgs atm used for color exploration")
//MACRO_CONFIG_INT(SvTestValC, sv_test_val_c, 255, 0, 255000, CFGFLAG_SERVER, "chiller's secret test cfgs atm used for color exploration")

MACRO_CONFIG_INT(SvShowClientDummysInMaster, sv_show_client_dummys_in_master, 1, 0, 1, CFGFLAG_SERVER, "0=hides clientdummys in master 1=shows clientdummys in master")

//#######################
//Instagib ChillerDragon#
//#######################
//Server/Config side
MACRO_CONFIG_INT(SvInstagibMode, sv_insta, 0, 0, 4, CFGFLAG_SERVER, "0=ddrace 1=gdm 2=undefined 3=idm 4=undefined") //undefined were LMSgrenade and LMSrifle but got removed because it was unfinished and only confused the real vanilla survival
MACRO_CONFIG_INT(SvInstaScore, sv_insta_score, 0, 0, 1, CFGFLAG_SERVER, "0=count from 0 on reconnect in scoreboard 1=load sql scores in scoreboard")
MACRO_CONFIG_INT(SvKillsToFinish, sv_kills_to_finish, 16, 5, 100, CFGFLAG_SERVER, "After how much kills a player gets finish (instagib)")
MACRO_CONFIG_INT(SvDDPPscore, sv_ddpp_score, 1, 0, 1, CFGFLAG_SERVER, "rank scoreboad by times or kills 0=pvp(vanilla) 1=ddpp(ddrace)")
MACRO_CONFIG_STR(SvDDPPgametype, sv_ddpp_gametype, 16, "", CFGFLAG_SERVER, "gametypes: '', 'fly', 'survival', 'vanilla', 'fng', 'battlegores'")
//Player/Minigame side
MACRO_CONFIG_INT(SvGrenadeArenaSlots, sv_grenade_arena_slots, 16, 0, 64, CFGFLAG_SERVER, "extra arena slots for grenade")
MACRO_CONFIG_INT(SvRifleArenaSlots, sv_rifle_arena_slots, 16, 0, 64, CFGFLAG_SERVER, "extra arena slots for rifle")
MACRO_CONFIG_INT(SvAllowInsta, sv_allow_insta, 1, 0, 1, CFGFLAG_SERVER, "allow the '/insta' command 1=allow 0=off")
MACRO_CONFIG_INT(SvAllowGrenade, sv_allow_grenade, 2, 0, 3, CFGFLAG_SERVER, "0=none 1=gdm 2=boomfng 3=gdm/boomfng")
MACRO_CONFIG_INT(SvAllowRifle, sv_allow_rifle, 2, 0, 3, CFGFLAG_SERVER, "0=none 1=idm 2=fng 3=idm/fng")
MACRO_CONFIG_INT(SvRifleScorelimit, sv_rifle_scorelimit, 100, 0, 9999, CFGFLAG_SERVER, "set the scorelimit for fng and idm insta minigames")
MACRO_CONFIG_INT(SvGrenadeScorelimit, sv_grenade_scorelimit, 100, 0, 9999, CFGFLAG_SERVER, "set the scorelimit for boomfng and gdm insta minigames")
MACRO_CONFIG_INT(SvHammerScaleX, sv_hammer_scale_x, 320, 1, 1000, CFGFLAG_SERVER, "linearly scale up hammer x power, percentage, for hammering enemies and unfrozen teammates") //importet from fng2
MACRO_CONFIG_INT(SvHammerScaleY, sv_hammer_scale_y, 120, 1, 1000, CFGFLAG_SERVER, "linearly scale up hammer y power, percentage, for hammering enemies and unfrozen teammates") //importet from fng2
MACRO_CONFIG_INT(SvMeltHammerScaleX, sv_melt_hammer_scale_x, 50, 1, 1000, CFGFLAG_SERVER, "linearly scale up hammer x power, percentage, for hammering frozen teammates") //importet from fng2
MACRO_CONFIG_INT(SvMeltHammerScaleY, sv_melt_hammer_scale_y, 50, 1, 1000, CFGFLAG_SERVER, "linearly scale up hammer y power, percentage, for hammering frozen teammates") //importet from fng2
//General Configs for both Server/Config && Player/Minigame
MACRO_CONFIG_INT(SvSpreePlayers, sv_spree_players, 5, 1, 60, CFGFLAG_SERVER, "how many players have to be online to count killingsprees")
MACRO_CONFIG_INT(SvNeededDamage2NadeKill, sv_needed_damage_2_nadekill, 3, 1, 5, CFGFLAG_SERVER, "how much nade damage is needed for a instagib grenade kill")
MACRO_CONFIG_INT(SvOnFireMode, sv_on_fire_mode, 1, 0, 1, CFGFLAG_SERVER, "no reload delay after hitting an enemy with rifle")

//battlegores (but could be used for ddnet++ as well)
MACRO_CONFIG_INT(SvFreezeKillDelay, sv_freeze_kill_delay, 0, 0, 1000, CFGFLAG_SERVER, "kill tees after being freeze x seconds (0=off)")

//Farm and Dummy Stuff
MACRO_CONFIG_INT(SvSpreeCountBots, sv_spree_count_bots, 1, 0, 1, CFGFLAG_SERVER, "0=bots dont count 1=bots count")
MACRO_CONFIG_INT(SvDummySeeDummy, sv_dummy_see_dummy, 1, 1, 1, CFGFLAG_SERVER | CFGFLAG_GAME, "1 dummys can see each other 0 they dont")
MACRO_CONFIG_INT(SvSpawnBlockProtection, sv_spawnblock_prot, 0, 0, 2, CFGFLAG_SERVER | CFGFLAG_GAME, "0=off 1=escape time 2=esctime+killban")
MACRO_CONFIG_INT(SvSpawnareaLowX, sv_spawnarea_low_x, 325, 0, 1000, CFGFLAG_SERVER | CFGFLAG_GAME, "low x position (as tile) of spawn area used for spawnblock prot")
MACRO_CONFIG_INT(SvSpawnareaLowY, sv_spawnarea_low_y, 191, 0, 1000, CFGFLAG_SERVER | CFGFLAG_GAME, "low y position (as tile) of spawn area used for spawnblock prot")
MACRO_CONFIG_INT(SvSpawnareaHighX, sv_spawnarea_high_x, 362, 0, 1000, CFGFLAG_SERVER | CFGFLAG_GAME, "high x position (as tile) of spawn area used for spawnblock prot")
MACRO_CONFIG_INT(SvSpawnareaHighY, sv_spawnarea_high_y, 206, 0, 1000, CFGFLAG_SERVER | CFGFLAG_GAME, "high y position (as tile) of spawn area used for spawnblock prot")
MACRO_CONFIG_INT(SvPointsMode, sv_points_mode, 2, 0, 2, CFGFLAG_SERVER, "0=/points is off 1=ddnet 2=ddpp(Blockpoints)")
MACRO_CONFIG_INT(SvPointsFarmProtection, sv_points_farm_prot, 20, 0, 600, CFGFLAG_SERVER | CFGFLAG_GAME, "after how many alive seconds block points count")
MACRO_CONFIG_INT(SvDummyBlockPoints, sv_dummy_block_points, 2, 0, 3, CFGFLAG_SERVER | CFGFLAG_GAME, "0=off 1=d 2=kd 3=kd (only in block area)")

//Quests
MACRO_CONFIG_INT(SvQuestCountBots, sv_quest_count_bots, 0, 0, 1, CFGFLAG_SERVER, "if server side bots can be the quest <specific player>") //could do ignore bots at all also in quests like "Hammer 3 tees"
MACRO_CONFIG_INT(SvQuestNeededPlayers, sv_quest_needed_players, 3, 1, 60, CFGFLAG_SERVER, "if less players online you cant play quests where you need specific players")
//MACRO_CONFIG_INT(SvQuestRifleSeconds, sv_quest_rifl_seconds, 280, 10, 6000, CFGFLAG_SERVER, "how much seconds players have to rifle <player> or 15 freezed players")
MACRO_CONFIG_INT(SvQuestRaceTime1, sv_quest_race_time1, 100, 5, 3000, CFGFLAG_SERVER | CFGFLAG_GAME, "how much seconds to finish the race in q3/1")
MACRO_CONFIG_INT(SvQuestRaceTime2, sv_quest_race_time2, 60, 5, 3000, CFGFLAG_SERVER | CFGFLAG_GAME, "how much seconds to finish the race in q3/2")
MACRO_CONFIG_INT(SvQuestRaceTime3, sv_quest_race_time3, 30, 5, 3000, CFGFLAG_SERVER | CFGFLAG_GAME, "how much seconds to finish the race in q3/4")
MACRO_CONFIG_INT(SvQuestSpecialRaceTime, sv_quest_specialrace_time, 60, 5, 6000, CFGFLAG_SERVER | CFGFLAG_GAME, "how much seconds to finish the specialrace in q3/7")
MACRO_CONFIG_INT(SvQuestRaceCondition, sv_quest_race_condition, 3, 0, 4, CFGFLAG_SERVER | CFGFLAG_GAME, "what condition to finish the race q3/9 check player.h too see what each value does")

// meteor
MACRO_CONFIG_INT(SvMeteorFriction, sv_meteor_friction, 5000, 0, 1000000, CFGFLAG_SERVER, "meteor friction")
MACRO_CONFIG_INT(SvMeteorMaxAccel, sv_meteor_max_accel, 2000, 0, 1000000, CFGFLAG_SERVER, "max meteor acceleration per player in pixel/tick^2")
MACRO_CONFIG_INT(SvMeteorAccelPreserve, sv_meteor_accel_preserve, 100000, 0, 1000000, CFGFLAG_SERVER, "how much acceleration is preserved with growing distance to the player")


//survival
MACRO_CONFIG_INT(SvNameplates, sv_nameplates, 0, 0, 1, CFGFLAG_SERVER, "hide or show nameplates in survival games")
MACRO_CONFIG_INT(SvSurvivalStartPlayers, sv_survival_start_players, 4, 3, 64, CFGFLAG_SERVER, "how many players are needed to start a survival game")
MACRO_CONFIG_INT(SvSurvivalGunAmmo, sv_survival_gun_ammo, 0, 0, 1, CFGFLAG_SERVER, "whether the tees have gun ammo on spawn or not")
MACRO_CONFIG_INT(SvSurvivalLobbyDelay, sv_survival_lobby_delay, 10, 5, 300, CFGFLAG_SERVER, "lobby start delay")
MACRO_CONFIG_INT(SvSurvivalDmPlayers, sv_survival_dm_players, 3, 3, 64, CFGFLAG_SERVER, "if less than x players start the deathmatch countdown")
MACRO_CONFIG_INT(SvSurvivalDmDelay, sv_survival_dm_delay, 5, 0, 60, CFGFLAG_SERVER, "after how many minutes the deathmatch should start 0=off")
MACRO_CONFIG_INT(SvSurvivalMaxGameTime, sv_survival_max_game_time, 120, 0, 1200, CFGFLAG_SERVER, "after how many minutes the game should end without winners 0=never")
MACRO_CONFIG_INT(SvAllowSurvival, sv_allow_survival, 0, 0, 1, CFGFLAG_SERVER, "allow survival command")
MACRO_CONFIG_INT(SvSurvivalKillProtection, sv_survival_kill_protection, 1, 0, 2, CFGFLAG_SERVER, "0=off 1=allowed in lobby 2=full on")

MACRO_CONFIG_INT(SvVanillaShotgun, sv_vanilla_shotgun, 0, 0, 1, CFGFLAG_SERVER|CFGFLAG_GAME, "Fix tunings for vanilla shotgun (breaks bullet tiles)")

//block tourna
MACRO_CONFIG_INT(SvAllowBlockTourna, sv_allow_block_tourna, 0, 0, 1, CFGFLAG_SERVER, "0=off 1=allow blocktournaments minigame")
MACRO_CONFIG_INT(SvBlockTournaPlayers, sv_block_tourna_players, 5, 0, 64, CFGFLAG_SERVER, "players needed to start an block tournament")
MACRO_CONFIG_INT(SvBlockTournaDelay, sv_block_tourna_delay, 60, 5, 360, CFGFLAG_SERVER, "how long players can '/join' block tournaments (lobby time)")
MACRO_CONFIG_INT(SvBlockTournaGameTime, sv_block_tourna_game_time, 10, 1, 5000, CFGFLAG_SERVER, "how long block tournas can take in minutes (if after this time nobody won its draw)")

//block deathmatch
MACRO_CONFIG_INT(SvBlockDMarena, sv_block_dm_arena, 1, 1, 2, CFGFLAG_SERVER, "1=arena1 2=arena2")

//blockwave
MACRO_CONFIG_INT(SvAllowBlockWave, sv_allow_block_wave, 0, 0, 2, CFGFLAG_SERVER, "0=off 1=allow blockwave minigame 2=only logged in")

MACRO_CONFIG_INT(SvAdventureBots, sv_adventure_bots, 0, 0, 2, CFGFLAG_SERVER, "number of vanilla pvp bots to spawn at TILE_BOTSPAWN_1")

//FNN fake neural network
MACRO_CONFIG_INT(SvFNNstartX, sv_fnn_start_x, 353, 0, 5000, CFGFLAG_SERVER, "where the dmm25 start because spawn points differ")
MACRO_CONFIG_INT(SvFNNtimeout, sv_fnn_timeout, 0, 0, 500000, CFGFLAG_SERVER, "after how many ticks the bot should restart 0=never")

//FileBased Accounts
MACRO_CONFIG_STR(SvFileAccPath, sv_file_acc_path, 128, "file_accounts", CFGFLAG_SERVER, "the path were the server searches the .acc files (no / after last directory)")


//Global chat (between ddpp servers)
MACRO_CONFIG_INT(SvAllowGlobalChat, sv_allow_global_chat, 0, 0, 1, CFGFLAG_SERVER, "0=off 1=messsages with @all in front get pushed into txt file")
MACRO_CONFIG_INT(SvGlobalChatServers, sv_global_chat_servers, 0, 0, 9, CFGFLAG_SERVER, "how many servers are connected to the global chat (needed for print confirmation)")
MACRO_CONFIG_STR(SvGlobalChatFile, sv_global_chat_file, 128, "global_chat.txt", CFGFLAG_SERVER, "path/to/file.txt where the global chat messages get pushed and pulled")

// configurable tiles cfg_tiles
MACRO_CONFIG_INT(SvCfgTile1, sv_cfg_tile_1, 0, 0, 10, CFGFLAG_SERVER, "use chat cmd for help '/admin cfg_tiles'")
MACRO_CONFIG_INT(SvCfgTile2, sv_cfg_tile_2, 0, 0, 10, CFGFLAG_SERVER, "use chat cmd for help '/admin cfg_tiles'")

//TODO: move me to sql data or somehting
//Supporter permissions
MACRO_CONFIG_INT(SvSupAccReset, sv_sup_acc_reset, 0, 0, 2, CFGFLAG_SERVER, "allow supporters cmds 1='/sql_logout' 2=1 and '/sql_logout_all'")

//hacky
MACRO_CONFIG_INT(SvSaveWrongRcon, sv_save_wrong_rcon, 0, 0, 1, CFGFLAG_SERVER, "saves wrong rcons in the wrong_rcon.txt (sv_wrong_rcon_file) file")
MACRO_CONFIG_INT(SvSaveWrongLogin, sv_save_wrong_login, 0, 0, 1, CFGFLAG_SERVER, "saves wrong '/login's in the wrong_login.txt (sv_wrong_login_file) file")
MACRO_CONFIG_STR(SvWrongRconFile, sv_wrong_rcon_file, 128, "wrong_rcon.txt", CFGFLAG_SERVER, "path/to/file.txt where the wrong rcon logins are stored")
MACRO_CONFIG_STR(SvWrongLoginFile, sv_wrong_login_file, 128, "wrong_login.txt", CFGFLAG_SERVER, "path/to/file.txt where the wrong account logins are stored")
MACRO_CONFIG_STR(SvRconHoneyPassword, sv_rcon_honey_password, 32, "", CFGFLAG_SERVER, "Remote console password for honeypot (should have no real access)")
MACRO_CONFIG_STR(SvRconFakePassword, sv_rcon_fake_password, 32, "", CFGFLAG_SERVER, "Remote console password for fake rcon (only send auth to client and do nothing on srv)")


//ddnet++ anti spoof/flood
// MACRO_CONFIG_INT(SvHaxx0rSpoof, sv_haxx0r_spoof, 0, 0, 1, CFGFLAG_SERVER, "dont touch if ur knoop (chillers anti spoof) 1=on 0=off")
// MACRO_CONFIG_INT(SvHaxx0rSpoofPort, sv_haxx0r_spoof_port, 8303, 8300, 65534, CFGFLAG_SERVER, "used for sv_haxx0r_spoof 1 (don't touch if ur knoop)")
MACRO_CONFIG_INT(SvCaptchaScore, sv_captcha_score, 3, 1, 10, CFGFLAG_SERVER, "how high the captcha score has to be to verfiy humans ('/captcha')")
MACRO_CONFIG_INT(SvRegisterHumanLevel, sv_register_human_level, 0, 0, 9, CFGFLAG_SERVER, "mimum human level to use register command")
MACRO_CONFIG_INT(SvLoginHumanLevel, sv_login_human_level, 0, 0, 9, CFGFLAG_SERVER, "mimum human level to use login command")
MACRO_CONFIG_INT(SvChatHumanLevel, sv_chat_human_level, 0, 0, 9, CFGFLAG_SERVER, "mimum human level to use the chat")
MACRO_CONFIG_INT(SvMaxRegisterPerIp, sv_max_register_per_ip, 2, 1, 10, CFGFLAG_SERVER, "how many accounts one ip can register before getting rate limited")
MACRO_CONFIG_INT(SvMaxLoginPerIp, sv_max_login_per_ip, 12, 4, 128, CFGFLAG_SERVER, "failed login attempts before getting rate limited (after 3 for 1min anyways)")
MACRO_CONFIG_INT(SvMaxNameChangesPerIp, sv_max_namechanges_per_ip, 2, 1, 120, CFGFLAG_SERVER, "how many times one ip can change the name (hourly) before the msg gets hidden")
MACRO_CONFIG_INT(SvShowRenameMessages, sv_hide_rename_msg, 0, 0, 1, CFGFLAG_SERVER, "show the '%s' -> '%s' message in logs (can get really spammy if players have rainbow skin/clan)")
MACRO_CONFIG_INT(SvShowConnectionMessages, sv_show_connection_msg, 3, 0, 3, CFGFLAG_SERVER, "0=none 1=join 2=leave 3=join/leave/spec (specified messages are shown)") // superusefull agianst reconnect trolls c:
MACRO_CONFIG_STR(SvHideConnectionMessagesPattern, sv_hide_connection_msg_pattern, 64, "", CFGFLAG_SERVER, "Names matching this regex pattern won't appear in chat on connect/disconnect/spec (\"\"=off))")
MACRO_CONFIG_INT(SvRconAttemptReport, sv_rcon_attempt_report, 3, 1, 9000, CFGFLAG_SERVER, "after how many failed rcon attempts in a row should it be reported")

//unused bcs no cfgs in system.c ._.
//MACRO_CONFIG_INT(SvFilterLogState, sv_filter_log_state, 0, 0, 1, CFGFLAG_SERVER, "0=off 1=only filter 2=exclude filter   (filter is sv_filter_log_str)")
//MACRO_CONFIG_STR(SvFilterLogString, sv_filter_log_str, 256, "", CFGFLAG_SERVER, "Is used to filter the server log depending on sv_filter_log_state")
MACRO_CONFIG_INT(SvMasterServerLogs, sv_mastersrv_logs, 1, 0, 1, CFGFLAG_SERVER, "1=on: show firewall/mastersrv msgs in console 0=off: save to ddnet++ variable (rcon cmd 'logs')")

//test
MACRO_CONFIG_INT(SvSpeedLogin, sv_speed_login, 0, 0, 1, CFGFLAG_SERVER, "only for testing login speed")
#endif
