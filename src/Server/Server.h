#pragma once

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif
#include "simple-web-server/server_http.hpp"
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

namespace Server
{
class Server
{
public:
    void Start();
    void Stop();

private:
    std::unique_ptr<SimpleWeb::Server<SimpleWeb::HTTP>> server = nullptr;
    std::thread server_thread;
};
} // namespace Server
