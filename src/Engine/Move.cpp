#include "Utils/Utils.h"

#include "Move.h"

namespace Engine
{
bool Move::Parse(std::string moveJson)
{
    rapidjson::Document d;
    d.Parse(moveJson);
    if (d.HasParseError()) {
        return false;
    }
    auto actionO = Utils::GetT<std::string>(d, "action");
    if (!actionO) {
        return false;
    }
    auto action = *actionO;
    if (action == "draw") {
        action_ = Action::Draw;
    } else if (action == "skip") {
        action_ = Action::Skip;
    } else if (action == "assemble") {
        action_ = Action::Assemble;
    } else if (action == "cast") {
        action_ = Action::Cast;
    } else {
        return false;
    }
    return true;
}

Move::Action Move::GetAction() const
{
    return action_;
}
} // namespace Engine
