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
    for (const auto& elm : cards.get_document().view()) {
        if (elm.type() != bsoncxx::type::k_document) {
            return false;
        }
        int idx = std::stoi(std::string(elm.key()));
        if (cards_.count(idx) > 0) {
            return false;
        }
        cards_[idx] = std::make_unique<Card>();
        bool res = cards_[idx]->Parse(idx, elm.get_document().view());
        if (!res) {
            return false;
        }
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
} // namespace Engine
