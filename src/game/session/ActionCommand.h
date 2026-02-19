#pragma once

#include "SessionTypes.h"
#include "../agents/AgentBehavior.h"

#include <QString>

namespace model {

class GameSession;

class ActionCommand
{
public:
    virtual ~ActionCommand() = default;
    virtual CommandResult execute(GameSession &session) const = 0;
};

class MoveCommand final : public ActionCommand
{
public:
    explicit MoveCommand(QString targetCellId);
    CommandResult execute(GameSession &session) const override;

private:
    QString targetCellId_;
};

class AttackCommand final : public ActionCommand
{
public:
    explicit AttackCommand(QString targetCellId);
    CommandResult execute(GameSession &session) const override;

private:
    QString targetCellId_;
};

class UseAgentSpecialCommand final : public ActionCommand
{
public:
    explicit UseAgentSpecialCommand(AgentSpecialAction action);
    CommandResult execute(GameSession &session) const override;

private:
    AgentSpecialAction action_;
};

} // namespace model
