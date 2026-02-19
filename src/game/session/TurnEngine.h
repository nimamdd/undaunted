#pragma once

#include "../model/Types.h"

namespace model {

class TurnEngine
{
public:
    void resetForBattle();

    bool actionUsedThisTurn() const;
    void markActionUsed();
    void onTurnEnded();

    bool canUsePrimaryAction(const GameState &state, QString &errorMessage) const;
    bool canEndTurn(const GameState &state, QString &errorMessage) const;

private:
    bool actionUsed_{false};
};

} // namespace model
