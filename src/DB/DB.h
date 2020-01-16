#pragma once

#include <string>
#include <optional>

#include "bsoncxx_wrap.h"

namespace DB
{
class DB
{
public:
    static DB& Instance();

    void test();
    void SetDbName(std::string name);

    std::string Insert(std::string collection, std::string object);
    std::optional<bsoncxx::document::value> Get(
        std::string collection, bsoncxx::document::view query);
    bsoncxx::builder::stream::array Find(std::string collection, bsoncxx::document::view query);
    //    mongocxx::cursor Find1(std::string collection, std::string query);
    int Update(std::string collection, std::string filter, bsoncxx::document::view query);
    void UpdateLegacy(std::string collection, std::string filter, std::string query);
    void Replace(std::string collection, std::string filter, bsoncxx::builder::stream::document& d);
    void Delete(std::string collection, std::string filter);

private:
    DB();
    struct Impl;
    std::unique_ptr<Impl> impl;
    std::string dbName_ = "potions";
};

} // namespace DB
