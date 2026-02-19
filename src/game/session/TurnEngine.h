#pragma once

#include "../model/Types.h"

namespace model {

class TurnEngine
{
public:
    void resetForBattle();

    bool canUsePrimaryAction(const GameState &state, QString &errorMessage) const;
};

} // namespace model
