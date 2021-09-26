#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#kewl database merge script by ChillerDragon
#designed to combine two DDNetPlusPlus databases

# !!! WARNING !!!
# this is totally unfisnished
# untested and probably dangerous lul
# ony use this with test databases

import sqlite3
import os.path
import csv
import sys

def max(data1, data2):
    if (data1 > data2):
        return data1;
    else:
        return data2;


con = sqlite3.connect('accounts.db') #source (data loose)
cur = con.cursor()
total_accounts = 0

con2 = sqlite3.connect('accounts_main.db') #destination (the bigger database)
cur2 = con2.cursor()
total_accounts2 = 0

#fetch total account amount
cur.execute("SELECT COUNT(*) FROM Accounts")
for row in cur:
    print("found " + str(row[0]) + " accounts")
    total_accounts = row[0]

cur2.execute("SELECT COUNT(*) FROM Accounts")
for row in cur2:
    print("found " + str(row[0]) + " accounts2")
    total_accounts2 = row[0]

#creating two dimensional array depending on database size (found accounts)
aData = [["(invalid)" for x in range(10)] for y in range(total_accounts)]
aData2 = [["(invalid)" for x in range(10)] for y in range(total_accounts2)]
aMerged = [["(invalid)" for x in range(10)] for y in range(max(total_accounts, total_accounts2))]

################################
# total shitcoded data fetcher #
################################

index = 0
cur.execute("SELECT Username FROM Accounts")
for row in cur:
    #print('{0} : Username={1}, Money={2}, xp={3}, shit={4}, police={5}, {6}'.format(row[0], row[12], row[13], row[20], row[22], row[]))
    #print('Username: {0}'.format(row[0]));
    #aNames.append(row[0]);
    aData[index][0] = row[0]
    index+=1

index = 0
cur.execute("SELECT Password FROM Accounts")
for row in cur:
    aData[index][1] = row[0]
    index+=1

index = 0
cur.execute("SELECT Money FROM Accounts")
for row in cur:
    aData[index][2] = row[0]
    index+=1

index = 0
cur.execute("SELECT Exp FROM Accounts")
for row in cur:
    aData[index][3] = row[0]
    index+=1

index = 0
cur.execute("SELECT Shit FROM Accounts")
for row in cur:
    aData[index][4] = row[0]
    index+=1

index = 0
cur.execute("SELECT BlockPoints FROM Accounts")
for row in cur:
    aData[index][5] = row[0]
    index+=1

index = 0
cur.execute("SELECT BlockKills FROM Accounts")
for row in cur:
    aData[index][6] = row[0]
    index+=1

index = 0
cur.execute("SELECT BlockDeaths FROM Accounts")
for row in cur:
    aData[index][7] = row[0]
    index+=1

index = 0
cur.execute("SELECT PoliceRank FROM Accounts")
for row in cur:
    aData[index][8] = row[0]
    index+=1

index = 0
cur.execute("SELECT TaserLevel FROM Accounts")
for row in cur:
    aData[index][9] = row[0]
    index+=1

# database 2

index = 0
cur2.execute("SELECT Username FROM Accounts")
for row in cur2:
    aData2[index][0] = row[0]
    index+=1

index = 0
cur2.execute("SELECT Password FROM Accounts")
for row in cur2:
    aData2[index][1] = row[0]
    index+=1

index = 0
cur2.execute("SELECT Money FROM Accounts")
for row in cur2:
    aData2[index][2] = row[0]
    index+=1

index = 0
cur2.execute("SELECT Exp FROM Accounts")
for row in cur2:
    aData2[index][3] = row[0]
    index+=1

index = 0
cur2.execute("SELECT Shit FROM Accounts")
for row in cur2:
    aData2[index][4] = row[0]
    index+=1

index = 0
cur2.execute("SELECT BlockPoints FROM Accounts")
for row in cur2:
    aData2[index][5] = row[0]
    index+=1

index = 0
cur2.execute("SELECT BlockKills FROM Accounts")
for row in cur2:
    aData2[index][6] = row[0]
    index+=1

index = 0
cur2.execute("SELECT BlockDeaths FROM Accounts")
for row in cur2:
    aData2[index][7] = row[0]
    index+=1

index = 0
cur2.execute("SELECT PoliceRank FROM Accounts")
for row in cur2:
    aData2[index][8] = row[0]
    index+=1

index = 0
cur2.execute("SELECT TaserLevel FROM Accounts")
for row in cur2:
    aData2[index][9] = row[0]
    index+=1


################################
#       print fetched data     #
################################

'''
for data in aData:
    print(
          " Name=" + data[0] + "\n" +
          " Money=" + str(data[2]) + "\n" +
          " xp=" + str(data[3]) + "\n" +
          " Shit=" + str(data[4]) + "\n" +
          " BlockPoints=" + str(data[5]) + "\n" +
          " BlockKills=" + str(data[6]) + "\n" +
          " BlockDeaths=" + str(data[7]) + "\n" +
          " PoliceRank=" + str(data[8]) + "\n" +
          " TaserLevel=" + str(data[9]) + "\n--------------------------------"
          );
    print(
            " Name=" + data[0] + "\t" +
            " Money=" + str(data[2]) + "\t" +
            " xp=" + str(data[3]) + "\t" +
            " Shit=" + str(data[4]) + "\t" +
            " BlockPoints=" + str(data[5]) + "\t" +
            " BlockKills=" + str(data[6]) + "\t" +
            " BlockDeaths=" + str(data[7]) + "\t" +
            " PoliceRank=" + str(data[8]) + "\t" +
            " TaserLevel=" + str(data[9])
    );
'''

IsDouble = False;
SameUsers = 0;
SameAccs = 0;
index = 0;

for data in aData:
    IsDouble = False
    for data2 in aData2:
        if data[0] == data2[0]:
            #print("Same Username: " + data[0])
            IsDouble = True;
            SameUsers+=1
            if data[1] == data2[1]:
                #print("Same password: " + data[0])
                SameAccs+=1
                aMerged[index][0] = data[0]; #username (no need for max value search)
                aMerged[index][1] = data[1]; #password (no need for max value search)
                for i in range(3,10):
                    val = data[i];
                    val2 = data2[i];
                    if (val and val2):
                        print("merging (" + str(val) + ") and (" + str(val2) +")")
                        aMerged[index][i] = max(int(val), int(val2));
                    else:
                        print("skippig");
                index+=1
            else:
                print("[" + data[0] + "] has pw1(" + data[1] + ") pw2(" + data2[1] + ")" + "\t Money=(" + str(data[2]) + "/" + str(data2[2]) + ")")
    if not IsDouble:
        for i in range(10):
            aMerged[index][i] = data[i];
        index+=1

for merge in aMerged:
    print(
          " Name=" + merge[0] + "\t" +
          " Money=" + str(merge[2]) + "\t" +
          " xp=" + str(merge[3]) + "\t" +
          " Shit=" + str(merge[4]) + "\t" +
          " BlockPoints=" + str(merge[5]) + "\t" +
          " BlockKills=" + str(merge[6]) + "\t" +
          " BlockDeaths=" + str(merge[7]) + "\t" +
          " PoliceRank=" + str(merge[8]) + "\t" +
          " TaserLevel=" + str(merge[9])
          );


print("============================")

print("database1(" + str(total_accounts) + ")" + " database2(" + str(total_accounts2) + ")" + " sameusernames(" + str(SameUsers) + ")" + " SamePW(" + str(SameAccs) + ")")

print("failed to merge (" + str(SameUsers - SameAccs) + ") accounts because they had same username but different password")
print("merge index(" + str(index) + ")")



