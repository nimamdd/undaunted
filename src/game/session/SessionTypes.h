#pragma once

#include <QString>

namespace model {

struct CommandResult {
    bool ok{false};
    QString message;
};

} // namespace model
