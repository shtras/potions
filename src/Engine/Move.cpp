#include "Utils/Utils.h"

#include "Move.h"

namespace Engine
{
Move::Move(std::string user)
    : user_(std::move(user))
{
}

void Move::ToJson(bsoncxx::builder::stream::document& d) const
{
    d << "user" << user_;
    auto action = d << "action";
    switch (action_) {
        case Engine::Move::Action::Draw:
            action << "draw";
            break;
        case Engine::Move::Action::Skip:
            action << "skip";
            break;
        case Engine::Move::Action::Assemble:
            action << "assemble";
            break;
        case Engine::Move::Action::Discard:
            action << "discard";
            break;
        case Engine::Move::Action::Cast:
            action << "cast";
            break;
        case Engine::Move::Action::EndTurn:
            action << "endturn";
            break;
        default:
            action << "unknown";
            break;
    }
    d << "card" << card_;
    bsoncxx::builder::stream::array arr;
    for (const auto& part : parts_) {
        bsoncxx::builder::stream::document partd;
        auto typed = partd << "id" << part.id << "type";
        switch (part.type) {
            case Requirement::Type::Ingredient:
                typed << "ingredient";
                break;
            case Requirement::Type::Recipe:
                typed << "recipe";
                break;
            case Requirement::Type::None:
                typed << "none";
                break;
            default:
                typed << "unknown";
                break;
        }
        arr << partd;
    }
    d << "parts" << arr;
}

bool Move::FromJson(const bsoncxx::document::view& bson)
{
    const auto& action = bson["action"];
    if (!action || action.type() != bsoncxx::type::k_utf8) {
        return false;
    }
    std::string actionStr(action.get_utf8().value);
    const auto& card = bson["card"];
    if (card) {
        if (card.type() != bsoncxx::type::k_int32) {
            return false;
        }
        card_ = card.get_int32().value;
    }
    if (actionStr == "draw") {
        action_ = Action::Draw;
    } else if (actionStr == "skip") {
        action_ = Action::Skip;
    } else if (actionStr == "discard") {
        action_ = Action::Discard;
        if (card_ == -1) {
            return false;
        }
    } else if (actionStr == "assemble") {
        action_ = Action::Assemble;
        if (card_ == -1) {
            return false;
        }
    } else if (actionStr == "cast") {
        action_ = Action::Cast;
        if (card_ == -1) {
            return false;
        }
    } else if (actionStr == "endturn") {
        action_ = Action::EndTurn;
    } else {
        return false;
    }

    if (action_ == Action::Assemble || action_ == Action::Cast) {
        const auto& parts = bson["parts"];
        if (!parts || parts.type() != bsoncxx::type::k_array) {
            return false;
        }
        for (const auto& part : parts.get_array().value) {
            const auto& type = part["type"];
            if (!type || type.type() != bsoncxx::type::k_utf8) {
                return false;
            }
            std::string typeStr(type.get_utf8().value);
            auto partType = Requirement::Type::None;
            if (typeStr == "ingredient") {
                partType = Requirement::Type::Ingredient;
            } else if (typeStr == "recipe") {
                partType = Requirement::Type::Recipe;
            } else {
                return false;
            }
            const auto& id = part["id"];
            if (!id || id.type() != bsoncxx::type::k_int32) {
                return false;
            }
            parts_.emplace_back();
            parts_.back().id = id.get_int32().value;
            parts_.back().type = partType;
        }
    }
    return true;
}

bool Move::Parse(std::string moveJson)
{
    return FromJson(bsoncxx::from_json(moveJson));
}

Move::Action Move::GetAction() const
{
    return action_;
}

const std::string& Move::GetUser() const
{
    return user_;
}

int Move::GetCard() const
{
    return card_;
}

std::vector<Card*> Move::GetParts(World* world) const
{
    std::vector<Card*> res;
    res.reserve(parts_.size());
    for (auto part : parts_) {
        res.push_back(world->GetCard(part.id));
    }
    return res;
}

} // namespace Engine
