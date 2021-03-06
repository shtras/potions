#include <string>
#include <sstream>
#include <iostream>

#include "spdlog_wrap.h"

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

#include "DB.h"

namespace DB
{
struct DB::Impl
{
    mongocxx::instance instance{};
};

DB& DB::Instance()
{
    static DB db;
    return db;
}

DB::DB()
    : impl(std::make_unique<Impl>())
{
}

void DB::test()
{
}

void DB::SetDbName(std::string name)
{
    dbName_ = std::move(name);
}

std::optional<bsoncxx::document::value> DB::Get(
    std::string collection, bsoncxx::document::view query)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client[dbName_];
    mongocxx::collection coll = db[collection];
    auto res = coll.find_one(query);
    if (res) {
        return *res;
    }
    return {};
}

bsoncxx::builder::stream::array DB::Find(std::string collection, bsoncxx::document::view query)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client[dbName_];
    mongocxx::collection coll = db[collection];
    auto cursor = coll.find(query);
    bsoncxx::builder::stream::array res;
    for (auto elm : cursor) {
        res << elm;
    }
    return res;
}

std::string DB::Insert(std::string collection, std::string object)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client[dbName_];
    mongocxx::collection coll = db[collection];
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result =
        coll.insert_one(bsoncxx::from_json(object));
    if (result) {
        return (*result).inserted_id().get_oid().value.to_string();
    }
    return "";
}

int DB::Update(std::string collection, std::string filter, bsoncxx::document::view query)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client[dbName_];
    mongocxx::collection coll = db[collection];
    auto res = coll.update_many(bsoncxx::from_json(filter), query);
    if (!res) {
        return 0;
    }
    return (*res).modified_count();
}

void DB::UpdateLegacy(std::string collection, std::string filter, std::string query)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client[dbName_];
    mongocxx::collection coll = db[collection];
    coll.update_many(bsoncxx::from_json(filter), bsoncxx::from_json(query));
}

void DB::Replace(std::string collection, std::string filter, bsoncxx::builder::stream::document& d)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client[dbName_];
    mongocxx::collection coll = db[collection];
    coll.replace_one(bsoncxx::from_json(filter), d.view());
}

void DB::Delete(std::string collection, std::string filter)
{
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client[dbName_];
    mongocxx::collection coll = db[collection];
    coll.delete_many(bsoncxx::from_json(filter));
}

} // namespace DB
