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
    const auto& cards = d["cards"];
    if (!cards || cards.type() != bsoncxx::type::k_document) {
        return false;
    }
    if (!parseCardsRange(cards.get_document().view(), DeckType::Base)) {
        return false;
    }

    const auto& uniCards = d["uniCards"];
    if (!uniCards || uniCards.type() != bsoncxx::type::k_document) {
        return false;
    }
    if (!parseCardsRange(uniCards.get_document().view(), DeckType::University)) {
        return false;
    }
    auto readList = [&](std::string name, std::set<int>& list) {
        const auto& l = d[name];
        if (!l || l.type() != bsoncxx::type::k_array) {
            return false;
        }
        for (const auto& i : l.get_array().value) {
            if (l.type() != bsoncxx::type::k_int32) {
                return false;
            }
            list.insert(i.get_int32().value);
        }
        return true;
    };
    if (!readList("critters", critters_)) {
        return false;
    }
    if (!readList("universalRecipies", universalRecipies_)) {
        return false;
    }
    if (!readList("universalIngresients", universalIngresients_)) {
        return false;
    }
    return true;
}

void World::PrepareDeck(std::vector<Card*>& deck, DeckType type) const
{
    deck.clear();
    deck.reserve(cards_.size());
    for (const auto& p : cards_) {
        auto card = p.second.get();
        if (GetCardType(card) != type) {
            continue;
        }
        deck.push_back(card);
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
    if (idx <= deckBounds_.at(DeckType::Base)) {
        return DeckType::Base;
    }
    if (idx <= deckBounds_.at(DeckType::University)) {
        return DeckType::University;
    }
    assert(idx <= deckBounds_.at(DeckType::Guild));
    return DeckType::Guild;
}

World::DeckType World::DeckFromString(const std::string& type) const
{
    if (type == "base") {
        return DeckType::Base;
    } else if (type == "university") {
        return DeckType::University;
    } else if (type == "guild") {
        return DeckType::Guild;
    }
    assert(0);
    return DeckType::Unknown;
}

std::string World::DeckToString(DeckType type) const
{
    switch (type) {
        case DeckType::Base:
            return "base";
        case DeckType::University:
            return "university";
        case DeckType::Guild:
            return "guild";
        default:
            assert(0);
    }
    return "unknown";
}

void World::ActivateExpansion()
{
    rules_->MinCardsInHand = 6;
    rules_->MaxHandToDraw = 8;
}

bool World::IsCritter(Card* c) const
{
    return critters_.count(c->GetID()) > 0;
}

bool World::IsUniversalRecipe(Card* c) const
{
    return universalRecipies_.count(c->GetID()) > 0;
}

bool World::HasUniversalIngredient(Card* c) const
{
    return universalIngredients_.count(c->GetID()) > 0;
}
} // namespace Engine
