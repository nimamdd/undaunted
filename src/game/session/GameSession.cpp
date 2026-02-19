#include "GameSession.h"

#include "ActionCommand.h"

#include "../board/BoardGraph.h"
#include "../model/Init.h"
#include "../scenario/ScenarioLoader.h"
#include "../turn/TurnSystem.h"
#include "../rules/Victory.h"

namespace model {

GameSession::GameSession(GameState &state)
    : state_(state)
{
}

bool GameSession::initializeNewBattle(const QString &playerAName,
                                      const QString &playerBName,
                                      const QString &boardPath,
                                      const QString &scenarioPath,
                                      bool useScenario,
                                      QString &errorMessage)
{
    state_ = buildInitialGameState(playerAName, playerBName);
    turnEngine_.resetForBattle();
    loaded_ = false;

    if (!loadBoardFromMapFile(state_.board, boardPath, errorMessage)) {
        return false;
    }

    if (useScenario) {
        if (!loadScenarioFromFile(state_, scenarioPath, errorMessage)) {
            return false;
        }
    } else {
        clearScenarioState(state_);
        updateGameStatus(state_);
    }

    if (state_.status == GameStatus::InProgress) {
        Card drawnCard{};
        if (!drawTurnCard(state_, drawnCard, errorMessage)) {
            return false;
        }
    }

    loaded_ = true;
    turnEngine_.resetForBattle();
    return true;
}

CommandResult GameSession::execute(const ActionCommand &command)
{
    if (!loaded_) {
        return {false, QStringLiteral("Battle is not loaded.")};
    }

    return command.execute(*this);
}

bool GameSession::isLoaded() const
{
    return loaded_;
}

bool GameSession::canUsePrimaryAction(QString &errorMessage) const
{
    return turnEngine_.canUsePrimaryAction(state_, errorMessage);
}

bool GameSession::activeCardAgent(AgentType &typeOut, QString &errorMessage) const
{
    if (!state_.turn.hasActiveCard) {
        errorMessage = QStringLiteral("No active card in current turn.");
        return false;
    }

    typeOut = state_.turn.activeCard.agent;
    return true;
}

GameState &GameSession::state()
{
    return state_;
}

const GameState &GameSession::state() const
{
    return state_;
}

} // namespace model
