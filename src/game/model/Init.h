#pragma once

#include "Types.h"

namespace model {

int defaultHp(AgentType type);
QVector<Card> defaultCards(AgentType type);
DeckState buildDefaultDeck();
QVector<AgentState> buildDefaultAgents(PlayerId owner);
PlayerState buildDefaultPlayer(PlayerId id, const QString &name);
GameState buildInitialGameState(const QString &playerAName, const QString &playerBName);

PlayerState *playerById(GameState &state, PlayerId id);
const PlayerState *playerById(const GameState &state, PlayerId id);

PlayerId opponentOf(PlayerId id);
AgentState *findAgent(PlayerState &player, AgentType type);
const AgentState *findAgent(const PlayerState &player, AgentType type);

QString playerIdName(PlayerId id);
QString agentTypeName(AgentType type);

} // namespace model

