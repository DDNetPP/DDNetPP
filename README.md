[![DDNet++](./data/ddnetpp.svg)](./data/ddnetpp.svg)
[![](https://github.com/DDNetPP/DDNetPP/workflows/Build/badge.svg)](https://github.com/DDNetPP/DDNetPP/actions?query=workflow%3ABuild+event%3Apush+branch%3Amaster)
================================

Based on DDNet: flavor of DDRace, a Teeworlds mod. See the [website](http://ddnet.tw) for more information.

Download DDNet++ version 0.0.7 for macOS/linux/windows [here on github](https://github.com/DDNetPP/DDNetPP/releases/tag/v.0.0.7).

Building
--------

DDNet++ uses cmake for building on macOS, linux and windows.

For building instructions and dependencies please check [ddnet](https://github.com/ddnet/ddnet/blob/master/README.md) and [teeworlds](https://github.com/teeworlds/teeworlds/blob/master/readme.md) READMEs.
But if you have all the dependencies installed this should work.

```
git clone --recursive https://github.com/DDNetPP/DDNetPP.git
cd DDNetPP
mkdir build
cd build
cmake ..
cmake --build .
```

DDNet++
--------

DDNet++ is the upgrade on top of the teeworlds mod ddracenetwork.
Mainly maintained by ChillerDragon and fokkonaut.
We added all the features missing in ddnet. For example:
- bloody
- rainbow
- accounts
- xp
- money
- block system (count kills and stats)
- minigames
  + [block tournaments](./docs/block_tournaments.md)
  + bomb
  + fng
  + blockwave
  + survival
- quests
- [shop](./docs/shop.md)
- police
- jail
- drop weapons and flags
- and much more

DDNet++ also has a lot of configs and commands. Make sure to check out [settings_and_commands.md](./docs/settings_and_commands.md) for more details.

Database
--------

DDNet++ is based on ddnet and uses the same database engine. But it uses a separate database for accounts than
for the ddrace race records. By default it uses sqlite3 but you can also configure it to use mysql (mariadb).


Only accounts are stored in the database.
To enable accounts you need to set a port and hostname in your config. Ideally before you enable accounts.

```
# your autoexec_server.cfg
sv_port 8303
sv_hostname "127.0.0.1" # <-- put your server ip here
sv_accounts 1
```

**sqlite3 (default)**

If accounts are enabled ddnet++ works without further configuration out of the box.
It stores the accounts in a database file determined by the config ``sv_database_path``
which defaults to **accounts.db**.

**mysql**

You have to enable mysql during compilation for example like this:

```sh
git clone --recursive https://github.com/DDNetPP/DDNetPP.git
cd DDNetPP
mkdir build
cd build
cmake .. -DMYSQL=ON
cmake --build .
```

Then you need a mysql server up and running (only mariadb is tested).
You also have to prepare a database and a user that has access to it. The [ddnet readme](https://github.com/ddnet/ddnet?tab=readme-ov-file#importing-the-official-ddnet-database) might be helpful if you do not know how that works.
Here a sample config connecting to the database. Put this into your autoexec_server.cfg

```
# add_accounts_sqlserver s['r'|'w'] s[Database] s[Prefix] s[User] s[Password] s[IP] i[Port] ?i[SetUpDatabase ?]

sv_use_mysql_for_accounts 1

add_accounts_sqlserver r teeworlds record teeworlds "PW2" "127.0.0.1" "3306"
add_accounts_sqlserver w teeworlds record teeworlds "PW2" "127.0.0.1" "3306"
```

Bugs
----

*Bug*:
```
*** [ACCOUNT] Login failed. This account is already logged in on another server.
```

*Explanation & fix*:


One account should not be logged in multiple times at the same time. To make sure this works with multiple servers sharing one database the logged in state is saved in the database. But somehow this fails to reset sometimes and users are unable to get into their account.
As a fix admin can use the '/sql_logout_all' chat command which updates the database to set all accounts that are currently not playing on this server and logged in on the current port to logged out. To avoid problems ``sv_port`` should not change during server runtime. The config ``sv_auto_fix_broken_accs`` can be set to ``"1"`` to automatically do this every few seconds.

*limitations of the fix:*

The fix is not supported well if the database holds more than 99 999 999 accounts.

An alternative is to do it for every account manually with the chat command '/sql_logout (accname)' but this is less save than sql_logout_all because this one does no check if the account is currently playing and is not limited to this port. So it could set a account logged out in the database while the account is logged in. And thus allow another login.

*Bug*:
```
*** [ACCOUNT] Login failed. This account is already logged in on another server.
```

*Explanation & fix*:

If you run multiple servers on different ports the server has to save which port the accounts are logged in. Newer ddnet allows you to not set ``sv_port`` or set it to 0 which picks a free port for you. But the database will then store 0 for all servers. So if you are using multiple servers sharing the same database you have to explicitly set ``sv_port`` to something non zero.
