#pragma once

#include "../model/Types.h"

namespace model {

struct AttackResult {
    bool executed{false};
    bool success{false};
    bool cardBurned{false};
    bool targetEliminated{false};

    int threshold{0};
    QVector<int> rolls;

    PlayerId attackerOwner{PlayerId::None};
    AgentType attackerType{AgentType::Scout};
    PlayerId targetOwner{PlayerId::None};
    AgentType targetType{AgentType::Scout};
    QString targetCellId;

    QString errorMessage;
};

bool canAttack(const GameState &state,
               PlayerId attackerOwner,
               AgentType attackerType,
               const QString &targetCellId,
               QString &errorMessage);

int computeAttackThreshold(const GameState &state,
                           PlayerId attackerOwner,
                           AgentType attackerType,
                           const QString &targetCellId,
                           QString &errorMessage);

AttackResult attack(GameState &state,
                    PlayerId attackerOwner,
                    AgentType attackerType,
                    const QString &targetCellId);

} // namespace model
