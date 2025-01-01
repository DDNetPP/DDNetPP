// ddnet++ tile character stuff
// see also ddnetpp/tiles.cpp

#include <engine/server/server.h>
#include <engine/shared/config.h>
#include <game/mapitems.h>
#include <game/mapitems_ddpp.h>
#include <game/server/ddpp/shop.h>
#include <game/server/gamecontext.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/player.h>

#include "character.h"

// TODO: this should probably moved to ddnetpp/tiles.cpp
bool CCharacter::HandleTilesDDPP(int Index)
{
	if(g_Config.m_SvOffDDPP)
		return false;

	CGameControllerDDRace *Controller = (CGameControllerDDRace *)GameServer()->m_pController;
	// DDNet++ finish tile
	if(((m_TileIndex == TILE_DDPP_END) || (m_TileFIndex == TILE_DDPP_END)) && !m_DDPP_Finished)
	{
		char aBuf[256];
		if(m_DDRaceState == DDRACE_STARTED)
		{
			float Time = (float)(Server()->Tick() - Controller->Teams().GetStartTime(m_pPlayer)) / ((float)Server()->TickSpeed());
			if(Time < 0.000001f)
				return false;
			str_format(aBuf, sizeof(aBuf), "'%s' finished the special race [%d:%5.2f]!", Server()->ClientName(m_pPlayer->GetCid()), (int)Time / 60, Time - ((int)Time / 60 * 60));
			GameServer()->SendChat(-1, TEAM_ALL, aBuf);

			// quest
			if(m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
			{
				if(m_pPlayer->m_QuestStateLevel == 7)
				{
					if((int)Time > g_Config.m_SvQuestSpecialRaceTime)
					{
						GameServer()->QuestFailed(m_pPlayer->GetCid());
					}
					else
					{
						GameServer()->QuestCompleted(m_pPlayer->GetCid());
					}
				}
			}
		}
		else
		{
			// str_format(aBuf, sizeof(aBuf), "'%s' finished the special race [%d seconds]!", Server()->ClientName(m_pPlayer->GetCid()), m_SpawnTick / Server()->TickSpeed()); //prints server up time in sec
			str_format(aBuf, sizeof(aBuf), "'%s' finished the special race !", Server()->ClientName(m_pPlayer->GetCid()));
			GameServer()->SendChat(-1, TEAM_ALL, aBuf);

			// quest
			if(m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
			{
				if(m_pPlayer->m_QuestStateLevel == 7)
				{
					if(Server()->Tick() > m_SpawnTick + Server()->TickSpeed() * g_Config.m_SvQuestSpecialRaceTime)
					{
						GameServer()->QuestFailed(m_pPlayer->GetCid());
					}
					else
					{
						GameServer()->QuestCompleted(m_pPlayer->GetCid());
					}
				}
			}
		}

		if(m_pPlayer->m_QuestState == CPlayer::QUEST_RACE)
		{
			if(m_pPlayer->m_QuestStateLevel == 6)
			{
				GameServer()->QuestCompleted(m_pPlayer->GetCid());
			}
			else if(m_pPlayer->m_QuestStateLevel == 8) //backwards
			{
				GameServer()->QuestAddProgress(m_pPlayer->GetCid(), 2, 1);
			}
		}

		m_DDPP_Finished = true;
	}

	// DDNet++ config tiles
	if((m_TileIndex == TILE_CONFIG_1) || (m_TileFIndex == TILE_CONFIG_1))
	{
		if(HandleConfigTile(g_Config.m_SvCfgTile1))
			return false;
	}
	else if((m_TileIndex == TILE_CONFIG_2) || (m_TileFIndex == TILE_CONFIG_2))
	{
		if(HandleConfigTile(g_Config.m_SvCfgTile2))
			return false;
	}

	// // TODO: use enum here instead of magic number
	// if (((m_TileIndex == 66) || (m_TileFIndex == 66)) && m_Core.m_Vel.x < 0)
	// {

	// 	if ((GameServer()->m_pController->m_apFlags[1]->GetCarrier() == this || ((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0]->GetCarrier() == this)
	// 	{
	// 	}
	// 	else
	// 	{
	// 		/*
	// 		if (!(m_LastIndexTile == 66 || m_LastIndexFrontTile == 66) ){
	// 		char aBuf[256];
	// 		str_format(aBuf, sizeof(aBuf), "You need a Flag to enter this Area!");
	// 		GameServer()->SendChatTarget(m_pPlayer->GetCid(), aBuf);
	// 		}*/

	// 		if ((int)GameServer()->Collision()->GetPos(MapIndexL).x)
	// 			if ((int)GameServer()->Collision()->GetPos(MapIndexL).x < (int)m_Core.m_Pos.x)
	// 				m_Core.m_Pos = m_PrevPos;
	// 		m_Core.m_Vel.x = 0;
	// 	}
	// }

	// if (((m_TileIndex == 67) || (m_TileFIndex == 67)) && m_Core.m_Vel.x > 0)
	// {

	// 	if (((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[1]->GetCarrier() == this || ((CGameControllerDDRace*)GameServer()->m_pController)->m_apFlags[0]->GetCarrier() == this) {
	// 	}
	// 	else {
	// 		/*
	// 		if (!(m_LastIndexTile == 67 || m_LastIndexFrontTile == 67) ){
	// 		char aBuf[256];
	// 		str_format(aBuf, sizeof(aBuf), "You need a Flag to enter this Area!");
	// 		GameServer()->SendChatTarget(m_pPlayer->GetCid(), aBuf);
	// 		}*/

	// 		if ((int)GameServer()->Collision()->GetPos(MapIndexL).x)
	// 			if ((int)GameServer()->Collision()->GetPos(MapIndexL).x < (int)m_Core.m_Pos.x)
	// 				m_Core.m_Pos = m_PrevPos;
	// 		m_Core.m_Vel.x = 0;
	// 	}

	// }

	// cosmetic tiles
	//rainbow
	if(((m_TileIndex == TILE_RAINBOW) || (m_TileFIndex == TILE_RAINBOW)))
	{
		if(((m_LastIndexTile == TILE_RAINBOW) || (m_LastIndexFrontTile == TILE_RAINBOW)))
			return false;

		if((m_Rainbow) || (m_pPlayer->m_InfRainbow))
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You lost rainbow!");
			m_Rainbow = false;
			m_pPlayer->m_InfRainbow = false;
		}
		else
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You got rainbow!");
			m_Rainbow = true;
		}
	}

	//bloody
	if(((m_TileIndex == TILE_BLOODY) || (m_TileFIndex == TILE_BLOODY)))
	{
		if(((m_LastIndexTile == TILE_BLOODY) || (m_LastIndexFrontTile == TILE_BLOODY)))
			return false;

		if((m_Bloody) || (m_StrongBloody) || (m_pPlayer->m_InfBloody))
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You lost bloody!");
			m_Bloody = false;
			m_StrongBloody = false;
			m_pPlayer->m_InfBloody = false;
		}
		else
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You got bloody!");
			m_Bloody = true;
		}
	}

	// atom
	if(((m_TileIndex == TILE_ATOM) || (m_TileFIndex == TILE_ATOM)))
	{
		if(((m_LastIndexTile == TILE_ATOM) || (m_LastIndexFrontTile == TILE_ATOM)))
			return false;

		if((m_Atom) || (m_pPlayer->m_InfAtom))
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You lost atom!");
			m_Atom = false;
			m_pPlayer->m_InfAtom = false;
		}
		else
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You got atom!");
			m_Atom = true;
		}
	}

	// trail
	if(((m_TileIndex == TILE_TRAIL) || (m_TileFIndex == TILE_TRAIL)))
	{
		if(((m_LastIndexTile == TILE_TRAIL) || (m_LastIndexFrontTile == TILE_TRAIL)))
			return false;

		if((m_Trail) || (m_pPlayer->m_InfTrail))
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You lost trail!");
			m_Trail = false;
			m_pPlayer->m_InfTrail = false;
		}
		else
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You got trail!");
			m_Trail = true;
		}
	}

	// spread gun
	if(((m_TileIndex == TILE_SPREAD_GUN) || (m_TileFIndex == TILE_SPREAD_GUN)))
	{
		if(((m_LastIndexTile == TILE_SPREAD_GUN) || (m_LastIndexFrontTile == TILE_SPREAD_GUN)))
			return false;

		if((m_autospreadgun) || (m_pPlayer->m_InfAutoSpreadGun))
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You lost spread gun!");
			m_autospreadgun = false;
			m_pPlayer->m_InfAutoSpreadGun = false;
		}
		else
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You got spread gun!");
			m_autospreadgun = true;
		}
	}

	m_LastIndexTile = m_TileIndex; // do not remove
	m_LastIndexFrontTile = m_TileFIndex; // do not remove

	//hammer tiles
	if(((m_TileIndex == TILE_NO_HAMMER) || (m_TileFIndex == TILE_NO_HAMMER)))
	{
		m_Core.m_aWeapons[WEAPON_HAMMER].m_Got = false;
		if(!SetWeaponThatChrHas()) // Cheat gun if hammer was last weapon
		{
			m_Core.m_aWeapons[WEAPON_GUN].m_Got = true;
		}
		SetWeaponThatChrHas();
	}

	//Money Tiles
	if(((m_TileIndex == TILE_MONEY) || (m_TileFIndex == TILE_MONEY)))
	{
		MoneyTile();
	}

	if(((m_TileIndex == TILE_MONEY_POLICE) || (m_TileFIndex == TILE_MONEY_POLICE)))
	{
		MoneyTilePolice();
	}

	if(((m_TileIndex == TILE_MONEY_PLUS) || (m_TileFIndex == TILE_MONEY_PLUS)))
	{
		MoneyTilePlus();
	}

	if(((m_TileIndex == TILE_FNG_SCORE) || (m_TileFIndex == TILE_FNG_SCORE)))
	{
		Die(m_pPlayer->GetCid(), WEAPON_WORLD, true, true);
	}

	if(((m_TileIndex == TILE_MONEY_DOUBLE) || (m_TileFIndex == TILE_MONEY_DOUBLE)))
	{
		MoneyTileDouble();
	}

	if(((m_TileIndex == TILE_KING_OF_THE_HILL) || (m_TileFIndex == TILE_KING_OF_THE_HILL)))
	{
		if((Server()->Tick() % 500) == 0 && !m_FreezeTime)
			m_pPlayer->m_KingOfTheHillScore++;
	}

	// ROOM POINT
	bool Allowed = false;
	if(g_Config.m_SvRoomState == 0) //all
	{
		Allowed = true;
	}
	else if(g_Config.m_SvRoomState == 1) //buy
	{
		Allowed = m_pPlayer->m_BoughtRoom;
	}
	else if(g_Config.m_SvRoomState == 2) //buy invite
	{
		Allowed = m_pPlayer->m_BoughtRoom || m_HasRoomKeyBySuperModerator;
	}
	else if(g_Config.m_SvRoomState == 3) //buy admin
	{
		Allowed = m_pPlayer->m_BoughtRoom || Server()->GetAuthedState(GetPlayer()->GetCid());
	}
	else if(g_Config.m_SvRoomState == 4) //buy admin invite
	{
		Allowed = m_pPlayer->m_BoughtRoom || Server()->GetAuthedState(GetPlayer()->GetCid()) || m_HasRoomKeyBySuperModerator;
	}

	//ROOMTILE
	if(((m_TileIndex == TILE_ROOM) || (m_TileFIndex == TILE_ROOM)) && !Allowed) // Admins got it free
	{
		//ChillerDragon upgrade to not cheat the map or stuff and tele too far

		//if (distance(m_Core.m_Pos, m_PrevSavePos) > 10 * 32)
		//{
		//dbg_msg("debug","Player's last pos too nearby distance: INT %d FLOAT %2.f", distance(m_Core.m_Pos, m_PrevSavePos), distance(m_Core.m_Pos, m_PrevSavePos));
		if(m_Core.m_Vel.x > 0)
		{
			m_Core.m_Pos = vec2(m_Core.m_Pos.x - 1 * 32, m_Core.m_Pos.y);
			m_Pos = vec2(m_Pos.x - 1 * 32, m_Pos.y);
			m_Core.m_Vel = vec2(-3, 0);
		}
		else
		{
			m_Core.m_Pos = vec2(m_Core.m_Pos.x + 1 * 32, m_Core.m_Pos.y);
			m_Pos = vec2(m_Pos.x + 1 * 32, m_Pos.y);
			m_Core.m_Vel = vec2(+3, 0);
		}
		//}
		//else
		//{
		//	dbg_msg("debug","distance ok loading PrevSavePos distance: INT %d FLOAT %2.f", distance(m_Core.m_Pos, m_PrevSavePos), distance(m_Core.m_Pos, m_PrevSavePos));

		//	m_Core.m_Pos = m_PrevSavePos;
		//	m_Pos = m_PrevSavePos;
		//	m_PrevPos = m_PrevSavePos;
		//	m_Core.m_Vel = vec2(0, 0);
		//	m_Core.m_HookedPlayer = -1;
		//	m_Core.m_HookState = HOOK_RETRACTED;
		//	m_Core.m_TriggeredEvents |= COREEVENT_HOOK_RETRACT;
		//	GameWorld()->ReleaseHooked(GetPlayer()->GetCid());
		//	m_Core.m_HookPos = m_Core.m_Pos;
		//}

		if(!m_WasInRoom)
		{
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You need a key to enter this area!\nTry '/buy room_key' to enter this area.");
		}

		m_WasInRoom = true;
	}

	if(m_TileIndex == TILE_BLOCK_DM_JOIN || m_TileFIndex == TILE_BLOCK_DM_JOIN)
	{
		if(!m_pPlayer->m_IsBlockDeathmatch)
		{
			m_pPlayer->m_IsBlockDeathmatch = true;
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "[BLOCK] you joined the deathmatch arena!");
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "[BLOCK] type /leave to leave");
		}
	}

	// sv_room_captcha
	if(m_TileIndex == TILE_CAPTCHA_VERIFY || m_TileFIndex == TILE_CAPTCHA_VERIFY)
	{
		m_pPlayer->OnHumanVerify();

		// human verify kills the character
		// so we have to drop all the other code to avoid nullptr exceptions
		return true;
	}

	if(m_TileIndex == TILE_BANK_IN || m_TileFIndex == TILE_BANK_IN) //BANKTILES
	{
		if(Server()->Tick() % 30 == 0 && GameServer()->m_IsBankOpen)
		{
			if(GameServer()->m_pController->HasFlag(this) != -1) //has flag
			{
				if(!m_pPlayer->IsLoggedIn()) // only print stuff if player is not logged in while flag carry
				{
					GameServer()->SendBroadcast("~ B A N K ~", m_pPlayer->GetCid(), 0);
				}
			}
			else // no flag --> print always
			{
				GameServer()->SendBroadcast("~ B A N K ~", m_pPlayer->GetCid(), 0);
			}
		}
		m_InBank = true;
	}

	if(m_TileIndex == TILE_SHOP || m_TileFIndex == TILE_SHOP) // SHOP
	{
		if(!GameServer()->Shop()->IsInShop(GetPlayer()->GetCid()))
		{
			m_EnteredShop = true;
			GameServer()->Shop()->EnterShop(GetPlayer()->GetCid());
		}
		if(Server()->Tick() % 450 == 0 || m_EnteredShop)
		{
			if(GameServer()->m_pController->HasFlag(this) != -1) //has flag
			{
				if(!m_pPlayer->IsLoggedIn()) // only print stuff if player is not logged in while flag carry
				{
					GameServer()->SendBroadcast("~ S H O P ~", m_pPlayer->GetCid(), 0);
				}
			}
			else // no flag --> print always
			{
				GameServer()->SendBroadcast("~ S H O P ~", m_pPlayer->GetCid(), 0);
			}
		}
		if(m_EnteredShop)
		{
			if(m_pPlayer->m_ShopBotAntiSpamTick <= Server()->Tick())
			{
				char aBuf[256];
				str_format(aBuf, sizeof(aBuf), "Welcome to the shop, %s! Press f4 to start shopping.", Server()->ClientName(m_pPlayer->GetCid()));
				SendShopMessage(aBuf);
			}
			m_EnteredShop = false;
		}
	}

	if(m_TileIndex == TILE_JAIL || m_TileFIndex == TILE_JAIL)
	{
		//GameServer()->SendBroadcast("You were arrested by the police!", m_pPlayer->GetCid(), 0); //dont spam people in jail this is just an tele tile
	}
	else if(m_TileIndex == TILE_JAILRELEASE || m_TileFIndex == TILE_JAILRELEASE)
	{
		//GameServer()->SendBroadcast("Your life as a gangster is over, don't get caught again!", m_pPlayer->GetCid(), 0); //dont send the message here wtf this is just an to tele tile
		m_InJailOpenArea = true;
		//dbg_msg("ddpp-tiles", "in jail release area");
	}

	if((m_TileIndex == TILE_VANILLA_MODE || m_TileFIndex == TILE_VANILLA_MODE) && !(m_pPlayer->m_IsVanillaDmg && m_pPlayer->m_IsVanillaWeapons))
	{
		if(m_pPlayer->DummyMode() != DUMMYMODE_ADVENTURE)
		{
			m_pPlayer->m_IsVanillaModeByTile = true;
			m_pPlayer->m_IsVanillaDmg = true;
			m_pPlayer->m_IsVanillaWeapons = true;
			m_pPlayer->m_IsVanillaCompetetive = true;
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You entered a vanilla area.");
		}
	}

	if((m_TileIndex == TILE_DDRACE_MODE || m_TileFIndex == TILE_DDRACE_MODE) && (m_pPlayer->m_IsVanillaDmg && m_pPlayer->m_IsVanillaWeapons))
	{
		if(m_pPlayer->DummyMode() == DUMMYMODE_ADVENTURE)
		{
			Die(m_pPlayer->GetCid(), WEAPON_SELF);
		}
		else
		{
			m_pPlayer->m_IsVanillaModeByTile = false;
			m_pPlayer->m_IsVanillaDmg = false;
			m_pPlayer->m_IsVanillaWeapons = false;
			m_pPlayer->m_IsVanillaCompetetive = false;
			GameServer()->SendChatTarget(GetPlayer()->GetCid(), "You entered a ddrace area.");
		}
	}

	// freeze (override)
	if(((m_TileIndex == TILE_FREEZE) || (m_TileFIndex == TILE_FREEZE)) && !m_Core.m_Super && !m_Core.m_DeepFrozen)
	{
		//Chiller Special
		if(m_Core.m_Pos.y > 223 * 32 && m_Core.m_Pos.y < 225 * 32 && m_Core.m_Pos.x < 438 * 32 && m_Core.m_Pos.x > 427 * 32 && m_fake_super)
			UnFreeze();
		else
			m_DummyFreezed = true;
	}
	return false;
}

void CCharacter::MoneyTile()
{
	m_OnMoneytile = MONEYTILE_NORMAL;
	if(Server()->Tick() % 50)
		return;
	if(IsInDDraceTeam())
		return;
	if(!g_Config.m_SvFreezeFarm)
		if(m_pPlayer && m_pPlayer->GetCharacter() && m_pPlayer->GetCharacter()->m_FreezeTime)
			return;
	if(!m_pPlayer->IsLoggedIn())
	{
		GameServer()->SendBroadcast(GameServer()->Loc("You need to be logged in to use moneytiles. \nGet an account with '/register <name> <pw> <pw>'", m_pPlayer->GetCid()), m_pPlayer->GetCid(), 0);
		return;
	}
	if(m_pPlayer->m_QuestState == CPlayer::QUEST_FARM)
	{
		if(m_pPlayer->m_QuestStateLevel < 7) // 10 money
		{
			m_pPlayer->m_QuestProgressValue2++;
			if(m_pPlayer->m_QuestProgressValue2 > m_pPlayer->m_QuestStateLevel)
			{
				GameServer()->QuestAddProgress(m_pPlayer->GetCid(), 10);
				m_pPlayer->m_QuestProgressValue2 = 0;
			}
		}
		else if(m_pPlayer->m_QuestStateLevel == 7)
		{
			// moneytile police
		}
		else if(m_pPlayer->m_QuestStateLevel == 8)
		{
			m_pPlayer->m_QuestProgressValue2++;
			if(m_pPlayer->m_QuestProgressValue2 > 10)
			{
				GameServer()->QuestAddProgress(m_pPlayer->GetCid(), 10);
				m_pPlayer->m_QuestProgressValue2 = 0;
			}
		}
	}
	if(m_pPlayer->IsMaxLevel())
	{
		if(m_pPlayer->m_xpmsg)
		{
			GameServer()->SendBroadcast("You reached the maximum level.", m_pPlayer->GetCid(), 0);
		}
		return;
	}

	int XP = 0;
	int Money = 0;
	int VIPBonus = 0;

	// flag extra xp
	if(GameServer()->m_pController->HasFlag(this) != -1)
	{
		XP += 1;
	}

	// vip+ get 2 bonus
	if(m_pPlayer->m_Account.m_IsSuperModerator)
	{
		XP += 2;
		Money += 2;
		VIPBonus = 2; // only for broadcast not used in calculation
	}
	// vip get 1 bonus
	else if(m_pPlayer->m_Account.m_IsModerator)
	{
		XP += 1;
		Money += 1;
		VIPBonus = 1; // only for broadcast not used in calculation
	}

	// tile gain and survival bonus
	XP += 1 + m_survivexpvalue;
	Money += 1;

	// give money & xp
	m_pPlayer->GiveXP(XP);
	m_pPlayer->MoneyTransaction(Money);
	m_pPlayer->m_MoneyTilesMoney += Money;

	// show msg
	if(m_pPlayer->m_xpmsg)
	{
		// skip if other broadcasts activated:
		if(!m_pPlayer->m_hidejailmsg)
		{
			if(m_pPlayer->m_Account.m_EscapeTime > 0 || m_pPlayer->m_Account.m_JailTime > 0)
			{
				return;
			}
		}

		char FixBroadcast[32];
		if((m_pPlayer->GetXP() >= 1000000) && m_survivexpvalue > 0)
			str_copy(FixBroadcast, "                                       ", sizeof(FixBroadcast));
		else
			FixBroadcast[0] = '\0';

		char aBuf[128];
		char aMoney[128];
		char aXp[128];
		char aLevel[128];
		str_format(aMoney, sizeof(aMoney), "%s [%" PRId64 "] +1", GameServer()->Loc("Money", m_pPlayer->GetCid()), m_pPlayer->GetMoney());
		str_format(aXp, sizeof(aXp), "XP [%" PRId64 "/%" PRId64 "] +1", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP());
		str_format(aLevel, sizeof(aLevel), "%s [%d]", GameServer()->Loc("Level", m_pPlayer->GetCid()), m_pPlayer->GetLevel());

		// money
		if(VIPBonus)
		{
			str_format(aBuf, sizeof(aBuf), " +%d vip", VIPBonus);
			str_append(aMoney, aBuf, sizeof(aMoney));
		}

		// xp
		if(GameServer()->m_pController->HasFlag(this) != -1)
			str_append(aXp, " +1 flag", sizeof(aXp));
		if(VIPBonus)
		{
			str_format(aBuf, sizeof(aBuf), " +%d vip", VIPBonus);
			str_append(aXp, aBuf, sizeof(aXp));
		}
		if(m_survivexpvalue > 0)
		{
			str_format(aBuf, sizeof(aBuf), " +%d survival", m_survivexpvalue);
			str_append(aXp, aBuf, sizeof(aXp));
		}

		str_format(aBuf, sizeof(aBuf), "%s\n%s\n%s", aMoney, aXp, aLevel);
		str_append(aBuf, FixBroadcast, sizeof(aBuf));
		GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
	}
}

void CCharacter::MoneyTilePolice()
{
	m_OnMoneytile = MONEYTILE_POLICE;
	if(Server()->Tick() % 50)
		return;
	if(IsInDDraceTeam())
		return;
	if(!GameServer()->m_IsPoliceFarmActive)
	{
		GameServer()->SendBroadcast("Too many players on police tiles", m_pPlayer->GetCid(), 0);
		return;
	}
	if(!g_Config.m_SvFreezeFarm)
		if(m_pPlayer && m_pPlayer->GetCharacter() && m_pPlayer->GetCharacter()->m_FreezeTime)
			return;
	if(!m_pPlayer->IsLoggedIn())
	{
		GameServer()->SendBroadcast(GameServer()->Loc("You need to be logged in to use moneytiles. \nGet an account with '/register <name> <pw> <pw>'", m_pPlayer->GetCid()), m_pPlayer->GetCid(), 0);
		return;
	}
	if(m_pPlayer->m_QuestState == CPlayer::QUEST_FARM)
	{
		if(m_pPlayer->m_QuestStateLevel == 7)
		{
			m_pPlayer->m_QuestProgressValue2++;
			if(m_pPlayer->m_QuestProgressValue2 > 10)
			{
				GameServer()->QuestAddProgress(m_pPlayer->GetCid(), 10);
				m_pPlayer->m_QuestProgressValue2 = 0;
			}
		}
	}
	if(m_pPlayer->IsMaxLevel())
	{
		if(m_pPlayer->m_xpmsg)
		{
			GameServer()->SendBroadcast("You have reached the maximum level.", m_pPlayer->GetCid(), 0);
		}
		return;
	}

	int XP = 0;
	int Money = 0;
	int VIPBonus = 0;

	// vip+ get 2 bonus
	if(m_pPlayer->m_Account.m_IsSuperModerator)
	{
		XP += 2;
		Money += 2;
		VIPBonus = 2; // only for broadcast not used in calculation
	}
	// vip get 1 bonus
	else if(m_pPlayer->m_Account.m_IsModerator)
	{
		XP += 1;
		Money += 1;
		VIPBonus = 1; // only for broadcast not used in calculation
	}

	// tile gain and survival bonus
	XP += 2 + m_survivexpvalue;
	Money += 1 + m_pPlayer->m_Account.m_PoliceRank;

	// give money & xp
	m_pPlayer->GiveXP(XP);
	m_pPlayer->MoneyTransaction(Money);
	m_pPlayer->m_MoneyTilesMoney += Money;

	// show msg
	if(m_pPlayer->m_xpmsg)
	{
		// skip if other broadcasts activated:
		if(!m_pPlayer->m_hidejailmsg)
		{
			if(m_pPlayer->m_Account.m_EscapeTime > 0 || m_pPlayer->m_Account.m_JailTime > 0)
			{
				return;
			}
		}

		char FixBroadcast[64];
		if((m_pPlayer->GetXP() >= 1000000) && m_survivexpvalue > 0)
			str_copy(FixBroadcast, "                                       ", sizeof(FixBroadcast));
		else
			FixBroadcast[0] = '\0';

		char aBuf[128];
		char aMoney[128];
		char aXp[128];
		char aLevel[128];
		str_format(aMoney, sizeof(aMoney), "%s [%" PRId64 "] +1", GameServer()->Loc("Money", m_pPlayer->GetCid()), m_pPlayer->GetMoney());
		str_format(aXp, sizeof(aXp), "XP [%" PRId64 "/%" PRId64 "] +2", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP());
		str_format(aLevel, sizeof(aLevel), "%s [%d]", GameServer()->Loc("Level", m_pPlayer->GetCid()), m_pPlayer->GetLevel());

		// money
		if(m_pPlayer->m_Account.m_PoliceRank > 0)
		{
			str_format(aBuf, sizeof(aBuf), " +%d police", m_pPlayer->m_Account.m_PoliceRank);
			str_append(aMoney, aBuf, sizeof(aMoney));
		}
		if(VIPBonus)
		{
			str_format(aBuf, sizeof(aBuf), " +%d vip", VIPBonus);
			str_append(aMoney, aBuf, sizeof(aMoney));
		}

		// xp
		if(VIPBonus)
		{
			str_format(aBuf, sizeof(aBuf), " +%d vip", VIPBonus);
			str_append(aXp, aBuf, sizeof(aXp));
		}
		if(m_survivexpvalue > 0)
		{
			str_format(aBuf, sizeof(aBuf), " +%d survival", m_survivexpvalue);
			str_append(aXp, aBuf, sizeof(aXp));
		}

		str_format(aBuf, sizeof(aBuf), "%s\n%s\n%s", aMoney, aXp, aLevel);
		str_append(aBuf, FixBroadcast, sizeof(aBuf));
		GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
	}
}

void CCharacter::MoneyTileDouble()
{
	m_OnMoneytile = MONEYTILE_DOUBLE;
	if(Server()->Tick() % 50)
		return;
	if(!m_pPlayer) // seems useless but pleases clang
		return;
	if(IsInDDraceTeam())
		return;
	if(!g_Config.m_SvFreezeFarm)
		if(m_pPlayer && m_pPlayer->GetCharacter() && m_pPlayer->GetCharacter()->m_FreezeTime)
			return;
	if(g_Config.m_SvMinDoubleTilePlayers == 0)
	{
		GameServer()->SendBroadcast("double moneytiles have been deactivated by an administrator", m_pPlayer->GetCid(), 0);
		return;
	}
	if(GameServer()->CountIngameHumans() < g_Config.m_SvMinDoubleTilePlayers)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "[%d/%d] players to activate the double moneytile", GameServer()->CountIngameHumans(), g_Config.m_SvMinDoubleTilePlayers);
		GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
		return;
	}
	if(!m_pPlayer->IsLoggedIn())
	{
		GameServer()->SendBroadcast(GameServer()->Loc("You need to be logged in to use moneytiles. \nGet an account with '/register <name> <pw> <pw>'", m_pPlayer->GetCid()), m_pPlayer->GetCid(), 0);
		return;
	}
	if(m_pPlayer->m_QuestState == CPlayer::QUEST_FARM)
	{
		if(m_pPlayer->m_QuestStateLevel < 7) // 10 money
		{
			m_pPlayer->m_QuestProgressValue2++;
			if(m_pPlayer->m_QuestProgressValue2 > m_pPlayer->m_QuestStateLevel)
			{
				GameServer()->QuestAddProgress(m_pPlayer->GetCid(), 10);
				m_pPlayer->m_QuestProgressValue2 = 0;
			}
		}
		else if(m_pPlayer->m_QuestStateLevel == 7)
		{
			// moneytile police
		}
		else if(m_pPlayer->m_QuestStateLevel == 8)
		{
			m_pPlayer->m_QuestProgressValue2++;
			if(m_pPlayer->m_QuestProgressValue2 > 10)
			{
				GameServer()->QuestAddProgress(m_pPlayer->GetCid(), 10);
				m_pPlayer->m_QuestProgressValue2 = 0;
			}
		}
	}
	if(m_pPlayer->IsMaxLevel())
	{
		if(m_pPlayer->m_xpmsg)
		{
			GameServer()->SendBroadcast("You reached the maximum level.", m_pPlayer->GetCid(), 0);
		}
		return;
	}

	int Exp = 0;
	int Money = 0;

	// flag extra xp
	if(GameServer()->m_pController->HasFlag(this) != -1)
	{
		Exp += 2;
	}

	// tile gain and survival bonus
	int Survival = (m_survivexpvalue + 1);
	Exp += 2 * Survival;
	Money += 4;

	// give money & xp
	m_pPlayer->GiveXP(Exp);
	m_pPlayer->MoneyTransaction(Money);
	m_pPlayer->m_MoneyTilesMoney += Money;

	// show msg
	if(m_pPlayer->m_xpmsg)
	{
		// skip if other broadcasts activated:
		if(!m_pPlayer->m_hidejailmsg)
		{
			if(m_pPlayer->m_Account.m_EscapeTime > 0 || m_pPlayer->m_Account.m_JailTime > 0)
			{
				return;
			}
		}

		char aBuf[128];
		char aMoney[128];
		char aXp[128];
		char aLevel[128];
		str_format(aMoney, sizeof(aMoney), "Money [%" PRId64 "] +4", m_pPlayer->GetMoney());
		str_format(aXp, sizeof(aXp), "XP [%" PRId64 "/%" PRId64 "] +2", m_pPlayer->GetXP(), m_pPlayer->GetNeededXP());
		str_format(aLevel, sizeof(aLevel), "%s [%d]", GameServer()->Loc("Level", m_pPlayer->GetCid()), m_pPlayer->GetLevel());

		// xp
		if(GameServer()->m_pController->HasFlag(this) != -1)
			str_append(aXp, " +2 flag", sizeof(aXp));
		if(m_survivexpvalue > 0)
		{
			str_format(aBuf, sizeof(aBuf), " +%d survival", Survival);
			str_append(aXp, aBuf, sizeof(aXp));
		}

		str_format(aBuf, sizeof(aBuf), "%s\n%s\n%s", aMoney, aXp, aLevel);
		GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 0);
	}
}

void CCharacter::MoneyTilePlus()
{
	if(!m_pPlayer->m_MoneyTilePlus)
		return;
	if(IsInDDraceTeam())
		return;
	m_pPlayer->m_MoneyTilePlus = false;

	if(m_pPlayer->IsMaxLevel())
	{
		GameServer()->SendChatTarget(m_pPlayer->GetCid(), "You touched a MoneyTile Plus!  +500money");
	}
	else
	{
		GameServer()->SendChatTarget(m_pPlayer->GetCid(), "You touched a MoneyTile Plus! +2500xp  +500money");
		m_pPlayer->GiveXP(2500);
	}
	if(m_pPlayer->m_xpmsg && m_pPlayer->IsLoggedIn())
	{
		char aBuf[128];
		str_format(
			aBuf,
			sizeof(aBuf),
			"Money [%" PRId64 "]\n"
			"XP [%" PRId64 "/%" PRId64 "]\n"
			"%s [%d]",
			m_pPlayer->GetMoney(),
			m_pPlayer->GetXP(),
			m_pPlayer->GetNeededXP(),
			GameServer()->Loc("Level", m_pPlayer->GetCid()),
			m_pPlayer->GetLevel());
		GameServer()->SendBroadcast(aBuf, m_pPlayer->GetCid(), 1);
	}
	m_pPlayer->MoneyTransaction(+500, "moneytile plus");
}

void CCharacter::SetSpawnWeapons()
{
	if(m_pPlayer->m_Account.m_UseSpawnWeapons && !m_pPlayer->IsInstagibMinigame() && !m_pPlayer->m_IsSurvivaling)
	{
		if(m_pPlayer->m_Account.m_SpawnWeaponShotgun)
		{
			m_Core.m_aWeapons[2].m_Got = true;
			m_Core.m_aWeapons[2].m_Ammo = m_pPlayer->m_Account.m_SpawnWeaponShotgun;
			m_pPlayer->m_SpawnShotgunActive = 1;
		}

		if(m_pPlayer->m_Account.m_SpawnWeaponGrenade)
		{
			m_Core.m_aWeapons[3].m_Got = true;
			m_Core.m_aWeapons[3].m_Ammo = m_pPlayer->m_Account.m_SpawnWeaponGrenade;
			m_pPlayer->m_SpawnGrenadeActive = 1;
		}

		if(m_pPlayer->m_Account.m_SpawnWeaponRifle)
		{
			m_Core.m_aWeapons[4].m_Got = true;
			m_Core.m_aWeapons[4].m_Ammo = m_pPlayer->m_Account.m_SpawnWeaponRifle;
			m_pPlayer->m_SpawnRifleActive = 1;
		}
	}
}
