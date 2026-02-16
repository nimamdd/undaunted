#include "TurnSystem.h"

#include "../model/Init.h"

#include <QRandomGenerator>

#include <utility>

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

bool isAgentUsable(const PlayerState &player, AgentType type)
{
    const AgentState *agent = findAgent(player, type);
    return agent != nullptr && agent->alive && !agent->cellId.isEmpty();
}

int firstCardIndexForType(const DeckState &deck, AgentType type)
{
    for (int i = 0; i < deck.drawPile.size(); ++i) {
        if (deck.drawPile.at(i).agent == type) {
            return i;
        }
    }
    return -1;
}

QVector<AgentType> switchOrderFrom(AgentType current)
{
    const QVector<AgentType> order = {
        AgentType::Scout,
        AgentType::Sniper,
        AgentType::Sergeant,
    };

    int currentIndex = -1;
    for (int i = 0; i < order.size(); ++i) {
        if (order.at(i) == current) {
            currentIndex = i;
            break;
        }
    }

    QVector<AgentType> out;
    if (currentIndex < 0) {
        return out;
    }

    for (int step = 1; step < order.size(); ++step) {
        out.push_back(order.at((currentIndex + step) % order.size()));
    }
    return out;
}

} // namespace

void shuffleDeck(DeckState &deck)
{
    if (!deck.discardPile.isEmpty()) {
        for (const Card &card : std::as_const(deck.discardPile)) {
            deck.drawPile.push_back(card);
        }
        deck.discardPile.clear();
    }
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

const Card *currentTurnCard(const GameState &state)
{
    return state.turn.hasActiveCard ? &state.turn.activeCard : nullptr;
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

bool switchTurnAgent(GameState &state, QString &errorMessage)
{
    if (state.status != GameStatus::InProgress) {
        errorMessage = QStringLiteral("Game is already finished.");
        return false;
    }

    if (!state.turn.hasActiveCard) {
        errorMessage = QStringLiteral("No active turn card to switch.");
        return false;
    }

    PlayerState *player = playerById(state, state.turn.currentPlayer);
    if (player == nullptr) {
        errorMessage = QStringLiteral("Invalid current player.");
        return false;
    }

    const AgentType currentType = state.turn.activeCard.agent;

    int replacementIndex = -1;
    for (const AgentType nextType : switchOrderFrom(currentType)) {
        if (!isAgentUsable(*player, nextType)) {
            continue;
        }
        replacementIndex = firstCardIndexForType(player->deck, nextType);
        if (replacementIndex >= 0) {
            break;
        }
    }

    if (replacementIndex < 0) {
        errorMessage = QStringLiteral("No other active agent card available to switch.");
        return false;
    }

    const Card previous = state.turn.activeCard;
    const Card replacement = player->deck.drawPile.at(replacementIndex);
    player->deck.drawPile.removeAt(replacementIndex);
    player->deck.drawPile.push_back(previous);
    state.turn.activeCard = replacement;
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
    for (const Card &card : player.deck.discardPile) {
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

    for (int i = 0; i < player.deck.discardPile.size(); ++i) {
        if (player.deck.discardPile[i].agent == type) {
            player.deck.discardPile.removeAt(i);
            return true;
        }
    }

    errorMessage = QStringLiteral("No %1 card left for player %2.")
                       .arg(agentTypeName(type), playerIdName(player.id));
    return false;
}

} // namespace model
