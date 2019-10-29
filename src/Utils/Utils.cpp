#include <sstream>
#include <fstream>

#include "Utils.h"

namespace Utils
{
std::string ReadFile(std::string& fileName)
{
    std::ifstream t(fileName);
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
    for (int i = 0; i < 32; ++i) {
        res[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return res;
}
} // namespace Utils
