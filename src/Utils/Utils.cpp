#include <sstream>
#include <fstream>

#include "Utils.h"

namespace Utils
{
std::optional<bsoncxx::document::value> ParseBson(const std::string& s)
{
    try {
        auto res = bsoncxx::from_json(s);
        return res;
    } catch (...) {
        return {};
    }
}

std::string ReadFile(std::string_view fileName)
{
    std::ifstream t(fileName.data());
    if (!t.good()) {
        return "";
    }
    std::string str;
    t.seekg(0, std::ios::end);
    str.reserve(static_cast<size_t>(t.tellg()));
    t.seekg(0, std::ios::beg);

    str.assign(std::istreambuf_iterator<char>(t), std::istreambuf_iterator<char>());
    return str;
}

std::string MakeUUID()
{
    std::string res;
    res.resize(32);
    static constexpr char alphanum[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    for (size_t i = 0; i < 32; ++i) {
        res[i] = alphanum[static_cast<size_t>(rand()) % (sizeof(alphanum) - 1)];
    }
    return res;
}

int64_t GetTime()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    return now_ms.time_since_epoch().count();
}
} // namespace Utils
