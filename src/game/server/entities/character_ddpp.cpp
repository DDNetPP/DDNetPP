// ddnet++ generic character stuff

#include <engine/shared/config.h>
#include <game/server/gamecontext.h>
#include <engine/server/server.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/player.h>

#include "laser.h"
#include "projectile.h"
#include "plasmabullet.h"

#include "character.h"

void CCharacter::DDPPDDRacePostCoreTick()
{
	if (!isFreezed)
		m_FirstFreezeTick = 0;
}

void CCharacter::SetSpookyGhost()
{
	if (m_pPlayer->m_IsBlockTourning || (m_pPlayer->m_IsSurvivaling && m_pPlayer->m_IsSurvivalLobby == false)) // no ghost in competetive minigames
		return;

	if (!m_SpookyGhostWeaponsBackupped)
	{

		for (int i = 0; i < NUM_WEAPONS; i++)
		{
			m_aSpookyGhostWeaponsBackup[i][1] = m_aWeapons[i].m_Ammo;
			m_aSpookyGhostWeaponsBackupGot[i][1] = m_aWeapons[i].m_Got;
			m_aWeapons[i].m_Ammo = 0;
			m_aWeapons[i].m_Got = false;
		}
		m_SpookyGhostWeaponsBackupped = true;
		m_aWeapons[1].m_Got = 1;
		m_aWeapons[1].m_Ammo = -1;
	}

	str_copy(m_pPlayer->m_TeeInfos.m_SkinName, "ghost", sizeof(m_pPlayer->m_TeeInfos.m_SkinName));
	m_pPlayer->m_TeeInfos.m_UseCustomColor = 0;

	m_pPlayer->m_SpookyGhostActive = 1;
}

void CCharacter::UnsetSpookyGhost()
{
	if (m_SpookyGhostWeaponsBackupped)
	{
		for (int i = 0; i < NUM_WEAPONS; i++)
		{
			m_aWeapons[i].m_Got = m_aSpookyGhostWeaponsBackupGot[i][1];
			if (m_pPlayer->m_IsVanillaWeapons || m_pPlayer->m_SpawnShotgunActive || m_pPlayer->m_SpawnGrenadeActive || m_pPlayer->m_SpawnRifleActive)
			{
				m_aWeapons[i].m_Ammo = m_aSpookyGhostWeaponsBackup[i][1];
			}
			else
			{
				m_aWeapons[i].m_Ammo = -1;
			}
		}
		m_SpookyGhostWeaponsBackupped = false;
	}

	str_copy(m_pPlayer->m_TeeInfos.m_SkinName, m_pPlayer->m_RealSkinName, sizeof(m_pPlayer->m_TeeInfos.m_SkinName));
	m_pPlayer->m_TeeInfos.m_UseCustomColor = m_pPlayer->m_RealUseCustomColor;

	m_pPlayer->m_SpookyGhostActive = 0;

	return;
}

void CCharacter::SaveRealInfos()
{
	if (!m_pPlayer->m_SpookyGhostActive)
	{
		str_copy(m_pPlayer->m_RealSkinName, m_pPlayer->m_TeeInfos.m_SkinName, sizeof(m_pPlayer->m_RealSkinName));
		m_pPlayer->m_RealUseCustomColor = m_pPlayer->m_TeeInfos.m_UseCustomColor;
		str_copy(m_pPlayer->m_RealClan, Server()->ClientClan(m_pPlayer->GetCID()), sizeof(m_pPlayer->m_RealClan));
		str_copy(m_pPlayer->m_RealName, Server()->ClientName(m_pPlayer->GetCID()), sizeof(m_pPlayer->m_RealName));
	}

	return;
}

bool CCharacter::SetWeaponThatChrHas()
{
	if (m_aWeapons[WEAPON_GUN].m_Got)
		SetWeapon(WEAPON_GUN);
	else if (m_aWeapons[WEAPON_HAMMER].m_Got)
		SetWeapon(WEAPON_HAMMER);
	else if (m_aWeapons[WEAPON_GRENADE].m_Got)
		SetWeapon(WEAPON_GRENADE);
	else if (m_aWeapons[WEAPON_SHOTGUN].m_Got)
		SetWeapon(WEAPON_SHOTGUN);
	else if (m_aWeapons[WEAPON_RIFLE].m_Got)
		SetWeapon(WEAPON_RIFLE);
	else
		return false;

	return true;
}

void CCharacter::ShopWindow(int Dir)
{

	m_ShopMotdTick = 0;

	// if you add something to the shop make sure to also add pages here and extend conshop in ddracechat.cpp.

	int m_MaxShopPage = 13; // UPDATE THIS WITH EVERY PAGE YOU ADD!!!!!

	if (Dir == 0)
	{
		m_ShopWindowPage = 0;
	}
	else if (Dir == 1)
	{
		m_ShopWindowPage++;
		if (m_ShopWindowPage > m_MaxShopPage)
		{
			m_ShopWindowPage = 0;
		}
	}
	else if (Dir == -1)
	{
		m_ShopWindowPage--;
		if (m_ShopWindowPage < 0)
		{
			m_ShopWindowPage = m_MaxShopPage;
		}
	}

	
	char aItem[256];
	char aLevelTmp[128];
	char aPriceTmp[16];
	char aTimeTmp[256];
	char aInfo[1028];

	if (m_ShopWindowPage == 0)
	{
		str_format(aItem, sizeof(aItem), "Welcome to the shop! If you need help, use '/shop help'.\n\n"
			"By shooting to the right you go one site forward,\n"
			"and by shooting left you go one site backwards.\n\n"
			"If you need more help, visit '/shop help'.");
	}
	else if (m_ShopWindowPage == 1)
	{
		str_format(aItem, sizeof(aItem), "        ~  R A I N B O W  ~      ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "5");
		str_format(aPriceTmp, sizeof(aPriceTmp), "1.500");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item until you're dead.");
		str_format(aInfo, sizeof(aInfo), "Rainbow will make your tee change the color very fast.");
	}
	else if (m_ShopWindowPage == 2)
	{
		str_format(aItem, sizeof(aItem), "        ~  B L O O D Y  ~      ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "15");
		str_format(aPriceTmp, sizeof(aPriceTmp), "3.500");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item until you're dead.");
		str_format(aInfo, sizeof(aInfo), "Bloody will give your tee a permanent kill effect.");
	}
	else if (m_ShopWindowPage == 3)
	{
		str_format(aItem, sizeof(aItem), "        ~  C H I D R A Q U L  ~      ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "2");
		str_format(aPriceTmp, sizeof(aPriceTmp), "250");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item until\n"
			"you disconnect.");
		str_format(aInfo, sizeof(aInfo), "Chidraqul is a minigame by ChillerDragon.\n"
			"More information about this game coming soon.");
	}
	else if (m_ShopWindowPage == 4)
	{
		str_format(aItem, sizeof(aItem), "        ~  S H I T  ~      ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "0");
		str_format(aPriceTmp, sizeof(aPriceTmp), "5");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "Shit is a fun item. You can use to '/poop' on other players.\n"
			"You can also see your shit amount in your '/profile'.");
	}
	else if (m_ShopWindowPage == 5)
	{
		str_format(aItem, sizeof(aItem), "        ~  R O O M K E Y  ~      ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "16");
		str_format(aPriceTmp, sizeof(aPriceTmp), "%d", g_Config.m_SvRoomPrice);
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item until\n"
			"you disconnect.");
		str_format(aInfo, sizeof(aInfo), "If you have the room key you can enter the bank room.\n"
			"It's under the spawn and there is a money tile.");
	}
	else if (m_ShopWindowPage == 6)
	{
		str_format(aItem, sizeof(aItem), "        ~  P O L I C E  ~      ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "18");
		str_format(aPriceTmp, sizeof(aPriceTmp), "100.000");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "Police officers get help from the police bot.\n"
			"For more information about the specific police ranks\n"
			"please visit '/policeinfo'.");
	}
	else if (m_ShopWindowPage == 7)
	{
		str_format(aItem, sizeof(aItem), "        ~  T A S E R  ~      ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "30");
		str_format(aPriceTmp, sizeof(aPriceTmp), "50.000");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "Taser replaces your unfreeze rifle with a rifle that freezes\n"
			"other tees. You can toggle it using '/taser <on/off>'.\n"
			"For more information about the taser and your taser stats,\n"
			"plase visit '/taser info'.");
	}
	else if (m_ShopWindowPage == 8)
	{
		str_format(aItem, sizeof(aItem), "    ~  P V P A R E N A T I C K E T  ~  ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "0");
		str_format(aPriceTmp, sizeof(aPriceTmp), "150");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "You can join the pvp arena using '/pvp_arena join' if you have a ticket.");
	}
	else if (m_ShopWindowPage == 9)
	{
		str_format(aItem, sizeof(aItem), "       ~  N I N J A J E T P A C K  ~     ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "21");
		str_format(aPriceTmp, sizeof(aPriceTmp), "10.000");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "It will make your jetpack gun be a ninja.\n"
			"Toggle it using '/ninjajetpack'.");
	}
	else if (m_ShopWindowPage == 10)
	{
		str_format(aItem, sizeof(aItem), "     ~  S P A W N S H O T G U N  ~   ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "33");
		str_format(aPriceTmp, sizeof(aPriceTmp), "600.000");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "You will have shotgun if you respawn.\n"
			"For more information about spawn weapons,\n"
			"please visit '/spawnweaponsinfo'.");
	}
	else if (m_ShopWindowPage == 11)
	{
		str_format(aItem, sizeof(aItem), "      ~  S P A W N G R E N A D E  ~    ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "33");
		str_format(aPriceTmp, sizeof(aPriceTmp), "600.000");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "You will have grenade if you respawn.\n"
			"For more information about spawn weapons,\n"
			"please visit '/spawnweaponsinfo'.");
	}
	else if (m_ShopWindowPage == 12)
	{
		str_format(aItem, sizeof(aItem), "       ~  S P A W N R I F L E  ~       ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "33");
		str_format(aPriceTmp, sizeof(aPriceTmp), "600.000");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "You will have rifle if you respawn.\n"
			"For more information about spawn weapons,\n"
			"please visit '/spawnweaponsinfo'.");
	}
	else if (m_ShopWindowPage == 13)
	{
		str_format(aItem, sizeof(aItem), "       ~  S P O O K Y G H O S T  ~     ");
		str_format(aLevelTmp, sizeof(aLevelTmp), "1");
		str_format(aPriceTmp, sizeof(aPriceTmp), "1.000.000");
		str_format(aTimeTmp, sizeof(aTimeTmp), "You own this item forever.");
		str_format(aInfo, sizeof(aInfo), "Using this item you can hide from other players behind bushes.\n"
			"If your ghost is activated you will be able to shoot plasma\n"
			"projectiles. For more information please visit '/spookyghostinfo'.");
	}
	else
	{
		str_format(aItem, sizeof(aItem), "");
	}
	//////////////////// UPDATE m_MaxShopPage ON TOP OF THIS FUNCTION!!! /////////////////////////


	char aLevel[128];
	str_format(aLevel, sizeof(aLevel), "Needed level: %s", aLevelTmp);
	char aPrice[16];
	str_format(aPrice, sizeof(aPrice), "Price: %s", aPriceTmp);
	char aTime[256];
	str_format(aTime, sizeof(aTime), "Time: %s", aTimeTmp);


	char aBase[512];
	if (m_ShopWindowPage > 0)
	{
		str_format(aBase, sizeof(aBase),
			"***************************\n"
			"        ~  S H O P  ~      \n"
			"***************************\n\n"
			"%s\n\n"
			"%s\n"
			"%s\n"
			"%s\n\n"
			"%s\n\n"
			"***************************\n"
			"If you want to buy an item press f3.\n\n\n"
			"              ~ %d ~              ", aItem, aLevel, aPrice, aTime, aInfo, m_ShopWindowPage);
	}
	else
	{
		str_format(aBase, sizeof(aBase),
			"***************************\n"
			"        ~  S H O P  ~      \n"
			"***************************\n\n"
			"%s\n\n"
			"***************************\n"
			"If you want to buy an item press f3.", aItem);
	}

	GameServer()->AbuseMotd(aBase, GetPlayer()->GetCID());

	m_ShopMotdTick = Server()->Tick() + Server()->TickSpeed() * 10; // motd is there for 10 sec

	return;
}

void CCharacter::StartShop()
{
	if (!m_InShop)
		return;
	if (m_PurchaseState == 2) // already in buy confirmation state
		return;
	if (m_ShopWindowPage != -1)
		return;

	ShopWindow(0);
	m_PurchaseState = 1;
}

void CCharacter::ConfirmPurchase()
{
	if ((m_ShopWindowPage == -1) || (m_ShopWindowPage == 0))
		return;

	char aBuf[256];
	str_format(aBuf, sizeof(aBuf),
		"***************************\n"
		"        ~  S H O P  ~      \n"
		"***************************\n\n"
		"Are you sure you want to buy this item?\n\n"
		"f3 - yes\n"
		"f4 - no\n\n"
		"***************************\n");

	GameServer()->AbuseMotd(aBuf, GetPlayer()->GetCID());

	m_PurchaseState = 2;

	return;
}

void CCharacter::PurchaseEnd(bool canceled)
{
	if (m_PurchaseState != 2) // nothing to end here
		return;

	char aResult[256];
	if (canceled)
	{
		char aBuf[256];
		str_format(aResult, sizeof(aResult), "You canceled the purchase.");
		str_format(aBuf, sizeof(aBuf),
			"***************************\n"
			"        ~  S H O P  ~      \n"
			"***************************\n\n"
			"%s\n\n"
			"***************************\n", aResult);

		GameServer()->AbuseMotd(aBuf, GetPlayer()->GetCID());
	}
	else
	{
		BuyItem(m_ShopWindowPage);
		ShopWindow(0);
	}

	m_PurchaseState = 1;

	return;
}

void CCharacter::BuyItem(int ItemID)
{

	if ((g_Config.m_SvShopState == 1) && !m_InShop)
	{
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You have to be in the shop to buy some items.");
		return;
	}

	char aBuf[256];

	if (ItemID == 1)
	{
		if (m_Rainbow)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already own rainbow.");
			return;
		}

		if (m_pPlayer->GetLevel() < 5)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Your level is too low! You need to be Lv.5 to buy rainbow.");
		}
		else
		{
			if (m_pPlayer->GetMoney() >= 1500)
			{
				m_pPlayer->MoneyTransaction(-1500, "bought 'rainbow'");
				m_Rainbow = true;
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought rainbow until death.");
			}
			else
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money! You need 1.500 money.");
			}
		}
	}
	else if (ItemID == 2)
	{
		if (m_Bloody)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already own bloody.");
			return;
		}

		if (m_pPlayer->GetLevel() < 15)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Your level is too low! You need to be Lv.15 to buy bloody.");
		}
		else
		{
			if (m_pPlayer->GetMoney() >= 3500)
			{
				m_pPlayer->MoneyTransaction(-3500, "bought 'bloody'");
				m_Bloody = true;
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought bloody until death.");
			}
			else
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money! You need 3 500.");
			}
		}
	}
	else if (ItemID == 3)
	{
		if (m_pPlayer->GetLevel() < 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You need to be Lv.2 or higher to buy 'chidraqul'.");
			return;
		}
		if (m_pPlayer->m_BoughtGame)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already own this game.");
			return;
		}
		if (m_pPlayer->GetMoney() >= 250)
		{
			m_pPlayer->MoneyTransaction(-250, "bought 'chidraqul'");
			m_pPlayer->m_BoughtGame = true;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought 'chidraqul' until you disconnect. Check '/chidraqul info' for more information.");
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money! You need 250 money.");
		}
	}
	else if (ItemID == 4)
	{
		if (m_pPlayer->GetMoney() >= 5)
		{
			m_pPlayer->MoneyTransaction(-5, "bought 'shit'");

			m_pPlayer->m_shit++;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought shit.");
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money!");
		}
	}
	else if (ItemID == 5)
	{
		if (m_pPlayer->GetLevel() < 16)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You need to be Lv.16 or higher to buy a key.");
			return;
		}
		if (m_pPlayer->m_BoughtRoom)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already own this item.");
			return;
		}
		if (g_Config.m_SvRoomState == 0)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Room has been turned off by admin.");
			return;
		}
		if (m_pPlayer->GetMoney() >= g_Config.m_SvRoomPrice)
		{
			m_pPlayer->MoneyTransaction(-g_Config.m_SvRoomPrice, "bought 'room_key'");
			m_pPlayer->m_BoughtRoom = true;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought a key. You can now enter the bankroom until you disconnect.");
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money! You need 5.000 money.");
		}
	}
	else if (ItemID == 6)
	{
		if (m_pPlayer->m_PoliceRank == 0)
		{
			if (m_pPlayer->GetLevel() < 18)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 18 to buy police.");
				return;
			}
		}
		else if (m_pPlayer->m_PoliceRank == 1)
		{
			if (m_pPlayer->GetLevel() < 25)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 25 to upgrade police to level 2.");
				return;
			}
		}
		else if (m_pPlayer->m_PoliceRank == 2)
		{
			if (m_pPlayer->GetLevel() < 30)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 30 to upgrade police to level 3.");
				return;
			}
		}
		else if (m_pPlayer->m_PoliceRank == 3)
		{
			if (m_pPlayer->GetLevel() < 40)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 40 to upgrade police to level 4.");
				return;
			}
		}
		else if (m_pPlayer->m_PoliceRank == 4)
		{
			if (m_pPlayer->GetLevel() < 50)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 50 to upgrade police to level 5.");
				return;
			}
		}


		if (m_pPlayer->m_PoliceRank > 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already bought maximum police lvl.");
			return;
		}

		if (m_pPlayer->GetMoney() >= 100000)
		{
			m_pPlayer->MoneyTransaction(-100000, "bought 'police'");
			m_pPlayer->m_PoliceRank++;
			str_format(aBuf, sizeof(aBuf), "You bought PoliceRank[%d]!", m_pPlayer->m_PoliceRank);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Not enough money! You need 100.000 money.");
		}
	}
	else if (ItemID == 7)
	{
		if (m_pPlayer->m_PoliceRank < 3)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't own a weapon license.");
			return;
		}

		if (m_pPlayer->m_TaserLevel == 0)
		{
			m_pPlayer->m_TaserPrice = 50000;
		}
		else if (m_pPlayer->m_TaserLevel == 1)
		{
			m_pPlayer->m_TaserPrice = 75000;
		}
		else if (m_pPlayer->m_TaserLevel == 2)
		{
			m_pPlayer->m_TaserPrice = 100000;
		}
		else if (m_pPlayer->m_TaserLevel == 3)
		{
			m_pPlayer->m_TaserPrice = 150000;
		}
		else if (m_pPlayer->m_TaserLevel == 4)
		{
			m_pPlayer->m_TaserPrice = 200000;
		}
		else if (m_pPlayer->m_TaserLevel == 5)
		{
			m_pPlayer->m_TaserPrice = 200000;
		}
		else if (m_pPlayer->m_TaserLevel == 6)
		{
			m_pPlayer->m_TaserPrice = 200000;
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Taser already max level.");
			return;
		}

		if (m_pPlayer->GetMoney() < m_pPlayer->m_TaserPrice)
		{
			str_format(aBuf, sizeof(aBuf), "Not enough money to upgrade taser. You need %d money.", m_pPlayer->m_TaserPrice);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			return;
		}

		m_pPlayer->MoneyTransaction(-m_pPlayer->m_TaserPrice, "bought 'taser'");

		m_pPlayer->m_TaserLevel++;
		if (m_pPlayer->m_TaserLevel == 1)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought a taser. (Type '/taser help' for all cmds)");
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Taser has been upgraded.");
		}
	}
	else if (ItemID == 8)
	{
		if (m_pPlayer->GetMoney() >= 150)
		{
			m_pPlayer->MoneyTransaction(-150, "bought 'pvp_arena_ticket'");
			m_pPlayer->m_pvp_arena_tickets++;

			str_format(aBuf, sizeof(aBuf), "You bought a pvp_arena_ticket. You have %d tickets.", m_pPlayer->m_pvp_arena_tickets);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money! You need 150 money.");
		}
	}
	else if (ItemID == 9)
	{
		if (m_pPlayer->GetLevel() < 21)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 21 to buy ninjajetpack.");
			return;
		}
		else if (m_pPlayer->m_NinjaJetpackBought)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already own ninjajetpack.");
		}
		else if (m_pPlayer->GetMoney() >= 10000)
		{
			m_pPlayer->MoneyTransaction(-10000, "bought 'ninjajetpack'");

			m_pPlayer->m_NinjaJetpackBought = 1;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought ninjajetpack. Turn it on using '/ninjajetpack'.");
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money!");
		}
	}
	else if (ItemID == 10)
	{
		if (m_pPlayer->GetLevel() < 33)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 33 to buy spawn shotgun.");
			return;
		}
		else if (m_pPlayer->m_SpawnWeaponShotgun == 5)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already have the maximum level for spawn shotgun.");
		}
		else if (m_pPlayer->GetMoney() >= 600000)
		{
			m_pPlayer->MoneyTransaction(-600000, "bought 'spawn_shotgun'");

			m_pPlayer->m_SpawnWeaponShotgun++;
			if (m_pPlayer->m_SpawnWeaponShotgun == 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought spawn shotgun. For more infos check '/spawnweaponsinfo'.");
			}
			else if (m_pPlayer->m_SpawnWeaponShotgun > 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Spawn shotgun upgraded.");
			}
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money!");
		}
	}
	else if (ItemID == 11)
	{
		if (m_pPlayer->GetLevel() < 33)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 33 to buy spawn grenade.");
			return;
		}
		else if (m_pPlayer->m_SpawnWeaponGrenade == 5)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already have the maximum level for spawn grenade.");
		}
		else if (m_pPlayer->GetMoney() >= 600000)
		{
			m_pPlayer->MoneyTransaction(-600000, "bought 'spawn_grenade'");

			m_pPlayer->m_SpawnWeaponGrenade++;
			if (m_pPlayer->m_SpawnWeaponGrenade == 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought spawn grenade. For more infos check '/spawnweaponsinfo'.");
			}
			else if (m_pPlayer->m_SpawnWeaponGrenade > 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Spawn grenade upgraded.");
			}
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money!");
		}
	}
	else if (ItemID == 12)
	{
		if (m_pPlayer->GetLevel() < 33)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 33 to buy spawn rifle.");
			return;
		}
		else if (m_pPlayer->m_SpawnWeaponRifle == 5)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already have the maximum level for spawn rifle.");
		}
		else if (m_pPlayer->GetMoney() >= 600000)
		{
			m_pPlayer->MoneyTransaction(-600000, "bought 'spawn_rifle'");

			m_pPlayer->m_SpawnWeaponRifle++;
			if (m_pPlayer->m_SpawnWeaponRifle == 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought spawn rifle. For more infos check '/spawnweaponsinfo'.");
			}
			else if (m_pPlayer->m_SpawnWeaponRifle > 1)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Spawn rifle upgraded.");
			}
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money!");
		}
	}
	else if (ItemID == 13)
	{
		if (m_pPlayer->GetLevel() < 1)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Level is too low! You need lvl 1 to buy the spooky ghost.");
			return;
		}
		else if (m_pPlayer->m_SpookyGhost)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You already have the spooky ghost.");
		}
		else if (m_pPlayer->GetMoney() >= 1000000)
		{
			m_pPlayer->MoneyTransaction(-1000000, "bought 'spooky_ghost'");

			m_pPlayer->m_SpookyGhost = 1;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You bought the spooky ghost. For more infos check '/spookyghostinfo'.");
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), "You don't have enough money!");
		}
	}
	else
	{
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Invalid shop item. Choose another one.");
	}
	
	/*else if (!str_comp_nocase(aItem, "atom"))
	{
	if (pPlayer->GetCharacter()->m_Atom)
	{
	pSelf->SendChatTarget(pResult->m_ClientID, "You already own atom");
	return;
	}

	if (pPlayer->GetLevel() < 15)
	{
	pSelf->SendChatTarget(pResult->m_ClientID, "your level is too low! you need level 15 to buy atom.");
	}
	else
	{
	if (pPlayer->GetMoney() >= 3500)
	{
	pPlayer->MoneyTransaction(-3500, "bought pvp_arena_ticket");
	pPlayer->GetCharacter()->m_Atom = true;
	pSelf->SendChatTarget(pResult->m_ClientID, "you bought atom until death.");
	}
	else
	{
	pSelf->SendChatTarget(pResult->m_ClientID, "you don't have enough money! You need 3 500.");
	}
	}
	}
	else if (!str_comp_nocase(aItem, "trail"))
	{
	if (pPlayer->GetCharacter()->m_Trail)
	{
	pSelf->SendChatTarget(pResult->m_ClientID, "you already own trail");
	return;
	}

	if (pPlayer->GetLevel() < 15)
	{
	pSelf->SendChatTarget(pResult->m_ClientID, "your level is too low! you need level 15 to buy trail.");
	}
	else
	{
	if (pPlayer->GetMoney() >= 3500)
	{
	pPlayer->MoneyTransaction(-3500, "bought pvp_arena_ticket");
	pPlayer->GetCharacter()->m_Trail = true;
	pSelf->SendChatTarget(pResult->m_ClientID, "you bought trail until death.");
	}
	else
	{
	pSelf->SendChatTarget(pResult->m_ClientID, "you don't have enough money! You need 3 500.");
	}
	}
	}*/

	return;
}

void CCharacter::DropLoot()
{
	if (m_pPlayer->m_IsSurvivaling && !m_pPlayer->m_IsSurvivalLobby)
	{
		// survival weapon, health and weapon drops
		DropArmor(rand() % 6);
		DropHealth(rand() % 6);
		DropWeapon(WEAPON_GUN);
		DropWeapon(WEAPON_SHOTGUN);
		DropWeapon(WEAPON_GRENADE);
		DropWeapon(WEAPON_RIFLE);
	}
	else if (!GameServer()->IsMinigame(m_pPlayer->GetCID()))
	{
		int SpecialGun = 0;
		if (m_Jetpack || m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun)
			SpecialGun = 1;
		// block drop 0-2 weapons
		DropWeapon(rand() % (NUM_WEAPONS - (3+SpecialGun)) + (2-SpecialGun)); // no hammer or ninja and gun only if special gun
		DropWeapon(rand() % (NUM_WEAPONS - (3+SpecialGun)) + (2-SpecialGun));
	}
}

void CCharacter::DropHealth(int amount)
{
	if (amount > 64) { amount = 64; }
	for (int i = 0; i < amount; i++)
	{
		while (GameServer()->m_vDropLimit[POWERUP_HEALTH].size() > (long unsigned int)g_Config.m_SvMaxDrops)
		{
			GameServer()->m_vDropLimit[POWERUP_HEALTH][0]->Reset();
			GameServer()->m_vDropLimit[POWERUP_HEALTH].erase(GameServer()->m_vDropLimit[POWERUP_HEALTH].begin());
		}
		CDropPickup *p = new CDropPickup(
			&GameServer()->m_World,
			POWERUP_HEALTH,
			300, // lifetime
			m_pPlayer->GetCID(),
			rand() % 3 - 1, // direction
			(float)(amount / 5), // force
			Team()
		);
		GameServer()->m_vDropLimit[POWERUP_HEALTH].push_back(p);
	}
}

void CCharacter::DropArmor(int amount)
{
	if (amount > 64) { amount = 64; }
	for (int i = 0; i < amount; i++)
	{
		while (GameServer()->m_vDropLimit[POWERUP_ARMOR].size() > (long unsigned int)g_Config.m_SvMaxDrops)
		{
			GameServer()->m_vDropLimit[POWERUP_ARMOR][0]->Reset();
			GameServer()->m_vDropLimit[POWERUP_ARMOR].erase(GameServer()->m_vDropLimit[POWERUP_ARMOR].begin());
		}
		CDropPickup *p = new CDropPickup(
			&GameServer()->m_World,
			POWERUP_ARMOR,
			300, // lifetime
			m_pPlayer->GetCID(),
			rand() % 3 - 1, // direction
			(float)(amount / 5), // force
			Team()
		);
		GameServer()->m_vDropLimit[POWERUP_ARMOR].push_back(p);
	}
}

void CCharacter::DropWeapon(int WeaponID)
{

	if ((isFreezed) || (m_FreezeTime) || (!m_aWeapons[WeaponID].m_Got)
		|| (m_pPlayer->IsInstagibMinigame())
		|| (m_pPlayer->m_SpookyGhostActive && WeaponID != WEAPON_GUN)
		|| (WeaponID == WEAPON_NINJA)
		|| (WeaponID == WEAPON_HAMMER && !m_pPlayer->m_IsSurvivaling && g_Config.m_SvAllowDroppingWeapons != 1 && g_Config.m_SvAllowDroppingWeapons != 2)
		|| (WeaponID == WEAPON_GUN && !m_Jetpack && !m_autospreadgun && !m_pPlayer->m_InfAutoSpreadGun && !m_pPlayer->m_IsSurvivaling && g_Config.m_SvAllowDroppingWeapons != 1 && g_Config.m_SvAllowDroppingWeapons != 2)
		|| (WeaponID == WEAPON_RIFLE && (m_pPlayer->m_SpawnRifleActive || m_aDecreaseAmmo[WEAPON_RIFLE]) && g_Config.m_SvAllowDroppingWeapons != 1 && g_Config.m_SvAllowDroppingWeapons != 3)
		|| (WeaponID == WEAPON_SHOTGUN && (m_pPlayer->m_SpawnShotgunActive || m_aDecreaseAmmo[WEAPON_SHOTGUN]) && g_Config.m_SvAllowDroppingWeapons != 1 && g_Config.m_SvAllowDroppingWeapons != 3)
		|| (WeaponID == WEAPON_GRENADE && (m_pPlayer->m_SpawnGrenadeActive || m_aDecreaseAmmo[WEAPON_GRENADE]) && g_Config.m_SvAllowDroppingWeapons != 1 && g_Config.m_SvAllowDroppingWeapons != 3)
		)
	{
		return;
	}

	if (m_pPlayer->m_vWeaponLimit[WeaponID].size() == 5)
	{
		m_pPlayer->m_vWeaponLimit[WeaponID][0]->Reset();
		m_pPlayer->m_vWeaponLimit[WeaponID].erase(m_pPlayer->m_vWeaponLimit[WeaponID].begin());
	}

	int m_CountWeapons = 0;

	for (int i = 5; i > -1; i--)
	{
		if (m_aWeapons[i].m_Got)
			m_CountWeapons++;
	}

	if (WeaponID == WEAPON_GUN && (m_Jetpack || m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun))
	{
		if (m_Jetpack && (m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun))
		{
			m_Jetpack = false;
			m_autospreadgun = false;
			m_pPlayer->m_InfAutoSpreadGun = false;
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You lost your jetpack gun");
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You lost your spread gun");
			GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));

			CWeapon *Weapon = new CWeapon(&GameServer()->m_World, WeaponID, 300, m_pPlayer->GetCID(), GetAimDir(), Team(), m_aWeapons[WeaponID].m_Ammo, true, true);
			m_pPlayer->m_vWeaponLimit[WEAPON_GUN].push_back(Weapon);
		}
		else if (m_Jetpack)
		{
			m_Jetpack = false;
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You lost your jetpack gun");
			GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));

			CWeapon *Weapon = new CWeapon(&GameServer()->m_World, WeaponID, 300, m_pPlayer->GetCID(), GetAimDir(), Team(), m_aWeapons[WeaponID].m_Ammo, true);
			m_pPlayer->m_vWeaponLimit[WEAPON_GUN].push_back(Weapon);
		}
		else if (m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun)
		{
			m_autospreadgun = false;
			m_pPlayer->m_InfAutoSpreadGun = false;
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You lost your spread gun");
			GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));

			CWeapon *Weapon = new CWeapon(&GameServer()->m_World, WeaponID, 300, m_pPlayer->GetCID(), GetAimDir(), Team(), m_aWeapons[WeaponID].m_Ammo, false, true);
			m_pPlayer->m_vWeaponLimit[WEAPON_GUN].push_back(Weapon);
		}
	}
	else if (m_CountWeapons > 1)
	{
		m_aWeapons[WeaponID].m_Got = false;
		GameServer()->CreateSound(m_Pos, SOUND_WEAPON_NOAMMO, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));

		CWeapon *Weapon = new CWeapon(&GameServer()->m_World, WeaponID, 300, m_pPlayer->GetCID(), GetAimDir(), Team(), m_aWeapons[WeaponID].m_Ammo);
		m_pPlayer->m_vWeaponLimit[WeaponID].push_back(Weapon);
	}

	SetWeaponThatChrHas();
	return;
}

void CCharacter::PvPArenaTick()
{
	if (m_pvp_arena_tele_request_time < 0)
		return;
	m_pvp_arena_tele_request_time--;

	if (m_pvp_arena_tele_request_time == 1)
	{
		if (m_pvp_arena_exit_request)
		{
			m_pPlayer->m_pvp_arena_tickets++;
			m_Health = 10;
			m_IsPVParena = false;
			m_isDmg = false;

			if (g_Config.m_SvPvpArenaState == 3) //tilebased and not hardcodet
			{
				m_Core.m_Pos = m_pPlayer->m_PVP_return_pos;
			}

			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[PVP] Successfully teleported out of arena.");
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[PVP] You got your ticket back because you have survived.");
		}
		else // join request
		{
			m_pPlayer->m_pvp_arena_tickets--;
			m_pPlayer->m_pvp_arena_games_played++;
			m_IsPVParena = true;
			m_isDmg = true;
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[PVP] Teleporting to arena... good luck and have fun!");
		}
	}

	if (m_Core.m_Vel.x < -0.02f || m_Core.m_Vel.x > 0.02f || m_Core.m_Vel.y != 0.0f)
	{
		GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[PVP] Teleport failed because you have moved.");
		m_pvp_arena_tele_request_time = -1;
	}
}

void CCharacter::DDPP_Tick()
{
	char aBuf[256];

	PvPArenaTick();

	if (m_RandomCosmetics)
	{
		if (Server()->Tick() % 22 == 0)
		{
			int r = rand() % 10;
			if (r == 0)
			{
				m_Rainbow ^= true;
			}
			else if (r == 1)
			{
				//m_StrongBloody ^= true;
			}
			else if (r == 2)
			{
				m_Bloody ^= true;
			}
			else if (r == 3)
			{
				m_Atom ^= true;
			}
			else if (r == 4)
			{
				m_Trail ^= true;
			}
			else if (r == 5)
			{
				m_autospreadgun ^= true;
			}
			else if (r > 8)
			{
				m_ninjasteam = true;
			}


			if (Server()->Tick() % 5 == 0 && m_ninjasteam)
			{
				m_ninjasteam = false;
			}
		}
	}

	if (GameServer()->m_BlockWaveGameState)
	{
		if (m_pPlayer->m_IsBlockWaving)
		{
			if (m_FreezeTime > 0 && !m_pPlayer->m_IsBlockWaveDead)
			{
				BlockWaveFreezeTicks++; //gets set to zer0 in Unfreeze() func
				if (BlockWaveFreezeTicks > Server()->TickSpeed() * 4)
				{
					str_format(aBuf, sizeof(aBuf), "[BlockWave] '%s' died.", Server()->ClientName(m_pPlayer->GetCID()));
					GameServer()->SendBlockWaveSay(aBuf);
					m_pPlayer->m_IsBlockWaveDead = true; //also gets set to zer0 on Unfreezefunc
				}
			}
		}
	}

	if (m_pPlayer->m_IsBlockTourning)
	{
		if (GameServer()->m_BlockTournaState == 2) //only do it ingame
		{
			if (m_FreezeTime)
			{
				m_BlockTournaDeadTicks++;
				if (m_BlockTournaDeadTicks > 15 * Server()->TickSpeed())
				{
					Die(m_pPlayer->GetCID(), WEAPON_SELF);
				}
			}
			else
			{
				m_BlockTournaDeadTicks = 0;
			}
		}
	}

	//spawnblock reducer
	if (Server()->Tick() % 1200 == 0 && m_pPlayer->m_SpawnBlocks > 0)
	{
		m_pPlayer->m_SpawnBlocks--;
	}

	//Block points (clear touchid on freeze and unfreeze agian)
	//if (m_pPlayer->m_LastToucherID != -1 && isFreezed) //didn't use m_FreezeTime because we want a freeze tile here not an freezelaser or something (idk about freeze canons)
	//{
	//	m_pPlayer->m_BlockWasTouchedAndFreezed = true;
	//}
	//if (m_pPlayer->m_BlockWasTouchedAndFreezed && m_FreezeTime == 0) //player got touched and freezed and unfreezed agian --> reset toucher because it isnt his kill anymore
	//{
	//	m_pPlayer->UpdateLastToucher(-1);
	//}
	//Better system: Remove LastToucherID after some unfreeze time this has less bugs and works also good in other situations like: your racing with your mate and then you rush away solo and fail and suicide (this situation wont count as kill). 
	if (m_pPlayer->m_LastToucherID != -1 && m_FreezeTime == 0)
	{
		//char aBuf[64];
		//str_format(aBuf, sizeof(aBuf), "ID: %d is not -1", m_pPlayer->m_LastToucherID); //ghost debug
		//dbg_msg("block", aBuf);

		m_pPlayer->m_LastTouchTicks++;
		if (m_pPlayer->m_LastTouchTicks > Server()->TickSpeed() * 3) //3 seconds unfreeze --> wont die block death on freeze suicide
		{
			//char aBuf[64];
			//str_format(aBuf, sizeof(aBuf), "'%s' [ID: %d] touch removed", Server()->ClientName(m_pPlayer->m_LastToucherID), m_pPlayer->m_LastToucherID);
			//GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			m_pPlayer->UpdateLastToucher(-1);
		}
	}

	/*
	// wtf why did i code that? xd
	// wait until blocker disconnects to not count as blocked ?!?
	//clear last toucher on disconnect/unexistance
	if (!GameServer()->m_apPlayers[m_pPlayer->m_LastToucherID])
	{
		m_pPlayer->UpdateLastToucher(-1);
	}
	*/

	//Block points (check for last touched player)
	//pikos hook check
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CCharacter *pChar = GameServer()->GetPlayerChar(i);

		if (!pChar || !pChar->IsAlive() || pChar == this)
			continue;
		if (pChar->Core()->m_HookedPlayer == m_pPlayer->GetCID())
		{
			m_pPlayer->UpdateLastToucher(i);

			//was debugging because somekills at spawn werent recongized. But now i know that the dummys just kill to fast even before getting freeze --> not a block kill. But im ok with it spawnblock farming bots isnt nice anyways
			//dbg_msg("debug", "[%d:%s] hooked [%d:%s]", i, Server()->ClientName(i), m_pPlayer->GetCID(), Server()->ClientName(m_pPlayer->GetCID()));
		}
	}
	if (m_Core.m_HookState == HOOK_GRABBED)
	{
		//m_Dummy_nn_touched_by_humans = true;
		//GameServer()->SendChat(m_pPlayer->GetCID(), CGameContext::CHAT_ALL, "dont get in my hook -.-");

		//Quest 2 level 8 Block 3 tees without using hook
		if (m_pPlayer->m_QuestState == CPlayer::QUEST_BLOCK && m_pPlayer->m_QuestStateLevel == 8)
		{
			if (m_pPlayer->m_QuestProgressValue)
			{
				//GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] don't use hook!");
				GameServer()->QuestFailed(m_pPlayer->GetCID());
			}
		}
	}

	// selfmade nobo code check if pChr is too near
	CCharacter *pChr = GameServer()->m_World.ClosestCharType(m_Pos, true, this);
	if (pChr && pChr->IsAlive() && pChr->m_Input.m_Direction) // no afk killers pls
	{
		// only count touches from unfreezed & grounded tees
		if (pChr->m_FreezeTime == 0 && pChr->IsGrounded())
		{
			if (pChr->m_Pos.x < m_Core.m_Pos.x + 45 && pChr->m_Pos.x > m_Core.m_Pos.x - 45 && pChr->m_Pos.y < m_Core.m_Pos.y + 50 && pChr->m_Pos.y > m_Core.m_Pos.y - 50)
			{
				m_pPlayer->UpdateLastToucher(pChr->GetPlayer()->GetCID());
			}
		}
	}

	//hook extras
	//if (m_pPlayer->m_IsHookRainbow)
	//{

	//}


	//dbg_msg("", "koordinaten: x=%d y=%d", (int)(m_Pos.x / 32.f), (int)(m_Pos.y / 32.f));
	//survivexp stuff
	if (m_AliveTime)
	{
		if (Server()->Tick() >= m_AliveTime + Server()->TickSpeed() * 6000)  //100min
		{
			m_survivexpvalue = 4;
		}
		else if (Server()->Tick() >= m_AliveTime + Server()->TickSpeed() * 3600)  //60min
		{
			m_survivexpvalue = 3;
		}
		else if (Server()->Tick() >= m_AliveTime + Server()->TickSpeed() * 1200)  //20min
		{
			m_survivexpvalue = 2;
		}
		else if (Server()->Tick() >= m_AliveTime + Server()->TickSpeed() * 300) //5min
		{
			m_survivexpvalue = 1;
		}
	}

	DDPP_FlagTick();

	if (m_pPlayer->m_GiftDelay > 0)
	{
		m_pPlayer->m_GiftDelay--;
		if (m_pPlayer->m_GiftDelay == 1)
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[GIFT] delay expired.");
		}
	}

	if (m_pPlayer->m_JailTime > 0)
	{
		m_pPlayer->m_EscapeTime = 0;
		m_pPlayer->m_JailTime--;
		char aBuf[256];
		str_format(aBuf, sizeof(aBuf), "Your are arrested for %d seconds. \nType '/hide jail' to hide this info.", m_pPlayer->m_JailTime / Server()->TickSpeed());
		if (Server()->Tick() % 40 == 0)
		{
			if (!m_pPlayer->m_hidejailmsg)
			{
				GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
			}
		}
		if (m_pPlayer->m_JailTime == 1)
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[JAIL] You were released from jail.");
			vec2 JailReleaseSpawn = GameServer()->Collision()->GetRandomTile(TILE_JAILRELEASE);
			//vec2 DefaultSpawn = GameServer()->Collision()->GetRandomTile(ENTITY_SPAWN);

			if (JailReleaseSpawn != vec2(-1, -1))
			{
				SetPosition(JailReleaseSpawn);
			}
			else //no jailrelease
			{
				//SetPosition(DefaultSpawn); //crashbug for mod stealer
				GameServer()->SendChatTarget(GetPlayer()->GetCID(), "[JAIL] no jail release spot found.");
			}
		}
	}

	if (m_pPlayer->m_EscapeTime > 0)
	{
		m_pPlayer->m_EscapeTime--;
		char aBuf[256];
		if (m_isDmg)
		{
			str_format(aBuf, sizeof(aBuf), "Avoid policehammers for the next %d seconds. \n!WARNING! DAMAGE IS ACTIVATED ON YOU!\nType '/hide jail' to hide this info.", m_pPlayer->m_EscapeTime / Server()->TickSpeed());
		}
		else
		{
			str_format(aBuf, sizeof(aBuf), "Avoid policehammers for the next %d seconds. \nType '/hide jail' to hide this info.", m_pPlayer->m_EscapeTime / Server()->TickSpeed());
		}

		if (Server()->Tick() % Server()->TickSpeed() * 60 == 0)
		{
			if (!m_pPlayer->m_hidejailmsg)
			{
				GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
			}
		}
		if (m_pPlayer->m_EscapeTime == 1)
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCID(), "Your life as a gangster is over. You are free now.");
			GameServer()->AddEscapeReason(GetPlayer()->GetCID(), "unknown");
			m_isDmg = false;
			m_Health = 10;
		}
	}

	if (g_Config.m_SvPvpArenaState == 1 || g_Config.m_SvPvpArenaState == 2) //the two old hardcodet maps ChillBlock5 and BlmapChill (new system uses tiles)
	{
		if (g_Config.m_SvPvpArenaState == 1) // ChillBlock5 pvp
		{
			if (m_IsPVParena)
			{
				if (m_Core.m_Pos.x > 414 * 32 && m_Core.m_Pos.x < 447 * 32 && m_Core.m_Pos.y < 175 * 32 && m_Core.m_Pos.y > 160 * 32) //in arena
				{

				}
				else //not in arena
				{

					m_pPlayer->m_PVP_return_posX = m_Core.m_Pos.x;
					m_pPlayer->m_PVP_return_posY = m_Core.m_Pos.y;


					//if not in arena tele to random arena spawn:

					int r = rand() % 3; // 0 1 2
					if (r == 0)
					{
						m_Core.m_Pos.x = 420 * 32;
						m_Core.m_Pos.y = 166 * 32 - 5;
					}
					else if (r == 1)
					{
						m_Core.m_Pos.x = 430 * 32;
						m_Core.m_Pos.y = 170 * 32;
					}
					else if (r == 2)
					{
						m_Core.m_Pos.x = 440 * 32;
						m_Core.m_Pos.y = 166 * 32 - 5;
					}

				}
			}
			else //not in pvp mode
			{
				if (m_Core.m_Pos.x > 414 * 32 && m_Core.m_Pos.x < 447 * 32 && m_Core.m_Pos.y < 175 * 32 && m_Core.m_Pos.y > 160 * 32) //in arena
				{
					m_Core.m_Pos.x = m_pPlayer->m_PVP_return_posX;
					m_Core.m_Pos.y = m_pPlayer->m_PVP_return_posY;
				}
				else //not in arena
				{

				}
			}
		}
		else if (g_Config.m_SvPvpArenaState == 2) // BlmapChill pvp
		{
			if (m_IsPVParena)
			{
				if (m_Core.m_Pos.x > 357 * 32 && m_Core.m_Pos.x < 369 * 32 && m_Core.m_Pos.y < 380 * 32 && m_Core.m_Pos.y > 364 * 32) //in arena
				{

				}
				else //not in arena
				{

					m_pPlayer->m_PVP_return_posX = m_Core.m_Pos.x;
					m_pPlayer->m_PVP_return_posY = m_Core.m_Pos.y;


					//if not in arena tele to random arena spawn:

					int r = rand() % 3; // 0 1 2
					if (r == 0)
					{
						m_Core.m_Pos.x = 360 * 32 + 44;
						m_Core.m_Pos.y = 379 * 32;
					}
					else if (r == 1)
					{
						m_Core.m_Pos.x = 366 * 32 + 53;
						m_Core.m_Pos.y = 379 * 32;
					}
					else if (r == 2)
					{
						m_Core.m_Pos.x = 363 * 32 + 53;
						m_Core.m_Pos.y = 373 * 32;
					}

				}
			}
			else //not in pvp mode
			{
				if (m_Core.m_Pos.x > 357 * 32 && m_Core.m_Pos.x < 369 * 32 && m_Core.m_Pos.y < 380 * 32 && m_Core.m_Pos.y > 364 * 32) //in arena
				{
					m_Core.m_Pos.x = m_pPlayer->m_PVP_return_posX;
					m_Core.m_Pos.y = m_pPlayer->m_PVP_return_posY;
				}
				else //not in arena
				{

				}
			}
		}
	}

	//Marcella's room code (used to slow down chat message spam)
	if (IsAlive() && (Server()->Tick() % 80) == 0 && m_WasInRoom)
	{
		m_WasInRoom = false;
	}

	if (Server()->Tick() % 200 == 0) //ddpp public slow tick
	{
		if (m_pPlayer->m_ShowInstaScoreBroadcast)
			m_UpdateInstaScoreBoard = true;
	}

	if (m_UpdateInstaScoreBoard) //gets printed on update or every 200 % whatever modulo ticks
	{
		if (m_pPlayer->m_IsInstaArena_gdm)
		{
			str_format(aBuf, sizeof(aBuf), "score: %04d/%04d                                                                                                                 0", m_pPlayer->m_InstaScore, g_Config.m_SvGrenadeScorelimit);
			GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
		}
		if (m_pPlayer->m_IsInstaArena_idm)
		{
			str_format(aBuf, sizeof(aBuf), "score: %04d/%04d                                                                                                                 0", m_pPlayer->m_InstaScore, g_Config.m_SvRifleScorelimit);
			GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
		}
	}
	m_UpdateInstaScoreBoard = false;

	//General var resetter by ChillerDragon [ I M P O R T A N T] leave var resetter last --> so it wont influence ddpp tick stuff
	if (Server()->Tick() % 20 == 0)
	{
		if (m_InBank)
		{
			if (m_TileIndex != TILE_BANK_IN && m_TileFIndex != TILE_BANK_IN)
			{
				GameServer()->SendBroadcast(" ", m_pPlayer->GetCID(), 0);
				m_InBank = false; // DDracePostCoreTick() (which handels tiles) is after DDPP_Tick() so while being in bank it will never be false because tiles are always stronger than DDPP tick        <---- this comment was made before the tile checker if clause but can be interesting for further resettings
			}
		}
		if (m_InShop)
		{
			if (m_TileIndex != TILE_SHOP && m_TileFIndex != TILE_SHOP)
			{
				if (m_pPlayer->m_ShopBotAntiSpamTick <= Server()->Tick())
				{
					SendShopMessage("Bye! Come back if you need something.");
					m_pPlayer->m_ShopBotAntiSpamTick = Server()->Tick() + Server()->TickSpeed() * 5;
				}

				if (m_ShopWindowPage != -1)
				{
					GameServer()->AbuseMotd("", GetPlayer()->GetCID());
				}

				GameServer()->SendBroadcast("", m_pPlayer->GetCID(), 0);

				m_PurchaseState = 0;
				m_ShopWindowPage = -1;

				m_InShop = false;
			}
		}
	}
	//fast resets
	m_InJailOpenArea = false;

}

void CCharacter::DDPP_FlagTick()
{
	if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(this) == -1)
		return;

	if (!m_pPlayer->IsLoggedIn())
		return; // GameServer()->SendBroadcast("You need an account to get xp from flags. \n Get an Account with '/register (name) (pw) (pw)'", m_pPlayer->GetCID());

	if (Server()->Tick() % 50 == 0)
	{
		if (((m_TileIndex == TILE_MONEY) || (m_TileFIndex == TILE_MONEY)))
			return;
		if (((m_TileIndex == TILE_MONEY_POLICE) || (m_TileFIndex == TILE_MONEY_POLICE)))
			return;
		if (((m_TileIndex == TILE_MONEY_DOUBLE) || (m_TileFIndex == TILE_MONEY_DOUBLE)))
			return;

		// no matter where (bank, moneytile, ...) quests are independent
		if (m_pPlayer->m_QuestState == CPlayer::QUEST_FARM)
		{
			if (m_pPlayer->m_QuestStateLevel == 9)
			{
				m_pPlayer->m_QuestProgressValue2++;
				if (m_pPlayer->m_QuestProgressValue2 > 20)
				{
					GameServer()->QuestAddProgress(m_pPlayer->GetCID(), 10);
					m_pPlayer->m_QuestProgressValue2 = 0;
				}
			}
		}

		if (m_pPlayer->IsMaxLevel())
		{
			if (m_pPlayer->m_xpmsg)
			{
				GameServer()->SendBroadcast("[FLAG] You reached the maximum level.", m_pPlayer->GetCID(), 0);
			}
			return;
		}

		int VIPBonus = 0;

		// vip+ get 2 bonus
		if (m_pPlayer->m_IsSuperModerator)
		{
			m_pPlayer->GiveXP(2);
			m_pPlayer->MoneyTransaction(+2);

			VIPBonus = 2;
		}

		// vip get 1 bonus
		else if (m_pPlayer->m_IsModerator)
		{
			m_pPlayer->GiveXP(1);
			m_pPlayer->MoneyTransaction(+1);

			VIPBonus = 1;
		}

		if (m_InBank && GameServer()->m_IsBankOpen)
		{
			if (VIPBonus)
			{
				if (!m_pPlayer->m_xpmsg)
				{
					GameServer()->SendBroadcast("~ B A N K ~", m_pPlayer->GetCID(), 0);
					// GameServer()->SendChatTarget(GetPlayer()->GetCID(), "You entered the bank. You can rob the bank with '/rob_bank'");  // lol no spam old unused commands pls
				}
				else if (m_survivexpvalue == 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ B A N K ~\nXP [%llu/%llu] +1 flag +%d vip", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus);
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					m_pPlayer->GiveXP(1);
				}
				else if (m_survivexpvalue > 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ B A N K ~\nXP [%llu/%llu] +1 flag +%d vip + %d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_survivexpvalue);
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					m_pPlayer->GiveXP(1); //flag
					m_pPlayer->GiveXP(m_survivexpvalue); // survival
				}
			}
			else
			{
				if (!m_pPlayer->m_xpmsg)
				{
					GameServer()->SendBroadcast("~ B A N K ~", m_pPlayer->GetCID(), 0);
				}
				else if (m_survivexpvalue == 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ B A N K ~\nXP [%llu/%llu] +1 flag", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP());
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					m_pPlayer->GiveXP(1);
				}
				else if (m_survivexpvalue > 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ B A N K ~\nXP [%llu/%llu] +1 flag +%d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_survivexpvalue);
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					m_pPlayer->GiveXP(1); //flag
					m_pPlayer->GiveXP(m_survivexpvalue); // survival
				}
			}
		}
		else if (m_InShop)
		{
			if (!m_pPlayer->m_xpmsg)
			{
				GameServer()->SendBroadcast("~ S H O P ~", m_pPlayer->GetCID(), 0);
			}
			else if (m_survivexpvalue == 0)
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "~ S H O P ~\nXP [%llu/%llu] +1 flag +%d vip", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus);
				GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
				m_pPlayer->GiveXP(1);
			}
			else if (m_survivexpvalue > 0)
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "~ S H O P ~\nXP [%llu/%llu] +1 flag +%d vip + %d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_survivexpvalue);
				GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
				m_pPlayer->GiveXP(1); //flag
				m_pPlayer->GiveXP(m_survivexpvalue); // survival
			}
			else
			{
				if (!m_pPlayer->m_xpmsg)
				{
					GameServer()->SendBroadcast("~ S H O P ~", m_pPlayer->GetCID(), 0);
				}
				else if (m_survivexpvalue == 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ S H O P ~\nXP [%llu/%llu] +1 flag", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP());
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					m_pPlayer->GiveXP(1);
				}
				else if (m_survivexpvalue > 0)
				{
					char aBuf[256];
					str_format(aBuf, sizeof(aBuf), "~ S H O P ~\nXP [%llu/%llu] +1 flag +%d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_survivexpvalue);
					GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
					m_pPlayer->GiveXP(1); //flag
					m_pPlayer->GiveXP(m_survivexpvalue); // survival
				}
			}
		}
		else  //not in bank
		{
			if (VIPBonus)
			{
				if (m_pPlayer->m_xpmsg)
				{
					if (m_survivexpvalue == 0)
					{
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "XP [%llu/%llu] +1 flag +%d vip", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
						m_pPlayer->GiveXP(1);
					}
					else if (m_survivexpvalue > 0)
					{
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "XP [%llu/%llu] +1 flag +%d vip +%d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), VIPBonus, m_survivexpvalue);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
						m_pPlayer->GiveXP(1); //flag
						m_pPlayer->GiveXP(m_survivexpvalue); // survival
					}
				}
			}
			else
			{
				if (m_pPlayer->m_xpmsg)
				{
					if (m_survivexpvalue == 0)
					{
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "XP [%llu/%llu] +1 flag", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP());
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
						m_pPlayer->GiveXP(1);
					}
					else if (m_survivexpvalue > 0)
					{
						char aBuf[256];
						str_format(aBuf, sizeof(aBuf), "XP [%llu/%llu] +1 flag +%d survival", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP(), m_survivexpvalue);
						GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCID(), 0);
						m_pPlayer->GiveXP(1); //flag
						m_pPlayer->GiveXP(m_survivexpvalue); // survival
					}
				}
			}
		}
	}
}

bool CCharacter::DDPP_Respawn()
{

	vec2 SpawnPos;

	if(!GameServer()->m_pController->CanSpawn(m_pPlayer->GetTeam(), &SpawnPos, m_pPlayer))
		return false;

	if (Server()->IsRecording(m_pPlayer->GetCID()))
		Server()->StopRecord(m_pPlayer->GetCID());

	SetPosition(SpawnPos);
	return true;
}

int CCharacter::DDPP_DIE(int Killer, int Weapon, bool fngscore)
{
	char aBuf[256];

	if (m_pPlayer->m_IsVanillaModeByTile) //reset vanilla mode but never go out of vanilla mode in survival
	{
		m_pPlayer->m_IsVanillaDmg = false;
		m_pPlayer->m_IsVanillaWeapons = false;
	}
	if (m_pPlayer->m_IsSurvivaling)
	{
		m_pPlayer->m_IsVanillaDmg = true;
		m_pPlayer->m_IsVanillaWeapons = true;
		m_pPlayer->m_IsVanillaCompetetive = true;
	}

	if (m_pPlayer->m_IsDummy && m_pPlayer->m_DummyMode == 33) //chillintelligenz
	{
		CIRestart();
	}

	// remove atom projectiles on death
	if (!m_AtomProjs.empty())
	{
		for (std::vector<CStableProjectile *>::iterator it = m_AtomProjs.begin(); it != m_AtomProjs.end(); ++it)
		{
			GameServer()->m_World.DestroyEntity(*it);
		}
		m_AtomProjs.clear();
	}

	// remove trail projectiles on death
	if (!m_TrailProjs.empty())
	{
		for (std::vector<CStableProjectile *>::iterator it = m_TrailProjs.begin(); it != m_TrailProjs.end(); ++it)
		{
			GameServer()->m_World.DestroyEntity(*it);
		}
		m_TrailProjs.clear();
	}

	Killer = BlockPointsMain(Killer, fngscore);
	BlockSpawnProt(Killer); //idk if this should be included in BlockPointsMain() but spawnkills no matter what kind are evil i guess but then we should rename it to SpawnKillProt() imo
	//BlockQuestSubDieFuncBlockKill(Killer); //leave this before killing sprees to also have information about killingspree values from dead tees (needed for quest2 lvl6) //included in BlockPointsMain because it handels block kills
	BlockQuestSubDieFuncDeath(Killer); //only handling quest failed (using external func because the other player is needed and its good to extract it in antoher func and because im funcy now c:) //new reason the first func is blockkill and this one is all kinds of death
	KillingSpree(Killer); // previously called BlockKillingSpree()
	BlockTourna_Die(Killer);
	DropLoot(); // has to be called before survival because it only droops loot if survival alive
	InstagibSubDieFunc(Killer, Weapon);
	SurvivalSubDieFunc(Killer, Weapon);

	if (GameServer()->IsDDPPgametype("battlegores"))
		if (GameServer()->m_apPlayers[Killer] && Killer != m_pPlayer->GetCID())
			GameServer()->m_apPlayers[Killer]->m_Score++;

	// TODO: combine with insta 1on1
	// insta kills
	if (Killer != m_pPlayer->GetCID() && GameServer()->m_apPlayers[Killer])
	{
		if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm || GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm)
		{
			GameServer()->DoInstaScore(3, Killer);
		}
		else if (GameServer()->IsDDPPgametype("fng"))
		{
			GameServer()->m_apPlayers[Killer]->m_Score += 3;
		}
	}

	// TODO: refactor this code and put it in own function
	// insta 1on1
	if (GameServer()->m_apPlayers[Killer])
	{
		if (GameServer()->m_apPlayers[Killer]->m_Insta1on1_id != -1 && Killer != m_pPlayer->GetCID() && (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm || GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm)) //is in 1on1
		{
			GameServer()->m_apPlayers[Killer]->m_Insta1on1_score++;
			str_format(aBuf, sizeof(aBuf), "%s:%d killed %s:%d", Server()->ClientName(Killer), GameServer()->m_apPlayers[Killer]->m_Insta1on1_score, Server()->ClientName(m_pPlayer->GetCID()), m_pPlayer->m_Insta1on1_score);
			if (!GameServer()->m_apPlayers[Killer]->m_HideInsta1on1_killmessages)
			{
				GameServer()->SendChatTarget(Killer, aBuf);
			}
			if (!m_pPlayer->m_HideInsta1on1_killmessages)
			{
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			}
			if (GameServer()->m_apPlayers[Killer]->m_Insta1on1_score >= 5)
			{
				GameServer()->WinInsta1on1(Killer, m_pPlayer->GetCID());
			}
		}
	}

	// TODO: refactor this code and put it in own function
	// balance battle
	if (m_pPlayer->m_IsBalanceBatteling && GameServer()->m_BalanceBattleState == 2) //ingame in a balance battle
	{
		if (GameServer()->m_BalanceID1 == m_pPlayer->GetCID())
		{
			if (GameServer()->m_apPlayers[GameServer()->m_BalanceID2])
			{
				GameServer()->SendChatTarget(GameServer()->m_BalanceID2, "[balance] you won!");
				GameServer()->SendChatTarget(GameServer()->m_BalanceID1, "[balance] you lost!");
				GameServer()->m_apPlayers[GameServer()->m_BalanceID1]->m_IsBalanceBatteling = false;
				GameServer()->m_apPlayers[GameServer()->m_BalanceID2]->m_IsBalanceBatteling = false;
				GameServer()->m_BalanceBattleState = 0;
				if (GameServer()->GetPlayerChar(GameServer()->m_BalanceID2))
				{
					GameServer()->GetPlayerChar(GameServer()->m_BalanceID2)->Die(GameServer()->m_BalanceID2, WEAPON_SELF);
				}
				//dbg_msg("balance", "%s:%d lost and %s:%d got killed too", Server()->ClientName(GameServer()->m_BalanceID1), GameServer()->m_BalanceID1, Server()->ClientName(GameServer()->m_BalanceID2), GameServer()->m_BalanceID2);
				GameServer()->StopBalanceBattle();
			}
		}
		else if (GameServer()->m_BalanceID2 == m_pPlayer->GetCID())
		{
			if (GameServer()->m_apPlayers[GameServer()->m_BalanceID1])
			{
				GameServer()->SendChatTarget(GameServer()->m_BalanceID1, "[balance] you won!");
				GameServer()->SendChatTarget(GameServer()->m_BalanceID2, "[balance] you lost!");
				GameServer()->m_apPlayers[GameServer()->m_BalanceID1]->m_IsBalanceBatteling = false;
				GameServer()->m_apPlayers[GameServer()->m_BalanceID2]->m_IsBalanceBatteling = false;
				GameServer()->m_BalanceBattleState = 0;
				if (GameServer()->GetPlayerChar(GameServer()->m_BalanceID1))
				{
					GameServer()->GetPlayerChar(GameServer()->m_BalanceID1)->Die(GameServer()->m_BalanceID1, WEAPON_SELF);
				}
				//dbg_msg("balance", "%s:%d lost and %s:%d got killed too", Server()->ClientName(GameServer()->m_BalanceID2), GameServer()->m_BalanceID2, Server()->ClientName(GameServer()->m_BalanceID1), GameServer()->m_BalanceID1);
				GameServer()->StopBalanceBattle();
			}
		}
	}

	// TODO: refactor this code and put it in own function
	// ChillerDragon pvparena code
	if (GameServer()->m_apPlayers[Killer])
	{
		if (GameServer()->GetPlayerChar(Killer) && Weapon != WEAPON_GAME && Weapon != WEAPON_SELF)
		{
			//GameServer()->GetPlayerChar(Killer)->m_Bloody = true;

			if (GameServer()->GetPlayerChar(Killer)->m_IsPVParena)
			{
				if (GameServer()->m_apPlayers[Killer]->IsMaxLevel() ||
					GameServer()->IsSameIP(Killer, m_pPlayer->GetCID()) || // dont give xp on dummy kill
					GameServer()->IsSameIP(m_pPlayer->GetCID(), GameServer()->m_apPlayers[Killer]->m_pvp_arena_last_kill_id) // dont give xp on killing same ip twice in a row
					)
				{
					GameServer()->m_apPlayers[Killer]->MoneyTransaction(+150, "pvp_arena kill");
					GameServer()->m_apPlayers[Killer]->m_pvp_arena_kills++;

					str_format(aBuf, sizeof(aBuf), "[PVP] +150 money for killing %s", Server()->ClientName(m_pPlayer->GetCID()));
					GameServer()->SendChatTarget(Killer, aBuf);
				}
				else
				{
					GameServer()->m_apPlayers[Killer]->MoneyTransaction(+150, "pvp_arena kill");
					GameServer()->m_apPlayers[Killer]->GiveXP(100);
					GameServer()->m_apPlayers[Killer]->m_pvp_arena_kills++;

					str_format(aBuf, sizeof(aBuf), "[PVP] +100 xp +150 money for killing %s", Server()->ClientName(m_pPlayer->GetCID()));
					GameServer()->SendChatTarget(Killer, aBuf);
				}

				int r = rand() % 100;
				if (r > 92)
				{
					GameServer()->m_apPlayers[Killer]->m_pvp_arena_tickets++;
					GameServer()->SendChatTarget(Killer, "[PVP] +1 pvp_arena_ticket        (special random drop for kill)");
				}
				GameServer()->m_apPlayers[Killer]->m_pvp_arena_last_kill_id = m_pPlayer->GetCID();
			}
		}
	}
	if (m_pPlayer) //victim
	{
		//m_pPlayer->m_InfRainbow = true;
		if (m_IsPVParena)
		{
			m_pPlayer->m_pvp_arena_deaths++;

			str_format(aBuf, sizeof(aBuf), "[PVP] You lost the arena-fight because you were killed by %s.", Server()->ClientName(Killer));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
		}
	}

	//bomb
	if (m_IsBombing)
	{
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[BOMB] you lost bomb because you died.");
	}

	m_pPlayer->UpdateLastToucher(-1);
	return Killer;
}

void CCharacter::BlockTourna_Die(int Killer)
{
	char aBuf[128];

	//Block tourna
	if (GameServer()->m_BlockTournaState == 2) //ingame
	{
		if (m_pPlayer->m_IsBlockTourning)
		{
			//update skill levels
			if (m_pPlayer->GetCID() == Killer) //selfkill
			{
				GameServer()->UpdateBlockSkill(-40, Killer);
			}
			else
			{
				int deadskill = m_pPlayer->m_BlockSkill;
				int killskill = GameServer()->m_apPlayers[Killer]->m_BlockSkill;
				int skilldiff = abs(deadskill - killskill);
				if (skilldiff < 1500) //pretty same skill lvl
				{
					if (deadskill < killskill) //the killer is better
					{
						GameServer()->UpdateBlockSkill(-29, m_pPlayer->GetCID()); //killed
						GameServer()->UpdateBlockSkill(+30, Killer); //killer
					}
					else //the killer is worse
					{
						GameServer()->UpdateBlockSkill(-40, m_pPlayer->GetCID()); //killed
						GameServer()->UpdateBlockSkill(+40, Killer); //killer
					}
				}
				else //unbalanced skill lvl --> punish harder and reward nicer
				{
					if (deadskill < killskill) //the killer is better
					{
						GameServer()->UpdateBlockSkill(-19, m_pPlayer->GetCID()); //killed
						GameServer()->UpdateBlockSkill(+20, Killer); //killer
					}
					else //the killer is worse
					{
						GameServer()->UpdateBlockSkill(-60, m_pPlayer->GetCID()); //killed
						GameServer()->UpdateBlockSkill(+60, Killer); //killer
					}
				}
			}

			//let him die and check for tourna win
			m_pPlayer->m_IsBlockTourning = false;
			int wonID = GameServer()->CountBlockTournaAlive();

			if (wonID == -404)
			{
				str_format(aBuf, sizeof(aBuf), "[BLOCK] error %d", wonID);
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				GameServer()->m_BlockTournaState = 0;
			}
			else if (wonID < 0 || wonID == -420)
			{
				if (wonID == -420)
				{
					wonID = 0;
				}
				wonID *= -1;
				str_format(aBuf, sizeof(aBuf), "[BLOCK] '%s' won the tournament (%d players).", Server()->ClientName(wonID), GameServer()->m_BlockTournaStartPlayers);
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				GameServer()->m_BlockTournaState = 3; //set end state


													  //give price to the winner
				int xp_rew;
				int points_rew;
				int money_rew;
				int skill_rew;
				if (GameServer()->m_BlockTournaStartPlayers <= 5) //depending on how many tees participated
				{
					xp_rew = 100;
					points_rew = 3;
					money_rew = 50;
					skill_rew = 10;
				}
				else if (GameServer()->m_BlockTournaStartPlayers <= 10)
				{
					xp_rew = 150;
					points_rew = 5;
					money_rew = 100;
					skill_rew = 20;
				}
				else if (GameServer()->m_BlockTournaStartPlayers <= 15)
				{
					xp_rew = 300;
					points_rew = 10;
					money_rew = 200;
					skill_rew = 30;
				}
				else if (GameServer()->m_BlockTournaStartPlayers <= 32)
				{
					xp_rew = 700;
					points_rew = 25;
					money_rew = 500;
					skill_rew = 120;
				}
				else if (GameServer()->m_BlockTournaStartPlayers <= 44)
				{
					xp_rew = 1200;
					points_rew = 30;
					money_rew = 1000;
					skill_rew = 400;
				}
				else
				{
					xp_rew = 25000;
					points_rew = 100;
					money_rew = 15000;
					skill_rew = 900;
				}

				str_format(aBuf, sizeof(aBuf), "[BLOCK] +%d xp", xp_rew);
				GameServer()->SendChatTarget(wonID, aBuf);
				str_format(aBuf, sizeof(aBuf), "[BLOCK] +%d money", money_rew);
				GameServer()->SendChatTarget(wonID, aBuf);
				str_format(aBuf, sizeof(aBuf), "[BLOCK] +%d points", points_rew);
				GameServer()->SendChatTarget(wonID, aBuf);

				GameServer()->m_apPlayers[wonID]->MoneyTransaction(+money_rew, "block tournament");
				GameServer()->m_apPlayers[wonID]->GiveXP(xp_rew);
				GameServer()->m_apPlayers[wonID]->GiveBlockPoints(points_rew);
				GameServer()->UpdateBlockSkill(+skill_rew, wonID);
			}
			else if (wonID == 0)
			{
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, "[BLOCK] nobody won the tournament");
				GameServer()->m_BlockTournaState = 0;
			}
			else if (wonID > 1)
			{
				str_format(aBuf, sizeof(aBuf), "[BLOCK] you died and placed as rank %d in the tournament", wonID + 1);
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			}
			else
			{
				str_format(aBuf, sizeof(aBuf), "[BLOCK] error %d", wonID);
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				GameServer()->m_BlockTournaState = 0;
			}
		}
	}
}

void CCharacter::MoveTee(int x, int y)
{
	m_Core.m_Pos.x += x;
	m_Core.m_Pos.y += y;
}

void CCharacter::ChillTelePort(float X, float Y)
{
	m_Core.m_Pos.x = X;
	m_Core.m_Pos.y = Y;
}

void CCharacter::ChillTelePortTile(int X, int Y)
{
	m_Core.m_Pos.x = X * 32;
	m_Core.m_Pos.y = Y * 32;
}

void CCharacter::FreezeAll(int seconds)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetCharacter())
		{
			GameServer()->m_apPlayers[i]->GetCharacter()->Freeze(seconds);
		}
	}
}

bool CCharacter::HasWeapon(int weapon)
{
	if (m_aWeapons[weapon].m_Got)
	{
		return true;
	}
	return false;
}

void CCharacter::KillSpeed()
{
	m_Core.m_Vel.x = 0.0f;
	m_Core.m_Vel.y = 0.0f;
}

void CCharacter::InstagibKillingSpree(int KillerID, int Weapon)
{
	char aBuf[128];

	//killingspree system by FruchtiHD and ChillerDragon stolen from twlevel (edited by ChillerDragon)
	CCharacter *pVictim = m_pPlayer->GetCharacter();
	CPlayer *pKiller = GameServer()->m_apPlayers[KillerID];
	if (GameServer()->CountConnectedPlayers() >= g_Config.m_SvSpreePlayers) //only count killing sprees if enough players are online (also counting spectators)
	{
		if (pVictim && pKiller)
		{

			if (Weapon == WEAPON_GAME)
			{
				if (pVictim->GetPlayer()->m_KillStreak >= 5)
				{
					//Check for new highscore
					if (g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2) //gdm & zCatch grenade
					{
						//dbg_msg("insta", "checking for highscore grenade");
						if (pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_GrenadeSpree)
						{
							pVictim->GetPlayer()->m_GrenadeSpree = pVictim->GetPlayer()->m_KillStreak;
							GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCID(), "New grenade Killingspree record!");
						}
						//str_format(aBuf, sizeof(aBuf), "last: %d top: %d", pVictim->GetPlayer()->m_KillStreak, pVictim->GetPlayer()->m_GrenadeSpree);
						//dbg_msg("insta", aBuf);
					}
					else if (g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4) // idm & zCatch rifle
					{
						//dbg_msg("insta", "checking for highscore rifle");
						if (pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_RifleSpree)
						{
							pVictim->GetPlayer()->m_RifleSpree = pVictim->GetPlayer()->m_KillStreak;
							GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCID(), "New rifle Killingspree record!");
						}
						//str_format(aBuf, sizeof(aBuf), "last: %d top: %d", pVictim->GetPlayer()->m_KillStreak, pVictim->GetPlayer()->m_GrenadeSpree);
						//dbg_msg("insta", aBuf);
					}

					str_format(aBuf, sizeof(aBuf), "%s's killingspree was ended by %s (%d Kills)", Server()->ClientName(pVictim->GetPlayer()->GetCID()), Server()->ClientName(pVictim->GetPlayer()->GetCID()), pVictim->GetPlayer()->m_KillStreak);
					pVictim->GetPlayer()->m_KillStreak = 0;
					GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
					GameServer()->CreateExplosion(pVictim->m_Pos, m_pPlayer->GetCID(), WEAPON_GRENADE, true, 0, m_pPlayer->GetCharacter()->Teams()->TeamMask(0));
				}
			}

			if (pVictim->GetPlayer()->m_KillStreak >= 5)
			{
				//Check for new highscore
				if (g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2) //gdm & zCatch grenade
				{
					//dbg_msg("insta", "checking for highscore grenade");
					if (pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_GrenadeSpree)
					{
						pVictim->GetPlayer()->m_GrenadeSpree = pVictim->GetPlayer()->m_KillStreak;
						GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCID(), "New grenade Killingspree record!");
					}
					//str_format(aBuf, sizeof(aBuf), "last: %d top: %d", pVictim->GetPlayer()->m_KillStreak, pVictim->GetPlayer()->m_GrenadeSpree);
					//dbg_msg("insta", aBuf);
				}
				else if (g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4) // idm & zCatch rifle
				{
					//dbg_msg("insta", "checking for highscore rifle");
					if (pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_RifleSpree)
					{
						pVictim->GetPlayer()->m_RifleSpree = pVictim->GetPlayer()->m_KillStreak;
						GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCID(), "New rifle Killingspree record!");
					}
					//str_format(aBuf, sizeof(aBuf), "last: %d top: %d", pVictim->GetPlayer()->m_KillStreak, pVictim->GetPlayer()->m_GrenadeSpree);
					//dbg_msg("insta", aBuf);
				}

				str_format(aBuf, sizeof(aBuf), "'%s's killingspree was ended by %s (%d Kills)", Server()->ClientName(pVictim->GetPlayer()->GetCID()), Server()->ClientName(pKiller->GetCID()), pVictim->GetPlayer()->m_KillStreak);
				pVictim->GetPlayer()->m_KillStreak = 0;
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				GameServer()->CreateExplosion(pVictim->m_Pos, m_pPlayer->GetCID(), WEAPON_GRENADE, true, 0, m_pPlayer->GetCharacter()->Teams()->TeamMask(0));
			}

			if (pKiller != pVictim->GetPlayer())
			{
				if (!pVictim->GetPlayer()->m_IsDummy || pKiller->m_IsDummy)
				{
					pKiller->m_KillStreak++;
				}
				pVictim->GetPlayer()->m_KillStreak = 0;
				str_format(aBuf, sizeof(aBuf), "'%s' is on a killing spree with %d Kills!", Server()->ClientName(pKiller->GetCID()), pKiller->m_KillStreak);

				if (pKiller->m_KillStreak % 5 == 0 && pKiller->m_KillStreak >= 5)
					GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);

				//Finish time if cfg val reached
				if (pKiller->m_KillStreak == g_Config.m_SvKillsToFinish && g_Config.m_SvInstagibMode) //only finish if sv_insta is on... needed for the future if we actiavte this killsys in ddrace mode (sv_insta 0) to dont fuck up race scores
				{
					CGameControllerDDRace* Controller = (CGameControllerDDRace*)GameServer()->m_pController;
					Controller->m_Teams.OnCharacterFinish(pKiller->GetCID());
				}
			}
		}
	}
	else //not enough players
	{
		//str_format(aBuf, sizeof(aBuf), "not enough tees %d/%d spree (%d)", GameServer()->CountConnectedPlayers(), g_Config.m_SvSpreePlayers, pKiller->m_KillStreak);
		//dbg_msg("insta", aBuf);
		if (pKiller != pVictim->GetPlayer())
		{
			if (!pVictim->GetPlayer()->m_IsDummy || pKiller->m_IsDummy)
			{
				pKiller->m_KillStreak++;
			}

			pVictim->GetPlayer()->m_KillStreak = 0;
			if (pKiller->m_KillStreak == 5)
			{
				str_format(aBuf, sizeof(aBuf), "[SPREE] %d players needed to start a spree.", g_Config.m_SvSpreePlayers);
				GameServer()->SendChatTarget(pKiller->GetCID(), aBuf);
				pKiller->m_KillStreak = 0; //reset killstreak to avoid some1 collecting 100 kills with dummy and then if player connect he could save the spree
			}
		}
	}
	pVictim->GetPlayer()->m_KillStreak = 0; //Important always clear killingspree of ded dude
}

int CCharacter::BlockPointsMain(int Killer, bool fngscore)
{
	if (m_FreezeTime <= 0)
		return Killer;
	if (m_pPlayer->m_LastToucherID == -1)
		return Killer;
	if (m_pPlayer->m_IsInstaMode_fng && !fngscore)
		return Killer; // Killer = KilledID --> gets count as selfkill in score sys and not counted as kill (because only fng score tiles score)

	if (m_pPlayer->m_LastToucherID == m_pPlayer->GetCID())
	{
		dbg_msg("block", "WARNING '%s' [ID: %d] blocked himself", Server()->ClientName(m_pPlayer->GetCID()), m_pPlayer->GetCID());
		return Killer;
	}

	char aBuf[128];
	Killer = m_pPlayer->m_LastToucherID; // kill message

	if (g_Config.m_SvBlockBroadcast == 1)  // send kill message broadcast
	{
		str_format(aBuf, sizeof(aBuf), "'%s' was blocked by '%s'", Server()->ClientName(m_pPlayer->GetCID()), Server()->ClientName(Killer));
		GameServer()->SendBroadcastAll(aBuf, 0);
	}

	BlockQuestSubDieFuncBlockKill(Killer);

	// track deaths of blocked
	if (!m_pPlayer->m_IsBlockWaving) // dont count block deaths in blockwave minigame
	{
		if (m_pPlayer->m_IsInstaArena_gdm)
		{
			//m_pPlayer->m_GrenadeDeaths++; // probably doesn't belong into blockmain but whatever //ye rly doesnt --> moved
		}
		else if (m_pPlayer->m_IsInstaArena_idm)
		{
			//m_pPlayer->m_RifleDeaths++; // probably doesn't belong into blockmain but whatever //ye rly doesnt --> moved
		}
		else
		{
			if (m_pPlayer->m_IsDummy)
			{
				if (g_Config.m_SvDummyBlockPoints)
				{
					m_pPlayer->m_BlockPoints_Deaths++;
				}
			}
			else
			{
				m_pPlayer->m_BlockPoints_Deaths++;
			}
		}
	}

	if (GameServer()->m_apPlayers[Killer])
	{
		// give kills and points to blocker
		if (!m_pPlayer->m_IsBlockWaving) // dont count block kills and points in blockwave minigame (would be too op lol)
		{
			if (m_pPlayer->m_IsDummy) // if dummy got killed make some exceptions
			{
				if (g_Config.m_SvDummyBlockPoints == 2 || (g_Config.m_SvDummyBlockPoints == 3 && GameServer()->IsPosition(Killer, 2))) //only count dummy kills if configt       cfg:3 block area or further count kills
				{
					if (Server()->Tick() >= m_AliveTime + Server()->TickSpeed() * g_Config.m_SvPointsFarmProtection)
					{
						GameServer()->m_apPlayers[Killer]->GiveBlockPoints(1);
					}
					GameServer()->m_apPlayers[Killer]->m_BlockPoints_Kills++;
				}
			}
			else
			{
				if (Server()->Tick() >= m_AliveTime + Server()->TickSpeed() * g_Config.m_SvPointsFarmProtection)
				{
					GameServer()->m_apPlayers[Killer]->GiveBlockPoints(1);
				}
				GameServer()->m_apPlayers[Killer]->m_BlockPoints_Kills++;
			}
		}
		// give xp reward to the blocker
		if (m_pPlayer->m_KillStreak > 4 && m_pPlayer->IsMaxLevel())
		{
			if (!GameServer()->m_apPlayers[Killer]->m_HideBlockXp)
			{
				str_format(aBuf, sizeof(aBuf), "+%d xp for blocking '%s'", m_pPlayer->m_KillStreak, Server()->ClientName(m_pPlayer->GetCID()));
				GameServer()->SendChatTarget(Killer, aBuf);
			}
			GameServer()->m_apPlayers[Killer]->GiveXP( m_pPlayer->m_KillStreak);
		}
		// bounty money reward to the blocker
		if (m_pPlayer->m_BlockBounty)
		{
			str_format(aBuf, sizeof(aBuf), "[BOUNTY] +%d money for blocking '%s'", m_pPlayer->m_BlockBounty, Server()->ClientName(m_pPlayer->GetCID()));
			GameServer()->SendChatTarget(Killer, aBuf);
			str_format(aBuf, sizeof(aBuf), "bounty '%s'", m_pPlayer->m_BlockBounty, Server()->ClientName(m_pPlayer->GetCID()));
			GameServer()->m_apPlayers[Killer]->MoneyTransaction(+m_pPlayer->m_BlockBounty, aBuf);
			m_pPlayer->m_BlockBounty = 0;
		}
	}
	return Killer;
}

void CCharacter::BlockSpawnProt(int Killer)
{
	char aBuf[128];
	if (GameServer()->m_apPlayers[Killer] && GameServer()->m_apPlayers[Killer]->GetCharacter() && m_pPlayer->GetCID() != Killer)
	{
		//punish spawn blockers
		if (GameServer()->IsPosition(Killer, 3)) //if killer is in spawn area
		{
			GameServer()->m_apPlayers[Killer]->m_SpawnBlocks++;
			if (g_Config.m_SvSpawnBlockProtection == 1 || g_Config.m_SvSpawnBlockProtection == 2)
			{
				GameServer()->SendChatTarget(Killer, "[WARNING] spawnblocking is illegal.");
				//str_format(aBuf, sizeof(aBuf), "[debug] spawnblocks: %d", GameServer()->m_apPlayers[Killer]->m_SpawnBlocks);
				//GameServer()->SendChatTarget(Killer, aBuf);

				if (GameServer()->m_apPlayers[Killer]->m_SpawnBlocks > 2)
				{
					str_format(aBuf, sizeof(aBuf), "'%s' is spawnblocking. catch him!", Server()->ClientName(Killer));
					GameServer()->SendAllPolice(aBuf);
					GameServer()->SendChatTarget(Killer, "Police is searching you because of spawnblocking.");
					GameServer()->m_apPlayers[Killer]->m_EscapeTime += Server()->TickSpeed() * 120; // + 2 minutes escape time
					GameServer()->AddEscapeReason(Killer, "spawnblock");
				}
			}
		}
	}
}

void CCharacter::BlockQuestSubDieFuncBlockKill(int Killer)
{
	if (!GameServer()->m_apPlayers[Killer])
		return;

	char aBuf[128];
	//QUEST
	if (GameServer()->m_apPlayers[Killer]->m_QuestState == CPlayer::QUEST_HAMMER)
	{
		if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 7)
		{
			if (GameServer()->m_apPlayers[Killer]->m_QuestProgressValue < 10)
			{
				//GameServer()->SendChatTarget(Killer, "[QUEST] hammer the tee 10 times before blocking him.");
			}
			else
			{
				GameServer()->QuestAddProgress(Killer, 11);
			}
		}
	}
	else if (GameServer()->m_apPlayers[Killer]->m_QuestState == CPlayer::QUEST_BLOCK)
	{
		if (GameServer()->IsSameIP(Killer, m_pPlayer->GetCID()))
		{
			if (!m_pPlayer->m_HideQuestWarning)
			{
				GameServer()->SendChatTarget(Killer, "[QUEST] your dummy doesn't count.");
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[QUEST] your dummy doesn't count."); //send it both so that he recives the message. i know this can be weird on lanpartys but fuck it xd
			}
		}
		else
		{
			if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 0)
			{
				GameServer()->QuestCompleted(Killer);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 1)
			{
				GameServer()->QuestAddProgress(Killer, 2);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 2)
			{
				GameServer()->QuestAddProgress(Killer, 3);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 3)
			{
				GameServer()->QuestAddProgress(Killer, 5);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 4)
			{
				GameServer()->QuestAddProgress(Killer, 10);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 5)
			{
				if (GameServer()->m_apPlayers[Killer]->m_QuestProgressValue < 5)
				{
					GameServer()->QuestAddProgress(Killer, 6, 5);
				}
				else
				{
					if (m_pPlayer->GetCID() != GameServer()->m_apPlayers[Killer]->m_QuestPlayerID)
					{
						str_format(aBuf, sizeof(aBuf), "[QUEST] You have to block '%s' to complete the quest.", Server()->ClientName(GameServer()->m_apPlayers[Killer]->m_QuestPlayerID));
						if (!m_pPlayer->m_HideQuestWarning)
						{
							GameServer()->SendChatTarget(Killer, aBuf);
						}
					}
					else
					{
						GameServer()->QuestAddProgress(Killer, 6);
					}
				}
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 6)
			{
				if (m_pPlayer->m_KillStreak < 5)
				{
					str_format(aBuf, sizeof(aBuf), "[QUEST] '%s' is only on a %d tee blockingspree", Server()->ClientName(m_pPlayer->GetCID()), m_pPlayer->m_KillStreak);
					if (!m_pPlayer->m_HideQuestWarning)
					{
						GameServer()->SendChatTarget(Killer, aBuf);
					}
				}
				else
				{
					GameServer()->QuestCompleted(Killer);
				}
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 7)
			{
				GameServer()->QuestAddProgress(Killer, 11);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 8)
			{
				GameServer()->QuestAddProgress(Killer, 3);
			}
			else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 9) //TODO: TEST THIS QUEST (should be working now)
			{
				//success (blocking player)
				if (((CGameControllerDDRace*)GameServer()->m_pController)->HasFlag(GameServer()->m_apPlayers[Killer]->GetCharacter()) != -1)
				{
					GameServer()->QuestAddProgress(Killer, 11);
				}
				else
				{
					if (!m_pPlayer->m_HideQuestWarning)
					{
						GameServer()->SendChatTarget(Killer, "[QUEST] You need the flag.");
					}
				}
			}
		}
	}
	else if (GameServer()->m_apPlayers[Killer]->m_QuestState == CPlayer::QUEST_RIFLE)
	{
		if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 7) // Rifle <specific player> and then block him [LEVEL 7]
		{
			if (GameServer()->m_apPlayers[Killer]->m_QuestPlayerID == m_pPlayer->GetCID())
			{
				if (GameServer()->m_apPlayers[Killer]->m_QuestProgressValue)
				{
					GameServer()->QuestAddProgress(Killer, 2);
				}
			}
			else
			{
				// GameServer()->SendChatTarget(Killer, "[QUEST] wrong tee");
			}
		}
		else if (GameServer()->m_apPlayers[Killer]->m_QuestStateLevel == 8) // Rifle 5 tees before blocking them [LEVEL 8]
		{
			if (GameServer()->m_apPlayers[Killer]->m_QuestProgressBool)
			{
				if (GameServer()->m_apPlayers[Killer]->m_QuestLastQuestedPlayerID == m_pPlayer->GetCID())
				{
					GameServer()->QuestAddProgress(Killer, 5);
					GameServer()->m_apPlayers[Killer]->m_QuestProgressBool = false;
					GameServer()->m_apPlayers[Killer]->m_QuestLastQuestedPlayerID = -1;
				}
				else
				{
					if (!m_pPlayer->m_HideQuestWarning)
					{
						GameServer()->SendChatTarget(Killer, "[QUEST] wrong tee");
					}
				}
			}
		}
	}
}

void CCharacter::BlockQuestSubDieFuncDeath(int Killer)
{
	if (Killer != m_pPlayer->GetCID() && m_pPlayer->m_QuestState == CPlayer::QUEST_BLOCK && m_pPlayer->m_QuestStateLevel == 7 && m_pPlayer->m_QuestProgressValue > 0)
	{
		GameServer()->QuestFailed(m_pPlayer->GetCID());
	}
	if (m_pPlayer->m_QuestStateLevel == 9 && m_pPlayer->m_QuestState == CPlayer::QUEST_HAMMER)
	{
		GameServer()->QuestFailed(m_pPlayer->GetCID());
	}
}

void CCharacter::KillingSpree(int Killer) // handles all ddnet++ gametype sprees (not other server types as fng or instagib only servers)
{
	char aBuf[128];
	// Somehow inspiration by //toast killingspree
	// system by FruchtiHD and ChillerDragon stolen from twlevel (edited by ChillerDragon)
	// stolen from DDnet++ instagib and edited agian by ChillerDragon
	// rewritten by ChillerDragon cuz tw bug
	// upgraded to handle instagib agian
	// rewritten by ChillerDragon in 2019 cuz old system was fucked in the head

	CPlayer *pKiller = GameServer()->m_apPlayers[Killer]; //removed pointer alien code and used the long way to have less bugsis with left players

	// dont count selfkills only count real being blocked as dead
	if (m_pPlayer->GetCID() == Killer)
	{
		//dbg_msg("SPREE", "didnt count selfkill [%d][%s]", Killer, Server()->ClientName(Killer));
		return;	
	}

	char aKillerName[32];
	char aSpreeType[16];

	if (GameServer()->m_apPlayers[Killer])
		str_format(aKillerName, sizeof(aKillerName), "'%s'", Server()->ClientName(Killer));
	else
		str_format(aKillerName, sizeof(aKillerName), "'%s'", m_pPlayer->m_aLastToucherName);
		// str_copy(aKillerName, "a player who left the game", sizeof(aKillerName));

	if (m_pPlayer->m_KillStreak >= 5)
	{
		GameServer()->GetSpreeType(m_pPlayer->GetCID(), aSpreeType, sizeof(aSpreeType), true);
		str_format(aBuf, sizeof(aBuf), "'%s's %s spree was ended by %s (%d Kills)", Server()->ClientName(m_pPlayer->GetCID()), aSpreeType, aKillerName, m_pPlayer->m_KillStreak);
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
		GameServer()->CreateExplosion(m_Pos, m_pPlayer->GetCID(), WEAPON_GRENADE, true, 0, m_pPlayer->GetCharacter()->Teams()->TeamMask(0));
	}

	if (pKiller)
	{
		//#################
		// KILLER (blocker)
		//#################
		if ((m_pPlayer->m_IsDummy && g_Config.m_SvSpreeCountBots) ||  //only count bots if configurated
			(!m_pPlayer->m_IsDummy)) //count all humans in killingsprees
		{
			GameServer()->m_apPlayers[Killer]->m_KillStreak++;
		}
		// only count killing sprees if enough players are online and ingame (alive)
		if (GameServer()->CountIngameHumans() < g_Config.m_SvSpreePlayers)
		{
			//dbg_msg("spree", "not enough tees %d/%d spree (%d)", GameServer()->CountConnectedPlayers(), g_Config.m_SvSpreePlayers, GameServer()->m_apPlayers[Killer]->m_KillStreak);
			if (GameServer()->m_apPlayers[Killer]->m_KillStreak == 5) // TODO: what if one has 6 kills and then all players leave then he can farm dummys?
			{
				str_format(aBuf, sizeof(aBuf), "[SPREE] %d/%d humans alive to start a spree.", GameServer()->CountIngameHumans(), g_Config.m_SvSpreePlayers);
				GameServer()->SendChatTarget(Killer, aBuf);
				GameServer()->m_apPlayers[Killer]->m_KillStreak = 0; // reset killstreak to avoid some1 collecting 100 kills with dummy and then if player connect he could save the spree
			}
		}
		else // enough players
		{
			if (GameServer()->m_apPlayers[Killer]->m_KillStreak % 5 == 0 && GameServer()->m_apPlayers[Killer]->m_KillStreak >= 5)
			{
				GameServer()->GetSpreeType(Killer, aSpreeType, sizeof(aSpreeType), false);
				str_format(aBuf, sizeof(aBuf), "%s is on a %s spree with %d kills!", aKillerName, aSpreeType, pKiller->m_KillStreak);
				GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
			}
		}
	}
	m_pPlayer->m_KillStreak = 0; //Important always clear killingspree of ded dude
}

void CCharacter::CITick()
{
	//Check for stuck --> restart
	if (isFreezed)
	{
		m_ci_freezetime++;
	}
	else
	{
		m_ci_freezetime = 0;
	}
	if (m_ci_freezetime > g_Config.m_SvCIfreezetime * Server()->TickSpeed())
	{
		Die(m_pPlayer->GetCID(), WEAPON_SELF); //call CIRestart() there
	}
}

void CCharacter::CIRestart()
{
	char aBuf[128];

	//str_format(aBuf, sizeof(aBuf), "%x", GameServer()->Score()->LoadCIData()); //linux compile error (doesnt work on win anyways)
	//if (!str_comp(aBuf, "error"))
	//{
	//	dbg_msg("CI", "error loading data...");
	//}
	//else
	//{
	//	dbg_msg("CI", "loaded DIST [%x]", GameServer()->Score()->LoadCIData());
	//}

	m_pPlayer->m_ci_latest_dest_dist = CIGetDestDist();
	str_format(aBuf, sizeof(aBuf), "Dist: %d", m_pPlayer->m_ci_latest_dest_dist);
	dbg_msg("CI", aBuf);

	if (m_pPlayer->m_ci_latest_dest_dist < m_pPlayer->m_ci_lowest_dest_dist)
	{
		str_format(aBuf, sizeof(aBuf), "NEW [%d] OLD [%d]", m_pPlayer->m_ci_latest_dest_dist, m_pPlayer->m_ci_lowest_dest_dist);
		dbg_msg("CI", aBuf);
		m_pPlayer->m_ci_lowest_dest_dist = m_pPlayer->m_ci_latest_dest_dist;
	}

	str_format(aBuf, sizeof(aBuf), "%d", m_pPlayer->m_ci_lowest_dest_dist);

	GameServer()->Score()->SaveCIData(aBuf);
}

int CCharacter::CIGetDestDist()
{
	//pythagoras mate u rock c:
	//a²+b²=c²
	int a = m_Core.m_Pos.x - g_Config.m_SvCIdestX;
	int b = m_Core.m_Pos.y - g_Config.m_SvCIdestY;
	//|a| |b|
	a = abs(a);
	b = abs(b);

	int c = sqrt((double)(a + b));

	return c;
}

void CCharacter::SurvivalSubDieFunc(int Killer, int weapon)
{
	bool selfkill = Killer == m_pPlayer->GetCID();
	if (m_pPlayer->m_IsSurvivalAlive && GameServer()->m_apPlayers[Killer]->m_IsSurvivalAlive) //ignore lobby and stuff
	{
		//=== DEATHS and WINCHECK ===
		if (m_pPlayer->m_IsSurvivaling)
		{
			if (GameServer()->m_survivalgamestate > 1) //if game running
			{
				GameServer()->SetPlayerSurvival(m_pPlayer->GetCID(), GameServer()->SURVIVAL_DIE);
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), "[SURVIVAL] you lost the round.");
				GameServer()->SurvivalCheckWinnerAndDeathMatch();
				GameServer()->SurvivalGetNextSpectator(m_pPlayer->GetCID(), Killer);
				GameServer()->SurvivalUpdateSpectators(m_pPlayer->GetCID(), Killer);
			}
		}

		//=== KILLS ===
		if (!selfkill)
		{
			if (GameServer()->m_apPlayers[Killer] && GameServer()->m_apPlayers[Killer]->m_IsSurvivaling)
			{
				GameServer()->m_apPlayers[Killer]->m_SurvivalKills++;
			}
		}
	}
}

void CCharacter::InstagibSubDieFunc(int Killer, int Weapon)
{
	//=== DEATHS ===
	if (g_Config.m_SvInstagibMode)
	{
		if (g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2) //gdm & zCatch grenade
		{
			m_pPlayer->m_GrenadeDeaths++;
		}
		else if (g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4) // idm & zCatch rifle
		{
			m_pPlayer->m_RifleDeaths++;
		}

		InstagibKillingSpree(Killer, Weapon);
	}
	else if (m_pPlayer->m_IsInstaArena_gdm)
	{
		m_pPlayer->m_GrenadeDeaths++;
	}
	else if (m_pPlayer->m_IsInstaArena_idm)
	{
		m_pPlayer->m_RifleDeaths++;
	}

	//=== KILLS ===
	if (GameServer()->m_apPlayers[Killer])
	{
		if (g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2 || GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm) //gdm & zCatch grenade
		{
			GameServer()->m_apPlayers[Killer]->m_GrenadeKills++;
		}
		else if (g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4 || GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm) // idm & zCatch rifle
		{
			GameServer()->m_apPlayers[Killer]->m_RifleKills++;
		}

		//=== MULTIS (credit to noby) ===
		if (Killer != m_pPlayer->GetCID()) // don't count selkills as multi
		{
			time_t ttmp = time(NULL);
			if ((ttmp - GameServer()->m_apPlayers[Killer]->m_lastkilltime) <= 5)
			{
				GameServer()->m_apPlayers[Killer]->m_multi++;
				if (GameServer()->m_apPlayers[Killer]->m_max_multi < GameServer()->m_apPlayers[Killer]->m_multi)
				{
					GameServer()->m_apPlayers[Killer]->m_max_multi = GameServer()->m_apPlayers[Killer]->m_multi;
				}
				char aBuf[128];
				if (GameServer()->IsDDPPgametype("fng"))
				{
					str_format(aBuf, sizeof(aBuf), "'%s' multi x%d!", Server()->ClientName(Killer), GameServer()->m_apPlayers[Killer]->m_multi);
					GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf);
				}
				if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_fng)
				{
					if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm) // grenade
					{
						str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' multi x%d!", Server()->ClientName(Killer), GameServer()->m_apPlayers[Killer]->m_multi);
						GameServer()->SayInsta(aBuf, 4);
					}
					else if (GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm) // rifle
					{
						str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' multi x%d!", Server()->ClientName(Killer), GameServer()->m_apPlayers[Killer]->m_multi);
						GameServer()->SayInsta(aBuf, 5);
					}
				}
			}
			else
			{
				GameServer()->m_apPlayers[Killer]->m_multi = 1;
			}
			GameServer()->m_apPlayers[Killer]->m_lastkilltime = ttmp;
		}
	}
}

bool CCharacter::IsHammerBlocked()
{
    //hammer delay on super jail hammer
    if (m_pPlayer->m_JailHammer > 1 && m_pPlayer->m_JailHammerDelay)
    {
        char aBuf[128];
        str_format(aBuf, sizeof(aBuf), "You have to wait %d minutes to use your super jail hammer agian.", (m_pPlayer->m_JailHammerDelay / Server()->TickSpeed()) / 60);
        GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
        return true;
    }
    return false;
}

void CCharacter::DDPPHammerHit(CCharacter *pTarget)
{
    /*pTarget->TakeDamage(vec2(0.f, -1.f) + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f, g_pData->m_Weapons.m_Hammer.m_pBase->m_Damage,
    m_pPlayer->GetCID(), m_Core.m_ActiveWeapon);*/

    // shop bot
    if (pTarget->m_pPlayer->m_IsDummy)
    {
        if (pTarget->m_pPlayer->m_DummyMode == 99)
        {
            StartShop();
        }
    }

    //Bomb (put it dat early cuz the unfreeze stuff)
    if (m_IsBombing && pTarget->m_IsBombing)
    {
        if (m_IsBomb) //if bomb hits others --> they get bomb
        {
            if (!pTarget->isFreezed && !pTarget->m_FreezeTime) //you cant bomb freezed players
            {
                m_IsBomb = false;
                pTarget->m_IsBomb = true;

                char aBuf[128];
                str_format(aBuf, sizeof(aBuf), "%s bombed %s", Server()->ClientName(m_pPlayer->GetCID()), Server()->ClientName(pTarget->GetPlayer()->GetCID()));
                GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "bomb", aBuf);
            }
        }
    }
    QuestHammerHit(pTarget);
    PoliceHammerHit(pTarget);
}

void CCharacter::PoliceHammerHit(CCharacter *pTarget)
{
    //Police catch gangstazz
    if (m_pPlayer->m_PoliceRank && pTarget->m_FreezeTime > 1 && m_pPlayer->m_JailHammer)
    {
        char aBuf[256];

        if (!GameServer()->IsMinigame(pTarget->GetPlayer()->GetCID()))
        {
            if (pTarget->GetPlayer()->m_EscapeTime) //always prefer normal hammer
            {
                if (pTarget->GetPlayer()->GetMoney() < 200)
                {
                    str_format(aBuf, sizeof(aBuf), "You caught the gangster '%s' (5 minutes arrest).", Server()->ClientName(pTarget->GetPlayer()->GetCID()));
                    GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
                    GameServer()->SendChatTarget(m_pPlayer->GetCID(), "+5 minutes extra arrest: He had no money to corrupt you!");

                    str_format(aBuf, sizeof(aBuf), "You were arrested for 5 minutes by '%s'.", Server()->ClientName(m_pPlayer->GetCID()));
                    GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCID(), aBuf);
                    GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCID(), "+5 minutes extra: You couldn't corrupt the police!");
                    pTarget->GetPlayer()->m_EscapeTime = 0;
                    pTarget->GetPlayer()->m_GangsterBagMoney = 0;
                    pTarget->GetPlayer()->JailPlayer(600); //10 minutes jail
                    pTarget->GetPlayer()->m_JailCode = rand() % 8999 + 1000;
                }
                else
                {
                    str_format(aBuf, sizeof(aBuf), "You caught the gangster '%s' (5 minutes arrest).", Server()->ClientName(pTarget->GetPlayer()->GetCID()));
                    GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
                    GameServer()->SendChatTarget(m_pPlayer->GetCID(), "+200 money (corruption)");
                    str_format(aBuf, sizeof(aBuf), "caught gangster '%s'", Server()->ClientName(pTarget->GetPlayer()->GetCID()));
                    m_pPlayer->MoneyTransaction(+200, aBuf);

                    str_format(aBuf, sizeof(aBuf), "You were arrested 5 minutes by '%s'.", Server()->ClientName(m_pPlayer->GetCID()));
                    GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCID(), aBuf);
                    GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCID(), "-200 money (corruption)");
                    pTarget->GetPlayer()->m_EscapeTime = 0;
                    pTarget->GetPlayer()->m_GangsterBagMoney = 0;
                    pTarget->GetPlayer()->JailPlayer(300); //5 minutes jail
                    pTarget->GetPlayer()->m_JailCode = rand() % 8999 + 1000;
                    pTarget->GetPlayer()->MoneyTransaction(-200, "jail");

                }
            }
            else //super jail hammer
            {
                if (m_pPlayer->m_JailHammer > 1)
                {
                    str_format(aBuf, sizeof(aBuf), "You jailed '%s' (%d seconds arrested).", Server()->ClientName(pTarget->GetPlayer()->GetCID()), m_pPlayer->m_JailHammer);
                    GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
                    m_pPlayer->m_JailHammerDelay = Server()->TickSpeed() * 1200; // can only use every 20 minutes super hammer

                    str_format(aBuf, sizeof(aBuf), "You were arrested by '%s' for %d seconds.", m_pPlayer->m_JailHammer, Server()->ClientName(m_pPlayer->GetCID()));
                    GameServer()->SendChatTarget(pTarget->GetPlayer()->GetCID(), aBuf);
                    pTarget->GetPlayer()->m_EscapeTime = 0;
                    pTarget->GetPlayer()->m_GangsterBagMoney = 0;
                    pTarget->GetPlayer()->JailPlayer(Server()->TickSpeed() * m_pPlayer->m_JailHammer);
                    pTarget->GetPlayer()->m_JailCode = rand() % 8999 + 1000;
                }
            }
        }
    }
}

void CCharacter::DDPPGunFire(vec2 Direction)
{
    if (m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
    {
        if (m_pPlayer->m_QuestStateLevel == 9) //race with conditions
        {
            if (g_Config.m_SvQuestRaceCondition == 1) //no gun (also jetpack)
            {
                GameServer()->QuestFailed2(m_pPlayer->GetCID());
            }
        }
    }

    //spooky ghost
    if (m_pPlayer->m_PlayerFlags&PLAYERFLAG_SCOREBOARD && m_pPlayer->m_SpookyGhost && m_Core.m_ActiveWeapon == WEAPON_GUN && m_CountSpookyGhostInputs)
    {
        m_TimesShot++;
        if ((m_TimesShot == 2) && !m_pPlayer->m_SpookyGhostActive)
        {
            SetSpookyGhost();
            m_TimesShot = 0;
        }
        else if ((m_TimesShot == 2) && m_pPlayer->m_SpookyGhostActive)
        {
            UnsetSpookyGhost();
            m_TimesShot = 0;
        }
        m_CountSpookyGhostInputs = false;
    }   
}

bool CCharacter::SpecialGunProjectile(vec2 Direction, vec2 ProjStartPos, int Lifetime)
{
    if(m_pPlayer->m_SpookyGhostActive && (m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun))
    {
        float a = GetAngle(Direction);
        a += (0.070f) * 2;

        new CPlasmaBullet
        (
            GameWorld(),
            m_pPlayer->GetCID(),	//owner
            ProjStartPos,			//pos
            Direction,				//dir
            0,						//freeze
            0,						//explosive
            0,						//unfreeze
            1,						//bloody
            1,						//ghost
            Team(),					//responibleteam
            6,						//lifetime
            1.0f,					//accel
            10.0f					//speed
        );
            
        new CPlasmaBullet
        (
            GameWorld(),
            m_pPlayer->GetCID(),						//owner
            ProjStartPos,								//pos
            vec2(cosf(a - 0.200f), sinf(a - 0.200f)),	//dir
            0,											//freeze
            0,											//explosive
            0,											//unfreeze
            1,											//bloody
            1,											//ghost
            Team(),										//responibleteam
            6,											//lifetime
            1.0f,										//accel
            10.0f										//speed
            );

        new CPlasmaBullet
        (
            GameWorld(),
            m_pPlayer->GetCID(),						//owner
            ProjStartPos,								//pos
            vec2(cosf(a - 0.040f), sinf(a - 0.040f)),	//dir
            0,											//freeze
            0,											//explosive
            0,											//unfreeze
            1,											//bloody
            1,											//ghost
            Team(),										//responibleteam
            6,											//lifetime
            1.0f,										//accel
            10.0f										//speed
        );

        GameServer()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
    }
    else if (m_autospreadgun || m_pPlayer->m_InfAutoSpreadGun)
    {
        //idk if this is the right place to set some shooting speed but yolo
        //just copied the general code for all weapons and put it here
        if (!m_ReloadTimer)
        {
            float FireDelay;
            if (!m_TuneZone)
                GameServer()->Tuning()->Get(38 + m_Core.m_ActiveWeapon, &FireDelay);
            else
                GameServer()->TuningList()[m_TuneZone].Get(38 + m_Core.m_ActiveWeapon, &FireDelay);
            m_ReloadTimer = FireDelay * Server()->TickSpeed() / 5000;
        }

        //----- ChillerDragon tried to create 2nd projectile -----
        //Just copy and pasted the whole code agian
        float a = GetAngle(Direction);
        a += (0.070f) * 2;

        new CProjectile
            (
                GameWorld(),
                WEAPON_GUN,//Type
                m_pPlayer->GetCID(),//Owner
                ProjStartPos,//Pos
                vec2(cosf(a), sinf(a)),//Dir
                Lifetime,//Span
                0,//Freeze
                0,//Explosive
                0,//Force
                -1,//SoundImpact
                WEAPON_GUN//Weapon
                );

        new CProjectile
            (
                GameWorld(),
                WEAPON_GUN,//Type
                m_pPlayer->GetCID(),//Owner
                ProjStartPos,//Pos
                vec2(cosf(a - 0.070f), sinf(a - 0.070f)),//Dir
                Lifetime,//Span
                0,//Freeze
                0,//Explosive
                0,//Force
                -1,//SoundImpact
                WEAPON_GUN//Weapon
                );

        new CProjectile
            (
                GameWorld(),
                WEAPON_GUN,//Type
                m_pPlayer->GetCID(),//Owner
                ProjStartPos,//Pos
                vec2(cosf(a - 0.170f), sinf(a - 0.170f)),//Dir
                Lifetime,//Span
                0,//Freeze
                0,//Explosive
                0,//Force
                -1,//SoundImpact
                WEAPON_GUN//Weapon
                );

        CProjectile *pProj = new CProjectile
            (
                GameWorld(),
                WEAPON_GUN,//Type
                m_pPlayer->GetCID(),//Owner
                ProjStartPos,//Pos
                Direction,//Dir
                Lifetime,//Span
                0,//Freeze
                0,//Explosive
                0,//Force
                -1,//SoundImpact
                WEAPON_GUN//Weapon
                );

        // pack the Projectile and send it to the client Directly
        CNetObj_Projectile p;
        pProj->FillInfo(&p);

        CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
        Msg.AddInt(1);
        for (unsigned i = 0; i < sizeof(CNetObj_Projectile) / sizeof(int); i++)
            Msg.AddInt(((int *)&p)[i]);

        Server()->SendMsg(&Msg, 0, m_pPlayer->GetCID());
        GameServer()->CreateSound(m_Pos, SOUND_GUN_FIRE, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
    }
    else if (m_pPlayer->m_SpookyGhostActive)
    {
        new CPlasmaBullet
        (
            GameWorld(), 
            m_pPlayer->GetCID(),	//owner
            ProjStartPos,			//pos
            Direction,				//dir
            0,						//freeze
            0,						//explosive
            0,						//unfreeze
            1,						//bloody
            1,						//ghost
            Team(),					//responibleteam
            6,						//lifetime
            1.0f,					//accel
            10.0f					//speed
        );
        GameServer()->CreateSound(m_Pos, SOUND_PICKUP_HEALTH, Teams()->TeamMask(Team(), -1, m_pPlayer->GetCID()));
    }
    else if (m_pPlayer->m_lasergun)
    {
        int RifleSpread = 1;
        float Spreading[] = { -0.070f, 0, 0.070f };
        for (int i = -RifleSpread; i <= RifleSpread; ++i)
        {
            float a = GetAngle(Direction);
            a += Spreading[i + 1];
            new CLaser(GameWorld(), m_Pos, vec2(cosf(a), sinf(a)), GameServer()->Tuning()->m_LaserReach, m_pPlayer->GetCID(), 0);
        }


        // summon meteor
        //CMeteor *pMeteor = new CMeteor(GameWorld(), ProjStartPos);
    }
    else
        return false;
    return true;
}

bool CCharacter::FreezeShotgun(vec2 Direction, vec2 ProjStartPos)
{
    if (m_freezeShotgun || m_pPlayer->m_IsVanillaWeapons) //freezeshotgun
    {
        int ShotSpread = 2;

        CMsgPacker Msg(NETMSGTYPE_SV_EXTRAPROJECTILE);
        Msg.AddInt(ShotSpread * 2 + 1);

        for (int i = -ShotSpread; i <= ShotSpread; ++i)
        {
            float Spreading[] = { -0.185f, -0.070f, 0, 0.070f, 0.185f };
            float a = GetAngle(Direction);
            a += Spreading[i + 2];
            float v = 1 - (absolute(i) / (float)ShotSpread);
            float Speed = mix((float)GameServer()->Tuning()->m_ShotgunSpeeddiff, 1.0f, v);
            CProjectile *pProj = new CProjectile(GameWorld(), WEAPON_SHOTGUN,
                m_pPlayer->GetCID(),
                ProjStartPos,
                vec2(cosf(a), sinf(a))*Speed,
                (int)(Server()->TickSpeed()*GameServer()->Tuning()->m_ShotgunLifetime),
                1, 0, 0, -1, WEAPON_SHOTGUN);

            // pack the Projectile and send it to the client Directly
            CNetObj_Projectile p;
            pProj->FillInfo(&p);

            for (unsigned i = 0; i < sizeof(CNetObj_Projectile) / sizeof(int); i++)
                Msg.AddInt(((int *)&p)[i]);
        }

        Server()->SendMsg(&Msg, MSGFLAG_VITAL, m_pPlayer->GetCID());

        GameServer()->CreateSound(m_Pos, SOUND_SHOTGUN_FIRE);
        return true;
    }
    return false;
}

void CCharacter::DDPPFireWeapon()
{
    QuestFireWeapon();
	m_AttackTick = Server()->Tick();

	if (m_pPlayer->m_IsVanillaWeapons)
	{
		if (m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo > 0) // -1 == unlimited
			m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;
	}

	if (m_aDecreaseAmmo[m_Core.m_ActiveWeapon]) // picked up a dropped weapon without infinite bullets (-1)
	{
		m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;

		if (m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo == 0)
		{
			m_aDecreaseAmmo[m_Core.m_ActiveWeapon] = false;
			m_aWeapons[m_Core.m_ActiveWeapon].m_Got = false;
			SetWeaponThatChrHas();
		}
	}

	// shop window
	if ((m_ChangeShopPage) && (m_ShopWindowPage != -1) && (m_PurchaseState == 1))
	{
		ShopWindow(GetAimDir());
		m_ChangeShopPage = false;
	}

	//spawn weapons

	if (m_pPlayer->m_SpawnShotgunActive && m_Core.m_ActiveWeapon == WEAPON_SHOTGUN) 
	{
		m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;
		if (m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo == 0)
		{
			m_pPlayer->m_SpawnShotgunActive = 0;
			SetWeaponGot(WEAPON_SHOTGUN, false);
			SetWeaponThatChrHas();
		}
	}

	if (m_pPlayer->m_SpawnGrenadeActive && m_Core.m_ActiveWeapon == WEAPON_GRENADE)
	{
		m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;
		if (m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo == 0)
		{
			m_pPlayer->m_SpawnGrenadeActive = 0;
			SetWeaponGot(WEAPON_GRENADE, false);
			SetWeaponThatChrHas();
		}
	}

	if (m_pPlayer->m_SpawnRifleActive && m_Core.m_ActiveWeapon == WEAPON_RIFLE)
	{
		m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo--;
		if (m_aWeapons[m_Core.m_ActiveWeapon].m_Ammo == 0)
		{
			m_pPlayer->m_SpawnRifleActive = 0;
			SetWeaponGot(WEAPON_RIFLE, false);
			SetWeaponThatChrHas();
		}
	}
}

void CCharacter::SendShopMessage(const char *pMsg)
{
	int recv = m_pPlayer->m_ShopBotMesssagesRecieved / 2; // 2 messages = enter + leave
	if (g_Config.m_SvMaxShopMessages != -1 && g_Config.m_SvMaxShopMessages <= recv)
		return;

	GameServer()->SendChat(GameServer()->GetShopBot(), CGameContext::CHAT_TO_ONE_CLIENT, pMsg, -1, m_pPlayer->GetCID());
	m_pPlayer->m_ShopBotMesssagesRecieved++;
}

bool CCharacter::InputActive()
{
	static bool IsWalk = false;
	// static bool IsFire = false;
	static bool IsJump = false;
	static bool IsHook = false;
	if (m_Input.m_Direction)
		IsWalk = true;
	// if (m_Input.m_Fire)
		// IsFire = true;
	if (m_Input.m_Jump)
		IsJump = true;
	if (m_Input.m_Hook)
		IsHook = true;
	if (IsWalk && IsJump && IsHook)
	{
		IsWalk = false;
		IsJump = false;
		IsHook = false;
		return true;
	}
	return false;
}

int CCharacter::GetAimDir()
{
	if (m_Input.m_TargetX < 0)
		return -1;
	else
		return 1;
	return 0;
}

void CCharacter::TakeHammerHit(CCharacter* pFrom)
{
	vec2 Dir;
	if (length(m_Pos - pFrom->m_Pos) > 0.0f)
		Dir = normalize(m_Pos - pFrom->m_Pos);
	else
		Dir = vec2(0.f, -1.f);

	vec2 Push = vec2(0.f, -1.f) + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f;
	//if (GameServer()->m_pController->IsTeamplay() && pFrom->GetPlayer() && m_pPlayer->GetTeam() == pFrom->GetPlayer()->GetTeam() && IsFreezed()) 
	if (false)
	{
		Push.x *= g_Config.m_SvMeltHammerScaleX*0.01f;
		Push.y *= g_Config.m_SvMeltHammerScaleY*0.01f;
	}
	else 
	{
		Push.x *= g_Config.m_SvHammerScaleX*0.01f;
		Push.y *= g_Config.m_SvHammerScaleY*0.01f;
	}

	m_Core.m_Vel += Push;
}

void CCharacter::KillFreeze(bool unfreeze)
{
	if (!g_Config.m_SvFreezeKillDelay)
		return;
	if (unfreeze) // stop counting
	{
		m_FirstFreezeTick = 0;
		return;
	}
	if (!m_FirstFreezeTick) // start counting
	{
		m_FirstFreezeTick = Server()->Tick();
		return;
	}
	if (Server()->Tick() - m_FirstFreezeTick > (Server()->TickSpeed() / 10) * g_Config.m_SvFreezeKillDelay)
	{
		Die(m_pPlayer->GetCID(), WEAPON_SELF);
		m_FirstFreezeTick = 0;
	}
}
