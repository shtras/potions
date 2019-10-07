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
    str.reserve(t.tellg());
    t.seekg(0, std::ios::beg);

    str.assign(std::istreambuf_iterator<char>(t), std::istreambuf_iterator<char>());
    return str;
}
} // namespace Utils
