#pragma once

#include <map>
#include <list>
#include <memory>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

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
    void ToJson(rapidjson::Writer<rapidjson::StringBuffer>& w) const;

private:
    World* world_ = nullptr;
    std::map<int, std::list<Card*>> cont_;
};
} // namespace Engine
