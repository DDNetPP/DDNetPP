/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_SHARED_CONFIG_VARIABLES_H
#define ENGINE_SHARED_CONFIG_VARIABLES_H
#undef ENGINE_SHARED_CONFIG_VARIABLES_H // this file will be included several times

// TODO: remove this
#include "././game/variables.h"

MACRO_CONFIG_STR(Password, password, 32, "", CFGFLAG_CLIENT|CFGFLAG_SERVER, "Password to the server")
MACRO_CONFIG_STR(Logfile, logfile, 128, "", CFGFLAG_SAVE|CFGFLAG_CLIENT|CFGFLAG_SERVER, "Filename to log all output to")
MACRO_CONFIG_INT(ConsoleOutputLevel, console_output_level, 0, 0, 2, CFGFLAG_CLIENT|CFGFLAG_SERVER, "Adjusts the amount of information in the console")

MACRO_CONFIG_STR(SvName, sv_name, 128, "unnamed server", CFGFLAG_SERVER, "Server name")
MACRO_CONFIG_STR(Bindaddr, bindaddr, 128, "", CFGFLAG_CLIENT|CFGFLAG_SERVER|CFGFLAG_MASTER, "Address to bind the client/server to")
MACRO_CONFIG_INT(SvPort, sv_port, 8303, 0, 0, CFGFLAG_SERVER, "Port to use for the server (Only ports 8303-8310 work in LAN server browser)")
MACRO_CONFIG_INT(SvExternalPort, sv_external_port, 0, 0, 0, CFGFLAG_SERVER, "External port to report to the master servers")
MACRO_CONFIG_STR(SvMap, sv_map, 128, "BlmapChill", CFGFLAG_SERVER, "Map to use on the server")
MACRO_CONFIG_INT(SvMaxClients, sv_max_clients, 64, 1, MAX_CLIENTS, CFGFLAG_SERVER, "Maximum number of clients that are allowed on a server (more than 64 crashes ddnet clients)")
MACRO_CONFIG_INT(SvMaxClientsPerIP, sv_max_clients_per_ip, 4, 1, MAX_CLIENTS, CFGFLAG_SERVER, "Maximum number of clients with the same IP that can connect to the server")
MACRO_CONFIG_INT(SvHighBandwidth, sv_high_bandwidth, 0, 0, 1, CFGFLAG_SERVER, "Use high bandwidth mode. Doubles the bandwidth required for the server. LAN use only")
MACRO_CONFIG_INT(SvRegister, sv_register, 1, 0, 1, CFGFLAG_SERVER, "Register server with master server for public listing")
MACRO_CONFIG_STR(SvRconPassword, sv_rcon_password, 32, "", CFGFLAG_SERVER, "Remote console password (full access)")
MACRO_CONFIG_STR(SvRconModPassword, sv_rcon_mod_password, 32, "", CFGFLAG_SERVER, "Remote console password for moderators (limited access)")
MACRO_CONFIG_STR(SvRconHoneyPassword, sv_rcon_honey_password, 32, "", CFGFLAG_SERVER, "Remote console password for honeypot (should have no real access)")
MACRO_CONFIG_STR(SvRconFakePassword, sv_rcon_fake_password, 32, "", CFGFLAG_SERVER, "Remote console password for fake rcon (only send auth to client and do nothing on srv)")
MACRO_CONFIG_INT(SvRconMaxTries, sv_rcon_max_tries, 30, 0, 100, CFGFLAG_SERVER, "Maximum number of tries for remote console authentication")
MACRO_CONFIG_INT(SvRconBantime, sv_rcon_bantime, 5, 0, 1440, CFGFLAG_SERVER, "The time a client gets banned if remote console authentication fails. 0 makes it just use kick")
MACRO_CONFIG_INT(SvAutoDemoRecord, sv_auto_demo_record, 0, 0, 1, CFGFLAG_SERVER, "Automatically record demos")
MACRO_CONFIG_INT(SvAutoDemoMax, sv_auto_demo_max, 10, 0, 1000, CFGFLAG_SERVER, "Maximum number of automatically recorded demos (0 = no limit)")
MACRO_CONFIG_INT(SvVanillaAntiSpoof, sv_vanilla_antispoof, 1, 0, 1, CFGFLAG_SERVER, "Enable vanilla Antispoof")

MACRO_CONFIG_INT(SvPlayerDemoRecord, sv_player_demo_record, 0, 0, 1, CFGFLAG_SERVER, "Automatically record demos for each player")
MACRO_CONFIG_INT(SvDemoChat, sv_demo_chat, 0, 0, 1, CFGFLAG_SERVER, "Record chat for demos")

MACRO_CONFIG_STR(EcBindaddr, ec_bindaddr, 128, "localhost", CFGFLAG_ECON, "Address to bind the external console to. Anything but 'localhost' is dangerous")
MACRO_CONFIG_INT(EcPort, ec_port, 0, 0, 0, CFGFLAG_ECON, "Port to use for the external console")
MACRO_CONFIG_STR(EcPassword, ec_password, 32, "", CFGFLAG_ECON, "External console password")
MACRO_CONFIG_INT(EcBantime, ec_bantime, 0, 0, 1440, CFGFLAG_ECON, "The time a client gets banned if econ authentication fails. 0 just closes the connection")
MACRO_CONFIG_INT(EcAuthTimeout, ec_auth_timeout, 30, 1, 120, CFGFLAG_ECON, "Time in seconds before the the econ authentification times out")
MACRO_CONFIG_INT(EcOutputLevel, ec_output_level, 1, 0, 2, CFGFLAG_ECON, "Adjusts the amount of information in the external console")

MACRO_CONFIG_INT(Debug, debug, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SERVER, "Debug mode")
MACRO_CONFIG_INT(DbgStress, dbg_stress, 0, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SERVER, "Stress systems")
MACRO_CONFIG_INT(DbgStressNetwork, dbg_stress_network, 0, 0, 0, CFGFLAG_CLIENT|CFGFLAG_SERVER, "Stress network")
MACRO_CONFIG_INT(DbgPref, dbg_pref, 0, 0, 1, CFGFLAG_SERVER, "Performance outputs")
MACRO_CONFIG_INT(DbgGraphs, dbg_graphs, 0, 0, 1, CFGFLAG_CLIENT, "Performance graphs")
MACRO_CONFIG_INT(DbgHitch, dbg_hitch, 0, 0, 0, CFGFLAG_SERVER, "Hitch warnings")
MACRO_CONFIG_STR(DbgStressServer, dbg_stress_server, 32, "localhost", CFGFLAG_CLIENT, "Server to stress")
MACRO_CONFIG_INT(DbgResizable, dbg_resizable, 0, 0, 0, CFGFLAG_CLIENT, "Enables window resizing")

// DDRace
MACRO_CONFIG_STR(SvWelcome, sv_welcome, 64, "", CFGFLAG_SERVER, "Message that will be displayed to players who join the server")
MACRO_CONFIG_INT(SvReservedSlots, sv_reserved_slots, 0, 0, 16, CFGFLAG_SERVER, "The number of slots that are reserved for special players")
MACRO_CONFIG_STR(SvReservedSlotsPass, sv_reserved_slots_pass, 32, "", CFGFLAG_SERVER, "The password that is required to use a reserved slot")
MACRO_CONFIG_INT(SvHit, sv_hit, 1, 0, 1, CFGFLAG_SERVER|CFGFLAG_GAME, "Whether players can hammer/grenade/laser eachother or not")
MACRO_CONFIG_INT(SvEndlessDrag, sv_endless_drag, 0, 0, 1, CFGFLAG_SERVER|CFGFLAG_GAME, "Turns endless hooking on/off")
MACRO_CONFIG_INT(SvTestingCommands, sv_test_cmds, 0, 0, 1, CFGFLAG_SERVER, "Turns testing commands aka cheats on/off")
MACRO_CONFIG_INT(SvFreezeDelay, sv_freeze_delay, 3, 1, 30, CFGFLAG_SERVER|CFGFLAG_GAME, "How many seconds the players will remain frozen (applies to all except delayed freeze in switch layer & deepfreeze)")
MACRO_CONFIG_INT(ClDDRaceBinds, cl_race_binds, 1, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Enable Default DDRace builds when pressing the reset binds button")
MACRO_CONFIG_INT(ClDDRaceBindsSet, cl_race_binds_set, 0, 0, 1, CFGFLAG_CLIENT|CFGFLAG_SAVE, "Whether the DDRace binds set or not (this is automated you don't need to use this)")
MACRO_CONFIG_INT(SvEndlessSuperHook, sv_endless_super_hook, 0, 0, 1, CFGFLAG_SERVER, "Endless hook for super players on/off")
MACRO_CONFIG_INT(SvHideScore, sv_hide_score, 0, 0, 1, CFGFLAG_SERVER, "Whether players scores will be announced or not")
MACRO_CONFIG_INT(SvSaveWorseScores, sv_save_worse_scores, 1, 0, 1, CFGFLAG_SERVER|CFGFLAG_GAME, "Whether to save worse scores when you already have a better one")
MACRO_CONFIG_INT(SvPauseable, sv_pauseable, 0, 0, 1, CFGFLAG_SERVER|CFGFLAG_GAME, "leave it 0 to let run ddpp nice")
MACRO_CONFIG_INT(SvPauseMessages, sv_pause_messages, 0, 0, 1, CFGFLAG_SERVER, "Whether to show messages when a player pauses and resumes")
MACRO_CONFIG_INT(SvPauseTime, sv_pause_time, 0, 0, 1, CFGFLAG_SERVER, "Whether '/pause' and 'sv_max_dc_restore' pauses the time of player or not")
MACRO_CONFIG_INT(SvPauseFrequency, sv_pause_frequency, 1, 0, 9999, CFGFLAG_SERVER, "The minimum allowed delay between pauses")

MACRO_CONFIG_INT(SvEmotionalTees, sv_emotional_tees, 1, -1, 1, CFGFLAG_SERVER, "Whether eye change of tees is enabled with emoticons = 1, not = 0, -1 not at all")
MACRO_CONFIG_INT(SvEmoticonDelay, sv_emoticon_delay, 3, 0, 9999, CFGFLAG_SERVER, "The time in seconds between over-head emoticons")
MACRO_CONFIG_INT(SvEyeEmoteChangeDelay, sv_eye_emote_change_delay, 1, 0, 9999, CFGFLAG_SERVER, "The time in seconds between eye emoticons change")


MACRO_CONFIG_INT(SvChatDelay, sv_chat_delay, 1, 0, 9999, CFGFLAG_SERVER, "The time in seconds between chat messages")
MACRO_CONFIG_INT(SvTeamChangeDelay, sv_team_change_delay, 3, 0, 9999, CFGFLAG_SERVER, "The time in seconds between team changes (spectator/in game)")
MACRO_CONFIG_INT(SvInfoChangeDelay, sv_info_change_delay, 5, 0, 9999, CFGFLAG_SERVER, "The time in seconds between info changes (name/skin/color), to avoid ranbow mod set this to a very high time")
MACRO_CONFIG_INT(SvVoteTime, sv_vote_time, 25, 1, 9999, CFGFLAG_SERVER, "The time in seconds a vote lasts")
MACRO_CONFIG_INT(SvVoteMapTimeDelay, sv_vote_map_delay,0,0,9999,CFGFLAG_SERVER, "The minimum time in seconds between map votes")
MACRO_CONFIG_INT(SvVoteDelay, sv_vote_delay, 10, 0, 9999, CFGFLAG_SERVER, "The time in seconds between a users votes")
MACRO_CONFIG_INT(SvVoteDelayAll, sv_vote_delay_all, 600, 0, 9999, CFGFLAG_SERVER, "The time in seconds between any vote (ddpp total afecting all players)")
MACRO_CONFIG_INT(SvVoteKickTimeDelay, sv_vote_kick_delay, 100, 0, 9999, CFGFLAG_SERVER, "The minimum time in seconds between kick votes")
MACRO_CONFIG_INT(SvVoteYesPercentage, sv_vote_yes_percentage, 50, 1, 100, CFGFLAG_SERVER, "The percent of people that need to agree or deny for the vote to succeed/fail")
MACRO_CONFIG_INT(SvVoteMajority, sv_vote_majority, 0, 0, 1, CFGFLAG_SERVER, "Whether No. of Yes is compared to No. of No votes or to number of total Players ( Default is 0 Y compare N)")
MACRO_CONFIG_INT(SvVoteMaxTotal, sv_vote_max_total, 0, 0, MAX_CLIENTS, CFGFLAG_SERVER, "How many people can participate in a vote at max (0 = no limit by default)")
MACRO_CONFIG_INT(SvVoteVetoTime, sv_vote_veto_time, 10, 0, 1000, CFGFLAG_SERVER, "Minutes of game time until a player can veto map change votes (0 = disabled)")
MACRO_CONFIG_INT(SvSpectatorVotes, sv_spectator_votes, 1, 0, 1, CFGFLAG_SERVER, "Choose if spectators are allowed to start votes")
MACRO_CONFIG_INT(SvKillDelay, sv_kill_delay,3,0,9999,CFGFLAG_SERVER, "The minimum time in seconds between kills")
MACRO_CONFIG_INT(SvSuicidePenalty, sv_suicide_penalty,0,0,9999,CFGFLAG_SERVER, "The minimum time in seconds between kill or /kills and respawn")

MACRO_CONFIG_INT(SvMapWindow, sv_map_window, 15, 0, 100, CFGFLAG_SERVER, "Map downloading send-ahead window")
MACRO_CONFIG_INT(SvFastDownload, sv_fast_download, 1, 0, 1, CFGFLAG_SERVER, "Enables fast download of maps")

MACRO_CONFIG_INT(SvShotgunBulletSound, sv_shotgun_bullet_sound, 0, 0, 1, CFGFLAG_SERVER, "Crazy shotgun bullet sound on/off")

MACRO_CONFIG_INT(SvCheckpointSave, sv_checkpoint_save, 1, 0, 1, CFGFLAG_SERVER, "Whether to save checkpoint times to the score file")
MACRO_CONFIG_STR(SvScoreFolder, sv_score_folder, 32, "records", CFGFLAG_SERVER, "Folder to save score files to")

#if defined(CONF_SQL)
MACRO_CONFIG_INT(SvUseSQL, sv_use_sql, 0, 0, 1, CFGFLAG_SERVER, "Enables SQL DB instead of record file")
MACRO_CONFIG_STR(SvSqlUser, sv_sql_user, 32, "nameless", CFGFLAG_SERVER, "SQL User")
MACRO_CONFIG_STR(SvSqlPw, sv_sql_pw, 32, "tee", CFGFLAG_SERVER, "SQL Password")
MACRO_CONFIG_STR(SvSqlIp, sv_sql_ip, 32, "127.0.0.1", CFGFLAG_SERVER, "SQL Database IP")
MACRO_CONFIG_INT(SvSqlPort, sv_sql_port, 3306, 0, 65535, CFGFLAG_SERVER, "SQL Database port")
MACRO_CONFIG_STR(SvSqlDatabase, sv_sql_database, 16, "teeworlds", CFGFLAG_SERVER, "SQL Database name")
MACRO_CONFIG_STR(SvSqlServerName, sv_sql_servername, 4, "UNK", CFGFLAG_SERVER, "SQL Server name that is inserted into record table")
MACRO_CONFIG_STR(SvSqlPrefix, sv_sql_prefix, 16, "record", CFGFLAG_SERVER, "SQL Database table prefix")
MACRO_CONFIG_INT(SvSaveGames, sv_savegames, 1, 0, 1, CFGFLAG_SERVER, "Enables savegames (/save and /load)")
MACRO_CONFIG_INT(SvSaveGamesDelay, sv_savegames_delay, 60, 0, 10000, CFGFLAG_SERVER, "Delay in seconds for loading a savegame")
#endif

MACRO_CONFIG_STR(SvRulesLine1, sv_rules_line1, 40, "", CFGFLAG_SERVER, "Rules line 1")
MACRO_CONFIG_STR(SvRulesLine2, sv_rules_line2, 40, "", CFGFLAG_SERVER, "Rules line 2")
MACRO_CONFIG_STR(SvRulesLine3, sv_rules_line3, 40, "", CFGFLAG_SERVER, "Rules line 3")
MACRO_CONFIG_STR(SvRulesLine4, sv_rules_line4, 40, "", CFGFLAG_SERVER, "Rules line 4")
MACRO_CONFIG_STR(SvRulesLine5, sv_rules_line5, 40, "", CFGFLAG_SERVER, "Rules line 5")
MACRO_CONFIG_STR(SvRulesLine6, sv_rules_line6, 40, "", CFGFLAG_SERVER, "Rules line 6")
MACRO_CONFIG_STR(SvRulesLine7, sv_rules_line7, 40, "", CFGFLAG_SERVER, "Rules line 7")
MACRO_CONFIG_STR(SvRulesLine8, sv_rules_line8, 40, "", CFGFLAG_SERVER, "Rules line 8")
MACRO_CONFIG_STR(SvRulesLine9, sv_rules_line9, 40, "", CFGFLAG_SERVER, "Rules line 9")
MACRO_CONFIG_STR(SvRulesLine10, sv_rules_line10, 40, "", CFGFLAG_SERVER, "Rules line 10")

MACRO_CONFIG_INT(SvTeam, sv_team, 0, 0, 3, CFGFLAG_SERVER|CFGFLAG_GAME, "Teams configuration (0 = off, 1 = on but optional, 2 = must play only with teams, 3 = forced random team only for you)")
MACRO_CONFIG_INT(SvTeamMaxSize, sv_max_team_size, MAX_CLIENTS, 1, MAX_CLIENTS, CFGFLAG_SERVER|CFGFLAG_GAME, "Maximum team size (from 2 to 64)")
MACRO_CONFIG_INT(SvTeamLock, sv_team_lock, 1, 0, 1, CFGFLAG_SERVER|CFGFLAG_GAME, "Enable team lock")
MACRO_CONFIG_INT(SvMapVote, sv_map_vote, 1, 0, 1, CFGFLAG_SERVER|CFGFLAG_GAME, "Whether to allow /map")

MACRO_CONFIG_STR(SvAnnouncementFileName, sv_announcement_filename, 128, "announcement.txt", CFGFLAG_SERVER, "file which will have the announcement, each one at a line")
MACRO_CONFIG_INT(SvAnnouncementInterval, sv_announcement_interval, 300, 1, 9999, CFGFLAG_SERVER, "time(minutes) in which the announcement will be displayed from the announcement file")
MACRO_CONFIG_INT(SvAnnouncementRandom, sv_announcement_random, 1, 0, 1, CFGFLAG_SERVER, "Whether announcements are sequential or random")

MACRO_CONFIG_INT(SvOldLaser, sv_old_laser, 0, 0, 1, CFGFLAG_SERVER|CFGFLAG_GAME, "Whether lasers can hit you if you shot them and that they pull you towards the bounce origin (0 for all new maps) or lasers can't hit you if you shot them, and they pull others towards the shooter")
MACRO_CONFIG_INT(SvSlashMe, sv_slash_me, 0, 0, 1, CFGFLAG_SERVER, "Whether /me is active on the server or not")
MACRO_CONFIG_INT(SvRejoinTeam0, sv_rejoin_team_0, 1, 0, 1, CFGFLAG_SERVER, "Make a team automatically rejoin team 0 after finish (only if not locked)")

MACRO_CONFIG_INT(SvResetPickups, sv_reset_pickups, 0, 0, 1, CFGFLAG_SERVER|CFGFLAG_GAME, "Whether the weapons are reset on passing the start tile or not")

MACRO_CONFIG_INT(ConnTimeout, conn_timeout, 100, 5, 1000, CFGFLAG_SAVE|CFGFLAG_CLIENT|CFGFLAG_SERVER, "Network timeout")
MACRO_CONFIG_INT(ConnTimeoutProtection, conn_timeout_protection, 1000, 5, 10000, CFGFLAG_SAVE|CFGFLAG_SERVER, "Network timeout protection")
MACRO_CONFIG_INT(SvShowOthers, sv_show_others, 1, 0, 1, CFGFLAG_SERVER, "Whether players can user the command showothers or not")
MACRO_CONFIG_INT(SvShowOthersDefault, sv_show_others_default, 0, 0, 1, CFGFLAG_SERVER, "Whether players see others by default")
MACRO_CONFIG_INT(SvShowAllDefault, sv_show_all_default, 0, 0, 1, CFGFLAG_SERVER, "Whether players see all tees by default")
MACRO_CONFIG_INT(SvMaxAfkTime, sv_max_afk_time, 0, 0, 9999, CFGFLAG_SERVER, "The time in seconds a player is allowed to be afk (0 = disabled)")
MACRO_CONFIG_INT(SvMaxAfkVoteTime, sv_max_afk_vote_time, 300, 0, 9999, CFGFLAG_SERVER, "The time in seconds a player can be afk and his votes still count (0 = disabled)")
MACRO_CONFIG_INT(SvPlasmaRange, sv_plasma_range, 700, 1, 99999, CFGFLAG_SERVER|CFGFLAG_GAME, "How far will the plasma gun track tees")
MACRO_CONFIG_INT(SvPlasmaPerSec, sv_plasma_per_sec, 3, 0, 50, CFGFLAG_SERVER|CFGFLAG_GAME, "How many shots does the plasma gun fire per seconds")
MACRO_CONFIG_INT(SvVotePause, sv_vote_pause, 1, 0, 1, CFGFLAG_SERVER, "Allow voting to pause players (instead of moving to spectators)")
MACRO_CONFIG_INT(SvVotePauseTime, sv_vote_pause_time, 10, 0, 360, CFGFLAG_SERVER, "The time (in seconds) players have to wait in pause when paused by vote")
MACRO_CONFIG_INT(SvTuneReset, sv_tune_reset, 1, 0, 1, CFGFLAG_SERVER, "Whether tuning is reset after each map change or not")
MACRO_CONFIG_STR(SvResetFile, sv_reset_file, 128, "reset.cfg", CFGFLAG_SERVER, "File to execute on map change or reload to set the default server settings")
MACRO_CONFIG_STR(SvInputFifo, sv_input_fifo, 128, "", CFGFLAG_SERVER, "Fifo file to use as input for server console")
MACRO_CONFIG_INT(SvDDRaceTuneReset, sv_ddrace_tune_reset, 1, 0, 1, CFGFLAG_SERVER, "Whether DDRace tuning(sv_hit, Sv_Endless_Drag & Sv_Old_Laser) is reset after each map change or not")
MACRO_CONFIG_INT(SvNamelessScore, sv_nameless_score, 0, 0, 1, CFGFLAG_SERVER, "Whether nameless tee has a score or not")
MACRO_CONFIG_INT(SvTimeInBroadcastInterval, sv_time_in_broadcast_interval, 1, 0, 60, CFGFLAG_SERVER, "How often to update the broadcast time")
MACRO_CONFIG_INT(SvDefaultTimerType, sv_default_timer_type, 0, 0, 1, CFGFLAG_SERVER, "Default way of displaying time either game/round timer or broadcast. 0 = game/round timer, 1 = broadcast")


// these might need some fine tuning
MACRO_CONFIG_INT(SvChatPenalty, sv_chat_penalty, 250, 50, 1000, CFGFLAG_SERVER, "chat score will be increased by this on every message, and decremented by 1 on every tick.")
MACRO_CONFIG_INT(SvChatThreshold, sv_chat_threshold, 1000, 50, 10000 , CFGFLAG_SERVER, "if chats core exceeds this, the player will be muted for sv_spam_mute_duration seconds")
MACRO_CONFIG_INT(SvSpamMuteDuration, sv_spam_mute_duration, 60, 0, 3600 , CFGFLAG_SERVER, "how many seconds to mute, if player triggers mute on spam. 0 = off")

MACRO_CONFIG_INT(SvEvents, sv_events, 1, 0, 1, CFGFLAG_SERVER, "Enable triggering of server events, like the happy eyeemotes on some holidays.")
MACRO_CONFIG_INT(SvRankCheats, sv_rank_cheats, 0, 0, 1, CFGFLAG_SERVER, "Enable ranks after cheats have been used (file based server only)")
MACRO_CONFIG_INT(SvShutdownWhenEmpty, sv_shutdown_when_empty, 0, 0, 1, CFGFLAG_SERVER, "Shutdown server as soon as noone is on it anymore")
MACRO_CONFIG_INT(SvKillProtection, sv_kill_protection, 20, 0, 9999, CFGFLAG_SERVER, "0 - Disable, 1-9999 minutes")
MACRO_CONFIG_INT(SvSoloServer, sv_solo_server, 0, 0, 1, CFGFLAG_SERVER|CFGFLAG_GAME, "Set server to solo mode (no player interactions, has to be set before loading the map)")
MACRO_CONFIG_STR(SvClientSuggestion, sv_client_suggestion, 128, "Get DDNet client from DDNet.tw to use all features on DDNet!", CFGFLAG_SERVER, "Broadcast to display to players without DDNet client")
MACRO_CONFIG_STR(SvClientSuggestionSupported, sv_client_suggestion_supported, 128, "Update to 11.4.3 or higher for full support on DDNet++ servers", CFGFLAG_SERVER, "Broadcast to display to players that their client isnt fully supported")
MACRO_CONFIG_STR(SvClientSuggestionOld, sv_client_suggestion_old, 128, "Your DDNet client is old, update it on DDNet.tw!", CFGFLAG_SERVER, "Broadcast to display to players with an old version of DDNet client")
MACRO_CONFIG_STR(SvClientSuggestionBot, sv_client_suggestion_bot, 128, "Your client has bots and can be remote controlled!\nPlease use another client like DDNet client from DDNet.tw", CFGFLAG_SERVER, "Broadcast to display to players with a known botting client")

// netlimit
MACRO_CONFIG_INT(SvNetlimit, sv_netlimit, 0, 0, 10000, CFGFLAG_SERVER, "Netlimit: Maximum amount of traffic a client is allowed to use (in kb/s)")
MACRO_CONFIG_INT(SvNetlimitAlpha, sv_netlimit_alpha, 50, 1, 100, CFGFLAG_SERVER, "Netlimit: Alpha of Exponention moving average")


/*************************************
*                                    *
*             DDNet++                *
*                                    *
**************************************/
//ChillerDragon

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
MACRO_CONFIG_INT(SvSurvivalKillProtection, sv_survival_kill_protection, 2, 0, 2, CFGFLAG_SERVER, "0=off 1=allowed in lobby 2=full on")

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
