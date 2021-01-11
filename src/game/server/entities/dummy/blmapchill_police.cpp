// made by fokkonaut and ChillerDragon

#include "blmapchill_police.h"

#include "../character.h"
#include <game/server/player.h>
#include <engine/shared/config.h>
#include <game/server/gamecontext.h>

CDummyBlmapChillPolice::CDummyBlmapChillPolice(class CCharacter *pChr, class CPlayer *pPlayer)
: CDummyBase(pChr, pPlayer)
{
	m_LovedX = 0;
	m_LovedY = 0;
	m_LowerPanic = 0;
	m_Speed = 70; // fix for crashbug (DONT REMOVE)
	m_HelpMode = 0;
	m_GrenadeJump = 0;
	m_SpawnTeleporter = 0;

	m_IsHelpHook = false;
	m_IsClosestPolice = false;
	m_DidRocketjump = false;
	m_HasTouchedGround = false;
	m_HasAlreadyBeenHere = false;
	m_HasStartGrenade = false;
	m_IsDJUsed = false;
	m_HashReachedCinemaEntrance = false;
}

void CDummyBlmapChillPolice::OnTick()
{
    m_pChr->ResetInput();
    Input()->m_Hook = 0;
    if (GetPos().x > 451 * 32 && GetPos().x < 472 * 32 && GetPos().y > 74 * 32 && GetPos().y < 85 * 32) // new spawn area, walk into the left SPAWN teleporter
    {
        Input()->m_Direction = -1;
        // jump into tele on spawn or jump onto edge after getting 5 jumps
        if (GetPos().x > 454 * 32 && GetPos().x < 463 * 32 + 2) // left side of new spawn area
        {
            Input()->m_Jump = 1;
            if (Server()->Tick() % 10 == 0)
                Input()->m_Jump = 0;
        }
    }
    else if (GetPos().x < 240 * 32 && GetPos().y < 36 * 32) // the complete zone in the map intselfs. its for resetting the dummy when he is back in spawn using tp
    {
        if (m_pChr->isFreezed && GetPos().x > 32 * 32)
        {
            if (Server()->Tick() % 60 == 0)
                GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3); // tear emote before killing
            if (Server()->Tick() % 500 == 0 && IsGrounded()) //kill when freeze
            {
                Die();
                return;
            }
        }
        if (GetPos().x < 24 * 32 && GetPos().y < 14 * 32 && GetPos().x > 23 * 32) // looking for tp and setting different aims for the swing
            m_SpawnTeleporter = 1;
        if (GetPos().x < 25 * 32 && GetPos().y < 14 * 32 && GetPos().x > 24 * 32) // looking for tp and setting different aims for the swing
            m_SpawnTeleporter = 2;
        if (GetPos().x < 26 * 32 && GetPos().y < 14 * 32 && GetPos().x > 25 * 32) // looking for tp and setting different aims for the swing
            m_SpawnTeleporter = 3;
        if (GetPos().y > 25 * 32 && GetPos().x > 43 * 32 && GetPos().y < 35 * 32) // kill
        {
            Die();
            return;
        }
        if (GetPos().y > 35 * 32 && GetPos().x < 43 * 32) // area bottom right from spawn, if he fall, he will kill
        {
            Die();
            return;
        }
        if (GetPos().x < 16 * 32) // area left of old spawn, he will kill too
        {
            Die();
            return;
        }
        else if (GetPos().y > 25 * 32) // after unfreeze hold hook to the right and walk right.
        {
            Input()->m_TargetX = 100;
            Input()->m_TargetY = 1;
            Input()->m_Direction = 1;
            Input()->m_Hook = 1;
            if (GetPos().x > 25 * 32 && GetPos().x < 33 * 32 && GetPos().y > 30 * 32) //set weapon and aim down
            {
                if (Server()->Tick() % 5 == 0)
                {
                    SetWeapon(3);
                }
                if (m_SpawnTeleporter == 1)
                {
                    Input()->m_TargetX = 190;
                    Input()->m_TargetY = 100;
                    LatestInput()->m_TargetX = 190;
                    LatestInput()->m_TargetY = 100;
                }
                else if (m_SpawnTeleporter == 2)
                {
                    Input()->m_TargetX = 205;
                    Input()->m_TargetY = 100;
                    LatestInput()->m_TargetX = 205;
                    LatestInput()->m_TargetY = 100;
                }
                else if (m_SpawnTeleporter == 3)
                {
                    Input()->m_TargetX = 190;
                    Input()->m_TargetY = 100;
                    LatestInput()->m_TargetX = 190;
                    LatestInput()->m_TargetY = 100;
                }
                if (GetPos().x > 31 * 32)
                {
                    Input()->m_Jump = 1;
                    Fire();
                }
            }
        }
        else if (GetPos().x > 33 * 32 && GetPos().x < 50 * 32 && GetPos().y > 18 * 32)
        {
            Input()->m_Direction = 1;
            if (GetPos().x > 33 * 32 && GetPos().x < 42 * 32 && GetPos().y > 20 * 32 && GetPos().y < 25 * 32)
            {
                GameServer()->SendEmoticon(m_pPlayer->GetCID(), 14); //happy emote when successfully did the grenaede jump
            }
            if (GetPos().x < 50 * 32)
            {
                Input()->m_Jump = 1;
                if (Server()->Tick() % 20 == 0)
                    Input()->m_Jump = 0;
                if (GetPos().y < 19 * 32 && m_pChr->m_FreezeTime == 0)
                    Fire();
                Input()->m_TargetX = -200;
                Input()->m_TargetY = 200;
            }
            // don't walk into freeze
            if (GetPos().x > 47 * 32 && GetPos().x < 50 * 32 && GetPos().y > 17 * 32)
            {
                Input()->m_Direction = -1;
            }
        }
        else if (GetPos().y < 16 * 32 && GetPos().x < 75 * 32 && GetPos().x > 40 * 32) // walking right on it
        {
            Input()->m_Direction = 1;
            Input()->m_Jump = 0;
            if (GetPos().y > 10 * 32 && GetPos().x > 55 * 32 && GetPos().x < 56 * 32) //jumping over gap
                Input()->m_Jump = 1;
        }
        if (GetPos().y > 15 * 32 && GetPos().x > 55 * 32 && GetPos().x < 65 * 32)
        {
            Input()->m_Direction = 1;
            if (GetPos().x > 63 * 32)
                Input()->m_Jump = 1;
        }
        // fallen into lower right area of the top block area before gores
        if (GetPos().y > 20 * 32 && GetPos().y < 25 * 32)
        {
            Input()->m_TargetX = GetPos().x > 67 * 32 ? -20 : 20;
            Input()->m_TargetY = 100;
            if (GetPos().x > 69 * 32)
                Input()->m_Direction = -1;
            if (IsGrounded())
                Input()->m_Jump = 1;
            if (GetPos().y < 23 * 32 + 20 && GetVel().y < -1.1f)
                Fire();
        }
        else if (GetPos().x > 75 * 32 && GetPos().x < 135 * 32) //gores stuff (the thign with freeze spikes from top and bottom)
        {
            Input()->m_Jump = 0;
            Input()->m_Direction = 1;
            // start jump into gores
            if (GetPos().x > 77 * 32 && GetPos().y > 13 * 32 && GetPos().x < 80 * 32)
                Input()->m_Jump = 1;
            // nade boost on roof
            if ((GetPos().x > 80 * 32 && GetPos().x < 90 * 32) ||
                (GetPos().x > 104 * 32 && GetPos().x < 117 * 32))
            {
                if (GetPos().y < 10 * 32)
                    Fire();
                Input()->m_TargetX = GetVel().x > 12.0f ? -10 : -100;
                Input()->m_TargetY = -180;
            }
            if (GetPos().x > 92 * 32 && GetPos().y > 12.5 * 32) // hooking stuff from now on
            {
                Input()->m_TargetX = 100;
                Input()->m_TargetY = -100;
                Input()->m_Hook = 1;
                if (GetPos().y < 14 * 32 && GetPos().x > 100 * 32 && GetPos().x < 110 * 32)
                    Input()->m_Hook = 0;
            }
            // Don't swing into roof when boost worked
            if (GetPos().x > 127 * 32 && GetPos().x < 138 * 32 && GetVel().x > 12.f && GetVel().y < -1.5)
                Input()->m_Hook = 0;
            if (GetPos().x > 120 * 32 && GetPos().y < 13 * 32)
                Input()->m_Hook = 0;
        }
        else if (GetPos().x > 135 * 32)
        {
            Input()->m_Direction = 1;
            // gores (the long way to 5 jumps)
            if (GetPos().x < 220 * 32)
            {
                if ((GetPos().y > 12 * 32 + 10 && GetVel().y > 4.1f) ||
                    (GetPos().y > 12 * 32 + 30 && GetVel().y > -1.1f))
                {
                    if (HookState() == HOOK_FLYING || HookState() == HOOK_GRABBED)
                    {
                        Input()->m_TargetX = -100;
                        Input()->m_TargetY = 100;
                        Fire();
                    }
                    else
                    {
                        Input()->m_TargetX = 100;
                        Input()->m_TargetY = -200;
                    }
                    Input()->m_Hook = 1;
                    if (GetPos().y < 12 * 32)
                    {
                        Input()->m_Hook = 0;
                    }
                    if (GetPos().x > 212 * 32)
                    {
                        Input()->m_Hook = 1;
                        Input()->m_TargetX = 100;
                        Input()->m_TargetY = -75;
                        Input()->m_Jump = 1;
                    }
                }
            }
            // 5 jumps area
            if (GetPos().x > 222 * 32)
            {
                Input()->m_Jump = 0;
                if (Jumps() < 5)
                {
                    if (GetPos().x > 227 * 32)
                        Input()->m_Direction = -1;
                    else
                        Input()->m_Direction = 1;
                }
                else // got 5 jumps go into tele
                {
                    Input()->m_TargetX = -200;
                    Input()->m_TargetY = 150;
                    if (GetPos().x < 229 * 32)
                    {
                        if (m_pChr->m_FreezeTime == 0)
                            Fire();
                    }
                    else // on right side of platform (do rj here)
                    {
                        if (IsGrounded() && !m_pChr->GetReloadTimer())
                        {
                            if (GetPos().x > 231 * 32)
                            {
                                m_DidRocketjump = true;
                                Fire();
                            }
                        }
                        else
                        {
                            if (GetPos().x > 229 * 32 + 10 && !m_DidRocketjump)
                                Input()->m_Direction = -1;
                        }
                    }
                }
            }
        }
    }
    if (m_pChr->isFreezed && GetPos().y < 410 * 32) // kills when in freeze and not in policebase
    {
        if (Server()->Tick() % 60 == 0)
            GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3); // tear emote before killing
        if (Server()->Tick() % 500 == 0 && IsGrounded()) // kill when freeze
        {
            Die();
            return;
        }
    }
    if (m_pChr->isFreezed && GetPos().x < 41 * 32 && GetPos().x > 33 * 32 && GetPos().y < 10 * 32) // kills when on speedup right next to the newtee spawn to prevent infinite flappy blocking
    {
        if (Server()->Tick() % 500 == 0) // kill when freeze
        {
            Die();
            return;
        }
    }
    // new spawn going left and hopping over the gap under the CFRM.
    // (the jump over the freeze gap before falling down is not here, its in line 38 (search for comment 'jump onto edge after getting 5 jumps'))
    if (m_GrenadeJump == 4 || (GetPos().x > 368 * 32 && GetPos().y < 340 * 32))
    {
        // change to gun
        if (Server()->Tick() % 3 == 0 && GetPos().x > 497 * 32)
            SetWeapon(1);
        Input()->m_Direction = -1;
        if (GetPos().x > 509 * 32 && GetPos().y > 62 * 32) // if bot gets under the table he will go right and jump out of the gap under the table
        {
            Input()->m_Direction = 1;
            if (GetPos().x > 511.5 * 32)
            {
                if (Server()->Tick() % 10 == 0)
                    Input()->m_Jump = 1;
            }
        }
        // jump over chairs
        else if (Server()->Tick() % 10 == 0 && GetPos().x > 505 * 32)
            Input()->m_Jump = 1;
        // jump out of the chair room
        if (GetPos().x < 497 * 32 && GetPos().x > 496 * 32)
            Input()->m_Jump = 1;
        // fallen too low backup jump
        if (GetPos().x > 469 * 32 && GetPos().y > 74 * 32)
        {
            Input()->m_Jump = 1;
            if (Server()->Tick() % 15 == 0)
                Input()->m_Jump = 0;
        }

        // above new spawn about to jump through freeze
        if (GetPos().y < 75 * 32)
        {
            // Slow down to hit ground before jumping through freeze
            if (GetPos().x > 465 * 32 && GetPos().x < 469 * 32)
            {
                if (!IsGrounded())
                    Input()->m_Direction = 1;
            }
            // Too slow to jump through freeze -> go back get speed
            if (GetPos().x > 455 * 32)
            {
                if (GetPos().x < 461 * 32 && GetVel().x > -9)
                    m_GetSpeed = true;
                if (GetPos().x > 465 * 32 || GetVel().x < -10)
                    m_GetSpeed = false;
                if (m_GetSpeed)
                {
                    Input()->m_Direction = 1;
                    Input()->m_TargetX = 200;
                    Input()->m_TargetY = 100;
                    Input()->m_Hook = 1;
                    if (Server()->Tick() % 15 == 0)
                        Input()->m_Hook = 0;
                }
            }
        }

        // rocket jump from new spawn edge to old map entry
        if (GetPos().x < 453 * 32 && GetPos().y < 80 * 32)
        {
            Input()->m_Direction = 0;
            if (Server()->Tick() % 10 == 0) // change to grenade
                SetWeapon(3);

            if (!m_pChr->m_FreezeTime && IsGrounded() && m_GrenadeJump == 0) // shoot up
            {
                Input()->m_Jump = 1;
                Input()->m_TargetX = 1;
                Input()->m_TargetY = -100;
                LatestInput()->m_TargetX = 1;
                LatestInput()->m_TargetY = -100;
                Fire();
                m_GrenadeJump = 1;
            }
            else if (GetVel().y > -7.6f && m_GrenadeJump == 1) // jump in air // basically a timer for when the grenade comes down
            {
                Input()->m_Jump = 1;
                m_GrenadeJump = 2;
            }
            if (m_GrenadeJump == 2 || m_GrenadeJump == 3) // double grenade
            {
                if (IsGrounded())
                    Input()->m_Direction = -1;
                if (GetVel().y < 0.09f && GetVel().x < -0.1f)
                {
                    Input()->m_Jump = 1;
                    Input()->m_TargetX = 100;
                    Input()->m_TargetY = 150;
                    LatestInput()->m_TargetX = 100;
                    LatestInput()->m_TargetY = 150;
                    Fire();
                    m_GrenadeJump = 4;
                }
            }
            if (m_GrenadeJump == 4)
            {
                Input()->m_Direction = -1;
                if (GetVel().y > 4.4f)
                    Input()->m_Jump = 1;
                if (GetPos().y > 85 * 32)
                    m_GrenadeJump = 0; // something went wrong abort and try fallback grenade jump
            }
        }
        else // Reset rj vars for fallback grenade jump and other reuse
        {
            m_GrenadeJump = 0;
        }
    }
    if (GetPos().x > 370 * 32 && GetPos().y < 340 * 32 && GetPos().y > 310 * 32) // bottom going left to the grenade jump
    {
        Input()->m_Direction = -1;
        if (GetPos().x < 422 * 32 && GetPos().x > 421 * 32) // bottom jump over the hole to police station
            Input()->m_Jump = 1;
        if (GetPos().x < 406 * 32 && GetPos().x > 405 * 32) // using 5jump from now on
            Input()->m_Jump = 1;
        if (GetPos().x < 397 * 32 && GetPos().x > 396 * 32)
            Input()->m_Jump = 1;
        if (GetPos().x < 387 * 32 && GetPos().x > 386 * 32)
            Input()->m_Jump = 1;
        if (GetPos().x < 377 * 32 && GetPos().x > 376 * 32) // last jump from the 5 jump
            Input()->m_Jump = 1;
        if (GetPos().y > 339 * 32) // if he falls into the hole to police station he will kill
        {
            Die();
            return;
        }
    }
    else if (GetPos().y > 296 * 32 && GetPos().x < 370 * 32 && GetPos().x > 350 * 32 && GetPos().y < 418 * 32) // getting up to the grenade jump part
    {
        if (IsGrounded())
        {
            m_HashReachedCinemaEntrance = true;
            Input()->m_Jump = 1;
        }
        else if (GetPos().y < 313 * 32 && GetPos().y > 312 * 32 && GetPos().x < 367 * 32)
            Input()->m_Direction = 1;
        else if (GetPos().x > 367 * 32)
            Input()->m_Direction = -1;
        if (!m_HashReachedCinemaEntrance && GetPos().x < 370 * 32)
            Input()->m_Direction = 0;
        if (GetVel().y > 0.0000001f && GetPos().y < 310 * 32)
            Input()->m_Jump = 1;
    }
    else if (GetVel().y > 0.001f && GetPos().y < 293 * 32 && GetPos().x > 366.4 * 32 && GetPos().x < 370 * 32)
    {
        Input()->m_Direction = -1;
        if (Server()->Tick() % 1 == 0)
            SetWeapon(3);
    }
    else if (GetPos().x > 325 * 32 && GetPos().x < 366 * 32 && GetPos().y < 295 * 32 && GetPos().y > 59 * 32) // insane grenade jump
    {
        if (IsGrounded() && m_GrenadeJump == 0) // shoot up
        {
            Input()->m_Jump = 1;
            Input()->m_TargetX = 0;
            Input()->m_TargetY = -100;
            LatestInput()->m_TargetX = 0;
            LatestInput()->m_TargetY = -100;
            Fire();
            m_GrenadeJump = 1;
        }
        else if (GetVel().y > -7.6f && m_GrenadeJump == 1) // jump in air // basically a timer for when the grenade comes down
        {
            Input()->m_Jump = 1;
            m_GrenadeJump = 2;
        }
        if (m_GrenadeJump == 2 || m_GrenadeJump == 3) // double grenade
        {
            if (GetPos().y > 58 * 32)
            {
                if (IsGrounded())
                    m_HasTouchedGround = true;
                if (m_HasTouchedGround == true)
                    Input()->m_Direction = -1;
                if (GetVel().y > 0.1f && IsGrounded())
                {
                    Input()->m_Jump = 1;
                    Input()->m_TargetX = 100;
                    Input()->m_TargetY = 150;
                    LatestInput()->m_TargetX = 100;
                    LatestInput()->m_TargetY = 150;
                    Fire();
                    m_GrenadeJump = 3;
                }
                if (m_GrenadeJump == 3)
                {
                    if (GetPos().x < 344 * 32 && GetPos().x > 343 * 32 && GetPos().y > 250 * 32) // air grenade for double wall grnade
                    {
                        Input()->m_TargetX = -100;
                        Input()->m_TargetY = -100;
                        LatestInput()->m_TargetX = -100;
                        LatestInput()->m_TargetY = -100;
                        Fire();
                    }
                }
            }
        }
        if (GetPos().x < 330 * 32 && GetVel().x == 0.0f && GetPos().y > 59 * 32) // if on wall jump and shoot
        {
            if (GetPos().y > 250 * 32 && GetVel().y > 6.0f)
            {
                Input()->m_Jump = 1;
                m_HasStartGrenade = true;
            }
            if (m_HasStartGrenade == true)
            {
                Input()->m_TargetX = -100;
                Input()->m_TargetY = 170;
                LatestInput()->m_TargetX = -100;
                LatestInput()->m_TargetY = 170;
                Fire();
            }
            if (GetPos().y < 130 * 32 && GetPos().y > 131 * 32)
                Input()->m_Jump = 1;
            if (GetVel().y > 0.001f && GetPos().y < 150 * 32)
                m_HasStartGrenade = false;
            if (GetVel().y > 2.0f && GetPos().y < 150 * 32)
            {
                Input()->m_Jump = 1;
                m_HasStartGrenade = true;
            }
        }
    }
    if (GetPos().y < 60 * 32 && GetPos().x < 337 * 32 && GetPos().x > 325 * 32 && GetPos().y > 53 * 32) // top of the grenade jump // shoot left to get to the right 
    {
        Input()->m_Direction = 1;
        Input()->m_TargetX = -100;
        Input()->m_TargetY = 0;
        LatestInput()->m_TargetX = -100;
        LatestInput()->m_TargetY = 0;
        Fire();
        Input()->m_Jump = 0;
        if (GetPos().x > 333 * 32 && GetPos().x < 335 * 32) // hook up and get into the tunnel thingy
        {
            Input()->m_Jump = 1;
            if (GetPos().y < 55 * 32)
                Input()->m_Direction = 1;
        }
    }
    if (GetPos().x > 337 * 32 && GetPos().x < 400 * 32 && GetPos().y < 60 * 32 && GetPos().y > 40 * 32) // hook thru the hookthru
    {
        Input()->m_TargetX = 0;
        Input()->m_TargetY = -1;
        LatestInput()->m_TargetX = 0;
        LatestInput()->m_TargetY = -1;
        Input()->m_Hook = 1;
    }
    if (GetPos().x > 339.5 * 32 && GetPos().x < 345 * 32 && GetPos().y < 51 * 32)
        Input()->m_Direction = -1;
    if (GetPos().x < 339 * 32 && GetPos().x > 315 * 32 && GetPos().y > 40 * 32 && GetPos().y < 53 * 32) // top of grenade jump the thing to go through the wall
    {
        Input()->m_Hook = 0;
        Input()->m_TargetX = 100;
        Input()->m_TargetY = 50;
        LatestInput()->m_TargetX = 100;
        LatestInput()->m_TargetY = 50;
        if (m_HasAlreadyBeenHere == false)
        {
            if (GetPos().x < 339 * 32)
            {
                Input()->m_Direction = 1;
                if (GetPos().x > 338 * 32 && GetPos().x < 339 * 32 && GetPos().y > 51 * 32)
                    m_HasAlreadyBeenHere = true;
            }
        }
        if (m_HasAlreadyBeenHere == true) //using grenade to get throug the freeze in this tunnel thingy
        {
            Input()->m_Direction = -1;
            if (GetPos().x < 338 * 32)
                Fire();
        }
        if (GetPos().x < 328 * 32 && GetPos().y < 60 * 32)
            Input()->m_Jump = 1;
    }
    // Stuck on the outside of the clu spike thing
    else if (GetPos().y > 120 * 32 && GetPos().y < 185 * 32 && GetPos().x > 233 * 32 && GetPos().x < 300 * 32)
    {
        if (GetPos().x < 272 * 32)
            Input()->m_Direction = 1;
        /*
        ##    <- deep and spikes and clu skip
        #
   ######
   ######
   ##      <-- stuck here
   ######
   ######
        #
        #  <-- or stuck here
        ##
         #    police entrance
    ######      |
    #           v
        */
    }
    // jumping over the big freeze to get into the tunnel
    else if (GetPos().y > 260 * 32 && GetPos().x < 325 * 32 && GetPos().y < 328 * 32 && GetPos().x > 275 * 32)
    {
        Input()->m_Direction = -1;
        Input()->m_Jump = 0;
        if (GetPos().y > 280 * 32 && GetPos().y < 285 * 32)
            Input()->m_Jump = 1;
        if (Server()->Tick() % 5 == 0)
            SetWeapon(1);
    }
    // after grenade jump and being down going into the tunnel to police staion
    else if (GetPos().y > 328 * 32 && GetPos().y < 345 * 32 && GetPos().x > 236 * 32 && GetPos().x < 365 * 32)
    {
        Input()->m_Direction = 1;
        if (GetPos().x > 265 * 32 && GetPos().x < 267 * 32)
            Input()->m_Jump = 1;
        if (GetPos().x > 282 * 32 && GetPos().x < 284 * 32)
            Input()->m_Jump = 1;
    }
    if (GetPos().x > 294 * 32 && GetPos().x < 297 * 32 && GetPos().y > 343 * 32 && GetPos().y < 345 * 32) // fix someone blocking flappy, he would just keep moving left into the wall and do nothing there
        Input()->m_Direction = 1;
    else if (GetPos().y > 337.4 * 32 && GetPos().y < 345 * 32 && GetPos().x > 295 * 32 && GetPos().x < 365 * 32) // walkking left in air to get on the little block
        Input()->m_Direction = -1;
    if (GetPos().y < 361 * 32 && GetPos().y > 346 * 32)
    {
        if (Server()->Tick() % 10 == 0)
            SetWeapon(3);
        Input()->m_Direction = 1;
        // slow down and go back to enter the 2 tiles wide hole
        if (GetPos().x > 321 * 32)
            Input()->m_Direction = -1;
        if (GetPos().x > 317 * 32 && GetVel().x > 5.5f)
            Input()->m_Direction = 0;
        if (GetPos().x > 316 * 32 && GetVel().x > 9.8f)
            Input()->m_Direction = -1;
        // Get enough speed before the rj
        if (GetPos().x < 297 * 32 && GetPos().x > 296 * 32)
            if (GetVel().x < 9.9f)
                m_GetSpeed = true;
        if (m_GetSpeed)
        {
            Input()->m_Direction = -1;
            if ((GetPos().x < 294 * 32 && IsGrounded()) || GetPos().x < 280 * 32)
                m_GetSpeed = false;
        }
        Input()->m_TargetX = -50;
        Input()->m_TargetY = 100;
        if (GetPos().x < 303 * 32)
        {
            if (GetPos().x > 296 * 32)
                Fire();
        }
        else
        {
            Input()->m_TargetX = 50;
            if (GetPos().x > 310 * 32 && GetPos().x < 312 * 32)
                Fire();
        }
        Input()->m_Jump = 0;
        if (GetVel().y > 0.0000001f && GetPos().y > 352.6 * 32 && GetPos().x < 315 * 32) // jump in air to get to the right
            Input()->m_Jump = 1;
    }
    if (GetPos().x > 180 * 32 && GetPos().x < 450 * 32 && GetPos().y < 450 * 32 && GetPos().y > 358 * 32) // wider police area with left entrance
    {
        if (GetPos().y < 408 * 32)
            if (Server()->Tick() % 10 == 0)
                SetWeapon(1);
        // walking right again to get into the tunnel at the bottom
        if (GetPos().x < 363 * 32)
        {
            Input()->m_Direction = 1;
        }
        // do not enter in pvp area or bank
        if (GetPos().x > 323 * 32 && GetPos().y < 408 * 32)
            Input()->m_Direction = -1;
        // police area entrance tunnel (left side)
        if (GetPos().x > 316 * 32 && GetPos().x < 366 * 32 && GetPos().y > 416 * 32)
        {
            // jump through freeze if one is close or go back if no vel
            for (int i = 10; i < 160; i+=20)
            {
                int tile = GameServer()->Collision()->GetCustTile(GetPos().x + i, GetPos().y);
                if (tile == TILE_FREEZE)
                {
                    if (GetVel().y > 1.1f)
                    {
                        Input()->m_Direction = -1;
                    }
                    if (IsGrounded() && GetVel().x > 8.8f)
                        Input()->m_Jump = 1;
                    break;
                }
            }
        }
        /* * * * * * * *
         * police area *
         * * * * * * * */
        if (GetPos().x > 380 * 32 && GetPos().x < 450 * 32 && GetPos().y < 450 * 32 && GetPos().y > 380 * 32)
        {
            if (GetPos().x < 397 * 32 && GetPos().y > 436 * 32 && GetPos().x > 388 * 32) // on the money tile jump loop, to prevent blocking flappy there
            {
                Input()->m_Jump = 0;
                if (Server()->Tick() % 20 == 0)
                    Input()->m_Jump = 1;
            }
            //detect lower panic (accedentally fall into the lower police base 
            if (!m_LowerPanic && GetPos().y > 437 * 32 && GetPos().y > m_LovedY)
            {
                m_LowerPanic = 1;
                GameServer()->SendEmoticon(m_pPlayer->GetCID(), 9); //angry emote
            }

            if (m_LowerPanic)
            {
                //Check for end panic
                if (GetPos().y < 434 * 32)
                {
                    if (IsGrounded())
                        m_LowerPanic = 0; //made it up yay
                }

                if (m_LowerPanic == 1)//position to jump on stairs
                {
                    if (GetPos().x < 400 * 32)
                        Input()->m_Direction = 1;
                    else if (GetPos().x > 401 * 32)
                        Input()->m_Direction = -1;
                    else
                        m_LowerPanic = 2;
                }
                else if (m_LowerPanic == 2) //jump on the left starblock element
                {
                    if (IsGrounded())
                    {
                        Input()->m_Jump = 1;
                        if (Server()->Tick() % 20 == 0)
                            Input()->m_Jump = 0;
                    }

                    //navigate to platform
                    if (GetPos().y < 435 * 32 - 10)
                    {
                        Input()->m_Direction = -1;
                        if (GetPos().y < 433 * 32)
                        {
                            if (GetVel().y > 0.01f && m_IsDJUsed == false)
                            {
                                Input()->m_Jump = 1; //double jump
                                if (!IsGrounded()) // this dummyuseddj is for only using default 2 jumps even if 5 jump is on
                                    m_IsDJUsed = true;
                            }
                        }
                        if (m_IsDJUsed == true && IsGrounded())
                            m_IsDJUsed = false;
                    }

                    else if (GetPos().y < 438 * 32) //only if high enough focus on the first lower platform
                    {
                        if (GetPos().x < 403 * 32)
                            Input()->m_Direction = 1;
                        else if (GetPos().x > 404 * 32 + 20)
                            Input()->m_Direction = -1;
                    }

                    if ((GetPos().y > 441 * 32 + 10 && (GetPos().x > 402 * 32 || GetPos().x < 399 * 32 + 10)) || m_pChr->isFreezed) //check for fail position
                        m_LowerPanic = 1; //lower panic mode to reposition
                }
            }
            else //no dummy lower panic
            {
                m_HelpMode = 0;
                //check if officer needs help
                CCharacter *pChr = GameWorld()->ClosestCharacter(GetPos(), true, m_pChr);
                if (pChr && pChr->IsAlive())
                {
                    if (GetPos().y > 435 * 32) // setting the destination of dummy to top left police entry bcs otherwise bot fails when trying to help --> walks into jail wall xd
                    {
                        m_LovedX = (392 + rand() % 2) * 32;
                        m_LovedY = 430 * 32;
                    }
                    //aimbot on heuzeueu
                    Input()->m_TargetX = pChr->GetCore().m_Pos.x - GetPos().x;
                    Input()->m_TargetY = pChr->GetCore().m_Pos.y - GetPos().y;
                    LatestInput()->m_TargetX = pChr->GetCore().m_Pos.x - GetPos().x;
                    LatestInput()->m_TargetY = pChr->GetCore().m_Pos.y - GetPos().y;

                    m_IsClosestPolice = false;

                    if (pChr->GetPlayer()->m_PoliceHelper || pChr->GetPlayer()->m_PoliceRank)
                        m_IsClosestPolice = true;

                    if (pChr->GetCore().m_Pos.x > 444 * 32 - 10) //police dude failed too far --> to be reached by hook (set too help mode extream to leave save area)
                    {
                        m_HelpMode = 2;
                        if (Jumped() > 1 && GetPos().x > 431 * 32) //double jumped and above the freeze
                            Input()->m_Direction = -1;
                        else
                            Input()->m_Direction = 1;
                        //doublejump before falling in freeze
                        if ((GetPos().x > 432 * 32 && GetPos().y > 432 * 32) || GetPos().x > 437 * 32) //over the freeze and too low
                        {
                            Input()->m_Jump = 1;
                            m_IsHelpHook = true;
                        }
                        if (IsGrounded() && GetPos().x > 430 * 32 && Server()->Tick() % 60 == 0)
                            Input()->m_Jump = 1;
                    }
                    else
                        m_HelpMode = 1;

                    if (m_HelpMode == 1 && GetPos().x > 431 * 32 + 10)
                        Input()->m_Direction = -1;
                    else if (m_HelpMode == 1 && GetPos().x < 430 * 32)
                        Input()->m_Direction = 1;
                    else
                    {
                        if (!m_IsHelpHook && m_IsClosestPolice)
                        {
                            if (m_HelpMode == 2) //police dude failed too far --> to be reached by hook
                            {
                                //if (GetPos().x > 435 * 32) //moved too double jump
                                //{
                                //	m_IsHelpHook = true;
                                //}
                            }
                            else if (pChr->GetCore().m_Pos.x > 439 * 32) //police dude in the middle
                            {
                                if (IsGrounded())
                                {
                                    m_IsHelpHook = true;
                                    Input()->m_Jump = 1;
                                    Input()->m_Hook = 1;
                                }
                            }
                            else //police dude failed too near to hook from ground
                            {
                                if (GetVel().y < -4.20f && GetPos().y < 431 * 32)
                                {
                                    m_IsHelpHook = true;
                                    Input()->m_Jump = 1;
                                    Input()->m_Hook = 1;
                                }
                            }
                        }
                        if (Server()->Tick() % 8 == 0)
                            Input()->m_Direction = 1;
                    }

                    if (m_IsHelpHook)
                    {
                        Input()->m_Hook = 1;
                        if (Server()->Tick() % 200 == 0)
                        {
                            m_IsHelpHook = false; //timeout hook maybe failed
                            Input()->m_Hook = 0;
                            Input()->m_Direction = 1;
                        }
                    }

                    //dont wait on ground
                    if (IsGrounded() && Server()->Tick() % 900 == 0)
                        Input()->m_Jump = 1;
                    //backup reset jump
                    if (Server()->Tick() % 1337 == 0)
                        Input()->m_Jump = 0;
                }

                if (!m_HelpMode)
                {
                    //==============
                    //NOTHING TO DO
                    //==============
                    //basic walk to destination
                    if (GetPos().x < m_LovedX - 32)
                        Input()->m_Direction = 1;
                    else if (GetPos().x > m_LovedX + 32 && GetPos().x > 384 * 32)
                        Input()->m_Direction = -1;

                    //change changing speed
                    if (Server()->Tick() % m_Speed == 0)
                    {
                        if (rand() % 2 == 0)
                            m_Speed = rand() % 10000 + 420;
                    }

                    //choose beloved destination
                    if (Server()->Tick() % m_Speed == 0)
                    {
                        if ((rand() % 2) == 0)
                        {
                            if ((rand() % 3) == 0)
                            {
                                m_LovedX = 420 * 32 + rand() % 69;
                                m_LovedY = 430 * 32;
                                GameServer()->SendEmoticon(m_pPlayer->GetCID(), 7);
                            }
                            else
                            {
                                m_LovedX = (392 + rand() % 2) * 32;
                                m_LovedY = 430 * 32;
                            }
                            if ((rand() % 2) == 0)
                            {
                                m_LovedX = 384 * 32 + rand() % 128;
                                m_LovedY = 430 * 32;
                                GameServer()->SendEmoticon(m_pPlayer->GetCID(), 5);
                            }
                            else
                            {
                                if (rand() % 3 == 0)
                                {
                                    m_LovedX = 420 * 32 + rand() % 128;
                                    m_LovedY = 430 * 32;
                                    GameServer()->SendEmoticon(m_pPlayer->GetCID(), 8);
                                }
                                else if (rand() % 4 == 0)
                                {
                                    m_LovedX = 429 * 32 + rand() % 64;
                                    m_LovedY = 430 * 32;
                                    GameServer()->SendEmoticon(m_pPlayer->GetCID(), 8);
                                }
                            }
                            if (rand() % 5 == 0) //lower middel base
                            {
                                m_LovedX = 410 * 32 + rand() % 64;
                                m_LovedY = 443 * 32;
                            }
                        }
                        else
                            GameServer()->SendEmoticon(m_pPlayer->GetCID(), 1);
                    }
                }
            }

            //dont walk into the lower police base entry freeze
            if (GetPos().x > 425 * 32 && GetPos().x < 429 * 32) //right side
            {
                if (GetVel().x < -0.02f && IsGrounded())
                    Input()->m_Jump = 1;
            }
            else if (GetPos().x > 389 * 32 && GetPos().x < 391 * 32) //left side
            {
                if (GetVel().x > 0.02f && IsGrounded())
                    Input()->m_Jump = 1;
            }

            //jump over the police underground from entry to enty
            if (GetPos().y > m_LovedY) //only if beloved place is an upper one
            {
                if (GetPos().x > 415 * 32 && GetPos().x < 418 * 32) //right side
                {
                    if (GetVel().x < -0.02f && IsGrounded())
                    {
                        Input()->m_Jump = 1;
                        if (Server()->Tick() % 5 == 0)
                            Input()->m_Jump = 0;
                    }
                }
                else if (GetPos().x > 398 * 32 && GetPos().x < 401 * 32) //left side
                {
                    if (GetVel().x > 0.02f && IsGrounded())
                    {
                        Input()->m_Jump = 1;
                        if (Server()->Tick() % 5 == 0)
                            Input()->m_Jump = 0;
                    }
                }

                //do the doublejump
                if (GetVel().y > 6.9f && GetPos().y > 430 * 32 && GetPos().x < 433 * 32 && m_IsDJUsed == false) //falling and not too high to hit roof with head
                {
                    Input()->m_Jump = 1;
                    if (!IsGrounded()) // this dummyuseddj is for only using default 2 jumps even if 5 jump is on
                        m_IsDJUsed = true;
                }
                if (m_IsDJUsed == true && IsGrounded())
                    m_IsDJUsed = false;
            }
        }
        // left side of police the freeze pit
        if (GetPos().y > 380 * 32 && GetPos().x < 381 * 32 && GetPos().x > 363 * 32)
        {
            Input()->m_Direction = 1;
            if (GetPos().x > 367 * 32 && GetPos().x < 368 * 32 && IsGrounded())
                Input()->m_Jump = 1;
            if (GetPos().y > 433.7 * 32)
                Input()->m_Jump = 1;
        }
        if (GetPos().x > 290 * 32 && GetPos().x < 450 * 32 && GetPos().y > 415 * 32 && GetPos().y < 450 * 32)
        {
            if (m_pChr->isFreezed) // kills when in freeze in policebase or left of it (takes longer that he kills bcs the way is so long he wait a bit longer for help)
            {
                if (Server()->Tick() % 60 == 0)
                    GameServer()->SendEmoticon(m_pPlayer->GetCID(), 3); // tear emote before killing
                if (Server()->Tick() % 3000 == 0 && (IsGrounded() || GetPos().x > 430 * 32)) // kill when freeze
                {
                    Die();
                    return;
                }
            }
        }
    }
}