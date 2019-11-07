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
#include "simple-web-server/client_http.hpp"
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
#ifdef FORCE_HTTP
    using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
#else
    using HttpServer = SimpleWeb::Server<SimpleWeb::HTTPS>;
#endif

public:
    void Start();
    void Stop();

private:
    std::pair<SimpleWeb::StatusCode, std::string> login(HttpServer::Request* request);
    std::pair<SimpleWeb::StatusCode, std::string> ping(HttpServer::Request* request);
    std::pair<SimpleWeb::StatusCode, std::string> createGame(HttpServer::Request* request);
    std::pair<SimpleWeb::StatusCode, std::string> deleteGame(HttpServer::Request* request);
    std::pair<SimpleWeb::StatusCode, std::string> joinGame(HttpServer::Request* request);
    std::pair<SimpleWeb::StatusCode, std::string> listGames(HttpServer::Request* request);
    std::pair<SimpleWeb::StatusCode, std::string> startGame(HttpServer::Request* request);
    std::pair<SimpleWeb::StatusCode, std::string> queryGame(HttpServer::Request* request);
    void makeTurn(HttpServer::Response* response, HttpServer::Request* request);
    void lastUpdate(HttpServer::Response* response, HttpServer::Request* request);
    std::pair<SimpleWeb::StatusCode, std::string> undo(HttpServer::Request* request);
    Session* getSession(std::string_view id);
    std::string createSession(const bsoncxx::document::view& userInfo);
    void retireSessions();
    void extendSession(Session* session);
    Session* validateRequest(HttpServer::Response* response, HttpServer::Request* request, rapidjson::Document& d);
    Session* validateRequest(HttpServer::Response* response, bsoncxx::document::view& d);
    Session* validateRequest(bsoncxx::document::view& d);
    Engine::Game* findGame(std::string& id);
    std::optional<bsoncxx::document::value> tryParseRequest(HttpServer::Request* request);

    void dumpGame(Engine::Game* g, bool pushState = false);

    std::unique_ptr<HttpServer> server = nullptr;
    std::thread server_thread;
    std::map<std::string, std::unique_ptr<Session>, std::less<>> sessions_;
    std::map<std::string, std::unique_ptr<Engine::Game>> games_;
    SimpleWeb::CaseInsensitiveMultimap corsHeader_ = {{"Access-Control-Allow-Origin", "*"}};
};
} // namespace Server
