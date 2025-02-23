// DDNet++

// This file can be included several times.

#ifndef CHAT_COMMAND
#define CHAT_COMMAND(name, params, flags, callback, userdata, help)
#endif

//account stuff
CHAT_COMMAND("changepassword", "?sss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConChangePassword, this, "change your account password with '/changepassword <old password> <new password> <new password repeat>'")
CHAT_COMMAND("register", "?sss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRegister, this, "Register an sql account")
CHAT_COMMAND("login", "?ss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLogin, this, "Login an sql account")
CHAT_COMMAND("register2", "?sss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRegister2, this, "Register an filebased account")
CHAT_COMMAND("login2", "?ss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLogin2, this, "Login an filebased account")
CHAT_COMMAND("logout", "", CFGFLAG_CHAT, ConAccLogout, this, "Logout from your account.")
CHAT_COMMAND("SQL", "?s?i?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSql, this, "SQL admistration (using sql id)")
CHAT_COMMAND("SQL_name", "?sss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSqlName, this, "SQL admistration (using acc name)")
CHAT_COMMAND("SQL_logout", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSqlLogout, this, "SQL admistration (doesnt really log out the player but sets his sql state to loggedout)")
CHAT_COMMAND("SQL_logout_all", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSqlLogoutAll, this, "SQL admistration (doesnt really log out the player but sets his sql state to loggedout)")
CHAT_COMMAND("acc2", "?s?s?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConACC2, this, "filebased acc sys admistration (using usernames)")
CHAT_COMMAND("acc_info", "?r[player name]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAcc_Info, this, "(admin-cmd) shows deeper information about accounts")
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
CHAT_COMMAND("score", "?s[time|level|block]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConScore, this, "changes score of players to time (race), level or block (points)")

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
CHAT_COMMAND("1vs1", "s[player name]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConOneVsOneBlock, this, "challenge one player to a block duel")
CHAT_COMMAND("leave", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLeave, this, "leave the current minigame")

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

CHAT_COMMAND("lasertext", "s[text]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLaserText, this, "write some text in the world (lasertext from fng)")
CHAT_COMMAND("loltext", "s[text]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLaserText, this, "write some text in the world (lasertext from fng)")

//Others
CHAT_COMMAND("market", "s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConStockMarket, this, "buy and sell virtual goods in the unregulated free market")
CHAT_COMMAND("captcha", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConCaptcha, this, "use this command to proof your not a robot")
CHAT_COMMAND("human_level", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConHumanLevel, this, "information about human level")
CHAT_COMMAND("lang", "s[en|ru]", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLang, this, "set language")

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
CHAT_COMMAND("cc", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConCC, this, "clear the chat by spamming fake connection messages")
