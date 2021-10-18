/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_DDRACECOMMANDS_H
#define GAME_SERVER_DDRACECOMMANDS_H
#undef GAME_SERVER_DDRACECOMMANDS_H // this file can be included several times
#ifndef CONSOLE_COMMAND
#define CONSOLE_COMMAND(name, params, flags, callback, userdata, help)
#endif

CONSOLE_COMMAND("kill_pl", "v[id]", CFGFLAG_SERVER, ConKillPlayer, this, "Kills player v and announces the kill")
CONSOLE_COMMAND("totele", "i[number]", CFGFLAG_SERVER|CMDFLAG_TEST, ConToTeleporter, this, "Teleports you to teleporter v")
CONSOLE_COMMAND("totelecp", "i[number]", CFGFLAG_SERVER|CMDFLAG_TEST, ConToCheckTeleporter, this, "Teleports you to checkpoint teleporter v")
CONSOLE_COMMAND("tele", "v[id] ?i[number]", CFGFLAG_SERVER|CMDFLAG_TEST, ConTeleport, this, "Teleports you (or player v) to player i")
CONSOLE_COMMAND("addweapon", "i[weapon-id]", CFGFLAG_SERVER|CMDFLAG_TEST, ConAddWeapon, this, "Gives weapon with id i to you (all = -1, hammer = 0, gun = 1, shotgun = 2, grenade = 3, rifle = 4, ninja = 5)")
CONSOLE_COMMAND("removeweapon", "i[weapon-id]", CFGFLAG_SERVER|CMDFLAG_TEST, ConRemoveWeapon, this, "removes weapon with id i from you (all = -1, hammer = 0, gun = 1, shotgun = 2, grenade = 3, rifle = 4)")
CONSOLE_COMMAND("shotgun", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConShotgun, this, "Gives a shotgun to you")
CONSOLE_COMMAND("grenade", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConGrenade, this, "Gives a grenade launcher to you")
CONSOLE_COMMAND("rifle", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConRifle, this, "Gives a rifle to you")
CONSOLE_COMMAND("jetpack","", CFGFLAG_SERVER|CMDFLAG_TEST, ConJetpack, this, "Gives jetpack to you")
CONSOLE_COMMAND("weapons", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConWeapons, this, "Gives all weapons to you")
CONSOLE_COMMAND("unshotgun", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConUnShotgun, this, "Takes the shotgun from you")
CONSOLE_COMMAND("ungrenade", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConUnGrenade, this, "Takes the grenade launcher you")
CONSOLE_COMMAND("unrifle", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConUnRifle, this, "Takes the rifle from you")
CONSOLE_COMMAND("unjetpack", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConUnJetpack, this, "Takes the jetpack from you")
CONSOLE_COMMAND("unweapons", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConUnWeapons, this, "Takes all weapons from you")
CONSOLE_COMMAND("ninja", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConNinja, this, "Makes you a ninja")
CONSOLE_COMMAND("super", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConSuper, this, "Makes you super")
CONSOLE_COMMAND("unsuper", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConUnSuper, this, "Removes super from you")
CONSOLE_COMMAND("unsolo", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConUnSolo, this, "Puts you out of solo part")
CONSOLE_COMMAND("undeep", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConUnDeep, this, "Puts you out of deep freeze")
CONSOLE_COMMAND("left", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConGoLeft, this, "Makes you move 1 tile left")
CONSOLE_COMMAND("right", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConGoRight, this, "Makes you move 1 tile right")
CONSOLE_COMMAND("up", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConGoUp, this, "Makes you move 1 tile up")
CONSOLE_COMMAND("down", "", CFGFLAG_SERVER|CMDFLAG_TEST, ConGoDown, this, "Makes you move 1 tile down")

CONSOLE_COMMAND("move", "i[x] i[y]", CFGFLAG_SERVER|CMDFLAG_TEST, ConMove, this, "Moves to the tile with x/y-number ii")
CONSOLE_COMMAND("move_raw", "i[x] i[y]", CFGFLAG_SERVER|CMDFLAG_TEST, ConMoveRaw, this, "Moves to the point with x/y-coordinates ii")
CONSOLE_COMMAND("force_pause", "i[id] i[seconds]", CFGFLAG_SERVER, ConForcePause, this, "Force i to pause for i seconds")
CONSOLE_COMMAND("force_unpause", "i[id]", CFGFLAG_SERVER, ConForcePause, this, "Set force-pause timer of i to 0.")
CONSOLE_COMMAND("showothers", "?i['0'|'1']", CFGFLAG_CHAT, ConShowOthers, this, "Whether to show players from other teams or not (off by default), optional i = 0 for off else for on")
CONSOLE_COMMAND("showall", "?i['0'|'1']", CFGFLAG_CHAT, ConShowAll, this, "Whether to show players at any distance (off by default), optional i = 0 for off else for on")

CONSOLE_COMMAND("list", "?s[filter]", CFGFLAG_CHAT, ConList, this, "List connected players with optional case-insensitive substring matching filter")

CONSOLE_COMMAND("mute", "", CFGFLAG_SERVER, ConMute, this, "");
CONSOLE_COMMAND("muteid", "v[id] i[seconds]", CFGFLAG_SERVER, ConMuteID, this, "");
CONSOLE_COMMAND("muteip", "s[ip] i[seconds]", CFGFLAG_SERVER, ConMuteIP, this, "");
CONSOLE_COMMAND("unmute", "v[id]", CFGFLAG_SERVER, ConUnmute, this, "");
CONSOLE_COMMAND("mutes", "", CFGFLAG_SERVER, ConMutes, this, "");

CONSOLE_COMMAND("freezehammer", "v[id]", CFGFLAG_SERVER, ConFreezeHammer, this, "Gives a player Freeze Hammer")
CONSOLE_COMMAND("unfreezehammer", "v[id]", CFGFLAG_SERVER, ConUnFreezeHammer, this, "Removes Freeze Hammer from a player")

// DDNet++

CONSOLE_COMMAND("hammer", "", CFGFLAG_SERVER, ConHammer, this, "Gives a hammer to you") //added by ChillerDragon and not as a cheat... because a hammer comon xd

// cosmetics
CONSOLE_COMMAND("OldRainbow", "v", CFGFLAG_SERVER, ConOldRainbow, this, "(old) activates rainbow until death")
CONSOLE_COMMAND("InfRainbow", "v", CFGFLAG_SERVER, ConInfRainbow, this, "activates rainbow until disconnect")
CONSOLE_COMMAND("OldBloody", "v", CFGFLAG_SERVER, ConOldBloody, this, "(old) activates bloody until death")
CONSOLE_COMMAND("InfBloody", "v", CFGFLAG_SERVER, ConInfBloody, this, "activates bloody until disconnect")
CONSOLE_COMMAND("OldAtom", "v", CFGFLAG_SERVER, ConOldAtom, this, "(old) activates atom until death")
CONSOLE_COMMAND("InfAtom", "v", CFGFLAG_SERVER, ConInfAtom, this, "activates atom until disconnect")
CONSOLE_COMMAND("OldAutoSpreadGun", "v", CFGFLAG_SERVER, ConOldAutoSpreadGun, this, "activates spread gun until death")
CONSOLE_COMMAND("InfAutoSpreadGun", "v", CFGFLAG_SERVER, ConInfAutoSpreadGun, this, "activates spread gun until disconnect")
CONSOLE_COMMAND("OldTrail", "v", CFGFLAG_SERVER, ConOldTrail, this, "(old) activates trail until death")
CONSOLE_COMMAND("InfTrail", "v", CFGFLAG_SERVER, ConInfTrail, this, "activates trail until disconnect")

CONSOLE_COMMAND("HomingMissile", "v", CFGFLAG_SERVER, ConHomingMissile, this, "toggles homing missile activate/deactive (grenade launcher)")

CONSOLE_COMMAND("disarm", "v", CFGFLAG_SERVER, Condisarm, this, "disarm a evil tee to prevent him doing evil stuff :)")
CONSOLE_COMMAND("dummymode", "vi", CFGFLAG_SERVER, Condummymode, this, "0 default, 23 cb5(racer), 29 cb5(blocker), 103 ctf5, 27 blmapchill(policebot), 32 blmapchill(solo policebot)")
CONSOLE_COMMAND("dummy_color", "vi", CFGFLAG_SERVER, ConDummyColor, this, "changes the color of a specific dummy")
CONSOLE_COMMAND("dummy_skin", "vr", CFGFLAG_SERVER, ConDummySkin, this, "changes the skin of a specific dummy")
CONSOLE_COMMAND("force_color", "vi", CFGFLAG_SERVER, ConForceColor, this, "changes the color of a specific player")
CONSOLE_COMMAND("force_skin", "vr", CFGFLAG_SERVER, ConForceSkin, this, "changes the skin of a specific player")
CONSOLE_COMMAND("heal", "v", CFGFLAG_SERVER, Conheal, this, "heals a tee's Health to 10 hp")
CONSOLE_COMMAND("ForceUnJail", "v", CFGFLAG_SERVER, ConForceUnJail, this, "unjails player instantly")
CONSOLE_COMMAND("ForceJail", "vi", CFGFLAG_SERVER, ConForceJail, this, "jails player by adminforce")
CONSOLE_COMMAND("ninjasteam", "v", CFGFLAG_SERVER, Conninjasteam, this, "activates a awesome NINJASTEAM")
CONSOLE_COMMAND("hammerfightmode", "v", CFGFLAG_SERVER, ConHammerfightMode, this, "activates hammerfightmode for a player")
CONSOLE_COMMAND("freezeShotgun", "v", CFGFLAG_SERVER, ConfreezeShotgun, this, "Gives you a freeze Shotgun")
CONSOLE_COMMAND("Damage", "v", CFGFLAG_SERVER, ConDamage, this, "Makes a player vulnerable")

CONSOLE_COMMAND("register_ban", "", CFGFLAG_SERVER, ConRegisterBan, this, "");
CONSOLE_COMMAND("register_ban_id", "vi", CFGFLAG_SERVER, ConRegisterBanID, this, "");
CONSOLE_COMMAND("register_ban_ip", "si", CFGFLAG_SERVER, ConRegisterBanIP, this, "");
CONSOLE_COMMAND("unregister_ban", "v", CFGFLAG_SERVER, ConUnRegisterBan, this, "");
CONSOLE_COMMAND("register_bans", "", CFGFLAG_SERVER, ConRegisterBans, this, "");

CONSOLE_COMMAND("login_ban", "", CFGFLAG_SERVER, ConLoginBan, this, "");
CONSOLE_COMMAND("login_ban_id", "vi", CFGFLAG_SERVER, ConLoginBanID, this, "");
CONSOLE_COMMAND("login_ban_ip", "si", CFGFLAG_SERVER, ConLoginBanIP, this, "");
CONSOLE_COMMAND("unlogin_ban", "v", CFGFLAG_SERVER, ConUnLoginBan, this, "");
CONSOLE_COMMAND("login_bans", "", CFGFLAG_SERVER, ConLoginBans, this, "");

CONSOLE_COMMAND("namechange_mute", "", CFGFLAG_SERVER, ConNameChangeMute, this, "");
CONSOLE_COMMAND("namechange_mute_id", "vi", CFGFLAG_SERVER, ConNameChangeMuteID, this, "");
CONSOLE_COMMAND("namechange_mute_ip", "si", CFGFLAG_SERVER, ConNameChangeMuteIP, this, "");
CONSOLE_COMMAND("namechange_unmute", "v", CFGFLAG_SERVER, ConNameChangeUnmute, this, "");
CONSOLE_COMMAND("namechange_mutes", "", CFGFLAG_SERVER, ConNameChangeMutes, this, "");

CONSOLE_COMMAND("destroylaser", "v", CFGFLAG_SERVER, ConDestroyLaser, this, "Gives a player Destroy Laser")
CONSOLE_COMMAND("freezelaser", "v", CFGFLAG_SERVER, ConFreezeLaser, this, "Gives a player Freeze Laser")

//SarKro
CONSOLE_COMMAND("unfreeze", "v", CFGFLAG_SERVER, ConUnFreeze, this, "Unfreezes player v")
CONSOLE_COMMAND("freeze", "v?i", CFGFLAG_SERVER, ConFreeze, this, "Freezes player v for i seconds (infinite by default)")

//HACK COMMAND ChillerDragon
//CONSOLE_COMMAND("hack", "vi", CFGFLAG_SERVER, ConHack, this, "dont use this command!")
CONSOLE_COMMAND("godmode", "v", CFGFLAG_SERVER, ConGodmode, this, "gives player i godmode (no damage in instagib)")

CONSOLE_COMMAND("logs", "?s[type]", CFGFLAG_SERVER, ConDDPPLogs, this, "shows ddnet++ logs (types: mastersrv)")

//ddpp sql
CONSOLE_COMMAND("sql_add", "?sss", CFGFLAG_SERVER, ConSQL_ADD, this, "adds an new column to the table")

//rcon api commands
CONSOLE_COMMAND("rcon_api_say_id", "vs", CFGFLAG_SERVER, ConRconApiSayID, this, "RCON API command dont use it") //sends a servermessage to player v
CONSOLE_COMMAND("rcon_api_alter_table", "is", CFGFLAG_SERVER, ConRconApiAlterTable, this, "RCON API command dont use it") //adds the column with the name s and the type i. i=0 INTEGER i=1 VARCHAR(4) i=2 VARCHAR(16) i=3 VARCHAR(32) i=4 VARCHAR(64) i=5 VARCHAR(128)   

#undef CONSOLE_COMMAND

#endif
