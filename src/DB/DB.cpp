#include <string>
#include <sstream>
#include <iostream>

#ifdef _MSC_VER
#pragma warning(push, 0)
#pragma warning(disable : 4265)
#endif
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

void foo(bsoncxx::builder::stream::value_context<bsoncxx::builder::stream::key_context<>> d)
{
    d << "b";
}

void DB::test()
{
    bsoncxx::builder::stream::document d;
    bsoncxx::builder::stream::document d1;
    bsoncxx::builder::stream::array a;
    auto t = (d << "a");
    foo(t);

    auto t1 = d << "ggg" << bsoncxx::builder::stream::open_array;
    t1 << 1;
    t1 << 2;
    t1 << bsoncxx::builder::stream::close_array;
    std::cout << bsoncxx::to_json(d);
    int aaa = 0;
}

std::string DB::LegacyGet(std::string collection, std::string query)
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

std::optional<bsoncxx::document::value> DB::Get(std::string collection, std::string query)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client["potions"];
    mongocxx::collection coll = db[collection];
    auto res = coll.find_one(bsoncxx::from_json(query));
    if (res) {
        return *res;
    }
    return {};
}

std::string DB::Find(std::string collection, std::string query)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client["potions"];
    mongocxx::collection coll = db[collection];
    auto res = coll.find(bsoncxx::from_json(query));
    std::stringstream ss;
    ss << "[";
    auto itr = res.begin();
    if (itr != res.end()) {
        ss << bsoncxx::to_json(*itr);
        ++itr;
    }
    for (; itr != res.end(); ++itr) {
        ss << "," << bsoncxx::to_json(*itr);
    }
    ss << "]";
    return ss.str();
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

void DB::Replace(std::string collection, std::string filter, bsoncxx::builder::stream::document& d)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client["potions"];
    mongocxx::collection coll = db[collection];
    coll.replace_one(bsoncxx::from_json(filter), d.view());
}

void DB::Delete(std::string collection, std::string filter)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client["potions"];
    mongocxx::collection coll = db[collection];
    coll.delete_one(bsoncxx::from_json(filter));
}

} // namespace DB
