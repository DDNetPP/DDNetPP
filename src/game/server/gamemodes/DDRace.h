/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
#ifndef GAME_SERVER_GAMEMODES_DDRACE_H
#define GAME_SERVER_GAMEMODES_DDRACE_H
#include <game/server/entities/door.h>
#include <game/server/gamecontroller.h>
#include <game/server/teams.h>

#include <map>
#include <vector>

struct CScoreInitResult;
class CGameControllerDDRace : public IGameController
{
public:
	bool m_IsGrounded;

	class CFlag *m_apFlags[2];

	virtual void Snap(int SnappingClient);

	virtual bool OnEntity(int Index, vec2 Pos);
	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);

	void DropFlag(int id, int dir = 1);
	void ChangeFlagOwner(int id, int character);
	int HasFlag(CCharacter *character);

	CGameControllerDDRace(class CGameContext *pGameServer);
	~CGameControllerDDRace();

	CGameTeams m_Teams;

	std::map<int, std::vector<vec2>> m_TeleOuts;
	std::map<int, std::vector<vec2>> m_TeleCheckOuts;

	void InitTeleporter();
	virtual void Tick();

	std::shared_ptr<CScoreInitResult> m_pInitResult;
};
#endif // GAME_SERVER_GAMEMODES_DDRACE_H
