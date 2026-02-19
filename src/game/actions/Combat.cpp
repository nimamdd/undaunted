#include "Combat.h"

#include "../agents/AgentBehavior.h"
#include "../board/BoardGraph.h"
#include "../model/Init.h"
#include "../rules/Victory.h"
#include "../turn/TurnSystem.h"

#include <QRandomGenerator>

namespace model {

namespace {

std::optional<AgentType> occupantForOwner(const CellNode *cell, PlayerId owner)
{
    if (cell == nullptr) {
        return std::nullopt;
    }
    if (owner == PlayerId::A) {
        return cell->occupantA;
    }
    if (owner == PlayerId::B) {
        return cell->occupantB;
    }
    return std::nullopt;
}

void clearCellOccupant(CellNode *cell, PlayerId owner)
{
    if (cell == nullptr) {
        return;
    }
    if (owner == PlayerId::A) {
        cell->occupantA.reset();
    } else if (owner == PlayerId::B) {
        cell->occupantB.reset();
    }
}

bool resolveTarget(const GameState &state,
                   PlayerId attackerOwner,
                   const QString &targetCellId,
                   PlayerId &targetOwner,
                   AgentType &targetType,
                   QString &errorMessage)
{
    const CellNode *targetCell = findCell(state.board, targetCellId);
    if (targetCell == nullptr) {
        errorMessage = QStringLiteral("Target cell is invalid: %1").arg(targetCellId);
        return false;
    }

    targetOwner = opponentOf(attackerOwner);
    const std::optional<AgentType> occ = occupantForOwner(targetCell, targetOwner);
    if (!occ.has_value()) {
        errorMessage = QStringLiteral("Target cell does not contain an enemy piece.");
        return false;
    }

    targetType = occ.value();
    return true;
}

} // namespace

bool canAttack(const GameState &state,
               PlayerId attackerOwner,
               AgentType attackerType,
               const QString &targetCellId,
               QString &errorMessage)
{
    if (behaviorFor(attackerType) == nullptr) {
        errorMessage = QStringLiteral("Unsupported attacker agent type.");
        return false;
    }

    if (state.status != GameStatus::InProgress) {
        errorMessage = QStringLiteral("Game is already finished.");
        return false;
    }

    const PlayerState *attacker = playerById(state, attackerOwner);
    if (attacker == nullptr) {
        errorMessage = QStringLiteral("Invalid attacker player.");
        return false;
    }

    const AgentState *agent = findAgent(*attacker, attackerType);
    if (agent == nullptr) {
        errorMessage = QStringLiteral("Attacker agent not found.");
        return false;
    }
    if (!agent->alive || agent->cellId.isEmpty()) {
        errorMessage = QStringLiteral("Attacker agent is not active on board.");
        return false;
    }

    PlayerId targetOwner = PlayerId::None;
    AgentType targetType = AgentType::Scout;
    if (!resolveTarget(state, attackerOwner, targetCellId, targetOwner, targetType, errorMessage)) {
        return false;
    }

    const QVector<const CellNode *> path = shortestPath(state.board, agent->cellId, targetCellId);
    if (path.isEmpty()) {
        errorMessage = QStringLiteral("No path found between attacker and target.");
        return false;
    }

    const PlayerState *targetPlayer = playerById(state, targetOwner);
    if (targetPlayer == nullptr) {
        errorMessage = QStringLiteral("Invalid target player.");
        return false;
    }
    if (countCards(*targetPlayer, targetType) <= 0) {
        errorMessage = QStringLiteral("Target agent has no cards left.");
        return false;
    }

    return true;
}

int computeAttackThreshold(const GameState &state,
                           PlayerId attackerOwner,
                           AgentType attackerType,
                           const QString &targetCellId,
                           QString &errorMessage)
{
    if (!canAttack(state, attackerOwner, attackerType, targetCellId, errorMessage)) {
        return 0;
    }

    const PlayerState *attacker = playerById(state, attackerOwner);
    const AgentState *attackerAgent = findAgent(*attacker, attackerType);

    PlayerId targetOwner = PlayerId::None;
    AgentType targetType = AgentType::Scout;
    if (!resolveTarget(state, attackerOwner, targetCellId, targetOwner, targetType, errorMessage)) {
        return 0;
    }

    const PlayerState *targetPlayer = playerById(state, targetOwner);
    const AgentState *targetAgent = findAgent(*targetPlayer, targetType);
    if (targetAgent == nullptr) {
        errorMessage = QStringLiteral("Target agent state not found.");
        return 0;
    }

    const QVector<const CellNode *> path = shortestPath(state.board, attackerAgent->cellId, targetCellId);
    const int shieldSum = pathShieldSum(path, true);
    int threshold = shieldSum + targetAgent->hp;
    if (threshold > 10) {
        threshold = 10;
    }
    if (threshold < 1) {
        threshold = 1;
    }
    return threshold;
}

AttackResult attack(GameState &state,
                    PlayerId attackerOwner,
                    AgentType attackerType,
                    const QString &targetCellId)
{
    AttackResult result;
    result.attackerOwner = attackerOwner;
    result.attackerType = attackerType;
    result.targetCellId = targetCellId;

    QString error;
    if (!canAttack(state, attackerOwner, attackerType, targetCellId, error)) {
        result.errorMessage = error;
        return result;
    }

    PlayerId targetOwner = PlayerId::None;
    AgentType targetType = AgentType::Scout;
    if (!resolveTarget(state, attackerOwner, targetCellId, targetOwner, targetType, error)) {
        result.errorMessage = error;
        return result;
    }

    const int threshold = computeAttackThreshold(state, attackerOwner, attackerType, targetCellId, error);
    if (threshold <= 0) {
        result.errorMessage = error.isEmpty() ? QStringLiteral("Failed to compute attack threshold.") : error;
        return result;
    }

    result.threshold = threshold;
    result.targetOwner = targetOwner;
    result.targetType = targetType;

    const AgentBehavior *behavior = behaviorFor(attackerType);
    if (behavior == nullptr) {
        result.errorMessage = QStringLiteral("Unsupported attacker agent type.");
        return result;
    }

    const int diceCount = behavior->attackDiceCount();
    result.rolls.reserve(diceCount);
    bool success = false;

    auto *rng = QRandomGenerator::global();
    for (int i = 0; i < diceCount; ++i) {
        const int roll = rng->bounded(1, 11);
        result.rolls.push_back(roll);
        if (roll >= threshold) {
            success = true;
        }
    }

    result.executed = true;
    result.success = success;
    if (!success) {
        return result;
    }

    PlayerState *targetPlayer = playerById(state, targetOwner);
    QString burnError;
    if (!burnOneCard(*targetPlayer, targetType, burnError)) {
        result.errorMessage = burnError;
        return result;
    }
    result.cardBurned = true;

    AgentState *targetAgent = findAgent(*targetPlayer, targetType);
    if (targetAgent != nullptr && targetAgent->hp > 0) {
        targetAgent->hp -= 1;
    }

    if (countCards(*targetPlayer, targetType) == 0) {
        CellNode *targetCell = findCell(state.board, targetCellId);
        if (targetAgent != nullptr) {
            targetAgent->alive = false;
            targetAgent->cellId.clear();
            targetAgent->hp = 0;
        }
        clearCellOccupant(targetCell, targetOwner);
        result.targetEliminated = true;
    }

    updateGameStatus(state);
    return result;
}

} // namespace model
