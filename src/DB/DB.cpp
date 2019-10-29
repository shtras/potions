#include "DB.h"

#ifdef _MSC_VER
#pragma warning(push, 0)
#pragma warning(disable : 4265)
#endif
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/types.hpp>

#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace DB
{
DB& DB::Instance()
{
    static DB db;
    return db;
}

DB::DB()
{
    mongocxx::instance instance{};
}

void DB::test()
{
    Insert("test", "{\"abc\": \"def\"}");
}

void DB::Insert(std::string collection, std::string object)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client["potions"];
    mongocxx::collection coll = db[collection];
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result = coll.insert_one(bsoncxx::from_json(object));
}
} // namespace DB
