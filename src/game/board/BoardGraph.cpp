#include "BoardGraph.h"

#include <QFile>
#include <QQueue>
#include <QRegularExpression>
#include <QSet>
#include <QTextStream>

#include <cmath>
#include <utility>

namespace model {

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
            node->offset = offset;

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

} // namespace model
