#pragma once

#include <map>
#include <list>
#include <memory>

#include "Card.h"

class Closet
{
public:
    void AddCard(std::shared_ptr<Card> card);
    bool CanRemoveCard(std::shared_ptr<Card> card);
    bool RemoveCard(std::shared_ptr<Card> card);

private:
    std::map<int, std::list<std::shared_ptr<Card>>> cont_;
};
