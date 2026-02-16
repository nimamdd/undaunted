#include "Init.h"

#include "../turn/TurnSystem.h"

namespace model {

int defaultHp(AgentType type)
{
    switch (type) {
    case AgentType::Scout:
        return 5;
    case AgentType::Sniper:
        return 4;
    case AgentType::Sergeant:
        return 3;
    }
    return 0;
}

QVector<Card> defaultCards(AgentType type)
{
    const int count = (type == AgentType::Scout) ? 4 : 3;

    QVector<Card> cards;
    cards.reserve(count);
    for (int i = 0; i < count; ++i) {
        cards.push_back(Card{type});
    }
    return cards;
}

DeckState buildDefaultDeck()
{
    DeckState deck;
    deck.drawPile.reserve(10);

    const QVector<AgentType> order = {
        AgentType::Scout,
        AgentType::Sniper,
        AgentType::Sergeant
    };

    for (AgentType type : order) {
        const QVector<Card> cards = defaultCards(type);
        for (const Card &card : cards) {
            deck.drawPile.push_back(card);
        }
    }

    return deck;
}

QVector<AgentState> buildDefaultAgents(PlayerId owner)
{
    QVector<AgentState> agents;
    agents.reserve(3);

    const QVector<AgentType> order = {
        AgentType::Scout,
        AgentType::Sniper,
        AgentType::Sergeant
    };

    for (AgentType type : order) {
        agents.push_back(AgentState{
            type,
            owner,
            QString{},
            defaultHp(type),
            true
        });
    }

    return agents;
}

PlayerState buildDefaultPlayer(PlayerId id, const QString &name)
{
    PlayerState player;
    player.id = id;
    player.name = name;
    player.deck = buildDefaultDeck();
    player.agents = buildDefaultAgents(id);
    return player;
}

GameState buildInitialGameState(const QString &playerAName, const QString &playerBName)
{
    GameState state;
    state.playerA = buildDefaultPlayer(PlayerId::A, playerAName);
    state.playerB = buildDefaultPlayer(PlayerId::B, playerBName);
    shuffleAllDecks(state);
    state.turn.currentPlayer = PlayerId::A;
    state.turn.turnIndex = 1;
    state.turn.hasActiveCard = false;
    state.status = GameStatus::InProgress;
    return state;
}

PlayerState *playerById(GameState &state, PlayerId id)
{
    switch (id) {
    case PlayerId::A:
        return &state.playerA;
    case PlayerId::B:
        return &state.playerB;
    case PlayerId::None:
        break;
    }
    return nullptr;
}

const PlayerState *playerById(const GameState &state, PlayerId id)
{
    switch (id) {
    case PlayerId::A:
        return &state.playerA;
    case PlayerId::B:
        return &state.playerB;
    case PlayerId::None:
        break;
    }
    return nullptr;
}

PlayerId opponentOf(PlayerId id)
{
    if (id == PlayerId::A) {
        return PlayerId::B;
    }
    if (id == PlayerId::B) {
        return PlayerId::A;
    }
    return PlayerId::None;
}

AgentState *findAgent(PlayerState &player, AgentType type)
{
    for (AgentState &agent : player.agents) {
        if (agent.type == type) {
            return &agent;
        }
    }
    return nullptr;
}

const AgentState *findAgent(const PlayerState &player, AgentType type)
{
    for (const AgentState &agent : player.agents) {
        if (agent.type == type) {
            return &agent;
        }
    }
    return nullptr;
}

QString playerIdName(PlayerId id)
{
    switch (id) {
    case PlayerId::A:
        return QStringLiteral("A");
    case PlayerId::B:
        return QStringLiteral("B");
    case PlayerId::None:
        return QStringLiteral("None");
    }
    return QStringLiteral("None");
}

QString agentTypeName(AgentType type)
{
    switch (type) {
    case AgentType::Scout:
        return QStringLiteral("Scout");
    case AgentType::Sniper:
        return QStringLiteral("Sniper");
    case AgentType::Sergeant:
        return QStringLiteral("Sergeant");
    }
    return QStringLiteral("Unknown");
}

} // namespace model

