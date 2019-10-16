#pragma once

#include <string>
#include <optional>

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

namespace Utils
{
std::string ReadFile(std::string& fileName);

template <typename T>
std::optional<T> GetT(const rapidjson::Value::ValueType& o, const char* name)
{
    if (!o.HasMember(name) || !o[name].Is<T>()) {
        return {};
    }
    return o[name].Get<T>();
}

} // namespace Utils
