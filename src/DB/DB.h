#pragma once

namespace DB
{
class DB
{
public:
    static DB& Instance();

    void test();

private:
    DB();
};

} // namespace DB
