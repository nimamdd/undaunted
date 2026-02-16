#pragma once

#include "../model/Types.h"

namespace model {

void clearScenarioState(GameState &state);
bool placeAgent(GameState &state, PlayerId owner, AgentType type, const QString &cellId, QString &errorMessage);
bool applyMark(GameState &state, PlayerId owner, const QString &cellId, QString &errorMessage);
bool applyControl(GameState &state, PlayerId owner, const QString &cellId, QString &errorMessage);
bool loadScenarioFromFile(GameState &state, const QString &path, QString &errorMessage);

} // namespace model

