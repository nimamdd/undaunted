#pragma once

#include "../model/Types.h"

namespace model {

enum class AgentSpecialAction {
    ScoutMark,
    SergeantControl,
    SergeantRelease,
};

class AgentBehavior
{
public:
    virtual ~AgentBehavior() = default;

    virtual AgentType type() const = 0;
    virtual bool canMoveTo(const GameState &state,
                           PlayerId owner,
                           const CellNode *to,
                           QString &errorMessage) const = 0;
    virtual int attackDiceCount() const = 0;

    virtual bool supportsSpecial(AgentSpecialAction action) const = 0;
    virtual bool executeSpecial(GameState &state,
                                PlayerId owner,
                                AgentSpecialAction action,
                                QString &errorMessage) const = 0;
};

const AgentBehavior *behaviorFor(AgentType type);
QString specialActionName(AgentSpecialAction action);

} // namespace model
