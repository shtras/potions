#include <random>
#include <algorithm>

#include "Utils/Utils.h"

#include "World.h"

namespace Engine
{
Card* World::GetCard(int idx) const
{
    if (cards_.count(idx) == 0) {
        return nullptr;
    }
    return cards_.at(idx).get();
}

bool World::parseCardsRange(const bsoncxx::document::view& bson, DeckType type)
{
    int maxIdx = -1;
    for (const auto& elm : bson) {
        if (elm.type() != bsoncxx::type::k_document) {
            return false;
        }
        int idx = std::stoi(std::string(elm.key()));
        maxIdx = std::max(maxIdx, idx);
        if (cards_.count(idx) > 0) {
            return false;
        }
        cards_[idx] = std::make_unique<Card>();
        bool res = cards_[idx]->Parse(idx, elm.get_document().view());
        if (!res) {
            return false;
        }
    }
    deckBounds_[type] = maxIdx;
    return true;
}

bool World::ParseCards(std::string filename)
{
    auto cont = Utils::ReadFile(filename);
    auto maybeD = Utils::ParseBson(cont);
    if (!maybeD) {
        return false;
    }
    auto d = (*maybeD).view();
    auto cards = d["cards"];
    if (!cards || cards.type() != bsoncxx::type::k_document) {
        return false;
    }
    if (!parseCardsRange(cards.get_document().view(), DeckType::Base)) {
        return false;
    }

    auto uniCards = d["uniCards"];
    if (!uniCards || uniCards.type() != bsoncxx::type::k_document) {
        return false;
    }
    if (!parseCardsRange(uniCards.get_document().view(), DeckType::University)) {
        return false;
    }
    return true;
}

void World::PrepareDeck(std::vector<Card*>& deck) const
{
    deck.clear();
    deck.reserve(cards_.size());
    for (const auto& p : cards_) {
        deck.push_back(p.second.get());
    }
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(deck.begin(), deck.end(), g);
}

Rules* World::GetRules() const
{
    return rules_.get();
}

World::DeckType World::GetCardType(Card* c) const
{
    auto idx = c->GetID();
    if (idx < deckBounds_.at(DeckType::Base)) {
        return DeckType::Base;
    }
    if (idx < deckBounds_.at(DeckType::University)) {
        return DeckType::University;
    }
    assert(idx < deckBounds_.at(DeckType::Guild));
    return DeckType::Guild;
}
} // namespace Engine
