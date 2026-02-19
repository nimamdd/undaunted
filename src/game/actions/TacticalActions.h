#pragma once

#include "../model/Types.h"

namespace model {

bool scoutMark(GameState &state, PlayerId owner, QString &errorMessage);

bool sergeantControl(GameState &state, PlayerId owner, QString &errorMessage);

bool sergeantRelease(GameState &state, PlayerId owner, QString &errorMessage);

} // namespace model
