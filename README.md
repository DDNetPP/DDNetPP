[![DDNet++](./data/ddnetpp.svg)](./data/ddnetpp.svg)
[![](https://github.com/DDNetPP/DDNetPP/workflows/Build/badge.svg)](https://github.com/DDNetPP/DDNetPP/actions?query=workflow%3ABuild+event%3Apush+branch%3Amaster)
================================

Based on DDNet: flavor of DDRace, a Teeworlds mod. See the [website](http://ddnet.tw) for more information.

Download DDNet++ version 0.0.7 for macOS/linux/windows [here on github](https://github.com/DDNetPP/DDNetPP/releases/tag/v.0.0.7).

Building
--------

DDNet++ supports bam4, bam5 and cmake building on macOS, linux and windows.
Choose between bam or cmake whatever you prefer.

If you want more building information feel free to check out the [ddnet](https://github.com/ddnet/ddnet/blob/master/README.md) and [teeworlds](https://github.com/teeworlds/teeworlds/blob/master/readme.md) READMEs.
Those might include valuable information since DDNet++ is a fork of those repositorys.

## bam
### linux / macOS
setup bam:
```
# it is recommended to add it to your PATH or a bin directory like /usr/bin/local
cd /tmp
git clone https://github.com/matricks/bam.git
cd bam
./make_unix.sh
```
build DDNet++:
```
git clone --recursive https://github.com/DDNetPP/DDNetPP.git
cd DDNetPP
/tmp/bam server_release
```
### windows
setup bam:
```
:: it is recommended to add C:\bam to your PATH
:: search for enviroment variables in windows and then edit path
:: append C:\bam to the list and clone the bam src there
cd C:\
git clone https://github.com/matricks/bam.git
cd bam
make_win32_mingw.bat
```
build DDNet++:
```
git clone --recursive https://github.com/DDNetPP/DDNetPP.git
cd DDNetPP
bam.exe server_release
```
## cmake
### linux
```
sudo apt install cmake
git clone --recursive https://github.com/DDNetPP/DDNetPP.git
cd DDNetPP
mkdir build && cd build
cmake ..
make
```
### macOS
```
# if you have no brew already install brew first:
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
# install cmake
brew install cmake
git clone --recursive https://github.com/DDNetPP/DDNetPP.git
cd DDNetPP
mkdir build && cd build
cmake ..
make
```
### windows
Download and install cmake from https://cmake.org/download/
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
- minigames (bomb/fng/blockwave/survival)
- quests
- shop
- police
- jail
- drop weapons and flags
- and much more

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

<<<<<<< HEAD
If you run multiple servers on different ports the server has to save which port the accounts are logged in. Newer ddnet allows you to not set ``sv_port`` or set it to 0 which picks a free port for you. But the database will then store 0 for all servers. So if you are using multiple servers sharing the same database you have to explicitly set ``sv_port`` to something non zero.
=======
Install [osxcross](https://github.com/tpoechtrager/osxcross), then add
`-DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/darwin.toolchain` and
`-DCMAKE_OSX_SYSROOT=/path/to/osxcross/target/SDK/MacOSX10.11.sdk/` to the
**initial** CMake command line.

Install `dmg` and `hfsplus` from
[libdmg-hfsplus](https://github.com/mozilla/libdmg-hfsplus) and `newfs_hfs`
from
[diskdev\_cmds](http://pkgs.fedoraproject.org/repo/pkgs/hfsplus-tools/diskdev_cmds-540.1.linux3.tar.gz/0435afc389b919027b69616ad1b05709/diskdev_cmds-540.1.linux3.tar.gz)
to unlock the `package_dmg` target that outputs a macOS disk image.

Importing the official DDNet Database
-------------------------------------

```bash
$ wget https://ddnet.org/stats/ddnet-sql.zip
$ unzip ddnet-sql.zip
$ yaourt -S mariadb mysql-connector-c++
$ mysql_install_db --user=mysql --basedir=/usr --datadir=/var/lib/mysql
$ systemctl start mariadb
$ mysqladmin -u root password 'PW'
$ mysql -u root -p'PW'
MariaDB [(none)]> create database teeworlds; create user 'teeworlds'@'localhost' identified by 'PW2'; grant all privileges on teeworlds.* to 'teeworlds'@'localhost'; flush privileges;
# this takes a while, you can remove the KEYs in record_race.sql to trade performance in queries
$ mysql -u teeworlds -p'PW2' teeworlds < ddnet-sql/record_*.sql

$ cat mine.cfg
sv_use_sql 1
add_sqlserver r teeworlds record teeworlds "PW2" "localhost" "3306"
add_sqlserver w teeworlds record teeworlds "PW2" "localhost" "3306"

$ mkdir build
$ cd build
$ cmake -DMYSQL=ON ..
$ make -j$(nproc)
$ ./DDNet-Server -f mine.cfg
```

<a href="https://repology.org/metapackage/ddnet/versions">
    <img src="https://repology.org/badge/vertical-allrepos/ddnet.svg?header=" alt="Packaging status" align="right">
</a>

Installation from Repository
----------------------------

Debian/Ubuntu

```bash
$ apt-get install ddnet

```

MacOS

```bash
$ brew install --cask ddnet
```

Fedora

```bash
$ dnf install ddnet
```

Arch Linux

```bash
$ yay -S ddnet
```

FreeBSD

```bash
$ pkg install DDNet
```

Benchmarking
------------

DDNet is available in the [Phoronix Test Suite](https://openbenchmarking.org/test/pts/ddnet). If you have PTS installed you can easily benchmark DDNet on your own system like this:

```bash
$ phoronix-test-suite benchmark ddnet
```

Better Git Blame
----------------

First, use a better tool than `git blame` itself, e.g. [`tig`](https://jonas.github.io/tig/). There's probably a good UI for Windows, too. Alternatively, use the GitHub UI, click "Blame" in any file view.

For `tig`, use `tig blame path/to/file.cpp` to open the blame view, you can navigate with arrow keys or kj, press comma to go to the previous revision of the current line, q to quit.

Only then you could also set up git to ignore specific formatting revisions:
```bash
git config blame.ignoreRevsFile formatting-revs.txt
```

(Neo)Vim Syntax Highlighting for config files
----------------------------------------
Copy the file detection and syntax files to your vim config folder:

```bash
# vim
cp -R other/vim/* ~/.vim/

# neovim
cp -R other/vim/* ~/.config/nvim/
```
>>>>>>> 17402cc43fdf51c8cb81b724228f11c423d17007^
