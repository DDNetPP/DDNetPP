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
    }
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
