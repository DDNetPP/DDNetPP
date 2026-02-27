// This file can be included several times.
// DDNet++

#ifndef CONSOLE_COMMAND
#define CONSOLE_COMMAND(name, params, flags, callback, userdata, help)
#endif

CONSOLE_COMMAND("hammer", "", CFGFLAG_SERVER, ConHammer, this, "Gives a hammer to you") // added by ChillerDragon and not as a cheat... because a hammer common xd

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

CONSOLE_COMMAND("disarm", "v[id]", CFGFLAG_SERVER, Condisarm, this, "disarm a evil tee to prevent him doing evil stuff :)")
CONSOLE_COMMAND("dummymode", "v[id] i[mode]", CFGFLAG_SERVER, Condummymode, this, "0 default, 23 cb5(racer), 29 cb5(blocker), 103 ctf5, 27 blmapchill(policebot), 32 blmapchill(solo policebot)")
CONSOLE_COMMAND("dummy_color", "v[id] i[color]", CFGFLAG_SERVER, ConDummyColor, this, "changes the color of a specific dummy")
CONSOLE_COMMAND("dummy_skin", "v[id] r[skin]", CFGFLAG_SERVER, ConDummySkin, this, "changes the skin of a specific dummy")
CONSOLE_COMMAND("force_color", "v[id] i[color]", CFGFLAG_SERVER, ConForceColor, this, "changes the color of a specific player")
CONSOLE_COMMAND("force_skin", "v[id] r[skin]", CFGFLAG_SERVER, ConForceSkin, this, "changes the skin of a specific player")
CONSOLE_COMMAND("heal", "v[id]", CFGFLAG_SERVER, Conheal, this, "heals a tee's Health to 10 hp")
CONSOLE_COMMAND("force_unjail", "v[id]", CFGFLAG_SERVER, ConForceUnJail, this, "unjails player instantly")
CONSOLE_COMMAND("force_jail", "v[id] i[seconds]", CFGFLAG_SERVER, ConForceJail, this, "jails player by adminforce")
CONSOLE_COMMAND("ninjasteam", "v[id]", CFGFLAG_SERVER, Conninjasteam, this, "activates a awesome NINJASTEAM")
CONSOLE_COMMAND("hammerfightmode", "v[id]", CFGFLAG_SERVER, ConHammerfightMode, this, "activates hammerfightmode for a player")
CONSOLE_COMMAND("freezeshotgun", "v[id]", CFGFLAG_SERVER, ConfreezeShotgun, this, "Gives you a freeze Shotgun")
CONSOLE_COMMAND("damage", "v[id]", CFGFLAG_SERVER, ConDamage, this, "Makes a player vulnerable")

CONSOLE_COMMAND("register_ban", "", CFGFLAG_SERVER, ConRegisterBan, this, "Use either 'register_ban_id <client_id> <seconds>' or 'register_ban_ip <ip> <seconds>'")
CONSOLE_COMMAND("register_ban_id", "vi", CFGFLAG_SERVER, ConRegisterBanId, this, "Ban a player from the registration system")
CONSOLE_COMMAND("register_ban_ip", "si", CFGFLAG_SERVER, ConRegisterBanIp, this, "Ban a player ip from the registration system")
CONSOLE_COMMAND("unregister_ban", "v", CFGFLAG_SERVER, ConUnRegisterBan, this, "Unban a player from the registration system")
CONSOLE_COMMAND("register_bans", "", CFGFLAG_SERVER, ConRegisterBans, this, "Unban a player ip from the registration system")

CONSOLE_COMMAND("login_ban", "", CFGFLAG_SERVER, ConLoginBan, this, "Use either 'login_ban_id <client_id> <seconds>' or 'login_ban_ip <ip> <seconds>'")
CONSOLE_COMMAND("login_ban_id", "vi", CFGFLAG_SERVER, ConLoginBanId, this, "Ban a player from the login system")
CONSOLE_COMMAND("login_ban_ip", "si", CFGFLAG_SERVER, ConLoginBanIp, this, "Ban a player ip from the login system")
CONSOLE_COMMAND("unlogin_ban", "v", CFGFLAG_SERVER, ConUnLoginBan, this, "Unban a player from the login system")
CONSOLE_COMMAND("login_bans", "", CFGFLAG_SERVER, ConLoginBans, this, "Unban a player ip from the login system")

CONSOLE_COMMAND("namechange_mute", "", CFGFLAG_SERVER, ConNameChangeMute, this, "Use either 'namechange_mute_id <client_id> <seconds>' or 'namechange_mute_ip <ip> <seconds>'")
CONSOLE_COMMAND("namechange_mute_id", "vi", CFGFLAG_SERVER, ConNameChangeMuteId, this, "Mute a player from changing name system by client ID")
CONSOLE_COMMAND("namechange_mute_ip", "si", CFGFLAG_SERVER, ConNameChangeMuteIp, this, "Mute a player from changing name system by IP address")
CONSOLE_COMMAND("namechange_unmute", "v", CFGFLAG_SERVER, ConNameChangeUnmute, this, "Unmute a player from name change system")
CONSOLE_COMMAND("namechange_mutes", "", CFGFLAG_SERVER, ConNameChangeMutes, this, "List all players muted from changing name system")

CONSOLE_COMMAND("dummies", "", CFGFLAG_SERVER, ConDummies, this, "List all connected dummy")
CONSOLE_COMMAND("plugins", "", CFGFLAG_SERVER, ConPlugins, this, "List and manage lua plugins")

CONSOLE_COMMAND("block_votes", "?i[minutes]", CFGFLAG_SERVER, ConBlockVotes, this, "disables all votes but 'unblock_votes'")
CONSOLE_COMMAND("unblock_votes", "", CFGFLAG_SERVER, ConUnblockVotes, this, "unlocks votes if they were blocked by 'block_votes'")

CONSOLE_COMMAND("freezelaser", "v[id]", CFGFLAG_SERVER, ConFreezeLaser, this, "Gives a player Freeze Laser")
CONSOLE_COMMAND("freezehammer", "v", CFGFLAG_SERVER, ConFreezeHammer, this, "Gives a player Freeze Hammer")
CONSOLE_COMMAND("lasergun", "v[id]", CFGFLAG_SERVER, ConLaserGun, this, "Toggles laser gun on and off for player v")

CONSOLE_COMMAND("heartgun", "v", CFGFLAG_SERVER, ConHeartGun, this, "Gives a player heart gun")

CONSOLE_COMMAND("unfreeze", "v[id]", CFGFLAG_SERVER, ConUnfreeze, this, "Unfreezes player v")
CONSOLE_COMMAND("freeze", "v[id] ?i", CFGFLAG_SERVER, ConFreeze, this, "Freezes player v for i seconds (infinite by default)")

CONSOLE_COMMAND("godmode", "v[id]", CFGFLAG_SERVER, ConGodmode, this, "gives player i godmode (no damage in instagib)")
CONSOLE_COMMAND("hide_player", "v[id]", CFGFLAG_SERVER, ConHidePlayer, this, "makes player invisible")
CONSOLE_COMMAND("verify_player", "v", CFGFLAG_SERVER, ConVerifyPlayer, this, "manually set a targets player human level to max (force solve captcha)")

CONSOLE_COMMAND("logs", "?s[type]", CFGFLAG_SERVER, ConDDPPLogs, this, "shows ddnet++ logs (types: mastersrv)")
CONSOLE_COMMAND("reload_spamfilters", "", CFGFLAG_SERVER, ConReloadSpamfilters, this, "reads spamfilters.txt (see also add_spamfilter)")
CONSOLE_COMMAND("add_spamfilter", "s[filter]", CFGFLAG_SERVER, ConAddSpamfilter, this, "writes to spamfilters.txt (see also list_spamfilters)")
CONSOLE_COMMAND("list_spamfilters", "", CFGFLAG_SERVER, ConListSpamfilters, this, "prints active spamfilters (see also reload_spamfilters)")

CONSOLE_COMMAND("sql_add", "?sss", CFGFLAG_SERVER, ConSql_ADD, this, "adds an new column to the table")

CONSOLE_COMMAND("set_shop_item_price", "s[item] s[price]", CFGFLAG_SERVER, ConSetShopItemPrice, this, "sets the price of a shop item")
CONSOLE_COMMAND("set_shop_item_description", "s[item] s[description]", CFGFLAG_SERVER, ConSetShopItemDescription, this, "sets the description of a shop item")
CONSOLE_COMMAND("set_shop_item_level", "s[item] i[level]", CFGFLAG_SERVER, ConSetShopItemLevel, this, "sets the needed level of a shop item")
CONSOLE_COMMAND("activate_shop_item", "?s[item]", CFGFLAG_SERVER, ConActivateShopItem, this, "activate shop item")
CONSOLE_COMMAND("deactivate_shop_item", "?s[item]", CFGFLAG_SERVER, ConDeactivateShopItem, this, "deactivate shop item")
CONSOLE_COMMAND("deactivate_all_shop_items", "", CFGFLAG_SERVER, ConDeactivateAllShopItems, this, "deactivates all shop items")
CONSOLE_COMMAND("activate_all_shop_items", "", CFGFLAG_SERVER, ConActivateAllShopItems, this, "activates all shop items")

CONSOLE_COMMAND("run_test", "i[test_number]", CFGFLAG_SERVER, ConRunTest, this, "WARNING WILL KILL THE SERVER! run integration tests (used in CI)")
CONSOLE_COMMAND("defer", "r[command]", CFGFLAG_SERVER, ConDeferCommand, this, "Run a cmd after the server fully initialized. Run cmds from cfg or cli that would crash if executed too early.")

CONSOLE_COMMAND("rcon_api_say_id", "vs", CFGFLAG_SERVER, ConRconApiSayId, this, "RCON API command dont use it") // sends a servermessage to player v
// adds the column with the name s and the type i. i=0 INTEGER i=1 VARCHAR(4) i=2 VARCHAR(16) i=3 VARCHAR(32) i=4 VARCHAR(64) i=5 VARCHAR(128)
CONSOLE_COMMAND("rcon_api_alter_table", "i[type]s[column]", CFGFLAG_SERVER, ConRconApiAlterTable, this, "RCON API command dont use it")
