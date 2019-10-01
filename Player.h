#pragma once

#include <set>
#include <memory>

#include "Card.h"

class Player
{
public:
    void AddCard(std::shared_ptr<Card> card);
    size_t HandSize() const;
    bool DiscardCard(std::shared_ptr<Card> card);

private:
    bool removeFromHand(std::shared_ptr<Card> card);
    int score_ = 0;
    std::set<std::shared_ptr<Card>> hand_;
};
