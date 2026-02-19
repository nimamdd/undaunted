#include "TurnSystem.h"

#include "../model/Init.h"

#include <QRandomGenerator>

namespace model {

namespace {

template <typename T>
void shuffleVector(QVector<T> &values)
{
    if (values.size() <= 1) {
        return;
    }

    auto *rng = QRandomGenerator::global();
    for (int i = values.size() - 1; i > 0; --i) {
        const int j = rng->bounded(i + 1);
        if (i != j) {
            values.swapItemsAt(i, j);
        }
    }
}

} // namespace

void shuffleDeck(DeckState &deck)
{
    shuffleVector(deck.drawPile);
}

void shufflePlayerDeck(PlayerState &player)
{
    shuffleDeck(player.deck);
}

void shuffleAllDecks(GameState &state)
{
    shufflePlayerDeck(state.playerA);
    shufflePlayerDeck(state.playerB);
}

bool drawTurnCard(GameState &state, Card &drawnCard, QString &errorMessage)
{
    if (state.status != GameStatus::InProgress) {
        errorMessage = QStringLiteral("Game is already finished.");
        return false;
    }

    if (state.turn.hasActiveCard) {
        errorMessage = QStringLiteral("Current turn card is already drawn.");
        return false;
    }

    PlayerState *player = playerById(state, state.turn.currentPlayer);
    if (player == nullptr) {
        errorMessage = QStringLiteral("Invalid current player.");
        return false;
    }

    if (player->deck.drawPile.isEmpty()) {
        errorMessage = QStringLiteral("Player %1 has no cards to draw.")
                           .arg(playerIdName(player->id));
        return false;
    }

    const Card card = player->deck.drawPile.at(0);
    player->deck.drawPile.removeAt(0);

    state.turn.activeCard = card;
    state.turn.hasActiveCard = true;
    drawnCard = card;
    return true;
}

bool endTurn(GameState &state, QString &errorMessage)
{
    if (state.status != GameStatus::InProgress) {
        errorMessage = QStringLiteral("Game is already finished.");
        return false;
    }

    if (!state.turn.hasActiveCard) {
        errorMessage = QStringLiteral("No active turn card to finish the turn.");
        return false;
    }

    PlayerState *player = playerById(state, state.turn.currentPlayer);
    if (player == nullptr) {
        errorMessage = QStringLiteral("Invalid current player.");
        return false;
    }

    player->deck.drawPile.push_back(state.turn.activeCard);
    state.turn.hasActiveCard = false;

    state.turn.currentPlayer = opponentOf(state.turn.currentPlayer);
    state.turn.turnIndex += 1;
    return true;
}

int countCards(const PlayerState &player, AgentType type)
{
    int total = 0;
    for (const Card &card : player.deck.drawPile) {
        if (card.agent == type) {
            ++total;
        }
    }
    return total;
}

bool burnOneCard(PlayerState &player, AgentType type, QString &errorMessage)
{
    for (int i = 0; i < player.deck.drawPile.size(); ++i) {
        if (player.deck.drawPile[i].agent == type) {
            player.deck.drawPile.removeAt(i);
            return true;
        }
    }

    errorMessage = QStringLiteral("No %1 card left for player %2.")
                       .arg(agentTypeName(type), playerIdName(player.id));
    return false;
}

} // namespace model
