#include <thread>

#include "spdlog_wrap.h"
#include "rapidjson/document.h"

#include "DB/DB.h"
#include "Utils/Utils.h"

#include "Server.h"

namespace Server
{
void Server::Start()
{
    server = std::make_unique<HttpServer>();
    server->config.port = 8080;
    server->resource["^/about$"]["GET"] = [&](std::shared_ptr<HttpServer::Response> response,
                                              std::shared_ptr<HttpServer::Request> request) {
        std::stringstream stream;
        stream << "<h1>Request from " << request->remote_endpoint().address().to_string() << ":"
               << request->remote_endpoint().port() << "</h1>";

        stream << request->method << " " << request->path << " HTTP/" << request->http_version;

        stream << "<h2>Query Fields</h2>";
        auto query_fields = request->parse_query_string();
        for (auto& field : query_fields)
            stream << field.first << ": " << field.second << "<br>";
        stream << "<h2>Header Fields</h2>";
        for (auto& field : request->header)
            stream << field.first << ": " << field.second << "<br>";
        response->write(stream);
    };

    server->resource["^/login$"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
                                               std::shared_ptr<HttpServer::Request> request) {
        login(response.get(), request.get());
    };

    server_thread = std::thread([&]() {
        // Start server
        server->start();
    });
}

void Server::Stop()
{
    server->stop();
    server_thread.join();
}

void Server::login(HttpServer::Response* response, HttpServer::Request* request)
{
    rapidjson::Document d;
    d.Parse(request->content.string());
    if (d.HasParseError()) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request);
    }
    auto loginO = Utils::GetT<std::string>(d, "login");
    if (!loginO) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request);
    }
    std::stringstream query;
    query << "{\"login\": \"" << *loginO << "\"}";
    auto& db = DB::DB::Instance();
    auto userInfoStr = db.Get("users", query.str());
    if (userInfoStr == "") {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized);
    }
    auto id = createSession();
    std::stringstream res;
    res << "{\"session_id\": \"" << id << "\"}";
    response->write(res);
}

Session* Server::getSession(std::string_view id)
{
    auto res = sessions_.find(id);
    if (res == sessions_.end()) {
        return nullptr;
    }
    return res->second.get();
}

std::string Server::createSession()
{
    auto res = Utils::MakeUUID();
    sessions_[res] = std::make_unique<Session>();
    sessions_[res]->id = res;
    sessions_[res]->expiration = std::chrono::system_clock::now() + std::chrono::minutes(10);
    return res;
}

} // namespace Server
