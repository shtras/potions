#pragma once

#include <map>
#include <list>
#include <memory>

#include "bsoncxx_wrap.h"

#include "World.h"
#include "Card.h"

namespace Engine
{
class Closet
{
public:
    explicit Closet(World* w);

    void AddCard(Card* card);
    bool CanRemoveCard(Card* card) const;
    bool RemoveCard(Card* card);
    bool FromJson(const bsoncxx::document::view& bson);
    void ToJson(
        bsoncxx::builder::stream::value_context<bsoncxx::builder::stream::key_context<>> d) const;
    bool HasIngredient(int idx) const;
    void ActivateExpansion();

private:
    World* world_ = nullptr;
    std::map<int, std::list<Card*>> cont_;
    bool spellsOnTop_ = false;
};
} // namespace Engine
