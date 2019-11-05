#include "Utils/Utils.h"

#include "Closet.h"

namespace Engine
{
Closet::Closet(World* w)
    : world_(w)
{
}

bool Closet::FromJson(const bsoncxx::document::view& bson)
{
    for (auto elm : bson) {
        std::string ingIdxStr(elm.key());
        if (elm.type() != bsoncxx::type::k_array) {
            return false;
        }
        for (const auto& cardElm : elm.get_array().value) {
            if (cardElm.type() != bsoncxx::type::k_int32) {
                return false;
            }
            auto card = cardElm.get_int32().value;
            AddCard(world_->GetCard(card));
        }
    }
    return true;
}

void Closet::ToJson(bsoncxx::builder::stream::value_context<bsoncxx::builder::stream::key_context<>> d) const
{
    bsoncxx::builder::stream::document cd;
    for (const auto& pair : cont_) {
        bsoncxx::builder::stream::array a;
        for (const auto& card : pair.second) {
            a << card->GetID();
        }
        cd << std::to_string(pair.first) << a;
    }
    d << cd;
}

void Closet::AddCard(Card* card)
{
    assert(card);
    if (card->GetType() == Card::Type::Spell) {
        cont_[card->GetIngredient()].push_front(card);
    } else {
        cont_[card->GetIngredient()].push_back(card);
    }
}

bool Closet::CanRemoveCard(Card* card)
{
    auto ingredient = card->GetIngredient();
    if (cont_.count(ingredient) == 0) {
        return false;
    }
    if (cont_.at(ingredient).back() != card) {
        return false;
    }
    return true;
}

bool Closet::RemoveCard(Card* card)
{
    assert(CanRemoveCard(card));
    auto ingredient = card->GetIngredient();
    cont_.at(ingredient).pop_back();
    if (cont_.at(ingredient).size() == 0) {
        cont_.erase(ingredient);
    }
    return true;
}

bool Closet::HasIngredient(int idx) const
{
    return cont_.count(idx) > 0;
}
} // namespace Engine
