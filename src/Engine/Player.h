#pragma once

#include <set>
#include <memory>

#include "bsoncxx_wrap.h"

#include "World.h"
#include "Card.h"

namespace Engine
{
class Player
{
public:
    Player(World* w, std::string& user);

    void AddCard(Card* card);
    size_t HandSize() const;
    void DiscardCard(Card* card);
    bool HasCard(Card* card) const;
    bool HasAssembled(Card* card) const;
    void AddAssembled(Card* card);
    void RemoveAssembled(Card* card);
    Card* FindAssembledWithPart(Card* part) const;
    void ToJson(bsoncxx::builder::stream::document& d, bool hidden = false) const;
    bool FromJson(const bsoncxx::document::view& bson);
    const std::string& GetUser() const;
    const std::set<Card*> GetAssembledCards() const;
    void AddScore(int score);

private:
    bool removeFromHand(Card* card);

    World* world_ = nullptr;
    int score_ = 0;
    std::set<Card*> hand_;
    std::set<Card*> assembledCards_;
    std::string user_;
};
} // namespace Engine
