#pragma once

#include <map>
#include <list>
#include <memory>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#ifdef _MSC_VER
#pragma warning(push, 0)
#pragma warning(disable : 4265)
#endif
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/types.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

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
