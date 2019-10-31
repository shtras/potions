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

#include "spdlog_wrap.h"

#include "DB.h"

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
    //Delete("games", "{\"_id\": { \"$oid\" : \"5dbb039d29d4711cfe2c26e3\"}}");
}

std::string DB::Get(std::string collection, std::string query)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client["potions"];
    mongocxx::collection coll = db[collection];
    auto res = coll.find_one(bsoncxx::from_json(query));
    if (res) {
        return bsoncxx::to_json(*res);
    }
    return "";
}

std::string DB::Insert(std::string collection, std::string object)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client["potions"];
    mongocxx::collection coll = db[collection];
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result = coll.insert_one(bsoncxx::from_json(object));
    if (result) {
        return (*result).inserted_id().get_oid().value.to_string();
    }
    return "";
}

void DB::Update(std::string collection, std::string filter, std::string query)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client["potions"];
    mongocxx::collection coll = db[collection];
    coll.update_many(bsoncxx::from_json(filter), bsoncxx::from_json(query));
}

void DB::Delete(std::string collection, std::string id)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client["potions"];
    mongocxx::collection coll = db[collection];
    coll.delete_one(bsoncxx::from_json(id));
}

} // namespace DB
