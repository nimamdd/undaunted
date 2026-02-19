#pragma once

#include "SessionTypes.h"

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

class ScoutMarkCommand final : public ActionCommand
{
public:
    CommandResult execute(GameSession &session) const override;
};

class SergeantControlCommand final : public ActionCommand
{
public:
    CommandResult execute(GameSession &session) const override;
};

class SergeantReleaseCommand final : public ActionCommand
{
public:
    CommandResult execute(GameSession &session) const override;
};

class SwitchAgentCommand final : public ActionCommand
{
public:
    CommandResult execute(GameSession &session) const override;
};

class EndTurnCommand final : public ActionCommand
{
public:
    CommandResult execute(GameSession &session) const override;
};

} // namespace model
