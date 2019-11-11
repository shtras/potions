#pragma once

#include <string>
#include <optional>

#include "spdlog_wrap.h"
#include "bsoncxx_wrap.h"

namespace Utils
{
template <typename E>
using enable_enum_t = std::enable_if_t<std::is_enum_v<E>, std::underlying_type_t<E>>;

template <typename E>
constexpr enable_enum_t<E> enum_value(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}

template <typename E, typename T = int>
constexpr std::enable_if_t<std::is_enum_v<E> && std::is_integral_v<T>, E> to_enum(T value) noexcept
{
    return static_cast<E>(value);
}

std::string ReadFile(std::string& fileName);
std::string MakeUUID();
int64_t GetTime();
std::optional<bsoncxx::document::value> ParseBson(const std::string& s);
} // namespace Utils
