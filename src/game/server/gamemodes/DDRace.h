/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_DDRACE_H
#define GAME_SERVER_GAMEMODES_DDRACE_H

#include <game/server/gamecontroller.h>

class CGameControllerDDRace : public IGameController
{
public:
	// ddnet++
	bool m_IsGrounded;
	void FlagTick();
	void HandleCharacterTilesDDPP(class CCharacter *pChr, int m_TileIndex, int m_TileFIndex, int Tile1, int Tile2, int Tile3, int Tile4, int FTile1, int FTile2, int FTile3, int FTile4, int PlayerDDRaceState);
	void Snap(int SnappingClient) override;
	bool OnEntity(int Index, int x, int y, int Layer, int Flags, bool Initial, int Number) override;

	CGameControllerDDRace(class CGameContext *pGameServer);
	~CGameControllerDDRace();

	CScore *Score();

	void HandleCharacterTiles(class CCharacter *pChr, int MapIndex) override;
	void SetArmorProgress(CCharacter *pCharacer, int Progress) override;

	void OnPlayerConnect(class CPlayer *pPlayer) override;
	void OnPlayerDisconnect(class CPlayer *pPlayer, const char *pReason, bool Silent = false) override;

	void OnReset() override;

	void Tick() override;

	void DoTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg = true) override;
};
#endif // GAME_SERVER_GAMEMODES_DDRACE_H
