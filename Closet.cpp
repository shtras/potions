#include "Closet.h"

void Closet::AddCard(std::shared_ptr<Card> card)
{
    cont_[card->GetIngredient()].push_back(card);
}

bool Closet::CanRemoveCard(std::shared_ptr<Card> card)
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

bool Closet::RemoveCard(std::shared_ptr<Card> card)
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
