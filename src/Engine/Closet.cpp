#include "Utils/Utils.h"

#include "Closet.h"

namespace Engine
{
Closet::Closet(World* w)
    : world_(w)
{
}

bool Closet::FromJson(const rapidjson::Value::ConstObject& o)
{
    for (auto itr = o.MemberBegin(); itr != o.MemberEnd(); ++itr) {
        if (!itr->value.IsArray()) {
            return false;
        }
        const auto& cards = itr->value.GetArray();
        for (rapidjson::SizeType i = 0; i < cards.Size(); ++i) {
            auto cardO = Utils::GetT<int>(cards[i]);
            if (!cardO) {
                return false;
            }
            int card = *cardO;
            AddCard(world_->GetCard(card));
        }
    }
    return true;
}

void Closet::AddCard(Card* card)
{
    assert(card);
    cont_[card->GetIngredient()].push_back(card);
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
    if (!CanRemoveCard(card)) {
        return false;
    }
    auto ingredient = card->GetIngredient();
    cont_.at(ingredient).pop_back();
    if (cont_.at(ingredient).size() == 0) {
        cont_.erase(ingredient);
    }
    return true;
}
} // namespace Engine
