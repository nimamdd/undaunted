#pragma once

#include "../model/Types.h"

namespace model {

void shuffleDeck(DeckState &deck);
void shufflePlayerDeck(PlayerState &player);
void shuffleAllDecks(GameState &state);

const Card *currentTurnCard(const GameState &state);
bool drawTurnCard(GameState &state, Card &drawnCard, QString &errorMessage);
bool endTurn(GameState &state, QString &errorMessage);
bool switchTurnAgent(GameState &state, QString &errorMessage);

int countCards(const PlayerState &player, AgentType type);
bool burnOneCard(PlayerState &player, AgentType type, QString &errorMessage);

} // namespace model
