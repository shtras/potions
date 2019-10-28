#pragma once
#include <string>
#include "Card.h"

namespace Engine
{
class Move
{
public:
    enum class Action { Draw, Skip, Assemble, Discard, Cast };
    struct Part
    {
        int id;
        Requirement::Type type;
    };
    bool Parse(std::string moveJson);
    Action GetAction() const;

private:
    Action action_ = Action::Skip;
    int card_ = -1;
    std::list<Part> parts_;
};
} // namespace Engine
