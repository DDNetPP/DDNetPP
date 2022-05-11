// DDNet** block tournaments

#ifndef GAME_SERVER_MINIGAMES_BLOCK_TOURNAMENT_H
#define GAME_SERVER_MINIGAMES_BLOCK_TOURNAMENT_H

#include "minigame_base.h"

class CBlockTournament : public CMinigame {
public:
    using CMinigame::CMinigame;

    void Tick() override;

    bool IsActive(int ClientID) override;

    // m_BlockTournaSpawnCounter
    int m_SpawnCounter; // is this generic enough for all games?
private:
    int m_Tick; // TODO: add minigame init and zero it there
};

#endif
