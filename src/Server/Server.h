#pragma once

#include <set>
#include <chrono>

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

#include "Engine/Game.h"

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
    void ping(HttpServer::Response* response, HttpServer::Request* request);
    void createGame(HttpServer::Response* response, HttpServer::Request* request);
    void deleteGame(HttpServer::Response* response, HttpServer::Request* request);
    void addPlayer(HttpServer::Response* response, HttpServer::Request* request);
    Session* getSession(std::string_view id);
    std::string createSession(const std::string& userInfo);
    void retireSessions();
    void extendSession(Session* session);
    Session* validateRequest(HttpServer::Response* response, HttpServer::Request* request, rapidjson::Document& d);
    Engine::Game* findGame(std::string& id);

    void dumpGame(Engine::Game* g);

    std::unique_ptr<HttpServer> server = nullptr;
    std::thread server_thread;
    std::map<std::string, std::unique_ptr<Session>, std::less<>> sessions_;
    std::map<std::string, std::unique_ptr<Engine::Game>> games_;
};
} // namespace Server
