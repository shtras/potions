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

    std::string Insert(std::string collection, std::string object);
    std::optional<bsoncxx::document::value> Get(std::string collection, std::string query);
    std::string Find(std::string collection, std::string query);
    void Update(std::string collection, std::string filter, std::string query);
    void Replace(std::string collection, std::string filter, bsoncxx::builder::stream::document& d);
    void Delete(std::string collection, std::string filter);

private:
    DB();
};

} // namespace DB
