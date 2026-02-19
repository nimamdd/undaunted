#include "TacticalActions.h"

#include "../board/BoardGraph.h"
#include "../model/Init.h"
#include "../rules/Victory.h"

namespace model {

namespace {

bool hasEnemyOnCell(const CellNode *cell, PlayerId owner)
{
    if (cell == nullptr) {
        return false;
    }
    if (owner == PlayerId::A) {
        return cell->occupantB.has_value();
    }
    if (owner == PlayerId::B) {
        return cell->occupantA.has_value();
    }
    return false;
}

bool isMarkedByOwner(const CellNode *cell, PlayerId owner)
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

bool validateAgentReady(const GameState &state,
                        PlayerId owner,
                        AgentType type,
                        const AgentState *&agentOut,
                        CellNode const *&cellOut,
                        QString &errorMessage)
{
    const PlayerState *player = playerById(state, owner);
    if (player == nullptr) {
        errorMessage = QStringLiteral("Invalid player.");
        return false;
    }

    const AgentState *agent = findAgent(*player, type);
    if (agent == nullptr) {
        errorMessage = QStringLiteral("%1 not found for player %2.")
                           .arg(agentTypeName(type), playerIdName(owner));
        return false;
    }
    if (!agent->alive) {
        errorMessage = QStringLiteral("%1 is not alive.").arg(agentTypeName(type));
        return false;
    }
    if (agent->cellId.isEmpty()) {
        errorMessage = QStringLiteral("%1 is not placed on board.").arg(agentTypeName(type));
        return false;
    }

    const CellNode *cell = findCell(state.board, agent->cellId);
    if (cell == nullptr) {
        errorMessage = QStringLiteral("Agent cell is invalid: %1").arg(agent->cellId);
        return false;
    }

    agentOut = agent;
    cellOut = cell;
    return true;
}

} // namespace

bool canScoutMarkInternal(const GameState &state, PlayerId owner, QString &errorMessage)
{
    if (state.status != GameStatus::InProgress) {
        errorMessage = QStringLiteral("Game is already finished.");
        return false;
    }

    const AgentState *scout = nullptr;
    const CellNode *cell = nullptr;
    if (!validateAgentReady(state, owner, AgentType::Scout, scout, cell, errorMessage)) {
        return false;
    }

    Q_UNUSED(scout);
    if (isMarkedByOwner(cell, owner)) {
        errorMessage = QStringLiteral("Current cell is already marked.");
        return false;
    }

    return true;
}

bool scoutMark(GameState &state, PlayerId owner, QString &errorMessage)
{
    if (!canScoutMarkInternal(state, owner, errorMessage)) {
        return false;
    }

    const PlayerState *player = playerById(state, owner);
    const AgentState *scout = findAgent(*player, AgentType::Scout);
    CellNode *cell = findCell(state.board, scout->cellId);

    if (owner == PlayerId::A) {
        cell->markedByA = true;
    } else if (owner == PlayerId::B) {
        cell->markedByB = true;
    } else {
        errorMessage = QStringLiteral("Invalid player.");
        return false;
    }

    return true;
}

bool canSergeantControlInternal(const GameState &state, PlayerId owner, QString &errorMessage)
{
    if (state.status != GameStatus::InProgress) {
        errorMessage = QStringLiteral("Game is already finished.");
        return false;
    }

    const AgentState *sergeant = nullptr;
    const CellNode *cell = nullptr;
    if (!validateAgentReady(state, owner, AgentType::Sergeant, sergeant, cell, errorMessage)) {
        return false;
    }

    Q_UNUSED(sergeant);
    if (hasEnemyOnCell(cell, owner)) {
        errorMessage = QStringLiteral("Cannot control a cell that has an enemy piece.");
        return false;
    }

    if (cell->controlledBy == opponentOf(owner)) {
        errorMessage = QStringLiteral("Cell is controlled by enemy; use release action.");
        return false;
    }

    return true;
}

bool sergeantControl(GameState &state, PlayerId owner, QString &errorMessage)
{
    if (!canSergeantControlInternal(state, owner, errorMessage)) {
        return false;
    }

    const PlayerState *player = playerById(state, owner);
    const AgentState *sergeant = findAgent(*player, AgentType::Sergeant);
    CellNode *cell = findCell(state.board, sergeant->cellId);
    cell->controlledBy = owner;
    updateGameStatus(state);
    return true;
}

bool canSergeantReleaseInternal(const GameState &state, PlayerId owner, QString &errorMessage)
{
    if (state.status != GameStatus::InProgress) {
        errorMessage = QStringLiteral("Game is already finished.");
        return false;
    }

    const AgentState *sergeant = nullptr;
    const CellNode *cell = nullptr;
    if (!validateAgentReady(state, owner, AgentType::Sergeant, sergeant, cell, errorMessage)) {
        return false;
    }

    Q_UNUSED(sergeant);
    if (cell->controlledBy != opponentOf(owner)) {
        errorMessage = QStringLiteral("Current cell is not controlled by enemy.");
        return false;
    }

    if (hasEnemyOnCell(cell, owner)) {
        errorMessage = QStringLiteral("Cannot release while enemy piece is present.");
        return false;
    }

    return true;
}

bool sergeantRelease(GameState &state, PlayerId owner, QString &errorMessage)
{
    if (!canSergeantReleaseInternal(state, owner, errorMessage)) {
        return false;
    }

    const PlayerState *player = playerById(state, owner);
    const AgentState *sergeant = findAgent(*player, AgentType::Sergeant);
    CellNode *cell = findCell(state.board, sergeant->cellId);
    cell->controlledBy = PlayerId::None;
    updateGameStatus(state);
    return true;
}

} // namespace model
