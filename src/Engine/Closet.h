#pragma once

#include <map>
#include <list>
#include <memory>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "bsoncxx_wrap.h"

#include "World.h"
#include "Card.h"

namespace Engine
{
class Closet
{
public:
    Closet(World* w);

    void AddCard(Card* card);
    bool CanRemoveCard(Card* card);
    bool RemoveCard(Card* card);
    bool FromJson(const bsoncxx::document::view& bson);
    void ToJson(bsoncxx::builder::stream::value_context<bsoncxx::builder::stream::key_context<>> d) const;
    bool HasIngredient(int idx) const;

private:
    World* world_ = nullptr;
    std::map<int, std::list<Card*>> cont_;
};
} // namespace Engine
