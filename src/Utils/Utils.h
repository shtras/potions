#pragma once

#include <string>
#include <optional>

#include "spdlog_wrap.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

namespace Utils
{
std::string ReadFile(std::string& fileName);

template <typename T>
std::optional<T> GetT(const rapidjson::Value::ValueType& o, const char* name)
{
    if (!o.HasMember(name) || !o[name].Is<T>()) {
        spdlog::error("Json value did not contain {0}", name);
        return {};
    }
    return o[name].Get<T>();
}

template <typename T>
std::optional<T> GetT(const rapidjson::Value::ValueType& o)
{
    if (!o.Is<T>()) {
        spdlog::error("Json value is unexpected");
        return {};
    }
    return o.Get<T>();
}


} // namespace Utils
