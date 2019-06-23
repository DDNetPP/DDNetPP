[![DDNet++](./data/ddnetpp.svg)](./data/ddnetpp.svg)
[![CircleCI Build Status](https://circleci.com/gh/DDNetPP/DDNetPP/tree/master.png)](https://circleci.com/gh/DDNetPP/DDNetPP)
================================

Based on DDNet: Our own flavor of DDRace, a Teeworlds mod. See the [website](http://ddnet.tw) for more information.

Download DDNet++ version 0.0.6 for macOS/linux/windows [here on github](https://github.com/DDNetPP/DDNetPP/releases/tag/v.0.0.6).

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
git clone https://github.com/DDNetPP/DDNetPP.git
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
git clone https://github.com/DDNetPP/DDNetPP.git
cd DDNetPP
bam.exe server_release
```
## cmake
### linux
```
sudo apt install cmake
git clone https://github.com/DDNetPP/DDNetPP.git
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
git clone https://github.com/DDNetPP/DDNetPP.git
cd DDNetPP
mkdir build && cd build
cmake ..
make
```
### windows
Download and install cmake from https://cmake.org/download/
```
git clone https://github.com/DDNetPP/DDNetPP.git
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
- minigames (bomb/fng/blockwave)
- quests
- shop
- police
- jail
- drop weapons and flags
- and much more

Known Bugs
----------

Teams (/team num) crash the server if teammembers kill after passing the startline.
