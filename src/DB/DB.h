#pragma once

#include <string>

namespace DB
{
class DB
{
public:
    static DB& Instance();

    void test();

    void Insert(std::string collection, std::string object);
    std::string Get(std::string collection, std::string query);

private:
    DB();
};

} // namespace DB
