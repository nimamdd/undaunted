#pragma once

#include "../model/Types.h"

namespace model {

CellNode *findCell(BoardState &board, const QString &cellId);
const CellNode *findCell(const BoardState &board, const QString &cellId);

bool loadBoardFromMapFile(BoardState &board, const QString &path, QString &errorMessage);
QVector<CellNode *> neighborsOf(BoardState &board, const QString &cellId);
QVector<const CellNode *> neighborsOf(const BoardState &board, const QString &cellId);

QVector<CellNode *> bfsTraversal(BoardState &board, const QString &startCellId);
QVector<const CellNode *> bfsTraversal(const BoardState &board, const QString &startCellId);

QVector<CellNode *> shortestPath(BoardState &board, const QString &startCellId, const QString &goalCellId);
QVector<const CellNode *> shortestPath(const BoardState &board, const QString &startCellId, const QString &goalCellId);

int pathShieldSum(const QVector<const CellNode *> &path, bool excludeEndpoints = true);

} // namespace model

