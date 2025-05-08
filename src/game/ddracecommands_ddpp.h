// DDNet++

// This file can be included several times.

#ifndef CONSOLE_COMMAND
#define CONSOLE_COMMAND(name, params, flags, callback, userdata, help)
#endif

CONSOLE_COMMAND("hammer", "", CFGFLAG_SERVER, ConHammer, this, "Gives a hammer to you") //added by ChillerDragon and not as a cheat... because a hammer comon xd

// cosmetics
CONSOLE_COMMAND("old_rainbow", "v", CFGFLAG_SERVER, ConOldRainbow, this, "(old) activates rainbow until death")
CONSOLE_COMMAND("inf_rainbow", "v", CFGFLAG_SERVER, ConInfRainbow, this, "activates rainbow until disconnect")
CONSOLE_COMMAND("old_bloody", "v", CFGFLAG_SERVER, ConOldBloody, this, "(old) activates bloody until death")
CONSOLE_COMMAND("inf_bloody", "v", CFGFLAG_SERVER, ConInfBloody, this, "activates bloody until disconnect")
CONSOLE_COMMAND("old_atom", "v", CFGFLAG_SERVER, ConOldAtom, this, "(old) activates atom until death")
CONSOLE_COMMAND("inf_atom", "v", CFGFLAG_SERVER, ConInfAtom, this, "activates atom until disconnect")
CONSOLE_COMMAND("old_autospreadgun", "v", CFGFLAG_SERVER, ConOldAutoSpreadGun, this, "activates spread gun until death")
CONSOLE_COMMAND("inf_autospreadgun", "v", CFGFLAG_SERVER, ConInfAutoSpreadGun, this, "activates spread gun until disconnect")
CONSOLE_COMMAND("old_trail", "v", CFGFLAG_SERVER, ConOldTrail, this, "(old) activates trail until death")
CONSOLE_COMMAND("inf_trail", "v", CFGFLAG_SERVER, ConInfTrail, this, "activates trail until disconnect")

CONSOLE_COMMAND("homing_missile", "v", CFGFLAG_SERVER, ConHomingMissile, this, "toggles homing missile activate/deactive (grenade launcher)")

CONSOLE_COMMAND("disarm", "v", CFGFLAG_SERVER, Condisarm, this, "disarm a evil tee to prevent him doing evil stuff :)")
CONSOLE_COMMAND("dummymode", "vi", CFGFLAG_SERVER, Condummymode, this, "0 default, 23 cb5(racer), 29 cb5(blocker), 103 ctf5, 27 blmapchill(policebot), 32 blmapchill(solo policebot)")
CONSOLE_COMMAND("dummy_color", "vi", CFGFLAG_SERVER, ConDummyColor, this, "changes the color of a specific dummy")
CONSOLE_COMMAND("dummy_skin", "vr", CFGFLAG_SERVER, ConDummySkin, this, "changes the skin of a specific dummy")
CONSOLE_COMMAND("force_color", "vi", CFGFLAG_SERVER, ConForceColor, this, "changes the color of a specific player")
CONSOLE_COMMAND("force_skin", "vr", CFGFLAG_SERVER, ConForceSkin, this, "changes the skin of a specific player")
CONSOLE_COMMAND("heal", "v", CFGFLAG_SERVER, Conheal, this, "heals a tee's Health to 10 hp")
CONSOLE_COMMAND("force_unjail", "v[player]", CFGFLAG_SERVER, ConForceUnJail, this, "unjails player instantly")
CONSOLE_COMMAND("force_jail", "v[player]i[seconds]", CFGFLAG_SERVER, ConForceJail, this, "jails player by adminforce")
CONSOLE_COMMAND("ninjasteam", "v", CFGFLAG_SERVER, Conninjasteam, this, "activates a awesome NINJASTEAM")
CONSOLE_COMMAND("hammerfightmode", "v", CFGFLAG_SERVER, ConHammerfightMode, this, "activates hammerfightmode for a player")
CONSOLE_COMMAND("freeze_shotgun", "v", CFGFLAG_SERVER, ConfreezeShotgun, this, "Gives you a freeze Shotgun")
CONSOLE_COMMAND("Damage", "v", CFGFLAG_SERVER, ConDamage, this, "Makes a player vulnerable")

CONSOLE_COMMAND("register_ban", "", CFGFLAG_SERVER, ConRegisterBan, this, "");
CONSOLE_COMMAND("register_ban_id", "vi", CFGFLAG_SERVER, ConRegisterBanId, this, "");
CONSOLE_COMMAND("register_ban_ip", "si", CFGFLAG_SERVER, ConRegisterBanIp, this, "");
CONSOLE_COMMAND("unregister_ban", "v", CFGFLAG_SERVER, ConUnRegisterBan, this, "");
CONSOLE_COMMAND("register_bans", "", CFGFLAG_SERVER, ConRegisterBans, this, "");

CONSOLE_COMMAND("login_ban", "", CFGFLAG_SERVER, ConLoginBan, this, "");
CONSOLE_COMMAND("login_ban_id", "vi", CFGFLAG_SERVER, ConLoginBanId, this, "");
CONSOLE_COMMAND("login_ban_ip", "si", CFGFLAG_SERVER, ConLoginBanIp, this, "");
CONSOLE_COMMAND("unlogin_ban", "v", CFGFLAG_SERVER, ConUnLoginBan, this, "");
CONSOLE_COMMAND("login_bans", "", CFGFLAG_SERVER, ConLoginBans, this, "");

CONSOLE_COMMAND("namechange_mute", "", CFGFLAG_SERVER, ConNameChangeMute, this, "");
CONSOLE_COMMAND("namechange_mute_id", "vi", CFGFLAG_SERVER, ConNameChangeMuteId, this, "");
CONSOLE_COMMAND("namechange_mute_ip", "si", CFGFLAG_SERVER, ConNameChangeMuteIp, this, "");
CONSOLE_COMMAND("namechange_unmute", "v", CFGFLAG_SERVER, ConNameChangeUnmute, this, "");
CONSOLE_COMMAND("namechange_mutes", "", CFGFLAG_SERVER, ConNameChangeMutes, this, "");

CONSOLE_COMMAND("dummies", "", CFGFLAG_SERVER, ConDummies, this, "");

CONSOLE_COMMAND("block_votes", "?i[minutes]", CFGFLAG_SERVER, ConBlockVotes, this, "disables all votes but 'unblock_votes'");
CONSOLE_COMMAND("unblock_votes", "", CFGFLAG_SERVER, ConUnblockVotes, this, "unlocks votes if they were blocked by 'block_votes'");

CONSOLE_COMMAND("freezelaser", "v", CFGFLAG_SERVER, ConFreezeLaser, this, "Gives a player Freeze Laser")
CONSOLE_COMMAND("freezehammer", "v", CFGFLAG_SERVER, ConFreezeHammer, this, "Gives a player Freeze Hammer")

//gun
CONSOLE_COMMAND("heartgun", "v", CFGFLAG_SERVER, ConHeartGun, this, "Gives a player heart gun")

//SarKro
CONSOLE_COMMAND("unfreeze", "v", CFGFLAG_SERVER, ConUnFreeze, this, "Unfreezes player v")
CONSOLE_COMMAND("freeze", "v?i", CFGFLAG_SERVER, ConFreeze, this, "Freezes player v for i seconds (infinite by default)")

//HACK COMMAND ChillerDragon
//CONSOLE_COMMAND("hack", "vi", CFGFLAG_SERVER, ConHack, this, "dont use this command!")
CONSOLE_COMMAND("godmode", "v", CFGFLAG_SERVER, ConGodmode, this, "gives player i godmode (no damage in instagib)")
CONSOLE_COMMAND("hide_player", "v", CFGFLAG_SERVER, ConHidePlayer, this, "makes player invisble")
CONSOLE_COMMAND("verify_player", "v", CFGFLAG_SERVER, ConVerifyPlayer, this, "manually set a targets player human level to max (force solve captcha)")

CONSOLE_COMMAND("logs", "?s[type]", CFGFLAG_SERVER, ConDDPPLogs, this, "shows ddnet++ logs (types: mastersrv)")
CONSOLE_COMMAND("reload_spamfilters", "", CFGFLAG_SERVER, ConReloadSpamfilters, this, "reads spamfilters.txt (see also add_spamfilter)")
CONSOLE_COMMAND("add_spamfilter", "s[filter]", CFGFLAG_SERVER, ConAddSpamfilter, this, "writes to spamfilters.txt (see also list_spamfilters)")
CONSOLE_COMMAND("list_spamfilters", "", CFGFLAG_SERVER, ConListSpamfilters, this, "prints actice spamfilters (see also reload_spamfilters)")

//ddpp sql
CONSOLE_COMMAND("sql_add", "?sss", CFGFLAG_SERVER, ConSql_ADD, this, "adds an new column to the table")

CONSOLE_COMMAND("set_shop_item_price", "s[item] s[price]", CFGFLAG_SERVER, ConSetShopItemPrice, this, "sets the price of a shop item")
CONSOLE_COMMAND("set_shop_item_description", "s[item] s[description]", CFGFLAG_SERVER, ConSetShopItemDescription, this, "sets the description of a shop item")
CONSOLE_COMMAND("set_shop_item_level", "s[item] i[level]", CFGFLAG_SERVER, ConSetShopItemLevel, this, "sets the needed level of a shop item")
CONSOLE_COMMAND("activate_shop_item", "?s[item]", CFGFLAG_SERVER, ConActivateShopItem, this, "activate shop item")
CONSOLE_COMMAND("deactivate_shop_item", "?s[item]", CFGFLAG_SERVER, ConDeactivateShopItem, this, "deactivate shop item")
CONSOLE_COMMAND("deactivate_all_shop_items", "", CFGFLAG_SERVER, ConDeactivateAllShopItems, this, "deactivates all shop items")
CONSOLE_COMMAND("activate_all_shop_items", "", CFGFLAG_SERVER, ConActivateAllShopItems, this, "activates all shop items")

CONSOLE_COMMAND("run_test", "i[test_number]", CFGFLAG_SERVER, ConRunTest, this, "WARNING WILL KILL THE SERVER! run integration tests (used in CI)");
CONSOLE_COMMAND("defer", "r[command]", CFGFLAG_SERVER, ConDeferCommand, this, "Run a cmd after the server fully initialized. Run cmds from cfg or cli that would crash if executed too early.");

//rcon api commands
CONSOLE_COMMAND("rcon_api_say_id", "vs", CFGFLAG_SERVER, ConRconApiSayId, this, "RCON API command dont use it") //sends a servermessage to player v
//adds the column with the name s and the type i. i=0 INTEGER i=1 VARCHAR(4) i=2 VARCHAR(16) i=3 VARCHAR(32) i=4 VARCHAR(64) i=5 VARCHAR(128)
CONSOLE_COMMAND("rcon_api_alter_table", "i[type]s[column]", CFGFLAG_SERVER, ConRconApiAlterTable, this, "RCON API command dont use it")
