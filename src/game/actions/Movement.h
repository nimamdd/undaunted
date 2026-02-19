#pragma once

#include "../model/Types.h"

namespace model {

bool moveAgent(GameState &state, PlayerId owner, AgentType type, const QString &toCellId, QString &errorMessage);

} // namespace model
