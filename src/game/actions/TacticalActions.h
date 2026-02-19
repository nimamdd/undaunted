#pragma once

#include "../model/Types.h"

namespace model {

bool canScoutMark(const GameState &state, PlayerId owner, QString &errorMessage);
bool scoutMark(GameState &state, PlayerId owner, QString &errorMessage);

bool canSergeantControl(const GameState &state, PlayerId owner, QString &errorMessage);
bool sergeantControl(GameState &state, PlayerId owner, QString &errorMessage);

bool canSergeantRelease(const GameState &state, PlayerId owner, QString &errorMessage);
bool sergeantRelease(GameState &state, PlayerId owner, QString &errorMessage);

} // namespace model
