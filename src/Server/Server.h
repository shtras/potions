#pragma once

#ifdef _MSC_VER
#pragma warning(push, 0)
#pragma warning(disable : 4265)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#include "simple-web-server/server_http.hpp"
#include "simple-web-server/server_https.hpp"
#ifdef _MSC_VER
#pragma warning(pop)
#else
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
