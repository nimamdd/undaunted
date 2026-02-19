#include "AgentBehavior.h"

#include "../actions/TacticalActions.h"

namespace model {

namespace {

bool isMarkedForOwner(const CellNode *cell, PlayerId owner)
{
    if (cell == nullptr) {
        return false;
    }

    if (owner == PlayerId::A) {
        return cell->markedByA;
    }
    if (owner == PlayerId::B) {
        return cell->markedByB;
    }
    return false;
}

class ScoutBehavior final : public AgentBehavior
{
public:
    bool canMoveTo(const GameState &, PlayerId, const CellNode *, QString &) const override
    {
        return true;
    }

    int attackDiceCount() const override
    {
        return 1;
    }

    bool supportsSpecial(AgentSpecialAction action) const override
    {
        return action == AgentSpecialAction::ScoutMark;
    }

    bool executeSpecial(GameState &state,
                        PlayerId owner,
                        AgentSpecialAction action,
                        QString &errorMessage) const override
    {
        if (action != AgentSpecialAction::ScoutMark) {
            errorMessage = QStringLiteral("Scout does not support this special action.");
            return false;
        }

        return scoutMark(state, owner, errorMessage);
    }
};

class SniperBehavior final : public AgentBehavior
{
public:
    bool canMoveTo(const GameState &, PlayerId owner, const CellNode *to, QString &errorMessage) const override
    {
        if (isMarkedForOwner(to, owner)) {
            return true;
        }

        errorMessage = QStringLiteral("Sniper can only move to marked cells.");
        return false;
    }

    int attackDiceCount() const override
    {
        return 3;
    }

    bool supportsSpecial(AgentSpecialAction) const override
    {
        return false;
    }

    bool executeSpecial(GameState &, PlayerId, AgentSpecialAction, QString &errorMessage) const override
    {
        errorMessage = QStringLiteral("Sniper has no special action.");
        return false;
    }
};

class SergeantBehavior final : public AgentBehavior
{
public:
    bool canMoveTo(const GameState &, PlayerId owner, const CellNode *to, QString &errorMessage) const override
    {
        if (isMarkedForOwner(to, owner)) {
            return true;
        }

        errorMessage = QStringLiteral("Sergeant can only move to marked cells.");
        return false;
    }

    int attackDiceCount() const override
    {
        return 1;
    }

    bool supportsSpecial(AgentSpecialAction action) const override
    {
        return action == AgentSpecialAction::SergeantControl ||
               action == AgentSpecialAction::SergeantRelease;
    }

    bool executeSpecial(GameState &state,
                        PlayerId owner,
                        AgentSpecialAction action,
                        QString &errorMessage) const override
    {
        if (action == AgentSpecialAction::SergeantControl) {
            return sergeantControl(state, owner, errorMessage);
        }
        if (action == AgentSpecialAction::SergeantRelease) {
            return sergeantRelease(state, owner, errorMessage);
        }

        errorMessage = QStringLiteral("Sergeant does not support this special action.");
        return false;
    }
};

const ScoutBehavior kScoutBehavior;
const SniperBehavior kSniperBehavior;
const SergeantBehavior kSergeantBehavior;

} // namespace

const AgentBehavior *behaviorFor(AgentType type)
{
    switch (type) {
    case AgentType::Scout:
        return &kScoutBehavior;
    case AgentType::Sniper:
        return &kSniperBehavior;
    case AgentType::Sergeant:
        return &kSergeantBehavior;
    }

    return nullptr;
}

QString specialActionName(AgentSpecialAction action)
{
    switch (action) {
    case AgentSpecialAction::ScoutMark:
        return QStringLiteral("Scout Mark");
    case AgentSpecialAction::SergeantControl:
        return QStringLiteral("Sergeant Control");
    case AgentSpecialAction::SergeantRelease:
        return QStringLiteral("Sergeant Release");
    }

    return QStringLiteral("Special Action");
}

} // namespace model
