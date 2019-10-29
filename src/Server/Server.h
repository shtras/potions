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

#include <set>
#include <chrono>

namespace Server
{
struct Session
{
    std::string id;
    std::string user;
    std::set<std::string> games;
    std::chrono::system_clock::time_point expiration;
};

class Server
{
    using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
    using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;

public:
    void Start();
    void Stop();

private:
    void login(HttpServer::Response* response, HttpServer::Request* request);
    Session* getSession(std::string_view id);
    std::string createSession();

    std::unique_ptr<HttpServer> server = nullptr;
    std::thread server_thread;
    std::map<std::string, std::unique_ptr<Session>, std::less<>> sessions_;
};
} // namespace Server
