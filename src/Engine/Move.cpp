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
    auto cardO = Utils::GetT<int>(d, "card");
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
        auto partsO = Utils::GetT<rapidjson::Value::ConstArray>(d, "parts");
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
