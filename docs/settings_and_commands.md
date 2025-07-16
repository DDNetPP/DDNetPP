# DDNet++ configs

+ `sv_off_ddpp` Turn off as many ddnet++ features as possible (aiming to run vanilla ddnet)
+ `sv_scorelimit` Score limit (0 disables)
+ `sv_timelimit` Time limit in minutes (0 disables)
+ `sv_vote_delay_all` The time in seconds between any vote (ddpp total afecting all players)
+ `sv_allow_minigame` allow users to play minigames
+ `sv_max_shop_messages` How many times the shopbot greets per player. (-1=infinite 0=off)
+ `sv_dummy_map_offset_x` move the dummyhardcoded moves x tiles (only for supported modes)
+ `sv_dummy_map_offset_y` move the dummyhardcoded moves y tiles (only for supported modes)
+ `sv_autoconnect_bots` 0=off 1=ChillBlock5
+ `sv_max_drops` Maximum amount of dropped healt and ammo (used for survival and admin commands)
+ `sv_min_admin_ping` remove admin ping from messages that are shorter than x (0=allow all admin pings)
+ `sv_auto_fix_broken_accs` search and fix accounts falsely set to logged in on this port (breaks on 99 999 999+ accs)
+ `sv_ddpp_shutdown` shutdown srv if players<sv_ddpp_shutdown_players and hour=sv_ddpp_shutdown_hour
+ `sv_ddpp_shutdown_hour` at this hour the server will restart if sv_ddpp_restart is 1
+ `sv_ddpp_shutdown_players` less than x players needed to successfully pass a sv_ddpp_shutdown
+ `sv_min_double_tile_players` minimum players for the double money tile to activate (0 = off)
+ `sv_kick_chilli_clan` Punish 'Chilli.*' members with wrong skin 0=off 1=freeze 2=kick
+ `sv_shop_state` 0=can buy from everywhere, 1=need to be in shop to buy smth
+ `sv_roomstate` 0=off 1=buy 2=buy/invite 3=buy/admin 4=buy/admin/invite
+ `sv_allow_chidraqul` allow users to play chidraqul
+ `sv_chidraqul_world_x` chidraqul world size (x)
+ `sv_chidraqul_slots` how many slots the chidraqul multiplayer allows
+ `sv_block_broadcast` shows all blocks in broadcast
+ `sv_fake_super` 0 to deactivate the /fake_super command
+ `sv_flag_sounds` blockflags make annoying sounds this command toggle the publicsounds
+ `sv_flag_hooking` Whether flags are hookable
+ `sv_allow_spawn_weapons` 0 to deactivate spawnweapons
+ `sv_allow_dropping_weapons` 0=off, 1=all (+spawnweaps), 2=normal weaps+hammer+gun, 3=normal weaps+spawnweaps, 4=normal weaps
+ `sv_show_bots_in_scoreboard` 0=hide, 1=hide minigame bots only, 2=show all bots (it takes some seconds to update)
+ `sv_finish_event` xp event gives more xp for finishing a map
+ `sv_accounts` 0=off 1=sql 2=filebased (changes logout all)" /*"0=off 1=blockcity 2=instagib(coming soon)"*
+ `sv_use_mysql_for_accounts` Enables MySQL backend instead of SQLite backend (needs sv_accounts 1)
+ `sv_require_login_to_join` 0=off 1=require account to play (see also sv_accounts)
+ `sv_require_login_to_vote` 0=everyone can vote 1=only logged in players can vote
+ `sv_super_spawn_x` x coord for the supermod spawn
+ `sv_super_spawn_y` y coord for the supermod spawn
+ `sv_nobo_spawn_x` x coord for the nobo spawn
+ `sv_nobo_spawn_y` y coord for the nobo spawn
+ `sv_nobo_spawn_time` how many minutes the nobo spawn lasts 0=off
+ `sv_poop_msg` 0=off 1=on 2=extreme(coudl fuck server but idk)
+ `sv_allow_bomb` 0=off 1=on
+ `sv_allow_bomb_selfkill` 0=off 1=on suicide in '/bomb' games
+ `sv_bomb_ticks` after how many ticks the bomb explodes
+ `sv_bomb_start_delay` after how many % 40 == 0 the game starts
+ `sv_bomb_spawn_x` x pos of the bomb arena spawn (players spawn at x+CID*2)
+ `sv_bomb_spawn_y` y pos of the bomb arena spawn
+ `sv_bomb_lockable` 0=No 1=Mods 2=all
+ `sv_bomb_unready_kick_delay` after how many ticks an unready player gets kicked out of lobby
+ `sv_ci_dest_x` Chillintelligenz X tile destination
+ `sv_ci_dest_y` Chillintelligenz Y tile destination
+ `sv_ci_freezetime` Chillintelligenz kill delay in secs
+ `sv_allow_balance` Allow balance battles or not 0=no 1=allow
+ `sv_allow_trade` Allow trade command (can be cheaty in races) 0=no 1=allow
+ `sv_show_client_dummys_in_master` 0=hides clientdummys in master 1=shows clientdummys in master
+ `sv_insta` 0=ddrace 1=gdm 2=undefined 3=idm 4=undefined") //undefined were LMSgrenade and LMSrifle but got removed because it was unfinished and only confused the real vanilla surviv
+ `sv_insta_score` 0=count from 0 on reconnect in scoreboard 1=load sql scores in scoreboard
+ `sv_kills_to_finish` After how much kills a player gets finish (instagib)
+ `sv_ddpp_score` rank scoreboad by times or kills 0=pvp(vanilla) 1=ddpp(ddrace)
+ `sv_grenade_arena_slots` extra arena slots for grenade
+ `sv_rifle_arena_slots` extra arena slots for rifle
+ `sv_allow_insta` allow the '/insta' command 1=allow 0=off
+ `sv_allow_grenade` 0=none 1=gdm 2=boomfng 3=gdm/boomfng
+ `sv_allow_rifle` 0=none 1=idm 2=fng 3=idm/fng
+ `sv_rifle_scorelimit` set the scorelimit for fng and idm insta minigames
+ `sv_grenade_scorelimit` set the scorelimit for boomfng and gdm insta minigames
+ `sv_hammer_scale_x` linearly scale up hammer x power, percentage, for hammering enemies and unfrozen teammates") //importet from fn
+ `sv_hammer_scale_y` linearly scale up hammer y power, percentage, for hammering enemies and unfrozen teammates") //importet from fn
+ `sv_melt_hammer_scale_x` linearly scale up hammer x power, percentage, for hammering frozen teammates") //importet from fn
+ `sv_melt_hammer_scale_y` linearly scale up hammer y power, percentage, for hammering frozen teammates") //importet from fn
+ `sv_spree_players` how many players have to be online to count killingsprees
+ `sv_needed_damage_2_nadekill` how much nade damage is needed for a instagib grenade kill
+ `sv_on_fire_mode` no reload delay after hitting an enemy with rifle
+ `sv_freeze_kill_delay` kill tees after being freeze x seconds (0=off)
+ `sv_spree_count_bots` 0=bots dont count 1=bots count
+ `sv_dummy_see_dummy` 1 dummys can see each other 0 they dont
+ `sv_spawnblock_prot` 0=off 1=escape time 2=esctime+killban
+ `sv_spawnarea_low_x` low x position (as tile) of spawn area used for spawnblock prot
+ `sv_spawnarea_low_y` low y position (as tile) of spawn area used for spawnblock prot
+ `sv_spawnarea_high_x` high x position (as tile) of spawn area used for spawnblock prot
+ `sv_spawnarea_high_y` high y position (as tile) of spawn area used for spawnblock prot
+ `sv_points_mode` 0=/points is off 1=ddnet 2=ddpp(Blockpoints)
+ `sv_points_farm_prot` after how many alive seconds block points count
+ `sv_dummy_block_points` 0=off 1=d 2=kd 3=kd (only in block area)
+ `sv_quest_count_bots` if server side bots can be the quest <specific player>") //could do ignore bots at all also in quests like "Hammer 3 tee
+ `sv_quest_needed_players` if less players online you cant play quests where you need specific players
+ `sv_quest_race_time1` how much seconds to finish the race in q3/1
+ `sv_quest_race_time2` how much seconds to finish the race in q3/2
+ `sv_quest_race_time3` how much seconds to finish the race in q3/4
+ `sv_quest_specialrace_time` how much seconds to finish the specialrace in q3/7
+ `sv_quest_race_condition` what condition to finish the race q3/9 check player.h too see what each value does
+ `sv_meteor_friction` meteor friction
+ `sv_meteor_max_accel` max meteor acceleration per player in pixel/tick^2
+ `sv_meteor_accel_preserve` how much acceleration is preserved with growing distance to the player
+ `sv_nameplates` hide or show nameplates in survival games
+ `sv_survival_start_players` how many players are needed to start a survival game
+ `sv_survival_gun_ammo` whether the tees have gun ammo on spawn or not
+ `sv_survival_lobby_delay` lobby start delay
+ `sv_survival_dm_players` if less than x players start the deathmatch countdown
+ `sv_survival_dm_delay` after how many minutes the deathmatch should start 0=off
+ `sv_survival_max_game_time` after how many minutes the game should end without winners 0=never
+ `sv_allow_survival` allow survival command
+ `sv_survival_kill_protection` 0=off 1=allowed in lobby 2=full on
+ `sv_vanilla_shotgun` Fix tunings for vanilla shotgun (breaks bullet tiles)
+ `sv_allow_block_tourna` 0=off 1=allow blocktournaments minigame
+ `sv_block_tourna_players` players needed to start an block tournament
+ `sv_block_tourna_delay` how long players can '/join' block tournaments (lobby time)
+ `sv_block_tourna_game_time` how long block tournas can take in minutes (if after this time nobody won its draw)
+ `sv_block_tourna_max_per_ip` How many players per ip can join the block tournament
+ `sv_block_dm_arena` 1=arena1 2=arena2
+ `sv_allow_block_1vs1` 0=off 1=allow /1vs1 command
+ `sv_1vs1_anti_ground_hook` 0=off after how many seconds of ground hooking the player will be frozen in /1vs1 minigame
+ `sv_allow_block_wave` 0=off 1=allow blockwave minigame 2=only logged in
+ `sv_adventure_bots` number of vanilla pvp bots to spawn at TILE_BOTSPAWN_1
+ `sv_mine_tee_hammer` break blocks near by with hammer
+ `sv_mine_tee_editor` custom clients can place any tile anywhere
+ `sv_fnn_start_x` where the dmm25 start because spawn points differ
+ `sv_fnn_timeout` after how many ticks the bot should restart 0=never
+ `sv_allow_global_chat` 0=off 1=messsages with @all in front get pushed into txt file
+ `sv_global_chat_servers` how many servers are connected to the global chat (needed for print confirmation)
+ `sv_cfg_tile_1` use chat cmd for help '/admin cfg_tiles'
+ `sv_cfg_tile_2` use chat cmd for help '/admin cfg_tiles'
+ `sv_freeze_farm` wether tees have to be unfrozen or not to use money/xp tiles
+ `sv_max_police_farm_players` deactivates all police tiles if too many players are on them (0 = no limit)
+ `sv_teleportation_delay` force players to stand still a few seconds before joining minigames
+ `sv_sup_acc_reset` allow supporters cmds 1='/sql_logout' 2=1 and '/sql_logout_all'
+ `sv_save_wrong_rcon` saves wrong rcons in the wrong_rcon.txt (sv_wrong_rcon_file) file
+ `sv_save_wrong_login` saves wrong '/login's in the wrong_login.txt (sv_wrong_login_file) file
+ `sv_spamfilter_mode` 0=off 1=silent 2=error 3=ghost spamfilters.txt drop mode (see also add_spamfilter)
+ `sv_require_chat_flag_to_register` require clients to send playerflag chat to be able to use /register command
+ `sv_require_chat_flag_to_chat` clients have to send playerflag chat to use public chat (commands are unrelated)
+ `sv_captcha_score` how high the captcha score has to be to verfiy humans ('/captcha')
+ `sv_register_human_level` mimum human level to use register command
+ `sv_login_human_level` mimum human level to use login command
+ `sv_chat_human_level` mimum human level to use the chat
+ `sv_max_register_per_ip` how many accounts one ip can register before getting rate limited
+ `sv_max_login_per_ip` failed login attempts before getting rate limited (after 3 for 1min anyways)
+ `sv_max_namechanges_per_ip` how many times one ip can change the name (hourly) before the msg gets hidden
+ `sv_hide_rename_msg` show the '%s' -> '%s' message in logs (can get really spammy if players have rainbow skin/clan)
+ `sv_show_connection_msg` 0=none 1=join 2=leave 3=join/leave/spec (specified messages are shown)
+ `sv_auto_anti_reconnect_flood` delay join/leave messages if too many connections happend (see also sv_show_connection_msg)
+ `sv_rcon_attempt_report` after how many failed rcon attempts in a row should it be reported
+ `sv_captcha_room` needs captcha spawn and verify tile in the map
+ `sv_client_suggestion_supported` Broadcast to display to players that their client isnt fully supported
+ `sv_minigame_default_skin` Change default skin in minigames
+ `sv_chidraqul_default_skin` Change default skin in chidraqul
+ `sv_room_price` changes the price for the '/shop' item 'room_key'
+ `sv_ad_string` advertisement shown at adv places xd
+ `sv_database_path` path/to/sqlite3_database.db used to save DDNet++ data like accounts
+ `sv_chat_discord_webhook` If set will post all chat messages there
+ `sv_display_score` values: time, level, block, current_spree, king_of_the_hill
+ `sv_language` en, ru
+ `sv_game_type_name` Displayed in server browser when sv_test_cmds is 0
+ `sv_game_type_name_test` Displayed in server browser when sv_test_cmds is 1
+ `sv_ddpp_gametype` gametypes: '', 'block', 'fly', 'survival', 'vanilla', 'fng', 'battlegores'
+ `sv_mine_tee_out_maps_dir` folder in which the minetee maps are written to that are created when the map is edited (require restart)
+ `sv_source_root_dir` absolute path to ddnet++ source code to run scripts from (require restart)
+ `sv_file_acc_path` the path were the server searches the .acc files (no / after last directory)
+ `sv_global_chat_file` path/to/file.txt where the global chat messages get pushed and pulled
+ `sv_wrong_rcon_file` path/to/file.txt where the wrong rcon logins are stored
+ `sv_wrong_login_file` path/to/file.txt where the wrong account logins are stored
+ `sv_rcon_honey_password` Remote console password for honeypot (should have no real access)
+ `sv_rcon_fake_password` Remote console password for fake rcon (only send auth to client and do nothing on srv)
+ `sv_hide_connection_msg_pattern` Names matching this regex pattern won't appear in chat on connect/disconnect/spec (\"\"=off))

# Rcon commands

+ `hammer` Gives a hammer to you
+ `old_rainbow` (old) activates rainbow until death
+ `inf_rainbow` activates rainbow until disconnect
+ `old_bloody` (old) activates bloody until death
+ `inf_bloody` activates bloody until disconnect
+ `old_atom` (old) activates atom until death
+ `inf_atom` activates atom until disconnect
+ `old_autospreadgun` activates spread gun until death
+ `inf_autospreadgun` activates spread gun until disconnect
+ `old_trail` (old) activates trail until death
+ `inf_trail` activates trail until disconnect
+ `homing_missile` toggles homing missile activate/deactive (grenade launcher)
+ `disarm` disarm a evil tee to prevent him doing evil stuff :)
+ `dummymode` 0 default, 23 cb5(racer), 29 cb5(blocker), 103 ctf5, 27 blmapchill(policebot), 32 blmapchill(solo policebot)
+ `dummy_color` changes the color of a specific dummy
+ `dummy_skin` changes the skin of a specific dummy
+ `force_color` changes the color of a specific player
+ `force_skin` changes the skin of a specific player
+ `heal` heals a tee's Health to 10 hp
+ `force_unjail` unjails player instantly
+ `force_jail` jails player by adminforce
+ `ninjasteam` activates a awesome NINJASTEAM
+ `hammerfightmode` activates hammerfightmode for a player
+ `freeze_shotgun` Gives you a freeze Shotgun
+ `Damage` Makes a player vulnerable
+ `register_ban` Use either 'register_ban_id <client_id> <seconds>' or 'register_ban_ip <ip> <seconds>'
+ `register_ban_id` Ban a player from the registration system
+ `register_ban_ip` Ban a player ip from the registration system
+ `unregister_ban` Unban a player from the registration system
+ `register_bans` Unban a player ip from the registration system
+ `login_ban` Use either 'login_ban_id <client_id> <seconds>' or 'login_ban_ip <ip> <seconds>'
+ `login_ban_id` Ban a player from the login system
+ `login_ban_ip` Ban a player ip from the login system
+ `unlogin_ban` Unban a player from the login system
+ `login_bans` Unban a player ip from the login system
+ `namechange_mute` Use either 'namechange_mute_id <client_id> <seconds>' or 'namechange_mute_ip <ip> <seconds>'
+ `namechange_mute_id` Mute a player from changing name system by client ID
+ `namechange_mute_ip` Mute a player from changing name system by IP address
+ `namechange_unmute` Unmute a player from name change system
+ `namechange_mutes` List all players muted from changing name system
+ `dummies` List all connected dummy
+ `block_votes` disables all votes but 'unblock_votes'
+ `unblock_votes` unlocks votes if they were blocked by 'block_votes'
+ `freezelaser` Gives a player Freeze Laser
+ `freezehammer` Gives a player Freeze Hammer
+ `heartgun` Gives a player heart gun
+ `unfreeze` Unfreezes player v
+ `freeze` Freezes player v for i seconds (infinite by default)
+ `godmode` gives player i godmode (no damage in instagib)
+ `hide_player` makes player invisible
+ `verify_player` manually set a targets player human level to max (force solve captcha)
+ `logs` shows ddnet++ logs (types: mastersrv)
+ `reload_spamfilters` reads spamfilters.txt (see also add_spamfilter)
+ `add_spamfilter` writes to spamfilters.txt (see also list_spamfilters)
+ `list_spamfilters` prints actice spamfilters (see also reload_spamfilters)
+ `sql_add` adds an new column to the table
+ `set_shop_item_price` sets the price of a shop item
+ `set_shop_item_description` sets the description of a shop item
+ `set_shop_item_level` sets the needed level of a shop item
+ `activate_shop_item` activate shop item
+ `deactivate_shop_item` deactivate shop item
+ `deactivate_all_shop_items` deactivates all shop items
+ `activate_all_shop_items` activates all shop items
+ `run_test` WARNING WILL KILL THE SERVER! run integration tests (used in CI)
+ `defer` Run a cmd after the server fully initialized. Run cmds from cfg or cli that would crash if executed too early.
+ `rcon_api_say_id` RCON API command dont use it
+ `rcon_api_alter_table` RCON API command dont use it

# Chat commands

+ `changepassword` change your account password with '/changepassword <old password> <new password> <new password repeat>'
+ `register` Register an sql account
+ `login` Login an sql account
+ `register2` Register an filebased account
+ `login2` Login an filebased account
+ `logout` Logout from your account
+ `SQL` SQL admistration (using sql id)
+ `SQL_name` SQL admistration (using acc name)
+ `SQL_logout` SQL admistration (doesnt really log out the player but sets his sql state to loggedout)
+ `SQL_logout_all` SQL admistration (doesnt really log out the player but sets his sql state to loggedout)
+ `acc2` filebased acc sys admistration (using usernames)
+ `acc_info` (admin-cmd) shows deeper information about accounts
+ `stats` shows the stats of the player r
+ `profile` player profiles more help at '/profile help'
+ `ascii` create ascii animations with it
+ `policechat` get more attention in chat with this command
+ `policehelper` '/policehelper help' for more help
+ `jail` police command
+ `jail_code` police command shows jail code of player
+ `report` report players and police will arrest em
+ `taser` everything about taser
+ `wanted` shows a list of wanted players. help the police to catch em
+ `togglespawn` switches the spawnpoint between normal and supermod spawn
+ `show` shows info. partner of '/hide' command.
+ `hide` hides info. partner of '/show' command.
+ `score` changes score of players to time (race), level or block (points)
+ `spawnweapons` toggles using spawnweapons or not
+ `spawnweaponsinfo` shows level of spawn weapons
+ `spookyghostinfo` help for spooky ghost
+ `shop` Shows the list of items that you can '/buy'
+ `buy` Buy something. To see all buyable items check '/shop'
+ `bank` for more info check '/bank'
+ `gangsterbag` only for real gangstazzZ
+ `money` shows your money and last transactions
+ `pay` give i amount of your '/money' to player r
+ `gift` send the player r money more info at '/gift'
+ `trade` trade weapons and other items with players on the server
+ `tr` the unsave short command for trade (warning don't use if you don't know what you are doing)
+ `chidraqul` chidraqul3 (minigame) more info '/chidraqul info'
+ `minigames` show stats and informations about all minigames
+ `pvp_arena` teleports you in the pvp-arena (you can die there)
+ `bomb` join, create and leave bomb games and more. more help at '/bomb help'
+ `insta` play instagib games like gdm or idm
+ `join` join the current event
+ `block` join block deathmatch
+ `balance` battle other players in tee balancing
+ `survival` play infamous survival mod
+ `blockwave` play block agianst bot waves
+ `1vs1` challenge one player to a block duel
+ `leave` leave the current minigame
+ `rainbow` accep/turn-off rainbow
+ `bloody` accep/turn-off bloody
+ `atom` accep/turn-off atom
+ `trail` accep/turn-off trail
+ `spread_gun` accep/turn-off spread_gun
+ `drop_health` cosmetic staff command to drop health
+ `drop_armor` cosmetic staff command to drop armor
+ `give` give extras to others or yourself.
+ `policeinfo` Get all info about police
+ `AccountInfo` shows info on how to register and login
+ `OfferInfo` shows info and stats about cosmetic offers
+ `event` shows running events
+ `viewers` shows your current stalker/fangrills
+ `ip` shows your own ip
+ `changelog` shows info about the different ddnet++ versions
+ `say_srv` says something as server
+ `broadcast_srv` broadcasts something as server
+ `hook` modify your hook with this command
+ `lasertext` write some text in the world (lasertext from fng)
+ `loltext` write some text in the world (lasertext from fng)
+ `market` buy and sell virtual goods in the unregulated free market
+ `captcha` use this command to proof your not a robot
+ `human_level` information about human level
+ `lang` set language
+ `poop` throw shit at the player r. Warning: you lose that shit.
+ `room` supermoderator command '/room help' for help
+ `spawn` teleport to spawn without dieing (costs money)
+ `quest` play little quest and earn rewards.
+ `bounty` blocker hitman command
+ `fng` configurate some fng settings '/fng help' for more help
+ `dcdummy` disconnect dummy by id
+ `166` command for admins TROLL166
+ `420` command for admins TROLL420
+ `tcmd3000` secret test command dont use
+ `flood` anti flood command
+ `admin` command for admins
+ `fnn` command for admins (fake neural network)
+ `a` allows communication between admins only
+ `live` get live stats of player r
+ `regex` test regex patterns
+ `mapsave` save current map status
+ `cc` clear the chat by spamming fake connection messages

# More

DDNet++ is based on ddnet so it also inherits all settings and commands from ddnet.
Make sure to check the [ddnet documentation](https://ddnet.org/settingscommands/) for those.

