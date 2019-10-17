#include "Closet.h"

namespace Engine
{
void Closet::AddCard(Card* card)
{
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
