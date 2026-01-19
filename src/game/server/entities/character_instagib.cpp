// ddnet++ instagib CCharacter methods
// TODO: this entire file should be deleted
//       sub gametypes have to be fully refactored
//       make it a proper minigame or a own gamecontroller

#include "character.h"

#include <base/system.h>

#include <engine/shared/config.h>

#include <generated/protocol.h>

#include <game/server/gamecontext.h>
#include <game/server/gamemodes/ddnet.h>
#include <game/server/player.h>

void CCharacter::DDPP_TakeDamageInstagib(int Dmg, int From, int Weapon)
{
	if(m_Godmode || (m_pPlayer->m_IsInstaArena_gdm && GameServer()->m_InstaGrenadeRoundEndTickTicker) || (m_pPlayer->m_IsInstaArena_idm && GameServer()->m_InstaRifleRoundEndTickTicker))
	{
		//CHEATER!!
	}
	else
	{
		if(From == m_pPlayer->GetCid())
		{
			m_pPlayer->m_Account.m_GrenadeShotsNoRJ--; //warning also reduce NoRJ shots on close kills
		}

		if(From != m_pPlayer->GetCid() && Dmg >= g_Config.m_SvNeededDamage2NadeKill)
		{
			if(m_pPlayer->m_IsInstaMode_fng || GameServer()->m_apPlayers[From]->m_IsInstaMode_fng)
			{
				if(!m_FreezeTime)
				{
					//char aBuf[256];
					//str_format(aBuf, sizeof(aBuf), "freezetime %d", m_FreezeTime);
					//GameServer()->SendChat(-1, TEAM_ALL, aBuf);
					Freeze(10);
					// on fire mode
					if(g_Config.m_SvOnFireMode == 1)
					{
						if(GameServer()->m_apPlayers[From] && GameServer()->m_apPlayers[From]->GetCharacter())
						{
							// this 200 * tickspeed / 1000
							// evaluates to 10 and matches ddnet-insta sv_on_fire_mode
							GameServer()->m_apPlayers[From]->GetCharacter()->m_ReloadTimer = 200 * Server()->TickSpeed() / 1000;
						}
					}
				}
				else
				{
					//GameServer()->SendChat(-1, TEAM_ALL, "returned cuz freeze time");
					//return false; //dont count freezed tee shots (no score or sound or happy emote)
					//dont return because we loose hammer vel then
					return; //we can return again because the instagib stuff has his own func and got moved out of TakeDamage();
				}
			}
			else
			{
				Die(From, Weapon);
			}

			//do scoring (by ChillerDragon)
			if(g_Config.m_SvInstagibMode)
			{
				GameServer()->m_apPlayers[From]->m_Minigame.m_Score++;
			}
			GameServer()->DoInstaScore(1, From);

			//save the kill
			//if (!m_pPlayer->m_IsInstaArena_fng) //damage is only a hit not a kill in insta ---> well move it completely al to kill makes more performance sense
			//{
			//	if (g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2 || GameServer()->m_apPlayers[From]->m_IsInstaArena_gdm) //gdm & zCatch grenade
			//	{
			//		GameServer()->m_apPlayers[From]->m_Account.m_GrenadeKills++;
			//	}
			//	else if (g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4 || GameServer()->m_apPlayers[From]->m_IsInstaArena_idm) // idm & zCatch rifle
			//	{
			//		GameServer()->m_apPlayers[From]->m_Account.m_RifleKills++;
			//	}
			//}

			//killingspree system by toast stolen from twf (shit af xd(has crashbug too if a killingspreeeer gets killed))
			//GameServer()->m_apPlayers[From]->m_KillStreak++;
			//char aBuf[256];
			//str_format(aBuf, sizeof(aBuf), "%s's Killingspree was ended by %s (%d Kills)", Server()->ClientName(m_pPlayer->GetCid()), Server()->ClientName(GameServer()->m_apPlayers[From]->GetCid()), m_pPlayer->m_KillStreak);
			//if (m_pPlayer->m_KillStreak >= 5)
			//{
			//	GameServer()->SendChat(-1, TEAM_ALL, aBuf);
			//	GameServer()->CreateExplosion(m_pPlayer->GetCharacter()->m_Pos, m_pPlayer->GetCid(), WEAPON_GRENADE, false, 0, m_pPlayer->GetCharacter()->Teams()->TeamMask(0));
			//}
			//m_pPlayer->m_KillStreak = 0;
			//char m_SpreeMsg[10][100] = { "on a killing spree", "on a rampage", "dominating", "unstoppable", "godlike", "prolike", "cheating", "the master","the best","imba" };
			//int iBuf = ((GameServer()->m_apPlayers[From]->m_KillStreak / 5) - 1) % 10;
			//str_format(aBuf, sizeof(aBuf), "%s is %s with %d Kills!", Server()->ClientName(GameServer()->m_apPlayers[From]->GetCid()), m_SpreeMsg[iBuf], GameServer()->m_apPlayers[From]->m_KillStreak);
			//if (m_pPlayer->m_KillStreak % 5 == 0 && GameServer()->m_apPlayers[From]->m_KillStreak >= 5)
			//	GameServer()->SendChat(-1, TEAM_ALL, aBuf);

			// set attacker's face to happy (taunt!)
			if(From >= 0 && From != m_pPlayer->GetCid() && GameServer()->m_apPlayers[From])
			{
				CCharacter *pChr = GameServer()->m_apPlayers[From]->GetCharacter();
				if(pChr)
				{
					pChr->m_EmoteType = EMOTE_HAPPY;
					pChr->m_EmoteStop = Server()->Tick() + Server()->TickSpeed();
				}
			}

			// do damage Hit sound
			if(From >= 0 && From != m_pPlayer->GetCid() && GameServer()->m_apPlayers[From])
			{
				// int64_t Mask = CmaskOne(From);
				// for(int i = 0; i < MAX_CLIENTS; i++)
				// {
				// 	if(GameServer()->m_apPlayers[i] && GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS && GameServer()->m_apPlayers[i]->m_SpectatorId == From)
				// 		Mask |= CmaskOne(i);
				// }
				// TODO: this was done to fix build after merge and is untested
				GameServer()->CreateSound(GameServer()->m_apPlayers[From]->m_ViewPos, SOUND_HIT, TeamMask());
			}

			//if zCatch mode --> move to spec
			if(g_Config.m_SvInstagibMode == 2 || g_Config.m_SvInstagibMode == 4) //grenade and rifle zCatch
			{
				if(From != m_pPlayer->GetCid())
				{
					m_pPlayer->SetTeam(-1, false);
				}

				//Save The Player in catch array
				GameServer()->m_apPlayers[From]->m_aCaughtId[m_pPlayer->GetCid()] = 1;
			}
		}
	}
}

void CCharacter::InstagibSubDieFunc(int Killer, int Weapon)
{
	//=== DEATHS ===
	if(g_Config.m_SvInstagibMode)
	{
		if(g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2) //gdm & zCatch grenade
		{
			m_pPlayer->m_Account.m_GrenadeDeaths++;
		}
		else if(g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4) // idm & zCatch rifle
		{
			m_pPlayer->m_Account.m_RifleDeaths++;
		}

		InstagibKillingSpree(Killer, Weapon);
	}
	else if(m_pPlayer->m_IsInstaArena_gdm)
	{
		m_pPlayer->m_Account.m_GrenadeDeaths++;
	}
	else if(m_pPlayer->m_IsInstaArena_idm)
	{
		m_pPlayer->m_Account.m_RifleDeaths++;
	}

	//=== KILLS ===
	if(GameServer()->m_apPlayers[Killer])
	{
		if(g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2 || GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm) //gdm & zCatch grenade
		{
			GameServer()->m_apPlayers[Killer]->m_Account.m_GrenadeKills++;
		}
		else if(g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4 || GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm) // idm & zCatch rifle
		{
			GameServer()->m_apPlayers[Killer]->m_Account.m_RifleKills++;
		}

		//=== MULTIS (credit to noby) ===
		if(Killer != m_pPlayer->GetCid()) // don't count selkills as multi
		{
			time_t ttmp = time(NULL);
			if((ttmp - GameServer()->m_apPlayers[Killer]->m_lastkilltime) <= 5)
			{
				GameServer()->m_apPlayers[Killer]->m_multi++;
				if(GameServer()->m_apPlayers[Killer]->m_max_multi < GameServer()->m_apPlayers[Killer]->m_multi)
				{
					GameServer()->m_apPlayers[Killer]->m_max_multi = GameServer()->m_apPlayers[Killer]->m_multi;
				}
				char aBuf[128];
				if(GameServer()->IsDDPPgametype("fng"))
				{
					str_format(aBuf, sizeof(aBuf), "'%s' multi x%d!", Server()->ClientName(Killer), GameServer()->m_apPlayers[Killer]->m_multi);
					GameServer()->SendChat(-1, TEAM_ALL, aBuf);
				}
				if(GameServer()->m_apPlayers[Killer]->m_IsInstaArena_fng)
				{
					if(GameServer()->m_apPlayers[Killer]->m_IsInstaArena_gdm) // grenade
					{
						str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' multi x%d!", Server()->ClientName(Killer), GameServer()->m_apPlayers[Killer]->m_multi);
						GameServer()->SendChatInsta(aBuf, WEAPON_GRENADE);
					}
					else if(GameServer()->m_apPlayers[Killer]->m_IsInstaArena_idm) // rifle
					{
						str_format(aBuf, sizeof(aBuf), "[INSTA] '%s' multi x%d!", Server()->ClientName(Killer), GameServer()->m_apPlayers[Killer]->m_multi);
						GameServer()->SendChatInsta(aBuf, WEAPON_LASER);
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

void CCharacter::InstagibKillingSpree(int KillerId, int Weapon)
{
	char aBuf[128];

	//killingspree system by FruchtiHD and ChillerDragon stolen from twlevel (edited by ChillerDragon)
	CCharacter *pVictim = m_pPlayer->GetCharacter();
	CPlayer *pKiller = GameServer()->m_apPlayers[KillerId];
	if(GameServer()->CountConnectedPlayers() >= g_Config.m_SvSpreePlayers) //only count killing sprees if enough players are online (also counting spectators)
	{
		if(pVictim && pKiller)
		{
			if(Weapon == WEAPON_GAME)
			{
				if(pVictim->GetPlayer()->m_KillStreak >= 5)
				{
					//Check for new highscore
					if(g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2) //gdm & zCatch grenade
					{
						//dbg_msg("insta", "checking for highscore grenade");
						if(pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_Account.m_GrenadeSpree)
						{
							pVictim->GetPlayer()->m_Account.m_GrenadeSpree = pVictim->GetPlayer()->m_KillStreak;
							GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCid(), "New grenade Killingspree record!");
						}
						//str_format(aBuf, sizeof(aBuf), "last: %d top: %d", pVictim->GetPlayer()->m_KillStreak, pVictim->GetPlayer()->m_Account.m_GrenadeSpree);
						//dbg_msg("insta", aBuf);
					}
					else if(g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4) // idm & zCatch rifle
					{
						//dbg_msg("insta", "checking for highscore rifle");
						if(pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_Account.m_RifleSpree)
						{
							pVictim->GetPlayer()->m_Account.m_RifleSpree = pVictim->GetPlayer()->m_KillStreak;
							GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCid(), "New rifle Killingspree record!");
						}
						//str_format(aBuf, sizeof(aBuf), "last: %d top: %d", pVictim->GetPlayer()->m_KillStreak, pVictim->GetPlayer()->m_Account.m_GrenadeSpree);
						//dbg_msg("insta", aBuf);
					}

					GameServer()->SendEndSpreeMessage(
						pVictim->GetPlayer()->GetCid(),
						pVictim->GetPlayer()->m_KillStreak,
						Server()->ClientName(pVictim->GetPlayer()->GetCid()));
					pVictim->GetPlayer()->m_KillStreak = 0;
					GameServer()->CreateExplosion(pVictim->m_Pos, m_pPlayer->GetCid(), WEAPON_GRENADE, true, 0, m_pPlayer->GetCharacter()->Teams()->TeamMask(0));
				}
			}

			if(pVictim->GetPlayer()->m_KillStreak >= 5)
			{
				//Check for new highscore
				if(g_Config.m_SvInstagibMode == 1 || g_Config.m_SvInstagibMode == 2) //gdm & zCatch grenade
				{
					//dbg_msg("insta", "checking for highscore grenade");
					if(pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_Account.m_GrenadeSpree)
					{
						pVictim->GetPlayer()->m_Account.m_GrenadeSpree = pVictim->GetPlayer()->m_KillStreak;
						GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCid(), "New grenade Killingspree record!");
					}
					//str_format(aBuf, sizeof(aBuf), "last: %d top: %d", pVictim->GetPlayer()->m_KillStreak, pVictim->GetPlayer()->m_Account.m_GrenadeSpree);
					//dbg_msg("insta", aBuf);
				}
				else if(g_Config.m_SvInstagibMode == 3 || g_Config.m_SvInstagibMode == 4) // idm & zCatch rifle
				{
					//dbg_msg("insta", "checking for highscore rifle");
					if(pVictim->GetPlayer()->m_KillStreak > pVictim->GetPlayer()->m_Account.m_RifleSpree)
					{
						pVictim->GetPlayer()->m_Account.m_RifleSpree = pVictim->GetPlayer()->m_KillStreak;
						GameServer()->SendChatTarget(pVictim->GetPlayer()->GetCid(), "New rifle Killingspree record!");
					}
					//str_format(aBuf, sizeof(aBuf), "last: %d top: %d", pVictim->GetPlayer()->m_KillStreak, pVictim->GetPlayer()->m_Account.m_GrenadeSpree);
					//dbg_msg("insta", aBuf);
				}

				GameServer()->SendEndSpreeMessage(
					pVictim->GetPlayer()->GetCid(),
					pVictim->GetPlayer()->m_KillStreak,
					Server()->ClientName(pKiller->GetCid())); // FIXME: what happens if the killer left?
				pVictim->GetPlayer()->m_KillStreak = 0;
				GameServer()->CreateExplosion(pVictim->m_Pos, m_pPlayer->GetCid(), WEAPON_GRENADE, true, 0, m_pPlayer->GetCharacter()->Teams()->TeamMask(0));
			}

			if(pKiller != pVictim->GetPlayer())
			{
				if(!pVictim->GetPlayer()->m_IsDummy || pKiller->m_IsDummy)
				{
					pKiller->m_KillStreak++;
				}
				pVictim->GetPlayer()->m_KillStreak = 0;
				if(pKiller->m_KillStreak % 5 == 0 && pKiller->m_KillStreak >= 5)
					GameServer()->SendSpreeMessage(pKiller->GetCid(), pKiller->m_KillStreak);

				//Finish time if cfg val reached
				if(pKiller->m_KillStreak == g_Config.m_SvKillsToFinish && g_Config.m_SvInstagibMode) //only finish if sv_insta is on... needed for the future if we activate this killsys in ddrace mode (sv_insta 0) to dont fuck up race scores
				{
					CGameControllerDDNet *Controller = (CGameControllerDDNet *)GameServer()->m_pController;
					Controller->Teams().OnCharacterFinish(pKiller->GetCid());
				}
			}
		}
	}
	else //not enough players
	{
		//str_format(aBuf, sizeof(aBuf), "not enough tees %d/%d spree (%d)", GameServer()->CountConnectedPlayers(), g_Config.m_SvSpreePlayers, pKiller->m_KillStreak);
		//dbg_msg("insta", aBuf);
		if(pKiller != pVictim->GetPlayer())
		{
			if(!pVictim->GetPlayer()->m_IsDummy || pKiller->m_IsDummy)
			{
				pKiller->m_KillStreak++;
			}

			pVictim->GetPlayer()->m_KillStreak = 0;
			if(pKiller->m_KillStreak == 5)
			{
				str_format(aBuf, sizeof(aBuf), GameServer()->Loc("%d players needed to start a spree.", pKiller->GetCid()), g_Config.m_SvSpreePlayers);
				GameServer()->SendChatTarget(pKiller->GetCid(), aBuf);
				pKiller->m_KillStreak = 0; //reset killstreak to avoid some1 collecting 100 kills with dummy and then if player connect he could save the spree
			}
		}
	}
	if(pVictim && pVictim->GetPlayer())
		pVictim->GetPlayer()->m_KillStreak = 0; //Important always clear killingspree of dead dude
}
