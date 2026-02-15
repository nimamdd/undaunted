#pragma once

#include <QString>
#include <QVector>
#include <QHash>

#include <memory>
#include <optional>
#include <vector>

namespace model {

enum class PlayerId {
    None,
    A,
    B
};

enum class AgentType {
    Scout,
    Sniper,
    Sergeant
};

enum class GameStatus {
    InProgress,
    WonByA,
    WonByB
};

struct Card {
    AgentType agent{};
};

struct DeckState {
    QVector<Card> drawPile;
    QVector<Card> discardPile;
};

struct AgentState {
    AgentType type{};
    PlayerId owner{PlayerId::None};
    QString cellId;
    int hp{0};
    bool alive{true};
};

struct CellNode {
    QString id;
    int shield{0};
    int row{0};
    int col{0};
    QVector<CellNode *> neighbors;

    bool markedByA{false};
    bool markedByB{false};
    PlayerId controlledBy{PlayerId::None};

    std::optional<AgentType> occupantA;
    std::optional<AgentType> occupantB;
};

struct BoardState {
    std::vector<std::unique_ptr<CellNode>> cells;
    QHash<QString, CellNode *> byId;
};

struct PlayerState {
    PlayerId id{PlayerId::None};
    QString name;
    DeckState deck;
    QVector<AgentState> agents;
};

struct TurnState {
    PlayerId currentPlayer{PlayerId::A};
    int turnIndex{1};
};

struct GameState {
    BoardState board;
    PlayerState playerA;
    PlayerState playerB;
    TurnState turn;
    GameStatus status{GameStatus::InProgress};
};

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

