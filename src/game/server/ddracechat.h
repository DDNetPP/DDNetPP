/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_DDRACECOMMANDS_H
#define GAME_SERVER_DDRACECOMMANDS_H
#undef GAME_SERVER_DDRACECOMMANDS_H // this file can be included several times
#ifndef CHAT_COMMAND
#define CHAT_COMMAND(name, params, flags, callback, userdata, help)
#endif

CHAT_COMMAND("credits", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConCredits, this, "Shows the credits of the DDRace mod")
CHAT_COMMAND("emote", "?si", CFGFLAG_CHAT|CFGFLAG_SERVER, ConEyeEmote, this, "Sets your tee's eye emote")
CHAT_COMMAND("eyeemote", "?s", CFGFLAG_CHAT|CFGFLAG_SERVER, ConSetEyeEmote, this, "Toggles use of standard eye-emotes on/off, eyeemote s, where s = on for on, off for off, toggle for toggle and nothing to show current status")
CHAT_COMMAND("settings", "?s", CFGFLAG_CHAT|CFGFLAG_SERVER, ConSettings, this, "Shows gameplay information for this server")
CHAT_COMMAND("help", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConHelp, this, "Shows help to command r, general help if left blank")
CHAT_COMMAND("info", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConInfo, this, "Shows info about this server")
CHAT_COMMAND("cc", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConCC, this, "clears the chat")
CHAT_COMMAND("me", "r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConMe, this, "Like the famous irc command '/me says hi' will display '<yourname> says hi'")
CHAT_COMMAND("w", "sr", CFGFLAG_CHAT|CFGFLAG_SERVER, ConWhisper, this, "Whisper something to someone (private message)")
CHAT_COMMAND("whisper", "sr", CFGFLAG_CHAT|CFGFLAG_SERVER, ConWhisper, this, "Whisper something to someone (private message)")
CHAT_COMMAND("c", "r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConConverse, this, "Converse with the last person you whispered to (private message)");
CHAT_COMMAND("converse", "r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConConverse, this, "Converse with the last person you whispered to (private message)");
CHAT_COMMAND("pause", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTogglePause, this, "Toggles pause")
CHAT_COMMAND("spec", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConToggleSpec, this, "Toggles spec (if not activated on the server, it toggles pause)")
CHAT_COMMAND("dnd", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConDND, this, "Toggle Do Not Disturb (no chat and server messages)")
CHAT_COMMAND("mapinfo", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConMapInfo, this, "Show info about the map with name r gives (current map by default)")
CHAT_COMMAND("timeout", "s", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTimeout, this, "Set timeout protection code s")
CHAT_COMMAND("save", "r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConSave, this, "Save team with code r")
CHAT_COMMAND("load", "r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConLoad, this, "Load with code r")
CHAT_COMMAND("map", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConMap, this, "Vote a map by name")
CHAT_COMMAND("rankteam", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTeamRank, this, "Shows the team rank of player with name r (your team rank by default)")
CHAT_COMMAND("teamrank", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTeamRank, this, "Shows the team rank of player with name r (your team rank by default)")
CHAT_COMMAND("rank", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConRank, this, "Shows the rank of player with name r (your rank by default)")
CHAT_COMMAND("rules", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConRules, this, "Shows the server rules")
CHAT_COMMAND("team", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConJoinTeam, this, "Lets you join team i (shows your team if left blank)")
CHAT_COMMAND("lock", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConLockTeam, this, "Lock team so noone else can join it")
CHAT_COMMAND("top5team", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTeamTop5, this, "Shows five team ranks of the ladder beginning with rank i (1 by default)")
CHAT_COMMAND("teamtop5", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTeamTop5, this, "Shows five team ranks of the ladder beginning with rank i (1 by default)")
CHAT_COMMAND("top5", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTop5, this, "Shows five ranks of the ladder beginning with rank i (1 by default)")
CHAT_COMMAND("showothers", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConShowOthers, this, "Whether to show players from other teams or not (off by default), optional i = 0 for off else for on")
CHAT_COMMAND("specteam", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConSpecTeam, this, "Whether to show players from other teams when spectating (on by default), optional i = 0 for off else for on")
CHAT_COMMAND("ninjajetpack", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConNinjaJetpack, this, "Whether to use ninja jetpack or not. Makes jetpack look more awesome")
CHAT_COMMAND("saytime", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConSayTime, this, "Privately messages someone's current time in this current running race (your time by default)")
CHAT_COMMAND("saytimeall", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConSayTimeAll, this, "Publicly messages everyone your current time in this current running race")
CHAT_COMMAND("time", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTime, this, "Privately shows you your current time in this current running race in the broadcast message")
CHAT_COMMAND("timer", "?s", CFGFLAG_CHAT|CFGFLAG_SERVER, ConSetTimerType, this, "Personal Setting of showing time in either broadcast or game/round timer, timer s, where s = broadcast for broadcast, gametimer for game/round timer, cycle for cycle, both for both, none for no timer and nothing to show current status")
CHAT_COMMAND("r", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConRescue, this, "Teleport yourself out of freeze (use sv_rescue 1 to enable this feature)")
CHAT_COMMAND("rescue", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConRescue, this, "Teleport yourself out of freeze (use sv_rescue 1 to enable this feature)")

CHAT_COMMAND("kill", "", CFGFLAG_CHAT|CFGFLAG_SERVER, ConProtectedKill, this, "Kill yourself")

//ChillerDragon
//DDNetPlusPlus (DDNet++)

//account stuff
CHAT_COMMAND("register", "sss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRegister, this, "Register")
CHAT_COMMAND("login", "ss", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLogin, this, "Login")
CHAT_COMMAND("logout", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConLogout, this, "Logout")
CHAT_COMMAND("SQL", "si?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConSQL, this, "SQL admistration")
CHAT_COMMAND("acc_info", "s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAcc_Info, this, "(admin-cmd) shows deeper information about accounts")

//police
CHAT_COMMAND("policechat", "r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPoliceChat, this, "get more attention in chat with this command")
CHAT_COMMAND("policetaser", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPolicetaser, this, "turn taser on/off")

//toggles
CHAT_COMMAND("togglejailmsg", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTogglejailmsg, this, "turns the information about arrest and escape time on/off")
CHAT_COMMAND("togglexpmsg", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConToggleXpMsg, this, "turns the xp/money messages you get from moneytiles/flags on or off")
CHAT_COMMAND("togglespawn", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConToggleSpawn, this, "switches the spawnpoint between normal and supermod spawn")

//money
CHAT_COMMAND("shop", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConShop, this, "Shows the list of items that you can '/buy'")
CHAT_COMMAND("buy", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBuy, this, "Buy something. To see all buyable items check '/shop'")
CHAT_COMMAND("money", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConMoney, this, "shows your money and last transactions")
CHAT_COMMAND("pay", "ir", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPay, this, "give i amount of your '/money' to player r")

//minigames
CHAT_COMMAND("MinigameLeft", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConMinigameLeft, this, "move left in the minigame. (/buy minigame)")
CHAT_COMMAND("MinigameRight", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConMinigameRight, this, "move right in the minigame. (/buy minigame)")
CHAT_COMMAND("MinigameUp", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConMinigameUp, this, "move up in the minigame. (/buy minigame)")
CHAT_COMMAND("MinigameDown", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConMinigameDown, this, "move down in the minigame. (/buy minigame)")

CHAT_COMMAND("pvp_arena", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPvpArena, this, "teleports you in the pvp-arena (you can die there)")
CHAT_COMMAND("bomb", "s?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBomb, this, "join, create and leave bomb games and more. more help at '/bomb help'")

//extras
CHAT_COMMAND("rainbow", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRainbow, this, "accep/turn-off bloody")
CHAT_COMMAND("bloody", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConBloody, this, "accep/turn-off bloody")
CHAT_COMMAND("atom", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAtom, this, "accep/turn-off atom")
CHAT_COMMAND("trail", "?s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTrail, this, "accep/turn-off trail")

CHAT_COMMAND("give", "s?r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConGive, this, "give extras to others or your self.")

//infos
CHAT_COMMAND("minigameinfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConMinigameinfo, this, "Shows info about the minigame")
CHAT_COMMAND("taserinfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTaserinfo, this, "Shows info about the taser")
CHAT_COMMAND("policeinfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPoliceInfo, this, "Get all info about police")
CHAT_COMMAND("AccountInfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAccountInfo, this, "shows info on how to register and login")
CHAT_COMMAND("ProfileInfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConProfileInfo, this, "shows info about the profile commands")
CHAT_COMMAND("OfferInfo", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConOfferInfo, this, "shows info and stats about cosmetic offers")
CHAT_COMMAND("event", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConEvent, this, "shows running events")

CHAT_COMMAND("changelog", "?i", CFGFLAG_CHAT | CFGFLAG_SERVER, ConChangelog, this, "shows infos about the different ddnet++ versions")

//Others
CHAT_COMMAND("StockMarket", "s", CFGFLAG_CHAT | CFGFLAG_SERVER, ConStockMarket, this, "buy and sell share values with this command")

CHAT_COMMAND("poop", "ir", CFGFLAG_CHAT | CFGFLAG_SERVER, ConPoop, this, "throw shit at the player r. Warning: you loose that shit.")





CHAT_COMMAND("tcmd3000", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConTCMD3000, this, "secret test command dont use")

//CHAT_COMMAND("add_policehelper", "r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAddPolicehelper, this, "promote players to policehelper until disconnect.")
//CHAT_COMMAND("remove_policehelper", "r", CFGFLAG_CHAT | CFGFLAG_SERVER, ConRemovePolicehelper, this, "remove the policehelper rank from other players")
//CHAT_COMMAND("afk", "", CFGFLAG_CHAT | CFGFLAG_SERVER, ConAfk, this, "Shows others that you are away-from-keyboard")


#if defined(CONF_SQL)
CHAT_COMMAND("times", "?s?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTimes, this, "/times ?s?i shows last 5 times of the server or of a player beginning with name s starting with time i (i = 1 by default)")
CHAT_COMMAND("points", "?r", CFGFLAG_CHAT|CFGFLAG_SERVER, ConPoints, this, "Shows the global points of a player beginning with name r (your rank by default)")
CHAT_COMMAND("top5points", "?i", CFGFLAG_CHAT|CFGFLAG_SERVER, ConTopPoints, this, "Shows five points of the global point ladder beginning with rank i (1 by default)")
#endif
#undef CHAT_COMMAND

#endif
