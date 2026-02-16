#pragma once

#include "../model/Types.h"

namespace model {

int controlledCellCount(const GameState &state, PlayerId owner);
int aliveAgentCount(const GameState &state, PlayerId owner);

GameStatus evaluateGameStatus(const GameState &state);
bool updateGameStatus(GameState &state);

} // namespace model

