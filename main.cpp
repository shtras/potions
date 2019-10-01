#include <iostream>
#include "rapidjson/document.h"

int main()
{
    rapidjson::Document d;
    d.Parse("{\"a\":\"b\"}");
    if (d.HasParseError()) {
        auto err = d.GetParseError();
        int a = 0;
    }
    rapidjson::Value::Object o = d.GetObject();
    std::cout << "Hello" << std::endl;
    return 0;
}