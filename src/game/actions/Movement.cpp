#include "Movement.h"

#include "../board/BoardGraph.h"
#include "../model/Init.h"
#include "../turn/TurnSystem.h"

#include <QStringList>

namespace model {

namespace {

bool isMarkedForOwner(const CellNode *cell, PlayerId owner)
{
    if (cell == nullptr) {
        return false;
    }
    if (owner == PlayerId::A) {
        return cell->markedByA;
    }
    if (owner == PlayerId::B) {
        return cell->markedByB;
    }
    return false;
}

bool isOccupied(const CellNode *cell)
{
    return cell != nullptr && (cell->occupantA.has_value() || cell->occupantB.has_value());
}

bool isNeighbor(const CellNode *from, const CellNode *to)
{
    if (from == nullptr || to == nullptr) {
        return false;
    }
    for (const CellNode *neighbor : from->neighbors) {
        if (neighbor == to) {
            return true;
        }
    }
    return false;
}

} // namespace

QVector<const CellNode *> movableNeighbors(const GameState &state, PlayerId owner, AgentType type)
{
    const PlayerState *player = playerById(state, owner);
    if (player == nullptr) {
        return {};
    }

    const AgentState *agent = findAgent(*player, type);
    if (agent == nullptr || !agent->alive || agent->cellId.isEmpty()) {
        return {};
    }

    const CellNode *from = findCell(state.board, agent->cellId);
    if (from == nullptr) {
        return {};
    }

    QVector<const CellNode *> out;
    for (const CellNode *neighbor : from->neighbors) {
        if (isOccupied(neighbor)) {
            continue;
        }

        if (type != AgentType::Scout && !isMarkedForOwner(neighbor, owner)) {
            continue;
        }

        out.push_back(neighbor);
    }
    return out;
}

bool canMoveAgent(const GameState &state, PlayerId owner, AgentType type, const QString &toCellId, QString &errorMessage)
{
    if (state.status != GameStatus::InProgress) {
        errorMessage = QStringLiteral("Game is already finished.");
        return false;
    }

    const PlayerState *player = playerById(state, owner);
    if (player == nullptr) {
        errorMessage = QStringLiteral("Invalid player.");
        return false;
    }

    const AgentState *agent = findAgent(*player, type);
    if (agent == nullptr) {
        errorMessage = QStringLiteral("Agent not found for player %1.")
                           .arg(playerIdName(owner));
        return false;
    }

    if (!agent->alive) {
        errorMessage = QStringLiteral("Agent %1 is not alive.")
                           .arg(agentTypeName(type));
        return false;
    }

    if (agent->cellId.isEmpty()) {
        errorMessage = QStringLiteral("Agent %1 is not placed on board.")
                           .arg(agentTypeName(type));
        return false;
    }

    const CellNode *from = findCell(state.board, agent->cellId);
    if (from == nullptr) {
        errorMessage = QStringLiteral("Agent source cell is invalid: %1")
                           .arg(agent->cellId);
        return false;
    }

    const CellNode *to = findCell(state.board, toCellId);
    if (to == nullptr) {
        errorMessage = QStringLiteral("Target cell is invalid: %1")
                           .arg(toCellId);
        return false;
    }

    if (from == to) {
        errorMessage = QStringLiteral("Target cell is same as current cell.");
        return false;
    }

    if (!isNeighbor(from, to)) {
        QStringList neighborIds;
        neighborIds.reserve(from->neighbors.size());
        for (const CellNode *neighbor : from->neighbors) {
            if (neighbor != nullptr) {
                neighborIds.push_back(neighbor->id);
            }
        }
        errorMessage = QStringLiteral("Target %1 is not adjacent to %2. Adjacent cells: %3")
                           .arg(toCellId, from->id, neighborIds.join(QStringLiteral(", ")));
        return false;
    }

    if (isOccupied(to)) {
        errorMessage = QStringLiteral("Target cell is occupied.");
        return false;
    }

    if (type != AgentType::Scout && !isMarkedForOwner(to, owner)) {
        errorMessage = QStringLiteral("%1 can only move to marked cells.")
                           .arg(agentTypeName(type));
        return false;
    }

    return true;
}

bool moveAgent(GameState &state, PlayerId owner, AgentType type, const QString &toCellId, QString &errorMessage)
{
    if (!canMoveAgent(state, owner, type, toCellId, errorMessage)) {
        return false;
    }

    PlayerState *player = playerById(state, owner);
    AgentState *agent = findAgent(*player, type);
    CellNode *from = findCell(state.board, agent->cellId);
    CellNode *to = findCell(state.board, toCellId);

    if (owner == PlayerId::A) {
        from->occupantA.reset();
        to->occupantA = type;
    } else if (owner == PlayerId::B) {
        from->occupantB.reset();
        to->occupantB = type;
    } else {
        errorMessage = QStringLiteral("Invalid player.");
        return false;
    }

    agent->cellId = toCellId;
    return true;
}

bool moveCurrentPlayerAgent(GameState &state, AgentType type, const QString &toCellId, QString &errorMessage)
{
    if (!state.turn.hasActiveCard) {
        errorMessage = QStringLiteral("No active card for current turn.");
        return false;
    }

    if (state.turn.activeCard.agent != type) {
        errorMessage = QStringLiteral("Current active card does not match selected agent.");
        return false;
    }

    return moveAgent(state, state.turn.currentPlayer, type, toCellId, errorMessage);
}

} // namespace model
