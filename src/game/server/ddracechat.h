/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */

// This file can be included several times.

#ifndef CHAT_COMMAND
#define CHAT_COMMAND(name, params, flags, callback, userdata, help, ddpp_al)
//ddpp_al = ddnet++ access level 0=all 1=vip 2=vip+ 3=rcon_authed
#endif

CHAT_COMMAND("credits", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConCredits, this, "Shows the credits of the DDNet mod")
CHAT_COMMAND("rules", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRules, this, "Shows the server rules")
CHAT_COMMAND("emote", "?s[emote name] i[duration in seconds]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConEyeEmote, this, "Sets your tee's eye emote")
CHAT_COMMAND("eyeemote", "?s['on'|'off'|'toggle']", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSetEyeEmote, this, "Toggles use of standard eye-emotes on/off, eyeemote s, where s = on for on, off for off, toggle for toggle and nothing to show current status")
CHAT_COMMAND("settings", "?s[configname]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSettings, this, "Shows gameplay information for this server")
CHAT_COMMAND("help", "?r[command]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConHelp, this, "Shows help to command r, general help if left blank")
CHAT_COMMAND("info", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConInfo, this, "Shows info about this server")
CHAT_COMMAND("list", "?s[filter]", CFGFLAG_CHAT, ConList, this, "List connected players with optional case-insensitive substring matching filter")
CHAT_COMMAND("me", "r[message]", CFGFLAG_CHAT | CFGFLAG_SERVER | CFGFLAG_NONTEEHISTORIC, ConMe, this, "Like the famous irc command '/me says hi' will display '<yourname> says hi'")
CHAT_COMMAND("w", "s[player name] r[message]", CFGFLAG_CHAT | CFGFLAG_SERVER | CFGFLAG_NONTEEHISTORIC, ConWhisper, this, "Whisper something to someone (private message)")
CHAT_COMMAND("whisper", "s[player name] r[message]", CFGFLAG_CHAT | CFGFLAG_SERVER | CFGFLAG_NONTEEHISTORIC, ConWhisper, this, "Whisper something to someone (private message)")
CHAT_COMMAND("c", "r[message]", CFGFLAG_CHAT | CFGFLAG_SERVER | CFGFLAG_NONTEEHISTORIC, ConConverse, this, "Converse with the last person you whispered to (private message)")
CHAT_COMMAND("converse", "r[message]", CFGFLAG_CHAT | CFGFLAG_SERVER | CFGFLAG_NONTEEHISTORIC, ConConverse, this, "Converse with the last person you whispered to (private message)")
CHAT_COMMAND("pause", "?r[player name]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTogglePause, this, "Toggles pause")
CHAT_COMMAND("spec", "?r[player name]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConToggleSpec, this, "Toggles spec (if not available behaves as /pause)")
CHAT_COMMAND("pausevoted", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTogglePauseVoted, this, "Toggles pause on the currently voted player")
CHAT_COMMAND("specvoted", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConToggleSpecVoted, this, "Toggles spec on the currently voted player")
CHAT_COMMAND("dnd", "", CFGFLAG_CHAT | CFGFLAG_SERVER | CFGFLAG_NONTEEHISTORIC, ConDND, this, "Toggle Do Not Disturb (no chat and server messages)")
CHAT_COMMAND("mapinfo", "?r[map]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConMapInfo, this, "Show info about the map with name r gives (current map by default)")
CHAT_COMMAND("timeout", "?s[code]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTimeout, this, "Set timeout protection code s")
CHAT_COMMAND("practice", "?i['0'|'1']", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPractice, this, "Enable cheats (currently only /rescue) for your current team's run, but you can't earn a rank")
CHAT_COMMAND("save", "?r[code]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSave, this, "Save team with code r.")
CHAT_COMMAND("load", "?r[code]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLoad, this, "Load with code r. /load to check your existing saves")
CHAT_COMMAND("map", "?r[map]", CFGFLAG_CHAT | CFGFLAG_SERVER | CFGFLAG_NONTEEHISTORIC, ConMap, this, "Vote a map by name")
CHAT_COMMAND("rankteam", "?r[player name]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTeamRank, this, "Shows the team rank of player with name r (your team rank by default)")
CHAT_COMMAND("teamrank", "?r[player name]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTeamRank, this, "Shows the team rank of player with name r (your team rank by default)")
CHAT_COMMAND("rank", "?r[player name]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRank, this, "Shows the rank of player with name r (your rank by default)")
CHAT_COMMAND("top5team", "?i[rank to start with]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTeamTop5, this, "Shows five team ranks of the ladder beginning with rank i (1 by default, -1 for worst)")
CHAT_COMMAND("teamtop5", "?i[rank to start with]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTeamTop5, this, "Shows five team ranks of the ladder beginning with rank i (1 by default, -1 for worst)")
CHAT_COMMAND("top5", "?i[rank to start with]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTop5, this, "Shows five ranks of the ladder beginning with rank i (1 by default, -1 for worst)")
CHAT_COMMAND("times", "?s[player name] ?i[number of times to skip]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTimes, this, "/times ?s?i shows last 5 times of the server or of a player beginning with name s starting with time i (i = 1 by default, -1 for first)")
CHAT_COMMAND("points", "?r[player name]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPoints, this, "Shows the global points of a player beginning with name r (your rank by default)")
CHAT_COMMAND("top5points", "?i[number]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTopPoints, this, "Shows five points of the global point ladder beginning with rank i (1 by default, -1 for worst)")

CHAT_COMMAND("team", "?i[id]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConJoinTeam, this, "Lets you join team i (shows your team if left blank)")
CHAT_COMMAND("lock", "?i['0'|'1']", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLockTeam, this, "Toggle team lock so no one else can join and so the team restarts when a player dies. /lock 0 to unlock, /lock 1 to lock.")
CHAT_COMMAND("unlock", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConUnlockTeam, this, "Unlock a team")
CHAT_COMMAND("invite", "r[player name]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConInviteTeam, this, "Invite a person to a locked team")

CHAT_COMMAND("showothers", "?i['0'|'1'|'2']", CFGFLAG_CHAT | CFGFLAG_SERVER, ConShowOthers, this, "Whether to show players from other teams or not (off by default), optional i = 0 for off, i = 1 for on, i = 2 for own team only")
CHAT_COMMAND("showall", "?i['0'|'1']", CFGFLAG_CHAT | CFGFLAG_SERVER, ConShowAll, this, "Whether to show players at any distance (off by default), optional i = 0 for off else for on")
CHAT_COMMAND("specteam", "?i['0'|'1']", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSpecTeam, this, "Whether to show players from other teams when spectating (on by default), optional i = 0 for off else for on")
CHAT_COMMAND("ninjajetpack", "?i['0'|'1']", CFGFLAG_CHAT | CFGFLAG_SERVER, ConNinjaJetpack, this, "Whether to use ninja jetpack or not. Makes jetpack look more awesome")
CHAT_COMMAND("saytime", "?r[player name]", CFGFLAG_CHAT | CFGFLAG_SERVER | CFGFLAG_NONTEEHISTORIC, ConSayTime, this, "Privately messages someone's current time in this current running race (your time by default)")
CHAT_COMMAND("saytimeall", "", CFGFLAG_CHAT | CFGFLAG_SERVER | CFGFLAG_NONTEEHISTORIC, ConSayTimeAll, this, "Publicly messages everyone your current time in this current running race")
CHAT_COMMAND("time", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTime, this, "Privately shows you your current time in this current running race in the broadcast message")
CHAT_COMMAND("timer", "?s['gametimer'|'broadcast'|'both'|'none'|'cycle']", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSetTimerType, this, "Personal Setting of showing time in either broadcast or game/round timer, timer s, where s = broadcast for broadcast, gametimer for game/round timer, cycle for cycle, both for both, none for no timer and nothing to show current status")
CHAT_COMMAND("r", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRescue, this, "Teleport yourself out of freeze (use sv_rescue 1 to enable this feature)")
CHAT_COMMAND("rescue", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRescue, this, "Teleport yourself out of freeze (use sv_rescue 1 to enable this feature)")

CHAT_COMMAND("kill", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConProtectedKill, this, "Kill yourself when kill-protected during a long game (use f1, kill for regular kill)")

//ChillerDragon
//DDNetPlusPlus (DDNet++)

//account stuff
CHAT_COMMAND("changepassword", "?sss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConChangePassword, this, "change your account password with '/password <old password> <new password> <new password repeat>'")
CHAT_COMMAND("register", "?sss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRegister, this, "Register an sql account")
CHAT_COMMAND("login", "?ss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLogin, this, "Login an sql account")
CHAT_COMMAND("register2", "?sss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRegister2, this, "Register an filebased account")
CHAT_COMMAND("login2", "?ss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLogin2, this, "Login an filebased account")
CHAT_COMMAND("acc_logout", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAccLogout, this, "Logout from your account.")
CHAT_COMMAND("SQL", "?s?i?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSQL, this, "SQL admistration (using sql id)")
CHAT_COMMAND("SQL_name", "?sss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSQLName, this, "SQL admistration (using acc name)")
CHAT_COMMAND("SQL_logout", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSQLLogout, this, "SQL admistration (doesnt really log out the player but sets his sql state to loggedout)")
CHAT_COMMAND("SQL_logout_all", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSQLLogoutAll, this, "SQL admistration (doesnt really log out the player but sets his sql state to loggedout)")
CHAT_COMMAND("acc2", "?s?s?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConACC2, this, "filebased acc sys admistration (using usernames)")
CHAT_COMMAND("acc_info", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAcc_Info, this, "(admin-cmd) shows deeper information about accounts")
CHAT_COMMAND("stats", "?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConStats, this, "shows the stats of the player r")
CHAT_COMMAND("profile", "?sr", CFGFLAG_CHAT | CFGFLAG_SERVER, ConProfile, this, "player profiles more help at '/profile help'")
CHAT_COMMAND("ascii", "?s?i?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAscii, this, "create ascii animations with it")

//police
CHAT_COMMAND("policechat", "r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPoliceChat, this, "get more attention in chat with this command")
//CHAT_COMMAND("policetaser", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPolicetaser, this, "turn taser on/off")
CHAT_COMMAND("policehelper", "?s?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPolicehelper, this, "'/policehelper help' for more help")
CHAT_COMMAND("jail", "?s?i?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConJail, this, "police command")
CHAT_COMMAND("jail_code", "r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConJailCode, this, "police command shows jail code of player")
CHAT_COMMAND("report", "?sr", CFGFLAG_CHAT | CFGFLAG_SERVER, ConReport, this, "report players and police will arrest em")

CHAT_COMMAND("taser", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTaser, this, "everything about taser")
CHAT_COMMAND("wanted", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConWanted, this, "shows a list of wanted players. help the police to catch em")

//toggles
CHAT_COMMAND("togglespawn", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConToggleSpawn, this, "switches the spawnpoint between normal and supermod spawn")
CHAT_COMMAND("show", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConShow, this, "shows info. partner of '/hide' command.")
CHAT_COMMAND("hide", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConHide, this, "hides info. partner of '/show' command.")

//score display
CHAT_COMMAND("score", "?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConScore, this, "changes score of players to race time, level or blockpoints")

//spawn weapons
CHAT_COMMAND("spawnweapons", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSpawnWeapons, this, "toggles using spawnweapons or not")
CHAT_COMMAND("spawnweaponsinfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSpawnWeaponsInfo, this, "shows level of spawn weapons")

// spooky ghost
CHAT_COMMAND("spookyghostinfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSpookyGhostInfo, this, "help for spooky ghost")

//money
CHAT_COMMAND("shop", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConShop, this, "Shows the list of items that you can '/buy'")
CHAT_COMMAND("buy", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBuy, this, "Buy something. To see all buyable items check '/shop'")
CHAT_COMMAND("bank", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBank, this, "for more info check '/bank'")
CHAT_COMMAND("gangsterbag", "?sr", CFGFLAG_CHAT | CFGFLAG_SERVER, ConGangsterBag, this, "only for real gangstazzZ")
CHAT_COMMAND("money", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConMoney, this, "shows your money and last transactions")
CHAT_COMMAND("pay", "?ir", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPay, this, "give i amount of your '/money' to player r")
CHAT_COMMAND("gift", "?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConGift, this, "send the player r money more info at '/gift'")
CHAT_COMMAND("trade", "?ssir", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTrade, this, "trade weapons and other items with players on the server")
CHAT_COMMAND("tr", "?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTr, this, "the unsave short command for trade (warning don't use if you don't know what you are doing)")

//minigames
CHAT_COMMAND("chidraqul", "s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConChidraqul, this, "chidraqul3 (minigame) more info '/chidraqul info'")

CHAT_COMMAND("minigames", "?ss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConMinigames, this, "show stats and informations about all minigames")
CHAT_COMMAND("pvp_arena", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPvpArena, this, "teleports you in the pvp-arena (you can die there)")
CHAT_COMMAND("bomb", "?s?i?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBomb, this, "join, create and leave bomb games and more. more help at '/bomb help'")
CHAT_COMMAND("insta", "?ssr", CFGFLAG_CHAT | CFGFLAG_SERVER, ConInsta, this, "play instagib games like gdm or idm")
CHAT_COMMAND("join", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConJoin, this, "join the current event")
CHAT_COMMAND("block", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBlock, this, "join block deathmatch")
CHAT_COMMAND("balance", "?sr", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBalance, this, "battle other players in tee balancing")
CHAT_COMMAND("survival", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSurvival, this, "play infamous survival mod")
CHAT_COMMAND("blockwave", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBlockWave, this, "play block agianst bot waves")

//extras
CHAT_COMMAND("rainbow", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRainbow, this, "accep/turn-off rainbow")
CHAT_COMMAND("bloody", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBloody, this, "accep/turn-off bloody")
CHAT_COMMAND("atom", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAtom, this, "accep/turn-off atom")
CHAT_COMMAND("trail", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTrail, this, "accep/turn-off trail")
CHAT_COMMAND("spread_gun", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAutoSpreadGun, this, "accep/turn-off spread_gun")
CHAT_COMMAND("drop_health", "?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConDropHealth, this, "cosmetic staff command to drop health")
CHAT_COMMAND("drop_armor", "?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConDropArmor, this, "cosmetic staff command to drop armor")

CHAT_COMMAND("give", "?sr", CFGFLAG_CHAT | CFGFLAG_SERVER, ConGive, this, "give extras to others or yourself.")

//info
//CHAT_COMMAND("taserinfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTaserinfo, this, "Shows info about the taser")
CHAT_COMMAND("policeinfo", "?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPoliceInfo, this, "Get all info about police")
CHAT_COMMAND("AccountInfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAccountInfo, this, "shows info on how to register and login")
//CHAT_COMMAND("ProfileInfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConProfileInfo, this, "shows info about the profile commands")
CHAT_COMMAND("OfferInfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConOfferInfo, this, "shows info and stats about cosmetic offers")
CHAT_COMMAND("event", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConEvent, this, "shows running events")
CHAT_COMMAND("viewers", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConViewers, this, "shows your current stalker/fangrills")
CHAT_COMMAND("ip", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConIp, this, "shows your own ip")

CHAT_COMMAND("changelog", "?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConChangelog, this, "shows info about the different ddnet++ versions")


//VIP and VIP+
CHAT_COMMAND("say_srv", "r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSayServer, this, "says something as server")
CHAT_COMMAND("broadcast_srv", "r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBroadcastServer, this, "broadcasts something as server")
CHAT_COMMAND("hook", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConHook, this, "modify your hook with this command")

//Others
CHAT_COMMAND("StockMarket", "s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConStockMarket, this, "buy and sell share values with this command")
CHAT_COMMAND("captcha", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConCaptcha, this, "use this command to proof your not a robot")
CHAT_COMMAND("human_level", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConHumanLevel, this, "information about human level")

CHAT_COMMAND("poop", "?ir", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPoop, this, "throw shit at the player r. Warning: you lose that shit.")


CHAT_COMMAND("room", "?sr", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRoom, this, "supermoderator command '/room help' for help")
CHAT_COMMAND("spawn", "?sr", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSpawn, this, "teleport to spawn without dieing (costs money)")

CHAT_COMMAND("quest", "?si", CFGFLAG_CHAT | CFGFLAG_SERVER, ConQuest, this, "play little quest and earn rewards.")
CHAT_COMMAND("bounty", "?sii", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBounty, this, "blocker hitman command")
CHAT_COMMAND("fng", "?si", CFGFLAG_CHAT | CFGFLAG_SERVER, ConFng, this, "configurate some fng settings '/fng help' for more help")

//CHAT_COMMAND("afk", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAfk, this, "Shows others that you are away-from-keyboard")


//admin
CHAT_COMMAND("dcdummy", "i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConDcDummy, this, "disconnect dummy by id")
CHAT_COMMAND("166", "vi", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTROLL166, this, "command for admins TROLL166")
CHAT_COMMAND("420", "vi", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTROLL420, this, "command for admins TROLL420")
CHAT_COMMAND("tcmd3000", "?sis", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTCMD3000, this, "secret test command dont use")
CHAT_COMMAND("flood", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAntiFlood, this, "anti flood command")
CHAT_COMMAND("admin", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAdmin, this, "command for admins")
CHAT_COMMAND("fnn", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConFNN, this, "command for admins (fake neural network)")
CHAT_COMMAND("a", "?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAdminChat, this, "allows communication between admins only")
CHAT_COMMAND("live", "?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLive, this, "get live stats of player r")
CHAT_COMMAND("regex", "?ss[pattern|string]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRegex, this, "test regex patterns")
CHAT_COMMAND("mapsave", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConMapsave, this, "save current map status")

#undef CHAT_COMMAND
