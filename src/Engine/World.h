#pragma once

#include <memory>
#include <map>
#include <vector>

#include "Rules.h"
#include "Card.h"

namespace Engine
{
class World
{
public:
    bool ParseCards(std::string filename);
    Card* GetCard(int idx) const;
    Rules* GetRules() const;
    void PrepareDeck(std::vector<Card*>& deck) const;

private:
    std::unique_ptr<Rules> rules_ = std::make_unique<Rules>();
    std::map<int, std::unique_ptr<Card>> cards_ = {};
};
} // namespace Engine
