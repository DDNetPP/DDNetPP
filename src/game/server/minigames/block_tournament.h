// DDNet** block tournaments

#ifndef GAME_SERVER_MINIGAMES_BLOCK_TOURNAMENT_H
#define GAME_SERVER_MINIGAMES_BLOCK_TOURNAMENT_H

#include "minigame_base.h"

class CBlockTournament : public CMinigame {
public:
    using CMinigame::CMinigame;

    void Tick() override;
};

#endif
