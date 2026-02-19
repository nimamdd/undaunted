#pragma once

#include "SessionTypes.h"
#include "TurnEngine.h"

namespace model {

class ActionCommand;

class GameSession
{
public:
    explicit GameSession(GameState &state);

    bool initializeNewBattle(const QString &playerAName,
                             const QString &playerBName,
                             const QString &boardPath,
                             const QString &scenarioPath,
                             bool useScenario,
                             QString &errorMessage);

    CommandResult execute(const ActionCommand &command);

    bool isLoaded() const;
    bool actionUsedThisTurn() const;

    bool canUsePrimaryAction(QString &errorMessage) const;
    bool canSwitchAgent(QString &errorMessage) const;
    bool canEndTurn(QString &errorMessage) const;

    bool activeCardAgent(AgentType &typeOut, QString &errorMessage) const;

    void markActionUsed();
    void onTurnEnded();

    GameState &state();
    const GameState &state() const;

private:
    GameState &state_;
    TurnEngine turnEngine_;
    bool loaded_{false};
};

} // namespace model
