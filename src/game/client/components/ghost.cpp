/* (c) Rajh, Redix and Sushi. */

#include <engine/ghost.h>
#include <engine/shared/config.h>
#include <engine/storage.h>

#include <game/client/race.h>

#include "ghost.h"
#include "menus.h"
#include "players.h"
#include "skins.h"

#include <game/client/gameclient.h>

const char *CGhost::ms_pGhostDir = "ghosts";

CGhost::CGhost() :
	m_NewRenderTick(-1), m_StartRenderTick(-1), m_LastDeathTick(-1), m_LastRaceTick(-1), m_Recording(false), m_Rendering(false) {}

void CGhost::GetGhostSkin(CGhostSkin *pSkin, const char *pSkinName, int UseCustomColor, int ColorBody, int ColorFeet)
{
	StrToInts(&pSkin->m_Skin0, 6, pSkinName);
	pSkin->m_UseCustomColor = UseCustomColor;
	pSkin->m_ColorBody = ColorBody;
	pSkin->m_ColorFeet = ColorFeet;
}

void CGhost::GetGhostCharacter(CGhostCharacter *pGhostChar, const CNetObj_Character *pChar, const CNetObj_DDNetCharacter *pDDnetChar)
{
	pGhostChar->m_X = pChar->m_X;
	pGhostChar->m_Y = pChar->m_Y;
	pGhostChar->m_VelX = pChar->m_VelX;
	pGhostChar->m_VelY = 0;
	pGhostChar->m_Angle = pChar->m_Angle;
	pGhostChar->m_Direction = pChar->m_Direction;
	int Weapon = pChar->m_Weapon;
	if(pDDnetChar != nullptr && pDDnetChar->m_FreezeEnd != 0)
	{
		Weapon = WEAPON_NINJA;
	}
	pGhostChar->m_Weapon = Weapon;
	pGhostChar->m_HookState = pChar->m_HookState;
	pGhostChar->m_HookX = pChar->m_HookX;
	pGhostChar->m_HookY = pChar->m_HookY;
	pGhostChar->m_AttackTick = pChar->m_AttackTick;
	pGhostChar->m_Tick = pChar->m_Tick;
}

void CGhost::GetNetObjCharacter(CNetObj_Character *pChar, const CGhostCharacter *pGhostChar)
{
	mem_zero(pChar, sizeof(CNetObj_Character));
	pChar->m_X = pGhostChar->m_X;
	pChar->m_Y = pGhostChar->m_Y;
	pChar->m_VelX = pGhostChar->m_VelX;
	pChar->m_VelY = 0;
	pChar->m_Angle = pGhostChar->m_Angle;
	pChar->m_Direction = pGhostChar->m_Direction;
	pChar->m_Weapon = pGhostChar->m_Weapon;
	pChar->m_HookState = pGhostChar->m_HookState;
	pChar->m_HookX = pGhostChar->m_HookX;
	pChar->m_HookY = pGhostChar->m_HookY;
	pChar->m_AttackTick = pGhostChar->m_AttackTick;
	pChar->m_HookedPlayer = -1;
	pChar->m_Tick = pGhostChar->m_Tick;
}

CGhost::CGhostPath::CGhostPath(CGhostPath &&Other) noexcept :
	m_ChunkSize(Other.m_ChunkSize), m_NumItems(Other.m_NumItems), m_vpChunks(std::move(Other.m_vpChunks))
{
	Other.m_NumItems = 0;
	Other.m_vpChunks.clear();
}

CGhost::CGhostPath &CGhost::CGhostPath::operator=(CGhostPath &&Other) noexcept
{
	Reset(Other.m_ChunkSize);
	m_NumItems = Other.m_NumItems;
	m_vpChunks = std::move(Other.m_vpChunks);
	Other.m_NumItems = 0;
	Other.m_vpChunks.clear();
	return *this;
}

void CGhost::CGhostPath::Reset(int ChunkSize)
{
	for(auto &pChunk : m_vpChunks)
		free(pChunk);
	m_vpChunks.clear();
	m_ChunkSize = ChunkSize;
	m_NumItems = 0;
}

void CGhost::CGhostPath::SetSize(int Items)
{
	int Chunks = m_vpChunks.size();
	int NeededChunks = (Items + m_ChunkSize - 1) / m_ChunkSize;

	if(NeededChunks > Chunks)
	{
		m_vpChunks.resize(NeededChunks);
		for(int i = Chunks; i < NeededChunks; i++)
			m_vpChunks[i] = (CGhostCharacter *)calloc(m_ChunkSize, sizeof(CGhostCharacter));
	}

	m_NumItems = Items;
}

void CGhost::CGhostPath::Add(const CGhostCharacter &Char)
{
	SetSize(m_NumItems + 1);
	*Get(m_NumItems - 1) = Char;
}

CGhostCharacter *CGhost::CGhostPath::Get(int Index)
{
	if(Index < 0 || Index >= m_NumItems)
		return 0;

	int Chunk = Index / m_ChunkSize;
	int Pos = Index % m_ChunkSize;
	return &m_vpChunks[Chunk][Pos];
}

void CGhost::GetPath(char *pBuf, int Size, const char *pPlayerName, int Time) const
{
	const char *pMap = Client()->GetCurrentMap();
	SHA256_DIGEST Sha256 = Client()->GetCurrentMapSha256();
	char aSha256[SHA256_MAXSTRSIZE];
	sha256_str(Sha256, aSha256, sizeof(aSha256));

	char aPlayerName[MAX_NAME_LENGTH];
	str_copy(aPlayerName, pPlayerName);
	str_sanitize_filename(aPlayerName);

	if(Time < 0)
		str_format(pBuf, Size, "%s/%s_%s_%s_tmp_%d.gho", ms_pGhostDir, pMap, aPlayerName, aSha256, pid());
	else
		str_format(pBuf, Size, "%s/%s_%s_%d.%03d_%s.gho", ms_pGhostDir, pMap, aPlayerName, Time / 1000, Time % 1000, aSha256);
}

void CGhost::AddInfos(const CNetObj_Character *pChar, const CNetObj_DDNetCharacter *pDDnetChar)
{
	int NumTicks = m_CurGhost.m_Path.Size();

	// do not start writing to file as long as we still touch the start line
	if(g_Config.m_ClRaceSaveGhost && !GhostRecorder()->IsRecording() && NumTicks > 0)
	{
		GetPath(m_aTmpFilename, sizeof(m_aTmpFilename), m_CurGhost.m_aPlayer);
		GhostRecorder()->Start(m_aTmpFilename, Client()->GetCurrentMap(), Client()->GetCurrentMapSha256(), m_CurGhost.m_aPlayer);

		GhostRecorder()->WriteData(GHOSTDATA_TYPE_START_TICK, &m_CurGhost.m_StartTick, sizeof(int));
		GhostRecorder()->WriteData(GHOSTDATA_TYPE_SKIN, &m_CurGhost.m_Skin, sizeof(CGhostSkin));
		for(int i = 0; i < NumTicks; i++)
			GhostRecorder()->WriteData(GHOSTDATA_TYPE_CHARACTER, m_CurGhost.m_Path.Get(i), sizeof(CGhostCharacter));
	}

	CGhostCharacter GhostChar;
	GetGhostCharacter(&GhostChar, pChar, pDDnetChar);
	m_CurGhost.m_Path.Add(GhostChar);
	if(GhostRecorder()->IsRecording())
		GhostRecorder()->WriteData(GHOSTDATA_TYPE_CHARACTER, &GhostChar, sizeof(CGhostCharacter));
}

int CGhost::GetSlot() const
{
	for(int i = 0; i < MAX_ACTIVE_GHOSTS; i++)
		if(m_aActiveGhosts[i].Empty())
			return i;
	return -1;
}

int CGhost::FreeSlots() const
{
	int Num = 0;
	for(const auto &ActiveGhost : m_aActiveGhosts)
		if(ActiveGhost.Empty())
			Num++;
	return Num;
}

void CGhost::CheckStart()
{
	int RaceTick = -m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer;
	int RenderTick = m_NewRenderTick;

	if(m_LastRaceTick != RaceTick && Client()->GameTick(g_Config.m_ClDummy) - RaceTick < Client()->GameTickSpeed())
	{
		if(m_Rendering && m_RenderingStartedByServer) // race restarted: stop rendering
			StopRender();
		if(m_Recording && m_LastRaceTick != -1) // race restarted: activate restarting for local start detection so we have a smooth transition
			m_AllowRestart = true;
		if(m_LastRaceTick == -1) // no restart: reset rendering preparations
			m_NewRenderTick = -1;
		if(GhostRecorder()->IsRecording()) // race restarted: stop recording
			GhostRecorder()->Stop(0, -1);
		int StartTick = RaceTick;

		if(GameClient()->m_GameInfo.m_BugDDRaceGhost) // the client recognizes the start one tick earlier than ddrace servers
			StartTick--;
		StartRecord(StartTick);
		RenderTick = StartTick;
	}

	TryRenderStart(RenderTick, true);
}

void CGhost::CheckStartLocal(bool Predicted)
{
	if(Predicted) // rendering
	{
		int RenderTick = m_NewRenderTick;

		vec2 PrevPos = m_pClient->m_PredictedPrevChar.m_Pos;
		vec2 Pos = m_pClient->m_PredictedChar.m_Pos;
		if(((!m_Rendering && RenderTick == -1) || m_AllowRestart) && CRaceHelper::IsStart(m_pClient, PrevPos, Pos))
		{
			if(m_Rendering && !m_RenderingStartedByServer) // race restarted: stop rendering
				StopRender();
			RenderTick = Client()->PredGameTick(g_Config.m_ClDummy);
		}

		TryRenderStart(RenderTick, false);
	}
	else // recording
	{
		int PrevTick = m_pClient->m_Snap.m_pLocalPrevCharacter->m_Tick;
		int CurTick = m_pClient->m_Snap.m_pLocalCharacter->m_Tick;
		vec2 PrevPos = vec2(m_pClient->m_Snap.m_pLocalPrevCharacter->m_X, m_pClient->m_Snap.m_pLocalPrevCharacter->m_Y);
		vec2 Pos = vec2(m_pClient->m_Snap.m_pLocalCharacter->m_X, m_pClient->m_Snap.m_pLocalCharacter->m_Y);

		// detecting death, needed because race allows immediate respawning
		if((!m_Recording || m_AllowRestart) && m_LastDeathTick < PrevTick)
		{
			// estimate the exact start tick
			int RecordTick = -1;
			int TickDiff = CurTick - PrevTick;
			for(int i = 0; i < TickDiff; i++)
			{
				if(CRaceHelper::IsStart(m_pClient, mix(PrevPos, Pos, (float)i / TickDiff), mix(PrevPos, Pos, (float)(i + 1) / TickDiff)))
				{
					RecordTick = PrevTick + i + 1;
					if(!m_AllowRestart)
						break;
				}
			}
			if(RecordTick != -1)
			{
				if(GhostRecorder()->IsRecording()) // race restarted: stop recording
					GhostRecorder()->Stop(0, -1);
				StartRecord(RecordTick);
			}
		}
	}
}

void CGhost::TryRenderStart(int Tick, bool ServerControl)
{
	// only restart rendering if it did not change since last tick to prevent stuttering
	if(m_NewRenderTick != -1 && m_NewRenderTick == Tick)
	{
		StartRender(Tick);
		Tick = -1;
		m_RenderingStartedByServer = ServerControl;
	}
	m_NewRenderTick = Tick;
}

void CGhost::OnNewSnapshot()
{
	if(!GameClient()->m_GameInfo.m_Race || Client()->State() != IClient::STATE_ONLINE)
		return;
	if(!m_pClient->m_Snap.m_pGameInfoObj || m_pClient->m_Snap.m_SpecInfo.m_Active || !m_pClient->m_Snap.m_pLocalCharacter || !m_pClient->m_Snap.m_pLocalPrevCharacter)
		return;

	bool RaceFlag = m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags & GAMESTATEFLAG_RACETIME;
	bool ServerControl = RaceFlag && g_Config.m_ClRaceGhostServerControl;

	if(g_Config.m_ClRaceGhost)
	{
		if(!ServerControl)
			CheckStartLocal(false);
		else
			CheckStart();

		if(m_Recording)
			AddInfos(m_pClient->m_Snap.m_pLocalCharacter, (m_pClient->m_Snap.m_LocalClientID != -1 && m_pClient->m_Snap.m_aCharacters[m_pClient->m_Snap.m_LocalClientID].m_HasExtendedData) ? &m_pClient->m_Snap.m_aCharacters[m_pClient->m_Snap.m_LocalClientID].m_ExtendedData : nullptr);
	}

	// Record m_LastRaceTick for g_Config.m_ClConfirmDisconnect/QuitTime anyway
	int RaceTick = -m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer;
	m_LastRaceTick = RaceFlag ? RaceTick : -1;
}

void CGhost::OnNewPredictedSnapshot()
{
	if(!GameClient()->m_GameInfo.m_Race || !g_Config.m_ClRaceGhost || Client()->State() != IClient::STATE_ONLINE)
		return;
	if(!m_pClient->m_Snap.m_pGameInfoObj || m_pClient->m_Snap.m_SpecInfo.m_Active || !m_pClient->m_Snap.m_pLocalCharacter || !m_pClient->m_Snap.m_pLocalPrevCharacter)
		return;

	bool RaceFlag = m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags & GAMESTATEFLAG_RACETIME;
	bool ServerControl = RaceFlag && g_Config.m_ClRaceGhostServerControl;

	if(!ServerControl)
		CheckStartLocal(true);
}

void CGhost::OnRender()
{
	// Play the ghost
	if(!m_Rendering || !g_Config.m_ClRaceShowGhost)
		return;

	int PlaybackTick = Client()->PredGameTick(g_Config.m_ClDummy) - m_StartRenderTick;

	for(auto &Ghost : m_aActiveGhosts)
	{
		if(Ghost.Empty())
			continue;

		int GhostTick = Ghost.m_StartTick + PlaybackTick;
		while(Ghost.m_PlaybackPos >= 0 && Ghost.m_Path.Get(Ghost.m_PlaybackPos)->m_Tick < GhostTick)
		{
			if(Ghost.m_PlaybackPos < Ghost.m_Path.Size() - 1)
				Ghost.m_PlaybackPos++;
			else
				Ghost.m_PlaybackPos = -1;
		}

		if(Ghost.m_PlaybackPos < 0)
			continue;

		int CurPos = Ghost.m_PlaybackPos;
		int PrevPos = maximum(0, CurPos - 1);
		if(Ghost.m_Path.Get(PrevPos)->m_Tick > GhostTick)
			continue;

		CNetObj_Character Player, Prev;
		GetNetObjCharacter(&Player, Ghost.m_Path.Get(CurPos));
		GetNetObjCharacter(&Prev, Ghost.m_Path.Get(PrevPos));

		int TickDiff = Player.m_Tick - Prev.m_Tick;
		float IntraTick = 0.f;
		if(TickDiff > 0)
			IntraTick = (GhostTick - Prev.m_Tick - 1 + Client()->PredIntraGameTick(g_Config.m_ClDummy)) / TickDiff;

		Player.m_AttackTick += Client()->GameTick(g_Config.m_ClDummy) - GhostTick;

		CTeeRenderInfo *pRenderInfo = &Ghost.m_RenderInfo;
		CTeeRenderInfo GhostNinjaRenderInfo;
		if(Player.m_Weapon == WEAPON_NINJA && g_Config.m_ClShowNinja)
		{
			// change the skin for the ghost to the ninja
			const auto *pSkin = m_pClient->m_Skins.FindOrNullptr("x_ninja");
			if(pSkin != nullptr)
			{
				bool IsTeamplay = false;
				if(m_pClient->m_Snap.m_pGameInfoObj)
					IsTeamplay = (m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags & GAMEFLAG_TEAMS) != 0;

				GhostNinjaRenderInfo = Ghost.m_RenderInfo;
				GhostNinjaRenderInfo.m_OriginalRenderSkin = pSkin->m_OriginalSkin;
				GhostNinjaRenderInfo.m_ColorableRenderSkin = pSkin->m_ColorableSkin;
				GhostNinjaRenderInfo.m_BloodColor = pSkin->m_BloodColor;
				GhostNinjaRenderInfo.m_SkinMetrics = pSkin->m_Metrics;
				GhostNinjaRenderInfo.m_CustomColoredSkin = IsTeamplay;
				if(!IsTeamplay)
				{
					GhostNinjaRenderInfo.m_ColorBody = ColorRGBA(1, 1, 1);
					GhostNinjaRenderInfo.m_ColorFeet = ColorRGBA(1, 1, 1);
				}
				pRenderInfo = &GhostNinjaRenderInfo;
			}
		}

		m_pClient->m_Players.RenderHook(&Prev, &Player, pRenderInfo, -2, IntraTick);
		m_pClient->m_Players.RenderHookCollLine(&Prev, &Player, -2, IntraTick);
		m_pClient->m_Players.RenderPlayer(&Prev, &Player, pRenderInfo, -2, IntraTick);
	}
}

void CGhost::InitRenderInfos(CGhostItem *pGhost)
{
	char aSkinName[64];
	IntsToStr(&pGhost->m_Skin.m_Skin0, 6, aSkinName);
	CTeeRenderInfo *pRenderInfo = &pGhost->m_RenderInfo;

	const CSkin *pSkin = m_pClient->m_Skins.Find(aSkinName);
	pRenderInfo->m_OriginalRenderSkin = pSkin->m_OriginalSkin;
	pRenderInfo->m_ColorableRenderSkin = pSkin->m_ColorableSkin;
	pRenderInfo->m_BloodColor = pSkin->m_BloodColor;
	pRenderInfo->m_SkinMetrics = pSkin->m_Metrics;
	pRenderInfo->m_CustomColoredSkin = pGhost->m_Skin.m_UseCustomColor;
	if(pGhost->m_Skin.m_UseCustomColor)
	{
		pRenderInfo->m_ColorBody = color_cast<ColorRGBA>(ColorHSLA(pGhost->m_Skin.m_ColorBody).UnclampLighting());
		pRenderInfo->m_ColorFeet = color_cast<ColorRGBA>(ColorHSLA(pGhost->m_Skin.m_ColorFeet).UnclampLighting());
	}
	else
	{
		pRenderInfo->m_ColorBody = ColorRGBA(1, 1, 1);
		pRenderInfo->m_ColorFeet = ColorRGBA(1, 1, 1);
	}

	pRenderInfo->m_Size = 64;
}

void CGhost::StartRecord(int Tick)
{
	m_Recording = true;
	m_CurGhost.Reset();
	m_CurGhost.m_StartTick = Tick;

	const CGameClient::CClientData *pData = &m_pClient->m_aClients[m_pClient->m_Snap.m_LocalClientID];
	str_copy(m_CurGhost.m_aPlayer, Client()->PlayerName());
	GetGhostSkin(&m_CurGhost.m_Skin, pData->m_aSkinName, pData->m_UseCustomColor, pData->m_ColorBody, pData->m_ColorFeet);
	InitRenderInfos(&m_CurGhost);
}

void CGhost::StopRecord(int Time)
{
	m_Recording = false;
	bool RecordingToFile = GhostRecorder()->IsRecording();

	if(RecordingToFile)
		GhostRecorder()->Stop(m_CurGhost.m_Path.Size(), Time);

	CMenus::CGhostItem *pOwnGhost = m_pClient->m_Menus.GetOwnGhost();
	if(Time > 0 && (!pOwnGhost || Time < pOwnGhost->m_Time))
	{
		if(pOwnGhost && pOwnGhost->Active())
			Unload(pOwnGhost->m_Slot);

		// add to active ghosts
		int Slot = GetSlot();
		if(Slot != -1)
			m_aActiveGhosts[Slot] = std::move(m_CurGhost);

		// create ghost item
		CMenus::CGhostItem Item;
		if(RecordingToFile)
			GetPath(Item.m_aFilename, sizeof(Item.m_aFilename), m_CurGhost.m_aPlayer, Time);
		str_copy(Item.m_aPlayer, m_CurGhost.m_aPlayer);
		Item.m_Time = Time;
		Item.m_Slot = Slot;

		// save new ghost file
		if(Item.HasFile())
			Storage()->RenameFile(m_aTmpFilename, Item.m_aFilename, IStorage::TYPE_SAVE);

		// add item to menu list
		m_pClient->m_Menus.UpdateOwnGhost(Item);
	}
	else if(RecordingToFile) // no new record
		Storage()->RemoveFile(m_aTmpFilename, IStorage::TYPE_SAVE);

	m_aTmpFilename[0] = 0;

	m_CurGhost.Reset();
}

void CGhost::StartRender(int Tick)
{
	m_Rendering = true;
	m_StartRenderTick = Tick;
	for(auto &Ghost : m_aActiveGhosts)
		Ghost.m_PlaybackPos = 0;
}

void CGhost::StopRender()
{
	m_Rendering = false;
	m_NewRenderTick = -1;
}

int CGhost::Load(const char *pFilename)
{
	int Slot = GetSlot();
	if(Slot == -1)
		return -1;

	if(GhostLoader()->Load(pFilename, Client()->GetCurrentMap(), Client()->GetCurrentMapSha256(), Client()->GetCurrentMapCrc()) != 0)
		return -1;

	const CGhostInfo *pInfo = GhostLoader()->GetInfo();

	if(pInfo->m_NumTicks <= 0 || pInfo->m_Time <= 0)
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ghost", "invalid header info");
		GhostLoader()->Close();
		return -1;
	}

	// select ghost
	CGhostItem *pGhost = &m_aActiveGhosts[Slot];
	pGhost->Reset();
	pGhost->m_Path.SetSize(pInfo->m_NumTicks);

	str_copy(pGhost->m_aPlayer, pInfo->m_aOwner);

	int Index = 0;
	bool FoundSkin = false;
	bool NoTick = false;
	bool Error = false;

	int Type;
	while(!Error && GhostLoader()->ReadNextType(&Type))
	{
		if(Index == pInfo->m_NumTicks && (Type == GHOSTDATA_TYPE_CHARACTER || Type == GHOSTDATA_TYPE_CHARACTER_NO_TICK))
		{
			Error = true;
			break;
		}

		if(Type == GHOSTDATA_TYPE_SKIN && !FoundSkin)
		{
			FoundSkin = true;
			if(!GhostLoader()->ReadData(Type, &pGhost->m_Skin, sizeof(CGhostSkin)))
				Error = true;
		}
		else if(Type == GHOSTDATA_TYPE_CHARACTER_NO_TICK)
		{
			NoTick = true;
			if(!GhostLoader()->ReadData(Type, pGhost->m_Path.Get(Index++), sizeof(CGhostCharacter_NoTick)))
				Error = true;
		}
		else if(Type == GHOSTDATA_TYPE_CHARACTER)
		{
			if(!GhostLoader()->ReadData(Type, pGhost->m_Path.Get(Index++), sizeof(CGhostCharacter)))
				Error = true;
		}
		else if(Type == GHOSTDATA_TYPE_START_TICK)
		{
			if(!GhostLoader()->ReadData(Type, &pGhost->m_StartTick, sizeof(int)))
				Error = true;
		}
	}

	GhostLoader()->Close();

	if(Error || Index != pInfo->m_NumTicks)
	{
		Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "ghost", "invalid ghost data");
		pGhost->Reset();
		return -1;
	}

	if(NoTick)
	{
		int StartTick = 0;
		for(int i = 1; i < pInfo->m_NumTicks; i++) // estimate start tick
			if(pGhost->m_Path.Get(i)->m_AttackTick != pGhost->m_Path.Get(i - 1)->m_AttackTick)
				StartTick = pGhost->m_Path.Get(i)->m_AttackTick - i;
		for(int i = 0; i < pInfo->m_NumTicks; i++)
			pGhost->m_Path.Get(i)->m_Tick = StartTick + i;
	}

	if(pGhost->m_StartTick == -1)
		pGhost->m_StartTick = pGhost->m_Path.Get(0)->m_Tick;

	if(!FoundSkin)
		GetGhostSkin(&pGhost->m_Skin, "default", 0, 0, 0);
	InitRenderInfos(pGhost);

	return Slot;
}

void CGhost::Unload(int Slot)
{
	m_aActiveGhosts[Slot].Reset();
}

void CGhost::UnloadAll()
{
	for(int i = 0; i < MAX_ACTIVE_GHOSTS; i++)
		Unload(i);
}

void CGhost::SaveGhost(CMenus::CGhostItem *pItem)
{
	int Slot = pItem->m_Slot;
	if(!pItem->Active() || pItem->HasFile() || m_aActiveGhosts[Slot].Empty() || GhostRecorder()->IsRecording())
		return;

	CGhostItem *pGhost = &m_aActiveGhosts[Slot];

	int NumTicks = pGhost->m_Path.Size();
	GetPath(pItem->m_aFilename, sizeof(pItem->m_aFilename), pItem->m_aPlayer, pItem->m_Time);
	GhostRecorder()->Start(pItem->m_aFilename, Client()->GetCurrentMap(), Client()->GetCurrentMapSha256(), pItem->m_aPlayer);

	GhostRecorder()->WriteData(GHOSTDATA_TYPE_START_TICK, &pGhost->m_StartTick, sizeof(int));
	GhostRecorder()->WriteData(GHOSTDATA_TYPE_SKIN, &pGhost->m_Skin, sizeof(CGhostSkin));
	for(int i = 0; i < NumTicks; i++)
		GhostRecorder()->WriteData(GHOSTDATA_TYPE_CHARACTER, pGhost->m_Path.Get(i), sizeof(CGhostCharacter));

	GhostRecorder()->Stop(NumTicks, pItem->m_Time);
}

void CGhost::ConGPlay(IConsole::IResult *pResult, void *pUserData)
{
	CGhost *pGhost = (CGhost *)pUserData;
	pGhost->StartRender(pGhost->Client()->PredGameTick(g_Config.m_ClDummy));
}

void CGhost::OnConsoleInit()
{
	m_pGhostLoader = Kernel()->RequestInterface<IGhostLoader>();
	m_pGhostRecorder = Kernel()->RequestInterface<IGhostRecorder>();

	Console()->Register("gplay", "", CFGFLAG_CLIENT, ConGPlay, this, "");
}

void CGhost::OnMessage(int MsgType, void *pRawMsg)
{
	// check for messages from server
	if(MsgType == NETMSGTYPE_SV_KILLMSG)
	{
		CNetMsg_Sv_KillMsg *pMsg = (CNetMsg_Sv_KillMsg *)pRawMsg;
		if(pMsg->m_Victim == m_pClient->m_Snap.m_LocalClientID)
		{
			if(m_Recording)
				StopRecord();
			StopRender();
			m_LastDeathTick = Client()->GameTick(g_Config.m_ClDummy);
		}
	}
	else if(MsgType == NETMSGTYPE_SV_CHAT)
	{
		CNetMsg_Sv_Chat *pMsg = (CNetMsg_Sv_Chat *)pRawMsg;
		if(pMsg->m_ClientID == -1 && m_Recording)
		{
			char aName[MAX_NAME_LENGTH];
			int Time = CRaceHelper::TimeFromFinishMessage(pMsg->m_pMessage, aName, sizeof(aName));
			if(Time > 0 && m_pClient->m_Snap.m_LocalClientID >= 0 && str_comp(aName, m_pClient->m_aClients[m_pClient->m_Snap.m_LocalClientID].m_aName) == 0)
			{
				StopRecord(Time);
				StopRender();
			}
		}
	}
}

void CGhost::OnReset()
{
	StopRecord();
	StopRender();
	m_LastDeathTick = -1;
	m_LastRaceTick = -1;
}

void CGhost::OnShutdown()
{
	OnReset();
}

void CGhost::OnMapLoad()
{
	OnReset();
	UnloadAll();
	m_pClient->m_Menus.GhostlistPopulate();
	m_AllowRestart = false;
}

int CGhost::GetLastRaceTick()
{
	return m_LastRaceTick;
}

void CGhost::RefindSkin()
{
	char aSkinName[64];
	for(auto &Ghost : m_aActiveGhosts)
	{
		if(!Ghost.Empty())
		{
			IntsToStr(&Ghost.m_Skin.m_Skin0, 6, aSkinName);
			if(aSkinName[0] != '\0')
			{
				CTeeRenderInfo *pRenderInfo = &Ghost.m_RenderInfo;

				const CSkin *pSkin = m_pClient->m_Skins.Find(aSkinName);
				pRenderInfo->m_OriginalRenderSkin = pSkin->m_OriginalSkin;
				pRenderInfo->m_ColorableRenderSkin = pSkin->m_ColorableSkin;
				pRenderInfo->m_SkinMetrics = pSkin->m_Metrics;
			}
		}
	}
	if(!m_CurGhost.Empty())
	{
		IntsToStr(&m_CurGhost.m_Skin.m_Skin0, 6, aSkinName);
		if(aSkinName[0] != '\0')
		{
			CTeeRenderInfo *pRenderInfo = &m_CurGhost.m_RenderInfo;

			const CSkin *pSkin = m_pClient->m_Skins.Find(aSkinName);
			pRenderInfo->m_OriginalRenderSkin = pSkin->m_OriginalSkin;
			pRenderInfo->m_ColorableRenderSkin = pSkin->m_ColorableSkin;
			pRenderInfo->m_SkinMetrics = pSkin->m_Metrics;
		}
	}
}
