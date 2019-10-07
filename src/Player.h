#pragma once

#include <set>
#include <memory>

#include "Card.h"

class Player
{
public:
    void AddCard(Card* card);
    size_t HandSize() const;
    bool DiscardCard(Card* card);
    bool HasCard(Card* card);
    void AddAssembled(Card* card);
    void RemoveAssembled(Card* card);

private:
    bool removeFromHand(Card* card);
    int score_ = 0;
    std::set<Card*> hand_;
    std::set<Card*> assembledCards_;
};
