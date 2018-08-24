#!/bin/bash
echo "----------------------------------"
echo " ---> GITHUB <---"
echo "ChillerDragon's mod update script (debug)"
echo "DDNet-Server_d (BlmapChill)"
echo "----------------------------------"
echo "This script fetches the git/DDNetPP/DDNetPP_d and replaces the BlmapChill/BlmapChill_srv_d"
echo "Server keeps running"
echo "Do you really want to run the update? [y/n]"
read -n 1 -p "" inp
echo ""
if [ "$inp" == "y" ]
then
    test
elif [ "$inp" == "Y" ]
then
    test
else
    echo "Stopping update..."
    exit
fi

cd /home/$USER/git/DDNetPP;
git pull;
./bam server_debug;

cd /home/$USER/BlmapChill/;
echo "[BACKUP] Server";
mv BlmapChill_srv_d BlmapChill_srv_d_old;
echo "[UPDATE] binary";
cp /home/$USER/git/DDNetPP/DDNetPP_d BlmapChill_srv_d;

echo "Do you want to update maps as well? [y/n]"
read -n 1 -p "" inp
echo ""
if [ "$inp" == "y" ]
then
    test
elif [ "$inp" == "Y" ]
then
    test
else
    echo "Stopping update..."
    exit
fi

echo "[UPDATE] maps"
cp /home/$USER/git/DDNetPP/maps/* maps/
