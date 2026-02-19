#include "ActionCommand.h"

#include "GameSession.h"

#include "../actions/Combat.h"
#include "../actions/Movement.h"
#include "../model/Init.h"
#include "../turn/TurnSystem.h"

#include <utility>

namespace model {

namespace {

CommandResult failure(const QString &message)
{
    return CommandResult{false, message};
}

CommandResult success(const QString &message)
{
    return CommandResult{true, message};
}

bool resolveCurrentAgent(const GameSession &session, AgentType &typeOut, QString &errorMessage)
{
    return session.activeCardAgent(typeOut, errorMessage);
}

QString winnerText(const GameState &state)
{
    if (state.status == GameStatus::WonByA) {
        return QStringLiteral("Winner: A");
    }
    if (state.status == GameStatus::WonByB) {
        return QStringLiteral("Winner: B");
    }
    return QString();
}

QString specialActionSuccessMessage(AgentSpecialAction action)
{
    switch (action) {
    case AgentSpecialAction::ScoutMark:
        return QStringLiteral("Scout marked current cell.");
    case AgentSpecialAction::SergeantControl:
        return QStringLiteral("Sergeant controlled current cell.");
    case AgentSpecialAction::SergeantRelease:
        return QStringLiteral("Sergeant released enemy-controlled cell.");
    }

    return QStringLiteral("Special action executed.");
}

} // namespace

MoveCommand::MoveCommand(QString targetCellId)
    : targetCellId_(std::move(targetCellId))
{
}

CommandResult MoveCommand::execute(GameSession &session) const
{
    QString error;
    if (!session.canUsePrimaryAction(error)) {
        return failure(error);
    }

    AgentType type{};
    if (!resolveCurrentAgent(session, type, error)) {
        return failure(error);
    }

    if (!moveAgent(session.state(),
                   session.state().turn.currentPlayer,
                   type,
                   targetCellId_,
                   error)) {
        return failure(error);
    }

    session.markActionUsed();
    return success(QStringLiteral("%1 moved to %2.")
                       .arg(agentTypeName(type), targetCellId_));
}

AttackCommand::AttackCommand(QString targetCellId)
    : targetCellId_(std::move(targetCellId))
{
}

CommandResult AttackCommand::execute(GameSession &session) const
{
    QString error;
    if (!session.canUsePrimaryAction(error)) {
        return failure(error);
    }

    AgentType type{};
    if (!resolveCurrentAgent(session, type, error)) {
        return failure(error);
    }

    const AttackResult result = attack(session.state(),
                                       session.state().turn.currentPlayer,
                                       type,
                                       targetCellId_);
    if (!result.executed) {
        return failure(result.errorMessage);
    }

    QString rollText;
    for (int i = 0; i < result.rolls.size(); ++i) {
        if (i) {
            rollText += QLatin1String(", ");
        }
        rollText += QString::number(result.rolls[i]);
    }

    QString message = QStringLiteral("Attack threshold %1 | Rolls: [%2]")
                          .arg(result.threshold)
                          .arg(rollText);
    if (result.success) {
        message += QStringLiteral(" | Hit");
        if (result.targetEliminated) {
            message += QStringLiteral(" | Enemy %1 eliminated")
                           .arg(agentTypeName(result.targetType));
        }
    } else {
        message += QStringLiteral(" | Miss");
    }

    if (session.state().status != GameStatus::InProgress) {
        const QString winner = winnerText(session.state());
        if (!winner.isEmpty()) {
            message += QStringLiteral(" | %1").arg(winner);
        }
    }

    session.markActionUsed();
    return success(message);
}

UseAgentSpecialCommand::UseAgentSpecialCommand(AgentSpecialAction action)
    : action_(action)
{
}

CommandResult UseAgentSpecialCommand::execute(GameSession &session) const
{
    QString error;
    if (!session.canUsePrimaryAction(error)) {
        return failure(error);
    }

    AgentType type{};
    if (!resolveCurrentAgent(session, type, error)) {
        return failure(error);
    }

    const AgentBehavior *behavior = behaviorFor(type);
    if (behavior == nullptr) {
        return failure(QStringLiteral("Unsupported active agent type."));
    }

    if (!behavior->supportsSpecial(action_)) {
        return failure(QStringLiteral("%1 does not support %2.")
                           .arg(agentTypeName(type), specialActionName(action_)));
    }

    if (!behavior->executeSpecial(session.state(),
                                  session.state().turn.currentPlayer,
                                  action_,
                                  error)) {
        return failure(error);
    }

    session.markActionUsed();
    return success(specialActionSuccessMessage(action_));
}

CommandResult EndTurnCommand::execute(GameSession &session) const
{
    QString error;
    if (!session.canEndTurn(error)) {
        return failure(error);
    }

    if (!endTurn(session.state(), error)) {
        return failure(error);
    }

    session.onTurnEnded();

    if (session.state().status == GameStatus::InProgress) {
        Card drawn{};
        if (!drawTurnCard(session.state(), drawn, error)) {
            return failure(error);
        }

        return success(QStringLiteral("Turn passed to %1.")
                           .arg(playerIdName(session.state().turn.currentPlayer)));
    }

    const QString winner = winnerText(session.state());
    if (!winner.isEmpty()) {
        return success(winner);
    }

    return success(QStringLiteral("Game finished."));
}

} // namespace model
