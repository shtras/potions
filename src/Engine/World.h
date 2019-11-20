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
    enum class DeckType { Unknown = -1, Base = 0, University = 1, Guild = 2 };

    bool ParseCards(std::string filename);
    Card* GetCard(int idx) const;
    Rules* GetRules() const;
    void PrepareDeck(std::vector<Card*>& deck, DeckType type) const;
    DeckType GetCardType(Card* c) const;
    DeckType DeckFromString(const std::string& type) const;
    std::string DeckToString(DeckType type) const;
    void ActivateExpansion();
    bool isCritter(Card* c) const;

private:
    bool parseCardsRange(const bsoncxx::document::view& bson, DeckType type);
    std::unique_ptr<Rules> rules_ = std::make_unique<Rules>();
    std::map<int, std::unique_ptr<Card>> cards_ = {};
    std::map<DeckType, int> deckBounds_{
        {DeckType::Base, -1}, {DeckType::University, -1}, {DeckType::Guild, -1}};
    std::set<int> critters_;
};
} // namespace Engine
