#include "Victory.h"

#include "../model/Init.h"

namespace model {

int controlledCellCount(const GameState &state, PlayerId owner)
{
    if (owner != PlayerId::A && owner != PlayerId::B) {
        return 0;
    }

    int count = 0;
    for (const auto &cell : state.board.cells) {
        if (cell->controlledBy == owner) {
            ++count;
        }
    }
    return count;
}

int aliveAgentCount(const GameState &state, PlayerId owner)
{
    const PlayerState *player = playerById(state, owner);
    if (player == nullptr) {
        return 0;
    }

    int alive = 0;
    for (const AgentState &agent : player->agents) {
        if (agent.alive) {
            ++alive;
        }
    }
    return alive;
}

GameStatus evaluateGameStatus(const GameState &state)
{
    const int controlA = controlledCellCount(state, PlayerId::A);
    const int controlB = controlledCellCount(state, PlayerId::B);
    if (controlA >= 7) {
        return GameStatus::WonByA;
    }
    if (controlB >= 7) {
        return GameStatus::WonByB;
    }

    const int aliveA = aliveAgentCount(state, PlayerId::A);
    const int aliveB = aliveAgentCount(state, PlayerId::B);
    if (aliveA == 0 && aliveB == 0) {
        return GameStatus::InProgress;
    }
    if (aliveB == 0) {
        return GameStatus::WonByA;
    }
    if (aliveA == 0) {
        return GameStatus::WonByB;
    }

    return GameStatus::InProgress;
}

bool updateGameStatus(GameState &state)
{
    const GameStatus next = evaluateGameStatus(state);
    const bool changed = (state.status != next);
    state.status = next;
    return changed;
}

} // namespace model

