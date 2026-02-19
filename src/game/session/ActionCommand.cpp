#include "ActionCommand.h"

#include "GameSession.h"

#include "../actions/Combat.h"
#include "../actions/Movement.h"
#include "../actions/TacticalActions.h"
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

bool resolveCurrentAgent(GameSession &session, AgentType &typeOut, QString &errorMessage)
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

    if (!moveCurrentPlayerAgent(session.state(), type, targetCellId_, error)) {
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

    const AttackResult result = attackCurrentPlayer(session.state(), type, targetCellId_);
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

CommandResult ScoutMarkCommand::execute(GameSession &session) const
{
    QString error;
    if (!session.canUsePrimaryAction(error)) {
        return failure(error);
    }

    if (!scoutMarkCurrentPlayer(session.state(), error)) {
        return failure(error);
    }

    session.markActionUsed();
    return success(QStringLiteral("Scout marked current cell."));
}

CommandResult SergeantControlCommand::execute(GameSession &session) const
{
    QString error;
    if (!session.canUsePrimaryAction(error)) {
        return failure(error);
    }

    if (!sergeantControlCurrentPlayer(session.state(), error)) {
        return failure(error);
    }

    session.markActionUsed();
    return success(QStringLiteral("Sergeant controlled current cell."));
}

CommandResult SergeantReleaseCommand::execute(GameSession &session) const
{
    QString error;
    if (!session.canUsePrimaryAction(error)) {
        return failure(error);
    }

    if (!sergeantReleaseCurrentPlayer(session.state(), error)) {
        return failure(error);
    }

    session.markActionUsed();
    return success(QStringLiteral("Sergeant released enemy-controlled cell."));
}

CommandResult SwitchAgentCommand::execute(GameSession &session) const
{
    QString error;
    if (!session.canSwitchAgent(error)) {
        return failure(error);
    }

    if (!switchTurnAgent(session.state(), error)) {
        return failure(error);
    }

    return success(QStringLiteral("Active agent switched to %1.")
                       .arg(agentTypeName(session.state().turn.activeCard.agent)));
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
