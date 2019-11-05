#pragma once

#include <string>
#include <optional>

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
