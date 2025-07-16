/* DDNet++ gamecore */

#include <engine/server/server.h>
#include <engine/shared/config.h>
#include <game/collision.h>
#include <game/server/ddpp/enums.h>
#include <game/server/entities/flag.h>

#include "gamecore.h"

void CCharacterCoreDDPP::SetFlagPos(int FlagId, vec2 Pos, bool AtStand, vec2 Vel, int CarrierId)
{
	m_aFlags[FlagId].m_Pos = Pos;
	m_aFlags[FlagId].m_Vel = Vel;
	m_aFlags[FlagId].m_AtStand = AtStand;
	m_aFlags[FlagId].m_CarrierId = CarrierId;
}

void CCharacterCore::DDPPTickHookFlying(vec2 NewPos)
{
	if(!g_Config.m_SvFlagHooking)
		return;

	vec2 ClosestPoint;
	for(int FlagId = 0; FlagId < 2; FlagId++)
	{
		CCharacterCoreDDPP::CFlagCore &Flag = m_DDNetPP.m_aFlags[FlagId];

		if(closest_point_on_line(m_HookPos, NewPos, Flag.m_Pos, ClosestPoint))
		{
			if(distance(Flag.m_Pos, ClosestPoint) > CFlag::ms_PhysSize + 2.0f)
				continue;
			if(Flag.m_AtStand)
				continue;

			if(Flag.m_CarrierId == -1 && m_HookedPlayer != CLIENT_ID_FLAG_RED && m_HookedPlayer != CLIENT_ID_FLAG_BLUE)
			{
				if(m_HookedPlayer == -1 /*|| distance(m_HookPos, m_aFlagPos[0]) < Distance*/)
				{
					m_TriggeredEvents |= COREEVENT_HOOK_ATTACH_PLAYER;
					m_HookState = HOOK_GRABBED;
					m_HookedPlayer = FlagId == FLAG_RED ? CLIENT_ID_FLAG_RED : CLIENT_ID_FLAG_BLUE;
				}
			}
			// else
			// {
			// 	dbg_msg(
			// 		"flag",
			// 		"distance=%s %.2f < %.2f flag=(%.2f/%.2f) pos=(%.2f/%.2f) carry=%d hookedPlayer=%s",
			// 		(distance(Flag.m_Pos, ClosestPoint) < CFlag::ms_PhysSize + 2.0f) ? "close_enough" : "toofar",
			// 		distance(Flag.m_Pos, ClosestPoint), CFlag::ms_PhysSize + 2.0f,
			// 		Flag.m_Pos.x / 32, Flag.m_Pos.y / 32,
			// 		ClosestPoint.x / 32, ClosestPoint.y / 32,
			// 		Flag.m_CarrierId,
			// 		(m_HookedPlayer == CLIENT_ID_FLAG_RED || m_HookedPlayer == FLAG_BLUE) ? "flag" : "no_flag");
			// }
		}
	}
}

bool CCharacterCore::HookFlag()
{
	int FlagId = -1;
	if(m_HookedPlayer == CLIENT_ID_FLAG_RED)
		FlagId = FLAG_RED;
	if(m_HookedPlayer == CLIENT_ID_FLAG_BLUE)
		FlagId = FLAG_BLUE;
	if(FlagId == -1)
		return false;

	if(m_DDNetPP.m_aFlags[FlagId].m_CarrierId == -1)
	{
		m_HookPos = m_DDNetPP.m_aFlags[FlagId].m_Pos;
	}
	else
	{
		m_HookedPlayer = -1;
		m_HookState = HOOK_RETRACTED;
		m_HookPos = m_Pos;
	}
	return true;
}

void CCharacterCore::DDPPTick()
{
	m_updateFlagVel = 0;

	if(m_DDNetPP.m_LastHookedTick != -1)
	{
		m_DDNetPP.m_LastHookedTick = m_DDNetPP.m_LastHookedTick + 1;
	}

	if(m_DDNetPP.m_LastHookedTick > SERVER_TICK_SPEED * 10)
	{
		m_DDNetPP.m_LastHookedPlayer = -1;
		m_DDNetPP.m_LastHookedTick = -1;
	}
	if(m_HookState == HOOK_GRABBED)
	{
		// TODO: do not call this 3 times
		HookFlag();

		if(m_HookedPlayer == CLIENT_ID_FLAG_RED)
		{
			if(m_DDNetPP.m_aFlags[0].m_AtStand)
			{
				m_HookedPlayer = -1;
				m_HookState = HOOK_RETRACTED;
				m_HookPos = m_Pos;
			}
		}
		if(m_HookedPlayer == CLIENT_ID_FLAG_BLUE)
		{
			if(m_DDNetPP.m_aFlags[1].m_AtStand)
			{
				m_HookedPlayer = -1;
				m_HookState = HOOK_RETRACTED;
				m_HookPos = m_Pos;
			}
		}
	}

	if(m_DDNetPP.m_LastHookedPlayer != CLIENT_ID_FLAG_BLUE && m_DDNetPP.m_LastHookedPlayer != CLIENT_ID_FLAG_RED && m_DDNetPP.m_LastHookedPlayer != -1 && !m_pWorld->m_apCharacters[m_DDNetPP.m_LastHookedPlayer])
	{
		m_DDNetPP.m_LastHookedPlayer = -1;
	}

	if(m_pWorld)
	{
		if(m_HookedPlayer == CLIENT_ID_FLAG_BLUE || m_HookedPlayer == CLIENT_ID_FLAG_RED)
		{
			float Distance;
			vec2 FlagVel;
			vec2 Dir;
			vec2 FPos;
			vec2 Temp;

			if(m_HookedPlayer == CLIENT_ID_FLAG_RED)
			{
				m_updateFlagVel = CLIENT_ID_FLAG_RED;
				Temp = m_DDNetPP.m_aFlags[0].m_Vel;
				FlagVel = m_DDNetPP.m_aFlags[0].m_Vel;
				FPos = m_DDNetPP.m_aFlags[0].m_Pos;
				Distance = distance(m_Pos, m_DDNetPP.m_aFlags[0].m_Pos);
				Dir = normalize(m_Pos - m_DDNetPP.m_aFlags[0].m_Pos);
			}
			if(m_HookedPlayer == CLIENT_ID_FLAG_BLUE)
			{
				m_updateFlagVel = CLIENT_ID_FLAG_BLUE;
				Temp = m_DDNetPP.m_aFlags[1].m_Vel;
				FlagVel = m_DDNetPP.m_aFlags[1].m_Vel;
				FPos = m_DDNetPP.m_aFlags[1].m_Pos;
				Distance = distance(m_Pos, m_DDNetPP.m_aFlags[1].m_Pos);
				Dir = normalize(m_Pos - m_DDNetPP.m_aFlags[1].m_Pos);
			}

			if(Distance > CFlag::ms_PhysSize * 1.50f) // TODO: fix tweakable variable
			{
				float Accel = m_pWorld->m_aTuning[g_Config.m_ClDummy].m_HookDragAccel * (Distance / m_pWorld->m_aTuning[g_Config.m_ClDummy].m_HookLength);
				float DragSpeed = m_pWorld->m_aTuning[g_Config.m_ClDummy].m_HookDragSpeed;

				Temp.x = SaturatedAdd(-DragSpeed, DragSpeed, FlagVel.x, Accel * Dir.x * 1.5f);
				Temp.y = SaturatedAdd(-DragSpeed, DragSpeed, FlagVel.y, Accel * Dir.y * 1.5f);

				/*int FMapIndex = Collision()->GetPureMapIndex(FPos);int FMapIndexL = Collision()->GetPureMapIndex(vec2(FPos.x + (28/2)+4,FPos.y));int FMapIndexR = Collision()->GetPureMapIndex(vec2(FPos.x - (28/2)-4,FPos.y));int FMapIndexT = Collision()->GetPureMapIndex(vec2(FPos.x,FPos.y + (28/2)+4));int FMapIndexB = Collision()->GetPureMapIndex(vec2(FPos.x,FPos.y - (28/2)-4));

				int m_FTileIndex = Collision()->GetTileIndex(FMapIndex);int m_FTileFlags = Collision()->GetTileFlags(FMapIndex);int m_FTileIndexL = Collision()->GetTileIndex(FMapIndexL);int m_FTileFlagsL = Collision()->GetTileFlags(FMapIndexL);int m_FTileIndexR = Collision()->GetTileIndex(FMapIndexR);
				int m_FTileFlagsR = Collision()->GetTileFlags(FMapIndexR);int m_FTileIndexB = Collision()->GetTileIndex(FMapIndexB);int m_FTileFlagsB = Collision()->GetTileFlags(FMapIndexB);int m_FTileIndexT = Collision()->GetTileIndex(FMapIndexT);int m_FTileFlagsT = Collision()->GetTileFlags(FMapIndexT);
				int m_FTileFIndex = Collision()->GetFrontTileIndex(FMapIndex);int m_FTileFFlags = Collision()->GetFTileFlags(FMapIndex);int m_FTileFIndexL = Collision()->GetFrontTileIndex(FMapIndexL);int m_FTileFFlagsL = Collision()->GetFTileFlags(FMapIndexL);int m_FTileFIndexR = Collision()->GetFrontTileIndex(FMapIndexR);int m_FTileFFlagsR = Collision()->GetFTileFlags(FMapIndexR);
				int m_FTileFIndexB = Collision()->GetFrontTileIndex(FMapIndexB);int m_FTileFFlagsB = Collision()->GetFTileFlags(FMapIndexB);int m_FTileFIndexT = Collision()->GetFrontTileIndex(FMapIndexT);int m_FTileFFlagsT = Collision()->GetFTileFlags(FMapIndexT);int m_FTileSIndex = (UseInput && IsRightTeam(FMapIndex))?Collision()->GetDTileIndex(FMapIndex):0;int m_FTileSFlags = (UseInput && IsRightTeam(FMapIndex))?Collision()->GetDTileFlags(FMapIndex):0;
				int m_FTileSIndexL = (UseInput && IsRightTeam(FMapIndexL))?Collision()->GetDTileIndex(FMapIndexL):0;int m_FTileSFlagsL = (UseInput && IsRightTeam(FMapIndexL))?Collision()->GetDTileFlags(FMapIndexL):0;int m_FTileSIndexR = (UseInput && IsRightTeam(FMapIndexR))?Collision()->GetDTileIndex(FMapIndexR):0;int m_FTileSFlagsR = (UseInput && IsRightTeam(FMapIndexR))?Collision()->GetDTileFlags(FMapIndexR):0;
				int m_FTileSIndexB = (UseInput && IsRightTeam(FMapIndexB))?Collision()->GetDTileIndex(FMapIndexB):0;int m_FTileSFlagsB = (UseInput && IsRightTeam(FMapIndexB))?Collision()->GetDTileFlags(FMapIndexB):0;int m_FTileSIndexT = (UseInput && IsRightTeam(FMapIndexT))?Collision()->GetDTileIndex(FMapIndexT):0;int m_FTileSFlagsT = (UseInput && IsRightTeam(FMapIndexT))?Collision()->GetDTileFlags(FMapIndexT):0;

				if(Temp.x > 0 && ((m_FTileIndex == TILE_STOP && m_FTileFlags == ROTATION_270) || (m_FTileIndexL == TILE_STOP && m_FTileFlagsL == ROTATION_270) || (m_FTileIndexL == TILE_STOPS && (m_FTileFlagsL == ROTATION_90 || m_FTileFlagsL ==ROTATION_270)) || (m_FTileIndexL == TILE_STOPA) || (m_FTileFIndex == TILE_STOP && m_FTileFFlags == ROTATION_270) || (m_FTileFIndexL == TILE_STOP && m_FTileFFlagsL == ROTATION_270) || (m_FTileFIndexL == TILE_STOPS && (m_FTileFFlagsL == ROTATION_90 || m_FTileFFlagsL == ROTATION_270)) || (m_FTileFIndexL == TILE_STOPA) || (m_FTileSIndex == TILE_STOP && m_FTileSFlags == ROTATION_270) || (m_FTileSIndexL == TILE_STOP && m_FTileSFlagsL == ROTATION_270) || (m_FTileSIndexL == TILE_STOPS && (m_FTileSFlagsL == ROTATION_90 || m_FTileSFlagsL == ROTATION_270)) || (m_FTileSIndexL == TILE_STOPA)))
					Temp.x = 0;
				if(Temp.x < 0 && ((m_FTileIndex == TILE_STOP && m_FTileFlags == ROTATION_90) || (m_FTileIndexR == TILE_STOP && m_FTileFlagsR == ROTATION_90) || (m_FTileIndexR == TILE_STOPS && (m_FTileFlagsR == ROTATION_90 || m_FTileFlagsR == ROTATION_270)) || (m_FTileIndexR == TILE_STOPA) || (m_FTileFIndex == TILE_STOP && m_FTileFFlags == ROTATION_90) || (m_FTileFIndexR == TILE_STOP && m_FTileFFlagsR == ROTATION_90) || (m_FTileFIndexR == TILE_STOPS && (m_FTileFFlagsR == ROTATION_90 || m_FTileFFlagsR == ROTATION_270)) || (m_FTileFIndexR == TILE_STOPA) || (m_FTileSIndex == TILE_STOP && m_FTileSFlags == ROTATION_90) || (m_FTileSIndexR == TILE_STOP && m_FTileSFlagsR == ROTATION_90) || (m_FTileSIndexR == TILE_STOPS && (m_FTileSFlagsR == ROTATION_90 || m_FTileSFlagsR == ROTATION_270)) || (m_FTileSIndexR == TILE_STOPA)))
					Temp.x = 0;
				if(Temp.y < 0 && ((m_FTileIndex == TILE_STOP && m_FTileFlags == ROTATION_180) || (m_FTileIndexB == TILE_STOP && m_FTileFlagsB == ROTATION_180) || (m_FTileIndexB == TILE_STOPS && (m_FTileFlagsB == ROTATION_0 || m_FTileFlagsB == ROTATION_180)) || (m_FTileIndexB == TILE_STOPA) || (m_FTileFIndex == TILE_STOP && m_FTileFFlags == ROTATION_180) || (m_FTileFIndexB == TILE_STOP && m_FTileFFlagsB == ROTATION_180) || (m_FTileFIndexB == TILE_STOPS && (m_FTileFFlagsB == ROTATION_0 || m_FTileFFlagsB == ROTATION_180)) || (m_FTileFIndexB == TILE_STOPA) || (m_FTileSIndex == TILE_STOP && m_FTileSFlags == ROTATION_180) || (m_FTileSIndexB == TILE_STOP && m_FTileSFlagsB == ROTATION_180) || (m_FTileSIndexB == TILE_STOPS && (m_FTileSFlagsB == ROTATION_0 || m_FTileSFlagsB == ROTATION_180)) || (m_FTileSIndexB == TILE_STOPA)))
					Temp.y = 0;
				if(Temp.y > 0 && ((m_FTileIndex == TILE_STOP && m_FTileFlags == ROTATION_0) || (m_FTileIndexT == TILE_STOP && m_FTileFlagsT == ROTATION_0) || (m_FTileIndexT == TILE_STOPS && (m_FTileFlagsT == ROTATION_0 || m_FTileFlagsT == ROTATION_180)) || (m_FTileIndexT == TILE_STOPA) || (m_FTileFIndex == TILE_STOP && m_FTileFFlags == ROTATION_0) || (m_FTileFIndexT == TILE_STOP && m_FTileFFlagsT == ROTATION_0) || (m_FTileFIndexT == TILE_STOPS && (m_FTileFFlagsT == ROTATION_0 || m_FTileFFlagsT == ROTATION_180)) || (m_FTileFIndexT == TILE_STOPA) || (m_FTileSIndex == TILE_STOP && m_FTileSFlags == ROTATION_0) || (m_FTileSIndexT == TILE_STOP && m_FTileSFlagsT == ROTATION_0) || (m_FTileSIndexT == TILE_STOPS && (m_FTileSFlagsT == ROTATION_0 || m_FTileSFlagsT == ROTATION_180)) || (m_FTileSIndexT == TILE_STOPA)))
					Temp.y = 0;*/

				m_UFlagVel = Temp;

				Temp.x = SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.x, -Accel * Dir.x * 0.25f);
				Temp.y = SaturatedAdd(-DragSpeed, DragSpeed, m_Vel.y, -Accel * Dir.y * 0.25f);
				m_Vel = ClampVel(m_MoveRestrictions, Temp);
			}
		}
	}
}
