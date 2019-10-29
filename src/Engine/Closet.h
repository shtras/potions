#pragma once

#include <map>
#include <list>
#include <memory>

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
    bool FromJson(const rapidjson::Value::ConstObject& o);

private:
    World* world_ = nullptr;
    std::map<int, std::list<Card*>> cont_;
};
} // namespace Engine
