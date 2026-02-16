#pragma once

#include "../model/Types.h"

namespace model {

QVector<const CellNode *> movableNeighbors(const GameState &state, PlayerId owner, AgentType type);
bool canMoveAgent(const GameState &state, PlayerId owner, AgentType type, const QString &toCellId, QString &errorMessage);
bool moveAgent(GameState &state, PlayerId owner, AgentType type, const QString &toCellId, QString &errorMessage);
bool moveCurrentPlayerAgent(GameState &state, AgentType type, const QString &toCellId, QString &errorMessage);

} // namespace model

