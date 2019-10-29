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
    rapidjson::Document d;
    d.Parse(cont);
    if (d.HasParseError()) {
        return false;
    }
    auto cardsO = Utils::GetT<rapidjson::Value::ConstObject>(d, "cards");
    if (!cardsO) {
        return false;
    }
    const auto& cards = *cardsO;
    for (auto itr = cards.MemberBegin(); itr != cards.MemberEnd(); ++itr) {
        if (!itr->value.IsObject()) {
            return false;
        }
        auto idxStr = itr->name.GetString();
        int idx = std::stoi(idxStr);
        if (cards_.count(idx) > 0) {
            return false;
        }
        cards_[idx] = std::make_unique<Card>();
        bool res = cards_[idx]->Parse(idx, itr->value);
        if (!res) {
            return false;
        }
    }
    if (cards_.size() != cards.MemberCount()) {
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
}
