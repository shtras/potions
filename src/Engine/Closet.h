#pragma once

#include <map>
#include <list>
#include <memory>

#include "Card.h"

namespace Engine
{
class Closet
{
public:
    void AddCard(Card* card);
    bool CanRemoveCard(Card* card);
    bool RemoveCard(Card* card);

private:
    std::map<int, std::list<Card*>> cont_;
};
} // namespace Engine
