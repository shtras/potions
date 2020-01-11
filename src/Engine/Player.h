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
    size_t AssembledSize() const;
    void DiscardCard(Card* card);
    bool HasCard(Card* card) const;
    bool HasAssembled(Card* card) const;
    void AddAssembled(Card* card);
    void RemoveAssembled(Card* card);
    Card* FindAssembledWithPart(Card* part) const;
    void ToJson(bsoncxx::builder::stream::document& d, bool hidden = false) const;
    bool FromJson(const bsoncxx::document::view& bson);
    bool HasCardWithIngredient(int id) const;
    bool HasAssembledCardWithParts() const;
    const std::string& GetUser() const;
    void AddScore(int score);
    bool HasTalisman(Card::TalismanType type) const;
    int GetScore() const;

private:
    bool removeFromHand(Card* card);
    void refreshTalismans();

    World* world_ = nullptr;
    int score_ = 0;
    std::set<Card*> hand_;
    std::set<Card*> assembledCards_;
    std::string user_;
    std::set<Card::TalismanType> talismans_;
};
} // namespace Engine
