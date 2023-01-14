/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_DDRACE_H
#define GAME_SERVER_GAMEMODES_DDRACE_H

#include <game/server/gamecontroller.h>
#include <game/server/teams.h>

#include <map>
#include <vector>

struct CScoreLoadBestTimeResult;
class CGameControllerDDRace : public IGameController
{
public:
	// ddnet++
	bool m_IsGrounded;
	class CFlag *m_apFlags[2];
	void DropFlag(int id, int dir = 1);
	void ChangeFlagOwner(int id, int character);
	int HasFlag(CCharacter *pChr);
	void FlagTick();
	void Snap(int SnappingClient) override;
	bool OnEntity(int Index, int x, int y, int Layer, int Flags, bool Initial, int Number) override;
	int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon) override;

	CGameControllerDDRace(class CGameContext *pGameServer);
	~CGameControllerDDRace();

	CScore *Score();

	void OnCharacterSpawn(class CCharacter *pChr) override;
	void HandleCharacterTiles(class CCharacter *pChr, int MapIndex) override;
	void HandleCharacterTilesDDPP(class CCharacter *pChr, int m_TileIndex, int m_TileFIndex, int Tile1, int Tile2, int Tile3, int Tile4, int FTile1, int FTile2, int FTile3, int FTile4, int PlayerDDRaceState);

	void OnPlayerConnect(class CPlayer *pPlayer, bool Silent = false) override;
	void OnPlayerDisconnect(class CPlayer *pPlayer, const char *pReason, bool Silent = false) override;

	void OnReset() override;

	void Tick() override;

	void DoTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg = true) override;

	int64_t GetMaskForPlayerWorldEvent(int Asker, int ExceptID = -1) override;

	void InitTeleporter();

	int GetPlayerTeam(int ClientID) const;

	CGameTeams m_Teams;

	std::map<int, std::vector<vec2>> m_TeleOuts;
	std::map<int, std::vector<vec2>> m_TeleCheckOuts;

	std::shared_ptr<CScoreLoadBestTimeResult> m_pLoadBestTimeResult;
};
#endif // GAME_SERVER_GAMEMODES_DDRACE_H
