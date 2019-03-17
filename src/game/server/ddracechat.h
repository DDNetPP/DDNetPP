/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_DDRACECOMMANDS_H
#define GAME_SERVER_DDRACECOMMANDS_H
#undef GAME_SERVER_DDRACECOMMANDS_H // this file can be included several times
#ifndef CHAT_COMMAND
#define CHAT_COMMAND(name, params, flags, callback, userdata, help, ddpp_al)
//ddpp_al = ddnet++ access level 0=all 1=vip 2=vip+ 3=rcon_authed
#endif

CHAT_COMMAND("credits", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConCredits, this, "Shows the credits of the DDRace mod", 0)
CHAT_COMMAND("emote", "?si", CFGFLAG_CHAT|CFGFLAG_SERVER, ConEyeEmote, this, "Sets your tee's eye emote", 0)
CHAT_COMMAND("eyeemote", "?s", CFGFLAG_CHAT|CFGFLAG_SERVER, ConSetEyeEmote, this, "Toggles use of standard eye-emotes on/off, eyeemote s, where s = on for on, off for off, toggle for toggle and nothing to show current status", 0)
CHAT_COMMAND("settings", "?s", CFGFLAG_CHAT|CFGFLAG_SERVER, ConSettings, this, "Shows gameplay information for this server", 0)
CHAT_COMMAND("help", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConHelp, this, "Shows help to command r, general help if left blank", 0)
CHAT_COMMAND("info", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConInfo, this, "Shows info about this server", 0)
CHAT_COMMAND("cc", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConCC, this, "clears the chat", 3)
CHAT_COMMAND("me", "r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConMe, this, "Like the famous irc command '/me says hi' will display '<yourname> says hi'", 0)
CHAT_COMMAND("w", "sr", CFGFLAG_CHAT|CFGFLAG_SERVER, ConWhisper, this, "Whisper something to someone (private message)", 0)
CHAT_COMMAND("whisper", "sr", CFGFLAG_CHAT|CFGFLAG_SERVER, ConWhisper, this, "Whisper something to someone (private message)", 0)
CHAT_COMMAND("c", "r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConConverse, this, "Converse with the last person you whispered to (private message)", 0)
CHAT_COMMAND("converse", "r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConConverse, this, "Converse with the last person you whispered to (private message)", 0)
CHAT_COMMAND("pause", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTogglePause, this, "Toggles pause", 0)
CHAT_COMMAND("spec", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConToggleSpec, this, "Toggles spec (if not activated on the server, it toggles pause)", 0)
CHAT_COMMAND("dnd", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConDND, this, "Toggle Do Not Disturb (no chat and server messages)", 0)
CHAT_COMMAND("mapinfo", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConMapInfo, this, "Show info about the map with name r gives (current map by default)", 0)
CHAT_COMMAND("timeout", "s", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTimeout, this, "Set timeout protection code s", 0)
CHAT_COMMAND("save", "r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConSave, this, "Save team with code r", 0)
CHAT_COMMAND("load", "r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConLoad, this, "Load with code r", 0)
CHAT_COMMAND("map", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConMap, this, "Vote a map by name", 0)
CHAT_COMMAND("rankteam", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTeamRank, this, "Shows the team rank of player with name r (your team rank by default)", 0)
CHAT_COMMAND("teamrank", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTeamRank, this, "Shows the team rank of player with name r (your team rank by default)", 0)
CHAT_COMMAND("rank", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConRank, this, "Shows the rank of player with name r (your rank by default)", 0)
CHAT_COMMAND("rules", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConRules, this, "Shows the server rules", 0)
CHAT_COMMAND("team", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConJoinTeam, this, "Lets you join team i (shows your team if left blank)", 0)
CHAT_COMMAND("lock", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConLockTeam, this, "Lock team so noone else can join it", 0)
CHAT_COMMAND("top5team", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTeamTop5, this, "Shows five team ranks of the ladder beginning with rank i (1 by default)", 0)
CHAT_COMMAND("teamtop5", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTeamTop5, this, "Shows five team ranks of the ladder beginning with rank i (1 by default)", 0)
CHAT_COMMAND("top5", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTop5, this, "Shows five ranks of the ladder beginning with rank i (1 by default)", 0)
CHAT_COMMAND("showothers", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConShowOthers, this, "Whether to show players from other teams or not (off by default), optional i = 0 for off else for on", 0)
CHAT_COMMAND("specteam", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConSpecTeam, this, "Whether to show players from other teams when spectating (on by default), optional i = 0 for off else for on", 0)
CHAT_COMMAND("ninjajetpack", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConNinjaJetpack, this, "Whether to use ninja jetpack or not. Makes jetpack look more awesome", 0)
CHAT_COMMAND("saytime", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConSayTime, this, "Privately messages someone's current time in this current running race (your time by default)", 0)
CHAT_COMMAND("saytimeall", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConSayTimeAll, this, "Publicly messages everyone your current time in this current running race", 0)
CHAT_COMMAND("time", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTime, this, "Privately shows you your current time in this current running race in the broadcast message", 0)
CHAT_COMMAND("timer", "?s", CFGFLAG_CHAT|CFGFLAG_SERVER, ConSetTimerType, this, "Personal Setting of showing time in either broadcast or game/round timer, timer s, where s = broadcast for broadcast, gametimer for game/round timer, cycle for cycle, both for both, none for no timer and nothing to show current status", 0)
CHAT_COMMAND("r", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConRescue, this, "Teleport yourself out of freeze (use sv_rescue 1 to enable this feature)", 0)
CHAT_COMMAND("rescue", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConRescue, this, "Teleport yourself out of freeze (use sv_rescue 1 to enable this feature)", 0)

CHAT_COMMAND("kill", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConProtectedKill, this, "Kill yourself", 0)

//ChillerDragon
//DDNetPlusPlus (DDNet++)

//account stuff
CHAT_COMMAND("changepassword", "?sss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConChangePassword, this, "change your account password with '/password <old password> <new password> <new password repeat>'", 0)
CHAT_COMMAND("register", "?sss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRegister, this, "Register an sql account", 0)
CHAT_COMMAND("login", "?ss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLogin, this, "Login an sql account", 0)
CHAT_COMMAND("register2", "?sss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRegister2, this, "Register an filebased account", 0)
CHAT_COMMAND("login2", "?ss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLogin2, this, "Login an filebased account", 0)
CHAT_COMMAND("acc_logout", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAccLogout, this, "Logout from your account.", 0)
CHAT_COMMAND("SQL", "?s?i?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSQL, this, "SQL admistration (using sql id)", 3)
CHAT_COMMAND("SQL_name", "?sss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSQLName, this, "SQL admistration (using acc name)", 3)
CHAT_COMMAND("SQL_logout", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSQLLogout, this, "SQL admistration (doesnt really log out the player but sets his sql state to loggedout)", 3)
CHAT_COMMAND("SQL_logout_all", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSQLLogoutAll, this, "SQL admistration (doesnt really log out the player but sets his sql state to loggedout)", 3)
CHAT_COMMAND("acc2", "?s?s?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConACC2, this, "filebased acc sys admistration (using usernames)", 3)
CHAT_COMMAND("acc_info", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAcc_Info, this, "(admin-cmd) shows deeper information about accounts", 3)
CHAT_COMMAND("stats", "?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConStats, this, "shows the stats of the player r", 0)
CHAT_COMMAND("profile", "?sr", CFGFLAG_CHAT | CFGFLAG_SERVER, ConProfile, this, "player profiles more help at '/profile help'", 0)
CHAT_COMMAND("ascii", "?s?i?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAscii, this, "create ascii animations with it", 0)

//police
CHAT_COMMAND("policechat", "r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPoliceChat, this, "get more attention in chat with this command", 0)
//CHAT_COMMAND("policetaser", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPolicetaser, this, "turn taser on/off")
CHAT_COMMAND("policehelper", "?s?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPolicehelper, this, "'/policehelper help' for more help", 0)
CHAT_COMMAND("jail", "?s?i?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConJail, this, "police command", 0)
CHAT_COMMAND("report", "?sr", CFGFLAG_CHAT | CFGFLAG_SERVER, ConReport, this, "report players and police will arrest em", 0)

CHAT_COMMAND("taser", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTaser, this, "everything about taser", 0)
CHAT_COMMAND("wanted", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConWanted, this, "shows a list of wanted players. help the police to catch em", 0)

//toggles
CHAT_COMMAND("togglejailmsg", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTogglejailmsg, this, "turns the information about arrest and escape time on/off", 0)
CHAT_COMMAND("togglexpmsg", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConToggleXpMsg, this, "turns the xp/money messages you get from moneytiles/flags on or off", 0)
CHAT_COMMAND("togglespawn", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConToggleSpawn, this, "switches the spawnpoint between normal and supermod spawn", 0)
CHAT_COMMAND("show", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConShow, this, "shows info. partner of '/hide' command.", 0)
CHAT_COMMAND("hide", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConHide, this, "hides info. partner of '/show' command.", 0)

//score display
CHAT_COMMAND("score", "?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConScore, this, "changes score of players to race time, level or blockpoints", 0)

//spawn weapons
CHAT_COMMAND("spawnweapons", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSpawnWeapons, this, "toggles using spawnweapons or not", 0)
CHAT_COMMAND("spawnweaponsinfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSpawnWeaponsInfo, this, "shows level of spawn weapons", 0)

// spooky ghost
CHAT_COMMAND("spookyghostinfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSpookyGhostInfo, this, "help for spooky ghost", 0)

//money
CHAT_COMMAND("shop", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConShop, this, "Shows the list of items that you can '/buy'", 0)
CHAT_COMMAND("buy", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBuy, this, "Buy something. To see all buyable items check '/shop'", 0)
CHAT_COMMAND("bank", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBank, this, "for more info check '/bank'", 0)
CHAT_COMMAND("gangsterbag", "?sr", CFGFLAG_CHAT | CFGFLAG_SERVER, ConGangsterBag, this, "only for real gangstazzZ", 0)
CHAT_COMMAND("money", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConMoney, this, "shows your money and last transactions", 0)
CHAT_COMMAND("pay", "?ir", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPay, this, "give i amount of your '/money' to player r", 0)
CHAT_COMMAND("gift", "?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConGift, this, "send the player r money more info at '/gift'", 0)
CHAT_COMMAND("trade", "?ssir", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTrade, this, "trade weapons and other items with players on the server", 0)
CHAT_COMMAND("tr", "?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTr, this, "the unsave short command for trade (warning don't use if you don't know what you are doing)", 0)

//minigames
CHAT_COMMAND("chidraqul", "s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConChidraqul, this, "chidraqul3 (minigame) more info '/chidraqul info'", 0)

CHAT_COMMAND("minigames", "?ss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConMinigames, this, "show stats and informations about all minigames", 0)
CHAT_COMMAND("pvp_arena", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPvpArena, this, "teleports you in the pvp-arena (you can die there)", 0)
CHAT_COMMAND("bomb", "?s?i?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBomb, this, "join, create and leave bomb games and more. more help at '/bomb help'", 0)
CHAT_COMMAND("insta", "?ssr", CFGFLAG_CHAT | CFGFLAG_SERVER, ConInsta, this, "play instagib games like gdm or idm", 0)
CHAT_COMMAND("join", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConJoin, this, "join the current event", 0)
CHAT_COMMAND("block", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBlock, this, "join block deathmatch", 0)
CHAT_COMMAND("balance", "?sr", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBalance, this, "battle other players in tee balancing", 0)
CHAT_COMMAND("survival", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSurvival, this, "play infamous survival mod", 0)
CHAT_COMMAND("blockwave", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBlockWave, this, "play block agianst bot waves", 0)

//extras
CHAT_COMMAND("rainbow", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRainbow, this, "accep/turn-off rainbow", 0)
CHAT_COMMAND("bloody", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBloody, this, "accep/turn-off bloody", 0)
CHAT_COMMAND("atom", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAtom, this, "accep/turn-off atom", 0)
CHAT_COMMAND("trail", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTrail, this, "accep/turn-off trail", 0)
CHAT_COMMAND("spread_gun", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAutoSpreadGun, this, "accep/turn-off spread_gun", 0)

CHAT_COMMAND("give", "?sr", CFGFLAG_CHAT | CFGFLAG_SERVER, ConGive, this, "give extras to others or yourself.", 1)

//info
//CHAT_COMMAND("taserinfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTaserinfo, this, "Shows info about the taser")
CHAT_COMMAND("policeinfo", "?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPoliceInfo, this, "Get all info about police", 0)
CHAT_COMMAND("AccountInfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAccountInfo, this, "shows info on how to register and login", 0)
//CHAT_COMMAND("ProfileInfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConProfileInfo, this, "shows info about the profile commands")
CHAT_COMMAND("OfferInfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConOfferInfo, this, "shows info and stats about cosmetic offers", 0)
CHAT_COMMAND("event", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConEvent, this, "shows running events", 0)
CHAT_COMMAND("viewers", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConViewers, this, "shows your current stalker/fangrills", 0)
CHAT_COMMAND("ip", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConIp, this, "shows your own ip", 0)

CHAT_COMMAND("changelog", "?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConChangelog, this, "shows info about the different ddnet++ versions", 0)


//VIP and VIP+
CHAT_COMMAND("say_srv", "r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSayServer, this, "says something as server", 1)
CHAT_COMMAND("broadcast_srv", "r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBroadcastServer, this, "broadcasts something as server", 2)
CHAT_COMMAND("hook", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConHook, this, "modify your hook with this command", 2)

//Others
CHAT_COMMAND("StockMarket", "s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConStockMarket, this, "buy and sell share values with this command", 0)

CHAT_COMMAND("poop", "?ir", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPoop, this, "throw shit at the player r. Warning: you lose that shit.", 0)


CHAT_COMMAND("room", "?sr", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRoom, this, "supermoderator command '/room help' for help", 0)

CHAT_COMMAND("quest", "?si", CFGFLAG_CHAT | CFGFLAG_SERVER, ConQuest, this, "play little quest and earn rewards.", 0)
CHAT_COMMAND("bounty", "?sii", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBounty, this, "blocker hitman command", 0)
CHAT_COMMAND("fng", "?si", CFGFLAG_CHAT | CFGFLAG_SERVER, ConFng, this, "configurate some fng settings '/fng help' for more help", 0)

//CHAT_COMMAND("afk", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAfk, this, "Shows others that you are away-from-keyboard")


//admin
CHAT_COMMAND("166", "vi", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTROLL166, this, "command for admins TROLL166", 3)
CHAT_COMMAND("420", "vi", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTROLL420, this, "command for admins TROLL420", 3)
CHAT_COMMAND("tcmd3000", "?sis", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTCMD3000, this, "secret test command dont use", 3)
CHAT_COMMAND("flood", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAntiFlood, this, "anti flood command", 3)
CHAT_COMMAND("admin", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAdmin, this, "command for admins", 3)
CHAT_COMMAND("fnn", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConFNN, this, "command for admins (fake neural network)", 3)
CHAT_COMMAND("a", "?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAdminChat, this, "allows communication between admins only", 3)
CHAT_COMMAND("live", "?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLive, this, "get live stats of player r", 3)

CHAT_COMMAND("points", "?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPoints, this, "Shows the points of a player beginning with name r (your rank by default)", 0)
#if defined(CONF_SQL)
CHAT_COMMAND("times", "?s?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTimes, this, "/times ?s?i shows last 5 times of the server or of a player beginning with name s starting with time i (i = 1 by default)")
//CHAT_COMMAND("points", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConPoints, this, "Shows the global points of a player beginning with name r (your rank by default)")
CHAT_COMMAND("top5points", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTopPoints, this, "Shows five points of the global point ladder beginning with rank i (1 by default)")
#endif
#undef CHAT_COMMAND

#endif
