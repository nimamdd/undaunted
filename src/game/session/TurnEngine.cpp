#include "TurnEngine.h"

namespace model {

void TurnEngine::resetForBattle()
{
}

bool TurnEngine::canUsePrimaryAction(const GameState &state, QString &errorMessage) const
{
    if (state.status != GameStatus::InProgress) {
        errorMessage = QStringLiteral("Game is already finished.");
        return false;
    }

    if (!state.turn.hasActiveCard) {
        errorMessage = QStringLiteral("No active card for current turn.");
        return false;
    }

    return true;
}

} // namespace model
