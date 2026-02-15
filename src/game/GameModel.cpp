#include "GameModel.h"

#include <QFile>
#include <QQueue>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <QTextStream>

#include <cmath>
#include <utility>

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

int defaultHp(AgentType type)
{
    switch (type) {
    case AgentType::Scout:
        return 5;
    case AgentType::Sniper:
        return 4;
    case AgentType::Sergeant:
        return 3;
    }
    return 0;
}

QVector<Card> defaultCards(AgentType type)
{
    const int count = (type == AgentType::Scout) ? 4 : 3;

    QVector<Card> cards;
    cards.reserve(count);
    for (int i = 0; i < count; ++i) {
        cards.push_back(Card{type});
    }
    return cards;
}

DeckState buildDefaultDeck()
{
    DeckState deck;
    deck.drawPile.reserve(10);

    const QVector<AgentType> order = {
        AgentType::Scout,
        AgentType::Sniper,
        AgentType::Sergeant
    };

    for (AgentType type : order) {
        const QVector<Card> cards = defaultCards(type);
        for (const Card &card : cards) {
            deck.drawPile.push_back(card);
        }
    }

    return deck;
}

QVector<AgentState> buildDefaultAgents(PlayerId owner)
{
    QVector<AgentState> agents;
    agents.reserve(3);

    const QVector<AgentType> order = {
        AgentType::Scout,
        AgentType::Sniper,
        AgentType::Sergeant
    };

    for (AgentType type : order) {
        agents.push_back(AgentState{
            type,
            owner,
            QString{},
            defaultHp(type),
            true
        });
    }

    return agents;
}

PlayerState buildDefaultPlayer(PlayerId id, const QString &name)
{
    PlayerState player;
    player.id = id;
    player.name = name;
    player.deck = buildDefaultDeck();
    player.agents = buildDefaultAgents(id);
    return player;
}

GameState buildInitialGameState(const QString &playerAName, const QString &playerBName)
{
    GameState state;
    state.playerA = buildDefaultPlayer(PlayerId::A, playerAName);
    state.playerB = buildDefaultPlayer(PlayerId::B, playerBName);
    state.turn.currentPlayer = PlayerId::A;
    state.turn.turnIndex = 1;
    state.status = GameStatus::InProgress;
    return state;
}

PlayerState *playerById(GameState &state, PlayerId id)
{
    switch (id) {
    case PlayerId::A:
        return &state.playerA;
    case PlayerId::B:
        return &state.playerB;
    case PlayerId::None:
        break;
    }
    return nullptr;
}

const PlayerState *playerById(const GameState &state, PlayerId id)
{
    switch (id) {
    case PlayerId::A:
        return &state.playerA;
    case PlayerId::B:
        return &state.playerB;
    case PlayerId::None:
        break;
    }
    return nullptr;
}

PlayerId opponentOf(PlayerId id)
{
    if (id == PlayerId::A) {
        return PlayerId::B;
    }
    if (id == PlayerId::B) {
        return PlayerId::A;
    }
    return PlayerId::None;
}

AgentState *findAgent(PlayerState &player, AgentType type)
{
    for (AgentState &agent : player.agents) {
        if (agent.type == type) {
            return &agent;
        }
    }
    return nullptr;
}

const AgentState *findAgent(const PlayerState &player, AgentType type)
{
    for (const AgentState &agent : player.agents) {
        if (agent.type == type) {
            return &agent;
        }
    }
    return nullptr;
}

QString playerIdName(PlayerId id)
{
    switch (id) {
    case PlayerId::A:
        return QStringLiteral("A");
    case PlayerId::B:
        return QStringLiteral("B");
    case PlayerId::None:
        return QStringLiteral("None");
    }
    return QStringLiteral("None");
}

QString agentTypeName(AgentType type)
{
    switch (type) {
    case AgentType::Scout:
        return QStringLiteral("Scout");
    case AgentType::Sniper:
        return QStringLiteral("Sniper");
    case AgentType::Sergeant:
        return QStringLiteral("Sergeant");
    }
    return QStringLiteral("Unknown");
}

CellNode *findCell(BoardState &board, const QString &cellId)
{
    auto it = board.byId.find(cellId);
    if (it == board.byId.end()) {
        return nullptr;
    }
    return it.value();
}

const CellNode *findCell(const BoardState &board, const QString &cellId)
{
    auto it = board.byId.find(cellId);
    if (it == board.byId.end()) {
        return nullptr;
    }
    return it.value();
}

bool loadBoardFromMapFile(BoardState &board, const QString &path, QString &errorMessage)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorMessage = QStringLiteral("Cannot open map file: %1").arg(path);
        return false;
    }

    board.cells.clear();
    board.byId.clear();

    struct NodePoint {
        CellNode *node{nullptr};
        double x{0.0};
        double y{0.0};
    };

    QVector<NodePoint> points;
    QRegularExpression tokenRe(QStringLiteral("\\|\\s*([A-Z]\\d{2}):(\\d)"));
    QTextStream in(&file);
    int rowIndex = 0;
    const double sqrt3 = std::sqrt(3.0);

    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.trimmed().isEmpty()) {
            continue;
        }

        const bool offset = line.startsWith(QLatin1Char(' '));
        auto it = tokenRe.globalMatch(line);
        int colIndex = 0;

        while (it.hasNext()) {
            const auto m = it.next();
            const QString cellId = m.captured(1);

            if (board.byId.contains(cellId)) {
                errorMessage = QStringLiteral("Duplicate cell id in map: %1").arg(cellId);
                board.cells.clear();
                board.byId.clear();
                return false;
            }

            auto node = std::make_unique<CellNode>();
            node->id = cellId;
            node->shield = m.captured(2).toInt();
            node->row = rowIndex;
            node->col = colIndex;

            CellNode *raw = node.get();
            board.cells.push_back(std::move(node));
            board.byId.insert(cellId, raw);

            const double x = colIndex * sqrt3 + (offset ? (sqrt3 / 2.0) : 0.0);
            const double y = rowIndex * 1.5;
            points.push_back(NodePoint{raw, x, y});

            ++colIndex;
        }

        if (colIndex > 0) {
            ++rowIndex;
        }
    }

    if (board.cells.empty()) {
        errorMessage = QStringLiteral("Map file is empty or invalid: %1").arg(path);
        return false;
    }

    const double neighborDistance = sqrt3;
    const double epsilon = 0.01;

    for (CellNode *cell : std::as_const(board.byId)) {
        cell->neighbors.clear();
    }

    for (int i = 0; i < points.size(); ++i) {
        for (int j = i + 1; j < points.size(); ++j) {
            const double dx = points[i].x - points[j].x;
            const double dy = points[i].y - points[j].y;
            const double dist = std::sqrt(dx * dx + dy * dy);
            if (std::abs(dist - neighborDistance) <= epsilon) {
                points[i].node->neighbors.push_back(points[j].node);
                points[j].node->neighbors.push_back(points[i].node);
            }
        }
    }

    return true;
}

QVector<CellNode *> neighborsOf(BoardState &board, const QString &cellId)
{
    const CellNode *cell = findCell(static_cast<const BoardState &>(board), cellId);
    if (cell == nullptr) {
        return {};
    }

    QVector<CellNode *> neighbors;
    neighbors.reserve(cell->neighbors.size());
    for (CellNode *neighbor : cell->neighbors) {
        neighbors.push_back(neighbor);
    }
    return neighbors;
}

QVector<const CellNode *> neighborsOf(const BoardState &board, const QString &cellId)
{
    const CellNode *cell = findCell(board, cellId);
    if (cell == nullptr) {
        return {};
    }

    QVector<const CellNode *> neighbors;
    neighbors.reserve(cell->neighbors.size());
    for (const CellNode *neighbor : cell->neighbors) {
        neighbors.push_back(neighbor);
    }
    return neighbors;
}

QVector<const CellNode *> bfsTraversal(const BoardState &board, const QString &startCellId)
{
    const CellNode *start = findCell(board, startCellId);
    if (start == nullptr) {
        return {};
    }

    QVector<const CellNode *> order;
    QQueue<const CellNode *> queue;
    QSet<const CellNode *> visited;

    queue.enqueue(start);
    visited.insert(start);

    while (!queue.isEmpty()) {
        const CellNode *current = queue.dequeue();
        order.push_back(current);

        for (const CellNode *neighbor : current->neighbors) {
            if (visited.contains(neighbor)) {
                continue;
            }
            visited.insert(neighbor);
            queue.enqueue(neighbor);
        }
    }

    return order;
}

QVector<CellNode *> bfsTraversal(BoardState &board, const QString &startCellId)
{
    const QVector<const CellNode *> order = bfsTraversal(static_cast<const BoardState &>(board), startCellId);
    QVector<CellNode *> mutableOrder;
    mutableOrder.reserve(order.size());
    for (const CellNode *node : order) {
        mutableOrder.push_back(findCell(board, node->id));
    }
    return mutableOrder;
}

QVector<const CellNode *> shortestPath(const BoardState &board, const QString &startCellId, const QString &goalCellId)
{
    const CellNode *start = findCell(board, startCellId);
    const CellNode *goal = findCell(board, goalCellId);
    if (start == nullptr || goal == nullptr) {
        return {};
    }

    if (start == goal) {
        return {start};
    }

    QQueue<const CellNode *> queue;
    QSet<const CellNode *> visited;
    QHash<const CellNode *, const CellNode *> parent;

    queue.enqueue(start);
    visited.insert(start);
    parent.insert(start, nullptr);

    while (!queue.isEmpty()) {
        const CellNode *current = queue.dequeue();
        if (current == goal) {
            break;
        }

        for (const CellNode *neighbor : current->neighbors) {
            if (visited.contains(neighbor)) {
                continue;
            }
            visited.insert(neighbor);
            parent.insert(neighbor, current);
            queue.enqueue(neighbor);
        }
    }

    if (!visited.contains(goal)) {
        return {};
    }

    QVector<const CellNode *> reversed;
    for (const CellNode *node = goal; node != nullptr; node = parent.value(node, nullptr)) {
        reversed.push_back(node);
    }

    QVector<const CellNode *> path;
    path.reserve(reversed.size());
    for (int i = reversed.size() - 1; i >= 0; --i) {
        path.push_back(reversed[i]);
    }

    return path;
}

QVector<CellNode *> shortestPath(BoardState &board, const QString &startCellId, const QString &goalCellId)
{
    const QVector<const CellNode *> path = shortestPath(static_cast<const BoardState &>(board), startCellId, goalCellId);
    QVector<CellNode *> mutablePath;
    mutablePath.reserve(path.size());
    for (const CellNode *node : path) {
        mutablePath.push_back(findCell(board, node->id));
    }
    return mutablePath;
}

int pathShieldSum(const QVector<const CellNode *> &path, bool excludeEndpoints)
{
    if (path.isEmpty()) {
        return 0;
    }

    int from = 0;
    int to = path.size();
    if (excludeEndpoints && path.size() >= 2) {
        from = 1;
        to = path.size() - 1;
    }

    int total = 0;
    for (int i = from; i < to; ++i) {
        total += path[i]->shield;
    }
    return total;
}

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

    return true;
}

} // namespace model
