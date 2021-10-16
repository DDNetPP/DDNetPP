[![DDNet++](./data/ddnetpp.svg)](./data/ddnetpp.svg)
[![](https://github.com/DDNetPP/DDNetPP/workflows/Build/badge.svg)](https://github.com/DDNetPP/DDNetPP/actions?query=workflow%3ABuild+event%3Apush+branch%3Amaster)
================================

Based on DDNet: Our own flavor of DDRace, a Teeworlds mod. See the [website](http://ddnet.tw) for more information.

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
