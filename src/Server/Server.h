#pragma once

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#include "simple-web-server/server_http.hpp"
#include "simple-web-server/server_https.hpp"
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

namespace Server
{
class Server
{
    using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
    using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;

public:
    void Start();
    void Stop();

private:
    std::unique_ptr<HttpServer> server = nullptr;
    std::thread server_thread;
};
} // namespace Server
