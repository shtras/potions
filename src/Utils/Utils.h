#pragma once

#include <string>
#include <optional>

#include "spdlog_wrap.h"
#include "bsoncxx_wrap.h"

namespace Utils
{
std::string ReadFile(std::string& fileName);
std::string MakeUUID();
int64_t GetTime();
std::optional<bsoncxx::document::value> ParseBson(const std::string& s);
} // namespace Utils
