#include "ScenarioLoader.h"

#include "../board/BoardGraph.h"
#include "../model/Init.h"
#include "../rules/Victory.h"

#include <QFile>
#include <QStringList>
#include <QTextStream>

namespace model {

namespace {

PlayerId parseOwner(const QString &raw)
{
    const QString owner = raw.trimmed().toUpper();
    if (owner == QStringLiteral("A")) {
        return PlayerId::A;
    }
    if (owner == QStringLiteral("B")) {
        return PlayerId::B;
    }
    return PlayerId::None;
}

bool parseAgentToken(const QString &raw, AgentType &typeOut)
{
    const QString t = raw.trimmed().toLower();
    if (t == QStringLiteral("scout")) {
        typeOut = AgentType::Scout;
        return true;
    }
    if (t == QStringLiteral("sniper")) {
        typeOut = AgentType::Sniper;
        return true;
    }
    if (t == QStringLiteral("sergeant") ||
        t == QStringLiteral("seregent") ||
        t == QStringLiteral("seargeant")) {
        typeOut = AgentType::Sergeant;
        return true;
    }
    return false;
}

bool isCellOccupied(const CellNode *cell)
{
    return cell != nullptr && (cell->occupantA.has_value() || cell->occupantB.has_value());
}

} // namespace

void clearScenarioState(GameState &state)
{
    for (auto &cell : state.board.cells) {
        cell->markedByA = false;
        cell->markedByB = false;
        cell->controlledBy = PlayerId::None;
        cell->occupantA.reset();
        cell->occupantB.reset();
    }

    auto resetPlayerAgents = [](PlayerState &player) {
        for (AgentState &agent : player.agents) {
            agent.cellId.clear();
            agent.hp = defaultHp(agent.type);
            agent.alive = true;
        }
    };

    resetPlayerAgents(state.playerA);
    resetPlayerAgents(state.playerB);
}

bool placeAgent(GameState &state, PlayerId owner, AgentType type, const QString &cellId, QString &errorMessage)
{
    CellNode *cell = findCell(state.board, cellId);
    if (cell == nullptr) {
        errorMessage = QStringLiteral("Scenario references unknown cell: %1").arg(cellId);
        return false;
    }

    if (isCellOccupied(cell)) {
        errorMessage = QStringLiteral("Cell is already occupied: %1").arg(cellId);
        return false;
    }

    PlayerState *player = playerById(state, owner);
    if (player == nullptr) {
        errorMessage = QStringLiteral("Invalid player owner for placement.");
        return false;
    }

    AgentState *agent = findAgent(*player, type);
    if (agent == nullptr) {
        errorMessage = QStringLiteral("Agent does not exist for player %1.")
                           .arg(playerIdName(owner));
        return false;
    }

    if (!agent->cellId.isEmpty()) {
        errorMessage = QStringLiteral("Agent %1 for player %2 is already placed at %3.")
                           .arg(agentTypeName(type), playerIdName(owner), agent->cellId);
        return false;
    }

    if (owner == PlayerId::A) {
        cell->occupantA = type;
    } else if (owner == PlayerId::B) {
        cell->occupantB = type;
    } else {
        errorMessage = QStringLiteral("Invalid player owner for placement.");
        return false;
    }

    agent->cellId = cellId;
    agent->alive = true;
    agent->hp = defaultHp(type);
    return true;
}

bool applyMark(GameState &state, PlayerId owner, const QString &cellId, QString &errorMessage)
{
    CellNode *cell = findCell(state.board, cellId);
    if (cell == nullptr) {
        errorMessage = QStringLiteral("Scenario references unknown cell: %1").arg(cellId);
        return false;
    }

    if (owner == PlayerId::A) {
        cell->markedByA = true;
        return true;
    }
    if (owner == PlayerId::B) {
        cell->markedByB = true;
        return true;
    }

    errorMessage = QStringLiteral("Invalid player owner for mark.");
    return false;
}

bool applyControl(GameState &state, PlayerId owner, const QString &cellId, QString &errorMessage)
{
    CellNode *cell = findCell(state.board, cellId);
    if (cell == nullptr) {
        errorMessage = QStringLiteral("Scenario references unknown cell: %1").arg(cellId);
        return false;
    }

    if (owner != PlayerId::A && owner != PlayerId::B) {
        errorMessage = QStringLiteral("Invalid player owner for control.");
        return false;
    }

    cell->controlledBy = owner;
    return true;
}

bool loadScenarioFromFile(GameState &state, const QString &path, QString &errorMessage)
{
    if (state.board.cells.empty()) {
        errorMessage = QStringLiteral("Board must be loaded before scenario.");
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorMessage = QStringLiteral("Cannot open scenario file: %1").arg(path);
        return false;
    }

    clearScenarioState(state);

    QTextStream in(&file);
    int lineNo = 0;

    while (!in.atEnd()) {
        ++lineNo;
        const QString rawLine = in.readLine().trimmed();
        if (rawLine.isEmpty()) {
            continue;
        }

        const QStringList mainParts = rawLine.split(QLatin1Char(':'));
        if (mainParts.size() != 2) {
            errorMessage = QStringLiteral("Invalid scenario line %1: %2").arg(lineNo).arg(rawLine);
            clearScenarioState(state);
            return false;
        }

        const QString cellId = mainParts[0].trimmed();
        const QStringList rhs = mainParts[1].split(QLatin1Char(','));
        if (rhs.size() != 2) {
            errorMessage = QStringLiteral("Invalid scenario line %1: %2").arg(lineNo).arg(rawLine);
            clearScenarioState(state);
            return false;
        }

        const PlayerId owner = parseOwner(rhs[0]);
        if (owner == PlayerId::None) {
            errorMessage = QStringLiteral("Invalid owner at line %1: %2").arg(lineNo).arg(rhs[0].trimmed());
            clearScenarioState(state);
            return false;
        }

        const QString token = rhs[1].trimmed();
        const QString tokenLower = token.toLower();

        bool ok = false;
        if (tokenLower == QStringLiteral("mark")) {
            ok = applyMark(state, owner, cellId, errorMessage);
        } else if (tokenLower == QStringLiteral("control")) {
            ok = applyControl(state, owner, cellId, errorMessage);
        } else {
            AgentType type{};
            if (!parseAgentToken(token, type)) {
                errorMessage = QStringLiteral("Invalid token at line %1: %2").arg(lineNo).arg(token);
                clearScenarioState(state);
                return false;
            }
            ok = placeAgent(state, owner, type, cellId, errorMessage);
        }

        if (!ok) {
            errorMessage = QStringLiteral("Line %1: %2").arg(lineNo).arg(errorMessage);
            clearScenarioState(state);
            return false;
        }
    }

    auto ensurePlaced = [&](PlayerId owner, AgentType type) {
        const PlayerState *player = playerById(state, owner);
        const AgentState *agent = player ? findAgent(*player, type) : nullptr;
        return agent != nullptr && !agent->cellId.isEmpty();
    };

    const bool allPlaced =
        ensurePlaced(PlayerId::A, AgentType::Scout) &&
        ensurePlaced(PlayerId::A, AgentType::Sniper) &&
        ensurePlaced(PlayerId::A, AgentType::Sergeant) &&
        ensurePlaced(PlayerId::B, AgentType::Scout) &&
        ensurePlaced(PlayerId::B, AgentType::Sniper) &&
        ensurePlaced(PlayerId::B, AgentType::Sergeant);

    if (!allPlaced) {
        errorMessage = QStringLiteral("Scenario must place Scout, Sniper, and Sergeant for both players.");
        clearScenarioState(state);
        return false;
    }

    updateGameStatus(state);
    return true;
}

} // namespace model
