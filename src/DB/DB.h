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

private:
    DB();
};

} // namespace DB
