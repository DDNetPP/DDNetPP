/* projectile scoped ddnet++ methods */
#include "character.h"
#include "projectile.h"

#include <engine/shared/config.h>

#include <generated/protocol.h>

#include <game/server/gamecontext.h>
#include <game/server/gamemodes/DDRace.h>
#include <game/server/player.h>
#include <game/server/teams.h>
#include <game/version.h>

bool CProjectile::IsDDPPVanillaProjectile(int Collide, vec2 PrevPos, vec2 CurPos, vec2 ColPos, vec2 NewPos, CCharacter *pOwnerChar, float Pt, float Ct)
{
	if(!pOwnerChar)
		return false;
	if(!pOwnerChar->GetPlayer())
		return false;
	if(!pOwnerChar->GetPlayer()->m_IsVanillaWeapons)
		return false;

	CCharacter *TargetChr = GameServer()->m_World.IntersectCharacter(PrevPos, CurPos, 6.0f, ColPos, pOwnerChar);

	if(m_LifeSpan > -1)
		m_LifeSpan--;

	if(TargetChr || Collide || m_LifeSpan < 0 || GameLayerClipped(CurPos))
	{
		if(m_LifeSpan >= 0 || m_Type == WEAPON_GRENADE)
			GameServer()->CreateSound(CurPos, m_SoundImpact);

		if(m_Explosive)
			GameServer()->CreateExplosion(ColPos, m_Owner, m_Type, m_Owner == -1, (!TargetChr ? -1 : TargetChr->Team()), -1LL);

		else if(TargetChr)
			TargetChr->TakeDamage(m_Direction * 0.001f, 1, m_Owner, m_Type);

		m_MarkedForDestroy = true;
	}

	int x = GameServer()->Collision()->GetIndex(PrevPos, CurPos);
	int z;
	if(g_Config.m_SvOldTeleportWeapons)
		z = GameServer()->Collision()->IsTeleport(x);
	else
		z = GameServer()->Collision()->IsTeleportWeapon(x);
	if(z && !GameServer()->Collision()->TeleOuts(z - 1).empty())
	{
		int Num = GameServer()->Collision()->TeleOuts(z - 1).size();
		m_Pos = GameServer()->Collision()->TeleOuts(z - 1)[(!Num) ? Num : rand() % Num];
		m_StartTick = Server()->Tick();
	}
	return true;
}
