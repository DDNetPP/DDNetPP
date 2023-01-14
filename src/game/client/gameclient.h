/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_GAMECLIENT_H
#define GAME_CLIENT_GAMECLIENT_H

#include "render.h"
#include <base/color.h>
#include <base/vmath.h>
#include <engine/client.h>
#include <engine/console.h>
#include <engine/shared/config.h>

#include <game/collision.h>
#include <game/gamecore.h>
#include <game/layers.h>
#include <game/teamscore.h>

#include <game/client/prediction/gameworld.h>

// components
#include "components/background.h"
#include "components/binds.h"
#include "components/broadcast.h"
#include "components/camera.h"
#include "components/chat.h"
#include "components/console.h"
#include "components/controls.h"
#include "components/countryflags.h"
#include "components/damageind.h"
#include "components/debughud.h"
#include "components/effects.h"
#include "components/emoticon.h"
#include "components/flow.h"
#include "components/freezebars.h"
#include "components/ghost.h"
#include "components/hud.h"
#include "components/items.h"
#include "components/killmessages.h"
#include "components/mapimages.h"
#include "components/maplayers.h"
#include "components/mapsounds.h"
#include "components/menu_background.h"
#include "components/menus.h"
#include "components/motd.h"
#include "components/nameplates.h"
#include "components/particles.h"
#include "components/players.h"
#include "components/race_demo.h"
#include "components/scoreboard.h"
#include "components/skins.h"
#include "components/sounds.h"
#include "components/spectator.h"
#include "components/statboard.h"
#include "components/tooltips.h"
#include "components/voting.h"

class CGameInfo
{
public:
	bool m_FlagStartsRace;
	bool m_TimeScore;
	bool m_UnlimitedAmmo;
	bool m_DDRaceRecordMessage;
	bool m_RaceRecordMessage;
	bool m_RaceSounds;

	bool m_AllowEyeWheel;
	bool m_AllowHookColl;
	bool m_AllowZoom;

	bool m_BugDDRaceGhost;
	bool m_BugDDRaceInput;
	bool m_BugFNGLaserRange;
	bool m_BugVanillaBounce;

	bool m_PredictFNG;
	bool m_PredictDDRace;
	bool m_PredictDDRaceTiles;
	bool m_PredictVanilla;

	bool m_EntitiesDDNet;
	bool m_EntitiesDDRace;
	bool m_EntitiesRace;
	bool m_EntitiesFNG;
	bool m_EntitiesVanilla;
	bool m_EntitiesBW;
	bool m_EntitiesFDDrace;

	bool m_Race;

	bool m_DontMaskEntities;
	bool m_AllowXSkins;

	bool m_HudHealthArmor;
	bool m_HudAmmo;
	bool m_HudDDRace;

	bool m_NoWeakHookAndBounce;
};

class CSnapEntities
{
public:
	IClient::CSnapItem m_Item;
	const void *m_pData;
	const CNetObj_EntityEx *m_pDataEx;
};

class CGameClient : public IGameClient
{
public:
	// all components
	CKillMessages m_KillMessages;
	CCamera m_Camera;
	CChat m_Chat;
	CMotd m_Motd;
	CBroadcast m_Broadcast;
	CGameConsole m_GameConsole;
	CBinds m_Binds;
	CParticles m_Particles;
	CMenus m_Menus;
	CSkins m_Skins;
	CCountryFlags m_CountryFlags;
	CFlow m_Flow;
	CHud m_Hud;
	CDebugHud m_DebugHud;
	CControls m_Controls;
	CEffects m_Effects;
	CScoreboard m_Scoreboard;
	CStatboard m_Statboard;
	CSounds m_Sounds;
	CEmoticon m_Emoticon;
	CDamageInd m_DamageInd;
	CVoting m_Voting;
	CSpectator m_Spectator;

	CPlayers m_Players;
	CNamePlates m_NamePlates;
	CFreezeBars m_FreezeBars;
	CItems m_Items;
	CMapImages m_MapImages;

	CMapLayers m_MapLayersBackGround = CMapLayers{CMapLayers::TYPE_BACKGROUND};
	CMapLayers m_MapLayersForeGround = CMapLayers{CMapLayers::TYPE_FOREGROUND};
	CBackground m_BackGround;
	CMenuBackground m_MenuBackground;

	CMapSounds m_MapSounds;

	CRaceDemo m_RaceDemo;
	CGhost m_Ghost;

	CTooltips m_Tooltips;

private:
	std::vector<class CComponent *> m_vpAll;
	std::vector<class CComponent *> m_vpInput;
	CNetObjHandler m_NetObjHandler;

	class IEngine *m_pEngine;
	class IInput *m_pInput;
	class IGraphics *m_pGraphics;
	class ITextRender *m_pTextRender;
	class IClient *m_pClient;
	class ISound *m_pSound;
	class CConfig *m_pConfig;
	class IConsole *m_pConsole;
	class IStorage *m_pStorage;
	class IDemoPlayer *m_pDemoPlayer;
	class IFavorites *m_pFavorites;
	class IServerBrowser *m_pServerBrowser;
	class IEditor *m_pEditor;
	class IFriends *m_pFriends;
	class IFriends *m_pFoes;
#if defined(CONF_AUTOUPDATE)
	class IUpdater *m_pUpdater;
#endif

	CLayers m_Layers;
	CCollision m_Collision;
	CUI m_UI;

	void ProcessEvents();
	void UpdatePositions();

	int m_PredictedTick;
	int m_aLastNewPredictedTick[NUM_DUMMIES];

	int m_LastRoundStartTick;

	int m_LastFlagCarrierRed;
	int m_LastFlagCarrierBlue;

	int m_aCheckInfo[NUM_DUMMIES];

	char m_aDDNetVersionStr[64];

	static void ConTeam(IConsole::IResult *pResult, void *pUserData);
	static void ConKill(IConsole::IResult *pResult, void *pUserData);

	static void ConchainSpecialInfoupdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainSpecialDummyInfoupdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainSpecialDummy(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainClTextEntitiesSize(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);

	static void ConTuneZone(IConsole::IResult *pResult, void *pUserData);

	static void ConchainMenuMap(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);

public:
	IKernel *Kernel() { return IInterface::Kernel(); }
	IEngine *Engine() const { return m_pEngine; }
	class IGraphics *Graphics() const { return m_pGraphics; }
	class IClient *Client() const { return m_pClient; }
	class CUI *UI() { return &m_UI; }
	class ISound *Sound() const { return m_pSound; }
	class IInput *Input() const { return m_pInput; }
	class IStorage *Storage() const { return m_pStorage; }
	class CConfig *Config() const { return m_pConfig; }
	class IConsole *Console() { return m_pConsole; }
	class ITextRender *TextRender() const { return m_pTextRender; }
	class IDemoPlayer *DemoPlayer() const { return m_pDemoPlayer; }
	class IDemoRecorder *DemoRecorder(int Recorder) const { return Client()->DemoRecorder(Recorder); }
	class IFavorites *Favorites() const { return m_pFavorites; }
	class IServerBrowser *ServerBrowser() const { return m_pServerBrowser; }
	class CRenderTools *RenderTools() { return &m_RenderTools; }
	class CLayers *Layers() { return &m_Layers; }
	CCollision *Collision() { return &m_Collision; }
	class IEditor *Editor() { return m_pEditor; }
	class IFriends *Friends() { return m_pFriends; }
	class IFriends *Foes() { return m_pFoes; }
#if defined(CONF_AUTOUPDATE)
	class IUpdater *Updater()
	{
		return m_pUpdater;
	}
#endif

	int NetobjNumCorrections()
	{
		return m_NetObjHandler.NumObjCorrections();
	}
	const char *NetobjCorrectedOn() { return m_NetObjHandler.CorrectedObjOn(); }

	bool m_SuppressEvents;
	bool m_NewTick;
	bool m_NewPredictedTick;
	int m_aFlagDropTick[2];

	// TODO: move this
	CTuningParams m_aTuning[NUM_DUMMIES];

	enum
	{
		SERVERMODE_PURE = 0,
		SERVERMODE_MOD,
		SERVERMODE_PUREMOD,
	};
	int m_ServerMode;
	CGameInfo m_GameInfo;

	int m_DemoSpecID;

	vec2 m_LocalCharacterPos;

	// predicted players
	CCharacterCore m_PredictedPrevChar;
	CCharacterCore m_PredictedChar;

	// snap pointers
	struct CSnapState
	{
		const CNetObj_Character *m_pLocalCharacter;
		const CNetObj_Character *m_pLocalPrevCharacter;
		const CNetObj_PlayerInfo *m_pLocalInfo;
		const CNetObj_SpectatorInfo *m_pSpectatorInfo;
		const CNetObj_SpectatorInfo *m_pPrevSpectatorInfo;
		const CNetObj_Flag *m_apFlags[2];
		const CNetObj_GameInfo *m_pGameInfoObj;
		const CNetObj_GameData *m_pGameDataObj;
		int m_GameDataSnapID;

		const CNetObj_PlayerInfo *m_apPlayerInfos[MAX_CLIENTS];
		const CNetObj_PlayerInfo *m_apInfoByScore[MAX_CLIENTS];
		const CNetObj_PlayerInfo *m_apInfoByName[MAX_CLIENTS];
		const CNetObj_PlayerInfo *m_apInfoByDDTeamScore[MAX_CLIENTS];
		const CNetObj_PlayerInfo *m_apInfoByDDTeamName[MAX_CLIENTS];

		int m_LocalClientID;
		int m_NumPlayers;
		int m_aTeamSize[2];

		// spectate data
		struct CSpectateInfo
		{
			bool m_Active;
			int m_SpectatorID;
			bool m_UsePosition;
			vec2 m_Position;
		} m_SpecInfo;

		//
		struct CCharacterInfo
		{
			bool m_Active;

			// snapshots
			CNetObj_Character m_Prev;
			CNetObj_Character m_Cur;

			CNetObj_DDNetCharacter m_ExtendedData;
			const CNetObj_DDNetCharacter *m_PrevExtendedData;
			bool m_HasExtendedData;
			bool m_HasExtendedDisplayInfo;

			// interpolated position
			vec2 m_Position;
		};

		CCharacterInfo m_aCharacters[MAX_CLIENTS];
	};

	CSnapState m_Snap;
	int m_aLocalTuneZone[NUM_DUMMIES];
	bool m_aReceivedTuning[NUM_DUMMIES];
	int m_aExpectingTuningForZone[NUM_DUMMIES];
	int m_aExpectingTuningSince[NUM_DUMMIES];

	// client data
	struct CClientData
	{
		int m_UseCustomColor;
		int m_ColorBody;
		int m_ColorFeet;

		char m_aName[MAX_NAME_LENGTH];
		char m_aClan[MAX_CLAN_LENGTH];
		int m_Country;
		char m_aSkinName[64];
		int m_SkinColor;
		int m_Team;
		int m_Emoticon;
		float m_EmoticonStartFraction;
		int m_EmoticonStartTick;
		bool m_Solo;
		bool m_Jetpack;
		bool m_CollisionDisabled;
		bool m_EndlessHook;
		bool m_EndlessJump;
		bool m_HammerHitDisabled;
		bool m_GrenadeHitDisabled;
		bool m_LaserHitDisabled;
		bool m_ShotgunHitDisabled;
		bool m_HookHitDisabled;
		bool m_Super;
		bool m_HasTelegunGun;
		bool m_HasTelegunGrenade;
		bool m_HasTelegunLaser;
		int m_FreezeEnd;
		bool m_DeepFrozen;
		bool m_LiveFrozen;

		CCharacterCore m_Predicted;
		CCharacterCore m_PrevPredicted;

		CTeeRenderInfo m_SkinInfo; // this is what the server reports
		CTeeRenderInfo m_RenderInfo; // this is what we use

		float m_Angle;
		bool m_Active;
		bool m_ChatIgnore;
		bool m_EmoticonIgnore;
		bool m_Friend;
		bool m_Foe;

		int m_AuthLevel;
		bool m_Afk;
		bool m_Paused;
		bool m_Spec;

		// Editor allows 256 switches for now.
		bool m_aSwitchStates[256];

		CNetObj_Character m_Snapped;
		CNetObj_Character m_Evolved;

		void UpdateRenderInfo(bool IsTeamPlay);
		void Reset();

		// rendered characters
		CNetObj_Character m_RenderCur;
		CNetObj_Character m_RenderPrev;
		vec2 m_RenderPos;
		bool m_IsPredicted;
		bool m_IsPredictedLocal;
		int64_t m_aSmoothStart[2];
		int64_t m_aSmoothLen[2];
		vec2 m_aPredPos[200];
		int m_aPredTick[200];
		bool m_SpecCharPresent;
		vec2 m_SpecChar;
	};

	CClientData m_aClients[MAX_CLIENTS];

	class CClientStats
	{
		int m_IngameTicks;
		int m_JoinTick;
		bool m_Active;

	public:
		CClientStats();

		int m_aFragsWith[NUM_WEAPONS];
		int m_aDeathsFrom[NUM_WEAPONS];
		int m_Frags;
		int m_Deaths;
		int m_Suicides;
		int m_BestSpree;
		int m_CurrentSpree;

		int m_FlagGrabs;
		int m_FlagCaptures;

		void Reset();

		bool IsActive() const { return m_Active; }
		void JoinGame(int Tick)
		{
			m_Active = true;
			m_JoinTick = Tick;
		};
		void JoinSpec(int Tick)
		{
			m_Active = false;
			m_IngameTicks += Tick - m_JoinTick;
		};
		int GetIngameTicks(int Tick) const { return m_IngameTicks + Tick - m_JoinTick; }
		float GetFPM(int Tick, int TickSpeed) const { return (float)(m_Frags * TickSpeed * 60) / GetIngameTicks(Tick); }
	};

	CClientStats m_aStats[MAX_CLIENTS];

	CRenderTools m_RenderTools;

	void OnReset();

	size_t ComponentCount() { return m_vpAll.size(); }

	// hooks
	void OnConnected() override;
	void OnRender() override;
	void OnUpdate() override;
	void OnDummyDisconnect() override;
	virtual void OnRelease();
	void OnInit() override;
	void OnConsoleInit() override;
	void OnStateChange(int NewState, int OldState) override;
	void OnMessage(int MsgId, CUnpacker *pUnpacker, int Conn, bool Dummy) override;
	void InvalidateSnapshot() override;
	void OnNewSnapshot() override;
	void OnPredict() override;
	void OnActivateEditor() override;
	void OnDummySwap() override;
	int OnSnapInput(int *pData, bool Dummy, bool Force) override;
	void OnShutdown() override;
	void OnEnterGame() override;
	void OnRconType(bool UsernameReq) override;
	void OnRconLine(const char *pLine) override;
	virtual void OnGameOver();
	virtual void OnStartGame();
	virtual void OnStartRound();
	virtual void OnFlagGrab(int TeamID);

	void OnWindowResize();
	static void OnWindowResizeCB(void *pUser);

	void OnLanguageChange();

	const char *GetItemName(int Type) const override;
	const char *Version() const override;
	const char *NetVersion() const override;
	int DDNetVersion() const override;
	const char *DDNetVersionStr() const override;

	// actions
	// TODO: move these
	void SendSwitchTeam(int Team);
	void SendInfo(bool Start);
	void SendDummyInfo(bool Start) override;
	void SendKill(int ClientID);

	// DDRace

	int m_aLocalIDs[NUM_DUMMIES];
	CNetObj_PlayerInput m_DummyInput;
	CNetObj_PlayerInput m_HammerInput;
	unsigned int m_DummyFire;
	bool m_ReceivedDDNetPlayer;

	class CTeamsCore m_Teams;

	int IntersectCharacter(vec2 HookPos, vec2 NewPos, vec2 &NewPos2, int ownID);

	int GetLastRaceTick() override;

	bool IsTeamPlay() { return m_Snap.m_pGameInfoObj && m_Snap.m_pGameInfoObj->m_GameFlags & GAMEFLAG_TEAMS; }

	bool AntiPingPlayers() { return g_Config.m_ClAntiPing && g_Config.m_ClAntiPingPlayers && !m_Snap.m_SpecInfo.m_Active && Client()->State() != IClient::STATE_DEMOPLAYBACK && (m_aTuning[g_Config.m_ClDummy].m_PlayerCollision || m_aTuning[g_Config.m_ClDummy].m_PlayerHooking); }
	bool AntiPingGrenade() { return g_Config.m_ClAntiPing && g_Config.m_ClAntiPingGrenade && !m_Snap.m_SpecInfo.m_Active && Client()->State() != IClient::STATE_DEMOPLAYBACK; }
	bool AntiPingWeapons() { return g_Config.m_ClAntiPing && g_Config.m_ClAntiPingWeapons && !m_Snap.m_SpecInfo.m_Active && Client()->State() != IClient::STATE_DEMOPLAYBACK; }
	bool AntiPingGunfire() { return AntiPingGrenade() && AntiPingWeapons() && g_Config.m_ClAntiPingGunfire; }
	bool Predict() { return g_Config.m_ClPredict && !(m_Snap.m_pGameInfoObj && m_Snap.m_pGameInfoObj->m_GameStateFlags & GAMESTATEFLAG_GAMEOVER) && !m_Snap.m_SpecInfo.m_Active && Client()->State() != IClient::STATE_DEMOPLAYBACK && m_Snap.m_pLocalCharacter; }
	bool PredictDummy() { return g_Config.m_ClPredictDummy && Client()->DummyConnected() && m_Snap.m_LocalClientID >= 0 && m_PredictedDummyID >= 0 && !m_aClients[m_PredictedDummyID].m_Paused; }
	CTuningParams GetTunes(int i) { return m_aTuningList[i]; }

	CGameWorld m_GameWorld;
	CGameWorld m_PredictedWorld;
	CGameWorld m_PrevPredictedWorld;

	std::vector<SSwitchers> &Switchers() { return m_GameWorld.m_Core.m_vSwitchers; }
	std::vector<SSwitchers> &PredSwitchers() { return m_PredictedWorld.m_Core.m_vSwitchers; }

	void DummyResetInput() override;
	void Echo(const char *pString) override;
	bool IsOtherTeam(int ClientID);
	int SwitchStateTeam();
	bool IsLocalCharSuper();
	bool CanDisplayWarning() override;
	bool IsDisplayingWarning() override;
	CNetObjHandler *GetNetObjHandler() override;

	void LoadGameSkin(const char *pPath, bool AsDir = false);
	void LoadEmoticonsSkin(const char *pPath, bool AsDir = false);
	void LoadParticlesSkin(const char *pPath, bool AsDir = false);
	void LoadHudSkin(const char *pPath, bool AsDir = false);
	void LoadExtrasSkin(const char *pPath, bool AsDir = false);

	void RefindSkins();

	struct SClientGameSkin
	{
		// health armor hud
		IGraphics::CTextureHandle m_SpriteHealthFull;
		IGraphics::CTextureHandle m_SpriteHealthEmpty;
		IGraphics::CTextureHandle m_SpriteArmorFull;
		IGraphics::CTextureHandle m_SpriteArmorEmpty;

		// cursors
		IGraphics::CTextureHandle m_SpriteWeaponHammerCursor;
		IGraphics::CTextureHandle m_SpriteWeaponGunCursor;
		IGraphics::CTextureHandle m_SpriteWeaponShotgunCursor;
		IGraphics::CTextureHandle m_SpriteWeaponGrenadeCursor;
		IGraphics::CTextureHandle m_SpriteWeaponNinjaCursor;
		IGraphics::CTextureHandle m_SpriteWeaponLaserCursor;

		IGraphics::CTextureHandle m_aSpriteWeaponCursors[6];

		// weapons and hook
		IGraphics::CTextureHandle m_SpriteHookChain;
		IGraphics::CTextureHandle m_SpriteHookHead;
		IGraphics::CTextureHandle m_SpriteWeaponHammer;
		IGraphics::CTextureHandle m_SpriteWeaponGun;
		IGraphics::CTextureHandle m_SpriteWeaponShotgun;
		IGraphics::CTextureHandle m_SpriteWeaponGrenade;
		IGraphics::CTextureHandle m_SpriteWeaponNinja;
		IGraphics::CTextureHandle m_SpriteWeaponLaser;

		IGraphics::CTextureHandle m_aSpriteWeapons[6];

		// particles
		IGraphics::CTextureHandle m_aSpriteParticles[9];

		// stars
		IGraphics::CTextureHandle m_aSpriteStars[3];

		// projectiles
		IGraphics::CTextureHandle m_SpriteWeaponGunProjectile;
		IGraphics::CTextureHandle m_SpriteWeaponShotgunProjectile;
		IGraphics::CTextureHandle m_SpriteWeaponGrenadeProjectile;
		IGraphics::CTextureHandle m_SpriteWeaponHammerProjectile;
		IGraphics::CTextureHandle m_SpriteWeaponNinjaProjectile;
		IGraphics::CTextureHandle m_SpriteWeaponLaserProjectile;

		IGraphics::CTextureHandle m_aSpriteWeaponProjectiles[6];

		// muzzles
		IGraphics::CTextureHandle m_aSpriteWeaponGunMuzzles[3];
		IGraphics::CTextureHandle m_aSpriteWeaponShotgunMuzzles[3];
		IGraphics::CTextureHandle m_aaSpriteWeaponNinjaMuzzles[3];

		IGraphics::CTextureHandle m_aaSpriteWeaponsMuzzles[6][3];

		// pickups
		IGraphics::CTextureHandle m_SpritePickupHealth;
		IGraphics::CTextureHandle m_SpritePickupArmor;
		IGraphics::CTextureHandle m_SpritePickupArmorShotgun;
		IGraphics::CTextureHandle m_SpritePickupArmorGrenade;
		IGraphics::CTextureHandle m_SpritePickupArmorNinja;
		IGraphics::CTextureHandle m_SpritePickupArmorLaser;
		IGraphics::CTextureHandle m_SpritePickupGrenade;
		IGraphics::CTextureHandle m_SpritePickupShotgun;
		IGraphics::CTextureHandle m_SpritePickupLaser;
		IGraphics::CTextureHandle m_SpritePickupNinja;
		IGraphics::CTextureHandle m_SpritePickupGun;
		IGraphics::CTextureHandle m_SpritePickupHammer;

		IGraphics::CTextureHandle m_aSpritePickupWeapons[6];
		IGraphics::CTextureHandle m_aSpritePickupWeaponArmor[4];

		// flags
		IGraphics::CTextureHandle m_SpriteFlagBlue;
		IGraphics::CTextureHandle m_SpriteFlagRed;

		// ninja bar (0.7)
		IGraphics::CTextureHandle m_SpriteNinjaBarFullLeft;
		IGraphics::CTextureHandle m_SpriteNinjaBarFull;
		IGraphics::CTextureHandle m_SpriteNinjaBarEmpty;
		IGraphics::CTextureHandle m_SpriteNinjaBarEmptyRight;

		bool IsSixup()
		{
			return m_SpriteNinjaBarFullLeft.IsValid();
		}
	};

	SClientGameSkin m_GameSkin;
	bool m_GameSkinLoaded;

	struct SClientParticlesSkin
	{
		IGraphics::CTextureHandle m_SpriteParticleSlice;
		IGraphics::CTextureHandle m_SpriteParticleBall;
		IGraphics::CTextureHandle m_aSpriteParticleSplat[3];
		IGraphics::CTextureHandle m_SpriteParticleSmoke;
		IGraphics::CTextureHandle m_SpriteParticleShell;
		IGraphics::CTextureHandle m_SpriteParticleExpl;
		IGraphics::CTextureHandle m_SpriteParticleAirJump;
		IGraphics::CTextureHandle m_SpriteParticleHit;
		IGraphics::CTextureHandle m_aSpriteParticles[10];
	};

	SClientParticlesSkin m_ParticlesSkin;
	bool m_ParticlesSkinLoaded;

	struct SClientEmoticonsSkin
	{
		IGraphics::CTextureHandle m_aSpriteEmoticons[16];
	};

	SClientEmoticonsSkin m_EmoticonsSkin;
	bool m_EmoticonsSkinLoaded;

	struct SClientHudSkin
	{
		IGraphics::CTextureHandle m_SpriteHudAirjump;
		IGraphics::CTextureHandle m_SpriteHudAirjumpEmpty;
		IGraphics::CTextureHandle m_SpriteHudSolo;
		IGraphics::CTextureHandle m_SpriteHudCollisionDisabled;
		IGraphics::CTextureHandle m_SpriteHudEndlessJump;
		IGraphics::CTextureHandle m_SpriteHudEndlessHook;
		IGraphics::CTextureHandle m_SpriteHudJetpack;
		IGraphics::CTextureHandle m_SpriteHudFreezeBarFullLeft;
		IGraphics::CTextureHandle m_SpriteHudFreezeBarFull;
		IGraphics::CTextureHandle m_SpriteHudFreezeBarEmpty;
		IGraphics::CTextureHandle m_SpriteHudFreezeBarEmptyRight;
		IGraphics::CTextureHandle m_SpriteHudNinjaBarFullLeft;
		IGraphics::CTextureHandle m_SpriteHudNinjaBarFull;
		IGraphics::CTextureHandle m_SpriteHudNinjaBarEmpty;
		IGraphics::CTextureHandle m_SpriteHudNinjaBarEmptyRight;
		IGraphics::CTextureHandle m_SpriteHudHookHitDisabled;
		IGraphics::CTextureHandle m_SpriteHudHammerHitDisabled;
		IGraphics::CTextureHandle m_SpriteHudShotgunHitDisabled;
		IGraphics::CTextureHandle m_SpriteHudGrenadeHitDisabled;
		IGraphics::CTextureHandle m_SpriteHudLaserHitDisabled;
		IGraphics::CTextureHandle m_SpriteHudGunHitDisabled;
		IGraphics::CTextureHandle m_SpriteHudDeepFrozen;
		IGraphics::CTextureHandle m_SpriteHudLiveFrozen;
		IGraphics::CTextureHandle m_SpriteHudTeleportGrenade;
		IGraphics::CTextureHandle m_SpriteHudTeleportGun;
		IGraphics::CTextureHandle m_SpriteHudTeleportLaser;
		IGraphics::CTextureHandle m_SpriteHudPracticeMode;
		IGraphics::CTextureHandle m_SpriteHudDummyHammer;
		IGraphics::CTextureHandle m_SpriteHudDummyCopy;
	};

	SClientHudSkin m_HudSkin;
	bool m_HudSkinLoaded;

	struct SClientExtrasSkin
	{
		IGraphics::CTextureHandle m_SpriteParticleSnowflake;
		IGraphics::CTextureHandle m_aSpriteParticles[1];
	};

	SClientExtrasSkin m_ExtrasSkin;
	bool m_ExtrasSkinLoaded;

	const std::vector<CSnapEntities> &SnapEntities() { return m_vSnapEntities; }

private:
	std::vector<CSnapEntities> m_vSnapEntities;
	void SnapCollectEntities();

	bool m_aDDRaceMsgSent[NUM_DUMMIES];
	int m_aShowOthers[NUM_DUMMIES];

	void UpdatePrediction();
	void UpdateRenderedCharacters();
	void DetectStrongHook();
	vec2 GetSmoothPos(int ClientID);

	int m_PredictedDummyID;
	int m_IsDummySwapping;
	CCharOrder m_CharOrder;
	int m_aSwitchStateTeam[NUM_DUMMIES];

	enum
	{
		NUM_TUNEZONES = 256
	};
	void LoadMapSettings();
	CTuningParams m_aTuningList[NUM_TUNEZONES];
	CTuningParams *TuningList() { return m_aTuningList; }

	float m_LastZoom;
	float m_LastScreenAspect;
	bool m_LastDummyConnected;
};

ColorRGBA CalculateNameColor(ColorHSLA TextColorHSL);

#endif
