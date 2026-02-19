#include "TurnEngine.h"

namespace model {

void TurnEngine::resetForBattle()
{
    actionUsed_ = false;
}

bool TurnEngine::actionUsedThisTurn() const
{
    return actionUsed_;
}

void TurnEngine::markActionUsed()
{
    actionUsed_ = true;
}

void TurnEngine::onTurnEnded()
{
    actionUsed_ = false;
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

    if (actionUsed_) {
        errorMessage = QStringLiteral("You already used an action this turn.");
        return false;
    }

    return true;
}

bool TurnEngine::canSwitchAgent(const GameState &state, QString &errorMessage) const
{
    if (state.status != GameStatus::InProgress) {
        errorMessage = QStringLiteral("Game is already finished.");
        return false;
    }

    if (!state.turn.hasActiveCard) {
        errorMessage = QStringLiteral("No active turn card to switch.");
        return false;
    }

    if (actionUsed_) {
        errorMessage = QStringLiteral("You cannot switch agent after using an action.");
        return false;
    }

    return true;
}

bool TurnEngine::canEndTurn(const GameState &state, QString &errorMessage) const
{
    if (state.status != GameStatus::InProgress) {
        errorMessage = QStringLiteral("Game is already finished.");
        return false;
    }

    if (!state.turn.hasActiveCard) {
        errorMessage = QStringLiteral("No active turn card to finish the turn.");
        return false;
    }

    if (!actionUsed_) {
        errorMessage = QStringLiteral("You must perform an action before ending the turn.");
        return false;
    }

    return true;
}

} // namespace model
