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
    d << "action" << std::string(actionNames_.at(action_));
    d << "card" << card_;
    if (ingredient_ >= 0) {
        d << "ingredient" << ingredient_;
    }
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
    d << "deck" << deckType_;
}

Move::Action Move::actionFromString(std::string_view str)
{
    auto res = std::find_if(actionNames_.begin(), actionNames_.end(),
        [&str](const auto& p) { return p.second == str; });
    if (res == actionNames_.end()) {
        return Action::Unknown;
    }
    return (*res).first;
}

bool Move::FromJson(const bsoncxx::document::view& bson)
{
    parts_.clear();
    const auto& action = bson["action"];
    if (!action || action.type() != bsoncxx::type::k_utf8) {
        return false;
    }
    std::string actionStr(action.get_utf8().value);
    action_ = actionFromString(actionStr);
    if (action_ == Action::Unknown) {
        return false;
    }
    const auto& card = bson["card"];
    if (card) {
        if (card.type() != bsoncxx::type::k_int32) {
            return false;
        }
        card_ = card.get_int32().value;
    }
    const auto& ingredient = bson["ingredient"];
    if (ingredient) {
        if (ingredient.type() != bsoncxx::type::k_int32) {
            return false;
        }
        ingredient_ = ingredient.get_int32().value;
    }
    if (action_ == Action::Draw) {
        const auto& deckType = bson["deck"];
        if (deckType && deckType.type() == bsoncxx::type::k_utf8) {
            deckType_ = std::string(deckType.get_utf8().value);
        }
    }
    if (action_ == Action::Assemble || action_ == Action::Discard || action_ == Action::Cast) {
        if (card_ == -1) {
            return false;
        }
    }

    if (action_ == Action::Assemble || action_ == Action::Cast) {
        const auto& parts = bson["parts"];
        if (parts) {
            if (parts.type() != bsoncxx::type::k_array) {
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

const std::string& Move::GetDeckType() const
{
    return deckType_;
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

int Move::GetIngredient() const
{
    return ingredient_;
}

} // namespace Engine
