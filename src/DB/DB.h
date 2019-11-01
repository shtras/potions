#pragma once

#include <string>

namespace DB
{
class DB
{
public:
    static DB& Instance();

    void test();

    std::string Insert(std::string collection, std::string object);
    std::string Get(std::string collection, std::string query);
    std::string Find(std::string collection, std::string query);
    void Update(std::string collection, std::string filter, std::string query);
    void Replace(std::string collection, std::string filter, std::string query);
    void Delete(std::string collection, std::string id);

private:
    DB();
};

} // namespace DB
