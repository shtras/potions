#include "Utils/Utils.h"

#include "Move.h"

namespace Engine
{
Move::Move(std::string& user)
    : user_(user)
{
}

bool Move::FromJson(const rapidjson::Value::ConstObject& o)
{
    auto actionO = Utils::GetT<std::string>(o, "action");
    if (!actionO) {
        return false;
    }
    auto cardO = Utils::GetT<int>(o, "card");
    if (cardO) {
        card_ = *cardO;
    }
    auto action = *actionO;
    if (action == "draw") {
        action_ = Action::Draw;
    } else if (action == "skip") {
        action_ = Action::Skip;
    } else if (action == "discard") {
        action_ = Action::Discard;
        if (card_ == -1) {
            return false;
        }
    } else if (action == "assemble") {
        action_ = Action::Assemble;
        if (card_ == -1) {
            return false;
        }
        auto partsO = Utils::GetT<rapidjson::Value::ConstArray>(o, "parts");
        if (!partsO) {
            return false;
        }
        const auto& parts = *partsO;
        for (rapidjson::SizeType i = 0; i < parts.Size(); ++i) {
            const auto& part = parts[i];
            auto typeO = Utils::GetT<std::string>(part, "type");
            if (!typeO) {
                return false;
            }
            auto type = Requirement::Type::None;
            if (*typeO == "ingredient") {
                type = Requirement::Type::Ingredient;
            } else if (*typeO == "recipe") {
                type = Requirement::Type::Recipe;
            } else {
                return false;
            }
            auto idO = Utils::GetT<int>(part, "id");
            if (!idO) {
                return false;
            }
            parts_.emplace_back();
            parts_.back().id = *idO;
            parts_.back().type = type;
        }
    } else if (action == "cast") {
        action_ = Action::Cast;
        if (!card_) {
            return false;
        }
    } else if (action == "endturn") {
        action_ = Action::EndTurn;
    } else {
        return false;
    }
    return true;
}

bool Move::Parse(std::string moveJson)
{
    rapidjson::Document d;
    d.Parse(moveJson);
    if (d.HasParseError()) {
        return false;
    }
    const auto& o = d.Get<rapidjson::Value::ConstObject>();
    return FromJson(o);
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

std::set<Card*> Move::GetParts(World* world) const
{
    std::set<Card*> res;
    for (auto part : parts_) {
        res.insert(world->GetCard(part.id));
    }
    return res;
}

} // namespace Engine
